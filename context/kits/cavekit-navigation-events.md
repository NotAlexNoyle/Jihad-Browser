---
created: "2026-06-30"
last_edited: "2026-07-04"
---

# Cavekit: Navigation, Loading & Events

## Scope
Navigation commands (load, history, stop) and the stream of load-lifecycle,
location, title, redirect, and history-state messages the renderer emits from
the engine's progress/URI listeners. Reference: `docs/IPC-CONTRACT.md`
(commands + server→adapter messages), `render/goanna/PORT-MAP.md`.

## Requirements

### R1: Navigation commands
**Description:** The page navigates per command.
**Acceptance Criteria:**
- [x] `openUrl` loads the URL.
- [x] `back`/`forward`/`reload`/`stop` perform the expected navigation.
- [x] `canGoBack`/`canGoForward` state is accurate and reflected in location messages.
- [x] `clearHistory` empties session history.
**Dependencies:** cavekit-ipc-contract.md (R1)

### R2: Inline HTML loading
**Description:** Content can be loaded directly without a network fetch.
**Acceptance Criteria:**
- [x] `setHtml(url, body)` renders the provided body as the document at the given base URL.
**Dependencies:** none

### R3: Load lifecycle events in correct order
**Description:** Loads produce an ordered, complete event stream.
**Acceptance Criteria:**
- [x] A normal load emits load-started, then progress updates from 0 to 100, then load-stopped and document-load-finished.
- [x] A failed load emits a failed-load / main-document-error message with domain, code, failing URL, and description.
- [x] `stop` mid-load ends the stream cleanly.
**Dependencies:** cavekit-engine-embedding.md (R3)

### R4: Location, title, and redirect events
**Description:** The client is told where the page is and what it is called.
**Acceptance Criteria:**
- [x] Navigation emits a location-changed message with the URL and back/forward state.
- [x] Title resolution emits title-changed (and the combined title-and-url message where upstream did).
- [x] An ordinary HTTP 3xx redirect is FOLLOWED in-browser (final location reported via location-changed) and does NOT emit url-redirected. *(2026-07: the isis app binds onUrlRedirected -> openResource -> the DEFAULT (stock) browser, so emitting url-redirected on a normal redirect made every redirecting site — google, most https — leave Jihad and reload forever. url-redirected is reserved for `addUrlRedirect` app-handoff rules only, R6.)*
**Dependencies:** cavekit-offscreen-rendering.md (R4)

### R5: History-state queries answered
**Description:** Query-style commands get matched responses.
**Acceptance Criteria:**
- [x] `getHistoryState(queryNum)` produces a history-state response carrying the same query number and correct back/forward flags.
**Dependencies:** cavekit-ipc-contract.md (R1)

### R6: Global history and link-click reporting
**Description:** Side-channel navigation events are reported.
**Acceptance Criteria:**
- [x] Navigation emits an update-global-history message.
- [x] A link activated by tap/click (not only an engine content-initiated load) emits a link-clicked message with the target URL — verified by `link_test`. *(review #5 H-2: the tap-interception path bypasses the engine's programmatic-load classifier, so the message must be emitted at interception.)*
- [x] URL-redirect rules registered via `addUrlRedirect` are honored.
**Dependencies:** cavekit-browser-services.md

### R7: Same-document (fragment) navigation
**Description:** Fragment-only navigations do not restart the full load lifecycle.
**Acceptance Criteria:**
- [x] Activating `<a href="#frag">` on the current document does not emit a full load-started/stopped cycle or reload the page (the tap path detects same-URL-sans-fragment and routes it to the in-page click path rather than a full load). *(review #5 M-1.)*
**Dependencies:** none

## Out of Scope
- Painting the page (cavekit-offscreen-rendering.md).
- Cookies/cache/settings that affect loads (cavekit-browser-services.md).

## Cross-References
- See also: cavekit-ipc-contract.md, cavekit-offscreen-rendering.md, cavekit-browser-services.md, cavekit-ui-shell.md

## Changelog
- 2026-06-30: Initial draft.
- 2026-07-04: Status reconciled to implementation — all R1–R6 verified (NAV/FAIL/HISTORY/REDIRECT/RULES/LINK PASS).
