---
created: "2026-07-08"
last_edited: "2026-07-08"
---

# Cavekit Adversarial Review #5 — tap→link-navigation + self-contained adapter

Scope: work since review #4 (`c8bd05a`). Reviewer: **cavekit `ck:inspector`** driving
**`codex review` / `codex exec`** (gpt-5.5, reasoning=high) as the adversarial engine,
synthesized against `context/kits/`.

- **Uncommitted (priority):** the tap→link-navigation path in
  `render/goanna/{GoannaRenderPage.cpp,.h,BrowserPageGoanna.cpp}`. Background: the click
  default-action does not navigate links in the offscreen/headless embedding, so the
  daemon resolves the tapped `<a>` href and navigates itself. An earlier build called
  `openUrl` synchronously inside the `clickAt` socket callback → re-entrant page teardown
  → SIGSEGV → core-dump I/O stall that **rebooted the device**.
- **Committed (`38418c2`):** self-contained coexistence — `render/adapter/` import
  (MIME `application/x-jihad-browser`, YAP name `jihad-browser`, Atlas-derived dstBuffer
  paint path), `packaging/`, build scripts.

**Verdict: REVISE** → all findings below **resolved**. `link_test` passes (`LINK PASS`,
`links=1 linkUrl=.../b`) after the fix.

## Findings and resolutions

| # | Sev | Finding | Resolution |
|---|-----|---------|-----------|
| H-1 | HIGH | Re-entrancy fix was incomplete: only `openUrl` was deferred, but `ActivateContent` (focus/blur/activate events), `ElementFromPoint(flushLayout=true)`, and the non-link `SendMouseEvent`+`DOMClick` still ran synchronously in the `clickAt` YAP socket callback (`mInTick==false`, no page-lifetime guard). Page script (onfocus/onmousedown/onclick → location/history/document.open) could tear the document down mid-callback → use freed engine objects → the same class of crash that rebooted the device. | **Fixed**: `BrowserPageGoanna::clickAt` now only records `(x,y,numClicks)`; **all** DOM interaction (`mPage->ClickAt`) runs in `pump()`, which executes inside `JihadBrowserServer::tick()`'s `mInTick`/`mReap` guard where page deletion is deferred. |
| H-2 | HIGH | Tapped links no longer emitted `msgLinkClicked` (navigation-events **R6** regression; `link_test` fails): routing through `openUrl→LoadUrl→BeginLoad` sets `mProgrammaticLoad`, so `OnStateChange` never sets `mLinkClicked`. | **Fixed**: `pump()` drains `TakeClickNav` and calls `mSink.msgLinkClicked(href)` before `openUrl`. `link_test` → `LINK PASS`. |
| M-1 | MED | `#fragment` links restarted the full load lifecycle: `GetHref` resolves to absolute, so `#frag` → `<current>#frag` passed the `http://` navigable test → full reload (stuck overlay, wrong history). | **Fixed**: `ClickAt` compares the resolved href sans-fragment to `CurrentUri()` sans-fragment; a same-document fragment link is treated as non-navigable (falls to the in-page click path). |
| M-2 | MED | `mClickNavUrl`/pending tap was never invalidated by a newer explicit nav → a queued tap could clobber a later address-bar/back/forward navigation. | **Fixed**: `mPendingClick=false` at the top of `openUrl`, `pageBackward`, `pageForward` — newest user intent wins. |
| M-3 | MED | Deferred nav ran *after* the pump budget was spent → single-pump callers (`link_test`) never processed the started load in that call. | **Fixed**: the queued tap + its navigation are processed at the **top** of `pump()`, before `PumpFor(msBudget)`. |
| M-4 | MED | `render/adapter/BrowserAdapter.cpp` fit-zoom guard compared old width to `mViewportHeight` (typo); on a 768x1024↔1024x768 rotation old-width==new-height so the branch was skipped and landscape kept the portrait zoom. | **Fixed**: compare to `mViewportWidth`. |
| M-5 | MED | `createBufferLock` treated `sem_open` failure as success — `SEM_FAILED` is `(sem_t*)-1` (non-zero), so `!= 0` passed; later `sem_post`/`sem_close` hit `(sem_t*)-1` and the shared-framebuffer lock (IPC R2) was silently absent. | **Fixed**: `if (m_bufferLock == SEM_FAILED) { m_bufferLock = 0; return false; }`. |
| M-6 | MED | Test harnesses (`build-{link,redirect,tls}-test.sh`) captured `RC=$?` but exited with the trailing `echo`'s status (0) → a failing test (incl. the H-2 regression) reported PASS to any exit-code-keyed CI. | **Fixed**: appended `exit "$RC"` to all three. |
| L-1 | LOW | Anchor-ancestor walk capped at depth 16 → deeply-nested inline link text wouldn't resolve. | **Fixed**: cap raised to 256 (still bounded as a cycle guard; the parent chain terminates at the document). |
| L-2 | LOW | `numClicks` ignored for links (double-tap direct-navigated like a single tap). | **Fixed**: direct-nav gated on `numClicks == 1`; otherwise falls to the mouse/DOMClick path. |
| L-3 | LOW | Adapter blit `srcStride = renderedWidth*4` assumes rendered==buffer width. | **Accepted** (self-consistent on the verified path; documented invariant `renderedWidth <= bufferWidth`). |

## Verified good (unchanged, credited by the review)
- **IPC wire contract preserved (R1/R5)**: client + server expose the identical 72 YAP
  opcodes with identical names/types; the rebrand is confined to plugin MIME +
  transport socket name. No command/message added, removed, renamed, or re-typed.
- **Pump-tick deferral mechanism is sound**; the paint blit is destination-bounded with
  a portrait/landscape orientation guard.

## Cavekit kit updates (proposed by the review)
- **cavekit-input-bridging R1**: add — input handlers must not perform navigation/engine
  teardown synchronously in the YAP socket callback; teardown-capable work runs in the
  page-lifetime-protected pump/tick (justified by H-1).
- **cavekit-navigation-events R6**: tighten — a link activated by tap/click (not only an
  engine content-initiated load) emits `msgLinkClicked`, verified by `link_test` (H-2).
- **cavekit-navigation-events R7 (new)**: same-document (`#frag`) navigation must not
  restart the full load lifecycle (M-1).
