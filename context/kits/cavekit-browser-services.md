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
- [x] Cookies and the HTTP disk cache PERSIST across daemon restarts (profile dir provided to the engine — `$JIHAD_PROFILE_DIR`, default `$APP/profile` on cryptofs, derived in `render/goanna/JihadRuntimePaths.h`). Without persistence, consent/login cookies die with the process and consent-gated modern sites re-prompt every launch (the Atlas WPE lesson). cookies.sqlite lives on VFAT: `toolkit.storage.synchronous=2` forces fsync-ordered journaling so power loss rolls back cleanly; the disk cache is disposable (checksummed, corruption = miss). *(IMPLEMENTED 2026-07-17, checkbox pending on-device verification: restart daemon, confirm a consent cookie survives + `/media/internal/jihad/profile/cookies.sqlite` exists.)* *(2026-07-31 reconciliation — that verification RAN and FAILED: `../impl/device-test-2026-07-19.md` Session 4 (2026-07-20) records "cookie/cache PERSISTENCE — GAP FOUND (not working on device)". The profile provider is wired (EngineHost passes `sJihadDirProvider` to `XRE_InitEmbedding2`, ProfD+ProfLD → `/media/internal/jihad/profile`), the prefs are correct (cookieBehavior=0, lifetimePolicy=0, `toolkit.storage.synchronous=2`), and the engine DOES create `startupCache/` there — but NO `cookies.sqlite` is ever created, even after loading github.com/duckduckgo.com plus a daemon restart. Cookies work in-session only. Open debug: cookie-service init on VFAT, or a ProfD-vs-startupCache path mismatch.)* *(2026-07-31 — DESKTOP HALF NOW PROVEN, commit 8953eab: `build/desktop/build-cookie-test.sh` sets a persistent cookie, tears the engine down, restarts it, and reads the cookie back — COOKIE-PERSISTENCE PASS with `cookies.sqlite` PRESENT in the profile. So the profile/dir-provider/prefs wiring is correct in principle and the 2026-07-20 device failure is NOT a design error. Box stays `[ ]` because the failure was device-specific (VFAT) and the on-device retest has not been re-run — and note the profile path in that device record, `/media/internal/…`, is itself now forbidden by cavekit-device-build.md R8, so the retest must happen against the new profile location. **REVISED 2026-08-01**: that location is `$APP/profile` on **cryptofs**, not `/var/palm/jihad/<variant>/` — `/var` has 49.6 MB free and is shared with system state, and both prior implementations of this browser on this device put cookies on cryptofs (isis: `CookieJarPath=/media/cryptofs/.browser/cookies`; Atlas: `cookies.db` in its app deviceroot). Atlas states the reason our 2026-07-20 run failed: /media/internal is VFAT, with no hard links and no real file locking, which is exactly what SQLite needs. cryptofs gives create/rename/lock. That change alone may resolve it.)* *(**2026-08-04, device: it did — at least for the half that was failing.** `$APP/profile/cookies.sqlite` NOW EXISTS on the TouchPad (98 KB, with live `-shm`/`-wal` companions updated during this session), directly contradicting the 2026-07-20 record that "NO cookies.sqlite is ever created". The same profile directory also now holds `places.sqlite`, `permissions.sqlite`, `webappsstore.sqlite`, `cert9.db` and the installed extensions — i.e. SQLite works there, which is exactly what VFAT denied. What is still NOT demonstrated is a cookie VALUE surviving a restart: nothing loaded during this session set a persistent cookie, so the store is schema-only. A `cookie set|count` probe was added to the inject channel for precisely that test (write a year-long cookie, restart the daemon, count) — it is committed and unrun. **RESOLVED the same day — DEVICE-VERIFIED.** With a `cookie set`/`cookie count` probe: a year-long cookie written in one daemon, the daemon stopped, a fresh one started — and it reads back, alongside two REAL `.github.com` cookies (`_octo`, `logged_in`) left over from earlier browsing, i.e. persistence across many restarts and a reboot, not just this test. Desktop shows the same (set → restart → count=1). The 2026-07-20 failure was the VFAT profile location, exactly as the Atlas note predicted; moving to `$APP/profile` on cryptofs (R8) fixed it.
      One trap recorded at the call site: `nsICookieService::SetCookieString` REQUIRES a channel — it derives origin attributes and the third-party decision from it and dereferences it unchecked, so passing null SIGSEGVs the daemon at address 0. That is what an earlier version of this probe did, and it is why a desktop run of it appeared to do nothing at all (the daemon had died mid-test).)*
