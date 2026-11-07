---
created: "2026-08-05"
last_edited: "2026-11-07"
---

# Cavekit: In-Browser Preferences (`about:preferences`)

## Scope
An in-browser settings page — `about:preferences`, with `about:settings` as an alias — of the
kind Pale Moon and Basilisk ship, available in **all three** front-end variants (Enyo, Mochi,
Mojo). Covers the page itself, the settings it exposes, how a change reaches the live engine
and disk, and how it reconciles with the settings surfaces that already exist.

**User requirement, 2026-08-05:** *"Implement about:preferences / about:settings like pale moon /
basilisk have in all 3 variants."*

Two other settings surfaces exist and stay: the frozen YAP settings commands
(`asyncCmdSetEnableJavaScript`, `asyncCmdSetBlockPopups`, `asyncCmdSetAcceptCookies`,
`asyncCmdSetMinFontSize`, `asyncCmdSetUserAgent`; cavekit-ipc-contract.md R1), and `about:config`,
the raw pref editor (cavekit-input-bridging.md R6). UXP ships the preferences machinery
(`toolkit/content/widgets/preferences.xml`) but no `about:preferences` page; in Pale Moon and
Basilisk that page is application chrome, so this port supplies its own, as it does
`chrome://branding/` and the XPI install prompt (cavekit-addons-extensions.md R2).

## Requirements

### R1: The page exists and opens in all three variants
**Description:** `about:preferences` is a real, registered about: URI that renders a settings page.

