---
created: "2026-06-30"
last_edited: "2026-11-07"
---

# Cavekit: Input Bridging

## Scope
Translating webOS input commands (delivered over YAP) into DOM input events the
engine understands: pointer/click, keyboard, touch, gestures, and drag-scroll,
with correct coordinate mapping under zoom and scroll. Reference:
`render/goanna/PORT-MAP.md` (input rows), `docs/IPC-CONTRACT.md` (input commands).

## Requirements

### R1: Pointer/click input
**Description:** Click and hold commands hit the right element.
**Acceptance Criteria:**
- [x] `clickAt(x,y,numClicks)` dispatches pointer/mouse events at the content coordinate; the element at that point receives them (verified via a page that reports the event target, or by a navigation that results from clicking a link).
- [x] Click count (single/double) is preserved.
- [x] `holdAt` produces the long-press/context behavior.
- [x] Input handlers do NOT perform navigation or other engine teardown synchronously inside the YAP socket callback; teardown-capable work (hit-test/activation/click/navigation) runs in the page-lifetime-protected pump/tick context. *(review #5 H-1: a synchronous link-navigation from the clickAt callback re-entered and crash-rebooted the device.)*
- [x] Tapping a **checkbox/radio** toggles it, and a page click handler (incl. framework/React click-driven onChange, which reads `.checked` AT click time) observes the NEW state. *(2026-07-20: a synthesized click fires `onclick` but not the default action in this embedding. `GoannaRenderPage::clickAt`'s INPUT branch sets `checked` with `SetChecked` before the click (F-001, so handlers see the new state) and dispatches bubbling `input`+`change` after it; `GetDisabled` skips `<fieldset disabled>`; `indeterminate` is cleared. Device: toggles both ways, repaints, the page's onchange runs.)*
- [x] **One tap is ONE click.** The adapter's pen path (handlePenClick → the double-click timer → a "click" hit
  test → the card may claim it in eventFired → `clickAt`) is the only sender; nothing else dispatches a click for
  the same tap. *(2026-10-25, found with the emulated finger: the adapter ALSO sent a clickAt from the
  `npPalmGestureSingleTapEvent` (added 2026-07-07 when the daemon answered no hit test), so every tap was two
  clicks — a checkbox toggled twice (`cb change=2 checked=0` for one tap on jihad-input-sweep.html), about:addons'
  Disable disabled and re-enabled in one tap, Preferences opened one run in five. The gesture's clickAt removed;
  one tap = one clickAt and `cb change=1 checked=1`, Disable/Enable/Preferences act, in all three shells.)* *(suite: `card-select`, `keyboard-fields`; `docs/FINGER-TESTS.md`)*
