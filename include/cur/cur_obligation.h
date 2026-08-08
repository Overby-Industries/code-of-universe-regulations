// cur_obligation.h — obligations that come due on a schedule, and the built-in
// test that notices when one has not been met.
//
// WHY THIS IS NOT A GUARD, AND COULD NOT HAVE BEEN
//
// Every guard in this library answers one question: may this event proceed? The
// transition table is consulted when something is submitted, the guards resolve
// from the context supplied with it, and a verdict comes back. That shape works
// for everything the library did until now.
//
// Six provisions in four titles need a different question answered:
//
//   CUR-H.7 §7.12(c)(3)  a restriction on liberty is reviewed at intervals
//   CUR-H.6 §6.8(a)      every report is investigated
//   CUR-FOUNDATION-010 §5  routine, random, and triggered audit
//   CUR-N.5 §5.2B        routine monitoring, independently of any complaint
//   CUR-S.4 §4.3(a)      decommissioning notice is issued before the act
//   CUR-S.4 §4.8(c)      a proportionate measure is reviewed at intervals
//
// Each asks whether an obligation came due and went unmet. A transition table
// cannot express that, and the reason is structural rather than a missing
// feature: a table is consulted when something is submitted, and in every one of
// these cases the failure IS that nothing was submitted. A guard requiring a
// current review would be checked only when someone bothered to submit the event
// it guards. The party who simply stops reviewing submits nothing at all, and
// nothing is exactly what a guard cannot catch.
//
// So this register is checked on a schedule rather than on a stimulus. It is the
// built-in test CUR-N.5 §5.2B describes — the flight computer's periodic BIT,
// which runs because the clock advanced and not because anyone suspected a
// fault. §5.2B(f) is the reasoning: a hazard running undetected in the
// background is the thing routine checking exists to surface, and a system that
// only looks when someone complains has delegated its monitoring to whoever is
// harmed enough to speak.
//
// WHICH WAY A LAPSE CUTS
//
// The single most important property here is the direction of failure, and it is
// set by CUR-H.7 §7.12(d): the burden at every review falls on the party
// asserting a restriction should continue, never on the party seeking its end.
//
// So a lapsed review does not leave a restriction quietly in place awaiting
// attention. It withdraws the support for it — see restriction_supported().
// Putting it the other way round produces indefinite detention with a review
// stapled on, which is what every system that has attempted this has actually
// built. The being under restriction never has to petition, never has to be
// noticed, and never has to have an advocate awake at the right moment. The
// clock does the work.
//
// WHAT THIS CANNOT DO
//
// It sees obligations that were opened. Conduct nobody recorded, by a party who
// opened no obligation, is invisible here exactly as it is invisible to the
// transition table. That limit is real and is stated in docs/cur-library-api.md
// §9.20 rather than papered over: this register narrows the unlogged gap, it
// does not close it. Closing it is an institutional problem, not a library one.

#ifndef CUR_OBLIGATION_H
#define CUR_OBLIGATION_H

#include <cstdint>
#include <string>
#include <vector>

#include "cur_entity.h"
#include "cur_event.h"

namespace cur {

// What kind of obligation, and therefore what a lapse means. Each value names
// the provision that requires it, because a lapse is published and "obligation
// overdue" on its own is not a reviewable statement.
enum ObligationKind : uint8_t {
    OBLIG_INVESTIGATE_REPORT = 0,  // CUR-H.6 §6.8(a)
    OBLIG_REVIEW_RESTRICTION,      // CUR-H.7 §7.12(c)(3)
    OBLIG_ROUTINE_AUDIT,           // CUR-FOUNDATION-010 §5
    OBLIG_MONITOR_PRACTICE,        // CUR-N.5 §5.2B
    OBLIG_DECOMMISSION_NOTICE,     // CUR-S.4 §4.3(a)
    OBLIG_MEASURE_REVIEW,          // CUR-S.4 §4.8(c)
    OBLIG_KIND_COUNT
};

const char* to_string(ObligationKind k);

// Severity of a lapse. A missed investigation and a missed routine sweep are not
// the same failure, and publishing them at one severity would flatten the
// difference between neglecting a being who reported harm and running late on a
// scheduled check.
FaultClass lapse_fault_class(ObligationKind k);

struct Obligation {
    uint32_t id = 0;
    ObligationKind kind = OBLIG_ROUTINE_AUDIT;

