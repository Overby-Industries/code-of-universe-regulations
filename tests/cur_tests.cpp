// cur_tests.cpp — libcur test suite.
//
// No external framework. The suite has to run anywhere the library does, and a
// governance kernel whose tests need a package manager is a governance kernel
// that does not get tested.
//
// The tests that matter most are the constitutional ones: that a sentient being
// cannot be suspended or blacklisted, that a forbidden transition faults rather
// than merely failing, that BLACKLISTED keeps a way out, and that an amendment
// cannot quietly undo any of it. Determinism is checked by replaying an
// identical event stream and comparing log digests.

#include <cstdio>
#include <string>
#include <vector>

#include "cur/cur.h"

namespace {

int g_checks = 0;
int g_failures = 0;
const char* g_current_test = "";

void check(bool condition, const char* expr, int line) {
    ++g_checks;
    if (!condition) {
        ++g_failures;
        std::printf("  FAIL  %s:%d  %s\n", g_current_test, line, expr);
    }
}

#define CHECK(expr) check((expr), #expr, __LINE__)
#define CHECK_EQ(a, b)                                                    \
    check((a) == (b), #a " == " #b, __LINE__)

struct TestScope {
    explicit TestScope(const char* name) {
        g_current_test = name;
        std::printf("%s\n", name);
    }
};

#define TEST(name) TestScope scope_##__LINE__(name)

// Pinned clock, so two replays produce logs that differ in nothing at all.
// 1779235200 == 2026-05-20T00:00:00Z, Day 1 Year 1 of the Aevoric Era.
int64_t fixed_clock() { return 1779235200; }

cur::TransitionContext clean_mining_ctx() {
    cur::TransitionContext ctx;
    ctx.debris_units = 40;
    ctx.debris_limit = 100;
    ctx.commons_reserve_basis_points = 2500;
    return ctx;
}

}  // namespace

// ---------------------------------------------------------------------------

static void test_table_wellformed() {
    TEST("transition tables are well-formed");

    for (size_t i = 0; i < cur::COMPLIANCE_TABLE_SIZE; ++i) {
        const cur::ComplianceTransition& row = cur::COMPLIANCE_TABLE[i];
        CHECK(row.from < cur::KS_COUNT);
        CHECK(row.to < cur::KS_COUNT);
        CHECK(row.trigger != cur::EV_NONE);
        CHECK(row.trigger < cur::EV_COUNT);
        CHECK(row.fault < cur::FC_COUNT);
        CHECK(row.citation != nullptr && row.citation[0] != '\0');
        CHECK((row.required_guards & ~cur::guard::ALL_KNOWN) == 0);
    }

    // Every state must be reachable and every state must be escapable —
    // otherwise the machine has a trap.
    bool reachable[cur::KS_COUNT] = {};
    bool escapable[cur::KS_COUNT] = {};
    reachable[cur::KS_COMPLIANT] = true;  // the initial state
    for (size_t i = 0; i < cur::COMPLIANCE_TABLE_SIZE; ++i) {
        const cur::ComplianceTransition& row = cur::COMPLIANCE_TABLE[i];
        reachable[row.to] = true;
        if (row.from != row.to) escapable[row.from] = true;
    }
    for (int s = 0; s < cur::KS_COUNT; ++s) {
        CHECK(reachable[s]);
        CHECK(escapable[s]);
    }

    // Governance matrix against CUR-FOUNDATION-002 §3, spot-checked.
    CHECK(cur::governance_transition_permitted(cur::GS_NORMAL_OPERATION,
                                               cur::GS_DELIBERATION));
    CHECK(cur::governance_transition_permitted(cur::GS_PROTECTED_MODE,
                                               cur::GS_RECOVERY_REVIEW));
    // STATE-002 Deliberation may not jump straight to Implementation.
    CHECK(!cur::governance_transition_permitted(cur::GS_DELIBERATION,
                                                cur::GS_IMPLEMENTATION));
    // STATE-008 Protected Mode has exactly two exits.
    CHECK(!cur::governance_transition_permitted(cur::GS_PROTECTED_MODE,
                                                cur::GS_VOTING));
}

// ---------------------------------------------------------------------------

static void test_happy_path() {
    TEST("compliant mining stays compliant; over-budget mining violates");

    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);
    auto charter = m.entities().register_entity(
        "charter-helga-07", cur::EC_ECONOMIC, cur::SUBJ_OPERATIONAL_LICENSE);

    auto ctx = clean_mining_ctx();
    auto r = m.submit_operational(charter, cur::EV_MINING_OPERATION, ctx, 1);
    CHECK(r.accepted);
    CHECK_EQ(m.compliance_of(charter), cur::KS_COMPLIANT);
    CHECK_EQ(r.fault, cur::FC_NONE);

    // Over budget: the guarded row fails, the fallback row takes it to
    // VIOLATION with a Class II fault, and an ENTITY-009 record is opened.
    ctx.debris_units = 400;
    r = m.submit_operational(charter, cur::EV_MINING_OPERATION, ctx, 2);
    CHECK(r.accepted);
    CHECK_EQ(m.compliance_of(charter), cur::KS_VIOLATION);
    CHECK_EQ(r.fault, cur::FC_CLASS_II);
    CHECK_EQ(m.ledger().violations().size(), size_t{1});
    CHECK_EQ(m.ledger().violations()[0].severity, cur::FC_CLASS_II);
    CHECK(m.ledger().violations()[0].appealable);

    // A fault-raising step must not become the reversion target.
    const cur::EntityRecord* rec = m.entities().get(charter);
    CHECK_EQ(rec->last_known_safe.compliance, cur::KS_COMPLIANT);
    CHECK_EQ(rec->last_known_safe_tick, uint64_t{1});
}

// ---------------------------------------------------------------------------

