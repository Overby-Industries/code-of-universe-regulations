// cur_state_machine.h — the FSM engine.
//
// Deterministic by construction:
//   - no wall clock in the evaluation path (logical tick/sequence only)
//   - no randomness
//   - no iteration over unordered containers during evaluation
//   - no dependence on address values or allocation order
//   - guards are pure predicates over a POD context
//
// Same regulation set + same event stream => same states, same faults, same
// audit trail (modulo the log's wall-clock metadata column). tests/ asserts it.
//
// Order of operations for every submitted event, and it does not vary:
//
//   1. validate       FOUNDATION-005 §9   malformed events are rejected
//   2. forbidden?     PDDC §12.3          checked BEFORE any table lookup
//        yes -> fault handler, §12.4, four steps, no discretion, return
//   3. table lookup   first matching row with satisfied guards wins
//        none -> refuse, log Class I, state unchanged
//   4. apply          update state, refresh last-known-safe if fault-free
//   5. record         FOUNDATION-002 §9, always, accepted or refused
//   6. notify         observers, after the record exists

#ifndef CUR_STATE_MACHINE_H
#define CUR_STATE_MACHINE_H

#include <cstdint>
#include <string>
#include <vector>

#include "cur_capture_index.h"
#include "cur_entity.h"
#include "cur_event.h"
#include "cur_event_log.h"
#include "cur_obligation.h"
#include "cur_regulation.h"
#include "cur_state.h"
#include "cur_violation.h"

namespace cur {

// What a single evaluation did.
struct TransitionResult {
    bool accepted = false;
    bool fault_raised = false;
    bool entered_protected_mode = false;
    bool reverted = false;

    RecordAxis axis = REC_AXIS_NONE;
    StateVector before;
    StateVector after;

    EventType trigger = EV_NONE;
    FaultClass fault = FC_NONE;
    ForbiddenState forbidden = FS_NONE;

    uint16_t guards_required = guard::NONE;
    uint16_t guards_satisfied = guard::NONE;

    const char* citation = "";
    std::string reason;

    uint64_t record_seq = 0;  // index into the audit trail
};

// Fault detail handed to observers — the machine-readable form of the Fault
// Declaration Notice in PDDC-GOVERNANCE-MECHANICS §12.3(d).
struct FaultRecord {
    FaultClass fault_class = FC_NONE;
    ForbiddenState forbidden = FS_NONE;
    EntityHandle entity = INVALID_ENTITY;
    std::string entity_id;
    EventType trigger = EV_NONE;
    uint64_t tick = 0;
    StateVector reverted_to;
    std::string citation;
    std::string description;
};

// ---------------------------------------------------------------------------
// Observer interface — the AI integration point
// ---------------------------------------------------------------------------
// Implement this to receive callbacks. The Godot bridge implements it and
// re-emits as signals; a CI harness, a logging sink, or an external agent can
// implement it just as well. All methods default to no-op so an implementer
// only overrides what it cares about.
//
// Observers are called AFTER the audit record exists, so an observer that
// throws or misbehaves cannot prevent a transition from being recorded.
// Observers must not mutate the machine; doing so during a callback is
// undefined and would break replay determinism.
class ICURObserver {
public:
    virtual ~ICURObserver() = default;

    virtual void on_transition(const TransitionResult& /*r*/,
                               const EntityRecord& /*e*/) {}
    virtual void on_refusal(const TransitionResult& /*r*/,
                            const EntityRecord& /*e*/) {}
    virtual void on_fault(const FaultRecord& /*f*/) {}
    virtual void on_certification(const EntityRecord& /*e*/, bool /*granted*/) {}
    virtual void on_protected_mode(const EntityRecord& /*e*/, bool /*entered*/,
                                   const FaultRecord& /*cause*/) {}
    virtual void on_amendment(const AmendmentProposal& /*p*/,
                              const AmendmentResult& /*r*/) {}
    virtual void on_capture_risk(double /*cri*/, CaptureRiskBand /*band*/) {}

    // An obligation matured unmet. Raised by run_builtin_test(), which is
    // driven by the clock rather than by any submitted event — see
    // cur_obligation.h for why this could not be a guard.
    virtual void on_obligation_lapsed(const Obligation& /*o*/,
                                      FaultClass /*severity*/) {}
};

// ---------------------------------------------------------------------------
// The engine
// ---------------------------------------------------------------------------
class CURStateMachine {
public:
    CURStateMachine();
    explicit CURStateMachine(RegulationSet regulations);

    // --- configuration ----------------------------------------------------
    RegulationSet& regulations() { return regulations_; }
    const RegulationSet& regulations() const { return regulations_; }

    EntityRegistry& entities() { return entities_; }
    const EntityRegistry& entities() const { return entities_; }

