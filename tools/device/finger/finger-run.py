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
#   wait rect SEL [BOUND] | wait line PATTERN [BOUND] | wait still REGION [BOUND]
#   calibrate SEL [TOL]                 the page-top calibration tap (the round stops on a disagreement)
#   tap SEL | tap-at X Y | chrome NAME | drag X0 Y0 X1 Y1 [MS] | flick X0 Y0 X1 Y1 [MS] | pinch CX CY D0 D1 [MS]
#   hold X Y [MS] | dtap X Y | pick SEL INDEX [ROWS] | card-to Y | pull-header | key CODE | type TEXT | inject CMD...
#   expect CHECK ARGS...                fl_check_CHECK: scroll EY KY TOL | card-scroll KY TOL | zoom Z TOL | hit SEL |
#                                       element-at SEL X Y | text SEL TEXT | text-has SEL TEXT | url PART |
#                                       dialog present|answered [BOUND] | keyboard up|down [BOUND] |
#                                       content REGION SEL [TEXT] | region-equal REGION [SAMPLE] |
#                                       region-changed REGION [SAMPLE] | top SEL [TOL]
#   sample NAME REGION                  a region's hash remembered for region-equal / region-changed
#   shoot NAME                          the shown page kept whatever the verdict (for a picture a kit note cites)
#   A gesture with no mark of its own (chrome, hold, dtap, pinch) needs a `wait` before the next `expect`; `sleep`
#   is not a verb of the form. The lint says so.
import sys, os, re, subprocess, json, time, datetime, shutil, tempfile, argparse

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
DEVSH = os.path.join(ROOT, "build", "webos-oe", "dev-sh.sh")
SHELLS = ["enyo", "mochi", "mojo"]
GESTURES = {"tap", "tap-at", "chrome", "drag", "flick", "pinch", "hold", "dtap", "pick", "card-to", "pull-header", "key", "type", "inject", "launch", "calibrate"}
UNMARKED = {"chrome", "hold", "dtap", "pinch"}       # no mark of their own: a wait must follow before an expect
WAITING_EXPECTS = {"keyboard", "dialog"}            # these expects wait with a bound themselves
CHECKS = {"scroll": "fl_check_scroll", "card-scroll": "fl_check_card_scroll", "zoom": "fl_check_zoom", "hit": "fl_check_hit",
          "element-at": "fl_check_element_at", "text": "fl_check_text", "text-has": "fl_check_text_has", "url": "fl_check_url",
          "dialog": "fl_check_dialog", "keyboard": "fl_check_keyboard", "content": "fl_check_content",
          "region-equal": "fl_check_region_equal", "region-changed": "fl_check_region_changed", "top": "fl_check_top"}

def sh_quote(s):
    return "'" + s.replace("'", "'\\''") + "'"

def split_args(rest):
    # arguments separated by spaces; a selector with spaces is quoted "…" or '…' (a \" inside "…" is a quote)
    out, cur, q, esc = [], "", None, False
    for c in rest:
        if esc: cur += c; esc = False
        elif q:
            if c == "\\": esc = True
            elif c == q: q = None
            else: cur += c
        elif c in "\"'": q = c
        elif c == " ":
            if cur: out.append(cur); cur = ""
        else: cur += c
    if cur: out.append(cur)
    return out
ARITY = {"calibrate": (1, 2), "tap": (1, 1), "tap-at": (2, 2), "chrome": (1, 1), "pick": (2, 3), "card-to": (1, 1), "sample": (2, 2), "shoot": (1, 1),
         "drag": (4, 5), "flick": (4, 5), "pinch": (4, 5), "hold": (2, 3), "dtap": (2, 4), "key": (1, 1), "launch": (1, 2)}
EXPECT_ARITY = {"scroll": (3, 3), "card-scroll": (2, 2), "zoom": (2, 2), "hit": (1, 1), "element-at": (3, 3), "text": (2, 2), "text-has": (2, 2),
                "url": (1, 1), "dialog": (1, 2), "keyboard": (1, 2), "content": (2, 3), "region-equal": (1, 2), "region-changed": (1, 2), "top": (1, 2)}

def parse(path):
    t = {"id": os.path.basename(path)[:-5], "shells": SHELLS[:], "page": "about:blank", "prefs": [], "kit": "", "person": "", "steps": [], "path": path}
    for n, raw in enumerate(open(path), 1):
        line = raw.strip()
        if not line or line.startswith("#"): continue
        m = re.match(r"^(id|shells|page|prefs|kit|person):\s*(.*)$", line)
        if m:
            k, v = m.group(1), m.group(2).strip()
            if k == "shells": t[k] = v.split()
            elif k == "prefs": t[k] = v.split()
            else: t[k] = v
            continue
        verb, rest = (line.split(None, 1) + [""])[:2]
        t["steps"].append((n, verb, rest))
    return t