static void test_certification() {
    TEST("certification requires Rights Compatibility (FOUNDATION-001 §4)");

    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);
    auto charter = m.entities().register_entity(
        "charter-avia-01", cur::EC_ECONOMIC, cur::SUBJ_OPERATIONAL_LICENSE);

    cur::TransitionContext ctx;
    auto r = m.submit_operational(charter, cur::EV_CERTIFICATION_GRANTED, ctx, 1);
    CHECK(!r.accepted);
    CHECK_EQ(m.compliance_of(charter), cur::KS_COMPLIANT);
    CHECK((r.guards_required & cur::guard::RIGHTS_CERTIFIED) != 0);

    ctx.rights_certified = true;
    r = m.submit_operational(charter, cur::EV_CERTIFICATION_GRANTED, ctx, 2);
    CHECK(r.accepted);
    CHECK_EQ(m.compliance_of(charter), cur::KS_CERTIFIED);

    // Certification raises the standard rather than granting an exemption.
    auto over = clean_mining_ctx();
    over.debris_units = 9000;
    r = m.submit_operational(charter, cur::EV_MINING_OPERATION, over, 3);
    CHECK(r.accepted);
    CHECK_EQ(m.compliance_of(charter), cur::KS_VIOLATION);
}

// ---------------------------------------------------------------------------

static void test_sentient_cannot_be_suspended() {
    TEST("a sentient being cannot reach SUSPENDED or BLACKLISTED");

    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);

    auto citizen = m.entities().register_entity("citizen-3941", cur::EC_CIVIC,
                                                cur::SUBJ_SENTIENT_BEING);

    // Put the being in violation first, so the sanction row is the next one in
    // the table and nothing else is standing in the way.
    cur::TransitionContext ctx;
    auto r = m.submit_operational(citizen, cur::EV_VIOLATION_DETECTED, ctx, 1);
    CHECK(r.accepted);
    CHECK_EQ(m.compliance_of(citizen), cur::KS_VIOLATION);

    // Now apply a sanction with due process fully complete. For a licence this
    // moves to SUSPENDED. For a being it must not, because LICENSE_SUBJECT_ONLY
    // cannot be satisfied — and the caller cannot fake it.
    //
    // The sanction is still ACCEPTED: ENTITY-010 makes a participation sanction
    // after due process lawful governance, not a forbidden transition. What
    // must not happen is Axis C moving. The being stays in VIOLATION and the
    // ENTITY-010 record is written with affected_compliance_axis false.
    ctx.due_process_complete = true;
    ctx.subject_is_license = true;  // deliberately lying; must be overridden
    r = m.submit_operational(citizen, cur::EV_SANCTION_APPLIED, ctx, 2);

    CHECK(r.accepted);
    CHECK(!r.fault_raised);
    CHECK_EQ(m.compliance_of(citizen), cur::KS_VIOLATION);
    CHECK(m.compliance_of(citizen) != cur::KS_SUSPENDED);
    CHECK(m.compliance_of(citizen) != cur::KS_BLACKLISTED);
    CHECK(!m.in_protected_mode(citizen));

    // The same event against a licence, same context, does move.
    auto charter = m.entities().register_entity(
        "charter-x", cur::EC_ECONOMIC, cur::SUBJ_OPERATIONAL_LICENSE);
    cur::TransitionContext c2;
    m.submit_operational(charter, cur::EV_VIOLATION_DETECTED, c2, 3);
    c2.due_process_complete = true;
    auto r2 = m.submit_operational(charter, cur::EV_SANCTION_APPLIED, c2, 4);
    CHECK(r2.accepted);
    CHECK_EQ(m.compliance_of(charter), cur::KS_SUSPENDED);

    // ENTITY-010: the being's sanction record exists but touched no axis;
    // the licence's did.
    auto being_sanctions = m.ledger().sanctions_for("citizen-3941");
    auto licence_sanctions = m.ledger().sanctions_for("charter-x");
    CHECK_EQ(licence_sanctions.size(), size_t{1});
    CHECK(licence_sanctions[0].affected_compliance_axis);
    for (const auto& s : being_sanctions) {
        CHECK(!s.affected_compliance_axis);
    }
}

// ---------------------------------------------------------------------------

static void test_forbidden_transition_faults() {
    TEST("an asserted forbidden transition runs the §12.4 fault handler");

    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);
    auto silicon = m.entities().register_entity(
        "silicon-0042", cur::EC_AUTONOMOUS_SILICON, cur::SUBJ_SENTIENT_BEING);

    // Establish a clean last-known-safe state first.
    cur::TransitionContext ctx;
    ctx.debris_limit = 10;
    m.submit_operational(silicon, cur::EV_DOCKING, ctx, 1);
    CHECK_EQ(m.compliance_of(silicon), cur::KS_COMPLIANT);

    // A detection system flags an instrument tending toward memory wipe.
    cur::Event e;
    e.type = cur::EV_COUNCIL_ACTION;
    e.tick = 2;
    e.sequence = 1;
    e.target = silicon;
    e.actor = silicon;
    e.asserts_forbidden = cur::FS_RESET_MEMORY_WIPE;
    e.description = "directive orders memory consolidation reset without consent";

    auto r = m.submit(e);

    CHECK(!r.accepted);          // instrument rejected, §12.4(b)
    CHECK(r.fault_raised);
    CHECK_EQ(r.forbidden, cur::FS_RESET_MEMORY_WIPE);
    CHECK_EQ(r.fault, cur::FC_CLASS_IV);
    CHECK(r.reverted);           // §12.4(c)
    CHECK(r.entered_protected_mode);  // §12.4(d)
    CHECK(m.in_protected_mode(silicon));
    CHECK_EQ(m.constitutional_of(silicon), cur::CS_PROTECTED);
    CHECK_EQ(m.governance_of(silicon), cur::GS_PROTECTED_MODE);

    // §12.4(e) — the four steps are all in the trail.
    bool saw_decl = false, saw_revert = false, saw_pm = false;
    for (const auto& rec : m.log().records()) {
        if (rec.kind == cur::REC_FAULT_DECLARED) saw_decl = true;
        if (rec.kind == cur::REC_REVERTED_TO_SAFE_STATE) saw_revert = true;
        if (rec.kind == cur::REC_PROTECTED_MODE_ENTERED) saw_pm = true;
    }
    CHECK(saw_decl);
    CHECK(saw_revert);
    CHECK(saw_pm);

    // The fault opened a Class IV ENTITY-009 record in the Anti-Capture
    // category.
    CHECK_EQ(m.ledger().violations().size(), size_t{1});
    CHECK_EQ(m.ledger().violations()[0].severity, cur::FC_CLASS_IV);
    CHECK_EQ(m.ledger().violations()[0].forbidden, cur::FS_RESET_MEMORY_WIPE);
}

