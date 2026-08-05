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
- [ ] `about:preferences` loads and renders a settings page — not an error page, not a `<parsererror>`, not a blank document. Verified by reading page CONTENT back (a pane heading), not by a title alone: `about:addons` proved that a page can report a plausible title while being an error document.
- [ ] `about:settings` resolves to the same page, as an alias rather than a second copy. (Pale Moon/Basilisk parity: users of either name land in the same place.)
- [ ] It opens in **each** of the three variants, each against its own daemon and profile.
- [ ] It renders with no browser chrome present — this embedding has no XUL browser window, and a page that assumes one is the failure mode this project has hit repeatedly (cavekit-addons-extensions.md R2, and the `amInstallTrigger` / `AddonManager` chrome assumptions in patch 0013).
**Dependencies:** cavekit-engine-embedding.md (R2), cavekit-addons-extensions.md (R2 — the branding-package precedent)

### R2: The panes cover what this device can actually change
**Description:** The settings offered are modeled on Pale Moon/Basilisk's categories, minus anything meaningless here.
**Acceptance Criteria:**
- [ ] The page presents named categories in the Pale Moon/Basilisk idiom (e.g. General, Content, Privacy, Security, Advanced). The exact set is an implementation choice; that they are NAMED and navigable is not.
- [ ] Every control maps to a real engine preference and changes it. **No decorative toggles** — a control that does nothing is worse than an absent one, and this codebase has already been bitten by exactly that (the Mochi prefs that persist but never reach the engine).
- [ ] A setting that cannot work in this embedding is **absent, not inert**. Named examples to decide explicitly rather than inherit: anything requiring a chrome window, plugin enable/disable (see cavekit-addons-extensions.md R7 — plugins are managed through `about:plugins`), and update settings (we operate no update server; `extensions.update.enabled` is false by design).
- [ ] At minimum the settings the frozen contract already carries are present and correct: JavaScript enable, popup blocking, cookie acceptance, minimum font size. These are the ones a card can already change, so a disagreement between the two surfaces is immediately user-visible.
**Dependencies:** R1, cavekit-browser-services.md (R1)

### R3: A change reaches the live engine AND the disk
**Description:** Changing a setting takes effect and survives a restart.
**Acceptance Criteria:**
- [ ] A change takes effect without restarting the daemon — demonstrated on a real page (e.g. disabling JavaScript stops a script from running on the next load), not merely by reading the pref back.
- [ ] The change is written to the variant's `prefs.js` and is still in effect after a clean shutdown and restart. Proof comes from the PROFILE, not from the UI that made the change — the pattern that closed cavekit-input-bridging.md R6 for `about:config`.
- [ ] The daemon's clean-shutdown path is what flushes it (cavekit-engine-embedding.md R2): a change made and then lost to a `kill -9` is expected, and a change lost to `stop <job>` is a bug.
- [ ] Each variant's changes land in its OWN profile and do not leak to the other two (cavekit-device-build.md R7).
**Dependencies:** R2, cavekit-engine-embedding.md (R2)

### R4: Reachable from each front-end without typing a URL
**Description:** A user finds it where they would look for settings.
**Acceptance Criteria:**
- [ ] Each variant's own menu/command surface has an entry that opens the page — Enyo's app menu, Mochi's overflow menu, Mojo's command menu.
- [ ] The entry opens it through the normal navigation path (the same `openUrl` any other page uses), so no new adapter call is introduced. **This must not widen the frozen `callBrowserAdapter` set** that cavekit-ui-shell.md R2 holds byte-identical.
- [ ] Where a variant already has a card-side preferences view, the relationship between the two is decided and documented rather than left to chance — see R5.
**Dependencies:** R1, cavekit-ui-shell.md (R2), cavekit-mochi-ui.md, cavekit-mojo-ui.md

### R5: One story for settings, not two
**Description:** The engine page and the existing card-side preference UIs must not disagree.
**Acceptance Criteria:**
- [ ] For every setting exposed in both places, there is ONE source of truth, and it is written down which one. The current split — Enyo pushes to the engine, Mochi persists to db8 without pushing, Mojo has nothing — is the state this criterion exists to end.
- [ ] Changing a setting in one surface is reflected in the other, or the other stops offering that setting. Either resolution is acceptable; silently disagreeing is not.
- [ ] The frozen YAP settings commands keep working for cards that use them (cavekit-ipc-contract.md R1) — this domain must not break the contract to tidy the UI.
**Dependencies:** R2, cavekit-mochi-ui.md (R2), cavekit-ipc-contract.md (R1)

### R6: Usable by a finger on the offscreen surface
**Description:** The page works under this project's rendering and input constraints.
**Acceptance Criteria:**
- [ ] Controls are hit-testable by touch at the device's 1024×768 — the same constraint that produced the Mojo toolbar overflow, where the card WebKit silently ignored unprefixed `box-sizing`/flexbox (cavekit-mojo-ui.md R4).
- [ ] Any dropdown/menulist inside the page opens and is operable, through the existing XUL popup compositing and input routing (cavekit-offscreen-rendering.md R7) — a `<menulist>` in a prefpane is a separate display root exactly as the `about:addons` tools menu is.
- [ ] The page does not overflow horizontally at 1024 px wide, in either orientation.
- [ ] Keyboard entry works in any text field the page offers (cavekit-input-bridging.md R6 — real key events reach XUL as of 2026-08-04).
**Dependencies:** R1, cavekit-offscreen-rendering.md (R7), cavekit-input-bridging.md (R6)

### R7: Provenance is preserved if the UI is adapted
**Description:** Borrowing from Pale Moon/Basilisk carries their licence, not their branding.
**Acceptance Criteria:**
- [ ] Any file adapted from Pale Moon, Basilisk or UXP keeps its MPL-2.0 header and records where it came from, per cavekit-licensing-branding.md.
- [ ] No Pale Moon / Basilisk / Moonchild branding, product names or artwork appear in the shipped page (cavekit-licensing-branding.md R3 strips those; this page must not reintroduce them).
- [ ] Newly authored files carry an MPL-2.0 header, matching the rest of `render/goanna` and the packaging we already ship.
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

## Changelog
- 2026-08-05: Initial draft, from the user requirement *"Implement about:preferences /
  about:settings like pale moon / basilisk have in all 3 variants"*. Grounded on two facts
  established by inspection rather than assumption: UXP ships the `<prefwindow>`/`<preference>`
  XBL machinery (`toolkit/content/widgets/preferences.xml`) but **no `about:preferences` page** —
  it is application-supplied in Pale Moon/Basilisk — and this port already ships two
  app-supplied chrome pieces (the branding package, the XPI install prompt), so the mechanism
  is proven. R5 exists because the three front-ends currently disagree about settings: Enyo
  pushes to the engine, Mochi persists to db8 without pushing, Mojo offers nothing.
