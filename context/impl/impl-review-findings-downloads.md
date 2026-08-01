---
created: "2026-07-31"
last_edited: "2026-07-31"
---

# Adversarial review — download lifecycle + cookie persistence (commit 8953eab)

**Reviewer: fable** (this project's cross-model adversarial reviewer; replaces the Codex CLI used
for reviews 1–5 — see [[jihad-codex-limit]]). Read-only review of `git show 8953eab`.

**No P0.** The frozen YAP surface was checked specifically and is intact: `BrowserServerBase.cpp`
untouched; the emitters use the existing frozen `msgDownloadStart/Progress/Error/Finished`
(0x2010/0x2011/0x2012/0x2013) and `asyncCmdCancelDownload` (0x1015) with identical field order and
struct layout; the new virtuals are appended at the end of `IPageMessageSink`.

Suspicions raised and **cleared** with evidence (recorded so they are not re-investigated):
refcount balance in `JihadTransferFactory::CreateInstance`; `nsCOMPtr<nsICancelable>` lifetime per
`nsITransfer.idl`; all callbacks main-thread-only via `NotifyTransfer`/`OnDataAvailable`
(`MOZ_ASSERT(NS_IsMainThread())`) — no repeat of the NSS marshaling bug; `gSink` not dangling
(destruction order in `Main.cpp`); Finished-after-cancel impossible (`BackgroundFileSaver::Finish`
overrides `mStatus`); path traversal in `file->Append(suggestedName)` closed upstream by
`nsExternalHelperAppService.cpp:1206-1208`.

| # | Sev | Where | Defect | Status |
|---|-----|-------|--------|--------|
| F-1 | P1 | `JihadBrowserServer.cpp:86-102` | Download messages route to `mLastProxy` (the *last-connected* card), not the card that started the download. With two cards, progress/finished go to the wrong client; and `reap()` nulls `mLastProxy` without re-electing, so after that card closes **every** remaining message including the one terminal is discarded. | FIXED |
| F-2 | P1 | `DownloadService.cpp:131-133,76-83` | `CreateFailedTransfer` (temp-file setup failed) emits `msgDownloadStart`, then is pinned in `gActive` forever with **no terminal message** — `NotifyTransfer` never runs. Every later `cancelDownload` then reports "aborted" while doing nothing, because `nsExternalAppHandler::Cancel` early-returns on `mCanceled`. | FIXED |
| F-3 | P2 | `DownloadService.cpp:266-281` | `jihadDownloadDir()` bypasses `JihadRuntimePaths.h`: no `/media/internal` guard (so `JIHAD_DOWNLOAD_DIR` can point at the user's volume — **R8 violation**), no `RuntimeDirUsable` validation (accepts a pre-existing dir, follows symlinks), and since the upstart job never sets that variable the real device path is the `NS_OS_TEMP_DIR` fallback → `/tmp`, which no `prerm` cleans. Also feeds F-2 by filling `/tmp`. | FIXED |
| F-4 | P2 | `DownloadService.cpp:222-227` | `CancelDownload(url)` aborts **every** transfer matching the URL (no `break`) → duplicate `msgDownloadError` for the same URL; a cancel for an unknown/completed URL emits nothing at all, so the client cannot distinguish "already done" from "never existed". | FIXED |
| F-5 | P2 | `DownloadService.cpp:183,194-199` | `FinalizePartFile()` discards `MoveTo`'s result and `Finish()` emits `msgDownloadFinished` regardless — a failed rename reports success pointing at the 0-byte placeholder `BeginSave` created. | FIXED |
| F-6 | P2 | `test/download_client.cpp:224` | `progressOK = progressBig >= 1` is satisfied by the single terminal progress notification `NotifyTransfer` always sends, so **the Progress half of the commit's claim is untested** — deleting per-chunk progress entirely would still PASS. | FIXED |
| F-7 | P3 | `build-download2-test.sh:125-127` | Prints the leftover-file count but asserts nothing, so the cancel-cleanup behaviour the commit claims is untested. | FIXED |
| F-8 | P3 | `build-cookie-test.sh:72-74` | The idempotence guard greps for a `jihad-embed` marker the script never writes, so the pref line is appended to the shared build output `$DIST/bin/goanna.js` on **every** run, unbounded. (Pre-exists in `build-services-test.sh`; replicated here.) | FIXED |
| F-9 | P3 | `DownloadService.cpp:106-111` | In-flight transfers are never cancelled at engine shutdown; `gActive` is leaked and every interrupted download leaves a `.part` plus an empty placeholder behind. Verified as litter/leak, **not** use-after-free. | FIXED |

## On the cookie claim

The reviewer independently confirmed the diagnosis against engine source: `nsXREDirProvider::GetFile`
returns `NS_ERROR_FAILURE` for the profile keys while `!mProfileNotified`, **before** consulting
`mAppProvider` (`toolkit/xre/nsXREDirProvider.cpp:288-309`), and only `DoStartup()` sets that flag —
so without `XRE_NotifyProfile()` the embedder's provider was genuinely unreachable and
`nsCookieService` fell back to memory-only. The test is **not vacuous**: run 2 is a separate process,
`/echo` reflects the `Cookie:` header the client actually sent, the cookie is `Max-Age=3600` (not a
session cookie), and the `cookies.sqlite` check is ANDed with the behavioural assertion.

Two honest limits, recorded rather than hidden:
- It proves persistence across a **clean** `XRE_TermEmbedding` only. The device case is a SIGTERM'd
  upstart daemon (and `respawn` after a crash) with no `profile-before-change`. Whether the device
  loses cookies on SIGTERM is **UNPROVEN**.
- R2 also names the **HTTP disk cache**; this test covers cookies only. R2 correctly stays open.
