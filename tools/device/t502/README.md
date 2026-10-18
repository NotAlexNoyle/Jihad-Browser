# T-502 device scripts (the compositor path's rounds, 2026-10-17)

Copied from the session's scratchpad so the next session has them. Paths inside point at the
session's scratchpad (`S=`) and at this repository; edit `S` or run them from a copy.

- `gpu-spike-start.sh <url> "<words>"` — on the device (through `build/webos-oe/dev-sh.sh`): closes the
  open browser cards, stops the upstart `jihad` job and any hand-started Enyo daemon, starts the
  daemon by hand with `JIHAD_GPU_COMPOSITOR=1` (plus the words: `IOF=<n>` sets the GPU io_fraction,
  `PREF=name=value` a user pref for the run, `GPREF=name=value` a default pref in goanna.js for the
  run, `NOGPU=1` the software path, anything else is daemon environment), launches the card, waits
  for three content frames before the screenshot, prints the compositor-path log lines.
- `spike-run.sh <name> <url> "<words>"` — the host side: runs the start script, fetches the frame
  dump and the frame sequence, lays the sequence out as a contact sheet (`check-seq.py`), checks
  the first frame (`check-frame.py`), saves the screenshot and the daemon log under
  `docs/rounds/t502-2026-10-17/spike-<name>-*`, prints the compose/read statistics.
- `gpu-spike-collect-stop.sh` — stops the hand-started daemon, restores the prefs a run changed.
- `spike-round34-build.sh` — the engine build chain: amends the temporary engine commit with
  `gfx` and `widget` changes, cross-builds libxul, makes the device bundle.
- `device-clean.sh`, `close-cards.sh`, `enyo-daemons.sh` — the device's daemons, stale page
  sockets and cards, when a run leaves them behind.
- `adapter-install.sh` — puts a freshly built Enyo adapter on the rootfs (the stock one is kept
  as `.orig`); `adapter-log-read5.sh` reads the adapter's blit-time probe log.
- `run-gpu-bench*.sh` — run `tools/device/gpu-bench.c` (built with the host cross compiler, see
  the file's header) and grep its sections.
- `shmdump.c` — reads a shared segment by key from a third process (the blank-card hunt).
