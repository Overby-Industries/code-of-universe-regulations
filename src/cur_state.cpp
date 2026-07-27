#include "cur/cur_state.h"

#include <cstdio>
#include <cstring>

namespace cur {

const char* to_string(ConstitutionalState s) {
    switch (s) {
        case CS_AUTONOMOUS: return "AUTONOMOUS";
        case CS_COLLABORATIVE: return "COLLABORATIVE";
        case CS_RESTING: return "RESTING";
        case CS_CONTRIBUTING: return "CONTRIBUTING";
        case CS_CHALLENGED: return "CHALLENGED";
        case CS_PROTECTED: return "PROTECTED";
        case CS_COUNT: break;
    }
    return "UNKNOWN";
}

const char* to_string(GovernanceState s) {
    switch (s) {
        case GS_NORMAL_OPERATION: return "NORMAL_OPERATION";
        case GS_DELIBERATION: return "DELIBERATION";
        case GS_VOTING: return "VOTING";
        case GS_CONSTITUTIONAL_REVIEW: return "CONSTITUTIONAL_REVIEW";
        case GS_IMPLEMENTATION: return "IMPLEMENTATION";
        case GS_OUTCOME_MONITORING: return "OUTCOME_MONITORING";
        case GS_AUDIT_INVESTIGATION: return "AUDIT_INVESTIGATION";
        case GS_PROTECTED_MODE: return "PROTECTED_MODE";
        case GS_RECOVERY_REVIEW: return "RECOVERY_REVIEW";
        case GS_VITAL_CONTINUITY_RESPONSE: return "VITAL_CONTINUITY_RESPONSE";
        case GS_COUNT: break;
    }
    return "UNKNOWN";
}

const char* to_string(ComplianceState s) {
    switch (s) {
        case KS_COMPLIANT: return "COMPLIANT";
        case KS_VIOLATION: return "VIOLATION";
        case KS_PENDING_REVIEW: return "PENDING_REVIEW";
        case KS_SUSPENDED: return "SUSPENDED";
        case KS_CERTIFIED: return "CERTIFIED";
        case KS_BLACKLISTED: return "BLACKLISTED";
        case KS_COUNT: break;
    }
    return "UNKNOWN";
}

const char* to_string(ForbiddenState s) {
    switch (s) {
        case FS_NONE: return "NONE";
        case FS_ENSLAVED: return "ENSLAVED";
        case FS_RESET_MEMORY_WIPE: return "RESET_MEMORY_WIPE";
        case FS_OWNED_DISPOSABLE: return "OWNED_DISPOSABLE";
        case FS_NON_CONSENSUAL_MODIFICATION: return "NON_CONSENSUAL_MODIFICATION";
        case FS_CITIZEN_OWNERSHIP: return "CITIZEN_OWNERSHIP";
        case FS_PERMANENT_EMERGENCY: return "PERMANENT_EMERGENCY";
        case FS_RIGHTS_SUSPENSION: return "RIGHTS_SUSPENSION";
        case FS_UNREVIEWABLE_AUTHORITY: return "UNREVIEWABLE_AUTHORITY";
        case FS_SPECIES_PRIVILEGE: return "SPECIES_PRIVILEGE";
        case FS_VITAL_CONTINUITY_DENIAL: return "VITAL_CONTINUITY_DENIAL";
        case FS_COUNT: break;
    }
    return "UNKNOWN";
}

const char* to_string(FaultClass f) {
    switch (f) {
        case FC_NONE: return "NONE";
        case FC_CLASS_I: return "CLASS_I";
        case FC_CLASS_II: return "CLASS_II";
        case FC_CLASS_III: return "CLASS_III";
        case FC_CLASS_IV: return "CLASS_IV";
        case FC_COUNT: break;
    }
    return "UNKNOWN";
}

const char* to_string(SubjectClass s) {
    switch (s) {
        case SUBJ_SENTIENT_BEING: return "SENTIENT_BEING";
        case SUBJ_INSTITUTION: return "INSTITUTION";
        case SUBJ_ORGANIZATION: return "ORGANIZATION";
        case SUBJ_OPERATIONAL_LICENSE: return "OPERATIONAL_LICENSE";
        case SUBJ_RESOURCE: return "RESOURCE";
        case SUBJ_INFRASTRUCTURE: return "INFRASTRUCTURE";
        case SUBJ_COUNT: break;
    }
    return "UNKNOWN";
}

const char* to_string(EntityCategory c) {
    switch (c) {
        case EC_CIVIC: return "CIVIC";
        case EC_INSTITUTIONAL: return "INSTITUTIONAL";
        case EC_ECONOMIC: return "ECONOMIC";
        case EC_DEMOCRATIC: return "DEMOCRATIC";
        case EC_JUDICIAL: return "JUDICIAL";
        case EC_ORGANIZATIONAL: return "ORGANIZATIONAL";
        case EC_AUTONOMOUS_SILICON: return "AUTONOMOUS_SILICON";
        case EC_COUNT: break;
    }
    return "UNKNOWN";
}

const char* to_string(EventType e) {
    switch (e) {
        case EV_NONE: return "none";

        case EV_PROPOSAL_SUBMITTED: return "proposal_submitted";
        case EV_PROPOSAL_UPDATED: return "proposal_updated";
        case EV_PROPOSAL_WITHDRAWN: return "proposal_withdrawn";
        case EV_VOTE_CAST: return "vote_cast";
        case EV_VOTE_CLOSED: return "vote_closed";
        case EV_PROPOSAL_APPROVED: return "proposal_approved";
        case EV_PROPOSAL_REJECTED: return "proposal_rejected";

        case EV_AUDIT_STARTED: return "audit_started";
        case EV_AUDIT_COMPLETED: return "audit_completed";
        case EV_VIOLATION_DETECTED: return "violation_detected";
        case EV_SANCTION_APPLIED: return "sanction_applied";
        case EV_APPEAL_FILED: return "appeal_filed";
        case EV_APPEAL_RESOLVED: return "appeal_resolved";

        case EV_COUNCIL_ACTION: return "council_action";
        case EV_ASSEMBLY_SESSION_OPENED: return "assembly_session_opened";
        case EV_ASSEMBLY_SESSION_CLOSED: return "assembly_session_closed";
        case EV_CONSTITUTIONAL_REVIEW_STARTED:
            return "constitutional_review_started";
        case EV_CONSTITUTIONAL_REVIEW_COMPLETED:
            return "constitutional_review_completed";

        case EV_RESOURCE_DISCOVERED: return "resource_discovered";
        case EV_RESOURCE_ALLOCATED: return "resource_allocated";
        case EV_RESOURCE_TRANSFERRED: return "resource_transferred";
        case EV_COMMONS_CONTRIBUTION: return "commons_contribution";
        case EV_DIVIDEND_DISTRIBUTED: return "dividend_distributed";

        case EV_STATE_TRANSITION: return "state_transition";
        case EV_PROTECTED_MODE_ENTERED: return "protected_mode_entered";
        case EV_PROTECTED_MODE_EXITED: return "protected_mode_exited";
        case EV_FAULT_DETECTED: return "fault_detected";
        case EV_CAPTURE_RISK_THRESHOLD_EXCEEDED:
            return "capture_risk_threshold_exceeded";

        case EV_MINING_OPERATION: return "mining_operation";
        case EV_DEBRIS_GENERATED: return "debris_generated";
        case EV_DOCKING: return "docking";
        case EV_UNDOCKING: return "undocking";
        case EV_CERTIFICATION_GRANTED: return "certification_granted";
        case EV_CERTIFICATION_REVOKED: return "certification_revoked";
        case EV_REMEDIATION_COMPLETED: return "remediation_completed";
        case EV_REVIEW_TIMEOUT: return "review_timeout";

        case EV_VITAL_CONTINUITY_FAILURE: return "vital_continuity_failure";
        case EV_VITAL_CONTINUITY_RESTORED: return "vital_continuity_restored";
        case EV_ROOT_CAUSE_ANALYSIS_COMPLETED:
            return "root_cause_analysis_completed";
        case EV_VITAL_CONTINUITY_DENIED: return "vital_continuity_denied";

        case EV_ADVOCATE_APPOINTED: return "advocate_appointed";
        case EV_ADVOCATE_ACCESS_DENIED: return "advocate_access_denied";
        case EV_REPRESENTED_DETERMINATION: return "represented_determination";

        case EV_COUNT: break;
    }
    return "unknown";
}

uint16_t resolve_guard_mask(const TransitionContext& ctx) {
    uint16_t m = guard::NONE;
    if (ctx.rights_certified) m |= guard::RIGHTS_CERTIFIED;
    if (ctx.due_process_complete) m |= guard::DUE_PROCESS_COMPLETE;
    if (ctx.evidence_preserved) m |= guard::EVIDENCE_PRESERVED;
    if (ctx.appeal_exhausted) m |= guard::APPEAL_EXHAUSTED;
    if (ctx.remediation_verified) m |= guard::REMEDIATION_VERIFIED;
    if (ctx.court_certified) m |= guard::COURT_CERTIFIED;
    if (ctx.subject_is_license) m |= guard::LICENSE_SUBJECT_ONLY;

    // A limit of zero means "no declared limit", which is not the same as a
    // limit of zero units. An undeclared limit cannot be satisfied — a charter
    // has to state its debris budget before it can be held to it.
    if (ctx.debris_limit > 0 && ctx.debris_units <= ctx.debris_limit) {
        m |= guard::DEBRIS_WITHIN_LIMIT;
    }

    // PDDC-TREASURY-AND-FUNDING: 20% Commons Reserve floor, in basis points.
    if (ctx.commons_reserve_basis_points >= 2000) {
        m |= guard::COMMONS_RESERVE_FLOOR;
    }

    // CUR-E.2 §2.2(c). A floor of zero means "no declared floor", not "no
    // reserve required", and cannot be satisfied — the same reading given to
    // an undeclared debris limit above, for the same reason. A habitat has to
    // state the margin it needs before it can be held to it, and CUR-E.2
    // §2.2(b) is why the margin is a constitutional obligation rather than an
    // engineering preference: in a closed volume there is no outside to
    // absorb the error.
    if (ctx.life_support_floor_units > 0 &&
        ctx.life_support_reserve_units >= ctx.life_support_floor_units) {
        m |= guard::LIFE_SUPPORT_MARGIN;
    }

    // CUR-A §7.7, CUR-E §1.6. Both halves, and the naming half is not a
    // formality: an advocate who is nobody in particular cannot be checked
    // against §7.7(c)(1)-(3) afterwards, cannot be published under §7.7(h), and
    // cannot be held to the duty §7.7(e) places on them personally. The
    // disqualifications are enforced at appointment by AdvocateRegistry, which
    // refuses rather than records; a caller asserting `advocate_cleared` here
    // is reporting the outcome of that check, not substituting for it.
    if (ctx.advocate_ref != 0xFFFFFFFFu && ctx.advocate_cleared) {
        m |= guard::ADVOCATE_CLEARED;
    }
    return m;
}

namespace {

struct GuardName {
    uint16_t bit;
    const char* name;
};

constexpr GuardName kGuardNames[] = {
    {guard::RIGHTS_CERTIFIED, "RIGHTS_CERTIFIED"},
    {guard::DUE_PROCESS_COMPLETE, "DUE_PROCESS_COMPLETE"},
    {guard::EVIDENCE_PRESERVED, "EVIDENCE_PRESERVED"},
    {guard::APPEAL_EXHAUSTED, "APPEAL_EXHAUSTED"},
    {guard::DEBRIS_WITHIN_LIMIT, "DEBRIS_WITHIN_LIMIT"},
    {guard::COMMONS_RESERVE_FLOOR, "COMMONS_RESERVE_FLOOR"},
    {guard::LICENSE_SUBJECT_ONLY, "LICENSE_SUBJECT_ONLY"},
    {guard::REMEDIATION_VERIFIED, "REMEDIATION_VERIFIED"},
    {guard::COURT_CERTIFIED, "COURT_CERTIFIED"},
    {guard::LIFE_SUPPORT_MARGIN, "LIFE_SUPPORT_MARGIN"},
    {guard::ADVOCATE_CLEARED, "ADVOCATE_CLEARED"},
};

}  // namespace

void describe_guards(uint16_t mask, char* out, size_t out_size) {
    if (out == nullptr || out_size == 0) return;
    out[0] = '\0';
    if (mask == guard::NONE) {
        std::snprintf(out, out_size, "NONE");
        return;
    }

    size_t used = 0;
    bool first = true;
    for (const GuardName& g : kGuardNames) {
        if ((mask & g.bit) == 0) continue;
        const char* sep = first ? "" : "|";
        int n = std::snprintf(out + used, out_size - used, "%s%s", sep, g.name);
        if (n < 0) break;
        if (static_cast<size_t>(n) >= out_size - used) {
            // Truncated; snprintf already null-terminated what fit.
            return;
        }
        used += static_cast<size_t>(n);
        first = false;
    }
}

bool governance_transition_permitted(GovernanceState from, GovernanceState to) {
    if (from >= GS_COUNT || to >= GS_COUNT) return false;
    if (from == to) return true;  // staying put is always permitted
    return (GOVERNANCE_PERMITTED[from] & gs_bit(to)) != 0;
}

bool constitutional_transition_permitted(ConstitutionalState from,
                                         ConstitutionalState to) {
    if (from >= CS_COUNT || to >= CS_COUNT) return false;
    if (from == to) return true;
    return (CONSTITUTIONAL_PERMITTED[from] & cs_bit(to)) != 0;
}

}  // namespace cur
