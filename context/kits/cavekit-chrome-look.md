---
created: "2026-10-12"
last_edited: "2026-10-12"
complexity: medium
---

# Cavekit: The Look and Feel of Jihad's Own Pages

## Scope
The engine shows a few pages of its own inside a card: the settings page (about:settings,
cavekit-preferences-ui.md), the Persona gallery (about:personas, cavekit-personas-ui.md R8) and the
add-ons manager (about:addons, cavekit-addons-extensions.md R2). Each variant's card is written in its
own framework — Enyo 1, Enyo 2 / Mochi, Mojo — with its own buttons, and these pages should look like
part of the card they are shown in, and move under a finger like any web page.

The owner, 2026-10-12: "buttons in mochi should be mochi style, buttons in enyo should be enyo style,
buttons in mojo should be mojo style" (on all Jihad pages: settings, about:personas and the restyled
about:addons); and, through the server agent's work order (PROMPT-PC.md, sections 4 and 5),
"about:personas has no scroll momentum" and "about:addons in the webOS style".

## Requirements

### R1: Each shell's own buttons
**Description:** A button on one of Jihad's pages looks like a button of the card it is in.
**Acceptance Criteria:**
- [x] On the settings page, about:personas and about:addons, buttons take the style of the variant
  showing them: Enyo 1's in the Enyo variant, Mochi's in the Mochi variant, Mojo's in the Mojo
  variant (shape, colours, pressed and disabled looks), at a finger's size (at least 44 px tall).
  *(Device, all three: Enyo's dark pill, Mochi's flat buttons, Mojo's glossy pill — its dark-scene
  look, since its light pill clashed with these dark pages — on all three pages, about:addons'
  toolbar buttons and dropdowns included.)*
- [x] The style is chosen by the variant the page's engine serves (its upstart job's JIHAD_BS_NAME,
  read before the page is drawn), not by the user agent or the screen size; elsewhere (the desktop
  build) the pages keep their own look.
- [x] The styles are drawn with CSS from the shells' looks; Enyo 1's and Mojo's framework files (Palm's,
  on the device only) are not copied.
**Dependencies:** cavekit-preferences-ui.md R8, cavekit-personas-ui.md R8

### R2: about:addons in the webOS style
**Description:** The add-ons manager looks like the rest of Jihad's pages: the settings page's dark
look (near-black page, charcoal bars, dark rounded cards, the blue-grey of the chosen pane),
finger-sized controls, Prelude where the device has it. (The server agent's work order asked for the
site's pale grey look; beside Jihad's dark pages it clashed — the owner, 2026-10-12.)
**Acceptance Criteria:**
- [x] Every category (Get Add-ons, Extensions, Personas, Plugins, Language packs when present), the
  detail view, the search field, the gear menu and Install from file… take the look, in all three
  variants. *(Device, all three.)* The look is Jihad's dark one, as on the settings page and
  about:personas: a light webOS look was tried first and clashed (the owner, 2026-10-12); every text
  is light; the site's new Get Add-ons pane is darkened inside it (only that pane).
- [~] Every function still works: install, enable, disable, remove, Undo, preferences, the update
  checks, a plugin's state menu, the Personas pane's Wear / Take off. *(Device: Wear, Take off,
  Uninstall, the gallery link and the plugin menu drawn and reached; the rest is the manager's own
  code under a style sheet — not each exercised.)*
- [x] Nothing in UXP's own files is edited: the look is a style sheet Jihad adds to the page.
- [x] An add-on's preferences fit the card's width: each is a block of its own (title, description,
  then its control), text fields are dark with light text, and nothing is cut off on the right or
  needs a sideways scroll. *(Device, Mochi, Palefill's Preferences, 2026-10-12: before, its
  controls ran past the right edge and the card zoomed out to 0.78 while the page loaded; after,
  every control and the homepage address inside the page, no sideways scrollbar.)* (The owner:
  "extension settings dont appear correctly (cut off)".)
