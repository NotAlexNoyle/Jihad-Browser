---
created: "2026-06-30"
last_edited: "2026-11-07"
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
- [x] The embedding runtime initializes successfully at daemon startup and shuts down cleanly at exit. *(Fixed 2026-08-04: `main()` never returned, so `XRE_TermEmbedding` never ran and the deferred savers (`XPIDatabase`, prefs) never flushed — a disabled add-on came back enabled after a restart. `Main.cpp` turns SIGTERM/SIGINT/SIGHUP into a flag the tick reads to quit the loop (`g_main_loop_quit` is not async-signal-safe), so `run()` returns into teardown. Device: a stop with a card connected logs the clean-shutdown line and writes no fault report. 2026-08-10 (T-132): `setpref s jihad.flushtest inside-window`, `stop jihad` about 3 s later, and `profile/prefs.js` holds the pref, read back by `getpref` after a restart. Use the `setpref`/`getpref` inject commands for pref tests; `jsurl` + `Components` cannot write a pref (no `Components` in a content scope) and fails silently.)*
- [x] **Teardown order: every page is destroyed BEFORE the runtime.** *(2026-08-04: the first clean shutdown with a card attached SIGSEGV'd in engine teardown (device, faultaddr=0x150). `main()` now scopes the server, which owns every page, so the pages go first. A new owner of an `nsIWebBrowser` belongs inside that scope.)*
- [x] **The teardown is BOUNDED and cannot become a fatal signal.** *(2026-08-05, after the clean-shutdown flush bricked the device at reboot: at system shutdown the daemons were killed while cryptofs, which holds the profile, was unmounting; teardown took SIGBUS, minicore dumps went to an unmounting `/var`, and the next boot found an unrecovered `/var` journal (incident and recovery steps: archive/engine-embedding-history.md). Fix: SIGBUS/SIGSEGV during teardown exit with `_exit(0)`, an `alarm()` deadline bounds the flush, and the upstart jobs carry `stop on runlevel [06]` so the daemons stop while cryptofs is still mounted. Reboot-tested 2026-08-05: back in ~10 s, all three daemons up, `signal 15 — shutting the engine down cleanly`, no SIGBUS, no minicore, no `/var` recovery.)*
- [x] **Exiting the loop at all exposed dead teardown code that had never run once.** *(2026-08-04: `~YapServer` deleted the never-initialised `YapServerPriv::deadlockDetector`, so the first four clean exits SIGSEGV'd; its destructor also quit a null loop and joined thread 0. Both guarded. Code on a path the process never takes is untested; expect more of it.)*
- [x] A working profile/data directory is established for the engine.
- [x] A browser instance is created per page and destroyed on `disconnect`/purge with no leak or crash across repeated create/destroy cycles.
**Dependencies:** cavekit-ipc-contract.md (R3)

### R3: Engine event loop integrated with the daemon loop
**Description:** Engine processing and the daemon's command/IPC loop coexist without starvation.
**Acceptance Criteria:**
- [x] Page loads make progress while the daemon stays responsive to incoming YAP commands.
- [x] No busy-wait/spin; CPU is idle when no work is pending. *(Closed 2026-10-01 on webOS CE 3.1.0. The tick follows the state (`render/browserserver/Main.cpp`, `BrowserPageGoanna::isBusy`): the frame period while a page is busy or a card sends commands, a 100 ms quiet tick after ~0.2 s with neither, 1 s with no card (cavekit-device-build.md R8); a blank page settles (cavekit-offscreen-rendering.md R8). Installed batch-2 packages (daemon `3e6aff5a…`), card connected on a static page, 60 s: 101 jiffies = 1.7% of a core (12.3% before the quiet tick), 10 wakeups/s; a 1 Hz ticker late by 73 ms mean / 118 max; a click handled within 20 ms; a 10 s VP8 clip 149 frames, 0 dropped; Flash 32.5 fps with 333 of 728 frame gaps in 32–39 ms (a plugin page is always busy). No card: 0.05% of a core. Cost: a timer with no command behind it is seen up to one quiet period late.)*
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
- 2026-11-07: Sweep: evidence notes compacted to date, task, proof and the deciding numbers; superseded notes and session narrative moved verbatim to `archive/engine-embedding-history.md`. No box changed state.
- 2026-10-06: R3's idle rule gained a clause (code + device): with no card the daemon still drains the engine main thread's READY events once per tick (one call when the queue is empty), and ticks at a 20 ms work period while events keep arriving (kept one second past the last one). Before, nothing on the chrome side — timers, async file I/O completions — ran until a card connected. Idle CPU with no card measured afterwards: 0.9–1.6 % of a core over a minute, no log lines.
- 2026-10-01 (end of day): R3 `[~]`→`[x]` — Flash pacing re-measured with the quiet tick in the build.
- 2026-10-01 (later still): R3 — quiet tick implemented and measured: a card open on a static page
  costs 2.2% of a core (was 12.3%). Stays `[~]` until the plugin frame pacing is re-measured with it.
- 2026-10-01 (later): R3 stays `[~]` — the no-card and blank-page defects are fixed and verified,
  but the static-page re-measurement found 12.3% of a core with a card merely open; closing
  condition rewritten around that.
- 2026-10-01: **Target platform is now webOS CE 3.1.0.** R3 "CPU is idle when no work is pending"
  `[x]`→`[~]`: an idle daemon ticks at 8 ms with no card, and `about:blank` repaints ~22 times a
  second forever (both measured on the device). R2's device-verified boxes are 3.0.5 evidence and
  are listed in cavekit-device-build.md R4's re-verification sweep.
- 2026-06-30: Initial draft.
- 2026-08-04: R2 corrected and extended. The "shuts down cleanly at exit" criterion had been met in error — `main()` never returned, so `XRE_TermEmbedding` never ran and the engine's deferred savers never flushed; SIGTERM is now handled. Two new criteria record what that exposed: pages must be destroyed before the runtime (the caution in `EngineHost::Shutdown` was never enforced, and violating it crashed on device), and `~YapServer`'s never-executed teardown was itself broken.
- 2026-07-04: Status reconciled to implementation — all R1–R4 verified: libxul builds out-of-tree (+ ARM cross-build this session), init/shutdown + 20/20 create-destroy no-leak, event loop integrated, engine not vendored (docs/ENGINE-SOURCE.md).
