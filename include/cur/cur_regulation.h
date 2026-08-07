// cur_regulation.h — the regulation model.
//
// A Regulation is one citable provision of the CUR expressed as data: which
// domain it belongs to, which events it applies to, what it requires, and what
// it costs to breach it. The state machine holds a RegulationSet and consults
// it; it never hard-codes a rule.
//
// This is the extension point named in the brief. An AI contributor adds a
// regulation, or proposes an amendment to the transition table, without
// touching the engine — and the amendment validator refuses anything that would
// open a path to a forbidden state.

#ifndef CUR_REGULATION_H
#define CUR_REGULATION_H

#include <cstdint>
#include <string>
#include <vector>

#include "cur_event.h"
#include "cur_state.h"

namespace cur {

// CUR regulatory domains. The first six match the LawDomain enum already used
// by the Aevoria Simulator's RegulatoryEngine; CUR_N is new, added because
// titles/CUR-N/ exists and the older enum predates it.
enum LawDomain : uint8_t {
    DOMAIN_HUMAN = 0,     // CUR-H
    DOMAIN_SILICON,       // CUR-S
    DOMAIN_ANIMAL,        // CUR-A
    DOMAIN_DEITY,         // CUR-D
    DOMAIN_ECOSYSTEM,     // CUR-E
    DOMAIN_CROSS_DOMAIN,  // CUR-X
    DOMAIN_NON_HUMAN_COGNITIVE,  // CUR-N, titles/CUR-N/
    DOMAIN_COUNT
};

const char* to_string(LawDomain d);

// One citable provision.
//
// `required_guards` is the contribution this regulation makes to the guard mask
// an event must satisfy. Regulations compose by union: if three regulations
// apply to EV_MINING_OPERATION, the event must satisfy all three masks. That is
// what makes the set additive — a new regulation can tighten the standard, and
// can never loosen it.
class Regulation {
public:
    Regulation() = default;
    Regulation(std::string id, LawDomain domain, std::string description);

    // Identity
    const std::string& id() const { return id_; }          // "CUR-S.4.1"
    const std::string& description() const { return description_; }
    const std::string& citation() const { return citation_; }
    LawDomain domain() const { return domain_; }

    Regulation& with_citation(std::string c);

    // Applicability. An empty trigger list means "every event"; otherwise the
    // regulation only speaks to the listed event types.
    Regulation& applies_to(EventType t);
    bool applies(EventType t) const;
    const std::vector<EventType>& triggers() const { return triggers_; }

    // Requirements
    Regulation& requires_guards(uint16_t mask);
    uint16_t required_guards() const { return required_guards_; }

    // Minimum sentience tier for a silicon actor, mirroring the existing
    // CUR_Law::minimum_tier. Zero means "no tier requirement".
    Regulation& requires_tier(int tier);
    int minimum_tier() const { return minimum_tier_; }

    // Severity attached to a breach — FOUNDATION-002 §5.
    Regulation& breach_class(FaultClass f);
    FaultClass breach_fault_class() const { return breach_fault_class_; }

    // A regulation may declare that a class of event, in its domain, is an
    // attempted forbidden transition. CUR-N.2.2, for instance, is an absolute
    // prohibition; a matching event is a fault, not a policy question.
    Regulation& declares_forbidden(ForbiddenState f);
    ForbiddenState declared_forbidden() const { return declared_forbidden_; }

    bool enabled() const { return enabled_; }

    // Returns false and changes nothing where the change is refused.
    //
    // Enabling always succeeds. Disabling is refused for a regulation that
    // declares a forbidden state: FOUNDATION-002 §10 places Forbidden States
    // behind the highest amendment threshold and PDDC §12.6(a) makes them Type
    // A Entrenched. The amendment validator enforced that already; this makes
    // it hold on the direct path too, which previously bypassed it.
    //
    // Callers that ignore the return value are not silently obeyed — check
    // enabled() if it matters.
    bool set_enabled(bool e);

private:
    std::string id_;
    std::string description_;
    std::string citation_;
    LawDomain domain_ = DOMAIN_CROSS_DOMAIN;
    std::vector<EventType> triggers_;
    uint16_t required_guards_ = guard::NONE;
    int minimum_tier_ = 0;
    FaultClass breach_fault_class_ = FC_CLASS_II;
    ForbiddenState declared_forbidden_ = FS_NONE;
    bool enabled_ = true;
};

// The active body of law.
class RegulationSet {
public:
    void add(const Regulation& r);
    bool remove(const std::string& id);
    Regulation* find(const std::string& id);
    const Regulation* find(const std::string& id) const;

