# CUR-E - ECOSYSTEM AND ENVIRONMENT REGULATIONS

*Code of Universe Regulations - Ecosystem and Environment*

- **Document ID:** CUR-E.7
- **Version:** 1.0.0-Draft
- **Date:** 2026-07-26
- **Status:** Draft
- **Depends On:** CUR-FOUNDATION-011 (CEIF), CUR-FOUNDATION-013, PDDC TITLE 5, PDDC-TREASURY-AND-FUNDING
- **Supersedes:** the `PENDING` citation carried by `libcur` regulation `CUR-E.DEBRIS`

## PART 7 - ORBITAL ENVIRONMENT AND DEBRIS

### §7.1 - Debris Budget

(a) Every operational authorisation permitting extraction, processing, construction, propulsion, or decommissioning within Commonwealth jurisdiction shall declare a debris budget before operations commence.

(b) A debris budget states the maximum quantity of untracked or uncontrolled material, expressed in the units prescribed by §7.4, that operations under the authorisation may generate in a stated period.

(c) An authorisation that has not declared a debris budget has no debris allowance. An undeclared budget is not an unlimited budget, and shall not be construed as one.

(d) Subsection (c) is operative in implementation. A declared limit of zero and an undeclared limit are distinct conditions: the former permits operations generating no debris, the latter permits no operations at all until a budget is declared.

(e) Operations exceeding the declared budget place the authorisation in violation. The violation attaches to the authorisation and not to any being holding or operating under it, consistent with PDDC §12.2(c).

### §7.2 - Zero-Waste Obligation

(a) Material extracted, processed, or displaced within Commonwealth jurisdiction shall be recovered, reused, or returned to controlled custody. Material that is neither recovered nor under control is waste.

(b) Waste is not a permitted output of any operation. It is a measured deficiency against the authorisation's declared budget, and is reported as such.

(c) An operation that generates recoverable material and elects not to recover it has generated waste, regardless of whether recovery was economically attractive at the time. Economic inconvenience is not a ground for exceeding a debris budget.

(d) Subsection (c) does not require recovery that is physically impossible or that would itself create greater environmental burden. An operator relying on this subsection shall document the basis and publish it under §7.6.

(e) The Commonwealth's objective is the prevention of environmental burden rather than its remediation after the fact, consistent with CUR-FOUNDATION-013's guiding principle applied to the orbital environment.

### §7.3 - Collision and Continuity Risk

(a) Orbital debris presents a compounding risk. Material that is untracked today constrains every trajectory thereafter, and the constraint does not decay on any governance timescale.

(b) Debris generation that threatens habitat life-support, power generation, communication networks, or transit corridors engages CUR-FOUNDATION-013. Such generation is a Vital Continuity Service risk, not merely an environmental one, and is assessed under the Vital Continuity Index.

(c) Where debris generation causes interruption, degradation, or depletion of a Vital Continuity Service, the affected domain enters STATE-010 Vital Continuity Response under CUR-FOUNDATION-002 §3. Restoration of the service precedes fault assignment against the operator.

(d) Nothing in subsection (c) reduces the operator's eventual accountability. Restoration comes first in sequence, not instead of accountability.

### §7.4 - Measurement and Reporting

(a) Debris shall be measured in tracked object count and in aggregate mass, reported separately. Neither measure alone is sufficient: a single large uncontrolled mass and a cloud of small fragments present different risks and neither should be able to conceal the other.

(b) Measurement shall be reported at intervals no greater than those prescribed by the authorisation, and in every case upon:

   (1) Completion of an extraction or processing campaign;

   (2) Any unplanned disassembly, collision, or containment loss; and

   (3) Decommissioning of any structure or vessel.

(c) Reported measurements are constitutional events under CUR-FOUNDATION-005 and are subject to audit under CUR-FOUNDATION-010.

(d) Failure to report is a Class I fault. Misreporting is a Class II fault. Misreporting that conceals a Vital Continuity Service risk under §7.3(b) is a Class III fault.

