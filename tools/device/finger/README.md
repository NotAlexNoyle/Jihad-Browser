# finger — the emulated finger for the TouchPad's finger rounds

`finger.c` sends the touch panel's own datagrams to LunaSysMgr, so a scripted tap, drag, flick, pinch, long
press or double tap travels the real path: hidd → LunaSysMgr (libhal's touchpanel module, the Qt QPA plugin) →
the card's WebKit → the BrowserAdapter plugin → the browser daemon → the page. Found and proven 2026-10-24
(`context/kits/cavekit-finger-emulation.md` has the path and the evidence).

## Build (the repository's toolchain) and install

```bash
TC=build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin
$TC/arm-webos-linux-gnueabi-gcc -O2 -march=armv7-a -mfpu=neon -mfloat-abi=softfp -std=gnu99 -o /tmp/finger tools/device/finger/finger.c
novacom put file:///tmp/finger < /tmp/finger          # then `chmod +x /tmp/finger` on the device
```

No binary is kept in the tree; `/tmp` on the device is wiped by a reboot.

## Commands (coordinates: the portrait screenshot's pixels, x 0..767, y 0..1023)

| | |
|---|---|
| `finger tap X Y [MS]` | a tap (down, 80 ms, up) |
| `finger dtap X Y [TAP_MS] [GAP_MS]` | a double tap |
| `finger hold X Y [MS]` | a long press (1500 ms): the context menu |
| `finger drag X0 Y0 X1 Y1 [MS]` | a drag at 100 scans a second (600 ms), lifted at rest |
| `finger flick X0 Y0 X1 Y1 [MS]` | a flick (160 ms), lifted while moving, with hidd's FLICK velocity: the scroller coasts |
| `finger pinch CX CY D0 D1 [MS]` | two fingers on the horizontal through CX,CY, from D0 to D1 apart |
| `finger raw TYPE CODE VALUE …` | one datagram of the given events (experiments); the EV_SYN is appended |

`FINGER_FRAME=raw` sends the coordinates in the panel's own frame (raw x = portrait y, raw y = 767 − portrait x);
`SCAN_HZ` changes the move rate; `FINGER_SOCKET` the socket path; `FINGER_TRACE=<file>` appends every scan sent
(`<ms> <finger> <x> <y> <phase>`, portrait coordinates, gettimeofday milliseconds, phase 1 down / 0 moving / −1 up)
after a `# <command> at <ms>` line — the finger's path beside the frames it caused (T-561).

## fbstream — the frame sampler (cavekit-finger-testing.md R2, T-561)

What the card SHOWED while the finger moved, read from the app framebuffer itself. The TouchPad draws the cards into
`/dev/fb1` (1024 × 768 × 32 bpp in the panel's landscape frame; three pages of 768 rows, the shown one named by the
pan offset, flipped at every frame LunaSysMgr presents — `fb0` is the system chrome and the keyboard). A portrait row
of the screen is a COLUMN of the framebuffer (portrait y = fb x, as the finger's frame), so the sampler reads every
STRIDE-th framebuffer row of the shown page and folds each pixel into its column's signature: per sample 1024
portrait-row signatures (an order-sensitive 32-bit hash down the sampled columns), each row's first pixel and whether
the row was one colour, the pan offset before and after the read (a read the card flipped under is taken again, twice
at most), the read's cost. Between samples it polls the pan offset (an ioctl, ~500 a second) and records every flip:
the time of every frame shown, to the millisecond, which 15 samples a second cannot give. Signatures only by default
(630 KB for 5 s at 15/s); the raw page (3 MB) on request. The daemon's timeline (`[jihad-tl] t=`), the finger's trace
and the sampler share one clock, gettimeofday milliseconds, so the host joins them.

```bash
$TC/arm-webos-linux-gnueabi-gcc -O2 -march=armv7-a -mfpu=neon -mfloat-abi=softfp -std=gnu99 -o /tmp/fbstream tools/device/finger/fbstream.c
novacom put file:///tmp/fbstream < /tmp/fbstream          # then chmod +x on the device; GLIBC_2.4 symbols only (the device's libc 2.8)
# on the device:
/tmp/fbstream /tmp/s.fbs -s 12 -f /tmp/s.fin &            # 12 s, 15 samples/s, stride 8; the finger joined from its trace at the end
FINGER_TRACE=/tmp/s.fin /tmp/finger drag 480 900 480 250 1000
wait
```

| option | |
|---|---|
| `-s SECONDS` | how long (default 12); SIGTERM / SIGINT ends it early with the file complete |
| `-z HZ` | samples a second (default 15) |
| `-k STRIDE` | every STRIDE-th framebuffer row read (default 8: 96 of 768). Measured on the device 2026-11-03 at 15/s: stride 8 reads in 2.7 ms and costs 5.6 % of a core, stride 4 13 ms and 12 %, stride 2 17 ms and 19 % at 20/s; `pread` (`-p`) twice mmap's cost |
| `-P US` | the pan poll's period (default 1000 µs; the device's nanosleep makes it ~2 ms) |
| `-R N` | every N-th sample's whole shown page to `OUT.rawNNNN` (3 MB each; `/tmp` is a 40 MB tmpfs) |
| `-L` | the last shown page whole to `OUT.last.raw` at the end |
| `-f TRACE` | the finger's trace: each sample gets the finger's last scan at or before it |
| `-c X0-X1` | the portrait columns the content signatures take (default all): the page's scrolling content, without a fixed sidebar (a blank content row under one has the sidebar's signature and never "moves") and without the card's scroll indicator at column 767 (the long page: `-c 0-756`; desktop Wikipedia: `-c 170-600`); the columns outside get a signature of their own (`sig2`), so a fixed part standing in columns is checked for holding still as a fixed header is in rows |
| `-U ROWS` | shoot on the event: when a run of ROWS or more one-colour rows appears where the previous sample had content, the whole shown page goes to `OUT.eventNNNN.raw` (three at most) — the frame a person would have had to see, kept without a person (`stream-pan.sh` pulls them to `fs-<name>.eventNNNN.raw`) |

