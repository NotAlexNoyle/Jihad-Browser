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
- [x] The app menu's "Find on Page" opens the find bar in Enyo, Mochi and Mojo; the long-press menu's "Find on Page"
  opens it where that menu offers it; Ctrl+F on a Bluetooth keyboard opens it in each *(2026-11-19: `find-steps` (the app menu), `find-hold-menu` (the long-press menu), `find-ctrl-f` (Ctrl+F on the emulated kernel keyboard, on a card just opened and with a page's field focused), Enyo, Mochi and Mojo, docs/rounds/find-2026-11-19. Found on the way: the daemon read Ctrl's mask 0x40 as Alt (cavekit-input-bridging.md R2, corrected); Mojo read key 17 as the phones' Sym key and opened its accent picker on the field; Enyo's page was drawn over the bar where no system keyboard resized the window)*
- [x] The bar opens with its field focused and the system keyboard up; the page beneath keeps its place *(2026-11-19: `find-steps`, `find-reopen`, `find-hold-menu`: `expect keyboard up` at the opening and the page at its scroll (the page's own `y=` in the frames), three shells, docs/rounds/find-2026-11-19)*
- [x] The bar is drawn in the interface's own look (cavekit-chrome-look.md), its controls at least 44 px tall, and it does
  not cover the page's matches it reports (R3) *(2026-11-19: each bar in its toolkit's controls and colours (the kept frames of docs/rounds/find-2026-11-19), its buttons 44 px or taller (Enyo and Mochi 44 px by their CSS, Mojo's own buttons 46 px); `find-steps` checks the match it reports above the keyboard and inside the page area below the bar. Mochi's bar became a row of its own (it lay over the page's top) and Mojo's strip stays while it shows)*
- [x] It works on the browser's own pages (about:settings, about:data) as on web pages *(2026-11-19: `find-own-pages`, "personas" on about:settings and "site data" on about:data, three shells, docs/rounds/find-2026-11-19)*
**Dependencies:** cavekit-ui-shell.md R4

### R2: Found as you type
**Description:** Typing finds; the match is visible.
**Acceptance Criteria:**
- [x] From the first letter typed, the first match from the top of what is shown is selected and scrolled into view;
  each further letter narrows the match where it stands (cavekit-ui-shell.md R4's rule) *(2026-11-19: `find-steps`: "zebra" typed by finger is 1 of 6 from its first letter, each letter searched (the daemon's log has one search per letter), three shells, docs/rounds/find-2026-11-19)*
- [x] The match is case-insensitive and finds accented and non-Latin text as typed (é, ß, a Cyrillic or Greek word) *(2026-11-19: `find-text`: ZEBRA as zebra (1 of 6), Straße, café, привет, καλημέρα, three shells, docs/rounds/find-2026-11-19; the page's `window.find()` can no longer leave the finder case-sensitive or whole-word)*
- [x] The current match is drawn so it is seen: a highlight that contrasts with the page in light and dark pages, never
  white on white (cavekit-ui-shell.md R4's 2026-10-01 finding) *(2026-11-19: `find-reopen`'s shot of the match on a black band, `find-steps`' shots on white, three shells, read by eye: the selection's blue under white text on both, docs/rounds/find-2026-11-19)*
- [x] A match inside a frame of the page, and a match inside a text field, is found and shown as one on the page is *(2026-11-19: `find-text` ("in a frame" 1 of 1, "okapi" only in a textarea, 1 of 1) and `find-steps` (the sixth zebra is the frame's: reached going round both ways), three shells, docs/rounds/find-2026-11-19. A step now goes on from the frame of the last match, and no other frame keeps a selection)*
**Dependencies:** R1

### R3: Next and Previous
**Description:** Both directions work, and going round is said.
**Acceptance Criteria:**
- [x] Next goes to the following match and Previous to the one before, each scrolled into view, in each interface; the
  system keyboard's Enter is Next *(2026-11-19: `find-steps`: Enter 2 of 6, Next 3, Previous 2 and 1, three shells, docs/rounds/find-2026-11-19. Mojo's field had never seen Enter (Mojo turns it into a Tab unless the field takes it: `requiresEnterKey`))*
- [x] Past the last match Next goes round to the first, and before the first Previous goes round to the last; the bar says
  it went round *(2026-11-19: `find-steps`: Previous from 1 is 6 of 6 "from the bottom", Next from 6 is 1 of 6 "from the top", three shells, docs/rounds/find-2026-11-19; a page with one match says it too)*
- [x] A match scrolled to is above the system keyboard and clear of the find bar *(2026-11-19: `find-steps`' `expect above-keyboard` and `expect visible` on the match, three shells, docs/rounds/find-2026-11-19. Prev and Next keep the field's focus, so the keyboard does not drop mid-step: its resize had put the page back where the card last had it)*
**Dependencies:** R2

### R4: How many, and none
**Description:** The bar says where the person is among the matches, and when there are none.
**Acceptance Criteria:**
- [x] The bar shows "n of m" for the current match among all on the page, updated as the person types and moves *(2026-11-19: `find-steps`, every step's "n of 6", three shells, docs/rounds/find-2026-11-19; the count is the page's component's (the engine's range finder, up to 1000 matches and 300 ms, "+" past them))*
- [x] A text not on the page is said at once ("No matches", the field marked), and Next and Previous do nothing then *(2026-11-19: `find-none`: "qqxz" is 0 of 0, the bar says "No matches" with the field in red and Prev and Next off (the shot), three shells, docs/rounds/find-2026-11-19)*
- [x] The count stays right as the page changes under the bar (a page that adds text): it is the count at the last search *(2026-11-19: `find-text`: a zebra the page adds is in the next search's count (7), three shells, docs/rounds/find-2026-11-19)*
**Dependencies:** R2, R3

### R5: Done, and what it leaves
**Description:** Closing the bar leaves the page as it was, at the last match.
**Acceptance Criteria:**
- [x] Done closes the bar and puts the keyboard away; the page stays scrolled to the last match, and no highlight remains *(2026-11-19: `find-steps`: after Done the keyboard is down, the page's selection is empty, and the page is where the match put it (not at the top), three shells, docs/rounds/find-2026-11-19. Mochi's Done had thrown before the clear (`resized()`, which is `resize()` in this Enyo); a match in a text field is collapsed too)*
- [x] Navigating to another page, back, or reloading closes the bar or clears its count; a new page is not searched with
  the old word *(2026-11-19: `find-navigate`: a load closes the bar and clears the page's match, three shells, docs/rounds/find-2026-11-19; back and reload start a load the same way)*
- [x] Opening the bar again on the same page offers the last word, selected, so typing replaces it *(2026-11-19: `find-reopen`: "zebra" offered selected, "last" typed replaces it (1 of 1); opened a third time, Next searches the offered word once, three shells, docs/rounds/find-2026-11-19)*
**Dependencies:** R2

### R6: In the suite
**Description:** All of the above by script, in the three interfaces.
**Acceptance Criteria:**
- [x] Each criterion of R1-R5 is a finger script of the suite (`docs/FINGER-TESTS.md`) run in Enyo, Mochi and Mojo, from
  test pages served on the host (a long page with a word many times, upper and lower case, accented and non-Latin text, a
  frame, a text field, a dark page) *(2026-11-19: eight scripts (`find-*`), each in the three shells, on `render/goanna/test/jihad-find.html`, about:settings, about:data and `jihad-input-echo.html`, docs/rounds/find-2026-11-19)*
- [x] The rounds' results are in `docs/rounds/`, and the README says what runs *(2026-11-19: docs/rounds/find-2026-11-19/README.md and its report; docs/FINGER-TESTS.md lists the scripts with their verdicts, and the README's status counts the criteria)*
**Dependencies:** R1-R5, cavekit-finger-testing.md

## Cross-references
- cavekit-ui-shell.md R4 (the engine's search through `findInPage`), R7 (the long-press menu)
- cavekit-ipc-contract.md (the YAP contract stays byte-identical: `FindString` already carries a direction; the
  adapter's scriptable methods may grow)
- cavekit-mochi-ui.md, cavekit-mojo-ui.md (each interface's menus), cavekit-chrome-look.md (the look),
  cavekit-input-bridging.md (the keyboard)

## Changelog
- 2026-11-19: created at the owner's request.
- 2026-11-19: all 19 criteria met (T-674..T-678), docs/rounds/find-2026-11-19; the answer carries no text (the Codex review).