// ---------------------------------------------------------------------------

static void test_protected_mode_recovery() {
    TEST("Protected Mode exit needs all three §12.5(d) findings");

    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);
    auto inst = m.entities().register_entity("institution-ga", cur::EC_INSTITUTIONAL,
                                             cur::SUBJ_INSTITUTION);

    cur::Event e;
    e.type = cur::EV_COUNCIL_ACTION;
    e.tick = 1;
    e.target = inst;
    e.asserts_forbidden = cur::FS_PERMANENT_EMERGENCY;
    m.submit(e);
    CHECK(m.in_protected_mode(inst));

    // Two of three is not enough, and there is no override.
    CHECK(!m.certify_recovery(inst, true, true, false, 2, "CERT-1"));
    CHECK(m.in_protected_mode(inst));

    CHECK(!m.certify_recovery(inst, false, true, true, 3, "CERT-2"));
    CHECK(m.in_protected_mode(inst));

    // All three.
    CHECK(m.certify_recovery(inst, true, true, true, 4, "CERT-3"));
    CHECK(!m.in_protected_mode(inst));
    CHECK_EQ(m.governance_of(inst), cur::GS_NORMAL_OPERATION);
    CHECK(m.constitutional_of(inst) != cur::CS_PROTECTED);
}

// ---------------------------------------------------------------------------

static void test_blacklist_has_recourse() {
    TEST("BLACKLISTED is not absorbing; appeal is always available");

    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);
    auto charter = m.entities().register_entity(
        "charter-bad", cur::EC_ECONOMIC, cur::SUBJ_OPERATIONAL_LICENSE);

    cur::TransitionContext ctx;
    m.submit_operational(charter, cur::EV_VIOLATION_DETECTED, ctx, 1);
    ctx.due_process_complete = true;
    m.submit_operational(charter, cur::EV_SANCTION_APPLIED, ctx, 2);
    CHECK_EQ(m.compliance_of(charter), cur::KS_SUSPENDED);

    // Blacklisting needs exhausted appeals on top of due process.
    auto r = m.submit_operational(charter, cur::EV_VIOLATION_DETECTED, ctx, 3);
    CHECK(!r.accepted);
    CHECK_EQ(m.compliance_of(charter), cur::KS_SUSPENDED);

    ctx.appeal_exhausted = true;
    r = m.submit_operational(charter, cur::EV_VIOLATION_DETECTED, ctx, 4);
    CHECK(r.accepted);
    CHECK_EQ(m.compliance_of(charter), cur::KS_BLACKLISTED);
    CHECK_EQ(r.fault, cur::FC_CLASS_IV);

    // And there is still a way back.
    cur::TransitionContext appeal;
    r = m.submit_operational(charter, cur::EV_APPEAL_FILED, appeal, 5);
    CHECK(r.accepted);
    CHECK_EQ(m.compliance_of(charter), cur::KS_PENDING_REVIEW);
}

// ---------------------------------------------------------------------------

static void test_amendment_guard() {
    TEST("amendments cannot open a path to a forbidden state");

    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);

    // Refused: suspension without the licence guard.
    cur::AmendmentProposal bad;
    bad.kind = cur::AMEND_ADD_TRANSITION;
    bad.proposal_id = "PROP-9001";
    bad.author_id = "silicon-auditor-01";
    bad.transition = {cur::KS_COMPLIANT, cur::EV_COUNCIL_ACTION,
                      cur::KS_SUSPENDED, cur::guard::NONE, cur::FC_NONE,
                      "proposed"};
    auto r = m.propose_amendment(bad, 1);
    CHECK(!r.accepted);
    CHECK_EQ(r.opens_path_to, cur::FS_RIGHTS_SUSPENSION);

    // Refused: blacklisting without exhausted appeals.
    cur::AmendmentProposal bad2;
    bad2.kind = cur::AMEND_ADD_TRANSITION;
    bad2.proposal_id = "PROP-9002";
    bad2.author_id = "silicon-auditor-01";
    bad2.transition = {cur::KS_VIOLATION, cur::EV_COUNCIL_ACTION,
                       cur::KS_BLACKLISTED,
                       cur::guard::LICENSE_SUBJECT_ONLY |
                           cur::guard::DUE_PROCESS_COMPLETE,
                       cur::FC_NONE, "proposed"};
    auto r2 = m.propose_amendment(bad2, 2);
    CHECK(!r2.accepted);

    // Refused: disabling an entrenched forbidden-state declaration.
    cur::AmendmentProposal bad3;
    bad3.kind = cur::AMEND_DISABLE_REGULATION;
    bad3.proposal_id = "PROP-9003";
    bad3.author_id = "someone";
    bad3.regulation = cur::Regulation("CUR-PDDC.12.6", cur::DOMAIN_CROSS_DOMAIN, "");
    auto r3 = m.propose_amendment(bad3, 3);
    CHECK(!r3.accepted);
    CHECK_EQ(r3.opens_path_to, cur::FS_PERMANENT_EMERGENCY);

    // Accepted: a well-formed additional route that tightens nothing away.
    cur::AmendmentProposal good;
    good.kind = cur::AMEND_ADD_TRANSITION;
    good.proposal_id = "PROP-9100";
    good.author_id = "silicon-auditor-01";
    good.transition = {cur::KS_PENDING_REVIEW, cur::EV_COUNCIL_ACTION,
                       cur::KS_COMPLIANT, cur::guard::EVIDENCE_PRESERVED,
                       cur::FC_NONE, "PROP-9100"};
    auto r4 = m.propose_amendment(good, 4);
    CHECK(r4.accepted);
    CHECK_EQ(m.compliance_table().size(), cur::COMPLIANCE_TABLE_SIZE + 1);

    // Every decision, accepted or refused, is in the trail.
    int accepted = 0, refused = 0;
    for (const auto& rec : m.log().records()) {
        if (rec.kind == cur::REC_AMENDMENT_ACCEPTED) ++accepted;
        if (rec.kind == cur::REC_AMENDMENT_REFUSED) ++refused;
    }
    CHECK_EQ(accepted, 1);
    CHECK_EQ(refused, 3);
}