The signatures are two per row: an exact 32-bit hash down the sampled columns and a profile (the sum of r+g+b). A
frame composed afresh at a zoom other than 1 (Wikipedia's fit zoom 0.6882) has every pixel a little different from
the last frame's — the bilinear phase — so the exact hash matches only the rows the band frames copied, and the
profile, which averages the difference away, matches the rest.

The file format is in the header of `fbstream.c`; `tools/host/finger-stream.py <s.fbs> [--trace s.fin] [--daemon
<daemon-gpu.log>] [--still a-b,c-d] [--rows lo-hi] [--tol N] [--dump] [--json out]` reads it: the sample rate and the
read's cost, the shown frames' spacing (from the flips), the shown scroll per sample (the shift by which the content
rows match the previous sample's: the exact signatures' vote, else the profiles' correlation), the tracking error
against the finger (rows and milliseconds, per gesture), the shown scroll going backwards under a finger that did not
(an older frame shown), stale bands (a run of content rows showing what was there before where the page's move says
other content belongs), uniform bands (one colour where the previous sample had content at that place, or where the
next sample shows the content that came late), whether the `--still` rows (the page's fixed header, the card's chrome)
held, the rows in which the columns outside `-c` changed, and with the daemon's log the delay from each published
frame to the card's flip and the frames the card did not present within 100 ms (a publish whose trailer says nothing
changed is left out). `tools/host/test-finger-stream.py <clean.fbs> [--trace] [--still] [--rows]` plants faults in a
clean stream (a duplicated band, the header moved 10 rows, a grey band) and checks each is reported where planted and
nowhere else (`docs/rounds/finger-2026-11-03/fs-long-p-head.fbs` with `--trace fs-long-p.fin --rows 139-1024 --still
29-139` is its input in the repository; the whole streams stay out of it). `stream-pan.sh <name> <url> [drags] ["<env>"] [1|0]
["<fbstream options>"]` runs the finger-pan round (`tools/device/t502/finger-pan.sh`) with the sampler beside the drags
(`stream-pan-device.sh`), the stream written under `/media/internal/jihad-stream` (never `/tmp`: a 40 MB tmpfs the
daemon's dumps share) and pulled to `$ROUND_DIR/fs-<name>.fbs/.fin` with the event shots; the 0 is the cost's control.
Its `fp-<name>-cpu.txt` holds the CPU ticks of the daemon and the LunaSysMgrs over the drags and, since 2026-11-04,
each daemon's memory before and after them (VmHWM, VmRSS, VmData, the status file's order): the CPU bands' kept tile locks double-buffer
the tiles on the content side, and the peak says what that costs.
The first round (2026-11-03, `docs/rounds/finger-2026-11-03/notes.txt`) read two defects of the shipped build no
round had seen: a grey band at the bottom of the screen under a drag (T-567) and a band under the long page's fixed
header a frame behind (T-568).

## A round

1. Open the card on the page with `luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"<app id>","params":{"target":"<url>"}}'`
   (the three app ids are in the workspace `CLAUDE.md`), wait for the load.
2. Gesture, wait, screenshot (`luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/x.png"}'`,
   `novacom get`, rotate −90° for portrait), judge by eye; for the GPU path read the daemon's timeline and frame
   dumps too (`tools/device/t502/README.md`).
3. The daemon's log is the oracle for what reached the engine (`/var/palm/jihad/<variant>/daemon.log`:
   `setZoomAndScroll` for a zoom, the scroll lines for a pan); LunaSysMgr's syslog (`/var/log/messages`) for what
   reached the window server (`editorFocused`, `IME:`, "Deadzone Tapped" for a touch in a dead zone,
   libhal's "Unexpected code" for a malformed datagram).
4. Delete the shots once judged (memory `clear-screenshots-when-done`); keep only what a kit note cites.
5. A `killall LunaSysMgr` with the USB cable in re-shows the system's "Connected — Tap USB Drive…" alert in the
   upper right (fb0, so the sampler never sees it; a finger aimed there hits it, and the owner sees it). After the
   restart's 45 s, `/tmp/finger tap 583 232` taps its Close (harmless on an empty card view); the owner asked.

What it cannot do: the accelerometer (rotation), the volume keys, hearing audio, a physical keyboard. Those
stay a person's in `docs/FINGER-TESTS.md`.
