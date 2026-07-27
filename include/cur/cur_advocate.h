// cur_advocate.h — representation of interests that cannot speak for
// themselves. CUR-A §7.7 (animal advocates), CUR-E §1.6 (environmental
// advocates).
//
// The two sections are drafted in parallel and this header implements both,
// because their disqualifications are identical and CUR-E §1.6(d) says so
// expressly: "CUR-A §7.7(c) states the same rule for animal advocacy and for
// the same reason." Two registries would let the rule drift apart in one domain
// and not the other.
//
// One design decision governs everything here. §7.7(d) and §1.6(d) provide that
// the disqualifications are "a disqualification, not a factor to be weighed",
// and that an appointment made in breach is void. So appoint() refuses. It does
// not return a warning, a score, or an appointment flagged for review — a void
// appointment is one that never existed, and the closest a library can come to
// that is declining to create the record.
//
// The second decision is what this file does NOT contain. CUR-E §1.7(a): "The
// appointment confers no authority over any being, no power to direct any
// being's conduct, and no standing to seek a measure against a being that the
// Code does not otherwise permit." There is accordingly no method here that
// produces a measure, a sanction, a state change, or a permission. An advocate
// registry that could do any of those would be a route by which speaking for a
// voiceless interest became authority over a being, which is the failure mode
// §1.7 exists to foreclose. confers_authority() is stated explicitly so the
// prohibition is greppable from code, in the same style as
// VitalContinuityModel::may_gate_service_access().

#ifndef CUR_ADVOCATE_H
#define CUR_ADVOCATE_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "cur_entity.h"
#include "cur_event.h"

namespace cur {

// Which section governs the appointment. Both carry the same disqualifications;
// the domain is recorded so a published appointment cites the right provision
// and so §1.6(b)'s stewardship consultation applies only where it belongs.
enum AdvocateDomain : uint8_t {
    ADOM_ANIMAL = 0,     // CUR-A §7.7 — an animal with standing under §7.6
    ADOM_ENVIRONMENTAL,  // CUR-E §1.6 — an environmental interest under §1.5
    ADOM_COUNT
};

const char* to_string(AdvocateDomain d);

// Why an appointment was refused. Each value names the subsection it failed,
// because §7.7(h) and §1.6(g) require the record to be publishable and "refused"
// on its own is not a reviewable answer.
enum AdvocateResult : uint8_t {
    ADV_APPOINTED = 0,
    ADV_REFUSED_UNKNOWN_PARTY,        // an unregistered handle
    ADV_REFUSED_SELF_REPRESENTATION,  // advocate and represented are the same
    ADV_REFUSED_NO_EXPERTISE,         // §7.7(b), §1.6(b)
    ADV_REFUSED_DEPENDENT_ON_PARTY,   // §7.7(c)(1), §1.6(c)(1)
    ADV_REFUSED_INTEREST_IN_OUTCOME,  // §7.7(c)(2), §1.6(c)(2)
    ADV_REFUSED_ADVERSE_REPRESENTATION,  // §7.7(c)(3), §1.6(c)(3)
    ADV_REFUSED_STEWARDS_NOT_CONSULTED,  // §1.6(b)
    ADV_REFUSED_ALREADY_APPOINTED,
    ADV_RESULT_COUNT
};

const char* to_string(AdvocateResult r);

// What an appointing body declares about a proposed advocate.
//
// Three of these are declarations of fact the library cannot verify and does not
// pretend to: whether expertise was demonstrated, whether the advocate depends
// on a party, whether they hold an interest in the outcome. Recording them makes
// them publishable under §7.7(h) and falsifiable afterwards, which is what turns
// a later discovery into grounds for voiding under §7.7(d).
//
// The fourth disqualification — §7.7(c)(3), representing both sides — is not
// declared. It is checked against the register, because it is the one the
// library can actually establish.
struct AdvocateDeclaration {
    EntityHandle advocate = INVALID_ENTITY;
    EntityHandle represented = INVALID_ENTITY;
    AdvocateDomain domain = ADOM_ANIMAL;
    std::string proceeding_id;

    // §7.7(b), §1.6(b) — expertise in the species, or in the ecology of the
    // system. Defaults false: an undeclared qualification is not a qualification,
    // the same reading given an undeclared life-support floor.
    bool expertise_demonstrated = false;

