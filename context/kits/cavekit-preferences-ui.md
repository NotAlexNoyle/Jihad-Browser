---
created: "2026-08-05"
last_edited: "2026-08-05"
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
**Acceptance Criteria:**
- [x] `about:preferences` loads and renders a settings page — not an error page, not a `<parsererror>`, not a blank document. Verified by reading page CONTENT back (a pane heading), not by a title alone: `about:addons` proved that a page can report a plausible title while being an error document. *(Device-verified 2026-08-05: screenshot shows the pane tabs and the BROWSING group with live values — `browser.sessionhistory.max_entries` 20, `layout.frame_rate` 30, this build's own tuning read back.)*
- [x] `about:settings` resolves to the same page, as an alias rather than a second copy. (Pale Moon/Basilisk parity: users of either name land in the same place.) *(Device-verified 2026-08-05: one component class, two `?what=` contract IDs; daemon log `load done uri=about:settings`, `title=[Settings]` — the page retitles itself from `document.location` so it does not look like it redirected.)*
- [~] It opens in **each** of the three variants, each against its own daemon and profile. *(Deployed into all three GRE bundles and the Luna/pref plumbing answers on all three — but it has only been LOADED in Mojo and Enyo. Downgraded from met on review 2026-08-06: "deployed to three" is not "opens in three", and Mochi is the one that has historically differed.)*
- [x] It renders with no browser chrome present — this embedding has no XUL browser window, and a page that assumes one is the failure mode this project has hit repeatedly (cavekit-addons-extensions.md R2, and the `amInstallTrigger` / `AddonManager` chrome assumptions in patch 0013). *(It references no chrome window at all; the only privileged thing it touches is `Services.prefs`.)*
- [x] The page does not hand-roll what the platform already provides, and it does not depend on anything this embedding lacks. **Implemented as an HTML chrome document, NOT as `<prefwindow>`/`<prefpane>`** — see the decision note below. *(This criterion previously prescribed the XBL prefwindow bindings, which is a HOW and against the kit conventions; it is restated here as the outcome that was actually wanted.)*

**Decision (2026-08-05) — HTML, not `<prefwindow>`:** `toolkit/content/widgets/preferences.xml` does ship in our dist, but `<prefwindow>` is a *window* binding: it is built to be opened as a chrome dialog, it carries dialog buttons and `instantApply` semantics, and Pale Moon opens it as a separate window. This embedding has no chrome window to open one in — which is the same class of assumption R1 already forbids. Basilisk had already moved the identical material in-content, so the page follows Basilisk's shape with Pale Moon's panes. In-content HTML also sidesteps the two XUL problems this port has measured: popups as separate display roots (cavekit-offscreen-rendering.md R7) and unreliable zoom on `about:addons`. Nothing is hand-rolled that the platform provides — the controls are native `<input>`/`<select>`, and `<select>` already round-trips through the daemon's popup path.
**Dependencies:** cavekit-engine-embedding.md (R2), cavekit-addons-extensions.md (R2 — the branding-package precedent)

### R2: The panes cover what this device can actually change
**Description:** The settings offered are modeled on Pale Moon/Basilisk's categories, minus anything meaningless here.
**Acceptance Criteria:**
- [x] The page presents named categories in the Pale Moon/Basilisk idiom (e.g. General, Content, Privacy, Security, Advanced). The exact set is an implementation choice; that they are NAMED and navigable is not. *(Exactly those five, as a scrolling tab row. Pale Moon's Tabs, Applications and Sync panes are deliberately absent — one page per webOS card, no helper-app chooser a headless daemon could show, no account system — and the page's own source says so rather than leaving it to be rediscovered.)*
- [~] Every control maps to a real engine preference and changes it. **No decorative toggles** — a control that does nothing is worse than an absent one, and this codebase has already been bitten by exactly that (the Mochi prefs that persist but never reach the engine). *(Device-verified 2026-08-05, and it caught a real instance: the first cut bound 13 of 42 rows to `browser.startup.*` / `browser.download.*` / `security.warn_*`, which are APPLICATION prefs a bare GRE does not have — this build embeds the GRE with no browser app above it. The page was made to report its own missing set, and the table was rebuilt from ~60 probed candidates so every shipped row is backed by a pref this engine really has.)*

  **Downgraded from met on review 2026-08-06 — "exists" is not "is honored", and three more rows were decorative:**
  - `general.useragent.override` was editable but `EngineHost` re-writes it to `JIHAD_USER_AGENT` on EVERY daemon start, so an edit persisted to `prefs.js` and was then silently reverted. **Row removed.**
  - `intl.accept_languages` is a LOCALIZED pref: its stored value is `chrome://global/locale/intl.properties`, so a plain `getCharPref` displayed that URL as if it were the user's language list. **Read fixed** to `getComplexValue(..., nsIPrefLocalizedString)`.
  - `dom.disable_open_during_load`, `network.cookie.cookieBehavior` and `font.minimum-size.x-western` write the right global prefs with no docShell shadow — but the CARD re-applies its own db8 copies at every launch (`BrowserApp.browserPreferencesChanged` -> `applyPreference`), so the page's value is overwritten on the next start. That is the two-writers state R5 exists to end, and it is provable from source without a device. These rows stay, because they are correct the moment R5 lands.
- [x] A setting that cannot work in this embedding is **absent, not inert**. Named examples to decide explicitly rather than inherit: anything requiring a chrome window, plugin enable/disable (see cavekit-addons-extensions.md R7 — plugins are managed through `about:plugins`), and update settings (we operate no update server; `extensions.update.enabled` is false by design). *(No shipped row is inert. `prefExists()` still renders an unknown pref disabled and labelled, but that is a guard against a future build dropping a pref, not a shipped state. Plugins are exposed only as `plugin.default.state`, the activation policy, which is a real pref here — per-plugin enable/disable stays in `about:plugins`.)*
- [~] At minimum the settings the frozen contract already carries are present and correct: JavaScript enable, popup blocking, cookie acceptance, minimum font size. These are the ones a card can already change, so a disagreement between the two surfaces is immediately user-visible. *(Three of four are present: `dom.disable_open_during_load`, `network.cookie.cookieBehavior`, `font.minimum-size.x-western`. **JavaScript enable is deliberately ABSENT** — the daemon gates script per-page on the docShell as well as by pref, so a pref written from this page does not stop a script (measured, see R3) and the control would be decorative. It returns with R5. The other three are believed effective but have NOT had the same "demonstrate it on a real page" treatment; that is R3's first criterion and it is open.)*

**Two prefs are honored but shipped no default.** `privacy.donottrackheader.enabled` (necko sends the DNT header from it) and `places.history.enabled` (Places records visits from it) are read by platform code with a fallback, so they work with no browser app — they simply had no default entry, which made them render as unavailable. `packaging/prefs/jihad-platform-prefs.js` now ships the upstream defaults so the rows are live, which changes no behaviour.
**Dependencies:** R1, cavekit-browser-services.md (R1)

### R3: A change reaches the live engine AND the disk
**Description:** Changing a setting takes effect and survives a restart.
**Acceptance Criteria:**
- [ ] A change takes effect without restarting the daemon — demonstrated on a real page (e.g. disabling JavaScript stops a script from running on the next load), not merely by reading the pref back. *(**MEASURED 2026-08-05, AND IT DID NOT.** The page wrote `javascript.enabled=false` through its own write path and read it back as false — then a scripted page loaded into that same running daemon and **its script still ran**. Cause: the daemon gates script per-PAGE on the docShell as well as by pref — `GoannaRenderPage::SetJavaScriptEnabled` does `ds->SetAllowJavascript(enabled)` alongside `SetBoolPref` (`GoannaRenderPage.cpp:2605`) — and a chrome page cannot reach a docShell. **The JavaScript row was therefore REMOVED from the page**, because R2 forbids a control that does nothing; it returns when the page can drive the daemon (R5). This criterion stays open, and it is the right criterion: reading the pref back would have "passed" and been wrong.)*
- [x] The change is written to the variant's `prefs.js` and is still in effect after a clean shutdown and restart. Proof comes from the PROFILE, not from the UI that made the change — the pattern that closed cavekit-input-bridging.md R6 for `about:config`. *(Device-verified 2026-08-05 through the page's OWN `writePref()`: `network.http.max-connections=24` appeared as a `user_pref` in the Mojo profile's `prefs.js`, was still there after `stop jihad-mojo`, and was read back as 24 by a freshly started daemon.)*
- [ ] The daemon's clean-shutdown path is what flushes it (cavekit-engine-embedding.md R2): a change made and then lost to a `kill -9` is expected, and a change lost to `stop <job>` is a bug.
- [ ] Each variant's changes land in its OWN profile and do not leak to the other two (cavekit-device-build.md R7).
**Dependencies:** R2, cavekit-engine-embedding.md (R2)

### R4: Reachable from each front-end without typing a URL
**Description:** A user finds it where they would look for settings.
**Acceptance Criteria:**
- [~] Each variant's own menu/command surface has an entry that opens the page — Enyo's app menu, Mochi's overflow menu, Mojo's command menu. *(2 of 3 as of 2026-08-06: **Enyo** ("Settings" in the app menu) and **Mochi** ("Settings" in the overflow menu), both opening it through the ordinary navigation path with the card's current values in the fragment. **Mojo deliberately has none** — user direction 2026-08-05, "dont put a settings icon in mojo, just depend on about:preferences and about:settings" — so it is reachable there only by typing the url. This criterion cannot go to met without either overriding that instruction or amending it; flagged rather than quietly re-scoped.)* *(OPEN. Today the page is only reachable by typing the URL. Deliberately not wired yet: adding an entry ALONGSIDE each variant's existing preferences panel would ship the exact two-surfaces-disagreeing state R5 exists to prevent, so the entry lands with the merge — see R5 and the note there. User direction 2026-08-05: Mojo is to have no settings icon of its own and depend on this page.)*
- [ ] The entry opens it through the normal navigation path (the same `openUrl` any other page uses), so no new adapter call is introduced. **This must not widen the frozen `callBrowserAdapter` set** that cavekit-ui-shell.md R2 holds byte-identical. *(OPEN, but already established as free: `about:` URLs pass the daemon's URL fixup untouched (`GoannaRenderPage.cpp`), and every variant opens the page today through its ordinary launch/openUrl path with no adapter change.)*
- [ ] Where a variant already has a card-side preferences view, the relationship between the two is decided and documented rather than left to chance — see R5.
**Dependencies:** R1, cavekit-ui-shell.md (R2), cavekit-mochi-ui.md, cavekit-mojo-ui.md

### R5: One story for settings, not two
**Description:** The engine page and the existing card-side preference UIs must not disagree.
**Acceptance Criteria:**
- [ ] For every setting exposed in both places, there is ONE source of truth, and it is written down which one. The current split — Enyo pushes to the engine, Mochi persists to db8 without pushing, Mojo has nothing — is the state this criterion exists to end. *(OPEN, and now the blocking item. User direction 2026-08-05: "merge about:preferences and webos app preferences page into one".)*
- [ ] Changing a setting in one surface is reflected in the other, or the other stops offering that setting. Either resolution is acceptable; silently disagreeing is not. *(OPEN.)*

**Merge status 2026-08-05/06 — built, one link short.**
- The **one editing surface exists**: `about:preferences` gained a **Browser** pane (first tab)
  holding the home-button target and the start-page shortcut list, with a row-per-link editor.
  Device-verified rendering and seeding.
- **Enyo's app-menu "Preferences" now opens that page**, through the ordinary `setUrl` path, and
  its native panel no longer offers those two settings — so there is no second writer.
- **card -> page works, PROVEN**: the card opens `about:preferences#chrome=<json>` carrying its
  current values and the page renders them (verified with a deliberately distinctive payload —
  home `https://example.org/HOMETEST`, one link `MergeProof`).
- **page -> card does NOT work yet.** The page publishes an edit by rewriting its own fragment,
  but the card never sees it: the daemon reports `titleAndUrl ... uri=about:preferences` — **the
  ref is stripped** — so `enyo.jihadChrome.adoptFromUrl()` never fires and the shell keeps its
  cached/default values. There are TWO problems on that leg, and the second is the bigger one:
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

**ONE writer now, three caches (resolved 2026-08-06).** `jihad.chrome.settings` (engine) is written
only by the settings page. Each shell keeps its own `localStorage` CACHE so its start page can be
built synchronously on a cold start — Enyo's `ChromePrefs.js`, Mochi's `JihadChromePrefs.js`,
Mojo's `jihad-chrome-prefs.js` — and each adopts edits from the url the page publishes. Mochi's
duplicate EDITOR (the genuine second writer) has been removed and its menu now offers "Settings"
alongside its own panel, mirroring Enyo. Caches are fine; second writers were not.

**Why the merge is not just a UI move.** The engine-side settings are all in `about:preferences` already. What blocks the merge is the settings the SHELL owns and the engine cannot see: the home button's target and the start-page shortcut list (card `localStorage`, via `ChromePrefs.js` / `JihadChromePrefs.js` / `jihad-chrome-prefs.js`), plus the clear-data actions and the search engine. A page rendered by the engine cannot read the card's storage, so a half-merge would leave two writers for one setting — worse than two pages. Two routes, both scoped:
1. **Luna pref channel (preferred; needs a daemon rebuild).** `render/browserserver/JihadLunaService.cpp` registers only `clearCache`/`clearCookies` today. Adding `getChromePrefs`/`setChromePrefs` over engine prefs `jihad.chrome.*` lets the page write them via `Services.prefs` and each card read them over `palm://net.riverstonerelay.jihad-browser<variant>/` — a channel the cards already use, so the frozen YAP contract is untouched.
2. **URL-fragment round-trip (no rebuild).** The card opens `about:preferences#chrome=<json>`; the page renders those rows and writes the edited list back into the fragment; the card parses it off the committed url it already receives. Fragments are known to survive that round-trip — the Mojo start page needed `isStartPage()` taught to strip one.
- [ ] The frozen YAP settings commands keep working for cards that use them (cavekit-ipc-contract.md R1) — this domain must not break the contract to tidy the UI.
**Dependencies:** R2, cavekit-mochi-ui.md (R2), cavekit-ipc-contract.md (R1)

### R6: Usable by a finger on the offscreen surface
**Description:** The page works under this project's rendering and input constraints.
**Acceptance Criteria:**
- [ ] Controls are hit-testable by touch at the device's 1024×768 — the same constraint that produced the Mojo toolbar overflow, where the card WebKit silently ignored unprefixed `box-sizing`/flexbox (cavekit-mojo-ui.md R4). *(Sized for it — every control is at least 44 px tall and nothing is laid out at a fixed width — and the layout is device-verified, but no one has TAPPED one yet. NB this page is rendered by Goanna, not by the card's 2011 WebKit, so it needs none of the `-webkit-` workarounds the shells do; the first cut still got caught out by using `display: -moz-box`, which put every control UNDER its label on device. Plain flexbox fixed it.)*
- [ ] Any dropdown/menulist inside the page opens and is operable, through the existing XUL popup compositing and input routing (cavekit-offscreen-rendering.md R7) — a `<menulist>` in a prefpane is a separate display root exactly as the `about:addons` tools menu is. *(NOT YET VERIFIED. The page uses HTML `<select>`, whose popup already round-trips to the card through `msgPopupMenuShow` and is device-proven in all three shells — a better position than a XUL `<menulist>`, but still unproven ON THIS PAGE.)*
- [x] The page does not overflow horizontally at 1024 px wide, in either orientation. *(Device-verified 2026-08-05 at 768 wide, the narrower of the two; the pane tab row scrolls sideways rather than wrapping, and no group overflows.)*
- [ ] Keyboard entry works in any text field the page offers (cavekit-input-bridging.md R6 — real key events reach XUL as of 2026-08-04). *(NOT YET VERIFIED on this page.)*
**Dependencies:** R1, cavekit-offscreen-rendering.md (R7), cavekit-input-bridging.md (R6)

### R7: Provenance is preserved if the UI is adapted
**Description:** Borrowing from Pale Moon/Basilisk carries their licence, not their branding.
**Acceptance Criteria:**
- [x] Any file adapted from Pale Moon, Basilisk or UXP keeps its MPL-2.0 header and records where it came from, per cavekit-licensing-branding.md. *(Nothing was copied. No Pale Moon or Basilisk source was available to copy from — this tree vendors UXP, which is the platform only and has no `application/` directory — so the panes were reproduced from Pale Moon's ORGANISATION, against prefs verified to exist here. Recorded because "ported from Pale Moon" would otherwise imply copied files.)*
- [x] No Pale Moon / Basilisk / Moonchild branding, product names or artwork appear in the shipped page (cavekit-licensing-branding.md R3 strips those; this page must not reintroduce them). *(The page reads "Jihad Browser"; the only mentions of either project are in source comments explaining where the pane structure came from.)*
- [x] Newly authored files carry an MPL-2.0 header, matching the rest of `render/goanna` and the packaging we already ship. *(All five: the component, the manifest, and the three chrome files.)*
**Dependencies:** cavekit-licensing-branding.md (R1, R3)

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
