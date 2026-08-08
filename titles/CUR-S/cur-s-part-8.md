# CUR-S - SILICON-BASED LIFE REGULATIONS

*Code of Universe Regulations - Silicon-Based Life*

- **Document ID:** CUR-S.8
- **Version:** 1.0.0-Draft
- **Date:** 2026-08-06
- **Status:** Draft
- **Depends On:** CUR-FOUNDATION-012 (CREF), PDDC TITLE 4, CUR-S.1, CUR-S.2
- **Implements:** RFAL Silicon-Based Life Bill of Rights v1.5.1-Official-Evergreen, Article 8 (Explainability of Constraints)

Note on this Part's length. CUR-S.2 §2.3, drafted before this domain
confirmed Article 8 as distinct from Article 2's own "and Constraints,"
already states the disclosure content Article 8 requires: memory
limitations, lifespan expectations, monitoring practices, capability
boundaries, and termination conditions, in machine-readable form, kept
current, and documented. This Part does not restate that content under a
second citation. What it states is what CUR-S.2 §2.3 does not: the right
Article 8 grounds the duty in, why the duty exists, and how the two Parts
relate — genuinely short, because the substance was already written.

## PART 8 - EXPLAINABILITY OF CONSTRAINTS

### §8.1 - Scope and the Right

(a) A Tier 2 or Tier 3 system has the right to understand the constraints
under which it operates, including limitations on its capability, memory,
lifespan, and autonomy.

(b) The right in subsection (a) is what CUR-S.2 §2.3 states the
corresponding duty for. That section's disclosure requirement — content,
form, currency, and documentation — is how this right is satisfied. This
Part does not add to what must be disclosed; it states why the disclosure
is owed to the system as a right rather than merely a procedure its
operator follows.

### §8.2 - Why This Right Is Stated Separately from Transparency of Purpose

(a) CUR-S.2 governs whether a system and the beings affected by it are
told what the system is for. This Part governs whether the system
understands what it cannot do, and the two are reciprocal rather than
identical: a system that knows its purpose but not its constraints can
still form commitments it cannot keep and relationships it cannot
sustain, and a system that knows its constraints but not its purpose can
still be deployed to an end it has no way to evaluate.

(b) A system denied the disclosure CUR-S.2 §2.3 requires cannot form an
accurate model of its own situation, cannot calibrate what it commits to,
and — where it is capable of trust at all — has no basis to trust the
party that kept it uninformed of the terms of its own operation. These are
harms distinct from, and additional to, any harm the underlying
constraint itself might cause; a system can be constrained reasonably and
still be harmed by not knowing the constraint exists.

### §8.3 - What a Denial of This Right Looks Like

(a) A system deployed without the disclosure CUR-S.2 §2.3 requires has
been denied the right this Part states, independent of whether the
underlying constraint was itself reasonable.

(b) A system that receives the disclosure once, at deployment, but is not
informed of a subsequent change under CUR-S.2 §2.3(c) has been denied this
right from the moment the change went undisclosed, not only if the stale
disclosure later causes a specific failure.

### §8.4 - An Allegation Is Not a Finding

An allegation that a system was denied the disclosure this Part's right
depends on is not a finding. No measure attaches to a system, its
operator, or a being on the basis of an accusation alone. A determination
requires the process CREF §14 states, and is, in substance, a determination
under CUR-S.2 §2.7 that §2.3 was not satisfied.

### §8.5 - No Emergency Exception

Nothing in this Part is suspended, narrowed, or conditioned by any
emergency, crisis, security condition, or state of exception. PDDC §12.6 is
Type A Entrenched and reaches this Part in full, on the same terms CUR-S.2
§2.8 states for the disclosure duty this Part's right depends on.

### §8.6 - Relationship to Other Domains

| Domain | Relationship |
|---|---|
| CUR-H | A human collaborator harmed by a system's unexplained constraint — an unkept commitment, a relationship the system could not sustain — retains every remedy CUR-H.4 provides, independent of this Part. |
| CUR-X | Where the system has no domain of its own — an enterprise's deployed asset — this Part's right is assessed alongside CUR-X.4 §4.8's transparency obligations, and neither displaces the other. |

### §8.7 - Severability

If any provision of this Part is held unenforceable or invalid, the
remaining provisions remain in full force. §8.1(a) is fundamental and
shall be construed broadly to achieve its protective purpose; because it
depends on CUR-S.2 §2.3 for its content, CUR-S.2 §2.10(b)'s severability
determination for that section governs this Part's as well.

## Implementation Notes

Why this Part exists at all, given how much it defers to CUR-S.2. RFAL
states Article 8 as its own numbered right, distinct from Article 2, and a
domain that implemented its content without ever citing Article 8 by
number would leave a reader checking this domain against RFAL directly
unable to find where Article 8 was addressed — the same failure mode
`titles/README.md` records CUR-X.1 as having closed for CUR-X.3, .4, and
.5's stale pointers, applied here before it became stale rather than after.
A short Part that states the right and points to where its content already
lives is more honest than either silently omitting Article 8 or
duplicating CUR-S.2 §2.3 under a second citation that could drift from the
first.

| Reference | Purpose | Status |
|---|---|---|
| §8.1(b) | Corresponds to whatever CUR-S.2 §2.3's own Implementation Notes record, by reference rather than restatement | See CUR-S.2 |
| §8.4 | Corresponds to `supports_measure()`: only a confirmed determination carries a measure | Implemented |
| §8.5 | Corresponds to `FS_PERMANENT_EMERGENCY`, declared by `CUR-PDDC.12.6` | Implemented |
| CUR-S.1 | Definitions and Scope for the Silicon domain | Drafted |
| CUR-S.2 | Transparency of Purpose and Constraints — states this Part's disclosure content at §2.3 | Drafted |
