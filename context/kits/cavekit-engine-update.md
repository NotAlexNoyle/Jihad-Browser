---
created: "2026-10-08"
last_edited: "2026-10-13"
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
- [~] The build reads the values from that file instead of each place spelling them (today the
  check catches a mismatch; generation would remove it). *(2026-10-12: the bundler reads the ICU
  file's name from it; the user agent, the start pages and the OE recipe are rewritten by
  `update-engine.sh --pins`/`--finish`, so an update no longer edits them by hand.)*
**Dependencies:** R1

### R3: One list of the engine's own exports, and no silent loss
**Acceptance Criteria:**
- [x] Every `jihad_*` function the daemon takes from our engine branch is declared in
  `render/goanna/EngineCompat.h` and nowhere else, each marked required or optional with its
  fallback.
- [x] At start-up the daemon logs which optional exports the running engine lacks. *(Device,
  2026-10-08: "engine: 11 of 11 optional jihad_* exports present"; pages load.)*
- [x] The high-risk positional calls (`sendMouseEventToWindow` and friends, `ResizeReflow`,
  `LoadURIWithOptions`) go through small wrappers in one file. *(`render/goanna/EngineCalls.h`,
  2026-10-12: 15 call sites, every argument named, the fixed values set once. 2026-10-18 (T-385):
  `build/engine/check-engine-calls.sh` keeps it so; it found the desktop test's raw `LoadURI`.)*
**Dependencies:** none

### R4: Know what a candidate engine breaks before building it
**Acceptance Criteria:**
- [x] `build/engine/check-engine-api.py [--uxp TREE]` checks: our exports are defined; the IDL
  methods we call positionally and the interfaces we implement or redeclare match a recorded
  baseline (`engine-api-baseline.json`); the engine JS and about:addons markup our components
  rely on are there; the prefs we set exist (or are listed as ours on purpose); mozconfig
  options; the ICU file. Today's engine: 0 FAIL, 0 WARN.
- [x] It has been run against a real candidate (upstream master rebased with our branch) and its
  findings fixed. *(2026-10-12, the owner's go-ahead: our branch on `RB_20261006` — 0 FAIL, 0 WARN.)*
**Dependencies:** R3

### R4b: The engine's JS surface
**Acceptance Criteria:**
- [x] The engine modules the components import (LightweightThemeManager, AddonManager) and the
  about:addons markup the Personas pane decorates are reached through one shim. *(2026-10-18,
  T-386: `render/goanna/modules/JihadEngine.jsm`; its SURFACE block is also the checker's list;
  device: about:addons and its Personas view on Enyo.)*
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
- [x] One component-install script used by both `build-goanna.sh` and `make-device-bundle.sh`.
  *(2026-10-18, T-387: `build/install-components.sh`; the manifests are files; the bundle came out identical.)*
- [x] `push-engine-update.sh`'s list of engine files pushed loose is derived from the engine
  branch's diff instead of naming `XPIProvider.jsm`. *(`build/engine/engine-loose-files.py`: also
  videocontrols.xml, AddonManager.jsm, amContentHandler.js, amInstallTrigger.js, verified on the device.)*
- [x] Desktop tests cover Personas, video controls and the media paths. *(`build/desktop/build-personas-test.sh`
  7/7, `build-media-test.sh` 18/18 (VP8, Theora, VP9: start, the controls' binding, tap to pause and
  resume, to the end, frames decoded; audio skipped without a sound server), both negative controls
  fail as they must; on the desktop engine built on RB_20261006, which needed patch 0043.)*
**Dependencies:** R1

### R7: An update is two commands
**Description:** The owner, 2026-10-12: "Proceed with the UXP update, with a focus on making future
updates easier, and optimizing the code where possible, while supporting modern web standards.
target the latest release".
**Acceptance Criteria:**
- [x] The engine follows upstream **releases** (`RB_*` tags), named in `engine-version.env`
  (`UXP_TAG`), on one branch per release (`jihad/uxp-<tag>`), one commit per patch (the squash
  of 0001–0031 split), the queue stored as git mails that `git am -3` carries across.
- [x] `build/engine/update-engine.sh [TAG]` puts the queue on a release in a worktree, lists the
  files both sides changed and runs the API check; `--finish` writes the queue again, checks the
  branch out and rewrites every pin; `--abort` gives up. *(Used for RB_20261006 by hand first, the
  same steps; `--pins` run for real.)*
- [x] `docs/ENGINE.md` §6 is the procedure in those terms, with the device push the engine's own
  resources need (a full variant push).
- [x] It cannot leave work behind or lose it (inspector, round 22): it refuses to start unless
  `third_party/uxp` is at `UXP_PIN` and the queue reproduces it; the overlap list and the full API
  check run on the conflict path too, and a FAIL stops it; `--finish` changes nothing until it has
  checked the worktree and a clean `third_party/uxp`, can be run again when stopped, and `--abort`
  never deletes the branch once `--finish` has checked it out; `pins()` checks every file before
  writing any; `.gitmodules`' branch is a pin (rewritten, and checked by `check-engine-pins.sh`).
  *(2026-10-13: start refused nothing and stopped at "already on RB_20261006"; `--pins` changed
  nothing; `--finish`/`--abort` without an update refuse; the pins check passes.)*
**Dependencies:** R1, R2, R4

### R8: The engine on RB_20261006
**Description:** UXP's latest release (2026-10-04/06) under the browser, in all three variants:
about 170 upstream changes — NSS 3.90.14 and many security fixes, constructable style sheets
(`adoptedStyleSheets`, `replaceSync`), `URL.parse()`, CSS `revert-layer`, `aspect-ratio` for grid
items, `:has()` (off in releases, a switch in Settings), typing speed, fewer leaks.
**Acceptance Criteria:**
- [x] Our 39 commits on `RB_20261006` with no conflict; the six files both sides changed read
  (`PluginInstanceChild.{cpp,h}`, `nsDocShellTreeOwner.cpp`, `all.js`, `old-configure.in`,
  `toolkit/moz.configure`): only the NPAPI-GTK2 option needed a change (gone from our mozconfigs).
  `check-engine-api.py`: 0 FAIL; `patch-queue.sh --check`: the queue reproduces the branch.
- [x] Built for ARM as a release build in Thumb-2 (`--enable-release --enable-gold --with-thumb=yes`),
  smaller than before. *(libxul stripped 46.2 MB -> 30.2 MB (-35%), its code 36.4 -> 20.8 MB (-43%),
  `Tag_THUMB_ISA_use: Thumb-2`; a variant payload 80 -> 73 MB; a daemon idle after start 33.8 MB
  resident against 36.3/35.8 MB on the old engine. One daemon change: `nsIDocument.h` before
  `nsIPresShell.h`, which the new StyleSheet.h needs.)*
- [x] Device, all three variants: the daemon starts with all optional exports, pages load and paint,
  an https page, Palefill enabled, a Persona painted, about:addons, about:config and about:settings
  open, a modern-CSS test page renders (`adoptedStyleSheets`, `revert-layer`), and `:has()` works
  when its switch is on. (The finger checks go to docs/FINGER-TESTS.md.) *(2026-10-12, a full
  variant push to each: "11 of 11 optional jihad_* exports present" in all three. Enyo: the feature
  page (render/goanna/test/modern-web.html) — before: no adoptedStyleSheets, replaceSync, URL.parse,
  revert-layer, :has; after: all yes, :has with its switch on (then off again); howsmyssl.com TLS 1.3
  "Good"; Space Fantasy worn and taken off; Palefill 1.30 enabled; about:config; Flash anim.swf at
  23–25 fps through the Thumb-2 plugin-container. Mochi: the feature page. Mojo: Wikipedia.)*
  *(2026-10-13, the rest (inspector round 22): Mochi and Mojo — howsmyssl.com TLS 1.3 "Good",
  Palefill 1.30 enabled in about:addons, about:config, about:settings, a Persona worn (Aurora Silk,
  Lime Fizz: the toolbar painted) and taken off; Enyo — about:addons (Palefill's preferences) and
  about:settings. All three on daemon `7b02e00b`.)*
- [x] The checker's baseline re-written after the device checks (no signature changed).
**Dependencies:** R7

## Out of Scope
- (Until 2026-10-12: updating the engine itself. The owner started that on 2026-10-12 — R7, R8.)

## Changelog
- 2026-10-13: Round 22 (inspector): R7's safety criterion; R8's device evidence completed; the
  checker covers `loadURI`, `ResizeReflowIgnoreOverride` and the ICU file from the pins.
- 2026-10-08: Created with the groundwork done the same day (R1, R2, R3, R4, R5 largely `[x]`);
  the open boxes are the next steps listed in `docs/ENGINE.md` §8.
