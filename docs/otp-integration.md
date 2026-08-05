# OTP Integration — How CUR Publishes into the Overby Ledger

*What CUR-X.5 requires of a publishing mechanism, and where each requirement is
satisfied in `transparency-protocol`.*

- **CUR corpus version:** 1.2.0-Official-Evergreen
- **CUR-X.5 version:** 1.0.0-Draft
- **OTP library version:** 1.0.0 (`transparency-protocol`, Apache 2.0)
- **Status:** Draft

---

## 1. Purpose

CUR-X.5 states what must be published, in what form, and the limits on its
use. It deliberately states properties rather than a mechanism — §5.4's own
note says an implementation is worked example, not requirement. The Overby
Transparency Protocol (`otp::Ledger` and `otp::Registry`, in the sibling
repository `transparency-protocol`) is that worked example, cited under
§5.4(g) and in this Part's own header as the design record CUR-X.5 is
referenced by.

This document is the mapping the two sides leave implicit. CUR-X.5 does not
name `otp::` types, because a title that named a specific library would stop
being satisfiable by any other. `transparency-protocol` does not know the CUR
corpus — its headers say so directly, and `kind`, `citation`, and `detail` are
free-form strings for exactly that reason. Neither document states the
correspondence between the two, and a mechanism built by reading one without
the other is a mechanism that satisfies whichever text its author happened to
read. This is the single document both are built against.

Nothing in this document creates a new rule. Where it states a mapping, the
rule is CUR-X.5's; where it states a signature, the implementation is
`otp::`'s. This document cites both and adds nothing of its own, on the same
discipline `docs/README.md` states for the manuals.

## 2. Where this sits

```
  CUR-X.5         States what is published, in what form, and the limits
  ───────         on its use. Properties, not a mechanism (§5.4's own note).
     │
     │  worked example
     ▼
  transparency-protocol   otp::Ledger — one publisher's hash-linked chain.
  ────────────────────    otp::Registry — the aggregator across publishers.
                           Knows nothing about CUR. kind/citation/detail
                           are free-form.
     │
     │  mapped by
     ▼
  This document   Which CUR-X.5 §5.2(a) event maps to which publish() call,
  ─────────────   which SubjectKind, and which citation string. Nothing here
                   is authoritative; CUR-X.5 and otp:: both are.
```

If this document and CUR-X.5 disagree about what must be published, CUR-X.5
governs. If this document and an `otp::` header disagree about a signature,
the header governs. This document is a map, not a third source of law.

## 3. Instance identity

`otp::Ledger::set_instance()` and `otp::Registry::set_instance()` both require
an instance string and a schema-version string, locked on first use (design
record §2, §6.2). For every ledger publishing CUR determinations under
CUR-X.5:

```cpp
ledger.set_instance("aevoria", "CUR-X.5/1.0.0");
```

`"aevoria"` is the instance identifier already used as the worked example in
`transparency-protocol/docs/LEDGER.md` §6, matching this civilisation's name
per `README.md`. `"CUR-X.5/1.0.0"` is the schema version: the Part and version
this Entry's disclosure and publishable-set rules were drawn from, so that a
later narrowing under §5.6(b) cannot be read back onto entries published under
an earlier, wider version (§5.6(e)).

A Commonwealth institution publishing under this Part uses this instance
string and no other. The Overby operations instance (`"overby-ops"`,
`"OTB-1/1.0"` in the `transparency-protocol` README) is a separate ledger for
separate subject matter — design record §2's first decision — and the two are
never aggregated in one `otp::Registry`.

## 4. The publishable set, mapped

CUR-X.5 §5.2(a) lists nine publishable event types. Each maps to an
`otp::Ledger::publish()` or `otp::Ledger::correct()` call. `subject_id` in
every row is whatever identifies the subject in the internal record — an
`EntityHandle`, a charter name, an authorisation ID — not a value this
document invents.

| §5.2(a) | Event | `otp::` call | `SubjectKind` | `Standing` |
|---|---|---|---|---|
| (1) | Determination and its measure | `publish()` | Per §5 below | `ST_DETERMINED` |
| (2) | Correction, dismissal, overturn | `correct(corrects_seq, ...)` | — (bound to original) | `ST_DISMISSED` / `ST_REMEDIED` / `ST_OVERTURNED` |
| (3) | Obligation matured unmet | `publish()` | `SUBJ_INSTITUTION` (the party that owed it) | `ST_DETERMINED` |
| (4) | Advocate appointment | `publish()` | See §6 below — open | `ST_DETERMINED` |
| (5) | Determination of death | `publish()` | `SUBJ_PERSON` | `ST_DETERMINED` |
| (6) | Capture Risk band change | `publish()` | `SUBJ_INSTITUTION` | `ST_DETERMINED` |
| (7) | Protected Mode entry/exit | `publish()` | `SUBJ_INSTITUTION` (the Commonwealth institution entering it) | `ST_DETERMINED` |
| (8) | Authorisation withdrawal, suspension, restriction | `publish()` | `SUBJ_INSTITUTION` | `ST_DETERMINED` |
| (9) | Amendment to §5.2(a) itself | `publish()` | `SUBJ_INSTITUTION` (the amending body) | `ST_DETERMINED` |

Row (2) is `correct()`, not a fresh `publish()`, in every case: CUR-X.5
§5.4(b) requires a correction to bind to the entry it corrects and never
replace it, which is exactly what `correct()` does and a second `publish()`
would not.

`citation` for every row is the CUR provision that required publication of
that specific instance — `"CUR-H.7 §7.15(e)"`, `"CUR-H.6 §6.8(a)"`,
`"CREF §15"`, and so on — never `"CUR-X.5"` itself. CUR-X.5 states what must
be published; it is not the reason any individual entry exists, and
`entries_naming()` or `find_naming()` results that read `"CUR-X.5"` on every
row would tell a holder nothing about what actually happened.

