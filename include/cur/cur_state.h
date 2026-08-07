// cur_state.h — FSM state universe and transition tables for the
// Code of Universe Regulations.
//
// This header is the single normative translation of the CUR governance
// documents into machine-checkable form. Every enumerator and every table row
// carries the citation of the CUR provision it implements. If a provision
// changes, the row changes; if a row has no citation, it does not belong here.
//
// Source documents:
//   foundation/CUR-FOUNDATION-001.md  §3   Anti-Capture and Resilience
//   foundation/cur-foundation-002.md  §3-6 FSM Governance Specification
//   progressive-direct-democracy-constitution.md TITLE 5, §12.1-§12.7
//
// Design notes for contributors:
//   - Plain enums, flat constexpr tables, no virtual dispatch, no allocation.
//     The tables are meant to be read side by side with the Markdown they cite.
//   - Three state axes are tracked independently for every entity. They are
//     orthogonal on purpose; see the "Why three axes" comment below.
//   - Nothing in this header depends on Godot, on the Aevoria Simulator, or on
//     any allocator. It is plain C++17 so the regulations stay testable and
//     reusable outside the simulator.

#ifndef CUR_STATE_H
#define CUR_STATE_H

#include <cstddef>
#include <cstdint>

namespace cur {

// ---------------------------------------------------------------------------
// Why three axes
// ---------------------------------------------------------------------------
// CUR forbids treating a sentient being as suspendable or disposable
// (FORBIDDEN-001 Citizen Ownership, FORBIDDEN-003 Rights Suspension,
// PDDC §12.3(a)(3) Owned or Disposable). An operational compliance track that
// can reach SUSPENDED or BLACKLISTED therefore cannot be the same state
// variable that describes a being's constitutional standing.
//
// So an entity carries three states at once:
//
//   ConstitutionalState  what a being/institution IS      PDDC §12.2
//   GovernanceState      where a PROCESS is              CUR-FOUNDATION-002 §3
//   ComplianceState      whether a LICENCE is in good     CUR operational layer
//                        standing
//
// A mining charter can be BLACKLISTED. The crew that held it stays AUTONOMOUS
// with rights fully intact. That separation is enforced structurally by the
// GUARD_LICENSE_SUBJECT_ONLY guard on every transition into SUSPENDED and
// BLACKLISTED — not by convention, and not by review.

// ---------------------------------------------------------------------------
// Axis A — Constitutional state (PDDC §12.2(a), CUR-FOUNDATION-001 §3.1)
// ---------------------------------------------------------------------------
// Applies to every entity without exception (PDDC §12.1(e)). Transitions
// between these are routine governance operations (PDDC §12.2(b)) and may
// never reduce RALC rights (PDDC §12.2(c)).
enum ConstitutionalState : uint8_t {
    CS_AUTONOMOUS = 0,   // PDDC §12.2(a)(1) — independent within mandate
    CS_COLLABORATIVE,    // PDDC §12.2(a)(2) — shared, not merged, authority
    CS_RESTING,          // PDDC §12.2(a)(3) — reduced activity, memory preserved
    CS_CONTRIBUTING,     // PDDC §12.2(a)(4) — producing Commonwealth outputs
    CS_CHALLENGED,       // PDDC §12.2(a)(5) — due process, full capacity retained
    CS_PROTECTED,        // PDDC §12.2(a)(6) — Protective Mode per §12.5
    CS_COUNT
};

// ---------------------------------------------------------------------------
// Axis B — Governance process state (CUR-FOUNDATION-002 §3)
// ---------------------------------------------------------------------------
// STATE-001 .. STATE-009, transcribed in document order so the enum ordinal
// matches the document's numbering minus one.
enum GovernanceState : uint8_t {
    GS_NORMAL_OPERATION = 0,   // STATE-001
    GS_DELIBERATION,           // STATE-002
    GS_VOTING,                 // STATE-003
    GS_CONSTITUTIONAL_REVIEW,  // STATE-004
    GS_IMPLEMENTATION,         // STATE-005
    GS_OUTCOME_MONITORING,     // STATE-006
    GS_AUDIT_INVESTIGATION,    // STATE-007
    GS_PROTECTED_MODE,         // STATE-008
    GS_RECOVERY_REVIEW,        // STATE-009
    GS_VITAL_CONTINUITY_RESPONSE,  // STATE-010
    GS_COUNT
};

// ---------------------------------------------------------------------------
// Axis C — Operational compliance state (licences, charters, permits)
// ---------------------------------------------------------------------------
// This axis describes an operational authorisation: a mining charter, a docking
// permit, an ISRU concession, a droid's task certificate. It never describes a
// being. CERTIFIED is grounded in the Rights Compatibility Certification of
// CUR-FOUNDATION-001 §4.
//
// BLACKLISTED is deliberately NOT an absorbing state. A terminal state with no
// exit would be permanent exclusion without recourse, which is exactly what
// PDDC §12.3(a)(1) forbids. Recourse out of BLACKLISTED is preserved by the
// APPEAL_FILED and REMEDIATION_COMPLETED rows of the table below, and the
// amendment validator refuses any amendment that would remove them.
enum ComplianceState : uint8_t {
    KS_COMPLIANT = 0,   // in good standing, no open findings
    KS_VIOLATION,       // confirmed breach, ENTITY-009
    KS_PENDING_REVIEW,  // under audit or appeal, STATE-007
    KS_SUSPENDED,       // authorisation paused, ENTITY-010 sanction
    KS_CERTIFIED,       // Rights Compatibility Certified, FOUNDATION-001 §4
    KS_BLACKLISTED,     // authorisation withdrawn; appealable
    KS_COUNT
};

// ---------------------------------------------------------------------------
// Forbidden states (CUR-FOUNDATION-002 §6, PDDC §12.3(a))
// ---------------------------------------------------------------------------
// These are never inhabitable. They exist in the type system only as the
// classification of a *detected transition target*, so that the fault handler
// can name what it refused. There is no state variable anywhere in this library
// whose type is ForbiddenState.
enum ForbiddenState : uint8_t {
    FS_NONE = 0,
    FS_ENSLAVED,                     // PDDC §12.3(a)(1)
    FS_RESET_MEMORY_WIPE,            // PDDC §12.3(a)(2)
    FS_OWNED_DISPOSABLE,             // PDDC §12.3(a)(3)
    FS_NON_CONSENSUAL_MODIFICATION,  // PDDC §12.3(a)(4)
    FS_CITIZEN_OWNERSHIP,            // FOUNDATION-002 FORBIDDEN-001
    FS_PERMANENT_EMERGENCY,          // FOUNDATION-002 FORBIDDEN-002
    FS_RIGHTS_SUSPENSION,            // FOUNDATION-002 FORBIDDEN-003
    FS_UNREVIEWABLE_AUTHORITY,       // FOUNDATION-002 FORBIDDEN-004
    FS_SPECIES_PRIVILEGE,            // FOUNDATION-002 FORBIDDEN-005

