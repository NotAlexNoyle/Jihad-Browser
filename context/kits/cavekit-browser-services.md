---
created: "2026-06-30"
last_edited: "2026-11-07"
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
- [x] `setBlockPopups` and `setAcceptCookies` take effect. *(2026-10-01, CE 3.1.0, shipping Enyo card driven through its db8 records: `setAcceptCookies 0` → a page's cookie refused, `1` → kept; `setBlockPopups 0/1` → a load-time `window.open` attempted / blocked.)*
- [x] A permitted `window.open` reaches the user: a script's request for a new window on a web address opens a new card (the frozen `msgCreatePage` → the card connects with that identifier → the address loads in it), and the opener is handed `null`, never an exception. A blocked one opens nothing. *(Added and met 2026-10-01; before, `window.open()` threw `NS_ERROR_FAILURE` with the blocker on and off. Device, all three variants, `render/goanna/test/jihad-popup.html` over http: a tap → `window.open: handing the address to a new card` → `createPage -> card: id=N` → `connect: card for createPage id=N` → the target's title in the new card (`../impl/evidence-2026-10-01/window-open-new-card_enyo_mochi_mojo.jpg`); a load-time call opens no card with the blocker on and a card with it off; `about:blank` is refused. Deliberate limits: the opener gets no window handle (no `opener`, no `postMessage`); only http/https/ftp addresses are handed over; a link or form with a `target` loads in the window that asked. Fixed alongside: the Mochi card set its identifier after the adapter connected; the adapter now re-sends its connect state when a real identifier arrives late.)*
**Dependencies:** none

