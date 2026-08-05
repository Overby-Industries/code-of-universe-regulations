# Dispute Resolution

*If you have a grievance in the Aevoric Commonwealth, which process handles
it, and what happens once you file — CREF and the Constitutional Court's
own mechanics, read together for the first time.*

- **Status:** Draft
- **Sources:** `foundation/cur-foundation-012.md` (CREF), `governance/pddc-governance-mechanics.md`,
  `titles/CUR-X/cur-x-part-3.md`, `titles/CUR-X/cur-x-part-1.md`

## Why this file exists

CUR-FOUNDATION-012 (CREF) states the abstract process by which a rights
violation is complained of, investigated, and remedied. `PDDC-GOVERNANCE-MECHANICS.md`
TITLE 3 states the Constitutional Court's actual operational mechanics —
docket numbers, deadlines, panel composition. CUR-X.3 states what happens
when more than one domain's rules apply to the same matter. Despite
governing overlapping ground, none of the three cites the other two. This
file is where they meet: which process a given grievance actually goes
through, start to finish, with the citation for each step. It creates no
new process and grants no new authority; where it summarises, the source
governs.

## First: what kind of matter is this

| Your situation | Process | Governing text |
|---|---|---|
| A Vital Continuity Service (food, water, shelter, healthcare, power, computational continuity, and the others CREF §4 lists) is interrupted, degraded, or withheld | **Restoration first, before anything else below** | CREF §7, §10; CUR-X.3 §3.5 |
| A right has been denied, restricted, interfered with, or removed without process | Constitutional Complaint and Review Process | CREF §5, §7 |
| An institution or individual is concentrating authority in a way that meets PDDC §13.1's capture criteria | Capture Tribunal proceeding | `PDDC-GOVERNANCE-MECHANICS.md` §7 |
| Two or more CUR domains indicate different outcomes for the same matter | Cross-Domain Conflict Resolution | CUR-X.3 |
| An instrument or action looks like it approaches a forbidden transition | Fault detection and the fault handler — see [`audit-protocol.md`](audit-protocol.md) and CUR-X.1 §1.7 | PDDC §12.3–§12.4 |
| An ordinary constitutional question, not urgent | Constitutional Court, ordinary docket | `PDDC-GOVERNANCE-MECHANICS.md` §6.3(b) |

More than one row can apply to the same complaint — a capture pattern can
also be a rights violation, and both proceed. §3.4 below states how.

## Vital Continuity always goes first

**Restoration precedes fault assignment, investigation, administrative
review, and resource accounting — not just at the Constitutional Court, at
every body this file describes.** CREF §7 states it for the complaint
process generally; CUR-X.3 §3.5 states the same rule for the specific case
where the interruption is also a cross-domain matter, and neither defers to
determine which applies first — restoration proceeds regardless of which
body eventually handles the rest. This is not a queue-jump exception argued
case by case. It is the first thing checked, every time, before the table
above is even consulted for anything else.

## The Constitutional Complaint and Review Process, in practice

CREF §8's lifecycle (Submission → Validation → Rights Review or Vital
Continuity Response → Investigation → Review → Determination → Remedy) is
the abstract shape. Here is what each step actually looks like at the
Constitutional Court:

| Step | What happens | Deadline | Source |
|---|---|---|---|
| Submission | Filed with the Governance Secretariat, acting as Court registrar | — | `PDDC-GOVERNANCE-MECHANICS.md` §6.2(a) |
| Validation | Registrar acknowledges, assigns a docket number | 48 hours (24 hours for an FSM-related petition, per PDDC §12.7(c)) | §6.2(b)–(c) |
| Investigation | Authorised investigators collect evidence, interview participants, review records, request audits — always respecting the rights of everyone involved | — | CREF §9 |
| Deliberation | Internal; not public. Decisions are published in full with reasoning | 60 days for ordinary questions; 48 hours for FSM fault/Protected Mode matters; 30 days for rights claims | `PDDC-GOVERNANCE-MECHANICS.md` §6.3 |
| Determination and Remedy | Restoration, correction, compensation, reinstatement, public disclosure, institutional reform | — | CREF §12 |

**Due process is not optional at any step.** No enforcement action proceeds
without notice, explanation, an opportunity to respond, and a route to
appeal — the one exception is an immediate protective action to preserve
life, continuity, safety, or constitutional integrity, and even that is
limited to the minimum scope necessary and cannot suspend rights, expand
authority, or bypass the rest of this list afterward. *(CREF §14)*

## Capture Tribunal proceedings

**Who can start one:** any Full Avian Member by petition, the Governance
Secretariat on a State Registry anomaly, the CC on conduct meeting PDDC
§13.1's criteria, or the Constitutional Court acting on its own initiative.
*(`PDDC-GOVERNANCE-MECHANICS.md` §7.2(a))*

**Timeline:** acknowledged within 48 hours; a prima facie determination
within 7 days; if prima facie evidence exists, the affected parties are
notified, the finding is published, the fault handler is triggered for the
affected domain, and a full evidentiary hearing convenes within 14 days; a
final decision follows within 30 days of that hearing. *(§7.2(c), §7.3)*

**Fault attribution is separate from the fault handler itself.** The fault
handler — reject, revert, Protected Mode — runs automatically and does not
wait for the Tribunal to determine who is responsible. Attribution answers
a different question afterward. *(§7.4)*

## Cross-domain disputes

Where the matter genuinely engages more than one CUR domain and their
indicated outcomes actually conflict — not merely overlap; CUR-X.3 §3.4
handles concurrent, non-conflicting application separately — CUR-X.3's
order of precedence resolves which governs: forbidden transitions first,
then Vital Continuity, then rights invariants, then consent, then the more
protective provision, then the more specific one, then the foundation
documents. *(CUR-X.3 §3.3)* **No domain is ranked above another to reach
this — see CUR-X.1 §1.6(a).**

**Standing is not limited by capacity to appear.** An animal, an entity
whose operational state limits participation, or an ecosystem is
represented rather than excluded, and representation never diminishes the
represented party's own standing. *(CUR-X.3 §3.7)*

## Appeals

Every process above is appealable. CREF §15 names Citizen Review,
Constitutional Court, and Independent Panels as reviewing bodies generally;
CUR-X.3 §3.9(e) states the specific rule for a cross-domain determination:
no determination is final against a being who has not had the opportunity
to be heard, directly or through representation.

## What no dispute-resolution process may do

**Grant emergency authority, suspend a right, or expand any body's mandate
beyond its ordinary constitutional one — including in the name of urgency,
crisis, or the dispute itself.** PDDC §12.6 is Type A Entrenched and reaches
every process on this page without exception. An interim measure pending
resolution is limited to preventing irreversible harm, reviewed at least
every 30 days, and void the moment it accomplishes the outcome sought on the
merits rather than merely preserving the status quo. *(CUR-X.3 §3.8)*