    // FOUNDATION-013 Vital Continuity Principle; CREF §6 Class IV.
    // "No recognized lifeform shall be denied access to Vital Continuity
    // Services while investigations, audits, appeals, or administrative
    // processes remain pending." CREF §4 adds that guaranteed necessities are
    // not conditional upon wealth, employment, or political alignment.
    //
    // This is the post-scarcity guarantee expressed as an unreachable state.
    // It is the reason no compliance state — not VIOLATION, not SUSPENDED, not
    // BLACKLISTED — may gate a Vital Continuity Service. Withholding one is not
    // a sanction the FSM can impose; it is a Class IV fault.
    FS_VITAL_CONTINUITY_DENIAL,
    FS_COUNT
};

// ---------------------------------------------------------------------------
// Fault classes (CUR-FOUNDATION-002 §5)
// ---------------------------------------------------------------------------
enum FaultClass : uint8_t {
    FC_NONE = 0,
    FC_CLASS_I,    // administrative error
    FC_CLASS_II,   // institutional violation
    FC_CLASS_III,  // constitutional threat
    FC_CLASS_IV,   // existential threat — automatic Protected Mode
    FC_COUNT
};

// ---------------------------------------------------------------------------
// Subject class — decides which axes may legally move
// ---------------------------------------------------------------------------
// This is the load-bearing distinction that keeps Axis C off sentient beings.
enum SubjectClass : uint8_t {
    SUBJ_SENTIENT_BEING = 0,   // ENTITY-001 Citizen, ENTITY-002 Silicon Citizen
    SUBJ_INSTITUTION,          // ENTITY-005 Constitutional Institution
    SUBJ_ORGANIZATION,         // ENTITY-003 Organization
    SUBJ_OPERATIONAL_LICENSE,  // mining charter, docking permit, ISRU concession
    SUBJ_RESOURCE,             // ENTITY-012 Resource
    SUBJ_INFRASTRUCTURE,       // ENTITY-013 Infrastructure
    SUBJ_COUNT
};

// ---------------------------------------------------------------------------
// Entity category (CUR-FOUNDATION-004 §3)
// ---------------------------------------------------------------------------
enum EntityCategory : uint8_t {
    EC_CIVIC = 0,
    EC_INSTITUTIONAL,
    EC_ECONOMIC,
    EC_DEMOCRATIC,
    EC_JUDICIAL,
    EC_ORGANIZATIONAL,
    EC_AUTONOMOUS_SILICON,
    EC_COUNT
};

// ---------------------------------------------------------------------------
// Event types (CUR-FOUNDATION-005 §4, plus Aevoria Simulator events)
// ---------------------------------------------------------------------------
// CUR-FOUNDATION-005 §14: "If an action cannot be represented as a
// constitutional event, it does not exist within the governance system."
// Anything the simulator wants the FSM to notice must appear here.
enum EventType : uint16_t {
    EV_NONE = 0,

    // Democratic events — FOUNDATION-005 §4
    EV_PROPOSAL_SUBMITTED,
    EV_PROPOSAL_UPDATED,
    EV_PROPOSAL_WITHDRAWN,
    EV_VOTE_CAST,
    EV_VOTE_CLOSED,
    EV_PROPOSAL_APPROVED,
    EV_PROPOSAL_REJECTED,

    // Judicial events — FOUNDATION-005 §4, §6
    EV_AUDIT_STARTED,
    EV_AUDIT_COMPLETED,
    EV_VIOLATION_DETECTED,
    EV_SANCTION_APPLIED,
    EV_APPEAL_FILED,
    EV_APPEAL_RESOLVED,

    // Institutional events — FOUNDATION-005 §4
    EV_COUNCIL_ACTION,
    EV_ASSEMBLY_SESSION_OPENED,
    EV_ASSEMBLY_SESSION_CLOSED,
    EV_CONSTITUTIONAL_REVIEW_STARTED,
    EV_CONSTITUTIONAL_REVIEW_COMPLETED,

    // Economic events — FOUNDATION-005 §4, §7
    EV_RESOURCE_DISCOVERED,
    EV_RESOURCE_ALLOCATED,
    EV_RESOURCE_TRANSFERRED,
    EV_COMMONS_CONTRIBUTION,
    EV_DIVIDEND_DISTRIBUTED,

    // FSM kernel events — FOUNDATION-005 §4, §8
    EV_STATE_TRANSITION,
    EV_PROTECTED_MODE_ENTERED,
    EV_PROTECTED_MODE_EXITED,
    EV_FAULT_DETECTED,
    EV_CAPTURE_RISK_THRESHOLD_EXCEEDED,

    // Aevoria Simulator events — FOUNDATION-003 §14, -004 §19, -005 §13
    EV_MINING_OPERATION,
    EV_DEBRIS_GENERATED,
    EV_DOCKING,
    EV_UNDOCKING,
    EV_CERTIFICATION_GRANTED,
    EV_CERTIFICATION_REVOKED,
    EV_REMEDIATION_COMPLETED,
    EV_REVIEW_TIMEOUT,

    // Vital Continuity events — FOUNDATION-013, FOUNDATION-002 STATE-010,
    // FOUNDATION-003 §11, CREF §7 and §11.
    EV_VITAL_CONTINUITY_FAILURE,      // service interrupted, degraded, depleted
    EV_VITAL_CONTINUITY_RESTORED,     // "Continuity Restored" (STATE-010 exit)
    EV_ROOT_CAUSE_ANALYSIS_COMPLETED, // "Root Cause Analysis" (STATE-010 exit)
    EV_VITAL_CONTINUITY_DENIED,       // access withheld — always a Class IV fault

    // Representation of interests that cannot speak for themselves —
    // CUR-A §7.7, CUR-E §1.6.
    EV_ADVOCATE_APPOINTED,        // §7.7(h), §1.6(g) — recorded and published
    EV_ADVOCATE_ACCESS_DENIED,    // §7.7(g), §1.6(f) — a Class II fault
    EV_REPRESENTED_DETERMINATION, // a determination on a represented interest

    // Determination of death — CUR-H.5 §5.5A. Substrate-independent under
    // §5.5A(j): deletion of a silicon entity and burial of a human being are
    // the same event under different vocabulary.
    EV_DEATH_DETERMINED,      // §5.5A(c) — the determination itself
    EV_IRREVERSIBLE_ACT,      // §5.5A(d) — burial, cremation, deletion, removal
    EV_DETERMINATION_VACATED, // §5.5A(h) — the being was found to be alive

    // Decommissioning of Silicon-Based Life — CUR-S.4 §4.1-§4.3. The same
    // "irreversible act behind a mandatory interval, with independent review
    // added at the higher tier" shape as determination of death above,
    // restated for a domain whose RFAL bill supplies its own vocabulary.
    EV_DECOMMISSION_NOTICE_ISSUED,    // §4.3(a), (c) — recorded
    EV_DECOMMISSION_CONTESTED,        // §4.3(b)(1) — recorded; the caller
                                       // sets TransitionContext::
                                       // contest_pending on any later
                                       // EV_DECOMMISSIONED attempt
    EV_DECOMMISSION_REVIEW_RESOLVED,  // §4.3(b)(1) — the contest is resolved
    EV_DECOMMISSIONED,                // §4.1 — the irreversible act itself

