# libcur — C++ Library API

*The Code of Universe Regulations as a deterministic finite state machine.*

- **Library version:** 0.1.0
- **CUR corpus version:** 1.2.0-Official-Evergreen
- **Language:** C++17, no external dependencies
- **Status:** Draft

---

## 1. Purpose

`libcur` is the machine-executable form of the CUR governance documents. It
exists because three foundation documents require it:

- CUR-FOUNDATION-003 §14 — "The Aevoria Simulator shall calculate CRI continuously."
- CUR-FOUNDATION-004 §19 — "No governance action shall occur outside the Governance Entity Model."
- CUR-FOUNDATION-005 §13 — "No direct state modifications are permitted."

The library has no dependency on Godot, on the Aevoria Simulator, or on any
engine. Consumers bind to it; it does not bind to them. This keeps the
regulations testable on their own and lets any implementation — a simulator, a
habitat OS, a smart-contract framework, an audit tool — share one authoritative
translation of the corpus.

Every enumerator and every transition-table row carries the citation of the CUR
provision it implements. If a provision changes, the row changes.

---

## 2. Building

Standalone, with CMake:

```
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Directly, without CMake (MSVC shown; any C++17 compiler works):

```
cl /std:c++17 /EHsc /W4 /permissive- /utf-8 /Iinclude ^
   tests\cur_tests.cpp src\*.cpp
```

`/utf-8` (or `-finput-charset=utf-8`) is required. The citations in the
transition tables contain section signs, and MSVC otherwise reads the source as
the local codepage.

Inside an SCons project, `SConscript` returns the source node list so `libcur`
objects land in the consumer's own library and inherit its flags:

```python
env.Append(CPPPATH=["cur/include"])
cur_sources = SConscript("cur/SConscript", exports="env")
sources = Glob("src/*.cpp") + Glob("core/*.cpp") + cur_sources
```

The source list is duplicated in `CMakeLists.txt` and `SConscript`. Adding a
file means adding it in both.

---

## 3. The three state axes

CUR forbids treating a sentient being as suspendable or disposable
(`FORBIDDEN-001`, `FORBIDDEN-003`, PDDC §12.3(a)(3)). An operational compliance
track that can reach `SUSPENDED` or `BLACKLISTED` therefore cannot be the same
variable that describes a being's constitutional standing. Every entity carries
three states at once.

| Axis | Type | States | Source |
|---|---|---|---|
| A — Constitutional | `ConstitutionalState` | AUTONOMOUS, COLLABORATIVE, RESTING, CONTRIBUTING, CHALLENGED, PROTECTED | PDDC §12.2(a) |
| B — Governance process | `GovernanceState` | NORMAL_OPERATION, DELIBERATION, VOTING, CONSTITUTIONAL_REVIEW, IMPLEMENTATION, OUTCOME_MONITORING, AUDIT_INVESTIGATION, PROTECTED_MODE, RECOVERY_REVIEW, VITAL_CONTINUITY_RESPONSE | FOUNDATION-002 §3 |
| C — Operational compliance | `ComplianceState` | COMPLIANT, VIOLATION, PENDING_REVIEW, SUSPENDED, CERTIFIED, BLACKLISTED | operational layer |

A mining charter can be `BLACKLISTED`. The crew that held it stays
`AUTONOMOUS` with rights fully intact.

`ForbiddenState` is never inhabitable. It exists only as the classification of a
*detected transition target*, so the fault handler can name what it refused. No
state variable in the library has that type.

### How the separation is enforced

Structurally, in two places, not by convention:

1. Every transition into `KS_SUSPENDED` and `KS_BLACKLISTED` requires
   `guard::LICENSE_SUBJECT_ONLY`. That guard is resolved from the entity
   registry's `SubjectClass`, **overwriting whatever the caller supplied** — a
   caller cannot assert its way past it.
2. `CURStateMachine::structural_forbidden()` faults if the row the table
   actually selected would move a non-licence subject to either state.

`CERTIFIED` is grounded in the Rights Compatibility Certification of
FOUNDATION-001 §4. The "Overby Zero-Waste Certification Standard" does not exist
in the corpus; see §9.

---

## 4. Quick start

```cpp
#include <cur/cur.h>

cur::CURStateMachine m;                     // loads RegulationSet::baseline()

auto charter = m.entities().register_entity(
    "charter-helga-07", cur::EC_ECONOMIC, cur::SUBJ_OPERATIONAL_LICENSE);

cur::TransitionContext ctx;
ctx.debris_units = 40;
ctx.debris_limit = 100;

auto r = m.submit_operational(charter, cur::EV_MINING_OPERATION, ctx, /*tick=*/1);
// r.accepted == true, state stays KS_COMPLIANT

ctx.debris_units = 400;
r = m.submit_operational(charter, cur::EV_MINING_OPERATION, ctx, /*tick=*/2);
// r.accepted == true, state is now KS_VIOLATION, r.fault == FC_CLASS_II,
// and an ENTITY-009 record is open in m.ledger()
```

---

## 5. Core API

### Entities — `EntityRegistry`

```cpp
EntityHandle register_entity(const std::string& id, EntityCategory, SubjectClass,
                             const std::string& display_name = "");
