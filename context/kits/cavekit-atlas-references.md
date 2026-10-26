---
created: "2026-10-26"
last_edited: "2026-10-26"
complexity: quick
---

# Cavekit: The Atlas Reference Repositories, Current

## Scope
The owner, 2026-10-26: **"update atlas browser reference repos to the latest by pulling from upstream."**

Jihad's adapter and daemon descend from the same isis-browser code as Herrie82's Atlas Browser, and the Atlas sources
are Jihad's working reference for the card side: the PGContext paint path (cavekit-offscreen-rendering.md R6), the
BrowserServer protocol's later commands, the scroll buffer, the WPE backend's engine seam (docs/PICKUP.md). They live
beside the project in `../Atlas/` as plain checkouts, read and never built or changed.

What the repositories are, read on 2026-10-26 (GitHub API):

| checkout | origin | its own upstream | newest push |
|---|---|---|---|
| `Atlas/atlas-browser-app` | `Herrie82/atlas-browser-app` — GitHub redirects it: the app moved to **`webOS-ports/org.webosports.app.atlas`** ("webOS Atlas Browser App based on WPE") | — | 2026-10-20 |
| `Atlas/BrowserAdapter` | `Herrie82/BrowserAdapter` | a fork of `woce/BrowserAdapter` (2013) | 2026-07-28 |
| `Atlas/BrowserServer` | `Herrie82/BrowserServer` | a fork of `isis-project/BrowserServer` (2013) | 2026-07-14 |
| `Atlas/atlas-wpe-backend` | `Herrie82/atlas-wpe-backend` | none (its own) | 2026-08-24 |

The Atlas work is the upstream: Herrie82's repositories and webOS-ports' app. The forks' parents (`woce`,
`isis-project`) are the 2013 code Atlas started from; "pulling from upstream" there would go backwards. All four
checkouts were at their remote's default-branch head on 2026-10-26 (`git fetch`, 0 commits behind), but the app's
remote URL is the old name and no checkout fetches branches other than its default one or records what was last read.

Reference: the session-start rule "fast-forward the reference repos first" (memory `update-workspace-repos-first`).

## Requirements

### R1: Each reference tracks its real upstream
**Description:** Every Atlas checkout fetches from where Atlas development happens now, all of it.
**Acceptance Criteria:**
- [ ] `atlas-browser-app`'s origin is `https://github.com/webOS-ports/org.webosports.app.atlas` (not the redirected old name); the other three keep Herrie82's repositories, and the fork parents (`woce`, `isis-project`) are added as a second, read-only remote named `ancestor` for comparison, never pulled into the default branch
- [ ] Every checkout has fetched all branches and tags of its origin (`git fetch --all --tags --prune`), and its default branch is at the origin's head by fast-forward only (a reference is never merged, rebased or edited)
- [ ] A checkout with local changes or a diverged default branch is reported and left alone, not reset
**Dependencies:** none

### R2: One command updates them and says what moved
**Description:** The update is repeatable at every session start.
**Acceptance Criteria:**
- [ ] A host script (`tools/host/update-references.sh`) runs R1 for the four Atlas checkouts (and takes other reference checkouts as arguments: `webos-docs`, `ref-forks/lunace-webos-ports`), with no network writes and no change to Jihad's own repository
- [ ] It prints, per checkout, the commits that arrived (count and one line each, newest first), new branches and tags, and the head it ended at; with nothing new it says so
- [ ] It refuses to run while any checkout has uncommitted changes in it, naming them
**Dependencies:** R1

### R3: What moved is read for Jihad
**Description:** An update is only useful if what changed upstream is weighed against Jihad.
**Acceptance Criteria:**
- [ ] The last-read head of each reference is recorded in `docs/REFERENCES.md` (repository, branch, commit, date), so the next update shows only the commits since
- [ ] For the commits since the last record, a short note in the same file says which touch something Jihad shares or mirrors (the BrowserServer command set and the YAP contract, cavekit-ipc-contract.md; the adapter's paint, scroll and input paths, cavekit-offscreen-rendering.md and cavekit-input-bridging.md; the app's card behaviour) and, per item, adopt / not applicable / owner's call
- [ ] Items marked "owner's call" are put to the owner as multiple-choice questions
**Dependencies:** R2, cavekit-ipc-contract.md R1

## Out of Scope
- Building or running Atlas, or merging Atlas code into Jihad (an "adopt" item becomes its own task on the kit it touches)
- The UXP engine (cavekit-engine-update.md) and the webOS SDK (cavekit-webos-sdk-update.md)

## Cross-References
- cavekit-ipc-contract.md — the protocol the adapter and server share
- cavekit-offscreen-rendering.md R6 — the Atlas paint path Jihad ported
- cavekit-engine-update.md — the same "read before you take it" discipline for the engine

## Changelog
- 2026-10-26: created from the owner's request; the upstream map read from the GitHub API.
