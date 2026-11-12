#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# finger-run.py — the runner of the finger suite (cavekit-finger-testing.md R4, R5; T-564). A test is a short script
# in one declarative form (tools/device/finger/suite/*.test); the runner lints the suite, refuses while the owner is
# testing (/tmp/jihad-owner-testing on the device), then for each shell runs each script in ONE device session (the
# shell's hand daemon of daemon-start.sh on the script's page, the script's steps compiled onto fingerlib.sh, one
# dev-sh.sh call), pulls the frames of the failed checks as JPEGs, restores the device after every script whether it
# passed or failed (daemon-stop.sh: the hand daemon gone, the profile's prefs as before, the job back, the dumps
# removed), checks the device at the end (device-check.sh) and writes one report under docs/rounds/finger-<date>/.
#
#   tools/device/finger/finger-run.py [--shells enyo,mochi,mojo] [--only id,id] [--suite DIR] [--self] [--lint]
#                                     [--round DIR]
#
# THE SCRIPT FORM (one step a line; `#` comments; blank lines ignored):
#   header lines    id: NAME            shells: enyo mochi mojo (default all three)
#                   page: URL           the page the card opens on (default about:blank)
#                   prefs: n=v n=v      prefs for this run (stripped after)
#                   kit: FILE Rn        what the test is evidence for (the report cites it)
#                   person: REASON      a person's item: no device session, listed as such
#   launch URL                          a page through the inject channel, its load done and a frame painted
#   wait load|painted|zoom|settled [BOUND]      a mark with a bound (a wait that times out is a FAILED check)
#   wait rect SEL [BOUND] | wait line PATTERN [BOUND] | wait still REGION [BOUND] | wait changed REGION [BOUND] |
#   wait text SEL TEXT [BOUND]
#   calibrate SEL [TOL]                 the page-top calibration tap (the round stops on a disagreement)
#   tap SEL | tap-at X Y | chrome NAME | drag X0 Y0 X1 Y1 [MS] | flick X0 Y0 X1 Y1 [MS] | pinch CX CY D0 D1 [MS]
#   hold X Y [MS] | hold-on SEL [MS] | dtap X Y | pick SEL INDEX [ROWS]
#   header-away                         Mojo's bar went with a scroll the page made itself: the tools' model says so
#   kernel-key CODE                     a key of the emulated kernel keyboard (fixture: kernel-kbd), a raw Linux code
#   type-keys TEXT | key-hold kb-backspace [MS]     the system keyboard by finger (type TEXT is the inject channel's) | card-to Y | pull-header | key CODE | type TEXT | inject CMD...
#   expect CHECK ARGS...                fl_check_CHECK: scroll EY KY TOL | card-scroll KY TOL | zoom Z TOL | hit SEL |
#                                       element-at SEL X Y | text SEL TEXT | text-has SEL TEXT | url PART |
#                                       dialog present|answered [BOUND] | keyboard up|down [BOUND] |
#                                       content REGION SEL [TEXT] | region-equal REGION [SAMPLE] |
#                                       region-changed REGION [SAMPLE] | top SEL [TOL]
#   drag-slow X0 Y0 X1 Y1 | drag-jitter X0 Y0 X1 Y1 [MS] | drag-paused X0 Y0 X1 Y1 [PAUSE] [MS]   the slow human's drags
#   pause MS                            the human's own delay before the NEXT GESTURE (a dialog answered late); never
#                                       a guard before a check — the lint refuses that
#   daemon-restart URL                  the daemon stopped as upstart stops it and started again on URL (persistence)
#   stream-start [S] [fbstream opts] … stream-stop   the frame sampler over the gestures between; the runner reads the
#                                       stream with `stream:` header options (finger-stream.py) into the report; `stream-mojo:`
#                                       replaces them on Mojo (its circles stand over the foot of the page, its bar moves the header)
#   sample NAME REGION                  a region's hash remembered for region-equal / region-changed
#   shoot NAME                          the shown page kept whatever the verdict (for a picture a kit note cites)
#   item: TEXT / section: NAME          the docs/FINGER-TESTS.md entry this script is (finger-list.py regenerates the list)
#   fixture: NAME ...                   suite/fixtures/NAME.sh up SHELL before the daemon starts, down SHELL after it stops
#   tap-off SEL DX DY                   a tap at an offset inside an element (a field in a frame); dwell MS between a check
#                                       and the same check: does it still hold?
#   known: T-NNN WORD [; T-NNN WORD]    failures each holding a WORD are those open defects': the verdict reads `known`
#   dismiss X Y                         a tap that closes a menu (it reaches no page, so no clickAt is awaited)
#   wait gone SEL | url PART | pref T NAME TEXT | differs REGION SAMPLE   more marks, each with an optional last BOUND
#   wait quiet PATTERN SECS            the ABSENCE of a line for SECS since the last mark (what must not happen: a video
#                                       starting by itself); the line coming is a FAILED check
#   tool: TEXT / skip: TEXT / needs: x y  an item covered by another tool (the settings walk), not run with a reason, or
#                                       needing what the host provides (proxy, webone, internet: `--with proxy,internet`)
#   expect stream stale|uniform|still|lag-max|error-max VALUE   checked on the host from the stream tool's reading
#   probe NAME 'JS'                    a script run inside the page (the compat component's on-demand probe: the header
#                                       needs prefs: jihad.compat.trace=true); what it returns is printed and kept for
#                                       `expect probe-has TEXT`. A page's Content-Security-Policy cannot refuse it.
#   A gesture with no mark of its own (chrome, hold, dtap, pinch) needs a `wait` before the next `expect`; `sleep`
#   is not a verb of the form. The lint says so.
import sys, os, re, subprocess, json, time, datetime, shutil, tempfile, argparse, base64

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
DEVSH = os.path.join(ROOT, "build", "webos-oe", "dev-sh.sh")
REFS = os.path.join(os.environ.get("JIHAD_FLASH_CACHE", os.path.expanduser("~/.cache/jihad-flash")), "refs")   # a movie's screens (cavekit-flash R2)
SHELLS = ["enyo", "mochi", "mojo"]
GESTURES = {"kernel-key", "flash-tap", "plugin-signal", "tap", "tap-off", "touch", "dismiss", "tap-at", "chrome", "drag", "drag-off", "drag-slow", "drag-jitter", "drag-paused", "flick", "pinch", "hold", "hold-on", "dtap", "pick", "card-to", "pull-header", "key", "type", "type-keys", "key-hold", "inject", "launch", "calibrate", "daemon-restart"}
UNMARKED = {"kernel-key", "chrome", "dismiss", "touch", "hold", "hold-on", "dtap", "pinch", "type-keys", "key-hold"}       # no mark of their own: a wait must follow before an expect
WAITING_EXPECTS = {"lock", "keyboard", "dialog", "flash-at", "flash-rate", "flash-rate-report", "flash-reacted", "plugin", "audio"}            # these expects wait with a bound themselves
CHECKS = {"scroll": "fl_check_scroll", "card-scroll": "fl_check_card_scroll", "card-scroll-min": "fl_check_card_scroll_min", "zoom": "fl_check_zoom",
          "zoom-min": "fl_check_zoom_min", "zoom-max": "fl_check_zoom_max", "visible": "fl_check_visible", "stream": None, "hit": "fl_check_hit", "file": "fl_check_file",
          "element-at": "fl_check_element_at", "text": "fl_check_text", "text-has": "fl_check_text_has", "url": "fl_check_url", "probe-has": "fl_check_probe_has",
          "dialog": "fl_check_dialog", "keyboard": "fl_check_keyboard", "content": "fl_check_content",
          "region-equal": "fl_check_region_equal", "region-changed": "fl_check_region_changed", "top": "fl_check_top",
          "flash-at": "fl_check_flash_at", "flash-drawn": "fl_check_flash_drawn", "flash-rate": "fl_check_flash_rate", "flash-rate-report": "fl_check_flash_rate_report", "flash-reacted": "fl_check_flash_reacted",
          "plugin": "fl_check_plugin", "audio": "fl_check_audio", "screen": "fl_check_screen", "flash-behind": "fl_check_flash_behind", "above-keyboard": "fl_check_above_keyboard", "press-rate": "fl_check_press_rate", "number": "fl_check_number", "flash-el": "fl_check_flash_el", "lock": "fl_check_lock"}

