---
created: "2026-06-30"
last_edited: "2026-08-04"
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
- [x] The embedding runtime initializes successfully at daemon startup and shuts down cleanly at exit. *(The shutdown half was marked met in error until 2026-08-04. `XRE_TermEmbedding` was only ever reached by falling off the end of `main()`, and **`main()` never returned**: the GLib loop ran forever and the process was killed by SIGTERM — which is exactly what `stop <job>` sends, i.e. the NORMAL device path. Nothing flushed. The visible cost was elsewhere and took a while to trace back here: an add-on disabled from about:addons came back ENABLED after a restart, because `XPIDatabase` and prefs are DEFERRED savers that only write during XPCOM shutdown. `Main.cpp` now handles SIGTERM/SIGINT/SIGHUP by flagging the tick, which quits the loop (`g_main_loop_quit` is not async-signal-safe), so `run()` returns into the ordinary teardown. Device-verified: a stop with a card connected logs the clean-shutdown line and writes no fault report.)*
- [x] **Teardown order: every page is destroyed BEFORE the runtime.** *(Added 2026-08-04. `EngineHost::Shutdown` has always carried a caution that `XRE_TermEmbedding` tears down the process-wide runtime and must not run while an `nsIWebBrowser` is live — but nothing enforced it, and nothing could hit it while the daemon simply never exited. The first clean shutdown with a card attached SIGSEGV'd in engine teardown (device, faultaddr=0x150). `main()` now scopes the server, which owns every page, so the pages go away first. If you add another owner of an `nsIWebBrowser`, it belongs inside that scope.)*
- [x] **The teardown is BOUNDED and cannot become a fatal signal.** *(Added 2026-08-05, after the clean-shutdown work above BRICKED THE DEVICE. The flush is right for `stop <job>`, and dangerous at SYSTEM shutdown — and the job cannot tell the two apart, because `stop on started start_update` is what every cryptofs-dependent job on this platform uses, the stock `browserserver` included. So at reboot we are killed by the late generic sweep, exactly while cryptofs is being torn down — and the engine profile LIVES on cryptofs, so `XRE_TermEmbedding` is writing and mmap-reading files whose backing store is vanishing. The device's own log, as it went down: `minicore_launch: CRASH! ld-2.23.so(1947) received 7` and the same for 1950 — **signal 7 is SIGBUS**, the classic result of touching an mmap'd region whose file went away. Two of the three daemons took it, each then writing a minicore dump to `/var` while `/var` was unmounting. The next boot found `/var` with an unrecovered journal and never came up: no syslog, no LunaSysMgr, novacom silent. Recovery took bootie plus `novacom boot mem://` with the doctor's `nova-installer-image-topaz.uImage`, an `e2fsck` of `/var`, and a reboot. The stock BrowserServer never had this exposure because it simply dies. Fixed by keeping the flush but making it survivable: SIGBUS/SIGSEGV during teardown are treated as EXPECTED (storage went away — the profile's sqlite and atomic-rename prefs are crash-safe by design) and answered with `_exit(0)`, plus an `alarm()` deadline so a wedged flush can never hold up an OS shutdown.)*
- [x] **Exiting the loop at all exposed dead teardown code that had never run once.** *(2026-08-04: `~YapServer` deletes `YapServerPriv::deadlockDetector`, which is never initialized and — with `run()`'s default `deadlockTimeoutMs` of -1 — is never created either. Indeterminate memory, non-null every time, so the first four clean exits all SIGSEGV'd there. Its own destructor also quit a null loop and joined thread 0 when the watchdog thread had not started. Both guarded. The general lesson for this embedding: **code on a path the process never takes is not "working", it is untested** — expect more of it as other never-run paths become reachable.)*
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
- 2026-08-04: R2 corrected and extended. The "shuts down cleanly at exit" criterion had been met in error — `main()` never returned, so `XRE_TermEmbedding` never ran and the engine's deferred savers never flushed; SIGTERM is now handled. Two new criteria record what that exposed: pages must be destroyed before the runtime (the caution in `EngineHost::Shutdown` was never enforced, and violating it crashed on device), and `~YapServer`'s never-executed teardown was itself broken.
- 2026-07-04: Status reconciled to implementation — all R1–R4 verified: libxul builds out-of-tree (+ ARM cross-build this session), init/shutdown + 20/20 create-destroy no-leak, event loop integrated, engine not vendored (docs/ENGINE-SOURCE.md).
