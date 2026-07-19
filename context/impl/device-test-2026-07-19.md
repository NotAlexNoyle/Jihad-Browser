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

## Test assets
- Full log: job tmp `retest-daemon.log` (189k lines; today = lines 180761+,
  extracted to `today.log`).
- fb1 screenshot pipeline validated (portrait rotate, BGRA).
