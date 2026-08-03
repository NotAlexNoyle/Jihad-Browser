---
created: "2026-08-02"
last_edited: "2026-08-03"
status: HANDOFF — read this first, then context/kits/cavekit-overview.md
---

# START HERE — handoff to the next Fable session

The browser **works** on the HP TouchPad: UXP/Goanna renders real pages end to end through the
frozen YAP contract into the unmodified isis UI, on **all three front-ends** (Enyo, Mochi, Mojo),
each driving its own daemon. This session (2026-08-02 → 08-03) fixed scrolling, long-press,
coordinate mapping, brought all three variants live, renamed the apps, put the logo on the about
pages, and built the `<select>` popup pipeline. What is still open, and the traps that cost time,
are below.

## Device facts you must know before touching anything

- **Screenshots come from `/dev/fb1`** (app layer), NOT `fb0` (status bar / chrome / alerts).
  1024x2304 virtual, 32bpp BGRA, stride 4096: `dd bs=4096 count=768`, interpret 1024x768 BGRA,
  `rotate(-90, expand)`. Wake the display first or every buffer reads black. **fb1 holds the LAST
  painted frame even after the card dies** — a screenshot is NOT proof the card is alive (this cost
  real time this session; confirm liveness from the daemon log's `client connected` / a fresh
  paint, not a screenshot).
- **App JS dev loop is UNRELIABLE right now — see the tooling wall below.** The documented loop is
  `novacom put` → `killall LunaSysMgr` → relaunch, but this session the card **froze on a cached
  build** and fresh JS would not load for hours despite reboot / version bump / md5-verified disk.
  **Fix the card dev loop before trusting any card-JS change** (impl-select-popup-2026-08-03.md).
- **After every `novacom put`, md5-compare both sides** (a push died mid-transfer earlier and left
  a zero-byte daemon; the exit status said success). NOTE: md5 verifies the FILE, not that the
  CARD reloaded it — those are different problems.
- **The user can perform physical taps/gestures on request** — the only way to exercise the pen
  path (the touchscreen is not an evdev device, nothing can be injected). The **daemon** has an
  off-by-default `$JIHAD_INJECT` self-drive channel for clicks/scroll/zoom without a human, but it
  needs a live adapter connection (or the offline `jihad-adapter-arm`).
- **Deploy tooling built this session:** `build/webos-oe/push-variant.sh` (full payload, md5-verified
  tarball + runs the variant's real postinst as root) and `push-engine-update.sh` (fast libxul +
  daemon swap, atomic mv, restarts the job). These are the autonomous device routes.

## What this session finished (all committed, `main`, NOT pushed)

- **Scrolling — DONE, user signed off ("scrolling feels good now").** Overscan paint with honest
  geometry, ≤2048-row region (SGX texture cap), direction-biased headroom; removed the settle
  gate; echo-suppression; fit-zoom floored to an identity blit; coverage-aware repaint +
  pan-cadence refresh. Opus-reviewed (16 findings, 3 blockers, all fixed pre-deploy).
  `impl-scroll-overscan-2026-08-02.md`.
- **Long-press — WORKS, user-confirmed (test-page banner went green).** Root cause: the daemon's
  `asyncCmdHitTest` was a stub and the adapter GATES every long-press on that round-trip. Real
  hit-test now implemented (HitTest.schema JSON).
- **Input coordinate mapping — fixed.** Input was dispatching DOCUMENT coords as VIEWPORT coords
  (presses landed a screenful low). One `docToViewport` at the drain covers click/mouse/
  contextmenu/touch/hit-test. Likely also the old "links below the fold don't navigate" cause.
- **All three variants LIVE (R7 real for the first time).** Mochi re-deployed, Mojo's first-ever
  run; cold boot auto-starts all three daemons on their own sockets (~27 MB RSS each), each card
  paints through its own engine, `device-independence-test.sh check` 24/24.
- **Apps renamed** Jihad Enyo / Jihad Mochi / Jihad Mojo; Enyo start page follows the VKB/orientation.
- **Jihad logo on `about:` and `about:jihad`/`about:isis`** (the source PNG has a baked-in
  checkerboard; border flood-fill replaces it per page background).

## Work queue — what to do next, in priority order

