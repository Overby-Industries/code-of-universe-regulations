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