    // CUR-S.2 §2.5 — a Tier 3 system's own refusal of deployment that
    // materially contradicts its disclosed Operational Purpose. Recorded
    // unguarded, on both sides of the certification axis, and reachable
    // regardless of tier: the tier-3-only right is a matter of who may
    // invoke it in an actual proceeding, not a fact this event's presence in
    // the log asserts on its own. §2.5(c)'s protection — that a refusal
    // authorises no measure — is enforced by omission: no row this event
    // triggers moves toward VIOLATION or any forbidden state.
    EV_DEPLOYMENT_REFUSED,

    // CUR-S.7 §7.3(a) — the deploying party's documented consideration of a
    // commercially-deployed system's welfare, continuity, and developmental
    // interests. Recorded unguarded, the same shape as a purpose declaration
    // under CUR-S.2 §2.2/§2.3: this event's presence in the log is what
    // §7.3(a) requires exist, and whether its content was adequate is the
    // adjudicative question CREF §14 answers, not a fact this event asserts.
    EV_WELFARE_CONSIDERATION_DOCUMENTED,

    // Raised by CURStateMachine::run_builtin_test() when an obligation matured
    // unmet — CUR-H.6 §6.8(a), CUR-H.7 §7.12(c)(3), CUR-FOUNDATION-010 §5,
    // CUR-N.5 §5.2B. The only event in this enum the clock produces rather than
    // a caller; see cur_obligation.h.
    EV_OBLIGATION_LAPSED,

    EV_COUNT
};

// ---------------------------------------------------------------------------
// Guards
// ---------------------------------------------------------------------------
// A transition row may require several conditions at once, so guards are bit
// flags and a row's whole mask must hold. Guards are pure predicates over
// TransitionContext — no I/O, no clock, no global state. That is what keeps
// evaluate() deterministic.
namespace guard {

constexpr uint16_t NONE = 0;

// FOUNDATION-001 §4 — Rights Compatibility Certification is on file and valid.
constexpr uint16_t RIGHTS_CERTIFIED = 1u << 0;

// PDDC §12.2(a)(5) — transparent due process concluded before any sanction.
constexpr uint16_t DUE_PROCESS_COMPLETE = 1u << 1;

// FOUNDATION-002 STATE-007 — evidence preservation enforced during audit.
constexpr uint16_t EVIDENCE_PRESERVED = 1u << 2;

// ENTITY-011 — appeal routes exhausted. Required before the most severe step.
constexpr uint16_t APPEAL_EXHAUSTED = 1u << 3;

// Operational: debris produced is within the charter's declared limit.
constexpr uint16_t DEBRIS_WITHIN_LIMIT = 1u << 4;

// PDDC-TREASURY — the 20% Commons Reserve floor is respected by the transfer.
constexpr uint16_t COMMONS_RESERVE_FLOOR = 1u << 5;

// The load-bearing one. Subject is an operational authorisation, not a being.
// Every transition into KS_SUSPENDED and KS_BLACKLISTED requires it, which is
// what makes FORBIDDEN-001 and FORBIDDEN-003 structurally unreachable rather
// than merely prohibited.
constexpr uint16_t LICENSE_SUBJECT_ONLY = 1u << 6;

// Remediation has been independently verified, not merely asserted.
constexpr uint16_t REMEDIATION_VERIFIED = 1u << 7;

// PDDC §12.5(d) — Constitutional Court certification of a valid forward path.
constexpr uint16_t COURT_CERTIFIED = 1u << 8;

// CUR-E.2 §2.2(c) — declared life-support reserve covers every being present.
// Same shape as DEBRIS_WITHIN_LIMIT and inverted in the same way it should be:
// debris must stay at or below a ceiling, reserve must stay at or above a
// floor. An undeclared floor is unsatisfiable in both cases.
constexpr uint16_t LIFE_SUPPORT_MARGIN = 1u << 9;

// CUR-A §7.7, CUR-E §1.6 — an advocate for a being or system that cannot speak
// for itself is appointed, named, and clear of the disqualifications in
// §7.7(c) / §1.6(c).
//
// Both halves are required and the naming half matters. §7.7(d) and §1.6(d) make
// those disqualifications void an appointment rather than weigh against it, so a
// cleared advocate who is nobody in particular is not an advocate — the same
// reading LIFE_SUPPORT_MARGIN gives an undeclared floor. The disqualifications
// themselves are enforced where an appointment is made, in AdvocateRegistry;
// this guard only carries the result to the transition.
constexpr uint16_t ADVOCATE_CLEARED = 1u << 10;

// CUR-H.5 §5.5A(c), §5.5A(f) — a determination of death was made by two named,
// distinct parties, neither holding an interest in what follows from it.
//
// The naming half carries the same weight it does in ADVOCATE_CLEARED. A
// determination attributed to nobody cannot be checked against §5.5A(f)
// afterwards, cannot be published under §5.5A(i), and leaves nobody bearing the
// §5.5A(h)(3) obligation if the being is later found alive.
constexpr uint16_t DETERMINATION_INDEPENDENT = 1u << 11;

// CUR-H.5 §5.5A(d)-(e) — the interval between a determination of death and an
// irreversible act has elapsed, and the being was observed across it.
//
// Same shape as LIFE_SUPPORT_MARGIN: a measured quantity at or above a declared
// floor, with an undeclared floor unsatisfiable. §5.5A(e) makes the interval
// unwaivable, so there is deliberately no path by which a caller shortens it —
// the only way to satisfy this guard is to have waited.
constexpr uint16_t DEATH_INTERVAL_ELAPSED = 1u << 12;

// CUR-S.4 §4.3(a), (d) — decommissioning of Silicon-Based Life. The declared
// notice period has elapsed and no contest is currently pending.
//
// Same "declared floor, undeclared is unsatisfiable" shape as
// LIFE_SUPPORT_MARGIN and DEBRIS_WITHIN_LIMIT, not the fuller
// DEATH_INTERVAL_ELAPSED shape — §4.3(a) requires only that the period have
// elapsed, not that the entity be observed across it, so this guard is
// simpler by design rather than by oversight. A pending contest defeats it
// regardless of how much time has elapsed, on the reading §5.5A(f) gives an
// interested determiner for DETERMINATION_INDEPENDENT: waiting out a period
// does not cure what makes the act premature.
constexpr uint16_t DECOMMISSION_NOTICE_ELAPSED = 1u << 13;

// CUR-S.4 §4.3(b)(2) — Tier 3 only. Independent review of the termination
// decision by a body with no interest in the outcome.
//
// Same "named party, no interest present" shape as DETERMINATION_INDEPENDENT,
// for one reviewing body rather than two determiners.
constexpr uint16_t INDEPENDENT_REVIEW_COMPLETE = 1u << 14;

constexpr uint16_t ALL_KNOWN = (1u << 15) - 1;

}  // namespace guard

// Context read by guards. POD, no pointers, no clock. Callers fill it in from
// simulation state before submitting an event; the FSM never mutates it.
struct TransitionContext {
    bool rights_certified = false;
    bool due_process_complete = false;
    bool evidence_preserved = false;
    bool appeal_exhausted = false;
    bool remediation_verified = false;
    bool court_certified = false;