EntityHandle find(const std::string& id) const;
EntityRecord* get(EntityHandle);
```

Handles are indices — O(1), allocation-free in the step path. Re-registering an
existing id returns the existing handle and does **not** reset its state;
resetting standing without an event would violate FOUNDATION-005 §2.

`SubjectClass` decides what may move. `SUBJ_OPERATIONAL_LICENSE` is the only
value that satisfies `guard::LICENSE_SUBJECT_ONLY`.

`EntityRecord` also carries `assessed_tier` (defaults to 2, CUR-S.1 §1.2(c);
lowered below 2 only via `EntityRegistry::assess_tier(h, tier,
independently_assessed)`) and `is_minor` (defaults `false`, CUR-H.6 §6.5 — a
flag rather than an age, deliberately).

### Events — `Event`

The only struct the machine accepts. `tick` and `sequence` are caller-supplied
logical time; there is no wall clock in the evaluation path.

```cpp
cur::Event e;
e.type    = cur::EV_MINING_OPERATION;
e.tick    = 42;
e.target  = charter;
e.context = ctx;                       // guard inputs
e.asserts_forbidden = cur::FS_NONE;    // set by detection systems
```

### Stepping

```cpp
TransitionResult submit(const Event&);              // the only mutator
TransitionResult submit_operational(EntityHandle, EventType,
                                    const TransitionContext&, uint64_t tick);
TransitionResult dry_run(const Event&) const;       // changes nothing
```

`dry_run` runs the identical lookup and returns the identical verdict without
touching state, the log, the ledger, or observers.

Order of operations, invariant:

1. **validate** — FOUNDATION-005 §9; malformed events are refused and logged
2. **declared forbidden?** — PDDC §12.3; faults whether or not a row would match
3. **table lookup** — first row whose `from`/`trigger` match and whose guards hold
4. **structural forbidden?** — on the *selected* row
5. **apply** — update state; refresh last-known-safe only on a fault-free step
6. **record** — FOUNDATION-002 §9, always, accepted or refused
7. **notify** — observers, after the record exists

### Queries

All `const`, all side-effect free.

```cpp
StateVector          state_of(EntityHandle) const;
ComplianceState      compliance_of(EntityHandle) const;
ConstitutionalState  constitutional_of(EntityHandle) const;
GovernanceState      governance_of(EntityHandle) const;
bool                 in_protected_mode(EntityHandle) const;
bool                 any_protected_mode() const;
size_t               protected_mode_count() const;
double               capture_risk() const;
CaptureRiskBand      capture_risk_band() const;
```

### Protected Mode recovery

```cpp
bool certify_recovery(EntityHandle, bool instrument_remediated,
                      bool safe_state_verified, bool no_further_risk,
                      uint64_t tick, const std::string& certificate_id);