- [x] Tapping XUL chrome content (`about:config`, `about:addons`) does NOT crash the daemon. *(2026-07-20: synthetic mouse events on XUL crashed the daemon and were skipped; since 2026-08-04 the skip is gone and the real defect is fixed (R6: 25 taps across `about:config`, daemon alive).)*
- [x] Activating XUL buttons (e.g. `about:config`'s "I promise to be careful!" `oncommand`). *(2026-08-04: clicking `warningButton` swaps the deck and the next taps resolve to `<treechildren>` in the real prefs table, so `oncommand` ran. See R6.)*
**Dependencies:** cavekit-offscreen-rendering.md (R5)

### R2: Keyboard input
**Description:** Key commands produce correct DOM keyboard events and text entry.
**Acceptance Criteria:**
- [x] `keyDown`/`keyUp` with a webOS key code, modifiers, and character produce DOM key events with the correct key and modifier state. *(2026-10-01, CE 3.1.0: with no text field focused the key fields had been forwarded raw (Right arrived as `keyCode 57507`, lower-case letters as the wrong DOM key, Ctrl and Alt swapped); they are now mapped before dispatch. Device, `render/goanna/test/jihad-input-sweep.html`: A → keydown 65 / keypress 65; a → 65 / 97; 5 → 53; Enter 13; Escape 27; arrows 37–40; PageUp 33; Delete 46; engine key scrolling works (three Down 153 px, PageDown 908). The modifier remap is from source; no modifier key was pressed on the device.)*
- [x] Typing into a focused text field inserts the expected characters (letters, digits, symbols, and non-ASCII), at the caret, with no per-keystroke navigation flash.
- [x] `insertStringAtCursor` inserts text at the caret.
- [x] A visible text caret is shown for the focused editable, and typing/editing happens at it (not append-only).
- [~] Tapping an editable raises the VKB reliably on EVERY page in a session, and focusing a field does not scroll the page content off-screen. *(Left for a real finger. The VKB follows engine-driven `editorFocused` from capture-phase focus/blur (2026-07-18, 8ebca68, Atlas IM-context port), confirmed 1→0→1 on device 2026-08-15 (`../impl/impl-device-2026-08-15.md`). T-258, 2026-10-01, CE 3.1.0, Enyo: one session over `jihad-input-echo.html`, `jihad-input-sweep.html` and `about:preferences`; each field tap logged `editorFocused=1` and `setWindowSize 768x651 … shrink=1` and the keyboard showed (`../impl/evidence-2026-10-01/vkb-session_three-pages_top-field_bottom-field_dismissed.jpg`); a field at the top of a long page did not move; a tap on plain text lowered the keyboard. Fixed then: a field at the bottom of a long page stayed under the keyboard; the daemon now scrolls a covered field's bottom edge a margin above the keyboard (`scroll 0,1758 -> 0,1985`, field at y=602 of 651) and the page returns on dismiss. The adapter does nothing itself to keep a focused field visible (`asyncCmdGetTextCaretBounds` is a no-op, T-016). Not measured: a low field on a fit-zoomed page, where the card zooms to 1.0 only after the resize. White band on a VKB toggle (T-133, 2026-08-10, from source): rows outside the adapter's rendered band are left undrawn and read white while the adapter's scroller springs the pan to the new boundary before the daemon paints at the new height; the daemon logs `uncovered pan … gapTop=N gapBot=N sinceResize=Nms dh=N`, and the candidate fix (shift the source window in `handlePaint` instead of clipping) is adapter-side and not shipped. On 2026-10-01 the dismiss logged `sinceResize=129ms dh=291`, not the single-digit case T-261 was conditional on.)*
**Dependencies:** none

### R2a: Full editable navigation and editing
**Description:** The on-screen-keyboard editing keys drive the focused `<input>`/`<textarea>` correctly.
**Acceptance Criteria:**
- [x] Backspace deletes the code point before the caret; forward-Delete the one after; both are surrogate-pair aware.
- [x] Left/Right move the caret by a full code point; Home/End jump to the field start/end; Up/Down move by line in a `<textarea>` (and degrade to Home/End in a single-line input) without landing between a surrogate pair. *(Closed 2026-10-01, CE 3.1.0. Key codes measured from a real key path: a kernel input keyboard (`render/goanna/test/uinput_kbd.c`) that LunaSysMgr hot-plugs as a Bluetooth keyboard, read at the daemon with `JIHAD_LOG_KEYS`: Up 0xE0A0, Down 0xE0A1, Left 0xE0A2, Right 0xE0A3, Home 0xE0B1, End 0xE0B2, PageUp 0xE0B3, PageDown 0xE0B4, Delete 0x7F, Backspace 0x08, Tab 0x09, Enter 0x0D (Escape never arrives). The 2026-07-15 Up/Down pair was swapped. Home/End move to the start/end of the caret's line. Re-run through `keyDown`: `abcd`, Left, Left, X → `abXcd`; Right, Y → `abXcYd`; `<textarea>` Up keeps the column; `a😀b`, Left, Left, X → `aX😀b`; Home H, End E → `HaX😀YbE`. Through the kernel keyboard on the Enyo and Mochi cards: letters, Enter, Backspace, Down, Home and End act as named. Not explained: LunaSysMgr sometimes kept Up (the card minimised) and Left/Right for itself; needs a real Bluetooth keyboard.)*
- [x] Typing or deleting while a range is selected replaces/removes the selection (e.g. an `onfocus=this.select()` search box).
- [x] Enter inserts a newline in a `<textarea>` and submits the form for a single-line `<input>`. Tab inserts a tab character in a `<textarea>` but moves focus to the next/prev text field in a single-line `<input>` (Shift+Tab goes back) — inserting a control char into a search/login value is wrong. *(2026-07-18: Enter submitted through `DOMClick()`, which crashes from native code; now `FireFormSubmit` (validate, cancelable `submit`, `form->Submit()`). Enter and tapped submit buttons both navigate; focus_test D/E pass on desktop.)* **Amended 2026-10-25:** in a `<textarea>` the PAGE sees the key first — a real keydown + keypress go in, and a page that cancels either has handled it (duckduckgo.com's search box is a textarea that submits on its own Enter handler: by finger, Enter did nothing before, now searches); otherwise the editor's line break stands (a plain textarea: "a", Enter, "b" → two lines, jihad-frame-field.html).*
- [x] Enter submission runs the form's constraint validation first: an INVALID form is not submitted and keeps focus + the keyboard so the user can correct it (never discards the edit target before knowing submission succeeded). *(2026-07 Codex F-290.)*
- [x] When a page focus/blur handler moves focus off the tapped/next field to a NON-text control (button/checkbox), the keystroke target is cleared rather than left on the old field — typing must never silently mutate a field that is no longer focused. Retargeting only follows focus to another text control. *(2026-07 Codex F-270/F-292.)*
- [x] After a programmatic value edit, a bubbling DOM `input` event is dispatched (from the guarded loop) so controlled/framework (React) inputs keep the edit instead of reverting it. *(Closed 2026-10-01, CE 3.1.0, `render/goanna/test/jihad-input-echo.html`, focus by coordinate tap, text through the normal insert path: plain `<input>` `n=1 last=[hello]`; `<textarea>` `n=1 last=[world]`; a controlled input keeps `xyz` through its re-render. The 2026-08-15 `echo: (none)` reading (cavekit-preferences-ui.md R6) was a harness fault: the card zooms to 1.0 when a field takes focus, so a second tap computed at the old zoom lands elsewhere.)*
- [x] Leaving an edited field fires `change` — on blur, before a tap elsewhere is delivered, and on Enter — and not when the value is back where it started. Without it a form that validates or saves on `change` never hears about typed text. *(Added and met 2026-10-01. Every daemon edit is a value-setter write, which the engine does not count as user input; the daemon records the value before a field's first edit and dispatches `change` from the guarded pump. Device, echo page `change` counter: typing → `c=0` while focused; tap the textarea → `c=1`; number field 12 + Enter → `c=1`; type 3, Backspace, tap away → still `c=1`; Enter with no form → `c=2`.)*
- [x] An `<input type=number>` is edited as the control, not as the anonymous text field the engine draws it with: after typing, the control's own `value` holds the number. *(Added and met 2026-10-01 (third session). The focus manager reports the inner field, the daemon adopted it as the edit target, and typing changed the picture while `value` stayed empty — a form would have submitted the old number. Focus lookups now resolve to the host control. Device: echo page, `num n=1 … value=[12]`; `about:preferences` number rows save.)*
- [x] A held Backspace accelerates to whole-word deletion after it auto-repeats for a short interval.
- [x] `<input type=number>`/`email` etc. (no text-selection API) still accept typed characters.
- [x] Focus/blur handlers that navigate or submit run in the page-lifetime-protected pump/tick context, never synchronously in the YAP key callback (same crash class as R1).
- [x] No keystroke, character, or field value is ever written to the daemon log (F-163).
**Dependencies:** cavekit-offscreen-rendering.md (RENDER_CARET — a visible caret requires the offscreen render to draw it)

### R3: Touch and gesture input
**Description:** Touch and gesture commands map to DOM touch/zoom behavior.
**Acceptance Criteria:**
- [x] `touchEvent` (with the touches payload) produces DOM touch events with matching touch points. *(2026-08-04: `touchEvent` parses every point of the adapter's `[{"x":N,"y":N,"state":S}, …]` array and dispatches one identifier per contact; `dom.w3c_touch_events.enabled` is 1 in `packaging/prefs/jihad-platform-prefs.js` (the engine's autodetect settles on off in this headless build). Inject `touch 0 400,600 460,620` → `touchstart(n=2 0@400,600 1@460,620)`.)*
- [~] A pinch gesture changes page zoom; a tap maps to a click. *(Left for a person: a real two-finger pinch on the device (T-151, BLOCKED-EXTERNAL). Neither half rides `doTouchEvent`: pinch takes the adapter's `doGestureStart/Change/End`, the zoom path device-verified 2026-07-27 through the inject channel (`../impl/zoom-fix-2026-07-27.md`, commit 8d7865c), and tap→click is R1. T-120 (2026-08-15) also enabled DOM touch delivery to content: the 28-byte `NpPalmTouchEvent` re-staged in `adapter-deps/staging/include/webkit/npapi/npapi.h` with a `sizeof == 28` build guard; `doTouchEvent` out of `#ifdef QT_FIXME` behind `JIHAD_TOUCH_EVENTS=1` in LunaSysMgr's environment (default off, never run on hardware); a daemon-side suppressor (`BrowserPageGoanna::touchSuppressesMouse`) drops the pen events of a gesture the page consumed. Stock isis and Atlas never forwarded touch. Full account and device procedure: `../impl/impl-touch-events.md`.)*
- [x] Multi-touch point count and coordinates are preserved. *(2026-08-04: `n=2` with exact coordinates through start and move; `touchend` reports `n=0` (the remaining contacts). Gecko dispatches one DOM event per changed touch, so a two-finger touchstart gives two `touchstart` events, each with `touches.length == 2`.)*
**Dependencies:** cavekit-offscreen-rendering.md (R5)

### R4: Drag and flick scrolling
**Description:** Drag commands scroll content and overflow regions.
**Acceptance Criteria:**
- [x] `dragStart`/`dragProcess`/`dragEnd` scroll the page and CSS-overflow elements.
- [x] Scrolling emits scrolled-to updates consistent with the rendered output.
**Dependencies:** cavekit-offscreen-rendering.md (R4)

### R5: Coordinate mapping under zoom/scroll
**Description:** Content coordinates resolve to the correct page location regardless of transform.
**Acceptance Criteria:**
- [x] With the page zoomed and scrolled, a click at a given content coordinate hits the element actually shown at that location. *(2026-07-27: the daemon advertised `contentZoom=mZoom` (~1.005) while rendering a near-1 fit zoom at 1×, so the adapter's click divisor was off; it now reports the effective render scale. Device: a tap on example.com's "Learn more" hit-tests the anchor and navigates (commit d4f0842; `../impl/zoom-fix-2026-07-27.md`).)*
- [x] Mapping is consistent between input and the geometry the renderer reports.
- [x] **No second division by the zoom** (owner 2026-10-08: "mojo tap targets seem inaccurate … I get duckduckgo when I click wikipedia"). The adapter's document coordinates are already CSS px; the daemon's `docToViewport` divided them by the zoom again (since 2026-08-02), so on any page shown at a zoom other than 1 a tap or long-press landed 1/zoom too far, in every shell. *(Device, 2026-10-08: Enyo, desktop Wikipedia at the 0.69 fit — a tap at view (300,400) maps to page (435,581) and hits the article's `<P>`; finger-like holds at 0.64 hit the `<A>` / `<IMG>` under the point.)*
**Dependencies:** cavekit-offscreen-rendering.md (R5)

### R6: XUL/chrome documents accept input like any other page
**Description:** Input works on XUL documents — `about:config`, `about:addons`, and every other chrome-privileged page — not just HTML content. User requirement, 2026-08-01: *"fix xul input so it works everywhere like about:config etc"*.

**Acceptance Criteria:**
- [x] The root cause of the XUL SIGSEGV is identified and named (which widget/frame path dereferences what `PuppetWidget` leaves null), not worked around. The existing `isXul` skip is REMOVED as part of this, not left in place behind a fixed path. *(2026-08-04: the `isXul` skip is gone and the crash does not reproduce. Neither defect was a null deref: (1) `PuppetWidget::GetCurrentWidgetListener` consulted only `mAttachedWidgetListener`, never falling back to `mWidgetListener` as the real widget does, so in this standalone embedding everything routed through `DispatchEvent` was dropped (mouse input hid it through `SendMouseEventToWindow`); fixed in patch 0005. (2) No `keypress` was synthesized, and XUL `<key>` elements match on keypress; `BrowserPageGoanna::keyDown` now sends it.)*
- [x] `SendMouseEvent` on a XUL document does not crash the daemon, under repeated taps across a full session. *(2026-08-04: 25 taps scattered across `about:config` — its warning deck, the prefs tree, headers and scrollbars — with the daemon alive at the end. That is the crash the 2026-07-20 skip was added for; it does not reproduce.)*
- [x] `about:config` is operable end to end: the warning button activates, the prefs list renders, a pref can be filtered, selected, and its value changed — through the adapter's normal synthesized input path. *(2026-08-04: the warning button's `oncommand` runs, the tree replaces the deck, typing filters, a double-click on the filtered row toggles the boolean. Proved from `profile/prefs.js` after a clean shutdown, both ways: `extensions.update.enabled` false→true writes the `user_pref`, true→false removes it. Tree rows are not DOM nodes, so reaching one needs the `clickoff <dx> <dy> <count> <selector>` inject command.)*
- [x] `about:addons` is operable: list items select, and enable/disable/remove buttons activate (this is what cavekit-addons-extensions.md R2 depends on).
- [x] XUL default actions actually run — matching the HTML lesson recorded in R1, where a synthesized click fires `onclick` but the engine skips the DEFAULT action in this embedding, so each native control needed hand-activation. Establish whether XUL needs the same treatment and say so explicitly rather than assuming the click suffices.
- [x] Keyboard input reaches XUL documents (text entry into `about:config`'s filter box), not only mouse. *(2026-08-04: with the `about:addons` tools menu open (`popups=1`), Escape rolls it up (`popups=0`), which is the XUL popup manager's own key handling; on content a document listener sees `down:65,press:65,up:65`. Causes and fix (patch 0005) in this requirement's first criterion.)*
- [x] No regression to HTML input: the R1/R4/R5 device evidence (checkbox/radio activation, drag scrolling, zoomed link hit-testing) still holds. *(2026-08-04, re-run on desktop after the engine change, since patch 0005 touches the path EVERY event takes: a checkbox click fires `change` with `checked=true`, a link click fires its handler, and typing into a text field still puts the text in (`VAL[Hi]`). The engine fix only ADDS a fallback where the attached listener is null, so the attached path — the one the content-process case uses — is untouched.)*
**Dependencies:** R1, cavekit-offscreen-rendering.md (the offscreen widget is the thing lacking what XUL expects)

### R7: Keyboard arbitration between a focused plugin and the card chrome

**Description:** A keystroke on this device has TWO possible consumers, and the adapter decides. The
key is always forwarded to the daemon, so the engine and any plugin see it either way; the adapter's
return value decides whether the CARD ALSO acts on it. That single boolean is the difference between
typing into a Flash game and typing into the address bar at the same time
(cavekit-addons-extensions.md R7).

The test it used — "is an HTML editor focused" — cannot describe a plugin. **A Flash game focuses no
editor**, so the flag is false for the whole session and every arrow key or WASD press also drove the
chrome. The mirror-image bug exists too: a page that auto-focuses an input on load leaves the flag
true forever, so after the user taps the address bar the adapter keeps eating every keystroke.

**Acceptance Criteria:**
- [x] Keys are only swallowed for content when the content actually has focus. *(2026-08-10 —
      `handleKeyDown`/`handleKeyUp` now gate on `mPageFocused` as well as `bEditorFocused`. Ported
      from Atlas BrowserAdapter commit `ce199bc` (Herman van Hazendonk, Apache-2.0); recorded in
      NOTICE. Fixes the reported "can't type an address after a site loaded".)*
- [~] Keys are swallowed for content when a PLUGIN has focus, even though no editor does.
      *(2026-08-10: `handleKeyDown`/`handleKeyUp` gate on `mPageFocused && (bEditorFocused || mFlashGestureLock)` (`render/adapter/BrowserAdapter.cpp`). T-107 (2026-08-15, source): the latch is set true only by a DOUBLE tap inside a plugin rect (`handlePenDoubleClick`, which also smart-zooms) or by `js_smartZoom`; `handlePenDown` only clears it, so a single tap on a Flash game gives no keyboard lock. Whether double-tap-to-engage is acceptable is a design decision. T-106: the gate is in the device's adapter by provenance (md5 `0410a348042af58a7e711b495db69833`); a `strings` grep for a member name proves nothing. Unverified: the latch's runtime value during Flash play; the shipped adapter logs nothing for it (`render/adapter/Debug.h` has `DEBUG` off). Device procedure: cavekit-addons-extensions.md R7, keyboard criterion, 2026-08-15 annotation.)*
- [~] Verified on device with an interactive plugin: keys drive the plugin and do NOT leak to the
      chrome while the plugin has focus, and DO reach the chrome again after a tap outside it.
      *(2026-08-10: the plugin half is proven: `render/goanna/test/make-key-swf.py` reacts to a keystroke in its own AVM1 (green 11768 → red 11750 across one `key 65`). Unverified: that the card does not also act on the key.)*
- [~] The virtual keyboard can be raised deliberately for a plugin that wants text, given that the *(2026-10-01, built and on the device, not exercised: no NPAPI variable lets a plugin ask for text, so `BrowserAdapter::updateFlashGestureLockStatus` calls the card's `editorFocused` handler with `true` when the plugin latch engages and with the editor's state when it clears; all three adapter impls carry it. Left for a person (T-289): a real double tap on a plugin, and whether a keyboard on every engaged plugin is acceptable; both are a person's.)*
      VKB is driven by `msgEditorFocused` and a plugin never sets it.

**Dependencies:** R2, cavekit-addons-extensions.md (R7), cavekit-ipc-contract.md
**Note:** the adapter half of this is the frozen-contract client (`render/adapter/`), so changes here
are NOT covered by `push-engine-update.sh` — they require rebuilding and installing the `.ipk`.

## Out of Scope
- Producing frames (cavekit-offscreen-rendering.md).
- IME/virtual-keyboard UI (webOS sysmgr concern; only the resulting key/text commands are handled here).
- Native XUL **popups/menus** as OS-level windows. If a popup requires a real toplevel widget the headless
  path cannot provide, an in-content fallback is acceptable — but that is a finding to record, not a
  silent omission.

## Cross-References
- See also: cavekit-offscreen-rendering.md, cavekit-navigation-events.md

## Changelog
- 2026-11-07: sweep; long evidence notes shortened, history moved verbatim to `archive/input-bridging-history.md`.
- 2026-11-06: a page zoomed in by a pinch no longer leaves the next page at that zoom: the daemon sends the adapter's "first content size after a page load" (0,0) on a new document, as the stock server does (`zoom-new-page`, all three shells).
- 2026-10-08: R5 — the daemon divided adapter coordinates by the zoom a second time (since 2026-08-02); removed, device-verified at fit zooms 0.64 and 0.69 (new `[x]`).
- 2026-10-01: target platform is now webOS CE 3.1.0. 3.0.5-era device boxes are listed in cavekit-device-build.md R4's re-verification sweep; the touch-event ABI in R3 was read from a 3.0.5 `libWebKitLuna.so`.
- 2026-06-30: Initial draft.
- 2026-07-04: R1, R2 keyDown/keyUp, R4 and R5 verified on desktop (INPUT/INPUT2/COORDMAP).
- 2026-07-15: R2 typing verified on device through the engine editor (UXP patch 0010); a visible caret needs patch 0011 (`RENDER_CARET`). R2a added.
- 2026-07-17: R2a hardened (Codex F-264..F-292).
- 2026-07-18: R2 VKB is engine-driven (Atlas IM-context port).
- 2026-07-20: R1 native checkbox/radio activation.
- 2026-07-27: R5 coordinate mapping resolved (d4f0842).
- 2026-08-01: adversarial review of T-067/T-068 (`../impl/impl-review-findings-xulinput.md`): eight findings, seven fixed in code (F-1: one tap gave two click sequences, fixed in the daemon), one (F-6) recorded.
- 2026-08-16: status sweep; no checkbox changed state.
