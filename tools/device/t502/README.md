# T-502 device scripts (the compositor path's rounds, 2026-10-17 and 2026-10-18)

Copied from the session's scratchpad so the next session has them. Paths inside point at the
session's scratchpad (`S=`) and at this repository; edit `S` or run them from a copy.

**novacom splits a quoted argument on spaces**: `dev-sh.sh script.sh "A=1 B=2"` reaches the device
as `$2=A=1 $3=B=2`. The start script reads every argument after the URL as a word; a device script
that reads only `$2` drops the rest silently (this cost a week of `IOF=100` and the dump env never
arriving).

- `gpu-spike-start.sh <url> <words...>` — on the device (through `build/webos-oe/dev-sh.sh`): closes
  the open browser cards, stops the upstart `jihad` job and any hand-started Enyo daemon, starts
  the daemon by hand with `JIHAD_GPU_COMPOSITOR=1` plus the words (`IOF=<n>` sets the GPU
  io_fraction, `PREF=name=value` a user pref for the run, `GPREF=name=value` a default pref in
  goanna.js for the run, `NOGPU=1` the software path, anything else is daemon environment),
  launches the card, waits for three content frames before the screenshot, prints the
  compositor-path log lines.
- `spike-run.sh <name> <url> "<words>"` — the host side for a static or animated page: runs the start
  script, fetches the frame dump and the frame sequence, lays the sequence out as a contact sheet
  (`check-seq.py`), checks the first frame (`check-frame.py`), saves the screenshot and the daemon
  log under `docs/rounds/t502-2026-10-17/spike-<name>-*`, prints the compose/read statistics.
