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
- [ ] A theme-only package with `colors` and no usable picture is offered with the usual confirm
  (naming its site) instead of failing as corrupt, and on yes becomes the Persona worn.
- [ ] Every shell paints it in its own colours: the bar in the theme's `frame` colour (with its
  `toolbar` colour where the theme gives one), the bar's text and glyphs following its text colour,
  with no picture of any kind drawn over them. *(Device, all three: screenshot.)*
- [ ] Its row in the Personas pane and its card in about:personas show those colours, with its
  name, author and version, like any other Persona. *(Device, one shell.)*
- [ ] Uninstalling it leaves nothing of it in the profile. *(Device: the profile's theme files and
  prefs checked before and after.)*
- [ ] A package whose colours are all unusable (missing, malformed or not CSS colours) is still
  refused, with the message that it is not a Persona Jihad can wear.
**Dependencies:** cavekit-personas-ui.md R4

### R2: Layered pictures
**Description:** A theme made of several pictures (`additional_backgrounds`, the Alpenglow style)
shows all of them, placed and tiled as the theme says, not only the first.
**Acceptance Criteria:**
- [ ] Every picture of `theme_frame` and `additional_backgrounds` (up to a stated limit, at least 6)
  is kept and drawn, `theme_frame` on top and then the others in their order, the first listed
  uppermost, over the frame colour.
- [ ] Each picture is placed by its `additional_backgrounds_alignment` entry and repeated by its
  `additional_backgrounds_tiling` entry (`no-repeat`, `repeat`, `repeat-x`, `repeat-y`); a missing
  entry means right top and no repeat, as in Firefox. A value outside CSS position syntax or the
  four tiling words is ignored, never passed through.
- [ ] Every shell draws the layers in its bar, and the Personas pane and about:personas show them.
  *(Device, all three: screenshot of a test theme drawn for this project with three layers in
  different corners and one tiled.)*
- [ ] Each layer is bounded as the single header is today (size cap, wallpapers shrunk, no
  symlinks or paths outside the package, nothing kept until the user says yes) and the total held
  for one Persona is bounded too. A layer in a format the engine cannot draw (SVG) is skipped; the
  Persona is still offered if anything drawable is left.
- [ ] Classic Personas and single-picture themes look exactly as before. *(Device: the bundled set
  unchanged in all three.)*
**Dependencies:** R1 (shares the "more than the engine keeps" storage)

### R3: More colour slots
**Description:** A theme's other colours reach the parts of each shell they were made for, so a
palette theme does not stop at the toolbar.
**Acceptance Criteria:**
- [ ] These slots are read from a theme package and kept with its Persona (others are ignored):
  `toolbar`, `toolbar_text` (alias `bookmark_text`), `toolbar_field`, `toolbar_field_text`,
  `toolbar_field_border`, `toolbar_field_focus`, `toolbar_field_text_focus`, `popup`, `popup_text`,
  `popup_border`, `popup_highlight`, `popup_highlight_text`, `icons`, `ntp_background`, `ntp_text`.
  Every value is a CSS colour or it is dropped.
- [ ] The address field takes `toolbar_field`, `toolbar_field_text` and `toolbar_field_border`, and
  the `_focus` pair while it is being typed in, in all three shells.
- [ ] The browser's own menus (the app menu, the link and image menus, the page menu) take
  `popup`, `popup_text`, `popup_border`, and `popup_highlight` / `popup_highlight_text` for the item
  under the finger, in every shell where the shell draws them itself.
- [ ] A shell's second bar (Mojo's command menu, Mochi's lower bar where shown) takes `toolbar` and
  `toolbar_text`; the glyph set (light or dark) follows `icons` when given, else the text colour.
- [ ] The start page, in the shells that have one, takes `ntp_background` and `ntp_text`.
- [ ] Wearing a classic Persona, or Default look, puts every one of those parts back as it was.
  *(Device, all three: screenshots with a palette theme, with a classic Persona, and with none.)*
- [ ] A part the theme gives no colour for keeps the shell's own look (or the Persona's accent and
  text colours where those already apply today).
**Dependencies:** R1

### R4: The same everywhere a Persona travels
**Description:** The new fields go wherever a Persona already goes.
**Acceptance Criteria:**
- [ ] A card that starts after the Persona was worn (the daemon's getTheme) paints it exactly as a
  running card did.
- [ ] What a card is sent still fits the notification channel; when it would not, the least
  important fields are dropped first (extra colour slots, then extra layers), never the whole theme.
- [ ] Undo, Default look, uninstall and the bundled set's editions handle these Personas as they
  handle classic ones. *(Device, one shell.)*
- [ ] An index Persona (about:personas, the add-on site) may carry the same fields, under the same
  checks (https pictures only).
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
- 2026-10-11: Created from the owner's choice of formats after the Persona-index options.
