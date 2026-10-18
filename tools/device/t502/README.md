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
logged; `verify2` compares the compositor against itself), `JIHAD_GPU_VERIFY_DUMP=<n>` (the nth
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