§5.3 (inaction is published) is row (3) above. It is not a distinct
mechanism — `run_builtin_test()` in `CURStateMachine`, which already detects a
matured unmet obligation on the clock (`ObligationRegister`), is the trigger;
the `publish()` call is the same call as every other row, with `kind` set to
something like `"obligation_lapsed"` and `standing` set to `ST_DETERMINED`
because a lapse is a fact the clock established, not an accusation awaiting
review.

## 5. Disclosure, mapped

CUR-X.5 §5.5 states the asymmetry; `otp::disclosure_for(SubjectKind)`
implements it exactly, and no `publish()` parameter can override it (§5.5(h);
`otp_ledger.h`'s own comment: "the absence is the protection").

| §5.5 | Subject | `SubjectKind` | `Disclosure` |
|---|---|---|---|
| (a) | Institution, enterprise, office, authorisation | `SUBJ_INSTITUTION` | `DISC_NAMED` |
| (b) | A being | `SUBJ_PERSON` | `DISC_COMMITTED` |
| (f) | Status unclear; a natural person trading in their own name | `SUBJ_PERSON` | `DISC_COMMITTED` |

§5.5(f)'s rule and the library's own default agree without needing to be
reconciled: `SUBJ_PERSON` is documented in `otp_ledger.h` as "the protective
default," and §5.5(f) resolves an unclear subject the same way. Where a caller
of `publish()` is unsure which `SubjectKind` applies, `SUBJ_PERSON` is correct
and requires no separate check against §5.5(f) — the two defaults were reached
independently and land in the same place.

§5.5(g) — the connection of a being's entries to one another is available
only to a party holding authority under CUR-H.6 §6.12 — is `commitment_key_`.
`set_commitment_key()` is called once, by whatever process CUR-H.6 §6.12
names, and held separately from the ledger and from any holder's copy. A
holder without the key can call `entries_naming()` freely and `verify()`
freely; `entries_committed_to()` and `commitment_for()` require the key and
are not exposed to a holder who does not have it.

## 6. Open questions, not resolved here

Three points where CUR-X.5 and `otp::` do not yet fully meet, none decided by
this document because none is this document's to decide:

**Advocate disclosure (§5.2(a)(4)).** CUR-A §7.7(h) and CUR-E §1.6(g) require
publishing an advocate's appointment, expertise, and any declared interest.
Whether the advocate — ordinarily a being — is `SUBJ_INSTITUTION` (named,
because the role is a public one and the declared interest is the point of
publishing at all) or `SUBJ_PERSON` (committed, on the general rule of §5.5(b))
is not settled by either text. §5.5(b)'s default and the purpose of publishing
a declared interest pull in different directions, and resolving it is a
reading of CUR-A §7.7(h) and CUR-E §1.6(g) together, not an integration
detail.

**External Trade (CUR-X.6 §6.6(a)).** CUR-X.6 requires publication of trade
above a threshold "under CUR-X.5 §5.2," but External Trade is not one of the
nine items §5.2(a) enumerates. CUR-X.6's own Implementation Notes already
records this as a gap. It is not closed here; closing it is a CUR-X.6 or
CUR-X.5 amendment, which §5.6(a) makes an ordinary one, not an integration
question.

**Schema-change entries (§5.2(a)(9), §5.6(b)).** `Entry::schema_version`
exists on every entry (§4 above), which closes the specific gap CUR-X.5's
Implementation Notes recorded for §5.6(e) — see §7 below. What is not yet in
`transparency-protocol` is a distinguished `kind` or method for publishing a
schema *change* itself, as opposed to an ordinary entry that happens to carry
a new version. Until one exists, a schema amendment publishes as a row (9)
`publish()` call with `kind = "schema_amended"`, which satisfies §5.2(a)(9)
today without a dedicated method.

## 7. Correcting CUR-X.5's own Implementation Notes

While preparing this mapping, one row of CUR-X.5's Implementation Notes was
found stale: `Entry::schema_version` (§4 above) already carries the version
CUR-X.5 §5.6(e) requires, which is what the row on §5.6(e) itself records as
missing. The row has been updated to reflect it — see that Part's
Implementation Notes and its own note on the correction.

## 8. What `transparency-protocol` does not yet do

From its own README §8, carried here so an implementer does not have to cross
repositories to find it: the hash is FNV-1a/64, adequate against accident and
casual tampering and not against a determined adversary; the commitment scheme
allows a key-holder to test a guess at an identity, which is why the key is
access-controlled rather than published; and an instance-identifier
registration scheme, streaming append to disk, a signature layer, and
anchoring helpers are all still open. None of these are CUR-X.5 gaps — §5.4
does not require any of them — and none are tracked here beyond this pointer.
`transparency-protocol/docs/DESIGN.md` §6 is the authoritative record of what
remains open there.

## 9. Properties, mapped

| §5.4 | Property | `otp::` |
|---|---|---|
| (a) | Append-only | No method removes or edits an `Entry`; `correct()` appends |
| (b) | Correction bound to original, original retained | `correct()`, `latest_standing()` |
| (d) | Gap-free sequencing | `seq` monotonic by construction |
| (e) | Independently verifiable | `verify()`, `Integrity` |
| (f) | Free to read, hold, publish into | No fee, account, or credential anywhere in the API |
| (h) | Independently holdable | `to_canonical()` / `from_canonical()` |
| (i) | No tradeable instrument | No token, stake, or gas anywhere in the design — design record §2 |

§5.8(b) (a holder may compare their copy against another and publish the
result) is `compare()`, returning a `Divergence` — the first sequence at which
two copies disagree, which is what a holder publishes.
