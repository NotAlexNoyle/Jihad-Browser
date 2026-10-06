---
created: "2026-06-30"
last_edited: "2026-10-01"
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
  *(**2026-08-10 (T-132) — one more piece of positive evidence, and one honest non-result.**
  POSITIVE: `profile/prefs.js` is REWRITTEN on every `stop jihad` — its mtime advanced on each of
  two consecutive stops in a clean-restart run with `inject: no page` count 0, so the flush is
  demonstrably running, not merely wired.
  **ANSWERED THE SAME DAY, once the tool existed — and it is a clean PASS.** A `setpref` inject
  command was added to the daemon for exactly this (see below), and with it:
  `setpref s jihad.flushtest inside-window` → `ok=1 readback=[inside-window]`, `stop jihad` about
  three seconds later, and `profile/prefs.js` then contains
  `user_pref("jihad.flushtest", "inside-window");` — after which a restart reads it back through
  `getpref`. **So a pref written moments before the stop LANDS; the SIGTERM flush is doing real
  work rather than being decorative, and the lazy save timer is not the only thing that ever
  writes.** Clean-restart run, `inject: no page` count 0.
  The rest of this note is the record of how NOT to test it, kept because both wrong turns cost a
  device run each:
  ORIGINAL NON-RESULT: the question was unanswerable at first because a pref could not be written
  from the inject channel at all. `jsurl` runs a
  `javascript:` URL with the system principal, but `Components.classes` is not exposed in a CONTENT
  window's scope, so the probe silently did nothing (proved by its own readback: the document title
  never changed). **The first attempt at this test was worse and is recorded so it is not
  repeated:** `probe()` stopped the daemon without relaunching the card, so every later inject hit
  `inject: no page` and the run was void — the trap docs/PICKUP.md warns about, walked straight
  into.
  **RESOLVED by building the lever**, which is the second of the two options that were listed here:
  `setpref`/`getpref` inject commands in the daemon, where `nsIPrefBranch` is already held. That is
  what produced the PASS above, and it is reusable — several preferences-ui criteria need exactly
  this. The other option (drive `about:config` through `clickid`/`text`) was not needed.
  **Do not retry via `jsurl` + `Components`: it cannot work, and it fails SILENTLY.**)*
