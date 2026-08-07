#include "cur/cur_advocate.h"

namespace cur {

const char* to_string(AdvocateDomain d) {
    switch (d) {
        case ADOM_ANIMAL: return "Animal (CUR-A §7.7)";
        case ADOM_ENVIRONMENTAL: return "Environmental (CUR-E §1.6)";
        case ADOM_HUMAN_INCAPACITY: return "Human incapacity (CUR-H.5 §5.7(g))";
        case ADOM_COUNT: break;
    }
    return "Unknown";
}

const char* to_string(AdvocateResult r) {
    // These strings are published under §7.7(h) and §1.6(g), so each names the
    // subsection the appointment failed rather than saying only that it did.
    switch (r) {
        case ADV_APPOINTED:
            return "Appointed";
        case ADV_REFUSED_UNKNOWN_PARTY:
            return "Refused: handle not registered (FOUNDATION-004 §19)";
        case ADV_REFUSED_SELF_REPRESENTATION:
            return "Refused: advocate and represented interest are the same entity";
        case ADV_REFUSED_NO_EXPERTISE:
            return "Refused: expertise not demonstrated (§7.7(b), §1.6(b))";
        case ADV_REFUSED_DEPENDENT_ON_PARTY:
            return "Refused: dependent on a party in question (§7.7(c)(1), §1.6(c)(1))";
        case ADV_REFUSED_INTEREST_IN_OUTCOME:
            return "Refused: holds an interest in the outcome (§7.7(c)(2), §1.6(c)(2))";
        case ADV_REFUSED_ADVERSE_REPRESENTATION:
            return "Refused: already represents an adverse party (§7.7(c)(3), §1.6(c)(3))";
        case ADV_REFUSED_STEWARDS_NOT_CONSULTED:
            return "Refused: stewarding people not consulted (§1.6(b))";
        case ADV_REFUSED_ALREADY_APPOINTED:
            return "Refused: an advocate is already appointed for this interest";
        case ADV_RESULT_COUNT:
            break;
    }
    return "Unknown";
}

void AdvocateRegistry::declare_party(const std::string& proceeding_id,
                                     EntityHandle party) {
    for (const auto& p : parties_) {
        if (p.first == proceeding_id && p.second == party) return;
    }
    parties_.emplace_back(proceeding_id, party);
}

void AdvocateRegistry::record_party_representation(
    const std::string& proceeding_id, EntityHandle advocate,
    EntityHandle party) {
    for (const auto& r : party_representations_) {
        if (r.proceeding_id == proceeding_id && r.advocate == advocate &&
            r.party == party) {
            return;
        }
    }
    PartyRelation rel;
    rel.proceeding_id = proceeding_id;
    rel.advocate = advocate;
    rel.party = party;
    party_representations_.push_back(rel);
}

bool AdvocateRegistry::represents_a_party(const std::string& proceeding_id,
                                          EntityHandle advocate) const {
    for (const auto& r : party_representations_) {
        if (r.proceeding_id == proceeding_id && r.advocate == advocate) {
            return true;
        }
    }
    return false;
}

AdvocateResult AdvocateRegistry::appoint(const AdvocateDeclaration& d,
                                         const EntityRegistry& registry,
                                         uint64_t tick) {
    // FOUNDATION-004 §19: no governance action outside the Governance Entity
    // Model. Both ends of the relation have to be entities the model knows.
    if (registry.get(d.advocate) == nullptr ||
        registry.get(d.represented) == nullptr) {
        return ADV_REFUSED_UNKNOWN_PARTY;
    }
    if (d.advocate == d.represented) {
        return ADV_REFUSED_SELF_REPRESENTATION;
    }

    // One live appointment per interest per proceeding. A second advocate would
    // make §7.7(e)'s duty — owed to the animal's interests as the animal would
    // have them — ambiguous at exactly the moment it matters.
    if (advocate_for(d.proceeding_id, d.represented) != INVALID_ENTITY) {
        return ADV_REFUSED_ALREADY_APPOINTED;
    }

    // §7.7(b), §1.6(b).
    if (!d.expertise_demonstrated) return ADV_REFUSED_NO_EXPERTISE;

    // §7.7(c)(1)-(2), §1.6(c)(1)-(2). Declared, not derived — the library cannot
    // audit a funding relationship. Recording the declaration is what makes it
    // falsifiable later, and a later finding voids the appointment through
    // void_appointment() rather than merely embarrassing the appointing body.
    if (d.dependent_on_party) return ADV_REFUSED_DEPENDENT_ON_PARTY;
    if (d.interest_in_outcome) return ADV_REFUSED_INTEREST_IN_OUTCOME;

    // §7.7(c)(3), §1.6(c)(3) — the one the register can establish for itself.
    // An advocate already acting for a party to this proceeding cannot also
    // speak for the interest whose treatment the proceeding concerns.
    if (represents_a_party(d.proceeding_id, d.advocate)) {
        return ADV_REFUSED_ADVERSE_REPRESENTATION;
    }

    // §1.6(b). Only environmental appointments carry this, and only where a
    // stewardship relationship exists under §1.5(b)(3). CUR-E §1.7(d) is why it
    // is a refusal and not a note: conservation has historically been a vehicle
    // for displacing exactly the peoples this consults.
    if (d.domain == ADOM_ENVIRONMENTAL && d.stewardship_relationship &&
        !d.stewards_consulted) {
        return ADV_REFUSED_STEWARDS_NOT_CONSULTED;
    }

    AdvocateAppointment a;
    a.declaration = d;
    a.appointed_tick = tick;
    appointments_.push_back(a);
    return ADV_APPOINTED;
}

EntityHandle AdvocateRegistry::advocate_for(const std::string& proceeding_id,
                                            EntityHandle represented) const {
    for (const auto& a : appointments_) {
        if (a.voided) continue;
        if (a.declaration.proceeding_id == proceeding_id &&
            a.declaration.represented == represented) {
            return a.declaration.advocate;
        }
    }
    return INVALID_ENTITY;
}

bool AdvocateRegistry::determination_permitted(const std::string& proceeding_id,
                                               EntityHandle represented) const {
    // Every disqualification was applied at appointment, and void_appointment()
    // removes an appointment from this answer the moment one is discovered
    // afterwards. So a live appointment is a cleared appointment, and there is
    // no second check to run here.
    return advocate_for(proceeding_id, represented) != INVALID_ENTITY;
}

bool AdvocateRegistry::void_appointment(const std::string& proceeding_id,
                                        EntityHandle represented,
                                        AdvocateResult reason, uint64_t tick) {
    for (auto& a : appointments_) {
        if (a.voided) continue;
        if (a.declaration.proceeding_id == proceeding_id &&
            a.declaration.represented == represented) {
            // Marked, not erased. §7.7(d) makes determinations reached with this
            // advocate voidable under §7.6(e), and identifying them afterwards
            // requires the appointment still be on the record.
            a.voided = true;
            a.voided_tick = tick;
            a.void_reason = reason;
            return true;
        }
    }
    return false;
}

void AdvocateRegistry::clear() {
    appointments_.clear();
    party_representations_.clear();
    parties_.clear();
}

}  // namespace cur
