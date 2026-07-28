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

    // The class this entity was registered with. Written once, by
    // register_entity, and never again.
    //
    // This field exists because every structural protection in the library keys
    // off subject class — CURStateMachine::structural_forbidden() returns
    // FS_NONE immediately for a licence — and `subject_class` above is a public
    // field on a record reachable through a non-const get(). Without this, a
    // caller could register a being, write SUBJ_OPERATIONAL_LICENSE over its
    // class, and walk it into KS_SUSPENDED with FORBIDDEN-001 and FORBIDDEN-003
    // never consulted. That is not a hypothetical: it was demonstrated against
    // this library before this field was added.
    //
    // It is the oldest evasion there is. The rule is not broken; the subject is
    // redefined until the rule does not apply to it. A protection that can be
    // switched off by relabelling the thing it protects is not a protection,
    // and the Code says so directly — CUR-X.4 §4.2(a) treats an enterprise as a
    // governance structure "in fact, regardless of how it is constituted or
    // described", and CUR-N.5 §5.2A grades practices rather than the names
    // given to them.
    SubjectClass registered_subject_class = SUBJ_SENTIENT_BEING;

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

    // Both resolve to the MORE PROTECTIVE of the registered and current class,
    // so the ratchet only ever turns toward protection.
    //
    // An entity registered as a being cannot be made suspendable by a later
    // write, because is_license() requires both fields to agree. An entity
    // registered as a licence and later marked sentient does gain protection,
    // and that direction is safe to allow: it removes reachable states, it does
    // not create them.
    //
    // This is the same precautionary default that makes SUBJ_SENTIENT_BEING the
    // registry's fallback — where the answer is contested, the protective
    // reading wins, and the burden falls on the party seeking to remove
    // protection rather than on the entity.
    bool is_sentient() const {
        return subject_class == SUBJ_SENTIENT_BEING ||
               registered_subject_class == SUBJ_SENTIENT_BEING;
    }
    bool is_license() const {
        return subject_class == SUBJ_OPERATIONAL_LICENSE &&
               registered_subject_class == SUBJ_OPERATIONAL_LICENSE;
    }

    // True where the two disagree. Prevention is not enough on its own: an
    // attempt to reclassify a being is evidence about the party that attempted
    // it, and silently refusing would discard that evidence. CUR-N.5 §5.2B is
    // the reasoning — a hazard running undetected in the background is the
    // thing routine checking exists to surface.
    bool subject_class_tampered() const {
        return subject_class != registered_subject_class;
    }
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