def lint(tests):
    errs = []
    for t in tests:
        pending = None       # an unmarked gesture waiting for a wait
        for n, verb, rest in t["steps"]:
            a = split_args(rest)
            if verb in ("sleep", "usleep"):
                errs.append(f"{t['id']}:{n}: `{verb}` is not a verb of the form (wait on a mark)")
            elif verb == "wait":
                if not a or a[0] not in ("load", "painted", "zoom", "settled", "rect", "line", "still"):
                    errs.append(f"{t['id']}:{n}: wait needs a mark: load|painted|zoom|settled|rect SEL|line PATTERN|still REGION")
                pending = None
            elif verb in GESTURES:
                if verb in UNMARKED: pending = (n, verb)
                else: pending = None
            elif verb == "expect":
                if not a or a[0] not in CHECKS:
                    errs.append(f"{t['id']}:{n}: expect needs a check: {', '.join(sorted(CHECKS))}")
                elif pending and a[0] not in WAITING_EXPECTS:
                    errs.append(f"{t['id']}:{n}: `expect {a[0]}` after `{pending[1]}` (line {pending[0]}) with no wait between: name the mark")
            elif verb in ("sample", "shoot"):
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
        if not t["person"] and not any(v == "expect" or (v == "wait") for _, v, _ in t["steps"]):
            errs.append(f"{t['id']}: no expect and no wait: nothing is checked")
    return errs