### 1. Finish the `<select>` popup — RESTORE THE CARD DEV LOOP FIRST — `impl-select-popup-2026-08-03.md`
The daemon/engine/adapter half is **done + device-verified** (a `<select>` tap serializes the real
options, emits `msgPopupMenuShow`, applies the returned index). The card list renders EMPTY and
could not be debugged because **two card-side tools broke**: fresh card JS would not load (frozen
cache), and `enyo.log` stopped reaching `palm-log`. **Do this first:** on a clean boot, prove fresh
JS loads via a boot-marker log line (NOT disk md5), and confirm `enyo.log` reaches `palm-log`
again. Then the current `Browser.js` (plain `Popup` + a `Button` per option) either works or the
`[JSEL]` diagnostic in git history pins it in one tap. Then mochi/mojo get their own idiom.

### 2. XUL `<menupopup>` (about:addons tools menu, context menus) — `impl-menupopup-2026-08-02.md`
DIAGNOSED: the popup widget IS created at the right place but 0x0 and never shown; it is a separate
display root the offscreen capture doesn't composite. Two-part fix (size+show the popup widget;
overlay-composite `GetVisiblePopups()` after the main paint). `build-popup-probe.sh` is the loop.
NOTE: `<select>` (item 1) is the higher-value, closer-to-done instance of the same "engine popup is
a separate display root" problem — finish it first, the menupopup overlay pass is bigger.

### 3. Chrome icon repaint latency — `impl-addons-icons-open.md`
`about:addons` icons render but LATE. Sync decode is on, so this is repaint-delivery latency (the
~150 ms paint rate limit / image-completion invalidation). Lower-priority polish.

### 4. XPI install (browser-services R3) — `impl-r8-palemoon-basilisk.md`, `impl-select-popup-2026-08-03.md`
The daemon `amIWebInstallPrompt` + `jihad-xpi-confirm` observer are AUTHORED and committed but
deliberately UNWIRED (no manifest) — a blocking confirm with no card reply path would hang the
daemon. Wire it only AFTER the card dialog reply path is proven (same card-loop dependency as #1,
and it should reuse whatever confirm mechanism the `<select>` popup ends up using).

### 5. R7 NPAPI plugins — configuration done; the blocker is that **windowless NPAPI does not exist
in a cairo-headless build** (`NPNVSupportsWindowless` answers false unless Win/Mac/X11-GTK). Must be
PORTED, not enabled. Neither reference browser faced this. Large; lowest priority.

### Also open (pre-existing, device-gated)
- cookie/cache persistence: no `cookies.sqlite` created on device despite a correct provider + prefs
  (browser-services R2; the one non-hardware debug lead).
- VKB white-band / "snap" jank (input R2); real pinch/touch gesture path (input R3).
- ui-shell R4 findInPage focused test (smallest open item).
- device LunaService methods (IPC R4).
- clean-clone reproducibility of the OE build (device-build R3, review #7/#8).
- TouchPad Go / Opal hardware + memory budget (device-build R5/R6).

## The F7 follow-up owed from the scroll work
The overscan header geometry now varies per frame, so a torn read mispositions a frame. Mitigated
by widening the in-flight reclaim valve to 2000 ms; the REAL fix (a frame sequence number in the
shared header + an adapter-side re-read guard) needs an **adapter rebuild** and is queued for the
next adapter change. `impl-scroll-overscan-2026-08-02.md`, finding F7.

## Verification standards this project learned the hard way (still true)

1. **Believe the artifact, not the exit status** (a `.ipk` install "succeeded" with a zero-byte
   daemon; a build script reported success after `mach configure` failed).
2. **fb1 holds the last frame — a screenshot is not proof of card liveness.** Confirm from the
   daemon log. (New this session; cost real time on the select popup.)
3. **md5 of the file on disk is not proof the card RELOADED it.** The WebAppMgr/LunaCE in-process
   JS cache can serve a stale build for hours. Prove card freshness with a runtime marker.
4. **One timed capture cannot distinguish "never" from "late"** (the icon investigation).
5. **Instrument the boundary before theorising** — but make sure the instrument's output actually
   arrives (this session, `enyo.log` silently stopped reaching `palm-log`).

## Uncommitted in the working tree, on purpose

`render/browserserver/JihadBrowserServer.cpp`, `render/goanna/GoannaRenderPage.{cpp,h}` — a
`$JIHAD_INJECT`-gated debug channel (`jsurl` runs privileged JS in a chrome document, `title` reads
it back). It compiles (the ARM daemon builds with it) and is off by default. Useful for the popup
investigations.
