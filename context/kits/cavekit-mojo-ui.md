---
created: "2026-07-31"
last_edited: "2026-10-06"
---

# Cavekit: Mojo UI Variant

## Scope
A third browser front-end (directory `app-mojo/`) implemented on **Mojo**, the original
(pre-Enyo) webOS application framework that ships with webOS 3.0.5 at
`/usr/palm/frameworks/mojo`. It is functionally a browser in its own right — not a
demonstration — and drives the **same** BrowserAdapter contract as the Enyo-1.0 shell
(cavekit-ui-shell.md) and the Enyo-2/Mochi shell (cavekit-mochi-ui.md).

It ships as its own `.ipk` (`net.riverstonerelay.jihad-browser-mojo`) and, per
cavekit-device-build.md **R7**, is a fully independent package: its own NPAPI MIME type,
adapter shim, adapter impl, YAP service name, socket, upstart job, and daemon process.
Installing or removing it has no effect on the Enyo or Mochi variants.

This domain is a presentation layer only — the engine, the daemon, and the YAP/Luna
contract are unchanged.

Reference: `app-mojo/README.md`, `docs/IPC-CONTRACT.md`, the Mojo framework docs
(`webos://knowledge/mojo`), the Enyo-1.0 shell in `app/` as the behavioral baseline.

## Requirements

### R1: Separate, coexisting Mojo application package
**Description:** The Mojo UI is packaged as its own webOS app, installable alongside both other variants.
**Acceptance Criteria:**
- [x] `app-mojo/appinfo.json` declares a distinct app id (`net.riverstonerelay.jihad-browser-mojo`) and the Jihad Browser title/icon set.
- [x] Installing it does not collide with, overwrite, or replace either the Enyo (`net.riverstonerelay.jihad-browser`) or Mochi (`…jihad-browser-mochi`) variant; all three can be installed at once and all three launch. *(Device-verified 2026-08-03: all three installed, cold boot auto-starts three daemons on their own sockets, `device-independence-test.sh check` 24/24.)*
- [x] Removing it leaves the other two fully functional (launch + load a page). *(Asserted by `device-independence-test.sh remove` + `check`, the R7 harness.)*
**Dependencies:** cavekit-device-build.md (R7)