- [x] **Teardown order: every page is destroyed BEFORE the runtime.** *(Added 2026-08-04. `EngineHost::Shutdown` has always carried a caution that `XRE_TermEmbedding` tears down the process-wide runtime and must not run while an `nsIWebBrowser` is live — but nothing enforced it, and nothing could hit it while the daemon simply never exited. The first clean shutdown with a card attached SIGSEGV'd in engine teardown (device, faultaddr=0x150). `main()` now scopes the server, which owns every page, so the pages go away first. If you add another owner of an `nsIWebBrowser`, it belongs inside that scope.)*
- [x] **The teardown is BOUNDED and cannot become a fatal signal.** *(Added 2026-08-05, after the clean-shutdown work above BRICKED THE DEVICE. The flush is right for `stop <job>`, and dangerous at SYSTEM shutdown — and the job cannot tell the two apart, because `stop on started start_update` is what every cryptofs-dependent job on this platform uses, the stock `browserserver` included. So at reboot we are killed by the late generic sweep, exactly while cryptofs is being torn down — and the engine profile LIVES on cryptofs, so `XRE_TermEmbedding` is writing and mmap-reading files whose backing store is vanishing. The device's own log, as it went down: `minicore_launch: CRASH! ld-2.23.so(1947) received 7` and the same for 1950 — **signal 7 is SIGBUS**, the classic result of touching an mmap'd region whose file went away. Two of the three daemons took it, each then writing a minicore dump to `/var` while `/var` was unmounting. The next boot found `/var` with an unrecovered journal and never came up: no syslog, no LunaSysMgr, novacom silent. Recovery took bootie plus `novacom boot mem://` with the doctor's `nova-installer-image-topaz.uImage`, an `e2fsck` of `/var`, and a reboot. The stock BrowserServer never had this exposure because it simply dies. Fixed by keeping the flush but making it survivable: SIGBUS/SIGSEGV during teardown are treated as EXPECTED (storage went away — the profile's sqlite and atomic-rename prefs are crash-safe by design) and answered with `_exit(0)`, plus an `alarm()` deadline so a wedged flush can never hold up an OS shutdown. **AND the real fix, one level up: the upstart jobs now also carry `stop on runlevel [06]`**, so all three daemons are asked to stop EARLY in the shutdown, while cryptofs is still mounted and the flush is safe — the bailout is the net, not the plan. `runlevel [06]` is the platform's own idiom (`ntpdate-sync` uses it) and boot runlevels are 2/3, so it never interferes with starting. **Reboot-tested on device 2026-08-05 and CLEAN**: back in ~10 s, LunaSysMgr and ls-hubd up, all three daemons auto-started, `signal 15 — shutting the engine down cleanly` / `exited` in the log, and — the things that defined the failure — no SIGBUS, no minicore dump, and no `/var` journal recovery.)*
- [x] **Exiting the loop at all exposed dead teardown code that had never run once.** *(2026-08-04: `~YapServer` deletes `YapServerPriv::deadlockDetector`, which is never initialized and — with `run()`'s default `deadlockTimeoutMs` of -1 — is never created either. Indeterminate memory, non-null every time, so the first four clean exits all SIGSEGV'd there. Its own destructor also quit a null loop and joined thread 0 when the watchdog thread had not started. Both guarded. The general lesson for this embedding: **code on a path the process never takes is not "working", it is untested** — expect more of it as other never-run paths become reachable.)*
- [x] A working profile/data directory is established for the engine.
- [x] A browser instance is created per page and destroyed on `disconnect`/purge with no leak or crash across repeated create/destroy cycles.
**Dependencies:** cavekit-ipc-contract.md (R3)

### R3: Engine event loop integrated with the daemon loop
**Description:** Engine processing and the daemon's command/IPC loop coexist without starvation.
**Acceptance Criteria:**
- [x] Page loads make progress while the daemon stays responsive to incoming YAP commands.
- [x] No busy-wait/spin; CPU is idle when no work is pending. *(**CLOSED `[~]`→`[x]` 2026-10-01 — the plugin re-measurement is in.** With the quiet tick in the build, `jihad-anim.swf` under the shipped configuration measured 32.6 fps with 51% of frame gaps in the 32–39 ms bucket and nothing deferred — the same shape as the 16 ms-tick reference run and better than the shipped build was before (cavekit-addons-extensions.md R7, 2026-10-01 table); the tick statistics show the frame-class period held throughout (`min=16 avg=23`), i.e. a plugin page never goes quiet. So all three states are now measured on the device: no card 0.05% of a core, a blank page settles, a card on a static page 2.2%.)* *(**2026-10-01 (later still) — the third state is FIXED and measured; `[~]` then waited only on the plugin re-measurement.** A quiet tick was added between "frame" and "idle" (`render/browserserver/Main.cpp`, `BrowserPageGoanna::isBusy`): after ~0.2 s of ticks in which no page is busy and no card has sent a command, the tick drops from the frame period to 100 ms; any card command restores the frame period at once (the server wakes it from the command path), and page-side activity is seen by the next quiet tick. Device, Enyo, daemon `27863809…`, card connected on `jihad-plain.html`, display on, no input, 60 s: **134 CPU ticks = 2.2% of a core** (was 12.3%), 11.5 wakeups a second (was ~125), 522 log bytes. Not delayed: three injected clicks on a quiet page were each picked up within the 20 ms polling resolution of the measurement; a 10 s VP8 clip still played to the end with `vq=149/0/0`. The stated cost: a timer that fires with no command behind it is noticed up to one quiet period late — on a page changing the DOM once a second the timer ran `max=216 ms, mean=65 ms` late and painted at 1 fps, and that page averaged 9.3% of a core because it spends much of each second back on the frame tick. **What still has to be shown before this closes:** the Flash frame-gap histogram is unchanged — by construction a page with a plugin is always "busy" and never goes quiet, but that is an argument until the plugin sweep measures it (cavekit-device-build.md R4, add-ons sweep).)* *(**2026-10-01 (later) — both defects this was downgraded for are FIXED and device-verified, and the re-measurement this note asked for found a THIRD state that is not idle. Stays `[~]`.** Fixed: no card → 0.05% of a core and ~1 wakeup a second (cavekit-device-build.md R8); `about:blank` → settles, 0 paints in 30 s (cavekit-offscreen-rendering.md R8). **Still not idle: a card connected on a STATIC page with no input.** Measured on the device (Enyo, `jihad-plain.html`, display on, 60 s): 737 CPU ticks = **12.3% of a core**, all of it the daemon's main thread (236 of 239 ticks in a 20 s per-thread sample), with the tick statistics reading `work avg=0–1 ms (pump avg 0–1, paint avg 0)` over ~205 ticks per 2 s. Nothing is painted and nothing is loading; the cost is the frame tick itself — ~125 pump-and-bookkeeping passes a second at ~1 ms each on a CPU clocked at its idle floor. (With the display off the same state measured 4.7% in the morning baseline.) So a browser left open on a static page costs an eighth of a core for as long as the card exists. **To close:** with a card connected on a static page and no input, the daemon uses about 2% of a core or less over 60 s, AND input and page-driven updates are not delayed for it — a tap, a key, a scroll and a timer-driven DOM change each still reach the screen as promptly as today — AND nothing that needs the frame grid (a loading page, playing media, a plugin) is slowed.)* *(**DOWNGRADED `[x]`→`[~]` 2026-10-01 (earlier) — measured false on the device (webOS CE 3.1.0) in two states.** (1) A daemon with NO card connected runs its frame tick at the 8 ms default forever: its own statistics lines show ~241 ticks per 2 s and ~2.8% of a core per variant, with nothing to pump or paint. (2) On `about:blank` the daemon performs a full ~35 ms software paint about 22 times a second forever, because a frame it will not publish is retried every tick. Neither is a spin in the tight-loop sense — the loop does sleep between ticks — but "CPU is idle when no work is pending" is exactly what both violate. The box had no evidence note; its basis was the 2026-07 desktop harness, which never held a daemon idle with no client. **Closes with** cavekit-device-build.md R8 "an idle daemon is idle" and cavekit-offscreen-rendering.md R8 "a genuinely blank page settles"; when both are met, re-measure idle CPU with a static page loaded and no input and record it here.)* *(2026-10-01 (third session), re-measured on the INSTALLED batch-2 packages (daemon `3e6aff5a…`): static page, card connected, 60 s — 101 jiffies = 1.7% of a core, 10 wakeups/s; 1 Hz ticker late by 73 ms mean / 118 max; a click handled within 20 ms; the 10 s VP8 clip ended with 149 frames, 0 dropped; Flash 32.5 fps with 333 of 728 gaps in 32–39 ms.)*
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
