# Proposals

*How a change to this repository gets proposed, discussed, and decided —
distinct from how the Aevoric Commonwealth's own General Assembly enacts
legislation inside the fiction.*

This folder is a real-world, repository-level process. It governs how a
contributor proposes adding, changing, or retiring something in the CUR
corpus itself — a new title, a new foundation document, a change to the
Format Guide. It is not the same thing as the in-fiction legislative process
`governance/pddc-governance-mechanics.md` TITLE 1 describes for the General
Assembly of the Aevoric Commonwealth. That process governs Avians inside the
story; this one governs contributors editing the repository that tells it.
The two are easy to conflate because this repository is written entirely in
its own fiction's voice, and `CONTRIBUTING.md` has at times blurred the two
directly — see its note on that below.

## The lifecycle

1. **Draft.** Copy [`TEMPLATE.md`](TEMPLATE.md), fill it out, and save it into
   [`ACTIVE/`](ACTIVE) with a clear, descriptive filename —
   `PROPOSAL_free-orbital-transit.md`, not `proposal1.md`.
2. **Check for duplicates.** Browse [`ACTIVE/`](ACTIVE) and
   [`ARCHIVED/`](ARCHIVED) first. `CONTRIBUTING.md` states this as the first
   step for exactly this reason.
3. **Open a pull request.** Title it `[PROPOSAL] <your title>`. Discussion
   happens on the PR.
4. **Resolve.** When a proposal is accepted, rejected, superseded, or
   withdrawn, move the file from `ACTIVE/` to `ARCHIVED/` and update its
   `Status` field. Nothing is deleted — a rejected or superseded proposal
   stays in `ARCHIVED/` as a record of what was considered and why it did not
   proceed, the same discipline `titles/README.md` requires of the corpus
   itself: a decision is disclosed, not erased.

## What belongs here versus what doesn't

A proposal under this process is about the repository's own content: drafting
a title, correcting a citation, adding a manual, restructuring a directory.
It is not a claim about what the Aevoric Commonwealth's General Assembly has
enacted — the corpus states what the Commonwealth's institutions have already
decided, in their own voice, and a repository-level proposal is the
mechanism by which a contributor proposes writing that text, not a vote
inside the fiction itself.

## A note on `CONTRIBUTING.md`

`CONTRIBUTING.md` describes this process in earlier language — it refers to
contributors as "Alliance citizens," names a `docs/CONSTITUTION.md` that
does not exist at that path (the constitutional instrument is
[`progressive-direct-democracy-constitution.md`](../progressive-direct-democracy-constitution.md)
at the repository root), and points to a `historical_analysis` folder that
is now [`history-lessons/`](../history-lessons). This folder is built to
match what `CONTRIBUTING.md` promises functionally — a template, an active
queue, an archive — using the terminology the rest of the repository has
since settled on. `CONTRIBUTING.md`'s own stale references are worth fixing
directly rather than worked around here.