- [ ] Low-memory guardrail: the renderer watches available system memory (rate-limited /proc/meminfo poll of MemFree+Buffers+(Cached−Shmem)) and fires the engine memory-pressure flush + malloc_trim when RAM runs short, throttled against GC thrash — 512 MB Pre 3 floor. Threshold via `$JIHAD_MEM_LOW_KB` (default 48 MB). *(IMPLEMENTED 2026-07-17 — pattern: Palm memchute watcher + Atlas memory budget; checkbox pending verification: heavy multi-site session on-device shows the `memory pressure:` log line and no blank-degradation/OOM.)* *(2026-07-31 reconciliation — HALF verified on device: `../impl/device-test-2026-07-19.md` Session 4 (2026-07-20) records the low-RAM prefs ACTIVE — daemon log "prefs check: surfacecache.max_size_kb=32768 (goanna.js low-RAM block ACTIVE)". Box stays open because the same record says the memory-pressure FLUSH line "needs a heavy multi-site session to trigger — not yet observed", which is exactly this AC's subject.)*
- [ ] The same actions are reachable via the LunaService methods on the device build.
**Dependencies:** cavekit-ipc-contract.md (R4)

### R3: JavaScript dialogs bridged with blocking semantics
**Description:** Script-initiated dialogs surface to the client and block the page until answered.
**Acceptance Criteria:**
- [x] `alert`, `confirm`, `prompt`, and HTTP-auth dialogs emit the corresponding dialog messages carrying a sync reply path. *(Implemented 2026-08-03: `BrowserPageGoanna` IS the `DialogSink` — it creates the FIFO, emits `msgDialog*(syncPipePath, …)`, and registers itself. Desktop-verified: a real dialog reached a stand-in card and its answer came back.)* **HISTORY — this AC was previously marked met in error, 2026-08-03 — this was marked met on the strength of `render/goanna/test/dialog_test.cpp`, which installs its OWN `DialogSink` and asserts the ENGINE side calls it. That is real, but it is only half the path: the DAEMON never calls `SetDialogSink` at all** (grep: the only callers are that test and the shutdown clear), so in the shipping binary `gSink` is null and an engine dialog reaches no card. Found while wiring the XPI confirm, which logs `sink=NONE — denying` on device: the prompt is raised correctly and then has nobody to ask. The card ends of this are built (Enyo/Mochi/Mojo all present dialogs) and the adapter carries the messages; what is missing is the daemon-side sink that turns an engine dialog into `msgDialog*`.
- [x] The page blocks until the client replies over the sync pipe, then resumes with the reply value. *(The daemon blocks on the FIFO — opened `O_NONBLOCK` and polled with a 60 s deadline, so an unanswered dialog takes its default instead of wedging the daemon — and parses the adapter's wire format: 4-byte big-endian length, then NUL-terminated args, arg0 `"1"`/`"0"` (`"2"` = SSL trust-once), arg1 prompt text or username. Desktop-verified through a real XPI install: accept came back, the page resumed, and the install completed with status 0. `msgSSLConfirm` still passes an empty pipe path — SSL confirm is the one dialog not yet routed this way.)*
**Dependencies:** cavekit-engine-embedding.md (R3)
**Device-verified 2026-08-03**: an engine confirm reached the Enyo card over the FIFO and its ACCEPT came back (`dialog confirm -> ACCEPT`), completing an XPI install on the TouchPad.

**The deadline has to be sized for a PERSON (2026-08-04).** The wait was two-phase: 5 s for
the card to "pick up" (POLLHUP clearing on the reply FIFO), then 60 s for the answer. The
premise was wrong. `BrowserAdapter::js_sendDialogResponse` opens the pipe ONLY when the user
taps a button — nothing opens the write end while the dialog is merely on screen — so the
pickup phase never ended, the 5 s deadline always won, and **any dialog a human took longer
than five seconds to answer was silently defaulted**, with the FIFO already unlinked by the
time they tapped. Every harness passed throughout, because a scripted answerer replies in
~300 ms; this is a defect that only a human could see, and the user reported it as "clicking
the button to install the add-on doesn't do anything". Now ONE 60 s deadline (`JIHAD_DIALOG_MS`
overrides it for harnesses), verified by answering deliberately late — an accept at 11.6 s
installs. **If you add a dialog kind, do not reintroduce a liveness heuristic based on the
reply pipe: this protocol gives no signal between "shown" and "answered".**

The wait is also timed in the log now (card-picked-up and answered), because "the dialog was
slow to appear" is otherwise unattributable. The engine side measures 2 ms from click to
dialog emitted, so any visible lag is the front-end drawing it.
**Remaining:** route `msgSSLConfirm` through the same FIFO — it still passes an empty pipe path, so a certificate prompt cannot be answered.

### R4: Downloads and MIME handoff
**Description:** Downloads and unsupported content are reported per the contract.
**Acceptance Criteria:**
- [x] A download emits start, progress, and finished messages; finished carries the temp file path and MIME type. *(2026-07-31, commit 8953eab — DOWNLOAD-LIFECYCLE PASS via `build/desktop/build-download2-test.sh`, which drives the REAL daemon over YAP from a local HTTP server and asserts the frozen messages: `msgDownloadStart` → 16 × `msgDownloadProgress` → `msgDownloadFinished` with mime `application/octet-stream` and the 524288-byte temp file present on disk. ROOT CAUSE of the previous gap: capturing only the `nsIHelperAppLauncherDialog` handoff left `nsExternalAppHandler::CreateTransfer` failing, which CANCELLED every download; registering an `@mozilla.org/transfer;1` (`nsITransfer`) implementation is mandatory, and its `init`/`onProgressChange64`/`onStateChange(STOP)` are what now drive the three messages.)*
- [x] `cancelDownload` aborts an in-progress download. *(2026-07-31, commit 8953eab — same test, second scenario: a 32 MB slow download is cancelled after 9 progress messages; the daemon logs `cancelDownload … -> aborted` and emits `msgDownloadError(0x804b0002 = NS_BINDING_ABORTED)` and NEVER `msgDownloadFinished`. The `nsICancelable` handed to `nsITransfer::init` is what is cancelled.)*
- [x] Unsupported MIME types emit a mime-not-supported / mime-handoff message rather than rendering.
**Dependencies:** cavekit-navigation-events.md (R6)

### R5: TLS / certificate error handling
**Description:** Certificate problems surface as a confirmable dialog.
**Acceptance Criteria:**
- [x] An invalid/untrusted certificate emits an SSL-confirm dialog with host, error code, and certificate reference. *(2026-08-03: it now carries a REPLY PIPE too — it was the last dialog passing an empty `syncPipePath`, so a card could show the prompt and never answer it.)*
- [~] Accepting proceeds with the load; rejecting aborts it. *(Both halves are now WIRED: the daemon waits on the FIFO and, on accept, calls `AcceptCurrentCert()` — which had existed since the TLS work with no caller outside its test — to remember the validity override, then reloads. Rejecting leaves the failed load standing, which is already the behaviour. NOT yet verified end to end: the desktop harness has no card to answer with, and a cert failure raises an engine ALERT before the SSL confirm, which wants investigating on its own. The `tls_test` path (`AcceptCurrentCert` + reload) still passes.)*
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
- 2026-07-31 (later): **R4 CLOSED on desktop** — both download boxes `[ ]`→`[x]` on commit 8953eab: registering an `@mozilla.org/transfer;1` implementation (the missing piece — without it `CreateTransfer` failed and cancelled every download) gives real start/progress/finished + a cancellable `nsICancelable`; DOWNLOAD-LIFECYCLE PASS. R2's persistence box gains the desktop proof (COOKIE-PERSISTENCE PASS, `cookies.sqlite` survives an engine restart) but stays open pending an on-device retest — which must now target the new `/var/palm/jihad/<variant>/` profile path required by cavekit-device-build.md R8, since the 2026-07-20 device failure was on VFAT.
- 2026-07-31: Reconciliation against recorded evidence (no box changed state; both stay open). R2's cookie/cache-persistence AC now records that the pending on-device verification RAN on 2026-07-20 and FOUND A GAP — no `cookies.sqlite` is created in the profile despite a correct dir provider + prefs (`../impl/device-test-2026-07-19.md` Session 4); the note previously read as merely "pending". R2's low-memory AC now records the half that IS device-verified (low-RAM prefs block ACTIVE in the daemon log) and the half that is not (the memory-pressure FLUSH line, never observed) — same source.
