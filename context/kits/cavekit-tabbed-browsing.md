---
created: "2026-10-06"
last_edited: "2026-10-06"
complexity: complex
---

# Cavekit: Tabbed Browsing

## Scope
Several pages open at once INSIDE one card, with a tab strip to move between them — the desktop
model — in all three shells (Enyo `app/`, Mochi `app-mochi/`, Mojo `app-mojo/`). webOS's own
"New Card" (one page per card, cards in the card view) stays exactly as it is; tabs are added
beside it, not in place of it.

**Status: GROUNDWORK ONLY (owner, 2026-10-06: "lay the groundwork for tabbed browsing but don't
implement it yet").** This kit states what tabs must do and the constraints they must satisfy.
Nothing in it is built. The design that satisfies it — which half lives in the daemon, which in
the shells, and why — is `docs/TABS-GROUNDWORK.md`; the tasks are Tier 71 of
`context/plans/build-site.md`, all pending.

The one constraint that shapes everything: **the BrowserAdapter↔BrowserServer YAP contract is
frozen** (cavekit-ipc-contract.md R1). No new YAP command or message, no change to the adapter's
JS method set. Tabs therefore cannot be "the adapter gets a tab API". What the shells may use is
the daemon's OWN Luna service (`palm://net.riverstonerelay.jihadBrowser*/`), which is this
project's and already carries printing, notifications and `pageCertificate` (cavekit-ipc-contract.md
R4 precedent).

Reference: `docs/TABS-GROUNDWORK.md`, `docs/IPC-CONTRACT.md`,
`render/browserserver/JihadBrowserServer.{h,cpp}` (one `Page` per adapter connection today),
cavekit-ui-shell.md R7/R8 (how a card already talks to its own daemon over Luna).

## Requirements

### R1: A tab strip in every shell
**Description:** The card shows which pages it has open and lets the user switch, open and close them with a finger.
**Acceptance Criteria:**
- [ ] A strip of tabs, each with the page's title (or host, until there is a title) and a close control, plus a "new tab" control; the active tab is told apart from the rest. In all three shells, in the shell's own idiom (Enyo `app/`, Mochi, Mojo), hit-testable at 1024x768 with a finger (controls at least 44 px tall).
- [ ] A tap on a tab makes it the active one: the page it holds is what the card renders and what the address bar, title, back/forward state, padlock and progress describe, within one frame of the switch being asked (no grey, no stale page — cavekit-offscreen-rendering.md R9 applies to the switch).
- [ ] "New tab" opens the shell's start page (or an address the caller gives) in a new tab and makes it active; the previous tab's page keeps its state (scroll, zoom, form text, history).
- [ ] Closing the active tab activates its neighbour (the one to its left, else to its right); closing the last tab leaves one tab on the start page — a card never has zero tabs.
- [ ] A long-press on a link offers "Open in New Tab" beside the existing "Open in New Card" (cavekit-ui-shell.md R7's menu); `window.open` / `target=_blank` keeps opening a NEW CARD as today (the daemon's `msgCreatePage` path), unless the preference in R5 says tabs.
- [ ] The strip hides when a card has one tab, unless the preference in R5 says always show it.
**Dependencies:** cavekit-ipc-contract.md (R1, R4), cavekit-ui-shell.md (R7), cavekit-mochi-ui.md, cavekit-mojo-ui.md, cavekit-preferences-ui.md (R5)