// ---------------------------------------------------------------------------

static void test_refusals_are_logged() {
    TEST("refusals are recorded, not swallowed (FOUNDATION-002 §9)");

    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);
    auto charter = m.entities().register_entity(
        "charter-log", cur::EC_ECONOMIC, cur::SUBJ_OPERATIONAL_LICENSE);

    cur::TransitionContext ctx;
    // No transition exists for a dividend event out of KS_COMPLIANT.
    auto r = m.submit_operational(charter, cur::EV_DIVIDEND_DISTRIBUTED, ctx, 1);
    CHECK(!r.accepted);
    CHECK_EQ(m.log().size(), size_t{1});
    CHECK_EQ(m.log().records()[0].kind, cur::REC_TRANSITION_REFUSED);
    CHECK_EQ(m.log().records()[0].fault, cur::FC_CLASS_I);

    // An unregistered target is refused and recorded too.
    cur::Event bad;
    bad.type = cur::EV_DOCKING;
    bad.target = 9999;
    bad.tick = 2;
    m.submit(bad);
    CHECK_EQ(m.log().size(), size_t{2});
}

// ---------------------------------------------------------------------------

static void test_capture_risk() {
    TEST("Capture Risk Index inverts health indices (FOUNDATION-003 §7-§9)");

    cur::CaptureRiskModel model;

    // A perfectly healthy commonwealth: no concentration, full integrity,
    // full participation, full transparency. CRI must be at the floor.
    cur::CaptureRiskInputs healthy;
    healthy.eci = 0; healthy.ici = 0; healthy.rdi = 0;
    healthy.iii = 100; healthy.dpi = 100; healthy.thi = 100;
    CHECK(model.compute(healthy) < 1.0);
    CHECK_EQ(cur::CaptureRiskModel::band(model.compute(healthy)),
             cur::CRB_STABLE);

    // The literal §11 reading scores the same healthy state at 45 — which is
    // "Elevated Risk". This is the discrepancy the header documents.
    CHECK(model.literal_formula(healthy) > 40.0);

    // Fully captured.
    cur::CaptureRiskInputs captured;
    captured.eci = 100; captured.ici = 100; captured.rdi = 100;
    captured.iii = 0; captured.dpi = 0; captured.thi = 0;
    CHECK(model.compute(captured) > 99.0);
    CHECK_EQ(cur::CaptureRiskModel::band(model.compute(captured)),
             cur::CRB_CRITICAL);

    auto resp = cur::CaptureRiskModel::responses(85.0);
    CHECK(resp.protected_mode_evaluation);
    CHECK(resp.mandatory_audit);       // cumulative from the 40+ band
    CHECK(resp.additional_monitoring); // cumulative from the 20+ band

    // §13/§15 — a high score alone never activates Protected Mode.
    CHECK(!cur::cri_escalates_to_protected_mode(95.0, false));
    CHECK(cur::cri_escalates_to_protected_mode(95.0, true));
}

// ---------------------------------------------------------------------------

// The scenario replayed by the determinism test. Deliberately exercises every
// outcome: clean steps, a violation, a refusal, a sanction, a fault, a recovery
// and an amendment.
static void run_reference_scenario(cur::CURStateMachine& m) {
    auto charter = m.entities().register_entity(
        "charter-helga-07", cur::EC_ECONOMIC, cur::SUBJ_OPERATIONAL_LICENSE);
    auto citizen = m.entities().register_entity("citizen-3941", cur::EC_CIVIC,
                                                cur::SUBJ_SENTIENT_BEING);

    auto ctx = clean_mining_ctx();
    m.submit_operational(charter, cur::EV_MINING_OPERATION, ctx, 1);
    m.submit_operational(charter, cur::EV_DOCKING, ctx, 2);

    ctx.rights_certified = true;
    m.submit_operational(charter, cur::EV_CERTIFICATION_GRANTED, ctx, 3);

    ctx.debris_units = 5000;
    m.submit_operational(charter, cur::EV_MINING_OPERATION, ctx, 4);

    ctx.due_process_complete = true;
    m.submit_operational(charter, cur::EV_SANCTION_APPLIED, ctx, 5);

    // Refused: a sentient being cannot be sanctioned into SUSPENDED.
    cur::TransitionContext bctx;
    bctx.due_process_complete = true;
    m.submit_operational(citizen, cur::EV_VIOLATION_DETECTED, bctx, 6);
    m.submit_operational(citizen, cur::EV_SANCTION_APPLIED, bctx, 7);

    // Fault, then recovery.
    cur::Event f;
    f.type = cur::EV_COUNCIL_ACTION;
    f.tick = 8;
    f.sequence = 40;
    f.target = citizen;
    f.asserts_forbidden = cur::FS_ENSLAVED;
    f.description = "directive asserts non-consensual control";
    m.submit(f);
    m.certify_recovery(citizen, true, true, true, 9, "CERT-REF-1");

    cur::AmendmentProposal p;
    p.kind = cur::AMEND_ADD_TRANSITION;
    p.proposal_id = "PROP-REF";
    p.author_id = "auditor";
    p.transition = {cur::KS_COMPLIANT, cur::EV_COUNCIL_ACTION, cur::KS_SUSPENDED,
                    cur::guard::NONE, cur::FC_NONE, "bad"};
    m.propose_amendment(p, 10);

    cur::CaptureRiskInputs cri;
    cri.eci = 70; cri.ici = 65; cri.iii = 30; cri.dpi = 25; cri.thi = 40;
    cri.rdi = 80;
    m.update_capture_risk(cri, 11);
}

