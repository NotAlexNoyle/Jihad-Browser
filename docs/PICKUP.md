# PICKUP — Jihad Browser, current state (2026-08-10 body, deltas 2026-08-15 … 2026-10-19 below)

**Read this file first. It is the only current handoff.** Everything else in `docs/` is
reference material for a specific subsystem, not a status document.

---

## After Jihad Browser: Global Jihad (a distinct project — NOT started; begins only when Jihad Browser is finished)

Decided with the owner on 2026-10-18. System-wide Goanna: every webOS app rendered by Goanna, not
only the browser's pages. It is its own project with its own name; nothing of it is built inside
Jihad Browser, and it starts when Jihad Browser is finished.

**Facts that shape it (verified on the owner's TouchPad, 2026-10-18):**

- No engine-swap layer exists in Atlas or in webOS CE. Both replace the engine only for the browser
  app, behind BrowserAdapter → YAP → BrowserServer (Atlas: WPE WebKit 2.52 in
  `atlas-wpe-backend`; Jihad: Goanna). `luna-webkit-api` is an IME/field API, not an engine seam.
- The owner's TouchPad runs **webOS CE 3.1.0** (`/etc/palm-build-info`), not stock 3.0.5. Its web
  app manager is a forked second `LunaSysMgr -s` (ppid = the first; 124 MB) running the
  `WebAppManager` class from the LunaSysMgr binary and mapping `libWebKitLuna.so` (12.8 MB, Palm's
  WebKit; `SysMgrWebPage : QWebPage`). Stock 3.0.5 has the same child, closed-source.
- Apps load the JS frameworks by absolute path: 13 third-party apps and the stock ones use
  `<script src="/usr/palm/frameworks/enyo/0.10/framework/enyo.js">` (that directory IS Enyo 1;
  `enyo/1.0` is empty), Mojo apps `/usr/palm/frameworks/mojo/mojo.js` or `mojoloader.js`. A
  patched framework there serves every app, no per-app change.
- The frameworks' WebKit-only surface, counted on the device. Enyo 1 (`enyo/0.10`: 419 js, 170 css,
  12 MB): `-webkit-border-image` ×569 (no Gecko alias), `-webkit-box`/`-webkit-box-flex` ×16,
  `-webkit-gradient` ×11, `webkitTransform` ×23, `webkitTransitionEnd` ×11,
  `webkitRequestAnimationFrame` ×4, 31 `PalmSystem` members, `PalmServiceBridge` in 8 files. Mojo
  (10 js, 60 css): `-webkit-border-image` ×257, `-webkit-palm-mouse-target` ×19 (Palm-only), 46
  `PalmSystem` members, `PalmServiceBridge` in 6 files. 41 apps installed: 39 web, 2 PDK
  (unaffected), 3 Mojo.

**The strategy (the owner's):**

1. **Rebuild the forked WebAppManager instance from the webOS CE source on Goanna**, in place of
   `libWebKitLuna.so`: the LunaSysMgr window protocol (`luna-sysmgr-ipc` messages; card, popup,
   dashboard and alert surfaces; lifecycle; launch parameters), the `PalmSystem` object,
   `PalmServiceBridge`, NPAPI hosting, the IME path (`luna-webkit-api`). Jihad's daemon is the
   engine host to start from (headless Goanna, many pages per process, offscreen rendering, the
   Luna bus, NPAPI, input bridging).
2. **Patch the frameworks in place** at `/usr/palm/frameworks` (Enyo 1 and Mojo:
   `-webkit-border-image` → `border-image`, the Palm-only CSS, the prefixed JS under
   `layout.css.prefixes.webkit`), once for all apps.
3. **Support both webOS CE 3.1.0 and stock 3.0.5.** On CE the open LunaSysMgr allows the chart's
   60 fps rows (the engine's compositor presenting into LunaSysMgr); on stock the shared-buffer
   readback path stays, at T-502's numbers.
4. First proof when the project starts: an Enyo 1 app loaded in the Jihad daemon with a
   `PalmSystem` shim, judged by eye (the frame dumps, not screenshots).

Memory: `global-jihad-project.md`. The workspace `CLAUDE.md` carries the pointer and the corrected
target line (both webOS 3.0.5 and CE 3.1.0).

## 2026-10-24 (evening) — the owner's two new directives; the emulated finger, where it stands (READ FIRST)

**Later the same evening — the round's part 1 run and its findings fixed (READ THIS FIRST):** T-523 and T-524 are
DONE (T-514 with them). `tools/device/finger/` has tap, dtap, hold, drag, drag2, flick, pinch, sleep, raw; the kit's
R2 is all ticked. The round's scrolling and gesture items are run in all three interfaces (`docs/FINGER-TESTS.md`
carries each result; `docs/rounds/finger-2026-10-24/zoom-grey.txt` the data). **The owner's "areas turn grey" was
found and fixed:** (1) the smart zoom fitted a 104 px cell (6.2x) and the engine's default maximum-scale of 10 passed
the adapter — the block rule (a third of the card) and a 4.0 default (daemon); (2) at rest the zoomed frame lost its
right third from 2.0 and everything from 2.98: the ROOT'S DISPLAYPORT BASE AND CLIP ARE IN LAYOUT DEVICE PX once the
pres shell has a resolution (Gecko divides them by it) — `jihad_layout_set_displayport2` (the card's shift, base and
margins times the resolution) and the root's clip no longer divided (engine c79d16f8 = patch 0059, UXP_PIN moved;
memory `compositor-scroll-traps` 6 and 7); (3) the second double tap and any tap on a zoomed page found nothing — the
GPU path keeps the position in the engine's scroll, not the render pan (`docOffset`/`gpuAlignForHitTest`), and a
zoomed-in page with no block answers the page's width (back to the fit); (4) the resolution is set BEFORE the
displayport and the displayport re-set at every zoom (a pinch from 2.33 to 2.8 clipped again; the zoom-out painted
160 tiles). **Still open, dispositioned to T-527:** the flick's coast gap (the engine paints only at the coast's end:
~0.5 s of blank lower card and no fixed header), the first frame at a new zoom (1.6-2.9 s on desktop Wikipedia, R6),
the clip above ~2.9 on a page laid out 1116x1374 (closed for now by a GPU maximum-scale of 4000 / the declared
viewport's larger side; a page with no viewport tag keeps the adapter's 4.0 and will clip above ~3.3).
**Device state:** DEV COPIES on all three variants — libxul 4d753b8f (c79d16f8 stripped) and daemon 7c29035a in each
`deviceroot/hl`, the originals kept as `libxul.so.orig` / `jihad-browserserver.orig` (c66cd841 / 350c83ed); the
upstart jobs run them. A packaged build and install (the three ipks) is the clean way to ship them: `build-all-device.sh`
after the commit. **T-525 begun (part 2):** the chrome buttons of all three shells act, Mojo's back and forward included (the hour spent on a "dead back" was a tap on the disabled forward box: Mojo's bar boxes are back 197-247, forward 251-301, home 305-355 … in card px, not where the icons seem to be). TWO TRAPS from it: (1) the card's WebKit serves an app's JS from its memory cache across relaunches — a shell file edited on the device runs only after `build/webos-oe/luna-restart.sh` (every probe before that restart was a no-op); (2) palm-log and syslog carry none of a Mojo card's Mojo.Log lines on this build — report through the daemon instead (navigate to a URL with the data in its query; the load line carries it). Enyo's long-press on text and on a field done. The Mojo daemon runs a0e151e1 (7c29035a plus the pageBackward/pageForward log lines; Enyo and Mochi still 7c29035a); the Mojo shell file on the device is its original. **Later still — T-525 DONE, T-526 mostly:** every item of parts 2 and 3 that a finger can reach is run (see `docs/FINGER-TESTS.md`, each item dated 2026-10-24). Found open: Mojo's link hold shows the text commands only (the shell's hold menu never sees the hit test's link facts — Mochi claims them in JihadWebView.eventFired; mojo-ui R6); the about:addons extension ROW's buttons do not act on a tap at the 0.78 fit zoom (the hit test answers the `richlistitem`; the detail view's buttons do act — Palefill was disabled that way and put back by editing `userDisabled`/`active` in extensions.json with the daemon stopped); Mochi's Paste from the field sheet lands nothing (the card's clipboard write or read; JihadBrowser.js pagePaste). The device: Palefill enabled, the Default look on, the Mojo prefs restored, fonts restored. **And later:** the T-526 leftovers done (the caret and the Backspace repeat, Copy from the address and Paste into a field, two downloads — the CE bench has no handler app to open either type); two of the three opens FIXED and verified by finger — Mojo's link/image hold gets Open in New Card / Share Link / Copy URL (the shell claims those holds through the widget assistant's `eventFired`), Mochi's Paste lands (pagePaste clicks the held point before inserting: the sheet had blurred the field). Still open: the about:addons extension ROW's buttons at the fit zoom (the detail view's buttons act). Shell dev copies on the device match the repo (originals kept as .orig beside them). **T-527 begun — the sensor bench done:** desktop Wikipedia's scripted pan 21-22 fps with the sensors, 25-27.5 with `sensord` and `atlas-sensord` stopped (the long page 30-35 / 33-37); sensord alone is ~30% of a core at idle. The sensors are running again. **Next:** the compositor-thread floor in the engine (the frames composed off the main thread so the engine's paints and script do not gate them; T-505/T-511/T-517's items are its parts), measured by the same pan-run against these numbers; and the one finger open left, the about:addons extension row's buttons at the fit zoom. Not pushed: this session's commits — Jihad-Browser de4fb43c..64413a73 (12 commits) and the engine fork 7a872e8f..c79d16f8 (patch 0059; the main repo's gitlink points at it). `bash .cavekit/handoff-2026-10-24/push.sh` pushes both, reads them back and checks the pins (the owner's, GitHub key passphrase).

**The owner:** (1) "proceed with everything blocked by the finger tests, not by assuming it works, but by driving your
own tests by emulating a finger and addressing any issues that crop up. if youre not sure how best to emulate a finger
on the touchpad consult codex"; (3b) the GPU floor levers are authorized: the bench without `sensord`/`atlas-sensord`
and compositor-thread work in the engine (memory `gpu-floor-levers-authorized`; T-504/505/511/517 re-opened in the
registry). Also: "remember to use cavekit 3.1 and clear screenshots when you no longer need them" (memory
`clear-screenshots-when-done`; the 807 untracked dumps were deleted). **Not yet done:** a kit and Tier 89 tasks for the
finger emulation and the re-opened GPU work — write them first (Cavekit 3.1: sketch, map, make).

**The input path, established from the device and the binaries (context in `.cavekit/handoff-2026-10-24/finger-context/`,
git-ignored):** the Cypress panel is read by `hidd` (`/dev/ctp_uart`, plugin `libhidtouchpanel.so`, config
`/etc/hidd/HidTouchpanel_evt1.xml`), not evdev; `/proc/bus/input/devices` has only keys. hidd SENDS each scan as a
datagram of 16-byte Linux `input_event`s to the AF_UNIX **datagram** socket that the CONSUMER binds at
`/var/run/hidd/TouchpanelEventSocket` — LunaSysMgr (pid of the window server) holds it (fd 18; `KeypadEventSocket` fd 19),
through `libhal.so` + `/lib/hal/modules/halTouchpanelMain.module` (`HidInitPluginTransport` binds; `hal_hidd_read_events`
recv()s into a 4 KB buffer), then the Qt QPA plugin `/usr/plugins/platforms/libqpalm.so` (`QPAHiddTpHandler::readHiddData`
→ `qt_translateRawTouchEvent`). The module's parser (`touchpanel_get_event`, decompiled): per finger `EV_FINGERID` (type 7)
opens a touch item (id = value*1000 + code, the timestamp), `EV_ABS` code 0/1 = x/y, `EV_KEY BTN_TOUCH` 1 = down, 0 = up
(QPA: a "down" adds a finger, so send it on the first scan only; the up on the last), `EV_GESTURE` (type 6) = gesture code
then x and y velocity in two events (FLICK = 6), one `EV_SYN` ends the scan, 20 fingers at most; an empty scan is dropped.
Public source of the later Nyx version of the QPA plugin: github.com/openwebos/qt
`src/plugins/platforms/palm/hiddtp_qpa.cpp` (states, addNewTouch/updateOldTouch, mouse emulation, FlickGesture).

**The tool:** `tools/device/finger/finger.c` (builds with the toolchain line in its header; tap, dtap, hold, drag, flick,
pinch, raw) sends such datagrams to the socket as root, taking PORTRAIT screenshot coordinates (raw x = portrait y,
raw y = 767 − portrait x: a tap at raw (10, 737) opened the app menu at portrait (30, 10)). **Proven 14:06:** libhal
logs "Unexpected code 0x5" for a bad datagram (the parser runs on ours); a 500 px drag on `jihad-long-static.html`
scrolled the top from paragraph 1 to 8, a 160 ms flick coasted to 33, a pinch out zoomed in, a long press opened the
context menu. NB `jihad-touch.html` is no oracle: a page's DOM touch listeners are not fed by the card's pan path.
Still to tune: the double tap (two 70 ms taps 120 ms apart changed nothing on a zoomed page) and the tap on the address
field (no keyboard). The kit `cavekit-finger-emulation.md` and Tier 89 (T-523…T-527) carry the work. The old next
experiments, now moot: (a) confirm the datagram is read at all — `strace -p <LunaSysMgr> -e recv` is not on the device,
so instead send `finger raw` events and watch `/var/log/messages` for libhal's `_hal_log__` lines ("Unexpected code",
"No touch items") with HAL_LOG_LEVEL raised, or use `hidd`'s own `CreateFingerUpEvent` Luna method to see a real
datagram's effect; (b) the id scheme (try code 0 / value 1 as hidd may), the `BTN_TOUCH` code (0x14a) and whether hidd
sends `EV_KEY` at all for touch on this build; (c) whether the QPA delivers to the card only after a HAL "touchpanel
mode" or when the card has focus; (d) the coordinate frame (landscape 1024x768 vs portrait). Codex: out of usage until
18:30 after 468k tokens of grepping, no report; the inspector is the backup.

## 2026-10-24 delta — Tier 88: the kits' open criteria closed on the current build; the commits chained for the owner's passphrase (READ FIRST)

**The owner:** "Continue the implementation of Jihad Browser using Cavekit version 3.1." The registry had no
unblocked task (the finger rounds, the tabs tier and T-504/505/511/517 are the owner's), so Tier 88 was added
to the site (T-518…T-521): the acceptance criteria no task covered any more, and the engine's staged review
fixes (0058) not yet in the patch queue.

**T-518 (records) done.** Seven criteria met by earlier tasks ticked with their evidence (gpu R11's review,
content-blocking R7's three, modern-web R1's two and T-500's two); T-501's criterion kept `[~]` until T-520's
MSE probe; the template placeholders never filled (`T500_KIT`, `T501_KIT`, `T501_DEVICE_RESULT`,
`DEVICE_STATE_EVENING`, `TRACE_PICKUP`) filled; a stray JSON blob reverted out of `.cavekit/config` (the owner's
one-line file); the README's "Where it stands" regenerated.

**T-519 (the GPU sweep) done.** On engine 0058, the 10 s scripted pan on six pages under `JIHAD_GPU_VERIFY=1`
(`docs/rounds/t519-2026-10-24/`): 0 rows structurally wrong on all; 0 rows differ at all on the long page, the
new `jihad-fixed-both.html` (an opaque header AND footer, nothing else), sticky-mix, scroll-js and Hacker News;
Wikipedia 3 of 70 frames resampling-class. The animation page differs only inside its animated block (the
verify reference samples the CSS animation at its own compose time): R2's first criterion is `[~]` with the
engine fix named (pin the reference's animation time to its job's). R4's four ticked on the compositor's lines
(two bands with both kept on the fixed-both page, Wikipedia's panels as 2 columns in 3-5 chunks; sticky-mix takes
the whole path as in T-503's rounds). R3's table, the GPU path against `NOGPU=1` on every page, is in
`docs/RENDERING-OPTIONS.md`: ahead everywhere (long 30-33 against 11-12.5 fps, Wikipedia 18.5-26 against
2.7-2.9, Hacker News 37-48 against 6-10).

**T-520 (the blocker's device criteria) done, one criterion failed and fixed by T-522.** On the Enyo card through the
inject channel (`url`, `setpref`, `gettext`, `rect`, `click`, `jsurl`), with `123count.com`, `ad.doubleclick.net`, a neutral
`jihad.test` and the 200-image page's twenty hosts mapped to the host PC in `/etc/hosts` for the run: a listed host
opens top-level; a `$third-party` host loads first-party and is refused third-party; an `@@||host^` exception (a list
copy with one added) loads the host; the master switch and hiding flip live; the settings line shows the versions and
a counter that rose 10 → 16; "Update the lists now" moved the lists three times (1105 → 1606 → 1630 → 1635/1641);
`shouldLoad` on 200 refused images is within 10 % of blocking off; MSE reports H.264 and AAC (T-501's criterion
ticked). **R4's sheet cost was not met**: hiding on cost the MDN page about 4 s a load (its copy: off 2.6-3.0 s, on
3.7-7.5; the real page off 11 s, on 15) and 8-12 s on a daemon's first load. Four variants of the cached sheet named
the cause: the 268 `@-moz-document regexp` groups (one per site-excepted generic selector, each repeating the 160
generic-hide hosts' 2.4 KB regexp), not the 13,079 keyed selectors — the same sheet under `url-prefix` scopes or with
no wrapper costs nothing. T-522 moves those selectors into the per-document sheet (`excSel`, compiler 11) and leaves
the global sheet one regexp group: the MDN copy now loads in 2.9-3.1 s with hiding on against 2.6-2.8 off (+8.7 %), a daemon's first load 3.7 s where it took 8-12; `jihad-hiding.html` still hidden, the site-excepted `#ad-carousel` hidden on `jihad.test` (`t522-run2-host.log`). The bench's three variants run the new component (a dev copy, md5 e8f6c123; the packages carry it at the next bundle build). On a `$generichide` host (jetzt.de mapped to the PC) generic hiding does not apply and the excepted selectors are not added (`t522-generichide-host.log`; the compile line: 268 excepted selectors per document, 154 generic-hide sites and 4 address prefixes). NB two earlier runs of that check timed out and looked like a 25 s stall: the host was not mapped (a `sed` on the prep script had rewritten its own guard) and the load's error alert waited 60 s for a card to answer. Ledger: `docs/MODERN-WEB.md`, T-520; logs `docs/rounds/t520-2026-10-24/`.
New pages: `jihad-tp-first.html`, `jihad-tp-third.html`, `jihad-exception.html`, `jihad-hiding.html`, `jihad-excsel.html`,
`jihad-mse-probe.html`, `jihad-plain-ok.png`.

**Traps found today.** (1) A verify run's frame rate is not the path's: `JIHAD_GPU_VERIFY=1` composes and reads
a whole reference per job (5-6 fps on a 30 fps page); the first sweep had to be run again without it
(memory `verify-runs-are-not-measurements`). (2) The device's `/etc/hosts` ends without a newline: a line
appended with `>>` glues onto `palm-webos`, the mapping fails, and the card then sits under a "connection was
refused" dialog while the daemon takes no inject for a while — append with a leading `\n` and check with
`ping -c 1 <name>`. (3) EasyList carries `@@://192.168.$generichide`: a test page served from the host PC's
address gets no generic hiding; serve it under a host name mapped in `/etc/hosts` (`jihad.test`). (4) The inject
channel's `gettext` passes its argument straight to `querySelector` (no `sel:` prefix, unlike `rect`), and a
`jsurl javascript:` whose completion value is a string replaces the document — wrap it in `void(...)`. (5) The
lists load on the first card connect (the cache in about a second): a page launched with the card has its
requests judged before they are ready, so a blocking test waits for the `blocker: cache:`/`compiled:` line first.
(6) Mojo launched with `target: about:preferences#pane=privacy` shows an empty card; the pane is reached through
its test hook's `U` step.

**Commits — MADE (T-521, 2026-10-24 13:49) AND PUSHED by the owner (13:5x, `.cavekit/handoff-2026-10-24/push.sh`: engine fork `706d4782..7a872e8f`, Jihad-Browser `62df604c..de4fb43c`, both read back equal, the pin public).** The owner's rule, 2026-10-24: the Gitea token and the
`id_ed25519_gitea_signing` key serve `git.trueog.net` / `git.true-og.net` only (memory `gitea-credentials-are-gitea-only`);
the two GitHub repos commit unsigned (`commit.gpgsign false` per repo in Jihad-Browser and `third_party/uxp`), so
no prompt. Engine fork `7a872e8f` (patch 0058: the review fixes), Jihad-Browser `506c4114` (A: T-487), `3f4e1a27` (B:
T-513/T-499 with the fold, the pins and the gitlink; `check-engine-pins.sh` all agree, the 58 patches reproduce the
pin), `318869f2` (C: Tier 88) and the records commit after it. The chain and its message files stay in
`.cavekit/handoff-2026-10-24/` (git-ignored). The push was the owner's (the engine fork's remote is `fork`; GitHub access is the `id_optiplex_new` key, a passphrase per connection): `push.sh` pushes the engine first, then main, reads both back, runs `check-pins.sh`.
The 807 untracked jpg/log/png dumps of the 2026-10-19 rounds (42 MB, none cited by a doc) were deleted on 2026-10-24 at the owner's word: clear screenshots when they are no longer needed.

## 2026-10-19 (night) delta — T-513, T-487, T-499 done; the verify mode's reference; the layout viewport per document (READ FIRST)

**2026-10-24 (the session resumed after the weekly limit).** Codex reviewed the whole of it and found nine
more problems, all fixed (`context/impl/impl-review-findings-gpu-2.md`, C1-C9): late writes on the
engine's opt-in read paths, the previously shown buffer reclaimable by the 2 s valve, the verify pool,
the document key (now the inner window's id), the adapter's window bound, the inject threads, and the
spike scripts' daemon waits and restore order. The bench runs that build: engine (patch 0058), daemon and
the three rebuilt adapters (installed in /usr/lib/jihad/<v>, LunaSysMgr restarted). Re-run on it: two
cards 22 frames and six flicks 40 frames, 0 rows wrong; the probe page's rotation and navigation; the
three default checks. **Commits:** the owner turned on SSH commit signing on 2026-10-21 (memory
`commits-are-ssh-signed`); the engine commit waited on the passphrase prompt, and the two Jihad-Browser
commits (T-487; T-513/T-499) are written up in the session, not yet made. The registry has no unblocked
task left: the finger rounds, the tabs tier (set aside by the owner) and T-504/505/511/517 (the circuit
breaker) are the owner's.

**T-513 (the review of Tier 87) done.** Codex was out of credits; two inspectors reviewed the tier and found
12 problems, all fixed (`context/impl/impl-review-findings-gpu-2.md`; the worst: every card close left ~16 MB of
GPU memory in the share group, and the express lane could compose into the buffer the card shows). Twenty card
open/close cycles add 76 KB (R10). R2's new criterion, a fast flick and two cards panned in turn under
`JIHAD_GPU_VERIFY=1`, passes: 40 flick frames with shifts to 1200 rows and 26 two-card frames, 0 rows wrong.
It first "failed" by 744 rows: the verify mode composed its reference at the job's poll, after the flick had
moved the displayport on, so the reference lacked tiles the published frame had (the frame was right, by eye).
The reference is now posted right behind its job on the same layer tree (`gpuJobStart`, a static pool of four
buffers never freed, because a given-up compose can still be read into its buffer late). New round tools:
`tools/device/t502/two-cards.sh`, the inject channel's `target focus|last` (`JihadBrowserServer.cpp`), the Enyo
hook's `A` step (bring the other card to the front). Notes `docs/rounds/t513-2026-10-19/`.

**T-487 (the blocklist updater) done** on the Enyo card: a forced update (EasyList and EasyPrivacy 2026-10-19 over
HTTPS; a rule only in the new EasyPrivacy blocked the next page's image), the network off (two "no download"
lines, nothing changed, no dialog), no repeat within 4 days or within a day of a failure across restarts. Two
bugs found and fixed in `jihadContentBlocker.js`: EasyPrivacy was refused as "truncated" on every update (held
against both lists' rule count), and a due update skipped at the timer's one early look waited six hours.

**T-499 (the layout viewport) done.** Three changes on top of daemon 17's rule: the card's own size resent (every
NPAPI SetWindow) keeps a followed layout viewport (it used to shrink to 768 and grow back each time); the layout
viewport is the document's (`GoannaRenderPage::DocumentKey`/`ResetLayoutSize`: a new document starts at the
card's size, so a device-width page after a followed one is no longer laid out at 1000 and shown at 0.77); and
the rule waits for `readyState` past "loading" and never follows about:blank (mid-parse the engine lays a page
out at 980 as if it had no meta viewport, and a device-width page was followed to 980). Probe page with rotation
and navigation, BBC, YouTube mobile, Wikipedia, DuckDuckGo: `docs/rounds/t499-2026-10-19/`. NB DuckDuckGo now
declares `width=device-width` and its menu no longer shows, so the rule (no meta viewport only) does not apply
to it any more; the probe page keeps the original geometry.

**Traps found tonight.** (1) The bench's Wi-Fi flapped for an hour (link up for seconds, gateway unreachable,
"could not be found" and one `SSL_ERROR_RX_MALFORMED_SERVER_HELLO` from DuckDuckGo that a retry cleared); a
`palm://com.palm.wifi/setstate` off/on brought it back for a while, and it settled by itself later. Check
`ping -c 1 duckduckgo.com` on the device before reading a failed load as ours. A daemon that met the outage kept
failing loads to the LAN ("connection refused") until the network was steady again. (2) A spike's
`jihad.viewport.followOverflow=false` stayed in the shipped Enyo profile for hours: a daemon still exiting saves
its prefs over an edit. `gpu-spike-start.sh`, `gpu-spike-collect-stop.sh` and `set-prefs.sh` now wait for the
daemon to be gone before they touch `prefs.js`. (3) Several cards from successive test launches each connect to
the daemon (a launch into the running app opens a new card); a "page init FAILED ... shm resolve" line is a card
already gone. Close the cards between runs.

## 2026-10-19 (evening) delta — T-505/T-511/T-517 blocked by the circuit breaker; T-512 diagnostics gated (READ FIRST)

**Where the frame floor stands.** Desktop Wikipedia's 10 s scripted pan runs 23.2-24.1 fps since engine
0055/0056 (rounds 346-357). Tried tonight and not kept, each measured on the same pan: deferring the
page's tile uploads to the composite (a new texture's conversion then costs 11 ms inside the render
pass), a texture pool topped up ahead (6-7 ms a conversion), partial tile uploads as sub-images
(`gpu-bench.c` section 18: cheap with the GPU idle, but only 38 of a pan's 167 uploads are small), the
express lane on its own slot for every frame. `JIHAD_GPU_READ_FINISH=1` shows why the copy cuts do not
pay: after the 12 ms base copy the GPU still has 5.3 ms of the band frame's compose left (~17 ms of GPU
work a frame), with both cores full (the bench's `sensord` at ~18 % of a core, from the Atlas browser's
`atlas-sensord`). The 480 px WebGL cube reaches the card at 19.5-20.9 fps against the page's own
23.8-25.1 (round 358). T-505, T-511 and T-517 are `blocked` in the registry under Cavekit's circuit
breaker: the owner's finger round (T-514) judges smoothness, and running the bench without
`atlas-sensord` is the owner's call. Notes: `docs/rounds/t502-2026-10-17/floor-r314-320.txt` (rounds
349-357), `webgl-r281-313.txt` (358), `bench18-r355.txt`; `context/impl/dead-ends.md`.

**T-512 (part, `e8b0e425`).** The `[jihad-gpu]` lines print only with `JIHAD_GPU_LOG=1` or a device
round's `JIHAD_GPU_TIMELINE`/`VERIFY`/`DUMP_EVERY` (engine 0057, `mozilla/gfx/JihadGpuLog.h`; the daemon's
`render/goanna/JihadGpuLog.h`): a shipped daemon logs three engine startup lines. `default-check.sh` counts
the express lane's frames from the `page closed: <n> frames` line; all three interfaces pass on the bench
(bundle 49ad5cf7). `gpu-spike-start.sh` sets `JIHAD_GPU_LOG=1` (`NOLOG=1` not). The bench's three variants
run this build.

**Registry.** T-517 added; T-512 no longer waits on T-504/T-505/T-511/T-517 (T-504 -> T-514 -> T-513 ->
T-512 was a cycle). Next: T-513, Tier 87's review (Codex by direct invocation, the inspector as backup).
The session that ran rounds 349-354 died mid-build: a build started from a session must be detached
(`setsid nohup`), or it dies with the session.

## 2026-10-19 delta — T-515: frames made off the main thread, the express lane; T-516, T-510, T-511 (READ FIRST)

**What it is.** With `JIHAD_GPU_EXPRESS=1` (and the GPU path) each card's YAP commands are read on a
thread of the proxy's own (`render/browserserver/Yap/YapProxy.cpp`), and the daemon's frame
bookkeeping is under one frame lock (`render/goanna/JihadFrameLock.h` explains it) that the main
thread lets go of only in the engine's pump and in a frame's layer paint. An express thread
(`BrowserPageGoanna.cpp`, `expressStep`) composes the layer tree as it stands at the card's newest
scroll and zoom and publishes the frame itself: while the card moves it makes every frame (the main
lane paints layers and moves the anchor), and otherwise it fills the main thread's stalls. Engine
patch 0048 splits the frame's paint from its compose post and lets any thread post and poll jobs.
It is ON by default since T-510 (same day), with the GPU path: the three upstart jobs set
`JIHAD_GPU_COMPOSITOR=1 JIHAD_GPU_EXPRESS=1`; `JIHAD_GPU_COMPOSITOR=0` or `user_pref("jihad.gpu.enabled",
false)` in a variant's profile give the software path (the daemon reads them before the engine starts and
logs "rendering: ..."). The bench's /etc/event.d jobs were replaced in a remount window (/ back to ro).

**What it did (rounds 240-267, `docs/rounds/t502-2026-10-17/express-r240-267.txt`).** A zoom while the
page's script holds the main thread: Wikipedia after its load 10/10 within 160-225 ms (without the
lane: median 681 ms, max 1116, three zooms never framed); a page holding the main thread 600 ms in
every 700 (`render/goanna/test/jihad-script-hold.html`) within 45-91 ms (without: median 450). That
page pans at 26-32 fps (2.4-3.8). Desktop Wikipedia pans at 19-21 fps: the page's 2.2 s restyle now
runs under frames every ~45 ms, and the floor is the compositor thread's ~45 ms a Wikipedia frame
whichever lane asks (T-517). Card double taps 40-449 ms: the slow ones are GPU draws of the other
resolution's tiles after a zoom in (T-516). No regression with it off; clean daemon exit.

**Traps.** (1) Two locks: the frame lock (the main thread, the express thread, the inject threads)
and the want lock (the reader thread's scroll and zoom; never held while waiting). The reader
thread never waits for the frame lock: a proxy's teardown joins it (a returned buffer tries for 4 ms
and is left to the main loop). (2) With the lane on, the inject channel's `pan` and `zoom` go
through the card's own path on a thread (`zoom <z> <x> <y> [delay ms]`), so they arrive while the
main thread is busy; without it they are applied in the tick as before. The inject file itself is
read only when the main thread is free. (3) "Served" is by command, not by position: commands are
tagged in arrival order by the proxy and the main lane serves the card once its frame took up the
newest (the engine clamps and rounds positions; comparing them made the lane redraw every frame).
(4) A main-lane frame posted before an express publish is dropped at its poll; one at a zoom the
main loop has not taken up yet is held (300 ms at most). (5) `JIHAD_GPU_ZOOM_DUMP=<path>` frames are
3-4 MB each on a 40 MB `/tmp`: use `JIHAD_GPU_ZOOM_DUMP=log` for timelines, delete files at once.
(6) A scripted pan's screenshot is meaningless (the card's own scroll does not move); judge by the
dumps, `JIHAD_GPU_TIMELINE=1` and the paint pipeline lines.

**T-516 (same day, partial).** The double taps' slow first frames were not draws: the first readback
after a pause ran 100-390 ms and the CPU swizzle 20-122 ms (the clocks after two idle seconds and
the engine's other threads). The YAP reader now raises the clock hold on a scroll, zoom or smart-zoom
request, and the Compositor and JihadExpress threads run at nice -6: double taps over 250 ms went from
4-8 of 12 to 0-2 (median 158). Left: the GPU's ramp after a pause. A warm-up frame was tried and
made it worse (dropped). Engine 0050: the timeline logs every job, compose and read.

**T-510 done (same day).** The default as above; `tools/device/t502/default-check.sh <variant>` loads,
pans, pinches and closes Wikipedia through each card's self-test hook on its own upstart daemon: all
three pass. NB: the shipped jobs carry no `JIHAD_INJECT`; inject-driven rounds use the hand-started
daemons (`gpu-spike-start.sh`), as before.

**T-511 (same day, partial): WebGL.** On the GPU path `getContext("webgl")` gets a context on the
Adreno through EGL (engine 0051); on the software path it stays null. Every pbuffer context on the GPU
path is in one share group, so a canvas's texture is composited with no copy (the driver has no
EGLImage from a GL texture). The spinning cube (`render/goanna/test/jihad-webgl-cube.html`) renders
right; 8 card open/close cycles and a context lose/restore without a fault (the fault before: a
destroyed context left current; released first now). Card rates: 240 px cube 26.5-27 fps, 360 px
21.5-23, 480 px 18-20.5 (the page draws 21.5-25): the compositor job's ~45 ms for a 482-row change,
the same floor as Wikipedia's frames, carried by T-517. Engine 0052 reads a narrow change's bands as
one column (44 against 47 ms a job); an adapter that invalidates only the changed strip measured +12 %
(not kept: it needs a side channel for the rect). Rounds in `docs/rounds/t502-2026-10-17/webgl-r281-313.txt`.

**Traps found 2026-10-19.** (1) `/var` full: the adapter's paint log gate (`/var/palm/jihad/enyo/adapterlog`,
set by `adapter-install.sh` on 10-17) grew the log to 46.7 MB of /var's 62; the daemon's main thread then
sat in `log_wait_commit` after its first frame (no frames, cards that would not close, loads aborted
0x804b0002) and a day of WebGL teardown runs looked like engine faults. The script no longer sets the
gate; when a run turns strange, `df -h /var` and the D-state threads' `wchan` first. (2) The adapter
(PDK gcc 4.3.3) had not built since T-515 (`<mutex>`/`<atomic>` in `YapProxy.h`): pthread locks now; the
three adapters build, the bench's Enyo one is the new build. (3) On the bench `sensord` runs at 15-23 %
CPU and `atlas-sensord` at ~3 % all the time (sensord has 389 CPU minutes): not ours, and it is a core's
fifth during every measurement.

**T-517 (same day, investigated).** A Wikipedia pan frame with the sticky panels' column chunks takes
40-46 ms on the compositor thread, one without 18-27: the two pinned panels are see-through sticky layers,
composed again every frame in 3-4 chunks of the 256-row band surface, and every pass costs ~5 ms of CPU
whatever its area. Both cores are full during a pan (the daemon ~1.5 cores, WebAppMgr ~18 %, and sensord
~18 % beside the Atlas browser's `atlas-sensord`, all the time). Tried and reverted: the shift on a worker,
the whole path, one main-thread notification a frame. `pan-run.sh` takes `PAN_TOP=1` for the CPU split.
Notes in `docs/rounds/t502-2026-10-17/floor-r314-320.txt`.

**T-517 (later the same day, partial).** Two cuts kept. The frame rows' cache clean is skipped where
the segments are mapped write-through (they are; the clean was 4 ms a frame). Frame windows: a frame may
start further into its shared segment (`render/browserserver/Src/JihadFrameWindow.h`: a 64-byte trailer
at the segment's end, written by the daemon before each publish, read by the adapter's
`BrowserOffscreen::rasterBuffer()`; only an adapter that marked the segment it made gets non-zero windows),
so a scrolled slot's frame is its window moved, not 3.7 MB of rows moved (engine 0053). Both lanes now keep
every slot's stale rows (the express lane used to drop them on each publish), and the express lane builds
on its own slot's frame when that measures faster. Long test page 30.5-36.4 fps (median gap 29 -> 26 ms);
Wikipedia still 19-21. **The bench's three adapters are the window-aware build** (installed by hand from
`build/webos-oe/adapter-deps/build-pdk/<v>/`); a package built before this commit would carry old ones and
the daemon would keep windows at 0 (correct, just slower).

**Review and T-504 (later the same day).** An inspector review of the day's work found six real problems
(the express lane's own-slot base racing unpolled main-lane jobs, the kept rects' copy, older bases with
sticky layers, failed jobs' records, a recentring before a failed post, WebGL after a GPU give-up); all
fixed (`context/impl/impl-review-findings-gpu.md`, engine 0054). Scripted double taps: 2 of 18 first
frames over 250 ms (286, 367), with the second core unplugged after a pause; the scripted tap skips the
touch path's governor ramp, so a finger's tap goes to the finger round. T-504's own token budget (the
plugin's 45k default) is spent: raise it in `.cavekit/token-ledger.json` to continue it. Failed approaches
of the day: `context/impl/dead-ends.md`.

**T-517 (evening).** Engine 0055 composes a band frame's regions (its bands and the sticky columns'
chunks) in one composite instead of one each, and 0056 skips tiles that miss the clip before locking them:
chunked Wikipedia frames compose in 10.6 ms where they took 22.5. The 10 s scripted pan runs 23.4-23.9 fps
(20.0-21.7 before); the remaining long gaps are the page's own work (tile uploads queued ahead of the
express frame on the compositor thread, its script on the other core). The README now has a "Where it
stands" section generated by `tools/readme-progress.py` from the kits: run it after a criterion is met.
T-504 is marked blocked in the registry (`.cavekit/task-status.json`) on the finger round.

**Next.** T-517: Wikipedia's sticky columns. A rule that kept see-through sticky columns once proven still
was tried and reverted (round 333: the contents panel's highlight follows the reading position, so it does
change during a pan); what is left is fewer passes (one tree traversal for all of a frame's regions). Then T-512 (fit to
keep), T-513 (review). Knobs in `tools/device/t502/README.md` (the T-515, T-516, T-511 and T-517 paragraphs).

## 2026-10-18 (night) delta — T-385…T-387 done, T-504 (zoom) mostly done (READ FIRST)

**Engine-update readiness (T-385…T-387) done** and reviewed (the inspector, Tier 74: APPROVE, the
four P2s and the cheap P3s fixed, `context/impl/impl-review-findings-6.md`). The desktop engine
builds on RB_20261006 again (patch 0043); desktop tests `personas` 7/7, `media` 18/18, `prefsui`
38/38.

**T-504, zoom (rounds 162-186), the owner's "areas turn grey and zoom rendering lags":**
- *The zoom hold:* on a zoom only the compositor's world transform and scroll change; the
  displayport and the engine's anchor follow once a frame at the new zoom is out (or 800 ms). On a
  settled page the first frame at a new zoom now comes 52-217 ms after the command (was 1.2-1.75 s).
- *A grey strip, zoomed in:* the pan's settle moved the engine to the card's x, which a page whose
  scroll port does not scroll sideways cannot reach (desktop Wikipedia at 1.23: x 245 CSS px), and
  published frames at x 0 under a card at x 302. The compositor keeps the scroll the engine cannot
  reach now.
- *Tile uploads (engine 0044):* the "upload waits behind the GPU" of T-502 was wrong. On the Adreno
  220 the **first write into a texture after its first draw** costs ~18 ms per 256x256 (proportional
  to the size: 75 ms at 512, 285 at 1024), any format, once per texture; later writes cost ~1 ms,
  under a queued draw too; a regenerated name pays again; the conversion survives a re-specification
  at another size; a clear of the texture through a framebuffer triggers it without a wait
  (`tools/device/gpu-bench.c` sections 11-16, `BENCH_ONLY=<n>`). New tile textures are converted at
  1x1 (1-2 ms) and textures are pooled (`JIHAD_GPU_TEX_POOL`, 48). Wikipedia: 55-59 slow re-uploads
  a zoom run (24-37 ms average) down to one. The pans gained too: desktop Wikipedia at its fit zoom
  19-25 fps (T-503 left it at 16.4-18.5).
- *Sharp zoomed text:* above a zoom of 1.1 the pres shell's resolution follows the zoom (no
  scale-to: layout adds a post-scale, the geometry is unchanged), so tiles land one to one on the
  card; below 1.1 the 1x paint stays (a fit zoom's resolution paints fewer pixels but resamples
  every image and glyph on the CPU: the Wikipedia pan lost 3-5 fps). `JIHAD_GPU_NO_RES=1`.
- *DEAA (engine 0045):* Gecko's edge antialiasing is on for any non-integer transform, a scale too;
  its shader variants compile on first use, ~150 ms each on this GPU, and the first frame of a zoom
  compiled two. No DEAA for axis-aligned scales on the GPU path (`JIHAD_GPU_DEAA=1` restores it).
- *Tools:* the Enyo test hook's `t<x>,<y>` is a double tap (the adapter's smartZoom);
  `JIHAD_GPU_ZOOM_DUMP=<path>` keeps the frame the card shows after each zoom; `JIHAD_GPU_PHASES`
  logs slow composites and draws.

**Open on T-504 (with T-505):** while the page's own script holds the main thread (desktop Wikipedia
runs its modules for ~10 s after the load event) the daemon cannot read the zoom command at all:
the first frame then takes 0.8-1.7 s, and pans stall the same way. The fix is frames made off the
main thread (the compositor thread composes with the card's scroll and zoom and publishes itself);
that is T-505's work, and it closes T-504's criterion.

**T-505 (rounds 187-201):** the per-page table (the GPU path ahead everywhere: the test page 30-35
fps against 2.7-3.2, sticky-mix 17-19 against 2.4-3.1, desktop Wikipedia 19-26 against 2.7-2.9,
Hacker News 31-37 against 6.4-10.4). The 56 ms+ gaps are the page's own restyle on the first scroll
(Wikipedia's sticky header, 955 ms), the paints of the displayport moves (58-121 ms) and short page
events; the compositor itself floors at ~30 ms a frame on Wikipedia (one composite per band and per
sticky column chunk, the row shift, the read). **Overscan** (engine 0046): the GPU frame carries 0.15
of the card's height above and below the view, so a pan or a busy main thread shows the page past
the view's edges, not grey; verify is clean on four pages; it costs ~10% of the rate. The frame is
capped at 1400 rows (the compositor's first pbuffer: a taller frame read its top rows from the
bottom ones, flipped). Wikipedia at 24 fps needs **frames made off the main thread** (T-515, new):
an IPC thread reads the card's scroll and zoom, the compositor composes and the frame is published
while the main thread runs the page's script or a paint (the YAP proxy's sends then need a lock,
the job state too, and the adapter's buffer returns must reach that thread).

Traps found: the card's flick step (`f`) does not move a page (only a finger pans; the injected
`pan` does); the device's `/tmp` filled with stale `.raw` dumps from earlier rounds and every
`dev-sh.sh` then failed with "No space left on device" (remove them with `novacom run
file:///bin/rm -- -f <each file>`; a `-c` string is split by novacom); a zoom injected through
`inject.cmd` is not seen by the card (its frames are scaled to the card's own zoom).

## 2026-10-18 (evening) delta — the GPU path under Cavekit: Tier 87, T-503 done (READ FIRST)

**The owner:** after a finger test on the GPU path (*"areas turn grey and zoom rendering of page
content lags behind actual page zoom state"*; it ran on a hand-started diagnostic daemon while a
scripted pan was going, see the memory `owner-tests-clash-with-pan-runs`), *"continue using cavekit
3.1 to finish the implementation"*. The GPU work is now a kit (`context/kits/cavekit-gpu-rendering.md`,
R1-R12), Tier 87 of the build site (T-502…T-514) and the registry (`.cavekit/task-status.json`:
Tier 87 added; T-363 closed because the owner moved Codex's owed reviews to the inspector on
2026-10-13; T-366/369/370 closed as the site records; the tabs groundwork T-347…T-353 set
`blocked` by the owner's 2026-10-06 word). Order of the loop: T-385…T-387 (engine-update
readiness), then Tier 87.

**T-503 done (rounds 138-150):** fixed and sticky content at minimum cost, and frames that verify.
- Opaque fixed/stuck layers (the layer or its opaque children cover every pixel it covers) keep
  their pixels from the base frame, per covered rect; only the rows the shift uncovers beside them
  are read. A sliver under 32 rows and every non-opaque one are read as their columns, cut into
  chunks packed side by side on the band surface. `JIHAD_GPU_NO_KEEP=1`, `JIHAD_GPU_NO_FIXED_COLS=1`.
- The draw culling never culled: a quad the clip leaves nothing of became a 2x2 rect at the origin
  (60 draws per band pass on Wikipedia). `JIHAD_GPU_NO_CULL=1`.
- A sticky layer the engine already laid out stuck (an anchor move zeroes the async offset) counts
  as stuck: its sticky ranges are probed with a step both ways (a stuck sidebar slid with the
  content for a frame after every anchor move).
- The root composition on the GPU path covers what the card shows (a page laid out wider than its
  window, Hacker News at 0.955: the card shows 804x986 CSS px of a 764x942 scroll port, and the
  bottom 42 rows and right 38 columns were clipped blank, then carried up by every pan), and the
  page gets a displayport of what the card shows from the start and at every zoom (the pan slack
  only from the first pan).
- `JIHAD_GPU_VERIFY=1` is trustworthy now: its whole composite changes nothing the next frame reads
  (it consumed anchor moves' changes and the next frames kept stale rows), it skips frames made at
  another zoom, and it reports rows **structurally wrong** (16-row blocks that sit better at another
  row offset or differ heavily at every offset) apart from sub-pixel resampling (a fractional
  scroll resamples sharp text by a few levels; a tile seam by one pixel). 0 structurally wrong rows
  on `jihad-long-static.html`, the new `jihad-sticky-mix.html` (opaque header and footer, a
  transparent sticky sidebar over stripes), desktop Wikipedia and Hacker News.
- `JIHAD_GPU_PHASES=1` times a composite's phases; `JIHAD_GPU_BAND_ROWS=<n>` sizes the band surface;
  `tools/device/gpu-bench.c` section 10 (column reads cost by the pixel).

| Page (scripted pan, `pan-run.sh`) | GPU path | Software path |
| --- | --- | --- |
| `jihad-long-static.html` | 27-34 fps | — |
| `jihad-sticky-mix.html` | 15-17.5 fps | 0.9-3.1 fps |
| desktop en.wikipedia.org (`PAN_DELAY=12`) | 16.4-18.5 fps (was 13-14) | 9-12.4 fps |
| news.ycombinator.com | 34-37.5 fps | — |
| `jihad-anim-css.html` (no pan) | 24-32 fps | — |

**Next (the loop's order):** T-385…T-387, then T-504 (zoom: the owner's finger test; a zoom-in's
first frame took 1.2 s and later ones 1.7 s, each 256x256 tile upload 12-63 ms behind the GPU's
queue), T-505 (Wikipedia to 24 fps), T-506…T-514.

## 2026-10-17 delta — the rendering pick: 2 + 4, a one-week spike first (READ FIRST)

**End of session 2026-10-17 (rounds 11-86), the state to pick up:** the compositor path draws
correct frames on the card and runs the animation page at **21-27 fps** (the chart's 24 fps row,
the GPU's floor for a four-layer composite plus a readback) and the scroll page at **12-17 fps**
(the chart's row not reached: the daemon scrolls the engine per step, so every frame repaints and
re-uploads content; the designed remedy is a compositor-side scroll inside the painted
displayport). The whole account, the measured table, what is in and what was tried and dropped:
`docs/RENDERING-OPTIONS.md` "T-502, the spike" (rewritten; the round 6-10 frame rates further
down this file were measured on black frames and are superseded). The device scripts:
`tools/device/t502/`. Traps: `docs/RENDERING-OPTIONS.md` (same section) and the memory
`gpu-wedge-needs-a-reboot`.

**Done on 2026-10-18 morning:** the engine's temporary commit is folded into patches 0040 (the
compositor) and 0041 (the Adreno 220's GL), the diagnostics-only files restored; the patch queue
reproduces the pin (`e0c04228`), `check-engine-pins.sh` says all agree; the device's Enyo card runs
the bundle built from that tree (`116788fa`: animation page 21-26 fps, scroll page 13-15); the
branch was fast-forwarded into `main` (the only permanent branch). The owner pushes `main` and
force-pushes the engine fork's `jihad/uxp-RB_20261006` (its history was rewritten by the fold).

**2026-10-18, later (corrected):** the compositor-side scroll is in (engine patch 0042 with the
two-band reads, the band-sized target and the displayport/metadata/surface fixes;
`gpuScrollTo` in the daemon with an anchor; `JIHAD_GPU_ENGINE_SCROLL` restores the per-step
engine scroll). The 17-25 fps pans reported earlier that day were frames whose content moved
only at engine anchor moves; `docs/RENDERING-OPTIONS.md` "T-502" has the corrected account and
the three causes. Verify a pan ONLY with `tools/device/t502/pan-run.sh` (mid-pan frame dumps)
and `JIHAD_GPU_VERIFY=1`; never with a screenshot (the adapter's view does not move in a
scripted pan). The numbers after the surface fix (rounds 95-100, bundle `574ff024`+glFinish): band target
21-25 fps, whole reads 19 fps, header still and at rows 0..44, the compositor's whole frames at a
pinned scroll correct (`verify2b-*.jpg`).

**Rounds 99-112 (2026-10-18, later): the white rows found and the pan made fast.** The "one
composite in five white below row 484" was the Adreno 220 driver binning an FBO render by the
EGL SURFACE's size: the frame (942 rows) composed into an FBO while the 1100x1400 pbuffer was
current lost its bottom 1400-942 = 458 GL rows in some composites (every quad drawn, raw scissor
and viewport correct, `glFinish` and a second read no help; a magenta clear proved those rows were
never rasterized). The context's pbuffer is now the frame's size (`JihadUsePBufferOfSize`), and the
rule is: every render target and viewport the same size as the current surface. Scroll-only
frames compose their bands stacked on their own 768x256 pbuffer and FBO (`JihadBeginBandPass`;
the switch INTO that surface costs 11-13 ms, so it stays current between band frames and a whole
composite switches back), the 2.9 MB row shift runs while the GPU works, the compositor wakes the
daemon's loop through a pipe the moment a job finishes (`jihad_offscreen_set_done_fd`), and the
displayport is set from a page's first pan only (a still or animated page paid for 3.5 viewports
of painting). The compositor-side scroll is engaged by a card-driven pan only and settles 300 ms
after it (the engine scrolled to the card's position, the async scroll off); a scroll the page
makes itself is followed as before. Also fixed: two reads of a job after its hand-off (a
use-after-free race the wake pipe made frequent), and round 91's "a tree change makes the other
slots stale whole" (an animated page read whole frames every frame).

Measured on the bench (CE 3.1.0), frames verified by the dumps and by eye:

| Page | Frame rate | Note |
| --- | --- | --- |
| `jihad-long-static.html`, scripted pan 0 → 3000 card px in 10 s | **27-32 fps** | frame gaps mostly 24-31 ms; header still; no white or repeated rows |
| `jihad-anim-css.html` | **26-31 fps** | the diff first, then only the changed rows composed on the band surface (round 119) |
| `jihad-scroll-js.html` (the page scrolls itself) | 13.5-17.4 fps | the shift from the diff, the displayport from its first scroll, small steps on the band surface; the card lags the page's scroll (pre-existing) |
| news.ycombinator.com (1249 CSS px tall: the scripted pan overshoots its end; the real card clamps) | 36-37 fps | frames right up to the page's end |
| www.bbc.com/news, github.com | 0.2-2.4 fps, and 0.1-1.7 on the software path | the page itself (script, layout) holds the main thread on this CPU; not the compositor |
| **en.m.wikipedia.org/wiki/HP_TouchPad** (redirects to the desktop site), the same pan after 12 s of loading | **13-14 fps** (software path: 9-12.4) | correct only since round 132-134 (before, the right third and the bottom were never painted and a sticky header doubled); stuck sticky sidebars are ~540 fixed rows re-read every step |

Rounds 129-134 (the real page, correctness): on a page laid out wider than the card (the desktop
Wikipedia at a fit zoom of 0.69: a 1116x1374 CSS px scroll port) the root scroll frame clipped to
the COMPOSITION size, the engine window (764x942): the GPU path never showed the right third or
the bottom. The root's composition is its scroll port on the GPU path now
(`CalculateCompositionSizeForFrame`); the displayport base is set to the scroll port every time and
its margins cover what the card shows plus the anchor slack; the fixed rows read again are widened
by the shift (the base frame's fixed pixels land just above them on a scroll down: a sticky
header showed twice); a second job builds on the job in flight (exact while scrolling, round 130).
The dump counter restarts with a scripted pan, so `pan-run.sh`'s dumps are mid-pan.

Rounds 126-128 (a real page): the displayport is 0.5 heights up and down and none sideways unless
zoomed in (`JIHAD_GPU_DP_Y`, `JIHAD_GPU_DP_X`); the 1.25-height one painted ~95 tiles on an
image-heavy page at a fit zoom of 0.69 and every tile upload waits behind the GPU on this chip
(one composite 300 ms, a job 3 s). The anchor is 0.25 heights. Only sticky layers the compositor
holds in place count as fixed rows, and band frames recompute the fixed rows at their end. **Test
every GPU change on a real page against `NOGPU=1` on the same pan** (`PAN_DELAY=12`).

Rounds 118-119: a page that scrolls itself gets the displayport too; a frame whose tree changed
takes the layer diff first, without a render (`JihadDiffNow`: the composite's own steps up to the
diff, the stored tree refreshed), and when the changed rows, the asked bands, the entering band and
the fixed rows fit 256 rows it is composed on the band surface like a scroll-only frame. Also
fixed: the round-115 shift was cleared before the bridge could read it (the daemon's own estimate
had been carrying it). Open: an engine anchor move during a pan still diffs as every row changed.

Rounds 115-117: the layer-tree diff treats the root-content layer's pure vertical translation
as a scroll (`JihadDiffScrollBegin`/`JihadDiffScrollShift`, LayerTreeInvalidation): the engine
shifts the kept rows by it and reports the frame's position back, so a page's own scroll reads
the entering band, not the frame. The diff region is now scaled by the world transform (it was
in the root layer's CSS px: on a fit-zoomed page an animation's changed rows were composed and
read at the wrong rows). The two-deep queue reads the composed job's changed rows (it lost them
before) and is used only when nothing scrolls.

Round 113 builds each new frame from the NEWEST finished slot (copied with the scroll since it,
only the entering band read; the fixed rows only when the scroll moved), unless the slot's own frame
is at the same scroll with little stale (an animated page). Measure a still or animated page with
`tools/device/t502/pan-run.sh <name> <url> "<words>" none` (12 s, no inject).


**Left, in order:** (1) the owner's finger test of a real pan and flick (the entry is in
`docs/FINGER-TESTS.md`); (2) the pan's remaining long gaps (56 ms+, 2-8 per 2 s window): not the
anchor moves any more (round 124: the painted region reaches the compositor as the region, not
its bounds, and changed regions are clipped to the frame before their bounds, so an anchor move is
a 46-72-row band frame) and not the anchor size (0.15/0.3/0.6 heights alike, round 125); one tick
showed a 101 ms gap between ticks with no pump or paint, i.e. another main-loop source: trace the
loop uncapped (the tick log stops at 120); (3) fixed rows as two intervals (a page with a fixed
header AND a fixed footer reads the whole frame on every pan step); (3) a fourth shared buffer for the
two-deep frame queue; (4) a damage rectangle in the frame header so the adapter invalidates and
LunaSysMgr uploads only the band; (5) option 2 (WebGL through EGL); (6) prune the `[jihad-gpu]`
diagnostics that remain in the engine and the daemon (bounded, cheap; the verify mode and the
dumps stay, they are the test). The device: the Enyo card carries the probing adapter (stock as
`BrowserAdapterImpl.so.orig`); Mochi and Mojo untouched; host servers 8077/8078 still running.


**The owner, this morning:** the rendering report (the Claude doc "Jihad rendering options: speed
comparison", https://claude.ai/code/artifact/6436a954-170a-48b2-8b35-e87dc4b3fbf5; the same tables
in `docs/RENDERING-OPTIONS.md`) was cut down step by step: dominated combinations out, 6 (CE-only
LunaSysMgr change) and 2-without-4 (11 fps WebGL) advised against, rows under 24 fps scroll out,
rows without WebGL out; a Flash games column (Flash's own 27 / 22 fps bounds every row; the
movie-sized GPU readback and upload were measured for it: `tools/device/gpu-bench.c`, 4.9–6.3 /
1.9–2.3 ms and 1.8 / 0.55 ms), a webOS column (every survivor runs on 3.0.5 and CE 3.1.0), colour
chips, a compare-and-contrast of the last two. Then the owner's answers to the choosing questions:
video sometimes, 24 fps video acceptable, styled video matters (fades, rounded corners, video under
a modal: the overlay cannot do these), no date, spike first. **The pick: 2 + 4** (GPU compositor
with readback, then WebGL through EGL). 5 is held back as a full-screen-only overlay if 24 fps
full-screen displeases by eye.

**T-502, the spike (week 1):** the OpenGL compositor up in the headless daemon on the device, one
real frame of a page read back into the YAP buffer and shown on the card, timed against the 42 ms
model. Everything it needs is upstream code; the entry points, all verified in `third_party/uxp`:
- OMTC on: drop `JIHAD_DISABLE_OMTC=1` from `packaging/event.d/{jihad,jihad-mochi,jihad-mojo}`
  (the override in `gfxPlatform::UsesOffMainThreadCompositing`, patch 0003, then does nothing).
- The widget: `PuppetWidget::GetLayerManager` (patch 0005, `widget/PuppetWidget.cpp:635`) takes the
  BasicLayerManager branch under `JihadOffscreen()`; the spike adds a branch that calls
  `nsBaseWidget::CreateCompositor(w, h)` (`widget/nsBaseWidget.cpp:1292`, in-process through
  `GPUProcessManager::CreateTopLevelCompositor`, a `ClientLayerManager`).
- Acceleration: `AccelerateLayersByDefault()` is false in this build (`gfx/thebes/gfxPlatform.cpp:2126`,
  no `MOZ_GL_PROVIDER`); `layers.acceleration.force-enabled` or `--with-gl-provider=EGL`; and
  `gfxPlatformHeadless` must list `LAYERS_OPENGL` among its compositor backends (check
  `GetAcceleratedCompositorBackends`). The headless `GfxInfo` answers UNKNOWN, blocks nothing.
- The GL context: `CompositorOGL::CreateContext` (`gfx/layers/opengl/CompositorOGL.cpp:106`) with
  `MOZ_LAYERS_PREFER_OFFSCREEN=1` calls `GLContextProvider::CreateOffscreen` = an EGL pbuffer
  through `GLContextProviderEGL` (the headless build's default provider, `gfx/gl/GLContextProvider.h`;
  `GLLibraryEGL` loads `libEGL.so` by name, the vendor library `gpu-bench` already drove).
- The readback: `CompositorOGL::EndFrame` copies to `mTarget` when one is set (`CopyToTarget`, the
  measured `glReadPixels`); `CompositorBridgeParent::ForceComposeToTarget(DrawTarget*)`
  (`gfx/layers/ipc/CompositorBridgeParent.cpp:1267`) is the entry; from the main thread the upstream
  road is `ClientLayerManager::BeginTransactionWithTarget` → `MakeSnapshotIfRequired` →
  `SendMakeSnapshot` (sync) → `RecvMakeSnapshot` → `ForceComposeToTarget` into a shmem surface the
  client copies to the target: i.e. a `jihad_offscreen_render_region` that paints with a target
  instead of `RenderDocument`.
- Measure: the daemon's existing paint timing lines, per frame, on the device; compare with 42 ms
  (composite 12.9 + readback 12.3 + copy 5.1 + scale 8.2 + Luna 3.8).
What the pick does to the 39 engine patches (0020 and 0035 no-ops, 0003 and 0005 half, 0006 and
0021 change, the rest untouched): `docs/RENDERING-OPTIONS.md`, "The pick".

**T-502 RESULT (the same day, five rounds):** the compositor runs on the Adreno 220 (EGL pbuffer,
`CompositorOGL`, patch 0040 behind `JIHAD_GPU_COMPOSITOR`), composites four layers in 1–5 ms and
animates a CSS transform without main-thread paints; the frame on the device is 54–96 ms against
the 42 ms model because the readback is clock-bound (22–60 ms in the daemon, 12–21 in the bench),
the snapshot road adds 12–18 ms and the swizzle 7–16. THE TRAPS: (1) the bundle's goanna.js
carries `layers.offmainthreadcomposition.force-disabled=true` (the env now wins over it); (2) a
child PuppetWidget's Destroy() tore down the SHARED ClientLayerManager (crash in
BeginTransactionWithTarget; the child only drops its reference now); (3) a glReadPixels from a
TEXTURE-backed FBO costs 370–640 ms on this driver — the stock offscreen GLScreenBuffer is one —
so the spike draws into a plain pbuffer context (no screen buffer; a renderbuffer FBO reads even
faster, 12 ms); (4) `CopyToTarget` flips unconditionally and an offscreen context has no view
flip, so the frame was upside down: the direct read copies row 0 to row 0; (5) the content upload
takes the tiled `glTexSubImage2D` path (180–240 ms a change) — must become whole-texture; (6) the
compositor animates without dirtying the widget, so the daemon needs the compositor's own
"composite due" signal (`JIHAD_GPU_FORCE_FPS` stands in). Numbers, table and verdict:
`docs/RENDERING-OPTIONS.md` "T-502, the spike"; logs and screenshots `docs/rounds/t502-2026-10-17/`.
**Round 6 (the owner said "continue"):** the read on the compositor thread straight into the
YAP buffer (`CompositorBridgeParent::JihadComposeAndRead`, a task posted to the compositor loop,
the main thread waits on a Monitor), a renderbuffer FBO as the window target, PULL MODE
(`CompositorVsyncScheduler::ScheduleComposition` only counts under the env; the daemon pulls a
frame when `jihad_offscreen_compositor_seq` moves — an animation keeps asking after each
composite), and the clock-floor leak (a daemon that dies holding the floor left scaling_min_freq
at 1188000; the probe now takes cpuinfo_min_freq when it finds the floor at the top). Measured:
compose 1.4–4.6 ms, read 7–17 ms, **17–24 fps on the animation page** (`spike-run6-*`).
**Rounds 7–10 (the owner: the chart's numbers are the floor, keep going):** patch 0041 (the Adreno
220 takes whole-texture uploads: `GLRenderer::AdrenoTM220`, `CanUploadSubTextures` false), the
read forces alpha and the daemon samples the nonblank count; 20–31 fps on the animation page. The
compositor path now scrolls through the ENGINE at every zoom (`setZoomAndScroll` /
`setScrollPosition` GPU branches, no render pan) meant to run inside a displayport of a viewport up and
down (`SetGpuViewport`) — found on 2026-10-18 to have failed on every round (`rv 0x80004005`), so
the engine painted the viewport only; the 21–24 fps on `jihad-scroll-js.html` were per-step engine
scrolls with viewport-sized paints. THE TRAP: `nsIDOMWindowUtils::setResolutionAndScaleTo` and
`setScrollPositionClampingScrollPortSize` each black the whole frame on this path (bisected with
`JIHAD_GPU_NO_RESOLUTION` / `JIHAD_GPU_NO_CLAMP`; the displayport was in truth never set, see
2026-10-18; the resolution route is untested since the y-flip and surface fixes), so the zoom is a
WORLD TRANSFORM on the compositor's tree (`jihad_offscreen_set_view(widget, scale, offX, offY)` →
`LayerManagerComposite::JihadSetWorldTransform`, applied in `ComputeEffectiveTransforms`). The
residual visual offset (a pinch-in pan past the layout viewport) is passed as 0 until wired; the
33 ms frame-gap floor seen in the pipeline stats is the adapter's ack cadence (one buffer in
flight), the next thing to look at. The first content frame costs 125–131 ms once (shader
programs linking), not per change as first written.
**Next:** the world-transform zoom verified on the device (the colours page at the card's 0.7837),
the residual offset for pinch-in, the adapter cadence / a second buffer in flight, the GPU clock
(`/sys/devices/platform/kgsl/msm_kgsl/kgsl-3d0/io_fraction`), the daemon's own per-frame work (T-499's layout viewport and the
fit zoom today crop the frame), the plugin and video layers through the ImageBridge; then 2.

**Carried over, unchanged:** daemon 18 (the hold's release fix) built, NOT deployed; FFmpeg libs on
Mochi only; T-499's paint path restoring the card size; the placeholders in the evening delta below.
**Device:** CE 3.1.0, the three daemons as the evening delta left them; `gpu-bench` ran twice more
from `/tmp` and was removed. Host servers on 8077/8078 may still be running.

---

## 2026-10-16 (evening) delta — the README's caveats worked: the stall, H.264, the viewport (READ FIRST)

**The owner:** "fix the stall"; then "move all caveats in the readme into a dedicated caveats section
at the bottom. also change the language to be less corporate advertisement like while still being
2011 era appropriate" (done, `4a76dbae`); then "work to address all of the caveats in the README.md
which can reasonably be classified as solvable problems". The ledger (`docs/MODERN-WEB.md`) has
T-499 (the viewport, built), T-500 (the stall) and T-501 (H.264/AAC) with the numbers.

**T-500, the stall:** the daemon has a stall sampler (`$JIHAD_STALL_SAMPLER=<ms>`,
`GoannaRenderPage.cpp`; resolve with `tools/host/stall-resolve.py` against the unstripped libxul in
the obj dir — addr2line over ten thousand addresses takes ~10 min on the host). 33 stalls resolved:
the pages' own JavaScript (JIT code + the runtime) two thirds, compile (baseline, Ion, regular
expressions) and GC/CC the rest; two 5-s stalls are cycle collections. The clock: Palm's
`ondemandtcl` governor (up_threshold 95, sampling 200 ms, max_tickle_window 3 s) kept the CPU at
192–384 MHz through a page load. **The clock hold** (`JihadBrowserServer.cpp` tick loop, daemon 16/17,
pref `jihad.cpu.hold`): the frequency floor (`scaling_min_freq`, both cores; cpu1 is hot-plugged)
goes to the top while the engine has work, back after 2 s of idle ticks. **THE TRAP OF THE DAY** (a
reboot): writing `scaling_governor` wedged the kernel's cpufreq lock — every cpufreq read and
every `luna-send` hung, `reboot` hung, only `novacom run file://sbin/reboot -- -f` got out; memory
`cpufreq-governor-switch-wedges-the-touchpad`. A floor write is safe (tested with capped reads).
Status: DONE (the build site's T-500 row).

**T-501, H.264/AAC:** `build/webos-oe/fetch-ffmpeg.sh` (host, sha256 pinned) then
`in-container.sh --arm build/webos-oe/build-ffmpeg-arm.sh` → `out-arm/ffmpeg-arm/lib/libavcodec.so.58`
+ `libavutil.so.56` (FFmpeg 4.4.5, H.264/AAC/MP3 decoders and parsers only, LGPL v2.1+, linked
against the toolchain's own sysroot — the Debian one has no crt1.o). The bundle ships them when
built; NOTICE and `licenses/runtime/ffmpeg-COPYING.LGPLv2.1`. Patents on H.264/AAC: the owner's
call, noted in NOTICE. Status: DONE; on 2026-10-19 a device updated only by `push-engine-update.sh` had lost the libraries (every MP4 refused) and the push now carries them: H.264, AAC and MP3 play to the end again.

**T-499, the viewport:** `GoannaRenderPage::SetLayoutSize` + the once-per-page rule in
`emitGeometry` (content wider than the card by 8 px, ≤ 1100 px, no meta viewport; pref
`jihad.viewport.followOverflow`); reset on navigation and on `setWindowSize`. Status: PARTIAL, open: the engine window resizes but the page still reads innerWidth 768 (the paint path restores the card's size).

**Not solvable from here (the README's Caveats say so):** the device scope; the system's own TLS;
WebGL (no GPU driver); Flash's availability; `palm-install`; Akamai's and Reddit's gates;
GitHub's empty parts — its page uses an import map (120 module preloads under it), which the
engine lacks, and GitHub's script policy allows only its asset host, so an import-map shim (which
loads modules from blob URLs) cannot run there: engine work. Mastodon's timeline: not looked at
again (needs a console-on visit). The double-tap/pinch sharpness: PICKUP's 2026-10 note stands
(cause not found).

**Device state (2026-10-16 evening):** superseded — the state the device was left in is in the newest delta at the top of this file.

---

## 2026-10-16 (afternoon) delta — the harness, the second signature, YouTube's mobile site, the web-compat shims (READ FIRST)

**The owner:** "continue" (the open items of the morning), then "fix the regular duckduckgo, and
speed up youtube". Everything below is in `docs/MODERN-WEB.md` T-496…T-498 with the numbers and
shots (`docs/rounds/t496-2026-10-16/`).

**T-496 — the identity read on the wire.** `render/goanna/test/jihad-identity-server.py` (host,
port 8078) fakes `cf-mitigated: challenge` and Startpage's `/sp/captcha-block` redirect, logs every
request's User-Agent header and takes each page's `navigator.userAgent` back. On Enyo: the no-loop
branch held (two loads one second apart, the second challenge shown; a new card made one load);
a learned site's top document, same-site frame, image, script, XHR AND a third-party frame
(httpbin.org) all carried the current identity, script agreeing. `_challengeOf` in
`components/jihadUserAgent.js` now has a second signature — a top-level 3xx to `/sp/captcha-block`
— measured on the harness and on the real Startpage: home renders; the search then meets Anubis's
proof of work, which finished in ~4 minutes and rendered the results (a cookie keeps the pass).
**eBay and old Reddit are not the identity's**: curl from the host gets the same Akamai 403 /
login redirect with either token (rows 9 and 11).

**T-497 — YouTube sped up.** Measured: the desktop watch page is 13.7 MB of script decompressed
(3.6 MB on the wire), the mobile site 3.0 MB; on the device the desktop page stalls the main
thread for 24 s and is still grey skeletons after a minute (274 MB), the mobile site's worst stall
is 5 s (162 MB) and its home feed renders in 18 s. So `jihad.ua.sites` (platform prefs) gives
youtube.com a **mobile Firefox identity**, applied by the identity component to the header AND
`navigator.userAgent` (frames and subresources too); the daemon's youtube.com row is OUT of
`EngineHost.cpp` (daemon 14). Settings → General → "Sites with a set identity" says so. The tap
on the mobile player is a finger test.

**T-498 — the web compatibility shims.** `render/goanna/components/jihadWebCompat.js` (contract
`@riverstonerelay.net/jihad/web-compat;1`, classID `{5a1c7e2d-…}`, manifest
`components/jihad-web-compat.manifest`, in REQUIRED, created at daemon start): at
`document-element-inserted` a sandbox with the page's principal and window defines on the page's
own prototypes what the engine lacks — `CSSStyleSheet.prototype.replace` (Etsy's 47 TypeErrors:
Lit tests `replace`, the engine has only `replaceSync`), `reportError`, `Intl.ListFormat`,
`Element.checkVisibility`, `AbortSignal.any`, `URLSearchParams.size`, `Array.fromAsync`, `inert`,
and the popover attribute with its methods and events — plus ONE agent sheet for the two
attributes' UA rules. The regular DuckDuckGo page already renders its results (zero script
errors, 39–42 s, a 13 s stall); the popover gap is what made its menu stand open. Prefs
`jihad.compat.enabled` (on) and `jihad.compat.trace` (off): the trace mode wraps the engine calls
that can throw an XPCOM result to a page and logs which one threw, with the page's own stack —
the bridge sees such throws only as "NS_ERROR_NOT_INITIALIZED: " at column 0 (the engine's
Exception object has no column; the daemon's console line now carries `source:line:column`
anyway, since daemon 12). Engine facts checked in `third_party/uxp` (what it has and lacks) are in
the ledger's T-498. The API probe page `render/goanna/test/jihad-apis.html` has a second list
(169 probes; it posts its full list to the harness server). Three trace rounds: the shims'
test page 16/16, Etsy's errors gone, Bluesky's NetworkErrors are the blocker's (telemetry
hosts). TWO TRAPS in the component: (1) with `sandboxPrototype: win`, a method taken from `window`
inside the sandbox comes bound to the prototype proxy and refuses the page's window as receiver
("'matchMedia' called on an object that does not implement interface Window") — the shims now
take the page's own window (`win.wrappedJSObject`) as an argument and the sandbox has NO
prototype (the probe keeps one, for bare names); (2) a wrapper of a window function must give
`this` back when the page called it bare. Both fixed in the final component (strict sources; the page's
iterator prototype through the page's `Array`); `docs/rounds/t496-2026-10-16/` has the three
trace logs, the harness server's log, the probe list and the shots.

**T-499 — found, OPEN:** DuckDuckGo's menu panel stands open because the page is 1000 px wide,
the engine's viewport is the 768 window, and the card fit-zooms the whole width: what a page
parks just past `innerWidth` is on screen. The ledger's T-499 has the design (an offscreen engine
window as wide as the overflow); the daemon's zoom/rotation history makes it a task of its own.

**THE TRAP of the day:** `build/webos-oe/build-daemon-arm.sh` runs with `set -o pipefail`, and
its export check used `grep -q`, which leaves at the first match — the writer took SIGPIPE and the
check failed AT RANDOM ("does not export free", then "… jemalloc_stats", on a file that had both).
Fixed (a grep that reads to the end). Also: the daemon is built against the EXTERNAL API — no
`xpcpublic.h`, no `nsContentUtils`, no `AutoJSAPI`; a JSContext is not reachable from it (that is
why the stack of a page error is printed from inside the page by the trace mode, not by the
bridge). And `git checkout -- <file>` to drop one failed patch dropped the good one with it; the
column change had to be re-applied.

**Device state (2026-10-16 14:55):** all three variants run the 13:15–13:19 packages (daemon 14
`f481552f`, identity component `affbb468` with the site rules, blocker v10) plus, by dev copy
after them, the final web-compat component `47c33ff6` (the same file the rebuilt packages
`build/webos-oe/out-all/` carry: enyo `ac14bfe7`, mochi `1bdc6189`, mojo `305fb1f6` — so a
re-push is not needed; the only difference is `layout.css.has-pseudo.enabled` appended to each
variant's `goanna.js` by hand where the package has it in the platform block) — jobs running,
one process each, Default look, `/tmp` clear, nothing hand-started. Learned lists: Enyo
`startpage.com`, Mochi `w3.org,medium.com`, Mojo none (product state). The shims' test page
28/28 and the API probe 131/169 on Mochi (`:has()` on: the `true` line is at the END of each
`goanna.js` — a guard that greps for the pref's NAME sees the engine's own `false` line at 2476 and
appends nothing; grep for `", true` — the first attempt of the day did exactly that and reported
"1 line" for the engine's). The host's two servers may still run: the probe
server on 8077 and the identity harness on 8078 (`pkill -f 'http.server 8077'`, `pkill -f
jihad-identity-server`).

**Open after this:** the finger tests above; the `NS_ERROR_NOT_INITIALIZED` thrower named by the
trace (the jQuery probe of 2026-10-16 cleared jQuery's start; the thrower is a later call of those pages, unnamed: `docs/MODERN-WEB.md`, T-498); `:has()` behind `layout.css.has-pseudo.enabled` untested; container
queries, `backdrop-filter`, `content-visibility` are engine work, not shims; the kit memory goal
(12 MB idle) unmet; the owner's push.

---

## 2026-10-15 (evening) delta — the v5.1 push, the round's rows, the probes (READ FIRST)

The session resumed after a usage-limit pause; the steps below were the plan at that pause and are being worked through in order (status in the build site, T-486…T-492):
1. **Push the v5.1 packages** (`build/webos-oe/out-all/`, 13:20: enyo `71dc8063`, mochi `1440d170`,
   mojo `34927e9f`; component v5.1, the daemon with the about:jihad text): the session's
   `final-push.sh` was `push-variant.sh enyo; mochi; mojo` then a check of each variant's daemon
   md5, `blocklists/`, `goanna.js` prefs, `chrome.manifest` line and the blocker's start line. One
   novacom session at a time (memory `one-novacom-session-at-a-time`).
2. **Verify v5.1** (the session's `verify5.sh`, recreate from PICKUP if the scratchpad is gone):
   `python3 -m http.server 8077` in `render/goanna/test/` on the host (the device reaches
   http://192.168.1.245:8077/), then in Mochi `jihad-blocking.html` over http (expect: the two host
   rules and the typed script refused, the typed image loaded, the facebook `/tr?` path refused,
   facebook's favicon and the page's own image loaded; the log lines say `blocked <type> by
   <rule>` with `jihad.blocking.log` true as a user pref), MDN clean, Settings → Privacy in Enyo
   and Mojo (the four rows), `about:jihad` (the lists' credit), `jihad-jsbench.html` in Mojo (the
   title line carries the ms; host V8: loop 167 ms; a JIT on this ARM ≈ 10×, an interpreter ≥ 30×),
   the memory A/B (Mochi idle after a cache load, with vs without the manifest line; v3 was
   69.8 vs 33.4 MB; v5.1 asks for a heap-minimize after install), and the hero shot: Enyo on
   https://www.bbc.com/news.
3. **The hero:** raw shots for `readme-art.py` are in the session scratchpad only (lost):
   retake Mojo on https://archive.org/, Mochi on MDN's CSS page (adverts blocked), Enyo on BBC
   News, all at the Default look; `python3 -I docs/screenshots/readme-art.py <raw> <fonts> docs/screenshots .`
   (Prelude fonts from the device's /usr/share/fonts). README alt text and CREDITS already describe
   that image; `docs/screenshots/cardview.jpg` on disk is still the old themed one (inspector F-6).
4. **Round 2's rows** (T-491): `docs/MODERN-WEB.md` "Round 2" has the reading and points at
   `docs/rounds/round2-2026-10-15-analysis.md`; write the per-site rows with cause and
   disposition; the shots are on the device's /tmp until it reboots (`shot-r2-<name>.png`).
5. **The fixes (T-492), then the second run** with v5.1 (`tools/device/browse-round.sh` after
   `novacom put file:///tmp/round2-sites.txt < tools/device/browse-round-sites.txt`,
   `dev-sh.sh -t 2500`): first the JIT question (jsbench), then the `dom.max_script_run_time`
   trade-off (60 s froze cards for the stall's length; 15 s asked), then the blocker's share
   (v3 vs v5.1 on the same pages), then the per-site faults.
6. Kit checkboxes (cavekit-content-blocking.md R1, R4, R5, R6; cavekit-modern-web.md R6) and the
   build site (T-486…T-492), commit, push (the owner's).

**Device at the end:** Mochi: component v3 (dev push), the job restored by the round script, Wi-Fi
on; Enyo and Mojo: the morning's packages (no blocker); Mochi's `goanna.js` carries the blocking
defaults by hand; Mojo and Mochi profiles carry `dom.max_script_run_time=60` and Mochi
`jihad.blocking.log=true` as user prefs; all variants at the Default look. The host's test http
server on port 8077 may still be running (harmless; `pkill -f 'http.server 8077'`).

## 2026-10-15 (afternoon) delta — the modern web ledger; an ad and tracker blocker in the engine (READ FIRST)

**The owner:** "dont show wikipedia. showcase the modern web", then, on three sites failing in one
screenshot round: "dont just breeze past that. document and fix issues loading the modern web.
build in an ad and tracker blocker if possible (following license terms)". Also "cardview.jpg
should be unthemed screenshots with a frutiger aero background to the cards in the image".

**Kits and tiers (new):** `context/kits/cavekit-modern-web.md` (R1 ledger, R2 slow-script guard,
R3 Cloudflare, R4 GitHub, R5 Wikipedia) and `context/kits/cavekit-content-blocking.md` (R1–R7);
build site Tier 85 (T-480…T-484) and Tier 86 (T-485…T-490); both in the overview.

**The ledger** `docs/MODERN-WEB.md`: every site of the day with result, cause and disposition.
Works: webosarchive, pivotce, howsmyssl, Bloxorz, codeberg, archive.org, mozilla.org, BBC News,
DuckDuckGo Lite, MDN, The Verge (at the 60 s guard). Partial: GitHub (deferred parts stay grey),
Mastodon (empty column), DuckDuckGo's full results page (an engine-feature case). Blocked:
w3.org (Cloudflare's check, even with the UA rule — console output pending).

**Fixed and device-verified:** T-481 `dom.max_script_run_time` 60 (`packaging/prefs/
jihad-platform-prefs.js`; The Verge renders in full, the endless script `jihad-busy.html` still
brings the dialog). **Built, not yet proven:** T-482 the `challenges.cloudflare.com` UA rule in
`EngineHost.cpp` (daemon `out-arm/jihad-browserserver-arm` 10:55, on Mochi); w3.org still stops.

**The blocker (T-485, T-486, T-488 in progress):** `render/goanna/components/jihadContentBlocker.js`
— EasyList + EasyPrivacy (`third_party/easylist/`, snapshots 202610151429, GPLv3+, sha256-pinned,
bundled raw under `blocklists/` by `make-device-bundle.sh`; NOTICE, CREDITS, about:jihad, the
About pane) compiled on the device in JavaScript in slices (2.5 s), cached in
`<profile>/jihad-blocklists/compiled.json`; `nsIContentPolicy` in the content-policy category;
EasyList's hiding rules as one USER_SHEET; prefs `jihad.blocking.*`; 4-day updates from
easylist.to; Settings → Privacy → "Content blocking" (two switches, a live text row, the first
button row: `text`/`button` row types in `preferences.js`). First device test on Mochi: 91,924
hosts, doubleclick and ads-twitter refused, the Settings rows live ("3 requests blocked since
start"). Then widened (compiler v2) to typed rules (`$script,third-party`: google-analytics),
`||host/path` prefixes (MDN's own `/pong/` adverts) and site-specific hiding rules
(`@-moz-document domain()` blocks): retest pending. Test pages: `jihad-blocking.html`,
`jihad-blocking-many.html`, `jihad-busy.html`; `tools/host/count-blocklist.py` is the independent
count (host 48,717/43,229, typed 60/141, path 1,528/4,051, site selectors 8,051/23).

**Dev loop for the blocker (no package rebuild):** the session's `push-blocker.sh`: every file
`novacom put` to the device's `/tmp` (one session at a time — this novacomd answers a second
session with "unexpected EOF from server"), then ONE `dev-sh.sh` script that stops the job, copies
into `deviceroot/hl/` (`components/`, `blocklists/`, `prefsui/content/`, the daemon), appends the
manifest line to `chrome.manifest`, clears `profile/jihad-blocklists` and `startupCache`, starts
the job. A `stop` through a 4-second `novacom run` hold breaks the next session; hold longer.
The blocking defaults were appended to Mochi's `goanna.js` by hand for the test (the package
build writes them from `jihad-platform-prefs.js`).

**Page errors on the device:** `JIHAD_JS_CONSOLE=all` — the job's `script` section copied to
`/tmp/jihad-dbg.sh` with the variable on the `exec env` line and run in the background of a
long-held `dev-sh.sh` while the job is stopped (the session's `console2.sh`); window the log by
line number after the newest "bridge on" (it rotates at 2 MB); the method is in
`docs/MODERN-WEB.md`. **What it showed (12:14–12:18):** w3.org — Cloudflare's Turnstile runs
without a script error and fails on its own verdict (T-482's UA rule is not enough; the next
experiment is a current Firefox token on the site's own requests = moving the base UA's
`Firefox/52.9`, the owner's call); GitHub — `ReferenceError: Turbo is not defined`, and Mastodon
— `SyntaxError: import.meta may only appear in a module`: ES-module loading, upstream UXP, no
configuration applies (T-483 done, rows dispositioned).

**The README:** "Ads and trackers, stopped in the engine" section; Known issues links the ledger;
the hero to be retaken on modern sites that work (Internet Archive, MDN without adverts, BBC
News) at the Default look on the Frutiger Aero backdrop (`readme-art.py` `wallpaper()` draws it
with `make-personas.py`'s primitives) — not yet composed or committed.

**The inspector's review of Tier 86 (T-490, round 26 in impl-review-findings-6.md):** REQUEST
CHANGES — 1 P1 (path rules on the page's own host: already fixed in v3), 6 P2, 15 P3; every
finding dispositioned there. **Component v5** carries the fixes: exceptions with options honoured
first at every level, typed rules per host, path rules with party and case folding, the content
types corrected, selector validation, a redirect check on `http-on-modify-request`, lazy page
host, the master switch gating hiding, version-only header reads, atomic writes done right, the
updater's flags, back-off, truncation check and jitter, logging the matched rule with the pref off
by default, install from the cache's own parse; site hiding rules per document (v4). Prefs:
`network.dns.disablePrefetch`, `network.http.speculative-parallel-limit=0`. The bundle greps the
hashes in NOTICE and the README; REQUIRED and the ipk check carry the lists, the component and the
GPL text. Test pages served over http from the host PC (`python3 -m http.server 8077` in
`render/goanna/test/`, http://192.168.1.245:8077/jihad-blocking.html) for the party cases.
v5 is in the packages built after 13:05 (`build-all-device.sh --skip-arm`) and verified by the
session's `verify5.sh` after `push-variant.sh` ×3.

**v5.1 on the device (14:20–14:50):** Enyo and Mochi carry the v5.1 packages (`push-variant.sh`;
the on-device checks pass: typed, path, own-host, first-party, MDN's `/pong/`). **Mojo's push
failed four times** (the USB link dropped mid-transfer of the 43 MB payload: "novacomd socket
was closed prematurely", then "unable to find device", the device re-enumerating with a new
novacom id each time) **and landed on the fifth, alone on the bus (14:53)**: all three variants
carry the v5.1 packages. Mochi alone also runs the experiment daemon (`c2459f0d`: the
Firefox/128 token for the ten bot-blocked hosts) and the component with the memory-reporter
line, by dev push; the final packages for all three wait on the experiment's verdict, the
Personas guard and the memory work (one rebuild, three pushes, each alone on the bus). **The benchmark** (`jihad-jsbench.html`, Mojo, host V8 in
brackets): loop 50M 976 ms (167), objects 2M 6,854 ms (200), strings 200k 8,280 ms (19), regex
100k 902 ms (17), DOM 20k 10,156 ms — the JIT is live for integer code (6×) but allocation-heavy
code is 34–436× slower: a garbage-collection or allocator problem, the first suspect for every
stall of the round. **Memory:** v5.1 +40 MB idle; heap-minimize did nothing; the component now
logs the memory reporter's figures after install (see the kit's R6).

## 2026-10-16 (later) delta — jemalloc shipped (daemon 11); the learned identity (T-495); the UA rule (READ FIRST)

**The owner:** "Do the engine rebuild with jemalloc" (T-493), "Keep the firefox 52.9 user agent",
then "I changed my mind the user agent should be 52.9 by default but adjust based on the needs
of the page" (the base UA in `JihadUserAgent.h` stays 52.9; a per-page identity is sanctioned —
the daemon's table for the hand-curated sites, and, if the running experiment shows Cloudflare
passes with a current token, a learned per-host identity: a response marked `cf-mitigated:
challenge` puts the host on a persisted list and the page is retried once with the modern
identity, header and docShell pin together — T-495), "Try to fix the challenges blocking sites
from loading including cloudflare and RECAPTCHA" (T-494).

**T-493 built:** `mozconfig.goanna-arm` now `--enable-jemalloc` (the reason in the file); the
engine rebuild took 22 minutes, not hours (incremental: the allocator change rebuilt `memory/` and
relinked libxul; log `build/webos-oe/out-arm/engine-jemalloc-build.log`). THE TRAP: on Linux UXP
puts the allocator in the PROGRAM, not libxul (`MOZ_GLUE_IN_PROGRAM=1` in `config/autoconf.mk`):
there is no `libmozglue.so`, the dist's `libmozglue.a`/`libmemory.a` are 33-byte expandlibs
stubs, and libxul.so leaves `malloc`/`free`/`jemalloc_stats` UNDEFINED for the executable to
export. `build-daemon-arm.sh` therefore links `memory/mozjemalloc/jemalloc.o` +
`memory/build/mozmemory_wrap.o` + `jemalloc_config.o` with `-rdynamic` (as xpcshell's link does)
and checks the exports with nm; a libxul from this config with an older daemon would fail at the
first `jemalloc_stats` call. After the daemon: `build-all-device.sh --skip-arm`, the three
pushes, then the memory report after the last card (`heap-allocated` reports now) and the
three-page RSS.

**T-494 → T-495, the challenges — DONE (the learned identity):** the engine's load policy was
cleared (probe page), the challenge frames emit nothing, and header/script consistency alone
changed nothing; a whole-page current identity passed Cloudflare on three of four sites. So:
`render/goanna/components/jihadUserAgent.js` — a top-level response marked `cf-mitigated:
challenge` learns the site's base domain (`jihad.ua.modernHosts`, persisted), cancels the
challenge page, pins the docShell to the Firefox/128 identity and loads the page again once; from
then on the site's requests (and its frames' — the challenge host included) carry the identity
in the header, and every document of it gets the docShell pin at `document-element-inserted`.
Measured on the device: w3.org and Medium learned, reloaded and rendered; the list survives a
restart; Settings → General → Browser identity shows the sites and has Forget (a finger test).
The daemon's `challenges.cloudflare.com` row is OUT of the table (it made the inconsistency);
the table stays hand-curated (google/bing/youtube/etsy rows at 128). Not covered: Startpage's own
JS check (no `cf-mitigated` header — a second signature would be needed), eBay, old Reddit's
login gate. The one-retry-per-ten-minutes branch was not exercised (no site challenged twice).

**Device state (2026-10-16 03:50):** all three variants run the 03:44–03:48 packages: daemon 11
`4c318d49` (jemalloc in the program), identity component `2196a37f`, blocker v10 (COMPILER_VERSION
10 — the cache recompiles once per profile), `memory.free_dirty_pages` on, jobs running, Default
look, `/` read-only, nothing hand-started, /tmp cleared. Mochi's profile holds the learned list
`w3.org,medium.com` (product state, left in place); Enyo's and Mojo's have none yet. The source
differs from the shipped component by ONE line: the "identity: ready" log now counts the learned
sites at log time (the daemon creates the component before the profile's prefs.js is read; the
pref observer brings the list in — the shipped line printed the creation-time 0). The next push
carries it; no rebuild was spent on a log line. Benchmark and memory numbers: `docs/MODERN-WEB.md`
(T-493). Open from this round: Startpage's own JS check (no `cf-mitigated` header — a second
learn signature), eBay, old Reddit's login gate, the no-loop branch untested, kit R3's frame
header unread on the wire; the memory kit goal (12 MB idle) stays unmet at 36 MB idle.

## 2026-10-16 delta — the second run read; the UA verdict; the daemon's memory after cards

**03:00, the memory question closed:** the full memory report after the last card (v9.1 writes
it behind `jihad.blocking.memoryDump`; `tools/host/memory-report-read.py` reads it) shows one
detached window — the last page, still being torn down — and no ghosts: closed pages are
released; what stays resident is the allocator's retention and libxul's paging (no jemalloc in
this build). **Proposed T-493:** an engine rebuild with jemalloc — the owner's call (hours).
**Shipped 02:27–02:35: the final packages on all three variants** (`out-all/`, 02:20: daemon 10
`b6d5683a`, component v9.1 `36d0c18e`, the lists, the prefs with the 64 MB threshold, the
Personas guard; `push-variant.sh` enyo/mochi/mojo each alone on the bus, all first-try), every
job running, the rounds' test user prefs removed from the three profiles (`set-prefs.sh`),
Default look everywhere, `/` read-only, nothing hand-started. The host's test http server on
:8077 may still run (harmless; `pkill -f 'http.server 8077'`). **Open, the owner's decisions:**
moving the base UA (`JihadUserAgent.h`, `Firefox/52.9` — four sites say what a current ESR
token gains, Cloudflare says what it does not); T-493, the engine rebuild with jemalloc. **Open,
work:** the NS_ERROR_NOT_INITIALIZED thrower (the next probe wraps XHR/storage/crypto/
sendBeacon); the updater's forced-update and network-off tests (R5); the inspector's F-21
(an independent list tokenizer); Cloudflare's challenge, Startpage's, eBay's, old Reddit's gate
and the four ES-module apps stay open/upstream with their causes in the ledger.

**02:00–02:11: daemon 10 on all three** (Mochi's binary had been truncated to 0 bytes by a `cp`
over the running job — "file too short", a respawn loop; the package push repaired it; memory
`hand-started-daemon-and-gc-threshold` item 3). **Measured:** idle with no card 34.8 MB (the
on-demand lists cost nothing); YouTube 163 / Reddit 339 / Bluesky 428 MB with the card; after
the last close the purge brings it to 226 MB and it stays — ~190 MB the purge does not reach.
Component **v9** adds CC → shrinking GC → CC after the last card and logs the compartment count
(a user compartment with no card = a zombie window); the session's `v9.sh` measures it. The
jQuery probe: jQuery loads clean; the NS_ERROR_NOT_INITIALIZED thrower is a later call the
sites make (unnamed, open; the next probe wraps XHR/storage/crypto/sendBeacon).

**Run 2c read** (`docs/MODERN-WEB.md` "Round 2, second run"): the Firefox/128 token made Google,
Bing, Etsy and YouTube render (rows kept in `EngineHost.cpp`, the five that changed nothing
removed; daemon 10); Cloudflare's challenge stops six sites now (it keys on the day's repeated
visits too) and is not a token matter; the API probe found the engine complete but for CSS
`:has()` and WebGL; the benchmark's allocation tests 5–6× faster with the GC fixes; the sites'
stalls are their own scripts. **The open item is memory: the daemon climbed to 589 MB and never
came back after cards closed** — daemon 9/10 purges (heap-minimize + malloc_trim) when the last
card leaves; the session's `memcheck.sh` measures it across YouTube, Reddit and Bluesky and runs
`jihad-jquery-probe.html` (the stack of the `NS_ERROR_NOT_INITIALIZED` thrower). Then: the
packages rebuilt with daemon 10, pushed ×3 (alone on the bus), PICKUP, commit. **The owner's
decision pending:** moving the base UA (`JihadUserAgent.h`, `Firefox/52.9`) to a current ESR —
the four sites say what it gains; Cloudflare says what it does not.

**PAUSED 16:36 (the owner: "pause, youre about to run out of credit"); the round's second run
(2c) then FINISHED on its own** — its log, analysis and shots are committed in
`docs/rounds/round2c-2026-10-15/`; read them first, with the comparison script below. It was run on the device (`tools/host/run-browse-round.sh docs/rounds/round2c-2026-10-15`,
about 45 min from 16:33). If it finished: `docs/rounds/round2c-2026-10-15/{round.log,analysis.md,shots/}`
exist and Mochi's job was restored by the script; compare with the first run by
`python3 -I tools/device/browse-round-compare.py docs/rounds/round2-2026-10-15.log docs/rounds/round2c-2026-10-15/round.log`,
read the probe shots (`probe-apis`: which APIs the engine lacks; `probe-jsbench`: the GC threshold
at 64), judge the UA experiment on the seven bot-blocked sites (Medium, Britannica, Etsy, Bing,
Google, Startpage, eBay) and YouTube — keep or revert the ten Firefox/128 rows in `EngineHost.cpp`
(daemon + packages rebuilt either way only if reverted) — and write the rows and the comparison
into `docs/MODERN-WEB.md`. If it was cut off: the Mochi daemon on the device is a HAND-STARTED one
and the job sits in "stop/waiting" — clean up first (memory `hand-started-daemon-and-gc-threshold`:
kill by /proc cmdline, stop, start, one process), delete `/tmp/shot-r2-*.png` on the device,
re-run the round. The host's test http server on :8077 may still be running (harmless).

**16:24–16:30: the v8 packages on all three variants** (`push-variant.sh` enyo/mochi/mojo, each
alone on the bus, all three first-try; daemon `daf72924` = daemon 8: the UA experiment, the
allocator trim, the card notifications; component v8; GC threshold 64; the Personas guard).
Mochi's job had been left in upstart's "stop/waiting" with a daemon alive outside it after the
hand-started round daemon — killed by cmdline, stop/start, clean. Device: 460 MB free idle.
**The round's second run (2c)** started 16:33 into `docs/rounds/round2c-2026-10-15/`.

**INCIDENT 15:50 (the owner: "the touchpad appears to be hung at 'sorry too many cards'"):** the
second round's first visit took the Mochi daemon to 214 MB resident — the GC allocation threshold
at 160 MB lets the heap grow that far before a major collection, three daemons share 1 GB with
LunaSysMgr — and webOS refused new cards. Recovered: the round stopped, the hand-started daemon
killed, the three jobs stopped (free 59 → 127 MB), cards closed, `luna-restart.sh` (the dialog and
the cards gone), the jobs back, Mochi's 160/192 user prefs removed. **The shipped threshold is
64 MB** (`jihad-platform-prefs.js`, rebuilt). Idle afterwards: ~30 MB free + 190 MB cache — thin:
each daemon holding the lists costs ~27 MB (v6) and all three run at boot. Next: the lists loaded
only while a card is connected and dropped after ten idle minutes (daemon notifications on
connect/disconnect, component v8), then the round again at a safe level.

**The performance work (15:00–15:35):** the GC log apportioned the stalls to NON-incremental full
collections at the engine's 30 MB allocation threshold, and the blocker's 100,000 hostname
strings made each one 3.5× longer. Fixed: `jihad-platform-prefs.js` ships
`javascript.options.mem.gc_allocation_threshold_mb` 160 + `high_water_mark` 192 (measured at
30/96/160 on Mojo, the ledger has the numbers); component **v6** keeps each host set as one
string plus typed-array hash tables (a full collection with the lists: 1,073 ms, was 3,878; idle
memory +27.5 MB, was +40, JS heap 7 MB); daemon 5 `malloc_trim`s on "jihad-blocker-installed"
(it returned pages). **The final packages** (v6, the Personas guard, daemon 5 — which also carries
the UA experiment — and the GC prefs) are built in `out-all/` (15:25) and NOT pushed: the second
round decides the UA experiment first; if it says revert, `EngineHost.cpp`'s ten Firefox/128
rows go, daemon rebuilt, packages rebuilt. **The second round** runs on Mochi (v6, daemon 5, GC
160, the probes first) into `docs/rounds/round2b-2026-10-15/` via `tools/host/run-browse-round.sh`.

**Round 2's first run is read (T-491 done):** 30 rows in `docs/MODERN-WEB.md`, shots in
`docs/rounds/`. **The finding: 7 of 30 visits were stopped by bot detection** (Cloudflare on
Medium and Britannica, DataDome on Etsy, Bing's captcha, Google's /sorry/, Startpage's
"suspended", eBay's error page), YouTube served "update your browser", old Reddit a login gate —
all to the Firefox 52.9–78 tokens the UA table sends. **The experiment (daemon build 4,
`EngineHost.cpp`):** a Firefox/128 token for those ten hosts only (google, bing, youtube, ebay,
reddit edited in place; medium, britannica, etsy, startpage added), to be measured in the second
run before the base UA (`JihadUserAgent.h`, `Firefox/52.9`) is touched — the owner's decision.
Performance: every page stalled the main thread 9–43 s (webos-ports.org 125 s); the JavaScript
speed of the ARM build (`jihad-jsbench.html`, in the verification run) decides what to do about
`dom.max_script_run_time`. One engine API throws `NS_ERROR_NOT_INITIALIZED` to page scripts on
four sites (`jihad-apis.html` probes it, the second run's first visit). Jihad's own
`jihadPersonas.js` logged "can't access dead object" when a card closed: guarded.

**Round 2 (T-491/T-492, the owner 2026-10-15: "browse the modern web as a pretend user for about
30 minutes. document any issues you find and then tackle them … also address any performance
issues you encounter"):** `tools/device/browse-round.sh` + `browse-round-sites.txt` (30 deep
URLs, 28 min of dwell) run through `dev-sh.sh -t 2400` in Mochi with the console on all sources;
one `@@` line per visit (load-done, paints, refusals, errors, first paint, load time, fps,
worst tick, RSS, load average) and a shot `/tmp/shot-r2-<name>.png`; `tools/device/
browse-round-analyse.py <log>` makes the ledger table and the issue list by the kit's thresholds
(cavekit-modern-web.md R6). Runs after the final package push; its "Round 2" section goes into
`docs/MODERN-WEB.md`, then the fixes.

**Device:** Mochi runs the new daemon + blocker (dev push); Enyo and Mojo the packaged state;
Mojo and Mochi profiles carry `dom.max_script_run_time=60` as user prefs; all variants at the
Default look. **Open:** the retest, w3.org/GitHub console output, the memory and timing numbers
(R6, R3), the package rebuild with everything (`build-all-device.sh --skip-arm` then
`push-variant.sh` ×3), the inspector's review (T-490), the hero, the commit.

## 2026-10-15 delta — the README rewritten with new screenshots (READ FIRST)

**The owner (2026-10-14 evening, the session the PC crash cut off):** "In the readme there should be
one slogan only: 'UXP inside, webOS alive, inshallah!'. Simplify the technical sections. The sort
order in text and images should be oldest to newest Mojo -> Enyo -> Mochi. Take all new screenshots
and make the document shorter, mostly featuring things atlas browser and isis browser cant do, and
as a member of the family of UXP browsers and projects"; "less emphasis on touchpad, its supposed to
work on all webos devices but only touchpad is supported for now"; "dont mention HP unless necessary
for attribution". Recovered 2026-10-15 from transcript `874e71a1` (it ended at the Mojo diagnosis;
its ten raw shots died with the device's reboot).

**Done:** README.md 278 → 178 lines: one slogan, "for webOS" with the TouchPad and TouchPad Go named
as the only supported devices so far, Mojo → Enyo → Mochi everywhere, HP only in the attribution
lines (isis-project, the SDK's terms, the trademark note), the UXP-family framing (engine, XPI
add-ons, Personas, Pale Moon's Settings panes, the Phoebus-based add-on site), the "what only Jihad
does" sections (Flash + modern web, add-ons and Personas, Settings and about:config, own NSS),
the TLS history table, its sources and the Acid3/html5test numbers dropped, Building cut to the four
commands. The per-distribution package table moved to `docs/DEVICE-BUILD.md` "To build" (it used to
point at the README for it). `docs/screenshots/`: five images composed by `readme-art.py` from
fresh raw shots — `cardview.jpg` (Mojo on Wikipedia's webOS article, Enyo on webosarchive.org
wearing Free Palestine, Mochi on Wikipedia's Palm article wearing Aurora Silk), `flash.webp`
(Bloxorz full-page in Enyo), `addons-personas.webp` (Mojo about:addons, Mochi about:personas),
`settings.webp` (Enyo Privacy pane, Mochi about:config), `tls.webp` (howsmyssl in Mojo); the
Persona strip, personas-install, customize and standards images deleted; CREDITS.md's screenshot
sections rewritten (Space Fantasy and Linen Light show in the gallery shot, CC BY-SA 3.0).

**Traps met:** (1) Wikipedia has no "Goanna (software)" article (a "no article" page was shot
first). (2) howsmyssl.com now rates the client "Improvable" for the missing post-quantum key
agreement; its Version box still says Good, TLS 1.3 — the README says exactly that. (3) Wearing a
Persona from a script: the manager wears `usedThemes[0]` when `lightweightThemes.isThemeSelected`
is true (UXP has no selectedThemeID), but the CARDS read `jihad.theme.current` (the `cardTheme()`
JSON with the picture as a `file:///…/profile/jihad-themes/<hash>-header.jpg` URL) through
`getTheme` at start — set all three with `tools/device/set-prefs.sh`, or the toolbar stays plain.
(4) The about:config warning page: `general.warnOnAboutConfig=false` for the shot. (5) After the
device's reboot its Wi-Fi came up disabled; the owner switched it on. (6) Launch with
`{"target": url}` after closing every Jihad card (`applicationManager/close` per processid), or a
relaunch lands on the existing card.

**Device:** both variants back to the Default look (`isThemeSelected` false, `jihad.theme.current`
dropped, Enyo's list order restored), Mochi's `general.warnOnAboutConfig` dropped, every card
closed; nothing installed; daemon `06e75a68` on all three, `/` read-only. novacomd was started by
the owner after the PC reboot.

**Post-quantum TLS, deferred by the owner (2026-10-15): "forget the post quantum for now, I will
wait and see if upstream adds it by december."** The facts: UXP ships NSS 3.90.14 (3.90 ESR + backports);
X25519MLKEM768 (group 0x11EC) needs NSS ≥ 3.10x, default since NSS 3.118 (NSPR 4.37); UXP's PSM
hard-codes the offered groups (`security/manager/ssl/nsNSSIOLayer.cpp:2364`). howsmyssl.com rates a
client with no PQ group "Improvable" since 2026-06-14 and **"Bad" from 2026-12-14** (its
`client_info.go`). If upstream has not moved by then, the plan: cross-build NSS ≥ 3.118 + NSPR 4.37
for ARMv7 and swap the ten NSS/NSPR `.so` files the bundle ships separately (`make-device-bundle.sh`;
the export tables carry no UXP additions, so a drop-in should hold), plus a ~3-line PSM patch on the
engine branch (group 4588 first, `SSL_SendAdditionalKeyShares(fd, 1)`) and a libxul rebuild; test
on the desktop build first. Or swap the README's howsmyssl image before December.

**Open:** unchanged from below (the engine push, R11 criterion 4's VM run — the VM was in /tmp and
is gone with the reboot —, the finger round). `docs/research/persona-licences.md` line 454's
"retake cardview.jpg, personas-strip.png…" note is now done (strip retired).

## 2026-10-14 (evening) delta — public on GitHub; the licensing audit; R11 on a Fedora VM (READ FIRST)

**The owner:** "Fix it so everyone can clone and build (redistributable webos app)", "Resolve R11 with
qemu", "dont ship any secrets in the code including login names", "ensure that all software patent
and trademark licenses in the repo are being respected".

**Public.** The owner pushed the engine branch `jihad/uxp-RB_20261006` (`706d4782`) to
`NotAlexNoyle/UXP-Jihad`, then this branch and `main` (fast-forward, 268 commits) to
`NotAlexNoyle/Jihad-Browser`, both at `62df604c`. A clone from GitHub passes
`tools/host/check-pins.sh`: eleven of eleven pins fetch from their public URLs. Every `git push` is
the owner's: the permission classifier refuses them here (memory `pushes-are-the-owners`).

**No login names.** `deploy/relay/` scripts read `/home/<admin>`; the builder's home in five docs is
`~`. The owner's address stays as the author line of the 39 engine patch files (git's record).

**Licensing** (impl-review-findings-6.md "Licensing audit", commit `62df604c`): a read-only audit
of what the packages carry found no blocker. Closed: NOTICE names the bundled runtime (glibc 2.23
as crosstool-NG modifies it, the GCC 9.4.0 runtime with the Runtime Library Exception, eight Debian
jessie libraries, the FreeType disclaimer, Lato, the hyphenation patterns) with a corresponding-
source paragraph; `licenses/` carries LGPL-2.1, GPL-3.0, the GCC exception and Debian's copyright
files, and every package ships all of it plus CREDITS.md (payload preflight checks three of them);
NOTICE names the modified engine's source; README and LICENSE name the component licences and the
live Moonchild policy pages; Apache 4(b) change notices on 29 HP-derived files; DuckDuckGo's icon
is a drawn magnifier (`list-icon-search.png`), Bing's logo gone; the engine's `services-sync.js`
defaults are blanked at bundle time (`make-device-bundle.sh`, checked: 7 palemoon.org lines → 0);
"Blazer" keyword and the Goanna-mark tagline gone; stray files removed. Patents: nothing encumbered
built or shipped (ffvpx = VP8/VP9/FLAC; no libavcodec, H.264, AAC, AV1, Widevine). **Deferred to
the next engine build** (the patches are commits on the engine branch): about:credits' unresolvable
URL (patch 0010) and `--disable-ffmpeg`. **To confirm with the owner:** CREDITS.md says the logo
was supplied by them in 2026 as the project's own.

**R11 on another host (criterion 4), QEMU/KVM:** Fedora 44 Cloud, SELinux Enforcing, user
`jihadbuild` uid 1001, podman 5.8.1 (`scratchpad/vm/`: `run-vm.sh`, `seed.iso` with the
cloud-init user, `guest-build.sh`, `start-git-daemon.sh`; the guest clones this checkout over a
host `git daemon` at 10.0.2.2 and the submodules from GitHub; the eight device libraries copied
in). It found two clean-clone gaps the uid-1000 PC never could, both fixed and committed:
- `--userns=keep-id` keeps the image's `USER builder` (1000) as the process, so a uid-1001 user
  could not write the mounts: podman now maps the user onto builder
  (`keep-id:uid=1000,gid=1000`) for the two images that set a USER (`a5acfd50`).
- crosstool-NG's 2022 mirror list no longer serves zlib 1.2.12: `toolchain/tarballs.manifest`
  (18 tarballs, sha256, URLs) and `fetch-tarballs.sh` run before ct-ng, which may no longer
  download (`72ef295f`, `f8abe74e` — the first commit had the two files ignored by
  `toolchain/*`, a trap).
The third run is building (toolchain at "Installing C library" at the 14-minute mark; the engine
follows). When it ends: compare its daemon, impls and packages with this checkout's; criterion 4
`[x]` with "Fedora 44, SELinux Enforcing, uid 1001" as the evidence; then `sudo poweroff` the guest
(`ssh -i scratchpad/vm/id_vm -p 2222 jihadbuild@127.0.0.1`) and stop the git daemon
(`kill $(cat scratchpad/vm/git-daemon.pid)`). The VM lives in the session scratchpad (/tmp): a
reboot loses it.

**Device:** the three packages rebuilt with the licensing payload are being pushed (enyo done,
mochi and mojo following); a check of impls, shims and a page load in each follows.

## 2026-10-14 (round 25) delta — the inspector's review of Tier 84, fixed, on the device; the clean clone matches (READ FIRST)

**Done this session:** the inspector reviewed `b37795fb..9ff7d71a` (REQUEST CHANGES: 2 P1, 7 P2,
16 P3; round 25 in impl-review-findings-6.md, every finding with its disposition). All fixed in
the code except P1 #1: **the engine pin `706d4782` is not on the public fork** (`jihad/uxp-RB_20261006`
was never pushed); `tools/host/check-pins.sh` shows it FAIL and the other ten ok. **Pushing that
branch is the owner's decision — ask.** The adapter impls were rebuilt with `-DQT_NO_DEBUG` (no home
path any more, 178,196 bytes each; the shims unchanged, `BrowserAdapterJihad.so` `4f603df3`), and
`build-all-device.sh --skip-arm` ran end to end: `out-all/` holds exactly this run's three packages
(enyo 1.0.4 `7bd178a1`, mochi `19f5a5bd`, mojo `ccfb5b58`). The Relay's two Squid scripts are in
`deploy/relay/` with a README (T-478 done; the server's hashes match).

**The clean clone** (`Jihad/.jihad-cleanclone-test`, now at `59040faf`): its engine and daemon built
from its own inputs; its adapter first failed on dangling symlinks because four submodules had
empty working trees from the aborted first clone (restored with `git submodule update --init
--force`; the build script now catches that state). With the fixes pulled, `build-all-device.sh`
ran end to end there: **shims (3), impls (3: `1bafcc0c`, `4b8c44ba`, `a5c205d1`) and the daemon
(`06e75a68`) byte-identical to this checkout's**, the three packages' 2,235 / 2,485 / 2,179 files
identical but libxul (its build id: `platform.ini` BuildID and `jihad-bundle.id` follow it) and
Mochi's BUNDLED-VERSIONS date; the control scripts identical. R11's "path-independent builds" is
`[x]`. The clone (several GB) can be deleted, or kept for the next R11 check.

**On the device:** the three packages pushed with `push-variant.sh` (enyo, mochi, mojo, the real
postinst each). `/usr/lib/jihad/<v>/BrowserAdapterImpl.so` is the round-25 impl in each (md5s above,
no home string), the app copies the same, the shims unchanged (`4f603df3`, so no reboot was needed),
daemon `06e75a68` on all three, `/` read-only. Each variant launched on https://example.com/ loaded
("load done", the title) and painted. The cards were closed by the check script's close call.

**Still open:** (1) the engine push — `jihad/uxp-RB_20261006` (`706d4782`) to the fork, the
owner's decision, then `tools/host/check-pins.sh` and R11 criterion 2 `[x]`; (2) R11 criterion 4
wants one run on an SELinux host or another uid; (3) the finger round (deferred). The server: the
confirmation `46-intention-quota` sent, no reply needed; nothing pending there.

## 2026-10-14 (later) delta — Tier 84: one repository builds Jihad; the PC rebooted mid-session

**The owner:** "is everything for jihad browser now centralized to …/Jihad-Browser or is there still
code out of tree for the build etc?", then "yes. and try to integrate some of the host tools into the
readme instructions for building jihad. it should be as self contained to its own repo as possible
while running on a wide variety of linux distros", and "use git submodules for build dependencies
for automation". Kit: cavekit-device-build.md R11; build site Tier 84 (T-471…T-479).

**Built** (`045c0fef` … `9ff7d71a`, nothing pushed):
- T-471 `045c0fef`: the build scripts read only the repository (the `../toolchains`,
  `../mochi-sampler`, `../../webos-stacks` fallbacks and the checkout-folder paths are gone;
  `build-webos` lives inside, git-ignored). Mochi's BUNDLED-VERSIONS names repository-relative
  paths, not the builder's home.
- T-472 `61fe5974`, `f94af9c9`, `9ff7d71a`: submodules `third_party/{crosstool-ng, libpbnjson, yajl,
  npapi-headers, webos-sdk-redux (shallow), pulseaudio (shallow)}` and `build/webos-oe/build-webos`
  (update=none); Jihad's own headers in `build/webos-oe/adapter-include/`; `host-path.sh` puts the
  SDK submodule's `palm-*` and `tools/host/bin` first on PATH; `gen-adapter-deps-manifest.sh
  --fetch-qt4 / --fetch-device-libs`; `gen-sysroot-manifest.sh --fetch-device-libs / --assemble`
  applying `arm-sysroot.recipe` (glibc 2.19 removed, `/lib` copied into `/usr/lib`, six symlinks,
  PulseAudio 0.9.22's headers) and refusing a tree whose digest differs; `toolchain/build.sh` builds
  crosstool-NG from the submodule.
- T-473 `e1ae67c0`: `fetch-pdk.sh` downloads HP's `palm-sdk_3.0.5-svn528736-pho676_i386.deb` from
  the Internet Archive (item `webOSLegacySDK`) and checks its sha256.
- T-474 `00a82f35`: `host-tools/Dockerfile` (32-bit libc, Java, binutils), `in-container.sh` (the
  repository mounted at its own path; `--arm` for the engine/daemon container), `build-all-device.sh`
  as the single entry point that fetches what is missing.
- T-475 `e3a5f4fc`: `tools/host/build-novacom.sh`. T-476 `e0f38f07`: `tools/device/`,
  `tools/test-servers/`, `tools/review/`. T-477 `5cace42e`: README "Building", DEVICE-BUILD.md,
  TOOLCHAIN.md.

**Verified, byte for byte:** the adapter (gcc 4.3.3, and the gcc 9 alternate) from the submodules,
in the container too; the three packages identical in content, host and container; the daemon
`06e75a68` through `in-container.sh --arm`; the Qt4 headers and the seven device libraries equal to
their pins; the sysroot assembled by the recipe digest-identical to the tree every build used; the
PDK from the Internet Archive identical to the copy in use; the toolchain rebuilt from the submodule:
same version string, identical compiler output, byte-identical `libc-2.23.so`, `ld-2.23.so` and
`libstdc++`, 24 of 5,652 files differ (timestamps, generated headers, logs).

**The clean-clone run** (`git clone --recurse-submodules` of this repository into
`Jihad/.jihad-cleanclone-test`, then `build-all-device.sh`; logs `Jihad/.jihad-cleanclone-build*.log`):
- The device phase matched all eight libraries. Then 32 of the 187 sysroot packages answered 404:
  archive.debian.org's CDN serves cached 404s for files the pool lists (trap below). `9ff7d71a`
  retries with a cache-buster; all 187 then came and `--check` passed.
- The engine build (hours) had run seven minutes when the PC went down (08:06 local). **Resumed at
  09:45 local, detached** (`setsid nohup bash build/webos-oe/build-all-device.sh`, log
  `.jihad-cleanclone-build3.log`); `mach build` continues from the object directory. When it ends,
  compare the clone's daemon, adapter and packages with the ones here (daemon `06e75a68`, adapter
  `4f603df3`, `build/webos-oe/out-all/`) and record the result under T-474 and R11. libxul need not
  match byte for byte (build paths); the daemon and the adapter must.

**The reboot (between 08:02 and 09:22 local) killed** the clean-clone build (resumed), the
inspector's Tier 84 review (its transcript stops mid-read, no verdict; relaunched at 09:46 on
`b37795fb..9ff7d71a`, T-479), and the `wormhole-rs send` of the Relay-scripts request (code
`76-almighty-briefcase` is dead; **resent as `11-informant-upshot`**;
`~/Documents/jihad-request-2026-10-14.tar` is the copy). T-478 asks the server agent for
`~/squid-hardening/jihad-splice.sh` and `hide-client-addresses.sh` unchanged, with sha256s, for
`deploy/relay/` with a README.

**Device:** read-only gets over novacom only (the eight libraries, twice); nothing installed; all
three variants on daemon `06e75a68`, `/` read-only, as the night delta left it. After the reboot
novacomd runs from `/usr/local/bin` (the repository's `tools/host/bin` one is the same source).

**Not committed, on purpose:** `.cavekit/config` carries a JSON block (session_budget) the owner
added; left alone.

**Open:** the clean-clone build's end (T-474's last box); the inspector's verdict and fixes (T-479);
the server's reply (T-478); the finger round (deferred, by the owner's word).

## 2026-10-14 delta — the site's round-24 update is live; T-470 done

**The server's reply** (`4-souvenir-bluebird`, kept as `~/Documents/jihad-reply-2026-10-14.tar`,
with the full deploy log): `f9bcb79` went live 2026-10-14 10:26 UTC through deploy-site.sh;
`www.prev` is `d9359ea`.
- install.sh: `… addons.riverstonerelay.org.conf is already the repository's; its hash is kept in
  /var/lib/riverstone-addons/site-block.sha256`. No `.new` or `.prev` written.
- The import: the five MPL Personas `updated (1.0), its pictures as they were`; the other 34
  unchanged.
- Smoke test on the server (`RS_SMOKE_CONNECT=127.0.0.1:443`): 43/43, exit 0. 39 Personas listed,
  0 hidden; the fail2ban jail empty.
- The feed check: `39 of 39`; Aurora Silk `https://mozilla.org/MPL/2.0/
  https://addons.riverstonerelay.org/credits/personas.txt 1.0`.
- Not tested on the server: the panel redirect and login timing (the panel's allowlist answers
  403 first) and the 503 (the live database was not stopped). The PC's PHP 8.4 tests stand.

**Checked from the PC** (through Cloudflare, no `RS_SMOKE_CONNECT`):
- The smoke test: 43/43, exit 0.
- The feed: 39 of 39, and every Persona's `homepageURL` equals its `sourceURL` (by design in
  classPersona.php). The server thought the other 34 had an empty one; the live feed shows none
  empty.

**Recorded:** T-470 DONE in build-site.md; cavekit-addons-site.md R5, R6 and R10 evidence and
changelog. R10's smoke criterion stays [~]: a colour Persona worn in Pale Moon and on the TouchPad
from the live site is still the owner's.

**To the server:** `jihad-confirm-2026-10-14` (CONFIRM.md: confirmed, homepageURL is meant, delete
this round's incoming and outgoing folders, keep `www.prev`). Wormhole code **6-resistor-dragnet**,
received 10:42 UTC; no reply needed. Copy in `~/Documents/jihad-confirm-2026-10-14.tar`.

**Device:** not touched today; as the night delta below left it.

**Open:** the finger round (docs/FINGER-TESTS.md; deferred, by the owner's word). Nothing is
pending on the server. Codex owes no review.

## 2026-10-13 (night) delta — review round 24: the inspector does Codex's owed review

**The owner:** "do the codex review with the cavekit 3.1 backup inspector agent instead. defer all
finger tests. remove leftover mojo file." Codex is no longer owed anything: T-456 and T-466 are
done by the inspector.

**Reviewed** (five inspectors in parallel, read-only):
- Jihad: `4e480610..6e7b31a0` (Tier 78), `6e7b31a0..5db7860c` (round 22), `5db7860c..9a38a708`
  (Tier 83).
- The site: `9e5521b..d9359ea` (deploy kit and credits) and `upstream/TRUNK..d9359ea` (the PHP fork).

**Results.** Tier 83: REQUEST CHANGES (1 P1, 1 P2, 5 P3). Everything else: APPROVE with P3s. All
fixed:
- **Jihad, `cfbfbcf2` (T-469).** Details in impl-review-findings-6.md, round 24.
  - **P1:** a router's certificate accepted with Trust Always lands in Certificate Manager's store
    and says cA=TRUE. R4 would have made it a root for every site at the next start. Now R4 takes
    only roots: self-issued, keyCertSign, no subjectAltName or extendedKeyUsage. It also never takes
    a certificate the daemon's SSL dialog installed: the daemon raises `jihad-platform-cert`, and
    the SHA-256 goes into `jihad.security.webos_certs.accepted`.
  - **P2:** a blob: frame raised the CA question. The question now needs a one-use offer from a CA
    certificate opened as the page itself.
  - **Also:** the proxy-login marker is now "[proxy] ". webOS's proxy setting keeps its last plan
    when a read fails. The gallery offers the bundled set. Link buttons are 44 px. The kits are
    brought up to date.
  - **Rejected:** matching removed roots by subject and key would refuse GTS R1–R4, GlobalSign and
    Firmaprofesional (in dead-ends.md).
- **The site, `71dd173` and `f9bcb79` (T-470).**
  - install.sh keeps an owner's edited site block. DEPLOY.md step 11 is corrected.
  - The MPL Personas have their licence and source links.
  - The panel's open redirect is closed, and a database outage answers 503.
  - The feed's order and per_page are fixed, and login timing no longer reveals user names.
  - Local copy: smoke test 43/43, and each fix checked.

**On the device** (CE 3.1.0; all three on daemon `06e75a68`, `jihadCerts.js` `7ee7ae12`):
- R4 still trusts the same 5 roots.
- Three certificates placed in Certificate Manager's store:
  - a router's with an IP SAN: skipped as siteLike;
  - one without keyUsage: skipped as noCertSign;
  - a CA with keyCertSign: trusted. Removed again afterwards.
- Trust Once on a router-certificate site logged the marker, and the session serial was swept.
- A blob: CA frame: refused, no dialog.
- The test CA as an HTTP link: asks ("Offered by 192.168.1.245:8896").
- A router certificate as a link: refused.
- Proxy logins read "The proxy …" in Enyo and Mochi; Mojo's sheet reads "[proxy] …".
- The Enyo hook gained `D` (report open dialogs) and `VN` (answer the SSL dialog).

**Not shown on the device:**
- The gallery's bundled offer: every profile already has edition 8, so only a fresh profile shows
  it.
- jihadProxy's keep-last: a failed read cannot be brought about on demand.
- The 44 px buttons: in the finger round.

**The server:** the site update is sent as `jihad-site-2026-10-13c` (SITE.md plus
`riverstone-addons-f9bcb79.tar.gz`, sha256 `13b43fe2…`), wormhole code **5-stupendous-solo**,
received 15:36. The
reply should give install.sh's site-block line, the import of the five MPL Personas (updated at
1.0), the smoke test, and the feed check (`39 of 39`, Aurora Silk's links).

**Device state as left**
- All three on daemon `06e75a68`, no proxy, no hooks, `/` read-only.
- Mojo's leftover `app/assistants/JihadSelfTest.js` (evening delta) is removed: no `*SelfTest*`
  file in any of the three.
- Certificate Manager's store as before: 0 files in /var/ssl/certs, 190 links.
- The test fingerprint pref is cleared, and the device's /tmp test files are deleted.
- LunaSysMgr restarted.

**Open:** the server's reply to `5-stupendous-solo` (T-470), then the finger round (deferred, by the
owner's word).

## 2026-10-13 (evening) delta — the Relay hides users' addresses; T-465 withdrawn

**The owner:** "I do want the relay to hide users addresses, and I want T-465 fixed."

**The Relay and users' addresses (T-468, waiting on the server):**
- Measured earlier today: through the Relay, http://httpbin.org/ip showed the user's home address
  ahead of the server's. Squid's `forwarded_for` is at its default, so `X-Forwarded-For` names the user
  on plain http, and on every https request it re-signs for other webOS clients. Jihad's spliced
  https was never affected.
- Sent as `jihad-relay-2026-10-13b` (RELAY2.md plus `riverstone-addons-d9359ea.tar.gz`, sha256
  `ca773d7a…`), wormhole code **18-hazardous-classroom**, received 13:35. It asks for
  `forwarded_for delete`, with `curl` checks before and after (from the server the client is
  127.0.0.1). It also asks whether the archive peer on :3130 uses the header, and what Squid's access
  log keeps about clients (report only; the owner decides). The site update (`d9359ea`, install.sh's
  site-block line) rides along.
- **The reply** (`~/Documents/jihad-reply-2026-10-13c.tar`, code 7-repellent-eyeglass):
  - `forwarded_for delete` is live since 18:12 UTC (`squid.conf` line 103, above `cache deny all`).
    Parse was clean, and Squid was reconfigured in place.
  - From 127.0.0.1, plain http and bumped https went from `127.0.0.1,116.202.108.111` to
    `116.202.108.111`; spliced (Jihad's UA) was unchanged.
  - Rollback: `doas bash ~/squid-hardening/hide-client-addresses.sh --undo`, or the backup
    `squid.conf.bak.2026-10-13-xff`.
  - The archive (:3130) and setup (:3129) servers never read the header and log nothing.
  - `d9359ea` is live (18:13), with install.sh's new line printed; smoke 43/43. `www.prev` is
    `68caed1`.
- **On the device (Enyo, the Relay chosen):** http://httpbin.org/ip shows only `116.202.108.111`, and
  httpbin still sees Squid's `Cache-Control` (so the page went through the Relay). R2's criterion
  and T-468 are closed. Enyo is back on no proxy.
- **The access log: the owner chose to keep it as it is** (option 1, 2026-10-13). Nothing goes to
  the server. What it holds: Squid's default `access_log` records
  every client's address (field 3) with time, URL (a bumped https request's full URL; a spliced one
  only `CONNECT host:443`), status and bytes. `cache.log` names addresses in some errors.
  - Rotation is by size: logrotate hourly, `size 4500M`, `rotate 2`. That keeps roughly a week to
    ten days on disk (~2 GB a day).
  - The server's abuse handling runs on those addresses: the bans of 06-08, 07-18 and 08-24 came
    from them.
  - Options:
    - keep as is;
    - shorten it, e.g. rotate daily and keep 1–2 days;
    - Squid's `client_netmask 255.255.255.0`, which logs only each client's /24 (abuse handling by
      network, not by address);
    - no address at all.

**T-465 withdrawn: not a redraw bug.**
- Both "old picture" sightings were screenshots taken before the pick ran. `card-selftest.sh run`
  shoots `seconds` after the launch: `page:w10;587,595;w2;s2;w6` with 30 s put the pick at about 30 s.
- Replayed exactly on today's build: the 30 s shot shows the old state, and the pick is logged after
  it; the same steps with 40 s show the new one.
- 15 of 15 picks given time redrew within a second: Enyo and Mochi; No proxy ↔ Relay, My own proxy →
  Relay, and the original Mochi case (own proxy saved, page scrolled deep, shrinks on the pick). The
  adapter's paint log showed every `msgPainted` followed by a paint.
- **`card-selftest.sh run` now prints the shot's UTC time and marks hook lines logged after it**
  (`(after the screenshot)`). If the step a picture should show is marked, the picture is from before
  it. Trap recorded in `context/impl/dead-ends.md`.

**Device state as left**
- All three on the same build (daemon `1a831adc`). No proxy, the adapter paint logs off (gates and
  logs removed), `/` read-only.
- Enyo and Mochi hooks restored (LunaSysMgr restarted).
- **Mojo still has `app/assistants/JihadSelfTest.js` on the device**, from the 2026-10-13 morning
  login test. It is not loaded (its `sources.json` names no hook). The earlier "no test hooks" check
  missed it: BusyBox `find -name '*selftest*'` is case-sensitive. My `rm` was refused by the
  permission classifier; it goes with the next Mojo install, or `card-selftest.sh mojo restore`, or
  the owner removes it.

**Open:** Codex on Tier 83 after Oct 18th, 11:59
AM (T-466, now including `card-selftest.sh` and the site to `d9359ea`); the finger round.

## 2026-10-13 (later) delta — Tier 83: proxies and webOS's certificates

**The owner:** "add support for riverstone relay proxy, and webone, and self hosted proxies in jihad.
also add support for system certificates including community patches if installed." Their answers:
- webOS's certificates: "Added + patched" (not HP's stock roots, not Mozilla-removed ones).
- The Relay: HTTPS passes through untouched. Later the same day: "make using riverstone relay ca an
  option if riverstone relay proxy is installed but off by default".
- The Relay is open (no login).
- A proxy's CA: ask when it is opened.
- Default: No proxy.

The kit is `context/kits/cavekit-proxies-certs.md`; build site Tier 83 (T-457..T-467).

**Built** (Jihad `e223591f`, `9394c08b`, records `b33d5524`; nothing pushed)
- **Settings → Advanced → Proxy:** No proxy (now the engine default too), Use webOS's proxy setting,
  Riverstone Relay, WebOne, My own proxy (HTTP/HTTPS/SOCKS, a direct list), Automatic configuration
  URL.
  - The engine's "system proxy" (type 5) does nothing in the headless build and is gone.
  - webOS's setting: the daemon reads `com.palm.connectionmanager/getNwProxiesConfig` on its own Luna
    connection at start, at every card connect, and when the section is shown.
    `components/jihadProxy.js` applies it.
- **Settings → Security → Certificates** (`components/jihadCerts.js`): at the first page the daemon
  raises `jihad-nss-ready`, and the v3 CAs (basicConstraints cA=TRUE) in webOS's stores are trusted in
  memory, except:
  - HP's 3.0.5 roots;
  - roots Mozilla removed;
  - roots Jihad already has;
  - expired ones;
  - any squid-for-webos CA.

  The lists come from HP's doctor and CCADB (`render/goanna/certs/make-cert-lists.py`). On the CE 3.1.0
  TouchPad it trusts the 5 roots newer than Jihad's NSS (TrustAsia ×2, OISTE ×2, SwissSign 2022-1).
- **The Relay's CA:** pinned in the component; trusted only with the switch on and the Relay in use.
- **Opening a CA certificate asks on the card** (a JS `nsICertificateDialogs`). The question names who
  offered it and only appears for the page itself, never a frame. WebOne's download-marked CA opens
  too. Added CAs are listed with Remove.
- **A 407 names the proxy** (Enyo and Mochi say "The proxy …"; Mojo's webOS sheet is titled "proxy …").
- **Test tools:** `render/goanna/test/jihad-test-proxy.py` (logging proxy; `--auth`; `--mitm` with a
  test CA, serving `/test-ca.crt` and `/proxy.pac`). The Enyo hook gained `W<user>:<pass>` (answer the
  login dialog).

**Verified on the device** (CE 3.1.0; Enyo throughout, Mochi and Mojo for the import, the sections
and a login proxy; kit evidence per criterion):
- **Certificates:** the five roots trusted; nothing written to cert9.db; the SwissSign 2022-1 test site
  loads with the switch on and fails off (TLS sessions cleared through the daemon); a CA in
  Certificate Manager's store trusted after a start and gone after removal; a v1 self-signed
  certificate skipped.
- **Proxies:** manual, PAC, webOS's setting (set with `configureNwProxies`, then cleared), a login
  proxy, WebOne 0.18.3 (PC container; its CA from its link, then https through it), and the Relay
  (re-signing; fails with its CA off, loads on).
- **Refusals:** the Relay's CA from a link, a v1 certificate, certificate frames; "Offered by" shown.

**Review round 23** (inspector; Codex out until Oct 18th): REQUEST CHANGES, with 1 P1 (v1 self-signed
taken as a root), 4 P2 and 10 P3; all fixed or noted in `9394c08b`, and device-checked. The site's 3
P3s are fixed in `68caed1`. Details in impl-review-findings-6.md, round 23.

**Found on the way**
- JS components are read as **Latin-1**: non-ASCII in their strings came out garbled (the print
  status's "…", the Persona importer's BOM strip). They are escaped now. **Use `\u` escapes in
  components.**
- Reinstalling over an adapter LunaSysMgr had mapped left `/` read-write (a deleted file still open).
  The install scripts now leave a byte-identical adapter alone; Mochi, Mojo and Enyo pushes all left
  `/` read-only.
- **Open, T-465:** after picking Riverstone Relay in a card's drop-down (the page grows), the card
  keeps its old picture, its own list included, until the next touch (Enyo, Mochi). The daemon did
  paint. A page growing by itself redraws fine (`/media/internal/jihad-grow.html`, kept for this).

**Server** (Riverstone-Addons `c322904`, `68caed1` on `jihad`, not pushed)
- The server's round-22 reply (`~/Documents/jihad-reply-2026-10-13.tar`): `057dbad` live, smoke 43/43,
  migrate silent. Its two suggestions are done:
  - install.sh replaces an untouched site block in place (a stamped hash), `.new` only for an edited
    one;
  - DEPLOY.md notes `default_server`.
- Sent as `jihad-relay-2026-10-13` (RELAY.md plus `riverstone-addons-68caed1.tar.gz`, sha256
  `df511346…`), wormhole code **7-consensus-backward**; received 12:52 (the sender exited 0 after
  connecting directly to the server). It holds:
  - the Squid rule `acl jihad_browser browser JihadBrowser/` + `ssl_bump splice jihad_browser`
    above the bump rules, with checks;
  - the site update.
- **The reply** (`~/Documents/jihad-reply-2026-10-13b.tar`, code 7-performance-bedlamp):
  - The Relay rule is live since 17:07 UTC: `squid.conf` lines 31–33, above `ssl_bump peek step1`.
    `squid -k parse` is clean, and Squid was reconfigured in place (pid 2631). Through the Relay,
    example.com's issuer is Cloudflare TLS Issuing ECC CA 3 for `JihadBrowser/1.0` and `squid-proxy`
    for any other agent. Rollback: `~/squid-hardening/jihad-splice.sh --undo`, or either
    `squid.conf.bak.2026-10-13-jihad*`.
  - Note for later checks: `curl -v` prints `issuer:` only after a good handshake, so the
    bumped case needs `-k`.
  - `68caed1` is live (17:06), smoke 43/43, the catalogue unchanged. `site-block.sha256` was
    written, equal to the live block. `www.prev` is `057dbad`.
  - Its suggestion, a line when the block is already the repository's, is done in Riverstone-Addons
    `d9359ea`, not deployed: it rides the next site update.
- **On the device (Enyo, the Relay chosen, its CA off):**
  - https://example.com/ loads, and its padlock names Cloudflare TLS Issuing ECC CA 3 (TLS 1.3).
  - http://httpbin.org/ip shows `108.52.189.109, 116.202.108.111`; https shows only the Relay's
    address, with no Squid header.
  - R2 is met. Squid forwards the client's address on plain http (`forwarded_for` at its
    default): the server's business, noted for the owner.

**Device state as left**
- All three pushed from the same build: daemon `1a831adc`, `jihadCerts.js` `cfb5cda3`, engine
  unchanged. Daemons running, `/` read-only, no test hooks; LunaSysMgr restarted.
- No proxy in all three, no Jihad-added CAs; the webOS-certificates switch at its default (on).
  webOS's own proxy setting empty.
- Default look in all three.
- Test assets on `/media/internal`: `jihad-modern-web.html`, `jihad-blur-order.html`,
  `jihad-grow.html` (T-465).

**Open** (superseded by the evening delta above: T-465 withdrawn)
- Codex on Tier 83 and round 22 after Oct 18th, 11:59 AM (T-456, T-466).
- The finger round (`docs/FINGER-TESTS.md`, a proxies section added).
- The desktop build on the new engine.

## 2026-10-13 delta — Tier 82: review round 22 (the inspector; Codex out until Oct 18th, 11:59 AM)

**Codex** ran out at 22:32 on all four ranges of `scratchpad/codex/r22.sh` (146k tokens on the first,
nothing written). Next retry: **Oct 18th, 2026 11:59 AM**, then run the same four ranges
(T-456). The inspector reviewed what no one had: Jihad `a90b7fe3..0821484f`, Jihad `23ffd9af` (the
engine update) and Riverstone-Addons `d66fbc5..jihad`. All three APPROVE; every finding was checked
against the code and fixed or noted (context/impl/impl-review-findings-6.md, round 22).

**Fixed and verified on the device**
- **Keyboard-down blur:** `change` now fires before `blur`. Enyo: a test page's title read
  `order:focus,input,change,blur` after the system put the keyboard away (`enyo.keyboard.forceHide`);
  the daemon's window went 942 → 651 → 942.
- **Settings fonts:**
  - Names are stored as Unicode (`nsISupportsString`). Georgia was written to the profile, and
    Restore left no `font.` pref.
  - The 2 px minimum size is sorted into the list.
  - "Noto Sans Thai" is not a phantom: `/usr/share/fonts/HeiT_nb.ttf` is that font, so the filter
    was right and the kit's note was wrong.
- **Deep links and anchors at load:** the daemon sent a page-made scroll before the page's new
  size, so the card clamped it to the top. The size now goes first. Enyo and Mochi open
  `about:settings#pane=content&section=fonts` on Fonts & Colors.
- **about:addons:** only the setting types the manager knows are laid out. The device check caught
  the first version breaking Palefill's checkbox row, which is now fixed.
- **Card pictures:**
  - The gallery swaps only the pictures that changed, so no card is redrawn.
  - The queue restarts itself.
  - Small headers get a `.small` mark, and failed decodes are not retried until restart.
  - Checked by deleting one picture: it was made again, with no error.
- **Mojo:**
  - The keyboard-away mark is tied to the page `openUrl` opened.
  - **New:** a card opened on a page no longer comes up with the keyboard over it. The start page
    still focuses the address.
- **Engine tooling:**
  - `update-engine.sh` refuses to start unless the branch is at `UXP_PIN` and the queue matches it.
  - `--finish` checks first and can be re-run; `--abort` cannot delete the branch once finishing.
  - `pins()` checks every file before writing; `.gitmodules`' branch is now a pin.
  - The checker covers `loadURI`, `ResizeReflowIgnoreOverride` and 60 app units, and reads ICU from
    the pins. `LoadURIPlain` wraps the last positional call.
  - Checks: 0 FAIL, 0 WARN; pins agree; the start path stopped at "already on RB_20261006".
- **R8's missing device checks:**
  - Mochi and Mojo: howsmyssl TLS 1.3 "Good", Palefill 1.30 enabled, about:config, about:settings,
    and a Persona worn and taken off.
  - Enyo: about:addons and about:settings.
- **Install scripts:** one push left `/` rw ("Device or resource busy" right after ls-hubd and db8;
  the later pushes then kept it rw). The restore is now retried for about 10 s; tested on BusyBox.
  All three were pushed again from a read-only `/` and came back read-only.

**Site (Riverstone-Addons `057dbad`, branch jihad, not pushed):**
- Migration 002's key is retried until it holds.
- The importer updates a Persona whose id is taken.
- The smoke test runs end to end on the server and is stricter on the feed.
- :443 answers only `addons.riverstonerelay.org`.
- Tested on the local copy and a throwaway nginx.
- Sent to the server agent as `jihad-followup-2026-10-13` (FOLLOWUP.md, package sha256 `a5097cde…`)
  and **received at 08:02** with code `4-revenue-transit` (direct connection, wormhole-rs exit 0).
  Earlier codes all lapsed unreceived: the public rendezvous server answered 502 and reset
  `9-determine-algol` and `16-hazardous-suspense`; `597-sympathy-checkup` hit its time limit;
  `4-october-fallout` was replaced at the owner's request. Its `install.sh` keeps the server's site
  block, so the host check is merged there by hand (the steps say how).

**Not fixed, noted:**
- Following the same settings deep link again while it is the address does nothing. An about: page
  cannot `replaceState` (UXP's docshell), and replacing the fragment would scroll.
- A desktop test of the fonts section waits for the desktop build on RB_20261006.

**Device state as left:**
- All three pushed from the same build: daemon `7b02e00b`, engine `jihad/uxp-RB_20261006`.
- Daemons running, `/` read-only, no test hooks; LunaSysMgr restarted after the last push.
- Default look in all three; no `font.` prefs; `:has()` off.
- Test assets on `/media/internal`: `jihad-modern-web.html`, `jihad-blur-order.html`.

**The server's reply** (`~/Documents/jihad-reply-2026-10-13.tar`: REPLY.md and the deploy log): `057dbad`
live 2026-10-13 13:10 UTC; smoke test 43/43 twice; the feed 39 of 39; `migrate.sh` silent (the
`uq_browserId` key is there); the host check merged (helper `merge-site-block.sh`, the old block kept
as `.conf.prev`). Step 4 printed 0, but another site's page answered: nginx there has an explicit
`default_server` on :443 (`share.riverstonerelay.org`), so the host check is defence in depth and
never fires. Its two suggestions for the next site package (T-463):
1. install.sh should replace an unedited site block in place (compare with the previous copy's
   version or a stamped hash) and write `.new` only for an edited one.
2. DEPLOY.md should tell the reader to check `nginx -T | grep default_server` on :443.

**Open:**
- Codex on round 22's ranges after Oct 18th, 11:59 AM.
- T-463: the server's two suggestions, in the next site package.
- The finger round (`docs/FINGER-TESTS.md`, three tests added).
- The desktop build on the new engine.

## 2026-10-12 (night, later) delta — Tier 81: the engine on UXP RB_20261006

**The owner:** "Proceed with the UXP update, with a focus on making future updates easier, and
optimizing the code where possible, while supporting modern web standards. target the latest
release".

**Done** (docs/ENGINE.md has the whole story)
- **Engine:** on UXP release `RB_20261006` (was master `b2594a4`, 2026-06-30). Our 39 commits came
  across with no conflict, one commit per patch now (the squash of 0001–0031 was split). The six
  files both sides changed were read; only `--disable-npapi-gtk2` had to go from the mozconfigs.
  `check-engine-api.py`: 0 FAIL, and the baseline was re-written with no signature changes. The
  branch is `jihad/uxp-RB_20261006` at `706d4782ba`, not pushed.
- **Future updates are two commands:**
  - `build/engine/update-engine.sh [TAG]`: puts the queue on the release with a three-way merge,
    lists the overlapping files and runs the API check.
  - `--finish`: writes the queue again, checks out the new branch and rewrites every pin (the user
    agent, start pages and OE recipe; the bundler reads the ICU name from `engine-version.env`).
  - `UXP_TAG` is now in the pins.
  - `render/goanna/EngineCalls.h` puts the 15 fragile positional engine calls behind named
    wrappers.
- **Optimised:** a release build (dead-code removal, identical-code folding) in Thumb-2.
  - libxul, stripped: 46.2 → 30.2 MB (−35%); its code −43%.
  - Payload: 80 → 73 MB per variant.
  - Idle daemon: 33.8 MB resident, against 36 MB on the old engine.
- **Modern web, measured with `render/goanna/test/modern-web.html`:** `adoptedStyleSheets`,
  `replaceSync`, `URL.parse()` and `revert-layer` now work. `:has()` works with its new Settings
  switch, and is off by default as in upstream releases, because it is slow on some pages. TLS 1.3
  confirmed on howsmyssl.com.
- **Fixed on the way:** `postinst`/`prerm` took the initramfs `rootfs` line for a read-write `/`,
  never remounted it, and failed on a read-only root. The generator now reads the real root
  mount. The daemon includes `nsIDocument.h`, which the new `StyleSheet.h` needs.

**Verified on the device** (a full `push-variant.sh` to each)
- All three: "11 of 11 optional jihad_* exports present".
- Enyo: the feature page before and after; `:has()` switched on, then back off; TLS 1.3; a Persona
  worn and taken off; Palefill enabled; about:config; Flash at 23–25 fps.
- Mochi: the feature page. Mojo: Wikipedia.

**Device state as left:**
- All three on the new engine (bundle `71cef1fd…`; libxul from `jihad/uxp-RB_20261006`), daemons
  running, `/` read-only, no test hooks.
- Default look in all three; `:has()` off.
- `/media/internal/jihad-modern-web.html` is left in place as a test asset.

**Rollback:** the old engine build is `build/webos-oe/out-arm/obj-jihad-goanna-arm.RB-b2594a4` (with
`jihad-browserserver-arm.RB-b2594a4`). Put it back as `obj-jihad-goanna-arm`, rebundle,
`build-variant-ipk.sh`, then `push-variant.sh` each variant.

**Open:**
- Codex round 22 at 22:31 (session cron; the queue is kept out of its diff).
- The finger round (`docs/FINGER-TESTS.md`, engine section added).
- The desktop test build is not rebuilt on the new engine yet (docs/ENGINE.md §7).

## 2026-10-12 (late night) delta — Tier 80: fonts in Settings; no Persona by default; card pictures in the background

**The owner's rules this evening (both in auto-memory):**
- All finger tests wait for one round at the end, collected in `docs/FINGER-TESTS.md`.
- No Persona is worn by default in any variant. Tests may wear one, but put every variant back to
  the Default look afterwards.

**Done:**
- **Fonts (T-438, `11f09e07`, preferences-ui R9).** Settings → Content → Fonts & Colors has fonts per
  script, as Pale Moon's and Basilisk's Fonts & Colors and Advanced fonts dialog offer them: the
  engine's own list of installed fonts, a sample line under each, and Restore default fonts. The
  page link `about:settings#pane=content&section=fonts` opens the section.
  - Licensing: no font is shipped. The device's fonts are used in place: HP's Prelude family, the
    Microsoft core fonts, and the CJK fonts webOS carries. The code is our own MPL-2.0.
  - Device, Enyo: Georgia was picked for Serif and the serif text was drawn in it; Restore default
    fonts brought back Times New Roman. The section draws correctly in all three variants.
- **Default look (T-440, `97f5e871`).** The test Personas were taken off in all three variants.
  Fresh profiles already wore none: nothing sets `isThemeSelected`.
- **Card pictures (T-441, `97f5e871`).**
  - The cause: making pictures inside the gallery's own request held it 200–600 ms a card, the
    budget ran out by the eighth, and decodes failed under image-cache pressure. Carbon Light kept
    its full header, the one the owner saw flashing.
  - The fix: a background queue makes them one at a time, takes the size from the decode, writes
    natively and discards each decode.
  - Device, Mojo: all 16 made in one opening; every card drawn.
  - Mojo ignores `palm-launch`'s target parameter, so open its pages through the test hook.
- **Server.** It replied (`41-savagery-shamrock`, kept in `scratchpad/server-reply/`): package
  `5c1d54c` is live, migrations 001 and 002 are in, 24 Personas were imported (39 listed), the
  smoke test passes, and the credits are served. Its notes:
  - the Persona list's column drift: fixed (Riverstone-Addons `f148f1e`, which also records the
    deploy in DEPLOY.md);
  - Free Palestine's `license` stays a sentence, because the site's card shows that field as its
    credit;
  - Free Palestine's id matches in both repositories.
  - Nothing more for the server until Codex round 22.

**Device state as left:**
- All three variants have the same `deviceroot/hl` contents (the chrome scripts are those of
  `11f09e07`; `push-engine-update.sh` prints the id); daemon `644eba8a…`; libxul unchanged.
- No test hooks.
- Default look in all three (no Persona worn).
- Enyo's Western font settings are at their defaults (Restore default fonts was run after the
  Georgia test).

**Open:** Codex round 22 at 22:31 (session cron), then the finger round when the owner calls it.

## 2026-10-12 (night) delta — Tier 79: the owner's evening list; review rounds 20–21

**The owner's list**
- Select All in the URL bar's Cut/Copy bubble.
- Momentum on Settings and about:personas.
- Personas flashing (Carbon Light 2.0).
- The keyboard down when Settings or about:personas opens.
- Extension settings cut off.
- A caret left in about:addons' search field after the keyboard closes.
- Typing in text boxes works (the owner's finger).

**Done (Jihad `272ce0cd`)**
- **Keyboard down on Jihad's pages:** opening about:settings, about:preferences, about:personas or
  about:addons from the address field or the app menu takes the focus off the address field.
  - Enyo blurs in `Browser.urlChanged`.
  - Mochi blurs in `keyboardAwayFor`, from `openUrl` and `urlChanged`.
  - Mojo blurs in `keyboardAwayFor`, from `openUrl` and on arriving by a link.
- **No caret left behind:** the daemon clears the page's focus when the window grows back by 120 px
  or more at the same width while a field is focused (`BlurFocusedEditable`). The engine's blur
  then reaches the shell as editor-unfocused.
- **Card pictures:** an installed Persona's card draws an 800 px copy of its picture
  (`<24 hex>-thumb.jpg` in `jihad-themes`), made at most 1.2 s' worth per opening and pruned.
- **Add-on preferences:** each preference is its own block, text fields are dark, and the
  desktop margins are trimmed.
- **Mochi flicks:** Mochi now defines `Mojo.handleGesture`, LunaSysMgr's callback for a card's
  gestures. Enyo 1 has it and Enyo 2 lacks it, so the earlier DOM `flick` listener could never fire.
  Flicks are passed in whole numbers.
- **Inspector round 20, fixed:**
  - about:addons measures only the view it shows;
  - a page's own scroll jump within 300 ms of a pan is now applied instead of lost;
  - CREDITS covers the README strip's CC pictures;
  - AMO card state, https-only redirects, and a full /tmp are handled;
  - formerly bundled Personas keep their licence lines;
  - the README is up to date;
  - dead code removed.
- **Site agent:** fixed the site's round-20 findings (`6845226`..`d66fbc5`). Colour Personas' ids
  now come from their colours (kits updated). There is a Cloudflare real-IP conf, and the smoke
  test checks the feed contract (37/37 locally).

**Verified on the device** (bundle `219f12cd…` plus the project-drawn licence line; daemon rebuilt
with the focus and echo changes; all three variants)
- **Focus, all three:** after focusing the address field, opening a Jihad page left the focus on
  BODY; example.org left it on INPUT. In Mochi and Mojo the window grew from 705 to 996 px, so the
  keyboard went down.
- **Card pictures, Enyo:** 2, then 9 copies were made over three openings. They are real
  800x53 / 800x80 pictures, the cards draw them, and each shows its licence.
- **Preferences, Mochi (Palefill):**
  - Before: the controls ran past the right edge and the page zoomed out to 0.78.
  - After: everything is inside the page, with no sideways scrollbar.
  - The manager still scrolls its detail box down to the preferences, so the add-on's name sits
    above, in a box the card cannot pan.

**Needs a finger (the owner)**
- Flicks coast in Mochi, Enyo and Mojo on Settings and about:personas.
- Closing the keyboard over about:addons' search field leaves no caret. A scripted tap focuses the
  field but raises no keyboard.
- The flashing is gone.

**Select All (T-436, `a90b7fe3`):** it can't be added to the URL bar's Cut/Copy bubble, because
the system draws that bubble (none of the shells, Enyo 1 or LunaSysMgr's sources do). The owner
chose select-on-tap for all three:
- Enyo: `selectAllOnFocus`, already there.
- Mochi: Enyo 2's `selectOnFocus`, new.
- Mojo: `focusSelectMode`, already there.

Verified on the device with hook steps `K` (focus), `S` (selection) and `RF` (refocus with a caret
at 3, against a bare input):
- Enyo and Mochi select 0–20 of 20 while the bare input keeps its caret.
- Mojo, blurred (`B`) then focused, selects 0–20 of 20.
- Don't use `RF` on Mojo: its widget swaps the input for a div and the address empties. It was
  removed from that hook.

**Reviews:** Codex ran out again at 17:3x after 389k tokens, before writing its findings; retry
**10:29 PM**. Its notes named the focus-clearing path and the AMO install flow as the riskiest. The
inspector is reviewing in its place (round 21): Jihad `6e7b31a0..a90b7fe3`, site `26a9fdf..d66fbc5`.

**Round 21 fixed (`5ccc75a5`; site `2923301`..`5c1d54c`):**
- **Daemon:** the keyboard-down blur moved into the guarded pump (it ran page script inside a
  socket callback). A field in a frame is cleared and reported by the daemon itself.
- **Card pictures:** PNG kept PNG, 1400 px, all pruned on each opening. Device, Enyo: the 800 px
  ones are gone, the 14 Persona pictures intact, the cards drawn.
- **about:addons:** hidden settings stay hidden; checkbox titles wrap. Device, Mochi: Palefill
  still fits.
- **Shells:** Mochi's blur no longer submits a half-typed address; Mojo no longer blurs twice.
- **Site:**
  - real IP in our own server block;
  - UXP's colour grammar;
  - migration 002 (unique `browserId`);
  - smoke test 43/43 with `RS_SMOKE_CONNECT`;
  - credits served at `/credits/*.txt`.
- Free Palestine's licence line reads "based on "Village of Battir" by Labour Palestine, CC BY
  2.0" in both repos.

**Sent to the server agent:** folder `jihad-reply-2026-10-12`, holding `REPLY.md` and
`riverstone-addons-5c1d54c.tar.gz` (sha256 `40a2dd4c147a899c…`). It went over wormhole-rs with
code **46-determine-drunken**, for the owner to relay. REPLY.md has the owner's overrides, what
was done per section, the feed contract, the deploy steps, and the memory-note changes.

**Device state as left:**
- All three variants on bundle `37c3757ca422b7bfddc35373b24b98fe`, daemon `644eba8a…`; libxul
  unchanged (`ddf8cc70…`); daemons running.
- Card scripts: Mochi `JihadBrowser.js`/`JihadWebView.js` and Mojo `main-assistant.js` as in
  `5ccc75a5`; Enyo `Browser.js` as in `272ce0cd`.
- No test hooks, no user.js, no test CA; `/tmp` 1% used.
- Enyo wears Space Fantasy, and its `jihad-themes` now holds 11 card pictures beside the 14
  Persona files. Mochi wears Aurora Silk, Mojo Lime Fizz.

**Open, in order**
1. **Codex round 22 at 10:29 PM.** A session cron (one-shot, 22:31) runs
   `scratchpad/codex/r22.sh` in four slices, newest first:
   - Jihad `6e7b31a0..HEAD`;
   - site deploy;
   - Jihad `4e480610..6e7b31a0`;
   - site PHP.

   A single run burned out at 389k tokens, so the range is split. If this session has ended, run
   them by hand with `run.sh`; it now takes `HEADREF` for an end commit.
2. **Finger tests: deferred, all of them, to one round at the end** (the owner, 2026-10-12). They
   are collected in `docs/FINGER-TESTS.md`. Add new ones there; don't ask the owner piecemeal.
3. **Server:** the server agent deploys per REPLY.md.
4. **Known gaps:**
   - the same palette from AMO and from the site is two Personas;
   - a layered Persona's id comes from the merged header;
   - in about:addons, a Preferences tap scrolls the detail box past the add-on's name, which the
     card cannot pan back to.

## 2026-10-12 (evening) delta — Tier 78: the server agent's work order, the gallery, each shell's look

**The inputs**
- The owner's asks: the gallery's load flicker; each shell's own buttons; a Free Palestine Persona,
  then "an artistic depiction of palestinian land", then "photo realistic".
- The server agent's work order, `~/Documents/PROMPT-PC.md`: the gallery showing
  the site, bundling two, more catalogue formats, momentum, and about:addons in the webOS style.
- The owner's answers along the way:
  - bundle Free Palestine + Aurora Silk;
  - MIT colour palettes allowed, credited;
  - the gallery shows two previews per source, a full search of each and links to the websites,
    with the site's whole list staying on its own page;
  - addons.mozilla.org searched through its API, since its pages show this engine a robot check;
  - Personas managed in the gallery, because the card cannot scroll about:addons' lists;
  - about:addons dark, like Jihad's other pages: the light webOS look clashed.

**Done (Jihad `89571fb6`, `6128e29b`; Riverstone-Addons `b939443`..`26a9fdf`, built by a subagent to a fixed feed contract)**
- **about:personas:**
  - opens at once, and nothing moves or redraws while it loads;
  - Your Personas: wear, take off, remove (asks again), Undo in the toast, Default look, Install
    from file;
  - the bundled ones that aren't installed;
  - the site and AMO, two previews each, searched in place, each with a link to its website;
  - AMO Install fetches the theme from AMO's hosts only;
  - `#site-q=WORDS` links to a site search.
- **Bundle:** Free Palestine (from the CC BY 2.0 photo "Village of Battir" by Labour Palestine,
  credited) and Aurora Silk, edition 8. The rest are in the site's `catalogue/`, and a push now
  removes no-longer-bundled pictures from the device.
- **Look:** each shell's buttons on Settings, about:personas and about:addons (`shell.js`/`shell.css`,
  from `JIHAD_BS_NAME`). The settings tabs wrap. about:addons is dark with light text, the shell's
  buttons, toolbar buttons and dropdowns, and links drawn as links. Its Personas category is the worn
  one, Default look and a link to the gallery. The site's new discover pane is darkened inside it.
- **Scrolling:**
  - the daemon echoes no scroll while the card streams positions (an echo reset coasts on
    zoom-1 pages);
  - Mochi forwards flicks;
  - about:addons' root grows to the length of the list it shows, so the card can pan it.
- **Mojo:** the lost-connection panel floats under the bar instead of moving it.
- **Site:**
  - migration 001 (kind, colors);
  - the importer's three kinds (a colour theme gets a drawn header, layered ones are merged);
  - the feed's `kind`/`colors` strings;
  - 23 MIT palettes;
  - the discover pane in the Jihad skin;
  - the panel's text;
  - fail2ban's include;
  - DEPLOY.md's "The update of 2026-10-12".

**Verified on the device** (bundles up to `cddf3ea3…`, daemon `d200a97f…`, all three)
- **Gallery, Enyo:** the gallery against the live site and AMO; an AMO install through the confirm;
  Catppuccin Mocha from the local copy colouring the toolbar. Both profile tests were put back
  md5-identical.
- **Buttons and about:addons:** in all three.
- **Mojo:** the bar holds still during a lost connection.

**Needs a finger (the owner):** flicks coast on about:personas and Settings (Enyo, Mochi, Mojo); a
long about:addons list pans; typing a search.

**Device state as left:** all three on bundle `cddf3ea3941af6d8c055d7490c76d56e`, daemon
`d200a97f…` (libxul unchanged), daemons running; no test hooks, no user.js, no test CA; `/tmp` holds
nothing of ours; Enyo wears Space Fantasy (profile md5-identical to before the tests), Mochi Aurora
Silk, Mojo Lime Fizz; each profile has Free Palestine added (not worn). Local copy running with the
catalogue imported (40 Personas).

**Open, in order**
1. The inspector on both ranges (running), then Codex at 5:29 PM on the same ranges. Fix, record.
2. Pack the site (the catalogue travels inside it), write the reply to the server agent, send both
   with one wormhole code for the owner to relay.

## 2026-10-12 (later) delta — Settings: Personas, Extensions and Plugins, one button each

**Done** (`7cd6a158` and the round-19 fixes after it; `cavekit-preferences-ui.md` R8, build site
Tier 77, all criteria met). The owner asked for:
- a clear route to about:personas and extensions in Settings on all three variants;
- the extension not named as the row's subheading;
- plugins with the Flash Player's details;
- one button per row.

The Browser pane of about:settings now starts with a "Personas, Extensions and Plugins" group (blue
buttons). It replaces the old Appearance group:
- **Personas:** what is worn and how many are installed. **Browse Personas** opens about:personas,
  whose "Personas manager" link opens about:addons on its Personas list (take off, remove, Undo).
- **Extensions:** "Add features to web pages and to the browser. N installed" (and how many are
  off). **Manage Extensions** opens about:addons on its Extensions list.
- **Plugins:** "Adobe Flash Player 10.3 d185, on. From the HP Flash plugin installed on this
  device." **Manage Plugins** opens about:addons on its Plugins list.

**The inspector (round 19):** APPROVE, with 1 P2 and 4 P3, all fixed.
- Flash on "Ask to Activate" never plays here. The row now says so; the Content pane's Flash row shows
  it instead of a blank list; and the daemon's `getFlash` reports only state 2 as on, which needed a
  daemon rebuild.
- About reads the plugin host, so a Flash that is off no longer reads "not installed".
- The kits were updated for the new route.
- `render/goanna/test/prefs-addons-rows.test.js` covers the wordings (8 cases).
- The buttons are no longer the red of a confirming Clear.

**Verified on the device**
- All three variants: the group renders, on a page opened through each shell's own Settings entry,
  and each button lands on its page. The taps came from the hooks, not a finger; the pages were read
  back and photographed. The final bundle was re-checked on all three.
- Enyo, with `prefs.js` edited while the daemon was stopped (then put back md5-identical): state 1
  and state 0 each read the same in all four places. The manager link lands on the Personas list.

**Open**
1. Codex on `7cd6a158` and its fixes, when credits return (5:29 PM), together with the site's PHP
   part (afternoon delta, item 1).
2. The afternoon delta's other items are unchanged.

**Device state as left**
- All three variants on bundle `23953a34e209e90c358275c5e89c0dde`, daemon `4d8502ab…` (libxul
  unchanged); daemons running.
- No test hooks (the shipped manifests were checked by md5). No user.js, no test CA.
- Enyo's `prefs.js` is as before the test; Flash is on everywhere.
- Personas as below. `/tmp` holds nothing of ours. The test hooks gained Mojo's `t` (a tap).

## 2026-10-12 (afternoon) delta — round 17 verified on the device; Codex round 18 on the deploy kit fixed

**Verified on the device** (the TouchPad connected at 12:28; one task at a time)
- Bundle `eaa828a1877aa3b441213d51b6c2524e` on all three variants (the id read back from each).
  Enyo's `BrowserApp.js` and Mojo's `main-assistant.js` (round 17's UI fixes) were pushed with
  `push-card-js.sh`, and each card's boot stamp showed in the log.
- `test-site-profile.sh` under the device's BusyBox 1.17, on a scratch app
  (`JIHAD_TEST_APP=/tmp/fakeapp`, a copy of Enyo's profile):
  - setup, a test's changes, then restore → md5-identical;
  - a stray file planted in the backup → restore refused, profile untouched;
  - a restore cut off between its moves → finished.
  Then real cycles on Enyo and Mojo: both profiles md5-identical afterwards, with no CA, no user.js
  and no backup left.
- about:personas update check (local copy): the test Persona installed at 1.0.3. The site at 1.0.2
  → "Your Personas are up to date."; at 1.0.4 → an Update card for 1.0.4.
- Enyo, on a card started on the start page with a light-field theme: the reload and stop glyphs are
  drawn ink.
- Mojo share menu: a highlighted row's label takes `popup_highlight_text`. This was read from the
  stylesheet, with the row's `selected` class set by the hook, not under a finger.
- about:personas shows "Licence: MPL-2.0" on the five project-drawn Personas.
- Test tooling, now committed:
  - Enyo hook step `G` (the glyphs; it runs on the start page);
  - Mojo hook step `H` (a submenu's colours);
  - `card-selftest.sh … run <scenario> -` launches with no address.

**Codex round 18** (credits back at 12:28). The site range was split, because it never fits in one
window:
- **`deploy/`: done.** 2 P1, 5 P2, 1 P3, all fixed in site `9e5521b` and tested on the local copy
  and scratch directories (`impl-review-findings-6.md`, round 18):
  - php-cli installed by name;
  - both nightly archives watched;
  - a Persona import changes the files and the row together or not at all;
  - the smoke test accepts the allowlist's 403;
  - the owner account change is logged;
  - the local copy imports the Personas at every start;
  - unknown install.sh arguments are refused;
  - swap-in also refuses empty directories and special files.
- **The PHP part: ran out again.** Retry at **5:29 PM, 2026-10-12**.

**Open, in order**
1. Codex on the PHP part of the site. The prompt now tells it to stay off the web:
   `DIFFOPTS=--diff-filter=d EXTRA="This part is the PHP site (everything outside deploy/, which was
   reviewed separately)." scratchpad/codex/run.sh ../Riverstone-Addons upstream/TRUNK OUT LOG . ':!deploy'
   ':!libraries' ':!*.png' ':!*.jpg' ':!*.gif' ':!*.ico'`. Then fix, record, and stop the local
   copy (`deploy/local/down.sh`, no wipe).
2. The deploy guide: the copy sent with round 17 (`41-confidence-breakup`) timed out unreceived.
   DEPLOY.md is unchanged since `b9810ad`. Send it with wormhole-rs when the owner asks for a code.
3. The owner's items from the 2026-10-11 delta (deploy, typed search, landscape) are unchanged. The
   root-on-Devuan parts of the deploy kit (OpenRC, cron, nginx, the login message) are checked only
   by reading the code.

**Device state as left**
- All three variants on bundle `eaa828a1…` (daemon `5183c623…`, libxul unchanged `ddf8cc70…`),
  daemons running. Enyo's and Mojo's app sources are as committed, plus dev stamps from the pushes.
- No test hooks: the shipped manifests are back, checked by md5. No user.js, no test CA.
- Enyo wears Space Fantasy and Mojo Lime Fizz; both profiles md5-identical to before the tests.
  Mochi was untouched and wears Aurora Silk.
- `/tmp` holds nothing of ours (YAP sockets only). No Jihad card is open (the last LunaSysMgr restart
  closed them).
- Local copy `riverstone-local` running (PHP 8.4), 16 Personas; the test Persona is back at 1.0.3.

---

## 2026-10-12 delta — Codex round 17 on the Jihad range fixed; the site range still owed

**Done**
- The updated deploy guide reached the owner's desktop over wormhole-rs (the first send waited out
  its two hours unreceived; a fresh code worked).
- **Codex finished the Jihad range** `69c44675..83f0e817`: 1 P1, 5 P2, 4 P3, all fixed in
  `df590b3b` except one kept by design (`impl-review-findings-6.md`, round 17):
  - `test-site-profile.sh` judges every copy by md5, never by `cp`'s status (the device's BusyBox
    `cp` can report EBADF for a copy that landed). The backup records each folder's entry count
    (hidden and nested entries too) and every file's md5; restore refuses a backup that does not
    match, before touching anything. `extensions.ini` and `addons.json` are backed up too. A restore
    that does not finish leaves the daemon **stopped** and the backup kept: run restore again.
  - about:personas offers only a higher version as an update; an answer that is not about the
    Persona counts as not checked.
  - A colour-only Persona's drawn picture keeps translucent colours translucent (opaque ones are
    byte-identical to before, so their ids do not change); a multi-picture theme failing part-way
    removes the pictures it put in place.
  - Enyo: the ink reload/stop glyphs on a light field no longer need the browser view to exist.
    Mojo: highlighted menu labels take the theme's highlight text colour.
  - By design: updating or re-installing the worn Persona has no Undo (`cavekit-personas-ui.md` R3).
- Verified here: `test-site-profile.sh` in a local simulation (dash, a fake novacom and upstart, the
  engine's certutil): a full cycle left the profile identical; a `cp` that copies and then reports
  EBADF → setup and restore still succeed; a folder copy really cut short → daemon stopped, backup
  kept, the second restore finished identical; a changed nested file, an added hidden file, a
  changed backed-up file and an old-format backup are each refused with nothing touched. The
  gradient: opaque output identical to the old code's, the translucent one decodes as RGBA.

- **The inspector on the round-16 deploy fixes** (`cdaed4c`): REVISE, 1 P1 + 3 P2 + 9 P3, all fixed
  in site `b9810ad` (round 17 in the findings file). install.sh puts back nginx rules this server's
  nginx rejects and stops (the P1: `nginx -t && …` never stopped it), clears its stamp while it
  runs, runs only as the copy in place and needs cron. The new `migrate.sh` brings the database up
  to a copy BEFORE `swap-in.sh` puts it live (dump first; a migration only adds and runs twice
  harmlessly; a rollback names what it lacks). The new `watch.sh` (cron.daily) puts certificate and
  backup trouble on the panel's front page and in the login message. `swap-in.sh` refuses three
  more ways of losing the way back; `pack.sh` sends only `jihad`; the local copy re-issues its
  certificates and applies migrations. Tested on a throwaway MariaDB 10.11, scratch copies and the
  local copy (34/34); the root-on-Devuan parts are code only.

**Open, in order**
1. **Codex on the site range** `upstream/TRUNK..jihad` (now through `b9810ad`): it ran out again at
   ~150k tokens; retry **12:28 PM, 2026-10-12**. The owner's deploy guide changed with `b9810ad`:
   send it to the desktop again.
2. **When the TouchPad is connected** (one task at a time):
   - push bundle `eaa828a1877aa3b441213d51b6c2524e` (it also carries the plain `MPL-2.0` licence
     line) with `build/webos-oe/push-engine-update.sh --daemon-only enyo mochi mojo`;
   - reinstall the Enyo and Mojo app packages (the glyph and menu fixes are UI code);
   - run `test-site-profile.sh` on a scratch app directory first (`JIHAD_TEST_APP=/tmp/fakeapp`,
     under the device's BusyBox 1.17; the simulation used dash), then a real cycle on Enyo;
   - then check the about:personas update check against the local copy, the Enyo glyphs on a light
     field, the Mojo highlighted menu text, and an about:personas card's "Licence: MPL-2.0".
3. The owner's items from the 2026-10-11 delta (deploy, typed search, landscape) are unchanged.

**Device state as left:** unchanged since 2026-10-11 (below); the TouchPad was not connected at
00:32 or at 07:48 (`novacom -l` empty). Nothing was pushed.

---

## 2026-10-11 delta — Jihad's add-on site (built and tested locally, not deployed), three more Persona formats, review round 14 (READ FIRST)

**The owner's choices** (the Persona-index options, 2026-10-11): a Phoebus add-on site at
`addons.riverstonerelay.org`; colour-only themes, layered pictures and more colour slots; "Originals +
vetted CC"; build it locally first, with nothing published; their Devuan + nginx + OpenRC server;
the full Phoebus feature set, with Personas the only content for now; about:personas as a native
gallery plus the site; a Palm-era skin; submissions from the owner only; updates only when asked.
Kits: `cavekit-persona-formats.md`, `cavekit-addons-site.md`; build site Tiers 75–76 (T-390–T-414).

**Built**
- **Jihad** (`9e104699`, `7bf951a1`, `efa6844a`, `948e1c88`):
  - Colour-only and layered theme packages install as Personas. Extras live beside the Persona in
    `profile/jihad-themes/extras-<sha24>.json`.
  - A theme's other colours reach the address field, the menus, Mojo's command row and the start
    page.
  - Prefs point at the site: getAddons, discover, update, `galleryURL`/`siteURL`, and the install
    permission.
  - about:personas pages through and searches the site's Personas, and has "Check for updates".
  - No background update checks.
- **The site** is in `../Riverstone-Addons`, branch `jihad`. **Its only remote is `upstream`, which
  is Moonchild's Phoebus: never push there.** The owner creates the project's repository and adds
  it as `origin`. What it has:
  - the fork's hygiene, with licences and NOTICE;
  - PHP 8.2/8.4;
  - hardening;
  - Personas with an importer from `render/goanna/personas/personas.json`;
  - a JSON feed;
  - the Jihad skin;
  - a rootless local copy (`deploy/local/up.sh` / `down.sh`);
  - the deploy kit (`deploy/DEPLOY.md`, `deploy/server/*.sh`).

**Verified on the device** (against the local copy over the LAN, with its test CA in a test profile)
- Colour-only and four-layer Personas in all three shells. The colour-only bar is now stretched:
  Enyo measured #3e2164 at the top and #7c3aa8 at the bottom.
- The colour slots in all three shells. A classic Persona puts every slot back.
- Uninstall leaves nothing behind (Enyo).
- Re-installing the worn Persona with new colours repaints it (#cc2222).
- `hsl()` arrives as `rgb()`; a value carrying a shadow is dropped.
- An SVG-only theme: its picture is left out, and the Persona is offered and worn on its colours.
- The site's feed in about:personas. Its pager goes from page 1 of 2 to 2 of 2; "More on the web"
  opens `/personas/`.
- Install from the feed. Check for updates → Update (the picture's md5 changed).
- Wear on the site's page. The discover pane.
- about:addons Check for Updates, 1.0 → 1.1.
- Offline, about:personas says the site cannot be reached.
- Screenshots in Enyo: the front page, `/personas/`, an add-on page and a 404 in the skin.

**Verified on the local copy**
- Smoke test 31/31 on PHP 8.4 and on 8.2, with every page type and the panel's pages; the PHP log
  stayed empty.
- A POST without the token, or from another origin, gets 403.
- A hostile Persona name shows as text in the page, in the `data-browsertheme` JSON and in the feed.
- An unreviewed add-on is 404 publicly and still served by the panel.
- `persona-admin.php` hide, show and delete work; the delete leaves symlink targets alone.

**Reviews**
- Codex r13 (Jihad, morning): fixed.
- Inspector round 14, standing in for Codex, on both repos: one P1 (the panel path) and the
  P2/P3s, all fixed. See `context/impl/impl-review-findings-6.md`.
- Inspector round 15, on the round-14 fixes: APPROVE for both. The P2 (picture sizes could be
  under-read) and eighteen P3s are all fixed, and the dangerous ones were checked on the device.
- **Codex owes a review** of Jihad `69c44675..HEAD` and of the site's `upstream/TRUNK..jihad`.
  It ran out of credits again at 17:00, and again at 22:10 (after ~216k/243k tokens, before writing
  a finding); retry at **3:07 AM, 2026-10-12**. The site range now includes the wormhole-rs deploy
  (`8535d3b`..`cdaed4c`). The inspector reviewed it as round 16 (APPROVE, 2 P2 + 12 P3, all fixed in
  `cdaed4c`: copies checked against a manifest before going live, `www.prev` as the way back, an
  install stamp deciding when install.sh must run, `deploy/migrations/`, the certificate warned of
  by the smoke test; `impl-review-findings-6.md`).
  The wrapper is `codex exec -m gpt-5.6-sol -s read-only` with Cavekit's prompt and the diff (see
  the codex-gate memory); give the site diff `--diff-filter=d` and leave out `libraries/`.

**TRAP — it did real damage.** The TouchPad's BusyBox 1.17 `grep` has **no `-x`**.
`test-site-profile.sh restore` tested membership with `grep -qxF … || rm`. Every test errored, so
both of the day's Enyo restores deleted *everything* in `profile/jihad-themes/` and
`profile/extensions/`.
- Fixed, and then redesigned in round 15: restore no longer decides what to delete by matching
  names. Setup copies the whole of `jihad-themes/` and `extensions/` (and every profile file a test
  touches) into a backup that is written as `$BAK.tmp` and renamed only when complete. Restore
  refuses an incomplete backup or a short folder copy, and otherwise swaps the folders back.
- Tested on a scratch profile on the device (`JIHAD_TEST_APP=/tmp/fakeapp`): a test's added,
  deleted and rewritten files all came back md5-identical; the refusals changed nothing; an
  interrupted restore finished on a second run. Then two real cycles on Enyo were md5-identical.
- Enyo repaired:
  - its 13 pictures copied from Mochi under the same names (Mochi's and Mojo's copies are
    md5-identical);
  - Palefill re-offered by deleting `jihad.bundled-addons.palefill@addons.martoks-place.de.offered`,
    which reinstalled 1.30, active.
- The jihad-site-test add-on (a test file) went with it.
- **After any device-side clean-up, count what is left, not only what was meant to go.**

**Open, in order**
1. **Codex re-review** of both ranges (above). Fix the findings, then re-test on the local copy and
   on the device.
2. **[OWNER] Deploy**, following `../Riverstone-Addons/deploy/DEPLOY.md`, which is now numbered
   steps in the order they happen. **Every file goes over magic-wormhole-rs** (`wormhole-rs`; the
   owner's rule, 2026-10-11: no other wormhole may be installed, not the Python `magic-wormhole`
   that apt and xbps offer). `deploy/server/pack.sh` sends the last commit, and `swap-in.sh` puts it
   in place on the server; the Personas and the backups go the same way (a folder arrives as
   `<name>.tar`). `sync.sh` (rsync) is gone (site `8535d3b`, `fd41f7e`). In short:
   1. wormhole-rs is already on the PC (`~/.cargo/bin`) and on the server.
   2. Point DNS for `addons.riverstonerelay.org` at the server.
   3. Send the site and put it in place with `swap-in.sh`.
   4. On the server, as root, run `install.sh`, then `make-owner.php` (it asks for the password).
   5. Send the Personas and import them.
   6. Run `smoke-test.sh`.
   7. Install a Persona from a desktop Pale Moon or Basilisk, and from Jihad (R10).

   A repository of the site's own is optional (hosting doesn't need one); its only remote is still
   Moonchild's `upstream`, which is never pushed to.

   Until then, Jihad points at a site that does not exist. That is by design: about:personas says it
   cannot be reached, and the discover pane shows its offline message.
3. **[OWNER, finger]** Typed search in about:personas (the on-screen keyboard).
4. **[OWNER, hands]** The site in landscape on the TouchPad itself (R7). A desktop, a phone and
   1024×768 were checked in headless Firefox; they read well.
5. Undo, Default look and editions with the new formats have been checked in code only.

**Testing against the local site**
1. Run `../Riverstone-Addons/deploy/local/up.sh` (`PHP=8.2` for 8.2). The CA is at
   `deploy/local/state/tls/ca.crt` and the owner password at `state/owner-password`, both
   git-ignored.
2. Run `build/webos-oe/test-site-profile.sh setup <variant>`, test, then `restore <variant>`.
3. Run `status`, and count `jihad-themes/` and `extensions/`.

Click coordinates: Enyo's and Mochi's `clickAt` take view pixels (device y − 82); Mojo's are
card-relative (device y − 28). Only a finger pans, so jump with anchors (`about:personas#online-page`,
`#check-updates`), and use a `U<url>` step to re-jump after a tap. Phoebus refuses curl's user agent;
pass `-A JihadBrowser/1.0`.

**Device state as left**
- **Not yet on the device:** bundle `b3476e74b62eb6202250da58e0020bc6` (the five project-drawn
  Personas' licence reads plain `MPL-2.0`, without "(drawn for Jihad Browser)"; the owner's ask,
  2026-10-11). The TouchPad was not connected (`novacom -l` empty); push it with
  `build/webos-oe/push-engine-update.sh --daemon-only enyo mochi mojo` and check an about:personas card.
- All three variants on bundle `e6f75a783e82e5717c7b1b9e8d487008` (daemon-only pushes, round-15
  component), daemons running.
- No test hooks, no user.js, no test CA.
- Enyo wears Space Fantasy, with 13 pictures and Palefill 1.30 active. Mochi wears Aurora Silk and
  Mojo Lime Fizz.
- Test Personas removed from all three. Carbon Light's bundled details restored in Enyo.
- `/tmp` holds nothing of ours. The 18 stale YAP sockets of dead daemons were removed; the live
  ones were kept.

## 2026-10-08 (night) delta — complete themes, the Persona gallery, a licensed bundle, engine-update groundwork (READ FIRST)

**The owner's eight asks and what came of each:**
1. **Complete themes** — investigated (`docs/research/complete-themes.md`): they skin XUL chrome,
   and Jihad's look lives in the cards, which only Personas reach; wearing one would switch the
   Persona off, needs a restart path this embedding lacks, and a skin missing a required file can
   stop the engine at start-up. Decision: not supported. `jihadInstallPrompt.js` explains and
   points to Personas; a toolkit-targeted one that would install is taken straight back out.
   **Device (Enyo):** a Pale Moon complete theme → refused with the message; a toolkit one →
   installed then removed, nothing left in the profile.
2. **"Get Personas" repository** — `about:personas` (packaging/prefsui/content/personas.*), Jihad's
   own gallery (Pale Moon/Basilisk use their Phoebus sites; Basilisk's `lightweightThemes.getMoreURL`
   is honoured). Cards with picture, author, licence, Wear; an online https index
   (`jihad.personas.galleryURL`, empty by default) installs through the usual confirm — **the owner
   decides whether and where to publish one.** **Device:** Wear from the gallery works.
3. **Licences** — survey `docs/research/persona-licences.md`. Bundled: our five + ten CC BY / BY-SA
   Firefox-era Personas (pinned sha256, `import-third-party.py`; credits `CREDITS.md`, `NOTICE`,
   cards). Removed: Water-Like test assets (ARR; tests use Carbon Light), every README picture with
   an unlicensed Persona (retaken), and those Personas from the device profiles (Running Foxes,
   Aurora Borealis Delights, Blue Nebula, Dark Fox, Purple Night, Water-Like, test Personas).
4. **Names** — Bubble Lagoon, Cloud Nine, Meadow Morning, Aurora Silk, Lime Fizz (formerly Aero
   Aqua/Sky/Meadow/Ribbons/Orbs; renamed in place on every profile, uninstalled ones stay so).
5. **Clean-up** — see "Device state" below; scratch files deleted at the end of the session.
6. **README** — 333 → 201 lines (features that matter; no long-press notes; dev notes → pointers).
7. **Finger tests** — deferred by the owner; every `[human at the device]` box in the kits says so.
8. **Engine-update groundwork** — `docs/ENGINE.md` (read it before touching the engine);
   `build/engine/`: `engine-version.env` + `check-engine-pins.sh` (all pins agree),
   `patch-queue.sh --check` (the 39-patch queue reproduces `UXP_PIN` exactly — it had drifted:
   0031–0039 added, obsolete files removed), `check-engine-api.py` (0 FAIL on today's engine);
   `render/goanna/EngineCompat.h` declares every `jihad_*` export; the daemon logs
   "engine: 11 of 11 optional jihad_* exports present" at start-up. `docs/ENGINE-SOURCE.md` (the MPL
   record) lists all 39 changes. Kit `cavekit-engine-update.md`, Tier 74 (next steps T-385…T-387).

**Note:** the five renamed picture files landed in the engine-groundwork commit `30a4970b`
(staged renames rode along); the content is right, the commit boundary is not.

**Reviews:** the inspector's round 12 over `2cc01662..392c39e3`: ten P3s, all fixed (see
`impl-review-findings-6.md`). Found while verifying: removing a complete theme left its skin selected
and the Persona queued off for the next start — fixed (prompt clears both), Enyo's profile repaired.
**Codex owes a pass over `9e08c610..HEAD`** (out of credits until 10:07 PM 2026-10-08).

**Device state left:** all three variants on bundle 143 (daemon with `EngineCompat.h`, the
gallery, 15 bundled Personas, the complete-theme fix); Personas worn: Enyo Space Fantasy, Mochi
Aurora Silk, Mojo Lime Fizz; profiles hold only licensed Personas; test hooks restored to the shipped
manifests (Luna restarted); the device's `/tmp` test pages and scripts removed, and 76 stale
`/tmp/yapserver.jihad-browser*<pid>-<n>` sockets of dead processes removed (see below); host test
servers stopped; session scratch (~900 MB) and `~/.cache/jihad-personas` deleted —
`import-third-party.py` re-downloads and re-checks the pinned packages when run.

**Minor, open:** each card connection creates `/tmp/yapserver.<app><pid>-<n>`; `YapClient`'s
destructor unlinks it, but killing LunaSysMgr (every `luna-restart.sh`) skips that, so they pile up
until a reboot (0-byte files on the 40 MB tmpfs). A daemon-side sweep of dead-pid sockets at start-up
would fix it without touching the wire contract.

---

## 2026-10-08 delta — taps land where drawn, long-press menus back, the Personas manager, a Frutiger Aero set, the README (READ FIRST)

**Owner's reports this day and what came of each** (all device-verified unless marked):
- **"mojo tap targets seem inaccurate … I get duckduckgo when I click wikipedia"** — FIXED, three
  causes: the daemon divided adapter coordinates by the zoom a second time (since 2026-08-02; every
  shell, any zoom ≠ 1); Mojo's widget single-tap handler clicked a second time; the device's
  PGContext blit ignored the header strip, so the page was drawn one strip higher than taps were
  mapped. Mojo start page: one `clickAt` on the drawn Wikipedia link, opens it.
- **"using the url bar to search the web is slow"** — the first-paint hold is at most 1 s per
  document (it waited for load end, 12 s on Wikipedia). Owner's finger check pending.
- **Long-press link/image menus never appeared (found while taking screenshots)** — FIXED: the
  device's stock `/etc/palm/browser/HitTest.schema` does not compile in the pbnjson the adapter
  links (rejects even `{}`), and a rejected parse left the hit-test object empty. The adapter now
  validates against its own embedded schema. Enyo: link → Open In New Card / Share Link / Copy URL;
  image → Copy To Photos / Share Image / Set Wallpaper; Mochi's link sheet opens.
- **"make it user friendly to manage and install personas, using about:addons as a base"** — new kit
  `cavekit-personas-ui.md`, built (Tier 73, T-365…T-371). `components/jihadPersonas.js` decorates
  the engine's own about:addons (no engine change): category "Personas"; rows with the Persona's
  picture, author, version (1.0 default), In use first; Wear / Take off / Uninstall (44 px);
  Default look; an Undo bar for 30 s; Get Personas (AMO); Install from file (`.xpi` in
  `/media/internal/downloads` or `/media/internal`). Settings → Appearance is a summary with
  Manage Personas. Verified in Enyo, Mochi and Mojo. Owner direction folded in the same day: Try
  dropped for Wear + Undo, author and version on all rows, "Persona" everywhere (confirm, toasts,
  Settings, README), "Uninstall" not "Remove".
- **"add fruiter aero style themes"** — five original Frutiger Aero Personas
  (`render/goanna/personas/make-aero-personas.py` draws them; bundled under `personas/`), added once
  to each profile's list (not worn); an edition bump replaces the set's older pictures.
- **"blue nebula looks like it isnt working properly"** — wallpaper-sized pictures (AMO's
  3840×2160) exceed the 32 MB image cache and never drew in the engine: kept 1600 px wide at
  install, and the cards fit them to the bar (`headerFit`). Already-installed big ones need a
  reinstall.
- **"dark fox preview … not working"** — a row added while the pane is open is redecorated once its
  XBL binding attaches.
- **README** — rewritten as a webOS-era product page (card view, Personas strip, framed shots, the
  sourced HTTPS-on-webOS history). Art rebuilt by `docs/screenshots/readme-art.py` from raw device
  captures (Prelude fonts from the device, not in the repo). howsmyssl.com reports TLS 1.3.

**Found on the way, fixed:** the `resource://jihad-themes` mapping was never made after a daemon
restart (the constructor runs before the profile is known) — converted Personas lost their
pictures; the pane's `window.setTimeout` from the component never fires — the component uses its
own nsITimer; the daemon re-drove a page's load of an about: page by its resolved file address
(about:addons → `file:///…/extensions.xul`, refused as XUL from a file) — now by the original.

**Reviews:** Codex r9 (taps/first paint/hit-test, 4 findings, all dispositioned); the inspector r10
on the owed range `20310b3e..e7b9aa7d` (Codex ran out mid-run; P1 Mojo widget restoring zoom 1.0
on Back/reload, pinch drift, header after a link, stale meta viewport, setHeaderHeight — all fixed,
Back verified); the inspector r11 on the Personas work (8 findings, all fixed or already fixed) in
`context/impl/impl-review-findings-6.md`. **Codex owes a pass over `9e08c610..HEAD` when its credits
return (it said 10:07 PM on 2026-10-08).**

**Get Personas verified against the live AMO:** its Persona pages' **Download file** link installs
through the usual confirm (the big button only offers Firefox). **Still open:** the empty
state (needs every Persona uninstalled); double-tap speed and block choice; Mochi text soft at
~1.43; the owner's finger pass over all of the above; coordmap_test (fixed to the contract, not run
— no desktop engine build on this host).

**Device state left:** daemons, components, settings page and the Aero set (edition 3) deployed
to all three (bundle 137); adapters in `/usr/lib/jihad/<variant>/` with md5 enyo a952549dafab,
mochi 819269398a78, mojo 76f9d6979c55 (rootfs ro). Personas worn: Enyo Aero Ribbons (its Aero Sky
uninstalled for the edition test), Mochi Aero Ribbons, Mojo Aero Orbs. Test hooks restored to the
shipped manifests in all three (Luna restarted). The host's theme test server (8091) and http.server
(8092) are stopped. `/media/internal/downloads/dark_fox-2.0.xpi` is left there from the
Install-from-file test. Reviews: the inspector's round 11 on the Personas work is in
`impl-review-findings-6.md` (8 findings, all dispositioned).

---

## 2026-10-07 delta — owner's finger test: Mojo width loop fixed, menus polished, About page, README, Flash games (READ FIRST)

**The owner's finger-test report (2026-10-07) and what came of each item:**
- **Mojo "Wikipedia glitches, jumps, a stripe at the left"** — FIXED (daemon): the meta viewport was
  re-sent whenever its computed initial scale changed; Mojo sizes its node from that scale, so the
  page shrank each round (768 → 526 → … → 76 px). Re-sent now only when the page's declaration
  changes. Verified on en.m.wikipedia.org.
- **Long-press "not working on anything", all three** — NOT REPRODUCED. The owner's holds DID reach
  the engine (`hold at …`, `page-hold … (pushed)` in all three logs). A finger-like scripted
  press → hold → lift (new hook steps: Enyo/Mochi `J[ms]`, Mojo `Z0`) opens and keeps the menu in
  all three, whether it opens before or after the lift. The engine's answer takes ~1 s from
  finger-down. **Needs the owner:** does nothing appear at all, or does something flash and vanish,
  and how long was the finger held?
- **Mojo circles not themed** — FIXED: accent-colour fill, glyph set by text colour, bookmark glyph
  included.
- **Mochi dropdowns** — rounded corners; the app menu now flush with the left edge.
- **Mochi pinch makes text blurry** — OPEN, partly characterised: the daemon re-renders at the new
  zoom (frames at `contentZoom` = zoom, 1:1), unpinched text is equally sharp in Mochi and Enyo
  (edge metric 34.2 / 34.5), Mochi at 1.25 and Enyo at 1.28, 1.35 and 1.87 are crisp, Mochi at
  1.43 is soft. Not a stretched bitmap (the card's scaler is nearest-neighbour). Cause not found.
- **Home button default** — DuckDuckGo Lite in all three and on the settings page; a stored old
  default (`start.duckduckgo.com`) moves once, a home the user set is kept.
- **Enyo refresh icon** — 2 px higher (measured: field rows 38–66, glyph 47–59).
- **Flash test bookmark** — the page lives at `/media/internal/jihad-flash-test/` (survives reboots
  and reinstalls; a copy is in `render/goanna/test/flash-test-page/`), bookmarked in all three
  through each shell's own Add Bookmark. It now also links five real games on archive.org (direct
  SWFs: Helicopter, Bloxorz, Motherload, QWOP, Alien Hominid). **Bloxorz verified running** as a
  full-page plugin document. Enyo's address bar sent a typed `file:///` address to search — fixed.
- **Flash games / Bluetooth** — research in the session scratch (`flash-research.md`): Armor Games
  embeds the real SWF (should run natively), Y8 needs a UA tweak, Kongregate/CrazyGames/Newgrounds/
  archive.org item pages force Ruffle, archive.org DIRECT downloads work. webOS 3 pairs Bluetooth
  KEYBOARDS natively (keys reach the card and the adapter forwards them to the page) but has NO
  Bluetooth mouse support at all (pairing refused for non-keyboard HID) — a mouse is not feasible
  without OS changes; the touchscreen is Flash's mouse (double-tap for gesture lock). Keyboard +
  Flash still needs a real keyboard to verify (`key.html` on the test page).
- **About page** — Settings → About: interface, version, app id, engine and build, Flash version,
  user agent, licences and credits, read at run time. Verified (Mochi).
- **README** — rewritten around what only Jihad does on webOS, with screenshots in
  `docs/screenshots/`.

**Review:** Codex ran out of credits mid-review again (retry 12:41 PM) → inspector on the round-8
fixes (round 8b in `impl-review-findings-6.md`): one P1 — URLs still unbounded on other messages,
and an oversized YAP string reached the adapter as NULL → `strlen(NULL)` in the card host; fixed in
the daemon and in the shared YAP codec — plus five P3s, all fixed.

**Still open:** Mojo pinch (pinch in then out → grey; structural — see mojo-ui R6; proposed:
viewport-sized node in Mojo with the adapter owning pan/zoom, as in Enyo/Mochi); Mochi blur at
~1.43; long-press by finger (needs the owner's detail); slow pages can paint unstyled while their
stylesheets load (the offscreen renderer does not honour paint suppression); Mojo focuses the
address field (keyboard up) when launched with a URL; the Mochi profile carries test personas in
its used-themes list (Forget them in Settings → Appearance).

**Later the same day (owner: "long press works", "proceed with mojo pinch fix", "mojo themed
buttons are still broken", "the buttons and virtual keyboard should not be touching"):** Mojo's
WebView is now a screen-sized view with the adapter owning pan and zoom (mojo-ui R6 — verified pinch
in/out by script; **panning needs the owner's finger**); the themed circles keep the framework's
25 px borders so glyphs stay centred; the command row sits 23 px above the keyboard. Adapter: one
gesture path for every shell, pinch centre in window pixels. Long-press: confirmed working by the
owner.

**Second Mojo pass (2026-10-07/08, owner: scrolling dead, double tap dead, circles flash at
load, Wikipedia unstyled until touched, "I want mojo's toolbar to scroll off"):** all addressed —
see mojo-ui R6. Two of them were not Mojo's: the daemon's double-tap (smart zoom) request had
always been a stub (every shell), and the offscreen renderer ignored the engine's first-paint
gate (every shell; now held up to 5 s and fully repainted on release). Panning by finger in the
new Mojo model, and double tap in Enyo/Mochi, still want the owner's hands.

**Device state:** all three daemons on this round's build; adapter on the rootfs for all three
(md5 enyo `83e2b9f0…`, mochi `5851cff3…`, mojo `77d28024…`); rootfs ro; test hooks restored on all
three (shipped manifests); Flash test page + bookmarks
as above; theme Water-Like on all three.

## 2026-10-06 (night) delta — Codex r7 fixed; menus match on all three; Flash regression fixed and drivers off; Mojo pinch fixed (READ FIRST)

**Owner's asks this session:** "continue implementing everything you can using cavekit 3.1 with
codex adversarial review" plus: the upper-right menu the same on all three (Atlas as reference);
"optimize flash player for gaming performance"; DuckDuckGo Lite/HTML as search options, Lite the
default. Mid-session: "mojo should have no top toolbar icons next to url. bottom circular buttons
only"; "put the mojo refresh icon embedded in the right of the urlbar instead of the bottom as a
circular button"; "pinch to zoom is broken. the page turns grey and I get the copy/select
all/reload/find on page menu instead". The (evening) delta below is superseded: personas are done
and committed (`20310b3e`), and all three shells were device-verified with Water-Like.

**Verified on the device (TouchPad 3.0.5, one task at a time):**
- **Codex round 7 (12 findings) fixed** — table and device evidence in
  `context/impl/impl-review-findings-6.md` (round 7): referrer/policy carried on re-driven
  navigations (6 cases on the echo server's `/ref?v=`), title flood bounded and coalesced
  (`/titleflood`: 12 sends in ~6 s, ≤1024 bytes), theme preview only from https+"install"
  (http preview ignored), static-theme package limits/pending file/hash id, theme origin taken at
  the event, Enyo theme on the lazily created views (start page and browser bars both painted).
- **Menus:** Mochi's upper-right is now Share ▾ / New Card / Bookmarks ▾ and its title-bar app
  menu (Find on Page · Settings · Print) opens from a real sysmgr relaunch. Mojo has NO icons by
  the URL (owner) — Share ▸ and Bookmarks ▸ are submenus on the bottom circles (History circle
  became Bookmarks), reload/stop sits inside the address field, the app menu is Find · Settings ·
  Print; new Bookmarks / Downloads / Print scenes. Enyo was already the reference.
- **Flash, gaming:** a regression from `debd1934` made every frame a full paint (15-16 fps,
  ~25 ms paints); fixed (34 fps paints, 4-5 ms, all partial). The child's own timer is now off by
  default and the daemon only pulls after 150 ms of plugin silence: one producer, histogram at
  32-39 ms, a third less tick work — and the CONTENT rate (what a game's speed is), measured with
  a grid SWF and two timed screenshots, is the same as with the old drivers (22.5 vs 22.1 fps):
  they only re-drew frames. Knobs: `JIHAD_PLUGIN_SELF_DRIVE=1`, `JIHAD_PLUGIN_QUIET_MS=0` restore
  the old behaviour for an A/B. Shorter plugin ticks (10/12 ms) changed nothing.
- **Mojo pinch:** two adapter defects on the path only the Mojo widget uses (JS `gestureStart`):
  the first finger's hold timer was never cancelled (→ the long-press menu with Mojo's grey scrim),
  and the adapter zoomed/scrolled itself on top of the widget's own pinch (→ grey page). Fixed in
  `BrowserAdapter.cpp` (`widgetGestureStart`, hold dropped if a gesture began since). Verified with
  the hook's finger-like `Z` step (a held-still control does raise the menu; the pinch does not;
  engine zoom tracks the widget 1.01 → 1.20; page drawn in and back out). **A real finger pinch by
  the owner is still the final word.**
- DuckDuckGo Lite default (Mochi reads `duckduckgo-lite`), HTML listed, one-time migration.

**Not done / open:**
- **Codex is out of credits until Oct 7 1:53 AM** — round 8 (all of the above) was reviewed by the
  `ck:inspector` backup: 1 P1 (a re-drive without a referrer ran as the SYSTEM principal — could
  reach `file:` / top-level `data:`), 1 P2 (static-theme zip sizes trusted), 5 P3; all fixed and
  the P1/P2 verified on the device (round 8 in the findings file). **Owed: Codex on this
  session's commit (`20310b3e..HEAD`) once its credits are back** — the memory note
  `codex-gate-needs-direct-invocation.md` has the invocation.
- Flash's own rate on this CPU (~27 fps for a 30 fps 320x240 SWF, ~22 at 640x480) is the limit
  now; `sensord` takes ~15-20 % CPU on the test device at all times (not ours). Whether evenly
  paced 27 fps reads well in a real game is an eye test for the owner.
- One unexplained miss: a static theme's live notification did not paint Mochi's bar once (two
  later runs did). Tabs: groundwork only, as asked.
- Finger tests remain the owner's: pinch on Mojo, the menus by touch, the in-field reload.

**Device state as left:** all three daemons run the same hand-pushed build (this session's final
bundle; the packages under `build/webos-oe/out-ipk/` are built from the night commit, the same
tree, but NOT installed as packages — the device has the pushed files). Adapter on the rootfs for all three
(`/usr/lib/jihad/<V>/BrowserAdapterImpl.so`, md5 enyo `d9144bc4…`, mochi `831549b7…`, mojo
`8f1a90bd…`; Mojo-only widget pinch), rootfs **ro** (checked). Test hooks **restored** on all three
(shipped manifests, no hook files). No Jihad card open; no plugin-container. Host echo server
stopped.
`/tmp` holds `jihad-anim.*`, `jihad-grid.*`, `jihad-plain.html` (small). Mochi's current theme is
the static Water-Like (`static-theme:0dff6242…`), Enyo and Mojo Water-Like. The Enyo daemon is
on its job again (it was hand-started for the A/B runs).

## 2026-10-06 (evening) delta — personas IN PROGRESS, stopped at the usage limit (superseded by the night delta above)

**Owner's asks this session, in order:** fix the slow add-on manager (DONE, see commit `fb8d2cfc`);
"ensure all options in about:config that are relevant are working" (DONE: effect audit, same
commit); tabs decisions (DONE in the kit/design: `window.open` → new tab, strip invisible with
one tab, no cap — responsive scrolling strip, Atlas `TabLayer.js` as the UI reference, "buttons
and tabs should also be themed" added to R1); **personas** — "support firefox 52 personas, pale
moon personas, and basilisk personas", then "use the persona water-like by stick177 to test all
3 variants. buttons and tabs should also be themed" — IN PROGRESS, uncommitted parts committed
as they stand in the commit after this one.

**What is built and device-verified (Mochi only):**
- Engine/daemon: `GoannaRenderPage` catches the theme sites' `InstallBrowserTheme` /
  `PreviewBrowserTheme` / `ResetBrowserThemePreview` events on the content document; the pump
  hands them to `components/jihadPersonas.js` (new) as page command `theme`; the component asks
  the user through the existing `jihad-xpi-confirm` door (wording "Allow <host> to change the
  browser's look to the theme: <name>"), applies through the engine's `LightweightThemeManager`
  (which persists the header image into the profile), records the theme in pref
  `jihad.theme.current` and pushes category `theme` on the notification channel; Luna
  `getTheme` (new, both buses) answers a starting card. The notification door now lets `theme`
  JSON through whole (it was cut at 200 chars — the card parsed nothing; the first bug found).
- Mochi card: the toolbar is painted (accent + persisted header file, `rgb(92,30,47) url(file:///…/profile/lightweighttheme-header…)`) both at start (getTheme) and live on install. Test site:
  scratch `echo-server.py` `/persona?v=N` (and `?v=water` = Water-Like by stick177, from AMO's
  converted static theme: `theme_frame` jpg, frame `#00ccff`, text `#000000`); hook steps `T`
  (toolbar style), `Z` (clear card-side), `Y`/`a` (confirm answers).
- Settings page: an "Appearance" section (current theme + Remove; used themes with Use/Forget)
  in the Browser pane — CODE ONLY, not yet seen on the device.
- Static-theme XPIs (what AMO serves every former persona as): `jihadInstallPrompt.js` hands a
  -207 corrupt install's file to `jihadPersonas.js`, which reads a theme-only `manifest.json`,
  unpacks the image under `<profile>/jihad-themes/` (served as `resource://jihad-themes/…`),
  and applies it as a lightweight theme with the user's yes — CODE ONLY, untested. Test link on
  the site: "Install the AMO static-theme XPI" (`/water-like.xpi`, sha256 `c6b8b50d…`).
- Button tinting in all three shells (`-webkit-mask-image` of the glyph + textcolor; Enyo
  `.enyo-button-icon`, Mochi `img.jihad-nav-img` with a transparent src, Mojo the command row's
  sprites, re-applied after the row's model changes) — CODE ONLY, untested on the device
  (whether this WebKit honours `-webkit-mask-image` is the first thing to check: hook `T` reports
  the first icon's mask/background).
- Enyo and Mojo `applyTheme` + `getTheme` + `theme` routing — CODE ONLY, untested.

**Next steps, in order:** `card-selftest.sh {enyo,mochi,mojo} push`; per shell run
`page:w6;Z;T;107,146;w3;a;w3;T;w5;T;w1` on `http://192.168.1.245:8091/persona?v=water` (the
echo server must be running on the host: `python3 <scratch>/echo-server.py 8091`; the Water-Like
header jpg and xpi live in the scratch dir — copy them somewhere durable, e.g.
`render/goanna/test/personas/`, before the scratch dir is lost); screenshots of the themed bar
and buttons on each shell; the Mochi click coordinates differ per shell (Enyo/Mojo: read the
page's button position from a screenshot: image (80,168) at 576x768 → view x = 107, y =
168·1.333 − view top); then the AMO XPI link; then `about:preferences` Appearance section;
then `restore` all three hooks, rebuild bundle + packages (`build-variant-ipk.sh`), install
as upgrades, remount ro; kit cavekit-addons-extensions.md gains **R10 (lightweight themes)**
with these criteria (not yet written); PARITY rows; Codex/inspector review of the persona code
(`jihadPersonas.js`, `jihadInstallPrompt.js` hook, `GoannaRenderPage` theme events, the three
`applyTheme`s); PICKUP.

**Device state as left (NOT clean):** all three daemons run the hand-pushed bundle
`9feea335…` (daemon with idle drain, referrer, title, DNT, theme-door; components incl.
`jihadPersonas.js`), NOT the installed packages (those are at `3bafb9bb…`). **The Mochi test
hook is LOADED** (`card-selftest.sh mochi restore` before anything else); Enyo and Mojo shells
on the device are the OLD ones (no `applyTheme`). Mochi's profile holds test personas
`jihad-test-persona-1/2/3` as used themes and persona 3 as current — remove via the Appearance
section or `LightweightThemeManager.currentTheme = null`. Mochi's `prefs.js` is back to its
20 user prefs (the effect-audit batches were restored). The host echo server may still be
running (pid in `pgrep -f echo-server`). Root fs: rw after the hand pushes? — check
(`mount | grep store-root`), remount ro. No card open.

## 2026-10-06 (go-ahead) delta — everything that needed no hands: reinstall audit, reboot, a real sheet, Codex round 6, SDK 0.4, tabs groundwork

**The owner's word:** "You have my go ahead for all remaining tasks that you can achieve without me
physically touching the device. Also lay the groundwork for tabbed browsing but dont implement it
yet." Then: "Update to the latest community sdk" and "Pause completely when you need a sudo
command from me" (saved as a memory rule).

**Done, device-verified:**
- **SDK 0.4** (`~/eclipse-workspace/webos-sdk-redux` at tag `0.4-Release`; the owner ran
  `sudo ./install.sh`): `/opt/PalmSDK/Current → 0.4`, new `novacomd` (built 09:50) running, device
  seen. Among its fixes: novacom no longer drops a packet whose payload trails its header at
  close — the "output after stdin EOF is lost" trap may be softer now; `dev-sh.sh` stays the rule.
- **Enyo and Mojo removed through the installer and installed from nothing** (T-231–T-233, T-273;
  Mochi was 2026-10-05). Each removal left no app dir / job / shim / impl / role / service / state
  dir / socket, the hub forgot the name, the other daemons kept their PIDs, the citizen-audit diff
  shows only the variant's own entries (the empty package-registry dir stays, known). Each fresh
  postinst rescanned the hub, started the daemon from its deviceroot, registered Luna on first
  start, a card rendered Wikipedia, Palefill landed (Mojo's after ~70 s of an open card — the
  add-on manager is slow to come ready; the installer retries each start). Enyo's db8 history
  exported and put back (69 rows); Mojo's localStorage history restored. Backup of both profiles,
  state dirs and localStorage: `~/eclipse-workspace/Jihad/device-backups/jihad-backup-20261006.tgz`
  (+ `db8-enyo-*.json`). Snapshots in `build/webos-oe/out-audit/*-1006.txt` (git-ignored, host only).
- **Reboot (T-234):** `com.palm.power/shutdown/machineReboot`; back on novacom in ~40 s; 2.5 min
  later all three jobs running from the shipped upstart files, each daemon `luna: serving` with
  no `FAILED` and no retry, public-bus answered, sockets up, no D-state threads, `/` read-only, no
  crash reports, a card per variant rendering a live page. T-236's condition did not arise. The
  standalone `alsa_probe`/`plugin_mime_probe` were not re-run (diagnostic programs from closed
  investigations; the daemon itself plays media and scans plugins).
- **One real print** (T-310's last box): the small page printed to the IPP sink first
  (`evidence-2026-10-06/mochi_print-small-page-as-the-sink-received-it.jpg`, 0.7 % ink), then the
  same dialog to the Epson ET-2800 at 192.168.1.253 over IPPS: `sheet 1 accepted after 456 ms,
  job-id 32`. **The owner should find one sheet with three lines of text in the tray.**
- **Codex round 6** on the inspector-only range (its credits came back, then ran out again before
  the second range — retry 2:53 PM): nine findings, four already fixed, one accepted, four fixed
  now and installed as an upgrade: the padlock follows the COMMITTED address (typed https showed
  the old page's padlock, and its certificate, under the new address — Enyo `pending` through
  ActionBar/URLSearch/AddressInput, Mochi `_pendingUrl`; hook steps `U`/`L` drive it); Cut is bound
  to the field the copy came from and the copy must be under 15 s old; copied text is let go after
  15 s; `services_test`/`xpi_mismatch_test` sinks return `int`; `push-engine-update.sh` refuses a
  full push from a stale bundle before touching the device. Record: `impl-review-findings-6.md`.
- **Tabbed browsing — GROUNDWORK ONLY:** `context/kits/cavekit-tabbed-browsing.md` (R1–R6),
  `docs/TABS-GROUNDWORK.md` (plan B: pages in the daemon, one connection per card, `tabs/*` over
  the variant's own Luna service, YAP untouched; why not one adapter per tab: 37.7 MB of shared
  memory per connection), Tier 71 (T-347…T-354) all PENDING, overview row. Three questions for
  the owner at the end of the design note. Nothing built.

**Host test note:** `services_test` and `input_test` (desktop GTK path under Xvfb) read a black
window before any click — the desktop harness, not the device path; not chased.

**Not done, and why:** the two pushes (`git -C third_party/uxp push fork jihad-engine-mods`;
`git push origin jihad/about-preferences-and-settings-merge`) were refused by the session's
permission gate as outward publication — the owner runs them. The OE SRCREV is bumped to
`db070f0003` (the gitlink's commit; the recipe clones the in-repo submodule, so it needs no
push). T-341 / T-354 and T-280–T-289 / T-317 need a finger.

**Device state:** all three variants from the packages at this commit (bundle `3bafb9bb…`, daemon
`59c72b75…`, libxul `ddf8cc70…`; md5 `cf2fe975 / 5887986c / a5a78d55`), installed as upgrades
after the fresh installs, then REBOOTED and checked. Root read-only (the reboot did it). No hooks,
no staged packages, no backup copy on the device, `/tmp` holds nothing of mine. Palefill in all
three profiles. No card open. Enyo history 70 rows (69 restored + Wikipedia).

## 2026-10-06 (later) delta — Mojo's Copy/Cut/padlock never reached the engine; wide zoomed regions seen; three test hooks

**Why this session existed:** "Is there anything else you can do without me using the device
physically?" — the items that did not need a finger: the Mochi padlock after a failed load, a
pinch through the adapter's own gesture entry points, and a test hook for the Mojo shell. Each
turned up something.

**Found and fixed (device-verified, scripted):**
- **Mojo Copy, Cut and the padlock's `pageFocused` had never reached the engine.** The Mojo WebView
  widget (framework, not ours) implements `copy(callback)` as `this.adapter.copy(callback)`, so the
  shell's `callBrowserAdapter("copy")` handed the adapter one `undefined` argument, which
  `js_copy` refuses ("Bad arguments") — silently, because the shell's "method unavailable" line
  was at `Mojo.Log.warn`, which never reaches the device log (a trap already in the Mojo kit).
  And the widget wraps no `cut` and no `pageFocused` at all. Fix (`app-mojo/app/assistants/
  main-assistant.js`): `askCopy()` always passes a function; a method the widget does not wrap is
  called on the plugin element itself (`view.querySelector("object")` — `element.mojo` is the
  widget's exposed face and does not carry its `adapter`); the unavailable line is now `error`.
  The method-name set is unchanged (kit mojo-ui R3). Verified: word Copy → clipboard; field Paste;
  Select All → Copy/Cut appear; Cut → clipboard has the text, field empty, engine says "Cut.";
  padlock tap on Wikipedia → the Certificate alert; `pageFocused` logged by the daemon.
- **Wide zoomed regions were thrown away at the content's right edge.** The pinch hook (`g` step:
  adapter `gestureStart/Change/End`) does zoom (`setZoomAndScroll zoom=1.2434 scroll=658,686`),
  and the first wide frame came out one viewport wide: `mLastContentW * Zeff` gave 1387 while the
  band (clamped in CSS px, scaled the other way) ended at 1388, and the band-inside-region test
  dropped the wide region. `BrowserPageGoanna.cpp`: the content is at least as wide as the band.
  Verified: `region columns: content 1388 px wide at zoom 1.2434 ... -> [0, +1388), rows up to
  2266`, the composite on screen is right, pinch back to 0.8447 → 943x3335 with sweeps. The
  `region columns:` line logs only when the decision changes (kept; it is what made this visible).
- **Mochi padlock after a failed load** (`page:w6;l;a;w3;l;w3` over `https://no-such-host.invalid/`):
  showing before the engine's alert is answered, gone after, the tap then says "No certificate
  information is available for this page." Mochi hook gained `a` (answer the engine's alert).

**New: a Mojo test hook** — `render/goanna/test/mojo-selftest.js`, `card-selftest.sh mojo push|run|restore`
(the script now knows each shell's sources, manifest and hook path). Steps: `hX,Y` hold, `m`/`mN`
read/pick the open `popupSubmenu` (its model's items; a pick calls the model's `onChoose` with the
item's command, the popup's element removed first — this framework build's submenu has no
`mojo.close`), `l` padlock (`handleAddressTap` at x=12), `d`/`a` read and answer the open
`showAlertDialog`, `k`/`c=`/`p` clipboard, `W`/`P` list the widget's and the plugin's methods,
`?` state. The scenario comes from `PalmSystem.launchParams.jihadTest` (the stage hands the scene
only the url). Every `callBrowserAdapter` call is logged with the road it took (widget / plugin /
nowhere) — that is how the Copy defect showed.

**Review:** Codex still out of credits (retry 6:46 AM) → `ck:inspector` on the diff; findings and
what was done with them are in `context/impl/impl-review-findings-6.md` (round 5).

**Still open (unchanged in kind):** owner-only finger tests (T-341: pan/pinch feel, a hold on a link
per shell, the glyph itself), removal/reinstall (T-231…), hands-on checklist (T-280…), one real
print, pushing the engine branch. The sideways pan still has no script (it is the adapter's drag).
Agent-doable when Codex's credits return (6:46 AM local): a Codex pass over the inspector-only
rounds (3 and 5 in `impl-review-findings-6.md`; the scratch wrapper is described under "Codex
gate" in the delta below). Nothing else of the owner's list is reachable without a finger.

**Device state:** all three variants installed through the installer from the packages built at
this commit (bundle `622ea210…`, daemon `59c72b75…`, libxul unchanged `ddf8cc70…`, adapters
unchanged; packages `build/webos-oe/out-ipk/*.ipk`, md5 `c0273e34 / 3956cd95 / d7b7c1cd`) — the device is
the repository head. Each variant smoke-tested on Wikipedia from the package (title and frames
in its daemon log). Palefill XPI in each profile. Root filesystem read-only. Test hooks: none
loaded, manifests shipped. Staged packages and my `/tmp` files removed (`/tmp` 688 KB used). No
crash reports of mine (`/var/log/reports` holds only `librdx`). No card open.

**Review fixes applied after the inspector (same commit):** see `impl-review-findings-6.md`
round 5. The things to know: the Mojo hook answers an alert through the dialog's own
`close(value)` (a page `confirm()` → "answered yes" on the device) and taps a submenu node with
`Mojo.Event.tap`; Find on Page in Mojo had also never reached the engine (no `findInPage`
wrapper) and now does (`find 'charlie' -> 1`); a copy the engine answers `false` to gets its own
"could not be copied" toast after 1.5 s unless the engine's word came first.

## 2026-10-06 delta — the 2026-10-05 work finished, reviewed, packaged, device cleaned

**Commits (local, NOT pushed):** `657282c4` long-press menu · `debd1934` composed frames + padlock +
review fixes · `afb0db6e` Palefill · `707795de` / `bb16ea4a` docs · `31c5f445` review round 4
(Codex on Palefill, inspector on the last fixes). Engine branch `jihad-engine-mods` (local; owner pushes, then bump the OE SRCREV in
`build/webos-oe/recipes-jihad/goanna/goanna_1.0.bb`): `0048d50fa9` (windowless plugin),
`628ee50c99` (plugin child env), `db070f0003` (AppCompat vs toolkit target).

**What the owner asked for and what it is now** (kits: ui-shell R7/R8, offscreen R9, addons R9,
device-build R4; `context/impl/impl-review-findings-6.md` has every review finding):
- Long-press menu on any content, all three shells (Copy/Cut/Paste/Select All, page actions).
  Device, Enyo + Mochi, scripted holds. Mojo needs a finger.
- "Pages only load when observed": frames are composed (copy then render ≤ 768 new rows), region
  is the whole segment, sweeps, wide when zoomed. Row-exact after jumps. A real pan needs a finger.
- DDG "links don't work" = Flash pages killing the daemon in a loop → engine fix + crash guard.
- Padlock → certificate (+ connection, trust state). Enyo + Mochi device-verified; Mojo drawn.
- Palefill 1.30 bundled unmodified (MPL-2.0, hash-pinned, NOTICE); installed enabled in all three.
- Codex gate: Cavekit 3.1's script is silently incompatible with Codex CLI 0.160 — run
  `codex exec` directly with its prompt; `ck:inspector` when Codex's credits run out (they did,
  twice). Memory: `codex-gate-needs-direct-invocation.md`, `device-test-limits.md`.

**Device state:** all three variants installed through the installer from the packages built at
`31c5f445` (bundle `4887551a…`, daemon `b7be3e3c…`, libxul `ddf8cc70…`, adapters `88303cfc /
51d10886 / 1b0ac09d`, Palefill in each) — the device IS the repository head; nothing hand-pushed
on top. Packages: `build/webos-oe/out-ipk/net.riverstonerelay.jihad-browser{,-mochi,-mojo}_*.ipk`
(Enyo 1.0.4, the others 1.0.0). Root filesystem read-only. Test hooks restored (none loaded). `/tmp` holds nothing of
mine (39 MB free). `/var/palm/jihad/adapter-backup-20261005` removed; the 2026-08-10 backup is
still there. Crash reports: all of 2026-10-05's were mine (Flash loop, busybox, one
`media-pipeline.real` SIGSEGV at 15:18 during the first package install) and were deleted;
my attempt to copy them to the host first produced empty files, so they are gone unread.
Palefill in every profile. No card open.

**Still open:**
- Owner only: finger tests (pan/pinch smoothness; Mojo menu + padlock tap; Mochi padlock after a
  failed load; a hold on a link per shell) — T-341; Enyo/Mojo removal + install from nothing
  (T-231…); hands-on checklist (T-280…); one real print.
- Known gaps left on purpose: fields inside iframes get the page menu; Palefill buffers whole
  responses of the sites it rewrites (its own design); one `window.jihadClip` per card.
- Trap: after an upgrade that replaces the adapter, `/` will not remount read-only until
  LunaSysMgr restarts.

## 2026-10-05 delta — printing built; a stale-script defect found and fixed; Mochi installed from nothing (read first)

Per-task record: `context/impl/impl-ce310-loop.md` (rows from "2026-10-05"); evidence:
`context/impl/evidence-2026-10-05/`; plan: tiers 62–65 of `context/plans/build-site.md`.
**Nothing was pushed.** 10 local commits since `c726e8a7`.

**What the owner asked, and the answers:**
- *"I have a network printer, so why can't the TouchPad see it?"* It can. webOS's print daemon
  resolved the Epson over mDNS and logged `not HP printer and doing printer filtering` — the stock
  print stack discards every printer that is not HP. So the browser now prints by itself:
  `render/goanna/components/jihadPrint.js` finds IPP printers (mDNS, or a typed address), draws the
  page as JPEG sheets and sends them over IPP/IPPS; both the Enyo and the Mochi shell have a Print
  dialog (`JihadPrintDialog.js`) in place of the system one. On the device both dialogs list the
  Epson ET-2800 (192.168.1.253) and a Mac-shared ET-2720. Mochi's Print box is `[x]`; the parity
  list has no open row.
- *Share Image / Set as Wallpaper:* both run on the Mochi card through the long-press sheet's own
  handlers. The wallpaper changed and was put back (`19.jpg`); Share Image opened Email.
- *Why was `/` read-write?* The installer service (`org.webosinternals.ipkgservice`, Preware's
  backend) remounts `/` read-write before running a package's script and never restores it. Measured
  from a read-only start. Our scripts leave the state as found. A reboot, or
  `mount -o remount,ro /`, puts it back.
- *The Atlas fault report:* Atlas's own — a SIGBUS in its allocator thread at the 2026-10-02
  shutdown, nothing of ours in it. Atlas and a Jihad card rendered side by side again today.

**An accident, and the defect behind it.** A print test meant as a dry run put one full dark sheet
through the owner's Epson. The new print component was on the device and the daemon had been
restarted, but the engine ran the PREVIOUS version from its compiled-script cache
(`$APP/cache/startupCache`, keyed by path, never invalidated in an embedding — and untouched by an
`.ipk` upgrade, so every upgraded script was exposed to the same thing). Fixed in the daemon
(commit `c4e9bb9e`): it fingerprints the bundle at start and drops that cache when the bundle
changed. Look for `startup cache: bundle …` in the daemon log.

**Traps found this session:**
- **Test printing against `render/goanna/test/ipp-sink.py` first**, never the real printer:
  `python3 render/goanna/test/ipp-sink.py --port 8090 --out DIR`, then
  `card-selftest.sh <shell> run print:192.168.1.245:8090 <url> 66`. It keeps each sheet as a JPEG.
- After pushing ANY engine script by hand, confirm `startup cache: bundle CHANGED … dropped` before
  believing the new code is what runs.
- mDNS from an ephemeral port gets no answers on this device: the firewall admits only
  `udp dpt:5353` and established flows. Ask from port 5353.
- The Luna hub forwards a bus SIGNAL only from a sender named `com.palm.*`, and the device's
  `luna-send -s` needs `-m com.palm.lunasend` for that. Without it the signal shows in `ls-monitor`
  and reaches nobody.
- `ipkgservice/install` wants BOTH `filename` (the bare name) and
  `url` (`file:///media/internal/.developer/<name>`), and after a FIRST install the launcher does not
  know the app until `ipkgservice/rescan` (Preware calls it; a script must too).
- A card's `luna-send` on the device quits before its answer if its stdin is at end-of-file — which
  is what a carelessly held novacom session gives it. Use `build/webos-oe/dev-sh.sh`.
- `com.palm.display/control/setState off` is refused while on USB (`blockDisplay`), and there is no
  touch input device to inject a tap into.

**Tools added:** `build/webos-oe/luna-restart.sh` (restart LunaSysMgr and close the USB alert it
leaves over the top right of the screen; `--close` for the alert alone), `build/webos-oe/dev-sh.sh`
(run a local script on the device, all output, returns when it ends),
`render/goanna/test/ipp-sink.py`, `packaging/branding/make-enyo-toolbar-icons.py` (the Home and
Bookmarks/History toolbar icons, redrawn at the owner's request), print scenarios in both test
hooks, `holdpick` in the Mochi hook. `device-citizen-audit.sh` now lists the installer's package
directories.

**Still open, and why:**
- Enyo and Mojo removal and install from nothing (T-231…T-234, T-236, T-273): only the owner can run
  a removal on this device. Mochi's half is done. When that happens, look at
  `/media/cryptofs/apps/usr/palm/packages/<id>` afterwards: Mochi's removal left it as an empty
  directory (recorded in cavekit-device-build.md R8, not fixed — `prerm` cannot `rmdir` a directory
  ipkg still has files in). The test extension is no longer installed in any profile; T-273 needs it
  put back in two variants first.
- A sheet of paper printed from the dialog, seen by a person; a real long-press; and the rest of
  the hands-on checklist (T-280…T-289, T-317) — deferred by the owner.
- Not built yet: the daemon's "luna: serving" log line now names `findPrinters` / `printPage` in
  source, but the installed daemon predates that one-string change.

**Device state as left (2026-10-05):**
- All three variants installed from the same build through the installer: bundle id
  `a4a2f8e2b8bf…`, libxul `65e3cc84891c…`, daemon `889263013e40…`. Mochi was a fresh install, Enyo
  (1.0.4) and Mojo upgrades. Each daemon runs from its own deviceroot and answers on the public bus.
- Jobs: Enyo and Mojo as shipped; **Mochi's job carries `JIHAD_INJECT=1`** (added by the install
  helper). No test hook in any app directory. No extension in any profile.
- `/` is read-only (put back by hand after the installs). Wallpaper `19.jpg`, as found. No Email or
  Atlas card left open; one Enyo browser card is open.
- `/tmp` keeps the helpers (`fresh-card.sh`, `inst3.sh`, `rm1.sh`) and the test pages, including
  `jihad-print.html` and `jihad-print-tall.html`. `/tmp` is a tmpfs: all of it goes at a reboot.
- Old staged packages are still in `/media/internal/.developer/` from earlier sessions
  (`jihad-browser.ipk`, `jihad-mochi.ipk`, `jihad-mojo.ipk`, the 1.0.2 and 1.0.3 Enyo `.ipk`s) —
  about 230 MB of the user's volume that nothing uses. Left for the owner to delete.

---

## 2026-10-02 delta — owner decisions carried out; the Mochi parity omissions built (read first)

The owner answered the open decisions and gave a go-ahead for the removal audit, the engine push
and a reboot. Per-task record: `context/impl/impl-ce310-loop.md` (rows from "2026-10-02" on);
evidence: `context/impl/evidence-2026-10-02/`; plan: tiers 54–61 of `context/plans/build-site.md`.
**Nothing was pushed** (see "Waiting on the owner").

**Decisions taken, and done:**
- The card panels lose Block Popups / Accept Cookies / Enable JavaScript (Enyo, Mochi); the settings
  page owns them; "More Settings" opens it. The Enyo card no longer replays its stored copies.
- Flash comes from the device's own plugin folder (`/media/cryptofs/apps/usr/lib/BrowserServerPlugins`,
  the HP Flash plugin app) — scanned after the profile folder, skipped when the profile has its own
  copy. One stored on/off value (`plugin.state.flash`): a row on the settings page, a real switch in
  the Enyo and Mochi panels through the variant's Luna service (`getFlash` / `enableFlash` /
  `disableFlash`; the setters accept only the variant's own card).
- No Mochi parity omission was accepted. Built and shown on the device: bookmark editing, bookmark
  thumbnails, downloads a page starts (one fetch, with retry), long-press menu, share sheet and Add
  to Launcher, certificate details, search-engine choice, Flash switch, Clear History that clears the
  engine too, a confirmation before each clear, grouped history dates. `app-mochi/PARITY.md` is
  refreshed. **Print is not built**: no printer the device can reach, so nothing could be verified.

**Defects found on the way and fixed:**
- Closing ANY card unregistered the add-on install prompt and the JS notification door for the rest
  of the daemon's life (the page destructor cleared a process-wide sink, and clearing it dropped
  once-per-process observers). Installs were downloaded and then cancelled with no prompt.
- The Mochi bookmarks list could never show a row (ordered by an un-indexed db8 property), and a
  Mochi panel hidden at layout time opened blank (never measured).
- With a profile copy and the system folder both scanned, Flash was registered twice.
- The Enyo certificate view could never open on webOS CE (see traps) and threw on a certificate with
  no common name.

**Waiting on the owner (approved, but the session's permission gate refused both again):**
- Engine push: `git -C third_party/uxp push fork jihad-engine-mods`, then bump the gitlink and the OE
  SRCREV (T-279). Two local engine commits: `1fff802335`, `9dedcd5fe5`.
- Variant removal (T-231…T-234, T-273): the audit script is on the device —
  `/tmp/rm1.sh <variant> <appid> <job> <luna name> <shim>` (volatile: re-push it from the scratch
  copy or rewrite it if `/tmp` was cleared). The test extension is installed in the Enyo and Mochi
  profiles for the "its extensions go, the other's stay" check. Profile backups of all three variants
  were taken to the host before anything else.
- A printer the device can reach, for Print (T-310).
- The human checklist (T-317): hands-on steps, listed at the end of the session's report.

**Traps found this session:**
- `/tmp` on the device is a 40 MB tmpfs. Three profile tarballs filled it; `novacom put` then fails
  with "No space left on device". Stream large things straight to the host instead.
- A card may load a local file ONLY from its own application directory or `/media/internal`
  (LunaSysMgr `isAllowedFileAccess`), and its WebKit refuses a local file over XMLHttpRequest. A
  script or image tag pointing into the app directory works. `/var/…` and `/tmp/…` do not.
- `com.palm.certificatemanager` does not exist for a card on webOS CE ("Service does not exist").
- While an engine dialog is up the daemon answers nothing over Luna either.
- This Enyo 2 has `resize()`, not `resized()`; a panel shown after layout needs it.
- db8 refuses an ordered query on a property the kind has no index for (`-3965`), and an Enyo 2 card
  shows that as an empty list, not an error.
- `pkill -f <pattern>` kills the shell that runs it if the pattern is in its own command line.
- An upgrade `.ipk` install overwrites the package's files but does NOT remove an extra file: a
  pushed test hook's manifest edit is undone by an install, its `JihadSelfTest.js` is left lying in
  the app directory. Always finish a hook session with `card-selftest.sh <shell> restore`.

**Tools added:** `build/webos-oe/card-selftest.sh` (+ `render/goanna/test/{mochi,enyo}-selftest.js`)
drives a shell's own chrome on the device through its handlers; `build/webos-oe/prefs-row-walk.sh`
walks every settings-page row.

**Device state as left:**
- All three variants installed through `org.webosinternals.ipkgservice`: libxul
  `65e3cc84891c59e9ede40cc7b3de0802`, daemon `1a70344214e9e266c894782ed10286b9`, goanna.js `97f981b3bc03…`,
  PDK adapter impls unchanged (enyo `6a35d778…`, mochi `44ab2e23…`, mojo `25aa85a4…`). Shell sources
  on the device compare equal (md5) to the repo at the last commit. The Enyo package installed is
  `690d8c3c…`; Mochi and Mojo were installed from an earlier build of the same payload
  (`a9ca38d8…`, `59d727a0…`) — the copies now in `build/webos-oe/out-ipk/` are a later rebuild whose
  payload is identical but whose package bytes are not.
- **The device was rebooted once** (owner-approved) with the shipped, unmodified jobs: all three
  daemons came up, Luna registered at first start, extensions survived. Afterwards `JIHAD_INJECT=1`
  was re-added to the three jobs (remount rw → edit → remount ro) and the daemons restarted. The
  root filesystem is READ-ONLY at rest now (it was read-write before the reboot, found that way).
- Flash: no variant has a copy in its profile any more (the hand-copied one was removed from Enyo);
  all three get it from the system folder; the value is ON in all three.
- **The test extension `jihad-effect@riverstonerelay.net` is installed in the Enyo and Mochi profiles
  on purpose** (for the removal audit) — it turns every page's background magenta in those two
  variants. Remove it from `about:addons` once the audit is done.
- `/tmp` was wiped by the reboot. Restaged: the test pages (`stage-test-pages.sh`), `fresh-card.sh`,
  `inst3.sh`, `fin1.sh`, `ps.sh` and the removal audit script `rm1.sh`. NOT restaged: the media clips
  (`stage-media-pages.sh`), the Flash test movies except `jihad-anim`, `uinput_kbd`.
- Cards open: Enyo and Mochi on `file:///tmp/jihad-plain.html`. No test hook on the device: both
  manifests are the shipped ones and no `JihadSelfTest.js` is left (an upgrade install leaves an
  extra file behind — it was removed by hand).
- Mochi card data left from the tests: a few history rows and downloads-view rows (their files are
  deleted); the search engine is back on DuckDuckGo; no bookmarks.
- Four fault reports in `/var/log/reports/librdx/`; the new one is `BrowserServer-atlas` SIGBUS at
  the moment of the reboot — Atlas's daemon, not ours. LunaSysMgr was restarted about a dozen times
  by `card-selftest.sh push` / `restore` (by design: it caches card JS).
- `/var` 23%. Residue still to delete by hand: `/media/internal/downloads/jihad-testfile{,-1,-2}.bin`.
- Cavekit: registry 96 complete, 18 blocked (owner / person / printer), 1 pending behind them. The
  loop was torn down at the stop. `.cavekit/config` carries an uncommitted edit of the owner's.

---

## 2026-10-01 (third session) delta — every agent-doable sweep run on CE 3.1.0 (SUPERSEDED where the delta above disagrees)

`/ck:make` ran the rest of the build site inline. Registry: 72 complete, 18 blocked (owner / human),
1 pending behind a blocked task (T-236). Per-task record: `context/impl/impl-ce310-loop.md`; the kits
carry the evidence on each criterion; screenshots in `context/impl/evidence-2026-10-01/`.
**Nothing was pushed.** Engine commits `1fff802335` and `9dedcd5fe5` on `third_party/uxp`
`jihad-engine-mods` are LOCAL; the parent gitlink and the OE SRCREV still say `bd49435b` (T-279).

**Verified on the device (webOS CE 3.1.0), all three variants unless said:**
- Preferences: the page renders and writes only its own variant's `prefs.js`; typed values save on
  blur and on Enter; JavaScript row is back and survives a card relaunch and a daemon restart; the four
  frozen settings work from the page and from the shipping Enyo card; Mojo/Enyo/Mochi adopt page edits.
  Every one of the 56 table rows was then walked on the device through its own control
  (`build/webos-oe/prefs-row-walk.sh`): all 56 change the engine pref they name. A real key path
  (the kernel keyboard) types into a settings field.
- `window.open` from a tap opens a new card (new, `nsIWindowProvider` → `msgCreatePage`).
- Input sweep (taps, click count, toggles, key events, non-ASCII, selection, Enter/Tab, zoomed link),
  `about:config` end to end, the `about:addons` tools menu, touch events with one and two fingers.
- Offscreen sweep (size tracking, exact colours, buffer rotation, messages), `<select>` → card list.
- Shell sweeps: launch target on the variant's own socket, find, alert, failed load, 302, HTTP auth.
- Daemon sweeps: cookie + disk-cache persistence, low-memory guardrail, public `clearCookies` /
  `clearCache` on install-started daemons, dialogs hold the page.
- Device-build sweep: C++14 program, ELF attributes = CPU, memory (126 MB after eight sites, cap 150),
  the seven adapter-deps libraries match the manifest on CE.
- Key codes MEASURED from a real key path. `render/goanna/test/uinput_kbd.c` makes a kernel keyboard
  that LunaSysMgr on CE takes for a Bluetooth one; its keys reach the card and the daemon.
- Volume HUD: a volume KEY press during page audio changes the level and raises the HUD.
- Flash 33 fps; the in-tree control plugin loads, draws at the right place, takes a tap and a key.

**Defects found by the sweeps and fixed (all device-verified, all committed):**
no `change` event after typing; number fields edited the wrong element; chrome pages never scrolled
natively; the card re-enabled JavaScript on every connect; settings edits reverted on Back;
pages without a viewport tag drew as a thumbnail in Mojo; page key events carried raw webOS codes;
Up/Down were swapped, no Home/End; a field low on a tall page stayed under the keyboard; popup taps
were divided by the zoom twice and an opened menu was not painted; Mojo ignored a launch `target`;
find/selection was white on white; HTTP auth raised no dialog at all; damage rectangles were not
scaled at a fit zoom (clipped plugin, inexact Flash box).

**Still open, and why:**
- **Needs the owner's word:** removing a variant (T-231–T-234, T-273 — the permission gate refused
  it as irreversible deletion, so fresh-from-nothing install, the removal audit and "extensions of a
  removed variant" are untested); pushing the engine branch and bumping gitlink + SRCREV (T-279);
  how a user gets Flash; which rows the card's own settings panel should keep (T-290); Flash pacing
  B-vs-C (T-287); anything that needs a reboot (no reboot was made).
- **Needs a person at the device (T-280–T-291):** finger on card chrome, media controls, VKB typing
  and held Backspace, pinch, a real double tap on a plugin (T-212 is built and on the device), the
  physical volume rocker, the Mochi parity sign-off.
- **Known and recorded, not fixed:** arrow keys do not scroll a fit-zoomed page; the keyboard-cover
  check runs only at zoom 1; LunaSysMgr sometimes keeps Up/Left/Right from the kernel keyboard for
  itself; a failed load shows the engine's alert AND the shell's error panel; `window.open` gives
  the opener no window handle.
- **GRE key handover (gre-widgets R3):** `JIHAD_GRE_KEYS=1` in a job file turns it on for a session.
  It works for typing/arrows/Home/End/Backspace/Delete and does not crash; Enter/Tab stay on the
  daemon path. A clean handover needs two engine changes — see the kit.

**Device state as left:**
- All three variants installed through `org.webosinternals.ipkgservice` from the packages now in
  `build/webos-oe/out-ipk/` (enyo 1.0.4 `a0df34da…`, mochi 1.0.0 `dbe4f747…`, mojo 1.0.0 `074be9d3…`):
  libxul `65e3cc84891c59e9ede40cc7b3de0802`, daemon `50615b82ba73f06bb72cdbfc30a663fd`, goanna.js
  `82f9bbb3150b…`, PDK adapter impls enyo `6a35d778…` mochi `44ab2e23…` mojo `25aa85a4…`. Daemons were
  started by the installs; LunaSysMgr was not restarted by them (same pids before and after).
- Jobs are the shipped ones plus `JIHAD_INJECT=1` (re-added by hand after each install — an install
  writes the shipped job). No `JIHAD_GRE_KEYS`, `PULSE_SINK`, pacing or log env left behind.
- Enyo profile has `plugins/libflashplayer.so` (hand-copied, stock md5); the test plugin was removed.
  Mochi/Mojo have none. No test pref is left in any `prefs.js` (Enyo's was compared against a copy
  after the row walk).
- Cards left open: Enyo on `about:preferences`, Mochi and Mojo on `file:///tmp/jihad-shell.html` (none on `about:blank`).
- The uinput test keyboard is not running. `/tmp` holds test pages, scripts, `uinput_kbd` and
  screenshots (volatile). Residue to delete by hand: `/media/internal/downloads/jihad-testfile{,-1,-2}.bin`.
  `/media/internal/.developer/` holds our three `.ipk`s next to other people's.
- No reboot was made all session. `/` is read-write at rest (as found). `/var` at 21%.
- Three fault reports in `/var/log/reports/librdx/`, all from before this session's sweeps (the
  upgrade crash of the second session and two deliberate kills); none new.
- Cavekit: the loop is torn down (`teardown-loop`). The default `session_budget` of 500,000 stopped
  it at 1.1M tokens; the owner raised the live number in `.cavekit/token-ledger.json` to 10,000,000.
  `.cavekit/config.json` still says 500,000 and is what a NEW ledger starts from. `.cavekit/config` is
  a `key=value` file (it holds `codex_model`) and has an uncommitted edit of the owner's.

**Traps found this session:**
- Inject `click X Y` takes DOCUMENT px × zoom; `rect` answers in viewport CSS px. The card zooms to
  1.0 when a field takes focus, so coordinates taken before the first focus are wrong after it.
- `57346` is `0xE002`, not `0xE0A2` (57506). Arrow codes: Up 57504, Down 57505, Left 57506, Right 57507.
- Card JS is cached in LunaSysMgr until it restarts; `push-card-js.sh` handles it.
- `takeScreenShot` writes asynchronously: wait two seconds before reading the file.
- The `partial paint` log line is rate-limited to one per 2 s — it is not a frame counter.
- A banned port (e.g. 9) raises an engine alert that holds the daemon until the pipe is answered.
- An engine dialog holds the daemon: answer the FIFO named in the `dialog … -> card (pipe …)` line.
- `goanna.js` on the device comes from `make-device-bundle.sh`; a pref edit needs it re-run.
- A pref SET through the page while its value equals the default leaves no line in `prefs.js`.

---

## 2026-10-01 (second session) delta — kits updated, six defects fixed and device-verified; stopped early on the usage limit (SUPERSEDED where the delta above disagrees)

Kits were brought in line with CE 3.1.0 first (commit `f12c1111`), the build site regenerated
(`context/plans/build-site.md`, T-201…T-293), then `/ck:make` ran inline. Per-task record with every
number: `context/impl/impl-ce310-loop.md`. Screenshots: `context/impl/evidence-2026-10-01/`.
Nothing was pushed. The Cavekit loop was torn down at the stop (usage limit); the task registry (`.cavekit/task-status.json`, 37/91 complete) is intact and `/ck:resume` picks it up.

**Verified on the device (webOS CE 3.1.0), kit boxes closed:**
- `<video>` plays to the end — engine fix `1fff8023` on `third_party/uxp` `jihad-engine-mods`
  (LOCAL, unpushed; gitlink and OE SRCREV still `bd49435b`). 12/12 runs, VP8 + Theora, file + http.
- Codec table measured (VP8/VP9/Theora/Vorbis/Opus/WAV/FLAC play; H.264/MP3/AAC refused). FLAC needed
  the ffvpx libs, which were built but never bundled — now in `make-device-bundle.sh` and the push script.
- Idle daemon: 0.05% CPU, ~1 wakeup/s, 0 log bytes (was 2.4%, 117/s, 48 B/s). Log capped while running
  (copy-then-truncate, two in-lifetime rotations shown). `about:blank` settles (0 paints / 30 s).
- Card open on a static page: 2.2% CPU (was 12.3%) — quiet tick.
- Luna -1027 on fresh install: CONFIRMED, fixed in postinst (hub rescan) and daemon (retry), verified
  through `org.webosinternals.ipkgservice` on all three variants.
- Upgrade crash found and fixed: postinst `cp` over the mapped adapter shim killed LunaSysMgr at the
  next card launch; it now replaces by rename (three upgrades since, LunaSysMgr survived).
- TLS: `TLS 1.3` captured from howsmyssl and Cloudflare trace.
- Media controls: the stuck `0:00` scrubber bubble reproduced and fixed (daemon grows the damage box);
  click-seek works; mute/slider reach the pulse stream. Play/pause both directions.
- Cert store on CE works; two daemon fixes (retry the FAILED url after trust; drop the redundant
  engine error alert before the SSL confirm). Toast seen in all three shells.
- R9 probes: glibc 2.8 writes the same pshared-mutex `__kind` as 2.23 (the "second trap" was a
  2.5-proxy artefact); Flash's 67-lib closure resolves on CE.
- Flash on CE: works once `libflashplayer.so` is in the profile. The shipped build was pacing it on
  the 8 ms tick (worst histogram); the daemon now uses 16 ms while a plugin is on the page.

**Still open, and why:**
- **Fresh install from NOTHING + the postinst reboot (T-231…T-234, T-273):** removing a variant was
  refused by the session's permission gate (irreversible deletion). Needs the owner's go-ahead. The
  installs done today were install-OVER through ipkgservice.
- **Human-only (not attempted, not counted):** finger on the media controls / volume slider, volume
  HUD (needs a hardware key press), Flash B-vs-C eyeball, Downloads-app look, card-chrome taps, VKB.
- **Not reached (usage limit):** Atlas coexistence run (identity columns ARE checked: all distinct),
  volume-HUD write-up (T-240), add-ons/XPI sweep, preferences sweep, input sweep, offscreen sweep,
  shell sweeps, daemon sweeps, memory sweep, T-212 (plugin VKB), T-214 (arrow-key inject finding),
  rebuild of the three `.ipk`s.
- **Decisions for the owner:** how a user is meant to get Flash (nothing ships or installs
  `libflashplayer.so`; the CE profile had none); push `1fff8023` to the fork and bump gitlink + SRCREV.
- v3.1.0 `scripts/codex-review.sh` still needs `a833abf` cherry-picked; no Codex tier gate ran.

**Device state as left:**
- All three variants ipkg-registered, libxul `65e3cc84891c59e9ede40cc7b3de0802`, daemon
  `46d3714d430162502b3630cf63d4854b`, goanna.js `01154654…`, ffvpx libs present. **The `.ipk`s in
  `build/webos-oe/out-ipk/` are STALE** (daemon `3e29f045`, old Mojo toast): run `build-variant-ipk.sh`.
- Jobs are the shipped ones plus `JIHAD_INJECT=1` (all three). No pacing or log-cap env left behind.
  Enyo's `MOZ_LOG` is gone.
- Enyo profile has `plugins/libflashplayer.so` (hand-copied, stock md5). Mochi/Mojo have none.
- Mojo toast fix (`bottom: 84px`) is on the device but the card runs cached JS until LunaSysMgr restarts.
- LunaSysMgr crashed and restarted once today (the upgrade defect, 14:49 UTC; report in
  `/var/log/reports/librdx/`). The USB dialog it brought up was dismissed. Display set
  `onWhenConnected:true`.
- Residue to delete: `/media/internal/downloads/jihad-testfile{,-1,-2}.bin`; `/tmp/jm/*` and
  `/tmp/*.sh`/`*.png` are volatile. Other people's ipks in `/media/internal/.developer/` were left alone.
- `/` is mounted read-write at rest on this CE image (found that way). `/var` at 13%.
- Platform cert store holds session serials 1–3 from the tests (swept at daemon teardown/startup).

**Traps found today (all in `impl-ce310-loop.md`):** display off/locked or a system dialog makes
`takeScreenShot` return a stale frame; a second disconnected card is what gets photographed; inject
`click` takes CARD px, `clickoff`/`clickid` bypass the input queue; a `launch` in the first minute
after an ipkgservice install is dropped; `ipkgservice/install` returns before it is finished.

---

## 2026-10-01 session delta — device is now webOS CE 3.1.0, Cavekit is now v3.1.0 (SUPERSEDED by the delta above; kept as the stock-take)

This was a stock-take plus one finished diagnosis. **No kit was edited and nothing was fixed** — the
user's standing instruction is "update the kit criteria to reflect the current state BEFORE completing
tasks", and the session ended by moving the tooling to Cavekit 3, which needs a restart. Everything
below is input for that kit update. Nothing here is committed except what `git log` shows.

### Process: Cavekit v3.1.0 (was v2.0.0)

- `~/.cavekit` is on branch `cavekit-3.1.0` (tag `v3.1.0`, `9aa1905`). The two local v2 edits are saved
  as commits on `local/v2.0.0-edits-2026-10-01` (`a833abf` codex-review invocation, `b062631` findings).
  **v3.1.0's `scripts/codex-review.sh` still carries the stale `codex --approval-mode … -p` call and
  `o4-mini`**, so Codex review degrades until `a833abf` is cherry-picked onto it.
- **Start Claude Code from `Jihad-Browser/`, not from the parent `Jihad/`.** v3 resolves `context/` and
  `.cavekit/` against the CWD (`cavekit-tools.cjs`: `CAVEKIT_PROJECT_ROOT || process.cwd()`), and both
  live here. The parent's `CLAUDE.md` still loads from a subdirectory.
- First commands: `/ck:init` (idempotent — creates the v3 runtime state next to the existing v2
  `.cavekit/config`), then the kit update below, then `/ck:map`, then `/ck:make`. `/ck:status` and
  `/ck:resume` are new. The `bp:` alias is gone.
- **TRAP — v3's tool cache will serve stale device readings.** While a loop is active
  (`.cavekit/.loop.json` exists) a PreToolUse hook answers `Read`/`Grep`/`Glob`, and any Bash command
  that BEGINS with `ls|find|cat|head|tail|wc|which|file|git status|log|diff|show…`, from a 120 s cache.
  A device probe or a re-read of a growing log that starts with one of those words returns the previous
  output. Commands that begin with `novacom`, `cd`, or a variable assignment are never cached — keep
  device commands in that shape.

### The device: webOS CE 3.1.0 (build 86, BUILDTIME 20260830), freshly installed

Measured on the device, not assumed:

- Kernel `2.6.35-palm-tenderloin` and **glibc 2.8 are unchanged**, so every cross-ABI statement in
  cavekit-device-build.md R9 still describes this device.
- Stock OpenSSL 0.9.8 is still `/usr/lib/libssl.so.0.9.8`; the new stack is side-by-side OpenSSL 1.1.1w
  (`ssl11`, `ssl11dl`, `ssl11mail`; `curl 7.88.1 … OpenSSL/1.1.1w`). **Jihad uses neither** — page TLS is
  the engine's own NSS 3.90 (`security.tls.version.max=4`). HTTPS loads work (`https://example.com`,
  `https://www.howsmyssl.com/a/check`), but **the negotiated `tls_version` string was never captured**
  (the grep for it came back empty on 2026-09-27) — that one read is still owed before any kit says
  "TLS 1.3 verified".
- CE preinstalls the **Atlas** browser (`org.webosports.app.atlas`, `BrowserAdapterAtlas.so`,
  `/tmp/yapserver.atlas`, its own daemon). A fourth browser now coexists; R7 independence has not been
  re-checked against it.
- Flash is still present (`com.palm.app.flashplugin-001`, same `libflashplayer.so`), the
  `com.palm.flashgraphics` role name is in the stock browserServer role files, and there is still no
  keyboard (`gpio-keys`, `pmic8058_pwrkey`, `headset` only).
- **Every `[x]` that says "device-verified" was verified on webOS 3.0.5.** None has been re-run on CE
  3.1.0 except what is listed in this delta.

### What the interrupted 2026-09-27 session did (recovered from its transcript)

Rebuilt the daemon (YAP contract OK, 73 commands), built all three `.ipk`s (enyo 1.0.4, mochi/mojo
1.0.0) and deployed all three with `push-variant.sh` (the real postinst, as root). On CE 3.1.0 that
gave: three daemons running, three YAP sockets, card renders, pages load over HTTP and HTTPS.

- **`<audio>` works on a clean install.** Vorbis: `paused=false t=9.926 dur=15`, ALSA `state: RUNNING`,
  pulse sink-input RUNNING at 100%. The 2026-08-20 "currentTime never advances, stream stuck PREPARED"
  finding in `context/impl/impl-media-controls.md` was an artifact of the hand-pushed engine that
  session left behind — it does not reproduce. The scrubber/volume-widget/volume-HUD items in that file
  have NOT been re-tested.
- **Fresh-install LS2 ordering — unverified, looks real.** The first daemon start after postinst logged
  `LSRegisterPalmService('net.riverstonerelay.jihadBrowser') FAILED (code=-1027)`; it registered after
  `ls-control scan-services` plus a daemon restart. If postinst starts the daemon before the hub has
  rescanned the role files, the toast channel and `clearCookies`/`getChromePrefs` are dead until the
  next restart. Read `packaging/gen-variant-scripts.sh` before believing either way.

### NEW DEFECT 1 — `<video>` dies with MEDIA_ERR_DECODE after 10–60 frames. ROOT CAUSE FOUND, fix NOT applied

Symptom, every codec (VP8, Theora), `file://` and `http://`:
`err=3:0x8007000e: DoDecode`, `vq=9/0/0` … `57/0/0`, then playback stops. The frame count at failure
varies run to run, which is the tell.

Cause, from two rounds of temporary stderr traces plus the source:

1. `BasicImageLayer::Paint` (`gfx/layers/basic/BasicImageLayer.cpp:72`) swaps the `ImageContainer`'s
   factory to the BasicLayerManager's `BasicImageFactory` **for the duration of the paint** and restores
   it afterwards. `IsCompositingCheap()` is false for BasicLayers, and BasicLayers is what paints here
   because OMTC is off (`JIHAD_DISABLE_OMTC=1`).
2. The decoder thread calls `ImageContainer::CreatePlanarYCbCrImage()` concurrently. Inside that window
   it gets a `BasicPlanarYCbCrImage`.
3. `VideoData::SetVideoDataToImage` always does `SetDelayedConversion(true)` then `CopyData()`, and
   `BasicPlanarYCbCrImage::CopyData` (`gfx/layers/basic/BasicImages.cpp:88`) does
   `RecyclingPlanarYCbCrImage::CopyData(aData); if (mDelayedConversion) { return false; }` — it reports
   failure for a copy that succeeded.
4. `CreateAndCopyData` returns null → the decoder returns `NS_ERROR_OUT_OF_MEMORY` → MEDIA_ERR_DECODE.

On desktop the paint window is microseconds; on the TouchPad a software paint is 40–160 ms, so a decode
lands inside it within a second or two. Evidence: the trace fired `CreateAndCopyData:
SetVideoDataToImage/CopyData FAILED` with NONE of the inner allocation/lock/size traces — i.e. the base
copy succeeded and the `false` came from the delayed-conversion line.

Proposed fix (three lines, engine, **not built, not verified**): in `BasicPlanarYCbCrImage::CopyData`,
return the base class's result and return `true` under delayed conversion. A delayed image is fully
usable — `GetAsSourceSurface()` already converts on demand when `mDecodedBuffer` is null.

This also re-dates two kit boxes: "VP8 plays" (2026-08-10) and the play/pause `[~]` (2026-08-15,
`t=0.73`) were both read inside the first second, before the failure.

Test assets: device `/tmp/jm/` (volatile) holds `jihad-media.html` (state readout in `#state`, pick the
clip with `?v=clip10v.webm`), `clip10{,v,a,g,op}.webm`, `clip10.ogv`, `tone15.{ogg,wav}`. Regenerate:
`ffmpeg -f lavfi -i "testsrc=size=320x240:rate=15:duration=10" -c:v libvpx -b:v 200k -an clip10v.webm`,
`ffmpeg -f lavfi -i "sine=frequency=440:duration=15" -c:a libvorbis -q:a 3 tone15.ogg`. The page itself
was pulled off the device and is now `render/goanna/test/jihad-media.html` (untracked, md5-matched to
the device copy); the clips are regenerated, not stored.

### NEW DEFECT 2 — an idle daemon fills `/var`

A daemon with NO card connected writes one `tick period` line every 2 s and nothing else: **82 B/s,
~7 MB/day per variant** while the device is awake. mochi and mojo had reached **10.1 MB each** after
3.6 days idle, on a 62 MB `/var`. Three variants idle is ~21 MB/day. The job rotates the log only at
start. Same family as `jihad-var-full-kills-daemon-log`: a full `/var` freezes the log and a frozen log
reads as a live one. The same lines show the idle daemon ticking at 8 ms with no page (`n≈241` per 2 s,
~2.8% CPU each, indefinitely).

### NEW DEFECT 3 — `about:blank` is a repaint storm

On `about:blank` the daemon logs `painted shmid=… bytes=0 … ms=35` about 22 times a second forever
(663 full paints in 30 s, 2.4 KB/s of log, ~210 MB/day). The full path gets a zero-byte frame, will not
publish it, and tries again every tick. `about:blank` is an allowlisted home page (`safeUrl`), so a user
can select this state. A static page with real content is quiet (16 lines / 30 s).

### Seen, NOT verified

On the media test page the card and daemon traded `emitGeometry contentSize=…` / `setZoomAndScroll
zoom=…` continuously (1662 → 2102 → 2022 → 2012 → 1972 → 2042 px). The page's `#state` is
`white-space:pre` and its event line grows, so the content width really does change every 500 ms —
this is probably the page and not a fit-width feedback loop. Do not record it as a defect without a
page whose content width is fixed.

### Kit work owed before any task (this is the next step, not optional)

- cavekit-device-build.md R4: the target is now CE 3.1.0; add criteria for fresh install, LS2
  registration on first start, coexistence with Atlas, and a re-verification sweep of the 3.0.5-only
  `[x]` boxes. R8: a log-growth bound and an idle-tick criterion (defect 2).
- cavekit-gre-widgets.md R1: `<video>` is broken past the first second (defect 1) — the "VP8 plays"
  and play/pause boxes are overstated. R7 still says "the engine has NO audio output at all", which
  has been false since 2026-08-15.
- cavekit-offscreen-rendering.md R8: a genuinely blank page must settle (defect 3).
- cavekit-browser-services.md R5: record the engine-NSS TLS position and the uncaptured `tls_version`.
- `context/impl/impl-media-controls.md`: the 2026-08-20 diagnosis is superseded (see above).

### Device and tree state as left

- All three variants installed from the 2026-09-27 `.ipk` payloads. **enyo runs a TRACED libxul**
  (md5 `cb514e9e11f34a6db94ceb1d44222cce`, prints `[jihad-media-trace]` on a failed frame copy); mochi
  and mojo run the `.ipk` build (`78c3b51a60d9959cbe313696e8ae26a7`). A push of all three puts them
  back in step.
- `/etc/event.d/jihad{,-mochi,-mojo}` all carry the device-local `JIHAD_INJECT=1`. **enyo's also carries
  `MOZ_LOG=sync,MediaDecoder:5,…`** from 2026-09-27 — it produced zero lines and should be removed
  (`sed -i 's/ MOZ_LOG=[^ ]*//' /etc/event.d/jihad`, then `stop jihad; start jihad`).
- The enyo card is parked on `/tmp/jihad-plain.html` (static). **Do not park on `about:blank`** — defect 3.
- mochi and mojo logs were truncated at 05:31 device time; `/var` is at 12% (54.7 MB free). At the
  idle rate that is days, not weeks — check `df /var` first thing.
- `third_party/uxp` is clean at `bd49435b`; the trace edits were reverted. The out-arm objdir libxul
  (2026-10-01 08:25) is the traced build and is stale against the tree until the next engine build.

---

## 2026-08-17 session delta — speaker route + live volume SOLVED, Mochi safeUrl done (read first)

The non-human-gated engineering surface is essentially exhausted this session. Kit-wide only **2**
criteria remain open (`grep -rn '\- \[ \]' context/kits`), BOTH keyboard/hardware-gated (there is no
keyboard on the device — open work item 1). PICKUP **open work item 3 is STALE**: R8's last criterion
is `[x]`, decided 2026-08-10 (geometry guards + the sub-rect argument bound the partial path; no blank
check needed).

- **Engine media now plays on the built-in SPEAKER at the system volume, and volume buttons track it
  live** (`render/goanna/BrowserPageGoanna.cpp`, commit `ed6cced6`; pairs with UXP-Jihad cubeb
  `bd49435b`). audiod refuses to route/scale OUR stream (its media volume is a PULSE control, proven by
  an `amixer contents` diff showing zero ALSA change on a button press), so the daemon drives the
  WM8994 mixer directly: a poll thread on `jihad_pulse_active_streams()` runs state-dir shell scripts
  it (re)writes from embedded constants — `audio-on.sh` (level `numid=74/86` from `getVolume`, then the
  DAC1→speaker switches `191/186/237/235/87`), `audio-off.sh` (mute amp `87` at rest → no max-volume
  burst), and `audio-vol.sh` every ~400ms (re-mirror `getVolume` → live tracking). `pulse.props` keeps
  the stream on `pcm_output` (NOT `media.role`/pmedia, which the forced route can't reach). Full detail
  + the dead ends: `context/impl/impl-audio-backend.md` §2026-08-17. Ear-confirmed: speaker ✓, no burst
  ✓, ~volume ✓. **RESIDUALS (human-gated, not shipped-blocking):** the volume OSD/HUD does not show for
  direct mixer writes (audiod-bypass, cosmetic); HTML5 video-controls taps are the separate offscreen
  hit-test item. **SHIP GATE:** parent gitlink + OE SRCREV stay at `81e10aa3` until UXP-Jihad
  `bd49435b` is pushed — the .ipk engine keeps the counter inert (audio unchanged) until then; the
  device runs the full hand-deployed stack now.
- **Mochi `safeUrl` allowlist done** (`app-mochi/source/JihadChromePrefs.js`): `http(s)://` or
  `about:(preferences|settings|jihad|isis|blank)` pass, everything else → `""`, applied in
  cleanLinks/loadHome/saveHome.

---

## 2026-08-16 session delta — bitbake `.ipk`s ship + the media scrubber is fixed (read first)

The two biggest open items are CLOSED, and this session's work IS committed (branch
`jihad/about-preferences-and-settings-merge`; engine mods on `third_party/uxp` branch
`jihad-engine-mods`).

- **OE bitbake produces all three `.ipk`s (device-build R3 DONE).** `bitbake
  net.riverstonerelay.jihad-browser{,-mochi,-mojo}` → `639 tasks ... all succeeded`, zero errors;
  each `.ipk` extracted + verified COMPLETE by content (daemon, libxul, plugin-container, ld-2.23,
  goanna.js, 1104-file chrome). Five recipe gaps were fixed to get here — see `../context/plans/build-site.md`
  T-154 and `../context/impl/dead-ends.md`. **Trap that ate two "successful" builds:** a goanna
  SRCREV bump does NOT re-fetch in this bitbake-1.18 setup (do_unpack keeps the old checkout); a
  chrome-only change needs `bitbake -c cleansstate goanna jihad-deviceroot
  net.riverstonerelay.jihad-browser{,-mochi,-mojo}` (after pre-warming the DL_DIR git mirror) + a
  CONTENT check of the packaged file, never mtime/size.
- **Media scrubber jitter/flash/no-settle FIXED (gre-widgets R1 back to `[x]`), device-confirmed.**
  Instrumentation proved the media/DOM state is perfect — the defect was the daemon's offscreen
  paint: damage-only repaint into three rotating buffers left GHOST thumbs (each buffer got only the
  latest frame's damage though it was several frames stale). Fixed with per-buffer damage
  accumulation (`BrowserPageGoanna` `BufFrame.dmg*`). Plus videocontrols: `ended` snaps the thumb to
  `duration`, and a 40ms interval interpolates the thumb from the live currentTime clock. 40ms is
  the ceiling — faster (25ms) overruns the adapter round-trip and FREEZES (checked against Atlas
  BrowserServer's identical double-buffer + wait-for-return model). Residual un-smoothness is the
  ~25–30fps software-paint cap, i.e. hardware.
- **Audio plays** through the cubeb PULSE backend (`JIHAD_PULSE_SINK=pmedia`), device-confirmed
  audible — the ALSA-bridged path was policy-muted; the pulse-native path gets 100%.
- **Engine provenance changed:** the jihad engine delta is now a durable commit `0c10df5c` on
  `third_party/uxp` branch `jihad-engine-mods` (pristine UXP `b2594a4` + the delta), pinned by the
  goanna recipe SRCREV — not only the desktop patch queue. Reproducibility caveat: that commit is
  now pushed to the fork `https://github.com/NotAlexNoyle/UXP-Jihad` (`.gitmodules` points there,
  branch `jihad-engine-mods`, parent gitlink bumped to `81e10aa3`), so a fresh clone + submodule
  update resolves the SRCREV.
- **Dotted-mochi `.ipk` was a 07-29 orphan (removed).** Current recipes emit only the HYPHENATED
  ids (`…-mochi` / `…-mojo`).

Device was used this session (TouchPad on novacom); it runs the pulse-audio libxul + per-buffer-fix
daemon + scrubber-fixed videocontrols. The 2026-08-15 and 2026-08-10 sections below are retained as
history — where they say "nothing is committed" or "the device is absent", read this delta instead.

---

## 2026-08-15 DEVICE session — the TouchPad reappeared near the end (historical delta)

Full transcript: `context/impl/impl-device-2026-08-15.md`. The device was plugged in mid-session and
the enyo variant was redeployed (ipk 1.0.4 + current daemon/adapter). Verified ON HARDWARE:

- **Cert store (T-135) — CLOSED end to end.** `/var/ssl/jihad/enyo` created (daemon is in group
  `luna`), real DER→PEM written, classified to ordinal 18 from `nsISSLStatus` flags, `CertInitCertMgr
  rc=0`, `Adding Entry … serial 6`, `CertAddTrustedCert` rc=0, NSS override + reload, page loads.
  `libPmCertificateMgr.so` present on 3.0.5. browser-services R5 device box → `[x]`.
- **Toast channel (T-146) — CLOSED.** `clearCookies` on the public bus pushed a notify and the Enyo
  card rendered the "Cookies cleared." toast, visible in a device screenshot. gre-widgets R5 last → `[x]`.
- **Focus transitions correct on device** (editorFocused 1→0→1); the desktop focus_test failure is
  harness-side, not the daemon.
- **AUDIO: the backend works and reaches pulse, but output is webOS-POLICY-gated — and the gate is now
  FULLY reverse-engineered.** `<audio>` gives `err=0` and pulse shows our sink-inputs at
  `s16le 2ch 44100Hz`, but sinks stay SUSPENDED — webOS powers the WM8994 codec route only while a
  policy-recognised media stream plays. Stock mechanism: `element.palm.audioClass="media"` (Palm
  WebKit extension, `media.js:288`) → `pmedia` sink + audiod media scenario. Port's path (specified,
  three parts, none human-gated, in `impl-audio-backend.md` §2026-08-15): (1) route engine ALSA at the
  `pmedia` sink via `ALSA_CONFIG_PATH`, (2) hold a `com.palm.audio/media` scenario ref-counted over the
  playback lifetime (daemon already has an outbound Luna client), (3) validate. the mechanism is now
  FULLY DECODED and the fix is DETERMINED (a bounded build task, not research). Read
  `module-palm-policy.so`: virtual sinks (pmedia/…) are NULL sinks; audiod bridges one to hardware +
  powers the codec ONLY for a stream CREATED on that virtual sink; default-device streams are muted
  ("PLEASE CATEGORIZE USING A VIRTUAL STREAM"). Engine decode+clock WORKS (t 0→3.00, ended, err=0) but
  is policy-muted on pcm_output at 0%. **The ALSA backend was a DEAD END (policy-muted); the cubeb PULSE
  backend was BUILT + DEPLOYED + DEVICE-VERIFIED 2026-08-15 and it UNMUTES engine audio.** The engine
  stream is now pulse-native (`application.name="Jihad Browser"`), `Volume: 100%`/uncorked (not the
  ALSA path's forced 0%), and `/proc/asound/…/status` shows the WM8994 DAC consuming its samples
  (`hw_ptr` advancing ~44100/s). Implemented: mozconfig `--enable-pulseaudio`, pulse-0.9.22 headers
  staged in the sysroot (device-ABI-matching; re-fetch via `curl
  freedesktop.org/software/pulseaudio/releases/pulseaudio-0.9.22.tar.gz`, headers `src/pulse/*.h` +
  a `libpulse.pc`), patch `0030-cubeb-pulse-webos-virtual-sink.patch` (`JIHAD_PULSE_SINK`; cubeb
  dlopens libpulse, no link dep — all 62 symbols present in the device lib). The pulse-native app
  IDENTITY is what unmutes it. **Residual (the only thing left, needs a human EAR — both automated
  oracles broken here: `parec` monitor = 0 bytes, `hw_ptr` runs during pause): whether the speaker
  physically emits, plus a possible daemon-side hold of the `com.palm.audio/media` scenario to power
  the speaker amp (the unmute does not need it, measured).** Full spec: `impl-audio-backend.md`
  §"RESULT 2026-08-15". **Bundle-Jessie-libasound decision is WRONG:** Jessie 1.0.28 → `err=3`,
  device's own `/usr/lib/libasound.so.2` → `err=0`. Stop shipping the bundled one. MP3/AAC/H.264
  decoders remain a separate, orthogonal gap.

Device traps found: two stray `jihad.bak.*` upstart jobs were contending for the enyo socket
(removed); cryptofs forbids symlinks; a `file://` XPI navigation does NOT trigger the install flow
(so T-103's device screenshot is still open — needs an http-served or card-initiated XPI). Device
left: enyo on current binaries, job env back to shipped flags, test certs swept, adapter backup at
`/var/palm/jihad/adapter-backup-20260815/`.

## 2026-08-15 session delta — read before the 2026-08-10 body

A cavekit loop (`context/plans/build-site.md`, now reconciled row-by-row; per-wave record in
`context/impl/loop-log.md` Iteration 3) closed or advanced every host-doable open criterion.
No device all session, so nothing below is device-verified unless it says so.

**Closed outright:** ipc-contract R1 (generator vendored+patched, regeneration reproduces the
shipped wire, zero hand edits — `render/browserserver/CodeGen/`); gre-widgets R5 first criterion
(notificationbox attach/dismiss PASS on the offscreen PuppetWidget path — `prefsui_test`);
gre-widgets R7 prefs-single-source (eleven rows observed from the shared file on desktop);
preferences-ui flush box (was already device-proven 2026-08-10, box reconciled); opal machine
config (kernel string was WRONG — board is `shortloin`; pinned from HP's own source drop);
T-115 prebuilt-input manifests (187-deb sysroot + adapter-deps, generators in `build/webos-oe/`).

**Built, awaiting a device:**
- **Engine audio output** — cubeb ALSA backend is IN the ARM libxul (`--enable-alsa`, Jessie
  alsa-lib staged; `DT_NEEDED libasound.so.2`). First listen: `jihad-media.html` with an
  Ogg/Vorbis or WAV asset (MP3/AAC/H.264 decoders are STILL OFF — ffvpx config, separate gap).
  Fallbacks if silent, in order, in `context/impl/impl-audio-backend.md` (PCM `default` may route
  to a nonexistent pulse plugin; `ALSA_CONFIG_PATH` override; drop-bundled-lib).
- **Cert store (T-135)** — all three 2026-08-10 defects fixed and desktop-proven live (real PEM
  certFile, Palm-ordinal error mapping off `nsISSLStatus` — the raw nsresult was a transport-class
  lie —, three-way trust with session-serial sweep at teardown AND startup). Platform writes via
  dlopen'd `libPmCertificateMgr`; first device line: `ls /usr/lib/libPmCertificateMgr.so*`.
  Read-direction (store→NSS) is COSTED not built (`context/impl/impl-cert-store.md`).
- **Touch events (T-120)** — fence out behind **`JIHAD_TOUCH_EVENTS=1` (default OFF)**; 28-byte
  header re-staged (sizeof guard proven both ways); double-activation suppressor daemon-side;
  device procedure in `context/impl/impl-touch-events.md`. Two real bugs fixed en route (TouchEnd
  OOB index; `npPalmEnableTouchEvents` was never set — un-fencing alone was a silent no-op).
- **Non-blocking toast channel (T-146/T-148)** — `jihad::PostNotification` → `notifications` Luna
  subscription → plain-DOM toasts in all three shells. The only informational dialog (T-103
  incompatibility alert) moved onto it; cookies/cache-cleared and add-on-installed notices ADDED
  (they emitted nothing before). Three device unknowns; one Preferences→Clear-cookies run splits
  them (`context/impl/impl-toast-channel.md`).
- **XPI mismatch alert (T-103)** — observer fires on desktop with a discriminating control; device
  screenshot is the closer.

**NEW TRAPS, same class as the body's:**
- **`out-ipk/` ipks ≤1.0.3 have NO libasound.so.2 and their daemon will NOT START** against the
  new libxul. Install 1.0.4+ only; the enyo 1.0.4 / mochi / mojo ipks built 2026-08-15 carry it
  (verified in all three).
- **`build-goanna-arm.sh` exited 1 on every SUCCESSFUL build until 2026-08-15** (grep|while under
  pipefail; 27/29 patches have no JS). Fixed; historical automation that trusted its exit code
  never saw its own staleness guards run.
- **`mFlashGestureLock` is set ONLY by a DOUBLE-tap** (`handlePenDoubleClick`, which also
  smart-zooms) — the "tap inside a plugin rect" wording in older notes is wrong, and NO latch log
  compiles (`Debug.h` DEBUG off). T-124/T-144 rescoped; keyboard-arbitration procedure rebuilt
  around the address-bar screenshot (addons kit R7, 2026-08-15 annotation).
- **`javascript:` URLs are dead in ALL chrome documents** (HTML too, not just XUL) — so
  `GoannaRenderPage::ScrollTo` is a no-op on chrome pages. And a **stale desktop profile** fakes
  pref regressions (see `context/impl/dead-ends.md`, both dated 2026-08-15).
- The dotted Mochi app id in the workspace CLAUDE.md is fixed (hyphens are correct).

**What remains, exhaustively:** every open criterion is device-, hardware-, or human-gated. With
a device + novacom (needs root for novacomd): the verification batch above, T-108 (hidd routing),
T-116, T-119/T-133, T-131, T-141, T-142→T-147. Human at device: T-150/T-151/T-152 and the frame
pacing EYEBALL decision (device deliberately left in Flash-alone — body below). Human decisions:
T-155 sign-off, T-156 Mojo settings exemption, whether to commit this tree (137+ files were
already uncommitted before 2026-08-15; the session added more and committed nothing).

---

## What this is

Jihad Browser: a fork of isis-browser (webOS 3 / HP TouchPad) with **UXP/Goanna** replacing
QtWebKit, keeping the isis UI and the BrowserAdapter↔BrowserServer YAP IPC contract. Three
variants ship independently: Enyo (`app/`), Mochi (`app-mochi/`), Mojo (`app-mojo/`).

Requirements live in `context/kits/` — start at `cavekit-overview.md`. The Flash/NPAPI work is
`cavekit-addons-extensions.md` **R7**.

---

## Flash: where it actually stands

Adobe Flash renders animated content on the TouchPad through UXP/Goanna, takes mouse and
keyboard input, **plays sound**, survives crashes and freeze/thaw, and holds ~30 fps composite
with patch `0027`. Two things are UNFINISHED: chrome-side keyboard arbitration (untestable on
this hardware) and frame PACING — the composite RATE is met, and the pacing ROOT CAUSE is now
found and measured: **two producers.** Flash invalidates itself and we also drive it, and our
frames land between its own. Isolated three ways; Flash alone is slower but has **zero** gaps
under 16 ms. The decision left is a design one, not a debugging one — see open work item 2.
**Audio is CLOSED**, at parity with stock; an
earlier version of this sentence listed "MP3 static (fix landed, unheard)" and that contradicted
the dated audio section below, which cost a session start. Precisely:

| | state |
|---|---|
| renders animated content | ✅ device-verified |
| **frame rate** | ✅ **composite 30.6-32.5 fps** via patch `0027` (CPU boost); gap avg 28-29 / max 45-49 ms |
| **mouse input** | ✅ `palm event 0x1/0x2/0x10000` at correct coordinates |
| **keyboard input → plugin** | ✅ Flash's own AVM1 runs on a keypress (green 11768 → red 11750) |
| **keyboard arbitration (chrome)** | ⚠️ **UNTESTABLE on this hardware** — no keyboard exists; synthetic one is read by hidd but never dispatched |
| crash containment | ✅ daemon survives a plugin SIGSEGV; plugin respawns on reload |
| spotlight event | ✅ suppressed by default (delivering it kills Flash — matches stock) |
| **audio** | ✅ **CLOSED 2026-08-10 at parity with stock.** All three defects were in the TEST ASSETS. The residual per-loop click is Flash restarting its MP3 decoder, and the STOCK browser does it identically |
| **frame PACING** | ⚠️ **ROOT CAUSE FOUND 2026-08-10: TWO PRODUCERS.** Flash invalidates itself ~25/s AND we drive it ~26/s, and our frames land between its own — that interleaving is the bunching. Isolated and measured all three ways: Flash alone is **slower but perfectly unbunched (zero gaps under 16 ms)**, the pull is faster and bunched. The device is LEFT in Flash-alone for a human to eyeball |

### Audio: solved, and the bug was in the TEST ASSET

**Flash audio needed no port-side change at all.** Every "silent" measurement in this tree was
taken against SWFs that could not have made a sound in any player.

`make-audio-swf.py` emitted `DefineShape` **and** `DefineSound` with the **same character ID 1**.
A SWF has ONE character dictionary shared by shapes and sounds, so the sound redefined an ID that
was already taken; the player keeps the first definition and drops the sound, `StartSound 1` then
resolves to a shape, and nothing plays. Silently: no error, no audio device ever opened, no mixer
thread. All three assets (PCM, MP3 mono, MP3 stereo) shared the bug, which is exactly why they
"behaved identically" and made this look like a player-side gate.

Fixed by giving the sound its own ID (`SOUND_ID = 2`). With the STOCK, UNPATCHED
`libflashplayer.so` the corrected SWF immediately produces the whole ALSA bring-up that was
missing for the entire investigation:

```
open("/usr/share/alsa/alsa.conf")                       = 27
access("/etc/asound.conf", R_OK)                        = 0
open("/usr/lib/alsa-lib/libasound_module_pcm_pulse.so") = 27
open("/usr/lib/libsndfile.so.1")                        = 27
```
plugin-container goes **3 → 5 threads**, and the tone is audible from the device speaker.

All three assets have been regenerated and re-pushed. Regenerate before trusting any audio
measurement anyway — `/tmp` is volatile, and any SWF still sitting on a device from before
2026-08-10 is a broken one. The `--mp3` variants need a BARE frame stream with no ID3; make one
with `ffmpeg -f lavfi -i "aevalsrc=..." -codec:a libmp3lame -write_xing 0 -id3v2_version 0`.
PCM is confirmed clean by ear. **MP3 was heard too and IS clean** — the per-iteration click is
Flash restarting its decoder at the loop point, and the stock browser does it identically, so it
is not a port defect. Judge MP3 quality ONLY with `jihad-audio-long.swf` (30 s, `--loops 1`),
which has no loop point at all; every other MP3 asset here clicks once per iteration no matter how
healthy the port is. There is no open audio item.

### The "audio gate" in the previous handoff was the VIDEO DECODER — do not chase it

The function at **0x70ee0** and the loader at **0x73020** have nothing to do with audio. Measured,
not guessed: 0x73020's string table is `libmm-omxcore.so` / `libOmxCore.so` resolving `OMX_Init`,
`OMX_GetHandle`, `OMX_GetComponentsOfRole`; 0x70ee0 passes `"video_decoder.avc"` (rodata 0x760cec)
and `strncmp`s candidates against `"OMX.Nvidia"` (0x760d00) with 128-byte entries
(`OMX_MAX_STRINGNAME_SIZE`). It is hardware H.264 decoder setup. `+24` is a codec enum, and it
**does** equal 7 — the factory at 0x5bff0 returns NULL unless the caller asks for type 7 and the
constructor at 0x72a30 stores that same 7 (`72a94: str r4,[r8,#24]`). The old note had the gate
inverted as well as in the wrong subsystem.

**Where the real ALSA code lives**, if it is ever needed again (all GOT-relative, base 0x85897c):
- **0x2f7bd0** — the ALSA loader. `dlopen("libasound.so")`, falls back to `"libasound.so.2"`,
  resolves ~50 `snd_*`, then NULL-checks every one; any NULL and it returns 0.
- **0x2f71e0** — `OpenPCM`, vtable slot +8 at vptr 0x84e440, ctor 0x2f75f0. Opens the PCM, sets
  access/format/rate/channels, 500 ms buffer / 20 ms period, then `pthread_create`s the mixer.
  It asks for **`plughw:0,0`** (0x7b54c8) as its FALLBACK name — the old note's `hw:0,0` was that
  string read four bytes in.
- `0x2fbf08` is a separate availability **probe** that loads libasound unconditionally at startup.
  That is the `dlopen` every previous trace saw; it proves nothing about playback.

**Already eliminated — do NOT redo these:**
- Our runtime can play sound. `render/goanna/test/alsa_probe.c` writes a square wave and it is
  AUDIBLE, under both the device's own loader and `./ld-2.23.so --library-path $HL`.
- Not PIpc (zero `PIpcClient` ctors even with an audio SWF), not LS2 permissions (role grants
  `outbound:["*"]`), not Flash's OEM profile (`/media/cryptofs/apps/etc/adobe/{mms,oem}.cfg`,
  `cctrl = 3`), not click-to-play deferred start, and not the host — `libWebKitLuna` has no
  PluginView audio methods and no ALSA/pulse references at all.
- Not the two gates that looked like candidates. Both were nopped on-device and changed NOTHING:
  the sample-count check `2f7234: cmp r1,#31 / ble` in OpenPCM, and the `audioEnabled` check
  `19ba54: beq` on `sound+0x270`. That pair of null results is what finally pointed at the asset.
- One real bug WAS found and fixed here: Flash's `kind not registered:
  'com.palm.app.flashplayer.prefs:1' (-3970)` on every instantiation. The db8 kind is now
  registered (owner `com.palm.flashgraphics`) and the error is gone. It was unrelated.

---

### Frame rate: the cause is CPU FREQUENCY, not the pipeline and not Flash

**Measured 2026-08-10.** Sampling `/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq` once a
second while `jihad-anim.swf` played:

```
192000 192000 192000 192000 192000 192000 192000 540000 384000 384000 384000 384000 384000
idle, right after navigating AWAY: 1188000
```

Flash renders at **192-384 MHz out of a 1188 MHz maximum**. Frame rate tracks it exactly — daemon
paints 20.8-23.9 fps, composite 16.3-29.1 fps — and the SPREAD is what reads as "skippy". The
system was only 69% busy across 2 cores at the time, so this was never a capacity wall; the
pipeline is clean (`deferred=0`, `wanted == done`), and on one run at a higher clock the same
pipeline reached **39.2 fps**, well above the 30 the content is authored at.

The governor is Palm's **`ondemandtcl`** — touch-biased. It boosts while the user is touching the
screen and decays otherwise, so passive playback (watching an animation, listening to audio) runs
at the floor. This also explains the audio xruns: MP3 decode at 192 MHz underruns.

**DO NOT "just set the governor to performance" — it WEDGES THE DEVICE.** Writing
`scaling_governor` while `ondemandtcl` is live deadlocks the cpufreq policy lock in the kernel.
Every later reader of any cpufreq sysfs node blocks in **uninterruptible D state** — including the
Jihad daemon, `powerlog`, and the restore script itself, so the governor never gets put back and
the daemon stops painting. D state cannot be killed; only a reboot clears it. Nothing persists
(it is a runtime sysfs value), so a reboot returns to `ondemandtcl`, but the whole session's
measurements are lost. This cost a device reboot on 2026-08-10.

**THE SAFE LEVER, AND IT WORKS — measured 2026-08-10.** `ondemandtcl` exposes ordinary governor
tunables at `/sys/devices/system/cpu/cpufreq/ondemandtcl/`. Writing those does NOT switch
governors and does NOT take the path that deadlocks; the write returns instantly and a readback
succeeds. Stock values and the two that matter:

```
up_threshold  = 95      <-- only ramps when a core exceeds 95% load
sampling_rate = 200000  <-- and only looks 5 times a second
```

95% is why Flash never boosts: the daemon runs ~26% and plugin-container ~11% of two cores, so no
single core ever approaches the threshold. Setting `up_threshold=40` and `sampling_rate=50000`
pins the clock at 1188000 and produces, on `jihad-anim.swf` (30 fps authored), single instance:

| | stock | tuned |
|---|---|---|
| daemon paints | 24.0-28.7 fps | **31.4-33.0 fps** |
| card composite | 20.4-27.5 fps | **30.7-35.2 fps** |
| frame gap avg / max | 34-42 / 63-81 ms | **30-31 / 49-56 ms** |

Composite now meets the authored rate AND is matched to the draw rate, which is what the
criterion asks for. `deferred` stayed 0 throughout, both before and after — the pipeline was
never the constraint.

**What is NOT yet done: making this OURS rather than a manual poke.** These are SYSTEM-WIDE
tunables, so the daemon must apply them only while a plugin instance is alive and restore the
stock values on teardown (and on crash — an upstart `post-stop` is the honest place for the
restore, since a daemon that dies with `up_threshold=40` leaves the whole device tuned for
battery-burn). Read the stock values rather than hard-coding 95/200000. Do NOT touch
`/sys/devices/system/cpu/cpufreq/override/{turbo_mode,vdd_*}` — that is overclocking, and
`vdd_freqs` goes to 1836000.

---

## Frame pacing — what was wrong, what is fixed, what is left (2026-08-10)

The user's report was "the pink square is smoother in the STOCK browser". It is a real defect and
the average frame rate never showed it: the daemon was delivering 35-42 fps for a 30 fps SWF.

**The measurement that found it is a gap HISTOGRAM, not min/avg/max.** Summary statistics cannot
distinguish "evenly late" from "unevenly on time", and every earlier session's numbers were
summary statistics. Added to the 2 s counter block; the failure looked like this:

```
gap hist ms <16=19 16-23=20 24-31=10 32-39=24 40-55=6 56+=0     <-- FLAT = no pacing at all
```

Frames were landing at effectively random instants between 8 and 55 ms, so the compositor held
each one for anywhere between half a refresh and three. Three separate causes, all measured:

1. **`PumpFor` burned the tick.** It `g_usleep(1000)`s every iteration, and a 1 ms sleep really
   costs ~3 ms on this device, so `PumpFor(4)` measured **12 ms** of a 16 ms tick with the page
   IDLE. The tick was therefore WORK-bound at 26-28 ms rather than timer-bound at 16, and since
   a frame can only be published on a tick, every gap quantised to that. Fixed by `PumpReady()`,
   which drains what is ready and returns without sleeping — **work avg 12 ms → 1 ms idle**.
   `PumpFor` KEEPS its old behaviour because the tests use it as "let the engine run for N ms".
2. **Nothing paced the publish.** The damage gate was an 8 ms floor deliberately set BELOW the
   tick so it never gated. A fixed 33 ms deadline grid (not a floor — a floor drifts and beats,
   which is what the 2026-08 note in the source records) moved 30% → 69% of frames into the
   32-39 ms bucket.
3. **The daemon could not tell a NEW plugin frame from a repeat**, because its only signal is
   PuppetWidget's sticky dirty boolean, which the refresh driver and ordinary page damage also
   set. That ambiguity makes BOTH available policies wrong: publishing on every dirty tick sent
   ~78 card frames for ~60 plugin frames (duplicates, and `deferred` 22-44 because the adapter
   could not drain them), while the 33 ms grid sent ~57 for ~68 and DROPPED frames, so the
   animation stepped two positions at once. Fixed by exporting `jihad_plugin_frame_seq()` from
   `PluginInstanceParent::RecvShow` (weak-linked exactly like `jihad_offscreen_take_dirty`) and
   publishing one card frame per plugin frame: **done=91 against palm draw=92, `deferred=0`.**

**Why stock is structurally smoother, and it is not a tuning difference.** Stock has exactly ONE
clock: `PluginViewQt`/`PluginViewPalm` pulls the plugin synchronously inside the page paint, and
the producer blocks until the consumer drains, so publish times ARE drain times. We have two
free-running clocks — the plugin child's own repaint timer (a delayed task RE-POSTED after its
work, so its period drifts) and the daemon's tick — and we SAMPLE the plugin rather than pull it.
That is the remaining gap, and it is the next thing to attack.

**Two things were tried and are NOT settled — the runs that judged them were confounded:**
- **An 8 ms tick** (`JIHAD_TICK_MS=8`). Works mechanically: tick period 9-11 ms, 1:1 delivery,
  `deferred=0`. But the plugin then redrew ~45 times/s for 30 fps content, so content frames are
  shown 1 or 2 times in a beat — even DELIVERY of uneven CONTENT, the same artifact one layer up.
  Worth revisiting only together with a draw rate that matches the content.
- **`JIHAD_PLUGIN_REPAINT_DIV=3`** on top of it, to bring draws back to ~30. Draw rate did land
  at 28-34 fps, but the daemon then logged `flashRect remove id=N (gone)` and stopped painting
  while the child kept drawing.
  **That was NOT caused by the divisor, and the first write-up of it here was wrong.** The same
  stall persisted after reverting to `JIHAD_TICK_MS=16` and the default divisor, and a CLEAN
  restart (`stop jihad; pkill -f jihad-browserserver; pkill -f plugin-container; start jihad;
  killall LunaSysMgr`) cleared it completely on the identical build: `done=61 palm draw=63
  deferred=0`. What actually happened is a plugin CRASH LOOP — the log carries a
  plugin-container maps dump — reached by cycling `inject url` between pages repeatedly. Once the
  child is in that state the plugin rect goes away, widget invalidation stops with it, and every
  later number reads as a pacing regression that is not one.
  **NEW TRAP, same family as the two below: repeated inject-driven navigation can wedge the
  plugin, and the symptom is silence, not an error.** `flashRect remove id=N (gone)` with the
  child still logging `palm draw` is the signature. Do a clean restart before believing any
  frame measurement that follows one.

Everything here is env-tunable for A/B without a rebuild: `JIHAD_TICK_MS` (4-64, default 8 in
code / 16 on the device job), `JIHAD_PLUGIN_FRAME_MS` (0 disables the pacer and the seq gate with
it), `JIHAD_PLUGIN_REPAINT_DIV` (1-8, child side), and — added 2026-08-10 because the frame
question could not be A/B'd without them — the two DRIVER switches:

| var | side | effect |
|---|---|---|
| `JIHAD_PLUGIN_PULL=0` | daemon | disables the host request ONLY, leaving the publish policy alone. Use this, NOT `JIHAD_PLUGIN_FRAME_MS=0`, which also drops the publish gate to the 8 ms floor and so changes two things at once |
| `JIHAD_PLUGIN_SELF_DRIVE=0` | child | disables the child's own repaint timer outright |

**Setting only `JIHAD_PLUGIN_PULL=0` does NOT isolate the plugin** — the child's host-drive lease
expires and its timer takes over, so you measure patch `0026`, not Flash. Both switches together
leave the plugin's own `NPN_InvalidateRect` as the sole frame source. Each logs a line when it
engages (`plugin PULL disabled` / `self-drive timer DISABLED`); grep for it before believing a run.

**And when reading `show req:`, `hostinval` counts everything that reached
`JihadPalmRepaintTick` — the pull AND the child timer. It is not "the pull" on its own.**

### The PULL is built and measured (patch `0029`, 2026-08-10) — and it did NOT fix the pacing

`0029` does exactly what the item above asked for: an async parent→child `JihadRequestFrame`, the
child answering it with its existing `JihadPalmRepaintTick()`, its own timer standing down on a
150 ms LEASE (not a latch — the daemon can stop asking at any moment, and a latched plugin would
then never paint again), and the daemon asking once per whole-tick grid slot from its own tick.
It is live on all three variants. **The mechanism works and the outcome is not a win:**

|  | `0028` (sample) | `0029` (pull) |
|---|---|---|
| daemon done | 29.4-31.9 fps | 32.8-34.0 fps |
| frame gap avg | 31-34 ms | 29-30 ms |
| frame gap max | 47-62 ms | 45-55 ms |
| gap hist `<16` ms | 1-4 of ~60 | **5-8 of ~68** |
| `deferred` | 0 | 0 |

Average and max improve; the HISTOGRAM gets flatter, which is the metric that matters. Both runs
were clean restarts of `jihad-anim.swf` at `JIHAD_TICK_MS=16`, log-growth and card-connection
asserted. Why it does not work is open work item 2: **the pull is not exclusive** — 3988 draws
against 3072 requests — so a third driver is producing most of the frames.

Two things worth keeping from this even if `0029` is later reverted:

- **THE TICK IS ITSELF A JITTERY CLOCK, and that bounds every pacing scheme.** Against a 16 ms
  timer the measured period is `min=15 avg=18 max=41`, with `work avg=10-11` (pump 7, paint 3).
  Locking the plugin to that clock inherits its jitter, so "one clock" is necessary and not
  sufficient. **`JIHAD_TICK_MS=8` is WORSE, measured, and not for the reason recorded earlier:**
  the tick cannot actually run at 8 ms (work is 11-13 ms, so the observed period stays 14-16 ms),
  the draw rate rises to 36-40 fps for 30 fps content, and the histogram spreads across the 16-23
  and 24-31 buckets. Leave the device job at 16.
- **There is no clean runtime A/B for the pull alone.** `JIHAD_PLUGIN_FRAME_MS=0` does stop the
  requests (the child's lease expires and it resumes self-driving, i.e. `0026`/`0028` behaviour),
  but the same variable also drops the publish policy to the 8 ms floor, so the two effects cannot
  be separated without a rebuild. Do not read a `=0` run as "the pull, off".

## Open work, in priority order

1. **Keyboard arbitration, card side — blocked on the hardware, not on the code.** Keys reach and
   drive Flash; what is NOT verified is that the CARD does not also act on them. The gate is
   deployed (`handleKeyDown/Up` on `mPageFocused && (bEditorFocused || mFlashGestureLock)`) and
   its return value IS the arbitration, so the expression is not in doubt — what needs proving is
   that `mFlashGestureLock` is true during play.

   **A human at the device cannot test this either.** There is no keyboard: `/proc/bus/input/
   devices` has only `gpio-keys`, `pmic8058_pwrkey`, `headset`, and webOS's on-screen keyboard is
   drawn by LunaSysMgr in-process, not through `/dev/input`. The daemon's `key` inject command
   goes straight to the page and bypasses the adapter entirely, which is why it proved "Flash gets
   keys" and can never prove anything about the chrome.

   `render/goanna/test/uinput_kbd.c` synthesises a real one (PDK build → device glibc, no bundled
   loader; `build` it with the PDK gcc plus `--sysroot`). `/dev/input/uinput` exists and works,
   the device registers as `event3`, and **both `hidd` and LunaSysMgr open it** — yet no key ever
   reaches the daemon and `window.pageYOffset` never moves. So hidd reads it and does not dispatch
   it. Next: `/etc/hidd/HidPlugins.xml` routes generic devices through `HidInputDev`
   (`/usr/lib/libhidinputdev.so`, `/var/run/hidd/InputDevEventSocket`) — find out what it requires
   of a device before it forwards. Also fix the observable: arrows were used on the assumption the
   chrome scrolls, which a touch OS may not do at all; the adapter's own comment says the leak
   appeared as keystrokes landing in the ADDRESS BAR, so screenshot the URL field instead.
   `jihad-keyarb.html` (tall page + key SWF + `gettext #sy` scroll readout) is staged for this.
2. **Frame pacing: the pull is BUILT (patch `0029`) and it is NOT the fix. Find the third draw
   driver.** The pull described in the previous handoff now exists end to end and is device-live —
   see the section above for the numbers. It did not tighten the gap histogram, and the reason is
   a measurement nobody had taken before: **the plugin's draw rate is not set by whoever you think
   drives it.** Over 123 s the child logged **3988 draws against 3072 host requests**, and at an
   8 ms tick the gap widened to 3988-vs-~1900 territory (38 fps of draws for ~15 requests/s). So
   the majority of draws — and ALL of the excess — come from neither the child's own timer (it is
   provably stood down: exactly one `frame clock -> HOST` transition and no fall back for the
   whole run) nor from Flash self-invalidating (`gJihadNPNInvalidateRectCalls` stays 0, which is
   precisely why the host-driven path keeps running).
   **ANSWERED 2026-08-10, and the suspect this paragraph used to name was WRONG.** The counters
   were added (every `AsyncShowPluginFrame` call site, tagged, reported next to `palm draw` as
   `show req:`) and `RecvUpdateBackground` measured **exactly zero** — as did every other
   background path. The hypothesis that it was parent-driven and correlated with daemon paints is
   refuted; do not re-run it.
   **It was OUR OWN LEASE FLAPPING.** Every draw traces to `InvalidateRect`, and the accounting
   closes to the unit: **1856 host requests + 1846 child-timer invalidates = 3702 total**, with
   the child logging `HOST -> SELF -> HOST`. The daemon does not re-ask while a request is
   outstanding — it waits out `kJihadPluginReqTimeoutMs` (250 ms) — but the child's lease was
   150 ms, i.e. SHORTER, so one late frame dropped the lease and restarted the second clock. The
   invariant is now written into the source: **lease > request timeout + one grid period +
   margin** (lease 500 ms). Verified: the clock log now shows one `-> HOST` and no fallback.
   **AND THEN THE REAL ONE — PATCH `0026`'s FOUNDING PREMISE IS FALSE.** With the lease fixed,
   1344 host requests still did not account for 2662 invalidates. Splitting the counter at the
   funnel by whether `JihadPalmRepaintTick` is on the stack settles it:
   **`hostinval` ≈ 27/s against `plugininval` ≈ 22/s.** Flash invalidates itself constantly.
   The reason three sessions believed otherwise is that `gJihadNPNInvalidateRectCalls` — the
   counter `0026` reads to decide "the plugin never asks to be repainted" — is **structurally
   blind**: it lives in `JihadNPNInterpose.cpp` and only sees calls that bind the
   `NPN_InvalidateRect` SYMBOL, while an ordinary NPAPI plugin calls through the NPNetscapeFuncs
   table, which `PluginModuleChild` fills with its own `_invalidaterect` (`:986`) that forwards
   straight to `InstCast(aNPP)->InvalidateRect` (`:1302-1310`) and never touches the interpose.
   It can read 0 forever while the plugin invalidates 22 times a second.
   **So the pacing defect is TWO PRODUCERS BEATING**, ours and Flash's, funnelled into one
   coalescing path (~49 invalidates/s collapsing to ~32 draws/s). That is exactly the shape that
   produces an unconcentrated gap histogram at a healthy average.
   **NEXT EXPERIMENT, and it is cheap:** measure Flash with BOTH drivers off. `JIHAD_PLUGIN_FRAME_MS=0`
   alone is not enough — it stops the pull, the lease expires and the child's own timer takes
   over, which is just `0026` again. It needs one more child-side kill switch to disable the
   timer outright. If Flash alone holds the authored rate, the host clock should be deleted for
   plugins that self-invalidate rather than tuned.
   Measure with the gap histogram, NOT with average fps: the average was 35-42 fps throughout the
   whole period the animation looked worst, and `0029` IMPROVED the average and the max while
   making the histogram flatter.
3. **~~The partial-paint path publishes with no blank check~~ — STALE, resolved 2026-08-10.** R8's
   last criterion is `[x]`: the second pass decided NO blank check is needed. The geometry guards
   (segment-size check) catch the corruption class, and a partial paint only ever rewrites a sub-rect
   of a frame that already passed the full path's non-blank check, so it cannot publish a blank frame
   invisibly. Do not re-open. (Full reasoning: cavekit-offscreen-rendering.md R8, lines ~200-218.)
4. **`plugin-container` aborts on teardown.** Every navigation away from a Flash page ends in
   `mozalloc_abort` (pc 0xc35c in plugin-container's own text) — a deliberate Gecko abort, not a
   segfault. Harmless today (the child is going away) but it is noise in every log.
5. **The in-daemon plugin scan is a landmine.** `nsPluginFile::GetPluginInfo` dlopens EVERY
   plugin in the daemon ("Sadly we have to load the library for this to work"), pulling
   libWebKitLuna/libPiranha/libv8/libEGL into it, and `dlclose` does not unload them. A hostile
   plugin can wedge the daemon before any plugin logging runs.

---

## Traps — every one cost real time and none is discoverable from the source

**Device / measurement**
- **AVERAGE FRAME RATE CANNOT SEE THE DEFECT THE USER IS REPORTING.** Throughout the entire
  period the animation looked worst by eye, the daemon reported 35-42 fps for a 30 fps source —
  ABOVE the content rate. min/avg/max cannot separate "evenly late" from "unevenly on time"
  either. Read the `gap hist ms` bucket line: flat across the buckets means no pacing, weight in
  one bucket means smooth. Every earlier session's frame-rate conclusions were drawn from summary
  statistics and are worth re-reading with that in mind.
- **THE PLUGIN'S DRAW RATE IS NOT SET BY WHOEVER YOU THINK DRIVES IT — count, do not reason.**
  Three sessions have now attributed the draw rate to a specific clock (the child's timer, then
  the daemon's tick, then the host pull) and every attribution was made from reading the code, not
  from a count. When `0029` finally put a counter on the request side the numbers did not agree
  with ANY of them: 3988 draws against 3072 requests, with the child's own timer provably stood
  down and Flash provably not self-invalidating. Whenever you change what drives a frame, log both
  ends and divide. The ratio is the finding; the fps number cannot express it.
  **And the follow-up is the sharper lesson: "provably stood down" was itself an inference, and it
  was wrong.** The child's timer was gated on a lease, one `frame clock -> HOST` line was taken as
  proof the gate held, and nobody checked for a LATER transition. There were three. Tagging every
  call site closed the accounting to the unit — 1856 host + 1846 timer = 3702 — and the answer was
  a bug in the gate, not a third party. A transition log only proves what it logged; grep for
  ALL of them, and make counters that must SUM to something you already measure.
- **A WEDGED PLUGIN LOOKS EXACTLY LIKE A PACING REGRESSION.** Cycling `inject url` between pages
  repeatedly can leave the child in a crash loop; the daemon then logs `flashRect remove id=N
  (gone)`, painting stops entirely, and the child keeps logging `palm draw` the whole time so the
  plugin looks alive. This produced a confident, WRONG attribution to a tick-rate change on
  2026-08-10 — the identical build measured healthy after a clean restart. Before believing any
  frame number that follows repeated navigation: `stop jihad; pkill -f jihad-browserserver;
  pkill -f plugin-container; start jihad; killall LunaSysMgr`.
- **AN EMPTY LOG PASSES EVERY CHECK.** `grep -c 'inject: no page' $LOG` returns 0 on a log that
  is empty because the daemon wrote NOTHING, which reads as "card fine" and is the opposite of
  the truth. This produced two clean-looking but entirely void runs on 2026-08-10. Assert the log
  GREW (`wc -c`) before believing any grep over it — a zero-byte log means the run never
  happened, not that it went well. Same class as the "card connected" trap below, one level down.
- **NEVER write `/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`.** Switching away from
  Palm's `ondemandtcl` deadlocks the cpufreq policy lock in the kernel; every later reader of any
  cpufreq node blocks in unkillable **D state** — the daemon, `powerlog`, and your own restore
  script, so the governor is never put back and the daemon stops painting. Only a reboot clears
  it, and **a clean `reboot` will NOT run**, because init waits on those D-state tasks: it takes
  `sync; sync; reboot -f`. The governor's own TUNABLES (`cpufreq/ondemandtcl/*`) are safe to
  write and return instantly — that is what patch `0027` uses. Cost a device reboot 2026-08-10.
- **Regenerating `/etc/event.d/<job>` from `packaging/gen-variant-scripts.sh` and pushing it
  verbatim KILLS THE INJECT CHANNEL.** The shipping template does not set `JIHAD_INJECT=1`; the
  device's copy carries it as a local addition. Lose it and every inject silently does nothing,
  the daemon logs nothing, and — see the first trap — every subsequent check "passes". Re-add
  `JIHAD_INJECT=1` to the `exec env` line whenever you reinstall the job, and remember the running
  daemon keeps its OLD environment until you `stop`/`start` it.
- **ASSERT THE CARD IS CONNECTED before believing any measurement.** If it disconnected, the
  daemon logs `inject: no page` and every downstream number — container present? draw count?
  thread count? — reads as a dramatic failure that is really "no page". This invalidated three
  of my own measurements and twice produced a wrong "audio kills the plugin" conclusion. Inject
  a known page and grep for `inject: no page` first.
- `novacom run` drops output arriving after host stdin hits EOF — pipe `sleep N |` into every
  run whose reply matters.
- `novacom run file://bin/sh -- -c '…'` silently drops the `-c` string. Stage a script with
  `novacom put` and run that.
- There is **no `timeout` binary** on the device. Background the process and `kill` it.
- `strace` DOES exist (`/usr/bin/strace`) and settled two problems in one run each.
- A screenshot captures the FOREGROUND card. The double-launch dance creates a second card, so a
  screenshot can silently be of the wrong one.
- The first card launch after `killall LunaSysMgr` does not navigate — launch twice, or use the
  inject channel (`JIHAD_INJECT=1`, `<state>/inject.cmd`), which is better.
- `/tmp` is cleared by a reboot; `/etc` comes back read-only (`mount -o remount,rw /`).

- **NEVER regenerate the YAP interface from `../ref-BrowserServer/CodeGen` — it would break every
  keystroke.** Established 2026-08-10 by building `YapCodeGen.cpp` against Qt 5.15 and RUNNING it,
  not by reading. The upstream `.defs` is stale against its **own** upstream output: it declares
  `KeyDown`/`KeyUp` (0x1008/0x1009) as two `uint16_t`, while both upstream's generated
  `BrowserServerBase.cpp` and ours read a third `int32 chr` — upstream hand-edited its generated
  file and never updated the `.defs` (the tell is the six-space indent on those lines inside a
  tab-indented generated file). Regenerating from it silently rewrites two LIVE commands from 12
  wire bytes to 4. Note this is a bigger break than the missing 0x1600, and the obvious fix —
  "just add our command to the upstream `.defs`" — would have shipped it.
  **Use the corrected in-tree copy: `render/browserserver/CodeGen/BrowserYapCommandMessages.defs`.**
  `build/webos-oe/check-yap-contract.sh` now runs inside `build-daemon-arm.sh` and fails the build
  on any drift (verified three ways: passes here, fails with 10 named defects on a real upstream
  regeneration, fails with exactly one on a regeneration from the corrected `.defs` — that one
  being the hand edit no `.defs` can express, since the generator emits every async command as
  `= 0;` and 0x1600 needs an empty default body to stay optional).
  **TRAP INSIDE THE TRAP:** a `.defs` line beginning with a SPACE hangs `YapCodeGen` forever —
  `ignoreLine()` scans for a non-space without advancing its pointer.

**Cross-ABI (bundled glibc 2.23 vs device glibc 2.8)** — see cavekit-device-build.md **R9**
- glibc 2.21 changed `sem_t`'s value word to `(tokens << 1) | has-waiters`. A named semaphore
  created by a 2.8 process reads to us as "0 tokens, waiters pending" and blocks FOREVER. This
  took Flash out entirely after one reboot and looked exactly like a code regression on
  untouched binaries — **it is a per-boot race, not a bug in your change.** Contained for
  `/PmLogLib` by `render/goanna/JihadPmLogSem.c` (patch 0025).
- `sem.browserserver.*` are created by the ADAPTER and nothing ever waits on them. The trap is
  ARMED, not sprung: adding a daemon-side `sem_wait` springs it, and `nwaiters` sits at a
  different offset too (8 in the 2.8 family, 12 in 2.23 — measured). Use SysV `semop` (a kernel
  object, no userspace layout) if a blocking handshake is ever needed. It does NOT fire on an
  EMPTY semaphore (raw 0 and `0 << 1 | 0` are the same word), so a first `sem_wait` would look
  fine right up until the adapter posts.
- **THIS WHOLE CLASS IS HOST-TESTABLE — stop spending device sessions on it.** Both toolchains are
  in the tree and `qemu-arm` runs their output on the build host: static-link one probe with the
  PDK gcc (sysroot glibc 2.5, the pre-2.21 side) and one with the crosstool gcc (2.23), run both,
  and the DIFFERENCE is the ABI. The `sem_t` hang above reproduces this way in about a minute.
  The PDK sysroot is 2.5 and the device is 2.8, so it is a pre-2.21 PROXY, not the device.
- **`pthread_mutex_t` PROCESS_SHARED is a SECOND armed trap, and the layouts being IDENTICAL is
  what makes it dangerous.** 24 bytes and the same fields in both, so every size and marker check
  passes — but glibc 2.23 sets `__kind = 128` (PTHREAD_MUTEX_PSHARED_BIT) and takes the futex
  private/shared flag from that bit, while the pre-2.21 side writes `__kind = 0` and returns
  EINVAL on a mutex whose `__kind` is 128. So one direction hangs under CONTENTION ONLY, and the
  other turns every `lock()` into a silent no-op. `ProcessMutex` is adapter-only and instantiated
  by nothing today; see cavekit-device-build.md R9 and the comment in `ProcessMutex.cpp`.
- `pthread_cond_t` still matches (48 bytes, same fields) — but **2.23 is the LAST glibc where that
  is true**; the condvar was rewritten in 2.25, and 2.24/2.25 tarballs are already in the
  toolchain source dir. `pthread_rwlock_t` ALREADY differs at 2.23 (same size, `__flags` narrowed
  and a new `__shared` byte added), which is the standing proof that `sizeof` does not clear a
  struct.
- The framebuffer header IS clean: `BrowserOffscreenInfo` is 32 bytes with identical offsets under
  both toolchains (plain int/double, so AAPCS fixes it and libc has no say). Do not re-check it.
- Prove each device library with a standalone probe run under BOTH loaders — the difference IS
  the diagnosis. See `plugin_mime_probe.c` and `alsa_probe.c`.

**Reverse-engineering libflashplayer.so**
- The code is **ARM, not Thumb**.
- String addresses are PIC: `add rX, r5, <negative literal>` with a base near 0x8ad010. Searching
  for an absolute address AND for `movw`/`movt` immediates both find ZERO references. Read the
  literal pool and add the base.
- Take PLT stub addresses from `objdump -d --section=.plt`, which labels them. A hand-rolled
  decode gave a wrong stub and made a BL scan report no callers.
- A function's entry can be one instruction BEFORE the `push` (the PIC `ldr rX,[pc,…]` comes
  first). Being off by 4 made a caller scan report zero callers and nearly sent the audio hunt
  down a "it must be a vtable" dead end.

**Build / deploy**
- **archive.debian.org's CDN serves cached 404s for files the pool lists.** 32 of the 187 pinned
  `.deb`s failed on a clean clone though every one is there; the same URL with a query string, or a
  later try, works. `--check-urls` used to probe five and pass. The fetchers now try five times with
  a cache-buster (`9ff7d71a`); sha256 still decides.
- **A PC reboot kills every background job of a session, and its wormhole-rs sends with them.** A
  wormhole code is one-use and lives in the sender process: after a reboot, resend and hand over the
  new code. The old session's subagent transcripts survive in
  `~/.claude/projects/<project>/<session-id>/subagents/agent-*.jsonl` (one that stops short has no
  verdict: run it again). A detached build (`setsid nohup … &`) survives a session, not a reboot;
  `mach build` resumes from its object directory.
- `push-engine-update.sh` pushes libxul + daemon + goanna.js. It does **NOT** push
  `plugin-container` or the adapter — **nor the loose JS components.**
  `components/jihadInstallPrompt.js` and `components/jihadAboutPreferences.js` live in each
  variant's `deviceroot/hl/components/` and are refreshed only by a bundle/`.ipk` push, or by
  pushing the one file per variant (which is what was done 2026-08-10 for the R3 install-failure
  observer). Editing one and running `push-engine-update.sh` ships nothing.
- **The variant app ids on device use a HYPHEN, not a dot: `net.riverstonerelay.jihad-browser-mochi`
  and `…-mojo`.** `CLAUDE.md` writes the Mochi id with a dot and is WRONG against the device.
  Pushing to the dotted path fails with `bad or error response from other side: 'file open failed'`,
  and the `ls` that follows reads as "this variant does not ship that file" — I briefly concluded
  mochi and mojo had no `components/` directory at all before checking the parent directory.
  Confirm a path against `ls /media/cryptofs/apps/usr/palm/applications/` before believing an absence.
- The `.ipk` ships the **PDK** adapter (`build-adapter-pdk.sh` → `adapter-deps/build-pdk/`), NOT
  the crosstool one (`build-adapter-arm.sh` → `adapter-deps/build/`). Rebuilding only the latter
  ships an old adapter in the package while the device runs the new one.
- The adapter runs INSIDE LunaSysMgr. Deploy it by swapping
  `/usr/lib/jihad/<variant>/BrowserAdapterImpl.so` + `/usr/lib/BrowserPlugins/BrowserAdapterJihad*.so`
  (needs `mount -o remount,rw /`), then restart LunaSysMgr. Back up first —
  `/var/palm/jihad/adapter-backup/` holds the pre-2026-08-10 pair.
- A hand-launched daemon survives `stop jihad` and serves a stale libxul. "Nothing changed" is a
  lie until you `pkill -f jihad-browserserver`.
- `cp` onto a mapped binary fails with "Bad file descriptor" on cryptofs — stop the daemon and
  `rm -f` first, or push to `.new` and `mv`.

---

## Build / deploy loop

```bash
# libxul (incremental ~45 s) and the daemon
podman run --rm --userns=keep-id \
  -v "$PWD/third_party/uxp:/src/uxp" -v "$PWD:/jihad" \
  -v "$PWD/build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi:/tc" \
  -v "$PWD/build/webos-oe/arm-sysroot/root:/sysroot" \
  -v "$PWD/build/desktop:/cfg" -v "$PWD/build/webos-oe:/armcfg" \
  -v "$PWD/build/webos-oe/out-arm:/out" \
  localhost/jihad-goanna-build bash /armcfg/build-goanna-arm.sh build   # or build-daemon-arm.sh

build/webos-oe/push-engine-update.sh enyo mochi mojo   # md5-verified

bash build/webos-oe/build-adapter-arm.sh    # adapter for a live swap
bash build/webos-oe/build-adapter-pdk.sh    # adapter the .ipk ships  <-- do both
bash build/webos-oe/build-variant-ipk.sh    # all three .ipks

bash build/webos-oe/build-test-plugin-arm.sh   # NPAPI control plugin
bash build/webos-oe/build-uinput-kbd.sh        # synthetic keyboard (PDK; needs --sysroot)
```

**Only `enyo` has libxul with patch `0027`.** `push-engine-update.sh enyo mochi mojo` pushes the
SAME libxul to all three, so just run it with all three variants next time you push and they are
back in step. Until then, a frame-rate measurement taken on mochi or mojo will show the OLD,
governor-limited numbers and look like a regression that is not one.

Clean device run:

```sh
stop jihad; sleep 2; pkill -f jihad-browserserver; pkill -f plugin-container; sleep 2
: > /var/palm/jihad/enyo/daemon.log
start jihad; sleep 3
killall LunaSysMgr; sleep 32
luna-send -n 1 palm://com.palm.applicationManager/launch \
  '{"id":"net.riverstonerelay.jihad-browser","params":{"target":"file:///tmp/jihad-plain.html"}}'
# then drive it with the inject channel:
echo 'url file:///tmp/jihad-anim.html' > /var/palm/jihad/enyo/inject.cmd
```

Read the `paint pipeline:` / `frame gap ms min/avg/max` counter pair for frame-rate work — the
gap SPREAD is what reads as stutter, not the average.

### Inject commands, and the two added 2026-08-10

`url`, `text`, `find`, `rect`, `gettext`, `clickid`, `dblclickid`, `clickoff`, `touch`, `cookie`,
`addon`, `spotlight`, `freeze`, `thaw`, `popups`, `jsurl`, and now:

| command | what it is for |
|---|---|
| `setpref <b\|i\|s> <name> <value>` | write a pref, with an immediate read-back on the same log line |
| `getpref <b\|i\|s> <name>` | read one back — after a restart, which is the half that proves persistence |
| `anon <selector>` | the element's NATIVE ANONYMOUS CONTENT: did an XBL binding actually attach. `?` = no such element, `0` = element exists and bound NOTHING (the real negative), otherwise a count plus the first few node names and `anonid`s |

**Both were added because their absence had already cost device runs, and both replace a probe
route that DOES NOT WORK — do not go back to it.** `jsurl` executes with the system PRINCIPAL but
in a CONTENT scope, so `Components` is not exposed: a `Components.classes[…preferences-service…]`
probe returns success and silently does nothing, which reads as a negative result about the thing
under test rather than a broken probe. Likewise `document.getAnonymousNodes` throws
`not a function` there regardless of privilege. Both questions have to be answered in the daemon,
which is in-process with libxul and already holds `nsIPrefBranch` and `nsIDOMDocumentXBL`.

---

## Test assets (`render/goanna/test/`)

| asset | what it answers |
|---|---|
| `make-anim-swf.py` → `jihad-anim.swf` | does content MOVE (x-centroid = frame number) |
| `make-audio-swf.py` → `jihad-audio*.swf` | audio; PCM by default, `--mp3`, `--stereo` |
| `make-key-swf.py` → `jihad-key.swf` | real AVM1: GREEN until a key is handled, then RED |
| `alsa_probe.c` | can THIS runtime play sound (run under both loaders) |
| `plugin_mime_probe.c` | reproduce the daemon's plugin scan standalone |
| `audio_shim.c` | LD_PRELOAD: trace dlopen/dlsym, substitute `snd_pcm_open` |
| `npapi_test_plugin.c` | control plugin — proves defects are OURS, not Flash's |
| `uinput_kbd.c` | synthesises a HARDWARE keyboard (this device has none) for chrome-side key tests |
| `abi_probe.c`, `sem_dump.c`, `mtx_dump.c` | **cross-ABI, and they need NO DEVICE.** Build each with both in-tree toolchains (PDK gcc 4.3.3 / glibc 2.5 and the crosstool 9.4 / glibc 2.23), run both under `qemu-arm`, and the diff IS the ABI. Reproduced the PmLogLib `sem_t` hang on the build host in about a minute. See the cross-ABI section at the end of this file for the 2.5-is-a-proxy caveat |

Pages/assets staged in `/tmp` on the device (volatile — re-push after any reboot). The HTML
wrappers are trivial and are NOT in the repo; regenerate them as a `<embed
type="application/x-shockwave-flash" src="<swf>" wmode="opaque" width="320" height="240">`:

| on-device page | asset | what it is for |
|---|---|---|
| `jihad-plain.html` | none | the SWF-FREE page. Navigate here first when you need a plugin-container to spawn FRESH on the next navigate, and to release the `0027` CPU boost |
| `jihad-anim.html` | `jihad-anim.swf` | frame-rate work |
| `jihad-audio.html` | `jihad-audio.swf` | PCM square — the only asset confirmed CLEAN by ear |
| `jihad-audio-mp3.html` / `-44k.html` | mono 22 k / stereo 44.1 k MP3 | MP3 decode path |
| `jihad-audio-5s.html` | `jihad-audio-5s.swf` | 5 s sine MP3. LOOPED, so it clicks once per iteration by design — that is Flash restarting its decoder, not a defect |
| `jihad-audio-long.html` | `jihad-audio-long.swf` | **30 s single-shot MP3 (`--loops 1`). The ONLY asset that can answer a question about audio QUALITY**, because it has no loop point at all. Every other MP3 asset here clicks per iteration no matter how healthy the port is |
| `jihad-keyarb.html` | `jihad-key.swf` + 3000 px scroll region + `#sy` readout | keyboard arbitration: read scroll back with `gettext #sy` |
| `jihad-media.html` | `jt.webm` (VP8) + `jt.mp3` | **ENGINE media, nothing to do with Flash.** Reports `canPlayType`/`readyState`/`currentTime`/`error` into `#r`; read it back with `gettext #r`. Measured 2026-08-10: **VP8 plays**, H.264 unsupported, **MP3 refused outright** (`error.code=4`) because the ARM build compiles cubeb with NO backend. Regenerate the two media files with `ffmpeg -f lavfi -i sine=…` and `-f lavfi -i testsrc=… -c:v libvpx` |

SWF authoring traps — **every one of these has already produced a false negative in this tree,
and a bad asset is indistinguishable from a broken port:**
- **Shapes and sounds share ONE character dictionary.** `DefineSound` may not reuse a
  `DefineShape` ID. The player keeps the first definition and drops the second, `StartSound`
  resolves to the wrong character, and the sound is silently never played. This cost a whole
  session chasing a nonexistent player-side audio gate. Fixed 2026-08-10 (`SOUND_ID = 2`).
- `0x00020000` is ClipEvent**KeyPress** (and carries a trailing KeyCode byte) — KeyDown is
  `0x00000040`; getting it wrong makes the SWF draw NOTHING rather than ignore keys.
- A sprite with no `Stop` on frame 1 loops, which looks exactly like a key response and is not.
- For MP3 event sounds the SoundData begins with a **SeekSamples SI16**, not the frame stream.

Parse an asset back out before believing a negative result from it. A 30-line tag walk that
prints each tag code and character ID is enough, and it is what caught the ID collision.

---

## Device state as left

- **2026-10-14 (round 25):** all three variants on daemon `06e75a68` with the round-25 impls
  (`1bafcc0c` enyo, `4b8c44ba` mochi, `a5c205d1` mojo; no home path), shims unchanged, no test
  hooks, no proxy, `/` read-only. Each was launched once on https://example.com/ and closed.
- **2026-08-10, later session:** the enyo daemon and libxul now also carry the frame-pacing work
  (`PumpReady`, the 1:1 seq gate, patch `0028`, the tick/gap counters). `/etc/event.d/jihad`
  carries `JIHAD_TICK_MS=16` and `JIHAD_PLUGIN_FRAME_MS=33` as device-local additions ALONGSIDE
  `JIHAD_INJECT=1` — all three are lost if the job is regenerated from the template. The device
  was last left on a CLEAN restart running `/tmp/jihad-anim.html`, measured healthy: `done=61
  palm draw=63 deferred=0`, gap mode 32-39 ms.
- **2026-08-10, pull session: ALL THREE VARIANTS ARE IN STEP** and were re-pushed together at the
  end of the session, so every one carries the same libxul and daemon with patches `0027`, `0028`
  AND `0029`. The previous enyo-ahead-of-mochi/mojo skew is gone, so a frame measurement on any
  variant is now comparable. (Read the current md5s off the device rather than trusting a number
  written here — this file has carried a stale one before. `push-engine-update.sh` prints them.)
- **The libxul on the device carries the pacing INSTRUMENTATION as well as the fix**, and it is
  what makes the open question answerable: the child logs `show req: setwin=… hostinval=…
  plugininval=… retry=… bgupd=… bgtail=… bgdestroy=…` next to `palm draw`, on the same 2 s
  boundary. `hostinval` is our pull, `plugininval` is the plugin invalidating itself. Those two
  counters are the whole of open work item 2 — read them before changing anything about pacing.
  Adapter is unchanged — still the PDK build matching the `.ipk` (md5
  `0410a348042af58a7e711b495db69833`).
- **THE DEVICE IS DELIBERATELY LEFT IN THE "FLASH ALONE" CONFIGURATION — this is not an accident
  and it is not the shipping default.** `/etc/event.d/jihad` carries `JIHAD_PLUGIN_PULL=0` and
  `JIHAD_PLUGIN_SELF_DRIVE=0`, so BOTH host frame drivers are off and the plugin's own
  `NPN_InvalidateRect` is the only frame source (verified: `hostinval` is exactly 0).
  **It is left this way so a human can LOOK at it**, because the eye is the only instrument that
  has ever detected this defect: Flash alone measures SLOWER (24-27 vs 30-33 fps) but perfectly
  unbunched (**zero** gaps under 16 ms, against 1-5 with the pull on). If evenly-spaced 25 fps
  reads better than bunched 32 fps, the host frame clock should be deleted — see the R7 frame-rate
  criterion for the full three-way table and the recommendation.
  **Revert to the shipped default in one line** (then `stop jihad; start jihad`):
  ```sh
  mount -o remount,rw / && sed -i 's/ JIHAD_PLUGIN_PULL=0 JIHAD_PLUGIN_SELF_DRIVE=0//' /etc/event.d/jihad
  ```
- `/etc/event.d/jihad` is back at **`JIHAD_TICK_MS=16`**. It was moved to 8 for one A/B during this
  session and moved back; 8 measured WORSE (see the pull section). The edit was a `sed` on that one
  token, never a regeneration from the template, so `JIHAD_INJECT=1` survived — verified after each
  edit. If you A/B it again, do it the same way and re-check `JIHAD_INJECT=1` afterwards.
- `/etc/event.d/jihad` is the normal job (`JIHAD_INJECT=1`, no `LD_PRELOAD`, no kill switches) and
  now also carries the `post-stop` governor restore. **TRAP: the SHIPPING template in
  `packaging/gen-variant-scripts.sh` does NOT set `JIHAD_INJECT=1` — that is a device-local
  addition.** Regenerating the job and pushing it verbatim silently kills the inject channel, and
  every subsequent test then "passes" against an empty log. Re-add it when you reinstall the job.
- db8 kind `com.palm.app.flashplayer.prefs:1` registered (owner `com.palm.flashgraphics`) — this
  is a deliberate addition that fixes Flash's only reported error. Remove with `delKind` if
  unwanted.
- `/var/palm/jihad/adapter-backup/` holds the pre-2026-08-10 adapter pair.
- Flash's `oem.cfg` was temporarily modified and has been RESTORED to original.
- **The device was REBOOTED on 2026-08-10** (forced: `sync; sync; reboot -f`, because a clean
  `reboot` hangs on D-state tasks — see the cpufreq trap). It came back healthy on `ondemandtcl`
  and all three daemons auto-started. Governor tunables are at STOCK (`up_threshold=95`,
  `sampling_rate=200000`); the daemon takes them to 40/50000 only while a plugin is instantiated.
- Test pages/SWFs re-pushed to `/tmp` after that reboot and ALL CURRENT — `jihad-anim`,
  `jihad-audio`, `jihad-audio-mp3`, `jihad-audio-44k`, `jihad-audio-5s`, `jihad-audio-sine`,
  `jihad-key`, plus `jihad-plain.html` and `jihad-keyarb.html`. `/tmp` is volatile: re-push after
  any reboot (see the Test assets table for what each one is for).
- The synthetic keyboard is NOT running — it was destroyed (`echo quit > /tmp/uinput.cmd`) and the
  process is gone. `/tmp/uinput_kbd` may or may not still be there; rebuild with
  `build/webos-oe/build-uinput-kbd.sh`.
- `libflashplayer.so` in the enyo variant's `profile/plugins/` is **stock**, md5
  `14c20ba9fc2183a93648c70488e0ec8e`, byte-identical to
  `/media/cryptofs/apps/usr/lib/BrowserServerPlugins/libflashplayer.so`, which is the restore
  source. Two patched builds were run there during the audio hunt and both have been removed.

---

## Uncommitted work in the tree (2026-08-10)

Nothing is committed. What this session added, and why, so it is not mistaken for cruft:

| path | what it is |
|---|---|
| `build/desktop/patches/0027-npapi-plugin-cpu-boost.patch` | **the frame-rate fix.** Holds a CPU-governor boost for the life of a plugin instance. Applied to `third_party/uxp` already, and the enyo device libxul is built from it |
| `packaging/gen-variant-scripts.sh` | adds the `post-stop` governor restore to the upstart template, so a crashed daemon cannot strand the tuning |
| `render/goanna/test/uinput_kbd.c` + `build/webos-oe/build-uinput-kbd.sh` | the synthetic hardware keyboard, for the open keyboard-arbitration criterion |
| `render/goanna/test/make-audio-swf.py` | two real asset bugs fixed: duplicate character ID, and `SeekSamples` 0 → 1105 |
| `render/goanna/test/audio_shim.c` | header rewritten — its old "measured" conclusion was wrong |
| `docs/PICKUP.md`, `context/kits/cavekit-addons-extensions.md` | this handoff and R7 |
| `build/desktop/patches/0028-npapi-plugin-frame-seq.patch` | **the 1:1 frame fix.** Exports `jihad_plugin_frame_seq()` from `PluginInstanceParent::RecvShow` so the daemon can tell a NEW plugin frame from a repeat. Applied to `third_party/uxp` already; the enyo device libxul is built from it. Dry-run verified against a reconstructed pristine copy, since several other patches also touch that file |
| `render/goanna/GoannaRenderPage.{h,cpp}` | `PumpReady()` (drain-and-return, the tick's pump) beside `PumpFor()` (spend-the-budget, what the tests need); weak `jihad_plugin_frame_seq` binding + `PluginFrameSeq()` |
| `render/goanna/BrowserPageGoanna.{h,cpp}` | the publish policy: 1:1 on the frame seq with a stale-damage fallback, the deadline pacer as the no-seq fallback, and the frame-gap HISTOGRAM that made the defect visible at all |
| `render/browserserver/JihadBrowserServer.cpp` | tick cadence counters (period vs work, split pump/paint) on a monotonic clock; the tick pumps with `drainOnly` |
| `render/browserserver/Main.cpp` | tick period is `JIHAD_TICK_MS` (4-64), so cadence can be A/B'd without a rebuild |
| `render/goanna/test/dump-swf.py` | walks a SWF back out and FAILS it on the three asset bugs that have each impersonated a port defect here (duplicate character ID, SeekSamples, SyncNoMultiple retrigger) |
| `render/goanna/test/make-audio-assets.sh` | regenerates every audio asset from scratch and verifies each one. The MP3 streams behind the checked-in SWFs previously existed only in `/tmp`, so the repo could not reproduce its own test assets |
| `build/webos-oe/stage-test-pages.sh` | pushes the SWFs + generates the HTML wrappers into the device `/tmp`, md5-verified. Replaces hand-making them after every reboot |
| `README.md`, `docs/*.md` | **de-duplicated against the kits** (user instruction 2026-08-10). The kits own requirements and per-criterion STATUS; docs own procedure. 233 lines of restated status removed, and with them three claims that had already gone stale and wrong: "Flash audio does not work", DEVICE-BUILD's "no device sysroot, no OE tree … and no TouchPad", and TOOLCHAIN's three-way toolchain choice presented as still open. README is now user-facing and positions against isis and Atlas |
| `context/plans/build-site.md` + `plan-overview.md` | regenerated from the kits: **56 tasks, 6 tiers, covering the 41 still-open criteria at 100%**, with the 8 hardware/human-gated ones quarantined in a final tier. The 2026-06-30 site is kept as `build-site-2026-06-30.archived.md` |
| `build/desktop/patches/0029-npapi-plugin-pull-frame.patch` | **the PULL.** New async parent→child `JihadRequestFrame`; the child answers with `JihadPalmRepaintTick()` and stands its own timer down on a 150 ms lease, logging every transition; `PluginInstanceParent` exports `jihad_plugin_request_frame()` over a file-static live-instance list. Applied to `third_party/uxp` already and live on all three variants. **Its own header records that it does NOT fix the pacing and why** — read it before building on it. Verified by reconstructing the pre-0029 baseline (reverse-applying only this session's edits), dry-running the patch onto it, and confirming it reproduces the working tree byte-for-byte |
| `render/goanna/BrowserPageGoanna.{h,cpp}` (pull) | `requestNextPluginFrame()`: the whole-tick request grid, the one-outstanding backpressure gate with a 250 ms timeout, and `jihadTickPeriodMs()` — the single definition of the tick period, which `Main.cpp` now calls instead of reading the env itself. Deliberately NOT gated on `mEmittedFlashRects`: that map is what EMPTIES when the plugin wedges, so gating there turns a recoverable wedge into a permanent one |
| `render/browserserver/JihadBrowserServer.cpp` (pull) | calls `requestNextPluginFrame()` once per tick — NOT from `maybePaint()`, which `returnBuffer()` also reaches off-tick, which would put requests off the grid they exist to stay on |
| `build/desktop/patches/make-0029.sh` + `.baseline-pre-0029/` | **the fix for the patch-authoring trap this file has warned about three times.** Reconstructs the correct baseline — `git show HEAD:` plus every EARLIER patch that touches the same four files (`0008 0017 0020 0022 0024 0026 0028`) — and diffs the live tree against it, so a new patch cannot silently fold in its predecessors or come out empty. `--rebuild-baseline` regenerates the dir. Generalise it before authoring `0030`; the file list and the `EARLIER` list are the only per-patch parts |

`third_party/uxp/dom/plugins/ipc/PluginModuleParent.cpp` is modified in place (that is how patches
work here — the build applies them to the shared tree, idempotently, with a dry-run gate). If you
regenerate `0027`, diff against a pristine copy of that file, not against the working tree.

---

## If you are picking this up cold, do this first

1. Read the Flash status table above, then the two open items at the top of **Open work**.
2. `novacom -l` — confirm `topaz-linux`. Re-push `/tmp` assets if the device rebooted.
3. The cheapest real progress available is **counting the third draw driver** (open work item 2):
   two 2 s counters, one on `PluginInstanceParent::SendUpdateBackground` and one on each
   `AsyncShowPluginFrame` call site, then one clean-restart run. Every pacing conclusion in this
   document rests on not knowing that number.
   **Audio is CLOSED — do not start there.** An earlier revision of this list sent you to an
   "MP3 static" question that the audio section above had already resolved at parity with stock;
   the contradiction cost a session start. If two parts of this file disagree, the dated
   subsection wins over this list.
4. Keyboard arbitration is NOT blocked on the browser — it is blocked on getting a key out of
   `hidd` into the card. Start at `/usr/lib/libhidinputdev.so`, not at the adapter.
5. Whatever you measure: assert the daemon log GREW, and assert the card is connected, BEFORE
   believing any number. Both failure modes look exactly like success.

---

## The cross-ABI class is now HOST-TESTABLE — do not wait for the device (2026-08-10)

`render/goanna/test/{abi_probe,sem_dump,mtx_dump}.c`, promoted out of a scratch dir because this
is the single most reusable thing found this session. Both toolchains are already in the tree and
`qemu-arm` runs their output on the build host:

- `build/webos-oe/pdk/opt/PalmPDK/arm-gcc` — gcc 4.3.3, sysroot glibc **2.5**
- `build/webos-oe/toolchain/.../arm-webos-linux-gnueabi` — gcc 9.4, glibc **2.23**

Static-link the same probe with each, run both under `qemu-arm`, and **the diff IS the ABI.**
This reproduced the PmLogLib `sem_t` hang off-device in about a minute — a bug that previously
cost a device session to find.

**CAVEAT, stated because it matters: the PDK sysroot is glibc 2.5 and the device is 2.8.** Both
are pre-2.21, so it is the right side of every cliff that matters here, but 2.5 is a PROXY and a
result that hinges on 2.5-vs-2.8 rather than pre-vs-post-2.21 is not proven by this method.
