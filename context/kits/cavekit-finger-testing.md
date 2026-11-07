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
- [x] A check library offers at least: the scroll (engine and card) within a tolerance; the zoom; the element at a point (`rect`); a field's text; the URL; a dialog present or answered; the keyboard up or down; content present in a frame region (at least 8 distinct colours and the expected text or element, never a darkness threshold); a region equal across samples (a fixed part held still); a region changed since a sample *(2026-11-04, T-563: `tools/device/finger/fingerlib.sh` — fl_check_scroll / _card_scroll, _zoom, _hit and _element_at, _text and _text_has, _url, _dialog, _keyboard (the window server's IME line), _content (8+ distinct colours by `fbstream -Q` AND the element's rect meeting the region, its text when given), fl_sample + _region_equal / _changed; `fingerlib-check.sh` runs each once on the three shells: `docs/rounds/finger-2026-11-04/notes.txt` "T-563")*
- [x] A failed check's report names the oracle line or the sample it read and keeps the frame it judged; a passed check keeps nothing *(2026-11-04: every `CHECK ok|FAIL NAME: evidence` line carries the oracle line or the region's numbers; a failure writes the shown page to `/media/internal/jihad-finger/NAME.last.raw` and the host wrapper pulls it — the deliberate failure of each round kept its frame (looked at), the passes kept none)*
- [x] The darkness-heuristic class cannot recur: no check in the library accepts a frame as content by brightness alone *(2026-11-04: the library has no brightness measure at all — `fbstream -Q` reports distinct colours, a hash and one-colour rows, and fl_check_content wants 8+ colours AND the element's rect)*
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
- [x] The sampler takes the shown page (the pan offset honoured) at ≥ 10 samples/s for 10 s on the device with the finger driving a drag, writing signatures only; its cost is measured and bounded: the daemon's frame rate on desktop Wikipedia with and without it differs by under 5 % (2026-11-03: 15/s over 35 s of 16 drags, 10.65 against 10.75 fps on Wikipedia, 18.4 against 18.5 on the long page)
- [x] The host tool reports, for a scripted drag on `jihad-long-static.html` and on desktop Wikipedia: the shown scroll per sample, the shown-frame spacing (median, p90), the tracking error (median, max), stale or repeated bands, uniform bands inside the page, the header's rows held still — and its reading of a known-good round is 0 stale, 0 uniform, the header still (2026-11-03: the reports stand — `docs/rounds/finger-2026-11-03/notes.txt`; late night, with T-568 fixed, the long page's round long-b2 reads 0 stale, 0 uniform, the header still over 16 drags, and the same build with the fix switched off reads 132 bands again; Wikipedia's reading, 0 stale and 27 bottom bands in 11 samples, is T-567's real defect, not the tool's)
- [x] A deliberately wrong frame (a dump with a band duplicated, a header moved 10 rows, a grey band painted in) is reported as such by the tool: the detectors are tested against planted faults, not only clean runs (2026-11-03: `tools/host/test-finger-stream.py`, PASS ×3 on both pages)
- [x] The daemon's verify mode and the stream agree on a clean pan (0 wrong rows there, 0 stale bands here) and the stream catches what the verify cannot see: the card's own redraw (a frame published right and shown wrong, or shown late) (2026-11-03: the long page 131 and 112 verified, 0 wrong, 0 stale; the stream times each published frame to the card's flip — median 14-18 ms, p90 25-29 — and caught the grey band the verify's reference is blind to)
**Dependencies:** R1

## R3: Geometry derived, calibrated, never guessed
**Description:** a test names page elements and chrome controls, not pixels. A page point's screen position comes
from the inject channel's `rect` of the element, the card's scroll and the shell's calibrated page top; chrome
controls come from a table per shell. Each round starts with a calibration tap per shell (a tap on a known element,
the `clickAt x= y=` line read back) and re-reads the geometry after each scroll or zoom; Mojo's header that scrolls
away, pickers that open under their select, pages wider than the card are rules of the library, not of the test.
**Acceptance Criteria:**
- [x] No test script holds a screen coordinate for a page element; chrome coordinates live in one table per shell with the calibration that checks them *(2026-11-04, T-563: a tap names an element (`fl_tap SEL`: its rect times the zoom, less the card's scroll, plus the calibrated top, less Mojo's bar while it is away); `fl_chrome NAME` is the per-shell table (measured on the 2026-11-04 screenshots; the address entry checked by the keyboard oracle on each shell); the walk and the library's round hold no page coordinate)*
- [x] A planted 20 px error in the page-top constant is caught by the calibration tap before any test runs, and the round stops with the reason *(2026-11-04: `fingerlib-check.sh enyo plant` — "CALIBRATION FAIL (enyo): the page's top measured 82 on the screen (the tap at y 202 landed on document y 120), the table says 102: off by 20 px — the round stops", exit 3, no check run; `docs/rounds/finger-2026-11-04/fl-check-enyo-plant.txt`)*
- [x] The existing walks (`tools/device/finger/prefs-finger-walk.sh`, the Mochi and Mojo scratch walks) run through the library with their hand-worked numbers gone, on all three shells *(2026-11-04: the walk's body is the library's calls — the calibration on the page's title, the tabs and controls by selector, the picker rule, waits on the clickAt, the picker's open and close and the pref's readback; the 82, the +22+38+44, the "a third down" scroll, the 45 and every sleep are gone; one pane a session, all three shells: Enyo 62 of 62, Mochi 62 of 62, Mojo 62 of 62 (the five tabs and the 57 rows each, every pref changed and put back by the finger on its own control, no pref left changed; 13-15 minutes a shell))*
**Dependencies:** R1

## R4: Waits on marks, and the slow human
**Description:** every step waits for the oracle's mark with a bound — the load done, `painted ... bytes>0`, the
keyboard's IME line, the dialog's line, the scroll settled (two equal samples), the zoom's `setZoomAndScroll` —
never a fixed sleep as the only guard. The suite includes the slow human: a dialog answered after the pickup
deadline a harness never reaches, a card closed and the job stopped the way upstart does before a persistence check,
a drag that pauses mid-way, a tap held longer than a tap and shorter than a hold.
**Acceptance Criteria:**
- [x] No bare `sleep N` stands between a gesture and its check in the suite's scripts (a lint over the suite says so); every wait names its mark and its bound, and a wait that times out is a failed check, not a skipped one *(2026-11-05, T-564: the script form has no sleep verb; `finger-run.py --lint` refuses a `sleep`, a plain `expect` after a gesture with no mark of its own (chrome, hold, dtap, pinch) with no `wait` between, a tap at a coordinate and a wrong arity — `suite/self/lint-bad.test.off` is the fixture, four findings; every `wait` names its mark (load, painted, zoom, settled, rect, line, still) with a bound and compiles to a failed check on its timeout; `docs/rounds/finger-2026-11-05/notes.txt`)*
- [x] The dialog test answers at 8 s and passes; the persistence tests stop the job with `stop <job>` and check after `start` *(2026-11-06, T-565: `dialog-slow-answer` (the confirm answered after a dwell of 8 s), `settings-persist` (`daemon-restart`: the daemon stopped as upstart does, started on the profile as it is, the pref read back); all three shells, `docs/rounds/finger-2026-11-05/t565/finger-report.md`)*
- [x] The pan test runs at three speeds (a slow drag of 2 px a scan, the 600 ms drag, the 160 ms flick) and reports each; a jittered path (±2 px) and a paused drag are gestures of the library *(2026-11-06, T-565: `pan-speeds` under one stream, per gesture in its report; `fl_drag_slow`, `fl_drag_jitter`, `fl_drag_paused`)*
**Dependencies:** R1, R2

## R5: One language, one runner, the device left as found
**Description:** a test is a short script in one declarative form (launch, wait-for, gesture, expect, shoot-on-fail)
that the runner executes on the host; a script runs in one device session (one novacom session at a time), the
suite runs the scripts of a shell one after another, refuses while the owner is testing (a lock file on the device
the owner touches), restores what a script changed (prefs, the Persona, the proxy, the jobs, the dumps in `/tmp`)
whether it passed or failed, and writes one report: each test's verdict with the oracle lines that decided it, the
frames of the failures, the round's numbers.
**Acceptance Criteria:**
- [x] The whole suite runs unattended on the three shells from one command and ends with the device as it began (the jobs running on their upstart daemons, no hand daemon, no gate file, Default look, No proxy, `/tmp` clear), checked by the runner itself at the end *(2026-11-05, T-564: `finger-run.py` — one command, the three shells, `daemon-stop.sh` after every script (the profile's prefs put back whole), `device-check.sh` at the end: 13 runs in 11 minutes — 9 passes (pan-long-drag, keyboard-address and settings-tabs on each of Enyo, Mochi and Mojo), 3 a person's (the accelerometer), 1 failure (the deliberate one of suite/self, its frame kept as kept/enyo-deliberate-fail-7-text_.jpg) — and the device check 26 of 26 ok at the end)*
- [x] The runner refuses to start while `/tmp/jihad-owner-testing` exists on the device and says so *(2026-11-05: "REFUSED: /tmp/jihad-owner-testing exists on the device — the owner is testing; remove it when done", exit 3)*
- [x] A report under `docs/rounds/finger-<date>/` lists every test with pass / fail / a person's and the evidence line; the pictures kept are the failures' and the cited ones only *(2026-11-05: `docs/rounds/finger-2026-11-05/finger-report.md` — the table of runs with the verdict, the counts and the evidence line, every run's lines, the device check; `kept/<shell>-<test>-<check>.jpg` for the failures (the deliberate one of `suite/self/`) and `shoot`s only; a `person:` script is listed as a person's with its reason)*
**Dependencies:** R1-R4

## R6: Coverage — the finger list as scripts, the kits citing them
**Description:** every item of `docs/FINGER-TESTS.md` an emulated finger can reach is a script with an id in the
suite, in each shell it names; a kit criterion the finger round covers cites the script id instead of a date and a
shot; what stays a person's (hearing, a physical keyboard, the accelerometer, the feel of smoothness beyond the
stream's numbers) is listed with the reason.
**Acceptance Criteria:**
- [x] Every reachable item of `docs/FINGER-TESTS.md` has a script; the list is regenerated from the suite's last report, with the person's items marked *(2026-11-06, T-565: 69 scripts, `finger-list.py --write`; the hand-run record kept as `docs/FINGER-TESTS-2026-10.md`. A person's: `rotate`, `flash-sound`, `audio-heard`, `mojo-launch-from-app`; not run with the reason: `tabbed-browsing`, `touchpad-go`, `keyboard-arbitration`, `webone`, which wants the owner's keyboard map and WebOne)*
- [x] The kit criteria that cite finger evidence cite a script id; `cavekit-gpu-rendering.md` R12 cites the stream's numbers for the pans, flicks, pinches and double taps *(2026-11-06, T-565: nineteen criteria in eleven kits name their scripts; R12's numbers are per shell from the round of record)*
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
- 2026-11-03: created from the owner's "devise a system for accurate autonomous finger testing"; R1-R7.
- 2026-11-03 (late night): R2 met (T-568's fix gave the known-good round).
- 2026-11-03 (night): R2 built (T-561, T-562): the sampler `fbstream` and the stream tool.
- 2026-11-04 (afternoon): R1 and R3 met (T-563): the check library `tools/device/finger/fingerlib.sh` and the geometry.
- 2026-11-06: R4 and R6 met (T-565): the suite whole on the three shells; eight real defects found and fixed. R7 is the owner's (T-566).
- 2026-11-05: R5 met and R4's lint criterion (T-564): the script form and the runner `finger-run.py`.