    // §7.7(c)(1), §1.6(c)(1) — employed by, contracted to, funded by, or
    // otherwise dependent on a party whose conduct is in question.
    bool dependent_on_party = false;

    // §7.7(c)(2), §1.6(c)(2) — a commercial, research, developmental, or
    // operational interest in the animal, the system, or the outcome.
    bool interest_in_outcome = false;

    // §1.6(b) — where the system is subject to a stewardship relationship under
    // §1.5(b)(3), appointment is made in consultation with the people holding
    // it. Ignored for ADOM_ANIMAL, which has no equivalent.
    bool stewardship_relationship = false;
    bool stewards_consulted = false;
};

// A live appointment.
struct AdvocateAppointment {
    AdvocateDeclaration declaration;
    uint64_t appointed_tick = 0;

    // §7.7(d), §1.6(d): an appointment later found to breach §7.7(c) is void,
    // and determinations reached with it are voidable. The record is kept and
    // marked rather than deleted, on the reasoning CREF §15 gives for
    // VS_OVERTURNED: a determination that has to be revisited is corrected in
    // the open, not erased.
    bool voided = false;
    uint64_t voided_tick = 0;
    AdvocateResult void_reason = ADV_APPOINTED;
};

// The register of appointments, and of who is a party to what.
//
// A "party" is an entity whose conduct or proposed activity is in question in a
// proceeding — the sense §7.7(c)(1) and §1.6(c)(1) use. Declaring parties is
// what lets §7.7(c)(3) be checked rather than merely asserted: an advocate who
// already represents a party in a proceeding cannot also be appointed for the
// interest that party's conduct is in question over.
class AdvocateRegistry {
public:
    // CUR-E §1.7(a). Returns false unconditionally, on purpose. Nothing in this
    // class produces authority over a being, and this states it in a form a
    // reader can find by searching rather than by trusting the comments.
    static constexpr bool confers_authority() { return false; }

    // Record that `party` is a party to `proceeding_id` — its conduct or
    // proposed activity is what the proceeding concerns.
    void declare_party(const std::string& proceeding_id, EntityHandle party);

    // Record that `advocate` acts for `party` in `proceeding_id`. This is
    // ordinary representation of a party, not advocacy for a voiceless interest,
    // and it is what §7.7(c)(3) checks against.
    void record_party_representation(const std::string& proceeding_id,
                                     EntityHandle advocate, EntityHandle party);

    // Appoint an advocate under §7.7(a) or §1.6(a).
    //
    // Refuses on any disqualification. `registry` is consulted only to confirm
    // both handles are registered — FOUNDATION-004 §19, "no governance action
    // outside the Governance Entity Model" — and is never mutated.
    AdvocateResult appoint(const AdvocateDeclaration& d,
                           const EntityRegistry& registry, uint64_t tick);

    // The advocate appointed for `represented` in `proceeding_id`, or
    // INVALID_ENTITY where none is appointed or the appointment was voided.
    //
    // Voided appointments do not answer this query. §7.7(d) makes them void, and
    // a void appointment supplying an advocate would defeat the point.
    EntityHandle advocate_for(const std::string& proceeding_id,
                              EntityHandle represented) const;

    // Whether a determination on `represented` may proceed: an appointment
    // exists, it is not voided, and it cleared every disqualification when made.
    // This is what a caller reports through TransitionContext::advocate_cleared.
    bool determination_permitted(const std::string& proceeding_id,
                                 EntityHandle represented) const;

    // §7.7(d), §1.6(d) — void an appointment on a disqualification discovered
    // after the fact. Returns false when no live appointment matches.
    bool void_appointment(const std::string& proceeding_id,
                          EntityHandle represented, AdvocateResult reason,
                          uint64_t tick);

    const std::vector<AdvocateAppointment>& appointments() const {
        return appointments_;
    }

    void clear();

private:
    struct PartyRelation {
        std::string proceeding_id;
        EntityHandle advocate = INVALID_ENTITY;
        EntityHandle party = INVALID_ENTITY;
    };

    bool represents_a_party(const std::string& proceeding_id,
                            EntityHandle advocate) const;

    std::vector<AdvocateAppointment> appointments_;
    std::vector<PartyRelation> party_representations_;
    std::vector<std::pair<std::string, EntityHandle>> parties_;
};

}  // namespace cur

#endif  // CUR_ADVOCATE_H
