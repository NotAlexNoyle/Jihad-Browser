---
created: "2026-07-31"
last_edited: "2026-08-03"
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
- [x] Address entry navigates: typing a URL (or a search term) loads the corresponding page. *(`JihadUrl.normalize` + the widget's `openURL`; device-driven navigations verified.)*
- [x] Back, forward, reload, and stop are present and drive the adapter. *(Command menu; stop/reload swap with load state.)*
- [x] Load state is visible to the user (progress indication + a stop/reload affordance that reflects whether a load is in flight).
- [x] The page title and the committed URL are reflected in the UI as the engine reports them. *(The address field; the separate title ROW was removed 2026-08-03 as redundant with the card title — user request.)*
- [x] A failed load surfaces an error to the user rather than leaving a blank card. *(The error panel + retry.)*
**Dependencies:** cavekit-navigation-events.md, cavekit-ipc-contract.md (R1)

### R3: Drives the unchanged BrowserAdapter contract
**Description:** The Mojo UI talks to the engine only through the existing contract; no engine or IPC change.
**Acceptance Criteria:**
- [x] The `callBrowserAdapter(...)` method set and the `palm://com.palm.browserServer/*` URIs it uses are a subset of the set the Enyo variant uses — no additions, no renames. *(Mojo: `{goBack, goForward, reloadPage, stopLoad}`; it uses no `browserServer` URI at all. The share action launches `com.palm.app.email` through the application manager, which is an app-launch service, not an engine call.)*
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
- [x] The buttons are **circular and centred** as a group, with tight spacing. *(Device-verified 2026-08-05: 50x50 each, `btn0` at x=170 — (768-8*54)/2 — i.e. centred.)*
**Dependencies:** cavekit-ui-shell.md (behavioral baseline)

### R7: Chrome-owned settings, and where they live
**Description:** The two settings this shell owns rather than the engine — the home button's target and the start page's shortcut list — are stored by the card and are editable.
**Added 2026-08-05 (user requests: a configurable home button; customizable start-page links).**
**Acceptance Criteria:**
- [x] Both are stored in the card's own `localStorage` (`app/models/jihad-chrome-prefs.js`), the same store the history list uses, because this package registers no db8 kinds. Defaults: `https://start.duckduckgo.com/` for home, and the three shortcuts the other two shells ship.
- [x] The start page renders the stored list. **The list travels in the url FRAGMENT**, because `start.html` is a document rendered by the ENGINE and therefore cannot read the card's `localStorage` the way the Enyo and Mochi start pages (which are app chrome) read theirs. `isStartPage()` compares without the fragment, or the shell stops recognising its own start page and the url leaks into the address bar.
- [x] The page still shows its default shortcuts when opened directly with no fragment, and with script off — the markup carries them, the script only replaces them.
- [ ] They are **editable**. *(OPEN. Per user direction 2026-08-05 this variant gets no settings UI of its own — "dont put a settings icon in mojo, just depend on about:preferences and about:settings" — so editing arrives with cavekit-preferences-ui.md R5's merge, not before.)*
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