### R2: Tabs within the frozen contract
**Description:** Tabs add nothing to the adapter or to the YAP contract, and a card's tabs are that card's alone. A switch leaves the card describing and showing the chosen page exactly as a navigation to it would have; a tab the user is not looking at keeps its state and asks nothing of the card. (Where the pages live — in the daemon, one connection per card — is the design's choice, `docs/TABS-GROUNDWORK.md`; this requirement states what must hold whichever way it is built.)
**Acceptance Criteria:**
- [ ] No YAP command or message is added, removed or changed; the adapter's exposed JS method set is unchanged. Verified the way cavekit-ipc-contract.md R1 is: the contract tables in `docs/IPC-CONTRACT.md` are byte-identical before and after; the adapter binary's method list is the same 70 names.
- [ ] Tab commands travel over the variant's OWN Luna service only — `tabs/new`, `tabs/switch`, `tabs/close`, `tabs/list` (names final in the design note) — answered for this variant's own card only, the way `pageCertificate` is (cavekit-ui-shell.md R8: `pageFocused` first, then the question). A card can never act on another card's tabs.
- [ ] On a switch the daemon re-binds the adapter connection to the chosen page and sends, through the ordinary YAP messages, everything the adapter and the card need to describe it: the "new page" sentinel the adapter already understands (`contentsSizeChanged(0,0)` — cavekit-navigation-events.md), then content size, meta-viewport, title and url, history state, load state, and a full frame. The card ends up exactly as it would after navigating to that page: address, title, padlock, back/forward, progress all for the active tab.
- [ ] A background tab's page keeps loading and running (its load finishes, its scripts run — timers may be throttled), but paints nothing: no shared-buffer writes, no `msgPainted`, no geometry messages to the card, no title/url/progress messages. Its state is recorded in the daemon and replayed on the switch.
- [ ] Tab state survives what it should: switching away and back keeps scroll position, zoom, form text and the find highlight; closing the card closes all its tabs (the daemon reaps every page of the connection, as it reaps the one today).
- [ ] The tab set belongs to the connection: when the card disconnects (closed, or LunaSysMgr restarted) every page of the set is destroyed with it. No page outlives its card.
**Dependencies:** cavekit-ipc-contract.md (R1, R4, R5), cavekit-navigation-events.md, cavekit-offscreen-rendering.md (R9)

### R3: Background tabs are quiet and bounded
**Description:** A tab the user is not looking at must not take the card's attention or the device's memory.
**Acceptance Criteria:**
- [ ] An alert / confirm / prompt / HTTP-auth / SSL dialog raised by a background tab's page is HELD in the daemon until that tab is active, then shown; the tab in the strip shows it has something to say. Nothing from a background tab ever appears over the active page. (cavekit-browser-services.md R3's dialog interception is where this attaches.)
- [ ] Downloads, notifications (`page-copied`, `print-*`, …) and `msgCreatePage` from a background tab are attributed to that tab (the daemon's `pageForDownload` by docshell key already does this for downloads) and do not disturb the active one.
- [ ] A hard cap on tabs per card, default 8, from a pref (`jihad.tabs.max`); "new tab" beyond it is refused with a toast, not silently dropped.
- [ ] Memory: a background tab holds no shared buffer and no compose cache (`BufFrame` slots are the connection's, not the page's), so its cost is its Goanna docshell alone. Measured on the device: eight tabs on typical pages keep the daemon under the low-memory watcher's threshold (cavekit-engine-embedding.md; the figure recorded in the design note once measured), and the daemon's existing low-memory response may additionally UNLOAD the oldest background tab's document (keeping its url, title and history) and reload it when it is switched to — shown in the strip as a dimmed tab.
- [ ] The idle rules hold with tabs: a card with eight tabs, none loading, ticks and logs no more than a card with one (cavekit-device-build.md R8 "an idle daemon is idle").
**Dependencies:** cavekit-browser-services.md (R3), cavekit-engine-embedding.md, cavekit-device-build.md (R8)

### R4: Tabs and cards coexist
**Description:** webOS's card model stays first-class; tabs add to it.
**Acceptance Criteria:**
- [ ] "New Card" in every shell, `window.open` and `msgCreatePage` behave exactly as before this work (device, each shell).
- [ ] Two cards of the same variant each have their own tab set; switching tabs in one changes nothing in the other (two adapter connections, two sets — verified with the two-card test hooks, `F`-step style, cavekit-ui-shell.md R8).
- [ ] The card view's thumbnail (`saveThumbnail`) shows the active tab's page.
- [ ] A tab can be moved out to its own card ("Move to New Card" on the tab's long-press): the daemon hands the page's url to a `msgCreatePage` and closes the tab; the state carried is the url and the history list (a live docshell cannot cross connections — stated, not hidden).
**Dependencies:** cavekit-navigation-events.md, cavekit-ui-shell.md (R8)

### R5: Preferences
**Description:** The few choices a user may want, on the settings page like every other preference (cavekit-preferences-ui.md R5: one Settings).
**Acceptance Criteria:**
- [ ] `jihad.tabs.enabled` (default on once built; the groundwork ships nothing, so no pref exists yet), `jihad.tabs.max` (default 8), `jihad.tabs.showStripAlways` (default off), `jihad.tabs.openLinksIn` (`card` | `tab`, default `card`, governs `window.open` and the long-press default).
- [ ] Each is a real engine pref the daemon reads (cavekit-preferences-ui.md R2: no decorative toggles); a change applies to the next tab action without a daemon restart.
**Dependencies:** cavekit-preferences-ui.md (R2, R5)

### R6: Test hooks and evidence
**Description:** Everything a hook can drive is driven before a finger is asked for.
**Acceptance Criteria:**
- [ ] The three test hooks (`render/goanna/test/*-selftest.js`) gain steps for new / switch / close / list through the shells' own handlers, and a `?` that reports the strip (tab count, active index, titles).
- [ ] Device-verified by hook, per shell: open three tabs on three pages, switch between them with the right address/title/padlock after each switch and a frame that is the right page (frame dump colour check against colour-coded test pages — the `jihad-long.html` method), close the middle one, close the last, background load finishing while another tab is active, a background `alert()` held then shown on switch, the cap refusing the ninth tab, two cards with independent sets.
- [ ] A finger: tab taps, the strip's close controls, "Open in New Tab" from a long-press on a link, the feel of a switch. [human at the device]
**Dependencies:** cavekit-device-build.md (R4)

## Known constraints and decisions taken at groundwork time
- **Why not one adapter instance per tab (several `<object>`s in one card):** each adapter connection allocates two shared buffers of 12.6 MB (`connect … sz=12582944`, two keys, plus an extra buffer) — eight tabs would pin ~300 MB of shared memory on a 1 GB device whose daemon already runs near the low-memory watcher; the stock WebKit also destroys an NPAPI instance that goes `display:none`, so hidden tabs could not be hidden. Pages in the daemon share the connection's one buffer set. Decided 2026-10-06; the measurement is in the design note.
- **What a switch cannot keep:** the adapter's own view state (zoom level, scroll offset as it believes them) is reset by the "new page" sentinel; the daemon re-sends the page's geometry and the card re-fits, so the user sees the page where they left it, but the transition is a document change in the adapter's eyes — a full frame, not a flip. Acceptable; stated.
- **Mojo's WebView widget** wraps only some adapter methods (cavekit-mojo-ui.md R3): tab commands go over Luna anyway, so this does not bite; the strip is plain Mojo UI.

## Changelog
- 2026-10-06: Created as groundwork only (owner's direction). Nothing built; design in `docs/TABS-GROUNDWORK.md`; Tier 71 tasks pending.