def sh_quote(s):
    return "'" + s.replace("'", "'\\''") + "'"

def split_args(rest):
    # arguments separated by spaces; a selector with spaces is quoted "…" or '…'. A backslash is kept (a grep pattern's
    # \| and \[ need it) unless it escapes the quote in use
    out, cur, q, i = [], "", None, 0
    for c, nxt in zip(rest, rest[1:] + " "):
        i += 1
        if q:
            if c == "\\" and nxt == q: continue
            if c == q and not (i >= 2 and rest[i - 2] == "\\"): q = None
            else: cur += c
        elif c in "\"'": q = c
        elif c == " ":
            if cur: out.append(cur); cur = ""
        else: cur += c
    if cur: out.append(cur)
    return out
ARITY = {"calibrate": (1, 2), "tap": (1, 1), "dismiss": (2, 2), "tap-off": (3, 3), "dwell": (1, 1), "tap-at": (2, 2), "chrome": (1, 1), "pick": (2, 3), "card-to": (1, 1), "sample": (2, 2), "shoot": (1, 1), "measure": (1, 1),
         "hold-on": (1, 2), "touch": (1, 1), "key-hold": (1, 2), "drag": (4, 5), "drag-off": (5, 6), "drag-slow": (4, 4), "drag-jitter": (4, 5), "drag-paused": (4, 6), "flick": (4, 5), "pinch": (4, 5), "hold": (2, 3), "dtap": (2, 4),
         "key": (1, 1), "launch": (1, 2), "pause": (1, 1), "daemon-restart": (1, 1), "stream-start": (0, 9), "stream-stop": (0, 0),
         "header-away": (0, 0), "kernel-key": (1, 1), "flash-tap": (6, 6), "flash-control": (2, 2), "plugin-signal": (1, 1), "sig-save": (2, 2), "probe": (2, 2)}
EXPECT_ARITY = {"above-keyboard": (1, 1), "press-rate": (1, 1), "number": (3, 3), "flash-el": (4, 4), "lock": (1, 2), "flash-behind": (3, 3), "flash-rate-report": (2, 2), "flash-at": (2, 3), "flash-drawn": (2, 2), "flash-rate": (4, 4), "flash-reacted": (3, 3), "plugin": (0, 2), "audio": (1, 2), "screen": (3, 4), "scroll": (3, 3), "card-scroll": (2, 2), "card-scroll-min": (1, 1), "zoom": (2, 2), "zoom-min": (1, 1), "zoom-max": (1, 1), "visible": (1, 1), "stream": (2, 2),
                "hit": (1, 1), "file": (2, 2), "element-at": (3, 3), "text": (2, 2), "text-has": (2, 2),
                "url": (1, 1), "dialog": (1, 2), "keyboard": (1, 2), "content": (2, 3), "region-equal": (1, 2), "region-changed": (1, 2), "top": (1, 2), "probe-has": (1, 1)}