### §7.5 - Commons Reserve Interaction

(a) Recovered material returned to controlled custody counts toward the operator's commons contribution under PDDC-TREASURY-AND-FUNDING.

(b) The twenty percent (20%) Commons Reserve floor is not satisfied by material that has been transferred while uncontrolled. Material must be under custody to count.

(c) An operator shall not discharge a Commons Reserve obligation by transferring debris liability to the commons. Transfers structured to that effect are void and constitute an attempted forbidden transition under PDDC §12.3(c).

### §7.6 - Transparency

(a) Debris budgets, reported measurements, recovery documentation under §7.2(d), and violation records are published at transparency level T0 (Public) under CUR-FOUNDATION-010 §8.

(b) No debris measurement may be classified above T0 on commercial grounds. Commercial sensitivity is not a constitutional justification for secrecy under CUR-FOUNDATION-010 §2.

(c) Any Avian or Recognised Life-Form may petition for audit of a declared budget or reported measurement under CUR-FOUNDATION-010 §10.

### §7.7 - Enforcement and Proportionality

(a) Exceeding a declared debris budget places the authorisation in violation and generates a Violation record under CUR-FOUNDATION-004 ENTITY-009 with severity assessed under CUR-FOUNDATION-002 §5.

(b) Sanctions available against an authorisation include restitution, recovery obligations, contract restriction, suspension, and in the case of repeated violation following completed due process and exhausted appeals, withdrawal of the authorisation.

(c) No sanction under this Part may withhold a Vital Continuity Service from any being, including beings employed by, resident within, or dependent upon the sanctioned operation. Sanctions attach to the authorisation, never to the necessities of those affected by it.

(d) Subsection (c) applies with equal force where withdrawal of an authorisation would leave dependent persons without a Vital Continuity Service. In that case the Commonwealth assumes continuity of the service before the withdrawal takes effect.

### §7.8 - Relationship to Other Domains

| Domain | Relationship |
|---|---|
| CUR-H | Human habitat life-support risk from debris is governed by §7.3(b) and CUR-FOUNDATION-013. |
| CUR-S | Debris threatening power or computational continuity for Silicon-Based Life engages CUR-S §4.5. |
| CUR-A | Debris affecting animal habitat or life-support systems engages the same continuity obligations. |
| CUR-D | No claimed authority exempts an operation from a debris budget. |
| CUR-N | Not ordinarily engaged. |
| CUR-X | Multi-domain debris disputes are resolved under CUR-X §3.1. |

### §7.9 - Severability

If any provision of this Part is held unenforceable or invalid, the remaining provisions remain in full force. The obligation in §7.7(c) is fundamental and shall be construed broadly.

## Implementation Notes

| Reference | Purpose | Status |
|---|---|---|
| §7.1 | Cited by `libcur` regulation `CUR-E.7.1`, replacing the prior `CUR-E.DEBRIS` `PENDING` citation | Implemented |
| §7.1(d) | Corresponds to `resolve_guard_mask` treating `debris_limit == 0` as unsatisfiable rather than permissive | Implemented |
| §7.1(e) | Corresponds to `guard::LICENSE_SUBJECT_ONLY` on Axis C transitions | Implemented |
| §7.3(c) | Corresponds to `GS_VITAL_CONTINUITY_RESPONSE` | Implemented |
| §7.5(b) | Corresponds to `guard::COMMONS_RESERVE_FLOOR` at 2000 basis points | Implemented |
| §7.7(c) | Corresponds to `FS_VITAL_CONTINUITY_DENIAL` | Implemented |
| §7.4 | Separate tracked-count and aggregate-mass measures | Not yet modelled; `TransitionContext` carries a single `debris_units` field |
| CUR-E.1 | Definitions and Scope for the Ecosystem domain | To be drafted |
| CUR-E.2 | Extraction and ISRU environmental standards | To be drafted |
