---
created: "2026-07-01"
last_edited: "2026-07-01"
---

# Codex Adversarial Review #4 — redirect/TLS/input/lifecycle increments

Scope: new code since review #3 (`b12b6f3`): redirect detection, TLS cert-error
detection, input coordinate mapping, drag/holdAt, freeze/thaw, safe findString,
addUrlRedirect, link-clicked, insertStringAtCursor. Reviewer: Codex CLI.

## Findings and resolutions

| # | Sev | Finding | Resolution |
|---|-----|---------|-----------|
| 1 | P0 | `mapToContent`/`dragProcess`: tiny zoom (e.g. 1e-9) → `surface/zoom` overflows the `int` cast | **Fixed**: `mZoom` clamped to [0.05, 20] on set (setZoomAndScroll + SetZoom); results clamped to ±1e6 in both mapToContent and dragProcess. |
| 2 | P0 | `thaw` unfreezes + repaints even when shm validation fails → maybePaint could write a stale/reused segment | **Fixed**: thaw now validates both segments fit the surface and stays FROZEN on failure (keys not swapped, mFrozen stays true). |
| 3 | P1 | link-clicked heuristic too coarse; internal `LoadURI` (ScrollTo/InsertText) could be reported as link-clicked | **Fixed**: mProgrammaticLoad set true immediately before the internal `javascript:` LoadURI in ScrollTo + InsertText, so any document start they trigger is classified programmatic. |
| 4 | P1 | per-load reset only in BeginLoad; stale state after content-initiated nav | **Mitigated** by #3 (internal navs marked programmatic) + #5 (top-level scoping); a genuine link-click starts a fresh top-level load whose completion resets state. |
| 5 | P1 | `STATE_IS_DOCUMENT` ≠ top-level: iframe loads could set page-level redirected/cert/failed/link | **Fixed**: OnStateChange computes `top` (aWebProgress->GetDOMWindow == content window) and gates redirect, link, and failed/cert on it. |
| 6 | P1 | all security-module doc failures treated as cert errors, suppressing msgFailedLoad | **Dispositioned**: acceptable — reject aborts the load either way and accept (the override) is device-gated; the error code is carried so the adapter can distinguish. Comment added. |
| 7 | P2 | `insertStringAtCursor` javascript: escaping only handles quotes/backslashes | **Fixed**: escaping now handles newlines/CR/tab; the op is marked programmatic (#3). |

Mitigated (no change): POSIX regex rules are `regfree`'d only after successful
`regcomp` (no leak/double-free); findString is crash-safe (avoids FindNext).

## Verification after fixes
- Daemon builds + DAEMON_UP.
- FAIL-EVENT PASS, LINK PASS, REDIRECT PASS, COORDMAP PASS, FREEZE-THAW PASS
  (top-level scoping + clamps + thaw guard don't regress the happy paths).

Fourth Codex adversarial review; P0/P1 fixed or dispositioned. Prior reviews:
impl-review-findings{,-daemon,-3}.md.
