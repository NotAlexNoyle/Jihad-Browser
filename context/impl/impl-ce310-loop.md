---
created: "2026-10-01"
last_edited: "2026-10-01"
---
# Implementation Tracking: webOS CE 3.1.0 loop (build site of 2026-10-01)

Build site: context/plans/build-site.md

Device: HP TouchPad (topaz), webOS CE 3.1.0 build 86. One device — every device task ran alone.

| Task | Status | Notes |
|------|--------|-------|
| T-201 | DONE | Housekeeping + pre-change baselines, see "Baselines" below. Enyo's leftover `MOZ_LOG=…` removed from `/etc/event.d/jihad` (`sed`, one token; `JIHAD_INJECT=1` intact in all three jobs). `/var` 12% (55.9 MB free). |
| T-202 | DONE | Engine fix `1fff8023` on `third_party/uxp` `jihad-engine-mods` (local, unpushed — T-279): `BasicPlanarYCbCrImage::CopyData` returns the base result, `true` under delayed conversion. |
| T-209 | DONE | `render/goanna/test/make-media-assets.sh` (11 clips, ffprobe-checked), `jihad-codecs.html` (per-format table), `jihad-input-echo.html`, `jihad-media.html` `#state` made fixed-width; `build/webos-oe/stage-media-pages.sh` stages them md5-verified into `/tmp/jm`. |
| T-216 | DONE | libxul rebuilt (incremental, 1 min). Stripped md5 `65e3cc84891c59e9ede40cc7b3de0802`. |
| T-219 | DONE | Engine-only push to enyo/mochi/mojo; all three read back `65e3cc84…`; daemon unchanged `cf92f351…`; goanna.js unchanged `5874eafd…`. Enyo is off the traced build. |
| T-220 | DONE | `<video>` plays to the end: 12/12 (VP8 + Theora × file + http × 3). gre-widgets R1 criterion → `[x]`. |

## Baselines (T-201, daemon `cf92f351…`, before any daemon change)

- **No-card idle, 62 s window:** mochi 148 CPU ticks (2.39% of a core), 7244 voluntary context
  switches (117/s), log +2970 B (48 B/s). mojo 146 ticks (2.35%), 7246 switches, +2970 B.
  Tick statistics line: `tick period ms min=8 avg=8 … n≈239` every 2 s.
- **Card connected, static page (enyo), same window:** 291 ticks (4.7%), log +2970 B.
- **about:blank:** 663 full paints in 30 s (measured the same morning, docs/PICKUP.md 2026-10-01).
- **Connect → first non-blank `painted`, static page, 3 runs:** 1.04 / 1.12 / 0.90 s
  (launch → `client connected`: 2.04 / 2.19 / 3.45 s). ~0.1–0.2 s polling granularity.

## Device facts found on the way (CE 3.1.0)

- **`/` is mounted read-write** (`/dev/mapper/store-root on / type ext3 (rw,…)`). On 3.0.5 it
  was read-only at rest. Not yet known whether CE ships it rw or an earlier session left it so;
  matters for device-build R8's "rootfs back to read-only" box (postinst keeps the state it found).
- `org.webosinternals.ipkgservice` is NOT at `/media/cryptofs/apps/usr/palm/services/` nor
  `/usr/palm/services/`; `org.webosinternals.preware` IS installed as an app. Locate the service
  before the fresh-install cycle (T-230/T-231).
- Other CE apps present: `org.webosarchive.atlas-default-browser`, `org.webosarchive.open-in-atlas`.
- Cards can be closed remotely: `applicationManager/running` → `processid`, then
  `applicationManager/close {"processId": …}`. Used for the connect-latency baseline.
