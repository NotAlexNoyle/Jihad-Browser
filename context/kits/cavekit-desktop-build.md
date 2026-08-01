---
created: "2026-06-30"
last_edited: "2026-07-04"
---

# Cavekit: Desktop Build & PoC Harness

## Scope
The Phase-1 x86_64 Linux build that proves the Goanna backend satisfies the
BrowserServer contract before any embedded-toolchain work. Covers build wiring,
a minimal YAP test client, and the Phase-1 end-to-end acceptance gate.
Reference: `build/desktop/README.md`, `docs/TOOLCHAIN.md`.

## Requirements

### R1: Desktop build of the daemon with the Goanna backend
**Description:** The render daemon builds and links against the Goanna backend on a normal Linux desktop.
**Acceptance Criteria:**
- [x] A documented build (single entry point) produces a runnable daemon on x86_64 Linux.
- [x] The daemon links the engine-agnostic IPC layer with the Goanna backend providing the page surface.
- [x] The build compiles out the LunaService dependency for desktop.
**Dependencies:** cavekit-engine-embedding.md (R1), cavekit-ipc-contract.md (R1)

### R2: Minimal YAP test client
**Description:** A small client drives the daemon without the full webOS stack.
**Acceptance Criteria:**
- [x] The client connects, allocates the shared buffers, and issues `connect` + `openUrl`.
- [x] It receives paint-ready notifications and writes the buffer to an image file.
- [x] It returns buffers per the contract so rendering continues.
**Dependencies:** cavekit-ipc-contract.md (R2), cavekit-offscreen-rendering.md (R3)

### R3: Phase-1 acceptance — end-to-end render
**Description:** A real page renders correctly through the whole pipe on desktop.
**Acceptance Criteria:**
- [x] Loading a known local HTML page (and a simple http page) yields a correct rendered image via the harness.
- [x] The expected load-lifecycle messages are observed in order.
- [x] Basic input (a click that follows a link) changes the rendered page.
**Dependencies:** cavekit-navigation-events.md (R3), cavekit-input-bridging.md (R1)

### R4: Optional — isis UI against the desktop daemon
**Description:** Where feasible, drive the daemon from the actual UI on desktop.
**Acceptance Criteria:**
- [x] A documented path drives the daemon from the Enyo UI or a faithful stand-in.
- [x] If a full desktop Enyo runtime is unavailable, this is recorded and the harness (R2/R3) is the acceptance vehicle instead. [human-review]
**Dependencies:** cavekit-ui-shell.md

## Out of Scope
- ARM cross-compilation and device packaging (cavekit-device-build.md).
- Performance/memory tuning (Phase 3).

## Cross-References
- See also: cavekit-engine-embedding.md, cavekit-ipc-contract.md, cavekit-offscreen-rendering.md, cavekit-navigation-events.md, cavekit-device-build.md

## Changelog
- 2026-06-30: Initial draft.
- 2026-07-04: Status reconciled to implementation — R1–R3 verified (DAEMON_UP, harness writes jihad-poc-render.ppm, ROUND-TRIP PASS); R4 recorded [human-review]: no desktop Enyo runtime, harness is the acceptance vehicle.
- 2026-07-31: **The R2/R3 harness had silently stopped working, and the passes restated after 2026-07-27 were inherited, not reproduced.** `render/browserserver/test/adapter_client.cpp` allocated its shmem segment as `W*H*4`, but the daemon writes the isis layout `[BrowserOffscreenInfo header][ARGB32 pixels]` and `paintToSharedBuffer()` silently returns when the segment cannot hold header+pixels — so no `msgPainted` ever arrived and the run timed out (exit 124). The daemon adopted that header in the rotation/zoom work (35ad1c1 → 8d7865c, 2026-07-27); the harness was last touched at 36e8513 and never followed. Confirmed pre-existing by building pristine HEAD in a scratch tree and reproducing the identical failure. The **device** path was never affected — the real BrowserAdapter always allocated header+pixels, which is why the layout change worked on hardware while the desktop stand-in quietly stopped. FIXED this session (segment sized for header+pixels, pixels read at `base+sizeof(BrowserOffscreenInfo)`, header geometry logged); ROUND-TRIP PASS re-verified independently by the parent session: `msgPainted … 685624 non-white px`, `header says 1024x768 zoom=1.000`, `painted=1 verified=1`, exit 0. R1–R3 keep their `[x]` because the capability was always real, but the evidence behind them is now current rather than inherited.
