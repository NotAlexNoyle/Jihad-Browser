---
created: "2026-07-01"
last_edited: "2026-07-01"
---

# Codex Adversarial Review #3 — engine-integration increments

Scope: the ~1450 lines added since commit `ea3f54d` (dialogs, downloads, geometry
events, resize/scroll/zoom, settings, failed-load, history, PoC image output).
Reviewer: Codex CLI (codex-cli 0.142.3), adversarial C++/XPCOM pass.

## Findings and resolutions

| # | Sev | Finding | Resolution |
|---|-----|---------|-----------|
| 1 | P0 | `DialogService` `gSink` raw global pointer — dangling if owner destroyed before XPCOM teardown; off-thread race | **Fixed**: `EngineHost::Shutdown` clears both sinks before `XRE_TermEmbedding` (process-lifetime backstop); documented main-thread + clear-before-teardown contract on `SetDialogSink`. Tests already clear before shutdown. |
| 2 | P0 | `DownloadService` `gSink` — same raw-global hazard | **Fixed**: same backstop + contract (`SetDownloadSink`). |
| 3 | P1 | `DownloadService::Show` never resolves the launcher (no SaveToDisk/Cancel) | **Documented (API-limited)**: this UXP's `nsIHelperAppLauncher` has **no `Cancel()`**; actually saving/cancelling needs the adapter to pick a destination over YAP (device work). Comment added; interception-only is intentional for the headless daemon. |
| 4 | P1 | Failed-load not scoped to the top-level nav — a subresource failure could mark the page failed | **Fixed**: capture scoped to `STATE_IS_DOCUMENT` (main document), excluding subresource `STATE_IS_REQUEST` failures. |
| 5 | P1 | Failure state reset only in `LoadUrl`; back/forward/reload could bleed a stale failure | **Fixed**: factored `BeginLoad()` (resets done + failure) called from `LoadUrl`, `GoBack`, `GoForward`, `Reload`. |
| 6 | P1 | Scroll via `javascript:` breaks with JS disabled | **Accepted limitation**: the frozen embedding API exposes no scroll entry point; documented. JS-off absolute scroll is an edge case; revisit with a non-frozen path on device. |
| 7 | P2 | `GetContentSize` via `GetRootBounds` may report viewport, not doc size | **Verified non-issue**: `geo_test` shows a 2500px page reports 2500 (full scrollable rect), not the 768 viewport. Comment added. |
| 8 | P2 | `ClearHistory` `PurgeHistory(count)` may be a no-op | **Verified working**: `history_test` shows canGoBack 1→0 after clear. Kept. |

## Verification after fixes
- NAV PASS, FAIL-EVENT PASS (no regression from scoping/BeginLoad).
- Daemon rebuilds + DAEMON_UP; adapter ROUND-TRIP PASS.

This is the third Codex adversarial review; P0/P1 items are fixed or documented as
API-/device-limited. See impl-review-findings.md and impl-review-findings-daemon.md
for the prior two.