    EventLog& log() { return log_; }
    const EventLog& log() const { return log_; }

    // ENTITY-009/010/011 records produced by this machine. The audit log says
    // what transitioned; the ledger says what the breach WAS, in the shape
    // FOUNDATION-004 defines. Records cross-reference each other by id.
    ViolationLedger& ledger() { return ledger_; }
    const ViolationLedger& ledger() const { return ledger_; }

    // Obligations that come due on a schedule — CUR-H.6 §6.8(a), CUR-H.7
    // §7.12(c)(3), CUR-FOUNDATION-010 §5, CUR-N.5 §5.2B.
    ObligationRegister& obligations() { return obligations_; }
    const ObligationRegister& obligations() const { return obligations_; }

    // The built-in test. Checks every open obligation against `tick`, logs each
    // lapse with the fault class its kind carries, opens a violation record
    // against the party that owed it, and notifies observers. Returns the
    // number of lapses found.
    //
    // This is the one entry point in the library not driven by a submitted
    // event. It exists because the failure it detects is that nothing was
    // submitted, which no guard can see. CUR-N.5 §5.2B calls it a routine
    // system check and that is exactly what it is: it runs because the clock
    // advanced, not because anyone suspected a fault.
    //
    // A lapse is a fault against `owed_by` and never against `concerning`.
    // CUR-H.7 §7.11(c) forbids a measure attaching to the being an obligation
    // protects, and a being left waiting for an investigation is the last party
    // who should acquire a record from the waiting.
    size_t run_builtin_test(uint64_t tick);

    CaptureRiskModel& capture_model() { return capture_model_; }

    // Strict mode refuses any event whose target is not registered, instead of
    // registering it implicitly. FOUNDATION-004 §19 argues for leaving this on.
    void set_strict_entities(bool strict) { strict_entities_ = strict; }
    bool strict_entities() const { return strict_entities_; }

    // --- observers --------------------------------------------------------
    // Not owned. Registration order is preserved and notification follows it,
    // so observer callbacks are deterministic too.
    void add_observer(ICURObserver* o);
    void remove_observer(ICURObserver* o);
    void clear_observers();

    // --- the step path ----------------------------------------------------
    // The only way to change any state in this library.
    TransitionResult submit(const Event& e);

    // Convenience for the common simulator case: an operational event against
    // one licence. Fills in the Event and calls submit().
    TransitionResult submit_operational(EntityHandle target, EventType type,
                                        const TransitionContext& ctx,
                                        uint64_t tick);

    // --- queries (all const, all side-effect free) ------------------------
    StateVector state_of(EntityHandle h) const;
    ComplianceState compliance_of(EntityHandle h) const;
    ConstitutionalState constitutional_of(EntityHandle h) const;
    GovernanceState governance_of(EntityHandle h) const;
    bool in_protected_mode(EntityHandle h) const;

    // Whole-machine Protected Mode: true when any entity is in it.
    bool any_protected_mode() const;
    size_t protected_mode_count() const;

    // Would this event be accepted, and to what? Runs the same lookup as
    // submit() and changes nothing — no state, no log, no observers.
    TransitionResult dry_run(const Event& e) const;

    // --- Protected Mode recovery -----------------------------------------
    // PDDC §12.5(d): exit requires Constitutional Court certification of all
    // three conditions. Passing anything but true for all three is refused and
    // logged; there is no override.
    bool certify_recovery(EntityHandle h, bool instrument_remediated,
                          bool safe_state_verified, bool no_further_risk,
                          uint64_t tick, const std::string& certificate_id);

    // --- amendments (AI contributor path) ---------------------------------
    // Validates and, if accepted, applies. Always logged either way.
    AmendmentResult propose_amendment(const AmendmentProposal& p, uint64_t tick);

    // Validation only — no application, no log, no observers. Lets a
    // contributor check an idea before proposing it.
    AmendmentResult validate_amendment(const AmendmentProposal& p) const;

    // The live compliance table: the constexpr baseline plus accepted
    // amendments. Rows added by amendment are appended, so baseline rows keep
    // their priority.
    const std::vector<ComplianceTransition>& compliance_table() const {
        return table_;
    }

    // --- capture risk -----------------------------------------------------
    // Recomputes CRI, notifies observers, and returns the score. Emits
    // EV_CAPTURE_RISK_THRESHOLD_EXCEEDED into the log on a band increase.
    double update_capture_risk(const CaptureRiskInputs& in, uint64_t tick);
    double capture_risk() const { return capture_risk_; }
    CaptureRiskBand capture_risk_band() const;

