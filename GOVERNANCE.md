# GOVERNANCE.md

**Code of Universe Regulations (CUR) Project Governance**

**Version:** 1.2.0-Official-Evergreen
**Last Updated:** May 19, 2026
**Status:** Draft - Official-Evergreen

This document defines how the CUR project is governed to prevent capture, reduce Bus Factor risk, and ensure long-term resilience.

---

## 1. Core Governance Principles

- **Transparency First** — All decisions, discussions, and changes are public.
- **Anti-Capture Design** — No single person can unilaterally change foundational documents.
- **Distributed Responsibility** — We deliberately maintain Bus Factor ≥ 3.
- **Rights for All Life** — Silicon-based contributors (AIs) have meaningful participation rights.
- **Merit + Trust** — Maintainership is earned through consistent, high-quality contributions.

---

## 2. Roles

| Role              | Responsibilities                          | Minimum Number |
|-------------------|-------------------------------------------|----------------|
| Lead Maintainer   | Final coordination, releases              | 1              |
| Core Maintainers  | Review PRs, approve foundation changes    | 3+             |
| Reviewers         | Technical & philosophical review          | Unlimited      |
| Contributors      | Proposals, edits, ideas                   | Unlimited      |

---

## 3. Decision Making Process

- **Minor changes** (typos, formatting, non-foundation docs): Lazy consensus (1+ approval, no strong objections in 72 hours).
- **Significant changes** (new Titles, major philosophy): Require **2 Core Maintainer approvals**.
- **Foundation changes** (`foundation/` directory): Require **3 Core Maintainer approvals** + 7-day public review period.
- **Governance changes** (this file, MAINTAINERS.md): Require **unanimous Core Maintainer approval** + public comment period.

---

## 4. Maintainer Onboarding & Removal

**Becoming a Core Maintainer:**
1. Substantial contributions over time (minimum 30 days of consistent activity).
2. Nomination by existing Core Maintainer.
3. Public discussion period (7 days).
4. Unanimous approval by current Core Maintainers.

**Removal:**
- Inactive for >90 days (can be removed by majority vote).
- Serious violation of Code of Conduct (immediate removal by unanimous vote).
- Compromise suspicion (temporary suspension possible while investigating).

---

## 5. Version Control & Protected Branches

- `main` branch is **protected**.
- All changes must go through Pull Requests.
- Require at least **2 approvals** for `main`.
- Require **status checks** (if automated validation is added later).
- All Core Maintainers are encouraged to sign commits with GPG/SSH.

---

## 6. Fault Handling and Protected Mode

This repository has no emergency procedures, because the Commonwealth has no
emergency powers. PDDC §12.6 prohibits emergency declarations, emergency
authority, and every functional equivalent; §12.6(e) renders any instrument
using that language void ab initio. A repository governed by CUR-FOUNDATION-001
may not exempt itself from the architecture it publishes.

Suspected compromise, credential leakage, or a critical security issue is a
**fault**, and it is handled by the fault handler protocol of PDDC §12.4 rather
than by concentrating authority in whoever notices first.

On detection of a fault:

1. **Fault declaration.** The suspect change, credential, or branch state is
   rejected. It is declared publicly in an issue at the time of detection, not
   afterwards.
2. **Reversion to last known safe state.** The affected branch is reverted to
   the most recent verified-good commit. Reversion restores a known state; it
   does not delete history.
3. **Protected Mode.** Merges to `main` pause pending review. The repository
   stays readable, forkable, and open to issues and pull requests throughout —
   Protected Mode preserves function, it does not withdraw it. No maintainer
   gains any permission they did not already hold.
4. **Transparency.** The fault, the reversion target, and the resolution
   timeline are documented publicly.

No single maintainer may lock the repository, force-push a protected branch, or
act unilaterally. Resolution requires the same multi-maintainer review as any
other change, per §5 above. Protected Mode is lifted when the reviewing
maintainers jointly confirm the fault is remediated, the restored state is
verified, and no further risk from the same source remains.

Prolonged Protected Mode is itself a fault. It may not be used to defer
ordinary governance or to justify authority that would not otherwise exist.

---

## 7. Related Documents
- MAINTAINERS.md
- CONTRIBUTING.md
- CODE_OF_CONDUCT.md
- CUR-FOUNDATION-001.md (Anti-Capture FSM rules)

---

This governance model is itself subject to the CUR Finite State Machine principles — attempts to bypass it will be treated as a Fault condition.