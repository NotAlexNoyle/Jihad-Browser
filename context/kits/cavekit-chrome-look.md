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
- [ ] On the settings page, about:personas and about:addons, buttons take the style of the variant
  showing them: Enyo 1's in the Enyo variant, Mochi's in the Mochi variant, Mojo's in the Mojo
  variant (shape, colours, pressed and disabled looks), at a finger's size (at least 44 px tall).
  *(Device, all three.)*
- [ ] The style is chosen by the variant the page's engine serves, not by the user agent or the
  screen size; a page opened somewhere else (the desktop build) keeps a plain default.
- [ ] The styles are drawn with CSS; nothing is copied from a framework without a licence that
  allows it, and anything borrowed is credited.
**Dependencies:** cavekit-preferences-ui.md R8, cavekit-personas-ui.md R8

### R2: about:addons in the webOS style
**Description:** The add-ons manager looks like the rest of Jihad's pages: the pale grey page,
charcoal bars, white rounded cards, finger-sized controls, Prelude where the device has it.
**Acceptance Criteria:**
- [ ] Every category (Get Add-ons, Extensions, Personas, Plugins, Language packs when present), the
  detail view, the search field, the gear menu and Install from file… take the look, in all three
  variants. *(Device, all three.)*
- [ ] Every function still works: install, enable, disable, remove, Undo, preferences, the update
  checks, a plugin's state menu, the Personas pane's Wear / Take off / Install from file.
  *(Device.)*
- [ ] Nothing in UXP's own files is edited: the look is a style sheet Jihad adds to the page.
**Dependencies:** cavekit-addons-extensions.md R2, cavekit-personas-ui.md R1

### R3: Jihad's pages move like web pages
**Description:** A flick on about:personas (and the settings page) carries on and slows down as on
any web page; it does not stop dead.
**Acceptance Criteria:**
- [ ] A flick up or down on about:personas and on the settings page coasts, in both directions and
  at both ends of the page, as it does on a web page in the same card. *(Device: the card's own
  flick path; a finger's feel is the owner's to judge.)*
- [ ] The cause is found and stated, and the fix is on the pages or in the engine, not a different
  scroller for these pages only.
**Dependencies:** cavekit-input-bridging.md

## Out of Scope
- The cards' own toolbars and menus (each shell's kit).
- The discover pane's content, which the add-on site serves (cavekit-addons-site.md).

## Cross-References
- cavekit-preferences-ui.md R8, cavekit-personas-ui.md R8, cavekit-addons-extensions.md R2.

## Changelog
- 2026-10-12: Created from the owner's asks (per-shell buttons) and the server agent's work order
  (about:addons in the webOS style, about:personas' scrolling).
