---
created: "2026-10-08"
last_edited: "2026-10-08"
complexity: medium
---

# Cavekit: Managing and Installing Personas

## Scope
A **Persona** (the engine calls it a lightweight theme) gives the browser's toolbar and buttons a
picture and colours. Personas already worked in all three shells: a Persona site's "Add", an
addons.mozilla.org Persona package, the confirm, the painted toolbar and buttons
(cavekit-addons-extensions.md R10). What was missing was a place a person would look for them and
enjoy using. The owner, 2026-10-08: **"make it user friendly to manage and install personas,
using about:addons as a base."**

`about:addons` is that place. The engine's add-ons manager already listed lightweight themes in a
**Themes** category. Measured on the device the same day, it was a desktop page on a tablet: a
generic icon instead of the Persona's art, buttons sized for a mouse pointer, no way back to "no
Persona" except disabling whichever was on, no way to get one except finding a site, and nothing
saying what a Persona is.

This kit makes that category the home of Personas by decorating the add-ons manager's own page:
the same manager, the same add-on model, the same install path R10 already secured. There is no
second manager; Settings → Appearance is a summary that links here.

**Naming (owner, 2026-10-08: "call themes Personas consistently throughout the user interface and
documentation"):** every word a person reads says "Persona": the category, the pane, the confirm,
the toasts, Settings, the README. Code identifiers keep the engine's own names.

The owner shaped it further during the build, the same day: "try and wear seem duplicative" (R3),
"author and version fields should be shown for all personas" and "default version 1.0" (R1), "use
Uninstall instead of Remove" (R2), "add fruiter aero style themes" (R7), "blue nebula looks like
it isnt working properly" (R4), "dark fox preview in about:addons not working" (R1), "the aero
themes dont display consistently in the persona preview in about:addons like the others" (R7).

Reference: `render/goanna/components/jihadPersonas.js` (install, conversion, the pane decorator,
the bundled set), UXP `toolkit/mozapps/extensions/` (`content/extensions.xul|js|xml`,
`LightweightThemeManager.jsm`), the shells' `applyTheme`s, `render/goanna/personas/` (the
bundled set and the script that draws it), `render/goanna/test/personas/`.

## Requirements

### R1: Every Persona shows its art, its author and its version
**Description:** In the Personas pane each Persona looks like itself, not like a generic add-on row.
**Acceptance Criteria:**
- [x] Each row shows the Persona: its picture over its accent colour, the name in its text colour
  on a pane that keeps it readable over any picture. *(Device, 2026-10-08: Enyo, Mochi and Mojo.)*
- [x] Every row shows the author ("by …", or "Author unknown") and the version (1.0 when the
  Persona names none). *(Device, all three.)*
- [x] The Persona worn is marked "In use" and listed first. *(Device, all three.)*
- [x] A Persona installed while the pane is open gets its picture too: the row is redecorated once
  its binding attaches. *(Device, Enyo: Dark Fox installed from file, then shown with its art.)*