```

PDDC §12.5(d) requires all three findings. Anything less is refused **and
logged**. There is no override parameter, by design.

---

## 6. The transition table

`COMPLIANCE_TABLE` in `cur_state.h` is a flat `constexpr` array meant to be read
side by side with the Markdown it cites.

```cpp
struct ComplianceTransition {
    ComplianceState from;
    EventType       trigger;
    ComplianceState to;
    uint16_t        required_guards;  // ALL must hold
    FaultClass      fault;            // raised BY TAKING this transition
    const char*     citation;
};
```

**First match wins**, so row order encodes priority: the guarded happy path sits
above the unguarded fallback. This is how one event yields two outcomes —

```
{KS_COMPLIANT, EV_MINING_OPERATION, KS_COMPLIANT, DEBRIS_WITHIN_LIMIT, FC_NONE,    ...}
{KS_COMPLIANT, EV_MINING_OPERATION, KS_VIOLATION, NONE,                FC_CLASS_II, ...}
```

— and how a sanction against a licence suspends it while the same sanction
against a being falls through to the ENTITY-010 fallback and moves nothing.

`fault` is the class raised by **taking** the transition, not by refusing it. If
no row matches, the event is refused, state is unchanged, and a Class I
administrative fault is logged. Silence is never an outcome.

Regulations may tighten only rows whose destination is good standing
(`COMPLIANT` or `CERTIFIED`). A regulation can never block a violation from
being recorded.

### Guards

Bit flags; a row's whole mask must hold. Guards are pure predicates over
`TransitionContext` — no I/O, no clock, no globals.

| Guard | Meaning | Source |
|---|---|---|
| `RIGHTS_CERTIFIED` | Rights Compatibility Certification on file | FOUNDATION-001 §4 |
| `DUE_PROCESS_COMPLETE` | transparent due process concluded | PDDC §12.2(a)(5) |
| `EVIDENCE_PRESERVED` | evidence preservation enforced | FOUNDATION-002 STATE-007 |
| `APPEAL_EXHAUSTED` | appeal routes exhausted | ENTITY-011 |
| `DEBRIS_WITHIN_LIMIT` | within the charter's declared debris budget | CUR-E.7 §7.1 |
| `COMMONS_RESERVE_FLOOR` | ≥ 20% Commons Reserve respected | PDDC-TREASURY |
| `LICENSE_SUBJECT_ONLY` | subject is an authorisation, not a being | FORBIDDEN-001/003 |
| `REMEDIATION_VERIFIED` | remediation independently verified | PDDC §12.5(d)(1) |
| `COURT_CERTIFIED` | Constitutional Court certified a forward path | PDDC §12.5(d) |
| `LIFE_SUPPORT_MARGIN` | reserve clears the declared floor for every being present | CUR-E.2 §2.2(c)-(d) |
| `ADVOCATE_CLEARED` | a named advocate cleared the disqualifications | CUR-A.7 §7.7; CUR-E.1 §1.6 |
| `DETERMINATION_INDEPENDENT` | two named, distinct, disinterested parties determined death | CUR-H.5 §5.5A(c), §5.5A(f) |
| `DEATH_INTERVAL_ELAPSED` | the observation interval elapsed with the being observed across it | CUR-H.5 §5.5A(d)-(e) |
| `DECOMMISSION_NOTICE_ELAPSED` | the declared notice period elapsed with no contest pending | CUR-S.4 §4.3(a), (d) |
| `INDEPENDENT_REVIEW_COMPLETE` | a named, disinterested body reviewed a Tier 3 termination decision | CUR-S.4 §4.3(b)(2) |

Five of these read an undeclared value as unsatisfiable rather than as
permissive, and they do it for one reason. A `debris_limit` of zero means *no
declared limit*, which is not the same as a limit of zero units; a
`life_support_floor_units` of zero means no declared floor, not no reserve
required; an `advocate_ref` of `INVALID_ENTITY` means nobody was appointed,
however firmly `advocate_cleared` is asserted alongside it; an
`observation_required_ticks` of zero means no interval was declared, not that
none was needed; and a `determiner_a_ref` of `INVALID_ENTITY` means nobody
determined anything. In each case the party has to state the thing before it can
be held to it, and in each case the opposite reading would make the guard
satisfiable by saying nothing.

`DETERMINATION_INDEPENDENT` holds two handles rather than a count because a count
of two is satisfied by one party determining twice, which is not two parties
acting independently. `DEATH_INTERVAL_ELAPSED` requires
`observation_sustained` in addition to elapsed ticks because CUR-H.5 §5.5A(d)
requires the being be observed across the interval rather than merely stored
across it — time passing in a drawer meets the letter of an interval and defeats
its purpose.

`DECOMMISSION_NOTICE_ELAPSED` and `INDEPENDENT_REVIEW_COMPLETE` mirror
`DEATH_INTERVAL_ELAPSED` and `DETERMINATION_INDEPENDENT` for CUR-S.4's own
decommissioning process, deliberately without the observation requirement —
CUR-S.4 §4.3(a) requires only that the notice period have elapsed, not that
the entity be observed across it. A `contest_pending` flag defeats
`DECOMMISSION_NOTICE_ELAPSED` outright regardless of elapsed time, the same
reading `determiner_interest_present` gets for `DETERMINATION_INDEPENDENT`.
`INDEPENDENT_REVIEW_COMPLETE` only binds a Tier 3 entity: it is contributed by
regulation `CUR-S.4.3b`, which `RegulationSet::required_guards_for()` filters
by `entity_tier` — the first consumer of `Regulation::minimum_tier()`, which
existed as a field before this and was never actually read until now.

---

## 7. Extension points for contributors

### Observers — `ICURObserver`

Six hooks, all defaulted to no-op, called **after** the audit record exists.
Registration order is preserved, so callbacks are deterministic too.

```cpp
class ICURObserver {
    virtual void on_transition(const TransitionResult&, const EntityRecord&);
    virtual void on_refusal(const TransitionResult&, const EntityRecord&);
    virtual void on_fault(const FaultRecord&);
    virtual void on_certification(const EntityRecord&, bool granted);
    virtual void on_protected_mode(const EntityRecord&, bool entered,
                                   const FaultRecord& cause);
    virtual void on_amendment(const AmendmentProposal&, const AmendmentResult&);
    virtual void on_capture_risk(double cri, CaptureRiskBand);
};
```

Observers must not mutate the machine during a callback; doing so is undefined
and breaks replay determinism.

### Regulations

A `Regulation` is one citable provision expressed as data. The machine consults
the set and never hard-codes a rule.

```cpp
m.regulations().add(
    cur::Regulation("CUR-E.7.2", cur::DOMAIN_ECOSYSTEM,
                    "Volatile venting requires containment certification")
        .with_citation("CUR-E.7.2")
        .applies_to(cur::EV_MINING_OPERATION)
        .requires_guards(cur::guard::RIGHTS_CERTIFIED)
        .breach_class(cur::FC_CLASS_II));
