---
created: "2026-10-11"
last_edited: "2026-10-11"
complexity: medium
---

# Cavekit: More Persona Formats

## Scope
Jihad installs two kinds of Persona today: the classic one (a header picture, an accent colour and a
text colour: what UXP's engine knows) and a theme-only addons.mozilla.org package whose
`theme_frame`, or first `additional_backgrounds` picture, becomes that header
(cavekit-addons-extensions.md R10, cavekit-personas-ui.md R4). Everything else a theme package can
say is lost, and a package with no picture at all is refused.

The owner, 2026-10-11, after the format survey (`docs/research/persona-archives.md`): support
**colour-only themes** (rated ★★★★☆: three quarters of the users in a sample of addons.mozilla.org's
150 most-used themes, 45 of 54 openly licensed), **layered pictures** (★★★☆☆) and **more colour
slots** (★★★☆☆). Animated and SVG pictures, dark variants, footers, dynamic and complete themes
stay out (see Out of Scope).

The engine's theme manager keeps only its own fields (id, name, headerURL, textcolor, accentcolor,
footerURL, iconURL, previewURL, author, description, homepageURL, updateURL, version) and drops the
rest, and it refuses a Persona without a header picture. What this kit adds has to survive that,
live in the profile beside the Persona, and leave with it when it is uninstalled.

A theme package's own words (Firefox 55–today; key names after Firefox 70, with the older aliases):
`images.theme_frame` (alias `headerURL`), `images.additional_backgrounds[]`,
`properties.additional_backgrounds_alignment[]`, `properties.additional_backgrounds_tiling[]`,
`colors.frame` (alias `accentcolor`), `colors.tab_background_text` (alias `textcolor`), and the
other `colors.*` slots listed under R3.

Reference: `render/goanna/components/jihadPersonas.js` (conversion, `cardTheme`, the pane, the
bundled set), the shells' `applyTheme` (`app/source/ActionBar.js`, `app-mochi/source/JihadBrowser.js`,
`app-mojo/app/assistants/main-assistant.js`), UXP
`toolkit/mozapps/extensions/LightweightThemeManager.jsm` (`MANDATORY`/`OPTIONAL`, `_sanitizeTheme`).

## Requirements

### R1: Colour-only themes
**Description:** A theme package that carries colours and no picture installs as a Persona and
looks like itself: Firefox Color, Colorways and the many palette themes (Catppuccin, Nord, Dracula…).
**Acceptance Criteria:**
- [x] A theme-only package with `colors` and no usable picture is offered with the usual confirm
  (naming its site) instead of failing as corrupt, and on yes becomes the Persona worn. *(Device, all
  three, 2026-10-11.)*
- [x] Every shell paints it in its own colours: the bar in the theme's `frame` colour (with its
  `toolbar` colour where the theme gives one), the bar's text and glyphs following its text colour,
  with no picture of any kind drawn over them. *(Device, all three: a frame-to-toolbar gradient
  stretched over the bar — Enyo measured #3e2164 at its top and #7c3aa8 at its bottom for a
  frame of #3b1d5e and a toolbar of #7a3fa8; inspector 2026-10-11:
  it was drawn as cover, which showed one blended row.)*
