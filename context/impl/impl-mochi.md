---
created: "2026-07-19"
last_edited: "2026-07-19"
---
# Implementation Tracking: mochi-ui

Build site: context/plans/build-site.md

| Task | Status | Notes |
|------|--------|-------|
| T-049 | DONE (desktop) — device residue open | 2026-07-18, commit c6d0e9a. appinfo title → "Jihad (Mochi)"; icons md5-match app/; `build/webos-oe/build-mochi-ipk.sh` bundles Enyo2 core (`../mochi-sampler/enyo`) + layout (`webos-stacks/mochi/lib/layout` — mochi-sampler lib/ dirs EMPTY; `LAYOUT_SRC` override) + Mochi (`../mochi`) → 1.4 MB ipk, 394 entries, verified rebuild on merged main. Dev dirs pruned (~12 MB cut). Dual-install on device pending hardware |
| T-050 | DONE | 2026-07-19, commit 2a79d71. Apache headers on all new app-mochi files; NOTICE gained Enyo 2 core + layout + Mochi (LG, Apache-2.0) credits, confirmed inside packaged ipk (closes codex F-390) |
| T-051 | DONE (desktop) — live handshake device-gated | 2026-07-19, commit 2a79d71. JihadWebView.js: NPAPI <object type="application/x-jihad-browser">, callBrowserAdapter surface frozen (set identical to app/: findInPage/goBack/goForward/reloadPage/stopLoad; Luna URIs clearCache/clearCookies), node.eventListener wiring, arg orders checked against render/adapter/BrowserAdapter.cpp |
| T-052 | DONE (desktop) — on-device layout review open | 2026-07-19, commit 2a79d71. Shell from mochi.Header/IconButton/InputDecorator/Input/ProgressBar/Popup + Fittable layout; inline SVG data-URI glyphs; no hardcoded px beyond shared 1024x768 |
| T-053 | PARTIAL | Shell built + polished on-device: Enyo-parity toolbar (back/forward, address with inline reload/stop, share, new-tab, history+bookmarks; PNG icon set), app-chrome crisp start page, no-autocap URL input, stageReady card-open. Remaining: bookmarks/history/downloads/find/prefs VIEWS + dialog set. Original: Feature-parity port: views (bookmarks/history/downloads/find/prefs/start page) + dialogs + full page/dialog/download callback surface + live-daemon nav wiring |

## Notes
- Framework sources stay outside repo; bundle at build time only.
- `../mochi-sampler/lib/{layout,mochi}` are empty dirs — real layout lives at
  `webos-stacks/mochi/lib/layout`; its enyo + mochi trees byte-identical to the
  packet sources (2.5.1-pre.1).
