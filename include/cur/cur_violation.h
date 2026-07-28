// cur_violation.h — ENTITY-009 Violation, ENTITY-010 Sanction,
// ENTITY-011 Appeal (CUR-FOUNDATION-004 §12-§14).
//
// The FSM produces violations and sanctions as a matter of course. GEM already
// defines their canonical shape, and FOUNDATION-004 §19 forbids governance
// actions outside the model, so the FSM emits these records rather than
// inventing a parallel representation of the same thing.
//
// Three independent axes describe any one breach. They are easy to conflate,
// so, explicitly:
//
//   LawDomain        WHOSE law was breached      CUR-H/S/A/D/E/X/N
//                    (cur_regulation.h)
//   ViolationCategory WHAT KIND of breach it is  Administrative, Economic,
//                    (here, ENTITY-009)          Constitutional, Rights-Based,
//                                                Anti-Capture
//   FaultClass       HOW SEVERE it is            Class I-IV, FOUNDATION-002 §5
//                    (cur_state.h)
//
// A violation record carries all three. Collapsing any two loses information
// the Constitutional Court and the Capture Tribunal both need.

#ifndef CUR_VIOLATION_H
#define CUR_VIOLATION_H

#include <cstdint>
#include <string>
#include <vector>

#include "cur_entity.h"
#include "cur_event.h"
#include "cur_regulation.h"
#include "cur_state.h"

namespace cur {

// ENTITY-009 Category column, verbatim from FOUNDATION-004 §12.
enum ViolationCategory : uint8_t {
    VC_ADMINISTRATIVE = 0,
    VC_ECONOMIC,
    VC_CONSTITUTIONAL,
    VC_RIGHTS_BASED,
    VC_ANTI_CAPTURE,
    VC_COUNT
};

// The lifecycle of a violation record, and the line between an allegation and
// a finding.
//
// CUR-N.5 §5.8(b) provides that a being named in a report is not thereby
// accountable, and that no record may characterise them as accountable before a
// determination. CUR-N.4 §4.3(a) provides that no measure is available except
// following a determination. Both rules depend on the record itself
// distinguishing "alleged" from "found", which is what these statuses do:
// VS_OPEN and VS_UNDER_REVIEW are allegations, the rest are determinations.
//
// VS_OVERTURNED exists because CUR-N.4 §4.12(d) and CUR-N.5 §5.10(e) require a
// determination overturned on appeal to be corrected in the record rather than
// erased. Reverting such a record to VS_DISMISSED would lose the fact that it
// was ever confirmed; leaving it VS_CONFIRMED would state something false.
enum ViolationStatus : uint8_t {
    VS_OPEN = 0,       // detected, not yet adjudicated — an allegation
    VS_UNDER_REVIEW,   // STATE-007 Audit Investigation — still an allegation
    VS_CONFIRMED,      // "Violation Confirmed" exit from STATE-007
    VS_DISMISSED,      // "Violation Dismissed" exit from STATE-007
    VS_REMEDIED,       // corrective action verified, PDDC §12.5(d)(1)
    VS_OVERTURNED,     // confirmed, then overturned on appeal — CREF §15
    VS_COUNT
};

// True once a determination has been made, either way. False while the record
// is still only an allegation.
constexpr bool is_adjudicated(ViolationStatus s) {
    return s == VS_CONFIRMED || s == VS_DISMISSED || s == VS_REMEDIED ||
           s == VS_OVERTURNED;
}

// True only where the record will carry a measure. A dismissed or overturned
// violation supports nothing, and a remedied one has already been answered.
// CUR-N.4 §4.3(a) is this predicate.
constexpr bool supports_measure(ViolationStatus s) { return s == VS_CONFIRMED; }

// Whether the record may move from one status to the next. Records are amended
// in place and never rewritten, so the permitted moves are one-way: nothing
// returns to VS_OPEN, because a violation that has been reviewed cannot become
// undetected. A dismissed record may be reopened for review on new evidence
// under CUR-N.4 §4.10(d), which is a move to VS_UNDER_REVIEW, not to VS_OPEN.
bool adjudication_permitted(ViolationStatus from, ViolationStatus to);

// ENTITY-010 Type column. The five listed under FOUNDATION-004 §13 Examples.
enum SanctionType : uint8_t {
    SANC_FINE = 0,
    SANC_RESTITUTION,
    SANC_OFFICE_REMOVAL,
    SANC_CONTRACT_RESTRICTION,
    SANC_TEMPORARY_PARTICIPATION_SUSPENSION,
    SANC_COUNT
};

enum SanctionStatus : uint8_t {
    SANC_PROPOSED = 0,
    SANC_ACTIVE,
    SANC_EXPIRED,
    SANC_VACATED,  // overturned on appeal
    SANC_STATUS_COUNT
};

// ENTITY-011 Status/Outcome, shared by Sanction::appeal_status.
enum AppealStatus : uint8_t {
    APPEAL_NONE = 0,   // no appeal filed
    APPEAL_FILED_,     // trailing underscore: EV_APPEAL_FILED is the event
    APPEAL_IN_REVIEW,
    APPEAL_UPHELD,     // original decision stands
    APPEAL_OVERTURNED,
    APPEAL_STATUS_COUNT
};

const char* to_string(ViolationCategory c);
const char* to_string(ViolationStatus s);
const char* to_string(SanctionType t);
const char* to_string(SanctionStatus s);
const char* to_string(AppealStatus s);

// Severity mapping. FOUNDATION-004 gives Violation a Severity field without
// enumerating it; FOUNDATION-002 §5 already enumerates severity as Class I-IV,
// so Severity is FaultClass rather than a fourth vocabulary.
ViolationCategory category_for_fault(FaultClass f, LawDomain d);

// ENTITY-009 Violation.
struct ViolationRecord {
    std::string violation_id;  // "VIO-000012"
    EntityHandle entity = INVALID_ENTITY;
    std::string entity_id;

