---
created: "2026-08-05"
last_edited: "2026-10-12"
---

# Cavekit: In-Browser Preferences (`about:preferences`)

## Scope
An in-browser settings page — `about:preferences`, with `about:settings` as an alias — of the
kind Pale Moon and Basilisk ship, available in **all three** front-end variants (Enyo, Mochi,
Mojo). Covers the page itself, the settings it exposes, how a change reaches the live engine
and disk, and how it reconciles with the settings surfaces that already exist.

**User requirement, 2026-08-05:** *"Implement about:preferences / about:settings like pale moon /
basilisk have in all 3 variants."*

### What already exists, and why this is not a duplicate
Three separate things touch settings today, and this domain exists because none of them is the
thing being asked for:

1. **The frozen YAP settings commands.** `asyncCmdSetEnableJavaScript`, `asyncCmdSetBlockPopups`,
   `asyncCmdSetAcceptCookies`, `asyncCmdSetMinFontSize`, `asyncCmdSetUserAgent` are part of the
   byte-identical contract (cavekit-ipc-contract.md R1) and are implemented in the daemon. This
   is how the CARD changes a setting today.
2. **Card-side preference UIs.** The Enyo app applies its prefs through those commands
   (`app/source/BrowserApp.js` `applyPreference`); the Mochi app has `JihadPreferences.js`, which
   persists toggles to its own db8 kind but deliberately does **not** push them into the engine
   (recorded as an intentional omission in `app-mochi/PARITY.md` and cavekit-mochi-ui.md R2).
   Mojo has no preferences UI at all.
3. **`about:config`.** Operable end to end as of 2026-08-04 (cavekit-input-bridging.md R6):
   filter, select a row, change a value, and the change persists to `prefs.js`. That is the raw
   pref editor, not a settings UI.

So the engine has real prefs, the cards have partial UIs over a fixed handful of them, and the
three variants disagree with each other. `about:preferences` is the one surface that can be the
same in all three, because it lives in the ENGINE rather than in each front-end's framework.

### The grounding fact for implementation
**UXP ships the preferences MACHINERY but not the PAGE.** `toolkit/content/widgets/preferences.xml`
provides the `<prefwindow>` / `<prefpane>` / `<preference>` XBL bindings, and
`toolkit/mozapps/preferences/` has a couple of shared dialogs (`changemp`, `removemp`,
`fontbuilder`). There is **no `about:preferences` in the GRE** — in Pale Moon and Basilisk that
page is application-supplied chrome. Confirmed by inspection 2026-08-05: the only GRE references
to the URI are in devtools/plugin tests.

This is the same shape as two things this port already ships itself: the `chrome://branding/`
package (without which `about:addons` was a `<parsererror>`) and the XPI install prompt component.
The precedent and the mechanism both exist; see cavekit-addons-extensions.md R2.

## Requirements

### R1: The page exists and opens in all three variants
**Description:** `about:preferences` is a real, registered about: URI that renders a settings page.

> **RE-VERIFIED 2026-08-10 on the CURRENT binaries — the staleness concern over this whole kit is
> closed.** Every `[x]` below was taken 2026-08-05/06, and the device has since been force-rebooted
> and re-pushed with patches `0027`/`0028`/`0029` plus a swapped PDK adapter, so it was fair to ask
> whether the prefs chrome still loads at all. It does: on a clean restart of the current build,
> `gettext body` on `about:preferences` returns the real page —
> *"Preferences Jihad Browser Browser General Content Privacy Security Advanced Home Page Home
> button target … Start Page Links Remove Add Link Restore Defaults …"*. Read back as page CONTENT,
> which is the standard this kit already insists on. Anyone working R2/R3/R6 can proceed without
> re-checking R1 first.