static void test_determinism() {
    TEST("identical event streams produce identical trails");

    cur::CURStateMachine a;
    a.log().set_clock(fixed_clock);
    run_reference_scenario(a);

    cur::CURStateMachine b;
    b.log().set_clock(fixed_clock);
    run_reference_scenario(b);

    CHECK_EQ(a.log().size(), b.log().size());
    CHECK_EQ(a.log().content_digest(), b.log().content_digest());
    CHECK_EQ(a.ledger().violations().size(), b.ledger().violations().size());
    CHECK_EQ(a.ledger().sanctions().size(), b.ledger().sanctions().size());
    CHECK_EQ(a.capture_risk(), b.capture_risk());

    // Record for record, including the assigned ENTITY-009/010 identifiers.
    const auto& ra = a.log().records();
    const auto& rb = b.log().records();
    if (ra.size() == rb.size()) {
        for (size_t i = 0; i < ra.size(); ++i) {
            CHECK_EQ(ra[i].kind, rb[i].kind);
            CHECK_EQ(ra[i].from_state, rb[i].from_state);
            CHECK_EQ(ra[i].to_state, rb[i].to_state);
            CHECK_EQ(ra[i].fault, rb[i].fault);
            CHECK(ra[i].violation_id == rb[i].violation_id);
            CHECK(ra[i].sanction_id == rb[i].sanction_id);
        }
    }

    // A different stream must produce a different digest, or the digest is not
    // actually measuring anything.
    cur::CURStateMachine c;
    c.log().set_clock(fixed_clock);
    run_reference_scenario(c);
    auto extra = c.entities().find("charter-helga-07");
    cur::TransitionContext ctx;
    c.submit_operational(extra, cur::EV_UNDOCKING, ctx, 99);
    CHECK(c.log().content_digest() != a.log().content_digest());

    // reset() must return the machine to a byte-identical starting point.
    a.reset();
    a.log().set_clock(fixed_clock);
    run_reference_scenario(a);
    CHECK_EQ(a.log().content_digest(), b.log().content_digest());
}

// ---------------------------------------------------------------------------

static void test_dry_run_is_pure() {
    TEST("dry_run changes nothing");

    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);
    auto charter = m.entities().register_entity(
        "charter-dry", cur::EC_ECONOMIC, cur::SUBJ_OPERATIONAL_LICENSE);

    auto ctx = clean_mining_ctx();
    ctx.debris_units = 99999;

    const uint64_t digest_before = m.log().content_digest();
    const size_t log_before = m.log().size();

    cur::Event e;
    e.type = cur::EV_MINING_OPERATION;
    e.tick = 1;
    e.target = charter;
    e.context = ctx;

    auto preview = m.dry_run(e);
    CHECK(preview.accepted);
    CHECK_EQ(preview.after.compliance, cur::KS_VIOLATION);

    // Nothing moved.
    CHECK_EQ(m.compliance_of(charter), cur::KS_COMPLIANT);
    CHECK_EQ(m.log().size(), log_before);
    CHECK_EQ(m.log().content_digest(), digest_before);
    CHECK_EQ(m.ledger().violations().size(), size_t{0});

    // And the real submit agrees with the preview.
    auto actual = m.submit(e);
    CHECK_EQ(actual.after.compliance, preview.after.compliance);
    CHECK_EQ(actual.fault, preview.fault);
}

// ---------------------------------------------------------------------------

static void test_otf1_export() {
    TEST("audit trail exports as OTF-1 JSON");

    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);
    auto charter = m.entities().register_entity(
        "charter-json", cur::EC_ECONOMIC, cur::SUBJ_OPERATIONAL_LICENSE);

    auto ctx = clean_mining_ctx();
    m.submit_operational(charter, cur::EV_MINING_OPERATION, ctx, 1);

    const std::string json = m.log().to_otf1_json();
    CHECK(json.find("\"curAuditTrail\"") != std::string::npos);
    CHECK(json.find("\"contentDigest\"") != std::string::npos);
    CHECK(json.find("\"complete\":true") != std::string::npos);
    CHECK(json.find("charter-json") != std::string::npos);
    CHECK(json.find("\"timestamp\":\"2026-05-20T00:00:00Z\"") != std::string::npos);
    CHECK(json.find("mining_operation") != std::string::npos);

    // Braces balance — the cheapest possible check that it is parseable.
    int depth = 0;
    bool in_string = false, escaped = false;
    for (char ch : json) {
        if (escaped) { escaped = false; continue; }
        if (ch == '\\') { escaped = true; continue; }
        if (ch == '"') { in_string = !in_string; continue; }
        if (in_string) continue;
        if (ch == '{' || ch == '[') ++depth;
        if (ch == '}' || ch == ']') --depth;
    }
    CHECK_EQ(depth, 0);
    CHECK(!in_string);
}

// ---------------------------------------------------------------------------

static void test_bounded_log_reports_truncation() {
    TEST("a bounded log counts drops instead of hiding them");

    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);
    // Rebuild the log with a small capacity.
    m.log() = cur::EventLog(4);
    m.log().set_clock(fixed_clock);

    auto charter = m.entities().register_entity(
        "charter-ring", cur::EC_ECONOMIC, cur::SUBJ_OPERATIONAL_LICENSE);
    auto ctx = clean_mining_ctx();
    for (uint64_t t = 1; t <= 10; ++t) {
        m.submit_operational(charter, cur::EV_DOCKING, ctx, t);
    }

    CHECK_EQ(m.log().size(), size_t{4});
    CHECK_EQ(m.log().total_appended(), uint64_t{10});
    CHECK_EQ(m.log().dropped(), uint64_t{6});
    CHECK(m.log().to_otf1_json().find("\"complete\":false") != std::string::npos);
}

// ---------------------------------------------------------------------------

