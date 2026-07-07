---
created: "2026-06-30"
last_edited: "2026-07-04"
---

# Cavekit: Engine Embedding & Build

## Scope
Bringing up the UXP/Goanna engine as an embedded renderer inside the daemon:
producing an embedding-capable engine build out-of-tree, initializing the
embedding runtime once per process, creating/destroying browser instances, and
integrating the engine's event loop with the daemon's. Reference:
`render/goanna/PORT-MAP.md`, `render/goanna/README.md`.

## Requirements

### R1: Out-of-tree, embedding-capable engine build
**Description:** Goanna is built from the upstream UXP source as a library the daemon links against — not a full standalone browser application.
**Acceptance Criteria:**
- [x] A committed, reproducible engine configuration builds the engine without a Pale Moon/Basilisk front-end application.
- [x] The build outputs the engine library plus the generated interface headers the backend consumes.
- [x] The build is reproducible from documented host prerequisites.
**Dependencies:** none (build-host prerequisites documented in docs/TOOLCHAIN.md)

### R2: Embedding runtime initialized once per process; instances managed cleanly
**Description:** The daemon starts the engine runtime once and creates a browser instance per page.
**Acceptance Criteria:**
- [x] The embedding runtime initializes successfully at daemon startup and shuts down cleanly at exit.
- [x] A working profile/data directory is established for the engine.
- [x] A browser instance is created per page and destroyed on `disconnect`/purge with no leak or crash across repeated create/destroy cycles.
**Dependencies:** cavekit-ipc-contract.md (R3)

### R3: Engine event loop integrated with the daemon loop
**Description:** Engine processing and the daemon's command/IPC loop coexist without starvation.
**Acceptance Criteria:**
- [x] Page loads make progress while the daemon stays responsive to incoming YAP commands.
- [x] No busy-wait/spin; CPU is idle when no work is pending.
- [x] Timers/async engine work fire on schedule (e.g., animated content advances).
**Dependencies:** none (foundational; Tier-2 domains build on this event loop)

### R4: Engine is not vendored into this repository
**Description:** The engine source is referenced externally, never copied in.
**Acceptance Criteria:**
- [x] The repository contains no copy of the UXP source tree.
- [x] The build references the external engine source/build location.
- [x] Engine object/build directories are git-ignored.
**Dependencies:** cavekit-licensing-branding.md (R4)

## Out of Scope
- Pixel readback and framebuffer delivery (cavekit-offscreen-rendering.md).
- Cross-toolchain for ARM (cavekit-device-build.md).
- Branding pref/resource overrides (cavekit-licensing-branding.md).

## Cross-References
- See also: cavekit-offscreen-rendering.md, cavekit-navigation-events.md, cavekit-desktop-build.md, cavekit-device-build.md, cavekit-licensing-branding.md

## Changelog
- 2026-06-30: Initial draft.
- 2026-07-04: Status reconciled to implementation — all R1–R4 verified: libxul builds out-of-tree (+ ARM cross-build this session), init/shutdown + 20/20 create-destroy no-leak, event loop integrated, engine not vendored (docs/ENGINE-SOURCE.md).
