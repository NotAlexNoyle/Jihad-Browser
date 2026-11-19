---
created: "2026-11-19"
last_edited: "2026-11-19"
complexity: medium
---

# Cavekit: Find on Page, in Each Interface

## Scope
The owner, 2026-11-19: **"new kit: per-variant find-on-page"**.

Each of the three interfaces (Enyo `app/`, Mochi `app-mochi/`, Mojo `app-mojo/`) has a find bar of its own, in its own
toolkit, and all three drive the same engine search through the adapter's `findInPage` (cavekit-ui-shell.md R4: the
engine searches, a match grows as you type and goes round past the last). What a person needs of it is not yet there in
any of them, read from the code before this kit (2026-11-19):

- **Previous does not go back.** The IPC's `FindString` carries a direction, but the adapter's `findInPage(string)` always
  sends forward; Enyo and Mochi show a Prev that is disabled or searches forward, Mojo has no Prev at all.
- **Nothing says how many, or that there is none.** The engine's answer (found or not) stops in the daemon: the bar shows
  no count, and a word that is not on the page looks the same as one that is.
- **Where the match lands, and what Done leaves,** are untested: a match under the keyboard or the find bar, a highlight
  or a selection left on the page after Done.

**Per interface** means each requirement holds in Enyo, Mochi and Mojo, each in its own look and with its own controls,
and each is checked in each by script (cavekit-finger-testing.md).

**Out of scope:** regular expressions, whole-word matching, find across tabs or cards, searching inside a Flash movie
(a plugin's text is its own), and finding in a PDF (no viewer in this browser).

## Requirements

### R1: Opened the ways a person looks
**Description:** The find bar is where a person expects it, in each interface, and opens ready to type.
**Acceptance Criteria:**
- [ ] The app menu's "Find on Page" opens the find bar in Enyo, Mochi and Mojo; the long-press menu's "Find on Page"
  opens it where that menu offers it; Ctrl+F on a Bluetooth keyboard opens it in each
- [ ] The bar opens with its field focused and the system keyboard up; the page beneath keeps its place
- [ ] The bar is drawn in the interface's own look (cavekit-chrome-look.md), its controls at least 44 px tall, and it does
  not cover the page's matches it reports (R3)
- [ ] It works on the browser's own pages (about:settings, about:data) as on web pages
**Dependencies:** cavekit-ui-shell.md R4

### R2: Found as you type
**Description:** Typing finds; the match is visible.
**Acceptance Criteria:**
- [ ] From the first letter typed, the first match from the top of what is shown is selected and scrolled into view;
  each further letter narrows the match where it stands (cavekit-ui-shell.md R4's rule)
- [ ] The match is case-insensitive and finds accented and non-Latin text as typed (é, ß, a Cyrillic or Greek word)
- [ ] The current match is drawn so it is seen: a highlight that contrasts with the page in light and dark pages, never
  white on white (cavekit-ui-shell.md R4's 2026-10-01 finding)
- [ ] A match inside a frame of the page, and a match inside a text field, is found and shown as one on the page is
**Dependencies:** R1

### R3: Next and Previous
**Description:** Both directions work, and going round is said.
**Acceptance Criteria:**
- [ ] Next goes to the following match and Previous to the one before, each scrolled into view, in each interface; the
  system keyboard's Enter is Next
- [ ] Past the last match Next goes round to the first, and before the first Previous goes round to the last; the bar says
  it went round
- [ ] A match scrolled to is above the system keyboard and clear of the find bar
**Dependencies:** R2

### R4: How many, and none
**Description:** The bar says where the person is among the matches, and when there are none.
**Acceptance Criteria:**
- [ ] The bar shows "n of m" for the current match among all on the page, updated as the person types and moves
- [ ] A text not on the page is said at once ("No matches", the field marked), and Next and Previous do nothing then
- [ ] The count stays right as the page changes under the bar (a page that adds text): it is the count at the last search
**Dependencies:** R2, R3

### R5: Done, and what it leaves
**Description:** Closing the bar leaves the page as it was, at the last match.
**Acceptance Criteria:**
- [ ] Done closes the bar and puts the keyboard away; the page stays scrolled to the last match, and no highlight remains
- [ ] Navigating to another page, back, or reloading closes the bar or clears its count; a new page is not searched with
  the old word
- [ ] Opening the bar again on the same page offers the last word, selected, so typing replaces it
**Dependencies:** R2

### R6: In the suite
**Description:** All of the above by script, in the three interfaces.
**Acceptance Criteria:**
- [ ] Each criterion of R1-R5 is a finger script of the suite (`docs/FINGER-TESTS.md`) run in Enyo, Mochi and Mojo, from
  test pages served on the host (a long page with a word many times, upper and lower case, accented and non-Latin text, a
  frame, a text field, a dark page)
- [ ] The rounds' results are in `docs/rounds/`, and the README says what runs
**Dependencies:** R1-R5, cavekit-finger-testing.md

## Cross-references
- cavekit-ui-shell.md R4 (the engine's search through `findInPage`), R7 (the long-press menu)
- cavekit-ipc-contract.md (the YAP contract stays byte-identical: `FindString` already carries a direction; the
  adapter's scriptable methods may grow)
- cavekit-mochi-ui.md, cavekit-mojo-ui.md (each interface's menus), cavekit-chrome-look.md (the look),
  cavekit-input-bridging.md (the keyboard)

## Changelog
- 2026-11-19: created at the owner's request.
