---
created: "2026-07-19"
last_edited: "2026-07-19"
---

# Device test 2026-07-19 — retest results (daemon 33a1aaa0 / libxul f7969264)

User retest + log forensics after the crash-safe-submit / dirty-repaint /
engine-editorFocused batch. AUTHORITATIVE open-issue list; supersedes the
open items of device-test-2026-07-17.md where noted.

## ✅ Fixed-confirmed on device
- **T2 Enter/address bar** — "Enter and address bar work." Old 14k-loop +
  DOMClick SIGSEGV class: 0 respawn-crashes of that signature in today's log.
- **T5 buttons** — "Buttons work."
- **T4 VKB raise** — "VKB always raises."
- **T3 stale frame (load path)** — example.com renders with zero input 12 s
  after remote launch; log shows alternating double-buffer `painted` frames.
- **Back button** works.
- Historical note: the Jul-17 "overload" log block (14,121 × google.com
  `content-nav re-drive ->` ping-pong, aborted with NS_BINDING_ABORTED) is
  OLD-daemon-era (log lines ≤180760). New daemon logs `re-drive GET ->` and
  showed ZERO such loops today.

## 🔴 U1 — Daemon still crash-loops on some non-editable taps (NEW ROOT)
**Report:** "clicking links … sometimes overloads the browser resulting in
full page loading screen"; html.ddg "thinks the page is still loading."
**Forensics:** 36 daemon restarts ("engine up") in today's 8.5k-line segment.
No dying words. Clustered last-actions before death: 14× `vkb tag=[DIV|HTML|
P|IMG|A|H1|BODY] editable=0` (i.e. mid-ClickAt, non-editable branch — right
before/inside the two `SendMouseEvent(mousedown/mouseup)` calls at
GoannaRenderPage.cpp:1585), 7× right after a `painted` line.
**Hypothesis:** page JS run synchronously inside SendMouseEvent dispatch hits
another native-code MOZ_CRASH (same class as DOMClick's SubjectPrincipal, or a
headless-unimplemented API). Needs a backtrace — desktop gdb repro loading a
JS-heavy page (google) and ClickAt-ing non-editable elements.
**Consequence chain:** crash → upstart respawn → adapter socket drop → app
full-card loading overlay ("overload") → app load-state wedged → VKB tap
suppressed on html.ddg ("still loading").

## 🔴 U2 — Search-suggestion dropdown items not tappable (google)
Suggested-searches listbox taps do nothing. Listbox is JS-rendered (role=
listbox/option, no <a href>); tap hits the non-editable path → likely killed
by U1 crash, or the item needs mousedown-activation semantics our synthetic
click doesn't trigger. Re-test after U1 fix.

## 🔴 U3 — Forward button: never greyed, doesn't work
canGoBack/canGoForward state either not emitted or not consumed (Enyo app).
Also "History is not working" (H view empty) → global-history emission
(update-global-history) or app-side db path broken on device. Investigate
daemon msgUpdateGlobalHistory + app HistoryList flow.

## 🔴 U4 — VKB viewport adjustment janky (T1 family, still open)
White band on top sometimes; rendering "snaps around"; page occasionally
still shoved off-screen. The focus-scroll-restore vs VKB-resize reflow fight
(F-247 restore + setWindowSize resize + engine scroll-into-view). T1
instrumentation lines (setWindowSize scroll before/after) present in today's
log — analyze `today.log` segment; consider: emit focused-field rect;
re-assert restore after resize settles; suppress engine scrollIntoView.

## 🔴 U5 — Landscape mode broken (explicit user ask)
Adapter rotation-guard (white frame on orientation mismatch) is a stopgap;
real fix = adapter handlePaint honoring buffer orientation vs dst (stride) or
daemon emitting rotated buffer. See DEVICE-HANDOFF 2026-07-06 landscape notes.

## 🟡 U6 — Self-drive requirement (explicit user ask)
"Learn how to drive the touchpad yourself": build an on-device input injector
(write input_event structs to /dev/input/eventX for tap/swipe; cross-build
with the crosstool toolchain) + fb1 screenshot loop → closed-loop testing
without a human. Existing: fb1 capture works (dd + PIL), palm-launch works.

## Progress this session (daemon 22520fd3 + app da0152c/39944f5)

