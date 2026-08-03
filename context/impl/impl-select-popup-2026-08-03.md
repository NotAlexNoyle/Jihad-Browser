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

## The real wall (2026-08-03, after a reboot + ~10 more deploy cycles): CARD TOOLING

The popup-content problem could NOT be resolved because **two card-side dev-loop tools are
broken on this device right now**, and both are needed to debug card JS:

1. **Fresh card JS will not load.** After the reboot the card froze on an early cached build
   (~the first `onOpenSelect` version): every subsequent edit — verified byte-for-byte on disk
   by md5 — had ZERO effect on the running card. Tried, none worked: `killall LunaSysMgr`,
   `appinfo` version bump (→1.0.4), a full reboot, and a close-then-relaunch (the close script's
   `running`-query parse returns nothing for the app, so it closes nothing and LunaSysMgr
   restores the card with its cached bundle). The card kept running the OLD `showSelectPopup`
   (empty popup, no diagnostic logs), which is why every "fix" looked identical on screen.
2. **Card JS logs will not capture.** `palm-log` (and a `palm-log` Monitor) reliably showed the
   DAEMON's stdout but never surfaced the card's `enyo.log` lines this session (earlier in the
   day they did) — so the one diagnostic that would settle "0 controls built" vs "built but not
   rendered" vs "exception" (`[JSEL] built controls=N … listNode=…`) never came back.

So the empty popup is UNDIAGNOSED between two hypotheses that need a working loop to separate:
(a) the card is simply still running stale JS and the current code is fine; (b) adding controls
to an already-created Popup + `render()` + `openAtCenter()` genuinely doesn't paint content in
this LunaCE/WebAppMgr embedding. The identical empty box across BOTH `PopupSelect.setItems` AND a
plain `Popup` + explicit `Button`s is equally explained by (a) — the card never ran either.

## Next step (do FIRST — restore the card dev loop, THEN debug the popup)

1. **Get fresh card JS to actually load.** Reboot, and BEFORE launching, confirm no jihad card
   is restorable — fix the close path first: capture the real `com.palm.applicationManager/running`
   JSON shape (this session's `.jihad-running.sh` parse produced nothing) and close by the real
   processId, or as a last resort `palm-install -r` + reinstall the `.ipk` (heavier; palm-install
   has hung before). Prove freshness with a one-line boot marker in the log, not by disk md5.
2. **Get card logs.** Confirm `enyo.log` reaches `palm-log` again on a clean boot (it did at
   ~08:11 today) before trusting any card diagnostic.
3. Then the current `Browser.js` (plain `Popup` + a `Button` per option, built in `showSelectPopup`)
   either just works, or the `[JSEL]` diagnostic (still in git history) tells you which of (a)/(b)
   it is in one tap.
4. Then mochi (Enyo2 kind) + mojo (Mojo dialog) get their own idiom per the per-variant rule.

## What IS solid (committed, not blocked)

The whole daemon/engine/adapter half is done and device-verified: a `<select>` tap serializes the
real options, emits `msgPopupMenuShow`, and applies the returned index (input/change) — plus the
`ClickAt` fix so a dropdown `<select>` never falls through to a normal click (the "box around
Apple" focus ring). Only the card's on-screen list rendering is unverified, gated on the tooling
above.
