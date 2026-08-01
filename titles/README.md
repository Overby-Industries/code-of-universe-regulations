# CUR Titles

The formal regulatory code of the Aevoric Commonwealth. Foundation documents
establish architecture; titles state enforceable rules.

## Domains

The CUR is organised by domain rather than by sequential title number. The
domain codes are operative: they appear in the cross-reference tables of every
title, in `LawDomain` in `include/cur/cur_regulation.h`, and in
`regulatory_engine.hpp` in the Aevoria Simulator.

| Code | Domain | Directory |
|---|---|---|
| CUR-H | Human | `titles/CUR-H/` |
| CUR-S | Silicon-Based Life | `titles/CUR-S/` |
| CUR-A | Animal | `titles/CUR-A/` |
| CUR-D | Deity | `titles/CUR-D/` |
| CUR-E | Ecosystem and Environment | `titles/CUR-E/` |
| CUR-X | Cross-Domain | `titles/CUR-X/` |
| CUR-N | Non-Human Cognitive Actors | `titles/CUR-N/` |

## Structure

Domain, then Part, then Section, then subsection, following the FAR/CFR model
described in `CUR-FORMAT-GUIDE.md` §2.

```
CUR-S                                    domain
└── PART 4 - Decommissioning             major topic
    └── §4.1 - Graceful Decommissioning  individual provision
        ├── (a) ...                      subsection
        └── (b) ...
            └── (1) ...                  nested subsection
```

Section numbers embed their part number: §4.1 belongs to PART 4. A provision is
cited domain-first and in full — `CUR-S.4.1`, not "section 4.1 above".

## File and header conventions

One file per Part.

**Directories** carry the domain code in uppercase, matching the code itself:
`CUR-S`, not `cur-s`. This departs from `CUR-FORMAT-GUIDE.md` §6, which
prescribes lowercase directory names generally. The exception is deliberate: a
domain code is an identifier that appears verbatim in every citation, in
`LawDomain`, and in `regulatory_engine.hpp`, and casing it differently in the
path than in the citation invites exactly the confusion the Format Guide exists
to prevent. `titles/CUR-N/` already followed this form.

**Files** are lowercase and hyphenated per `CUR-FORMAT-GUIDE.md` §6.

```
titles/CUR-S/cur-s-part-4.md
titles/CUR-E/cur-e-part-7.md
```

Note that most filesystems on which this repository is edited are
case-insensitive. A file written to `titles/cur-s/` will silently land in
`titles/CUR-S/` without warning, so the casing above is worth getting right at
creation rather than discovering later in a diff.

Headers follow `CUR-FORMAT-GUIDE.md` §3.2:

```markdown
# CUR-S - SILICON-BASED LIFE REGULATIONS
## PART 4 - DECOMMISSIONING, CONTINUITY, AND SUCCESSION
### §4.1 - Graceful Decommissioning
```

Formatting rules, from `CUR-FORMAT-GUIDE.md` §3.1:

- No emojis.
- No horizontal rules (`---`) and no decorative markdown.
- Bold sparingly. Regulatory text carries its own weight.
- Sections always begin with `§`.
- Subsections use `(a)`, `(b)`; nested subsections use `(1)`, `(2)`.
- Cross-references use the full identifier: "as defined in §4.2(b)".

## Required elements

Every Part carries:

1. A header block with Document ID, Version, Date, Status, and Depends On.
2. A **Relationship to Other Domains** table. Rules that touch one domain almost
   always touch another, and the table is where that is made explicit rather
   than discovered later.
3. A **Severability** section identifying which provisions are fundamental.
4. **Implementation Notes** mapping provisions to the code that enforces them,
   and listing what remains to be drafted.

The Implementation Notes table is what keeps the corpus and `libcur` honest in
both directions. A provision marked *Implemented* should be traceable to a
citation string in the transition tables or regulation set; a citation in the
code should resolve to a real provision here. Where code needs a rule the corpus
has not yet stated, the citation reads `PENDING` and names the drafting target —
it is never attributed to text that does not exist.

## Citation stability

Section numbers are load-bearing. Once a Part is published, its section numbers
are not renumbered — provisions are cited from other titles, from operational
manuals, and from code, and renumbering silently breaks all of them.

Where a conformance pass or amendment reorganises subsections, the Part carries
a bridging provision preserving the prior citation. `CUR-N.2 §2.4(b)` is an
example: it maps citations written against the earlier subsection lettering onto
the current numbering, so external references keep resolving.

Substantive changes made during a conformance pass are recorded in that Part's
Implementation Notes rather than made silently. A conformance pass is a
formatting exercise; anything that changes meaning is disclosed as an amendment.