- **U6 self-drive — DONE.** Daemon inject channel (`/media/internal/jihad/inject.cmd`,
  polled ~5×/s in tick) drives the active card: click/hold/key/text/url/back/
  forward/reload/stop/scroll/drag/size/zoom. Proven: `inject url` navigates,
  `back`/`forward` both work at the daemon. `build/webos-oe/device-retest.sh`
  streams the T-signal log. Enables closed-loop testing (fb1 capture + inject).
- **U3 History — ROOT-CAUSED + FIXED.** The app wrote to stock kinds
  `com.palm.browser{history,bookmarks,preferences}:1` (owner com.palm.app.browser;
  no permission for the Jihad app id) → every write DENIED (-3963). Fix: Jihad-OWNED
  kinds `net.riverstonerelay.jihad-browser.{history,bookmarks,preferences}:1` +
  permissions (app da0152c), all 8 dbKind refs + appinfo universalSearch repointed
  (39944f5). Proven on device: putKind/putPermissions as the app + a history
  put/find round-trips. `build/webos-oe/register-db-kinds.sh` registers them after
  a dev `palm-install` (the SDK tool skips the appinstaller's db step; a real ipk
  install does it). REMAINING: confirm the app's own loadStopped→updateHistory→put
  fires on address-bar/link nav (my inject only drives web content, not the app UI).
- **U3 Forward — daemon side CLEARED.** `titleAndUrl back=/fwd=` logging proves the
  daemon emits correct flags (fwd=1 only after Back); adapter (msgTitleAndUrlChanged,
  4 args) → framework BasicWebView (titleURLChange→doPageTitleChanged, 4 args) →
  Browser.pageTitleChanged→gotHistoryState→actionbar.setCanGoForward all wired.
  So greying/forward SHOULD work — needs a user confirm on real nav; if still wrong
  it is a subtle app-state reset, not the nav engine.
- **U1 crash — instrumented, not yet caught.** NOT OOM (swap barely used). Core
  capture ARMED (`core_pattern` → /media/internal/jihad/cores, `ulimit -c unlimited`
  in the upstart job) + a breadcrumb around SendMouseEvent in ClickAt, so the next
  real crash yields a core+log naming the faulting element. Could NOT reproduce
  synthetically (toy onclick/location pages + google taps survived); needs the
  user's real tap targets or a caught core.
- **U4 VKB jank — quantified.** Window sizes seen: portrait full 768×942 ↔ VKB
  768×602 (13k×), landscape 1024×686/768. The jank is the 942↔602 reflow fight on
  VKB toggle. Fix still open (emit focused-field rect / re-assert restore post-resize).
- **U5 landscape — scoped.** The card stays portrait under a daemon `size` inject
  (chrome down the left edge); landscape is webOS CARD-orientation driven (adapter
  composite + window manager), not daemon-simulable. appinfo has no orientation
  lock, so the card is free to rotate — the remaining work is the adapter
  (BrowserAdapterImpl.so) landscape composite (stale-orientation blit → scanlines;
  staged rotation guard) which needs a PDK adapter rebuild + physical rotation test.

## Session 2 additions (user retest + new reports)

- **History — CONFIRMED WORKING by user** ("history works"). db has real entries
  (trueog.net, Gmail). Closes the U3-History item.