**Acceptance Criteria:**
- [x] `about:preferences` loads and renders a settings page — not an error page, not a `<parsererror>`, not a blank document. Verified by reading page CONTENT back (a pane heading), not by a title alone: `about:addons` proved that a page can report a plausible title while being an error document. *(Device-verified 2026-08-05: screenshot shows the pane tabs and the BROWSING group with live values — `browser.sessionhistory.max_entries` 20, `layout.frame_rate` 30, this build's own tuning read back.)*
- [x] `about:settings` resolves to the same page, as an alias rather than a second copy. (Pale Moon/Basilisk parity: users of either name land in the same place.) *(Device-verified 2026-08-05: one component class, two `?what=` contract IDs; daemon log `load done uri=about:settings`, `title=[Settings]` — the page retitles itself from `document.location` so it does not look like it redirected.)*
- [x] It opens in **each** of the three variants, each against its own daemon and profile. *(Device 2026-08-06, all three: `rect sel:button[data-pane=advanced]` resolves the Advanced tab to `473,95 100x44`. Mojo's card takes no `target` launch param, so Mojo was driven with an engine `url about:preferences` on its own daemon and profile.)*
- [x] It renders with no browser chrome present — this embedding has no XUL browser window, and a page that assumes one is the failure mode this project has hit repeatedly (cavekit-addons-extensions.md R2, and the `amInstallTrigger` / `AddonManager` chrome assumptions in patch 0013). *(It references no chrome window at all; the only privileged thing it touches is `Services.prefs`.)*
- [x] The page does not hand-roll what the platform already provides, and it does not depend on anything this embedding lacks. **Implemented as an HTML chrome document, NOT as `<prefwindow>`/`<prefpane>`** — see the decision note below.

**Decision (2026-08-05), HTML, not `<prefwindow>`:** `<prefwindow>` is a chrome-window binding and this embedding has no chrome window to open one in. The page follows Basilisk's in-content shape with Pale Moon's panes; its controls are native `<input>`/`<select>`, and `<select>` uses the daemon's popup path.
**Dependencies:** cavekit-engine-embedding.md (R2), cavekit-addons-extensions.md (R2 — the branding-package precedent)

### R2: The panes cover what this device can actually change
**Description:** The settings offered are modeled on Pale Moon/Basilisk's categories, minus anything meaningless here.
**Acceptance Criteria:**
- [x] The page presents named categories in the Pale Moon/Basilisk idiom (e.g. General, Content, Privacy, Security, Advanced). The exact set is an implementation choice; that they are NAMED and navigable is not. *(Exactly those five, as a scrolling tab row. Pale Moon's Tabs, Applications and Sync panes are deliberately absent — one page per webOS card, no helper-app chooser a headless daemon could show, no account system — and the page's own source says so rather than leaving it to be rediscovered.)*
- [x] Every control maps to a real engine preference and changes it. **No decorative toggles** — a control that does nothing is worse than an absent one, and this codebase has already been bitten by exactly that (the Mochi prefs that persist but never reach the engine). *(2026-08-05: rows rebuilt from ~60 probed candidates so each is backed by a pref this engine has; the first cut bound 13 of 42 to application prefs a bare GRE lacks. `general.useragent.override` row removed (`EngineHost` rewrites it at every start); `intl.accept_languages` is read through `nsIPrefLocalizedString`. T-143 (2026-08-10): the daemon's `SetMinFontSize`/`SetBlockPopups`/`SetAcceptCookies`, and later `SetEnableJavaScript`, write the pref DEFAULT branch, so the page's user value survives a card relaunch; the card re-sends its values on every adapter connect from `enyo.BasicWebView.initView()`, which is framework code. 2026-10-01, CE 3.1.0: every row walked on the Enyo card through its own control, 56 of 56 changed the pref they name and were put back; writes land only in that variant's profile. Closed 2026-10-02 after T-290; the later `plugin.state.flash` row walked `2 → 0 → 2`.)*
- [x] A setting that cannot work in this embedding is **absent, not inert**. Named examples to decide explicitly rather than inherit: anything requiring a chrome window, plugin enable/disable (see cavekit-addons-extensions.md R7 — plugins are managed through `about:plugins`), and update settings (we operate no update server; `extensions.update.enabled` is false by design). *(No shipped row is inert. `prefExists()` renders an unknown pref disabled and labelled, as a guard. Plugin rows are the activation policies (`plugin.default.state`, `plugin.state.flash`); per-plugin enable/disable stays in `about:plugins`. 2026-10-01: "Block pop-up windows" had no visible effect because nothing could open a window; fixed in the daemon (`window.open` opens a new card, cavekit-browser-services.md R1) and measured in all three variants.)*
- [x] At minimum the settings the frozen contract already carries are present and correct: JavaScript enable, popup blocking, cookie acceptance, minimum font size. These are the ones a card can already change, so a disagreement between the two surfaces is immediately user-visible. *(Closed 2026-10-01, CE 3.1.0, all three variants, each changed on the page and shown on `render/goanna/test/jihad-settings.html`: pop-ups off → `LOADPOPUP:attempted`; minimum font 20 → a 6 px line measures `h=23` (was 7); cookies Never → `COOKIE:refused`; JavaScript off → `JS:OFF`. Each `user_pref` restored.)*

**Two prefs are honored but shipped no default.** `privacy.donottrackheader.enabled` (necko sends the DNT header from it) and `places.history.enabled` (Places records visits from it) are read by platform code with a fallback, so they work with no browser app — they simply had no default entry, which made them render as unavailable. `packaging/prefs/jihad-platform-prefs.js` now ships the upstream defaults so the rows are live, which changes no behaviour.
- [x] **The card panels carry no Block Popups / Accept Cookies / Enable JavaScript rows.** *(OWNER DECISION 2026-10-02, T-290: remove them. The settings page owns those three values; a card switch that sets a default the page's value silently overrides is the decorative control this requirement forbids.)* The Enyo and Mochi panels lose the three rows, each shell keeps its entry that opens the settings page, and a value a card stored for them earlier is no longer re-applied at launch. The frozen commands themselves stay on the wire (cavekit-ipc-contract.md R1). *(MET 2026-10-02, CE 3.1.0: no such controls on the live Enyo or Mochi panel, and More Settings opened `about:preferences#chrome=…` (`../impl/evidence-2026-10-02/enyo_settings-panel-without-content-rows_more-settings-opens-page.jpg`, `../impl/evidence-2026-10-02/mochi_settings-panel_clear-confirm_bookmark-editor.jpg`); the three names are gone from `applyPreference`'s map. Mojo never had the rows. `build/webos-oe/card-selftest.sh`. Since 2026-10-05 the card panels are gone entirely (R5).)*
**Dependencies:** R1, cavekit-browser-services.md (R1)

- [x] **The rows DO what they say — effect-verified, not value-verified.** *(2026-10-06, owner: "ensure all options … that are relevant are working". Host page (`echo-server.py`: `/effects`, `/echo`) on the Mochi card, prefs set in `prefs.js` between daemon restarts. Effect seen, both ways where there are two: images, JavaScript, pop-ups, Adobe Flash, local storage, default and minimum font size, languages (`Accept-Language: de-DE,de;q=0.5`), location, referer, cookies, maximum TLS. Decorative rows dealt with: DNT is now sent by the daemon's request observer (UXP has no DNT code); "Disable WebGL" and "Zoom text and images together" removed. Daemon fixes from the audit: Referer on re-driven navigations; title changes after load reported (`nsIEmbeddingSiteWindow::SetTitle`). Value-verified only (2026-10-01): smooth scrolling, session history size, frame rate, caret browsing, the about:config warning, media autoplay, page fonts/colors, referer spoofing, history/form memory, cookie lifetime/third-party, add-on install/blocklist/update/compatibility, logins, minimum TLS, mixed content, OCSP, client certificate, cache, connections/pipelining/prefetch/IPv6/proxy, animated images. 55 rows.)*

- [x] **DuckDuckGo Lite is the default search engine, with DuckDuckGo HTML beside it** (owner 2026-10-06: "add lite/html duckduckgo as a default search engine option and make it the default"). The list, in every shell and on this page: DuckDuckGo Lite (`https://lite.duckduckgo.com/lite/?q=`, the default), DuckDuckGo HTML (`https://html.duckduckgo.com/html/?q=`), DuckDuckGo, Google, Wikipedia. A stored choice of the OLD default ("duckduckgo") moves to Lite ONCE and the move is recorded, so a user who picks full DuckDuckGo afterwards keeps it; any other stored choice is left alone. The page migrates only its own stored copy — a card's payload is that card's choice and is not re-migrated. *(Device: Mochi reads `engine=duckduckgo-lite`, search url `lite.duckduckgo.com`; Mojo's migration checked under node; Enyo's stored "google" kept.)*

### R3: A change reaches the live engine AND the disk
**Description:** Changing a setting takes effect and survives a restart.
**Acceptance Criteria:**
- [x] A change takes effect without restarting the daemon — demonstrated on a real page (e.g. disabling JavaScript stops a script from running on the next load), not merely by reading the pref back. *(Closed 2026-10-01, CE 3.1.0, all three variants: the page's JavaScript row off → marker page `JS:OFF`, a reload keeps it; card relaunched (`setEnableJavaScript 1` logged) → still off; daemon restarted → still off, one line in `prefs.js`; re-checking restores `JS:ON`. The failed 2026-08-05 run did not navigate after writing. The real defect was the card's relaunch value landing on the user branch; it is now the default, and the docShell's script flag follows the effective pref at each load.)*
- [x] The change is written to the variant's `prefs.js` and is still in effect after a clean shutdown and restart. Proof comes from the PROFILE, not from the UI that made the change — the pattern that closed cavekit-input-bridging.md R6 for `about:config`. *(Device-verified 2026-08-05 through the page's OWN `writePref()`: `network.http.max-connections=24` appeared as a `user_pref` in the Mojo profile's `prefs.js`, was still there after `stop jihad-mojo`, and was read back as 24 by a freshly started daemon.)*
- [x] The daemon's clean-shutdown path is what flushes it (cavekit-engine-embedding.md R2): a change made and then lost to a `kill -9` is expected, and a change lost to `stop <job>` is a bug. *(T-132, device 2026-08-10: `setpref s jihad.flushtest inside-window`, then `stop jihad` ~3 s later; `profile/prefs.js` carries the `user_pref` and a restart reads it back.)*
- [x] Each variant's changes land in its OWN profile and do not leak to the other two (cavekit-device-build.md R7). *(2026-08-06, from the profiles: `jihad.chrome.settings` is only in enyo's `prefs.js`, `network.http.max-connections` only in mojo's.)*
**Dependencies:** R2, cavekit-engine-embedding.md (R2)

### R4: Reachable from each front-end without typing a URL
**Description:** A user finds it where they would look for settings.
**Acceptance Criteria:**
- [x] Each variant's own menu/command surface has an entry that opens the page — Enyo's app menu, Mochi's overflow menu, Mojo's command menu. *(3 of 3 since 2026-08-15: Enyo app menu, Mochi overflow menu, Mojo app menu (`app-mojo/app/assistants/main-assistant.js` `setupAppMenu`, "Settings" → `JihadChromePrefs.settingsUrl()`). The owner withdrew the 2026-08-05 "no settings icon in mojo" instruction.)*
- [x] The entry opens it through the normal navigation path (the same `openUrl` any other page uses), so no new adapter call is introduced. *(Enyo `this.$.browser.setUrl(enyo.jihadChrome.settingsUrl())`, Mochi `this.openUrl(enyo.jihadChrome.settingsUrl())`, Mojo `this.openUrl(JihadChromePrefs.settingsUrl())`; no `callBrowserAdapter` call was added.)* **This must not widen the frozen `callBrowserAdapter` set** that cavekit-ui-shell.md R2 holds byte-identical. *(`about:` URLs pass the daemon's URL fixup untouched, `GoannaRenderPage.cpp`.)*
- [x] Where a variant already has a card-side preferences view, the relationship between the two is decided and documented rather than left to chance — see R5. *(Decided 2026-08-06; see "One writer, three caches" under R5.)*
**Dependencies:** R1, cavekit-ui-shell.md (R2), cavekit-mochi-ui.md, cavekit-mojo-ui.md

### R5: One story for settings, not two
**Description:** The engine page and the existing card-side preference UIs must not disagree.
**Acceptance Criteria:**
- [x] For every setting exposed in both places, there is ONE source of truth, and it is written down which one. The current split — Enyo pushes to the engine, Mochi persists to db8 without pushing, Mojo has nothing — is the state this criterion exists to end. *(See "One writer, three caches" below.)*
- [x] Changing a setting in one surface is reflected in the other, or the other stops offering that setting. Either resolution is acceptable; silently disagreeing is not. *(Device 2026-08-06: a real click on the page's "Restore this pane's defaults" → `saveChrome()` → `publishToFragment()` → daemon same-document location `about:preferences#chrome=…` → `msgTitleAndUrlChanged` → the card, which logged `links=1` before and `links=3` after. Needed a body in `PageChrome::OnLocationChange`.)*

**Merge status 2026-10-05 — DONE.** Each card's own "Browser Settings" panel is gone; the app menu has one **Settings** entry that opens `about:settings`, whose Browser pane carries search engine, home page, Clear Data (bookmarks, history, cookies, cache — the card-owned lists cleared through the notification door) and the start-page links; the card adopts the chrome settings from the page's URL fragment. Help is removed. Device-verified on Enyo and Mochi with real clicks (commit `b38a87fc`); Mojo's merged settings ran on the device only from 2026-10-05's package install on.

**One writer, three caches (2026-08-06).** `jihad.chrome.settings` is written only by the settings
page. Each shell keeps a read-only `localStorage` cache (Enyo `ChromePrefs.js`, Mochi
`JihadChromePrefs.js`, Mojo `jihad-chrome-prefs.js`) so its start page builds synchronously on a
cold start, and adopts edits from the `#chrome=` fragment the page publishes. A fragment change is a
same-document navigation with no load stop; `PageChrome::OnLocationChange` records it and the pump
re-reports url and title (`emitLocationAndTitleCore()`, which skips the failure/cert half). The
daemon's Luna service serves both buses (not on the first start after a fresh install, as observed
on CE): `clearCache`/`clearCookies` public, `getChromePrefs` private-only
(cavekit-browser-services.md R2).

- [x] The frozen YAP settings commands keep working for cards that use them (cavekit-ipc-contract.md R1) — this domain must not break the contract to tidy the UI. *(Closed 2026-10-01, CE 3.1.0, shipping Enyo card, no test hook: changing its db8 preference record (`luna-send -a <appid> palm://com.palm.db/merge`) made the card send the command, logged by the daemon's handler (T-215). `setBlockPopups 0` → a load-time `window.open` goes `blocked` → `attempted`; `setAcceptCookies 0` → `COOKIE:refused`; `setEnableJavaScript 0` → `JS:OFF`; each restored. `setMinFontSize 2` is sent on every connect: 20 → the 6 px line is 23 px tall, 2 → 7 px. The YAP contract check (T-129) passes with all 73 commands.)*
**Dependencies:** R2, cavekit-mochi-ui.md (R2), cavekit-ipc-contract.md (R1)

### R6: Usable by a finger on the offscreen surface
**Description:** The page works under this project's rendering and input constraints.
**Acceptance Criteria:**
- [x] Controls are hit-testable by touch at the device's 1024×768 — the same constraint that produced the Mojo toolbar overflow, where the card WebKit silently ignored unprefixed `box-sizing`/flexbox (cavekit-mojo-ui.md R4). *(Real finger 2026-10-26: the emulated finger on the card's own input path (LunaSysMgr touch panel → card → adapter coordinate transform → daemon). Enyo: every row of the table (57 over six panes), the Fonts box, the Browser pane's controls, pane reset, Forget, Update. Mochi and Mojo: every tab and one control of each kind (checkbox, picker, number and text field, a Clear dialog). Each change read back from the engine and put back (`docs/rounds/t527-2026-10-25/`, `tools/device/finger/prefs-finger-walk.sh`). Not tapped: the certificates' Remove buttons (the owner's trust). Controls are at least 44 px tall. 2026-10-01: taps below the first screenful of a chrome page missed because the daemon scrolled with a `javascript:` URL, which does not run in a chrome document; it now scrolls natively. No horizontal overflow at 1024 wide in any pane (widest row 994).)* *(suite: `settings-every-control`, `settings-tabs`; `docs/FINGER-TESTS.md`)*
- [x] Any dropdown/menulist inside the page opens and is operable, through the existing XUL popup compositing and input routing (cavekit-offscreen-rendering.md R7) — a `<menulist>` in a prefpane is a separate display root exactly as the `about:addons` tools menu is. *(2026-08-06, all three: a coordinate click on the Content pane's first `<select>` produces `popupMenuShow id=sel1 items->/var/palm/jihad/<variant>/popup-sel1.json`. The page uses HTML `<select>`, not a XUL `<menulist>`, so it needs no separate-display-root compositing.)*
- [x] The page does not overflow horizontally at 1024 px wide, in either orientation. *(Device-verified 2026-08-05 at 768 wide, the narrower of the two; the pane tab row scrolls sideways rather than wrapping, and no group overflows.)*
- [~] Keyboard entry works in any text field the page offers (cavekit-input-bridging.md R6 — real key events reach XUL as of 2026-08-04). *(Left for a person: the on-screen keyboard under a finger (T-284). 2026-08-15: a chrome document had no `mFocusedEditable` (`insert: branch=NO-TARGET`); `GoannaRenderPage::InsertText` now recovers the target from `document.activeElement` when it is a text input. 2026-10-01, CE 3.1.0, all three variants: typed text is saved; the daemon dispatches `change` on blur, before a tap elsewhere, or on Enter, and resolves a `type=number` row's anonymous inner field to the control. Minimum font size typed 3 and blurred → 3 in `prefs.js`; Backspace, 4, Enter → 4; home-url + Enter → "Home page saved.". The same through a kernel input keyboard that LunaSysMgr takes for a Bluetooth one (cavekit-input-bridging.md R2a): End, Backspace, `3`, Enter → pref 3.)*

**The 2026-08-06 "I cant scroll down in about:preferences" report** was the engine under-reporting
content size: `nsIDOMWindowUtils::GetRootBounds` falls back to the viewport when there is no root
scroll frame. `GoannaRenderPage::GetContentSize` now also reads
`documentElement.scrollHeight/scrollWidth` and takes the larger, and geometry is re-measured after
paint so a page that rebuilds itself (the pane switcher) is re-sized. The user confirmed: "scrolling
under advanced is working."
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
  and the same page in Times New Roman again.)* A font's name is stored as Unicode (UXP's
  `setCharPref` narrows each character to a byte; a CJK family name would be garbled). *(Device,
  Enyo, 2026-10-13: Georgia picked from the list → `user_pref("font.name.serif.x-western",
  "Georgia")` through `nsISupportsString`; Restore → no `font.` pref left.)* A size the list does not
  hold (the card's minimum of 2 px) takes its place in order. *(Device, Enyo: "None, 2 px, 9 px …".)*
- [x] Each font list is the engine's own enumeration of what is installed: "Default (<the font the
  engine would use>)" first, then the fonts of that kind, then every other installed font. Choosing
  Default clears the setting rather than writing a name. Fontconfig's generic aliases are left out,
  and a font is offered for a script when the engine lists it for Latin text or for that script (so
  one that cannot draw the script is not); "Other writing systems" names no language, so it lists
  every font, as in Pale Moon. *(Device, Enyo: Default, then the 14 installed —
  Arial, Courier New, Dotum, Georgia, Hei S, Heisei Kaku Gothic, Lucida Console, Prelude, Prelude
  Condensed, PreludeCompressedWGL, PreludeCondensedWGL, PreludeWGL, Times New Roman, Verdana; the
  aliases are gone, and so is Noto Sans Thai, which draws Thai only. (Recorded 2026-10-12 as a name
  "with no font file behind it": wrong — `/usr/share/fonts/HeiT_nb.ttf` is Noto Sans Thai, Google's,
  as fontconfig's cache and `fc-query` say; it belongs in the Thai list. Inspector round 22.) The
  engine names no default on this device, so the first entry reads "Default".)*
- [x] Restore defaults puts the group's font settings back (the pane's Reset restores every script's);
  a sample line shows each chosen font; `about:settings#pane=content&section=fonts` opens the section.
  *(Device, all three: the section drawn in each shell's look.)* *(2026-10-13: the page scrolled to it
  but the card stayed at the top — the daemon sent the jump before the page's new size; fixed, and
  Enyo and Mochi now open on Fonts & Colors.)* The same link followed again while it is the page's
  address does nothing (no hashchange; an about: page cannot take it out of its address) — noted.
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
  carries it.

## Known gaps found by adversarial review (2026-08-06)
- The prefs-ui block is in both `build/desktop/build-goanna.sh` and the device bundle script, and
  both fail the build loudly if their inputs are missing (fixed 2026-08-06).
- **The chrome URL path is load-bearing and undocumented.** The page gets the system principal from
  `nsChromeProtocolHandler`, which calls `SetOwner(systemPrincipal)` only when the chrome path
  begins `/content/`. `chrome://jihad-prefs/content/preferences.html` satisfies that. Rename the
  package part to anything else and the page silently loses privileges and every `Services.prefs`
  call throws.

## Changelog
- 2026-11-07: sweep; long evidence notes shortened, history moved verbatim to `archive/preferences-ui-history.md`.
- 2026-11-06 (T-565's whole runs): an emptied number field gives the setting its default back and shows it (`settings-int-emptied`). It wrote 0, and "Back/forward entries to keep" at 0 turned the Back button off with no word said; the three profiles on the bench had carried that 0 since a round between 2026-10-25 and 2026-11-04.
- 2026-10-12: R8 added (the owner's asks): a Personas, Extensions and Plugins group heads the Browser
  pane, one button per row, extensions described rather than named, the Flash Player's version and
  state shown; device-verified in all three variants.
- 2026-10-06 (night): R2 — DuckDuckGo Lite the default search engine, HTML added; one-time migration of the old default.
- 2026-10-06: R2 effect audit (see the new criterion): 12 rows effect-verified both ways on the device; DNT made real in the daemon; WebGL and zoom rows removed as decorative on this engine/device; two daemon defects found and fixed (no Referer on re-driven navigations; title changes after load not reported).
- 2026-10-06: R5 recorded as done (one Settings page, 2026-10-05).
- 2026-10-01 (third session): prefs sweep on CE in three variants. R2 contract-minimum box, R3 live-effect box and R5 frozen-commands box closed with device evidence. JavaScript row restored (the card's value is now a default). Found and fixed: typed text never saved (no `change`; number rows edited an inner field), taps below the first screen missed on chrome pages (engine never scrolled), each edit pushed a history entry and Back undid the latest edit in the card, `window.open` had no path at all.
- 2026-10-01: target platform is now webOS CE 3.1.0. Device-verified boxes are listed in cavekit-device-build.md R4's re-verification sweep.
- 2026-08-05: R1, R2 and R7 met; R3/R4/R6 partially. `about:preferences` and `about:settings` render on device.
- 2026-08-05: Initial draft, from the user requirement quoted in Scope.
