---
created: "2026-08-01"
last_edited: "2026-08-01"
---

> **2026-08-01 — all eight findings addressed; see "Resolution" at the bottom for what was done,
> what was measured, and the one thing that still needs the device.** Statuses in the table below
> are updated in place; the reviewer's evidence columns are left untouched.

# Adversarial review — XUL/chrome input fix (T-067/T-068, in commit f2462dbc)

**Reviewer: fable.** Reviewed against cavekit-input-bridging.md **R6**. The change was committed
mixed into a packaging commit by mistake, so it was isolated by path.

## Chain verification — the root cause is genuine

The reviewer independently re-derived the claimed chain in `third_party/uxp` rather than trusting
the summary, and it holds: `nsIWidget::UsePuppetWidgets()` is `XRE_IsContentProcess()`
(`widget/nsIWidget.h:1870`); `ShouldAttachToTopLevel()` falls to the `XP_WIN || MOZ_WIDGET_GTK` +
`typeChrome` branch and returns false; `PuppetWidget::GetCurrentWidgetListener()` returns
`mAttachedWidgetListener` with **no `mWidgetListener` fallback** (`widget/PuppetWidget.cpp:1778-1786`),
so `DispatchEvent` leaves `aStatus = eIgnore` when it is null. The headless device toolkit is
PuppetWidget-based. `…ToWindow` genuinely bypasses the widget via
`presShell->HandleEvent(...)` — **and passes `nullptr` for `aPreventDefault`**, which is where two
of the defects below originate.