- **Bookmarks — FIXED (216bae1).** User: "adding bookmarks via share menu doesn't
  show up." The bookmark never reached the db though the kind + write are fine.
  Root: addBookmark called createPageImages() (saveViewToFile/generateIconFromFile/
  resizeImage via callBrowserAdapter, writing under /var/luna/data/browser/icons,
  mode d-wxr----t owner luna → app user can't write) BEFORE the put; the failure
  aborted addBookmark. Wrapped createPageImages in try/catch (addBookmark +
  showAddtoLauncherDialog) so the record always saves; thumbnails best-effort.
  RETEST: add a bookmark via share menu → should appear in Bookmarks.
- **Downloads — WIRED (218ed0e).** The engine download interceptor was installed
  but no sink was ever set (SetDownloadSink only got nullptr) → captured downloads
  went nowhere. Now JihadBrowserServer IS the DownloadSink: OnDownload → active
  card's new IPageMessageSink::msgMimeHandoffUrl → BrowserServerBase::
  msgMimeHandoffUrl (YAP 0x2014) → adapter mimeHandoffUrl → app handleResource →
  com.palm.downloadmanager. Daemon 74ebea4b deployed. RETEST: tap a real download
  link (e.g. a .zip); end-to-end (app → downloadmanager → completion) unverified.
- **Mochi 'loads forever' — FIXED (a8f282a), verified on device.** REAL root
  cause (from /var/log/messages: `Uncaught ReferenceError: Mojo is not defined`
  led here): webOS keeps the app card's loading spinner up until the app signals
  it rendered. The Enyo-1 variant gets that free via the system framework's
  `launch="bridged"`; the bundled Enyo-2 Mochi app never called
  `PalmSystem.stageReady()`, so the card never presented even though the WebView
  connected + the start page loaded. Added stageReady() after renderInto — the
  card now OPENS and shows the Mochi start page (screenshot confirmed). Also
  (141e28d) added a Mochi-styled dark start page on empty launch + #1c1c1c view
  bg (no white flash). REMAINING: address bar shows the raw data: URL of the
  start page instead of "about:jihad" (cosmetic); Mochi still carries codex
  cycle-2 findings F-434..443 and the full parity port T-053 is unbuilt.
- **U1 crash — CORE CAPTURED.** A 362 MB core (core.ld-2.23.so.17076) was caught
  and pulled to the job tmp; crashes cluster on the VKB resize (rapid 942↔686
  window-height flips). Backtrace with the cross-gdb + unstripped ARM libxul is
  the next step. NOT reproducible via synthetic taps.

## Session 3 (2026-07-19/20) — crash ROOT-CAUSED+FIXED, Mochi UI overhaul

- **U1 crash — FIXED (commit 2be6d85).** Captured a core, backtraced with cross-gdb
  + a -g rebuild: SIGBUS in GoannaRenderPage::BeginLoad with this==0xa, from
  BrowserPageGoanna::pump. A synchronous nav inside pump() (Enter-submit /
  tapped link / content re-drive) spins the engine's nested event loop, which
  re-enters tick() and re-pumps the SAME page mid-navigation, corrupting mPage.
  Fix: `if (mInTick) return;` re-entrancy guard in tick(). Verified: the exact
  google/ddg Enter+tap sequence that gave 36 restarts now gives ZERO across two
  stress rounds. This underlies U1 (overload), U2 (un-clickable suggestions), and
  the html.ddg "still loading" artifact. Deployed as a -g build for future cores.
- **Downloads — WIRED (218ed0e).** DownloadSink was never set; now JihadBrowserServer
  is the sink → msgMimeHandoffUrl → app → com.palm.downloadmanager. Needs a real
  download to verify end-to-end.
- **Bookmarks — FIXED (216bae1).** createPageImages() (thumbnail write to a dir the
  app user can't write) threw and aborted addBookmark before the put; wrapped in
  try/catch so the record always saves.
- **Mochi variant — major overhaul (multiple commits, latest 09c050c):**
  stageReady() so the card opens; Enyo-parity toolbar (back/forward, address with
  inline reload/stop, share, new-tab, history+bookmarks) as a consistent PNG icon
  set (this engine renders no mochi sprites / CSS-SVG / needed font glyphs / CSS
  circles); app-chrome start page with the crisp bundled logo (a big logo in a
  WebView data: URL exceeded the engine's URL limit → blank-white card — the root
  of "entire card is 100% white"); "Jihad Browser" title on both variants.
- **Enyo start page (478c616):** centred brand block (smaller logo + name +
  "Enyo UI ★ Goanna/6.9 UXP/b2594a4") matching Mochi.

## Still open (hard)
- **U4/T1 VKB jank** — white band on top, rendering "snaps around", page shoved
  off-screen on VKB raise (942↔602 reflow fight). Not fixed.
- **U5 landscape** — needs adapter composite rebuild + physical rotation test.
- Keyboard-focus-on-start (focusing the app input did not raise the VKB) and
  URL-bar off-white shade (mochi's white decorator override resisted; reverted to
  keep the card rendering) — both deferred.

## Test assets
- Full log: job tmp `retest-daemon.log` (189k lines; today = lines 180761+,
  extracted to `today.log`).
- fb1 screenshot pipeline validated (portrait rotate, BGRA).