- `pan-run.sh <name> <url> "<words>"` — a scripted finger pan (the inject channel's `pan 0 3000
  10000`: 0 → 3000 card px over 10 s) with `JIHAD_GPU_DUMP_EVERY=60`: fetches frames 60, 120 and
  180 as `docs/rounds/t502-2026-10-17/pan-<name>-frame-<n>.jpg`. **A screenshot cannot verify a
  scripted pan** (the adapter's view does not move); the dumps can, and so can the daemon's verify
  mode. `pan-three.sh` runs three configurations in a row (whole reads, two bands on the full
  target, the band target) for a bisect.
- `gpu-spike-collect-stop.sh` — stops the hand-started daemon, restores the prefs a run changed.
- `final-build.sh` — the build chain after the engine fold: cross-builds libxul from the engine
  working tree, builds the daemon, makes the device bundle (the bundle id is its last line). Push
  with `build/webos-oe/push-engine-update.sh enyo`. `spike-round34-build.sh` is the OLD chain that
  amended a temporary engine commit; do not use it after the fold.
- `device-clean.sh`, `close-cards.sh`, `enyo-daemons.sh` — the device's daemons, stale page
  sockets and cards, when a run leaves them behind.
- `adapter-install.sh` — puts a freshly built Enyo adapter on the rootfs (the stock one is kept
  as `.orig`); `adapter-log-read5.sh` reads the adapter's blit-time probe log.
- `run-gpu-bench*.sh` — run `tools/device/gpu-bench.c` (built with the host cross compiler, see
  the file's header) and grep its sections; `run-gpu-bench9.sh` the band-sized-target section,
  `run-gpu-bench9b.sh` every readback line.
- `shmdump.c` — reads a shared segment by key from a third process (the blank-card hunt).

Daemon environment words the rounds use (all off by default): `JIHAD_GPU_DUMP_EVERY=<n>` (every
nth published frame, three at most, raw BGRA at `<JIHAD_GPU_DUMP>-<frame>`), `JIHAD_GPU_VERIFY=1`
(after a partial job the same frame composed whole at the job's scroll, the differing row runs
logged; `verify2` compares the compositor against itself; NB an animating layer's rows differ under it, since the
reference samples the CSS animation at its own compose time — T-519, `jihad-anim-css.html`: the animated
block's 160 rows in 117 of 119 frames, every other row equal; and the frame rates measured with it on are not
the path's, every job then composes and reads a whole reference too), `JIHAD_GPU_VERIFY_DUMP=<n>` (the nth
verify pair written raw), `JIHAD_GPU_WHOLE=1` (every frame read whole), `JIHAD_GPU_NO_BAND_TARGET=1`
(bands read from the frame-sized target), `JIHAD_GPU_SURFACES=1` (the intermediate surface for a
scaled container back on), `JIHAD_GPU_ENGINE_SCROLL=1` (the per-step engine scroll instead of the
compositor's), `JIHAD_GPU_NO_DP=1` (no displayport), `JIHAD_GPU_READ_THREAD=1`, `JIHAD_GPU_PASS=1`,
`JIHAD_GPU_FRESH_TEX=1`, `JIHAD_GPU_NO_FENCE=1` (the opt-in paths of 2026-10-17).

Added in rounds 99-112: `JIHAD_GPU_CLEAR_MAGENTA=1` (frames cleared magenta: rows no draw reached
show as such), `JIHAD_GPU_DRAWSTATS=1` (per composite: the window draws, raw vs cached scissor and
viewport, a W/C/M read sequence), `JIHAD_GPU_WHITE=1` (the daemon's white row runs per job),
`JIHAD_GPU_READ_VERIFY=1`, `JIHAD_GPU_READ_SCRATCH=1`, `JIHAD_GPU_READ_CHUNK=<rows>`,
`JIHAD_GPU_DUMP_TARGET=<path>` (readback A/Bs), `JIHAD_GPU_READ_FINISH=1` (a glFinish before a
read), `JIHAD_GPU_KEEP_PB=1` / `JIHAD_GPU_PB_TARGETS=1` (the old pbuffer / target sizes, A/B),
`JIHAD_GPU_ONE_TARGET=1`, `JIHAD_GPU_BAND_SURFACE=0` (bands stacked in the frame target instead of
their own surface), `JIHAD_GPU_NO_DONE_FD=1` (no wake pipe).

Rounds 113-117: `JIHAD_GPU_NO_LATEST=1` (no copy from the newest slot), `JIHAD_GPU_DIFFLOG=1` (the
layers whose transforms moved, per composite), `JIHAD_GPU_ONE_DEEP=1` (no second job while one is
composed). `pan-run.sh <name> <url> "<words>" none` measures a page for 12 s without a pan.

Round 119: `JIHAD_GPU_NO_DIFF_FIRST=1` (a tree change composes on the frame surface, as before).

Round 125: `JIHAD_GPU_ANCHOR=<fraction of a height>` (how far the card's scroll goes from the engine's
anchor before the engine is moved; 0.3 by default).

Rounds 126-128: `JIHAD_GPU_DP_Y=<heights>` (0.5), `JIHAD_GPU_DP_X=<widths>` (0, half a width when zoomed in)
set the displayport margins; `PAN_DELAY=<s>` before `pan-run.sh` waits for a real page to load before the pan.

Rounds 138-150 (T-503): `JIHAD_GPU_NO_FIXED_COLS=1` (the daemon asks for the fixed rows whole, as
before), `JIHAD_GPU_NO_KEEP=1` (no opaque fixed layer kept), `JIHAD_GPU_NO_CULL=1` (the old draw
culling), `JIHAD_GPU_BAND_ROWS=<64..942>` (the band surface's rows, 256), `JIHAD_GPU_PHASES=1` (a
composite's phases every 20: shadow properties, async transforms, post-process, diff, begin frame,
prepare, render layers with draws, culled draws and uploads, end frame), `JIHAD_GPU_VERIFY_DUMP_WRONG=1`
(with `JIHAD_GPU_VERIFY=1`: the first frame with wrong rows and the first structurally wrong one
written to `/tmp/vw-*.raw` and `/tmp/vs-*.raw` with their whole composites). Verify's line reports
rows that differ at all, rows **wrong** (more than 4 pixels over 64 levels off) and rows
**structurally wrong** (16-row blocks better aligned at another row offset or far off at every
offset): only the last is an error; the first two include a fractional scroll's resampling.
`run-gpu-bench10.sh` runs the bench's column-read section. `render/goanna/test/jihad-sticky-mix.html`:
an opaque fixed header and footer, a transparent sticky sidebar over a striped page.

T-504: `zoom-run.sh <name> <url> "<env>" "<zoom in>" "<zoom out>"` injects the card's zoom command
(`zoom <z> <x> <y>`, setZoomAndScroll) twice with screenshots 1 s and 5 s after each and prints the
timeline. A pinch through the card's own gesture calls: `card-selftest.sh enyo push`, the spike
start, then `card-selftest.sh enyo run "page:w10;g2.46,384,470;g0.41,384,470;w3" <url> 30 shot.jpg`.
`JIHAD_GPU_NO_ZOOM_HOLD=1` restores the old zoom order; `JIHAD_GPU_ZOOMLOG=1` logs every tick of a hold.

T-504, rounds 162-177: `JIHAD_GPU_ZOOM_DUMP=<path>` (every frame at the new zoom published in the 3 s
after zoom n rewritten to `<path>.<n>`, n up to 6: the file ends as the frame the card shows at
rest; pull and look at them with `check-frame.py <raw> 768 942 out.jpg`, then delete them: /tmp
holds 40 MB and stale dumps filled it once), `JIHAD_GPU_TEX_POOL=<n>` (the engine's pool of tile
textures, 48; 0 deletes them as before), `JIHAD_GPU_NO_TEX_CONVERT=1` (new tile textures not
converted at 1x1: the Adreno's ~18 ms first write after a draw comes back),
`JIHAD_GPU_NO_RES=1` (the layers painted at 1x and magnified, as before the zoom's resolution).
The card's self-test hook (`render/goanna/test/enyo-selftest.js`) scripts a double tap with
`t<x>,<y>` (the adapter's smartZoom) beside the pinch `g<scale>,<x>,<y>`.
`tools/device/gpu-bench.c` sections 11-15 (`BENCH_ONLY=<n>`): back-to-back tile uploads, which
writes wait, texture setups, the conversion kept across a re-specification, a clear-based conversion.
Rounds 178-186: `JIHAD_GPU_DEAA=1` (Gecko's edge antialiasing for scaled layers back: its shader
variants compile on first use, ~150 ms each); with `JIHAD_GPU_PHASES=1` a composite of 80 ms or more
logs its own phases and a draw of 5 ms or more logs its effect, texture and size (bounded).
`gpu-bench.c` section 16: a texture filter switch after draws (no cost).

T-505, rounds 187-201: `JIHAD_GPU_OVERSCAN=<fraction of the card's height>` (0.15: rows composed above
and below the view once the page has been panned, so the card has content to pan into; 0 for a
view-sized frame; frames are capped at 1400 rows, the compositor's first pbuffer). `pan-run.sh`
sizes the frame dumps by their bytes (frames are taller than the card with overscan); delete
`tools/device/t502/pan-frame-*.bgra` and the device's `/tmp/frame.raw*` after a run.

T-506, rounds 202-203: with `JIHAD_INJECT=1`, an injected click, key, text or jsurl marks the time
and the daemon logs `input latency: the first changed frame ... out N ms after the input` for the
first GPU frame with changed rows published after it.

T-507, rounds 204-212: the Enyo test hook's `<` (back), `>` (forward), `R` (reload) and
`O<up|left|right|down|free>` (hold the card in an orientation: the system rotates the window); the
inject channel's `thumb <path>` (a thumbnail of what the card shows, written at the next frame);
`render/goanna/test/jihad-card-events.html` (a select, a field, links, a word to find).

T-509, rounds 229-232: `JIHAD_GPU_JOB_BOUND_MS=<ms>` (3000: a frame not composed in that time gives
the GPU path up for the daemon's life); the engine's test switches `JIHAD_GPU_TEST_NO_EGL=1` and
`JIHAD_GPU_TEST_STALL=<n>`; `three-daemons.sh gpu|soft` starts the three variants' daemons by hand
on one path, opens a page in each and reports memory, idle CPU and LunaSysMgr's state;
`three-daemons-restore.sh` puts the upstart jobs back.

T-515, rounds 240-267 (the express lane: frames made off the main thread): `JIHAD_GPU_EXPRESS=1` with
the GPU path (each card's YAP commands read on a thread of the proxy's own, the frame lock, an
express thread that composes and publishes frames while the main thread is in the page's script or
a layer paint; `render/goanna/JihadFrameLock.h` says how), `JIHAD_GPU_EXPRESS_MS=<ms>` (45: how long
the card waits before the express lane fills a stall), `JIHAD_GPU_EXPRESS_AWAY_MS=<ms>` (15: how
long the main thread has been in the engine first), `JIHAD_GPU_EXPRESS_LEAD_MS=<ms>` (120: while the
card's last scroll or zoom is that recent the express lane makes every frame and the main lane paints
layers only; 0 turns the lead off), `JIHAD_GPU_EXPRESS_EARLY=1` (the second frame posted right behind
the first: no faster on Wikipedia, older frames), `JIHAD_GPU_TIMELINE=1` (`[jihad-tl] t=<ms>` per
published frame of either lane and per long pump or layer paint, one clock),
`JIHAD_GPU_ZOOM_DUMP=log` (the zoom timeline without the 3-4 MB frame files; with the express lane
each main frame also says how long after the zoom ARRIVED). With the express lane the inject
channel's `pan` and `zoom <z> <x> <y> [delay ms]` go through the card's own path (the proxy's reader
side), on a thread, so they arrive while the main thread is busy as a card's do.
`zoom-seq.sh <name> <url> "<env>" [n]` sends n zooms that way at five points of a page script's
hold and prints each first frame from the zoom's arrival; `render/goanna/test/jihad-script-hold.html`
holds the main thread 600 ms in every 700 (`?hold=<ms>&every=<ms>`; `?hold=0` off). Express off
(`JIHAD_GPU_EXPRESS_MS=999999 JIHAD_GPU_EXPRESS_LEAD_MS=0` with it on) is the same timing path
without its frames: the baseline.

T-516, rounds 270-280: `JIHAD_GPU_TIMELINE=1` also logs every compose job (queue, compose), every
compose's breakdown and every read (engine 0050), the clock hold's transitions and the CPU clocks of
each express frame. `JIHAD_CLOCK_HOLD_MS=<ms>` (2000: idle time before the CPU floor goes back; the
YAP reader thread raises it on a card's scroll, zoom or smart-zoom request), `JIHAD_GPU_FRAME_NICE=<n>`
(-6: the Compositor and JihadExpress threads ahead of the engine's others; 0 leaves them). Double taps
through the card: `card-selftest.sh enyo run "page:w15;t300,400;w2;..."` with `JIHAD_GPU_ZOOM_DUMP=log`,
each tap's first frame in the `zoom frame written (express)` lines.

T-510: the upstart jobs start the GPU path with the express lane (`JIHAD_GPU_COMPOSITOR=1
JIHAD_GPU_EXPRESS=1`); `JIHAD_GPU_COMPOSITOR=0` or `user_pref("jihad.gpu.enabled", false)` in the
variant's profile give the software path (`tools/device/set-prefs.sh <variant> 'jihad.gpu.enabled'`
with a file holding the line; the same without a file clears it). `default-check.sh <enyo|mochi|mojo>`
runs a load, pan, pinch and close through the card's self-test hook on the variant's own daemon and
counts its frames, zooms, faults and fallbacks.

T-511, rounds 281-313 (WebGL): on the GPU path WebGL contexts are made on the device's GPU and share
textures with the compositor (engine 0051). `JIHAD_GPU_NO_SHARE=1` keeps them unshared (each frame read
back through a canvas-sized pbuffer, ~2/3 the rate); `JIHAD_GPU_NO_PB_READ=1` reads unshared frames
from the texture framebuffer as stock does (~1/2 again). `JIHAD_GPU_NO_COLUMN_READ=1` reads changed
bands as whole rows (engine 0052: one column when the change is narrow). The test page
`render/goanna/test/jihad-webgl-cube.html` (served by the harness at :8077): `?size=<px>`, `?checks=0`,
`?lose=1` (lose and restore the context), `?report=ext` (the missing extensions in the title),
`?stop=<s>` (still frames after it: `JIHAD_GPU_VERIFY=1` must find every slot right), `?ticker=1` (a
counter beside the canvas on its rows). Its rate and checks are in the daemon log's "title changed"
lines; the card's rate in the paint pipeline lines. `adapter-install.sh` no longer switches the
adapter's paint log on: its gate (`<state>/adapterlog`) has no cap and filled /var once (see the
script).

T-517 (the compositor job's floor): `pan-run.sh` takes `PAN_TOP=1` (the processes' and the daemon's
threads' CPU over 4 s of the pan). Frame windows (`render/browserserver/Src/JihadFrameWindow.h`): a frame
may start further into its segment, written in a trailer the window-aware adapter reads;
`JIHAD_GPU_NO_WINDOWS=1` keeps every frame at the segment's start (and the 3.7 MB row move of a scroll),
`JIHAD_GPU_EXPRESS_NO_OWN=1` keeps the express lane copying from the newest slot. `JIHAD_GPU_CACHE_CLEAN=1`
cleans the frame rows' cache after each frame although the segments are mapped write-through. NB: the
adapters must be the window-aware build (`build-adapter-pdk.sh`, then each variant's
`/usr/lib/jihad/<v>/BrowserAdapterImpl.so` and a Luna restart); an older adapter makes the daemon keep
windows at 0. Engine 0055 composes a band frame's bands and column chunks as regions of one composite
(`JIHAD_GPU_PASS_PER_REGION=1`: a composite each, as before); 0056 skips a tile whose bounds miss the clip
before its texture is locked (`JIHAD_GPU_NO_TILE_CULL=1` for A/B). With `JIHAD_GPU_TIMELINE=1` a page's
layer transaction that holds the compositor thread 3 ms or more is on the timeline with its tile uploads
(rounds 349-357), and `JIHAD_GPU_READ_FINISH=1` puts a glFinish before each read, so the read line splits
the GPU's remaining compose from the pixels' read. `gpu-bench.c` section 18 (`BENCH_ONLY=18`) times
sub-image uploads into converted tile textures.

T-512: the `[jihad-gpu]` diagnostics are off by default; `JIHAD_GPU_LOG=1` turns them on, and so do
`JIHAD_GPU_TIMELINE`, `JIHAD_GPU_VERIFY` and `JIHAD_GPU_DUMP_EVERY` (a device round's own switches). The
gate is one macro in the engine (`gfx/2d/JihadGpuLog.h`, exported as `mozilla/gfx/JihadGpuLog.h`) and one in
the daemon (`render/goanna/JihadGpuLog.h`); failures and fallbacks print anyway. `gpu-spike-start.sh` sets
`JIHAD_GPU_LOG=1` for a hand-started daemon (`NOLOG=1` leaves it off); the upstart jobs do not, and a page's
close prints one line with its express-lane frame count (`page closed: <n> frames by the express lane`),
which `default-check.sh` counts.

T-513 (R2's fast flick and two cards): `JIHAD_GPU_VERIFY=1` composes its reference right behind each partial
job, on the same layer tree, and compares at the job's publish (a reference composed later met a tree a fast
flick had moved on from and called right frames wrong). `two-cards.sh <url2> [pan]`, after
`gpu-spike-start.sh <url1> "JIHAD_INJECT=1 JIHAD_GPU_VERIFY=1"`: a fast flick on card 1, the app relaunched
on url2 (a second card whose Enyo test hook brings card 1 back after 28 s with its `A` step: push the hook
first, `card-selftest.sh enyo push`), a flick on card 2, then on card 1 again through the inject channel's
`target focus` (commands to the card that last said it has the focus; `target last` goes back to the newest
connection). The spike start sets `jihad.viewport.followOverflow` false; `PREF=jihad.viewport.followOverflow=true`
runs with the shipped rule.


T-527 (the zoom's sharp frame, `docs/rounds/t527-2026-10-25/notes.txt`): a zoom that changes the resolution
paints what the card shows first and the margins once a frame of that paint is out (2 s at most);
`JIHAD_GPU_NO_CRITICAL=1` paints the margins at once, `JIHAD_GPU_CRIT_MARGIN=<CSS px>` gives the first paint a
margin all round. The express lane publishes a main-lane frame composed while the main thread is away in the
engine for `JIHAD_GPU_EXPRESS_TAKEOVER_MS` (40; 0 turns it off), and makes no frame that would draw the painted
tiles at under half their size (a double tap out from 2.33). `JIHAD_GPU_ZOOM_DUMP_SEQ=1` with
`JIHAD_GPU_ZOOM_DUMP=<path>` keeps each lane's first three frames at a zoom (`<path>.<n>.<k>`, `<path>.x<n>.<k>`;
3.7 MB each: delete them at once). `finger-zoom.sh <name> <url> "<env>"` runs the emulated finger's double tap in
and out and a pinch in and out on the spike daemon (`finger-zoom-device.sh`; `/tmp/finger` on the device) and
keeps the log in `ROUND_DIR`. Engine 0062: the root's displayport base the daemon sets survives the paint.

Older device-round switches still in the code (T-502), for completeness: `JIHAD_GPU_DUMP2=<path>` (the 5th
frame's buffer as the adapter reads it), `JIHAD_GPU_DUMP_SEQ=<path>` (the first 12 content frames, every
4th pixel, appended to one file, with their content extents in the log), `JIHAD_GPU_FORCE_FPS=1` (a frame
asked for every tick, the spike's way before the compositor's own signal), `JIHAD_GPU_NO_WORLD=1` and
`JIHAD_GPU_NO_WORLD_ENGINE=1` (the card's zoom not applied as the compositor's world transform, daemon side
and engine side: bisection only), `JIHAD_GPU_OLD_SCROLL=1` (the card's scroll as the engine's scroll, not
the compositor's async scroll), `JIHAD_GPU_SNAPSHOT=1` (the round-5 snapshot road instead of the
compositor's frame), `JIHAD_GPU_TWO_DEEP_STILL=1` (the second job in flight only while nothing scrolls,
round 116's rule).