| # | Sev | Where | Defect | Status |
|---|-----|-------|--------|--------|
| F-1 | **P0** | `GoannaRenderPage.cpp:1409-1424` + `ref-BrowserAdapter/BrowserAdapter.cpp:1574-1584` | **One tap now delivers TWO click sequences on every `user-scalable=no` page.** The adapter's pen path fires whenever `shouldPassInputEvents()` — true when the content fits the viewport and the meta-viewport is non-scalable, not only on interactive rects — *and* Jihad's tap→`asyncCmdClickAt` fires. Making `MouseEvent()` live (it was widget-dropped before) turned that into: engine click #1 toggles a checkbox, `ClickAt` samples the already-new state and dispatches click #2 which toggles it back — the control looks dead while firing two `change` events. Same chain double-submits forms and races two loads on a link tap (the `NS_BINDING_ABORTED` failure the code's own comment warns about). The tap→`clickAt` justification only considered the interactive-rect clause. **Neither `xul_test` (drives `ClickAt` directly) nor an `about:config` session can ever show this.** | **FIXED** (daemon-side dedup; `xul_test` phase K) |
| F-2 | P1 | `GoannaRenderPage.cpp:1723-1735` | The checkbox/radio fallback infers "the engine did not act" from unchanged state — but UXP **reverts** the pre-toggle on `eConsumeNoDefault` (`HTMLInputElement.cpp:4483-4505`). So a `preventDefault`ed click, or a page that manages `checked` itself, is indistinguishable from a dropped click, and the fallback force-toggles it and fires phantom `input`/`change`. The information needed to decide was discarded by the fix itself (`aPreventDefault = nullptr` on the ToWindow path). | **FIXED** (capture-phase click probe; `xul_test` L1) |
| F-3 | P1 | `GoannaRenderPage.cpp:1758-1789` | The submit fallback fires **exactly when the engine deliberately declined**: `onsubmit` returning false runs the handler twice per tap; a `preventDefault`ed submit click still navigates; and a **disabled** submit button submits, because the classifier checks the `type` attribute but never `GetDisabled` (unlike the checkbox branch). The busy-flag interlock's *timing* was verified sound — `OnLinkClickSync` sets `mBusyFlags` synchronously inside the mouseup dispatch — so the hole is not a race; it is that "nothing started loading" includes every legitimate refusal. | **FIXED** (same probe + `GetDisabled`; `xul_test` L2/L3) |
| F-4 | P1 | `GoannaRenderPage.cpp:1302-1345, 1636-1640` | **The named root cause cannot explain the recorded SIGSEGV — a dropped event cannot dump core.** Either the 2026-07-20 "SendMouseEvent on XUL SIGSEGVs (core dumped)" attribution was wrong (and what produced the core is still unknown), or the device path differs from desktop in a way the analysis misses. The code comments claim "the underlying fault is fixed"; the evidence supports only "the dropped-event defect is fixed". Meanwhile the new path executes **strictly more** XUL frame code on device than anything that ever ran there — `holdAt` → `contextmenu` → the XUL `<menupopup>` open path, tree frames, column pickers — none of which the 12-tap desktop run exercised, on a device whose widget probe shows theme components ABSENT. | **FIXED** (comments corrected; claim reduced to what is established) |
| F-5 | P2 | `build-xul-test.sh:61-65`, `xul_test.cpp:79` | **The R6 harness cannot fail.** `rc` is initialised to 0 and never assigned by any phase; the script prints `"xul_test exit: $?"` as its last command and exits 0 — even a SIGSEGV (139) becomes script-exit 0. This is the **ninth** fail-open instance found in this project's verification code. Phase I, the one double-toggle guard, tests only the single-`ClickAt` path and structurally cannot see F-1. | **FIXED** (every phase gates; failure path demonstrated 3 ways; phases K+L added) |
| F-6 | P2 | `docs/PICKUP.md:41-46` | "`about:config` is fully operable on desktop" overstates what was measured: no test selects a tree row or changes a pref value (R6 AC3's back half), and **keyboard into XUL is only `SetValue`-based text insertion** — `KeyEvent` still goes through `SendKeyEvent` → the widget → dropped, as the code itself admits. The promised `PuppetWidget::DispatchEvent` fallback patch **does not exist in the tree** (grepped `build/desktop/patches/` and `build/webos-oe/`). So R6 AC6 is not met by any shipped pathway. | **FIXED** (docs + kit corrected; AC3/AC6 now `[~]` with what was measured) |
| F-7 | P3 | `GoannaRenderPage.cpp:1708-1718` | Post-click focus adoption fights the tap-away policy in the same function: if a page cancels `mousedown` (common in drag/custom-UI libraries), focus never moves, the code re-adopts the still-focused **password** field the user just tapped away from, and the VKB pops back up. | **FIXED** (adoption requires the tap to have caused the focus) |
| F-8 | P3 | `EngineHost.cpp:650-655` | The console-bridge "is this ours" filter passes `about:`-sourced **and empty-source** scripts, so content in an `about:blank` iframe can echo page data into the persistent device log — the F-163 class the filter exists to prevent. | **FIXED** (chrome/resource always, about: allowlist, empty source only for non-script messages) |

## Claims attacked that HELD (recorded so they are not re-litigated)

- **Clipboard helper ABI:** the hand-declared IID is byte-identical to `nsIClipboardHelper.idl`'s
  uuid and the two-method vtable order matches the IDL exactly, so the scriptable QI from
  `config.js:21` succeeds. Registration defers to a real toolkit helper if present. No security
  consequence — `Components.classes` is chrome-only and the text stays in-process.
- **R5 coordinate evidence transfers:** both dispatch paths share every line of
  `nsContentUtils::SendMouseEvent` up to the `aToWindow` branch — same `GetWidget(offset)`, same
  `ToWidgetPoint(...)` — so zoom/scroll mapping is bit-identical. The bypass does **not** invalidate
  R5's on-device evidence.
- **R4 drag scrolling cannot regress:** `dragProcess` is `GetScrollXY`+`ScrollTo`, no mouse events.
- **The `isXul` skip is truly gone** — zero hits for `isXul` or the XUL namespace string in
  `render/goanna/`, and no namespace branch remains downstream. R6 AC1's first clause is met.

## The standing lesson, now at nine instances

F-5 is the ninth fail-open defect found in this project's *verification* code this session (after
eight `… | grep -q` under `pipefail`, a stale-socket liveness read, a backgrounded shell that
cannot print "Segmentation fault", and an audit watching paths that no longer existed). A test that
cannot fail is worse than no test, because it is counted as evidence. **Any new harness in this repo
should be assumed guilty until its failure path has been demonstrated.**

---

# Resolution — 2026-08-01

