#include "cur/cur_entity.h"

namespace cur {

EntityHandle EntityRegistry::register_entity(const std::string& id,
                                             EntityCategory category,
                                             SubjectClass subject_class,
                                             const std::string& display_name) {
    auto it = by_id_.find(id);
    if (it != by_id_.end()) {
        // Re-registration keeps the existing handle and state. Silently
        // resetting an entity's standing because someone registered twice
        // would be a state change with no event behind it, which
        // FOUNDATION-005 §2 does not allow.
        return it->second;
    }

    EntityHandle h = static_cast<EntityHandle>(records_.size());

    EntityRecord rec;
    rec.handle = h;
    rec.id = id;
    rec.display_name = display_name.empty() ? id : display_name;
    rec.category = category;
    rec.subject_class = subject_class;
    // Written here and nowhere else. See the field comment in cur_entity.h for
    // why the library keeps a second copy it never updates.
    rec.registered_subject_class = subject_class;
    rec.state = StateVector{};
    rec.last_known_safe = rec.state;
    rec.last_known_safe_tick = 0;

    records_.push_back(std::move(rec));
    by_id_[id] = h;
    return h;
}

EntityHandle EntityRegistry::find(const std::string& id) const {
    auto it = by_id_.find(id);
    return it == by_id_.end() ? INVALID_ENTITY : it->second;
}

bool EntityRegistry::assess_tier(EntityHandle h, int tier,
                                 bool independently_assessed) {
    EntityRecord* rec = get(h);
    if (rec == nullptr) return false;

    // CUR-S.1 §1.2(e), CUR-S.4 §4.2(d): only an independent assessment may
    // narrow protection. Raising the tier, or holding it at 2 or above,
    // needs no such affirmation — the ratchet only turns toward protection.
    if (tier < 2 && !independently_assessed) return false;

    rec->assessed_tier = tier;
    return true;
}

EntityRecord* EntityRegistry::get(EntityHandle h) {
    if (h == INVALID_ENTITY || h >= records_.size()) return nullptr;
    return &records_[h];
}

const EntityRecord* EntityRegistry::get(EntityHandle h) const {
    if (h == INVALID_ENTITY || h >= records_.size()) return nullptr;
    return &records_[h];
}

void EntityRegistry::clear() {
    records_.clear();
    by_id_.clear();
}

}  // namespace cur
