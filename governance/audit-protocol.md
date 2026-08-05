# Audit Protocol

*Every audit mechanism in the Aevoric Commonwealth — what triggers one, who
conducts it, and where it's published — gathered by function.*

- **Status:** Draft
- **Sources:** `foundation/cur-foundation-010.md` (CTAF), `governance/pddc-governance-mechanics.md`,
  `pddc-treasury-and-funding.md`, `foundation/cur-foundation-012.md` (CREF)

## Why this file exists

Two layers already state how auditing works, and neither cites the other.
`CUR-FOUNDATION-010` (the Constitutional Transparency & Audit Framework, or
CTAF) states the general framework — audit types, transparency levels, what
triggers a review — that applies to every Commonwealth institution as a CUR
foundation document. `PDDC-GOVERNANCE-MECHANICS.md` and
`PDDC-TREASURY-AND-FUNDING.md` state the specific, scheduled instances of it:
the Independent Fiscal Auditor's Annual Audit, the Governance Secretariat's
quarterly State Registry reconciliation, the Annual FSM Monitoring Report.
This file is where the general framework and its specific instances meet.
It states no new audit and creates no new authority; where it summarises,
the source document governs.

## The general framework (CTAF)

**Every exercise of authority is auditable by default. Secrecy requires
constitutional justification, not the other way round.** *(CUR-FOUNDATION-010
§2)*

**Four audit types:**

| Type | Trigger | Purpose |
|---|---|---|
| Routine | Scheduled | Prevent drift, verify compliance |
| Random | Unannounced, selected by constitutional procedure | Deter misconduct, detect hidden issues |
| Triggered | CRI threshold exceeded, citizen complaint, FSM alert, rights violation report, resource irregularity | Highest-level review, used when constitutional integrity is at risk |
| Protected Mode | Automatic, on fault-handler activation | Mandatory, not extraordinary — grants the auditor no authority beyond an ordinary audit, and is deliberately not called an "emergency audit" |

*(CUR-FOUNDATION-010 §5)*

**No entity may audit itself.** Auditors are independent from what they
audit, structurally, not as a matter of practice. *(§15)*

**Four transparency levels** gate what a finding is published at: T0 Public
(proposals, voting outcomes, audit summaries, budgets), T1 Internal
(authorised institutions only — investigation notes, preliminary findings),
T2 Protected (constitutional authorisation required — personal information,
security-sensitive records), T3 Restricted (Constitutional Court review only
— active investigations, protected witness information). *(§8)* CUR-X.4
§4.8(b) sets T0 as the floor for a Continuity Enterprise specifically; §4.8(c)
forecloses "commercial sensitivity" as a reason to sit at a lower level.

## The scheduled instances

| Audit | Conducted by | Frequency | Published | Source |
|---|---|---|---|---|
| State Registry reconciliation | Governance Secretariat | Quarterly | FTS/OTF-1, to CoC and IFA | `PDDC-GOVERNANCE-MECHANICS.md` §12.1(d)–(e) |
| Annual FSM Monitoring Report | Governance Secretariat, with CoC and Capture Tribunal | Annually | FTS/OTF-1, presented to first GA Ordinary Session of the year | §12.6 |
| Annual Audit (all Commonwealth finances) | Independent Fiscal Auditor | Annually, within 90 days of fiscal year close | FTS/OTF-1 | `PDDC-TREASURY-AND-FUNDING.md` §10.2 |
| FSM Financial Monitoring (Protected Mode transactions specifically) | Independent Fiscal Auditor, as a standalone section of the Annual Audit | Annually | FTS/OTF-1, prominently | §10.3 |
| Quarterly Financial Reports | Treasury Assembly | Quarterly, within 30 days of quarter close | FTS/OTF-1, to GA | §10.4 |
| ISRU licensee compliance reports | License holders file; Independent Fiscal Auditor reviews | Quarterly filing, annual review | FTS/OTF-1 | §8.3(d)–(e), reproduced at `PDDC-GOVERNANCE-MECHANICS.md` |
| Sector Annual Financial Report | Each Sector, reviewed by TA | Annually, by Month 2 | FTS/OTF-1, within 14 days | `PDDC-TREASURY-AND-FUNDING.md` §12.1 |
| ISRU license concentration limit | Independent Fiscal Auditor | Annually | To TA and GA | §11.2(b) |

## Independence, named twice on purpose

The Independent Fiscal Auditor's independence is stated as a structural
safeguard in `PDDC-TREASURY-AND-FUNDING.md` §10.1(d): any attempt to direct,
constrain, or penalise the IFA is itself a capture attempt under PDDC §13.1.
This is the same rule CTAF §15 states generally ("no entity may audit
itself"), applied to the one auditor whose independence the corpus names
individually rather than leaving to the general rule alone — because
financial capture of the auditor is one of the specific capture patterns
`PDDC-TREASURY-AND-FUNDING.md` §11.1(b)(3) names.

## Where an audit's findings go if something is wrong

An audit finding of a constitutional violation, capture risk, or
"Protected Mode recommended" *(CUR-FOUNDATION-010 §12)* does not resolve
itself — see [`dispute-resolution.md`](dispute-resolution.md) for the
process a finding routes into from there, including Capture Tribunal
proceedings and the fault handler.

## What an audit is not

**An audit does not impose a measure.** Findings and recommendations
(§13 above) are inputs to a determination made with due process elsewhere,
not a sanction in themselves — CREF §14's due process requirements
(notice, explanation, opportunity to respond, appeal) apply to whatever
follows an audit finding, the audit itself is not that process.
