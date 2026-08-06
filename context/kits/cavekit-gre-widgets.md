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
- [x] Established 2026-08-05 and NOT to be re-derived: **as the engine ships**, DOM-level support is present — `type` stays `"date"` (not downgraded to `text`), `value` round-trips an ISO date, and `dom.forms.datetime` defaults `true` while `dom.forms.datetime.timepicker` and `.others` default `false`. **This describes the UNCONFIGURED engine; it is no longer what Jihad ships** — see the last criterion, which turns the feature off, after which `type` is `text`. (The two read as a contradiction until 2026-08-06; they are sequential, not simultaneous.) *(Re-confirmed on device 2026-08-05 from a probe page that prints `type` and `value` back: `type=date | value=2026-08-05`, rendered as a segmented `08 / 05 / 2026` field.)*
- [x] Whether the `datetimebox.xml` binding ATTACHES is settled. *(Settled by decision rather than by inspection — see the criterion below: the feature is OFF, so the binding cannot attach, and the NAC walk is moot. Recorded in `../impl/impl-gre-widget-inventory.md` so the next person does not re-run the probe.)*
- [x] If it attaches: the picker is a XUL **popup** … It must composite and be operable, or be routed card-side the way `<select>` was. *(NOT APPLICABLE now the feature is off. Retained because it is the condition to re-check if `dom.forms.datetime` is ever turned back on.)*
- [x] Measured 2026-08-05: tapping a date field opened NO popup (`popups=0` before and after). Either that is fixed, or date/time input is turned OFF at the pref and the reason recorded — **a field that looks editable and cannot be edited is the worst of the three outcomes.** *(**Side effect, recorded 2026-08-06:** with `dom.forms.datetime`, `.timepicker` and `.others` all
false, `HTMLInputElement::ValueAsDateEnabled` is false, so the WebIDL member **`valueAsDate`
disappears from `HTMLInputElement`** — a content-visible API change, not just a widget change. A
site that feature-detects it takes a different path. Also, once the element is a plain text input
there is NO sanitisation, so a site reading `.value` expecting `YYYY-MM-DD` gets whatever was typed;
"the value survives" was only ever proven for one ISO string.)* *(RESOLVED 2026-08-05 by turning it off. `packaging/prefs/jihad-platform-prefs.js` sets `dom.forms.datetime` false with the reason next to the value; device-verified from the same probe page, which now prints `type=text | value=2026-08-05` and renders the date input identically to a plain text input. The value survives the downgrade, and the VKB and the engine's editing keys already handle a text field.)*
**Dependencies:** R1, cavekit-offscreen-rendering.md (R7), cavekit-input-bridging.md (R6)

### R3: Text-field editing keys come from the GRE, not from a hand-rolled map
**Description:** `platformHTMLBindings.xml` already maps editing keys to editor commands; we should stop duplicating it.
**Acceptance Criteria:**
- [x] `platformHTMLBindings.xml` ships in the dist and carries the standard editing set — **verified 2026-08-05: 102 handlers**, mapping `VK_LEFT`/`VK_RIGHT`/`VK_UP`/`VK_DOWN`/Home/End/Delete and the word-wise and selection variants to `cmd_*` editor commands.
- [ ] These bindings are REACHED for a focused HTML text field. They are not today: `BrowserPageGoanna::keyDown` intercepts whenever `HasFocusedEditable()` is true and drives the editor directly, so the engine's own key handling never runs. That interception was correct when synthesized key events were being dropped by `PuppetWidget` — it is not obviously correct now that they are not (cavekit-input-bridging.md R6, 2026-08-04).
- [ ] The hand-rolled keycode map in `keyDown` — the webOS `0xE0Ax` block, the Apple `0xF7xx` block, the Qt `0x0100001x` block — is reduced to only what the platform genuinely needs (the webOS VKB's own codes), with everything the GRE already handles deleted rather than duplicated.
- [ ] No regression to typing: text entry, Backspace with its accelerate-run behaviour, Enter-to-submit and caret movement all still work on device. **This is the highest-risk item in this domain** — typing is the most-used path in the browser, and the current map exists because of real device measurements.
**Dependencies:** cavekit-input-bridging.md (R2, R6)

### R4: Pages we author use the shipped chrome widgets
**Description:** Stop hand-rolling UI that the GRE already provides.
**Acceptance Criteria:**
- [~] Any about: page or chrome UI this project ships uses what the platform provides rather than re-implementing it — the shipped bindings (`dialog.xml`, `tabbox.xml`, `groupbox.xml`, `listbox.xml`, `scale.xml`, `numberbox.xml`, `spinbuttons.xml`) where the page is a chrome window, and native HTML form controls where it is an in-content document. **Bespoke re-implementations of either are what this forbids.**

  **`preferences.xml` was struck from this list 2026-08-05, after building the first consumer.** `prefwindow` is a *window* binding — dialog buttons, `instantApply` semantics, opened as a chrome dialog — and this embedding has no chrome window to open one in, which is the same class of assumption that broke `about:addons`. `about:preferences` is therefore in-content HTML (Basilisk's shape, Pale Moon's panes) using native `<input>`/`<select>`. That is NOT hand-rolling: `<select>`'s popup already round-trips to the card through `msgPopupMenuShow` and is device-proven in all three shells, which is a better position than an unproven XUL `<menulist>`. See cavekit-preferences-ui.md R1 for the full decision.
- [ ] Two are already proven to work in THIS embedding and should be preferred where they fit: `richlistbox` (drives `about:addons`) and `tree` (drives `about:config`, including row selection and value editing).
- [x] A binding that does NOT work offscreen is recorded in `../impl/impl-gre-widget-inventory.md` with what failed, so the next page does not rediscover it. *(`datetimebox.xml` is the first entry with a verdict: its picker never opened on device and the feature is now off at the pref — recorded there with the measurement and the way back.)*
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
- [x] Recorded, so nobody mistakes the UI for the problem. **The diagnosis this criterion used to carry was WRONG and is corrected here** — it said `FindNext` SIGSEGVs because it dereferences "a frame-selection controller the offscreen browser never sets up", and that sent this port down a two-week dead end. The real cause: `nsWebBrowserFind::SearchInFrame` runs a same-origin check through `nsContentUtils::SubjectPrincipal()`, which opens with `MOZ_CRASH("Accessing the Subject Principal without an AutoJSAPI on the stack is forbidden")` when there is no JSContext. An EMBEDDER calls `FindNext` from C++ with no script on the stack, so it aborts — and `MOZ_CRASH` presents as SIGSEGV at address 0, which is exactly the fault that was misattributed. **The selection machinery was fine all along; the crash happened before any of it.** **Now proven by an automated gate (2026-08-06).** `render/goanna/test/find_test.cpp` used to PRINT
the result of `page.Find(...)` and assert nothing, with `ok = freezeOK` and a comment restating the
old wrong expectation — so the binary could not fail because of find, whatever find did. It now
asserts BOTH directions and gates the exit code on them. Run on the desktop harness:
`[find] findString hit=1 miss=0 ok=1` — a present string is found, an absent one is not. **This is
the first repeatable, hardware-free proof that find works**, and it replaces the device anecdote.
*(Same run reports `freezeOK=0` — `painted load=0 freeze=0 thaw=0`. That half was already failing
before this change was made and is NOT find-related; it is most likely the container having no
display/shm rather than a real regression, but it is unverified either way and is tracked in
cavekit-offscreen-rendering.md, not here. The binary exits 4 on that half.)*
Fixed by consulting the subject principal only when a JSContext exists (patch 0015, `third_party/uxp/embedding/components/find/nsWebBrowserFind.cpp`, which carries the full explanation inline). Find-in-page now works.
- [x] The find UI is taken from `findbar.xml` rather than rebuilt. *(RESOLVED AS OUT OF SCOPE, and the premise is void — it was conditioned on "once an offscreen-safe selection controller exists", which was never the blocker. `findbar.xml` is chrome-WINDOW UI, the same category as `browser.xml` and `autocomplete.xml` already excluded above: there is no chrome window here, and the find bar is card-side in all three variants for the same reason the `<select>` dropdown is serialized to the card — the isis/Atlas model, cavekit-ui-shell.md R5. Moved to Out of Scope below.)*
**Dependencies:** cavekit-ui-shell.md (R4)

### R7: Anything disabled is disabled on purpose
**Description:** No half-working web-platform features.
**Acceptance Criteria:**
- [ ] Every content-facing capability in this domain is either working, or turned off at a pref with the reason recorded in `packaging/prefs/jihad-platform-prefs.js` next to the value.
- [~] The prefs file stays the single shared source appended by BOTH builds, so desktop and device cannot disagree — the rule established when `dom.w3c_touch_events.enabled` was found off on a touchscreen-only device (cavekit-input-bridging.md R3). *(Verified 2026-08-05: `build/desktop/build-goanna.sh` (PLATFORM_PREFS) and `build/webos-oe/make-device-bundle.sh` both append `packaging/prefs/jihad-platform-prefs.js`, and the datetime decision above went into that one file rather than into either build.) **Downgraded on review 2026-08-06:** true of `jihad-platform-prefs.js`, but NOT of the whole pref surface — `make-device-bundle.sh` also appends ~16 device-only prefs inline (the low-RAM tuning block), three of which back `about:preferences` rows, so those rows read "not available" on desktop. And the desktop build installs neither the `prefsui` package nor the about: module, so `about:preferences` does not exist there at all.*
**Dependencies:** R1, R2

## Out of Scope
- Bindings that cannot apply to this embedding — `browser.xml`, `remote-browser.xml` (we ARE the
  browser; there is no second process), `autocomplete.xml` (the URL bar is card-side in all three
  variants), `editor.xml`, `wizard.xml`, `filefield.xml` (no platform file picker).
- The `<select>` dropdown, which is deliberately NOT a GRE popup here — it is serialized to the
  card (cavekit-ui-shell.md R5), the isis/Atlas model.
- `findbar.xml`, for the same reason (moved here from R6, 2026-08-05). It is chrome-window UI;
  the find bar is card-side in all three variants. The engine side of find — `FindNext` — works
  as of patch 0015; what was recorded as its blocker was a misdiagnosis, see R6.
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