def parse(path):
    t = {"id": os.path.basename(path)[:-5], "shells": SHELLS[:], "page": "about:blank", "prefs": [], "kit": "", "person": "", "steps": [], "path": path,
         "item": "", "section": "", "stream": "", "stream-mojo": "", "tool": "", "needs": [], "skip": "", "fixture": [], "known": ""}
    for n, raw in enumerate(open(path), 1):
        line = raw.strip()
        if not line or line.startswith("#"): continue
        m = re.match(r"^(id|shells|page|prefs|kit|person|item|section|stream|stream-mojo|tool|needs|skip|fixture|known):\s*(.*)$", line)
        if m:
            k, v = m.group(1), m.group(2).strip()
            if k == "shells": t[k] = v.split()
            elif k in ("prefs", "needs", "fixture"): t[k] = v.split()
            else: t[k] = v
            continue
        # `mojo: wait content 10,90,400,300 5`: a step for ONE shell (a panel that lies elsewhere there)
        m = re.match(r"^(enyo|mochi|mojo):\s+(.+)$", line)
        sh = None
        if m: sh, line = m.group(1), m.group(2).strip()
        verb, rest = (line.split(None, 1) + [""])[:2]
        t["steps"].append((n, verb, rest, sh))
    return t

def lint(tests):
    errs = []
    for t in tests:
        pending = None       # an unmarked gesture waiting for a wait
        for n, verb, rest, _sh in t["steps"]:
            a = split_args(rest)
            if verb in ("sleep", "usleep"):
                errs.append(f"{t['id']}:{n}: `{verb}` is not a verb of the form (wait on a mark)")
            elif verb == "wait":
                if not a or a[0] not in ("load", "painted", "zoom", "settled", "rect", "gone", "line", "still", "changed", "content", "differs", "text", "url", "pref", "log", "quiet"):
                    errs.append(f"{t['id']}:{n}: wait needs a mark: load|painted|zoom|settled|rect SEL|gone SEL|line PATTERN|still REGION|changed REGION|differs REGION SAMPLE|text SEL TEXT|url PART|pref T NAME TEXT|quiet PATTERN SECS")
                pending = None
            elif verb in GESTURES:
                if verb in UNMARKED: pending = (n, verb)
                else: pending = None
            elif verb == "pause":
                # the slow human's own delay (a dialog answered late, a hand resting): never a guard — what follows must
                # be a gesture, not a check
                nxt = [v for m2, v, _, _ in t["steps"] if m2 > n]
                if not a or not a[0].isdigit() or int(a[0]) > 30000: errs.append(f"{t['id']}:{n}: pause takes milliseconds up to 30000")
                if nxt and nxt[0] not in GESTURES and nxt[0] not in ("stream-stop", "shoot"): errs.append(f"{t['id']}:{n}: a pause is the human's delay before a gesture, never a guard before `{nxt[0]}`")
            elif verb == "dwell":
                # a persistence check: what held just before still holds after MS — a check on each side, so it guards nothing
                steps = t["steps"]; i = [m2 for m2, _, _, _ in steps].index(n)
                prev = steps[i - 1][1] if i > 0 else ""; nxt = steps[i + 1][1] if i + 1 < len(steps) else ""
                if prev not in ("expect", "wait") or nxt != "expect": errs.append(f"{t['id']}:{n}: dwell stands between a check and the same check (does it still hold?), not `{prev}` and `{nxt}`")
            elif verb in ("stream-start", "stream-stop"):
                pass
            elif verb == "probe":
                pending = None     # it waits for the page's answer
                if "jihad.compat.trace=true" not in t["prefs"]: errs.append(f"{t['id']}:{n}: a probe needs `prefs: jihad.compat.trace=true` in the header")
            elif verb == "expect":
                if not a or a[0] not in CHECKS:
                    errs.append(f"{t['id']}:{n}: expect needs a check: {', '.join(sorted(CHECKS))}")
                elif pending and a[0] not in WAITING_EXPECTS:
                    errs.append(f"{t['id']}:{n}: `expect {a[0]}` after `{pending[1]}` (line {pending[0]}) with no wait between: name the mark")
            elif verb in ("sample", "shoot", "measure", "flash-control", "sig-save", "header-away"):
                pass
            else:
                errs.append(f"{t['id']}:{n}: unknown verb `{verb}`")
            if verb in ("tap", "chrome") and a and a[0].lstrip("-").isdigit():
                errs.append(f"{t['id']}:{n}: `{verb}` takes an element or a chrome name, not a coordinate")
            lo_hi = ARITY.get(verb) or (EXPECT_ARITY.get(a[0]) if verb == "expect" and a else None)
            if lo_hi:
                na = len(a) - (1 if verb == "expect" else 0)
                if na < lo_hi[0] or na > lo_hi[1]:
                    errs.append(f"{t['id']}:{n}: `{verb}{' ' + a[0] if verb == 'expect' else ''}` takes {lo_hi[0]}{'-' + str(lo_hi[1]) if lo_hi[1] != lo_hi[0] else ''} argument(s), got {na} — quote a selector with spaces")
        if not (t["person"] or t["tool"] or t["skip"]) and not any(v == "expect" or (v == "wait") for _, v, _, _ in t["steps"]):
            errs.append(f"{t['id']}: no expect and no wait: nothing is checked")
    return errs

