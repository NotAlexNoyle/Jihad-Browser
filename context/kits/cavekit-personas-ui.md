---
created: "2026-10-08"
last_edited: "2026-10-08"
complexity: medium
---

# Cavekit: Managing and Installing Personas

## Scope
Personas (lightweight themes) already WORK in all three shells — a theme site's "Add", an AMO
static-theme package, the confirm, the painted toolbar and buttons, Settings → Appearance
(cavekit-addons-extensions.md R10). What is missing is a place a person would *look* for them and
enjoy using. The owner, 2026-10-08: **"make it user friendly to manage and install personas,
using about:addons as a base."**

`about:addons` is that place. The engine's own add-ons manager already lists lightweight themes
under **Themes** (the LightweightThemeManager's add-on provider): name, author, Enable / Disable /
Remove, a "More" link. Measured on the device the same day, it is a desktop page shown on a
tablet: a generic jigsaw icon instead of the theme's art, buttons sized for a mouse pointer, no
way back to "no theme" except disabling whichever is on, no way to get a theme except finding a
theme website, and nothing tells the user what a persona is.

This kit makes the **Themes** pane of `about:addons` the home of personas, by changing the
add-ons manager's presentation and adding to it — the same manager, the same add-on model, the
same install path R10 already secured. It does not add a second theme manager; Settings →
Appearance (cavekit-preferences-ui.md) becomes a short summary that links here.

Reference: `render/goanna/components/jihadPersonas.js` (install, static-theme conversion, card
notification), UXP `toolkit/mozapps/extensions/` (`content/extensions.xul|js|css`,
`LightweightThemeManager.jsm`), the shells' `applyTheme`s, `render/goanna/test/personas/`.

## Requirements

### R1: Every theme shows its art
**Description:** In the Themes pane each persona is shown as what it looks like, not as a
generic add-on row.
**Acceptance Criteria:**
- [ ] Each row carries a preview of the theme: its header image (cropped to the row) over its
  accent colour, with the name in the theme's own text colour — the row looks like the toolbar
  will.
- [ ] The theme in use is marked plainly ("In use") and sorted first; the rest follow most
  recently used first.
- [ ] Author and, when the theme has one, its description or home page are shown under the name.
- [ ] A theme whose image is missing or unreadable still shows its colours and name.
**Dependencies:** cavekit-addons-extensions.md R10

### R2: One tap to wear, one tap to go back
**Description:** Changing the look is a single, finger-sized action.
**Acceptance Criteria:**
- [ ] Tapping a theme row's **Wear** (the manager's Enable) applies it; every open card of this
  variant repaints within a second (the existing `theme` notification).
- [ ] A **Default look** entry is always first in the list: tapping it takes the theme off,
  exactly as Settings → Appearance → Remove does, and is marked "In use" when no theme is on.
- [ ] **Remove** forgets a theme and deletes the images the browser unpacked for it
  (`<profile>/jihad-themes/` for converted static themes) — nothing is left on disk.
- [ ] Every control is at least 44 px tall and wide at the card's width (768 portrait, 1024
  landscape) — usable with a finger, in every shell.
**Dependencies:** R1

### R3: A safe try-on
**Description:** A person can see a theme on the browser before keeping it.
**Acceptance Criteria:**
- [ ] **Try** previews a theme on the open cards for a few seconds (the engine's own preview,
  which the manager ends by itself), with Keep / Undo; Undo or the timeout restores what was on.
- [ ] A try-on is never remembered as the chosen theme (R10's rule: only an installed theme is
  persisted).
**Dependencies:** R2

### R4: Installing, from where people find themes
**Description:** Getting a new persona does not require knowing which website still hosts them.
**Acceptance Criteria:**
- [ ] **Install from file…** (the manager's own gear-menu entry) takes an AMO static-theme
  `.xpi` from the device and installs it through the same conversion and confirm as a download
  (R10 limits apply: manifest ≤ 64 KB, image ≤ 4 MB, png/jpeg/gif, pending file kept only on yes).
- [ ] A **Get themes** link in the Themes pane opens the AMO themes listing in a card; installing
  one there goes through the existing download → conversion → confirm, and the new theme
  appears in the pane, worn.
- [ ] Theme sites using the classic `InstallBrowserTheme` event (Firefox 52 / Pale Moon /
  Basilisk personas) keep working exactly as now.
- [ ] Both header forms AMO serves are accepted: `theme_frame` / `headerURL` and the first of
  `additional_backgrounds` (implemented 2026-10-08).
**Dependencies:** cavekit-addons-extensions.md R10

### R5: It explains itself
**Description:** Someone who has never heard of personas understands them from the pane.
**Acceptance Criteria:**
- [ ] With no themes installed the pane says, in a sentence or two, what a persona is and how to
  get one, with the Get themes link — not an empty list.
- [ ] Settings → Appearance shows the theme in use and a **Manage themes** button that opens
  `about:addons` on the Themes pane; its list and buttons move here (no second manager).
**Dependencies:** R1, R4

### R6: The same in every shell, verified
**Description:** All three variants get the same pane and the same behaviour.
**Acceptance Criteria:**
- [ ] Enyo, Mochi and Mojo each show the reworked Themes pane (it is the engine's page, shared),
  and each repaints its toolbar and buttons on Wear / Default / Try.
- [ ] Verified on the device by test hook in each shell: wear a theme from the pane, go back to
  the default, try one and undo, remove one and check its images are gone, install from file.
- [ ] Screenshots of the pane in each shell, at least one with no theme.
**Dependencies:** R1–R5

## Out of Scope
- Full WebExtension themes (images per toolbar part, `theme` API, dynamic themes) — this engine
  installs no WebExtensions; static themes are converted to personas (R10).
- Animated themes beyond their first frame.
- Theming the engine's own pages (`about:addons` itself stays in the system look; the theme is
  the browser chrome's).

## Cross-References
- cavekit-addons-extensions.md R10 — what a persona is here, install security, persistence.
- cavekit-preferences-ui.md R2/R5 — the settings page's Appearance section becomes a summary.
- cavekit-tabbed-browsing.md R1 — tabs, when built, wear the theme too.

## Changelog
- 2026-10-08: Created (owner: "make it user friendly to manage and install personas, using
  about:addons as a base"). Baseline measured on the device: the Themes pane lists personas with
  Enable / Disable / Remove and a generic icon; nothing else. R4's `additional_backgrounds`
  criterion already implemented (three of five AMO themes tried that day needed it).