    FaultClass severity = FC_NONE;        // Severity   — FOUNDATION-002 §5
    ViolationCategory category = VC_ADMINISTRATIVE;  // Category — §12
    LawDomain domain = DOMAIN_CROSS_DOMAIN;          // whose law
    ForbiddenState forbidden = FS_NONE;   // set when the breach was a fault

    std::string evidence;    // Evidence — free-form, audit-visible
    ViolationStatus status = VS_OPEN;
    bool appealable = true;  // Appealable

    std::string regulation_id;  // which provision was breached
    std::string citation;
    EventType trigger = EV_NONE;
    uint64_t tick = 0;
    uint64_t log_record_seq = 0;  // back-reference into the audit trail
};

// ENTITY-010 Sanction.
//
// A sanction is a record about an entity, not a state of one. That distinction
// carries weight: SANC_TEMPORARY_PARTICIPATION_SUSPENSION is a lawful sanction
// under FOUNDATION-004 §13 and may attach to a sentient being after due
// process, but it never moves that being's Axis C state to KS_SUSPENDED —
// beings have no Axis C state to move. Suspending a *being* rather than an
// *authorisation* would be FORBIDDEN-003 Rights Suspension. The FSM keeps the
// two apart by requiring guard::LICENSE_SUBJECT_ONLY on every Axis C
// transition into KS_SUSPENDED, while leaving sanction records unrestricted.
struct SanctionRecord {
    std::string sanction_id;    // "SANC-000004"
    std::string violation_id;   // ViolationID back-reference
    EntityHandle entity = INVALID_ENTITY;
    std::string entity_id;

    SanctionType type = SANC_CONTRACT_RESTRICTION;
    uint64_t duration_ticks = 0;  // 0 == indefinite pending review
    SanctionStatus status = SANC_PROPOSED;
    AppealStatus appeal_status = APPEAL_NONE;

    uint64_t imposed_tick = 0;
    std::string citation;
    uint64_t log_record_seq = 0;

    // True when this sanction moved an Axis C state. False for sanctions on
    // sentient beings, which are recorded without any Axis C effect.
    bool affected_compliance_axis = false;
};

// ENTITY-011 Appeal.
struct AppealRecord {
    std::string appeal_id;       // "APL-000002"
    std::string target_decision; // violation_id or sanction_id
    EntityHandle appellant = INVALID_ENTITY;
    std::string appellant_id;
    std::string evidence;
    AppealStatus status = APPEAL_FILED_;
    std::string outcome;
    uint64_t filed_tick = 0;
    uint64_t log_record_seq = 0;
};

// Append-only ledger for the three. Same discipline as the audit log: records
// are added and amended in place by status change, never deleted. IDs are
// assigned from a dense counter so they are reproducible across replays.
class ViolationLedger {
public:
    const ViolationRecord& open_violation(ViolationRecord r);
    const SanctionRecord& impose_sanction(SanctionRecord r);
    const AppealRecord& file_appeal(AppealRecord r);

    ViolationRecord* find_violation(const std::string& id);
    SanctionRecord* find_sanction(const std::string& id);
    AppealRecord* find_appeal(const std::string& id);

    // Most recent open or confirmed violation for an entity, if any.
    ViolationRecord* latest_open_violation(EntityHandle h);

    // Most recent violation for an entity that can carry a measure — that is,
    // one that has actually been determined against the entity. This is the
    // accessor a sanction path must use; latest_open_violation also returns
    // allegations, and attaching an active measure to an allegation is what
    // CUR-N.4 §4.3(a) and CUR-N.5 §5.8(b) forbid.
    ViolationRecord* latest_sanctionable_violation(EntityHandle h);

    // Move a violation record to a new status. Returns false and changes
    // nothing if the move is not permitted by adjudication_permitted, so a
    // caller cannot quietly un-confirm or re-open a record.
    bool adjudicate(const std::string& violation_id, ViolationStatus outcome);

    // Move a sanction from SANC_PROPOSED to SANC_ACTIVE. Returns false and
    // changes nothing unless the violation behind it is VS_CONFIRMED. A
    // sanction with no violation_id at all can never be activated: a measure
    // has to answer something.
    bool activate_sanction(const std::string& sanction_id);

    const std::vector<ViolationRecord>& violations() const { return violations_; }
    const std::vector<SanctionRecord>& sanctions() const { return sanctions_; }
    const std::vector<AppealRecord>& appeals() const { return appeals_; }

    std::vector<ViolationRecord> violations_for(const std::string& entity_id) const;
    std::vector<SanctionRecord> sanctions_for(const std::string& entity_id) const;

    void counts_by_category(uint32_t out[VC_COUNT]) const;

    void reset();

private:
    std::vector<ViolationRecord> violations_;
    std::vector<SanctionRecord> sanctions_;
    std::vector<AppealRecord> appeals_;
    uint64_t next_violation_ = 1;
    uint64_t next_sanction_ = 1;
    uint64_t next_appeal_ = 1;
};

}  // namespace cur

#endif  // CUR_VIOLATION_H
