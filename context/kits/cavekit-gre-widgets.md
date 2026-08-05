---
created: "2026-08-05"
last_edited: "2026-08-05"
---

# Cavekit: GRE Widget Bindings

## Scope
The XBL widget bindings UXP already ships in our dist, and whether they actually work here.
**49 binding files** are installed at `dist/bin/chrome/toolkit/content/global/bindings/`, wired
up in two places that this domain keeps carefully apart:

- **`xul.css` wires most of them for CHROME documents** — any page we author gets them free.
- **`html.css` and `forms.css` wire four for CONTENT** — they apply to every website, whether or
  not we ever thought about them: `videocontrols.xml`, `datetimebox.xml`
  (`date-input`/`time-input`), and `platformHTMLBindings.xml` (`inputFields`/`textAreas`).

Full inventory, including the ones deliberately not used: `../impl/impl-gre-widget-inventory.md`.

**Why this is a domain and not a footnote.** The content-facing four are user-facing web-platform
behaviour that NO existing requirement mentions — a website with a `<video>` or an
`<input type="date">` exercises engine paths this project has never tested. And the chrome-facing
ones are free implementation we keep not using: the preferences page in cavekit-preferences-ui.md
is the immediate consumer, but so is every future about: page.

**User requirement, 2026-08-05:** *"bring in all useful widgets as goals."*

## Requirements

### R1: HTML5 media elements work, or the limit is documented
**Description:** `<video>`/`<audio>` get the GRE's own controls, and playback either works or its failure is recorded precisely.
**Acceptance Criteria:**
- [ ] The `videocontrols.xml#videoControls` binding ATTACHES to a `<video>` in content — verified by reaching its anonymous content, not by the `controls` DOM attribute (which reflects regardless and proves nothing).
- [ ] The controls render into the offscreen surface and are visible in a captured frame.
- [ ] Play/pause is operable by tap through the normal input path, and the control's state changes in response.
- [ ] **Codec reality is measured and written down, whichever way it goes.** Which containers/codecs this ARM build can actually decode is unrecorded today and is upstream of every other media question — a build with no usable decoder makes the controls cosmetic. State what plays, what does not, and why (missing decoder vs missing hardware path).
- [ ] If media is not viable on this device, `<video>` degrades visibly rather than silently: the element must not render as a permanent blank box with working-looking controls.
**Dependencies:** cavekit-offscreen-rendering.md (R2), cavekit-input-bridging.md (R1)

### R2: Date and time inputs are either right or off
**Description:** `<input type="date">` / `<input type="time">` behave, or are explicitly disabled.
**Acceptance Criteria:**
- [ ] Established 2026-08-05 and NOT to be re-derived: DOM-level support is present — `type` stays `"date"` (not downgraded to `text`), `value` round-trips an ISO date, and `dom.forms.datetime` defaults `true` while `dom.forms.datetime.timepicker` and `.others` default `false`.
- [ ] Whether the `datetimebox.xml` binding ATTACHES is settled. The 2026-08-05 probe could not answer it: it queried anonids on the `<input>`, but the binding is bound to a native-anonymous `<xul:datetimebox>` one level deeper, so the empty result measured the probe. Walk from the input's frame to its NAC child, then query there.
- [ ] If it attaches: the picker is a XUL **popup**, i.e. a separate display root — the same class as the `<select>` dropdown and the `about:addons` tools menu, both of which needed cavekit-offscreen-rendering.md R7's compositing and input routing before they worked. It must composite and be operable, or be routed card-side the way `<select>` was.
- [ ] Measured 2026-08-05: tapping a date field opened NO popup (`popups=0` before and after). Either that is fixed, or date/time input is turned OFF at the pref and the reason recorded — **a field that looks editable and cannot be edited is the worst of the three outcomes.**
**Dependencies:** R1, cavekit-offscreen-rendering.md (R7), cavekit-input-bridging.md (R6)