Fixed in the working tree (uncommitted). Files touched: `render/goanna/GoannaRenderPage.cpp`,
`render/goanna/GoannaRenderPage.h`, `render/goanna/EngineHost.cpp`,
`render/goanna/test/xul_test.cpp`, `build/desktop/build-xul-test.sh`,
`docs/PICKUP.md`, `context/kits/cavekit-input-bridging.md`.
**The adapter was NOT touched** — see F-1 below for why that was a decision, not an omission.

## F-1 (P0) — one tap now delivers ONE click sequence

**Reproduced first, on the desktop.** A new `xul_test` phase K replays the real wire order for a
tap on a `shouldPassInputEvents()` page — `mouseEvent(down)`, `mouseEvent(up)`, a pump (the daemon
defers `clickAt` to the tick), `clickAt` — on a checkbox whose `onchange` repaints the body. Against
the unfixed daemon:

```
[xul] K2: pen pair + clickAt -> green=0 red=719697 -> double-toggled back to UNCHECKED (F-1)
[xul] K: FAIL (rc|=128)
```

**Fixed in the daemon, not the adapter.** `GoannaRenderPage::MouseEvent` records a down/up pair
that lands within 16 content px of itself (a tap, not a drag) with a CLOCK_MONOTONIC timestamp;
`ClickAt` matches it (≤1000 ms, ≤24 px) and, when it matches, stands down: no second
`SendMouseEventToWindow` pair, no `mClickNavUrl`, no checkbox/submit fallback. The record is
CONSUMED by the first `clickAt` after it, so a stale pair can never suppress a later tap.

Why here rather than in the adapter — the reviewer explicitly left this open:

1. **The adapter option loses capability.** Suppressing the tap→`clickAt` when the pen path fired
   would silence, on exactly the pages where the pen path is live, everything `clickAt` does that
   the raw pair does not: the hit-test, the near-miss link touch target, the VKB editable
   classification and focus/caret work, and the crash-safe deferral into the guarded pump. The
   daemon-side dedup drops only the duplicate DOM dispatch and keeps all of it.
2. **The adapter option cannot be tested here.** No desktop harness runs the adapter; a fix there
   would ship on the strength of an argument. The daemon fix is exercised by `xul_test` phase K on
   every run.
3. **Robustness to the adapter changing under us** (the reviewer's own point): the daemon does not
   depend on `shouldPassInputEvents()` staying what it is, nor on which client sent what. Any
   client that sends both — the shared adapter behind all three UI variants, or the inject
   channel — is covered.
4. Cost is four ints, a timestamp and one pointer.

**Which path wins: the raw pen pair**, necessarily — it is delivered first and cannot be recalled,
and down/up/click is the correct browser sequence anyway. Two measurements say nothing is lost by
standing down: `K1` (the pair alone checks the checkbox) and `K3` (the pair alone navigates an
`<a href>` through the engine's own click default action — which is exactly why `clickAt`'s direct
navigate had to be suppressed too, or the tap would start two loads).

## F-2 (P1) — the checkbox/radio fallback now detects DELIVERY, not state

A one-shot capture-phase `click` listener (`ClickProbe`) is registered on the tapped control just
before dispatch and removed immediately after. `clickDelivered = rawClickDelivered || probe->Saw()`.
The hand-flip runs only when **no click reached the control at all**; `defaultPrevented` is read off
the retained event after dispatch and logged for diagnosis but never used to activate — a page that
cancels its click has refused, and re-running it by hand is the bug. Capture phase specifically, so
a content handler calling `stopPropagation()` cannot hide the click and trick the fallback.

## F-3 (P1) — the submit fallback, plus the missing `GetDisabled`

Same delivery gate, and the classifier now asks `nsIDOMHTMLButtonElement::GetDisabled` /
`nsIDOMHTMLInputElement::GetDisabled` (not the attribute), which also catches a button inside a
`<fieldset disabled>` — the check the checkbox branch already had. That check is load-bearing under
delivery detection: a disabled control receives no click, so "no click arrived" would otherwise read
as a dropped event and submit the form. The busy-flag interlock is **unchanged** — its timing was
verified sound by the reviewer and was left alone. The block was restructured so the "the page
declined" diagnostic is printed only for a tap that really was on a submit control.

## F-4 (P1) — the claim reduced to what is established

Both comment blocks now say: the dropped-event defect is fixed and XUL input works on the desktop
headless build; the 2026-07-20 "SendMouseEvent on XUL SIGSEGVs (core dumped)" is **unattributed** —
a discarded event cannot dump core, so either that attribution was wrong or something
device-specific is unaccounted for. The removal of the `isXul` skip is stated as desktop-proven, not
device-proven, and the note about the new path reaching more XUL frame code on device
(`holdAt` → `contextmenu` → `<menupopup>`, tree frames) is recorded at the site. The root-cause
chain itself is untouched — it is correct and it is the valuable part.

## F-5 (P2) — the harness can fail, DEMONSTRATED

`xul_test.cpp` now ORs a distinct bit into `rc` on every gated phase's failure condition (A, B, D,
E, G, H, I, K, L) and prints the mask; `build-xul-test.sh` captures `rc=$?` **before** echoing and
`exit $rc` (the `echo` used to be the last command, so the script's status was always 0).

Also fixed as part of this: phase C's default click coordinate was the viewport centre (empty
chrome), so D and E could never observe the XUL default action or the filter box in a default run —
the "about:config is operable" claim rested on a hand-passed `JIHAD_XUL_CLICK`. The default is now
the measured centre of the "I promise to be careful!" button (236,394 in the 1024x768 render, read
off `/out/xul-config.ppm`).

