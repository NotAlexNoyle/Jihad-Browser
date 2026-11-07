# cavekit-mojo-ui.md history, moved out of the kit by the 2026-11-07 sweep; verbatim.

## R2, criterion 2 (removed 2026-11-07)

*(2026-10-01 (third session): a launch that carries a link did NOT open it — `stage-assistant.js` read only `params.url`, and the system hands a browser a link as `{target: url}` (the key the other two shells read). Measured: launched with a target, the card opened its start page. Fixed; after it `openUrl file:///tmp/jihad-shell.html` on the launch.)*

## R2, criterion 7 (removed 2026-11-07)

*(Added and met 2026-10-01 (third session). Found by accident: on this card every page without `<meta name=viewport>` drew as a 76x99 px thumbnail in the top-left corner — the Mojo WebView sizes its node to page × scale and was being handed an "initial scale" of 0.1 for such pages, over and over (cavekit-offscreen-rendering.md R4), with a 0x0 node in between when the keyboard came up mid-load. Earlier device checks used pages that declare a viewport, which is why it was never seen. Fixed in the daemon (no viewport message for a page without the tag) and the adapter (a 0x0 window is not a size). Device: the 980-wide test page now lays out at the card width at zoom 1.0, stable over 18 s, screenshot `../impl/evidence-2026-10-01/mojo-no-viewport-page-after-fix.jpg`; a device-width page and `about:preferences` are unchanged.)*

## R3, criterion 1 (removed 2026-11-07)

*(2026-10-05: Mojo `{copy, cut, findInPage, goBack, goForward, insertStringAtCursor, pageFocused, reloadPage, selectAll, stopLoad}` — every one also called by the Enyo shell; the five new ones arrived with the long-press menu and the padlock. Mojo: `{goBack, goForward, reloadPage, stopLoad}` was the 2026-08 set; it uses no `browserServer` URI at all. The share action launches `com.palm.app.email` through the application manager, which is an app-launch service, not an engine call.)*

## R4, criterion 3 (removed 2026-11-07)

*(Topaz verified 2026-08-03 after the chrome overflow fix below. **TouchPad Go moved out of this criterion 2026-08-04** — every Opal gate now lives in cavekit-device-build.md R6 as ONE criterion, rather than the same missing hardware blocking five criteria across four kits.)*

## R6, criterion 9 (long-press menu and padlock) (removed 2026-11-07)

