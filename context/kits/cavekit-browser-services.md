---
created: "2026-06-30"
last_edited: "2026-07-31"
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
- [ ] Cookies and the HTTP disk cache PERSIST across daemon restarts (profile dir provided to the engine — `$JIHAD_PROFILE_DIR`, default `<greDir>/../profile`). Without persistence, consent/login cookies die with the process and consent-gated modern sites re-prompt every launch (the Atlas WPE lesson). cookies.sqlite lives on VFAT: `toolkit.storage.synchronous=2` forces fsync-ordered journaling so power loss rolls back cleanly; the disk cache is disposable (checksummed, corruption = miss). *(IMPLEMENTED 2026-07-17, checkbox pending on-device verification: restart daemon, confirm a consent cookie survives + `/media/internal/jihad/profile/cookies.sqlite` exists.)* *(2026-07-31 reconciliation — that verification RAN and FAILED: `../impl/device-test-2026-07-19.md` Session 4 (2026-07-20) records "cookie/cache PERSISTENCE — GAP FOUND (not working on device)". The profile provider is wired (EngineHost passes `sJihadDirProvider` to `XRE_InitEmbedding2`, ProfD+ProfLD → `/media/internal/jihad/profile`), the prefs are correct (cookieBehavior=0, lifetimePolicy=0, `toolkit.storage.synchronous=2`), and the engine DOES create `startupCache/` there — but NO `cookies.sqlite` is ever created, even after loading github.com/duckduckgo.com plus a daemon restart. Cookies work in-session only. Open debug: cookie-service init on VFAT, or a ProfD-vs-startupCache path mismatch.)*
- [ ] Low-memory guardrail: the renderer watches available system memory (rate-limited /proc/meminfo poll of MemFree+Buffers+(Cached−Shmem)) and fires the engine memory-pressure flush + malloc_trim when RAM runs short, throttled against GC thrash — 512 MB Pre 3 floor. Threshold via `$JIHAD_MEM_LOW_KB` (default 48 MB). *(IMPLEMENTED 2026-07-17 — pattern: Palm memchute watcher + Atlas memory budget; checkbox pending verification: heavy multi-site session on-device shows the `memory pressure:` log line and no blank-degradation/OOM.)* *(2026-07-31 reconciliation — HALF verified on device: `../impl/device-test-2026-07-19.md` Session 4 (2026-07-20) records the low-RAM prefs ACTIVE — daemon log "prefs check: surfacecache.max_size_kb=32768 (goanna.js low-RAM block ACTIVE)". Box stays open because the same record says the memory-pressure FLUSH line "needs a heavy multi-site session to trigger — not yet observed", which is exactly this AC's subject.)*
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
- 2026-07-17: R2 gained cookie/disk-cache PERSISTENCE (profile dir provider) and the low-memory guardrail (memory-pressure watcher), both ported patterns from Atlas/stock BrowserServer — see NOTICE. goanna.js gained Arctic-Fox/Mypal68-style JS-heap + media/network low-RAM prefs (build/webos-oe/make-device-bundle.sh).
- 2026-07-31: Reconciliation against recorded evidence (no box changed state; both stay open). R2's cookie/cache-persistence AC now records that the pending on-device verification RAN on 2026-07-20 and FOUND A GAP — no `cookies.sqlite` is created in the profile despite a correct dir provider + prefs (`../impl/device-test-2026-07-19.md` Session 4); the note previously read as merely "pending". R2's low-memory AC now records the half that IS device-verified (low-RAM prefs block ACTIVE in the daemon log) and the half that is not (the memory-pressure FLUSH line, never observed) — same source.