- [x] Its row in the Personas pane shows those colours, with its name, author and version, like
  any other Persona. *(Device, Enyo.)* (about:personas lists the Personas that come with Jihad and
  the add-on site's; an installed package is neither.)
- [x] Uninstalling it leaves nothing of it in the profile. *(Device, Enyo: its picture and extras
  file deleted, and no trace in prefs.js once saved.)*
- [~] A package whose colours are all unusable (missing, malformed or not CSS colours) is still
  refused, with the message that it is not a Persona Jihad can wear. *(Code; not exercised on the
  device.)*
**Dependencies:** cavekit-personas-ui.md R4

### R2: Layered pictures
**Description:** A theme made of several pictures (`additional_backgrounds`, the Alpenglow style)
shows all of them, placed and tiled as the theme says, not only the first.
**Acceptance Criteria:**
- [x] Every picture of `theme_frame` and `additional_backgrounds` (up to a stated limit, at least 6)
  is kept and drawn, `theme_frame` on top and then the others in their order, the first listed
  uppermost, over the frame colour. *(6 besides theme_frame. Device, all three.)*
- [x] Each picture is placed by its `additional_backgrounds_alignment` entry and repeated by its
  `additional_backgrounds_tiling` entry (`no-repeat`, `repeat`, `repeat-x`, `repeat-y`); a missing
  entry means right top and no repeat, as in Firefox. A value outside CSS position syntax or the
  four tiling words is ignored, never passed through. *(Device for the valid ones; the rest code.)*
- [x] Every shell draws the layers in its bar, and the Personas pane shows them. *(Device, all
  three: the test theme drawn for this project — a moon at left top, a sun at right top, hills along
  the bottom repeating, dots tiled; Enyo's pane row too, its first picture at its own size since the
  inspector's 2026-10-11 review.)*
- [x] Each layer is bounded as the single header is today (size cap, wallpapers shrunk, no
  symlinks or paths outside the package, nothing kept until the user says yes) and the total held
  for one Persona is bounded too (6 Mpx, checked from each picture's header before it is shrunk or
  decoded). A layer in a format the engine cannot draw (SVG) is skipped; the Persona is still offered
  if anything drawable is left. *(Device, Enyo: `jihad-test-svg.xpi` — "picture h.svg left out: not a
  format this browser draws", then offered through the confirm and worn on its colours alone; a
  10000² PNG refused unread, Codex r13. The rest code.)*
- [x] Classic Personas and single-picture themes look exactly as before. *(Device: the bundled set
  unchanged in all three.)*
**Dependencies:** R1 (shares the "more than the engine keeps" storage)

### R3: More colour slots
**Description:** A theme's other colours reach the parts of each shell they were made for, so a
palette theme does not stop at the toolbar.
**Acceptance Criteria:**
- [x] These slots are read from a theme package and kept with its Persona (others are ignored):
  `toolbar`, `toolbar_text` (alias `bookmark_text`), `toolbar_field`, `toolbar_field_text`,
  `toolbar_field_border`, `toolbar_field_focus`, `toolbar_field_text_focus`, `popup`, `popup_text`,
  `popup_border`, `popup_highlight`, `popup_highlight_text`, `icons`, `ntp_background`, `ntp_text`.
  Every value is one CSS colour or it is dropped, and is passed on as `rgb()`/`rgba()`. *(Device,
  Enyo: an `hsl()` value arrives as rgb(); "red, 0 0 0 2000px black" is dropped — inspector
  2026-10-11: text that merely used colour characters reached the stylesheets.)*
- [x] The address field takes `toolbar_field`, `toolbar_field_text` and `toolbar_field_border`, and
  the `_focus` pair while it is being typed in, in all three shells. *(Device, all three.)*
- [x] The browser's own menus (the app menu, the link and image menus, the page menu) take
  `popup`, `popup_text`, `popup_border`, and `popup_highlight` / `popup_highlight_text` for the item
  under the finger, in every shell where the shell draws them itself. *(Device: Enyo's and Mochi's
  app menus, Mojo's share menu.)*
- [x] A shell's second bar (Mojo's command menu, Mochi's lower bar where shown) takes `toolbar` and
  `toolbar_text`; the glyph set (light or dark) follows `icons` when given, else the text colour.
  *(Device, Mojo; Enyo and Mochi have no second bar.)*
- [x] The start page, in the shells that have one, takes `ntp_background` and `ntp_text`. *(Device:
  Enyo and Mojo on screen; Mochi's computed colour, its overlay not showing over a loaded page.)*
- [x] Wearing a classic Persona, or Default look, puts every one of those parts back as it was.
  *(Device, all three, with a classic Persona.)*
- [x] A part the theme gives no colour for keeps the shell's own look (or the Persona's accent and
  text colours where those already apply today).
**Dependencies:** R1

### R4: The same everywhere a Persona travels
**Description:** The new fields go wherever a Persona already goes.
**Acceptance Criteria:**
- [x] A card that starts after the Persona was worn (the daemon's getTheme) paints it exactly as a
  running card did. *(Device: every test run relaunched the card.)*
- [x] What a card is sent still fits the notification channel; when it would not, the least
  important fields are dropped first (extra colour slots, then extra layers), never the whole theme.
  *(Code.)*
- [~] Undo, Default look, uninstall and the bundled set's editions handle these Personas as they
  handle classic ones; re-installing one under the id already worn repaints the cards. *(Device,
  Enyo: uninstall, and the same picture re-installed with new colours repainted — inspector
  2026-10-11, it did not; Undo, Default look and editions code only.)*
- [~] An index Persona (about:personas, the add-on site) may carry the same colour slots (not
  pictures beyond its header), under the same checks. *(Code.)*
**Dependencies:** R1–R3, cavekit-personas-ui.md R3, R8

## Out of Scope
- Animated pictures beyond their first frame, and SVG pictures (rated ★★☆☆☆: low demand, mostly
  NonCommercial or All Rights Reserved, the cards' WebKit cannot animate APNG).
- `dark_theme` and `color_scheme` (webOS has no dark-mode switch), footers (invisible in Firefox
  since 2014 and dropped by addons.mozilla.org's 2019 migration), dynamic themes (need the
  WebExtension `theme` API, removed from UXP on 2019-02-13), complete themes
  (`docs/research/complete-themes.md`).
- Colours for tabs (`tab_*`): the shells have no tab strip yet (cavekit-tabbed-browsing.md).

## Cross-References
- cavekit-personas-ui.md R1, R3, R4, R8: the pane, Undo, conversion, the gallery.
- cavekit-addons-extensions.md R10: install security for Personas.
- cavekit-addons-site.md: the site's Personas carry the classic fields and may carry these.

## Changelog
- 2026-10-11 (later): Built and device-verified in all three shells; the inspector's review (Codex out
  of credits) fixed the colour-only bar's fit, the pane's layered rows, re-installs under the worn
  id, colour values that were text, and the pixel budget's timing. R1/R2's about:personas card and
  R4's index fields narrowed to what about:personas actually lists.
- 2026-10-11: Created from the owner's choice of formats after the Persona-index options.