**Failure path demonstrated three ways** (each break reverted immediately after; the tree is clean
of them):

1. **A real defect, found before the fix.** Phase K against the unfixed daemon:
   `[xul] K2: ... double-toggled back to UNCHECKED (F-1)` → `[xul] done rc=128 FAIL` →
   `== xul_test exit: 128 ==` → `SCRIPT-EXIT=128`.
2. **F-2/F-3 gates deliberately removed** (`clickDelivered` forced `false`, both `GetDisabled`
   checks deleted):
   ```
   [xul] L1: preventDefault()ed checkbox -> phantom change FIRED (F-2 REGRESSION) (red=719697)
   [xul] L2: onsubmit=false -> green=0 red=42864 -> handler ran TWICE (F-3 REGRESSION)
   [xul] L3: disabled submit button -> form SUBMITTED (F-3 REGRESSION) (red=42864)
   [xul] done rc=384 FAIL
   ```
   **This run also caught a defect in the fix itself:** the script reported
   `== xul_test exit: 128 ==` while `rc` was 384 — a process status is 8 bits, so bit 8 was
   silently dropped, and 128+ collides with the shell's own 128+signal encoding. The harness now
   prints the full mask and returns `rc ? 1 : 0`. A bitmask returned as an exit status is itself a
   fail-open.
3. **A deliberate null dereference after phase A** — the reviewer's exact scenario, a SIGSEGV the
   old runner reported as success:
   ```
   Segmentation fault
   == xul_test exit: 139 ==
   SCRIPT-EXIT=139
   ```

New phase **L** covers F-2/F-3 as regressions: L1 a `preventDefault`ed checkbox (no phantom
`change`), L2 `onsubmit` returning false (the handler paints green on run 1 and red on run 2, so a
double-submit is a colour), L3 a disabled submit button. Phase **J** (`about:addons`) is
deliberately NOT gated and says so in the file — it renders a `<parsererror>` for a separate, named
reason (the branding strip: `No chrome package registered for chrome://branding/locale/brand.dtd`),
so gating it would make this harness fail for something it does not test. Surviving the tap is still
checked, by the process not dying.

## F-6 (P2) — docs corrected