def compile_steps(t):
    out = []
    for n, verb, rest in t["steps"]:
        a = split_args(rest); q = [sh_quote(x) for x in a]; name = f"{n}-{verb}"
        if verb == "launch":
            out.append(f'fl_load {q[0]} {a[1] if len(a) > 1 else 40} && echo "WAIT ok {n}-launch: load done" || fl_check FAIL "{n}-launch" "no load done for {a[0]}"')
        elif verb == "wait":
            mark = a[0]; b = a[-1] if len(a) > 1 and a[-1].isdigit() else ""
            if mark in ("load", "painted", "zoom"):
                out.append(f'fl_wait_{mark} {b or ""} >/dev/null && echo "WAIT ok {n}-{mark}" || fl_check FAIL "{n}-wait-{mark}" "no {mark} mark within the bound"')
            elif mark == "settled":
                out.append(f'fl_wait_settled {b or 5} && echo "WAIT ok {n}-settled: the card\'s scroll $(fl_card_scroll)" || fl_check FAIL "{n}-wait-settled" "the card\'s scroll did not settle"')
            elif mark == "rect":
                out.append(f'fl_wait_rect {q[1]} {b or 5} && echo "WAIT ok {n}-rect {a[1]}" || fl_check FAIL "{n}-wait-rect" "no element {a[1]} within the bound"')
            elif mark == "line":
                out.append(f'l=$(fl_wait {q[1]} {b or 5}) && echo "WAIT ok {n}-line: $l" || fl_check FAIL "{n}-wait-line" "no line {a[1]} within the bound"')
            elif mark == "still":
                out.append(f'fl_wait_still {q[1]} {b or 4} && echo "WAIT ok {n}-still {a[1]}" || fl_check FAIL "{n}-wait-still" "the region {a[1]} never held still"')
        elif verb == "calibrate": out.append(f"fl_calibrate {' '.join(q)}")
        elif verb == "tap": out.append(f"fl_tap {q[0]} || fl_check FAIL \"{name}\" \"the tap on {a[0]} reached no clickAt\"")
        elif verb == "tap-at": out.append(f"fl_tap_at {q[0]} {q[1]} || fl_check FAIL \"{name}\" \"the tap at {a[0]},{a[1]} reached no clickAt\"")
        elif verb == "chrome": out.append(f"fl_chrome_tap {q[0]} || fl_check FAIL \"{name}\" \"no chrome entry {a[0]}\"")
        elif verb == "drag": out.append(f"fl_drag {' '.join(q)}")
        elif verb == "flick": out.append(f"fl_flick {' '.join(q)}")
        elif verb == "pinch": out.append(f"/tmp/finger pinch {' '.join(q)}; echo \"  pinch {rest}\"")
        elif verb == "hold": out.append(f"/tmp/finger hold {' '.join(q)}; echo \"  hold {rest}\"")
        elif verb == "dtap": out.append(f"/tmp/finger dtap {' '.join(q)}; echo \"  double tap {rest}\"")
        elif verb == "pick": out.append(f"fl_pick {' '.join(q)} || fl_check FAIL \"{name}\" \"the pick on {a[0]} did not open or close\"")
        elif verb == "card-to": out.append(f"fl_card_to {q[0]}")
        elif verb == "pull-header": out.append("fl_pull_header")
        elif verb == "key": out.append(f"fl_ask \"key {a[0]}\" 'inject key ' 3 >/dev/null || fl_check FAIL \"{name}\" \"the key {a[0]} was not echoed\"")
        elif verb == "type": out.append(f"fl_ask \"text {rest}\" 'inject text ' 3 >/dev/null || fl_check FAIL \"{name}\" \"the text was not echoed\"")
        elif verb == "inject": out.append(f"fl_mark; fl_inject {sh_quote(rest)}; echo \"  inject {rest}\"")
        elif verb == "expect":
            out.append(f"{CHECKS[a[0]]} \"{n}-{a[0]}\" {' '.join(q[1:])}")
        elif verb == "sample": out.append(f"fl_sample {' '.join(q)}")
        elif verb == "shoot": out.append(f"fl_keep {q[0]}")
    return "\n".join(out) + "\n"

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
    ap.add_argument("--no-end-check", action="store_true")
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
    report = {"date": datetime.datetime.now().isoformat(timespec="seconds"), "shells": shells, "tests": [], "end_check": ""}
    t_start = time.time()
    for shell in shells:
        for t in tests:
            if shell not in t["shells"]: continue
            rec = {"shell": shell, "id": t["id"], "kit": t["kit"], "verdict": "", "checks": [], "kept": [], "seconds": 0, "note": ""}
            report["tests"].append(rec)
            if t["person"]:
                rec["verdict"] = "a person's"; rec["note"] = t["person"]; print(f"[{shell}] {t['id']}: a person's — {t['person']}"); continue
            t0 = time.time()
            print(f"[{shell}] {t['id']}: the daemon on {t['page']}")
            rc, out = devsh(open(os.path.join(HERE, "daemon-start.sh")).read(), [shell, t["page"]] + ["PREF=" + p for p in t["prefs"]], 150, tmp)
            start_line = [l for l in out.splitlines() if "content frames" in l or "FATAL" in l]
            rec["start"] = start_line[-1][:200] if start_line else out.strip()[-200:]
            if "FATAL" in out or not start_line:
                rec["verdict"] = "fail"; rec["note"] = "the daemon did not start: " + out.strip().splitlines()[-1][:160]
            else:
                body = "chmod +x /tmp/fbstream /tmp/finger 2>/dev/null\n" + lib + f"fl_init {shell}\n" + compile_steps(t) + "fl_summary\n"
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
                fails = [l for l in rec["checks"] if l.startswith("CHECK FAIL") or l.startswith("CALIBRATION FAIL")]
                ended = any(l.startswith("CHECKS ") for l in out.splitlines())
                if not ended and not any("CALIBRATION FAIL" in l for l in rec["checks"]):
                    rec["verdict"] = "fail"; rec["note"] = "the session did not reach its end (rc %d)" % rc
                else:
                    rec["verdict"] = "fail" if fails else "pass"
                open(os.path.join(round_dir, f"finger-{shell}-{t['id']}.txt"), "w").write(out)
            rc, out = devsh(open(os.path.join(HERE, "daemon-stop.sh")).read(), [shell], 120, tmp)
            rec["restore"] = out.strip().splitlines()[-1] if out.strip() else "(no answer)"
            rec["seconds"] = round(time.time() - t0)
            print(f"[{shell}] {t['id']}: {rec['verdict']} ({len(rec['checks'])} lines, {len(rec['kept'])} kept, {rec['seconds']} s) — {rec['restore']}")
    if not o.no_end_check:
        rc, out = devsh(open(os.path.join(HERE, "device-check.sh")).read(), [], 60, tmp)
        report["end_check"] = out.strip()
        print("end-of-suite device check:\n" + "\n".join("  " + l for l in out.strip().splitlines()))
    report["seconds"] = round(time.time() - t_start)
    # the report
    md = [f"# Finger suite report — {report['date']}", "", f"Shells: {', '.join(shells)}. Scripts: {len(tests)}. {report['seconds']} s.", "",
          "| shell | test | verdict | checks | evidence | kit |", "|---|---|---|---|---|---|"]
    for r in report["tests"]:
        ok = sum(1 for l in r["checks"] if l.startswith("CHECK ok") or l.startswith("WAIT ok")); bad = sum(1 for l in r["checks"] if "FAIL" in l)
        ev = r["note"] or (" / ".join(l for l in r["checks"] if "FAIL" in l) if bad else (r["checks"][-1] if r["checks"] else ""))
        kept = ("; frames: " + ", ".join(r["kept"])) if r["kept"] else ""
        md.append(f"| {r['shell']} | {r['id']} | **{r['verdict']}** | {ok} ok, {bad} failed | {ev[:300].replace('|', '/')}{kept} | {r['kit']} |")
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
