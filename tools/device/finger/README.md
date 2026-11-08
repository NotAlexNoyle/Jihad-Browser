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

## A probe inside a page: the component's on-demand probe, not `jsurl`

The inject channel's `jsurl javascript:…` runs a javascript: URL in the page: it appends elements, sets the title and
window variables (checked 2026-11-08 on the long page), but a page whose Content-Security-Policy forbids inline script
(YouTube) refuses it WITHOUT A WORD — `inject jsurl ok=1` and nothing ran, and a throw inside it is as silent. For such
a page set the compat component's probe instead: with `jihad.compat.trace` on, `setpref s jihad.compat.probe <base64>`
through the inject channel runs the script at once in every live top document (a sandbox with the page's window as
prototype; no CSP applies) and logs `probe [host] <what it returned>` (up to 6000 chars). The device has no `base64`:
encode on the host, and give each probe a different text (a comment at its end) — an unchanged pref fires nothing.

## stallwatch — the real-time witness of a stall (cavekit-device-headroom.md R1, T-591)

`stallwatch.c` runs as SCHED_FIFO 60 with its pages locked and writes a line a second to a tmpfs file: how late its
own wake-up was (a late witness means the kernel itself stalled), the load, each core's user/nice/system/idle/
iowait/irq/softirq jiffies over the second, swap-ins, major faults, allocation and compaction stalls, free memory,
the I2C error interrupt's count (this device's light sensor is dead: 240 of them a second), and for the pids named
their state, CPU and wait channel; every other second the tasks in D state with their wait channels. Built like
`finger.c` (`-o /tmp/stallwatch`, GLIBC 2.4 and 2.7 symbols only); `headroom-stress.sh` starts it when it is there.
Written after 2026-11-08's stress, when the device's whole user space got no CPU for three minutes (syslog, novacomd
at nice -4, the window server) while the kernel kept its timestamps, and nothing recorded why.

```bash
/tmp/stallwatch /tmp/stallwatch.log 600 $(pgrep novacomd) $(pgrep -f LunaSysMgr) <daemon pids> &
```

## fingerlib — the check library and the geometry (cavekit-finger-testing.md R1, R3; T-563)

