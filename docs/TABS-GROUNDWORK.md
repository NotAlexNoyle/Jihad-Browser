# Tabbed browsing — groundwork (design, not implementation)

**Status (2026-10-06): nothing here is built.** The owner asked for the groundwork for tabbed
browsing and, in the same breath, not to implement it yet. This note is that groundwork: what
exists today that tabs would stand on, the two ways to build them, why one is chosen, where
each change would land, what it would cost, what it cannot do, and how it would be proved. The
requirements are `context/kits/cavekit-tabbed-browsing.md`; the tasks are Tier 71 of
`context/plans/build-site.md`, all pending. When the owner says go, start from the kit, this
note and those tasks — not from a blank page.

## 1. What a card is today

- One card = one adapter instance (an `<object>` with the variant's MIME type) = one YAP
  connection = one `BrowserPageGoanna` in that variant's daemon. `JihadBrowserServer` keeps
  `std::map<YapProxy*, Page> mPages` with `struct Page { BrowserPageGoanna* page; ProxySink* sink; }`
  (`render/browserserver/JihadBrowserServer.h`); `pageFor(proxy)` resolves every command;
  `reap(proxy)` destroys the page when the client disconnects. `mConnectOrder` knows which
  connection is newest; `mLastProxy` is the inject/download fallback.
- The adapter gives the page two shared buffers at `connect` and a third at `setExtraBuffer`:
  on the device `connect 960x1400 … sz=12582944` twice, `extra buffer … sz=12582944` — about
  **37.7 MB of shared memory per connection**, pinned for the card's life. The daemon paints
  composed frames into the current slot (`BufFrame`, up to 4096 rows, wide when zoomed —
  cavekit-offscreen-rendering.md R9).
- Every message the card sees about its page goes through `ProxySink` → YAP → adapter → the
  card's JS: `msgPainted`, load started/progress/stopped, `msgTitleAndUrlChanged`,
  `msgContentsSizeChanged`, meta-viewport, history state, dialogs, `msgCreatePage`.
- `window.open` / `target=_blank`: the page asks for a new page (`msgNewPageRequested`), the
  daemon sends the card `msgCreatePage(id)`, the card opens a NEW CARD whose adapter connects
  with that id, and the daemon opens the requested url in that new connection's page
  (`mPendingNewPages`, `asyncCmdConnect`). That is the card metaphor's "tabs", and it stays.
- The adapter treats `msgContentsSizeChanged(0,0)` as "new page": it drops the live offscreen
  buffer and resets zoom-fit (`BrowserPageGoanna.cpp` around `emitGeometry`, where the daemon
  deliberately never sends a degenerate size by accident). That sentinel is the hinge a tab
  switch would turn on.
- The daemon's own Luna service (`JihadLunaService.cpp`) is this project's and is not frozen:
  public `{clearCache, clearCookies, notifications, getFlash, enableFlash, disableFlash,
  saveThumbnail, deleteThumbnail, clearThumbnails, findPrinters, printPage, pageCertificate}`,
  private `{clearCache, clearCookies, getChromePrefs, getFlash, notifications}`.
  `pageCertificate` shows how a Luna request is answered for the asking card only: the card
  calls the adapter's `pageFocused(true)` first (a YAP command that already exists), the
  daemon notes which connection has the focus (`mFocusProxy`), and the Luna handler answers for
  that connection (cavekit-ui-shell.md R8, 2026-10-05).
- What is frozen: the YAP command/message tables (`docs/IPC-CONTRACT.md`, cavekit-ipc-contract.md
  R1) and the adapter's 70 exposed JS methods. The Mojo shell additionally may not call a
  method the Enyo shell does not (cavekit-mojo-ui.md R3).

## 2. Two ways to build tabs

### A. Several adapter instances in one card (one `<object>` per tab)
The shell keeps N plugin elements and shows one. Nothing in the daemon changes: N connections,
N pages, N buffer sets.

Why not:
- **Memory.** 37.7 MB of shared memory per connection (measured above). Eight tabs ≈ 300 MB
  pinned, on a 1 GB device where the daemon already runs with a low-memory watcher
  (`EngineHost.cpp`, `/proc/meminfo` poll → `memory-pressure`). Not viable.
- **Hidden plugins.** The stock WebKit destroys an NPAPI instance whose element goes
  `display:none` (NPP_Destroy), so a hidden tab could not stay alive; keeping all N laid out
  and stacked means N adapters composing frames the user never sees.
- **The card's chrome** would have to multiplex N streams of title/url/progress/dialog
  messages itself, in three shells, three times.

### B. Pages in the daemon, one connection per card (CHOSEN)
A tab is a `BrowserPageGoanna` in the daemon. The card's one connection renders whichever page
is **bound** to it; the others live unbound. Tab commands go over the variant's own Luna
service; the YAP contract and the adapter are untouched.

Why:
- One buffer set per card whatever the tab count; a background tab costs its docshell alone.
- The adapter already knows how to take a "new page" (the sentinel) and re-fit.
- The shells stay thin: a strip, four Luna calls, and the notifications they already subscribe to.
- It is the same shape as `pageCertificate`: Luna for the ask, `pageFocused` for the identity.

## 3. Design B in detail (what would change, where)

### Daemon — `render/browserserver/JihadBrowserServer.{h,cpp}`
- `struct Page` → `struct TabSet { std::vector<Tab> tabs; size_t active; ProxySink* sink; }`,
  `struct Tab { BrowserPageGoanna* page; RecordingSink* rec; uint32_t id; std::string title, url; bool loading; int progress; bool canBack, canFwd; std::vector<HeldDialog> held; }`.
  `mPages[proxy]` holds the set; `pageFor(proxy)` returns `tabs[active].page` (every YAP
  command keeps working unchanged, against the active tab).
- **Binding.** The active tab's page is constructed with the connection's `ProxySink` and the
  connection's buffers (as today). A background tab's page is constructed with (or re-pointed
  to) a `RecordingSink` that stores title/url/load/history/dialog state and sends nothing. A
  page's sink is a reference today (`BrowserPageGoanna(mHost, *sink)`); it becomes a pointer the
  set can swap — the one structural change in the daemon. Buffers: `init(w, h, key1, key2, sz)`
  attaches the shared buffers to the page; a background page gets none (`attachBuffers` /
  `detachBuffers` split out of `init`, plus the extra buffer), and `maybePaint` returns at once
  when unbound. `BufFrame` slots and the compose cache (`mLatestSlot`, sweeps, `mGrowOwed`) are
  per page today; on switch the new page's cache is stale by definition → it starts with
  `mLatestSlot = -1` (a full frame of up to `kInitialRows`), which is what a navigation does.
- **Switch (`tabs/switch`)**: detach buffers + sink from the old active page (it keeps its
  docshell, scroll, zoom, DOM), attach them to the new one, then emit in order:
  `msgContentsSizeChanged(0,0)` (the sentinel), content size, meta-viewport, `msgTitleAndUrlChanged`,
  history state, load started/progress or stopped as recorded, then paint. The adapter and the
  card arrive at the same state a navigation would have left. One frame, no grey (R2).
- **New (`tabs/new {url?}`)**: cap check (`jihad.tabs.max`), construct a page unbound, open the
  url (or the shell's start page: the shell passes it), then switch to it.
- **Close (`tabs/close {id}`)**: if active, switch to the left neighbour (else right) first; destroy
  the page the way `reap` does today (deferred to after the tick). Closing the last tab opens a
  new one on the start page first (R1: never zero).
- **List (`tabs/list`)**: ids, titles, urls, active, loading, "has a held dialog", "unloaded".
- **Notifications**: a `tabs` category on the existing notification channel, one message per
  change (`{"event":"changed","tabs":[…],"active":N}`), so the strip redraws without polling —
  the same subscription the shells hold for toasts/`page-hold`/`print-*`.
- **Identity**: every `tabs/*` request is answered for `mFocusProxy` only (the card calls
  `pageFocused(true)` first — exactly `pageCertificate`'s rule). A card with no focus claim is
  refused with `returnValue:false`.
- **Held dialogs** (R3): `DialogService` routes an engine dialog to the page's sink; a
  `RecordingSink` queues it (`HeldDialog {kind, text, default…}`) and the engine's prompter
  blocks on the pipe as today (`dialog-N.fifo`) — so a background tab's script WAITS at its
  `alert()` until the tab is switched to, when the queue is replayed through the live sink. The
  strip shows the tab wants attention (from `tabs/list`'s flag).
- **Downloads / createPage / notifications from a background tab**: `pageForDownload` already
  matches by docshell key; `msgCreatePage` from a background page is passed through (a new
  CARD is fine from any tab); print and page-hold only ever concern the active tab.
- **Reap**: `clientDisconnected` destroys every tab of the set.
- **Low memory**: the watcher's `memory-pressure` already lets Goanna shed caches; a second
  step may unload the oldest background tab (`page->openUrl("about:blank")` keeping `url`,
  `title` and the session history in the `Tab`; reloaded on switch) — R3; measure first.

### Daemon — `render/browserserver/JihadLunaService.{h,cpp}`, `Main.cpp`
Four new methods on the public bus (`tabs/new`, `tabs/switch`, `tabs/close`, `tabs/list`),
installed next to `pageCertificate` with the same `LunaService…Fn` lambda pattern; the `tabs`
notification category. Nothing on the private bus (shells use the public one).

### Engine side — `render/goanna/BrowserPageGoanna.{h,cpp}`
- Sink by pointer, swappable; `attachBuffers/detachBuffers`; `maybePaint` no-op when unbound;
  a `replayStateTo(sink)` that re-sends the geometry/title/history messages from the page's own
  state (most exist as `emitGeometry` and the title/url path; they need a "force" flag because
  they log/send only on change — see `mLastContentW/H`, `mViewportSent`).
- Timers of a background page: Goanna's own background-tab throttling is tied to
  `docShell->SetIsActive(false)` — set it on unbind, clear on bind. That also stops the page
  painting for itself.

### Shells
- **Enyo (`app/source/`)**: a `TabStrip` kind under the `ActionBar` (hidden with one tab), its
  rows from `tabs/list` / the `tabs` notification via `PalmService`, taps → `pageFocused(true)`
  then the Luna call (the `certService` pattern in `Browser.js`); long-press menu gains "Open
  in New Tab" (`BrowserContextMenu.js`) → `tabs/new {url}`; "New Card" untouched.
- **Mochi (`app-mochi/source/JihadBrowser.js`)**: the same as an inline strip above the view
  (`JihadServices.js` carries the Luna calls); the hold sheet gains the item.
- **Mojo (`app-mojo/app/assistants/main-assistant.js`)**: a strip row above the command row (it
  scrolls horizontally like the row does); `serviceRequest` to the own service, as
  `showPageCertificate` does. The strip is plain Mojo UI, so cavekit-mojo-ui.md R3's method
  set is untouched (no new `callBrowserAdapter` literal).
- All three: the `tabs` notification is routed in the notification switch next to `page-hold`.

### Preferences — `packaging/prefs`, `render/goanna/components/jihadAboutPreferences.js`
`jihad.tabs.enabled`, `jihad.tabs.max`, `jihad.tabs.showStripAlways`, `jihad.tabs.openLinksIn`
as real prefs with rows on the settings page (cavekit-preferences-ui.md R2/R5).

### Test hooks — `render/goanna/test/*-selftest.js`, `build/webos-oe/card-selftest.sh`
Steps `T` (new tab, optional url), `tN` (switch), `xN` (close), `?` reporting the strip; frame
dumps against colour-coded pages (`jihad-long.html` method) to prove the frame after a switch
is the right page.

## 4. What it costs and what it cannot do (say it before building)
- A switch is a **document change in the adapter's eyes**: the sentinel drops its buffer and
  zoom-fit; the daemon re-sends geometry and the card re-fits. The user sees the page where
  they left it, but as a full frame, not an instant flip. (A flip would need a per-tab buffer
  set — plan A's memory.)
- A background page keeps its docshell: memory per background tab is the document (a few MB
  for text, tens for image-heavy pages). The cap (8) and the unload step are the guard; the
  numbers must be MEASURED on the device before the cap is final (T-351).
- A tab cannot move to another card live: "Move to New Card" carries url + history, not the
  DOM (R4 says so).
- Cards of the same variant each have a set; a card of ANOTHER variant is another daemon —
  no cross-variant tabs (by design, R7 of device-build).
- The Luna answer-for-the-focused-card rule means a tab action from a card that has not
  claimed focus is refused; the shells always claim first (as for `pageCertificate`).

## 5. Order of work (Tier 71, pending)
T-347 prefs and kit wiring → T-348 daemon tab set, bind/unbind, switch/new/close/list over
Luna, notifications → T-349 recording sink: held dialogs, replay → T-350 the three strips +
"Open in New Tab" → T-351 memory measurement, cap, unload step → T-352 hooks + device
verification → T-353 review (Codex / inspector) → T-354 owner's finger test.

## 6. Open questions for the owner (not blocking the groundwork)
1. Default for `window.open` / `target=_blank`: new card (today's behaviour) or new tab?
2. Should the strip be visible with one tab (desktop habit) or hidden (webOS habit)? Default
   proposed: hidden.
3. Cap default 8 — fine, or lower for the TouchPad Go?