def compile_steps(t, shell=None):
    out = []
    for n, verb, rest, sh in t["steps"]:
        if sh and sh != shell: continue
        a = split_args(rest); q = [sh_quote(x) for x in a]; name = f"{n}-{verb}"
        m = [x.replace('"', "'").replace('$', '').replace('`', '') for x in a]   # a's words safe inside a "…" message
        if verb == "launch":
            out.append(f'fl_load {q[0]} {a[1] if len(a) > 1 else 40} && echo "WAIT ok {n}-launch: load done" || fl_check FAIL "{n}-launch" "no load done for {m[0]}"')
        elif verb == "wait":
            mark = a[0]; b = a[-1] if len(a) > 1 and a[-1].isdigit() else ""
            if mark in ("load", "painted", "zoom"):
                out.append(f'fl_wait_{mark} {b or ""} >/dev/null && echo "WAIT ok {n}-{mark}" || fl_check FAIL "{n}-wait-{mark}" "no {mark} mark within the bound"')
            elif mark == "settled":
                out.append(f'fl_wait_settled {b or 5} && echo "WAIT ok {n}-settled: the card\'s scroll $(fl_card_scroll)" || fl_check FAIL "{n}-wait-settled" "the card\'s scroll did not settle"')
            elif mark == "rect":
                out.append(f'fl_wait_rect {q[1]} {b or 5} && echo "WAIT ok {n}-rect {m[1]}" || fl_check FAIL "{n}-wait-rect" "no element {m[1]} with a size within the bound"')
            elif mark == "gone":
                out.append(f'fl_wait_gone {q[1]} {b or 5} && echo "WAIT ok {n}-gone {m[1]}" || fl_check FAIL "{n}-wait-gone" "{m[1]} still there after the bound"')
            elif mark == "url":
                out.append(f'l=$(fl_wait_url {q[1]} {b or 10}) && echo "WAIT ok {n}-url: $l" || fl_check FAIL "{n}-wait-url" "the URL [$(fl_url)] never held {m[1]}"')
            elif mark == "pref":
                out.append(f'l=$(fl_wait_pref_has {q[1]} {q[2]} {q[3]} {a[4] if len(a) > 4 else 5}) && echo "WAIT ok {n}-pref {m[2]} = [$l]" || fl_check FAIL "{n}-wait-pref" "{m[2]} = [$l], never held {m[3]}"')
            elif mark == "differs":
                out.append(f'fl_wait_differs {q[1]} {q[2]} {a[3] if len(a) > 3 else 5} && echo "WAIT ok {n}-differs {m[1]}" || fl_check FAIL "{n}-wait-differs" "{m[1]} still shows the sample {m[2]} after the bound"')
            elif mark == "line":
                out.append(f'l=$(fl_wait {q[1]} {b or 5}) && echo "WAIT ok {n}-line: $l" || fl_check FAIL "{n}-wait-line" "no line {m[1]} within the bound"')
            elif mark == "quiet":
                out.append(f'fl_mark; l=$(fl_wait_quiet {q[1]} {b or 10}) && fl_check ok "{n}-quiet" "no line {m[1]} for {b or 10} s" || fl_check FAIL "{n}-wait-quiet" "the line came: $l"')
            elif mark == "still":
                out.append(f'fl_wait_still {q[1]} {b or 4} && echo "WAIT ok {n}-still {m[1]}" || fl_check FAIL "{n}-wait-still" "the region {m[1]} never held still"')
            elif mark == "log":
                out.append(f'l=$(fl_wait_log {q[1]} {b or 5}) && echo "WAIT ok {n}-log: $l" || fl_check FAIL "{n}-wait-log" "no line {m[1]} in the daemon log within the bound"')
            elif mark == "content":
                out.append(f'fl_wait_content {q[1]} {b or 10} && echo "WAIT ok {n}-content {m[1]}" || fl_check FAIL "{n}-wait-content" "no content in {m[1]} within the bound"')
            elif mark == "changed":
                out.append(f'fl_wait_changed {q[1]} {b or 5} && echo "WAIT ok {n}-changed {m[1]}" || fl_check FAIL "{n}-wait-changed" "the region {m[1]} did not change within the bound"')
            elif mark == "text":
                out.append(f'l=$(fl_wait_text {q[1]} {q[2]} {b if len(a) > 3 else 5}) && echo "WAIT ok {n}-text {m[1]} = [$l]" || fl_check FAIL "{n}-wait-text" "{m[1]} never read {m[2]} within the bound"')
        elif verb == "calibrate": out.append(f"fl_calibrate {' '.join(q)}")
        elif verb == "tap": out.append(f"fl_tap {q[0]} || fl_check FAIL \"{name}\" \"the tap on {m[0]} reached no clickAt\"")
        elif verb == "dismiss": out.append(f"fl_mark; /tmp/finger tap {q[0]} {q[1]}; echo \"  a tap to dismiss at {a[0]},{a[1]}\"")
        elif verb == "tap-off": out.append(f"fl_tap_off {' '.join(q)} || fl_check FAIL \"{name}\" \"the tap in {m[0]} reached no clickAt\"")
        elif verb == "touch": out.append(f"fl_touch {q[0]} || fl_check FAIL \"{name}\" \"no element {m[0]} to touch\"")
        elif verb == "dwell": out.append(f"usleep {int(a[0]) * 1000}; echo \"  dwell {a[0]} ms (does it still hold?)\"")
        elif verb == "tap-at": out.append(f"fl_tap_at {q[0]} {q[1]} || fl_check FAIL \"{name}\" \"the tap at {a[0]},{a[1]} reached no clickAt\"")
        elif verb == "chrome": out.append(f"fl_chrome_tap {q[0]} || fl_check FAIL \"{name}\" \"no chrome entry {a[0]}\"")
        elif verb == "drag": out.append(f"fl_drag {' '.join(q)}")
        elif verb == "drag-off": out.append(f"fl_drag_off {' '.join(q)} || fl_check FAIL \"{name}\" \"no element {m[0]} to drag on\"")
        elif verb == "drag-slow": out.append(f"fl_drag_slow {' '.join(q)}")
        elif verb == "drag-jitter": out.append(f"fl_drag_jitter {' '.join(q)}")
        elif verb == "drag-paused": out.append(f"fl_drag_paused {' '.join(q)}")
        elif verb == "pause": out.append(f"usleep {int(a[0]) * 1000}; echo \"  pause {a[0]} ms (the slow human)\"")
        elif verb == "daemon-restart": out.append(f"fl_daemon_restart {q[0]} || fl_check FAIL \"{name}\" \"the daemon did not come back on {m[0]}\"")
        elif verb == "stream-start": out.append(f"fl_stream_start {' '.join(q)}")
        elif verb == "stream-stop": out.append("fl_stream_stop")
        elif verb == "flick": out.append(f"fl_flick {' '.join(q)}")
        elif verb == "pinch": out.append(f"fl_mark; /tmp/finger pinch {' '.join(q)}; echo \"  pinch {rest}\"")
        elif verb == "hold": out.append(f"fl_mark; /tmp/finger hold {' '.join(q)}; echo \"  hold {rest}\"")
        elif verb == "hold-on": out.append(f"fl_hold {' '.join(q)} || fl_check FAIL \"{name}\" \"no element {m[0]}\"")
        elif verb == "dtap": out.append(f"fl_mark; /tmp/finger dtap {' '.join(q)}; echo \"  double tap {rest}\"")
        elif verb == "pick": out.append(f"fl_pick {' '.join(q)} || fl_check FAIL \"{name}\" \"the pick on {m[0]} did not open or close\"")
        elif verb == "card-to": out.append(f"fl_card_to {q[0]}")
        elif verb == "pull-header": out.append("fl_pull_header")
        elif verb == "header-away": out.append("FL_HDR=1")
        elif verb == "kernel-key": out.append(f"echo 'key {a[0]} 1' > /tmp/uinput.cmd; usleep 500000; echo '  kernel key {a[0]}'")
        elif verb == "key": out.append(f"fl_ask \"key {a[0]}\" 'inject key ' 3 >/dev/null || fl_check FAIL \"{name}\" \"the key {a[0]} was not echoed\"")
        elif verb == "type": out.append(f"fl_ask \"text {rest}\" 'inject text ' 3 >/dev/null || fl_check FAIL \"{name}\" \"the text was not echoed\"")
        elif verb == "type-keys": out.append(f"fl_type {sh_quote(rest.strip().strip(chr(39)).strip(chr(34)))} || fl_check FAIL \"{name}\" \"a character has no key\"")
        elif verb == "key-hold": out.append(f"fl_kb_hold {' '.join(q)} || fl_check FAIL \"{name}\" \"no key {a[0]}\"")
        elif verb == "inject": out.append(f"fl_mark; fl_inject {sh_quote(rest)}; echo \"  inject {rest}\"")
        elif verb == "flash-tap": out.append(f"fl_flash_tap {' '.join(q)} || fl_check FAIL \"{name}\" \"the movie reported nothing after the tap\"")
        elif verb == "flash-control": out.append(f"fl_flash_control {' '.join(q)}")
        elif verb == "plugin-signal": out.append(f"kill -{a[0]} $(fl_plugin_pid); echo \"  plugin-container sent {a[0]}\"")
        elif verb == "sig-save": out.append(f"echo \"SIG {a[0]} $(fl_screen_sig {q[1]})\"")
        elif verb == "expect":
            if a[0] == "stream": out.append(f"echo \"  (the stream's {a[1]} checked on the host)\"")
            elif a[0] == "screen":      # expect screen REGION REF same|other [TOL]: the reference by id, from the corpus's cache
                ref = os.path.join(REFS, a[2] + ".sig")
                if os.path.exists(ref): out.append(f"fl_check_screen \"{n}-screen\" {q[1]} {open(ref).read().strip()} {' '.join(q[3:])}")
                else: out.append(f"fl_check FAIL \"{n}-screen\" \"no reference {a[2]} (sig-save, --make-refs)\"")
            else: out.append(f"{CHECKS[a[0]]} \"{n}-{a[0]}\" {' '.join(q[1:])}")
        elif verb == "sample": out.append(f"fl_sample {' '.join(q)}")
        elif verb == "shoot": out.append(f"fl_keep {q[0]}")
        elif verb == "measure": out.append(f"echo \"  measure {m[0]}: $(fl_rect {q[0]} 2>/dev/null || echo none) (x y w h card-x card-y scroll-x scroll-y center-x center-y, document px)\"")
        elif verb == "probe":        # a different text each time (a comment with the line): an unchanged pref fires nothing
            b = base64.b64encode((a[1] + "\n//" + str(n)).encode()).decode()
            out.append(f'fl_probe {q[0]} {b} || fl_check FAIL "{name}" "no answer to the probe {m[0]} within 30 s"')
    return "\n".join(out) + "\n"

