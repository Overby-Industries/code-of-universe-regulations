#include "cur/cur_violation.h"

#include <cstdio>

namespace cur {

const char* to_string(ViolationCategory c) {
    switch (c) {
        case VC_ADMINISTRATIVE: return "Administrative";
        case VC_ECONOMIC: return "Economic";
        case VC_CONSTITUTIONAL: return "Constitutional";
        case VC_RIGHTS_BASED: return "Rights-Based";
        case VC_ANTI_CAPTURE: return "Anti-Capture";
        case VC_COUNT: break;
    }
    return "Unknown";
}

const char* to_string(ViolationStatus s) {
    switch (s) {
        case VS_OPEN: return "OPEN";
        case VS_UNDER_REVIEW: return "UNDER_REVIEW";
        case VS_CONFIRMED: return "CONFIRMED";
        case VS_DISMISSED: return "DISMISSED";
        case VS_REMEDIED: return "REMEDIED";
        case VS_COUNT: break;
    }
    return "UNKNOWN";
}

const char* to_string(SanctionType t) {
    switch (t) {
        case SANC_FINE: return "Fine";
        case SANC_RESTITUTION: return "Restitution";
        case SANC_OFFICE_REMOVAL: return "Office Removal";
        case SANC_CONTRACT_RESTRICTION: return "Contract Restriction";
        case SANC_TEMPORARY_PARTICIPATION_SUSPENSION:
            return "Temporary Participation Suspension";
        case SANC_COUNT: break;
    }
    return "Unknown";
}

const char* to_string(SanctionStatus s) {
    switch (s) {
        case SANC_PROPOSED: return "PROPOSED";
        case SANC_ACTIVE: return "ACTIVE";
        case SANC_EXPIRED: return "EXPIRED";
        case SANC_VACATED: return "VACATED";
        case SANC_STATUS_COUNT: break;
    }
    return "UNKNOWN";
}

const char* to_string(AppealStatus s) {
    switch (s) {
        case APPEAL_NONE: return "NONE";
        case APPEAL_FILED_: return "FILED";
        case APPEAL_IN_REVIEW: return "IN_REVIEW";
        case APPEAL_UPHELD: return "UPHELD";
        case APPEAL_OVERTURNED: return "OVERTURNED";
        case APPEAL_STATUS_COUNT: break;
    }
    return "UNKNOWN";
}

ViolationCategory category_for_fault(FaultClass f, LawDomain d) {
    // Domain decides first where it is decisive: a breach in the human, animal,
    // silicon or non-human-cognitive domains is a rights question regardless of
    // how severe it is, because those domains exist to protect standing.
    switch (d) {
        case DOMAIN_HUMAN:
        case DOMAIN_ANIMAL:
        case DOMAIN_SILICON:
        case DOMAIN_NON_HUMAN_COGNITIVE:
            if (f >= FC_CLASS_III) return VC_RIGHTS_BASED;
            break;
        case DOMAIN_ECOSYSTEM:
            if (f <= FC_CLASS_II) return VC_ECONOMIC;
            break;
        default:
            break;
    }

    // Otherwise severity decides. FOUNDATION-002 §5: Class I administrative,
    // Class II institutional, Class III constitutional, Class IV existential.
    switch (f) {
        case FC_CLASS_I: return VC_ADMINISTRATIVE;
        case FC_CLASS_II: return VC_ECONOMIC;
        case FC_CLASS_III: return VC_CONSTITUTIONAL;
        case FC_CLASS_IV: return VC_ANTI_CAPTURE;
        default: return VC_ADMINISTRATIVE;
    }
}

namespace {

std::string make_id(const char* prefix, uint64_t n) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%s-%06llu", prefix,
                  static_cast<unsigned long long>(n));
    return std::string(buf);
}

}  // namespace

const ViolationRecord& ViolationLedger::open_violation(ViolationRecord r) {
    if (r.violation_id.empty()) {
        r.violation_id = make_id("VIO", next_violation_);
    }
    ++next_violation_;
    violations_.push_back(std::move(r));
    return violations_.back();
}

const SanctionRecord& ViolationLedger::impose_sanction(SanctionRecord r) {
    if (r.sanction_id.empty()) {
        r.sanction_id = make_id("SANC", next_sanction_);
    }
    ++next_sanction_;
    sanctions_.push_back(std::move(r));
    return sanctions_.back();
}

const AppealRecord& ViolationLedger::file_appeal(AppealRecord r) {
    if (r.appeal_id.empty()) {
        r.appeal_id = make_id("APL", next_appeal_);
    }
    ++next_appeal_;
    appeals_.push_back(std::move(r));
    return appeals_.back();
}

ViolationRecord* ViolationLedger::find_violation(const std::string& id) {
    for (ViolationRecord& v : violations_) {
        if (v.violation_id == id) return &v;
    }
    return nullptr;
}

SanctionRecord* ViolationLedger::find_sanction(const std::string& id) {
    for (SanctionRecord& s : sanctions_) {
        if (s.sanction_id == id) return &s;
    }
    return nullptr;
}

AppealRecord* ViolationLedger::find_appeal(const std::string& id) {
    for (AppealRecord& a : appeals_) {
        if (a.appeal_id == id) return &a;
    }
    return nullptr;
}

ViolationRecord* ViolationLedger::latest_open_violation(EntityHandle h) {
    for (auto it = violations_.rbegin(); it != violations_.rend(); ++it) {
        if (it->entity != h) continue;
        if (it->status == VS_OPEN || it->status == VS_UNDER_REVIEW ||
            it->status == VS_CONFIRMED) {
            return &(*it);
        }
    }
    return nullptr;
}

std::vector<ViolationRecord> ViolationLedger::violations_for(
    const std::string& entity_id) const {
    std::vector<ViolationRecord> out;
    for (const ViolationRecord& v : violations_) {
        if (v.entity_id == entity_id) out.push_back(v);
    }
    return out;
}

std::vector<SanctionRecord> ViolationLedger::sanctions_for(
    const std::string& entity_id) const {
    std::vector<SanctionRecord> out;
    for (const SanctionRecord& s : sanctions_) {
        if (s.entity_id == entity_id) out.push_back(s);
    }
    return out;
}

void ViolationLedger::counts_by_category(uint32_t out[VC_COUNT]) const {
    for (int i = 0; i < VC_COUNT; ++i) out[i] = 0;
    for (const ViolationRecord& v : violations_) {
        if (v.category < VC_COUNT) ++out[v.category];
    }
}

void ViolationLedger::reset() {
    violations_.clear();
    sanctions_.clear();
    appeals_.clear();
    next_violation_ = 1;
    next_sanction_ = 1;
    next_appeal_ = 1;
}

}  // namespace cur
