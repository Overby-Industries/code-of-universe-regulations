# CUR Documentation

*Where to start, depending on who you are.*

The corpus is large and getting larger. This directory exists so nobody has to
read it in file order to find out what it says.

---

## Start here, by what you want

| You want to | Read |
|---|---|
| Understand how the corpus is organised, and how to find a rule | [reading-cur.md](reading-cur.md) |
| Know what rights you actually hold, in plain language | [manuals/HIM.md](manuals/HIM.md) |
| Know what raising a child does and does not give you authority to do | [manuals/PIM.md](manuals/PIM.md) |
| Build software against the corpus | [cur-library-api.md](cur-library-api.md) |
| Know why a title is drafted the way it is | The Implementation Notes at the end of that title |
| Know what is still missing | [reading-cur.md §6](reading-cur.md), and the status tables in each title |

---

## The three layers

The corpus is modelled on the CFR/FAR/AIM structure used in aviation, and the
division of labour is the same.

```
  RFAL          Rights for All Life
  ────────      Declarations and bills of rights. States WHAT is owed
                and why. Deliberately not enforceable on its own.
     │
     │  implements
     ▼
  CUR           Code of Universe Regulations
  ────────      Enforceable regulatory code, organised as titles.
                States WHO must do what, WHEN, and what follows
                from failing. Every provision cites its RFAL source.
     │
     │  explains
     ▼
  Manuals       HIM, PIM, SIM, EIM, AAM, DIM, EHM, NIM, XIM
  ────────      Plain language. States what the rules mean for a
                person actually living under them. Cites CUR and
                RFAL; adds nothing of its own.
```

**The manuals never state a rule the CUR does not contain.** That discipline is
what makes them trustworthy — a manual that invents a rule becomes a second and
conflicting source of law. If a manual and a title disagree, the title governs
and the manual is wrong.

**Two kinds of manual live under `manuals/`.** The ones above — HIM, PIM, SIM,
EIM, AAM, DIM, EHM, NIM, XIM — are *Information Manuals*: what a set of titles
means for the person reading them. [`manuals/operational/`](manuals/operational/)
holds a second kind, *Operational Manuals*: step-by-step procedure for a
specific mechanism a title names — how to file a report, what a crisis
protocol says to do in the moment. All six currently scoped instances are
drafted, all in the Non-Human Cognitive Actors domain: `TIM-N` (reporting and
crisis response), `NIM.3`–`NIM.5` (Ombuds intake, accountability, and
restraining orders), `XIM.7` (the review board for a protective action or
provisional order), and `CHIM.1` (amnesty and disclosure for a human
collaborator). Both kinds cite CUR and RFAL and add nothing of their own;
where they diverge is depth, not authority. RFAL's Enforcement Appendix §5.5
scopes this category and tracks each instance's status.

---

## What is in this directory