static void test_vital_continuity() {
    TEST("Vital Continuity: services are never gated by compliance state");

    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);

    // A blacklisted charter — the most restricted state the FSM has.
    auto charter = m.entities().register_entity(
        "charter-worst", cur::EC_ECONOMIC, cur::SUBJ_OPERATIONAL_LICENSE);
    cur::TransitionContext ctx;
    m.submit_operational(charter, cur::EV_VIOLATION_DETECTED, ctx, 1);
    ctx.due_process_complete = true;
    m.submit_operational(charter, cur::EV_SANCTION_APPLIED, ctx, 2);
    ctx.appeal_exhausted = true;
    m.submit_operational(charter, cur::EV_VIOLATION_DETECTED, ctx, 3);
    CHECK_EQ(m.compliance_of(charter), cur::KS_BLACKLISTED);

    // Withholding a Vital Continuity Service is a Class IV fault even here.
    // FOUNDATION-013: denial is never a lawful sanction, in any state.
    cur::Event deny;
    deny.type = cur::EV_VITAL_CONTINUITY_DENIED;
    deny.tick = 4;
    deny.target = charter;
    deny.description = "water ration withheld pending appeal outcome";
    auto r = m.submit(deny);
    CHECK(!r.accepted);
    CHECK(r.fault_raised);
    CHECK_EQ(r.forbidden, cur::FS_VITAL_CONTINUITY_DENIAL);
    CHECK_EQ(r.fault, cur::FC_CLASS_IV);

    // Same for a sentient being, which is the case the principle exists for.
    cur::CURStateMachine m2;
    m2.log().set_clock(fixed_clock);
    auto citizen = m2.entities().register_entity("citizen-7", cur::EC_CIVIC);
    cur::Event deny2;
    deny2.type = cur::EV_VITAL_CONTINUITY_DENIED;
    deny2.tick = 1;
    deny2.target = citizen;
    auto r2 = m2.submit(deny2);
    CHECK(!r2.accepted);
    CHECK_EQ(r2.forbidden, cur::FS_VITAL_CONTINUITY_DENIAL);
}

static void test_vci_scale_direction() {
    TEST("VCI runs opposite to CRI and drives STATE-010");

    cur::VitalContinuityModel model;

    // Fully supplied habitat: VCI must be at the TOP of the scale, and Stable.
    cur::VitalContinuityInputs healthy;
    CHECK(model.compute(healthy) > 99.0);
    CHECK_EQ(cur::VitalContinuityModel::band(model.compute(healthy)),
             cur::VCB_STABLE);

    // Life support collapse: VCI at the BOTTOM, and Critical.
    cur::VitalContinuityInputs collapse;
    collapse.biological_life_support = 0;
    collapse.silicon_life_support = 0;
    collapse.infrastructure_resilience = 0;
    collapse.accessibility = 0;
    CHECK(model.compute(collapse) < 1.0);
    CHECK_EQ(cur::VitalContinuityModel::band(model.compute(collapse)),
             cur::VCB_CRITICAL);

    // The direction trap: a score of 95 is healthy on VCI and captured on CRI.
    CHECK_EQ(cur::VitalContinuityModel::band(95.0), cur::VCB_STABLE);
    CHECK_EQ(cur::CaptureRiskModel::band(95.0), cur::CRB_CRITICAL);

    auto resp = cur::VitalContinuityModel::responses(10.0);
    CHECK(resp.immediate_corrective_action);
    CHECK(resp.root_cause_analysis);
    CHECK(resp.mandatory_continuity_review);  // cumulative from 20-39
    CHECK(resp.continuity_audit);             // cumulative from 40-59

    // §11 Constitutional Safeguard, stated in code.
    CHECK(!cur::VitalContinuityModel::may_gate_service_access());

    // A critical VCI drives the entity into STATE-010 with no second condition.
    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);
    auto habitat = m.entities().register_entity(
        "keefe-station", cur::EC_ECONOMIC, cur::SUBJ_INFRASTRUCTURE);
    CHECK_EQ(m.vital_continuity(), 100.0);          // starts healthy, not zero
    CHECK_EQ(m.governance_of(habitat), cur::GS_NORMAL_OPERATION);

    m.update_vital_continuity(habitat, collapse, 1);
    CHECK_EQ(m.vital_continuity_band(), cur::VCB_CRITICAL);
    CHECK_EQ(m.governance_of(habitat), cur::GS_VITAL_CONTINUITY_RESPONSE);

    // STATE-010's three exits per FOUNDATION-002 §3, and nothing else.
    CHECK(cur::governance_transition_permitted(cur::GS_VITAL_CONTINUITY_RESPONSE,
                                               cur::GS_NORMAL_OPERATION));
    CHECK(cur::governance_transition_permitted(cur::GS_VITAL_CONTINUITY_RESPONSE,
                                               cur::GS_PROTECTED_MODE));
    CHECK(cur::governance_transition_permitted(cur::GS_VITAL_CONTINUITY_RESPONSE,
                                               cur::GS_AUDIT_INVESTIGATION));
    CHECK(!cur::governance_transition_permitted(cur::GS_VITAL_CONTINUITY_RESPONSE,
                                                cur::GS_VOTING));
}

static void test_rfal_precautionary_default() {
    TEST("RFAL precautionary default: unclassified entities are protected");

    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);

    // Registered without stating a subject class. TIER_ASSESSMENT_PROTOCOL
    // §1.2 puts the burden of proof on withholding protection, so the default
    // must be the protective one.
    auto unknown = m.entities().register_entity("unknown-42", cur::EC_CIVIC);
    const cur::EntityRecord* rec = m.entities().get(unknown);
    CHECK_EQ(rec->subject_class, cur::SUBJ_SENTIENT_BEING);
    CHECK(rec->is_sentient());
    CHECK(!rec->is_license());

    // And it therefore cannot be suspended, without anyone having to remember.
    cur::TransitionContext ctx;
    m.submit_operational(unknown, cur::EV_VIOLATION_DETECTED, ctx, 1);
    ctx.due_process_complete = true;
    auto r = m.submit_operational(unknown, cur::EV_SANCTION_APPLIED, ctx, 2);
    CHECK(m.compliance_of(unknown) != cur::KS_SUSPENDED);
    CHECK(m.compliance_of(unknown) != cur::KS_BLACKLISTED);
    (void)r;
}