def device_ready(bound=300):
    """The device on novacom, and the system's "Connected" alert closed by LOOKING at the screen
    (tools/device/close-usb-alert.sh), the owner's standing rule: a drop inside a command can come back before any
    poll sees it gone, so a blind tap on "was away" missed alerts (2026-11-09). Returns the seconds the device was away
    (-1: it did not come back)."""
    r = subprocess.run([os.path.join(ROOT, "tools", "device", "close-usb-alert.sh"), str(bound)],
                       capture_output=True, text=True, errors="replace")
    out = r.stdout.strip()
    if r.returncode != 0 and "no device" in out:
        return -1
    if "came back" in out or "was up" in out or "STILL UP" in out:
        print("  " + out.replace("\n", "; "))
    m = re.search(r"came back after (\d+) s", out)
    return int(m.group(1)) if m else 0

def devsh(script_text, args, bound, tmp):
    p = os.path.join(tmp, "s.sh")
    open(p, "w").write(script_text)
    r = subprocess.run([DEVSH, "-t", str(bound), p] + args, capture_output=True, text=True, errors="replace")
    return r.returncode, r.stdout + r.stderr

def raw_to_jpg(raw, jpg):
    try:
        import numpy as np
        from PIL import Image
        b = np.fromfile(raw, dtype=np.uint8)
        fb = b[:1024 * 768 * 4].reshape(768, 1024, 4)[:, :, [2, 1, 0]]
        Image.fromarray(np.ascontiguousarray(np.transpose(fb[::-1], (1, 0, 2)))).save(jpg, quality=80)
        return True
    except Exception as e:
        print(f"  (the frame {raw} could not be converted: {e})"); return False

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--shells", default=",".join(SHELLS)); ap.add_argument("--only", default="")
    ap.add_argument("--suite", default=os.path.join(HERE, "suite")); ap.add_argument("--self", action="store_true", help="the suite's own scripts under suite/self too")
    ap.add_argument("--lint", action="store_true", help="lint the suite and stop"); ap.add_argument("--round", default="")
    ap.add_argument("--make-refs", action="store_true", help="a sig-save line stores its reference in the corpus's cache where there is none")
    ap.add_argument("--merge", action="store_true", help="keep the round's last report and replace only the runs made now (marked rerun), the kept frames of the replaced runs removed")
    ap.add_argument("--no-end-check", action="store_true"); ap.add_argument("--with", dest="with_", default="", help="what the host provides this run: proxy, webone, internet (scripts with `needs:` beyond it are skipped)")
    o = ap.parse_args()
    files = sorted(f for f in os.listdir(o.suite) if f.endswith(".test"))
    paths = [os.path.join(o.suite, f) for f in files]
    if o.self and os.path.isdir(os.path.join(o.suite, "self")):
        paths += sorted(os.path.join(o.suite, "self", f) for f in os.listdir(os.path.join(o.suite, "self")) if f.endswith(".test"))
    tests = [parse(p) for p in paths]
    if o.only: tests = [t for t in tests if t["id"] in o.only.split(",")]
    errs = lint(tests)
    print(f"lint: {len(tests)} scripts, {len(errs)} finding(s)")
    for e in errs: print("  " + e)
    if o.lint or errs: sys.exit(2 if errs else 0)
    shells = [s for s in o.shells.split(",") if s in SHELLS]
    round_dir = o.round or os.path.join(ROOT, "docs", "rounds", "finger-" + datetime.date.today().isoformat())
    os.makedirs(os.path.join(round_dir, "kept"), exist_ok=True)
    tmp = tempfile.mkdtemp()
    # the owner's lock
    rc, out = devsh("[ -e /tmp/jihad-owner-testing ] && echo LOCKED || echo free\n", [], 30, tmp)
    if "LOCKED" in out:
        print("REFUSED: /tmp/jihad-owner-testing exists on the device — the owner is testing; remove it when done"); sys.exit(3)
    if "free" not in out:
        print("the device did not answer the lock check:\n" + out); sys.exit(3)
    lib = open(os.path.join(HERE, "fingerlib.sh")).read()
    # the sampler from its source, pushed once
    tc = os.path.join(ROOT, "build", "webos-oe", "toolchain", "out-toolchain", "x-tools", "arm-webos-linux-gnueabi", "bin", "arm-webos-linux-gnueabi-gcc")
    fb = os.path.join(tmp, "fbstream")
    if subprocess.run([tc, "-O2", "-march=armv7-a", "-mfpu=neon", "-mfloat-abi=softfp", "-std=gnu99", "-o", fb, os.path.join(HERE, "fbstream.c")]).returncode == 0:
        subprocess.run(["novacom", "put", "file:///tmp/fbstream"], stdin=open(fb, "rb"))
    fg = os.path.join(tmp, "finger")
    if subprocess.run([tc, "-O2", "-march=armv7-a", "-mfpu=neon", "-mfloat-abi=softfp", "-std=gnu99", "-o", fg, os.path.join(HERE, "finger.c")]).returncode == 0:
        subprocess.run(["novacom", "put", "file:///tmp/finger"], stdin=open(fg, "rb"))
    subprocess.run(["novacom", "put", "file:///tmp/fl-daemon-start.sh"], stdin=open(os.path.join(HERE, "daemon-start.sh"), "rb"))   # fl_daemon_restart
    report = {"date": datetime.datetime.now().isoformat(timespec="seconds"), "shells": shells, "tests": [], "end_check": ""}
    t_start = time.time()
    for shell in shells:
        for t in tests:
            if shell not in t["shells"]: continue
            rec = {"shell": shell, "id": t["id"], "kit": t["kit"], "item": t["item"], "section": t["section"], "verdict": "", "checks": [], "kept": [], "seconds": 0, "note": ""}
            report["tests"].append(rec)
            if t["person"]:
                rec["verdict"] = "a person's"; rec["note"] = t["person"]; print(f"[{shell}] {t['id']}: a person's — {t['person']}"); continue
            if t["tool"]:
                rec["verdict"] = "by a tool"; rec["note"] = t["tool"]; print(f"[{shell}] {t['id']}: by a tool — {t['tool']}"); continue
            if t["skip"]:
                rec["verdict"] = "not run"; rec["note"] = t["skip"]; print(f"[{shell}] {t['id']}: not run — {t['skip']}"); continue
            lacking = [x for x in t["needs"] if x not in o.with_.split(",")]
            if lacking:
                rec["verdict"] = "not run"; rec["note"] = "needs " + ", ".join(lacking) + " (run with --with " + ",".join(t["needs"]) + ")"; print(f"[{shell}] {t['id']}: not run — {rec['note']}"); continue
            t0 = time.time()
            for fx in t["fixture"]:
                r3 = subprocess.run([os.path.join(HERE, "suite", "fixtures", fx + ".sh"), "up", shell], capture_output=True, text=True, errors="replace")
                rec["note"] = (rec["note"] + " " + r3.stdout.strip().splitlines()[-1] if r3.stdout.strip() else rec["note"]).strip()
            if device_ready() < 0:
                rec["verdict"] = "fail"; rec["note"] = "the device did not come back in 300 s"; print(f"[{shell}] {t['id']}: no device"); continue
            print(f"[{shell}] {t['id']}: the daemon on {t['page']}")
            rc, out = devsh(open(os.path.join(HERE, "daemon-start.sh")).read(), [shell, t["page"]] + ["PREF=" + p for p in t["prefs"]], 150, tmp)
            start_line = [l for l in out.splitlines() if "content frames" in l or "FATAL" in l]
            rec["start"] = start_line[-1][:200] if start_line else out.strip()[-200:]
            if "FATAL" in out or not start_line:
                rec["verdict"] = "fail"; rec["note"] = "the daemon did not start: " + out.strip().splitlines()[-1][:160]
            else:
                body = "chmod +x /tmp/fbstream /tmp/finger 2>/dev/null\n" + lib + f"fl_init {shell}\n" + compile_steps(t, shell) + "fl_summary\n"
                bound = min(900, 90 + 25 * len(t["steps"]))
                rc, out = devsh(body, [shell], bound, tmp)
                for l in out.splitlines():
                    if l.startswith("CHECK ") or l.startswith("WAIT ") or l.startswith("CALIBRATION FAIL") or l.startswith("calibrated"):
                        rec["checks"].append(l)
                    m = re.search(r"frame kept: (/media/internal/jihad-finger/\S+)", l)
                    if m:
                        raw = os.path.join(tmp, os.path.basename(m.group(1)))
                        with open(raw, "wb") as fh: subprocess.run(["novacom", "get", "file://" + m.group(1)], stdout=fh)
                        jpg = os.path.join(round_dir, "kept", f"{shell}-{t['id']}-{os.path.basename(m.group(1)).replace('.last.raw', '')}.jpg")
                        if raw_to_jpg(raw, jpg): rec["kept"].append(os.path.relpath(jpg, round_dir))
                        os.unlink(raw)
                if any(v == "stream-stop" for _, v, _, _ in t["steps"]):   # the stream pulled and read (R6: the numbers in the report)
                    for ext in ("fbs", "fin"):
                        with open(os.path.join(tmp, "stream." + ext), "wb") as fh: subprocess.run(["novacom", "get", "file:///media/internal/jihad-finger/stream." + ext], stdout=fh)
                    if os.path.getsize(os.path.join(tmp, "stream.fbs")) > 100:
                        keep = os.path.join(round_dir, f"fs-{shell}-{t['id']}.fbs"); shutil.copy(os.path.join(tmp, "stream.fbs"), keep)
                        shutil.copy(os.path.join(tmp, "stream.fin"), keep[:-4] + ".fin")
                        r2 = subprocess.run([sys.executable, "-I", os.path.join(ROOT, "tools", "host", "finger-stream.py"), keep, "--trace", keep[:-4] + ".fin"] + ((t["stream-mojo"] if shell == "mojo" and t["stream-mojo"] else t["stream"]).split()), capture_output=True, text=True, errors="replace")
                        rec["stream"] = [l for l in r2.stdout.splitlines() if re.search(r"stale|uniform|tracking|spacing|samples|still|backwards|shown frames", l)][:12]
                        rec["checks"].append("STREAM " + " | ".join(rec["stream"])[:600])
                        # `expect stream KEY VALUE` lines are checked here from the stream tool's reading: stale / uniform (bands, at
                        # most VALUE), still (rows moved, at most VALUE), lag-max / error-max (ms / px, at most VALUE)
                        text = r2.stdout
                        for n2, v2, rest2, _sh in t["steps"]:
                            a2 = split_args(rest2)
                            if v2 != "expect" or not a2 or a2[0] != "stream" or len(a2) < 3: continue
                            key, want = a2[1], a2[2]
                            pat = {"stale": r"stale bands (\d+)", "uniform": r"uniform bands (\d+)", "still": r"still rows moved (\d+)",
                                   "lag-max": r"lag median \S+ ms, max (\d+)", "error-max": r"error median \S+ px, max (\d+)"}.get(key)
                            m2 = re.search(pat, text) if pat else None
                            if not m2: rec["checks"].append(f"CHECK FAIL {n2}-stream-{key}: the stream's reading has no {key}")
                            elif int(m2.group(1)) <= int(want): rec["checks"].append(f"CHECK ok {n2}-stream-{key}: {key} {m2.group(1)} (at most {want})")
                            else: rec["checks"].append(f"CHECK FAIL {n2}-stream-{key}: {key} {m2.group(1)} (want at most {want})")
                    else: rec["checks"].append("STREAM none pulled")
                fails = [l for l in rec["checks"] if l.startswith("CHECK FAIL") or l.startswith("CALIBRATION FAIL")]
                ended = any(l.startswith("CHECKS ") for l in out.splitlines())
                if not ended and not any("CALIBRATION FAIL" in l for l in rec["checks"]):
                    rec["verdict"] = "fail"; rec["note"] = "the session did not reach its end (rc %d)" % rc
                else:
                    rec["verdict"] = "fail" if fails else "pass"
                    # `known: T-NNN word`: failures all holding `word` are that open defect's, reported and not new
                    if fails and t["known"]:
                        pairs = [k.strip().partition(" ") for k in t["known"].split(";")]
                        if all(any(w and w in l for _, _, w in pairs) for l in fails):
                            kids = ", ".join(k for k, _, w in pairs if any(w and w in l for l in fails))
                            rec["verdict"] = "known"; rec["note"] = f"the open defect {kids}: " + " / ".join(x[:90] for x in fails)
                open(os.path.join(round_dir, f"finger-{shell}-{t['id']}.txt"), "w").write(out)
                if o.make_refs:
                    for l in out.splitlines():
                        m3 = re.match(r"SIG (\S+) ([0-9a-f]{1152})$", l.strip())
                        if m3 and not os.path.exists(os.path.join(REFS, m3.group(1) + ".sig")):
                            os.makedirs(REFS, exist_ok=True); open(os.path.join(REFS, m3.group(1) + ".sig"), "w").write(m3.group(2) + "\n"); print(f"  reference stored: {m3.group(1)}")
            rc, out = devsh(open(os.path.join(HERE, "daemon-stop.sh")).read(), [shell], 120, tmp)
            for fx in t["fixture"]:
                subprocess.run([os.path.join(HERE, "suite", "fixtures", fx + ".sh"), "down", shell], capture_output=True, text=True, errors="replace")
            rec["restore"] = next((l for l in reversed(out.strip().splitlines()) if "daemons running" in l), out.strip().splitlines()[-1] if out.strip() else "(no answer)")
            rec["seconds"] = round(time.time() - t0)
            print(f"[{shell}] {t['id']}: {rec['verdict']} ({len(rec['checks'])} lines, {len(rec['kept'])} kept, {rec['seconds']} s) — {rec['restore']}")
    if not o.no_end_check:
        rc, out = devsh(open(os.path.join(HERE, "device-check.sh")).read(), [], 60, tmp)
        report["end_check"] = out.strip()
        print("end-of-suite device check:\n" + "\n".join("  " + l for l in out.strip().splitlines()))
    report["seconds"] = round(time.time() - t_start)
    prev_path = os.path.join(round_dir, "finger-report.json")
    if o.merge and os.path.exists(prev_path):   # a rerun of some runs: the report of the round stays one report, and says what was run again
        prev = json.load(open(prev_path))
        new = {(r["shell"], r["id"]): r for r in report["tests"]}
        for r in report["tests"]: r["rerun"] = report["date"]
        for r in prev["tests"]:
            n = new.get((r["shell"], r["id"]))
            if n is not None:
                for k in r.get("kept", []):
                    if k not in n.get("kept", []) and os.path.exists(os.path.join(round_dir, k)): os.remove(os.path.join(round_dir, k))
        report["tests"] = [new.pop((r["shell"], r["id"]), r) for r in prev["tests"]] + list(new.values())
        report["shells"] = sorted(set(prev.get("shells", [])) | set(shells), key=SHELLS.index)
        report["first_date"] = prev.get("first_date", prev.get("date")); report["seconds"] += prev.get("seconds", 0)
        shells = report["shells"]
    # the report
    md = [f"# Finger suite report — {report['date']}", "", f"Shells: {', '.join(shells)}. Scripts: {len(tests)}. {report['seconds']} s.", "",
          "| shell | test | verdict | checks | evidence | kit |", "|---|---|---|---|---|---|"]
    if report.get("first_date"): md[1:1] = ["", f"First run {report['first_date']}; runs marked (rerun) were run again by `--merge`, and the device-check below is the last run's."]
    for r in report["tests"]:
        ok = sum(1 for l in r["checks"] if l.startswith("CHECK ok") or l.startswith("WAIT ok")); bad = sum(1 for l in r["checks"] if "FAIL" in l)
        ev = r["note"] or (" / ".join(l for l in r["checks"] if "FAIL" in l) if bad else (r["checks"][-1] if r["checks"] else ""))
        kept = ("; frames: " + ", ".join(r["kept"])) if r["kept"] else ""
        md.append(f"| {r['shell']} | {r['id']}{' (rerun)' if r.get('rerun') else ''} | **{r['verdict']}** | {ok} ok, {bad} failed | {ev[:300].replace('|', '/')}{kept} | {r['kit']} |")
    md += ["", "## The device at the end", "", "```", report["end_check"] or "(not checked)", "```", "",
           "## Every test's lines", ""]
    for r in report["tests"]:
        md.append(f"### {r['shell']} / {r['id']} — {r['verdict']}"); md.append("")
        if r.get("start"): md.append("    " + r["start"])
        md += ["    " + l for l in r["checks"]] or ["    " + (r["note"] or "(no lines)")]
        md.append("")
    open(os.path.join(round_dir, "finger-report.md"), "w").write("\n".join(md) + "\n")
    json.dump(report, open(os.path.join(round_dir, "finger-report.json"), "w"), indent=1)
    shutil.rmtree(tmp, ignore_errors=True)
    n_fail = sum(1 for r in report["tests"] if r["verdict"] == "fail")
    print(f"report: {os.path.join(round_dir, 'finger-report.md')} — {len(report['tests'])} runs, {n_fail} failed")
    sys.exit(1 if n_fail or (report["end_check"] and "DEVICE FAIL" in report["end_check"]) else 0)

if __name__ == "__main__":
    main()