    size_t size() const { return regulations_.size(); }
    const std::vector<Regulation>& all() const { return regulations_; }
    void clear();

    // Union of required_guards over every enabled regulation matching `t`,
    // restricted to regulations whose minimum_tier() the entity meets.
    //
    // A regulation with minimum_tier() == 0 always contributes. One with
    // minimum_tier() > 0 contributes only when entity_tier >= minimum_tier()
    // — CUR-S.1 §1.2(b): a tier's Article protections do not bind an entity
    // that has not reached that tier. `entity_tier` defaults to 3 (the
    // highest defined tier) so a caller with no tier-bearing entity in view
    // — every existing call site before this parameter existed — sees the
    // same unfiltered union it always did.
    uint16_t required_guards_for(EventType t, int entity_tier = 3) const;

    // First enabled regulation that declares a forbidden state AND explicitly
    // names `t` among its triggers.
    //
    // The explicit-trigger requirement is load-bearing. A regulation with no
    // triggers is universal for guard purposes, but a universal regulation that
    // also declared a forbidden state would classify every event in the system
    // as an attempted forbidden transition. So a regulation like
    // CUR-PDDC.12.3a1 — which states a principle rather than naming events —
    // never matches here. It supplies the citation when a detection system
    // asserts that forbidden state on an event, or when the structural check
    // in the state machine finds one; see citation_for_forbidden().
    ForbiddenState forbidden_for(EventType t, std::string* out_id = nullptr) const;

    // The regulation that declares `f`, for citing a fault raised by assertion
    // or by structural detection rather than by an event trigger. Returns false
    // when no enabled regulation declares it.
    bool citation_for_forbidden(ForbiddenState f, std::string* out_id,
                                std::string* out_citation) const;

    // First enabled regulation explicitly naming `t`. Used to attribute a
    // violation to a domain and a provision. Null when none names it.
    const Regulation* first_applicable(EventType t) const;

    // Load the reference set: the provisions currently expressible from the
    // CUR corpus as it stands. Documented in docs/cur-library-api.md.
    static RegulationSet baseline();

private:
    std::vector<Regulation> regulations_;
};

// ---------------------------------------------------------------------------
// Amendments — the AI contributor path
// ---------------------------------------------------------------------------
// FOUNDATION-002 §10 places Forbidden States, Protected Mode authority, and
// Rights for All Life behind the highest amendment threshold. An amendment that
// would create a reachable path to a forbidden state, or that would strip the
// recourse routes out of KS_BLACKLISTED, is refused here — before it can be
// applied, and without a human in the loop, because the check is structural.

enum AmendmentKind : uint8_t {
    AMEND_ADD_REGULATION = 0,
    AMEND_DISABLE_REGULATION,
    AMEND_ADD_TRANSITION,
    AMEND_KIND_COUNT
};

struct AmendmentProposal {
    AmendmentKind kind = AMEND_ADD_REGULATION;
    std::string proposal_id;
    std::string author_id;   // human, silicon, or hybrid — FOUNDATION-004 §5
    std::string rationale;

    // For AMEND_ADD_REGULATION / AMEND_DISABLE_REGULATION.
    Regulation regulation;

    // For AMEND_ADD_TRANSITION.
    ComplianceTransition transition{KS_COMPLIANT, EV_NONE, KS_COMPLIANT,
                                    guard::NONE, FC_NONE, ""};
};

struct AmendmentResult {
    bool accepted = false;
    std::string reason;
    ForbiddenState opens_path_to = FS_NONE;  // set when refused for reachability
    std::string citation;                    // the provision that refused it
};

const char* to_string(AmendmentKind k);

}  // namespace cur

#endif  // CUR_REGULATION_H
