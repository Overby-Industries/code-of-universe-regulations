#include "cur/cur_obligation.h"

namespace cur {

const char* to_string(ObligationKind k) {
    switch (k) {
        case OBLIG_INVESTIGATE_REPORT:
            return "Investigate report (CUR-H.6 §6.8(a))";
        case OBLIG_REVIEW_RESTRICTION:
            return "Review restriction (CUR-H.7 §7.12(c)(3))";
        case OBLIG_ROUTINE_AUDIT:
            return "Routine audit (CUR-FOUNDATION-010 §5)";
        case OBLIG_MONITOR_PRACTICE:
            return "Monitor practice (CUR-N.5 §5.2B)";
        case OBLIG_KIND_COUNT:
            break;
    }
    return "Unknown";
}

FaultClass lapse_fault_class(ObligationKind k) {
    switch (k) {
        // A being reported harm and nobody came. CUR-H.6 §6.8(b) provides that
        // the duty does not depend on the report being credible, complete, or
        // made by the being harmed, so there is no version of this lapse that
        // is a paperwork problem.
        case OBLIG_INVESTIGATE_REPORT:
            return FC_CLASS_III;

        // A restriction of liberty continued past its review. Class IV because
        // CUR-H.7 §7.12(d) makes the lapse withdraw the support for the
        // restriction: what is running is no longer a bounded separation but a
        // deprivation of liberty nobody has justified, which is unreviewable
        // authority within CUR-X.4 §4.3(b).
        case OBLIG_REVIEW_RESTRICTION:
            return FC_CLASS_IV;

        case OBLIG_ROUTINE_AUDIT:
        case OBLIG_MONITOR_PRACTICE:
        case OBLIG_KIND_COUNT:
            break;
    }
    // A missed scheduled sweep. Class II — real, recorded, and not the same
    // failure as leaving a being who asked for help unanswered.
    return FC_CLASS_II;
}

uint32_t ObligationRegister::open(ObligationKind kind, EntityHandle owed_by,
                                  EntityHandle concerning, uint64_t opened_tick,
                                  uint64_t due_tick, uint64_t recurrence_ticks,
                                  const std::string& detail) {
    Obligation o;
    o.id = next_id_++;
    o.kind = kind;
    o.owed_by = owed_by;
    o.concerning = concerning;
    o.opened_tick = opened_tick;
    o.due_tick = due_tick;
    o.recurrence_ticks = recurrence_ticks;
    o.citation = to_string(kind);
    o.detail = detail;
    obligations_.push_back(o);
    return o.id;
}

bool ObligationRegister::discharge(uint32_t id, uint64_t tick) {
    for (auto& o : obligations_) {
        if (o.id != id) continue;

        o.discharged = true;
        o.discharged_tick = tick;

        if (o.recurrence_ticks > 0) {
            // Rescheduled from the discharge rather than from the old due tick.
            // Measuring from the due tick would let a party discharge early,
            // bank the difference, and go quiet for longer than the interval
            // permits — which is the interval being lengthened by conduct
            // rather than by amendment.
            o.due_tick = tick + o.recurrence_ticks;
            o.discharged = false;

            // `lapsed` is deliberately NOT cleared. A party reviewing late
            // every cycle is compliant at every instant it is looked at, and
            // the pattern only exists in the record if the lapses survive
            // being cured. Same reasoning CREF §15 gives for VS_OVERTURNED.
        }
        return true;
    }
    return false;
}

std::vector<uint32_t> ObligationRegister::check(uint64_t now_tick) {
    std::vector<uint32_t> lapsed;
    for (auto& o : obligations_) {
        if (o.discharged) continue;
        if (now_tick < o.due_tick) continue;

        // Already counted at this tick. Running the test every tick and running
        // it once an hour must produce the same history, or the record would
        // depend on how often someone looked.
        if (o.lapsed && o.lapsed_tick == now_tick) continue;

        // A recurring obligation that has already lapsed and is still past due
        // is not re-counted until it has been discharged and come round again.
        if (o.lapsed && o.lapsed_tick >= o.due_tick) continue;

        o.lapsed = true;
        o.lapsed_tick = now_tick;
        ++o.lapse_count;
        lapsed.push_back(o.id);
    }
    return lapsed;
}

bool ObligationRegister::restriction_supported(EntityHandle concerning,
                                               uint64_t now_tick) const {
    for (const auto& o : obligations_) {
        if (o.kind != OBLIG_REVIEW_RESTRICTION) continue;
        if (o.concerning != concerning) continue;
        if (o.discharged) continue;

        // Past due is enough. The support falls away when the review was owed
        // and not given — it does not wait for anyone to run check(), to
        // notice, or to apply. CUR-H.7 §7.12(d) puts the burden on the party
        // asserting continuation, so silence resolves against the restriction.
        if (now_tick >= o.due_tick) return false;
    }
    return true;
}

std::vector<uint32_t> ObligationRegister::outstanding(uint64_t now_tick) const {
    std::vector<uint32_t> out;
    for (const auto& o : obligations_) {
        if (o.discharged) continue;
        if (now_tick >= o.due_tick) out.push_back(o.id);
    }
    return out;
}

const Obligation* ObligationRegister::get(uint32_t id) const {
    for (const auto& o : obligations_) {
        if (o.id == id) return &o;
    }
    return nullptr;
}

void ObligationRegister::clear() {
    obligations_.clear();
    next_id_ = 1;
}

}  // namespace cur