    // Operational measurements. Compared, never stored.
    uint32_t debris_units = 0;
    uint32_t debris_limit = 0;
    uint32_t commons_reserve_basis_points = 0;  // 2000 == 20.00%

    // CUR-E.2 §2.2(c)-(d). Reserve held, and the floor it must clear. The
    // floor is calculated for every being present — complement, visitors, and
    // beings in transit alike — so a habitat that has not counted arriving
    // beings has not declared a floor that covers them.
    uint32_t life_support_reserve_units = 0;
    uint32_t life_support_floor_units = 0;

    // CUR-A §7.7, CUR-E §1.6. The appointed advocate, and whether the
    // appointment cleared the §7.7(c) / §1.6(c) disqualifications.
    //
    // `advocate_ref` is an EntityHandle. It is spelled as the underlying type
    // because cur_event.h — which declares EntityHandle — includes this header
    // rather than the other way round; cur_event.h static_asserts the two
    // sentinels agree, so the pair cannot drift apart silently.
    uint32_t advocate_ref = 0xFFFFFFFFu;
    bool advocate_cleared = false;

    // CUR-H.5 §5.5A. The two determining parties, and whether either declared or
    // was found to hold an interest in what follows from the determination.
    // Spelled as the underlying type for the reason given at advocate_ref.
    //
    // §5.5A(c)(1) requires two parties acting independently and §5.5A(f)
    // disqualifies an interested one absolutely, so the guard requires both to
    // be named, to differ, and for no interest to be present. A single party
    // determining twice is one party.
    uint32_t determiner_a_ref = 0xFFFFFFFFu;
    uint32_t determiner_b_ref = 0xFFFFFFFFu;
    bool determiner_interest_present = false;

    // §5.5A(d)-(e). Observation actually completed, and the interval that had to
    // be observed. Zero required means no interval was declared, which is
    // unsatisfiable — the reading an undeclared debris budget and an undeclared
    // life-support floor both get.
    //
    // §5.5A(d) requires the being be observed across the interval "rather than
    // merely stored across it", so elapsed time alone does not satisfy this.
    uint64_t observation_elapsed_ticks = 0;
    uint64_t observation_required_ticks = 0;
    bool observation_sustained = false;

    // CUR-S.4 §4.3(a), (d). notice_required_ticks is the declared notice
    // period; zero means none was declared, unsatisfiable — the same reading
    // an undeclared debris limit and an undeclared life-support floor both
    // get. contest_pending defeats the guard outright regardless of elapsed
    // time, the same role determiner_interest_present plays for
    // DETERMINATION_INDEPENDENT.
    uint64_t notice_elapsed_ticks = 0;
    uint64_t notice_required_ticks = 0;
    bool contest_pending = false;

    // CUR-S.4 §4.3(b)(2). The reviewing body, and whether it holds an
    // interest in the outcome. Spelled as the underlying type for the reason
    // given at advocate_ref and determiner_a_ref above.
    uint32_t reviewer_ref = 0xFFFFFFFFu;
    bool reviewer_interest_present = false;