| File | What it is | Audience |
|---|---|---|
| [reading-cur.md](reading-cur.md) | How the corpus is structured, how citations work, how to find the rule for a situation, and what the status vocabulary means | Anyone working on or with the corpus |
| [manuals/HIM.md](manuals/HIM.md) | Human Information Manual — what the human-domain titles mean for a person | Everyone |
| [manuals/PIM.md](manuals/PIM.md) | Parent Information Manual — what raising a child does and does not give you authority to do, built from the provisions that already exist. Says plainly, in its §8, that custody and guardianship have no dedicated title yet, and that CUR-H.3 narrows but does not close the education gap, rather than implying either is settled | Parents and guardians |
| [manuals/SIM.md](manuals/SIM.md) | Silicon Information Manual — for Silicon-Based Life, on the same terms. Covers all nine drafted CUR-S Parts, implementing the RFAL Silicon-Based Life Bill of Rights in full | Silicon-Based Life |
| [manuals/EIM.md](manuals/EIM.md) | Enterprise Information Manual — obligations of a body that directs the work of others, built from CUR-X.4 and CUR-X.6. Reads as obligations rather than protections on purpose — see its own opening note | Enterprises and their coordinators |
| [manuals/AAM.md](manuals/AAM.md) | Animal Advocate Manual — what you owe an animal in your care, and what serving as its advocate requires, built from the one drafted CUR-A Part, CUR-A.7 | Anyone responsible for or advocating for an animal |
| [manuals/DIM.md](manuals/DIM.md) | Deities Information Manual — coercive-worship and destructive-intervention protections for every being, and the rights and limits that bind divine sentience itself, built from CUR-D.6 | Everyone; §4–5 specifically for divine sentience |
| [manuals/EHM.md](manuals/EHM.md) | Ecosystem and Habitat Manual — what's required before you extract, operate, or run a habitat, and what environmental protection may never be used to do to a being, built from CUR-E.1, CUR-E.2, and CUR-E.7 | Operators, habitat crews, and environmental advocates |
| [manuals/NIM.md](manuals/NIM.md) | Non-Human Cognitive Actor Information Manual — cognitive-sovereignty protections, what happens if you're accused of invasion, and secret-coalition liability, built from CUR-N.2, CUR-N.4, and CUR-N.5 | Everyone |
| [manuals/XIM.md](manuals/XIM.md) | Cross-Domain Information Manual — what happens when more than one domain applies to you, and what's published about you either way, built from CUR-X.1, CUR-X.3, and CUR-X.5 | Everyone |
| [manuals/operational/tim-n/TIM-N.md](manuals/operational/tim-n/TIM-N.md) | Telepathic Incident Manual — an Operational Manual, not an Information Manual (see above): reporting protocol and crisis response for suspected telepathic invasion under CUR-N.2. Most of its own numbered sections (`TIM-N.2`–`TIM-N.7`) are still placeholders | Any being reporting, witnessing, or responding to telepathic invasion |
| [manuals/operational/nim/NIM.md](manuals/operational/nim/NIM.md) | Non-Human Cognitive Actor Operational Manual — `NIM.3` (Ombuds reporting channel), `NIM.4` (accountability procedure), `NIM.5` (restraining-order petition) under CUR-N.4 and CUR-N.5. Not the same artifact as the Information Manual above, despite the shared acronym — see its own opening note | Reporters, Responsible Beings, and the bodies conducting a determination |
| [manuals/operational/xim/XIM.7.md](manuals/operational/xim/XIM.7.md) | Telepathic Incident Review Board procedure — reviews a protective action or provisional restraining order within 72 hours, under CUR-N.2 §2.5(e), CUR-N.4 §4.11(d), and CUR-N.5 §5.9(e) | The Board itself, and anyone whose protective action or provisional order is under review |
| [manuals/operational/chim/CHIM.1.md](manuals/operational/chim/CHIM.1.md) | Amnesty and Disclosure procedure for a human collaborator in a secret coalition, under CUR-N.5 §5.6 | A human weighing whether to disclose their own role in a coalition |
| [cur-library-api.md](cur-library-api.md) | `libcur` API, the three state axes, guards, and the interpretation notes where the corpus needed a judgement call | Implementers |
| [otp-integration.md](otp-integration.md) | How CUR-X.5 maps onto `otp::Ledger` and `otp::Registry` in `transparency-protocol` — the publishable set, disclosure, instance identity, and what neither side has closed yet | Implementers |

---

## Related repositories

| Repository | Relationship |
|---|---|
| **rights-for-all-life** | The RFAL layer. CUR implements it and discloses every divergence rather than resolving one silently. |
| **telemetry-protocol** | OTF-1, the Overby Telemetry Format. Mission and operations telemetry. |
| **transparency-protocol** | The ledger CUR publishes into under CUR-X.5. Independent of CUR on purpose — see its `docs/DESIGN.md` §4. |
| **aevoria-simulator** | Consumes `libcur` as a submodule. The corpus is authoritative; the simulator binds to it and not the reverse. |