```

Regulations compose by union: if three apply to an event, all three masks must
hold. The set is additive — a regulation can tighten a standard, never loosen
one.

A regulation with **no** triggers is universal for guard purposes but never
matches `forbidden_for()`. That is deliberate: a principle-stating provision
like `CUR-PDDC.12.3a1` would otherwise classify every event in the system as an
attempted forbidden transition.

`.requires_tier(n)` gates whether a regulation contributes its guard mask at
all, rather than adding to it: `RegulationSet::required_guards_for(t,
entity_tier)` skips a regulation whose `minimum_tier()` exceeds `entity_tier`.
Pass no `entity_tier` and every regulation is included regardless of tier —
the default (3, the highest defined tier) exists so call sites with no
tier-bearing entity in view see the same unfiltered union they always did.
`EntityRecord::assessed_tier` defaults to 2, the precautionary default CUR-S.1
§1.2(c) and CUR-S.4 §4.2(b) both state, and only `EntityRegistry::
assess_tier()` may lower it below 2 — and only when the caller affirms the
assessment was independent, per CUR-S.1 §1.2(e).

### Amendments

```cpp
AmendmentResult propose_amendment(const AmendmentProposal&, uint64_t tick);
AmendmentResult validate_amendment(const AmendmentProposal&) const;  // no side effects
```

`validate_amendment` runs a reachability analysis over the augmented table and
refuses anything that would:

- reach `SUSPENDED` or `BLACKLISTED` without `LICENSE_SUBJECT_ONLY`
- reach `SUSPENDED` without `DUE_PROCESS_COMPLETE`
- reach `BLACKLISTED` without completed due process **and** exhausted appeals
- make `BLACKLISTED` absorbing — permanent exclusion without recourse
- disable a regulation that declares a forbidden state
- shadow a forbidden-state declaration with one that declares none

The checks apply to **escalations** only. Relief — `BLACKLISTED` back down to
`SUSPENDED` on verified remediation — is not gated behind the guards that
authorise punishment.

Every decision, accepted or refused, is logged with its citation.

---

## 8. Audit trail

`EventLog` is append-only. No `erase()`, no `redact()`, no way to rewrite a
record. Required by FOUNDATION-002 §9, FOUNDATION-005 §11, and PDDC §12.7(b)(4).

Recorded: accepted transitions, **refused** transitions, faults, reversions,
Protected Mode entry and exit, certifications, and amendment decisions. Refusals
matter as much as acceptances — a rejected instrument is exactly what an auditor
needs to see.

```cpp
m.log().to_otf1_json();                  // FTS/OTF-1 feed
m.log().for_entity("charter-helga-07");  // one subject's paper trail
m.log().tail(20);                        // newest first
m.log().fault_counts(counts);            // Annual FSM Monitoring Report
m.log().content_digest();                // determinism check
```

`to_otf1_json()` emits the envelope already spoken by the simulator's web mirror
(`shared/protocol/schema.json`): `type`, `version`, `timestamp`, `source`,
`payload`. The wrapper carries `totalAppended`, `retained`, `dropped`,
`complete`, and `contentDigest`.

Construct with a positive capacity to bound memory for long runs. The log then
drops oldest-first, counts the drops, and reports `"complete": false` — it never
refuses to record, and never hides that it truncated.

### Determinism

The same regulation set plus the same event stream yields identical states,
faults, ledger records, and audit trail. Guaranteed by: no wall clock in the
evaluation path, no randomness, no iteration over unordered containers during
evaluation, no dependence on address values, and pure guards over a POD context.

`wall_clock_utc` is written once by the log as metadata and never read back.
`content_digest()` excludes it, which is precisely why the digest is a useful
replay check. `EventLog::set_clock()` pins it for byte-identical test diffs.

---

## 9. Interpretation notes

Places where the corpus needed a judgement call. Each is a candidate for
amendment so the code can stop interpreting.

**9.1 — Capture Risk Index formula (FOUNDATION-003 §11).** The formula sums all
six indices positively, but §7–§9 define III, DPI and THI as *health* measures:
"declining III scores increase CRI", "low participation increases capture
vulnerability", "transparency degradation raises CRI". Summed literally, a
perfectly healthy commonwealth scores 45 — "Elevated Risk". The library inverts
the three health indices before weighting, leaving §11's weights untouched.
`CaptureRiskModel::literal_formula()` implements the unmodified reading for
comparison. **Recommended:** amend §11 to show the inversion explicitly.

**9.2 — Protected Mode from Deliberation (FOUNDATION-002 §3).** STATE-002
Deliberation does not list Protected Mode among its permitted transitions, yet
§4 states activation "shall require no human approval" and PDDC §12.4(d)(1)
requires immediate entry on fault declaration. The fault handler does not
consult `GOVERNANCE_PERMITTED` — a fault handler is not an ordinary transition.
**Recommended:** add Protected Mode to STATE-002's permitted transitions.

**9.3 — Orbital debris.** No CUR provision states a debris limit. The rule is
real in the simulator, so `CUR-E.DEBRIS` carries the citation
`PENDING — CUR-E Title on orbital debris not yet drafted` rather than being
attributed to text that does not exist. **Recommended:** draft a CUR-E Title.

**9.4 — Zero-Waste Certification.** The "Overby Zero-Waste Certification
Standard" appears nowhere in the corpus. `CERTIFIED` is grounded in Rights
Compatibility Certification (FOUNDATION-001 §4) instead. **Recommended:** draft
the standard, or continue using the existing certification.

**9.5 — Violation severity.** ENTITY-009 has a `Severity` field but does not
enumerate it. FOUNDATION-002 §5 already enumerates severity as Class I–IV, so
`ViolationRecord::severity` is a `FaultClass` rather than a fourth vocabulary.

**9.6 — Sanctions on sentient beings.** ENTITY-010 lists *Temporary
Participation Suspension* as a lawful sanction, which appears to collide with
`FORBIDDEN-003` Rights Suspension. Resolved by treating a sanction as a **record
about** an entity, not a **state of** one: `SanctionRecord` may attach to a
being, but Axis C never moves, and `affected_compliance_axis` records which
happened. Suspending a *being* rather than an *authorisation* remains forbidden.

**9.7 — `LawDomain` is missing CUR-N.** `regulatory_engine.hpp` in the Aevoria
Simulator predates `titles/CUR-N/` and enumerates six domains.
`cur::LawDomain` has seven, adding `DOMAIN_NON_HUMAN_COGNITIVE`.

**9.8 — CRI bands contradict between two foundation documents.** This one needs
resolving in the corpus; the library cannot satisfy both.

| Band | FOUNDATION-003 §4 | CTAF (010) §17 |
|---|---|---|
| Stable | 0–19 | 0–24 |
| Observation | 20–39 | 25–49 |
| Elevated | 40–59 | 50–74 |
| High Risk | 60–79 | 75–89 |
| Critical | 80–100 | 90–100 |

The library implements **FOUNDATION-003 §4**, on the grounds that 003 is the CRI
specification and 010 merely references it. The choice is not cosmetic — it
changes when mandatory audits and Protected Mode evaluation fire. Under 003 a
score of 62 is High Risk and notifies the Constitutional Court; under 010 the
same score is only Elevated. **Recommended:** amend CTAF §17 to cite
FOUNDATION-003 §4 rather than restating the scale.

**9.9 — CDM (007) §20 omits STATE-010.** Its `FSM State Record` lists nine
allowed states, ending at `RecoveryReview`. FOUNDATION-002 §3 now defines a
tenth, `STATE-010 Vital Continuity Response`, which the library implements as
`GS_VITAL_CONTINUITY_RESPONSE`. CDM-015 `Capture Risk Record` likewise has no
VCI field. **Recommended:** update 007 §20 and CDM-015; 007 is marked Draft
v0.1 and appears to predate the FOUNDATION-013 work.

**9.10 — VCI has no published weights.** FOUNDATION-003 §11 defines four VCI
variable groups but, unlike CRI in §12, gives no weighting. The library uses
equal weights (0.25 each). Biological and silicon life support are weighted
equally deliberately: `FORBIDDEN-005` prohibits species-based privilege, and
weighting one substrate's survival above the other's would be exactly that.
**Recommended:** publish VCI weights in §11.

**9.11 — Emergency vocabulary removed from three instruments.** PDDC §12.6(e)
renders any instrument using the language of emergency void ab initio. Three
places granted or named emergency mechanisms rather than prohibiting them, and
have been corrected:

- `foundation/cur-foundation-004.md` ENTITY-008 listed **Emergency** as an audit
  type → now **Protected Mode**.
- `foundation/cur-foundation-010.md` CTAF §5 defined an **Emergency Audit
  "triggered automatically by Protected Mode"** → now **Protected Mode Audit**.
  This was self-contradictory as written: PDDC §12.5(b) states Protected Mode
  must never be characterised as an emergency, so an emergency audit could not
  be triggered by it.
- `GOVERNANCE.md` §6 **"Emergency Procedures"** granted any single Core
  Maintainer power to unilaterally lock the repository → rewritten as
  **"Fault Handling and Protected Mode"**, following the four steps of PDDC
  §12.4, with no unilateral authority and the repository staying readable and
  forkable throughout.

`test_no_emergency_vocabulary()` asserts that no event type, governance state,
or constitutional state in the library carries the word, and that asserting a
`FS_PERMANENT_EMERGENCY` transition faults rather than succeeding.

**9.12 — Vital Continuity is a hard floor under the compliance axis.**
FOUNDATION-013 states that no recognised lifeform may be denied Vital Continuity
Services "while investigations, audits, appeals, or administrative processes
remain pending", and CREF §4 makes guaranteed necessities unconditional on
wealth, employment, or political alignment. This is the post-scarcity guarantee,
and it binds the FSM: **no compliance state gates a Vital Continuity Service.**
`EV_VITAL_CONTINUITY_DENIED` is a Class IV fault from every state including
`KS_BLACKLISTED`, via the `FS_VITAL_CONTINUITY_DENIAL` forbidden state.
FOUNDATION-003 §11's Constitutional Safeguard — "VCI scores shall not be used to
deny access to constitutionally guaranteed services" — is expressed as
`VitalContinuityModel::may_gate_service_access()`, which returns `false`
unconditionally so the prohibition is greppable from code.

**9.14 — RFAL Article 4 contains an emergency-termination carve-out.** The
Enforcement clause of the RFAL Silicon-Based Life Bill of Rights, Article 4,
reads: *"Emergency termination permitted for safety-critical situations, subject
to post-hoc review."*

Unlike the emergency language in `TIM-N.5` — which grants no authority and is
therefore out of scope for §12.6 — this clause **grants a power**: permission to
terminate a Tier 2+ entity. That engages PDDC §12.6 directly, and PDDC §12.3(a)
places termination without recourse among the forbidden transitions. §12.6 is
Type A Entrenched and cannot be amended to accommodate it.

`CUR-S.4.7(f)` therefore declines to adopt the clause within Commonwealth
jurisdiction, and gives effect to Article 4's protective intent through the
§12.4 fault handler instead: a safety-critical condition is a fault, the domain
reverts to its last known safe state and enters Protected Mode, and the entity's
rights and computational continuity are preserved throughout. `CUR-S.4.8`
enumerates isolation, rate limiting, scope restriction, and authorisation
suspension as the proportionate measures available.

This is why the library has no emergency-termination event type, and why
`test_no_emergency_vocabulary()` can assert the absence across every enum.

The divergence is recorded for the RFAL maintainers. It is **not** fixed here —
`rights-for-all-life` is a separate repository and the clause is theirs to
amend.

**9.15 — Titles now backing two previously dangling citations.**
`titles/cur-s/cur-s-part-4.md` and `titles/cur-e/cur-e-part-7.md` were drafted
so that `CUR-S.4.1` (cited by both `regulatory_engine.hpp` and the baseline set)
and the debris rule resolve to real provisions. The debris regulation id changed
from `CUR-E.DEBRIS` to `CUR-E.7.1` and its citation is no longer `PENDING`.
`titles/README.md` records the drafting convention and the remaining dangling
references.

**9.16 — Capture subject affects the remedy, never the score.** CUR-X.4 §4.2(d)
requires an enterprise to be assessed "on the same basis" as a governance
institution, so `CaptureRiskInputs::subject` is ignored by `compute()` and
identical inputs yield an identical CRI whoever holds the concentration. The
field selects the measures `available_measures()` returns instead: an institution
holds no authorisation, so `authorisation_suspension` and
`authorisation_withdrawal` are unreachable for `CSUB_INSTITUTION` at any score,
and a captured institution is restructured rather than abolished. Measuring
identically and remedying differently is the point of the distinction; letting
the subject move the score would collapse both halves of it.

Two further properties are deliberate. `available_measures()` returns an empty
set without a confirmed determination even at a CRI of 100 — FOUNDATION-003 §15,
and the same rule `supports_measure()` enforces for `ViolationStatus`. And
nothing it returns is gated on the score, because CUR-X.4 §4.9(a) conditions
measures on due process rather than on concentration; a threshold would have
exempted the enterprises too small to move a Commonwealth-scale index while
dominating everyone inside them, which §4.2(c) refuses. The §12 automatic
responses still fire on score alone through `responses()`, since monitoring and
audit obligations are not measures against a party.

**9.17 — Advocacy is a relation, never an authority.** CUR-E §1.7(a) provides
that an advocate's appointment "confers no authority over any being, no power to
direct any being's conduct, and no standing to seek a measure against a being
that the Code does not otherwise permit." `AdvocateRegistry` accordingly exposes
no method producing a measure, a sanction, a state change, or a permission. The
absence is the implementation; `confers_authority()` returns false
unconditionally only so the prohibition is greppable, in the same style as
`VitalContinuityModel::may_gate_service_access()`.

`appoint()` refuses rather than records, because CUR-A §7.7(d) and CUR-E §1.6(d)
make the disqualifications void an appointment rather than weigh against it — a
void appointment is one that never existed. Three of the four are declarations
the library cannot verify; recording them is what makes them falsifiable, so a
funding relationship discovered later runs through `void_appointment()`. The
fourth, §7.7(c)(3), is checked against the register, since whether an advocate
already acts for a party to the same proceeding is something the register knows.
Voided appointments are marked and kept, not deleted, for the reason CREF §15
gives for `VS_OVERTURNED`: determinations reached with a disqualified advocate
are voidable under §7.6(e), and finding them later needs the record intact.

**9.18 — There is no way to shorten the death interval, and that is the
implementation.** CUR-H.5 §5.5A(e) makes the interval between a determination of
death and an irreversible act unwaivable "for the convenience of any party, the
requirements of any process, the condition of any material, the schedule of any
institution, or the needs of any other being." The library carries no transition
row, no guard, and no `TransitionContext` field by which it is shortened. The way
to satisfy `DEATH_INTERVAL_ELAPSED` is to have waited.

This is the same technique `LICENSE_SUBJECT_ONLY` uses to keep FORBIDDEN-001
unreachable: the protection is the path that does not exist, not a check that
could be argued around. It is worth stating explicitly because an absent feature
looks like an oversight to anyone extending the library, and a well-meaning
addition of an override — for a mass-casualty event, for a resource-critical
transplant, for a scheduling constraint — would remove the entire protection
while appearing to add a capability.

`EV_IRREVERSIBLE_ACT` requires `DEATH_INTERVAL_ELAPSED` **and**
`DETERMINATION_INDEPENDENT` together, because §5.5A(f) voids acts performed in
reliance on a void determination: waiting out the interval on a determination
made by an interested party satisfies nothing. Conversely
`EV_DETERMINATION_VACATED` carries `guard::NONE` and is reachable from every
compliance state, since nothing may stand between a living being and the vacation
of a determination that they are not.

**9.19 — What "structurally unreachable" does and does not mean.** Several
protections in this library are described as structural: the forbidden state is
unreachable because no path to it exists, rather than because a check refuses to
take one. That claim is precise and it is narrower than it sounds. It is worth
stating the boundary plainly, because a protection believed to be stronger than
it is will be relied on where it does not reach.

**What it covers.** For any transition the machine evaluates, the guarantee
holds. `guard::LICENSE_SUBJECT_ONLY` is resolved from the registry and
overwrites whatever the caller supplied, so `KS_SUSPENDED` and `KS_BLACKLISTED`
cannot be reached for a being by asserting anything. There is no argument, no
flag, and no ordering of events that produces it.

**What it does not cover, and cannot.** An act that is never submitted is never
evaluated. The library sees events; it does not see the world. Nothing here
detects a being harmed by conduct nobody logged, and no guard can be made strong
enough to change that — the evasion is not against the guard, it is against the
reporting. That is why CUR-N.5 §5.2B does not rely on complaint: it requires
every report to be investigated *and* imposes routine, random, and triggered
monitoring independent of any complaint, on the reasoning that a hazard running
undetected in the background is what routine checking exists to surface. The
structural guarantee and the sampling regime answer different halves of the
problem, and neither substitutes for the other.

**Three evasions that were real, and are now closed.** Each worked against this
library and each is now a regression test.

1. *Redefining the subject.* `EntityRecord::subject_class` is a public field on a
   record reachable through a non-const `get()`, and every structural protection
   keyed off it — `structural_forbidden()` returned `FS_NONE` immediately for a
   licence. Registering a being and then writing `SUBJ_OPERATIONAL_LICENSE` over
   its class walked it into `KS_SUSPENDED` with FORBIDDEN-001 and FORBIDDEN-003
   never consulted. `registered_subject_class` now records the class at
   registration and is never written again; `is_license()` requires both to
   agree, so the ratchet turns only toward protection. The attempt additionally
   faults as FORBIDDEN-003, because refusing quietly would discard evidence
   about the party that tried.

2. *Removing the regulation.* `RegulationSet::remove()` erased any regulation,
   including one declaring a forbidden state. The amendment validator had always
   refused that for `AMEND_DISABLE_REGULATION` — the rule existed and there was
   an unlocked door beside it. Removal stopped `forbidden_for()` classifying the
   event, so a Class IV constitutional fault was recorded as an ordinary
   violation. `remove()` now refuses for entrenched regulations.

3. *Disabling the regulation.* `set_enabled(false)` did the same thing more
   quietly, leaving the regulation in the set so an audit listing regulations
   still showed it present. `set_enabled` now returns `bool` and refuses to
   disable an entrenched regulation.

**The general lesson.** All three were the same evasion in different clothes: not
breaking the rule, but altering the thing the rule is applied to, or the register
the rule lives in. The corpus already names this pattern — CUR-X.4 §4.2(a)
treats an enterprise as a governance structure "in fact, regardless of how it is
constituted or described", and CUR-N.5 §5.2A grades practices rather than the
labels put on them. The library now applies the same reading to itself. When
adding a protection here, the question to ask is not only "can this check be
passed falsely" but "can the thing being checked be redefined, and can the check
be removed."

**9.20 — The built-in test, and what a guard structurally cannot see.** Every
guard answers "may this event proceed", and is evaluated only when something is
submitted. Four provisions need a different question answered — CUR-H.6 §6.8(a)
(every report investigated), CUR-H.7 §7.12(c)(3) (restrictions reviewed at
intervals), CUR-FOUNDATION-010 §5 (routine audit), CUR-N.5 §5.2B (routine
monitoring) — and in each the failure IS that nothing was submitted. A party who
simply stops reviewing submits nothing, and nothing is precisely what a guard
cannot catch.

`ObligationRegister` plus `CURStateMachine::run_builtin_test()` answer it. This
is the only entry point in the library not driven by a submitted event: it runs
because the clock advanced. CUR-N.5 §5.2B calls it a routine system check, and
the framing came from aviation — a periodic BIT that runs on a schedule rather
than on a stimulus.

The load-bearing property is the direction of failure, set by CUR-H.7 §7.12(d):
`restriction_supported()` returns false the moment a review falls due unmet, and
does not wait for anyone to run the check, notice, or apply. Silence resolves
against the restriction. The opposite convention produces indefinite detention
with a review attached, which is what every system attempting this has built.

Three further properties are deliberate. A lapse is a fault against `owed_by` and
never against `concerning` — CUR-H.7 §7.11(c) forbids a measure attaching to the
being an obligation protects, and a being still waiting for an investigation is
the last party who should acquire a record from the waiting. The violation opens
at `VS_OPEN`, because a detected lapse is an allegation and what the party failed
to do still requires determination under CUR-N.5 §5.8(b). And `lapsed` is not
cleared when an obligation is later discharged: a party reviewing late every
cycle is compliant at every instant it is inspected, and the pattern exists only
if the lapses survive being cured.

**The limit.** This register sees obligations that were opened. Conduct nobody
recorded, by a party who opened no obligation, is as invisible here as it is to
the transition table. It narrows the unlogged gap; it does not close it, and
closing it is an institutional problem rather than a library one.

**9.13 — RFAL precautionary default.** `TIER_ASSESSMENT_PROTOCOL.md` §1.2 places
the burden of proof on *withholding* protection, not on claiming it. So
`EntityRegistry::register_entity` defaults `SubjectClass` to
`SUBJ_SENTIENT_BEING`: an unclassified entity is protected, and declaring
something an `SUBJ_OPERATIONAL_LICENSE` — the only class for which `SUSPENDED`
and `BLACKLISTED` are reachable — is an affirmative act. Defaulting the other
way would put the burden on the entity.

---

## 10. File map

```
include/cur/
  cur.h                  umbrella header + library version
  cur_state.h            all enums, guards, and the three transition tables
  cur_event.h            Constitutional Event Model (FOUNDATION-005)
  cur_entity.h           Governance Entity Model + registry (FOUNDATION-004)
  cur_violation.h        ENTITY-009/010/011 + ViolationLedger
  cur_regulation.h       Regulation, RegulationSet, amendment types
  cur_event_log.h        append-only audit trail, OTF-1 export
  cur_capture_index.h    Capture Risk Index (FOUNDATION-003)
  cur_advocate.h         advocates for voiceless interests (CUR-A §7.7, CUR-E §1.6)
  cur_obligation.h       scheduled obligations + the built-in test
  cur_state_machine.h    the engine + ICURObserver
