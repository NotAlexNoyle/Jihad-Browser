---
created: "2026-07-19"
last_edited: "2026-07-19"
---
# Implementation Tracking: mochi-ui

Build site: context/plans/build-site.md

| Task | Status | Notes |
|------|--------|-------|
| T-049 | DONE | 2026-07-18, commit c6d0e9a. appinfo title → "Jihad (Mochi)"; icons md5-match app/; `build/webos-oe/build-mochi-ipk.sh` bundles Enyo2 core (`../mochi-sampler/enyo`) + layout (`webos-stacks/mochi/lib/layout` — mochi-sampler lib/ dirs EMPTY; `LAYOUT_SRC` override) + Mochi (`../mochi`) → 1.4 MB ipk, 394 entries, verified rebuild on merged main. Dev dirs pruned (~12 MB cut). Dual-install on device pending hardware |
| T-050 | TODO | Mochi licensing/attribution (Apache headers, NOTICE) |
| T-051 | TODO | Enyo-2 WebView control bound to unchanged BrowserAdapter; must also route MIME `application/x-jihad-browser` (self-contained arch, see app/source/JihadEngineOverride.js) |
| T-052 | TODO | Mochi controls + layout both TouchPad models |
| T-053 | TODO | Feature-parity port (blocked by T-051, T-052) |

## Notes
- Framework sources stay outside repo; bundle at build time only.
- `../mochi-sampler/lib/{layout,mochi}` are empty dirs — real layout lives at
  `webos-stacks/mochi/lib/layout`; its enyo + mochi trees byte-identical to the
  packet sources (2.5.1-pre.1).