### R2: Cache and cookie management
**Description:** Cache and cookies can be inspected/cleared per the contract.
**Acceptance Criteria:**
- [x] `clearCache` empties the engine cache.
- [x] `clearCookies` removes stored cookies.
- [x] With cookie acceptance disabled, set-cookie responses do not persist.
- [x] Cookies and the HTTP disk cache PERSIST across daemon restarts (profile dir provided to the engine — `$JIHAD_PROFILE_DIR`, default `$APP/profile` on cryptofs, derived in `render/goanna/JihadRuntimePaths.h`). Without persistence, consent/login cookies die with the process and consent-gated modern sites re-prompt every launch (the Atlas WPE lesson). cookies.sqlite lives on VFAT: `toolkit.storage.synchronous=2` forces fsync-ordered journaling so power loss rolls back cleanly; the disk cache is disposable (checksummed, corruption = miss). *(Device 2026-08-04: a year-long cookie written by the inject `cookie set` probe reads back after a daemon restart, alongside real `.github.com` cookies from earlier browsing. The 2026-07-20 failure (no `cookies.sqlite`, `../impl/device-test-2026-07-19.md`) was the VFAT profile at `/media/internal`; the profile is now `$APP/profile` on cryptofs (cavekit-device-build.md R8). Desktop: `build/desktop/build-cookie-test.sh`, COOKIE-PERSISTENCE PASS (commit 8953eab). `nsICookieService::SetCookieString` needs a channel; passing null crashes the daemon.)*
- [x] Low-memory guardrail: the renderer watches available system memory (rate-limited /proc/meminfo poll of MemFree+Buffers+(Cached−Shmem)) and fires the engine memory-pressure flush + malloc_trim when RAM runs short, throttled against GC thrash — 512 MB Pre 3 floor. Threshold via `$JIHAD_MEM_LOW_KB` (default 48 MB). *(Device 2026-08-04: `memory pressure:` logged 5 times in ~112 s of a multi-site session, which also shows the 30 s throttle on a 5 s poll; the daemon stayed alive and painting. The `$JIHAD_MEM_LOW_KB` ceiling was raised from 256 MB to 1 GB so the guardrail can be made to fire on demand (an 8-site session moved available memory only 441 → 406 MB); the shipped 48 MB default is unchanged.)*
- [x] The same actions are reachable via the LunaService methods on the device build. *(2026-08-05 — via this variant's OWN service, `palm://net.riverstonerelay.jihadBrowser/`, not `com.palm.browserServer`; full evidence and the three naming/role/struct traps are in cavekit-ipc-contract.md R4. The apps now call their own variant's URI.)*
  *(2026-08-06: the 2026-08-05 check used a privileged private-bus caller, and the daemon then registered with `LSRegister` (private only), so the cards' `clearCookies`/`clearCache` had never worked. It now registers with `LSRegisterPalmService` + `LSPalmServiceRegisterCategory` + `LSGmainAttachPalmService` against the public role files `packaging/gen-variant-scripts.sh` installs; public `clearCache` as the card's own appId (`luna-send -P -a net.riverstonerelay.jihad-browser`) replies and the daemon logs it. `getChromePrefs` is private-only (it returns the user's home page and shortcuts; public callers get `Unknown method`). Not tested by a person: tapping the Enyo clear-data buttons.)*
*(2026-10-01 (third session), CE 3.1.0, Enyo: a persistent cookie is still there after `stop`/`start` (`cookie count=1` both sides); the HTTP DISK CACHE persists too — the second load of the same URL after the restart reached the server as a conditional request (`200`, then `304`); `clearCookies` on the PUBLIC bus empties the jar (`count=0`) and `clearCache` makes the next load a `200` again; with `JIHAD_MEM_LOW_KB=1000000` the guardrail fired (`memory pressure: 468240 kB available (< 1000000 kB) — flushing engine caches`).)*
**Dependencies:** cavekit-ipc-contract.md (R4)

### R3: JavaScript dialogs bridged with blocking semantics
**Description:** Script-initiated dialogs surface to the client and block the page until answered.
**Acceptance Criteria:**
- [x] `alert`, `confirm`, `prompt`, and HTTP-auth dialogs emit the corresponding dialog messages carrying a sync reply path. *(Implemented 2026-08-03: `BrowserPageGoanna` IS the `DialogSink` — it creates the FIFO, emits `msgDialog*(syncPipePath, …)`, and registers itself. Desktop-verified: a real dialog reached a stand-in card and its answer came back.)* *(2026-10-01, CE 3.1.0, all three shells: HTTP auth had raised no dialog (the prompt factory answered `NS_NOINTERFACE` for `nsIAuthPrompt2`); `JihadPrompter` now implements `nsIAuthPrompt2::PromptAuth` and routes it to `msgDialogUserPassword`. A server wanting `jihad:pw`: the card shows "The server 192.168.1.245:8090 - jihad-test requires a username and password" (`../impl/evidence-2026-10-01/http-auth-dialog_enyo_mochi_mojo.jpg`), the daemon is held meanwhile, cancel → the 401 page, user+password → `AUTH-OK` (`401` then `200` at the server). The same day: `confirm` → `c: true`, `prompt` → `p: [hello]`, `alert` in each shell, each holding the page until the reply.)*
- [x] The page blocks until the client replies over the sync pipe, then resumes with the reply value. *(The daemon blocks on the FIFO — opened `O_NONBLOCK` and polled with a 60 s deadline, so an unanswered dialog takes its default instead of wedging the daemon — and parses the adapter's wire format: 4-byte big-endian length, then NUL-terminated args, arg0 `"1"`/`"0"` (`"2"` = SSL trust-once), arg1 prompt text or username. Desktop-verified through a real XPI install: accept came back, the page resumed, and the install completed with status 0. SSL confirm uses the same pipe (`msgSSLConfirm2`, R5).)*
- [x] **Dialogs and notices survive a card being closed.** *(Added and met 2026-10-02 — a defect found while preparing the removal audit.)* *(Every closing page cleared the process-wide dialog sink, which also unregistered the add-on install prompt and the JS notification door, so after the first card closed an `InstallTrigger` install was cancelled with no prompt (`callback status=-210`). The observers are now dropped only at process shutdown, and a closing page that owns the dialogs hands them to the newest live page. Device (CE 3.1.0, Enyo, daemon `db39962f…`): after three connects and two disconnects, `xpi confirm: … (sink=yes)`, accept → `callback status=0`, `notify: addon: Jihad Effect Test installed. (pushed)`. The two-cards case is from source.)*
**Dependencies:** cavekit-engine-embedding.md (R3)
**Device-verified 2026-08-03**: an engine confirm reached the Enyo card over the FIFO and its ACCEPT came back (`dialog confirm -> ACCEPT`), completing an XPI install on the TouchPad.

**The dialog deadline is sized for a person (2026-08-04).** `BrowserAdapter::js_sendDialogResponse`
opens the reply pipe only when the user taps a button, so the protocol gives no signal between
"shown" and "answered". The daemon waits one 60 s deadline (`JIHAD_DIALOG_MS` overrides it for
harnesses); an earlier 5 s "pickup" phase silently defaulted any dialog a person took longer than
5 s to answer, while every scripted harness passed. **If you add a dialog kind, do not reintroduce a
liveness heuristic based on the reply pipe.** The log times the wait; the engine takes 2 ms from
click to dialog emitted. `msgSSLConfirm2` takes the same FIFO; the empty-pipe `msgSSLConfirm`
survives only as a back-compat overload (`JihadBrowserServer.cpp`).

### R4: Downloads and MIME handoff
**Description:** Downloads and unsupported content are reported per the contract.
**Acceptance Criteria:**
- [x] A download emits start, progress, and finished messages; finished carries the temp file path and MIME type. *(2026-07-31, commit 8953eab: `build/desktop/build-download2-test.sh` drives the real daemon over YAP from a local HTTP server: `msgDownloadStart` → 16 × `msgDownloadProgress` → `msgDownloadFinished` with mime `application/octet-stream` and the 524288-byte temp file on disk. An `@mozilla.org/transfer;1` (`nsITransfer`) implementation is mandatory; without it `CreateTransfer` cancels every download.)*
- [x] `cancelDownload` aborts an in-progress download. *(2026-07-31, commit 8953eab — same test, second scenario: a 32 MB slow download is cancelled after 9 progress messages; the daemon logs `cancelDownload … -> aborted` and emits `msgDownloadError(0x804b0002 = NS_BINDING_ABORTED)` and NEVER `msgDownloadFinished`. The `nsICancelable` handed to `nsITransfer::init` is what is cancelled.)*
- [x] Unsupported MIME types emit a mime-not-supported / mime-handoff message rather than rendering.
**Dependencies:** cavekit-navigation-events.md (R6)

### R5: TLS / certificate error handling
**Description:** Certificate problems surface as a confirmable dialog.
**Acceptance Criteria:**
- [x] An invalid/untrusted certificate emits an SSL-confirm dialog with host, error code, and certificate reference. *(T-135, 2026-08-15, desktop: against a self-signed 127.0.0.1 endpoint the daemon emits `msgSSLConfirm2(pipe, "127.0.0.1", 18, "<state>/certs/jihad-cert-43-1.pem")`, and that PEM's SHA1 fingerprint matches the server's certificate; `../impl/impl-cert-store.md`. Device: the cert-store criterion below.)*
  - Certificate reference: `nsIX509Cert::getRawDER` armoured as PEM, written under
    `render/goanna/JihadCertStore.cpp::certDir()` (`/var/ssl/jihad/<variant>` when usable, else
    `<runtime state dir>/certs`), mode 0644 so `com.palm.certificatemanager` can read it, unlinked
    when the dialog resolves.
  - Error code: `PalmSslErrorOrdinalFor(Problem, nsresult)` maps `nsISSLStatus` flags (untrusted,
    domain mismatch, not valid at this time, self-signed, `notAfter`) to the card's Palm ordinal. The
    nsresult is only the fallback: `0x805a2fe3` decodes to a transport "malformed server hello".
    Where OpenSSL's code and the card's wording disagree the wording wins: expiry → 0,
    not-yet-valid → 10.
- [x] Accepting proceeds with the load; rejecting aborts it. *(2026-08-04, against a real self-signed TLS server: accept → `STOP status=0x0` after the override; reject → no load, cert error `0x805a2fe3`. `nsIBadCertListener2::NotifyCertProblem` had never reached the embedder; patch 0014 routes it from `nsDocShell::GetInterface` through the tree owner to the chrome, as `nsIPrompt` travels. Recovering the cert after the failure is a dead end: the failed channel's `SSLStatus` is null and UXP has no `nsIRecentBadCerts`.)*
- [x] On the device build, this integrates with the webOS certificate store as the upstream path did. [human-review on device] *(Device 2026-08-15, TouchPad on 3.0.5, `../impl/impl-device-2026-08-15.md`: a self-signed TLS server from the card; the daemon writes the PEM to `/var/ssl/jihad/enyo` (so it can write `/var/ssl`), classifies `flags=1(untrusted=1 selfSigned=1 mismatch=1) -> ordinal 18`, and on Trust Once logs `CertInitCertMgr rc=0`, `Adding Entry with serial number 6 to DB`, `cert: platform store now trusts serial 6 (session)`, reloads, `STOP status=0x0`. All nine `libPmCertificateMgr.so` symbols resolve, including the read-direction ones. The FIFO reply is a wire frame (`printf '\000\000\000\002\062\000'`: 4-byte BE length + `"2\0"`), not a bare `2`.)*

  Design (T-135, `render/goanna/JihadCertStore.{h,cpp}`; full record `../impl/impl-cert-store.md`):
  `libPmCertificateMgr.so` is `dlopen`ed and only paths and serials cross the boundary, because the
  daemon must not link OpenSSL (the device has 0.9.8). Trust Always = `CertInstallKeyPackage` +
  `CertAddAuthorizedCert`; Trust Once = `CertInstallKeyPackage` + `CertAddTrustedCert`, its serials
  swept at teardown and at startup (`<state>/cert-session-serials`). Reading the platform store into
  the engine came later: `components/jihadCerts.js`, cavekit-proxies-certs.md R4. Trap:
  `render/browserserver/Src/BrowserServer.cpp` and `Src/Main.cpp` carry upstream's `USE_CERT_MGR`
  block but are not compiled.
- [x] **The TLS stack page loads actually use is stated, and the negotiated protocol version is CAPTURED from a real page load on the device — not inferred from a pref.** *(2026-10-01, CE 3.1.0, Enyo card connected, engine libxul `65e3cc84891c59e9ede40cc7b3de0802` (NSS 3.90): `https://www.howsmyssl.com/a/check` → `"tls_version":"TLS 1.3"` (21 suites offered, the TLS 1.3 suites first); `https://www.cloudflare.com/cdn-cgi/trace` → `tls=TLSv1.3`, `http=http/2`. Page TLS is the engine's own NSS (`security.tls.version.max=4`), independent of the platform's OpenSSL 0.9.8 and of CE's side-by-side 1.1.1w; Jihad links neither. CE's OpenSSL matters only to the platform cert store and to anything that shells out to `curl`.)*
- [x] **The platform cert store path still works on webOS CE 3.1.0.** *(2026-10-01, CE 3.1.0: `libPmCertificateMgr.so` is the same Dec-2011 library (md5 `9414761f69ae21afc6bb2e34c0608d9a`, links OpenSSL 0.9.8). Self-signed host endpoint `https://192.168.1.245:8443`, Enyo, Trust Once over the reply FIFO: PEM in `/var/ssl/jihad/enyo/` → `ordinal 18` → `CertInitCertMgr rc=0` → `Adding Entry with serial number 3 to DB` → `… trusts serial 3 (session)` → `load done uri=https://192.168.1.245:8443/index.html`, page marker `CERT PAGE LOADED`; the card rendered the ordinal-18 sentence and three answers (`../impl/evidence-2026-10-01/ssl-confirm-dialog-enyo-no-preceding-alert.jpg`). Fixed in the run: after trust the daemon reloaded the page being left instead of retrying the failed URL; the engine's own connection-error alert came before the trust question and is now suppressed when a certificate error was captured. Not exercised: Trust Always, and the card's "View Certificate" button (still disabled two seconds after the dialog appeared); both need a person at the device.)*
**Dependencies:** none (desktop TLS-confirm flow stands alone; device cert-store integration is tracked one-way in cavekit-device-build.md R4)

## Out of Scope
- The navigation event stream itself (cavekit-navigation-events.md).
- Rendering of any resulting page (cavekit-offscreen-rendering.md).

## Cross-References
- See also: cavekit-ipc-contract.md, cavekit-navigation-events.md, cavekit-engine-embedding.md, cavekit-device-build.md

## Changelog
- 2026-11-07: sweep; long evidence notes shortened, history moved verbatim to `archive/browser-services-history.md`.
- 2026-10-01 (later still): R5 cert-store-on-CE criterion `[ ]`→`[x]`; two defects found in the run and
  fixed in the daemon (post-trust reload went to the page being left; a redundant engine error
  alert preceded the trust prompt).
- 2026-10-01 (later): R5 TLS criterion `[ ]`→`[x]` — negotiated version captured on the device from
  two servers: `TLS 1.3` (howsmyssl) and `TLSv1.3` over HTTP/2 (Cloudflare trace).
- 2026-10-01: target platform is now webOS CE 3.1.0; R5 gained the TLS-stack and cert-store-on-CE criteria. 3.0.5-only device boxes are listed in cavekit-device-build.md R4's re-verification sweep.
- 2026-06-30: Initial draft.
- 2026-07-04: R1, R2 clear cache/cookies, R3 dialogs, R4 MIME handoff and R5 SSL confirm/reject verified on desktop (SETTINGS2/SERVICES/DIALOG/DOWNLOAD/TLS PASS).
- 2026-07-17: R2 gained cookie/disk-cache persistence and the low-memory guardrail, ported from Atlas/stock BrowserServer (see NOTICE); low-RAM prefs in `build/webos-oe/make-device-bundle.sh`.
- 2026-07-31: R4 closed on desktop (commit 8953eab, DOWNLOAD-LIFECYCLE PASS); R2 persistence proven on desktop (COOKIE-PERSISTENCE PASS).
- 2026-08-15: R5 first criterion closed (T-135): PEM certificate reference, Palm ordinal from `nsISSLStatus` flags, three-way answer (Trust Always writes `cert_override.txt`). New `render/goanna/JihadCertStore.{h,cpp}`; record `../impl/impl-cert-store.md`.
