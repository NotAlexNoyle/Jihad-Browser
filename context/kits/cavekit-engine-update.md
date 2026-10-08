---
created: "2026-10-08"
last_edited: "2026-10-08"
complexity: complex
---

# Cavekit: Engine Update Readiness

## Scope
The owner, 2026-10-08: **"lay the groundwork for a uxp/goanna update by making our interaction
with it more modular and maintainable by humans."** This kit is that groundwork. It does not
update the engine (standing rule: no UXP update from upstream unless asked).

The engine is UXP (Pale Moon's platform) with our own branch of changes, embedded by a headless
daemon. An inventory taken the same day found: the patch queue had drifted from the engine
branch (eight commits and part of the squashed series missing — a desktop build lacked the Flash
child's self-drive switch); the engine docs described an older setup; the engine's own exports
were declared inline in two files, eleven of them weak (a dropped one fails silently); pins and
version strings lived in six places; and nothing could tell, before building, what a candidate
engine would break. The engine is 76 upstream commits behind.

Reference: `docs/ENGINE.md` (the guide), `docs/ENGINE-SOURCE.md` (the MPL record),
`build/engine/` (the tools), `render/goanna/EngineCompat.h`.

## Requirements

### R1: One source of truth for the engine's changes
**Acceptance Criteria:**
- [x] The rule is written down: every engine change is a commit on `jihad-engine-mods`; the patch
  queue is generated from those commits. (`docs/ENGINE.md` §2)
- [x] A check proves the queue, applied to the upstream base, reproduces the pinned engine tree
  exactly, without a checkout: `build/engine/patch-queue.sh --check` — passes (39 patches).
- [x] A new engine commit becomes the next patch with one command (`patch-queue.sh --add`).
- [x] The queue's drift is repaired (0031 the squash remainder, 0032–0039 the later commits) and
  its obsolete files (a never-applied snapshot, a WIP patch, the hand-diff script and its
  baseline) are gone.
**Dependencies:** cavekit-engine-embedding.md R1

### R2: One place for pins and version strings
**Acceptance Criteria:**
- [x] `build/engine/engine-version.env` holds the upstream base, our pinned commit, the Goanna
  version, the short hash the user agent shows, and the ICU data file.
- [x] `build/engine/check-engine-pins.sh` fails when the gitlink, the OE recipe's SRCREV, the user
  agent, the three start pages or the ICU file name disagree with it — passes today.
- [ ] The build reads the values from that file instead of each place spelling them (today the
  check catches a mismatch; generation would remove it).
**Dependencies:** R1

### R3: One list of the engine's own exports, and no silent loss
**Acceptance Criteria:**
- [x] Every `jihad_*` function the daemon takes from our engine branch is declared in
  `render/goanna/EngineCompat.h` and nowhere else, each marked required or optional with its
  fallback.
- [x] At start-up the daemon logs which optional exports the running engine lacks. *(Device,
  2026-10-08: "engine: 11 of 11 optional jihad_* exports present"; pages load.)*
- [ ] The high-risk positional calls (`sendMouseEventToWindow` and friends, `ResizeReflow`,
  `LoadURIWithOptions`) go through small wrappers in one file.
**Dependencies:** none

### R4: Know what a candidate engine breaks before building it
**Acceptance Criteria:**
- [x] `build/engine/check-engine-api.py [--uxp TREE]` checks: our exports are defined; the IDL
  methods we call positionally and the interfaces we implement or redeclare match a recorded
  baseline (`engine-api-baseline.json`); the engine JS and about:addons markup our components
  rely on are there; the prefs we set exist (or are listed as ours on purpose); mozconfig
  options; the ICU file. Today's engine: 0 FAIL, 0 WARN.
- [ ] It has been run against a real candidate (upstream master rebased with our branch) and its
  findings fixed. [owner's decision: when to update]
**Dependencies:** R3

### R4b: The engine's JS surface
**Acceptance Criteria:**
- [ ] The engine modules the components import (LightweightThemeManager, AddonManager) and the
  about:addons markup the Personas pane decorates are reached through one shim.
**Dependencies:** R4

### R5: Written for a human
**Acceptance Criteria:**
- [x] `docs/ENGINE.md`: where the engine is, the rule, the delta by area, every touchpoint with how
  likely an update breaks it (and which breaks are silent), the tools, a step-by-step update
  procedure, which tests guard which touchpoint, a device checklist, and what is not done yet.
- [x] `docs/ENGINE-SOURCE.md` (the MPL-2.0 record) lists every one of the 39 changes with its
  files; `docs/UXP.md` and `build/desktop/README.md` point at `ENGINE.md` instead of describing
  the old setup.
**Dependencies:** R1–R4

### R6: Builds that cannot drift
**Acceptance Criteria:**
- [ ] One component-install script used by both `build-goanna.sh` and `make-device-bundle.sh`.
- [ ] `push-engine-update.sh`'s list of engine files pushed loose is derived from the engine
  branch's diff instead of naming `XPIProvider.jsm`.
- [ ] Desktop tests cover Personas, video controls and the media paths.
**Dependencies:** R1

## Out of Scope
- Updating the engine itself (rebasing onto upstream, rebuilding, device regression) — a
  separate, owner-initiated piece of work that this kit prepares for.

## Changelog
- 2026-10-08: Created with the groundwork done the same day (R1, R2, R3, R4, R5 largely `[x]`);
  the open boxes are the next steps listed in `docs/ENGINE.md` §8.
