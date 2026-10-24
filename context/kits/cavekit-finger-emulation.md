---
created: "2026-10-24"
last_edited: "2026-10-24"
---

# Finger emulation — the finger round driven by the agent

**Why this kit exists.** 41 acceptance criteria across the kits waited for a person at the TouchPad
(`docs/FINGER-TESTS.md`), because the two scripted ways in — the card's JS self-test hook, which calls the
shell's own gesture handlers, and the daemon's inject channel, which pans the engine directly — skip the real
input path, and "nothing but a finger pans a page". The owner, 2026-10-24: "proceed with everything blocked by the
finger tests, not by assuming it works, but by driving your own tests by emulating a finger and addressing any
issues that crop up". So: an emulated finger that travels the same path a finger does, and the round run with
it, judged by eye from screenshots and frame dumps, with what no emulation can reach left for a person.

**The path, established 2026-10-24** (the binaries and the device; `.cavekit/handoff-2026-10-24/finger-context/`):
`hidd` reads the Cypress panel over `/dev/ctp_uart` and sends every scan as one datagram of 16-byte Linux
`input_event`s to the AF_UNIX datagram socket LunaSysMgr binds at `/var/run/hidd/TouchpanelEventSocket`
(libhal's `halTouchpanelMain.module`; `touchpanel_get_event` parses: `EV_FINGERID` type 7 opens a finger item,
`EV_ABS` 0/1 x/y, `EV_KEY BTN_TOUCH` 1 down 0 up on the first and last scan, `EV_GESTURE` type 6 the gesture
code and two velocities, one `EV_SYN` a scan, 20 fingers at most); the Qt QPA plugin `libqpalm.so` turns them into
`QTouchEvent`s, LunaSysMgr into the card's events, the card into the adapter's, the adapter into the daemon's.
A root process sending the same datagrams is indistinguishable from the panel. The panel's frame is landscape
(raw x = portrait y, raw y = 767 − portrait x); the tool takes portrait screenshot coordinates.

## R1: The emulated finger travels the real path
**Description:** `tools/device/finger/finger.c`, built with the repository's toolchain, run as root on the device,
sends the panel's datagrams to LunaSysMgr's socket. No change to hidd, LunaSysMgr, the card, the adapter or the
daemon; nothing to restart; the real panel keeps working beside it.
**Acceptance Criteria:**
- [x] A raw datagram with an `EV_ABS` code the parser does not know makes LunaSysMgr log libhal's "Unexpected code": the datagrams are parsed by the consumer the panel feeds *(2026-10-24: `finger raw 7 1 0 3 5 123 1 0x14a 1` → syslog `LunaSysMgr: Unexpected code 0x5`)*
- [x] A tap at the app menu's position opens the app menu; the coordinate frame is recorded *(2026-10-24: raw (10, 737) = portrait (30, 10) opened "Find on Page / Settings / Print"; the frame above)*
- [x] A drag on a long page scrolls it, through the card and the adapter into the daemon (the screenshot shows other paragraphs at the top), where the card's hook and the inject channel could not *(2026-10-24: `jihad-long-static.html`, a 500 px drag in 600 ms moved the top from paragraph 1 to 8)*
- [x] The tool's header documents the build line, the commands and the frame; `tools/device/finger/README.md` says how a round uses it (the card launched by `luna-send`, the shot after each gesture, the daemon log read) *(2026-10-24)*
**Dependencies:** none

## R2: Every gesture of the round, with a visible reaction
**Description:** tap, double tap, long press, drag, flick (lifted while moving, with hidd's FLICK velocity), pinch in
and out, a two-finger drag; typing on the on-screen keyboard as taps on its keys; each timed like a hand (100 Hz
scans, 80 ms taps, 1.5 s holds).
**Acceptance Criteria:**
- [x] A flick coasts past where a drag of the same length stops *(2026-10-24: the same 500 px, 600 ms drag to paragraph 8, 160 ms flick to paragraph 33)*
- [x] A pinch out zooms the page in and a pinch in zooms it out *(2026-10-24: 200 → 500 px apart at the centre, the text at roughly twice the size; the pinch in is the round's)*
- [x] A long press on a page opens the context menu *(2026-10-24: "Select All / Reload / Add Bookmark / Find on Page")*
- [x] A double tap zooms a page in and, again, back out (the card's smart zoom), as the adapter's `smartZoom` does for the hook's `t` step *(2026-10-24: on desktop Wikipedia `finger dtap 384 600` made the daemon log `smartZoom at 559,752 -> block [550,734 654,770]`; 50/80 and 40/60 ms fire too, 100 ms taps 200 ms apart do not (past the double-click interval); the earlier "nothing" was `jihad-long-static.html`, a device-width page with no block to zoom to. What the card showed after it is the round's first finding: T-524.)*
- [x] A tap on the address field raises the keyboard and selects the address; a tap on a key types its letter *(2026-10-24: `finger tap 384 45` raised the keyboard with the whole address selected, a second tap brought Cut/Copy (the first try at y = 40 hit the field's edge); `finger tap 53 739` on the q key typed "q" over the selection)*
- [x] A two-finger drag scrolls (the second finger must not cancel the first) *(2026-10-24: `finger drag2 354 800 354 400` took `jihad-long-static.html` from paragraph 1 to 4)*
**Dependencies:** R1

## R3: The round driven and judged
**Description:** every line of `docs/FINGER-TESTS.md` is run with the emulated finger in each interface it names,
the result judged from the screenshot or the frame dumps by eye (memory `verify-device-frames-by-eye`), the issues
found fixed (Cavekit's revise step: the kit criterion the issue belongs to gets the fix and the re-run), and the
criterion it covers ticked in its own kit with the evidence. What no emulation reaches — hearing audio, a physical
keyboard, the accelerometer's rotation, the feel of smoothness beyond a frame timeline — stays listed for a person,
and is said so, never ticked.
**Acceptance Criteria:**
- [~] Each item of `docs/FINGER-TESTS.md` carries its result (done with the finger: the shot or dump cited, the date; or "a person": why) and the kit criteria it covers are ticked or left with the reason *(2026-10-24: the scrolling and gesture items (part 1, T-524) carry theirs; parts 2 and 3 (T-525, T-526) next)*
- [~] Every issue the round finds is fixed and its item re-run, or dispositioned with the cause *(2026-10-24, part 1: the double tap's cell and 6.2x, the zoomed frame's lost right side, the zoomed hit test, the pinch and zoom-out frames — fixed and re-run; the flick's coast gap, the first frame's 1.6-2.9 s at a new zoom and the clip above 2.9 on a wide page — dispositioned to T-527 with the data, `docs/rounds/finger-2026-10-24/zoom-grey.txt`)*
- [ ] The GPU path's R6 (the first frame at a new zoom within 250 ms, measured by the daemon's timeline under a real pinch and double tap) and R12 (pans, flicks, pinches and double taps on real pages in the three interfaces, judged from the frame dumps and the timelines) are closed by the round
**Dependencies:** R2

## R4: The tool fits the repository
**Acceptance Criteria:**
- [x] `tools/device/finger/` builds from the repository (the toolchain line in the header), ships no binary, and `docs/FINGER-TESTS.md`'s head says the round is the agent's with it *(2026-10-24)*
- [x] The screenshots a round takes are deleted once judged; only the shots a kit note cites stay (memory `clear-screenshots-when-done`) *(2026-10-24: every shot of part 1 deleted after judging; the notes cite the daemon's lines, not shots)*
**Dependencies:** R1
