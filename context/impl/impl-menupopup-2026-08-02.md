---
created: "2026-08-02"
last_edited: "2026-08-02"
status: DIAGNOSED — popup widget IS created (at the right place, 0x0, never shown/painted); fix is a two-parter
---

# menupopup — the diagnostic (build-popup-probe.sh)

The instrumentation (patch 0012 + the `[jihad-widget]` lines in patch 0005) run against a plain
`<select>` driven through the inject channel, headless. The decisive lines:

```
[jihad-bs] clickAt (200,120) <SELECT> href=- nav=0        # the tap lands on the <select>
[jihad-widget] create type=2 popup=1 ... bounds=100,144 0x0
[jihad-bs] clickAt (200,190) <HTML> ...                    # 2nd tap hits page bg — dropdown not there
```

So, settled:

1. **The popup machinery is NOT dead.** Clicking the `<select>` creates a real popup widget
   (`eWindowType_popup`, `IsPopup()=1`) at **(100,144)** — exactly under the control
   (`left:100 top:100 height:44`). The combobox open path works.
2. **The popup widget is created 0x0 and never shown.** No `[jihad-widget] popup show=1` line and
   no `[jihad-popup] ShowPopupCallback` line ever fire — the popup child is created (the eager
   non-leaf path in `nsMenuPopupFrame::Init`), but it is never sized (`0x0`) and its
   ShowPopupCallback path does not run to completion in this offscreen embedding.
3. Because the second tap at (200,190) resolves to `<HTML>`, the dropdown occupies no space on
   screen — consistent with 0x0 + unpainted.

This is the same root that blocks the about:addons tools menu and (very likely) `contextmenu`
delivery: all three ride the XUL popup path.

## The fix is two parts (both needed; neither alone is enough)

**A. Size + show the popup widget.** In the offscreen PuppetWidget popup path, the frame reflow's
`SetPopupPosition`/`Resize` is not sizing `mBounds` (stays 0x0), so `JihadEnsureDrawTarget` has
nothing to allocate and ShowPopupCallback has no geometry. Needs the popup's `nsMenuPopupFrame`
measured size pushed into the PuppetWidget popup child's `Resize`, and `Show(true)` honored for
the popup window type (today Show on a popup only flips `mVisible`).

**B. Composite open popups as an overlay.** A popup is a SEPARATE display root
(`nsLayoutUtils::GetDisplayRootFrame` returns the popup frame itself). `RenderRegion`/`ReadPixels`
render only the main content docShell's presShell, so even a correctly-sized popup would not
appear in the shared buffer. After the main paint, enumerate
`nsXULPopupManager::GetInstance()->GetVisiblePopups()` and, for each, `RenderDocument` (or
`nsLayoutUtils::PaintFrame`) its frame into the buffer at the popup's screen offset. The daemon
already owns the buffer and the offsets; this is a bounded addition to `paintToSharedBuffer`,
gated so a zero-popup page pays nothing.

## How to iterate
`build/desktop/build-popup-probe.sh` (self-wraps under Xvfb; a plain `<select>` + two injected
clicks) prints every `[jihad-widget]`/`[jihad-popup]` line and the clickAt resolution. Add
`JIHAD_DUMP=1` to see whether the overlay lands once part B is in. The device path is the same
minus Xvfb (cairo-headless has no GTK), so desktop is the right first loop.

## Also confirmed working here
- clickAt coordinate mapping (`clickAt 200,120 (doc 200,120)` — identity at scroll 0, and the
  device run proved the non-zero-scroll case).
- Hit-test resolves the tapped element (`<SELECT>`, `<HTML>`), which is what the long-press card
  gate consumes.
