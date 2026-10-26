---
created: "2026-10-26"
last_edited: "2026-10-26"
complexity: standard
---

# Cavekit: The Engine on UXP's Latest Release (2026-10-26)

## Scope
The owner, 2026-10-26: **"update uxp/goanna to the latest release as of october 26th, 2026."** This is the first
update made with the procedure cavekit-engine-update.md built (`build/engine/update-engine.sh`, `docs/ENGINE.md` §6-7).

What upstream has, read on 2026-10-26 (`git ls-remote` of https://repo.palemoon.org/MoonchildProductions/UXP and its
compare view):
- The newest **release** tag (`RB_*`, a UXP shipped with a Pale Moon release) is **`RB_20261006`** (`5f290d1`, Pale Moon
  35.0.2) — the engine's base today. Nothing newer is tagged `RB_`.
- The `release` branch is at **`RC_20261026`** (`6b18be9`, "[js] Avoid OOB array copy", 2026-10-26), 12 commits past it
  (with `RC_20261024`, `037cc5f`, between): a JS self-hosting fix, the emSign roots removed and certdata to September
  2026, SQLite 3.54.1, a lock in the component manager's memory reports, a selection fix in layout/generic, a strong ref
  in the mutation observer, an feTile overflow in gfx/2d, `jar:` no longer loadable by anyone, and three Windows-only
  file-extension commits. Goanna stays 6.9.0, NSS unchanged, ICU `icudt78l.dat` unchanged.
- None of the 12 touches a file our 69 patches touch, except the certdata: patch 0060 already carries `037cc5f`'s roots,
  so it becomes empty on the new base.

Reference: `build/engine/engine-version.env`, `build/engine/update-engine.sh`, `docs/ENGINE.md`, `docs/ENGINE-SOURCE.md`,
`context/kits/cavekit-certificate-trust.md` R2 (roots follow UXP's release).

## Requirements

### R1: The target is the owner's choice of release
**Description:** "The latest release" is either the newest `RB_` (already ours) or the release branch's candidate.
**Acceptance Criteria:**
- [x] The owner's answer is recorded here: `RC_20261026` now, or the next `RB_` tag when Pale Moon ships it (this
  kit's tasks then wait for it) *(2026-10-26, the owner: **wait for the RB_ tag**. The kit waits for the first `RB_`
  newer than `RB_20261006`; check with `git ls-remote --tags https://repo.palemoon.org/MoonchildProductions/UXP.git`)*
- [ ] The chosen tag's commit and date are pinned in `build/engine/engine-version.env` (`UXP_BASE`/`UXP_TAG`), and a tag
  name of the `RC_` form is accepted by `update-engine.sh` (its default lookup matches `RB_` only) and by
  `check-engine-pins.sh`
**Dependencies:** cavekit-engine-update.md R2

### R2: The patch queue rebased by the procedure, nothing lost
**Acceptance Criteria:**
- [ ] `update-engine.sh <tag>` applies the queue on the new base in a worktree; patch 0060 (the roots, now upstream) is
  dropped or kept empty as the owner decides, and every other patch applies (or its conflict is resolved and recorded)
- [ ] `update-engine.sh --finish` moves the engine branch (`jihad/uxp-<tag>`), the pins (the OE recipe's SRCREV, the
  user agent's short hash, the three start pages, `.gitmodules`), and `patch-queue.sh --check` reproduces the new pin
  exactly; `check-engine-pins.sh` passes
- [ ] `check-engine-api.py` reports no export lost; the daemon's start line says every optional export is present
- [ ] `JIHAD_APP_BUILD_ID` is bumped; `docs/ENGINE-SOURCE.md` names the new base
**Dependencies:** R1

### R3: The roots still UXP release's
**Acceptance Criteria:**
- [ ] `tools/host/audit-roots.py` finds the built-in roots equal to the new base's certdata (0 findings), and the
  roots' pin (`ROOTS_SOURCE`, `ROOTS_SHA256`) names the new base's commit
**Dependencies:** R2, cavekit-certificate-trust.md R2

### R4: The device runs it, nothing regressed
**Acceptance Criteria:**
- [ ] The desktop engine build and its tests pass (`docs/ENGINE.md` §7)
- [ ] The ARM engine, daemon and three packages are built and installed by full `push-variant.sh` runs; default-check
  passes in all three variants
- [ ] The areas the new commits touch are checked on the device: Jihad's own pages that load through `jar:` and
  `resource:` (about:settings, about:addons, about:personas, the start page) open; a profile with cookies and history
  survives the SQLite update (cookies kept, history listed); selection in a page (a long press) and an SVG filter page
  render
- [ ] The engine fork's branch and main are the owner's to push (`.cavekit/handoff-2026-10-24/push.sh`)
**Dependencies:** R2, R3

## Out of Scope
- Newer engine features or a Goanna version change (none in this range)
- The low-power optimizations (cavekit-lowpower-uxp.md) — separate work on top of whichever base this kit lands

## Cross-References
- cavekit-engine-update.md — the procedure this kit uses
- cavekit-certificate-trust.md R2 — roots follow UXP's release
- cavekit-device-build.md — the packages and pushes

## Changelog
- 2026-10-26: created from the owner's request; upstream read the same day (RB_20261006 newest release, RC_20261026 on
  the release branch).