    // --- vital continuity -------------------------------------------------
    // Recomputes VCI and logs a band CHANGE in either direction — degradation
    // and recovery both matter for continuity, unlike capture risk where only
    // the worsening direction triggers a response.
    //
    // A critical score (<20) drives the affected entity into STATE-010 Vital
    // Continuity Response with no second condition and no approval, per
    // FOUNDATION-002 §4 and FOUNDATION-013.
    //
    // Mind the direction: VCI 100 is healthy, VCI 0 is collapse. Opposite to
    // capture_risk(). See cur_capture_index.h.
    double update_vital_continuity(EntityHandle h,
                                   const VitalContinuityInputs& in,
                                   uint64_t tick);
    double vital_continuity() const { return vital_continuity_; }
    VitalContinuityBand vital_continuity_band() const;
    VitalContinuityModel& continuity_model() { return continuity_model_; }

    // --- lifecycle --------------------------------------------------------
    // Clears entities, log, amendments, and CRI; keeps regulations and
    // observers. Use between simulator runs.
    void reset();

private:
    // Step 2 — PDDC §12.3, declared forbidden transitions. Runs before the
    // table is consulted, because an asserted forbidden transition must fault
    // whether or not any row would have matched it.
    ForbiddenState classify_forbidden(const Event& e,
                                      std::string* out_citation) const;

    // Step 4 — structural check on the row the table actually selected. Faults
    // if that row would move a subject which is not an operational
    // authorisation into SUSPENDED or BLACKLISTED.
    //
    // Deliberately checks the SELECTED row rather than the first row matching
    // from/trigger. Those differ, and the difference is the point: a sanction
    // against a being skips the SUSPENDED row on its guard and lands on the
    // ENTITY-010 fallback, which is lawful governance and must not fault. Only
    // a row that survived its guards and still targets SUSPENDED or
    // BLACKLISTED represents a real attempt, which in practice means a guard
    // was dropped by amendment or by hand-editing the table.
    ForbiddenState structural_forbidden(const ComplianceTransition& row,
                                        const EntityRecord& target,
                                        std::string* out_citation) const;

    // Step 3 — first matching row with satisfied guards. Null when none match.
    const ComplianceTransition* lookup(ComplianceState from, EventType trigger,
                                       uint16_t satisfied,
                                       uint16_t extra_required) const;

    // Any row matching from/trigger regardless of guards, so a refusal can
    // report which guards were missing rather than just "no".
    const ComplianceTransition* lookup_ignoring_guards(ComplianceState from,
                                                       EventType trigger) const;

    // Step 2 continued — PDDC §12.4, four steps, no discretion.
    void run_fault_handler(EntityRecord& target, const Event& e,
                           ForbiddenState forbidden, FaultClass fc,
                           const std::string& citation, TransitionResult& out);

    // Reachability check used by validate_amendment().
    bool opens_forbidden_path(const std::vector<ComplianceTransition>& table,
                              ForbiddenState* out_which,
                              std::string* out_why) const;

    // Resolve guards for an event against a target, forcing subject_is_license
    // from the registry rather than trusting the caller.
    uint16_t satisfied_guards(const Event& e, const EntityRecord& target) const;

    // Fill the subject, actor, and full-state-vector columns on a record.
    // Every LogRecord this class writes goes through here, so CTAF §14 and
    // CAPS §17 cannot be satisfied at one log site and forgotten at another.
    void stamp_record(LogRecord& r, const EntityRecord& e,
                      EntityHandle actor) const;

    void notify_transition(const TransitionResult& r, const EntityRecord& e);
    void notify_refusal(const TransitionResult& r, const EntityRecord& e);
    void notify_fault(const FaultRecord& f);

    // Mint the ENTITY-009 record behind a transition into KS_VIOLATION, and
    // the ENTITY-010 record behind an applied sanction.
    std::string record_violation(const EntityRecord& target, const Event& e,
                                 FaultClass fc, ForbiddenState forbidden,
                                 const std::string& regulation_id,
                                 const std::string& citation,
                                 uint64_t log_seq);
    std::string record_sanction(const EntityRecord& target, const Event& e,
                                bool affected_compliance_axis,
                                const std::string& citation, uint64_t log_seq);

    RegulationSet regulations_;
    EntityRegistry entities_;
    EventLog log_;
    ViolationLedger ledger_;
    ObligationRegister obligations_;
    CaptureRiskModel capture_model_;

    std::vector<ComplianceTransition> table_;  // baseline + amendments
    std::vector<ICURObserver*> observers_;

    double capture_risk_ = 0.0;
    CaptureRiskBand capture_risk_band_ = CRB_STABLE;

    // Starts at 100.0, not 0.0. On this axis zero means total life-support
    // collapse, so a default-constructed machine must start healthy.
    double vital_continuity_ = 100.0;
    VitalContinuityBand vital_continuity_band_ = VCB_STABLE;
    VitalContinuityModel continuity_model_;

    bool strict_entities_ = true;
};

}  // namespace cur

#endif  // CUR_STATE_MACHINE_H