### R2: Working browser front-end on Mojo
**Description:** The Mojo variant is a usable browser, not a scaffold.
**Acceptance Criteria:**
- [x] A Mojo scene hosts a web-render surface bound to this variant's NPAPI MIME type and displays a real rendered page. *(Device 2026-08-03: card paints example.com and local pages through its own daemon.)*
- [x] Address entry navigates: typing a URL (or a search term) loads the corresponding page. *(`JihadUrl.normalize` + the widget's `openURL`; device-driven navigations verified.)* *(2026-10-01 (third session): a launch that carries a link did NOT open it — `stage-assistant.js` read only `params.url`, and the system hands a browser a link as `{target: url}` (the key the other two shells read). Measured: launched with a target, the card opened its start page. Fixed; after it `openUrl file:///tmp/jihad-shell.html` on the launch.)*
- [x] Back, forward, reload, and stop are present and drive the adapter. *(Command menu; stop/reload swap with load state.)*
- [x] Load state is visible to the user (progress indication + a stop/reload affordance that reflects whether a load is in flight).
- [x] The page title and the committed URL are reflected in the UI as the engine reports them. *(The address field; the separate title ROW was removed 2026-08-03 as redundant with the card title — user request.)*
- [x] A failed load surfaces an error to the user rather than leaving a blank card. *(The error panel + retry.)* *(2026-10-01 (third session), CE: a refused connection shows the panel ("Can't load this page / Try Again") after `loaderr failed=1 code=0x804b000d` — and BEFORE it the engine's own alert ("The connection was refused when attempting to contact …"), which the daemon waits on. Two reports of one failure; the same in the other two shells. Recorded, not changed.)*
- [x] A page with NO viewport tag fills the card like any other, instead of collapsing. *(Added and met 2026-10-01 (third session). Found by accident: on this card every page without `<meta name=viewport>` drew as a 76x99 px thumbnail in the top-left corner — the Mojo WebView sizes its node to page × scale and was being handed an "initial scale" of 0.1 for such pages, over and over (cavekit-offscreen-rendering.md R4), with a 0x0 node in between when the keyboard came up mid-load. Earlier device checks used pages that declare a viewport, which is why it was never seen. Fixed in the daemon (no viewport message for a page without the tag) and the adapter (a 0x0 window is not a size). Device: the 980-wide test page now lays out at the card width at zoom 1.0, stable over 18 s, screenshot `../impl/evidence-2026-10-01/mojo-no-viewport-page-after-fix.jpg`; a device-width page and `about:preferences` are unchanged.)*
**Dependencies:** cavekit-navigation-events.md, cavekit-ipc-contract.md (R1)

### R3: Drives the unchanged BrowserAdapter contract
**Description:** The Mojo UI talks to the engine only through the existing contract; no engine or IPC change.
**Acceptance Criteria:**
- [x] The `callBrowserAdapter(...)` method set and the `palm://com.palm.browserServer/*` URIs it uses are a subset of the set the Enyo variant uses — no additions, no renames. *(2026-10-05: Mojo `{copy, cut, findInPage, goBack, goForward, insertStringAtCursor, pageFocused, reloadPage, selectAll, stopLoad}` — every one also called by the Enyo shell; the five new ones arrived with the long-press menu and the padlock. Mojo: `{goBack, goForward, reloadPage, stopLoad}` was the 2026-08 set; it uses no `browserServer` URI at all. The share action launches `com.palm.app.email` through the application manager, which is an app-launch service, not an engine call.)*
- [x] The render surface is created with THIS variant's MIME type only; it never loads another variant's adapter or the stock `application/x-palm-browser`. *(`app/models/jihad-engine-override.js`; `verifyEngineRouting()` asserts it at runtime and surfaces a visible error if it ever regressed.)*
- [x] No Goanna/UXP-specific identifiers appear in `app-mojo/`. *(The start page's engine line is user-facing branding text, not an API identifier.)*
**Dependencies:** cavekit-ipc-contract.md (R1, R5), cavekit-ui-shell.md (R2)

### R4: Built with Mojo framework idiom; layout fits both TouchPad models
**Description:** The UI is a real Mojo app, using the framework's own structures.
**Acceptance Criteria:**
- [x] The app is composed from Mojo's own constructs (stage + scene assistants, `sources.json` manifest, Mojo widgets and view templates) rather than framework-agnostic markup with hand-rolled behavior. *(Scenes `main` + `history`; TextField/ProgressBar/Button/List widgets; `Mojo.Menu.commandMenu`.)*
- [x] It runs against the device's system Mojo framework rather than bundling a copy of it.
- [x] Layout is usable on the TouchPad (Topaz) screen. *(Topaz verified 2026-08-03 after the chrome overflow fix below. **TouchPad Go moved out of this criterion 2026-08-04** — every Opal gate now lives in cavekit-device-build.md R6 as ONE criterion, rather than the same missing hardware blocking five criteria across four kits.)*
**Dependencies:** cavekit-device-build.md (R3, R6)

### R5: Licensing and attribution
**Description:** The Mojo variant respects all licenses.
**Acceptance Criteria:**
- [x] New `app-mojo/` source files carry Apache-2.0 headers.
- [x] The package ships the composite `LICENSE` + `NOTICE` (it bundles the MPL-2.0 engine like the other variants).
**Dependencies:** cavekit-licensing-branding.md (R1, R2)

### R6: Card chrome parity actions
**Description:** The bottom command menu offers the actions a user expects of a browser card, not just page navigation.
**Added 2026-08-03 (user request).**
**Acceptance Criteria:**
- [x] Back, forward, and reload-or-stop are joined by **new card**, **history**, and **share**, all as command-menu buttons.
- [x] New card opens a genuine second card (a new Mojo *stage*), not another scene on this card's stack.
- [x] Share hands the current page to the mail app using the same launch id and parameters as the Enyo shell's share dialog; it is disabled on the start page.
- [x] History is per-variant and self-contained: this package registers **no db8 kinds**, so it keeps its own capped, de-duplicated list in the card's own storage (`app/models/jihad-history.js`) and shows it in a pushed scene that navigates on tap and can clear the list. It writes nothing outside the app's storage and sees no other variant's history.
- [x] Icons the framework has no glyph for are app-shipped **32x64 two-frame sprites** (soft-white normal frame, opaque pressed frame) referenced by a menu item's `iconPath` — a 32x32 image renders as a cropped, off-centre glyph.
- [x] **Find** is in the row, as an icon (a magnifying glass), not a text item. *(Added 2026-08-05 on user report "there is no find UI", then "make find a magnifying glass instead of text": a text item is the one thing in the row that cannot be circular, and the row reads as a set of round controls.)*
- [x] **Home** sits immediately right of forward and opens the configured home page. It is never disabled — unlike back/forward it does not depend on page history. *(Added 2026-08-05 by user request. Device-verified.)*
- [x] The row **scrolls horizontally** rather than overflowing, so it can hold as many controls as the Enyo and Mochi shells. *(Device-verified 2026-08-05: 8 buttons, `client=768 scroll=768 overflowX=auto`.)*
- [~] **Long-press menu and padlock** (cavekit-ui-shell.md R7, R8): every hold goes to the engine (Mojo's web view claims none), and the card the user is in opens a `popupSubmenu` with the page's or the field's commands; the padlock is the address field's background on an https page, its tap answered with `showAlertDialog`. *(2026-10-06: driven from the Mojo test hook (`render/goanna/test/mojo-selftest.js`, `card-selftest.sh mojo`) on the device: a hold on page text opens Copy / Select All / Reload / Find on Page and Copy puts the word on the clipboard; a hold in a field opens Paste / Select All, Paste inserts the clipboard, Select All then adds Copy / Cut, and Cut leaves the text on the clipboard, the field empty and the engine saying "Cut."; the padlock tap on Wikipedia brings the Certificate alert. Found and fixed on the way: Copy and Cut had never reached the engine on Mojo — the WebView widget's `copy(callback)` hands an absent callback to the adapter as `undefined`, which the adapter refuses ("Bad arguments"), and the widget wraps no `cut` and no `pageFocused` at all. The shell now always passes a function to copy, and a method the widget does not wrap is called on the plugin element itself (R3).)* The finger on a link, and the hold's feel, are the owner's. [human at the device]
- [x] The buttons are **circular and centred** as a group, with tight spacing. *(Device-verified 2026-08-05: 50x50 each, `btn0` at x=170 — (768-8*54)/2 — i.e. centred.)*
- [x] **The other shells' upper-right menu, on the circles** (owner 2026-10-06: "the upper right menu has the same options on all 3 variants (see atlas for reference)", then "mojo should have no top toolbar icons next to url. bottom circular buttons only"). The **Share** circle opens Add Bookmark · Share Link (Email · Text Message · Copy Address) · Add to Launcher; the **Bookmarks** circle (it replaces the History circle) opens Bookmarks · History · Downloads; New Card stays a circle. Both are the framework's own command-menu `submenu` items, so the menu pops at the tapped circle. Bookmarks are the card's own capped list (`app/models/jihad-bookmarks.js`, no db8 kinds); Downloads lists what the engine saved (`app/models/jihad-downloads.js`, fed by the adapter's download callbacks). The address row carries nothing but the field. *(Device 2026-10-06, Mojo hook `Cshare`/`Clibrary` + `m`/`mN`: both menus, Add Bookmark → listed in Bookmarks, History and Downloads scenes.)*
- [x] **Reload / Stop is inside the address field's right end**, not a circle (owner 2026-10-06: "put the mojo refresh icon embedded in the right of the urlbar instead of the bottom as a circular button"): the ink glyph (the field is white whatever the theme), Stop while loading. *(Device 2026-10-06, hook `J`: refresh → tap → stop while loading → refresh.)*
- [x] **App menu = the other shells'**: Find on Page · Settings · Print. Print is this shell's own scene over the daemon's printing (mDNS / typed address, the same Luna methods as Enyo and Mochi). *(Device 2026-10-06: real sysmgr relaunch with `open-app-menu` shows the three; Print lists the network's printers. A print to paper was not made.)*
- [x] **Pinch zooms** (owner 2026-10-06: "pinch to zoom is broken. the page turns grey and I get the copy/select all/reload/find on page menu instead"). Two adapter defects, both on the path only the Mojo WebView widget uses (its `gestureStart/Change/End` JS calls): the long-press timer the first finger armed was never cancelled, so the hold fired mid-pinch; and the adapter zoomed and scrolled itself on top of the widget's own pinch (which scales the node and scrolls the scene), so the visible part of the node fell on rows never painted. Now a widget-driven gesture cancels the hold and leaves zoom to the widget, the engine following the node's width as it already did. *(Device 2026-10-06, hook `Z` — a finger-down on the plugin then the gesture through the widget's own listeners, with a held-still control that does raise the menu: no hold, engine zoom tracks the widget 1.01 → 1.20, page drawn; pinch back out drawn. A finger test by the owner is still the final word.)*
**Dependencies:** cavekit-ui-shell.md (behavioral baseline)

### R7: Chrome-owned settings, and where they live
**Description:** The two settings this shell owns rather than the engine — the home button's target and the start page's shortcut list — are stored by the card and are editable.
**Added 2026-08-05 (user requests: a configurable home button; customizable start-page links).**
**Acceptance Criteria:**
- [x] Both are stored in the card's own `localStorage` (`app/models/jihad-chrome-prefs.js`), the same store the history list uses, because this package registers no db8 kinds. Defaults: `https://start.duckduckgo.com/` for home, and the three shortcuts the other two shells ship.
- [x] The start page renders the stored list. **The list travels in the url FRAGMENT**, because `start.html` is a document rendered by the ENGINE and therefore cannot read the card's `localStorage` the way the Enyo and Mochi start pages (which are app chrome) read theirs. `isStartPage()` compares without the fragment, or the shell stops recognising its own start page and the url leaks into the address bar.
- [x] The page still shows its default shortcuts when opened directly with no fragment, and with script off — the markup carries them, the script only replaces them.
- [~] They are **editable**. *(PARTIAL 2026-08-10, T-112 — was `[ ]`. The **page→card leg now exists**: `app/models/jihad-chrome-prefs.js` gained `isSettingsUrl()` + `adoptFromUrl()` ported from the Enyo shell's `ChromePrefs`, with the SAME path-only gate and the SAME `https?` / `about:(preferences|settings|jihad|isis|blank)` allowlist, and `main-assistant.js` calls `adoptChromePrefs()` from BOTH committed-url handlers — so an edit made on `about:preferences` is adopted by this card. Three things are still open or worth knowing:* *(**2026-10-01 (third session) — DEVICE-VERIFIED on CE 3.1.0, page → card, and it found a defect that undid edits; `[~]` only because opening the page from the card's menu and typing are a finger's.** On the Mojo card: `about:preferences`, the home-url field and the first shortcut's name each edited by coordinate tap + inserted text + Enter ("Home page saved.", "Start page links saved."), the page published `{"homeUrl":"…/t254","startLinks":[{"title":"DuckDuckGoX254",…` in its fragment, and one Back returned to the start page, which the card re-opened as `start.html#links=[{"title":"DuckDuckGoX254",…` — the deferred redraw of note (c) working as designed. A card relaunch still showed the edited list, and restoring it on the page restored it in the card. **The defect:** the page published each edit by assigning `location.hash`, which pushes a history entry per edit, so Back stepped to the PREVIOUS fragment, the card was handed that older payload as a committed url and adopted it — the latest edit silently reverted in the card while the page's own store kept it. Fixed with `location.replace`; after it one Back leaves the page. The same edit was checked on the Enyo and Mochi cards, whose start pages showed the new shortcut name after a relaunch (`../impl/evidence-2026-10-01/start-page-adopts-settings-edit_enyo_mochi.jpg`).)*
  *(a) **The card→page leg is NOW PRESENT (updated 2026-08-15).** ~~There is no `settingsUrl()` here, because per the 2026-08-05 user direction this variant gets no settings entry~~ — the user WITHDREW that direction on 2026-08-15 (T-156) and chose to add the entry. So `jihad-chrome-prefs.js` gained `settingsUrl()` (builds `about:preferences#chrome=<{homeUrl,startLinks}>` from THIS card's home + shortcuts), and `main-assistant.js` gained an app-menu "Settings" item (`handleCommand` `jihad-settings` → `openUrl(settingsUrl())`). The page now opens on this card's values, and the full card↔page↔card round-trip (settingsUrl out, adoptFromUrl back) is closed. Device-verified: the Mojo card launches clean with the app menu; opens via `openUrl` with zero new adapter calls. See cavekit-preferences-ui.md R4, now `[x]` for all three variants.)*
  *(b) **NOT DEVICE-VERIFIED — no device was available.** What WAS verified: the gate's premise, read in source (`render/goanna/components/jihadAboutPreferences.js` `getURIFlags` returns only `ALLOW_SCRIPT` and deliberately withholds `URI_SAFE_FOR_UNTRUSTED_CONTENT`, so content cannot navigate into `about:preferences` — which is what makes a PATH test a gate and a SCHEME test not one); and the gate + allowlist exercised off-device against a `localStorage` stub — `about:blank#chrome=`, `about:blankfoo#chrome=`, an `https:` page and a case-shifted `ABOUT:PREFERENCES` all rejected; `about:settings` accepted; a payload carrying `javascript:`/`data:`/`file:` values passes the gate and is stripped to nothing by the allowlist; a pre-poisoned store is neutralised on READ; the 12-row cap holds; a repeat adopt returns false.*
  *(c) **The start-page redraw is DEFERRED here, not immediate, and the straight port of it would have been dead code.** Enyo/Mochi rebuild their start page in place because it is card chrome; ours is a DOCUMENT whose list travels in the url fragment, so a redraw is a NAVIGATION — and "redraw it if it is showing" can never fire, because the url that passes the adopt gate is `about:preferences`. A one-shot flag spends the redraw the next time the start page commits, which is the case that actually breaks (Back onto the start page's own history entry with its stale `#links=` fragment). Costs one extra history entry; a SECOND open Mojo card does not learn about the edit until it renavigates to its own start page.)*
- [x] Two defects found in review 2026-08-06, both fixed and device-verified the same day: the error-panel retry now re-opens `startPageUrl()` rather than the bare `this.startUrl`, so a retry no longer silently drops the user's shortcut list back to the built-in defaults; and `logCommandRowGeometry` is behind `LOG_COMMAND_ROW_GEOMETRY` (default false) instead of running on every card launch at ERROR level. *(Verified: 0 card errors and 0 `JIHAD-CMDGEOM` lines after a launch, where it previously logged on every one. The probe is kept, off, because this row's layout is CSS-overridden and has broken twice.)*
**Dependencies:** cavekit-preferences-ui.md (R5), cavekit-ui-shell.md

## Out of Scope
- Any change to the rendering engine, BrowserServer, or the YAP/Luna contract.
- Feature parity with the Enyo shell's long tail (bookmarks/downloads/preferences views).
  R2 defines the bar: a working browser; R6 adds the chrome actions the user asked for.
  Anything beyond that is YAGNI until asked for.
- Replacing either other variant — all three ship.

## Platform constraints learned on device
- **The card WebKit (~534.x) ignores unprefixed `box-sizing` and modern flexbox.** Card
  chrome CSS must use `-webkit-box-sizing` (and `-webkit-box` flexbox). Unprefixed
  declarations parse away silently: the toolbar measured **784 px on a 768 px screen**
  because its padding was added to `width:100%` (device-measured 2026-08-03). This applies
  to card chrome only — pages rendered by our own Goanna engine are modern.
- **`Mojo.stringifyJSON` does not exist in this framework build** — `Mojo.parseJSON` does. Calling
  it throws `Object #<an Object> has no method 'stringifyJSON'`, which is a silent data-loss bug
  wherever it is used to write: `app/models/jihad-history.js` used it, so **Mojo history never
  persisted at all** until this surfaced (2026-08-05) from an exception in the main scene's
  `setup()`. Use native `JSON.stringify`.
- **`Mojo.Log.warn` never reaches `/var/log/messages`** — it is below the card's log threshold.
  `Mojo.Log.error` does. A diagnostic logged at warn level looks exactly like a probe that did
  not run; a command-row geometry probe read as "produces no output" for weeks because of this.
- **The framework's command menu positions its items absolutely OVER a `.palm-menu-fade` block
  that comes first among the row's children.** Making the items `static` (which is what lets them
  flow left-to-right and scroll) drops them BELOW that block — a full row height down, outside the
  bar's own 50 px box, where `overflow-y: hidden` clips them and the bar renders as an empty
  strip. Take the fade out of flow. Measured 2026-08-05: row at y=606, every button at y=656.
- **Mojo's `TextField` is TWO nodes that swap, not one.** The real `<input>` ("-write") is shown
  while the field has focus; a read-only `<div>` ("-read") takes over the moment it blurs. Only
  the input carries the framework's white field background, so field chrome placed on the input
  disappears when the keyboard is dismissed — the url sat as bare text on the toolbar. Put the
  chrome on the CONTAINER, which is present in both states. Proven 2026-08-05 by dumping
  `outerHTML` in both states.
- `<select>` dropdowns need **no app code in this variant**: the system framework's WebView
  widget already implements `showPopupMenu -> popupSubmenu -> selectPopupMenuItem`. It only
  ever needed the daemon to emit the isis JSON shape (cavekit-addons-extensions.md,
  `context/impl/impl-select-popup-2026-08-03.md`).

## Cross-References
- See also: cavekit-ui-shell.md (Enyo 1.0 variant), cavekit-mochi-ui.md (Enyo 2/Mochi variant),
  cavekit-ipc-contract.md, cavekit-navigation-events.md, cavekit-device-build.md (R7 independence,
  R8 install footprint), cavekit-licensing-branding.md

## Changelog
- 2026-10-06 (night): R6 — the upper-right menu's options on the bottom circles (Share ▸, Bookmarks ▸), reload/stop in the address field, the app menu Find · Settings · Print, and pinch fixed (hold cancelled; the widget owns the zoom).
- 2026-10-06: R6 gains the long-press menu and the padlock (code in, finger needed); R3's method set updated.
- 2026-10-06 (later): R6's menu and padlock device-verified from a Mojo test hook; Copy/Cut/pageFocused were found never to reach the engine on Mojo and fixed (the widget's `copy(undefined)`, and its missing `cut`/`pageFocused` wrappers — now called on the plugin element; the method-name set is unchanged).
- 2026-10-01: **Target platform is now webOS CE 3.1.0.** The Scope line's "ships with webOS 3.0.5"
  describes the framework this variant was verified against; whether CE 3.1.0's
  `/usr/palm/frameworks/mojo` is the same build is part of cavekit-device-build.md R4's
  re-verification sweep, where every device-verified box in this kit is listed.
- 2026-08-05: R6 extended (find as a magnifying-glass icon, home right of forward, the row
  scrolling, and the buttons circular + centred) and **R7 added** for the two chrome-owned
  settings. Four platform constraints recorded, all found by measuring on device rather than
  by reading CSS: the missing `Mojo.stringifyJSON` (which had silently disabled history
  persistence), `Mojo.Log.warn` being below the log threshold, the `.palm-menu-fade` block that
  clips statically-positioned menu items, and the TextField's two-node focus swap. Every one of
  these was a bug the user reported as a visual symptom ("the buttons are not visible", "the url
  bar looks incorrect") and none was diagnosable from the source alone.
- 2026-08-03: Reconciled against the device. R1–R3 and R4's first two ACs marked met (all three
  variants live on hardware; the R7 harness passes 24/24); R4's Opal AC is the only one left,
  and it is hardware-gated. Added **R6** (command-menu parity actions: new card / history /
  share) for the chrome the user asked for, and recorded the two platform constraints found
  while building it — the card WebKit's `-webkit-` prefix requirement and the fact that Mojo
  needs no app-side `<select>` popup code.
- 2026-07-31: Initial draft. Created when the user required all three front-ends (Enyo, Mochi, Mojo)
  to work standalone; `app-mojo/` had been a documented skeleton tracked only as one acceptance
  criterion under cavekit-device-build.md R3.