**Acceptance Criteria:**
- [x] `about:preferences` loads and renders a settings page — not an error page, not a `<parsererror>`, not a blank document. Verified by reading page CONTENT back (a pane heading), not by a title alone: `about:addons` proved that a page can report a plausible title while being an error document. *(Device-verified 2026-08-05: screenshot shows the pane tabs and the BROWSING group with live values — `browser.sessionhistory.max_entries` 20, `layout.frame_rate` 30, this build's own tuning read back.)*
- [x] `about:settings` resolves to the same page, as an alias rather than a second copy. (Pale Moon/Basilisk parity: users of either name land in the same place.) *(Device-verified 2026-08-05: one component class, two `?what=` contract IDs; daemon log `load done uri=about:settings`, `title=[Settings]` — the page retitles itself from `document.location` so it does not look like it redirected.)*
- [x] It opens in **each** of the three variants, each against its own daemon and profile. *(**MET 2026-08-06, all three, each against its own daemon and profile.** Proven by reading page CONTENT back, not a title: in every variant `rect sel:button[data-pane=advanced]` resolves the Advanced pane tab to `473,95 100x44`, i.e. the pane row really rendered. Enyo and Mochi were launched at the url; **Mojo's card does not accept a `target` launch param** (its framework differs from the Enyo shell's), so it was driven with an engine `url about:preferences` — the same daemon and the same profile, which is what this criterion is about. Mojo reaching it from its own UI is R4's business, and user direction there is that it has no settings entry.)*
- [x] It renders with no browser chrome present — this embedding has no XUL browser window, and a page that assumes one is the failure mode this project has hit repeatedly (cavekit-addons-extensions.md R2, and the `amInstallTrigger` / `AddonManager` chrome assumptions in patch 0013). *(It references no chrome window at all; the only privileged thing it touches is `Services.prefs`.)*
- [x] The page does not hand-roll what the platform already provides, and it does not depend on anything this embedding lacks. **Implemented as an HTML chrome document, NOT as `<prefwindow>`/`<prefpane>`** — see the decision note below. *(This criterion previously prescribed the XBL prefwindow bindings, which is a HOW and against the kit conventions; it is restated here as the outcome that was actually wanted.)*

**Decision (2026-08-05) — HTML, not `<prefwindow>`:** `toolkit/content/widgets/preferences.xml` does ship in our dist, but `<prefwindow>` is a *window* binding: it is built to be opened as a chrome dialog, it carries dialog buttons and `instantApply` semantics, and Pale Moon opens it as a separate window. This embedding has no chrome window to open one in — which is the same class of assumption R1 already forbids. Basilisk had already moved the identical material in-content, so the page follows Basilisk's shape with Pale Moon's panes. In-content HTML also sidesteps the two XUL problems this port has measured: popups as separate display roots (cavekit-offscreen-rendering.md R7) and unreliable zoom on `about:addons`. Nothing is hand-rolled that the platform provides — the controls are native `<input>`/`<select>`, and `<select>` already round-trips through the daemon's popup path.
**Dependencies:** cavekit-engine-embedding.md (R2), cavekit-addons-extensions.md (R2 — the branding-package precedent)

### R2: The panes cover what this device can actually change
**Description:** The settings offered are modeled on Pale Moon/Basilisk's categories, minus anything meaningless here.
**Acceptance Criteria:**
- [x] The page presents named categories in the Pale Moon/Basilisk idiom (e.g. General, Content, Privacy, Security, Advanced). The exact set is an implementation choice; that they are NAMED and navigable is not. *(Exactly those five, as a scrolling tab row. Pale Moon's Tabs, Applications and Sync panes are deliberately absent — one page per webOS card, no helper-app chooser a headless daemon could show, no account system — and the page's own source says so rather than leaving it to be rediscovered.)*
- [x] Every control maps to a real engine preference and changes it. **No decorative toggles** — a control that does nothing is worse than an absent one, and this codebase has already been bitten by exactly that (the Mochi prefs that persist but never reach the engine). *(Device-verified 2026-08-05, and it caught a real instance: the first cut bound 13 of 42 rows to `browser.startup.*` / `browser.download.*` / `security.warn_*`, which are APPLICATION prefs a bare GRE does not have — this build embeds the GRE with no browser app above it. The page was made to report its own missing set, and the table was rebuilt from ~60 probed candidates so every shipped row is backed by a pref this engine really has.)*

  **Downgraded from met on review 2026-08-06 — "exists" is not "is honored", and three more rows were decorative:**
  - `general.useragent.override` was editable but `EngineHost` re-writes it to `JIHAD_USER_AGENT` on EVERY daemon start, so an edit persisted to `prefs.js` and was then silently reverted. **Row removed.**
  - `intl.accept_languages` is a LOCALIZED pref: its stored value is `chrome://global/locale/intl.properties`, so a plain `getCharPref` displayed that URL as if it were the user's language list. **Read fixed** to `getComplexValue(..., nsIPrefLocalizedString)`.
  - `dom.disable_open_during_load`, `network.cookie.cookieBehavior` and `font.minimum-size.x-western` write the right global prefs with no docShell shadow — but the CARD re-applies its own copy at every launch, so the page's value is overwritten on the next start. That is the two-writers state R5 exists to end, and it is provable from source without a device. These rows stay, because they are correct the moment R5 lands. ~~*(`BrowserApp.browserPreferencesChanged` -> `applyPreference`)*~~ ***That attribution is WRONG — struck 2026-08-10; see T-143 immediately below.***

  **T-143, 2026-08-10 — fixed in the DAEMON, and the bullet above sent the fix to the wrong file. NO DEVICE was available; every statement here is established from source.**
  - **`font.minimum-size.x-western` never passes through the card's preference machinery at all.** There is no db8 copy of it (the seeded kind is blockPopups / acceptCookies / enableJavascript / rememberPasswords), no entry in `applyPreference`'s `preferenceMap`, and the name-fallback `o[inPreference]` finds nothing because `Browser` has no such member. Its only source is the literal `minFontSize: 2` on the WebView in `app/source/Browser.js`.
  - **The unconditional re-applier is the ENYO FRAMEWORK, not this app.** `enyo.BasicWebView.initView()` calls `blockPopupsChanged` / `acceptCookiesChanged` / `enableJavascriptChanged` / `minFontSizeChanged` on EVERY adapter connect, from its own published defaults (`blockPopups: true`, `acceptCookies: true`) — read it at `build/webos-oe/pdk/opt/PalmSDK/0.1/share/framework/enyo/1.0/framework/source/palm/controls/BasicWebView.js`. That is platform framework code and no app-side edit reaches it; `BrowserApp`'s db8 replay is only the SECOND of two writers.
  - Therefore **deleting the db8 replay in `BrowserApp.js` — what this task was written to do — would have made the state WORSE**: it removes the only writer carrying a user choice and leaves the one carrying a hardcoded constant, so popups/cookies would still be forced `true` at every launch AND the card's own remembered setting would stop working. It was not done.

  **DECISION: the PAGE owns the value, the CARD supplies the DEFAULT.** `jihad::SetMinFontSize` / `SetBlockPopups` / `SetAcceptCookies` (`render/goanna/GoannaRenderPage.cpp`) now write the pref service's DEFAULT branch instead of the user branch. A user value always shadows a default and defaults are never serialised, so an `about:preferences` edit survives the next card launch and `prefs.js` carries only the page's value. The frozen YAP commands are untouched (cavekit-ipc-contract.md R1) — same commands, same wire, and on a profile with no user pref the effective value is exactly what it was. **`enableJavascript` is deliberately left CARD-OWNED** on the user branch: the daemon gates script per docShell as well as by pref (R3), so the card command is the only thing that actually stops a script, and the page ships no JS row.

  **KNOWN CORNER, recorded rather than hidden:** once a user has set one of these three from `about:preferences`, the card panel's own Block Popups / Accept Cookies toggle stops having a visible effect — decorative in that narrow case, which this criterion forbids. R5's other branch (drop those two rows from `app/source/Preferences.js`) closes it, and was NOT taken unilaterally because it deletes a shipped card control. That is the decision this criterion is now waiting on.

  **STILL `[~]`; what is unproven is the READ side.** The daemon edit was COMPILED, not merely written: baseline and patched file both built inside `localhost/jihad-goanna-build` with the exact `build-daemon-arm.sh` flags (arm gcc 9.4, `-fno-exceptions -fno-rtti -Os -DJIHAD_OFFSCREEN_ONLY`). Nothing has RUN. Cheapest device check: change `network.cookie.cookieBehavior` from the page, relaunch the card, then confirm it is still a `user_pref` in that variant's `prefs.js` and reads back changed in `about:config`. `render/goanna/test/settings2_test.cpp` needs no change — it reads back on the root branch, which returns the default when no user value exists.

  **2026-10-01 (third session) — THE READ SIDE IS PROVEN on webOS CE 3.1.0, in all three variants; what keeps this `[~]` is now ONLY the owner decision (T-290).** From `about:preferences` itself, by coordinate click and typed text: the Block-pop-ups checkbox wrote `user_pref("dom.disable_open_during_load", false)` and a typed number wrote `user_pref("font.minimum-size.x-western", 3)`; `network.cookie.cookieBehavior` was set to 1. The card was then relaunched and re-sent its own values — the daemon logged `setBlockPopups 1`, `setAcceptCookies 1`, `setMinFontSize 2` — and the effective prefs were still `false` / `1` / `3`, still in `prefs.js`. Each write landed in that variant's profile only (user-pref line counts enyo/mochi/mojo `1/0/0`, `0/1/0`, `0/0/1`). Two things changed with it:
  - **The JavaScript row is back** (R3): the card's `setEnableJavaScript` now writes the default branch like the other three, so the page's choice survives a card relaunch and a daemon restart. That makes the known corner one row wider — once the user has set JavaScript, pop-ups or cookies on the page, the card panel's own toggle for it no longer shows through. Dropping those card rows is still the owner's call.
  - **"Block pop-up windows" used to change nothing a user could see** — this embedding had no new-window path, so `window.open()` failed with the blocker on and off. That is fixed in the daemon (cavekit-browser-services.md R1): a permitted `window.open` now opens a new card, so the row has a visible effect in both directions.
  **2026-10-01 (third session) — EVERY ROW WALKED ON THE DEVICE, and the page side of this criterion is met; it stays `[~]` only for the owner's decision on the CARD's rows (T-290).** One script took the row table out of `preferences.js` itself (56 rows: 30 checkboxes, 13 dropdowns, 10 number fields, 3 text fields) and, for each row on the Enyo card (CE 3.1.0, card connected, log growth asserted): read the engine pref, changed it THROUGH THE ROW'S OWN CONTROL — a coordinate tap for a checkbox, the card list answered for a dropdown, End + Backspace + typed text + Enter for a field — read the status line and the engine pref again, then put it back the same way and read it a third time. **56 of 56 controls changed the pref they name** (`general.smoothScroll false → true → false`, `security.tls.version.max 4 → 1 → 4`, `network.proxy.type 5 → 0 → 5`, `browser.display.background_color #FFFFFF → #FFFFF → #FFFFFF`, …); no row was unavailable, none was inert. 55 were put back through the control; `intl.accept_languages` was not, and that was the TEST's fault (it computed the character to type back from the stored chrome URL rather than from the displayed list), so `prefs.js` was restored from the copy taken before the walk and compared equal afterwards. One thing that row does show: its default is a localized pref, so once edited there is no way on the page to return to "the default" other than typing the same list — noted, not changed. The Browser pane (home page, shortcuts) was covered earlier the same day. **What remains is not on this page:** the card panel's Block Popups / Accept Cookies rows now only set a default that the page's value overrides — whether they stay is T-290.
  **2026-10-02 — CLOSED `[~]`→`[x]`.** The owner decided T-290 (remove the card rows; criterion below, met), which was the only thing keeping this open after the 56-row walk. The page gained one row since the walk, `plugin.state.flash` ("Adobe Flash"), walked the same way after the batch-3 install: `2 → 0 → 2` through its own dropdown.
- [x] A setting that cannot work in this embedding is **absent, not inert**. Named examples to decide explicitly rather than inherit: anything requiring a chrome window, plugin enable/disable (see cavekit-addons-extensions.md R7 — plugins are managed through `about:plugins`), and update settings (we operate no update server; `extensions.update.enabled` is false by design). *(No shipped row is inert. `prefExists()` still renders an unknown pref disabled and labelled, but that is a guard against a future build dropping a pref, not a shipped state. Plugins are exposed only as `plugin.default.state`, the activation policy, which is a real pref here — per-plugin enable/disable stays in `about:plugins`.)* *(**2026-10-01 (third session) — one row WAS inert in effect and this box did not know it:** "Block pop-up windows" set the pref correctly, but nothing could open a window in either state, so the user saw no difference. Not a page defect — the row was right and the engine embedding was missing the path; fixed there (window.open → a new card, cavekit-browser-services.md R1) and measured in all three variants: blocker on, a load-time `window.open` opens nothing; blocker off, it opens a card. The box stands.)*
- [x] At minimum the settings the frozen contract already carries are present and correct: JavaScript enable, popup blocking, cookie acceptance, minimum font size. These are the ones a card can already change, so a disagreement between the two surfaces is immediately user-visible. *(Three of four are present: `dom.disable_open_during_load`, `network.cookie.cookieBehavior`, `font.minimum-size.x-western`. **JavaScript enable is deliberately ABSENT** — the daemon gates script per-page on the docShell as well as by pref, so a pref written from this page does not stop a script (measured, see R3) and the control would be decorative. It returns with R5. The other three are believed effective but have NOT had the same "demonstrate it on a real page" treatment; that is R3's first criterion and it is open.)* *(**CLOSED `[~]`→`[x]` 2026-10-01 (third session) — all four are on the page and each was changed ON THE PAGE and shown on a real page, in all three variants (webOS CE 3.1.0, `render/goanna/test/jihad-settings.html` over http).** Popups row off → `LOADPOPUP:attempted` where it had read `LOADPOPUP:blocked`; minimum font size typed as 20 → a 6 px line measured `h=23` where it had been `h=7`; cookies set to Never through the card's own list → `COOKIE:refused` where it had read `COOKIE:kept`; JavaScript unchecked → the page's static `JS:OFF`. Each wrote its `user_pref` line and each was restored, leaving none. The `<select>` pick was delivered with the new `popupsel` inject, which enters at the same `popupMenuSelect` the card's list calls; a finger on that list is the human half, R6.)*

**Two prefs are honored but shipped no default.** `privacy.donottrackheader.enabled` (necko sends the DNT header from it) and `places.history.enabled` (Places records visits from it) are read by platform code with a fallback, so they work with no browser app — they simply had no default entry, which made them render as unavailable. `packaging/prefs/jihad-platform-prefs.js` now ships the upstream defaults so the rows are live, which changes no behaviour.
- [x] **The card panels carry no Block Popups / Accept Cookies / Enable JavaScript rows.** *(OWNER DECISION 2026-10-02, T-290: remove them. The settings page owns those three values; a card switch that sets a default the page's value silently overrides is the decorative control this requirement forbids.)* The Enyo and Mochi panels lose the three rows, each shell keeps its entry that opens the settings page, and a value a card stored for them earlier is no longer re-applied at launch. The frozen commands themselves stay on the wire (cavekit-ipc-contract.md R1). *(MET 2026-10-02, webOS CE 3.1.0, batch-3 packages installed through the supported installer. Enyo panel: no `blockPopups` / `acceptCookies` / `enableJavascript` controls exist (read off the live card), what is left is the search engine, Enable Flash, More Settings and the four clear buttons, and More Settings lands the card on `about:preferences#chrome=…` (`../impl/evidence-2026-10-02/enyo_settings-panel-without-content-rows_more-settings-opens-page.jpg`). Mochi panel: the same, plus its search-engine rows (`../impl/evidence-2026-10-02/mochi_settings-panel_clear-confirm_bookmark-editor.jpg`). Mojo never had such rows. The Enyo card no longer replays its stored copies: the three names are gone from the map `applyPreference` uses. Shown through each shell's own handlers by `build/webos-oe/card-selftest.sh`; a finger on the menu entry is on the human checklist.)*
**Dependencies:** R1, cavekit-browser-services.md (R1)

- [x] **The rows DO what they say — effect-verified, not value-verified.** *(2026-10-06, owner: "ensure all options … that are relevant are working". A page served from the host (`scratch echo-server.py`: `/effects` reports in its title what the engine let it do; `/echo` reports the request headers) was loaded on the Mochi card with the prefs set in `prefs.js` between daemon restarts, in batches. Effects seen, both ways where there are two: Load images (2 → the image is not loaded), Enable JavaScript (off → no script runs), Block pop-up windows (on → `window.open` is swallowed; off → "handing the address to a new card"), Adobe Flash (0 → `navigator.plugins` empty), store data locally (off → `localStorage` throws), Default font size (24 → 24px computed), Minimum font size (20 → a 6px span renders 22px tall), Preferred languages (`de-DE, de` → `Accept-Language: de-DE,de;q=0.5` and `navigator.language`), location (off → no `navigator.geolocation`), Send the referer header (0 → none), Accept cookies (2 → `document.cookie` stays empty), Maximum TLS version (1.0 → Wikipedia fails with an SSL error, no certificate question). **Three rows were found decorative and dealt with:** "Send a Do Not Track signal" — UXP removed DNT from the engine entirely (no code reads the pref), so the daemon's request observer now sends `DNT: 1` itself when the pref is on (header seen at the server); "Disable WebGL" — WebGL is never available on the offscreen surface (`getContext("webgl")` is null with the pref off), row REMOVED; "Zoom text and images together" — `browser.zoom.full` is a front-end pref and this browser's zoom is the daemon's render-time scale, row REMOVED. **Two engine-side defects the audit exposed, fixed in the daemon:** no navigation the daemon re-drives on the page's behalf (every tapped link, every `location.href`) carried a Referer — it does now; and a page that changes `document.title` after its load never told the card — it does now (`nsIEmbeddingSiteWindow::SetTitle`). Rows NOT effect-tested (value-level verification of 2026-10-01 stands; each is read by engine code that exists in this tree): smooth scrolling, session history size, frame rate, caret browsing, the about:config warning, media autoplay, page fonts/colors, referer spoofing, history/form memory, cookie lifetime/third-party, add-on install/blocklist/update/compatibility, logins, minimum TLS, mixed content, OCSP, client certificate, cache, connections/pipelining/prefetch/IPv6/proxy, animated images. The page now has 55 rows.)*

- [x] **DuckDuckGo Lite is the default search engine, with DuckDuckGo HTML beside it** (owner 2026-10-06: "add lite/html duckduckgo as a default search engine option and make it the default"). The list, in every shell and on this page: DuckDuckGo Lite (`https://lite.duckduckgo.com/lite/?q=`, the default), DuckDuckGo HTML (`https://html.duckduckgo.com/html/?q=`), DuckDuckGo, Google, Wikipedia. A stored choice of the OLD default ("duckduckgo") moves to Lite ONCE and the move is recorded, so a user who picks full DuckDuckGo afterwards keeps it; any other stored choice is left alone. The page migrates only its own stored copy — a card's payload is that card's choice and is not re-migrated. *(Device: Mochi reads `engine=duckduckgo-lite`, search url `lite.duckduckgo.com`; Mojo's migration checked under node; Enyo's stored "google" kept.)*

### R3: A change reaches the live engine AND the disk
**Description:** Changing a setting takes effect and survives a restart.
**Acceptance Criteria:**
- [x] A change takes effect without restarting the daemon — demonstrated on a real page (e.g. disabling JavaScript stops a script from running on the next load), not merely by reading the pref back. *(**MEASURED 2026-08-05, AND IT DID NOT.** The page wrote `javascript.enabled=false` through its own write path and read it back as false — then a scripted page loaded into that same running daemon and **its script still ran**. Cause: the daemon gates script per-PAGE on the docShell as well as by pref — `GoannaRenderPage::SetJavaScriptEnabled` does `ds->SetAllowJavascript(enabled)` alongside `SetBoolPref` (`GoannaRenderPage.cpp:2605`) — and a chrome page cannot reach a docShell. **The JavaScript row was therefore REMOVED from the page**, because R2 forbids a control that does nothing; it returns when the page can drive the daemon (R5). This criterion stays open, and it is the right criterion: reading the pref back would have "passed" and been wrong.)* *(**CLOSED `[~]`→`[x]` 2026-10-01 (third session). The page's OWN write path does stop script on the next page loaded — measured on the device, in all three variants.** With the JavaScript row temporarily restored on Enyo: the checkbox wrote `javascript.enabled = false`, the marker page loaded afterwards read `JS:OFF`, and a reload kept it off. So of the two 2026-08-05 explanations the true one is the second: that run did not NAVIGATE after writing. What that run could not have seen is the real defect: the card re-sent `setEnableJavaScript 1` on its next launch and the daemon wrote it to the USER branch, so the page's choice lasted exactly until the card was reopened (measured: relaunch → pref `true`, `JS:ON`). Fixed in the daemon — the card's value is now the default, and the docShell's script flag follows the effective pref at every load the daemon starts — and the row is restored on the page. After the fix, in each variant: page row off → `JS:OFF`; card relaunched (`setEnableJavaScript 1` logged) → still `false`, still `JS:OFF`; daemon restarted → still off, one line in `prefs.js`; the settings page itself still works with script off (chrome is immune) and unchecking restores `JS:ON` and removes the line. The other three settings are shown on a real page under R2.)*
  *(**2026-08-10 — HALF OF IT NOW DEMONSTRATED, and it narrows the 2026-08-05 failure considerably.**
  With the new `setpref` inject command, on one clean-restart run (`inject: no page` 0), the full
  cycle works in BOTH directions on a real scripted page:*
  ```
  getpref b javascript.enabled = [true]     -> gettext #j = [SCRIPT-RAN]
  setpref b javascript.enabled 0 -> [false] -> gettext #j = [NOSCRIPT]
  setpref b javascript.enabled 1 -> [true]  -> gettext #j = [SCRIPT-RAN]
  ```
  ***So the PREF alone does gate script for pages loaded afterwards, in the same running daemon —
  no docShell call needed.*** *That is NOT what 2026-08-05 concluded, and the difference matters:
  it means `SetAllowJavascript` is not load-bearing for a page navigated to AFTER the pref change,
  and the JavaScript row is therefore not blocked by the docShell reachability problem recorded
  above.*
  ***Why this is `[~]` and not `[x]`:*** *the write here came from the INJECT CHANNEL, not from
  `about:preferences` itself. Two candidate explanations for the 2026-08-05 failure survive and
  they need different fixes — the page's own write path may not reach the same pref branch, or that
  test may not have NAVIGATED after writing (this run navigated away and back each time, and a
  document already loaded keeps its docShell's `allowJavascript`). Settle which, and the row can
  come back. **Do not restore the JavaScript row on the strength of this run alone.**)*
- [x] The change is written to the variant's `prefs.js` and is still in effect after a clean shutdown and restart. Proof comes from the PROFILE, not from the UI that made the change — the pattern that closed cavekit-input-bridging.md R6 for `about:config`. *(Device-verified 2026-08-05 through the page's OWN `writePref()`: `network.http.max-connections=24` appeared as a `user_pref` in the Mojo profile's `prefs.js`, was still there after `stop jihad-mojo`, and was read back as 24 by a freshly started daemon.)*
- [x] The daemon's clean-shutdown path is what flushes it (cavekit-engine-embedding.md R2): a change made and then lost to a `kill -9` is expected, and a change lost to `stop <job>` is a bug. *(**MET — device-verified 2026-08-10 under T-132; this box was left unticked by oversight and reconciled 2026-08-15 from the build-site record.** Clean PASS, and it needed a new tool first: `setpref s jihad.flushtest inside-window` → `ok=1 readback=[inside-window]`, `stop jihad` ~3 s later, and `profile/prefs.js` then carries `user_pref("jihad.flushtest", "inside-window")`; a restart reads it back via `getpref`. A pref written moments before the stop LANDS — the SIGTERM flush does real work and the lazy save timer is not the only writer. Two earlier VOID attempts are recorded on the build-site row (T-132): one stopped the daemon without relaunching the card so every later inject hit `inject: no page`; one used `jsurl` + `Components`, which is not exposed in a content scope, so the probe silently did nothing.)*
- [x] Each variant's changes land in its OWN profile and do not leak to the other two (cavekit-device-build.md R7). *(**MET 2026-08-06, cross-proven in BOTH directions from the profiles themselves rather than from the UI.** Two different page-written prefs, each present in exactly one variant and absent from the other two: `jihad.chrome.settings` is in the **enyo** profile's `prefs.js` only (written by the settings page this session), and `network.http.max-connections` is in the **mojo** profile's only (written by the page on 2026-08-05). Three separate `prefs.js` files, three sizes, no cross-contamination.)*
**Dependencies:** R2, cavekit-engine-embedding.md (R2)

### R4: Reachable from each front-end without typing a URL
**Description:** A user finds it where they would look for settings.
**Acceptance Criteria:**
- [x] Each variant's own menu/command surface has an entry that opens the page — Enyo's app menu, Mochi's overflow menu, Mojo's command menu. *(**3 of 3 as of 2026-08-15** — Mojo entry added when the user withdrew the exemption; see the resolution note below. Earlier: 2 of 3 as of 2026-08-06: **Enyo** ("Settings" in the app menu) and **Mochi** ("Settings" in the overflow menu), both opening it through the ordinary navigation path with the card's current values in the fragment. **Mojo deliberately has none** — user direction 2026-08-05, "dont put a settings icon in mojo, just depend on about:preferences and about:settings" — so it is reachable there only by typing the url. This criterion cannot go to met without either overriding that instruction or amending it; flagged rather than quietly re-scoped.)* ~~*(OPEN. Today the page is only reachable by typing the URL. Deliberately not wired yet: adding an entry ALONGSIDE each variant's existing preferences panel would ship the exact two-surfaces-disagreeing state R5 exists to prevent, so the entry lands with the merge — see R5 and the note there.)*~~ ***STALE — struck 2026-08-10.** It is the OLDER of the two notes on this box and it flatly contradicts the newer one beside it: Enyo and Mochi both got their entry on 2026-08-06, so "only reachable by typing the URL" has been false for both since then.*

  **RESOLVED 2026-08-15 — the user WITHDREW the Mojo exemption; Mojo now has a Settings entry, so all three variants do. `[~]`→`[x]`.** Asked as a product-owner decision (the criterion could not be closed by code while the 2026-08-05 "no settings icon in mojo" instruction stood), the user chose "add a Mojo entry after all," reversing that instruction. Implemented: `app-mojo/app/assistants/main-assistant.js` gains a `setupAppMenu()` with a single **"Settings"** item (`command: "jihad-settings"`) and a `handleCommand` arm `this.openUrl(JihadChromePrefs.settingsUrl())`; `app-mojo/app/models/jihad-chrome-prefs.js` gains `settingsUrl()` building `about:preferences#chrome=<{homeUrl,startLinks}>` from the card's current values (the same fragment contract `adoptFromUrl` reads back). Placed in the APP MENU (text item), not the crowded icon-only command row, which is also where Enyo puts Settings. Verified on device: the Mojo card launches clean with the app menu set up (no error), and — like Enyo/Mochi — the entry opens the page through the ordinary `openUrl` path with **no new `callBrowserAdapter` method** (`git diff` of the two Mojo files adds zero adapter calls). So the frozen adapter set (cavekit-ui-shell R2 / mojo-ui) is untouched. The original exemption note follows for the record.

  ~~**STATUS OF THIS CRITERION, resolved 2026-08-10.** It stays `[~]` and it is the one box here that
  cannot be closed by writing code, because as worded ("EACH variant's own menu/command surface")
  it contradicts an explicit, quoted user instruction recorded in the note above — *"dont put a
  settings icon in mojo, just depend on about:preferences and about:settings"* (2026-08-05).
  Enyo and Mochi are done; Mojo is done-by-decision.~~ *(Superseded 2026-08-15 — the user withdrew the instruction; see above.)*
- [x] The entry opens it through the normal navigation path (the same `openUrl` any other page uses), so no new adapter call is introduced. *(**MET.** Enyo's entry is `this.$.browser.setUrl(enyo.jihadChrome.settingsUrl())` and Mochi's is `this.openUrl(enyo.jihadChrome.settingsUrl())` — both the ordinary path. Verified against the frozen set rather than asserted: `git diff main -- app/` shows the only `callBrowserAdapter` line added on this branch is `findInPage`, which is an existing contract command, not a new one. Nothing widened.)* **This must not widen the frozen `callBrowserAdapter` set** that cavekit-ui-shell.md R2 holds byte-identical. *(OPEN, but already established as free: `about:` URLs pass the daemon's URL fixup untouched (`GoannaRenderPage.cpp`), and every variant opens the page today through its ordinary launch/openUrl path with no adapter change.)*
- [x] Where a variant already has a card-side preferences view, the relationship between the two is decided and documented rather than left to chance — see R5. *(**MET.** Decided and written down: the settings page is the ONE writer of `jihad.chrome.settings`; each shell keeps a read-only `localStorage` cache so its start page builds synchronously on a cold start, and adopts edits off the committed url. Mochi's duplicate editor — the genuine second writer — was removed. Recorded in R5's "ONE writer now, three caches" note and in each shell's ChromePrefs header.)*
**Dependencies:** R1, cavekit-ui-shell.md (R2), cavekit-mochi-ui.md, cavekit-mojo-ui.md

### R5: One story for settings, not two
**Description:** The engine page and the existing card-side preference UIs must not disagree.
**Acceptance Criteria:**
- [x] For every setting exposed in both places, there is ONE source of truth, and it is written down which one. The current split — Enyo pushes to the engine, Mochi persists to db8 without pushing, Mojo has nothing — is the state this criterion exists to end. *(MET. `jihad.chrome.settings` (engine pref) is written ONLY by the settings page; each shell keeps a read-only `localStorage` cache so its start page can build synchronously on a cold start. Mochi's duplicate EDITOR — the genuine second writer — was removed. Written down in the "ONE writer now, three caches" note below and in each shell's ChromePrefs header.)*
- [x] Changing a setting in one surface is reflected in the other, or the other stops offering that setting. Either resolution is acceptable; silently disagreeing is not. *(**MET, device-verified end to end 2026-08-06**, on the real page rather than a stand-in: a real click on the page's own "Restore this pane's defaults" control (inject `clickid reset-pane`, `ok=1`) → `saveChrome()` → `publishToFragment()` → daemon `same-document location -> about:preferences#chrome=%7B…%7D` → `msgTitleAndUrlChanged` → the card. Proven from the CARD's own state, not from the url: seeded with a one-link payload the card logged `links=1`, and after the click it logged `links=3` — the cache moved because of the page's edit. Needed one fix, `PageChrome::OnLocationChange` (see the corrected note below).)*

**Merge status 2026-10-05 — DONE.** Each card's own "Browser Settings" panel is gone; the app menu has one **Settings** entry that opens `about:settings`, whose Browser pane carries search engine, home page, Clear Data (bookmarks, history, cookies, cache — the card-owned lists cleared through the notification door) and the start-page links; the card adopts the chrome settings from the page's URL fragment. Help is removed. Device-verified on Enyo and Mochi with real clicks (commit `b38a87fc`); Mojo's merged settings ran on the device only from 2026-10-05's package install on. The 2026-08 note below is history.

**Merge status 2026-08-05/06 — built, one link short.**
- The **one editing surface exists**: `about:preferences` gained a **Browser** pane (first tab)
  holding the home-button target and the start-page shortcut list, with a row-per-link editor.
  Device-verified rendering and seeding.
- **Enyo's app-menu "Preferences" now opens that page**, through the ordinary `setUrl` path, and
  its native panel no longer offers those two settings — so there is no second writer.
- **card -> page works, PROVEN**: the card opens `about:preferences#chrome=<json>` carrying its
  current values and the page renders them (verified with a deliberately distinctive payload —
  home `https://example.org/HOMETEST`, one link `MergeProof`).
- **page -> card WORKS as of 2026-08-06.** Of the two problems recorded below, **the first never
  existed** and the second was real and is fixed.

  **"The ref is stripped" was FALSE, and the evidence for it was a dead log.** The daemon has
  always reported the full url: launching at `about:preferences#chrome=LIVEMARK789` logs
  `titleAndUrl … uri=about:preferences#chrome=LIVEMARK789`, and the card receives it verbatim
  (the adapter's `msgTitleAndUrlChanged` is a pass-through). The `uri=about:preferences` line the
  earlier note was read from came from an **older load in a log that had stopped being written
  to**: the three variants' `daemon.log` files had grown to 55 MB of the device's 62 MB `/var`,
  taking it to 100% full, after which every `fprintf(stderr, …)` failed with ENOSPC silently
  while the daemon kept serving pages normally. A frozen log is indistinguishable from a live
  one. **Before trusting any daemon.log line, check that the file is still growing.** Root cause
  of the growth was `emitGeometry`'s unconditional log line, retried from the pump on every tick
  (1263 of the last 1400 lines); it now logs only on a real change, and the upstart job rotates
  the log at 2 MB.

  The second problem was real: the fix is a body in `PageChrome::OnLocationChange`, which records
  `LOCATION_CHANGE_SAME_DOCUMENT` for the pump to drain and re-report. Covered by a new
  `emitLocationAndTitleCore()` that deliberately skips the failure/cert half of
  `emitLocationAndTitle()` — those describe the last real LOAD, and re-running them on a fragment
  change would re-raise the previous load's failure on a page that never reloaded.

  Historical record of the two problems as they were originally diagnosed:
  1. The reported uri arrives without the ref — but **`CurrentUri()` is NOT the stripper, and
     the earlier note saying so was wrong** (corrected 2026-08-06): it is `GetCurrentURI()` ->
     `GetSpec()`, and `nsSimpleURI::GetSpec` appends the ref; `about:` URIs are `nsSimpleURI`s;
     and `mAliasUrl` only ever applies to `about:jihad`/`about:isis`. This project's OWN Mojo
     start page round-trips `start.html#links=...` successfully, which proves fragments survive
     in general. So the loss is either `about:`-specific (the channel / `originalURI` handling
     in `jihadAboutPreferences.js`) or happens card/adapter-side before `openUrl`. **Cheapest
     test: one `fprintf` of the raw url at the top of `BrowserPageGoanna::openUrl` and one of
     `CurrentUri()` after load-stop, then launch with `about:preferences#chrome=X`.**
  2. That emission happens on **load stop**. A fragment-only change is a same-document
     navigation and produces no load stop, so even with the ref included the card would not be
     told. Closing this needs a same-document location notification (an
     `onLocationChange` with `LOCATION_CHANGE_SAME_DOCUMENT`) plumbed to `msgLocationChanged`,
     or the page must signal by a means that is a real load. **The hook already exists and is
     already being delivered:** `PageChrome::OnLocationChange` (`GoannaRenderPage.cpp`) is an
     empty `return NS_OK` stub, and `PageChrome` is registered via `AddWebBrowserListener` with
     `NOTIFY_ALL`, which includes `NOTIFY_LOCATION`. Same-document location changes are arriving
     and being discarded. This is a body in an existing stub, not new plumbing.
  Everything else in the chain is built and verified.
- **The Luna route was built and then abandoned, and the reason is worth keeping.**
  `getChromePrefs` was added to `render/browserserver/JihadLunaService.cpp` (plus
  `jihad::GetChromeSettings` reading one JSON pref) and **works** — `luna-send -a
  com.palm.configurator palm://net.riverstonerelay.jihadBrowser/getChromePrefs` returns
  `{"returnValue":true,"settings":""}` on all three variants. But the daemon registers on the
  **private** bus, and an app CARD is on the public bus, so the card's call never arrives
  (measured: zero calls reached the daemon). **CORRECTED 2026-08-06 (adversarial review):** the reason given here — "making it public needs
  an LS2 role file in a system path, which this project does not do" — is **factually false about
  this repo**. `packaging/gen-variant-scripts.sh` already writes BOTH `/usr/share/ls2/roles/prv/`
  and `/usr/share/ls2/roles/pub/` role files (with `inbound: ["*"]`) plus a dbus service file, in
  the variant postinst. The public role is already installed. The real blocker is only that the
  daemon calls `LSRegister` (private handle); `LSRegisterPalmService` +
  `LSPalmServiceRegisterCategory` + `LSGmainAttachPalmService` would serve both buses against role
  files we already ship. That is a ~10-line daemon change, not a policy wall.
  **Follow-up this exposes:** `app/source/Browser.js` already calls
  `palm://net.riverstonerelay.jihadBrowser/clearCookies` FROM THE CARD. If private-only is right,
  that has never worked, and cavekit-browser-services.md R2's marks were verified with
  `luna-send -a com.palm.configurator` — a privileged caller — which does not exercise the
  shipped path.
  **BOTH CONFIRMED AND FIXED 2026-08-06.** Private-only WAS right, and the card's `clearCookies`/
  `clearCache` had never worked: measured on device, `luna-send -P` (public) reached the daemon
  not at all while `luna-send -n 1` (private) replied and logged. The daemon now registers with
  the palm-service family and serves both buses. The two method tables are deliberately
  different: `clearCache`/`clearCookies` are PUBLIC because the card calls them, and
  `getChromePrefs` is **PRIVATE-ONLY** — the fragment route above removed the card's need for it,
  and it returns the user's home page and shortcut list, which on the public bus any app on the
  device could read. Full measurements in cavekit-browser-services.md R2.

**ONE writer now, three caches (resolved 2026-08-06).** `jihad.chrome.settings` (engine) is written
only by the settings page. Each shell keeps its own `localStorage` CACHE so its start page can be
built synchronously on a cold start — Enyo's `ChromePrefs.js`, Mochi's `JihadChromePrefs.js`,
Mojo's `jihad-chrome-prefs.js` — and each adopts edits from the url the page publishes. Mochi's
duplicate EDITOR (the genuine second writer) has been removed and its menu now offers "Settings"
alongside its own panel, mirroring Enyo. Caches are fine; second writers were not.

**Why the merge is not just a UI move.** The engine-side settings are all in `about:preferences` already. What blocks the merge is the settings the SHELL owns and the engine cannot see: the home button's target and the start-page shortcut list (card `localStorage`, via `ChromePrefs.js` / `JihadChromePrefs.js` / `jihad-chrome-prefs.js`), plus the clear-data actions and the search engine. A page rendered by the engine cannot read the card's storage, so a half-merge would leave two writers for one setting — worse than two pages. Two routes, both scoped:
1. **Luna pref channel (preferred; needs a daemon rebuild).** `render/browserserver/JihadLunaService.cpp` registers only `clearCache`/`clearCookies` today. Adding `getChromePrefs`/`setChromePrefs` over engine prefs `jihad.chrome.*` lets the page write them via `Services.prefs` and each card read them over `palm://net.riverstonerelay.jihad-browser<variant>/` — a channel the cards already use, so the frozen YAP contract is untouched.
2. **URL-fragment round-trip (no rebuild).** The card opens `about:preferences#chrome=<json>`; the page renders those rows and writes the edited list back into the fragment; the card parses it off the committed url it already receives. Fragments are known to survive that round-trip — the Mojo start page needed `isStartPage()` taught to strip one.
- [x] The frozen YAP settings commands keep working for cards that use them (cavekit-ipc-contract.md R1) — this domain must not break the contract to tidy the UI. *(**2026-08-15 (T-116) — device-tested; `[ ]`→`[~]`, advanced but not cleanly closed, and the reason is instructive.** STRUCTURE is proven: the YAP contract check (T-129) passes with all 73 commands incl. `SetEnableJavaScript`/`SetBlockPopups`/`SetAcceptCookies`/`SetMinFontSize`/`SetUserAgent`, and `enyo.BasicWebView.initView()` SENDS them on every card connect (BrowserApp.js:328). DEFAULT EFFECTS are honoured on device, observed directly: a `window.open` page loads `POPUP-BLOCKED` (default `setBlockPopups(true)` is in effect), cookies persist (`setAcceptCookies`, cookie-test), scripts run (`setEnableJavascript(true)` default). **A clean runtime TOGGLE could NOT be shown through the inject channel, and both attempts hit CONFOUNDS rather than command failures:** driving `setBlockPopups(false)` via a temporary card hook then reloading still read `POPUP-BLOCKED`, because the test popup fires DURING page load and `dom.disable_open_during_load` blocks it independently of the popup-blocker; and `setEnableJavascript(false)` did not stop the next page's script, which is the SAME docShell/pref timing subtlety this kit's R3 documents as open (JS is gated per-docShell as well as by pref). So a full toggle proof needs either daemon-side logging on the command handlers (a rebuild) or a real user gesture for an on-user-gesture popup — neither closeable from the inject path, which is not equivalent to the adapter path. The card-side test hook was temporary and has been REVERTED; Browser.js is back to shipping. Net: the commands reach and their defaults are honoured; the toggle demonstration is confound-blocked, not contract-broken.)* *(**CLOSED `[~]`→`[x]` 2026-10-01 (third session) — each command driven by the SHIPPING Enyo card, with no test hook, and its effect read on a real page (webOS CE 3.1.0).** The card watches its own db8 preference records, so changing a record (`luna-send -a <appid> palm://com.palm.db/merge`) makes the unmodified card send the command; the daemon's handler line (T-215) shows it arriving. `blockPopups=false` → `setBlockPopups 0`, effective pref `false`, a load-time `window.open` goes from `blocked` to `attempted`; `acceptCookies=false` → `setAcceptCookies 0` → `COOKIE:refused`; `enableJavascript=false` → `setEnableJavaScript 0` → `JS:OFF`; each restored and read back (`JS:ON`, `COOKIE:kept`). `setMinFontSize 2` is sent on every connect (it has no db8 record) and the engine honours the pref it sets: 20 → the 6 px line is 23 px tall, 2 → 7 px. Nothing in the card was changed, so there is nothing to revert. The 2026-08-15 confounds are both explained: the popup probe then ran at load time against a blocker that was on, and the JavaScript check did not navigate.)*
**Dependencies:** R2, cavekit-mochi-ui.md (R2), cavekit-ipc-contract.md (R1)

### R6: Usable by a finger on the offscreen surface
**Description:** The page works under this project's rendering and input constraints.
**Acceptance Criteria:**
- [~] Controls are hit-testable by touch at the device's 1024×768 — the same constraint that produced the Mojo toolbar overflow, where the card WebKit silently ignored unprefixed `box-sizing`/flexbox (cavekit-mojo-ui.md R4). *(Sized for it — every control is at least 44 px tall and nothing is laid out at a fixed width — and the layout is device-verified, but no one has TAPPED one yet. NB this page is rendered by Goanna, not by the card's 2011 WebKit, so it needs none of the `-webkit-` workarounds the shells do; the first cut still got caught out by using `display: -moz-box`, which put every control UNDER its label on device. Plain flexbox fixed it.)* *(2026-08-06: a control has now been ACTIVATED on device — an injected `click 523 117` on the Advanced pane tab switched panes, screenshot-confirmed — so the page's own hit-testing and click handling work at device size. Still `[~]` and not `[x]` because that click enters at `BrowserPageGoanna::clickAt`, one layer below a real finger: it does not exercise the card's touch handling or the adapter's coordinate transform. **A HUMAN TAP is the last step.** Two things worth keeping from that run: `clickid`/`DebugClickElement` finds the element (`rect` reports its box correctly) and returns `ok=1` while NOT firing an HTML `<button>`'s click listener — a harness bug that reads as a product failure, so drive controls by coordinate; and this is how the scroll defect below was found.)* *(2026-08-16 — checkbox reconciled `[ ]`→`[~]` to match this criterion's OWN note, which already states "Still `[~]` and not `[x]`" (the `[ ]` was a leftover from before the 2026-08-06 addendum). Evidence unchanged and all device-side: the injected `click 523 117` switched the Advanced pane 2026-08-06 (screenshot-confirmed), the in-page `<select>` is operable on device in all three variants (next criterion, `[x]`), and the layout is device-verified. What keeps it off `[x]` is unchanged too — only a REAL FINGER exercising the card touch handler + adapter coordinate transform is unproven, the human gate in docs/DEVICE-HUMAN-TEST-PROCEDURES.md §2. No new device work; a checkbox-vs-note correction.)* *(**2026-10-01 (third session) — a defect that made half of every pane untappable was found and fixed; still `[~]` for the finger.** The daemon scrolled the engine with a `javascript:` URL, which does not run in a chrome document, so on this page the engine never left 0,0: the picture panned (the adapter pans its own buffer) but a tap below the first screenful was hit-tested against the unscrolled viewport and resolved to nothing — measured in the Mochi card (655 px window): `scroll 0 400` left a field at y=767 and the tap on it hit `<null>`. Now a native scroll; measured after: the same field tapped and typed into in Mochi, and in Enyo the Advanced pane's reset button at document y=1335 hit after `scroll 0 500` (`<BUTTON>`, "Everything in this pane was already at its default."). Also measured: no horizontal overflow at 1024 wide in any of the six panes (`contentSize=1024x937` in a 1024x690 window, widest row 994).)*
- [x] Any dropdown/menulist inside the page opens and is operable, through the existing XUL popup compositing and input routing (cavekit-offscreen-rendering.md R7) — a `<menulist>` in a prefpane is a separate display root exactly as the `about:addons` tools menu is. *(**MET 2026-08-06 — now proven ON THIS PAGE, in all three variants.** Tapping the first `<select>` in the Content pane (`rect sel:select` → `438,209 297x44`, then a coordinate click at its centre) produces `popupMenuShow id=sel1 items->/var/palm/jihad/<variant>/popup-sel1.json` in enyo, mochi and mojo — i.e. the options serialized and the card-side popup path fired, which is the same route already device-proven end to end for page `<select>`s (user-confirmed pick, 2026-08-03). NB the page uses HTML `<select>`, not a XUL `<menulist>`, so it never needed the separate-display-root compositing that criterion anticipated.)*
- [x] The page does not overflow horizontally at 1024 px wide, in either orientation. *(Device-verified 2026-08-05 at 768 wide, the narrower of the two; the pane tab row scrolls sideways rather than wrapping, and no group overflows.)*
- [~] Keyboard entry works in any text field the page offers (cavekit-input-bridging.md R6 — real key events reach XUL as of 2026-08-04). *(**STILL OPEN, and now MEASURED rather than merely unverified — it does NOT work on this page, while it DOES work on ordinary content.** On a plain `file://` page an injected `text` lands: the field went from `BASE-` to `TYPEDBASE-`, screenshot-confirmed. On `about:preferences` the same sequence tapped the home-url field, the daemon emitted `editorFocused=1 fieldType=0` (so the field WAS detected editable and focused), the inject channel logged `inject text (7 chars)` — and the field's rendered value did not change at all. The page's `change` handler therefore never ran and nothing was saved. Two candidates, neither yet distinguished: (a) `GoannaRenderPage::InsertText` silently returns when `mChrome->mFocusedEditable` is null, and `clickAt`'s focus-manager retarget can null it — in a CHROME document `nsIFocusManager::GetFocusedElement` is scoped to the focused window, which this embedding may not have set; (b) `edGetValue` fails for this control and the fallthrough does `SetTextContent`, which changes nothing visible on an `<input>`. **Next step: one `fprintf` in `InsertText` reporting focused-editable presence and which branch it took — BRANCH ONLY, never the text or the value.** That instrument is now BUILT — see the block below. The criterion stays `[ ]`: nothing about typing works any better, and the instrument has NOT been run.)* *(**2026-10-01 (third session) — typed text was never SAVED, on any field of this page, and that is fixed and device-verified in all three variants; still `[~]` for a finger and the on-screen keyboard.** The 2026-08-15 fix made the characters land; nothing then told the page. Two causes, both in the daemon: (1) every edit goes through the value setter, which the engine treats as a script write, so leaving the field fired no `change` — and this page, like most forms, saves on `change`; (2) the number rows are `<input type=number>`, whose on-screen text is an anonymous inner field, and that inner field was the edit target — `insert: branch=value/sel tag=[INPUT] type=[text] … inDoc=0` on a number row — so the control's own value never changed at all. Now the daemon dispatches `change` when an edited field loses focus, before a tap elsewhere is delivered, or on Enter, and resolves the edit target to the control. Measured by coordinate tap + inserted text: the minimum-font-size row typed to 3 and blurred → `font.minimum-size.x-western = 3` in the status line and in `prefs.js`; Backspace, 4, Enter → 4 with no blur; typed and deleted back → no write; the home-url field + Enter → "Home page saved." and the new value in the card's fragment.)* *(2026-10-01 (third session), later: now also through a REAL key path, not the inject channel — a kernel input keyboard that LunaSysMgr on CE takes for a Bluetooth one (cavekit-input-bridging.md R2a). Minimum-font-size field focused by a coordinate tap, then End, Backspace, `3`, Enter from that keyboard → status line `font.minimum-size.x-western = 3` and the engine pref 3; Backspace, `2`, Enter → 2. And the row walk above typed into all ten number fields and the three text fields through the daemon's key path. Left for a person: the same with the ON-SCREEN keyboard under a finger (T-284) — the only reason this is not `[x]`.)*
  *(**2026-08-15 — THE INSTRUMENT WAS RUN ON DEVICE and the root cause is SETTLED to candidate (a). `[ ]`→`[~]`.** With `JIHAD_LOG_INSERT=1`, about:preferences loaded (it needs the card's `#chrome=` fragment to render its panes — bare `about:preferences` shows only header/footer, a finding in itself), a test id added to the home-url input, `clickid` on it, then an injected `text`. The daemon logged `insert: branch=NO-TARGET chrome=1 editable=0 text=ok` — so `InsertText` sees `mChrome` but `mChrome->mFocusedEditable` is **NULL**, and `clickid … ok=0` corroborates that the click did not establish an editable focus in the chrome document. **This CONFIRMS candidate (a) and RULES OUT (b):** the insert never reached the `edGetValue`/`SetTextContent` branches — it returned at the NO-TARGET guard because there is no focused editable. Content pages set it fine (`clickid in1`→`editorFocused=1`, verified same session); chrome documents do not, consistent with `nsIFocusManager::GetFocusedElement` being scoped to a focused WINDOW this embedding does not set for a chrome doc. **THE FIX (daemon code + rebuild, next session): establish/track `mFocusedEditable` for chrome documents** — set the focused window, or resolve the focused editable through the chrome docShell's own focus controller rather than the top-level focus manager. Test artifacts reverted (the input id and `JIHAD_LOG_INSERT` were removed after the run).)*
  *(**FIXED + DEVICE-VERIFIED 2026-08-15 — the daemon fix was implemented, rebuilt, deployed and confirmed on hardware; chrome-page text entry now WORKS.** `GoannaRenderPage::InsertText` gained a fallback: when `mChrome->mFocusedEditable` is null it recovers the target from the focused document's `document.activeElement` (per-document, so it is NOT subject to the focus-manager window-scoping that made chrome docs fail), accepting it only if `edIsTextInput`. Only fires when the focus-event tracking already missed — content pages are untouched, and an unfocused page yields a non-text activeElement that is rejected, so there is no false target. Device proof, `JIHAD_LOG_INSERT` on: focusing the about:preferences home-url input and injecting text logged `insert: recovered editable from document.activeElement` then `insert: branch=value/sel tag=[INPUT] type=[url] before=29 want=32 readback=32 inDoc=1` — the value grew by exactly the inserted length, confirmed by the daemon's own `edGetValue` readback. So keyboard entry now reaches a chrome-page text field through the daemon's real editable path (the same `InsertText` the VKB uses, not the separate inject-`key` path). **Residual: a REAL finger tap must establish the focus** (make the field `document.activeElement`) — the same human-tap gate as this R6's hit-testability box and T-150; my test focused the field via a scripted `.focus()`. The daemon fix rides `push-engine-update.sh` (it is in `jihad-browserserver`, not the adapter). Test-only preferences.js hooks (id + focus) reverted.)*

  **INSTRUMENT LANDED 2026-08-10 (T-131) AND NOT RUN — there was no device in that session, so nothing here is a device result.** `GoannaRenderPage::InsertText` prints one line naming the branch it took plus a value-length read straight back off the element; `BrowserPageGoanna::keyDown` prints one line naming whether a focused editable existed at all. The keyDown line is not redundant: a VKB keystroke with no focused editable never reaches `InsertText`, so the InsertText probe alone prints NOTHING for the on-device typing case and an empty log would read as "the instrument did not fire" rather than as the answer. (The inject `text` channel does not pass through `keyDown` — it calls `InsertText` directly — which is why one probe cannot cover both.) Both are gated on `JIHAD_LOG_INSERT`; unset is silent, so a normal session cannot be spammed. Neither ever prints the text or the value — branch, tag/type and LENGTHS only, because this path also carries `<input type=password>` (F-163). What was VERIFIED is only that it builds: both TUs compile to ARM objects under the device toolchain with the daemon's exact flags (gcc 9.4 cross, `-fno-exceptions -fno-rtti -Os -DJIHAD_OFFSCREEN_ONLY`). Reading key:

  | log line | what it means |
  |---|---|
  | `keydown: editable=0` | the keystroke never reached `InsertText` — `HasFocusedEditable()` was false and the key fell through to engine dispatch |
  | `insert: branch=NO-TARGET` | `mFocusedEditable` was null when the text arrived — candidate (a)'s SHAPE, but see the correction below for which path actually nulls it |
  | `branch=value/sel readback==want inDoc=1` | the edit LANDED in the live DOM: the defect is downstream (no repaint, no `change`), not in `InsertText` |
  | `branch=value/sel readback==before` | the element took the write and refused it — `SetValue` no-oped |
  | `... inDoc=0` | the edit landed on a DETACHED element: real, invisible, and not one of the two candidates |
  | `branch=value/append` | no selection API on this control (F-220) |
  | `branch=textContent` | candidate (b): `edGetValue` really does fail for this control |

  **CORRECTION, and it is a correction to this criterion's own text: candidate (a) as written is contradicted by the evidence recorded three sentences earlier.** `clickAt`'s focus-manager retarget has two arms. The arm that NULLS `mFocusedEditable` — focus moved to a non-text control — also sets `mEditorFocused = false` in the same three statements (Codex F-326), and `mEditorFocused` is exactly what the `editorFocused=%d` line in `BrowserPageGoanna::pump` prints. Had that arm run, the recorded log would read `editorFocused=0`. It read `1`. So the retarget arm that can be live here is the OTHER one — `edIsTextInput(foc)` true, edit target silently moved to a DIFFERENT field — which is a different defect with a different fix. Established by reading `GoannaRenderPage.cpp` and `BrowserPageGoanna.cpp`, not by running anything.

  **Two candidates this criterion never listed, and both produce the recorded pair (`editorFocused=1` with no visible change) exactly:**
  1. **The engine focus LISTENER, not the focus manager.** `PageChrome::HandleEvent` sets `mFocusedEditable = (text && mUserInteracted) ? el : nullptr` on EVERY focus event, so a focus event whose target is not editable nulls the type target — and it never touches `mEditorFocused`, so the VKB stays raised and the log still says `1`. On reading, this is the only path that yields the recorded pair with no contradiction.
  2. **A DETACHED element.** `preferences.js` rebuilds its rows (the pane switcher rebuilds the DOM — the same rebuild that forced this R6's scroll re-measure). A detached `<input>` accepts `SetValue` and reads the new value back perfectly while being invisible on screen. `inDoc` exists in the log line for this case and for nothing else.

  **A cheap prediction that also ties this to T-110:** the int rows are `input[type=number]` (`packaging/prefsui/content/preferences.js:467`), whose `selectionStart` is null, so those rows must print `branch=value/append`. A `type=number` row printing `value/sel` would mean the F-220 selection-API assumption is wrong on this build.

  **The run (device, one restart):** add `JIHAD_LOG_INSERT=1` to the `exec env` line of `/etc/event.d/jihad` — **keeping `JIHAD_INJECT=1`, which the shipping template does not set and which a job regeneration silently drops** — then `stop jihad; start jihad` (a running daemon keeps its old environment), relaunch the card, open `about:preferences`, tap the home-url field, then `text …` on the inject channel. Assert the log GREW and that `inject: no page` is absent before reading anything: both failure modes look exactly like success. `type=[url]` in the line confirms the edit reached a url row (the page's only `input[type=url]`s are the home-url field and the start-page link fields), so a retarget onto some other input shows up as a different `type=`.

  **Separately found and unexplained: the DOM `input` event does not reach page script.** On the content-page test the value visibly changed and the page's own `input` listener never fired (`echo: (none)`), even though `InsertText` sets `mPendingInputEl` and `BrowserPageGoanna::pump` calls `FlushPendingInputEvent()` first thing on every tick. This is wider than this page — any site doing live validation, search-as-you-type or a controlled React input sees nothing. Not chased further this session; recorded so it is not rediscovered as a page bug.
**The "I cant scroll down in about:preferences" report — DIAGNOSED AND FIXED 2026-08-06.** Two
earlier diagnoses (drag-selects-text, root background) were wrong, both confounded by the
`jihad-effect` test add-on painting every document magenta; that add-on has now been removed from
the device profile and the page's real colours are screenshot-confirmed. The actual cause was in
the ENGINE, not the page, and it is not specific to this page at all:
- `GoannaRenderPage::GetContentSize` used only `nsIDOMWindowUtils::GetRootBounds`, which computes
  `scrollRange + scrollPort` from `presShell->GetRootScrollFrameAsScrollable()` — **and falls back
  to the root FRAME's rect when that is null**, i.e. reports exactly the viewport however tall the
  content is. Measured on the Advanced pane: `body` is `764x1397` in a 942-tall window, and the
  daemon reported `contentSize=768x942`. The adapter decides whether there is anything to scroll
  from that number, so there was nothing to scroll. Fixed by also reading
  `documentElement.scrollHeight/scrollWidth` and taking the LARGER of the two — correct with or
  without a root scroll frame, and never under-reporting. Now reports `768x1397`.
- Geometry was also never RE-measured after load: it was emitted on load completion and on resize
  only, so a page that rebuilds itself (the pane switcher) kept its original size forever. The
  dirty flag turned out not to fire for that change, so the re-measure now hangs off the paint,
  which by definition happened. `emitGeometry` still only emits on a real change.
**CONFIRMED BY THE USER 2026-08-06: "scrolling under advanced is working."** The daemon half was
proven by measurement (`contentSize` 942 → 1397, matching the measured `body` height); the finger
half could not be — the visible frame is composed by the ADAPTER from its own pan position, and
real touch cannot be injected on this device (the touchscreen is not an evdev node, the same limit
as the F-9/holdAt items in docs/PICKUP.md). A human scroll closed it.
**Dependencies:** R1, cavekit-offscreen-rendering.md (R7), cavekit-input-bridging.md (R6)

### R7: Provenance is preserved if the UI is adapted
**Description:** Borrowing from Pale Moon/Basilisk carries their licence, not their branding.
**Acceptance Criteria:**
- [x] Any file adapted from Pale Moon, Basilisk or UXP keeps its MPL-2.0 header and records where it came from, per cavekit-licensing-branding.md. *(Nothing was copied. No Pale Moon or Basilisk source was available to copy from — this tree vendors UXP, which is the platform only and has no `application/` directory — so the panes were reproduced from Pale Moon's ORGANISATION, against prefs verified to exist here. Recorded because "ported from Pale Moon" would otherwise imply copied files.)*
- [x] No Pale Moon / Basilisk / Moonchild branding, product names or artwork appear in the shipped page (cavekit-licensing-branding.md R3 strips those; this page must not reintroduce them). *(The page reads "Jihad Browser"; the only mentions of either project are in source comments explaining where the pane structure came from.)*
- [x] Newly authored files carry an MPL-2.0 header, matching the rest of `render/goanna` and the packaging we already ship. *(All five: the component, the manifest, and the three chrome files.)*
**Dependencies:** cavekit-licensing-branding.md (R1, R3)

### R8: A clear way to Personas, extensions and plugins
**Description:** Owner, 2026-10-12: "add a clear route to about:personas and extensions in browser
settings on all 3 variants", then "palefill should not be listed as the subheading for extensions.
also add plugins with flash player info to the settings page" and "one button per personas,
extensions, plugins". The settings page is where people look for how the browser looks and what it
runs; until now it offered only a "Manage Personas" button that went to about:addons.
**Acceptance Criteria:**
- [x] The Browser pane, the one the settings page opens on, starts with a "Personas, Extensions and
  Plugins" group, above everything else in it, with one button on each row. *(Device, all three;
  again on the final bundle `23953a34…`.)*
- [x] The Personas row says which Persona is worn (or that none is) and how many are installed;
  **Browse Personas** opens about:personas. *(Device, all three: "Wearing Space Fantasy by fx5800p.
  13 installed." in Enyo, Aurora Silk in Mochi, Lime Fizz in Mojo.)*
- [x] The Extensions row describes extensions rather than naming any, and says how many are
  installed and how many are turned off; **Manage Extensions** opens about:addons on its Extensions
  list. *(Device, all three: "Add features to web pages and to the browser. 1 installed.")*
- [x] The Plugins row gives the Flash Player's version, whether it is on, off or blocked, and where
  it comes from (or that it is not installed), and counts any other plugins; **Manage Plugins**
  opens about:addons on its Plugins list, where Flash's state is set and its details are. *(Device,
  all three: "Adobe Flash Player 10.3 d185, on. From the HP Flash plugin installed on this device."
  The other wordings: `render/goanna/test/prefs-addons-rows.test.js`, 8 cases.)*
- [x] Flash set to "Ask to Activate" (which about:addons offers, and which this browser cannot
  carry out: nothing activates the plugin afterwards, so it does not play) is said to be so on the
  Plugins row, shown as itself in the Content pane's Adobe Flash row (not as an empty list), and
  reported as off by the cards' Flash switch; the About pane's Flash line reads the same plugin
  host, so a Flash that is off is not called "not installed". *(Inspector 2026-10-12. Device, Enyo:
  state 1 and state 0 each read the same on the Plugins row, the Content row, About and `getFlash`.)*
- [x] The gallery's "Personas manager" link opens about:addons on its Personas list, the way on from
  Settings' one Personas button to taking a Persona off, removing it and Undo. *(Device, Enyo.)*
  *(Superseded later on 2026-10-12: the card cannot scroll about:addons' list, so the gallery holds
  the installed Personas itself — Your Personas, with Wear, Take off, Remove and Undo — and the link
  went; cavekit-personas-ui.md R8.)*
**Dependencies:** R1, R4, cavekit-personas-ui.md (R8, the gallery), cavekit-addons-extensions.md (R2, R7)

### R9: Fonts, as Pale Moon and Basilisk offer them
**Description:** The settings page lets the user choose the fonts pages are drawn in, as Pale Moon's and
Basilisk's Fonts & Colors (and their Advanced fonts dialog) do, using only the fonts already on the
device (the owner, 2026-10-12: "Add a setting for adjusting font like pale moon / basilisk have.
ensure software and font licensing / redistribution terms are respected").
**Acceptance Criteria:**
- [x] For a chosen script ("Fonts for": Western by default, and the other groups the engine knows,
  among them Cyrillic, Greek, Japanese, Korean and Chinese), the user sets the proportional font
  (serif or sans-serif), the serif, sans-serif and monospace fonts, the proportional and monospace
  sizes, and the minimum font size; each writes the engine's own `font.*` preference for that group,
  and takes effect on the next page drawn. *(Device, Enyo, 2026-10-12: Serif set to Georgia from
  the list, a page of serif text drawn after it in Georgia's old-style figures; Restore default fonts,
  and the same page in Times New Roman again.)*
- [x] Each font list is the engine's own enumeration of what is installed: "Default (<the font the
  engine would use>)" first, then the fonts of that kind, then every other installed font. Choosing
  Default clears the setting rather than writing a name. Fontconfig's generic aliases and names
  the engine lists for no script are left out. *(Device, Enyo: Default, then the 14 installed —
  Arial, Courier New, Dotum, Georgia, Hei S, Heisei Kaku Gothic, Lucida Console, Prelude, Prelude
  Condensed, PreludeCompressedWGL, PreludeCondensedWGL, PreludeWGL, Times New Roman, Verdana; the
  aliases and a "Noto Sans Thai" with no font file behind it are gone. The engine names no default on
  this device, so the first entry reads "Default".)*
- [x] Restore defaults puts the group's font settings back (the pane's Reset restores every script's);
  a sample line shows each chosen font; `about:settings#pane=content&section=fonts` opens the section.
  *(Device, all three: the section drawn in each shell's look.)*
- [x] "Allow pages to choose their own fonts" and the page-colour settings stay where they are.
- [x] Licensing: no font file is added to any package — the device's own fonts (HP's Prelude family,
  the Microsoft core fonts and the CJK fonts webOS ships) are used where they are installed, never
  copied, so their redistribution terms are not engaged. The page's code is the project's own
  (MPL-2.0, like the rest of the settings page; it follows UXP's FontBuilder approach, MPL-2.0, and
  copies no Pale Moon or Basilisk code, names or artwork beyond describing what they offer).
- [~] Device: changing the serif font changes the serif text of a page drawn afterwards, in all three
  variants; Restore defaults brings it back. *(Measured in Enyo; Mochi and Mojo draw the same page,
  which writes the same engine preferences — the pick itself there is part of the finger round.)*
**Dependencies:** R1–R3, R6

## Out of Scope
- `about:config` itself — the raw pref editor is cavekit-input-bridging.md R6, and it is done.
- Add-on management UI — `about:addons`, cavekit-addons-extensions.md R2.
- Plugin enable/disable — `about:plugins`, cavekit-addons-extensions.md R7.
- Sync/account settings: there is no account system in this browser and none is planned.
- The webOS system-level browser settings app (a platform surface we do not own).

## Cross-References
- See also: cavekit-addons-extensions.md (the branding-package precedent that makes an
  app-supplied chrome page work at all), cavekit-input-bridging.md (R6 — XUL input, and the
  `about:config` pattern for proving a pref change from the profile),
  cavekit-ipc-contract.md (R1 — the frozen settings commands this must not break),
  cavekit-ui-shell.md / cavekit-mochi-ui.md / cavekit-mojo-ui.md (R4's per-variant entry points),
  cavekit-offscreen-rendering.md (R7 — popup compositing, which a `<menulist>` needs),
  cavekit-licensing-branding.md (R7).

## Implementation
- Component: `render/goanna/components/jihadAboutPreferences.js` — an `nsIAboutModule` claiming
  both `@mozilla.org/network/protocol/about;1?what=preferences` and `?what=settings`, registered
  by manifest exactly as `jihadInstallPrompt.js` is. **No C++ change and no daemon rebuild**: the
  engine's own about: table (`jihadAboutPage` in `render/goanna/BrowserPageGoanna.cpp`) stays as
  it is, serving only the two static credit pages it was written for.
- Page: `packaging/prefsui/content/{preferences.html,preferences.js,preferences.css}`, registered
  as `chrome://jihad-prefs/` by `packaging/prefsui/jihad-prefsui.manifest`.
- `chrome://`, not `resource://`, because the page must keep the system principal to write
  `Services.prefs`. `getURIFlags` returns `ALLOW_SCRIPT` and deliberately **not**
  `URI_SAFE_FOR_UNTRUSTED_CONTENT`, so web content can neither link nor redirect into it.
- Installed by `build/webos-oe/make-device-bundle.sh` (next to the branding and install-prompt
  blocks, same idempotent manifest-append and same fail-the-build file check), so a fresh build
  carries it. Deployed to the three on-device bundles by hand for the 2026-08-05 verification.

## Known gaps found by adversarial review (2026-08-06)
- **FIXED 2026-08-06: `about:preferences` now exists on the DESKTOP build too.** The prefs-ui block
  is mirrored into `build/desktop/build-goanna.sh` next to the branding and XPI-prompt blocks, so
  R2/R3/R6 can be exercised on the desktop harness instead of only on hardware. Both that block and
  the device one now **fail the build loudly** if their inputs are missing, rather than skipping —
  the silent-skip guard is what let the two builds drift in the first place.
- **The chrome URL path is load-bearing and undocumented.** The page gets the system principal from
  `nsChromeProtocolHandler`, which calls `SetOwner(systemPrincipal)` only when the chrome path
  begins `/content/`. `chrome://jihad-prefs/content/preferences.html` satisfies that. Rename the
  package part to anything else and the page silently loses privileges and every `Services.prefs`
  call throws.

## Changelog
- 2026-10-12: R8 added (the owner's asks): a Personas, Extensions and Plugins group heads the Browser
  pane, one button per row, extensions described rather than named, the Flash Player's version and
  state shown; device-verified in all three variants.
- 2026-10-06 (night): R2 — DuckDuckGo Lite the default search engine, HTML added; one-time migration of the old default.
- 2026-10-06: R2 effect audit (see the new criterion): 12 rows effect-verified both ways on the device; DNT made real in the daemon; WebGL and zoom rows removed as decorative on this engine/device; two daemon defects found and fixed (no Referer on re-driven navigations; title changes after load not reported).
- 2026-10-06: R5 recorded as done (one Settings page, 2026-10-05).
- 2026-10-01 (third session): prefs sweep on CE in three variants. R2 contract-minimum box, R3 live-effect box and R5 frozen-commands box closed with device evidence. JavaScript row restored (the card's value is now a default). Found and fixed: typed text never saved (no `change`; number rows edited an inner field), taps below the first screen missed on chrome pages (engine never scrolled), each edit pushed a history entry and Back undid the latest edit in the card, `window.open` had no path at all.
- 2026-10-01: **Target platform is now webOS CE 3.1.0.** No box changed state here. R1's banner
  "RE-VERIFIED 2026-08-10 on the CURRENT binaries … proceed without re-checking R1" is about 3.0.5
  and no longer licenses skipping R1. Device-verified boxes are listed in
  cavekit-device-build.md R4's re-verification sweep. Two notes in R5 are now traps: "trust a log
  only if it is still growing" (an idle daemon's log grows forever, so growth does not prove a
  live page) and "the daemon now registers … and serves both buses" (not on the first start after
  a fresh install, as observed on CE).
- 2026-08-05: R1, R2 and R7 met; R3/R4/R6 partially. `about:preferences` and `about:settings`
  both render on device. Two corrections worth keeping: (a) R1's "build it on `<prefwindow>`"
  criterion prescribed a HOW, against the kit conventions, AND prescribed the wrong thing —
  `<prefwindow>` is a chrome-WINDOW binding and this embedding has no window to open it in; it
  is restated as an outcome and the page is in-content HTML, Basilisk's shape with Pale Moon's
  panes. (b) The first cut had 13 of 42 rows bound to application-level prefs (`browser.startup.*`,
  `browser.download.*`, `security.warn_*`) that a bare GRE does not ship — caught by making the
  page report its own missing set rather than by reading it, which is the same "prove it from the
  other side" discipline that closed `about:config`. R4 and R5 are held together on purpose: an
  entry point added before the merge would ship the two-disagreeing-surfaces state R5 forbids.
- 2026-08-05: Initial draft, from the user requirement *"Implement about:preferences /
  about:settings like pale moon / basilisk have in all 3 variants"*. Grounded on two facts
  established by inspection rather than assumption: UXP ships the `<prefwindow>`/`<preference>`
  XBL machinery (`toolkit/content/widgets/preferences.xml`) but **no `about:preferences` page** —
  it is application-supplied in Pale Moon/Basilisk — and this port already ships two
  app-supplied chrome pieces (the branding package, the XPI install prompt), so the mechanism
  is proven. R5 exists because the three front-ends currently disagree about settings: Enyo
  pushes to the engine, Mochi persists to db8 without pushing, Mojo offers nothing.