- [x] A Persona whose picture cannot be drawn still shows its colours, name, author and version.
  *(Device, Enyo: the full-size Blue Nebula before R4's fix.)*
**Dependencies:** cavekit-addons-extensions.md R10

### R2: Wearing, the default look, uninstalling — with a finger
**Description:** Changing the look is one finger-sized tap.
**Acceptance Criteria:**
- [x] **Wear** puts a Persona on, and every open card of the variant repaints. *(Device: Enyo,
  Mochi and Mojo, from the pane; the toolbar or Mojo's circles change.)*
- [x] A **Default look** entry sits above the list, "In use" when no Persona is worn, with a Wear
  that takes the Persona off. *(Device, Enyo: In use after the worn Persona was uninstalled.)*
- [~] Default look's own Wear, tapped. *(Code path is the manager's `currentTheme = null`, as
  Settings' old Remove was; not tapped on the device yet.)*
- [x] **Uninstall** (the manager's Remove, relabelled on every add-on) forgets a Persona and deletes
  the picture the browser unpacked for it. *(Device, Enyo: the old Blue Nebula's file removed.)*
- [x] Every control is at least 44 px tall and 64 px wide. *(Measured: the row buttons' box is 44 px.)*
**Dependencies:** R1

### R3: A safe change
**Description:** Changing the look can always be taken back. (Superseded design: a separate Try
button with Keep / Undo. The owner found Try and Wear duplicative; Wear plus Undo gives the same
safety with one button.)
**Acceptance Criteria:**
- [x] After any change of look, the pane shows "Now wearing …" with **Undo** for 30 s; Undo puts
  back the Persona (or the default look) worn before. *(Device: Enyo, Wear then Undo, back to the
  previous Persona; Mochi and Mojo show the bar.)*
- [x] Undo's own change is not itself undoable (no ping-pong).
**Dependencies:** R2

### R4: Installing, from where people find Personas
**Description:** Getting a new Persona does not require knowing which website still hosts them.
**Acceptance Criteria:**
- [x] **Install from file…** lists the `.xpi` files in `/media/internal/downloads` and
  `/media/internal`. Tapping one installs it through the same conversion and confirm as a download
  (R10 limits apply). *(Device, Enyo: an AMO Persona put in downloads → confirm → worn.)* The
  engine's own gear-menu entry needs a file picker this embedding does not have.
- [ ] **Get Personas** opens addons.mozilla.org's listing in the card, and a Persona installed
  there appears in the pane, worn. *(Built; the AMO page in this engine not yet tried.)*
- [x] Persona sites using the classic `InstallBrowserTheme` event (Firefox 52 / Pale Moon /
  Basilisk) keep working. *(cavekit-addons-extensions.md R10, device 2026-10-06; Mochi's prompt
  re-checked 2026-10-08 with the new wording.)*
- [x] Both header forms AMO serves are accepted: `theme_frame` / `headerURL`, and the first of
  `additional_backgrounds`.
- [x] A wallpaper-sized picture (taller than 400 px; AMO ships 3840×2160) is kept 1600 px wide at
  install (decoded straight at that size; full size it exceeds the engine's 32 MB image cache and
  never draws), and the cards fit it to the bar instead of showing its corner. *(Device, Enyo:
  Blue Nebula re-installed as 1600×900; the bar shows the nebula, the pane its picture.)*
**Dependencies:** cavekit-addons-extensions.md R10

### R5: It explains itself
**Description:** Someone who has never heard of Personas understands them from the pane.
**Acceptance Criteria:**
- [x] The pane opens with a sentence on what a Persona is. *(Device, all three.)*
- [~] With no Personas installed, it also says how to get one (Get Personas, Install from file).
  *(Code; every profile now holds the bundled set, so the empty state needs them uninstalled.)*
- [x] Settings → Appearance shows the Persona worn and how many are installed, with **Manage
  Personas**, which opens the pane. *(Device, Mochi. Found on the way: the daemon re-drove the
  page's load of about:addons by its resolved file address, which the engine refuses as XUL loaded
  from a file; it now re-drives an about: page by its original address.)*
**Dependencies:** R1, R4

### R6: The same in every shell, verified
**Description:** All three variants get the same pane and the same behaviour.
**Acceptance Criteria:**
- [x] Enyo, Mochi and Mojo each show the Personas pane and repaint on Wear. *(Device, 2026-10-08.)*
- [x] Screenshots of the pane and of the bars, with no Persona and with several. *(README, `docs/screenshots/`.)*
- [ ] A finger on the device: wear, undo, uninstall, install from file. [human at the device]
**Dependencies:** R1–R5

### R7: Personas that come with Jihad
**Description:** A Frutiger Aero set (owner: "add fruiter aero style themes") ships with the
browser: glossy water, sky, a green hill, light ribbons, lime orbs, drawn for this project.
**Acceptance Criteria:**
- [x] Five Personas, drawn from gradients and shapes by `render/goanna/personas/make-aero-personas.py`
  (so the art is the project's own), 1600×160, interest in the top right where every bar shows it.
- [x] They join each profile's Persona list once, not worn, as ordinary rows: the same picture,
  Wear, Uninstall, author ("Jihad Browser") and version ("1.0") as every other Persona. One the
  user uninstalls stays uninstalled. *(Device, all three.)*
- [x] A new edition of the set replaces its own older pictures (matched by name and author), and
  carries the choice over if the older one was worn. *(Device, Mochi: the earlier Aero Meadow
  retired, its picture deleted.)*
**Dependencies:** R1, R2

## Out of Scope
- Full WebExtension themes (images per toolbar part, the `theme` API, dynamic themes). This engine
  installs no WebExtensions; theme-only packages are converted to Personas (R10).
- Animated Personas beyond their first frame.
- Theming the engine's own pages (`about:addons` itself keeps the system look).

## Cross-References
- cavekit-addons-extensions.md R10: what a Persona is here, install security, persistence.
- cavekit-preferences-ui.md R2/R5: Settings' Appearance section is a summary.
- cavekit-tabbed-browsing.md R1: tabs, when built, wear the Persona too.

## Changelog
- 2026-10-08 (later): Built and device-verified in all three shells (R1–R5, R7), with the
  owner's same-day direction: one Wear with Undo (R3 rewritten), author and version on every
  row, Uninstall, "Persona" everywhere, the wallpaper fit, the bundled Frutiger Aero set (R7,
  new). Open: Get Personas against AMO, Default look's own Wear tapped, the empty state, and
  the owner's finger pass.
- 2026-10-08: Created. Baseline measured on the device: the Themes pane listed Personas with
  Enable / Disable / Remove and a generic icon; nothing else.
