#include "cur/cur_event.h"

namespace cur {

const char* to_string(EventPriority p) {
    switch (p) {
        case P1_INFORMATIONAL: return "P1";
        case P2_GOVERNANCE: return "P2";
        case P3_JUDICIAL: return "P3";
        case P4_CONSTITUTIONAL: return "P4";
        case P5_EXISTENTIAL: return "P5";
        case P_COUNT: break;
    }
    return "P?";
}

EventPriority default_priority(EventType t) {
    switch (t) {
        // FSM kernel events touching Protected Mode are existential — §10 P5.
        case EV_PROTECTED_MODE_ENTERED:
        case EV_PROTECTED_MODE_EXITED:
        case EV_FAULT_DETECTED:
            return P5_EXISTENTIAL;

        // Constitutional — §10 P4.
        case EV_CONSTITUTIONAL_REVIEW_STARTED:
        case EV_CONSTITUTIONAL_REVIEW_COMPLETED:
        case EV_CAPTURE_RISK_THRESHOLD_EXCEEDED:
        case EV_STATE_TRANSITION:
            return P4_CONSTITUTIONAL;

        // Judicial — §10 P3.
        case EV_AUDIT_STARTED:
        case EV_AUDIT_COMPLETED:
        case EV_VIOLATION_DETECTED:
        case EV_SANCTION_APPLIED:
        case EV_APPEAL_FILED:
        case EV_APPEAL_RESOLVED:
        case EV_CERTIFICATION_REVOKED:
        case EV_REVIEW_TIMEOUT:
            return P3_JUDICIAL;

        // Governance — §10 P2.
        case EV_PROPOSAL_SUBMITTED:
        case EV_PROPOSAL_UPDATED:
        case EV_PROPOSAL_WITHDRAWN:
        case EV_VOTE_CAST:
        case EV_VOTE_CLOSED:
        case EV_PROPOSAL_APPROVED:
        case EV_PROPOSAL_REJECTED:
        case EV_COUNCIL_ACTION:
        case EV_ASSEMBLY_SESSION_OPENED:
        case EV_ASSEMBLY_SESSION_CLOSED:
        case EV_CERTIFICATION_GRANTED:
        case EV_REMEDIATION_COMPLETED:
            return P2_GOVERNANCE;

        // Everything else — economic flow and routine simulator traffic.
        default:
            return P1_INFORMATIONAL;
    }
}

EventValidation validate_event(const Event& e) {
    EventValidation v;

    // FOUNDATION-005 §9: events must reference valid entities and carry a
    // timestamp. Logical time is the timestamp here; tick 0 is legitimate
    // (it is the first tick), so the check is on the type and target instead.
    if (e.type == EV_NONE || e.type >= EV_COUNT) {
        v.reason = "event type is EV_NONE or out of range";
        return v;
    }
    if (e.target == INVALID_ENTITY) {
        v.reason = "event has no target entity (FOUNDATION-005 §9)";
        return v;
    }
    if (e.priority >= P_COUNT) {
        v.reason = "event priority out of range";
        return v;
    }
    if (e.asserts_forbidden >= FS_COUNT) {
        v.reason = "asserted forbidden state out of range";
        return v;
    }

    v.valid = true;
    v.reason = "";
    return v;
}

}  // namespace cur
