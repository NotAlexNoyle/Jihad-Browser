---
created: "2026-08-03"
last_edited: "2026-08-03"
status: RESOLVED on device (popup renders all options; Opus-hardened apply guard). Open — one physical tap to confirm the reply, mochi/mojo card handlers, optgroup header rows
---

# `<select>` dropdown -> card popup — RESOLVED (the framework owns the card side)

## The real root cause (found 2026-08-03 after the dev loop was restored)

**The card never needed any app-side popup code.** The isis-era stack handles the whole
card side by itself:

1. Adapter `msgPopupMenuShow` reads the JSON file, unlinks it, and invokes `showPopupMenu`
   on the plugin node's `eventListener` (= the framework `BasicWebView` instance).
2. `BasicWebView.showPopupMenu -> doOpenSelect`, and the **`enyo.WebView` WRAPPER consumes
   that event itself**: `showSelect -> createSelectPopup -> PopupList`, reply via
   `callBrowserAdapter("selectPopupMenuItem", [id, idx])` (idx = row, or -1 on dismiss).
   The wrapper NEVER re-publishes `onOpenSelect` to the app — the app's handler (and the
   custom `Popup`/`Button` list built last session) was unreachable dead code, deleted.
3. `createSelectPopup` parses **`items[].text` / `items[].isEnabled`** (+ top-level
   `selectedIdx`, unused) — the exact isis `BrowserComboBox.cpp` shape. Our daemon wrote
   `label`/`enabled`/`selected`, so the framework rendered a list of **undefined captions:
   the "empty popup"**. Fixed: daemon emits the isis shape byte-for-byte
   (`text`/`isEnabled`/`isSeparator:false`/`isLabel:false` + `selectedIdx`).

Device-verified (commit e3de7d8a): injected tap on the test `<select>` -> `popupMenuShow`
-> card popup renders **alpha/beta/gamma/delta with captions** (fb1, with a fresh daemon
paint in the log as the liveness proof).

## Opus adversarial review (FIX-FIRST) — folded into the same commit

- **#1 (blocker)** The framework `PopupList` never reads its `disabled` flag — every row is
  tappable — so `ApplySelectPopup` is the ONLY enforcement point: it now re-reads the
  options collection and refuses disabled options (**#2** incl. an enclosing disabled
  `<optgroup>`, walked to the `<select>`), out-of-range indexes (**#4** — page JS can
  rewrite options while the popup is up), and no-op picks (**#3** — re-selecting the
  current option fires no `input`/`change`, as in real browsers).
- **#6** A short/empty popup-json write now fails CLOSED (unlink + drop the message):
  forwarding it would throw in `createSelectPopup` after `showSpinner()` and leave the
  modal scrim stuck over the card until relaunch.
- **#7** The popup id sequence is process-global (isis `idSeq` parity): the framework
  caches popups BY ID and a daemon page rebuild restarting at `sel1` would reopen a stale
  cached list.

## End-to-end confirmed + anchored (2026-08-03, later the same day)

- **User-confirmed pick**: tapping "gamma" applied the selection and fired the page's
  onchange (the test banner updated). The Enyo select popup is fully working.
- **Anchoring fixed** (commit 322f26bd): the popup was opening dead-centre because the
  stock `_selectRect` path can never run (BasicWebView publishes no onClick). The daemon
  now appends an ADDITIVE `"rect"` key (card px; inverse of `docToViewport`, zoom/pan
  aware) to the popup JSON, and delegating patches on `enyo.WebView.showSelect`/
  `openSelect` (JihadEngineOverride) open the list flush under the box, centred, clamped,
  flipping above when it would run off screen. fb1-verified.

## Still open

2. **Mochi + Mojo card handlers** — both variants' custom WebViews have NO `showPopupMenu`
   callback (the NPAPI bridge drops it silently). Each needs its own small popup + the
   `selectPopupMenuItem` reply, parsing the isis shape (`app-mochi/source/JihadWebView.js`
   "adapter -> app callbacks" block; mojo `app-mojo/app/assistants/main-assistant.js`).
3. **Optgroup header rows** (review #5): options inside `<optgroup>` render as a flat list
   (isis emitted `isLabel` group rows). Needs a daemon-side index remap between list rows
   and option indexes — do NOT add rows without remapping the reply.
4. Cosmetic (review): option text truncates at 200 UTF-8 bytes (≈66 CJK chars, no
   ellipsis); the popup always opens centered (`_selectRect` is never set — stock-framework
   behaviour, not a daemon bug); `selectedIdx` has no framework consumer (kept for contract
   fidelity).

## Tooling this rode on (see impl-NEXT-AGENT-START-HERE.md)

The card dev loop is `build/webos-oe/push-card-js.sh` (stamp-proven reloads). Key traps
now encoded there: `novacom run` DROPS late output at host stdin EOF (`sleep 4 |` every
run that must reply — this masqueraded as "luna-send blackouts"/"enyo.log dead" all
session); the WebAppMgr in-process JS cache really serves stale builds (LunaSysMgr restart
per cycle, default); launch the card on a page via launch params
(`{"id":…,"params":{"url":…}}`) or the adapter/engine never connects.
