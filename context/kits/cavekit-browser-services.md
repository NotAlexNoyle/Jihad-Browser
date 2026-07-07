---
created: "2026-06-30"
last_edited: "2026-07-04"
---

# Cavekit: Browser Services

## Scope
Page-level services the engine must provide behind the existing contract:
settings/preferences, cookie and cache management, JavaScript dialogs,
downloads and MIME handoff, and TLS/certificate error handling. Reference:
`docs/IPC-CONTRACT.md` (settings, dialog, download, SSL messages),
`render/goanna/PORT-MAP.md`.

## Requirements

### R1: Engine settings applied
**Description:** Settings commands change engine behavior.
**Acceptance Criteria:**
- [x] `setUserAgent` changes the User-Agent sent on subsequent requests (verifiable via an echo endpoint or local server log).
- [x] `setEnableJavaScript` enables/disables script execution.
- [x] `setMinFontSize` enforces a minimum rendered font size.
- [x] `setBlockPopups` and `setAcceptCookies` take effect.
**Dependencies:** none

### R2: Cache and cookie management
**Description:** Cache and cookies can be inspected/cleared per the contract.
**Acceptance Criteria:**
- [x] `clearCache` empties the engine cache.
- [x] `clearCookies` removes stored cookies.
- [x] With cookie acceptance disabled, set-cookie responses do not persist.
- [ ] The same actions are reachable via the LunaService methods on the device build.
**Dependencies:** cavekit-ipc-contract.md (R4)

### R3: JavaScript dialogs bridged with blocking semantics
**Description:** Script-initiated dialogs surface to the client and block the page until answered.
**Acceptance Criteria:**
- [x] `alert`, `confirm`, `prompt`, and HTTP-auth dialogs emit the corresponding dialog messages carrying a sync reply path.
- [x] The page blocks until the client replies over the sync pipe, then resumes with the reply value.
**Dependencies:** cavekit-engine-embedding.md (R3)

### R4: Downloads and MIME handoff
**Description:** Downloads and unsupported content are reported per the contract.
**Acceptance Criteria:**
- [ ] A download emits start, progress, and finished messages; finished carries the temp file path and MIME type.
- [ ] `cancelDownload` aborts an in-progress download.
- [x] Unsupported MIME types emit a mime-not-supported / mime-handoff message rather than rendering.
**Dependencies:** cavekit-navigation-events.md (R6)

### R5: TLS / certificate error handling
**Description:** Certificate problems surface as a confirmable dialog.
**Acceptance Criteria:**
- [x] An invalid/untrusted certificate emits an SSL-confirm dialog with host, error code, and certificate reference.
- [ ] Accepting proceeds with the load; rejecting aborts it.
- [ ] On the device build, this integrates with the webOS certificate store as the upstream path did. [human-review on device]
**Dependencies:** none (desktop TLS-confirm flow stands alone; device cert-store integration is tracked one-way in cavekit-device-build.md R4)

## Out of Scope
- The navigation event stream itself (cavekit-navigation-events.md).
- Rendering of any resulting page (cavekit-offscreen-rendering.md).

## Cross-References
- See also: cavekit-ipc-contract.md, cavekit-navigation-events.md, cavekit-engine-embedding.md, cavekit-device-build.md

## Changelog
- 2026-06-30: Initial draft.
- 2026-07-04: Reconciled — R1 settings + R2 clear cache/cookies + R3 JS dialogs (blocking) verified (SETTINGS2/SERVICES/DIALOG PASS); R4 MIME/download handoff captured (DOWNLOAD PASS); R5 invalid-cert->SSL-confirm + reject-aborts (TLS PASS). Pending: device LunaService cache/cookie methods, download start/progress/finished + cancel, SSL accept-proceeds + webOS cert store (device).