*(2026-10-06: driven from the Mojo test hook (`render/goanna/test/mojo-selftest.js`, `card-selftest.sh mojo`) on the device: a hold on page text opens Copy / Select All / Reload / Find on Page and Copy puts the word on the clipboard; a hold in a field opens Paste / Select All, Paste inserts the clipboard, Select All then adds Copy / Cut, and Cut leaves the text on the clipboard, the field empty and the engine saying "Cut."; the padlock tap on Wikipedia brings the Certificate alert. Found and fixed on the way: Copy and Cut had never reached the engine on Mojo — the WebView widget's `copy(callback)` hands an absent callback to the adapter as `undefined`, which the adapter refuses ("Bad arguments"), and the widget wraps no `cut` and no `pageFocused` at all. The shell now always passes a function to copy, and a method the widget does not wrap is called on the plugin element itself (R3).)*

*(2026-10-24, the emulated finger (T-525): a held link got the text commands only — the Mojo widget has no `eventFired`, so the adapter's hit test (isLink, linkUrl, isImage, imageUrl) never reached the shell. Now `wa.eventFired` on the widget assistant claims link and image holds and `showHoldMenu` offers Open in New Card / Share Link / Copy URL (image: Open Image in New Card / Copy Image URL); Open in New Card verified. Text, field and other holds still go to the engine.)*

## R6, criterion 15 (pinch zooms) (removed 2026-11-07)

— REOPENED 2026-10-07 (superseded by the screen-sized view above): pinch in then out on en.m.wikipedia.org leaves the card grey (the daemon paints good frames at the final zoom; the card does not show them). The hold no longer fires mid-pinch. Root cause is structural: the Mojo WebView sizes the plugin node to the whole page and pans by scrolling the scene, while this adapter is a viewport-sized plugin that pans and zooms itself; pinch is where the two models collide. Proposed: keep the node viewport-sized in this shell and let the adapter own pan and zoom, as in the Enyo and Mochi shells. Earlier note (owner 2026-10-06: "pinch to zoom is broken. the page turns grey and I get the copy/select all/reload/find on page menu instead"). Two adapter defects, both on the path only the Mojo WebView widget uses (its `gestureStart/Change/End` JS calls): the long-press timer the first finger armed was never cancelled, so the hold fired mid-pinch; and the adapter zoomed and scrolled itself on top of the widget's own pinch (which scales the node and scrolls the scene), so the visible part of the node fell on rows never painted. Now a widget-driven gesture cancels the hold and leaves zoom to the widget, the engine following the node's width as it already did. *(Device 2026-10-06, hook `Z` — a finger-down on the plugin then the gesture through the widget's own listeners, with a held-still control that does raise the menu: no hold, engine zoom tracks the widget 1.01 → 1.20, page drawn; pinch back out drawn. A finger test by the owner is still the final word.)*

## R7, criterion 4 (editable) (removed 2026-11-07)

*(PARTIAL 2026-08-10, T-112 — was `[ ]`. The **page→card leg now exists**: `app/models/jihad-chrome-prefs.js` gained `isSettingsUrl()` + `adoptFromUrl()` ported from the Enyo shell's `ChromePrefs`, with the SAME path-only gate and the SAME `https?` / `about:(preferences|settings|jihad|isis|blank)` allowlist, and `main-assistant.js` calls `adoptChromePrefs()` from BOTH committed-url handlers — so an edit made on `about:preferences` is adopted by this card. Three things are still open or worth knowing:* *(**2026-10-01 (third session) — DEVICE-VERIFIED on CE 3.1.0, page → card, and it found a defect that undid edits; `[~]` only because opening the page from the card's menu and typing are a finger's.** On the Mojo card: `about:preferences`, the home-url field and the first shortcut's name each edited by coordinate tap + inserted text + Enter ("Home page saved.", "Start page links saved."), the page published `{"homeUrl":"…/t254","startLinks":[{"title":"DuckDuckGoX254",…` in its fragment, and one Back returned to the start page, which the card re-opened as `start.html#links=[{"title":"DuckDuckGoX254",…` — the deferred redraw of note (c) working as designed. A card relaunch still showed the edited list, and restoring it on the page restored it in the card. **The defect:** the page published each edit by assigning `location.hash`, which pushes a history entry per edit, so Back stepped to the PREVIOUS fragment, the card was handed that older payload as a committed url and adopted it — the latest edit silently reverted in the card while the page's own store kept it. Fixed with `location.replace`; after it one Back leaves the page. The same edit was checked on the Enyo and Mochi cards, whose start pages showed the new shortcut name after a relaunch (`../impl/evidence-2026-10-01/start-page-adopts-settings-edit_enyo_mochi.jpg`).)*

  *(a) **The card→page leg is NOW PRESENT (updated 2026-08-15).** ~~There is no `settingsUrl()` here, because per the 2026-08-05 user direction this variant gets no settings entry~~ — the user WITHDREW that direction on 2026-08-15 (T-156) and chose to add the entry. So `jihad-chrome-prefs.js` gained `settingsUrl()` (builds `about:preferences#chrome=<{homeUrl,startLinks}>` from THIS card's home + shortcuts), and `main-assistant.js` gained an app-menu "Settings" item (`handleCommand` `jihad-settings` → `openUrl(settingsUrl())`). The page now opens on this card's values, and the full card↔page↔card round-trip (settingsUrl out, adoptFromUrl back) is closed. Device-verified: the Mojo card launches clean with the app menu; opens via `openUrl` with zero new adapter calls. See cavekit-preferences-ui.md R4, now `[x]` for all three variants.)*
  *(b) **NOT DEVICE-VERIFIED — no device was available.** What WAS verified: the gate's premise, read in source (`render/goanna/components/jihadAboutPreferences.js` `getURIFlags` returns only `ALLOW_SCRIPT` and deliberately withholds `URI_SAFE_FOR_UNTRUSTED_CONTENT`, so content cannot navigate into `about:preferences` — which is what makes a PATH test a gate and a SCHEME test not one); and the gate + allowlist exercised off-device against a `localStorage` stub — `about:blank#chrome=`, `about:blankfoo#chrome=`, an `https:` page and a case-shifted `ABOUT:PREFERENCES` all rejected; `about:settings` accepted; a payload carrying `javascript:`/`data:`/`file:` values passes the gate and is stripped to nothing by the allowlist; a pre-poisoned store is neutralised on READ; the 12-row cap holds; a repeat adopt returns false.*
  *(c) **The start-page redraw is DEFERRED here, not immediate, and the straight port of it would have been dead code.** Enyo/Mochi rebuild their start page in place because it is card chrome; ours is a DOCUMENT whose list travels in the url fragment, so a redraw is a NAVIGATION — and "redraw it if it is showing" can never fire, because the url that passes the adopt gate is `about:preferences`. A one-shot flag spends the redraw the next time the start page commits, which is the case that actually breaks (Back onto the start page's own history entry with its stale `#links=` fragment). Costs one extra history entry; a SECOND open Mojo card does not learn about the edit until it renavigates to its own start page.)*

## Changelog 2026-10-01 (removed 2026-11-07)

- 2026-10-01: **Target platform is now webOS CE 3.1.0.** The Scope line's "ships with webOS 3.0.5"
  describes the framework this variant was verified against; whether CE 3.1.0's
  `/usr/palm/frameworks/mojo` is the same build is part of cavekit-device-build.md R4's
  re-verification sweep, where every device-verified box in this kit is listed.

## Changelog 2026-08-05 (removed 2026-11-07)

- 2026-08-05: R6 extended (find as a magnifying-glass icon, home right of forward, the row
  scrolling, and the buttons circular + centred) and **R7 added** for the two chrome-owned
  settings. Four platform constraints recorded, all found by measuring on device rather than
  by reading CSS: the missing `Mojo.stringifyJSON` (which had silently disabled history
  persistence), `Mojo.Log.warn` being below the log threshold, the `.palm-menu-fade` block that
  clips statically-positioned menu items, and the TextField's two-node focus swap. Every one of
  these was a bug the user reported as a visual symptom ("the buttons are not visible", "the url
  bar looks incorrect") and none was diagnosable from the source alone.

## Changelog 2026-08-03 (removed 2026-11-07)

- 2026-08-03: Reconciled against the device. R1–R3 and R4's first two ACs marked met (all three
  variants live on hardware; the R7 harness passes 24/24); R4's Opal AC is the only one left,
  and it is hardware-gated. Added **R6** (command-menu parity actions: new card / history /
  share) for the chrome the user asked for, and recorded the two platform constraints found
  while building it — the card WebKit's `-webkit-` prefix requirement and the fact that Mojo
  needs no app-side `<select>` popup code.

## Changelog 2026-07-31 (removed 2026-11-07)

- 2026-07-31: Initial draft. Created when the user required all three front-ends (Enyo, Mochi, Mojo)
  to work standalone; `app-mojo/` had been a documented skeleton tracked only as one acceptance
  criterion under cavekit-device-build.md R3.
