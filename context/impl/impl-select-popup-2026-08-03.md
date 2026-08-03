---
created: "2026-08-03"
last_edited: "2026-08-03"
status: daemon+adapter+card pipeline built & desktop-proven; card popup renders over the plugin but showed EMPTY (stale-JS suspected); device was crash-looping from the day's LunaSysMgr churn — reboot + retest
---

# `<select>` dropdown -> card-native popup (Atlas msgPopupMenuShow model)

The user asked for popup support "referencing Atlas". The decisive finding: **isis/Atlas never
rendered `<select>` dropdowns in the card** — QtWebKit/WPE paint the combobox popup natively into
the plugin surface, so neither has an `onOpenSelect` app handler. Our Goanna engine can't paint it
(the XUL/native combobox is a separate display root that comes up 0x0, impl-menupopup-2026-08-02.md),
so we take Atlas's *IPC contract* (already inherited byte-identical) and render the list card-side.

## The pipeline (built this session)

1. **Daemon/engine** (`GoannaRenderPage::BuildSelectPopup`): a tap on a dropdown `<select>`
   (not multiple, size<=1) reads the options via the DOM (`nsIDOMHTMLSelectElement`/options),
   serializes `{"selected":N,"items":[{"label","enabled"},...]}`, holds the element (AddRef'd),
   and skips the normal click. Dedup guards the raw+gesture double-tap (700 ms / same element).
2. **Daemon** (`BrowserPageGoanna::emitSelectPopupIfPending`, drained in the click path): writes
   the JSON to `<state>/popup-<id>.json` (0644, R8-safe) and emits `msgPopupMenuShow(id, file)`.
3. **Adapter** (inherited): reads+unlinks the file, calls the WebView's `showPopupMenu(id, json)`.
4. **Framework BasicWebView** (already present!): `showPopupMenu` -> `doOpenSelect` -> the
   `onOpenSelect` event. **Do NOT patch showPopupMenu** — an earlier patch here shadowed the
   framework method with a broken `this.bubble()` (BasicWebView has no `bubble`), which killed it.
5. **Card** (`app/source/Browser.js`): `onOpenSelect: "showSelectPopup"` -> parse JSON ->
   `PopupSelect.setItems([{caption,value:index}])` + `openAtCenter` (deferred one turn so the
   opening tap's release can't dismiss the modal). Choice -> `callBrowserAdapter(
   "selectPopupMenuItem",[id,idx])`; dismiss sends idx -1.
6. **Daemon** (`asyncCmdPopupMenuSelect` -> `popupMenuSelect` -> `ApplySelectPopup`): sets
   `selectedIndex` + fires input/change; -1 releases the held element.

## Proven on desktop (build-popup-probe.sh, Xvfb)

A `<select>` tap injected through `$JIHAD_INJECT`: `clickAt <SELECT>` -> `popupMenuShow id=sel1
items->…/popup-sel1.json` with correct JSON (alpha/beta/gamma) -> adapter receives YAP 0x2019.
End to end minus the on-screen card list.

## On device — where it stands (unfinished)

- Daemon side VERIFIED on device: every `<select>` tap emits `popupMenuShow` with the right
  options; dedup collapses the double-tap to one; `popupMenuSelect id=… idx=…` routes back.
- The card popup DID render over the plugin (fb1 screenshot: a small popup centre-screen) — so
  **z-order is fine** (Enyo popups composite above the NPAPI surface, like the alert dialogs).
- But it rendered **EMPTY**, and the deployed Browser.js on disk did not match the card's
  behaviour → **stale WebAppMgr JS** is the strong suspect (killall LunaSysMgr wasn't reliably
  busting the in-process source cache during the rapid iteration; bumped appinfo to 1.0.4 to
  force a version reload).
- Late in the session the card began **crash-looping** (client connect -> one paint ->
  `client disconnected`, load aborts NS_BINDING_ABORTED 0x804b0002), consistent with the device
  destabilising after ~30 LunaSysMgr restarts in a day (a churn failure mode the notes warn about),
  not obviously the popup code (the crash persisted with the popup-open suppressed).

## Next step (do FIRST, on a freshly rebooted device)

A reboot clears the churn instability AND guarantees fresh JS (no cache) and fresh daemons. Then:
1. Load `file:///var/palm/jihad/enyo/selecttest.html`, tap the `<select>`.
2. Expect the centred PopupSelect to show Apple/Banana/Cherry/Durian; pick one; the page's
   `onchange` updates "Chosen:" and the daemon logs `popupMenuSelect id=… idx=<n>`.
3. If the popup is still EMPTY on genuinely-fresh JS, the bug is `PopupSelect.setItems` timing
   (canCreateItems/componentsReady) — force it by creating the MenuItems explicitly (defaultKind
   is `MenuItem`, `caption` renders) and calling `render()` before `openAtCenter`.
4. Then mochi (Enyo2 kind) + mojo (Mojo dialog) get their own idiom per the per-variant rule.
