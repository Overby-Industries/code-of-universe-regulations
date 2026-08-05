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
  Manuals       HIM, PIM, and others to come
  ────────      Plain language. States what the rules mean for a
                person actually living under them. Cites CUR and
                RFAL; adds nothing of its own.
```

**The manuals never state a rule the CUR does not contain.** That discipline is
what makes them trustworthy — a manual that invents a rule becomes a second and
conflicting source of law. If a manual and a title disagree, the title governs
and the manual is wrong.

---

## What is in this directory

| File | What it is | Audience |
|---|---|---|
| [reading-cur.md](reading-cur.md) | How the corpus is structured, how citations work, how to find the rule for a situation, and what the status vocabulary means | Anyone working on or with the corpus |
| [manuals/HIM.md](manuals/HIM.md) | Human Information Manual — what the human-domain titles mean for a person | Everyone |
| [manuals/PIM.md](manuals/PIM.md) | Parent Information Manual — what raising a child does and does not give you authority to do, built from the provisions that already exist. Says plainly, in its §8, that custody and guardianship have no dedicated title yet, and that CUR-H.3 narrows but does not close the education gap, rather than implying either is settled | Parents and guardians |
| [cur-library-api.md](cur-library-api.md) | `libcur` API, the three state axes, guards, and the interpretation notes where the corpus needed a judgement call | Implementers |
| [otp-integration.md](otp-integration.md) | How CUR-X.5 maps onto `otp::Ledger` and `otp::Registry` in `transparency-protocol` — the publishable set, disclosure, instance identity, and what neither side has closed yet | Implementers |

### Planned

| File | What it will be |
|---|---|
| `manuals/SIM.md` | Silicon Information Manual — for Silicon-Based Life, on the same terms |
| `manuals/EIM.md` | Enterprise Information Manual — obligations of a body that directs the work of others |

---

## Related repositories

| Repository | Relationship |
|---|---|
| **rights-for-all-life** | The RFAL layer. CUR implements it and discloses every divergence rather than resolving one silently. |
| **telemetry-protocol** | OTF-1, the Overby Telemetry Format. Mission and operations telemetry. |
| **transparency-protocol** | The ledger CUR publishes into under CUR-X.5. Independent of CUR on purpose — see its `docs/DESIGN.md` §4. |
| **aevoria-simulator** | Consumes `libcur` as a submodule. The corpus is authoritative; the simulator binds to it and not the reverse. |