### R3: Text-field editing keys come from the GRE, not from a hand-rolled map
**Description:** `platformHTMLBindings.xml` already maps editing keys to editor commands; we should stop duplicating it.
**Acceptance Criteria:**
- [ ] `platformHTMLBindings.xml` ships in the dist and carries the standard editing set — **verified 2026-08-05: 102 handlers**, mapping `VK_LEFT`/`VK_RIGHT`/`VK_UP`/`VK_DOWN`/Home/End/Delete and the word-wise and selection variants to `cmd_*` editor commands.
- [ ] These bindings are REACHED for a focused HTML text field. They are not today: `BrowserPageGoanna::keyDown` intercepts whenever `HasFocusedEditable()` is true and drives the editor directly, so the engine's own key handling never runs. That interception was correct when synthesized key events were being dropped by `PuppetWidget` — it is not obviously correct now that they are not (cavekit-input-bridging.md R6, 2026-08-04).
- [ ] The hand-rolled keycode map in `keyDown` — the webOS `0xE0Ax` block, the Apple `0xF7xx` block, the Qt `0x0100001x` block — is reduced to only what the platform genuinely needs (the webOS VKB's own codes), with everything the GRE already handles deleted rather than duplicated.
- [ ] No regression to typing: text entry, Backspace with its accelerate-run behaviour, Enter-to-submit and caret movement all still work on device. **This is the highest-risk item in this domain** — typing is the most-used path in the browser, and the current map exists because of real device measurements.
**Dependencies:** cavekit-input-bridging.md (R2, R6)

### R4: Pages we author use the shipped chrome widgets
**Description:** Stop hand-rolling UI that the GRE already provides.
**Acceptance Criteria:**
- [ ] Any about: page or chrome UI this project ships is built from the shipped bindings — `preferences.xml` (`prefwindow`/`prefpane`/`preference`), `dialog.xml`, `tabbox.xml`, `groupbox.xml`, `listbox.xml`, `scale.xml`, `numberbox.xml`, `spinbuttons.xml` — rather than bespoke markup.
- [ ] Two are already proven to work in THIS embedding and should be preferred where they fit: `richlistbox` (drives `about:addons`) and `tree` (drives `about:config`, including row selection and value editing).
- [ ] A binding that does NOT work offscreen is recorded in `../impl/impl-gre-widget-inventory.md` with what failed, so the next page does not rediscover it.
**Dependencies:** cavekit-preferences-ui.md (R1)

### R5: Non-blocking messages have somewhere to go
**Description:** `notification.xml` (`notificationbox`) gives in-content message bars.
**Acceptance Criteria:**
- [ ] A `notificationbox` renders and is dismissable in a chrome page in this embedding.
- [ ] It is used where a message does not deserve a modal: "add-on installed", "cookies cleared", "this page tried to open a popup". Today the ONLY way to tell the user anything is `msgDialog*`, which BLOCKS the daemon until the card answers (cavekit-browser-services.md R3) — a heavy price for an informational line, and a deadline risk for anything the user ignores.
**Dependencies:** R4

### R6: The find bar is the GRE's, and the real blocker is named
**Description:** `findbar.xml` is a complete find UI we do not use.
**Acceptance Criteria:**
- [ ] Recorded, so nobody mistakes the UI for the problem: `findbar.xml` ships and provides the whole find-in-page interface, but it drives `nsIWebBrowserFind`/`nsITypeAheadFind` — and `FindNext` SIGSEGVs in this offscreen configuration because it dereferences a frame-selection controller the offscreen browser never sets up (re-tested 2026-08-04, `find_test` exit 139). **The missing piece is the selection controller, not the interface.**
- [ ] Once an offscreen-safe selection controller exists (cavekit-ui-shell.md R4), the find UI is taken from `findbar.xml` rather than rebuilt — and the binding doubles as a ready-made way to exercise that path while fixing it.
**Dependencies:** cavekit-ui-shell.md (R4)

### R7: Anything disabled is disabled on purpose
**Description:** No half-working web-platform features.
**Acceptance Criteria:**
- [ ] Every content-facing capability in this domain is either working, or turned off at a pref with the reason recorded in `packaging/prefs/jihad-platform-prefs.js` next to the value.
- [ ] The prefs file stays the single shared source appended by BOTH builds, so desktop and device cannot disagree — the rule established when `dom.w3c_touch_events.enabled` was found off on a touchscreen-only device (cavekit-input-bridging.md R3).
**Dependencies:** R1, R2

## Out of Scope
- Bindings that cannot apply to this embedding — `browser.xml`, `remote-browser.xml` (we ARE the
  browser; there is no second process), `autocomplete.xml` (the URL bar is card-side in all three
  variants), `editor.xml`, `wizard.xml`, `filefield.xml` (no platform file picker).
- The `<select>` dropdown, which is deliberately NOT a GRE popup here — it is serialized to the
  card (cavekit-ui-shell.md R5), the isis/Atlas model.
- The preferences PAGE itself — cavekit-preferences-ui.md. This domain supplies its parts.

## Cross-References
- See also: `../impl/impl-gre-widget-inventory.md` (the full 49-binding inventory and the
  per-binding verdicts), cavekit-preferences-ui.md (R4's first consumer),
  cavekit-offscreen-rendering.md (R7 — popup compositing, which R2's picker needs),
  cavekit-input-bridging.md (R2/R6 — the key path R3 wants to hand back to the engine),
  cavekit-ui-shell.md (R4 — findInPage, which R6 depends on).

## Changelog
- 2026-08-05: Initial draft, from the user requirement *"bring in all useful widgets as goals"*
  after an inventory found 49 shipped bindings and only a handful in use. The four CONTENT-facing
  bindings are the reason this is a domain rather than a checklist: `videocontrols`, `datetimebox`
  and `platformHTMLBindings` apply to every website, and no requirement anywhere mentioned them.
  R3 is the one with teeth — the GRE already maps 102 editing-key handlers that our own `keyDown`
  currently prevents from ever running.