`docs/PICKUP.md`: "about:config is fully operable on desktop" → what was measured (warning
button `oncommand` runs and the prefs tree replaces the deck; typing filters the list), what was
not (selecting a tree row, changing a pref value — R6 AC3's back half), that the filter text goes in
via `InsertText`/the engine editor while `KeyEvent` is still widget-dropped, and that none of it has
run on the device since the change.

`cavekit-input-bridging.md`: R6 AC3 and AC6 moved to `[~]` with those notes. AC6 records explicitly
that the promised `PuppetWidget::DispatchEvent` fallback patch **does not exist in the tree**
(`build/desktop/patches/` has 0001–0010, none touching PuppetWidget).

**Recommendation on that patch, not taken here:** it is worth adding. Two lines in
`PuppetWidget::GetCurrentWidgetListener()` — fall back to `mWidgetListener` when
`mAttachedWidgetListener` is null, exactly as `nsWindow::DispatchEvent`'s `GetListener()` does —
would fix the defect at its source instead of routing around it, and would make `SendKeyEvent` and
every other widget-routed synthesis work (real keydown/keypress into XUL, i.e. AC6's actual ask).
Cost: a full libxul rebuild (desktop **and** ARM) plus re-bundling, and everything currently
verified against the shipped engine would want re-running. Flagged, not started.

## F-7 (P3) — focus adoption requires the tap to have caused the focus

Focus is snapshotted before the dispatch (as a bare pointer, compared only, never dereferenced) and
compared after. Adoption now needs `focus changed` **OR** `the focused control is the tapped element
or anonymous content inside it` (`jihadIsSelfOrInside`, walking the DOM parent chain, which for XBL
anonymous content leads back to the bound element). A cancelled `mousedown` satisfies neither, so it
can no longer re-raise the VKB over a password field the user tapped away from.

The second clause was added because the first alone **broke phase E**: re-tapping the
already-focused `about:config` filter box changes nothing, and a strict "focus must have changed"
rule dropped the edit target on the floor (`E: filter did NOT apply`, `rc=8`). Caught by the
harness, one run after it was written — which is the argument for phase E having a gate.

On the F-1 dedup path the snapshot is taken at the raw `mousedown` instead, since that is when the
focus move actually happened.

## F-8 (P3) — console bridge source filter tightened

`chrome://` and `resource://` always; `about:` only as an exact-match allowlist
(`about:config`, `about:addons`, `about:preferences`, `about:support`) rather than a prefix, because
a prefix admits `about:blank`, which is content; and an empty source name only when the message is
**not** an `nsIScriptError` (a plain `nsIConsoleMessage` comes from XPCOM internals — the chrome
registry, NSS — never from page JS, whereas an eval/`data:` script error has no source name).
Verified the useful diagnostics survive: `js-console (no source):0 No chrome package registered for
chrome://branding/locale/brand.properties` and the `about:addons` XML parse error both still print.

## F-9 — FIXED 2026-08-02

`mouseEvent()` and `holdAt()` no longer dispatch into the engine from the YAP socket callback.
They record into `mPendingMouse` (`BrowserPageGoanna.h`); `pump()` drains the queue inside the
tick's `mInTick`/`mReap` page-lifetime guard, **before** the pending click — so on a
`shouldPassInputEvents()` page the pen path's mousedown/mouseup still land ahead of the tap's
synthesized click, the order a real pointer would produce. Details:

- **mousemove coalescing.** A drag delivers ~60 moves/s while `pump()` runs on the ~16 ms tick,
  so consecutive moves collapse into the newest position. Without it the queue grows faster than
  it drains and every stale intermediate point is still dispatched. `mousedown`/`mouseup` are
  never coalesced — each is semantically significant.
- **Queue bound** of 256, dropping the oldest, so a page whose handler never yields cannot grow
  it without limit.
- **`holdAt` maps coordinates at QUEUE time**, not dispatch time: `mapToContent` depends on the
  current zoom/scroll, and a fling between the callback and the next pump would otherwise resolve
  the long-press against a viewport the user was no longer looking at.
- **Cleared on explicit navigation** (`openUrl`, back, forward, reload) alongside `mPendingClick`
  — queued events aimed at the old document must not land on the new one.
- Swap-and-clear on drain, and a `!mPage` re-check each iteration, so a handler that tears the
  page down or queues more input cannot reenter the loop.

### Fable adversarial review of the F-9 fix — 5 findings, all addressed 2026-08-02

| # | Sev | What | Resolution |
|---|-----|------|------------|
| F1 | P2 | **Ordering inversion re-opens the double-activation.** YAP sockets live on the default GMainContext, and page JS run during a drain can spin a nested GLib loop (sync XHR in `onmousedown`). The finger-up `mouseEvent` and the tap's `clickAt` are then both delivered mid-drain: the up lands in the already-swapped (empty) queue while the click sets the separate `mPendingClick` slot, so the click ran BEFORE its own mouseup. The F-1 dedup record is written at mouseup *dispatch* time, so it did not exist yet → `ClickAt` fully activated, then the queued mouseup let the engine synthesize a second click. **Double form POST.** | FIXED — took the *complete* fix, not the suggested one-line `mPendingMouse.empty()` guard: clicks are now `PM_CLICK` entries in the SAME queue, so ordering is total and the interleaving is unrepresentable. The one-line guard would also have starved a click for as long as a drag kept the queue non-empty. `mPendingClick`/`X`/`Y`/`N` are gone. |
| F2 | P2 | **`holdAt` double-transformed its coordinates.** The adapter sends `asyncCmdHoldAt(m_penDownDoc.x, …)` — the SAME content-space point it passes to `asyncCmdMouseEvent(mousedown)` — but `holdAt` still called `mapToContent`, re-dividing by zoom and re-adding the engine scroll. Long-press on a page scrolled to y=1000 resolved at y≈2000: wrong element or none. Same bug already fixed for `clickAt`. Also forced a layout flush (`GetScrollXY(flushLayout=true)`) inside the YAP callback. | FIXED — verified the claim against `BrowserAdapter.cpp` before acting (both call sites read `m_penDownDoc`). `mapToContent` removed; coords used directly. |
| F3 | P3 | 256-entry cap dropped the oldest entry of any type, which can split a down/up pair (lone mouseup = stray DOM event; lone mousedown latches `:active` + page drag state). Comment claimed "oldest non-move", which the code did not do. | FIXED — dropping a press now also drops its matching release (stopping at the next press, i.e. a gesture that genuinely had none). Comment matches the code. |
| F4 | P3 | Swap-and-clear defeated clear-on-navigation for the in-flight batch: a nested-loop `openUrl` mid-drain cleared the (already swapped, empty) member queue while the local copy kept dispatching into the superseded document. | FIXED — `mNavGen` is bumped by every explicit navigation and re-read each drain iteration alongside `!mPage`. |
| F5 | P3 | `touchEvent` still dispatched `touchstart`/`touchend` straight from the YAP callback — the identical crash class. Unreachable today (the adapter's `doTouchEvent` is `#ifdef QT_FIXME`'d out) but the daemon command IS wired. | FIXED — routed through the same queue (`PM_TOUCH*`), so it is already safe whenever touch forwarding is enabled. |

Fable also confirmed, with receipts, three things that are NOT defects: draining mouse events before the click is *load-bearing* (the F-1 dedup depends on the mouseup dispatching first); move coalescing is sound because only a trailing move merges; and `mPendingMouse` was cleared at exactly the sites `mPendingClick` was. It noted the `!mPage` re-check is harmless but weaker than its comment implied — real safety comes from deferred teardown plus `MouseEvent` re-resolving `GetWindowUtils` per dispatch.

**Still open (pre-existing, not caused by this work):** `setHTML` clears neither input queue nor `mPendingEditActions`; back/forward/reload clear `mPendingMouse` but not `mPendingEditActions` while `openUrl` clears both. Neither is reachable from a live adapter path today.

**Verification.** Desktop harnesses: `xul` PASS, `link` PASS, `input` PASS (`click DID change the
page`), `focus` PASS. `input2` FAILs — but it fails **identically without the change**: the change
was stashed and the test re-run, giving byte-identical `holdOK=0 dragOK=1 insertOK=0`, exit 4.
Its `holdAt green=-1` is the harness's shm readback returning −1, a pre-existing paint-readback
failure unrelated to input dispatch. `dragOK=1` on both sides is the positive signal that the
queued mousemove path still scrolls.

**DEVICE-VERIFIED 2026-08-02 with a physical tap** (the user performed it; no software path can
inject one). A `user-scalable=no` page — which makes `shouldPassInputEvents()` true, so the tap is
delivered by the adapter's PEN path *and* as `asyncCmdClickAt` — with a checkbox that renders its
own verdict. One tap produced `PASS checked=true change=1 click=1`, and the daemon log shows the
ordering the fix exists to guarantee:

```
[jihad-bs] clickAt x=380 y=127 n=1
[jihad-bs] clickAt (380,127): pen path already delivered this tap's click — no second activation (F-1)
[jihad-bs] mouseSend <INPUT> at 380,127 : SKIPPED, pen path already clicked (F-1)
```

The pen path's mouseup dispatched BEFORE the queued tap's `ClickAt` — only possible because clicks
now ride the same ordered queue — so the F-1 dedup record (written at mouseup dispatch time) already
existed and suppressed the second activation. Exactly one activation reached the DOM. This is
precisely the interleaving fable's F1 predicted would double-activate with a separate
`mPendingClick` slot, and it is now closed on hardware. Test page kept at
`/var/palm/jihad/enyo/taptest.html` (source: it renders PASS/FAIL itself, so no log parsing needed).

**The `holdAt` coordinate fix is still NOT device-verified** — it needs a long-press on a SCROLLED
page, which also requires a physical gesture.

Historical note (superseded by the above): The adapter's pen path is only reached
by real `NpPalmPenEvent`s from LunaSysMgr's touch handling. Attempted and rejected:
`PalmSystem.simulateMouseClick(200, 332, …)` fires from the card and was confirmed to run
(`[Jihad] taptest down/up` in `palm-log`), but produced **no** `clickAt` and no mouse line in the
daemon log — a synthesized WebKit DOM click is not delivered to a windowless NPAPI plugin. The
touchscreen is not an evdev device (`/proc/bus/input/devices` has only gpio-keys,
pmic8058_pwrkey, headset), so `uinput` cannot inject one either. Confirming this on hardware
needs a **physical** tap: load a `user-scalable=no` page, tap a checkbox, and confirm it ends
CHECKED with ONE `change`, plus `clickAt (x,y): pen path already delivered this tap's click` in
the daemon log.

## NOT fixed — recorded, with reasons

- **F-9 (superseded by the section above; original text kept for the reasoning)** — `asyncCmdMouseEvent` runs the DOM dispatch synchronously in the YAP socket
  callback.** `JihadBrowserServer::asyncCmdMouseEvent` → `BrowserPageGoanna::mouseEvent` →
  `GoannaRenderPage::MouseEvent` with no deferral, unlike `clickAt`, which records only and runs
  everything from the guarded pump *because* an activation can run page JS that tears the document
  down mid-callback — the SIGSEGV whose core dump stalled I/O hard enough to REBOOT the device
  (R1 AC4, review #5 H-1). While every synthesized event was being discarded this was harmless;
  since T-067 it is live, and on `shouldPassInputEvents()` pages it is now the *primary* tap path.
  `holdAt` → `MouseEvent("contextmenu")` has the same shape. NOT fixed here: the exposure is
  identical before and after this change (F-1 only removes the second dispatch, it does not add
  reachability), the fix is a real behavioural change (queue mouse events, drain them in `pump()`
  ahead of the pending click, coalesce `mousemove`) affecting drag timing, and no desktop harness
  can exercise socket-callback re-entrancy. It should be done deliberately, with the device
  available.
- **Adapter-side suppression of the tap→`clickAt`.** Deliberately not done — see F-1. If the daemon
  dedup ever needs belt-and-braces, the two are independent and compose, but doing it now would
  cost the adapter's capability on those pages for no testable gain.
- **`about:addons`** (R6 AC4) remains broken for the already-named branding-strip reason. Out of
  scope for these findings.

## What still needs the device

F-1's on-device confirmation is **blocked** behind the separate card→adapter investigation: with no
adapter ever connecting (`NPP_New` never reached, reproduced with the monolithic adapter and no
shim), the wire `mouseEvent` path cannot be exercised on hardware at all. When an adapter does
connect: tap a checkbox on a `user-scalable=no` page and confirm it ends CHECKED with ONE `change`,
and watch the daemon log for `clickAt (x,y): pen path already delivered this tap's click`. Until
then F-1 is desktop-verified only — which is also true of every other line of the T-067 change.

**Addendum, 2026-08-01 — the tenth instance came from fixing the ninth.** The first version of the
repaired harness returned its phase bitmask as the process exit status. The demonstration run masked
384 down to 128 in transit. The rule holds one level further out than it looks: a harness that
*reports* wrongly is a harness that cannot fail, and only running the failure path shows it.
