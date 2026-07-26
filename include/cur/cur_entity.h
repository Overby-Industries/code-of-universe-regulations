// cur_entity.h — Governance Entity Model (CUR-FOUNDATION-004).
//
// FOUNDATION-004 §19: "No governance action shall occur outside the Governance
// Entity Model." Every subject the FSM touches is registered here first.
// FOUNDATION-004 §20: "No authority may exist without an entity definition."

#ifndef CUR_ENTITY_H
#define CUR_ENTITY_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "cur_event.h"
#include "cur_state.h"

namespace cur {

// The three-axis state vector carried by every entity.
struct StateVector {
    ConstitutionalState constitutional = CS_AUTONOMOUS;
    GovernanceState governance = GS_NORMAL_OPERATION;
    ComplianceState compliance = KS_COMPLIANT;

    bool operator==(const StateVector& o) const {
        return constitutional == o.constitutional && governance == o.governance &&
               compliance == o.compliance;
    }
    bool operator!=(const StateVector& o) const { return !(*this == o); }
};

// A registered entity — the union of ENTITY-001 through ENTITY-014 attributes
// that the FSM actually needs. Attributes that only feed reporting (Reputation,
// WealthScore, and so on) live in `metrics` so the struct does not have to grow
// a field every time a new index is defined.
struct EntityRecord {
    EntityHandle handle = INVALID_ENTITY;
    std::string id;  // e.g. "citizen-3941", "charter-helga-07"
    std::string display_name;

    EntityCategory category = EC_CIVIC;
    SubjectClass subject_class = SUBJ_SENTIENT_BEING;

    StateVector state;

    // PDDC §12.4(c) — the last known safe state to revert to. Updated after
    // every accepted, fault-free transition; read by the fault handler.
    StateVector last_known_safe;
    uint64_t last_known_safe_tick = 0;

    // PDDC §12.5 — Protected Mode bookkeeping.
    bool protected_mode = false;
    uint64_t protected_mode_entered_tick = 0;
    ForbiddenState protected_mode_cause = FS_NONE;

    // Counters, for the Annual FSM Monitoring Report
    // (PDDC-GOVERNANCE-MECHANICS §12.6).
    uint32_t transitions_accepted = 0;
    uint32_t transitions_refused = 0;
    uint32_t faults_raised = 0;

    // Open-ended numeric attributes: TrustIndex, ParticipationScore,
    // InfluenceScore, WealthScore, TransparencyScore... (FOUNDATION-004 §4-§17,
    // FOUNDATION-002 §7). Kept as a map so adding an index is a data change.
    std::unordered_map<std::string, double> metrics;

    bool is_sentient() const { return subject_class == SUBJ_SENTIENT_BEING; }
    bool is_license() const { return subject_class == SUBJ_OPERATIONAL_LICENSE; }
};

// The registry. Handles are indices, so lookup in the step path is O(1) and
// allocation-free; the string map is only used at registration time.
class EntityRegistry {
public:
    // `subject_class` defaults to SUBJ_SENTIENT_BEING deliberately, and the
    // default is the protective one.
    //
    // Rights for All Life, TIER_ASSESSMENT_PROTOCOL §1.2: "Any system whose
    // inner experience cannot be definitively ruled out is presumed Tier 1
    // unless demonstrated otherwise. The burden of proof for withholding Tier 1
    // consideration falls on the assessor."
    //
    // So an omitted or unknown subject class must resolve to the class with the
    // most protection, not the least. A caller who has not yet classified an
    // entity gets a sentient being — which can never be suspended or
    // blacklisted on Axis C. Declaring something an OPERATIONAL_LICENSE is an
    // affirmative act, and it is the only way to make those states reachable.
    // Getting this backwards would put the burden of proof on the entity.
    EntityHandle register_entity(const std::string& id, EntityCategory category,
                                 SubjectClass subject_class = SUBJ_SENTIENT_BEING,
                                 const std::string& display_name = "");

    EntityHandle find(const std::string& id) const;

    EntityRecord* get(EntityHandle h);
    const EntityRecord* get(EntityHandle h) const;

    size_t size() const { return records_.size(); }
    const std::vector<EntityRecord>& all() const { return records_; }

    void clear();

private:
    std::vector<EntityRecord> records_;
    std::unordered_map<std::string, EntityHandle> by_id_;
};

}  // namespace cur

#endif  // CUR_ENTITY_H
