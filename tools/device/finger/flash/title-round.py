#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# title-round.py -- every title of the corpus measured the same way in a browser (cavekit-flash.md R3, T-573): drawn by colours, the
# content rate apart from the composite, the shown frames' spacing, and the reaction to a tap against a no-input control, from the
# frames, in fb1. HOST side, one device session a title.
#
#   title-round.py stock|enyo|mochi|mojo [--only id,id] [--load S] [--write]     the stock browser (stock-bench.sh up first) or a shell
#                                                     on its own job daemon, as shipped; flash-serve.py running
#   --write  the numbers into flash-corpus.json under "stock"
# Run with python3 -I.
import argparse, json, os, re, subprocess, sys, tempfile, time

HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(HERE))))
MANIFEST = os.path.join(ROOT, "tools", "host", "flash-corpus.json"); DEVSH = os.path.join(ROOT, "build", "webos-oe", "dev-sh.sh")
HOST = "192.168.1.245:8097"; CARD_W = 768
APPS = {"stock": ("net.riverstonerelay.stockbench", 82), "enyo": ("net.riverstonerelay.jihad-browser", 82),
        "mochi": ("net.riverstonerelay.jihad-browser-mochi", 78), "mojo": ("net.riverstonerelay.jihad-browser-mojo", 73)}


def rect_of(t, m):
    sw, sh = t.get("width") or 640, t.get("height") or 480
    if t.get("player") and not t.get("height"): sh = 360
    k = min(1.0, CARD_W / float(sw))
    return round(sw * k), round(sh * k)


def script(t, m, app, top, extra=0):
    w, h = rect_of(t, m); R = "0,%d,%d,%d" % (top, w, top + h); cx, cy = w // 2, top + h // 2
    load = extra + (3 if t.get("player") else 12 + min(30, t["size"] // 400000))   # a clip may be ten seconds: measured while it plays
    return f"""
close_all() {{ for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{{}}' 2>/dev/null | tr ',' '\\n' | grep -A1 '"{app}"' | grep processid | sed 's/.*"processid": *"\\([^"]*\\)".*/\\1/'); do luna-send -n 1 palm://com.palm.applicationManager/close '{{"processId":"'"$pid"'"}}' >/dev/null 2>&1; done; }}
close_all; usleep 500000
luna-send -n 1 palm://com.palm.applicationManager/launch '{{"id":"{app}","params":{{"target":"http://{HOST}/title/{t['id']}"}}}}' | grep -q true || echo "LAUNCH FAIL"
sleep {load}
echo "DRAWN $(/tmp/fbstream /tmp/fl-x -Q {R} 2>/dev/null)"
echo "RATE $(/tmp/fbstream /tmp/fl-rate -C {R} -s 8 -P 2000 2>/dev/null)"
/tmp/fbstream /tmp/fl-ctl -C {R} -s 3 -P 2000 >/dev/null 2>&1
/tmp/fbstream /tmp/fl-aft -C {R} -s 3 -P 2000 -B /tmp/fl-ctl > /tmp/fl-aft.out 2>&1 &
usleep 500000; {"" if t.get("no_tap") else f"/tmp/finger tap {cx} {cy};"} wait
echo "REACT $(cat /tmp/fl-aft.out)"
close_all; rm -f /tmp/fl-x /tmp/fl-rate /tmp/fl-ctl /tmp/fl-aft /tmp/fl-aft.out
"""


def parse(out):
    r = {}
    m = re.search(r"DRAWN .*colours (\d+)", out); r["colours"] = int(m.group(1)) if m else 0
    r["drawn"] = r["colours"] >= 8
    m = re.search(r"RATE .*flips \d+ \(([\d.]+)/s\), changes \d+ \(([\d.]+)/s\), reads \d+, distinct (\d+)(?:, spacing median (\d+) p90 (\d+) max (\d+) ms)?", out)
    if m:
        r["composite"] = float(m.group(1)); r["content"] = float(m.group(2)); r["distinct"] = int(m.group(3))
        if m.group(4): r["spacing"] = [int(m.group(4)), int(m.group(5)), int(m.group(6))]
    m = re.search(r"REACT .* new (\d+) of (\d+)(?: first (\d+) ms)?", out)
    if m:
        r["reacted"] = int(m.group(1)) * 2 >= max(1, int(m.group(2)))
        if m.group(3) and r["reacted"]: r["react_ms"] = max(0, int(m.group(3)) - 500)
    return r


def main():
    ap = argparse.ArgumentParser(); ap.add_argument("browser", choices=list(APPS)); ap.add_argument("--only", default=""); ap.add_argument("--write", action="store_true"); ap.add_argument("--load", type=int, default=0, help="seconds more before measuring"); ap.add_argument("--restart", type=int, default=0, help="a fresh daemon every N titles, a shell only: the player dies after some 40 movies in a row (T-580)"); o = ap.parse_args()
    app, top = APPS[o.browser]
    m = json.load(open(MANIFEST)); only = set(x for x in o.only.split(",") if x)
    res = {}; tmp = tempfile.mkdtemp(); count = 0; job = {"enyo": "jihad", "mochi": "jihad-mochi", "mojo": "jihad-mojo"}.get(o.browser)
    for t in m["titles"]:
        if t["class"] == "asset" or (only and t["id"] not in only): continue
        count += 1
        if job and o.restart and count % o.restart == 0:
            q = os.path.join(tmp, "r.sh"); open(q, "w").write("stop %s >/dev/null 2>&1; sleep 2; start %s >/dev/null 2>&1; sleep 6\n" % (job, job))
            subprocess.run([DEVSH, "-t", "40", q], capture_output=True)
        p = os.path.join(tmp, "s.sh"); open(p, "w").write(script(t, m, app, top, o.load))
        t0 = time.time(); r0 = subprocess.run([DEVSH, "-t", str(90 + o.load), p], capture_output=True, text=True, errors="replace"); out = r0.stdout + r0.stderr
        r = parse(out); r["date"] = time.strftime("%Y-%m-%d"); res[t["id"]] = r
        print("%-26s v%-3s %4sfps drawn=%-5s colours %4d content %5.1f composite %5.1f spacing %-16s reacted %-5s %s  (%d s)" % (t["id"], t.get("swf_version", "-"), t.get("fps", "-"), r["drawn"], r["colours"], r.get("content", -1), r.get("composite", -1), r.get("spacing"), r.get("reacted"), r.get("react_ms", ""), time.time() - t0), flush=True)
        if o.write:
            m2 = json.load(open(MANIFEST))
            for x in m2["titles"]:
                if x["id"] == t["id"]: x[o.browser] = r
            json.dump(m2, open(MANIFEST, "w"), indent=1, ensure_ascii=False); open(MANIFEST, "a").write("\n")
    print(json.dumps({"titles": len(res), "drawn": sum(1 for r in res.values() if r["drawn"])}))


if __name__ == "__main__":
    main()