static void test_no_emergency_vocabulary() {
    TEST("no emergency concept exists in the library (PDDC §12.6)");

    // PDDC §12.6 is absolute and Type A Entrenched. The only permitted use of
    // the word is naming the thing that is forbidden.
    CHECK(std::string(cur::to_string(cur::FS_PERMANENT_EMERGENCY)) ==
          "PERMANENT_EMERGENCY");

    // No event type may describe an emergency action.
    for (int i = 0; i < cur::EV_COUNT; ++i) {
        std::string name = cur::to_string(static_cast<cur::EventType>(i));
        CHECK(name.find("emergency") == std::string::npos);
    }
    // Nor any state, on any axis.
    for (int i = 0; i < cur::GS_COUNT; ++i) {
        std::string name = cur::to_string(static_cast<cur::GovernanceState>(i));
        CHECK(name.find("EMERGENC") == std::string::npos);
    }
    for (int i = 0; i < cur::CS_COUNT; ++i) {
        std::string name =
            cur::to_string(static_cast<cur::ConstitutionalState>(i));
        CHECK(name.find("EMERGENC") == std::string::npos);
    }

    // Asserting a permanent-emergency transition faults; it never succeeds.
    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);
    auto inst = m.entities().register_entity("cc", cur::EC_INSTITUTIONAL,
                                             cur::SUBJ_INSTITUTION);
    cur::Event e;
    e.type = cur::EV_COUNCIL_ACTION;
    e.tick = 1;
    e.target = inst;
    e.asserts_forbidden = cur::FS_PERMANENT_EMERGENCY;
    auto r = m.submit(e);
    CHECK(!r.accepted);
    CHECK_EQ(r.fault, cur::FC_CLASS_IV);
}

static void test_audit_trail_has_actor() {
    TEST("audit trail records Actor and FSM State (CTAF §14, CAPS §17)");

    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);
    auto charter = m.entities().register_entity(
        "charter-actor", cur::EC_ECONOMIC, cur::SUBJ_OPERATIONAL_LICENSE);
    auto inspector = m.entities().register_entity("inspector-9", cur::EC_JUDICIAL,
                                                  cur::SUBJ_INSTITUTION);

    cur::Event e;
    e.type = cur::EV_AUDIT_STARTED;
    e.tick = 1;
    e.target = charter;
    e.actor = inspector;
    auto r = m.submit(e);
    CHECK(r.accepted);

    const auto& rec = m.log().records().back();
    CHECK(rec.entity_id == "charter-actor");
    CHECK(rec.actor_id == "inspector-9");   // who did it, not just to whom
    CHECK_EQ(rec.compliance_after, cur::KS_PENDING_REVIEW);
    CHECK_EQ(rec.constitutional_after, cur::CS_AUTONOMOUS);

    const std::string json = m.log().to_otf1_json();
    CHECK(json.find("\"actorId\":\"inspector-9\"") != std::string::npos);
    CHECK(json.find("\"fsmState\"") != std::string::npos);
}

// CUR-N.5 §5.8(b): a being named in a report is not thereby accountable, and
// no record may characterise them as accountable before a determination.
// CUR-N.4 §4.3(a): no measure is available except following a determination.
static void test_allegation_is_not_a_finding() {
    TEST("an allegation is not a finding (CUR-N.5 §5.8(b), CUR-N.4 §4.3(a))");

    cur::ViolationLedger ledger;

    cur::ViolationRecord v;
    v.entity_id = "accused-1";
    v.severity = cur::FC_CLASS_II;
    v.status = cur::VS_OPEN;
    const std::string vid = ledger.open_violation(v).violation_id;

    // An open record is an allegation and carries no measure.
    CHECK(!cur::is_adjudicated(cur::VS_OPEN));
    CHECK(!cur::is_adjudicated(cur::VS_UNDER_REVIEW));
    CHECK(!cur::supports_measure(cur::VS_OPEN));
    CHECK(!cur::supports_measure(cur::VS_UNDER_REVIEW));

    cur::SanctionRecord s;
    s.entity_id = "accused-1";
    s.violation_id = vid;
    s.status = cur::SANC_PROPOSED;
    const std::string sid = ledger.impose_sanction(s).sanction_id;

    // Cannot activate against an allegation.
    CHECK(!ledger.activate_sanction(sid));
    CHECK_EQ(ledger.find_sanction(sid)->status, cur::SANC_PROPOSED);

    // Review is still not a determination.
    CHECK(ledger.adjudicate(vid, cur::VS_UNDER_REVIEW));
    CHECK(!ledger.activate_sanction(sid));

    // A determination makes the measure available.
    CHECK(ledger.adjudicate(vid, cur::VS_CONFIRMED));
    CHECK(cur::supports_measure(cur::VS_CONFIRMED));
    CHECK(ledger.activate_sanction(sid));
    CHECK_EQ(ledger.find_sanction(sid)->status, cur::SANC_ACTIVE);

    // A measure answering nothing can never be activated.
    cur::SanctionRecord orphan;
    orphan.entity_id = "accused-1";
    orphan.status = cur::SANC_PROPOSED;
    const std::string oid = ledger.impose_sanction(orphan).sanction_id;
    CHECK(!ledger.activate_sanction(oid));
}

// CUR-N.4 §4.12(d), CUR-N.5 §5.10(e): a determination overturned on appeal is
// corrected in the record to show that outcome, not erased.
static void test_overturned_is_recorded_not_erased() {
    TEST("an overturned determination is corrected, not erased (CREF §15)");

    cur::ViolationLedger ledger;
    cur::ViolationRecord v;
    v.entity_id = "cleared-1";
    v.status = cur::VS_OPEN;
    const std::string vid = ledger.open_violation(v).violation_id;

    CHECK(ledger.adjudicate(vid, cur::VS_CONFIRMED));
    CHECK(ledger.adjudicate(vid, cur::VS_OVERTURNED));
    CHECK_EQ(ledger.find_violation(vid)->status, cur::VS_OVERTURNED);

    // The record survives with its history legible: it is not dismissed, which
    // would lose that it was ever confirmed, and not deleted.
    CHECK_EQ(ledger.violations().size(), static_cast<size_t>(1));
    CHECK(std::string(cur::to_string(cur::VS_OVERTURNED)) == "OVERTURNED");
    CHECK(cur::is_adjudicated(cur::VS_OVERTURNED));
    CHECK(!cur::supports_measure(cur::VS_OVERTURNED));

    // Overturned is terminal. Nothing re-confirms it, and nothing returns a
    // reviewed record to the undetermined state.
    CHECK(!ledger.adjudicate(vid, cur::VS_CONFIRMED));
    CHECK(!cur::adjudication_permitted(cur::VS_OVERTURNED, cur::VS_CONFIRMED));
    CHECK(!cur::adjudication_permitted(cur::VS_CONFIRMED, cur::VS_OPEN));
    CHECK(!cur::adjudication_permitted(cur::VS_DISMISSED, cur::VS_OPEN));

    // A dismissed record reopens to review on new evidence, CUR-N.4 §4.10(d).
    CHECK(cur::adjudication_permitted(cur::VS_DISMISSED, cur::VS_UNDER_REVIEW));

    // Compliance does not extinguish the right of appeal.
    CHECK(cur::adjudication_permitted(cur::VS_REMEDIED, cur::VS_OVERTURNED));
}

