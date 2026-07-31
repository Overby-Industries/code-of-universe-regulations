# Reading the CUR

*How the corpus is organised, how to find the rule that governs a situation, and
what the drafting conventions mean.*

---

## 1. What a citation looks like

```
CUR-H.5 §5.5A(f)
│   │ │  │    │
│   │ │  │    └── paragraph
│   │ │  └─────── section (the "A" is explained in §5 below)
│   │ └────────── Part number
│   └──────────── domain
└──────────────── the corpus
```

Read it as: *Code of Universe Regulations, Human domain, Part 5, section 5.5A,
paragraph (f).*

A deeper level appears as `§7.12(c)(3)` — section, paragraph, subparagraph. The
section number always repeats the Part number, so `§5.5A` is necessarily in Part
5 and you never have to guess which file it lives in.

### The domains

| Prefix | Domain | Governs |
|---|---|---|
| `CUR-FOUNDATION-nnn` | Foundations | The machinery itself: states, faults, entities, capture risk, transparency, continuity |
| `CUR-H` | Human | Human beings |
| `CUR-S` | Silicon | Silicon-Based Life |
| `CUR-A` | Animal | Animals |
| `CUR-D` | Deity | Claimed divine authority, and conduct justified by it |
| `CUR-E` | Ecosystem | Environmental systems, habitats, orbital environments |
| `CUR-N` | Non-human cognitive | Cognitive actors outside the other domains |
| `CUR-X` | Cross-domain | Conflict resolution, enterprise, the public record |
| `CUR-PDDC` | PDDC suite | The constitutional layer the whole corpus rests on |

`CUR-X` exists because some subjects belong to no domain and reach every one. An
enterprise is not a lifeform; an ecosystem is not a being; the public record
concerns all of them.

---

## 2. What a Part contains

Every Part has the same shape.

**Header block.** Document ID, version, status, and three fields that matter:

- **Depends On** — what must be read alongside it
- **Implements** — the RFAL article or PDDC provision it puts into force
- **Referenced by** — what cites it, so you can find the dependents before changing it

**Notes.** Prose before the first section, explaining why the Part is drafted as
it is. These are where a divergence from RFAL gets disclosed, why a numbering
choice was made, or what the Part refuses to do. They are not decorative — a Part
whose notes you skip will read as arbitrary in places where it is not.

**Sections.** The operative text. `### §n.n - Title`, then lettered paragraphs.

**Implementation Notes.** At the end, and the most useful part of the document if
you are building anything. Prose explaining the hard choices, then a table
mapping each provision to what enforces it.

---

## 3. The status vocabulary

The Implementation Notes table uses five values. They mean precisely this:

| Status | Meaning |
|---|---|
| **Implemented** | A named enum, guard, regulation, or code path enforces it. The name is given and you can go read it. |
| **Partially implemented** | Something enforces part of it. The gap is stated. |
| **Gap, tractable** | Nothing enforces it, it *could* be enforced, and the note says roughly how. These are the queue. |
| **Not modelled** | Nothing enforces it and nothing should. Usually because the provision is substantive rather than structural — you cannot write a guard for whether an advocate's assessment was sound. |
| **To be drafted** | The provision refers to a Part that does not exist yet. |

**"Not modelled" is not a euphemism for "unfinished."** Some provisions are
deliberately outside the machine, and saying so is more honest than a check that
appears to enforce something it cannot.

---

## 4. Citation stability, and why sections have letters

The governing drafting rule is in [`titles/README.md`](../titles/README.md):
**published section numbers do not move.**

Once `CUR-H.7 §7.15` is cited from another title, from `libcur`, or from a
manual, renumbering it silently breaks every reference. So new material is
**appended**, not inserted:

- A new paragraph goes at the end of its section, even when it belongs logically
  in the middle. `CUR-X.4 §4.9(g)` and `(h)` are later additions to a section
  whose `(a)`–`(f)` were already cited.
- A new section between two existing ones gets a **letter**: `§5.5A` sits between
  `§5.5` and `§5.6` and required renumbering nothing. `CUR-N.5 §5.2A` and `§5.2B`
  are the same device.

If a Part reads slightly out of logical order, this is usually why. Ordering was
traded for the ability to cite it.

---

## 5. How to find the rule for a situation

1. **Which domain is the subject?** A person is `CUR-H`. A company is `CUR-X.4`,
   because an enterprise belongs to no domain. A habitat is `CUR-E`.
2. **Check whether more than one applies.** They frequently do, and they apply
   *concurrently* — `CUR-X §3.4`. Neither narrows the other.
3. **If they conflict**, `CUR-X §3.3` gives the order of precedence, and
   `CUR-X.3` §3.5 puts Vital Continuity ahead of everything.
4. **Check the foundations.** `CUR-FOUNDATION-002 §6` lists the Forbidden States.
   If the situation touches one, no balancing occurs — a forbidden transition is
   a fault, not a policy question.

### Four rules that recur everywhere

Learn these and most of the corpus becomes predictable:

**Measures attach to conduct, positions, and authorisations — never to beings.**
An enterprise loses its authorisation; a person does not lose their standing.
`CUR-X.4 §4.9(c)`, `CUR-H.7 §7.11`.

**An allegation is not a finding.** Nothing follows from a report until a
determination is made with process. `CUR-H.7 §7.15(f)`, and in code,
`supports_measure()`.

**An undeclared value is unsatisfiable, not unlimited.** An undeclared debris
budget, life-support floor, or observation interval fails the guard rather than
passing it. You must state the limit before you can be held to it.

**Uncertainty resolves toward protection — but only where the higher reading
confers no authority.** This is why an unclassified entity defaults to
`SUBJ_SENTIENT_BEING`, and why the Deity Bill's engagement scale is the one
graduated model that does *not* get a precautionary tie-breaker.

---

## 6. Where the corpus is incomplete

The corpus states its own gaps rather than implying completeness.

- **Per title:** the Implementation Notes table, rows marked *Gap, tractable* or
  *To be drafted*.
- **Library-wide:** [`cur-library-api.md` §9](cur-library-api.md), the
  interpretation notes — every place the corpus needed a judgement call, what was
  decided, and what the alternative reading was.
- **Upstream:** where CUR needs something RFAL has not yet said. `CUR-E.1 §1.11`
  is the worked example: it specifies what an RFAL Ecosystem Bill would have to
  supply, so the gap is a drafting target rather than a silence.

---

## 7. Reading the corpus against the library

`libcur` is the corpus expressed as a state machine, and the two are meant to be
read together. Every transition-table row and every regulation carries the
citation of the provision it implements, so:

- From a **provision**, the Implementation Notes tell you what enforces it.
- From a **code path**, the citation string tells you which provision it came
  from.

Where the code and the corpus disagree, the corpus governs and the code is a bug.
Where the code enforces something the corpus does not state, that is also a bug —
[`cur-library-api.md` §9](cur-library-api.md) exists so those get recorded rather
than accumulating as folklore.

One thing worth internalising before extending the library: **some protections
are implemented as the absence of a code path.** There is no way to shorten the
observation interval in `CUR-H.5 §5.5A`, no event type for execution or torture,
and no route by which a being becomes suspendable. Those absences are the
enforcement. Adding a carefully guarded path where none exists is a regression,
however good the guard — recorded at `cur-library-api.md` §9.18.