`fingerlib.sh` is the device side of a finger test: busybox sh functions a test's host wrapper puts ahead of the test's
own body and runs as ONE script in one novacom session (`build/webos-oe/dev-sh.sh`) against the shell's hand daemon
(`daemon-start.sh <shell> <url> [PREF=name=value …]`: the shipped GPU path with the express lane plus the inject
channel and the `[jihad-gpu]` log on `daemon-gpu.log`; `daemon-stop.sh <shell> [log]` puts the upstart job back and
strips the run's prefs). Its header lists every function; the shape:

- **Oracles, never an eye.** `fl_rect`, `fl_text`, `fl_pref`, `fl_zoom`, `fl_url`, `fl_card_scroll` read the inject
  channel's answers and the daemon log; `fl_check_keyboard` reads the window server's syslog (`IME: Window got focus
  change in sysmgr <app id> focused: 1|0`); `fl_check_dialog` the daemon's `dialog …: card picked up` / `dialog … ->`
  lines; the frame checks (`fl_check_content`, `fl_sample` + `fl_check_region_equal` / `_changed`) read the shown page
  through `fbstream -Q X0,Y0,X1,Y1` (one shot: distinct colours, an order-sensitive hash, one-colour rows). Content is
  "8+ distinct colours AND the expected element's screen rect meeting the region (and its text when given)" — there is
  no brightness check in the library, so the darkness class cannot come back. A region meant to hold still ends at
  column 756: the card's scroll indicator stands in 757..767 and fades after a scroll.
- **Waits on marks.** `fl_mark` notes the log's size; `fl_wait PATTERN BOUND` polls five times a second for the line
  since the mark and fails at the bound; `fl_wait_pref`, `fl_wait_rect`, `fl_wait_settled` (the card's scroll read
  twice the same), `fl_wait_load`, `fl_wait_painted`, `fl_wait_zoom` are the named waits. No sleep stands for a check.
- **Geometry derived (R3).** A tap names an element: `fl_screen SEL` = its rect (document px: the viewport rect plus the
  engine's scroll) times the zoom, less the card's scroll, plus the shell's page top, less Mojo's 45 px header once the
  card is scrolled — re-read every time. `fl_calibrate SEL` taps a known element aimed with the shell's table constant
  (Enyo 82, Mochi 78, Mojo 73) and reads the daemon's `clickAt` back: the top measured; off the table by more than
  10 px the round STOPS (exit 3) with the reason. `fl_chrome NAME` is the per-shell table of the chrome (back, forward,
  home, address, reload, share, newcard, history; Mojo's bottom bar and its search; the system alert's Close).
  `fl_pick SEL INDEX [ROWS]` is the picker rule: the select brought mid-screen, the list waited for by a still frame
  region (`fl_wait_still`: the card's own animations have no log line, the shown frame is their oracle), then the
  row — under the select on Enyo and Mochi (the first row 60 px under the select's centre, 44 px a row), in Mojo's
  popup (x 243..523, rows of 51 px) centred on the visible card area wherever the select is — lower with the keyboard
  down, higher with it up — so measured where it is: `fbstream -Q` reports the span of light rows at the region's
  centre column (the white popup over the dimmed page) and the rows are counted from its top; the daemon never counts
  that popup open. `fl_bring` scrolls the card so an element sits mid-view before a tap, above the keyboard while it
  is up (`fl_kb`: the window server's last IME line).
- **Verdicts.** A check prints `CHECK ok NAME: evidence` or `CHECK FAIL NAME: evidence`; a failure keeps the shown
  page under `/media/internal/jihad-finger/NAME.last.raw` (the host wrapper pulls it), a pass keeps nothing;
  `fl_summary` prints `CHECKS n FAILS m` and exits 1 on a failure.

`fingerlib-check.sh [shell] [plant]` is the library's own round: the long page (the calibration, every check once, a
deliberate failure whose frame is pulled), then desktop Wikipedia at its fit zoom (a second calibration tap measures
the zoom in the geometry); `plant` puts the page-top constant 20 px wrong and the calibration must stop the round.
`prefs-finger-walk.sh [shell] [filter]` is the settings walk on the library, one pane a session, on all three shells.
The first rounds (2026-11-04, `docs/rounds/finger-2026-11-04/notes.txt` "T-563"): the library's round Enyo 15 of 16,
Mochi 14 of 16, Mojo 16 of 17 (the deliberate failure the one each keeps a frame for; Mochi's other a shell
finding, T-569), the plant stopped with "off by 20 px", the walks Enyo 62 of 62, Mochi 62 of 62, Mojo 62 of 62 (the five tabs and the 57 rows each, every pref changed and put back by the finger on its own control, no pref left changed; 13-15 minutes a shell).

## The suite: the script form and the runner (cavekit-finger-testing.md R4, R5; T-564)

`suite/*.test` are the tests, one declarative form (the runner's header has the grammar): header lines (`id:`,
`shells:`, `page:` the card's page, `prefs:` run prefs, `kit:` what it is evidence for, or `person:` for an item no
finger reaches), then one step a line — `launch URL`, `wait load|painted|zoom|settled|rect SEL|line PATTERN|still
REGION [BOUND]`, `calibrate SEL`, the gestures (`tap SEL`, `tap-at X Y`, `chrome NAME`, `drag`, `flick`, `pinch`,
`hold`, `dtap`, `pick SEL INDEX [ROWS]`, `card-to Y`, `pull-header`, `key CODE`, `type TEXT`, `inject CMD`),
`expect CHECK ARGS` (every fl_check of the library), `sample NAME REGION`, `shoot NAME` (a picture a kit note will
cite). No `sleep`: a gesture with no mark of its own (chrome, hold, dtap, pinch) needs a `wait` before the next
plain `expect`; a wait that times out is a FAILED check; a tap takes an element, never a coordinate. The lint
(`finger-run.py --lint`) says so; `suite/self/lint-bad.test.off` is its fixture (renamed `.test` it is refused with
the three findings).

`finger-run.py [--shells enyo,mochi,mojo] [--only id,id] [--self] [--round DIR]` is the runner: the lint, then the
owner's lock (`/tmp/jihad-owner-testing` on the device: the runner refuses and says so; `touch` it before testing by
hand, remove it after), the sampler built from source and pushed once, then for each shell each script in ONE device
session — `daemon-start.sh` on the script's page (the profile's prefs copied first), the steps compiled onto
`fingerlib.sh` and run by one `dev-sh.sh` call, the failed checks' frames pulled as JPEGs into `<round>/kept/`,
`daemon-stop.sh` after every script whatever the verdict (the hand daemon gone, prefs.js put back as found, the
dumps and scratch removed, a leftover device script killed, the job back) — and at the end `device-check.sh`: the
three jobs running, no daemon with the inject channel, no test hook, no adapter gate, no gate file or device script
or dump in `/tmp`, no scratch under `/media/internal`, the Default look (each daemon's `getTheme`), No proxy (no proxy
user pref). The report `<round>/finger-report.md` (+ `.json`) lists every run with its verdict (pass / fail / a
person's), its checks' lines and the kept frames, then the device check; the pictures kept are the failures' and
the `shoot`s only. `suite/self/` holds the suite's own scripts (a deliberate failure for the report's path), run
with `--self`.

Since T-565 the suite is every reachable item of the finger list. More of the form: `item:` and `section:` (the entry in
`docs/FINGER-TESTS.md`, which `finger-list.py --write` regenerates from the suite and its last report; the hand-run record
is `docs/FINGER-TESTS-2026-10.md`), `fixture: NAME` (`suite/fixtures/NAME.sh up|down SHELL` around the daemon: `pads`, twelve
sideloaded test extensions; `persona-file`, a Persona in the downloads folder; `auth-proxy`, the login proxy on the host),
`needs: internet` (run with `--with internet`), `tool:` and `skip:` with their reason, and `known: T-NNN WORD` (failures all
holding WORD are that open defect's: the verdict reads `known`, not `fail`). Marks: `wait gone SEL`, `url PART`, `pref T NAME
TEXT`, `differs REGION SAMPLE` (a menu that came up during a hold), `changed REGION`, `text SEL TEXT`. Gestures: `hold-on SEL`,
`dismiss X Y` (a tap that closes a menu reaches no page, so no clickAt is awaited), `tap-off SEL DX DY` (inside a frame the
rect cannot see), `drag-slow`, `drag-jitter`, `drag-paused`, `type-keys TEXT` and `key-hold kb-backspace` (the system keyboard
by finger; `fl_kb_key` is its map), `daemon-restart URL` (stopped as upstart stops it, for a persistence check), `stream-start`
and `stream-stop` with `expect stream stale|uniform|still|lag-max|error-max VALUE` read on the host, and `dwell MS` between a
check and the same check (does it still hold?). `pause MS` is the human's delay before a gesture only: the lint refuses it as a
guard before a check.

What the first rounds taught, not in the source: calibrate on an element whose tap does nothing (a tap on a dialog's button
raises the dialog and blocks the engine); a dialog logs nothing until it is answered, but its reply pipe stands in the state
directory while it is up; a page back from the back-forward cache sends `titleAndUrl` and no `load done`; a pinch's zoom settles
after several lines, and the frame at it comes later; about:addons opens on Get Add-ons; about:settings opens on Browser;
`gettext` returns the first 400 characters, so read a heading rather than `body`; a value starting with a digit is not a number
(`192.168.1.245` broke prefs.js); Enyo's login dialog centres in what the keyboard leaves, Mojo's sheet comes with the keyboard up.

**The first whole runs** (2026-11-05/06) taught these. A dialog's reply pipe stands before the card has drawn the dialog, and Enyo's popup
is as tall as its text: its buttons are counted from its last light row once two reads agree (`fl_dialog_last`). A daemon started on
a page whose script never ends answers nothing until its dialog's deadline has passed, so reach such a page from a free daemon
(`inject url`). A pinch by the finger takes the page about 0.56x (or 2.67x) of a 4x spread: go back to the fit zoom by several, and
a pinch at the fit zoom asks for nothing, so no zoom mark comes. The frames (fb1) never show the system keyboard: what stands behind
it shows instead, and a tap at the bottom row types. A card's `card=` runs past what is shown at the end of a page (about:addons:
338 against 265, a tap's clickAt agreeing with the frame), so at a page's end tap a point and read which element took it. A hold on
a field raises the keyboard; dismiss its popup above the keyboard. The card claims a hold on a link or an image: no `hold at` line
for those, the menu's frame is the mark. A page may drop an `h1`, a site its `input`: a selector on a real site is read, not assumed.
A page the profile has in its cache asks no proxy for anything: a proxy test goes to a path no round has loaded. Mojo's sheets slide
up (a tap too soon lands on the other button) and its login sheet stands on the keyboard or at the bottom: its buttons are counted
from its measured top. Its circles stand over the foot of the page and its bar moves the fixed header: `stream-mojo:` gives its
stream reading other rows. `--merge` runs some scripts again into the round's report, marked (rerun). A network that answers
`SSL_ERROR_RX_MALFORMED_SERVER_HELLO` or fails a name for a few minutes fails every real site's script: read the frame first.

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
   upper right (fb0, so the sampler never sees it; a finger aimed there hits it, and the owner sees it). Never restart
   bare: `bash build/webos-oe/dev-sh.sh -t 120 tools/device/finger/sysmgr-restart.sh` kills, waits, taps its Close and
   takes the screenshot in one script; the owner asked twice.

What it cannot do: the accelerometer (rotation), the volume keys, hearing audio, a physical keyboard. Those
stay a person's in `docs/FINGER-TESTS.md`.