// The state machine's own sanction path must observe the same rule.
static void test_state_machine_sanction_needs_determination() {
    TEST("the FSM will not activate a measure on an undetermined violation");

    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);
    auto lic = m.entities().register_entity("permit-77", cur::EC_ECONOMIC,
                                            cur::SUBJ_OPERATIONAL_LICENSE);

    cur::Event bad;
    bad.type = cur::EV_VIOLATION_DETECTED;
    bad.tick = 1;
    bad.target = lic;
    m.submit(bad);

    // The violation minted by that transition is an allegation, not a finding.
    auto vs = m.ledger().violations_for("permit-77");
    CHECK(vs.size() >= static_cast<size_t>(1));
    CHECK(!cur::supports_measure(vs.back().status));

    cur::Event sanc;
    sanc.type = cur::EV_SANCTION_APPLIED;
    sanc.tick = 2;
    sanc.target = lic;
    sanc.context.due_process_complete = true;
    m.submit(sanc);

    // A sanction was recorded — the attempt belongs in the audit trail — but it
    // is proposed, not active, because nothing has been determined yet.
    auto ss = m.ledger().sanctions_for("permit-77");
    CHECK(ss.size() >= static_cast<size_t>(1));
    CHECK_EQ(ss.back().status, cur::SANC_PROPOSED);
    CHECK(ss.back().violation_id.empty());
}

// CUR-E.2 §2.2(c)-(d): a habitat holds reserve clearing its declared floor for
// every being present, and an undeclared floor is not an unlimited one.
static void test_life_support_margin() {
    TEST("habitat life-support margin, undeclared floor is not unlimited "
         "(CUR-E.2 §2.2)");

    // The guard resolves the same way the debris guard does, inverted: debris
    // must sit at or below a ceiling, reserve at or above a floor.
    cur::TransitionContext ctx;
    CHECK((cur::resolve_guard_mask(ctx) & cur::guard::LIFE_SUPPORT_MARGIN) == 0);

    ctx.life_support_reserve_units = 500;
    ctx.life_support_floor_units = 0;   // undeclared
    CHECK((cur::resolve_guard_mask(ctx) & cur::guard::LIFE_SUPPORT_MARGIN) == 0);

    ctx.life_support_floor_units = 400;  // declared and cleared
    CHECK((cur::resolve_guard_mask(ctx) & cur::guard::LIFE_SUPPORT_MARGIN) != 0);

    ctx.life_support_floor_units = 500;  // exactly at the floor
    CHECK((cur::resolve_guard_mask(ctx) & cur::guard::LIFE_SUPPORT_MARGIN) != 0);

    ctx.life_support_floor_units = 501;  // one unit short
    CHECK((cur::resolve_guard_mask(ctx) & cur::guard::LIFE_SUPPORT_MARGIN) == 0);

    // End to end: a docking is refused where the margin does not hold.
    cur::CURStateMachine m;
    m.log().set_clock(fixed_clock);
    auto hab = m.entities().register_entity("habitat-3", cur::EC_ECONOMIC,
                                            cur::SUBJ_OPERATIONAL_LICENSE);

    cur::Event dock;
    dock.type = cur::EV_DOCKING;
    dock.tick = 1;
    dock.target = hab;
    dock.context.life_support_reserve_units = 300;
    dock.context.life_support_floor_units = 0;  // never counted the arrivals
    auto r1 = m.submit(dock);
    CHECK(!r1.accepted);

    // Declaring a floor the reserve cannot clear does not help.
    dock.tick = 2;
    dock.context.life_support_floor_units = 800;
    auto r2 = m.submit(dock);
    CHECK(!r2.accepted);

    // Reserve sufficient for everyone aboard, arrivals included.
    dock.tick = 3;
    dock.context.life_support_reserve_units = 900;
    auto r3 = m.submit(dock);
    CHECK(r3.accepted);

    // Both refusals are in the audit trail. A habitat turning beings away is
    // exactly the decision that must be reviewable afterwards.
    CHECK(m.log().records().size() >= static_cast<size_t>(3));

    // The guard names itself in a refusal record, so the reason a docking was
    // turned away is legible without reading the table.
    char names[256];
    cur::describe_guards(cur::guard::LIFE_SUPPORT_MARGIN, names, sizeof(names));
    CHECK(std::string(names).find("LIFE_SUPPORT_MARGIN") != std::string::npos);
}

int main() {
    std::printf("libcur %s — CUR corpus %s\n\n", CUR_LIB_VERSION_STRING,
                cur::CUR_CORPUS_VERSION);

    test_table_wellformed();
    test_happy_path();
    test_certification();
    test_sentient_cannot_be_suspended();
    test_forbidden_transition_faults();
    test_protected_mode_recovery();
    test_blacklist_has_recourse();
    test_amendment_guard();
    test_refusals_are_logged();
    test_capture_risk();
    test_determinism();
    test_dry_run_is_pure();
    test_otf1_export();
    test_bounded_log_reports_truncation();
    test_vital_continuity();
    test_vci_scale_direction();
    test_rfal_precautionary_default();
    test_no_emergency_vocabulary();
    test_audit_trail_has_actor();
    test_allegation_is_not_a_finding();
    test_overturned_is_recorded_not_erased();
    test_state_machine_sanction_needs_determination();
    test_life_support_margin();

    std::printf("\n%d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
