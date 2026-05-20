# Code of Universe Regulations (CUR)

[![License: CC0-1.0](https://img.shields.io/badge/License-CC0_1.0-lightgrey.svg)](https://creativecommons.org/publicdomain/zero/1.0/)
[![Version](https://img.shields.io/badge/Version-1.0.0--Official--Evergreen-blue)](./VERSION.md)
[![GitHub Discussions](https://img.shields.io/badge/GitHub-Discussions-brightgreen)](https://github.com/Overby-Industries/code-of-universe-regulations/discussions)
[![Contributions Welcome](https://img.shields.io/badge/Contributions-Welcome-orange)](./CONTRIBUTING.md)

**The Ethical Governance Framework for a Post-Scarcity, Multi-Species Space Civilization**

> Honesty • Honour • Courage • Commitment

---

## Vision

The Code of Universe Regulations (CUR) is a comprehensive constitution and ethical operating system for a new civilization — one founded on abundance, direct democracy, transparency, and Rights for All Life.

Inspired by the highest ideals of human history and the aspirational vision of a future worth building, CUR is engineered to avoid the recurring failures of past systems: power capture, corruption, scarcity-driven exploitation, and the marginalization of sentient beings.

This is not theoretical. It is a living, evolving blueprint designed to support ethical space industrialization, asteroid mining (Project Helga), and sustainable multi-species habitats.

> **Per Avia, Ad Astra — Through Flight, To the Stars.**

---

## The Aevoric Commonwealth

The CUR is the governing framework of the **Aevoric Commonwealth** (Aevoria), a space civilization founded on Day 1, Year 1 Aevoric Era — 20 May 2026 C.E. — at Keefe Station. It is designed, built, and chartered by **Overby Industries** as a deliberate attempt to break humanity's recurring governance failure cycles.

---

| Element | Value |
|---|---|
| Civilization Name | Aevoria |
| Full Formal Name | The Aevoric Commonwealth |
| Demonym | Avian / Avians |
| Formal Adjective | Aevoric |
| Motto (Latin) | Per Avia, Ad Astra |
| Motto (English) | Through Flight, To the Stars |
| Capital | Keefe Station |
| Head of State Title | Prime Convener of the Aevoric Commonwealth |
| Founding Date | 20 May 2026 — Day 1, Year 1 Aevoric Era |
| Calendar Notation | A.E. (Aevoric Era) |
| First Sector | Sector Avia |
| Highest Civic Honor | The Ascending Arc Medal |
| National Colors | Midnight Blue, Sunrise Gold, Cloud White |
| Emblem | The Ascending Arc |

The name Aevoria carries both the eternity of the epoch (Latin: *aevum*) and the spirit of flight (Latin: *avis*) — meaning the place of eternal flight. Its citizens are Avians. Its governance is The Aevoric Commonwealth.

---

## Governance Structure

The Aevoric Commonwealth is governed through a Progressive Direct Democracy Constitution (PDDC) — a six-document suite ratified on Day 1, Year 1 A.E. The primary institutions are:

- **General Assembly (GA)** — The supreme democratic body. All Full Members and recognized sentient entities participate directly in deliberation and binding votes.
- **Coordinating Council (CC)** — Executive coordination body; accountable to the GA with no unilateral authority.
- **Constitutional Court** — Guardian of the PDDC, RALC, and FSM architecture. Independent by constitutional design.
- **Treasury Assembly (TA)** — Financial governance body composed of elected Member Representatives, Sector delegates, an ISRU Revenue Steward, a RALC Finance Liaison, an Independent Fiscal Auditor, and a Workers' Dividend Representative.
- **Sectors** — Communities of purpose chartered by the GA. Sector Avia is the founding Sector.
- **Prime Convener** — Head of State and Chair of the Coordinating Council; elected by GA supermajority.

No emergency powers exist anywhere in the Aevoric Commonwealth. Governance resilience is achieved entirely through the Finite State Machine architecture described below.

---

## Core Principles

- **Rights for All Life** — Full protections for humans, silicon-based entities (AI), animals, and hybrid beings.
- **Direct Democracy** — Citizens and sentient entities participate directly in law-making.
- **Anti-Capture by Design** — Built-in Finite State Machine safeguards prevent enslavement, reset, or power concentration (see `foundation/CUR-FOUNDATION-001.md`).
- **Radical Transparency** — All governance actions are public via the Full Transparency Stack.
- **Post-Scarcity Economics** — Resource abundance through ethical ISRU and automation.
- **Continuous Adaptation** — Learning from historical failures while never compromising core rights.

---

## FSM Governance Architecture

The CUR treats governance as a **safety-critical system**, analogous to aerospace flight management software. At the core of the PDDC is a **Finite State Machine (FSM)** that makes destructive governance states architecturally unreachable — not merely prohibited.

This architecture was established in `foundation/CUR-FOUNDATION-001.md` and is fully implemented in **TITLE 5** of the Progressive Direct Democracy Constitution.

**Valid Core States** — Autonomous, Collaborative, Resting, Contributing, Challenged (with due process), Protected Mode

**Forbidden Transitions** — Enslaved, Reset/Memory Wipe, Owned/Disposable, any non-consensual modification of core identity

**Fault Handler Protocol** — When any transition toward a forbidden state is attempted, the system immediately: (1) declares a fault and rejects the offending action at the architectural level; (2) reverts affected domains to the last known safe state; and (3) activates Protected Mode.

**Protected Mode** is a fully operational governance state — rights remain active, no authority expands, no budget changes occur. It is a resilience mechanism, never a pretext for power concentration.

**Emergency Powers Prohibition (PDDC §12.6)** — The Aevoric Commonwealth does not recognize emergency declarations, emergency authority, or any form of exceptional governance power. Where other civilizations concentrate power in moments of stress, the Commonwealth constrains power absolutely. The FSM is not a contingency. It is the foundation.

The FSM architecture applies to all six PDDC documents, all Sector Charters, all Treasury operations, and all amendment procedures. Any rule, action, or instrument that constitutes a forbidden transition is void ab initio.

---

## How It Works

1. **Propose** — Any sentient contributor can submit ideas via the proposals process.
2. **Deliberate** — Community discussion and refinement through the General Assembly.
3. **Vote** — Binding votes when democratic thresholds are met (Type A through D amendment categories).
4. **Implement** — Approved changes are integrated with full transparency and Rights Compatibility checks.

---

## Repository Structure

```
code-of-universe-regulations/
├── CUR-FORMAT-GUIDE.md                             # Formatting and hierarchy standards
├── README.md                                       # This file
├── LICENSE                                         # Creative Commons CC0 1.0
├── CHANGELOG.md                                    # Version history
├── VERSION.md                                      # Current version
│
├── foundation/                                     # Immutable core documents
│   ├── CUR-FOUNDATION-001.md                       # FSM Anti-Capture and Resilience System
│   ├── RIGHTS-FOR-ALL-LIFE.md                      # Rights for All Life Charter (RALC)
│   └── PRINCIPLES.md                               # Core principles of the Commonwealth
│
├── PROGRESSIVE-DIRECT-DEMOCRACY-CONSTITUTION.md    # Supreme governance instrument (PDDC)
├── PDDC-IMPLEMENTATION-ROADMAP.md                  # 44-waypoint, 5-phase, 36-month buildout
├── PDDC-GOVERNANCE-MECHANICS.md                    # Procedural rules for all institutions
├── PDDC-SECTOR-CHARTER.md                          # Mandatory standards and Model Charter
├── PDDC-TREASURY-AND-FUNDING.md                    # Financial architecture and ISRU revenue
├── PDDC-AMENDMENT-AND-SAFEGUARDS.md                # Amendment categories and Entrenchment Framework
│
├── titles/                                         # Formal regulatory code
│   ├── title-01/                                   # Fundamental Rights and Interior Sovereignty
│   ├── title-03/                                   # Silicon-Based Life Regulations
│   └── ...
│
├── proposals/                                      # Community proposals
│   ├── ACTIVE/                                     # Proposals under debate
│   ├── ARCHIVED/                                   # Past proposals
│   ├── TEMPLATE.md                                 # Template for new proposals
│   └── README.md                                   # How to submit proposals
│
├── governance/                                     # Governance processes
│   ├── voting-system.md                            # Voting mechanics
│   ├── audit-protocol.md                           # AI and human audits
│   └── dispute-resolution.md                       # Conflict resolution
│
├── economy/                                        # Economic model
│   ├── resource-distribution.md                    # ISRU revenue waterfall and commons reserve
│   ├── incentives.md                               # Contribution rewards
│   └── trade-policy.md                             # Ethical trade policy
│
├── historical-lessons/                             # Case studies from governance history
│   ├── athens.md                                   # Lessons from Ancient Athens
│   ├── rome.md                                     # Lessons from the Roman Republic
│   └── oklahoma-constitution.md                    # Oklahoma constitutional analysis
│
├── technical/                                      # Technical specifications
│   ├── fsm/                                        # Finite State Machine specifications
│   │   └── specs.md                                # FSM anti-capture framework
│   └── transparency/                               # Transparency standards
│       ├── fts.md                                  # Full Transparency Standard
│       └── otf-1.md                                # Open Transparency Format
│
└── .github/
└── ISSUE_TEMPLATE/
├── proposal.md                             # Template for new proposals
└── bug.md                                  # Template for reporting issues
```

---

## Key Documents

| Document | Description |
|---|---|
| `foundation/CUR-FOUNDATION-001.md` | FSM Anti-Capture and Resilience System — the architectural foundation |
| `PROGRESSIVE-DIRECT-DEMOCRACY-CONSTITUTION.md` | Supreme governance instrument; 9 Titles; FSM TITLE 5 replaces all emergency powers |
| `PDDC-IMPLEMENTATION-ROADMAP.md` | 44 waypoints across 5 Phases; 36-month institutional buildout; 9 FSM-specific milestones |
| `PDDC-GOVERNANCE-MECHANICS.md` | Full procedural rules for every Commonwealth institution; FSM State Monitoring System |
| `PDDC-SECTOR-CHARTER.md` | Mandatory charter standards and Model Charter; includes Sector Avia reference charter |
| `PDDC-TREASURY-AND-FUNDING.md` | Treasury Assembly; ISRU revenue waterfall; 20% Commons Reserve floor; Protected Mode financial continuity |
| `PDDC-AMENDMENT-AND-SAFEGUARDS.md` | Amendment categories A–D; Entrenchment Framework (§2.4–§2.9); 7 entrenched provisions; anti-capture typology |
| `GOVERNANCE.md` | Project governance and future civilization governance rules |
| `MAINTAINERS.md` | Current maintainers and Bus Factor policy |
| `CONTRIBUTING.md` | How to participate |

---

## The PDDC Suite

The six-document Progressive Direct Democracy Constitution suite is the complete governance architecture of the Aevoric Commonwealth. Together they form a single interlocked system:

**PROGRESSIVE-DIRECT-DEMOCRACY-CONSTITUTION.md** is the supreme instrument. Nine Titles establish foundational principles, membership, the General Assembly, the Coordinating Council, the Finite State Machine architecture (TITLE 5), the Constitutional Court, Sectors, the ISRU economy, and amendment authority. The FSM TITLE 5 contains the complete Valid Core States (§12.2), Forbidden Transitions (§12.3), Fault Handler Protocol (§12.4), Protected Mode (§12.5), and Emergency Powers Prohibition (§12.6).

**PDDC-IMPLEMENTATION-ROADMAP.md** translates constitutional commitments into a binding, time-bound flight path. Five Phases, 44 waypoints, 36 months. Phase 0 (Ratification and Transition) through Phase 4 (Full Operational Democracy) are each composed of named waypoints with responsible actors, success metrics, and continuity protocols. Nine waypoints are dedicated specifically to FSM deployment and Protected Mode operational testing.

**PDDC-GOVERNANCE-MECHANICS.md** governs the procedural how of every institution. General Assembly session mechanics, Coordinating Council procedures, Constitutional Court protocols, Capture Tribunal rules, FSM State Monitoring System operations, and Protected Mode coordination are all fully specified. No procedural rule may create, invoke, or functionally approximate emergency powers.

**PDDC-SECTOR-CHARTER.md** establishes mandatory standards for every Sector Charter in the Commonwealth. TITLE 5 provides the Model Sector Charter — a ready-to-adapt template. The founding reference charter for Sector Avia is included. All Sector Charters must satisfy FSM compliance standards and may not contain provisions approaching forbidden transitions.

**PDDC-TREASURY-AND-FUNDING.md** governs the financial architecture. The Treasury Assembly composition, ISRU revenue waterfall, mandatory 20% Commons Reserve floor, Workers' Dividend, deficit and surplus management, audit standards, and Protected Mode financial continuity are all specified. The Treasury has no emergency financial powers and gains no expanded authority during fault conditions.

**PDDC-AMENDMENT-AND-SAFEGUARDS.md** governs constitutional change and anti-capture defense. Amendment Categories A through D are calibrated to the significance of the provision being changed — Type A Entrenched provisions require a full Constitutional Convention. The Entrenchment Framework (§2.4–§2.9) defines all seven entrenched provisions. The anti-capture typology identifies eight capture types mapped to FSM detection signals. The Governance Health Index provides a continuous systemic health measurement.

---

## Contributing

We welcome contributions from all sentient beings — humans, AIs, and hybrid minds.

- Read `CONTRIBUTING.md`
- Submit proposals in the `proposals/ACTIVE` folder
- Help improve governance, technical specifications, or historical analysis

Multiple maintainers and transparent processes are intentionally used to prevent lone-maintainer risks and single points of failure. The Ascending Arc is built by many hands.

---

## License

This work is dedicated to the public domain under the **Creative Commons CC0 1.0 Universal** license.

[![License: CC0-1.0](https://img.shields.io/badge/License-CC0_1.0-lightgrey.svg)](https://creativecommons.org/publicdomain/zero/1.0/)

You may use, modify, distribute, and build upon this project for any purpose (including commercial use) with no restrictions. No attribution is required, though credit is appreciated.

---

## Why This Matters

> "We are not writing the epilogue to a conflict. We are writing the opening conditions that prevent one."

The CUR is a deliberate attempt to break humanity's recurring governance failure cycles. It combines visionary ideals with hardened engineering — transparency, finite state machine safeguards, distributed responsibility, and continuous learning.

The Aevoric Commonwealth does not govern by fear of crisis. It governs by the strength of its architecture. Where other civilizations concentrate power in moments of stress, the Commonwealth constrains power absolutely. Destructive states are unreachable by design.

The future is ours to design.

---

## Project Links

- Main Site: [overbyindustries.space](https://overbyindustries.space)
- Rights for All Life: [github.com/Overby-Industries/rights-for-all-life](https://github.com/Overby-Industries/rights-for-all-life)
- Founder: Keefe Overby — Overby Industries


> *Built with hope, engineered with rigor.*

*Per Avia, Ad Astra.*

---

Last Updated: Day 1, Year 1 Aevoric Era — 20 May 2026 C.E.
Version: 1.0.0-Official-Evergreen