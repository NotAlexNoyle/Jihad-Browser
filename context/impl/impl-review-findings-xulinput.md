---
created: "2026-08-01"
last_edited: "2026-08-01"
---

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
| F-1 | **P0** | `GoannaRenderPage.cpp:1409-1424` + `ref-BrowserAdapter/BrowserAdapter.cpp:1574-1584` | **One tap now delivers TWO click sequences on every `user-scalable=no` page.** The adapter's pen path fires whenever `shouldPassInputEvents()` — true when the content fits the viewport and the meta-viewport is non-scalable, not only on interactive rects — *and* Jihad's tap→`asyncCmdClickAt` fires. Making `MouseEvent()` live (it was widget-dropped before) turned that into: engine click #1 toggles a checkbox, `ClickAt` samples the already-new state and dispatches click #2 which toggles it back — the control looks dead while firing two `change` events. Same chain double-submits forms and races two loads on a link tap (the `NS_BINDING_ABORTED` failure the code's own comment warns about). The tap→`clickAt` justification only considered the interactive-rect clause. **Neither `xul_test` (drives `ClickAt` directly) nor an `about:config` session can ever show this.** | OPEN |
| F-2 | P1 | `GoannaRenderPage.cpp:1723-1735` | The checkbox/radio fallback infers "the engine did not act" from unchanged state — but UXP **reverts** the pre-toggle on `eConsumeNoDefault` (`HTMLInputElement.cpp:4483-4505`). So a `preventDefault`ed click, or a page that manages `checked` itself, is indistinguishable from a dropped click, and the fallback force-toggles it and fires phantom `input`/`change`. The information needed to decide was discarded by the fix itself (`aPreventDefault = nullptr` on the ToWindow path). | OPEN |
| F-3 | P1 | `GoannaRenderPage.cpp:1758-1789` | The submit fallback fires **exactly when the engine deliberately declined**: `onsubmit` returning false runs the handler twice per tap; a `preventDefault`ed submit click still navigates; and a **disabled** submit button submits, because the classifier checks the `type` attribute but never `GetDisabled` (unlike the checkbox branch). The busy-flag interlock's *timing* was verified sound — `OnLinkClickSync` sets `mBusyFlags` synchronously inside the mouseup dispatch — so the hole is not a race; it is that "nothing started loading" includes every legitimate refusal. | OPEN |
| F-4 | P1 | `GoannaRenderPage.cpp:1302-1345, 1636-1640` | **The named root cause cannot explain the recorded SIGSEGV — a dropped event cannot dump core.** Either the 2026-07-20 "SendMouseEvent on XUL SIGSEGVs (core dumped)" attribution was wrong (and what produced the core is still unknown), or the device path differs from desktop in a way the analysis misses. The code comments claim "the underlying fault is fixed"; the evidence supports only "the dropped-event defect is fixed". Meanwhile the new path executes **strictly more** XUL frame code on device than anything that ever ran there — `holdAt` → `contextmenu` → the XUL `<menupopup>` open path, tree frames, column pickers — none of which the 12-tap desktop run exercised, on a device whose widget probe shows theme components ABSENT. | OPEN |
| F-5 | P2 | `build-xul-test.sh:61-65`, `xul_test.cpp:79` | **The R6 harness cannot fail.** `rc` is initialised to 0 and never assigned by any phase; the script prints `"xul_test exit: $?"` as its last command and exits 0 — even a SIGSEGV (139) becomes script-exit 0. This is the **ninth** fail-open instance found in this project's verification code. Phase I, the one double-toggle guard, tests only the single-`ClickAt` path and structurally cannot see F-1. | OPEN |
| F-6 | P2 | `docs/DEVICE-HANDOFF.md:41-46` | "`about:config` is fully operable on desktop" overstates what was measured: no test selects a tree row or changes a pref value (R6 AC3's back half), and **keyboard into XUL is only `SetValue`-based text insertion** — `KeyEvent` still goes through `SendKeyEvent` → the widget → dropped, as the code itself admits. The promised `PuppetWidget::DispatchEvent` fallback patch **does not exist in the tree** (grepped `build/desktop/patches/` and `build/webos-oe/`). So R6 AC6 is not met by any shipped pathway. | OPEN |
| F-7 | P3 | `GoannaRenderPage.cpp:1708-1718` | Post-click focus adoption fights the tap-away policy in the same function: if a page cancels `mousedown` (common in drag/custom-UI libraries), focus never moves, the code re-adopts the still-focused **password** field the user just tapped away from, and the VKB pops back up. | OPEN |
| F-8 | P3 | `EngineHost.cpp:650-655` | The console-bridge "is this ours" filter passes `about:`-sourced **and empty-source** scripts, so content in an `about:blank` iframe can echo page data into the persistent device log — the F-163 class the filter exists to prevent. | OPEN |

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
