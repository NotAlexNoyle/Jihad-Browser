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
| T-221 | DONE | Codec table on device (jihad-codecs.html): VP8/VP9/Theora/VP8+Vorbis/Vorbis/Opus/WAV play to the end; H.264/MP3/AAC refused (no decoder). FOUND: FLAC advertised but failed — ffvpx libs (`libmozavcodec.so`, `libmozavutil.so`) are dlopen'd, were never bundled. Fixed in make-device-bundle.sh + push-engine-update.sh; libs pushed to all three (md5 `a4eaef65…`, `d65eaf82…`); FLAC plays, others unchanged. `anon video`/`anon audio` show `videocontrols[1]` (binding attached). Error-overlay screenshots archived in `evidence-2026-10-01/`. gre-widgets R1 codec box → `[x]`. |
| T-222 | DONE (agent part) | Play/pause both directions via inject `click` (adapter clickAt entry): pause t=11.370 held 3 s; resume → 17.65. Kit box stays `[~]` for the finger (T-285). |
| T-223 | DONE | `packaging/prefs/jihad-platform-prefs.js` media block rewritten to the measured state (comment-only + unchanged pin). gre-widgets R7 first box → `[x]`. Reaches the device with the next goanna.js push (T-225). |
| T-224 | DONE | R10 on media: stock tunables untouched (read back 95/200000 before and after). 640x360@24 VP8: 648–1188 MHz, 473/0 frames; 320x240: 384–1188 MHz, 0 dropped. Decision: do not extend the boost. device-build R10 box → `[x]`. |
| T-226 | MOOT | Conditional on T-224 finding drops at the clock floor; it found neither. Nothing to build. |
| T-203 | DONE | Idle tick: `Main.cpp` arms the frame tick while a card has a page and a 1 s tick otherwise; `JihadBrowserServer::setWakeHook` re-arms on connect; inject polled on every idle tick; no stats window across an idle stretch. |
| T-204 | DONE (code) | No tick-stats line with no page or < 20 ms work per 2 s; `log_cap_cb` every 30 s, copy-then-truncate at 2 MB (`JIHAD_LOG_MAX` test knob), one kept generation; numbers in the job comment + `packaging/README.md`. Device verification: T-227. |
| T-205 | DONE (code) | Blank settle: 500 ms after load done a persisting blank frame is published once; refused blanks retried on a 100 ms grid. Device verification: T-228. |
| T-206 | DONE (code) | Confirmed from source: postinst wrote the role then started the daemon with no hub rescan, and the daemon registered exactly once. Fixed both ends: `ls-control scan-services` in postinst before `start`; registration retry with backoff (5→60 s, ~11 min) in `JihadLunaService.cpp`. Real reinstall: T-231..T-234. |
| T-211 | DONE | `g_message("jihad: flashGestureLock -> …")` at the single transition point in the adapter. |
| T-213 | DONE | PUA swallow extended to 0xE0E2–0xE0E7; `JIHAD_LOG_KEYS` logs non-printing key triples only (printable keys are never logged). |
| T-215 | DONE | One log line per frozen settings command; `render/goanna/test/jihad-settings.html` (click-opened popup, script marker, cookie, min-font). |
| T-217 | DONE | ARM daemon built, YAP contract OK (73 commands, 58 messages). Stripped md5 `3e29f045cf0f9bb392ef96f8eb8a7139`. |
| T-218 | DONE | PDK + crosstool adapters and three `.ipk`s (enyo 1.0.4, mochi/mojo 1.0.0); each payload verified: libxul `65e3cc84`, daemon `3e29f045`, ffvpx libs, `libasound.so.2`, goanna.js `01154654`, postinst carries the hub rescan. |
| T-225 | DONE | Daemon push ×3 (md5 read back). Idle: 0.05% CPU, ~1 wakeup/s, 0 log bytes per variant; inject answers `no page`; connect latency unchanged; re-idle on card close, same PID. device-build R8 idle box → `[x]`. |
| T-207 | DONE | `build/webos-oe/r9-device-probes.sh` (build / mutex / closure); PDK-linked dynamic `mtx_dump_pdk` built (NEEDED libpthread.so.0, libc.so.6). Device run: T-245. |
| T-208 | DONE | The three toolchain recipe files are tracked. `UXP-Jihad` `jihad-engine-mods` head = `bd49435b` = parent gitlink = OE SRCREV (checked with `git ls-remote` over https). Today's engine fix `1fff8023` is local only → push + gitlink/SRCREV bump is T-279 (needs the user's credential; no push this session). |
| T-210 | DONE | `app-mochi/PARITY.md` refreshed: own-service URIs, `selectPopupMenuItem`, home control, stored shortcuts, settings entry, toast, corrected dialog note. Human sign-off stays T-291. |

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
- **The display was OFF and the device LOCKED** for the first hour of this session
  (`com.palm.display/control/status` → `"state":"off"`); `takeScreenShot` then returns the stale
  lock screen — three "different" screenshots with the same md5 were the tell. Woken with
  `display/control/setState {"state":"unlock"}` + `setProperty {"onWhenConnected":true}`.
  The engine runs page timers, decodes and paints with the display off (the `<video>` matrix was
  taken that way; a later run with the display on agrees), but ANY pixel evidence needs it on.
- Inject coordinate spaces, measured: `click x y n` and `move x y` take CARD pixels (CSS px ×
  page zoom) and go through the input queue; `clickoff dx dy n <id>` / `rect <id>` take a bare
  element id (or `sel:<css>`), CSS px, and bypass the queue — `clickoff` on the video play button
  returned `ok=1` and did not toggle it. `gettext` truncates long text (~330 chars).
- Cards can be closed remotely: `applicationManager/running` → `processid`, then
  `applicationManager/close {"processId": …}`. Used for the connect-latency baseline.