- [~] A list longer than the card (Extensions, Plugins, search results) can be scrolled: the engine
  gives a XUL page no scroller and the card scrolls only pages, so the page's root grows to the
  list's whole length and the card pans it. *(Device: the daemon reports the taller page; the pan
  needs a finger and a long list — the owner's test.)*
**Dependencies:** cavekit-addons-extensions.md R2, cavekit-personas-ui.md R1

### R3: Jihad's pages move like web pages
**Description:** A flick on about:personas (and the settings page) carries on and slows down as on
any web page; it does not stop dead.
**Acceptance Criteria:**
- [~] A flick up or down on about:personas and on the settings page coasts, in both directions and
  at both ends of the page, as it does on a web page in the same card. *(Built; a flick needs a
  finger — scripted pointer events do not reach the plugin — so the owner judges it.)*
- [~] The cause is found and stated, and the fix is in the engine and the shells, not a different
  scroller for these pages only (the likeliest causes, from the code; a finger confirms): Jihad's pages are drawn at zoom 1, the only zoom at which the daemon
  echoes the engine's scroll position to the card, and an echo resets the card's kinetic scroller;
  the daemon now echoes nothing while the card is streaming positions (within 300 ms of the last).
  Mochi never handed a flick to the adapter at all: LunaSysMgr gives a card its gestures by calling
  `Mojo.handleGesture(type, properties)`, which Enyo 1 defines (compatibility/webosGesture.js) and
  Enyo 2 does not, so every flick in Mochi was dropped; Mochi now defines it the same way and hands
  a flick that began on the web view to the adapter, in whole numbers (the adapter refuses others).
  (The first attempt, a DOM `flick` listener, could never fire — inspector round 20.)
**Dependencies:** cavekit-input-bridging.md

### R4: The keyboard on Jihad's pages
**Description:** Jihad's own pages open with the whole page in view, and a field the keyboard has
left behind does not keep its caret (the owner, 2026-10-12: "when about:settings or about:personas
is open the keyboard should be dismissed at first so the whole page is visible"; "when the
keyboard is closed in about:addons the cursor in the search panel is still there").
**Acceptance Criteria:**
- [x] Opening about:settings, about:preferences, about:personas or about:addons from the address
  field or the app menu takes the focus off the address field, so the keyboard goes down; opening
  any other page leaves it where it was. *(Device, all three, 2026-10-12, the address field
  focused by the card's own handler: Enyo and Mojo focus on BODY after a Jihad page, INPUT after
  example.org; Mochi's and Mojo's windows grew back from 705 to 996 px — the keyboard went down.)*
- [~] When the keyboard goes down while a field in the page has the focus, the field gives up the
  focus (no caret left), and the shell is told the editor lost it. *(Built in the daemon: a window
  that grows back by a keyboard's height at the same width clears the page's focus. A scripted tap
  focuses the field but raises no keyboard, so only a finger can test it — the owner's.)*
- [x] A tap on the address field selects the whole address, in all three variants (the owner's
  choice, 2026-10-12: "do option 1 for all 3 variants"), since Select All cannot be added to the
  Cut/Copy bubble — the system draws it (none of the shells, Enyo 1 or LunaSysMgr's sources do).
  Enyo: `selectAllOnFocus` (already there); Mochi: Enyo 2's `selectOnFocus` (new); Mojo:
  `focusSelectMode` (already there). *(Device, 2026-10-12: the field refocused with a caret at 3
  selects 0–20 of 20 in Enyo and Mochi while a bare input beside it keeps its caret; Mojo, blurred
  then focused, selects 0–20 of 20, where without the mode an API focus puts the caret at the end.
  A finger's tap is the owner's to confirm.)*
**Dependencies:** cavekit-input-bridging.md

## Out of Scope
- The cards' own toolbars and menus (each shell's kit).
- The discover pane's content, which the add-on site serves (cavekit-addons-site.md).

## Cross-References
- cavekit-preferences-ui.md R8, cavekit-personas-ui.md R8, cavekit-addons-extensions.md R2.

## Changelog
- 2026-10-12: Created from the owner's asks (per-shell buttons) and the server agent's work order
  (about:addons in the webOS style, about:personas' scrolling).