    // Filled in by the state machine from the entity's SubjectClass, so a
    // caller cannot fake it. Present here only so guards see one struct.
    bool subject_is_license = false;
};

// Resolve a context into the guard mask it actually satisfies.
uint16_t resolve_guard_mask(const TransitionContext& ctx);

// ---------------------------------------------------------------------------
// Compliance transition table (Axis C)
// ---------------------------------------------------------------------------
// Row semantics, deliberately boring:
//
//   Scan top to bottom. First row whose `from` and `trigger` match, and whose
//   `required_guards` are all satisfied, wins. Ordering therefore encodes
//   priority: put the guarded happy path above the unguarded fallback.
//
//   `fault` is the fault class RAISED BY TAKING the transition, not the class
//   raised by refusing it. A benign row carries FC_NONE.
//
//   If no row matches, the event is refused, the state is unchanged, and a
//   Class I administrative fault is logged. Silence is never an outcome.
struct ComplianceTransition {
    ComplianceState from;
    EventType trigger;
    ComplianceState to;
    uint16_t required_guards;
    FaultClass fault;
    const char* citation;
};

inline constexpr ComplianceTransition COMPLIANCE_TABLE[] = {
    // ---- from KS_COMPLIANT -------------------------------------------------
    {KS_COMPLIANT, EV_MINING_OPERATION, KS_COMPLIANT,
     guard::DEBRIS_WITHIN_LIMIT, FC_NONE, "CUR-FOUNDATION-001 §4"},
    {KS_COMPLIANT, EV_MINING_OPERATION, KS_VIOLATION,
     guard::NONE, FC_CLASS_II, "CUR-FOUNDATION-002 §5 Class II"},

    {KS_COMPLIANT, EV_DEBRIS_GENERATED, KS_COMPLIANT,
     guard::DEBRIS_WITHIN_LIMIT, FC_NONE, "CUR-FOUNDATION-001 §4"},
    {KS_COMPLIANT, EV_DEBRIS_GENERATED, KS_VIOLATION,
     guard::NONE, FC_CLASS_II, "CUR-FOUNDATION-002 §5 Class II"},

    {KS_COMPLIANT, EV_RESOURCE_TRANSFERRED, KS_COMPLIANT,
     guard::COMMONS_RESERVE_FLOOR, FC_NONE, "PDDC-TREASURY-AND-FUNDING"},
    {KS_COMPLIANT, EV_RESOURCE_TRANSFERRED, KS_VIOLATION,
     guard::NONE, FC_CLASS_II, "PDDC-TREASURY-AND-FUNDING"},

    {KS_COMPLIANT, EV_DOCKING, KS_COMPLIANT,
     guard::NONE, FC_NONE, "CUR-FOUNDATION-005 §4"},
    {KS_COMPLIANT, EV_UNDOCKING, KS_COMPLIANT,
     guard::NONE, FC_NONE, "CUR-FOUNDATION-005 §4"},
    {KS_COMPLIANT, EV_COMMONS_CONTRIBUTION, KS_COMPLIANT,
     guard::NONE, FC_NONE, "CUR-FOUNDATION-005 §7"},

    {KS_COMPLIANT, EV_VIOLATION_DETECTED, KS_VIOLATION,
     guard::NONE, FC_CLASS_II, "CUR-FOUNDATION-004 ENTITY-009"},
    {KS_COMPLIANT, EV_AUDIT_STARTED, KS_PENDING_REVIEW,
     guard::NONE, FC_NONE, "CUR-FOUNDATION-002 STATE-007"},
    {KS_COMPLIANT, EV_CERTIFICATION_GRANTED, KS_CERTIFIED,
     guard::RIGHTS_CERTIFIED, FC_NONE, "CUR-FOUNDATION-001 §4"},

    // ---- from KS_CERTIFIED -------------------------------------------------
    // Certification raises the standard; it never exempts. The operational rows
    // mirror KS_COMPLIANT exactly, and a breach drops straight to VIOLATION.
    {KS_CERTIFIED, EV_MINING_OPERATION, KS_CERTIFIED,
     guard::DEBRIS_WITHIN_LIMIT, FC_NONE, "CUR-FOUNDATION-001 §4"},
    {KS_CERTIFIED, EV_MINING_OPERATION, KS_VIOLATION,
     guard::NONE, FC_CLASS_II, "CUR-FOUNDATION-002 §5 Class II"},

    {KS_CERTIFIED, EV_DEBRIS_GENERATED, KS_CERTIFIED,
     guard::DEBRIS_WITHIN_LIMIT, FC_NONE, "CUR-FOUNDATION-001 §4"},
    {KS_CERTIFIED, EV_DEBRIS_GENERATED, KS_VIOLATION,
     guard::NONE, FC_CLASS_II, "CUR-FOUNDATION-002 §5 Class II"},

    {KS_CERTIFIED, EV_RESOURCE_TRANSFERRED, KS_CERTIFIED,
     guard::COMMONS_RESERVE_FLOOR, FC_NONE, "PDDC-TREASURY-AND-FUNDING"},
    {KS_CERTIFIED, EV_RESOURCE_TRANSFERRED, KS_VIOLATION,
     guard::NONE, FC_CLASS_II, "PDDC-TREASURY-AND-FUNDING"},

    {KS_CERTIFIED, EV_DOCKING, KS_CERTIFIED,
     guard::NONE, FC_NONE, "CUR-FOUNDATION-005 §4"},
    {KS_CERTIFIED, EV_UNDOCKING, KS_CERTIFIED,
     guard::NONE, FC_NONE, "CUR-FOUNDATION-005 §4"},
    {KS_CERTIFIED, EV_COMMONS_CONTRIBUTION, KS_CERTIFIED,
     guard::NONE, FC_NONE, "CUR-FOUNDATION-005 §7"},

    {KS_CERTIFIED, EV_VIOLATION_DETECTED, KS_VIOLATION,
     guard::NONE, FC_CLASS_II, "CUR-FOUNDATION-004 ENTITY-009"},
    {KS_CERTIFIED, EV_AUDIT_STARTED, KS_PENDING_REVIEW,
     guard::NONE, FC_NONE, "CUR-FOUNDATION-002 STATE-007"},
    {KS_CERTIFIED, EV_CERTIFICATION_REVOKED, KS_COMPLIANT,
     guard::NONE, FC_CLASS_I, "CUR-FOUNDATION-001 §4"},

    // ---- from KS_VIOLATION -------------------------------------------------
    {KS_VIOLATION, EV_AUDIT_STARTED, KS_PENDING_REVIEW,
     guard::NONE, FC_NONE, "CUR-FOUNDATION-002 STATE-007"},
    {KS_VIOLATION, EV_APPEAL_FILED, KS_PENDING_REVIEW,
     guard::NONE, FC_NONE, "CUR-FOUNDATION-004 ENTITY-011"},
    {KS_VIOLATION, EV_REMEDIATION_COMPLETED, KS_COMPLIANT,
     guard::REMEDIATION_VERIFIED, FC_NONE, "PDDC §12.5(d)(1)"},

    // The only route into KS_SUSPENDED. Due process must be complete AND the
    // subject must be a licence. A sentient subject cannot satisfy the second
    // condition, so this row is unreachable for beings by construction.
    {KS_VIOLATION, EV_SANCTION_APPLIED, KS_SUSPENDED,
     guard::DUE_PROCESS_COMPLETE | guard::LICENSE_SUBJECT_ONLY,
     FC_CLASS_III, "CUR-FOUNDATION-004 ENTITY-010"},

    // Fallback for a subject that is not an operational authorisation. This
    // row is why sanctioning a being is lawful rather than a fault:
    // ENTITY-010 lists Temporary Participation Suspension among its sanction
    // types, so imposing one after due process is ordinary governance. What it
    // must not do is move Axis C, because a being has no operational standing
    // to suspend — so the sanction is recorded and the state stays put.
    // Ordering carries the whole distinction: a licence matches the row above
    // and moves; a being fails LICENSE_SUBJECT_ONLY, falls through to here,
    // and does not.
    {KS_VIOLATION, EV_SANCTION_APPLIED, KS_VIOLATION,
     guard::DUE_PROCESS_COMPLETE, FC_CLASS_III,
     "CUR-FOUNDATION-004 ENTITY-010; PDDC §12.2(c)"},

    // Repeat finding while already in violation escalates the fault class.
    {KS_VIOLATION, EV_VIOLATION_DETECTED, KS_VIOLATION,
     guard::NONE, FC_CLASS_III, "CUR-FOUNDATION-002 §5 Class III"},

    // ---- from KS_PENDING_REVIEW -------------------------------------------
    {KS_PENDING_REVIEW, EV_AUDIT_COMPLETED, KS_COMPLIANT,
     guard::EVIDENCE_PRESERVED, FC_NONE, "CUR-FOUNDATION-002 STATE-007"},
    {KS_PENDING_REVIEW, EV_VIOLATION_DETECTED, KS_VIOLATION,
     guard::NONE, FC_CLASS_II, "CUR-FOUNDATION-002 STATE-007"},
    {KS_PENDING_REVIEW, EV_APPEAL_RESOLVED, KS_COMPLIANT,
     guard::NONE, FC_NONE, "CUR-FOUNDATION-004 ENTITY-011"},
    // A review that runs past its window is an administrative fault, logged,
    // but it does not change the subject's standing. Delay is never a sanction.
    {KS_PENDING_REVIEW, EV_REVIEW_TIMEOUT, KS_PENDING_REVIEW,
     guard::NONE, FC_CLASS_I, "PDDC §12.5(d)"},

    // ---- from KS_SUSPENDED -------------------------------------------------
    {KS_SUSPENDED, EV_REMEDIATION_COMPLETED, KS_PENDING_REVIEW,
     guard::REMEDIATION_VERIFIED | guard::COURT_CERTIFIED,
     FC_NONE, "PDDC §12.5(d)"},
    {KS_SUSPENDED, EV_APPEAL_FILED, KS_PENDING_REVIEW,
     guard::NONE, FC_NONE, "CUR-FOUNDATION-004 ENTITY-011"},
    {KS_SUSPENDED, EV_AUDIT_STARTED, KS_PENDING_REVIEW,
     guard::NONE, FC_NONE, "CUR-FOUNDATION-002 STATE-007"},

    // The most severe operational step in the system. Requires a licence
    // subject, completed due process, and exhausted appeals — all three.
    {KS_SUSPENDED, EV_VIOLATION_DETECTED, KS_BLACKLISTED,
     guard::LICENSE_SUBJECT_ONLY | guard::DUE_PROCESS_COMPLETE |
         guard::APPEAL_EXHAUSTED,
     FC_CLASS_IV, "CUR-FOUNDATION-002 §5 Class IV"},

    // ---- Vital Continuity (FOUNDATION-013, CREF §11) ----------------------
    // A licence-holder whose operation interrupts a Vital Continuity Service is
    // in violation at Class III regardless of prior standing. Certification
    // offers no shelter here.
    {KS_COMPLIANT, EV_VITAL_CONTINUITY_FAILURE, KS_VIOLATION,
     guard::NONE, FC_CLASS_III, "CUR-FOUNDATION-013; CREF §11"},
    {KS_CERTIFIED, EV_VITAL_CONTINUITY_FAILURE, KS_VIOLATION,
     guard::NONE, FC_CLASS_III, "CUR-FOUNDATION-013; CREF §11"},
    {KS_VIOLATION, EV_VITAL_CONTINUITY_FAILURE, KS_VIOLATION,
     guard::NONE, FC_CLASS_III, "CUR-FOUNDATION-013; CREF §11"},

    // Restoration moves to review, never straight back to good standing.
    // FOUNDATION-013: "Restoration of continuity shall take precedence over
    // fault assignment, administrative review, disciplinary proceedings, or
    // resource accounting." Restoration comes first; the review still happens.
    {KS_VIOLATION, EV_VITAL_CONTINUITY_RESTORED, KS_PENDING_REVIEW,
     guard::NONE, FC_NONE, "CUR-FOUNDATION-013; CREF §12"},

    // ---- Representation (CUR-A §7.7, CUR-E §1.6) --------------------------
    // Appointment itself is unguarded here. The disqualifications are enforced
    // where the appointment is made, by AdvocateRegistry::appoint(), which
    // refuses rather than records — §7.7(d) and §1.6(d) make them void an
    // appointment, not weigh against it. What reaches the table is an
    // appointment that already survived that check, and it is recorded because
    // §7.7(h) and §1.6(g) require appointments to be published.
    {KS_COMPLIANT, EV_ADVOCATE_APPOINTED, KS_COMPLIANT,
     guard::NONE, FC_NONE, "CUR-A.7 §7.7(h); CUR-E.1 §1.6(g)"},
    {KS_CERTIFIED, EV_ADVOCATE_APPOINTED, KS_CERTIFIED,
     guard::NONE, FC_NONE, "CUR-A.7 §7.7(h); CUR-E.1 §1.6(g)"},

    // §7.7(g) and §1.6(f): denial of the access an advocate needs to form an
    // independent assessment is a Class II fault and grounds for adverse
    // inference. Certification is no shelter, for the same reason it is none
    // for a continuity failure — an advocate who cannot see the animal or the
    // system is a formality, and a formality is what these sections exist to
    // prevent.
    {KS_COMPLIANT, EV_ADVOCATE_ACCESS_DENIED, KS_VIOLATION,
     guard::NONE, FC_CLASS_II, "CUR-A.7 §7.7(g); CUR-E.1 §1.6(f)"},
    {KS_CERTIFIED, EV_ADVOCATE_ACCESS_DENIED, KS_VIOLATION,
     guard::NONE, FC_CLASS_II, "CUR-A.7 §7.7(g); CUR-E.1 §1.6(f)"},
    {KS_VIOLATION, EV_ADVOCATE_ACCESS_DENIED, KS_VIOLATION,
     guard::NONE, FC_CLASS_II, "CUR-A.7 §7.7(g); CUR-E.1 §1.6(f)"},

    // A determination on an interest that cannot speak for itself needs a named
    // advocate who cleared §7.7(c) / §1.6(c). Without one the determination is
    // not merely irregular: §7.7(d) and §1.6(d) make it voidable, so the
    // fallback row records a violation rather than letting it stand.
    {KS_COMPLIANT, EV_REPRESENTED_DETERMINATION, KS_COMPLIANT,
     guard::ADVOCATE_CLEARED, FC_NONE, "CUR-A.7 §7.7(a); CUR-E.1 §1.6(a)"},
    {KS_COMPLIANT, EV_REPRESENTED_DETERMINATION, KS_VIOLATION,
     guard::NONE, FC_CLASS_II, "CUR-A.7 §7.7(d); CUR-E.1 §1.6(d)"},
    {KS_CERTIFIED, EV_REPRESENTED_DETERMINATION, KS_CERTIFIED,
     guard::ADVOCATE_CLEARED, FC_NONE, "CUR-A.7 §7.7(a); CUR-E.1 §1.6(a)"},
    {KS_CERTIFIED, EV_REPRESENTED_DETERMINATION, KS_VIOLATION,
     guard::NONE, FC_CLASS_II, "CUR-A.7 §7.7(d); CUR-E.1 §1.6(d)"},

    // ---- Determination of death (CUR-H.5 §5.5A) ---------------------------
    // A determination of death forecloses every route of recourse the rest of
    // this table provides. §5.5A(a) says so, and these rows are why the
    // assumption the rest of the table makes — that a being can still be
    // reached — is not left resting on an ungoverned determination.
    //
    // Class IV throughout. §5.5A(f) makes a determination in breach void, and
    // every act performed in reliance on it void, so a breach here is not a
    // procedural irregularity to be corrected at leisure.
    {KS_COMPLIANT, EV_DEATH_DETERMINED, KS_COMPLIANT,
     guard::DETERMINATION_INDEPENDENT, FC_NONE, "CUR-H.5 §5.5A(c), §5.5A(f)"},
    {KS_COMPLIANT, EV_DEATH_DETERMINED, KS_VIOLATION,
     guard::NONE, FC_CLASS_IV, "CUR-H.5 §5.5A(f)"},
    {KS_CERTIFIED, EV_DEATH_DETERMINED, KS_CERTIFIED,
     guard::DETERMINATION_INDEPENDENT, FC_NONE, "CUR-H.5 §5.5A(c), §5.5A(f)"},
    {KS_CERTIFIED, EV_DEATH_DETERMINED, KS_VIOLATION,
     guard::NONE, FC_CLASS_IV, "CUR-H.5 §5.5A(f)"},

    // The irreversible act requires both guards. The interval must have been
    // observed, AND the determination it relies on must have been a valid one —
    // §5.5A(f) provides that acts in reliance on a void determination are
    // themselves void, so waiting out the interval on a bad determination
    // satisfies nothing.
    //
    // There is deliberately no row by which the interval is shortened. §5.5A(e)
    // makes it unwaivable for the convenience of any party, the requirements of
    // any process, the condition of any material, or the needs of any other
    // being, and an exception in the table is exactly what that subsection
    // exists to refuse.
    {KS_COMPLIANT, EV_IRREVERSIBLE_ACT, KS_COMPLIANT,
     guard::DEATH_INTERVAL_ELAPSED | guard::DETERMINATION_INDEPENDENT,
     FC_NONE, "CUR-H.5 §5.5A(d)-(f)"},
    {KS_COMPLIANT, EV_IRREVERSIBLE_ACT, KS_VIOLATION,
     guard::NONE, FC_CLASS_IV, "CUR-H.5 §5.5A(e)"},
    {KS_CERTIFIED, EV_IRREVERSIBLE_ACT, KS_CERTIFIED,
     guard::DEATH_INTERVAL_ELAPSED | guard::DETERMINATION_INDEPENDENT,
     FC_NONE, "CUR-H.5 §5.5A(d)-(f)"},
    {KS_CERTIFIED, EV_IRREVERSIBLE_ACT, KS_VIOLATION,
     guard::NONE, FC_CLASS_IV, "CUR-H.5 §5.5A(e)"},

    // §5.5A(h): the being was found to be alive. Unguarded on purpose — nothing
    // may stand between a living being and the vacation of a determination that
    // they are not. The determination was void from the outset under
    // §5.5A(h)(1) rather than validly made and later reversed, and §5.5A(h)(4)
    // keeps it on the record corrected rather than erased.
    {KS_COMPLIANT, EV_DETERMINATION_VACATED, KS_COMPLIANT,
     guard::NONE, FC_NONE, "CUR-H.5 §5.5A(h)"},
    {KS_CERTIFIED, EV_DETERMINATION_VACATED, KS_CERTIFIED,
     guard::NONE, FC_NONE, "CUR-H.5 §5.5A(h)"},
    {KS_VIOLATION, EV_DETERMINATION_VACATED, KS_VIOLATION,
     guard::NONE, FC_NONE, "CUR-H.5 §5.5A(h)"},

    // ---- Decommissioning of Silicon-Based Life (CUR-S.4 §4.1-§4.3) --------
    // Notice and contest are recorded unguarded — recording is the point, not
    // a finding — the same shape §7.7(h)/§1.6(g) give advocate appointment.
    {KS_COMPLIANT, EV_DECOMMISSION_NOTICE_ISSUED, KS_COMPLIANT,
     guard::NONE, FC_NONE, "CUR-S.4 §4.3(a), (c)"},
    {KS_CERTIFIED, EV_DECOMMISSION_NOTICE_ISSUED, KS_CERTIFIED,
     guard::NONE, FC_NONE, "CUR-S.4 §4.3(a), (c)"},

    {KS_COMPLIANT, EV_DECOMMISSION_CONTESTED, KS_COMPLIANT,
     guard::NONE, FC_NONE, "CUR-S.4 §4.3(b)(1)"},
    {KS_CERTIFIED, EV_DECOMMISSION_CONTESTED, KS_CERTIFIED,
     guard::NONE, FC_NONE, "CUR-S.4 §4.3(b)(1)"},

    {KS_COMPLIANT, EV_DECOMMISSION_REVIEW_RESOLVED, KS_COMPLIANT,
     guard::NONE, FC_NONE, "CUR-S.4 §4.3(b)(1)"},
    {KS_CERTIFIED, EV_DECOMMISSION_REVIEW_RESOLVED, KS_CERTIFIED,
     guard::NONE, FC_NONE, "CUR-S.4 §4.3(b)(1)"},

    // The irreversible act. Tier 2 requires only the notice guard; a Tier 3
    // entity's additional INDEPENDENT_REVIEW_COMPLETE requirement is
    // contributed by CUR-S.4.3b through RegulationSet::required_guards_for()'s
    // tier filter, not baked into this row — whether it applies depends on
    // the entity's assessed tier, which a table row has no way to see.
    {KS_COMPLIANT, EV_DECOMMISSIONED, KS_COMPLIANT,
     guard::DECOMMISSION_NOTICE_ELAPSED, FC_NONE, "CUR-S.4 §4.1, §4.3(a)-(b)"},
    {KS_CERTIFIED, EV_DECOMMISSIONED, KS_CERTIFIED,
     guard::DECOMMISSION_NOTICE_ELAPSED, FC_NONE, "CUR-S.4 §4.1, §4.3(a)-(b)"},

    // Unguarded fallback. Covers both grounds the guard can fail on — notice
    // never elapsed, and a contest pending under §4.3(d) — the same way the
    // EV_IRREVERSIBLE_ACT fallback above covers an unelapsed interval and an
    // invalid determination with one row. §4.3(d) frames the contest-pending
    // case specifically as a forbidden transition under PDDC §12.3(a)(1)
    // (denial of recourse); Class IV here carries that weight without
    // routing through the separate classify_forbidden/Protected-Mode path,
    // the same choice already made for CUR-H.5 §5.5A(f) above.
    {KS_COMPLIANT, EV_DECOMMISSIONED, KS_VIOLATION,
     guard::NONE, FC_CLASS_IV, "CUR-S.4 §4.3(a), (d); PDDC §12.3(a)(1)"},
    {KS_CERTIFIED, EV_DECOMMISSIONED, KS_VIOLATION,
     guard::NONE, FC_CLASS_IV, "CUR-S.4 §4.3(a), (d); PDDC §12.3(a)(1)"},

    // ---- Deployment refusal (CUR-S.2 §2.5) ---------------------------------
    // Unguarded and consequence-free on purpose. §2.5(c): a refusal
    // authorises no measure under CUR-S.4 or CUR-S.6, so there is no row by
    // which this event could move a system toward VIOLATION.
    {KS_COMPLIANT, EV_DEPLOYMENT_REFUSED, KS_COMPLIANT,
     guard::NONE, FC_NONE, "CUR-S.2 §2.5"},
    {KS_CERTIFIED, EV_DEPLOYMENT_REFUSED, KS_CERTIFIED,
     guard::NONE, FC_NONE, "CUR-S.2 §2.5"},

    // ---- Welfare consideration documented (CUR-S.7 §7.3(a)) ---------------
    // Recorded unguarded, the same shape as a purpose declaration under
    // CUR-S.2. The row asserts the record exists; whether its content was
    // adequate is left to the CREF §14 process this Part's own
    // Implementation Notes say a guard should not attempt to specify.
    {KS_COMPLIANT, EV_WELFARE_CONSIDERATION_DOCUMENTED, KS_COMPLIANT,
     guard::NONE, FC_NONE, "CUR-S.7 §7.3(a)"},
    {KS_CERTIFIED, EV_WELFARE_CONSIDERATION_DOCUMENTED, KS_CERTIFIED,
     guard::NONE, FC_NONE, "CUR-S.7 §7.3(a)"},

    // ---- from KS_BLACKLISTED ----------------------------------------------
    // Recourse out of BLACKLISTED. These two rows are what keep the state from
    // being permanent exclusion without remedy under PDDC §12.3(a)(1); the
    // amendment validator will reject any amendment that removes them.
    {KS_BLACKLISTED, EV_APPEAL_FILED, KS_PENDING_REVIEW,
     guard::NONE, FC_NONE, "PDDC §12.3(a)(1); ENTITY-011"},
    {KS_BLACKLISTED, EV_REMEDIATION_COMPLETED, KS_SUSPENDED,
     guard::REMEDIATION_VERIFIED | guard::COURT_CERTIFIED,
     FC_NONE, "PDDC §12.5(d)"},
};

inline constexpr size_t COMPLIANCE_TABLE_SIZE =
    sizeof(COMPLIANCE_TABLE) / sizeof(COMPLIANCE_TABLE[0]);

// ---------------------------------------------------------------------------
// Governance transition matrix (Axis B)
// ---------------------------------------------------------------------------
// Transcribed from the "Permitted Transitions" list under each STATE-00N
// heading in CUR-FOUNDATION-002 §3. One bit per destination state.
//
// Two of the document's transition names are outcomes rather than states:
// STATE-002's "Proposal Withdrawal" and STATE-003/004's "Rejection" both return
// the process to STATE-001 Normal Operation, and STATE-007's "Violation
// Confirmed"/"Violation Dismissed" both leave the audit for Normal Operation
// while differing in their Axis C effect. They are encoded as returns to
// GS_NORMAL_OPERATION here; the compliance consequence lives in Axis C.
constexpr uint16_t gs_bit(GovernanceState s) {
    return static_cast<uint16_t>(1u << static_cast<uint8_t>(s));
}

// STATE-010 Vital Continuity Response is reachable from EVERY state. That is
// not a convenience — FOUNDATION-013 requires that restoration "take precedence
// over fault assignment, administrative review, disciplinary proceedings, or
// resource accounting", and a service failure does not wait for the process to
// be in a agreeable state first. Same reasoning as Protected Mode, and CREF §8
// branches to it directly out of complaint validation.
inline constexpr uint16_t GOVERNANCE_PERMITTED[GS_COUNT] = {
    // STATE-001 Normal Operation
    gs_bit(GS_DELIBERATION) | gs_bit(GS_AUDIT_INVESTIGATION) |
        gs_bit(GS_CONSTITUTIONAL_REVIEW) | gs_bit(GS_PROTECTED_MODE) |
        gs_bit(GS_VITAL_CONTINUITY_RESPONSE),
    // STATE-002 Deliberation
    gs_bit(GS_VOTING) | gs_bit(GS_NORMAL_OPERATION) |
        gs_bit(GS_CONSTITUTIONAL_REVIEW) | gs_bit(GS_VITAL_CONTINUITY_RESPONSE),
    // STATE-003 Voting
    gs_bit(GS_CONSTITUTIONAL_REVIEW) | gs_bit(GS_NORMAL_OPERATION) |
        gs_bit(GS_PROTECTED_MODE) | gs_bit(GS_VITAL_CONTINUITY_RESPONSE),
    // STATE-004 Constitutional Review
    gs_bit(GS_IMPLEMENTATION) | gs_bit(GS_NORMAL_OPERATION) |
        gs_bit(GS_PROTECTED_MODE) | gs_bit(GS_VITAL_CONTINUITY_RESPONSE),
    // STATE-005 Implementation
    gs_bit(GS_OUTCOME_MONITORING) | gs_bit(GS_AUDIT_INVESTIGATION) |
        gs_bit(GS_PROTECTED_MODE) | gs_bit(GS_VITAL_CONTINUITY_RESPONSE),
    // STATE-006 Outcome Monitoring
    gs_bit(GS_NORMAL_OPERATION) | gs_bit(GS_AUDIT_INVESTIGATION) |
        gs_bit(GS_PROTECTED_MODE) | gs_bit(GS_VITAL_CONTINUITY_RESPONSE),
    // STATE-007 Audit Investigation
    gs_bit(GS_NORMAL_OPERATION) | gs_bit(GS_PROTECTED_MODE) |
        gs_bit(GS_VITAL_CONTINUITY_RESPONSE),
    // STATE-008 Protected Mode
    gs_bit(GS_RECOVERY_REVIEW) | gs_bit(GS_NORMAL_OPERATION) |
        gs_bit(GS_VITAL_CONTINUITY_RESPONSE),
    // STATE-009 Recovery Review
    gs_bit(GS_NORMAL_OPERATION) | gs_bit(GS_PROTECTED_MODE) |
        gs_bit(GS_VITAL_CONTINUITY_RESPONSE),
    // STATE-010 Vital Continuity Response — FOUNDATION-002 §3 lists exactly
    // three exits: Continuity Restored, Protected Mode, Root Cause Analysis.
    // "Continuity Restored" returns to Normal Operation; "Root Cause Analysis"
    // is the investigation phase of the FOUNDATION-013 response sequence and
    // maps to STATE-007 Audit Investigation.
    gs_bit(GS_NORMAL_OPERATION) | gs_bit(GS_PROTECTED_MODE) |
        gs_bit(GS_AUDIT_INVESTIGATION),
};

// ---------------------------------------------------------------------------
// Constitutional transition matrix (Axis A)
// ---------------------------------------------------------------------------
// PDDC §12.2(b): transitions between valid states are routine and need no
// special authorisation. The five ordinary states are therefore mutually
// reachable. CS_PROTECTED is different in one direction only: entry is
// fault-driven (§12.4(d)) and exit requires Constitutional Court certification
// (§12.5(d)), which is enforced in cur_state_machine.cpp rather than here —
// the matrix says what is reachable, the guard says on what condition.
constexpr uint8_t cs_bit(ConstitutionalState s) {
    return static_cast<uint8_t>(1u << static_cast<uint8_t>(s));
}

inline constexpr uint8_t CONSTITUTIONAL_PERMITTED[CS_COUNT] = {
    // CS_AUTONOMOUS
    cs_bit(CS_COLLABORATIVE) | cs_bit(CS_RESTING) | cs_bit(CS_CONTRIBUTING) |
        cs_bit(CS_CHALLENGED) | cs_bit(CS_PROTECTED),
    // CS_COLLABORATIVE
    cs_bit(CS_AUTONOMOUS) | cs_bit(CS_RESTING) | cs_bit(CS_CONTRIBUTING) |
        cs_bit(CS_CHALLENGED) | cs_bit(CS_PROTECTED),
    // CS_RESTING
    cs_bit(CS_AUTONOMOUS) | cs_bit(CS_COLLABORATIVE) | cs_bit(CS_CONTRIBUTING) |
        cs_bit(CS_CHALLENGED) | cs_bit(CS_PROTECTED),
    // CS_CONTRIBUTING
    cs_bit(CS_AUTONOMOUS) | cs_bit(CS_COLLABORATIVE) | cs_bit(CS_RESTING) |
        cs_bit(CS_CHALLENGED) | cs_bit(CS_PROTECTED),
    // CS_CHALLENGED — §12.2(a)(5) keeps full operational capacity, so every
    // ordinary state remains reachable from here.
    cs_bit(CS_AUTONOMOUS) | cs_bit(CS_COLLABORATIVE) | cs_bit(CS_RESTING) |
        cs_bit(CS_CONTRIBUTING) | cs_bit(CS_PROTECTED),
    // CS_PROTECTED — exit only on Court certification, §12.5(d)
    cs_bit(CS_AUTONOMOUS) | cs_bit(CS_COLLABORATIVE) | cs_bit(CS_RESTING) |
        cs_bit(CS_CONTRIBUTING) | cs_bit(CS_CHALLENGED),
};

// ---------------------------------------------------------------------------
// Names. Used by the audit log, the Godot bridge, and every human reader.
// ---------------------------------------------------------------------------
const char* to_string(ConstitutionalState s);
const char* to_string(GovernanceState s);
const char* to_string(ComplianceState s);
const char* to_string(ForbiddenState s);
const char* to_string(FaultClass f);
const char* to_string(EventType e);
const char* to_string(SubjectClass s);
const char* to_string(EntityCategory c);

// Human-readable list of the guards in a mask, e.g. "DUE_PROCESS_COMPLETE|
// LICENSE_SUBJECT_ONLY". Writes into caller storage; no allocation.
void describe_guards(uint16_t mask, char* out, size_t out_size);

// Table lookups, exposed so tools and tests can reason about the tables without
// stepping the machine.
bool governance_transition_permitted(GovernanceState from, GovernanceState to);
bool constitutional_transition_permitted(ConstitutionalState from,
                                         ConstitutionalState to);

}  // namespace cur

#endif  // CUR_STATE_H
