---
created: "2026-08-02"
last_edited: "2026-08-02"
status: OPEN — the about:addons settings/utilities button
---

# `about:addons` settings button (next to the search bar)

## What it actually is

`chrome/toolkit/content/mozapps/extensions/extensions.xul:137`:

```xml
<toolbarbutton id="header-utils-btn" class="header-button" type="menu"
        tooltiptext="&toolsMenu.tooltip;">
  <menupopup id="utils-menu">
    <menuitem id="utils-updateNow"   command="cmd_findAllUpdates"/>
    <menuitem id="utils-viewUpdates" .../>
    ...
```

So it is **not** a plain button: it is `type="menu"` and its whole function is to open the
`utils-menu` **`<menupopup>`**. Two separate things can therefore be wrong with it, and they need
separating before either is "fixed":

1. **Appearance** — its icon comes from the mozapps skin like the category icons, so it is subject to
   the same *slow to render* repaint latency now tracked in `impl-addons-icons-open.md`. If it looks
   blank at first and fills in later, that is that defect, not a button defect.
2. **Function** — opening a `<menupopup>` requires a **popup widget**. In this embedding that is a
   PuppetWidget child with `mWindowType == eWindowType_popup`. PuppetWidget does model popup
   children (`third_party/uxp/widget/PuppetWidget.cpp:63` tests for `eWindowType_popup`, and `:347`
   asserts a child is one), so popups are **not obviously unsupported** — this must be TESTED, not
   assumed broken.

## Test it before changing anything

Tap the button on device and observe:

- menu appears → only the icon/latency issue applies; nothing structural to fix.
- nothing happens → XUL popup rendering/positioning in the headless embedding is the real defect,
  and it is **much broader than this button**: `<select>` dropdowns, context menus and every XUL
  menu depend on the same popup path. That would make it a P1 in its own right, not an about:addons
  cosmetic.

Log while testing: the daemon logs nothing for popup creation today, so add instrumentation at the
popup widget creation path first — otherwise "no popup was created" and "a popup was created but
painted nowhere" are indistinguishable, which is the same trap that cost time on `holdAt` and on the
icons.

## Note

A tap here also exercises the input path on a XUL chrome document, so it doubles as a check that
chrome input works end-to-end (cavekit-input-bridging R6).