    // The party that must act. A lapse is a fault against this party and against
    // nobody else — in particular never against `concerning`.
    EntityHandle owed_by = INVALID_ENTITY;

    // The being, interest, or subject the obligation exists to protect. Held so
    // that restriction_supported() can answer for them and so a published lapse
    // says who was left waiting. No measure ever attaches here; CUR-H.7
    // §7.11(c) and CUR-X.4 §4.9(c) both forbid it, and there is deliberately no
    // function on this class that takes `concerning` and produces a consequence
    // for them.
    EntityHandle concerning = INVALID_ENTITY;

    uint64_t opened_tick = 0;
    uint64_t due_tick = 0;

    // Zero means one-shot. Non-zero reschedules on discharge, which is what
    // makes a review obligation recur under CUR-H.7 §7.12(c)(3) rather than
    // being satisfied once and forgotten.
    uint64_t recurrence_ticks = 0;

    bool discharged = false;
    uint64_t discharged_tick = 0;

    // Set by check() when the obligation matured unmet. Kept rather than
    // cleared on a later discharge: CREF §15 and the VS_OVERTURNED reasoning
    // apply here too, because a party that reviews late every time looks
    // compliant at every instant and the pattern is only visible if the lapses
    // survive.
    bool lapsed = false;
    uint64_t lapsed_tick = 0;
    uint32_t lapse_count = 0;

    std::string citation;
    std::string detail;
};

class ObligationRegister {
public:
    // CUR-H.7 §7.11(c). Returns false unconditionally. Nothing in this class
    // produces a measure, a sanction, a state change, or a restriction against
    // the being an obligation concerns. Stated so the prohibition is greppable,
    // in the same style as AdvocateRegistry::confers_authority().
    static constexpr bool binds_the_protected() { return false; }

    // Open an obligation. `due_tick` is when it must have been met by.
    uint32_t open(ObligationKind kind, EntityHandle owed_by,
                  EntityHandle concerning, uint64_t opened_tick,
                  uint64_t due_tick, uint64_t recurrence_ticks,
                  const std::string& detail = "");

    // Mark an obligation met. A recurring obligation is rescheduled from the
    // discharge, not from the original due tick, so a party cannot accumulate
    // credit by discharging early and then going quiet.
    bool discharge(uint32_t id, uint64_t tick);

    // THE BUILT-IN TEST. Marks every obligation that has matured unmet and
    // returns their ids. Runs because the clock advanced; takes no event, and
    // is deliberately not reachable from the submit path.
    //
    // Calling it repeatedly at the same tick does not re-lapse an obligation
    // already lapsed at that tick, so a caller running it every tick and a
    // caller running it hourly see the same history.
    std::vector<uint32_t> check(uint64_t now_tick);

    // CUR-H.7 §7.12(d), and the reason this class exists in this shape.
    // CUR-S.4 §4.8(c) states the same rule for a proportionate measure short
    // of decommissioning, so this checks OBLIG_REVIEW_RESTRICTION and
    // OBLIG_MEASURE_REVIEW alike — both name the identical failure under
    // different provisions rather than two different failures.
    //
    // False once a review obligation concerning this entity has matured unmet.
    // The restriction is not thereby "ended" by the library — ending it is an
    // act someone performs — but it is no longer supported, and a party
    // continuing it is continuing something the Code does not authorise.
    //
    // Note which handle this takes. It answers for the being the restriction is
    // imposed ON, not for the party imposing it, because the being is who the
    // provision protects and requiring them to be identified as a petitioner
    // would reintroduce the burden §7.12(d) removes.
    bool restriction_supported(EntityHandle concerning, uint64_t now_tick) const;

    // Obligations that have matured unmet and not since been discharged.
    std::vector<uint32_t> outstanding(uint64_t now_tick) const;

    const Obligation* get(uint32_t id) const;
    const std::vector<Obligation>& all() const { return obligations_; }
    size_t size() const { return obligations_.size(); }
    void clear();

private:
    std::vector<Obligation> obligations_;
    uint32_t next_id_ = 1;
};

}  // namespace cur

#endif  // CUR_OBLIGATION_H
