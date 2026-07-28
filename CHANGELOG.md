# Changelog

All notable changes to the Code of Universe Regulations (CUR) will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [1.1.0-Official-Evergreen] — 2026-07-27

Built against Rights for All Life v1.6.0. See `VERSION.md` for the
correspondence table between the two lineages.

### Added
- `libcur` — the CUR regulatory FSM in C++17, no external dependencies. Three
  orthogonal state axes (constitutional, governance, compliance), flat
  `constexpr` transition tables, deterministic evaluation, and a mandatory audit
  record for every transition. 621 checks, 0 failures.
- `titles/` — the enforceable regulatory code, organised by domain. Conventions
  and drafting order in `titles/README.md`.
  - **CUR-S.4** Decommissioning, Continuity, and Succession
  - **CUR-E.7** Debris budgets
  - **CUR-N.2** Prohibition on Non-Consensual Telepathic Invasion (conformance
    pass from the pre-convention form)
  - **CUR-X.3** Cross-Domain Conflict Resolution
  - **CUR-H.2** Freedom of Thought, Conscience, and Inquiry — section numbers
    aligned with RFAL Human Bill Article II so that `CUR-H.2.5`, cited by TIM-N.1
    and TIM-N, resolves to §2.5(e)
  - **CUR-H.4** Psychological Harm and Integrity
  - **CUR-D.6** Prohibition on Coercive Worship
- `scripts/bump-version.ps1` — release tooling. Rewrites `-Official-Evergreen`
  version headers and reports every document it did not touch. Leaves `-Draft`
  title versions alone, since those track a Part's own drafting state rather
  than the corpus release.

### Changed
- All corpus documents brought to 1.1.0. Foundation documents 007 through 011
  encoded a version inside their Status field ("Draft v0.1") and carried no
  Version line; 012 carried a version with no status suffix; 013 had no header
  block at all. All are now on the 001-006 convention.
- `VERSION.md` rewritten: distinguishes corpus version from per-title document
  version, documents the release process, and records the CUR/RFAL
  correspondence table.
- Foundation documents 005 and 007 through 011 reformatted. Five had lost all
  markdown structure in a copy-paste — no headers, no code fences, no lists. 005
  additionally had two merged code fences, one of which had swallowed the
  heading of section 7 so that an entire section heading was invisible when
  rendered. Verified by word-multiset comparison against the originals: zero
  words changed.

### Fixed
- Emergency-powers language removed wherever it granted authority over another
  being, per PDDC §12.6, which is absolute and Type A Entrenched. `CUR-N.2 §2.5`
  was retitled from "Emergency Exception" to "Protective Action Exception" and
  re-expressed as a CREF §10 protective action, preserving its four conditions
  unchanged and adding an explicit burden of proof.
- `titles/CUR-D/cur-d-part-6.md` conformed to RFAL v1.5.3, which separated the
  deity engagement scale (E0-E3) from the Tier Assessment Protocol's moral-status
  scale after the two were found to share a numbering convention and nothing
  else.

## [1.0.0-Draft] — 2026-05

### Added
- Initial repository structure based on Grok's proposed architecture
- `foundation/` directory with CUR-FOUNDATION-001 (FSM Anti-Capture)
- `GOVERNANCE.md` — decision-making, maintainer rules, voting procedures
- `MAINTAINERS.md` — current maintainers and roles
- `GLOSSARY.md` — defined terms for the CUR system
- `proposals/` directory with ACTIVE/ARCHIVED structure and TEMPLATE.md
- `historical-lessons/` directory with oklahoma-constitution.md
- `economy/` directory with resource-distribution, incentives, and trade-policy stubs
- `technical/` directory with fsm/ and transparency/ stubs
- Title structures established for all seven regulatory domains
- `foundation/RIGHTS-FOR-ALL-LIFE.md` — core principles document
- `foundation/PRINCIPLES.md` — foundational principles reference

### Changed
- N/A (initial repository setup)

### Deprecated
- N/A

### Removed
- N/A

### Fixed
- N/A

### Security
- Established `MAINTAINERS.md` to prevent bus-factor vulnerabilities
- Established `GOVERNANCE.md` with anti-capture protections

---

## Version History Overview

| Version | Date | Summary |
|---|---|---|
| 1.0.0-Draft | 2026-05 | Initial CUR repository establishment |

---

## How to Update This File

1. When making a change, add an entry under the appropriate heading in the `[Unreleased]` section
2. When releasing, replace `[Unreleased]` with the version number and date, and create a new `[Unreleased]` section
3. Entries should be concise and human-readable
4. Reference issue numbers and pull request numbers where applicable

---

*This changelog is maintained as part of the Code of Universe Regulations democratic process.*
*All dates in YYYY-MM format.*