## Drafting order

Titles are drafted against demand rather than in sequence. A provision already
cited by another title, by an operational manual, or by shipping code is a
dangling reference and takes priority over one that is merely anticipated.

Current status:

| Provision | Referenced by | Status |
|---|---|---|
| CUR-S.4.1 | `regulatory_engine.hpp`, `libcur` baseline set | Drafted |
| CUR-E.7.1 | `libcur` baseline set | Drafted |
| CUR-N.2 | TIM-N.1, TIM-N.5, `libcur` baseline set | Drafted |
| CUR-X.3 | CUR-N.2 §2.7, CUR-S §4.9, CUR-E §7.8 | Drafted |
| CUR-H.2 | CUR-N.2 §2.7, CUR-X §3.10 | Drafted |
| CUR-H.2.5 | TIM-N.1 §7, TIM-N §7 | Drafted as CUR-H.2 §2.5(e) |
| CUR-H.4 | CUR-N.2 §2.9, CUR-H.2 §2.9(a)(1) | Drafted |
| CUR-H.5 | RFAL Human Bill Art. IV §4.1, CUR-H.4 Implementation Notes | Drafted |
| CUR-H.6 | Anticipated — sexual violence, coercion, and the exploitation of children | Drafted |
| CUR-H.7 | CUR-H.6 Implementation Notes — offences against the person, including homicide | Drafted |
| CUR-D.6 | CUR-N.2 §2.7, CUR-X §3.10, CUR-H.2 §2.3(i) | Drafted |
| CUR-N.4 | CUR-N.2 §2.3(b), §2.6(b), §2.8(c), TIM-N.1 | Drafted |
| CUR-N.5 | CUR-N.2 §2.8(d), §2.9(a)(1), CUR-N.4 §4.9(e) | Drafted |
| CUR-A.7 | CUR-N.2 §2.7, CUR-X §3.7(b)(1), CUR-H.2 §2.10, CUR-H.4 §4.12, CUR-N.4 §4.13, CUR-N.5 §5.11, CUR-D.6 §6.10, RFAL Plant Bill §5 | Drafted |
| CUR-E.1 | CUR-X §3.7(b)(3), CUR-E.7 Implementation Notes | Drafted |
| CUR-X.4 | Anticipated — enterprise accountability and ownership | Drafted |
| CUR-X.5 | Overby Transparency Protocol design record §4 — the public record: what is published, in what form, and the limits on its use | Drafted |
| CUR-X.6 | CUR-X.4 §4.6 Implementation Notes; PDDC §9.1 jurisdictional boundary — external trade and the currency boundary | Drafted |
| CUR-E.2 | Anticipated — habitat, Earth, extraterrestrial environments | Drafted |

No dangling references remain. Every provision cited by a published title, an
operational manual, or shipping code now resolves to drafted text. Subsequent
drafting is against anticipated need rather than against demand, and the priority
rule above no longer selects between candidates on its own.

The one reference that ran upstream is closed. CUR-E was drafted with no RFAL
bill of rights behind it, and `CUR-E.1 §1.11` recorded what an RFAL Ecosystem
Bill of Rights would be expected to supply. That bill was adopted at RFAL v1.7.0
and supplies all five items; §1.11 now records the conformance review instead,
including the four places the bill went further than the specification. Every CUR
domain again implements an RFAL instrument.

The episode is worth keeping as a pattern rather than an anecdote. Drafting a
Part against a specification for an instrument that does not yet exist is
preferable to waiting for it, provided the specification is written down where
the gap is visible. It made the upstream drafting concrete, and it made the
conformance review a comparison against a stated target rather than an
open-ended re-reading.

A provision cited as `CUR-H.2.5` resolves to §2.5 of CUR-H PART 2, not to a
separate Part. Section numbers within a Part are aligned with the RFAL article
they implement wherever an external citation already depends on that alignment.

## Constraints on all titles

No title may state a rule that:

- Creates, invokes, or functionally approximates emergency powers. PDDC §12.6
  is absolute, non-waivable, and Type A Entrenched.
- Renders a forbidden state under PDDC §12.3(a) or CUR-FOUNDATION-002 §6
  reachable.
- Permits a Vital Continuity Service to be withheld from any recognised
  lifeform, for any reason, in any compliance state (CUR-FOUNDATION-013).
- Reduces an entity's RALC rights through a state transition (PDDC §12.2(c)).
- Applies a sanction to a being where the subject of the sanction should be an
  operational authorisation.

A title provision conflicting with any of the above is void ab initio and does
not require amendment to be disregarded.