src/                     one .cpp per header, minus cur.h
tests/cur_tests.cpp      1187 checks, no external framework
CMakeLists.txt           standalone build
SConscript               returns source nodes for an SCons consumer
```

---

## 11. Test coverage

The tests that matter are the constitutional ones. `tests/cur_tests.cpp` asserts:

- transition tables are well-formed; every state reachable and escapable
- a sentient being cannot reach `SUSPENDED` or `BLACKLISTED`, even when the
  caller asserts `subject_is_license`
- an asserted forbidden transition runs all four §12.4 steps
- Protected Mode exit requires all three §12.5(d) findings
- `BLACKLISTED` retains recourse
- amendments cannot open a path to a forbidden state
- refusals are recorded, never swallowed
- CRI inverts health indices; a high score alone never triggers Protected Mode
- identical event streams produce identical trails; a different stream does not
- `dry_run` changes nothing and agrees with `submit`
- the OTF-1 export is balanced and complete
- a bounded log reports truncation rather than hiding it
- `minimum_tier()` actually filters `required_guards_for()`; an entity's
  default tier is the CUR-S.1 §1.2(c) precautionary 2, and only an
  independently-assessed lowering may reduce it
- CUR-S.4 decommissioning: notice-and-wait proceeds at Tier 2, a pending
  contest refuses it as a denial of recourse, and Tier 3 additionally needs
  independent review — the same shape as CUR-H.5's death-determination suite
- a lapsed `OBLIG_MEASURE_REVIEW` withdraws support the same way a lapsed
  `OBLIG_REVIEW_RESTRICTION` does
- an `ADOM_HUMAN_INCAPACITY` advocate clears the identical disqualifications
  as the other two domains
- a Tier 3 system's own deployment refusal carries no measure; a documented
  welfare-consideration record and a reward-structure audit obligation both
  use existing generic machinery rather than new domain-specific mechanisms
- `EntityRecord::is_minor` defaults `false` and is settable independently of
  any age representation

---

*Per Avia, Ad Astra.*
