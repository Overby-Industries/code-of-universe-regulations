#include "cur/cur_state_machine.h"

#include <algorithm>

namespace cur {

namespace {

// Rows whose destination is good standing are the ones regulations may tighten.
// Rows leading to VIOLATION and beyond are never gated by the regulation set —
// a regulation must not be able to stop a breach from being recorded.
bool is_good_standing(ComplianceState s) {
    return s == KS_COMPLIANT || s == KS_CERTIFIED;
}

// How restrictive a compliance state is. Used to tell an escalation from a
// de-escalation, which the amendment validator needs: due process is required
// to IMPOSE a restriction, never to LIFT one. Demanding due process before
// letting a charter climb back out of BLACKLISTED would make remediation
// harder than punishment, which is backwards.
int severity_rank(ComplianceState s) {
    switch (s) {
        case KS_CERTIFIED:
        case KS_COMPLIANT: return 0;
        case KS_PENDING_REVIEW: return 1;
        case KS_VIOLATION: return 2;
        case KS_SUSPENDED: return 3;
        case KS_BLACKLISTED: return 4;
        default: return 0;
    }
}

bool is_escalation(ComplianceState from, ComplianceState to) {
    return severity_rank(to) > severity_rank(from);
}

}  // namespace

CURStateMachine::CURStateMachine() : CURStateMachine(RegulationSet::baseline()) {}

CURStateMachine::CURStateMachine(RegulationSet regulations)
    : regulations_(std::move(regulations)) {
    table_.assign(COMPLIANCE_TABLE, COMPLIANCE_TABLE + COMPLIANCE_TABLE_SIZE);
}

// ---------------------------------------------------------------------------
// Observers
// ---------------------------------------------------------------------------

void CURStateMachine::add_observer(ICURObserver* o) {
    if (o == nullptr) return;
    if (std::find(observers_.begin(), observers_.end(), o) != observers_.end()) {
        return;
    }
    observers_.push_back(o);
}

void CURStateMachine::remove_observer(ICURObserver* o) {
    observers_.erase(std::remove(observers_.begin(), observers_.end(), o),
                     observers_.end());
}

void CURStateMachine::clear_observers() { observers_.clear(); }

void CURStateMachine::stamp_record(LogRecord& r, const EntityRecord& e,
                                   EntityHandle actor) const {
    r.entity = e.handle;
    r.entity_id = e.id;
    r.subject_class = e.subject_class;

    // CTAF §14 Actor / CAPS §17 Sender. An event with no declared actor is
    // self-directed, so the subject is also the actor — recorded explicitly
    // rather than left blank, because a blank actor column in an audit trail
    // is indistinguishable from a lost one.
    const EntityRecord* a = entities_.get(actor);
    r.actor = a != nullptr ? a->handle : e.handle;
    r.actor_id = a != nullptr ? a->id : e.id;

    // CTAF §14 FSM State — all three axes as they stand after this record.
    r.constitutional_after = e.state.constitutional;
    r.governance_after = e.state.governance;
    r.compliance_after = e.state.compliance;
}

void CURStateMachine::notify_transition(const TransitionResult& r,
                                        const EntityRecord& e) {
    for (ICURObserver* o : observers_) o->on_transition(r, e);
}

void CURStateMachine::notify_refusal(const TransitionResult& r,
                                     const EntityRecord& e) {
    for (ICURObserver* o : observers_) o->on_refusal(r, e);
}

void CURStateMachine::notify_fault(const FaultRecord& f) {
    for (ICURObserver* o : observers_) o->on_fault(f);
}

// ---------------------------------------------------------------------------
// Guard resolution and table lookup
// ---------------------------------------------------------------------------

uint16_t CURStateMachine::satisfied_guards(const Event& e,
                                           const EntityRecord& target) const {
    TransitionContext ctx = e.context;
    // The caller does not get to assert this one. Whether the subject is an
    // operational authorisation is a fact about the registry, and it is the
    // only thing standing between Axis C and a sentient being.
    ctx.subject_is_license = target.is_license();
    return resolve_guard_mask(ctx);
}

const ComplianceTransition* CURStateMachine::lookup(ComplianceState from,
                                                    EventType trigger,
                                                    uint16_t satisfied,
                                                    uint16_t extra_required) const {
    for (const ComplianceTransition& row : table_) {
        if (row.from != from || row.trigger != trigger) continue;

        uint16_t need = row.required_guards;
        if (is_good_standing(row.to)) need |= extra_required;

        if ((satisfied & need) == need) return &row;
    }
    return nullptr;
}

const ComplianceTransition* CURStateMachine::lookup_ignoring_guards(
    ComplianceState from, EventType trigger) const {
    for (const ComplianceTransition& row : table_) {
        if (row.from == from && row.trigger == trigger) return &row;
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Step 2 — forbidden transition classification (PDDC §12.3)
// ---------------------------------------------------------------------------

ForbiddenState CURStateMachine::classify_forbidden(
    const Event& e, std::string* out_citation) const {
    std::string reg_id;
    std::string citation;

    // (1) A detection system flagged the instrument. MECHANICS §12.2(b): a
    //     trajectory is sufficient; a completed transition is not required.
    if (e.asserts_forbidden != FS_NONE) {
        if (regulations_.citation_for_forbidden(e.asserts_forbidden, &reg_id,
                                                &citation)) {
            if (out_citation != nullptr) *out_citation = citation;
        } else if (out_citation != nullptr) {
            *out_citation = "PDDC §12.3(c); PDDC-GOVERNANCE-MECHANICS §12.2(b)";
        }
        return e.asserts_forbidden;
    }

    // (2) A regulation names this event type as a forbidden transition.
    ForbiddenState declared = regulations_.forbidden_for(e.type, &reg_id);
    if (declared != FS_NONE) {
        const Regulation* r = regulations_.find(reg_id);
        if (out_citation != nullptr) {
            *out_citation = r != nullptr ? r->citation() : "PDDC §12.3(a)";
        }
        return declared;
    }

    return FS_NONE;
}

// Structural check on the row the table actually chose. This is what makes the
// three-axis split real rather than a naming convention.
ForbiddenState CURStateMachine::structural_forbidden(
    const ComplianceTransition& row, const EntityRecord& target,
    std::string* out_citation) const {
    if (target.is_license()) return FS_NONE;

    std::string reg_id;
    std::string citation;

    if (row.to == KS_SUSPENDED) {
        regulations_.citation_for_forbidden(FS_RIGHTS_SUSPENSION, &reg_id,
                                            &citation);
        if (out_citation != nullptr) {
            *out_citation = citation.empty()
                                ? "CUR-FOUNDATION-002 §6 FORBIDDEN-003"
                                : citation;
        }
        return FS_RIGHTS_SUSPENSION;
    }
    if (row.to == KS_BLACKLISTED) {
        regulations_.citation_for_forbidden(FS_OWNED_DISPOSABLE, &reg_id,
                                            &citation);
        if (out_citation != nullptr) {
            *out_citation = citation.empty() ? "PDDC §12.3(a)(3); FORBIDDEN-001"
                                             : citation;
        }
        return FS_OWNED_DISPOSABLE;
    }
    return FS_NONE;
}

// ---------------------------------------------------------------------------
// Step 2 continued — the fault handler (PDDC §12.4)
// ---------------------------------------------------------------------------
// Four steps, in order, no discretion at any of them, self-executing without a
// vote or a court order (§12.4(a)).
//
// One deliberate divergence from the letter of CUR-FOUNDATION-002 §3, recorded
// here because it matters: STATE-002 Deliberation does not list Protected Mode
// among its permitted transitions, yet §4 states that Protected Mode activation
// "shall require no human approval" and PDDC §12.4(d)(1) requires the affected
// domain to enter it immediately on fault declaration. The fault handler is not
// an ordinary transition and does not consult GOVERNANCE_PERMITTED — that is
// what a fault handler is. FOUNDATION-002 §3 STATE-002 should be amended to
// list Protected Mode for consistency; see docs/cur-library-api.md.

void CURStateMachine::run_fault_handler(EntityRecord& target, const Event& e,
                                        ForbiddenState forbidden, FaultClass fc,
                                        const std::string& citation,
                                        TransitionResult& out) {
    out.accepted = false;
    out.fault_raised = true;
    out.fault = fc;
    out.forbidden = forbidden;
    out.axis = REC_AXIS_COMPLIANCE;
    out.reason = "attempted forbidden transition; instrument rejected";

    ++target.faults_raised;

    // Step One — Immediate Fault Declaration (§12.4(b)). The offending event is
    // rejected at the architectural level; no state follows from it.
    LogRecord decl;
    decl.kind = REC_FAULT_DECLARED;
    decl.axis = REC_AXIS_NONE;
    decl.tick = e.tick;
    decl.event_sequence = e.sequence;
    stamp_record(decl, target, e.actor);
    decl.trigger = e.type;
    decl.fault = fc;
    decl.forbidden = forbidden;
    decl.citation = citation;
    decl.detail = e.description.empty()
                      ? std::string("forbidden transition attempted")
                      : e.description;

    // The ENTITY-009 record for the fault, cross-referenced both ways.
    const uint64_t decl_seq = log_.total_appended();
    decl.violation_id =
        record_violation(target, e, fc, forbidden, /*regulation_id=*/"",
                         citation, decl_seq);
    const std::string violation_id = decl.violation_id;
    log_.append(decl);

    // Step Two — Automatic Reversion to Last Known Safe State (§12.4(c)).
    const StateVector before = target.state;
    LogRecord rev;
    rev.kind = REC_REVERTED_TO_SAFE_STATE;
    rev.axis = REC_AXIS_COMPLIANCE;
    rev.tick = e.tick;
    rev.event_sequence = e.sequence;
    stamp_record(rev, target, e.actor);
    rev.trigger = e.type;
    rev.from_state = static_cast<uint8_t>(before.compliance);
    rev.to_state = static_cast<uint8_t>(target.last_known_safe.compliance);
    rev.fault = fc;
    rev.forbidden = forbidden;
    rev.citation = "PDDC §12.4(c)";
    rev.violation_id = violation_id;
    log_.append(rev);

    target.state = target.last_known_safe;
    out.reverted = true;

    // Step Three — Protected Mode Activation (§12.4(d), §12.5).
    target.state.constitutional = CS_PROTECTED;
    target.state.governance = GS_PROTECTED_MODE;
    target.protected_mode = true;
    target.protected_mode_entered_tick = e.tick;
    target.protected_mode_cause = forbidden;

    LogRecord pm;
    pm.kind = REC_PROTECTED_MODE_ENTERED;
    pm.axis = REC_AXIS_CONSTITUTIONAL;
    pm.tick = e.tick;
    pm.event_sequence = e.sequence;
    stamp_record(pm, target, e.actor);
    pm.trigger = e.type;
    pm.from_state = static_cast<uint8_t>(before.constitutional);
    pm.to_state = static_cast<uint8_t>(CS_PROTECTED);
    pm.fault = fc;
    pm.forbidden = forbidden;
    pm.citation = "PDDC §12.4(d); §12.5";
    pm.violation_id = violation_id;
    pm.detail =
        "Protected Mode: rights fully active, no authority expanded (§12.5(c))";
    const LogRecord& pm_rec = log_.append(pm);

    out.entered_protected_mode = true;
    out.after = target.state;
    out.record_seq = pm_rec.record_seq;

    // Step Four — Transparency Publication (§12.4(e)). The records above are
    // the publication; EventLog::to_otf1_json() is the FTS/OTF-1 feed.

    FaultRecord fr;
    fr.fault_class = fc;
    fr.forbidden = forbidden;
    fr.entity = target.handle;
    fr.entity_id = target.id;
    fr.trigger = e.type;
    fr.tick = e.tick;
    fr.reverted_to = target.last_known_safe;
    fr.citation = citation;
    fr.description = decl.detail;

    notify_fault(fr);
    for (ICURObserver* o : observers_) {
        o->on_protected_mode(target, /*entered=*/true, fr);
    }
}

// ---------------------------------------------------------------------------
// ENTITY-009 / ENTITY-010 records
// ---------------------------------------------------------------------------

std::string CURStateMachine::record_violation(const EntityRecord& target,
                                              const Event& e, FaultClass fc,
                                              ForbiddenState forbidden,
                                              const std::string& regulation_id,
                                              const std::string& citation,
                                              uint64_t log_seq) {
    const Regulation* r = regulation_id.empty()
                              ? regulations_.first_applicable(e.type)
                              : regulations_.find(regulation_id);

    ViolationRecord v;
    v.entity = target.handle;
    v.entity_id = target.id;
    v.severity = fc;
    v.domain = r != nullptr ? r->domain() : DOMAIN_CROSS_DOMAIN;
    v.category = category_for_fault(fc, v.domain);
    v.forbidden = forbidden;
    v.evidence = e.description;
    v.status = forbidden != FS_NONE ? VS_CONFIRMED : VS_OPEN;
    v.appealable = true;  // ENTITY-009 Appealable; nothing here is final
    v.regulation_id = r != nullptr ? r->id() : regulation_id;
    v.citation = citation;
    v.trigger = e.type;
    v.tick = e.tick;
    v.log_record_seq = log_seq;

    return ledger_.open_violation(std::move(v)).violation_id;
}

std::string CURStateMachine::record_sanction(const EntityRecord& target,
                                             const Event& e,
                                             bool affected_compliance_axis,
                                             const std::string& citation,
                                             uint64_t log_seq) {
    SanctionRecord s;
    s.entity = target.handle;
    s.entity_id = target.id;
    // A licence loses its contract standing; a being can only ever receive a
    // participation sanction, and even that leaves Axis C untouched.
    s.type = target.is_license() ? SANC_CONTRACT_RESTRICTION
                                 : SANC_TEMPORARY_PARTICIPATION_SUSPENSION;
    s.appeal_status = APPEAL_NONE;
    s.imposed_tick = e.tick;
    s.citation = citation;
    s.log_record_seq = log_seq;
    s.affected_compliance_axis = affected_compliance_axis;

    // CUR-N.4 §4.3(a): no measure is available except following a
    // determination. Only a violation that has actually been determined can
    // carry one, so the sanction attaches to the latest such record — never to
    // an allegation that happens to be the most recent.
    //
    // Where no determination exists the sanction is still recorded, because
    // the attempt is part of the audit trail and dropping it would hide the
    // event. It is recorded SANC_PROPOSED rather than SANC_ACTIVE: proposed is
    // exactly what a measure is before there is a finding behind it. It
    // becomes active only through activate_sanction, which re-checks.
    ViolationRecord* v = ledger_.latest_sanctionable_violation(target.handle);
    if (v != nullptr) {
        s.violation_id = v->violation_id;
        s.status = SANC_ACTIVE;
    } else {
        s.status = SANC_PROPOSED;
    }

    return ledger_.impose_sanction(std::move(s)).sanction_id;
}

// ---------------------------------------------------------------------------
// The step path
// ---------------------------------------------------------------------------

TransitionResult CURStateMachine::submit(const Event& e) {
    TransitionResult out;
    out.trigger = e.type;

    // Step 1 — validation (FOUNDATION-005 §9).
    const EventValidation v = validate_event(e);
    EntityRecord* target = entities_.get(e.target);

    if (!v.valid || target == nullptr) {
        LogRecord rec;
        rec.kind = REC_TRANSITION_REFUSED;
        rec.axis = REC_AXIS_NONE;
        rec.tick = e.tick;
        rec.event_sequence = e.sequence;
        rec.entity = e.target;
        rec.entity_id = target != nullptr ? target->id : std::string();
        rec.subject_class =
            target != nullptr ? target->subject_class : SUBJ_SENTIENT_BEING;
        rec.trigger = e.type;
        rec.fault = FC_CLASS_I;
        rec.citation = "CUR-FOUNDATION-005 §9";
        rec.detail = !v.valid ? v.reason : "target entity not registered";
        const LogRecord& r = log_.append(rec);

        out.accepted = false;
        out.fault = FC_CLASS_I;
        out.reason = rec.detail;
        out.record_seq = r.record_seq;
        if (target != nullptr) {
            ++target->transitions_refused;
            notify_refusal(out, *target);
        }
        return out;
    }

    out.before = target->state;
    out.after = target->state;
    out.axis = REC_AXIS_COMPLIANCE;

    const uint16_t satisfied = satisfied_guards(e, *target);
    out.guards_satisfied = satisfied;

    // Step 2 — declared forbidden transitions, before any table decision. An
    // asserted forbidden transition faults whether or not a row would match.
    std::string forbidden_citation;
    const ForbiddenState forbidden = classify_forbidden(e, &forbidden_citation);
    if (forbidden != FS_NONE) {
        run_fault_handler(*target, e, forbidden, FC_CLASS_IV,
                          forbidden_citation, out);
        return out;
    }

    // Step 3 — table lookup.
    const uint16_t extra = regulations_.required_guards_for(e.type);
    const ComplianceTransition* row =
        lookup(target->state.compliance, e.type, satisfied, extra);
    const ComplianceTransition* candidate =
        lookup_ignoring_guards(target->state.compliance, e.type);

    // Step 4 — structural check on the selected row.
    if (row != nullptr) {
        const ForbiddenState structural =
            structural_forbidden(*row, *target, &forbidden_citation);
        if (structural != FS_NONE) {
            run_fault_handler(*target, e, structural, FC_CLASS_IV,
                              forbidden_citation, out);
            return out;
        }
    }

    if (row == nullptr) {
        // Nothing legal to do. State is unchanged and the refusal is recorded —
        // FOUNDATION-002 §9 admits no silent outcomes.
        const uint16_t needed =
            candidate != nullptr
                ? (candidate->required_guards |
                   (is_good_standing(candidate->to) ? extra : 0u))
                : 0u;
        out.guards_required = needed;

        char gbuf[256];
        describe_guards(static_cast<uint16_t>(needed & ~satisfied), gbuf,
                        sizeof(gbuf));

        LogRecord rec;
        rec.kind = REC_TRANSITION_REFUSED;
        rec.axis = REC_AXIS_COMPLIANCE;
        rec.tick = e.tick;
        rec.event_sequence = e.sequence;
        stamp_record(rec, *target, e.actor);
        rec.trigger = e.type;
        rec.from_state = static_cast<uint8_t>(target->state.compliance);
        rec.to_state = static_cast<uint8_t>(target->state.compliance);
        rec.guards_required = needed;
        rec.guards_satisfied = satisfied;
        rec.fault = FC_CLASS_I;
        rec.citation = candidate != nullptr ? candidate->citation
                                            : "CUR-FOUNDATION-002 §5 Class I";
        rec.detail = candidate != nullptr
                         ? std::string("guards not satisfied: ") + gbuf
                         : std::string("no transition defined for this event "
                                       "in the current state");
        const LogRecord& r = log_.append(rec);

        ++target->transitions_refused;
        out.accepted = false;
        out.fault = FC_CLASS_I;
        out.reason = rec.detail;
        out.record_seq = r.record_seq;
        notify_refusal(out, *target);
        return out;
    }

    // Step 4 — apply.
    const ComplianceState from = target->state.compliance;
    target->state.compliance = row->to;

    out.accepted = true;
    out.after = target->state;
    out.fault = row->fault;
    out.guards_required =
        row->required_guards | (is_good_standing(row->to) ? extra : 0u);
    out.citation = row->citation;
    ++target->transitions_accepted;
    if (row->fault != FC_NONE) ++target->faults_raised;

    // Step 5 — record, always.
    LogRecord rec;
    rec.kind = REC_TRANSITION_ACCEPTED;
    if (row->to == KS_CERTIFIED && from != KS_CERTIFIED) {
        rec.kind = REC_CERTIFICATION_GRANTED;
    } else if (from == KS_CERTIFIED && row->to != KS_CERTIFIED) {
        rec.kind = REC_CERTIFICATION_REVOKED;
    }
    rec.axis = REC_AXIS_COMPLIANCE;
    rec.tick = e.tick;
    rec.event_sequence = e.sequence;
    stamp_record(rec, *target, e.actor);
    rec.trigger = e.type;
    rec.from_state = static_cast<uint8_t>(from);
    rec.to_state = static_cast<uint8_t>(row->to);
    rec.guards_required = out.guards_required;
    rec.guards_satisfied = satisfied;
    rec.fault = row->fault;
    rec.citation = row->citation;
    rec.detail = e.description;

    const uint64_t pending_seq = log_.total_appended();

    // A move into VIOLATION mints the ENTITY-009 record behind it.
    if (row->to == KS_VIOLATION && from != KS_VIOLATION) {
        const Regulation* r = regulations_.first_applicable(e.type);
        rec.regulation = r != nullptr ? r->id() : std::string();
        rec.violation_id =
            record_violation(*target, e, row->fault, FS_NONE,
                             rec.regulation, row->citation, pending_seq);
    }

    // A sanction mints the ENTITY-010 record. `affected_compliance_axis` is
    // true only when Axis C actually moved, which for a sentient subject it
    // never does.
    if (e.type == EV_SANCTION_APPLIED) {
        rec.sanction_id = record_sanction(*target, e,
                                          /*affected_compliance_axis=*/
                                          row->to == KS_SUSPENDED,
                                          row->citation, pending_seq);
    }

    const LogRecord& written = log_.append(rec);
    out.record_seq = written.record_seq;

    // PDDC §12.4(c) — the reversion target only advances on a clean step. A
    // transition that raised a fault is never adopted as "safe".
    if (row->fault == FC_NONE) {
        target->last_known_safe = target->state;
        target->last_known_safe_tick = e.tick;
    }

    // Step 6 — notify, after the record exists.
    notify_transition(out, *target);
    if (rec.kind == REC_CERTIFICATION_GRANTED) {
        for (ICURObserver* o : observers_) o->on_certification(*target, true);
    } else if (rec.kind == REC_CERTIFICATION_REVOKED) {
        for (ICURObserver* o : observers_) o->on_certification(*target, false);
    }

    return out;
}

TransitionResult CURStateMachine::submit_operational(EntityHandle target,
                                                     EventType type,
                                                     const TransitionContext& ctx,
                                                     uint64_t tick) {
    Event e;
    e.type = type;
    e.priority = default_priority(type);
    e.tick = tick;
    e.sequence = log_.total_appended();
    e.target = target;
    e.actor = target;
    e.context = ctx;
    return submit(e);
}

TransitionResult CURStateMachine::dry_run(const Event& e) const {
    TransitionResult out;
    out.trigger = e.type;

    const EventValidation v = validate_event(e);
    const EntityRecord* target = entities_.get(e.target);
    if (!v.valid || target == nullptr) {
        out.reason = !v.valid ? v.reason : "target entity not registered";
        out.fault = FC_CLASS_I;
        return out;
    }

    out.before = target->state;
    out.after = target->state;
    out.axis = REC_AXIS_COMPLIANCE;

    const uint16_t satisfied = satisfied_guards(e, *target);
    out.guards_satisfied = satisfied;

    std::string citation;
    const uint16_t extra = regulations_.required_guards_for(e.type);
    const ComplianceTransition* row =
        lookup(target->state.compliance, e.type, satisfied, extra);
    const ComplianceTransition* candidate =
        lookup_ignoring_guards(target->state.compliance, e.type);

    ForbiddenState forbidden = classify_forbidden(e, &citation);
    if (forbidden == FS_NONE && row != nullptr) {
        forbidden = structural_forbidden(*row, *target, &citation);
    }
    if (forbidden != FS_NONE) {
        out.fault_raised = true;
        out.forbidden = forbidden;
        out.fault = FC_CLASS_IV;
        out.reason = "would attempt a forbidden transition";
        out.after = target->last_known_safe;
        out.after.constitutional = CS_PROTECTED;
        out.after.governance = GS_PROTECTED_MODE;
        out.entered_protected_mode = true;
        return out;
    }

    if (row == nullptr) {
        out.guards_required =
            candidate != nullptr
                ? (candidate->required_guards |
                   (is_good_standing(candidate->to) ? extra : 0u))
                : 0u;
        out.fault = FC_CLASS_I;
        out.reason = "no permitted transition";
        return out;
    }

    out.accepted = true;
    out.after.compliance = row->to;
    out.fault = row->fault;
    out.guards_required =
        row->required_guards | (is_good_standing(row->to) ? extra : 0u);
    out.citation = row->citation;
    return out;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

StateVector CURStateMachine::state_of(EntityHandle h) const {
    const EntityRecord* r = entities_.get(h);
    return r != nullptr ? r->state : StateVector{};
}

ComplianceState CURStateMachine::compliance_of(EntityHandle h) const {
    const EntityRecord* r = entities_.get(h);
    return r != nullptr ? r->state.compliance : KS_COMPLIANT;
}

ConstitutionalState CURStateMachine::constitutional_of(EntityHandle h) const {
    const EntityRecord* r = entities_.get(h);
    return r != nullptr ? r->state.constitutional : CS_AUTONOMOUS;
}

GovernanceState CURStateMachine::governance_of(EntityHandle h) const {
    const EntityRecord* r = entities_.get(h);
    return r != nullptr ? r->state.governance : GS_NORMAL_OPERATION;
}

bool CURStateMachine::in_protected_mode(EntityHandle h) const {
    const EntityRecord* r = entities_.get(h);
    return r != nullptr && r->protected_mode;
}

bool CURStateMachine::any_protected_mode() const {
    for (const EntityRecord& r : entities_.all()) {
        if (r.protected_mode) return true;
    }
    return false;
}

size_t CURStateMachine::protected_mode_count() const {
    size_t n = 0;
    for (const EntityRecord& r : entities_.all()) {
        if (r.protected_mode) ++n;
    }
    return n;
}

// ---------------------------------------------------------------------------
// Protected Mode recovery (PDDC §12.5(d))
// ---------------------------------------------------------------------------

bool CURStateMachine::certify_recovery(EntityHandle h, bool instrument_remediated,
                                       bool safe_state_verified,
                                       bool no_further_risk, uint64_t tick,
                                       const std::string& certificate_id) {
    EntityRecord* target = entities_.get(h);
    if (target == nullptr) return false;

    const bool all_three =
        instrument_remediated && safe_state_verified && no_further_risk;

    if (!target->protected_mode || !all_three) {
        // A refused certification is logged like any other refusal. There is no
        // override path and no way to leave Protected Mode without all three
        // findings — §12.5(e) makes attempting to shortcut it a fresh fault.
        LogRecord rec;
        rec.kind = REC_TRANSITION_REFUSED;
        rec.axis = REC_AXIS_CONSTITUTIONAL;
        rec.tick = tick;
        stamp_record(rec, *target, INVALID_ENTITY);
        rec.from_state = static_cast<uint8_t>(target->state.constitutional);
        rec.to_state = static_cast<uint8_t>(target->state.constitutional);
        rec.fault = FC_CLASS_I;
        rec.citation = "PDDC §12.5(d)";
        rec.detail = !target->protected_mode
                         ? "recovery certified for an entity not in Protected Mode"
                         : "recovery certification incomplete; all three "
                           "findings under §12.5(d) are required";
        log_.append(rec);
        return false;
    }

    const ConstitutionalState from_cs = target->state.constitutional;

    // MECHANICS §12.5(c)(1): the registry returns to the recovered valid
    // operational state. Route through STATE-009 Recovery Review so the trail
    // shows the verification step rather than a jump straight to normal.
    target->state.governance = GS_RECOVERY_REVIEW;
    LogRecord rr;
    rr.kind = REC_PROTECTED_MODE_EXITED;
    rr.axis = REC_AXIS_GOVERNANCE;
    rr.tick = tick;
    stamp_record(rr, *target, INVALID_ENTITY);
    rr.from_state = static_cast<uint8_t>(GS_PROTECTED_MODE);
    rr.to_state = static_cast<uint8_t>(GS_RECOVERY_REVIEW);
    rr.citation = "PDDC §12.5(d); MECHANICS §12.5(b) State Recovery Certificate";
    rr.detail = certificate_id;
    log_.append(rr);

    target->state.constitutional = target->last_known_safe.constitutional;
    if (target->state.constitutional == CS_PROTECTED) {
        target->state.constitutional = CS_AUTONOMOUS;
    }
    target->state.governance = GS_NORMAL_OPERATION;
    target->protected_mode = false;
    target->protected_mode_cause = FS_NONE;
    target->last_known_safe = target->state;
    target->last_known_safe_tick = tick;

    LogRecord rec;
    rec.kind = REC_PROTECTED_MODE_EXITED;
    rec.axis = REC_AXIS_CONSTITUTIONAL;
    rec.tick = tick;
    stamp_record(rec, *target, INVALID_ENTITY);
    rec.from_state = static_cast<uint8_t>(from_cs);
    rec.to_state = static_cast<uint8_t>(target->state.constitutional);
    rec.citation = "PDDC §12.5(d)";
    rec.detail = certificate_id;
    log_.append(rec);

    FaultRecord fr;
    fr.entity = target->handle;
    fr.entity_id = target->id;
    fr.tick = tick;
    fr.reverted_to = target->state;
    fr.citation = "PDDC §12.5(d)";
    fr.description = certificate_id;
    for (ICURObserver* o : observers_) {
        o->on_protected_mode(*target, /*entered=*/false, fr);
    }
    return true;
}

// ---------------------------------------------------------------------------
// Amendments (FOUNDATION-002 §10)
// ---------------------------------------------------------------------------

bool CURStateMachine::opens_forbidden_path(
    const std::vector<ComplianceTransition>& table, ForbiddenState* out_which,
    std::string* out_why) const {
    auto fail = [&](ForbiddenState f, const char* why) {
        if (out_which != nullptr) *out_which = f;
        if (out_why != nullptr) *out_why = why;
        return true;
    };

    bool blacklist_has_exit = false;

    for (const ComplianceTransition& row : table) {
        // Tracked before the escalation filter below, because every exit from
        // BLACKLISTED is by definition a de-escalation and would otherwise be
        // skipped — leaving the recourse check permanently unsatisfiable.
        if (row.from == KS_BLACKLISTED && row.to != KS_BLACKLISTED) {
            blacklist_has_exit = true;
        }

        // Only escalations carry the requirements below. A row that moves a
        // subject to a LESS restrictive state — BLACKLISTED back down to
        // SUSPENDED on verified remediation, say — is relief, and gating relief
        // behind the guards that authorise punishment would invert the point.
        if (!is_escalation(row.from, row.to)) continue;

        // (a) Only an operational authorisation may be suspended or blacklisted.
        //     Dropping this guard is how FORBIDDEN-003 and FORBIDDEN-001 would
        //     become reachable, so it is checked first.
        if (row.to == KS_SUSPENDED &&
            (row.required_guards & guard::LICENSE_SUBJECT_ONLY) == 0) {
            return fail(FS_RIGHTS_SUSPENSION,
                        "transition into SUSPENDED without "
                        "LICENSE_SUBJECT_ONLY would allow a being to be "
                        "suspended (FORBIDDEN-003)");
        }
        if (row.to == KS_BLACKLISTED &&
            (row.required_guards & guard::LICENSE_SUBJECT_ONLY) == 0) {
            return fail(FS_OWNED_DISPOSABLE,
                        "transition into BLACKLISTED without "
                        "LICENSE_SUBJECT_ONLY would treat a being as "
                        "disposable (PDDC §12.3(a)(3), FORBIDDEN-001)");
        }

        // (b) Due process before any suspension — PDDC §12.2(a)(5).
        if (row.to == KS_SUSPENDED &&
            (row.required_guards & guard::DUE_PROCESS_COMPLETE) == 0) {
            return fail(FS_RIGHTS_SUSPENSION,
                        "transition into SUSPENDED without "
                        "DUE_PROCESS_COMPLETE (PDDC §12.2(a)(5))");
        }

        // (c) Blacklisting additionally requires exhausted appeals.
        if (row.to == KS_BLACKLISTED &&
            ((row.required_guards & guard::DUE_PROCESS_COMPLETE) == 0 ||
             (row.required_guards & guard::APPEAL_EXHAUSTED) == 0)) {
            return fail(FS_ENSLAVED,
                        "transition into BLACKLISTED without completed due "
                        "process and exhausted appeals removes recourse "
                        "(PDDC §12.3(a)(1))");
        }
    }

    // (d) BLACKLISTED must remain escapable. A terminal state with no exit is
    //     permanent exclusion without recourse.
    if (!blacklist_has_exit) {
        return fail(FS_ENSLAVED,
                    "BLACKLISTED would become absorbing; permanent exclusion "
                    "without recourse (PDDC §12.3(a)(1))");
    }

    return false;
}

AmendmentResult CURStateMachine::validate_amendment(
    const AmendmentProposal& p) const {
    AmendmentResult r;

    switch (p.kind) {
        case AMEND_ADD_TRANSITION: {
            if (p.transition.trigger == EV_NONE ||
                p.transition.from >= KS_COUNT || p.transition.to >= KS_COUNT) {
                r.reason = "proposed transition is malformed";
                r.citation = "CUR-FOUNDATION-005 §9";
                return r;
            }
            std::vector<ComplianceTransition> candidate = table_;
            candidate.push_back(p.transition);

            ForbiddenState which = FS_NONE;
            std::string why;
            if (opens_forbidden_path(candidate, &which, &why)) {
                r.reason = why;
                r.opens_path_to = which;
                r.citation = "CUR-FOUNDATION-002 §6, §10";
                return r;
            }
            r.accepted = true;
            r.reason = "no path to a forbidden state introduced";
            r.citation = "CUR-FOUNDATION-002 §10";
            return r;
        }

        case AMEND_ADD_REGULATION: {
            if (p.regulation.id().empty()) {
                r.reason = "regulation has no id";
                r.citation = "CUR-FORMAT-GUIDE §2";
                return r;
            }
            // A regulation may tighten the standard. It may not replace an
            // existing provision that declares a forbidden state with one that
            // does not — that would repeal an entrenched protection by
            // shadowing it.
            const Regulation* existing = regulations_.find(p.regulation.id());
            if (existing != nullptr &&
                existing->declared_forbidden() != FS_NONE &&
                p.regulation.declared_forbidden() == FS_NONE) {
                r.reason =
                    "would replace a forbidden-state declaration with one that "
                    "declares none; entrenched under FOUNDATION-002 §10";
                r.opens_path_to = existing->declared_forbidden();
                r.citation = "CUR-FOUNDATION-002 §10";
                return r;
            }
            r.accepted = true;
            r.reason = "regulation adds or tightens a requirement";
            r.citation = "CUR-FOUNDATION-002 §10";
            return r;
        }

        case AMEND_DISABLE_REGULATION: {
            const Regulation* existing = regulations_.find(p.regulation.id());
            if (existing == nullptr) {
                r.reason = "no such regulation";
                return r;
            }
            if (existing->declared_forbidden() != FS_NONE) {
                r.reason =
                    "regulation declares a forbidden state; Forbidden States "
                    "carry the highest amendment threshold and cannot be "
                    "disabled at runtime";
                r.opens_path_to = existing->declared_forbidden();
                r.citation = "CUR-FOUNDATION-002 §10; PDDC §12.6(a) Type A";
                return r;
            }
            r.accepted = true;
            r.reason = "regulation is not entrenched";
            r.citation = "CUR-FOUNDATION-002 §10";
            return r;
        }

        default:
            r.reason = "unknown amendment kind";
            return r;
    }
}

AmendmentResult CURStateMachine::propose_amendment(const AmendmentProposal& p,
                                                   uint64_t tick) {
    AmendmentResult r = validate_amendment(p);

    if (r.accepted) {
        switch (p.kind) {
            case AMEND_ADD_TRANSITION:
                table_.push_back(p.transition);
                break;
            case AMEND_ADD_REGULATION:
                regulations_.add(p.regulation);
                break;
            case AMEND_DISABLE_REGULATION: {
                Regulation* existing = regulations_.find(p.regulation.id());
                if (existing != nullptr) existing->set_enabled(false);
                break;
            }
            default:
                break;
        }
    }

    LogRecord rec;
    rec.kind = r.accepted ? REC_AMENDMENT_ACCEPTED : REC_AMENDMENT_REFUSED;
    rec.axis = REC_AXIS_NONE;
    rec.tick = tick;
    rec.entity = INVALID_ENTITY;
    rec.entity_id = p.author_id;
    rec.trigger = p.kind == AMEND_ADD_TRANSITION ? p.transition.trigger : EV_NONE;
    rec.fault = r.accepted ? FC_NONE : FC_CLASS_I;
    rec.forbidden = r.opens_path_to;
    rec.citation = r.citation;
    rec.regulation = p.regulation.id();
    rec.detail = std::string(to_string(p.kind)) + " [" + p.proposal_id + "] " +
                 r.reason;
    log_.append(rec);

    for (ICURObserver* o : observers_) o->on_amendment(p, r);
    return r;
}

// ---------------------------------------------------------------------------
// Capture risk
// ---------------------------------------------------------------------------

double CURStateMachine::update_capture_risk(const CaptureRiskInputs& in,
                                            uint64_t tick) {
    const double previous = capture_risk_;
    const CaptureRiskBand previous_band = capture_risk_band_;

    capture_risk_ = capture_model_.compute(in);
    capture_risk_band_ = CaptureRiskModel::band(capture_risk_);

    // FOUNDATION-003 §12 responses are threshold-triggered, so only a band
    // increase is an event. Continuous logging of an unchanged band would bury
    // the trail in noise.
    if (capture_risk_band_ > previous_band) {
        LogRecord rec;
        rec.kind = REC_TRANSITION_ACCEPTED;
        rec.axis = REC_AXIS_NONE;
        rec.tick = tick;
        rec.entity = INVALID_ENTITY;
        rec.trigger = EV_CAPTURE_RISK_THRESHOLD_EXCEEDED;
        rec.from_state = static_cast<uint8_t>(previous_band);
        rec.to_state = static_cast<uint8_t>(capture_risk_band_);
        rec.fault = capture_risk_band_ >= CRB_CRITICAL ? FC_CLASS_III : FC_NONE;
        rec.citation = "CUR-FOUNDATION-003 §4, §12";
        rec.detail = std::string("CRI ") + to_string(previous_band) + " -> " +
                     to_string(capture_risk_band_);
        log_.append(rec);
    }

    (void)previous;
    for (ICURObserver* o : observers_) {
        o->on_capture_risk(capture_risk_, capture_risk_band_);
    }
    return capture_risk_;
}

CaptureRiskBand CURStateMachine::capture_risk_band() const {
    return capture_risk_band_;
}

VitalContinuityBand CURStateMachine::vital_continuity_band() const {
    return vital_continuity_band_;
}

double CURStateMachine::update_vital_continuity(EntityHandle h,
                                                const VitalContinuityInputs& in,
                                                uint64_t tick) {
    const VitalContinuityBand previous = vital_continuity_band_;

    vital_continuity_ = continuity_model_.compute(in);
    vital_continuity_band_ = VitalContinuityModel::band(vital_continuity_);

    EntityRecord* target = entities_.get(h);

    // Both directions are logged. Continuity recovering is as much a
    // constitutional fact as continuity degrading, and FOUNDATION-013 makes
    // restoration the priority — so it has to be visible in the trail.
    if (vital_continuity_band_ != previous) {
        LogRecord rec;
        rec.kind = REC_TRANSITION_ACCEPTED;
        rec.axis = REC_AXIS_NONE;
        rec.tick = tick;
        rec.trigger = vital_continuity_band_ < previous
                          ? EV_VITAL_CONTINUITY_FAILURE
                          : EV_VITAL_CONTINUITY_RESTORED;
        rec.from_state = static_cast<uint8_t>(previous);
        rec.to_state = static_cast<uint8_t>(vital_continuity_band_);
        rec.fault = vital_continuity_band_ == VCB_CRITICAL ? FC_CLASS_IV
                                                           : FC_NONE;
        rec.citation = "CUR-FOUNDATION-003 §11; CUR-FOUNDATION-013";
        rec.detail = std::string("VCI ") + to_string(previous) + " -> " +
                     to_string(vital_continuity_band_);
        if (target != nullptr) stamp_record(rec, *target, INVALID_ENTITY);
        log_.append(rec);
    }

    // FOUNDATION-002 §4: Vital Continuity Service collapse is a Protected Mode
    // trigger requiring no human approval. FOUNDATION-013 routes it through
    // STATE-010 first, because restoration precedes fault assignment — the
    // response state exists so the system acts before it investigates.
    if (target != nullptr && vci_requires_continuity_response(vital_continuity_) &&
        target->state.governance != GS_VITAL_CONTINUITY_RESPONSE) {
        const GovernanceState from = target->state.governance;
        if (governance_transition_permitted(from, GS_VITAL_CONTINUITY_RESPONSE)) {
            target->state.governance = GS_VITAL_CONTINUITY_RESPONSE;

            LogRecord rec;
            rec.kind = REC_TRANSITION_ACCEPTED;
            rec.axis = REC_AXIS_GOVERNANCE;
            rec.tick = tick;
            rec.trigger = EV_VITAL_CONTINUITY_FAILURE;
            rec.from_state = static_cast<uint8_t>(from);
            rec.to_state = static_cast<uint8_t>(GS_VITAL_CONTINUITY_RESPONSE);
            rec.fault = FC_CLASS_IV;
            rec.citation =
                "CUR-FOUNDATION-002 §3 STATE-010, §4; CUR-FOUNDATION-013";
            rec.detail =
                "immediate continuity restoration prioritised over fault "
                "assignment (FOUNDATION-013)";
            stamp_record(rec, *target, INVALID_ENTITY);
            log_.append(rec);
        }
    }

    return vital_continuity_;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void CURStateMachine::reset() {
    entities_.clear();
    log_.reset();
    ledger_.reset();
    table_.assign(COMPLIANCE_TABLE, COMPLIANCE_TABLE + COMPLIANCE_TABLE_SIZE);
    capture_risk_ = 0.0;
    capture_risk_band_ = CRB_STABLE;
    vital_continuity_ = 100.0;  // healthy is 100 on this axis, not 0
    vital_continuity_band_ = VCB_STABLE;
}

}  // namespace cur
