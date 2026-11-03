---
created: "2026-11-03"
last_edited: "2026-11-03"
---

# Autonomous finger testing — the finger round that judges itself

**Why this kit exists.** The emulated finger (`cavekit-finger-emulation.md`) put the round in the agent's hands:
every gesture travels the real path. What judges the result is still an eye on a still: a screenshot two seconds
apart, a frame dump at a fixed count, a log line found by grep, coordinates worked out by hand per shell and
re-learned each round (`finger-walk-geometry`), waits that are sleeps. Each of these has cost a finding: black
frames called fine for seven rounds (`verify-device-frames-by-eye`), a pan "verified" by a still that showed one
frame (`verify-pans-by-dumps`), a redraw bug that was a screenshot taken before the step (`device-test-limits`),
dialogs that every harness answered faster than a hand (`jihad-harness-passes-human-fails`), a 45 px Mojo tap
that was the header scrolling away. The owner, 2026-11-03: "devise a system for accurate autonomous finger
testing". Accurate means: the verdict a person at the device would give, reached from evidence a machine read,
every time, with nothing a sleep or a glance decides.

**The instruments it stands on (measured 2026-11-03):** the TouchPad's app content is drawn into `/dev/fb1`
(1024 × 2304 × 32 bpp: three 768-row pages, the shown one by the framebuffer's pan offset; `fb0` is the system
chrome and the keyboard); the whole device reads in 0.15 s and one page in ~0.05 s, so a sampler can take the
shown page 10-20 times a second where `takeScreenShot` takes one still in ~2 s; a per-row signature (a 32-bit sum
or CRC a row) is a few KB a sample and a few ms on the ARM, a full hash of a frame is not (md5 0.28 s). The
daemon's inject channel answers `rect`, `gettext`, `cardscroll` and the daemon log carries `clickAt x= y=`, the
scroll lines, `setZoomAndScroll`, `painted ... bytes>0`, the dialog lines; LunaSysMgr's syslog carries the IME and
the deadzone lines. The emulated finger sends scans at 100 Hz with hidd's velocities.

