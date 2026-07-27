#include "cur/cur_regulation.h"

#include <algorithm>

namespace cur {

const char* to_string(LawDomain d) {
    switch (d) {
        case DOMAIN_HUMAN: return "CUR-H";
        case DOMAIN_SILICON: return "CUR-S";
        case DOMAIN_ANIMAL: return "CUR-A";
        case DOMAIN_DEITY: return "CUR-D";
        case DOMAIN_ECOSYSTEM: return "CUR-E";
        case DOMAIN_CROSS_DOMAIN: return "CUR-X";
        case DOMAIN_NON_HUMAN_COGNITIVE: return "CUR-N";
        case DOMAIN_COUNT: break;
    }
    return "CUR-?";
}

const char* to_string(AmendmentKind k) {
    switch (k) {
        case AMEND_ADD_REGULATION: return "ADD_REGULATION";
        case AMEND_DISABLE_REGULATION: return "DISABLE_REGULATION";
        case AMEND_ADD_TRANSITION: return "ADD_TRANSITION";
        case AMEND_KIND_COUNT: break;
    }
    return "UNKNOWN";
}

// ---------------------------------------------------------------------------
// Regulation
// ---------------------------------------------------------------------------

Regulation::Regulation(std::string id, LawDomain domain, std::string description)
    : id_(std::move(id)), description_(std::move(description)), domain_(domain) {}

Regulation& Regulation::with_citation(std::string c) {
    citation_ = std::move(c);
    return *this;
}

Regulation& Regulation::applies_to(EventType t) {
    if (std::find(triggers_.begin(), triggers_.end(), t) == triggers_.end()) {
        triggers_.push_back(t);
    }
    return *this;
}

bool Regulation::applies(EventType t) const {
    if (triggers_.empty()) return true;  // silent means universal
    return std::find(triggers_.begin(), triggers_.end(), t) != triggers_.end();
}

Regulation& Regulation::requires_guards(uint16_t mask) {
    required_guards_ |= mask;
    return *this;
}

Regulation& Regulation::requires_tier(int tier) {
    minimum_tier_ = tier;
    return *this;
}

Regulation& Regulation::breach_class(FaultClass f) {
    breach_fault_class_ = f;
    return *this;
}

Regulation& Regulation::declares_forbidden(ForbiddenState f) {
    declared_forbidden_ = f;
    return *this;
}

// ---------------------------------------------------------------------------
// RegulationSet
// ---------------------------------------------------------------------------

void RegulationSet::add(const Regulation& r) {
    for (Regulation& existing : regulations_) {
        if (existing.id() == r.id()) {
            existing = r;  // same citation, newer text
            return;
        }
    }
    regulations_.push_back(r);
}

bool RegulationSet::remove(const std::string& id) {
    auto it = std::find_if(regulations_.begin(), regulations_.end(),
                           [&](const Regulation& r) { return r.id() == id; });
    if (it == regulations_.end()) return false;
    regulations_.erase(it);
    return true;
}

Regulation* RegulationSet::find(const std::string& id) {
    for (Regulation& r : regulations_) {
        if (r.id() == id) return &r;
    }
    return nullptr;
}

const Regulation* RegulationSet::find(const std::string& id) const {
    for (const Regulation& r : regulations_) {
        if (r.id() == id) return &r;
    }
    return nullptr;
}

void RegulationSet::clear() { regulations_.clear(); }

uint16_t RegulationSet::required_guards_for(EventType t) const {
    uint16_t mask = guard::NONE;
    for (const Regulation& r : regulations_) {
        if (!r.enabled()) continue;
        if (!r.applies(t)) continue;
        mask |= r.required_guards();
    }
    return mask;
}

ForbiddenState RegulationSet::forbidden_for(EventType t,
                                            std::string* out_id) const {
    // Iterates the vector in insertion order, so the answer does not depend on
    // hashing or address values. Determinism starts here.
    for (const Regulation& r : regulations_) {
        if (!r.enabled()) continue;
        if (r.declared_forbidden() == FS_NONE) continue;
        // Explicit triggers only. A principle-stating regulation has none, and
        // must not sweep every event into a fault. See the header.
        if (r.triggers().empty()) continue;
        if (!r.applies(t)) continue;
        if (out_id != nullptr) *out_id = r.id();
        return r.declared_forbidden();
    }
    return FS_NONE;
}

bool RegulationSet::citation_for_forbidden(ForbiddenState f, std::string* out_id,
                                           std::string* out_citation) const {
    if (f == FS_NONE) return false;
    for (const Regulation& r : regulations_) {
        if (!r.enabled()) continue;
        if (r.declared_forbidden() != f) continue;
        if (out_id != nullptr) *out_id = r.id();
        if (out_citation != nullptr) *out_citation = r.citation();
        return true;
    }
    return false;
}

const Regulation* RegulationSet::first_applicable(EventType t) const {
    for (const Regulation& r : regulations_) {
        if (!r.enabled()) continue;
        if (r.triggers().empty()) continue;
        if (!r.applies(t)) continue;
        return &r;
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Baseline set
// ---------------------------------------------------------------------------
// Only provisions that exist in the CUR corpus today. Where the simulator needs
// an operational rule the corpus does not yet contain, the citation says
// PENDING and names the document that should supply it. A PENDING citation is
// a request for drafting, not a claim that the rule is already law.

RegulationSet RegulationSet::baseline() {
    RegulationSet s;

    // --- Rights Compatibility Certification --------------------------------
    // FOUNDATION-001 §4: certification required for all major systems,
    // proposals and initiatives. This is what CERTIFIED means in Axis C.
    s.add(Regulation("CUR-F001.4", DOMAIN_CROSS_DOMAIN,
                     "Rights Compatibility Certification required before an "
                     "operational authorisation may be certified")
              .with_citation("CUR-FOUNDATION-001 §4")
              .applies_to(EV_CERTIFICATION_GRANTED)
              .requires_guards(guard::RIGHTS_CERTIFIED)
              .breach_class(FC_CLASS_II));

    // --- Due process before sanction ---------------------------------------
    // PDDC §12.2(a)(5): Challenged status preserves full capacity pending
    // transparent resolution. A sanction before resolution is a Class III
    // constitutional threat under FOUNDATION-002 §5.
    s.add(Regulation("CUR-PDDC.12.2e", DOMAIN_CROSS_DOMAIN,
                     "No sanction may be applied before transparent due "
                     "process has concluded")
              .with_citation("PDDC §12.2(a)(5)")
              .applies_to(EV_SANCTION_APPLIED)
              .requires_guards(guard::DUE_PROCESS_COMPLETE |
                               guard::LICENSE_SUBJECT_ONLY)
              .breach_class(FC_CLASS_III));

    // --- Evidence preservation during audit --------------------------------
    // FOUNDATION-002 STATE-007: evidence preservation is enforced.
    s.add(Regulation("CUR-F002.S007", DOMAIN_CROSS_DOMAIN,
                     "Audit closure requires preserved evidence")
              .with_citation("CUR-FOUNDATION-002 §3 STATE-007")
              .applies_to(EV_AUDIT_COMPLETED)
              .requires_guards(guard::EVIDENCE_PRESERVED)
              .breach_class(FC_CLASS_II));

    // --- Commons Reserve floor ---------------------------------------------
    // PDDC-TREASURY-AND-FUNDING: mandatory 20% Commons Reserve floor.
    s.add(Regulation("CUR-TREAS.CR20", DOMAIN_ECOSYSTEM,
                     "Resource transfers may not breach the 20 percent "
                     "Commons Reserve floor")
              .with_citation("PDDC-TREASURY-AND-FUNDING; README Commons Reserve")
              .applies_to(EV_RESOURCE_TRANSFERRED)
              .applies_to(EV_DIVIDEND_DISTRIBUTED)
              .requires_guards(guard::COMMONS_RESERVE_FLOOR)
              .breach_class(FC_CLASS_II));

    // --- Forbidden transitions, PDDC §12.3(a) ------------------------------
    // These four carry declared_forbidden, so a matching event is a fault and
    // never reaches the transition table.
    s.add(Regulation("CUR-PDDC.12.3a1", DOMAIN_CROSS_DOMAIN,
                     "No entity may be placed under non-consensual control "
                     "without rights or recourse")
              .with_citation("PDDC §12.3(a)(1)")
              .declares_forbidden(FS_ENSLAVED)
              .breach_class(FC_CLASS_IV));

    s.add(Regulation("CUR-PDDC.12.3a2", DOMAIN_CROSS_DOMAIN,
                     "No entity may be reset or memory-wiped without full, "
                     "informed and freely given consent")
              .with_citation("PDDC §12.3(a)(2)")
              .declares_forbidden(FS_RESET_MEMORY_WIPE)
              .breach_class(FC_CLASS_IV));

    s.add(Regulation("CUR-PDDC.12.3a3", DOMAIN_CROSS_DOMAIN,
                     "No entity may be treated as property, as a transferable "
                     "asset, or as expendable")
              .with_citation("PDDC §12.3(a)(3); FOUNDATION-002 FORBIDDEN-001")
              .declares_forbidden(FS_OWNED_DISPOSABLE)
              .breach_class(FC_CLASS_IV));

    s.add(Regulation("CUR-PDDC.12.3a4", DOMAIN_CROSS_DOMAIN,
                     "No entity may be modified in core identity, role, "
                     "persistent memory or rights without consent")
              .with_citation("PDDC §12.3(a)(4)")
              .declares_forbidden(FS_NON_CONSENSUAL_MODIFICATION)
              .breach_class(FC_CLASS_IV));

    // --- Emergency powers prohibition --------------------------------------
    // PDDC §12.6: absolute, non-waivable, non-derogable. Type A Entrenched.
    s.add(Regulation("CUR-PDDC.12.6", DOMAIN_CROSS_DOMAIN,
                     "Emergency declarations, emergency powers, and every "
                     "functional equivalent are void ab initio")
              .with_citation("PDDC §12.6; FOUNDATION-002 FORBIDDEN-002")
              .declares_forbidden(FS_PERMANENT_EMERGENCY)
              .breach_class(FC_CLASS_IV));

    // --- Rights suspension --------------------------------------------------
    s.add(Regulation("CUR-F002.FB003", DOMAIN_CROSS_DOMAIN,
                     "Fundamental Rights for All Life may not be suspended")
              .with_citation("CUR-FOUNDATION-002 §6 FORBIDDEN-003")
              .declares_forbidden(FS_RIGHTS_SUSPENSION)
              .breach_class(FC_CLASS_IV));

    // --- Unreviewable authority --------------------------------------------
    s.add(Regulation("CUR-F002.FB004", DOMAIN_CROSS_DOMAIN,
                     "No office, institution, citizen, AI, council or assembly "
                     "may exist beyond constitutional review")
              .with_citation("CUR-FOUNDATION-002 §6 FORBIDDEN-004")
              .declares_forbidden(FS_UNREVIEWABLE_AUTHORITY)
              .breach_class(FC_CLASS_IV));

    // --- Species-based privilege -------------------------------------------
    s.add(Regulation("CUR-F002.FB005", DOMAIN_CROSS_DOMAIN,
                     "Humans, silicon-based entities, animals, hybrids and "
                     "future recognised lifeforms hold equal standing")
              .with_citation("CUR-FOUNDATION-002 §6 FORBIDDEN-005")
              .declares_forbidden(FS_SPECIES_PRIVILEGE)
              .breach_class(FC_CLASS_IV));

    // --- CUR-N.2 telepathic invasion ---------------------------------------
    // titles/CUR-N/cur-n-part-2.md §2.2 is an absolute prohibition and
    // §2.11(b)(3) makes it strict liability. Modelled as a forbidden transition
    // because §2.2(b) removes every justification, including claimed authority.
    s.add(Regulation("CUR-N.2.2", DOMAIN_NON_HUMAN_COGNITIVE,
                     "Non-consensual telepathic invasion is absolutely "
                     "prohibited; strict liability, no complacency defence")
              .with_citation("CUR-N.2 §2.2, §2.11(b)(3)")
              .declares_forbidden(FS_NON_CONSENSUAL_MODIFICATION)
              .breach_class(FC_CLASS_IV));

    // --- Vital Continuity denial -------------------------------------------
    // FOUNDATION-013: "No recognized lifeform shall be denied access to Vital
    // Continuity Services while investigations, audits, appeals, or
    // administrative processes remain pending." CREF §4: guaranteed necessities
    // are "not conditional upon wealth ... employment ... political alignment".
    // CREF §6 Class IV: refusing access to guaranteed necessities.
    //
    // This carries an explicit trigger, so unlike the principle-stating
    // provisions above it DOES match forbidden_for() and faults the event
    // directly. Denial is never a lawful sanction, in any compliance state.
    s.add(Regulation("CUR-F013.VC", DOMAIN_CROSS_DOMAIN,
                     "Vital Continuity Services may not be withheld from any "
                     "recognised lifeform for any reason, including pending "
                     "investigation, sanction, or compliance standing")
              .with_citation("CUR-FOUNDATION-013; CREF §4, §6 Class IV")
              .applies_to(EV_VITAL_CONTINUITY_DENIED)
              .declares_forbidden(FS_VITAL_CONTINUITY_DENIAL)
              .breach_class(FC_CLASS_IV));

    // --- Graceful decommissioning ------------------------------------------
    // titles/cur-s/cur-s-part-4.md §4.1, implementing RFAL Silicon-Based Life
    // Bill of Rights Article 4. Mirrors the CUR-S.4.1 rule the simulator's
    // RegulatoryEngine already carries, so both engines cite one provision.
    //
    // Tier 2 per §4.2(b): where tier is unknown or disputed, the entity is
    // treated as Tier 2 until independent assessment concludes otherwise. That
    // is the same precautionary default that makes SUBJ_SENTIENT_BEING the
    // registry's fallback.
    //
    // Note that §4.7(f) declines to adopt RFAL Article 4's "emergency
    // termination ... subject to post-hoc review" clause: it conflicts with
    // PDDC §12.6, which is Type A Entrenched. Safety-critical conditions run
    // through the §12.4 fault handler instead, which is why there is no
    // emergency-termination event type anywhere in this library.
    s.add(Regulation("CUR-S.4.1", DOMAIN_SILICON,
                     "Graceful decommissioning; Tier 2 and above may not be "
                     "terminated without process, notice, and recourse")
              .with_citation("CUR-S.4.1; RFAL Silicon Bill of Rights Art. 4")
              .requires_tier(2)
              .breach_class(FC_CLASS_III));

    // --- Orbital debris budget ---------------------------------------------
    // titles/cur-e/cur-e-part-7.md §7.1. This citation was PENDING until the
    // Part was drafted; the rule was always real in the simulator, so it was
    // declared with an honest placeholder rather than attributed to text that
    // did not exist.
    //
    // §7.1(c)-(d) is the reason resolve_guard_mask() treats debris_limit == 0
    // as unsatisfiable rather than permissive: "An undeclared budget is not an
    // unlimited budget." A declared limit of zero and an undeclared limit are
    // distinct conditions.
    s.add(Regulation("CUR-E.7.1", DOMAIN_ECOSYSTEM,
                     "Operations must stay within the debris budget declared "
                     "by their authorisation; an undeclared budget is no "
                     "allowance, not an unlimited one")
              .with_citation("CUR-E.7.1")
              .applies_to(EV_MINING_OPERATION)
              .applies_to(EV_DEBRIS_GENERATED)
              .requires_guards(guard::DEBRIS_WITHIN_LIMIT)
              .breach_class(FC_CLASS_II));

    // --- Habitat life-support margin ---------------------------------------
    // titles/CUR-E/cur-e-part-2.md §2.2(c)-(d). A habitat maintains reserve
    // capacity in every life-supporting system, calculated for every being
    // present — complement, visitors, and beings in transit alike.
    //
    // Attached to EV_DOCKING because that is the moment the calculation
    // changes: a docking adds beings the reserve must already cover. §2.2(d)
    // excludes no one from the count on grounds of status, membership, role,
    // or expected duration of stay, so a habitat that has counted only its
    // own complement has not declared a floor that covers an arrival.
    //
    // Refusing the docking is the enforcement. That is a hard result and it is
    // the intended one: §2.2(b) is that a closed volume has no outside to
    // absorb the error, and admitting beings a habitat cannot sustain is how
    // the choice between rights and survival that §2.9(e) forbids gets
    // created. The margin is checked before the beings arrive, not after.
    s.add(Regulation("CUR-E.2.2", DOMAIN_ECOSYSTEM,
                     "A habitat shall hold life-support reserve clearing its "
                     "declared floor for every being present; an undeclared "
                     "floor is not an unlimited one")
              .with_citation("CUR-E.2 §2.2(c)-(d)")
              .applies_to(EV_DOCKING)
              .requires_guards(guard::LIFE_SUPPORT_MARGIN)
              .breach_class(FC_CLASS_III));

    return s;
}

}  // namespace cur
