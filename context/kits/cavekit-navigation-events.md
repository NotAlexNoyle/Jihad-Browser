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
- [x] Redirects emit a url-redirected message.
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
- [x] Intercepted link activations emit a link-clicked message.
- [x] URL-redirect rules registered via `addUrlRedirect` are honored.
**Dependencies:** cavekit-browser-services.md

## Out of Scope
- Painting the page (cavekit-offscreen-rendering.md).
- Cookies/cache/settings that affect loads (cavekit-browser-services.md).

## Cross-References
- See also: cavekit-ipc-contract.md, cavekit-offscreen-rendering.md, cavekit-browser-services.md, cavekit-ui-shell.md

## Changelog
- 2026-06-30: Initial draft.
- 2026-07-04: Status reconciled to implementation — all R1–R6 verified (NAV/FAIL/HISTORY/REDIRECT/RULES/LINK PASS).