## R1: Verdicts from oracles, never from an eye alone
**Description:** every finger test asserts on evidence a machine reads: the engine's state through the daemon's
log and the inject channel (the scroll, the zoom, the element under a point, a field's text, the loaded URL, a
dialog's state), the shown pixels through the frame stream of R2, and the window server's log for the touch's
arrival. A picture is kept only when a check fails, or when a kit note cites it; an eye on a picture confirms a
failure, it never passes a test.
**Acceptance Criteria:**
- [ ] A check library offers at least: the scroll (engine and card) within a tolerance; the zoom; the element at a point (`rect`); a field's text; the URL; a dialog present or answered; the keyboard up or down; content present in a frame region (at least 8 distinct colours and the expected text or element, never a darkness threshold); a region equal across samples (a fixed part held still); a region changed since a sample
- [ ] A failed check's report names the oracle line or the sample it read and keeps the frame it judged; a passed check keeps nothing
- [ ] The darkness-heuristic class cannot recur: no check in the library accepts a frame as content by brightness alone
**Dependencies:** cavekit-finger-emulation.md R1, R2

## R2: The frame stream — what the card showed, sampled while the finger moved
**Description:** a device-side sampler reads the shown page of the app framebuffer at ten samples a second or
more for the length of a gesture, records each sample's time, the finger's position at that time and the rows'
signatures (and the raw page when asked), and leaves the device as it found it. A host tool turns a stream into
the shown scroll per sample (the signature shift between samples), the shown-frame spacing (distinct from the
daemon's published rate), the tracking error against the finger in pixels and milliseconds, repeated bands or
stale rows (a signature run that reappears where the shift says it should not), uniform bands inside the page
(blank or grey where content belongs), and whether the fixed and stuck parts' rows held still.
**Acceptance Criteria:**
- [ ] The sampler takes the shown page (the pan offset honoured) at ≥ 10 samples/s for 10 s on the device with the finger driving a drag, writing signatures only; its cost is measured and bounded: the daemon's frame rate on desktop Wikipedia with and without it differs by under 5 %
- [ ] The host tool reports, for a scripted drag on `jihad-long-static.html` and on desktop Wikipedia: the shown scroll per sample, the shown-frame spacing (median, p90), the tracking error (median, max), stale or repeated bands, uniform bands inside the page, the header's rows held still — and its reading of a known-good round is 0 stale, 0 uniform, the header still
- [ ] A deliberately wrong frame (a dump with a band duplicated, a header moved 10 rows, a grey band painted in) is reported as such by the tool: the detectors are tested against planted faults, not only clean runs
- [ ] The daemon's verify mode and the stream agree on a clean pan (0 wrong rows there, 0 stale bands here) and the stream catches what the verify cannot see: the card's own redraw (a frame published right and shown wrong, or shown late)
**Dependencies:** R1

## R3: Geometry derived, calibrated, never guessed
**Description:** a test names page elements and chrome controls, not pixels. A page point's screen position comes
from the inject channel's `rect` of the element, the card's scroll and the shell's calibrated page top; chrome
controls come from a table per shell. Each round starts with a calibration tap per shell (a tap on a known element,
the `clickAt x= y=` line read back) and re-reads the geometry after each scroll or zoom; Mojo's header that scrolls
away, pickers that open under their select, pages wider than the card are rules of the library, not of the test.
**Acceptance Criteria:**
- [ ] No test script holds a screen coordinate for a page element; chrome coordinates live in one table per shell with the calibration that checks them
- [ ] A planted 20 px error in the page-top constant is caught by the calibration tap before any test runs, and the round stops with the reason
- [ ] The existing walks (`tools/device/finger/prefs-finger-walk.sh`, the Mochi and Mojo scratch walks) run through the library with their hand-worked numbers gone, on all three shells
**Dependencies:** R1

## R4: Waits on marks, and the slow human
**Description:** every step waits for the oracle's mark with a bound — the load done, `painted ... bytes>0`, the
keyboard's IME line, the dialog's line, the scroll settled (two equal samples), the zoom's `setZoomAndScroll` —
never a fixed sleep as the only guard. The suite includes the slow human: a dialog answered after the pickup
deadline a harness never reaches, a card closed and the job stopped the way upstart does before a persistence check,
a drag that pauses mid-way, a tap held longer than a tap and shorter than a hold.
**Acceptance Criteria:**
- [ ] No bare `sleep N` stands between a gesture and its check in the suite's scripts (a lint over the suite says so); every wait names its mark and its bound, and a wait that times out is a failed check, not a skipped one
- [ ] The dialog test answers at 8 s and passes; the persistence tests stop the job with `stop <job>` and check after `start`
- [ ] The pan test runs at three speeds (a slow drag of 2 px a scan, the 600 ms drag, the 160 ms flick) and reports each; a jittered path (±2 px) and a paused drag are gestures of the library
**Dependencies:** R1, R2

## R5: One language, one runner, the device left as found
**Description:** a test is a short script in one declarative form (launch, wait-for, gesture, expect, shoot-on-fail)
that the runner executes on the host; a script runs in one device session (one novacom session at a time), the
suite runs the scripts of a shell one after another, refuses while the owner is testing (a lock file on the device
the owner touches), restores what a script changed (prefs, the Persona, the proxy, the jobs, the dumps in `/tmp`)
whether it passed or failed, and writes one report: each test's verdict with the oracle lines that decided it, the
frames of the failures, the round's numbers.
**Acceptance Criteria:**
- [ ] The whole suite runs unattended on the three shells from one command and ends with the device as it began (the jobs running on their upstart daemons, no hand daemon, no gate file, Default look, No proxy, `/tmp` clear), checked by the runner itself at the end
- [ ] The runner refuses to start while `/tmp/jihad-owner-testing` exists on the device and says so
- [ ] A report under `docs/rounds/finger-<date>/` lists every test with pass / fail / a person's and the evidence line; the pictures kept are the failures' and the cited ones only
**Dependencies:** R1-R4

## R6: Coverage — the finger list as scripts, the kits citing them
**Description:** every item of `docs/FINGER-TESTS.md` an emulated finger can reach is a script with an id in the
suite, in each shell it names; a kit criterion the finger round covers cites the script id instead of a date and a
shot; what stays a person's (hearing, a physical keyboard, the accelerometer, the feel of smoothness beyond the
stream's numbers) is listed with the reason.
**Acceptance Criteria:**
- [ ] Every reachable item of `docs/FINGER-TESTS.md` has a script; the list is regenerated from the suite's last report, with the person's items marked
- [ ] The kit criteria that cite finger evidence cite a script id; `cavekit-gpu-rendering.md` R12 cites the stream's numbers for the pans, flicks, pinches and double taps
**Dependencies:** R5

## R7: Accuracy proven against a person
**Description:** the suite is accurate when its verdicts match the owner's at the device. Once the suite stands, the
owner runs the finger list on the same build and the two verdicts are compared item by item; a verdict the owner
contradicts is a defect of the suite — its check, its geometry or its instrument — fixed and re-run; what the suite
cannot judge is recorded, not glossed.
**Acceptance Criteria:**
- [ ] One comparison round recorded: the suite's report beside the owner's verdicts, every disagreement dispositioned (the suite fixed, or the item moved to a person's with the reason)
- [ ] The owner's "finds it smooth" (gpu-rendering R12) is answered by the stream's numbers (the tracking error, the shown-frame spacing) and the owner's own word on the same drags, and the two agree or the gap is named
**Dependencies:** R6 (BLOCKED-EXTERNAL: the owner at the device)

## Non-goals
- A camera at the screen, the accelerometer, the volume keys, audio heard, a physical keyboard: a person's.
- Replacing the daemon's verify mode or the frame dumps: the stream watches the card's side; the verify mode
  watches the compositor's. Both stay.
- A pixel-exact reference of the shown frame: the stream judges structure (shift, stale, uniform, held still) and
  content presence, not every pixel.

## References
- `context/kits/cavekit-finger-emulation.md` — the finger on the real path (R1, R2 met)
- `docs/FINGER-TESTS.md` — the list this kit turns into scripts
- `tools/device/finger/README.md`, `tools/device/t502/README.md` — the tools and the daemon's switches
- memories: `verify-device-frames-by-eye`, `verify-pans-by-dumps`, `device-test-limits`, `finger-walk-geometry`,
  `jihad-harness-passes-human-fails`, `one-novacom-session-at-a-time`, `owner-tests-clash-with-pan-runs`,
  `clear-screenshots-when-done`, `jihad-screen-capture`

## Changelog
- 2026-11-03: created from the owner's "devise a system for accurate autonomous finger testing"; R1-R7; the framebuffer sampler's feasibility measured on the device (fb1 three pages, 0.15 s a read, md5 too slow, row sums the way).
