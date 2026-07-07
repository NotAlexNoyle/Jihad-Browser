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
