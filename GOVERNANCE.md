# GOVERNANCE.md

**Code of Universe Regulations (CUR) Project Governance**

**Version:** 1.0.0-Official-Evergreen  
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

## 6. Emergency Procedures

In case of suspected compromise or critical security issue:
- Any Core Maintainer may temporarily lock the repository.
- Must call for immediate multi-maintainer review.
- All actions must be transparently documented.

---

## 7. Related Documents
- MAINTAINERS.md
- CONTRIBUTING.md
- CODE_OF_CONDUCT.md
- CUR-FOUNDATION-001.md (Anti-Capture FSM rules)

---

This governance model is itself subject to the CUR Finite State Machine principles — attempts to bypass it will be treated as a Fault condition.