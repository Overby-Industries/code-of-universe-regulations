# VERSION.md

**Current corpus version: 1.1.0-Official-Evergreen**

## Versioning Convention for the CUR Project

- **Major.Minor.Patch-Status**
  - Major: Breaking philosophical or structural changes
  - Minor: Significant new content without breaking core
  - Patch: Fixes, clarifications, small improvements
- **Status Suffix**:
  - `-Official-Evergreen` → Stable, reviewed foundation
  - `-Draft` → Work in progress
  - `-Proposal` → Under community review

Example: `1.1.0-Official-Evergreen`

## Two kinds of version number

Not every version in this repository tracks the corpus release, and conflating
the two loses information.

**Corpus version.** Foundation documents, operational manuals, and governance
documents carry the corpus version and move together on every release. They are
`-Official-Evergreen`.

**Document version.** Each title in `titles/` carries its own version tracking
the drafting state of that Part — `CUR-N.2` is at `1.1.0-Draft` because it went
through a conformance pass, while `CUR-H.2` is at `1.0.0-Draft` because it has
been drafted once. These are `-Draft` and do not move on a corpus release. A
title's version is a statement about that title, not about the corpus.

The two are distinguished by status suffix, which is what makes the release
tooling safe to run.

## Bumping the version

Do not edit version headers by hand. The Rights for All Life framework
maintained a rule requiring identical version numbers across its corpus for
several releases while the numbers silently diverged, because the rule was
applied manually. Use the script:

```
./scripts/bump-version.ps1 -Version 1.2.0 -DryRun
./scripts/bump-version.ps1 -Version 1.2.0
```

It rewrites `-Official-Evergreen` version headers, the README badge, and any
document index version column, then reports every document it did not touch so
that nothing is silently left behind. It does not touch `-Draft` documents,
`build/`, or `CHANGELOG.md`. The changelog is written by hand.

The same script is maintained in the Rights for All Life repository at
`process/bump-version.ps1`. Either copy accepts `-Root` and will operate on
either repository.

## Relationship to Rights for All Life

CUR implements the Rights for All Life framework: RFAL supplies the declarations
and bills of rights, CUR supplies the enforceable regulatory code. The two
repositories keep **independent version numbers on synchronised releases**. Each
lineage stays truthful about its own history, and the table below records which
RFAL version a given CUR version was built against.

| CUR | RFAL | Date | Notes |
|---|---|---|---|
| 1.0.1 | 1.5.1 | 2026-05 | Foundation documents 001-013; PDDC |
| 1.1.0 | 1.6.0 | 2026-07-27 | Titles CUR-S.4, CUR-E.7, CUR-N.2, CUR-X.3, CUR-H.2, CUR-H.4, CUR-D.6; `libcur`; RFAL adds Plant Life Bill and five-bill integration |

When citing an RFAL provision from a CUR title, cite the RFAL version the
provision was read at — for example `CUR-S §4.7(f)` cites "RFAL v1.5.2" because
that is the release in which Silicon Bill Article 4 was corrected. Those
citations are historical statements and are not updated by a version bump.

## Release checklist

1. Draft or amend the documents.
2. Run the test suite if `libcur` changed.
3. Run `./scripts/bump-version.ps1 -Version X.Y.Z -DryRun` and read the report,
   including the "left alone" list.
4. Run it without `-DryRun`.
5. Write the `CHANGELOG.md` entry by hand.
6. Update the correspondence table above if the RFAL version moved.
