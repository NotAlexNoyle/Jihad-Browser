#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# video-round.py -- each clip of the codec and size table on the instrumented player, in a browser (cavekit-flash.md R8 and R3, T-578):
# the picture drawn, the content rate over ten seconds of play, the shown frames' spacing, and a kept picture in which the page's own
# report lines (the stream's time, its dropped frames, the player's rate) stand beside the movie. It is the one reading the stock
# browser can give too (its page cannot be asked), so it is what R3's bar for a clip is made of; the finger suite's flash-video-*
# scripts read the reports themselves in the three shells. HOST side, one device session a clip; flash-serve.py on 8097 and the test
# pages on 8077 running; flash-video-clips.sh run once (the clips are made into the cache).
#   video-round.py stock|enyo|mochi|mojo DIR [--only id,id]      DIR/<browser>-video.json, DIR/<browser>-<clip>.jpg
# Run with python3 -I.
import argparse, json, os, re, subprocess, tempfile, time
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(HERE))))
DEVSH = os.path.join(ROOT, "build", "webos-oe", "dev-sh.sh")
APPS = {"stock": ("net.riverstonerelay.stockbench", 82), "enyo": ("net.riverstonerelay.jihad-browser", 82),
        "mochi": ("net.riverstonerelay.jihad-browser-mochi", 78), "mojo": ("net.riverstonerelay.jihad-browser-mojo", 73)}
PAGE = "http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/"
# the clip, its codec, its rows, its authored rate
CLIPS = [("gen-spark-240.flv", "spark", 240, 24), ("gen-spark-360.flv", "spark", 360, 24), ("gen-spark-480.flv", "spark", 480, 24),
         ("gen-h264-240.flv", "h264", 240, 24), ("gen-h264-360.flv", "h264", 360, 24), ("gen-h264-480.flv", "h264", 480, 24),
         ("mrandmrs-vp6.flv", "vp6", 176, 24), ("harrypotter-vp6.flv", "vp6", 272, 24), ("i004-vp6a.flv", "vp6a", 430, 15),
         ("zelda-spark.flv", "spark", 120, 12), ("sheep-spark.flv", "spark", 480, 25), ("bbb-h264-360.mp4", "h264 (mp4)", 360, 30)]


def script(clip, app, top, secs):
    R = "40,%d,440,%d" % (top + 40, top + 340)
    return f"""
close_all() {{ for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{{}}' 2>/dev/null | tr ',' '\\n' | grep -A1 '"{app}"' | grep processid | sed 's/.*"processid": *"\\([^"]*\\)".*/\\1/'); do luna-send -n 1 palm://com.palm.applicationManager/close '{{"processId":"'"$pid"'"}}' >/dev/null 2>&1; done; }}
close_all; usleep 500000
luna-send -n 1 palm://com.palm.applicationManager/launch '{{"id":"{app}","params":{{"target":"{PAGE}{clip}"}}}}' | grep -q true || echo "LAUNCH FAIL"
i=0; while [ $i -lt 30 ]; do c=$(/tmp/fbstream /tmp/fl-x -Q {R} 2>/dev/null | sed -n 's/.*colours \\([0-9]*\\).*/\\1/p'); [ "${{c:-0}}" -ge 8 ] && break; sleep 1; i=$((i+1)); done; echo "FIRST $i"
sleep 1
echo "RATE $(/tmp/fbstream /tmp/fl-rate -C {R} -s {secs} -P 2000 2>/dev/null)"
echo "DRAWN $(/tmp/fbstream /tmp/fl-x -Q {R} 2>/dev/null)"
luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{{"file":"/tmp/fl-video.png"}}' >/dev/null 2>&1; usleep 700000
close_all; rm -f /tmp/fl-x /tmp/fl-rate
"""


def main():
    ap = argparse.ArgumentParser(); ap.add_argument("browser", choices=list(APPS)); ap.add_argument("dir"); ap.add_argument("--only", default=""); o = ap.parse_args()
    app, top = APPS[o.browser]; os.makedirs(o.dir, exist_ok=True); only = set(x for x in o.only.split(",") if x); tmp = tempfile.mkdtemp()
    out_json = os.path.join(o.dir, o.browser + "-video.json"); res = json.load(open(out_json)) if os.path.exists(out_json) else {}
    for clip, codec, rows, fps in CLIPS:
        cid = os.path.splitext(clip)[0]
        if only and cid not in only: continue
        secs = 6 if cid in ("sheep-spark", "bbb-h264-360") else 10          # the short clips: measured while they play
        p = os.path.join(tmp, "s.sh"); open(p, "w").write(script(clip, app, top, secs))
        r0 = subprocess.run([DEVSH, "-t", "90", p], capture_output=True, text=True, errors="replace"); out = r0.stdout + r0.stderr
        r = {"codec": codec, "rows": rows, "fps": fps, "date": time.strftime("%Y-%m-%d")}
        m = re.search(r"FIRST (\d+)", out); r["first_s"] = int(m.group(1)) if m else -1
        m = re.search(r"DRAWN .*colours (\d+)", out); r["colours"] = int(m.group(1)) if m else 0
        m = re.search(r"RATE .*changes \d+ \(([\d.]+)/s\), reads \d+, distinct (\d+)(?:, spacing median (\d+) p90 (\d+) max (\d+) ms)?", out)
        r["content"] = float(m.group(1)) if m else 0.0
        if m and m.group(3): r["spacing"] = [int(m.group(3)), int(m.group(4)), int(m.group(5))]
        png = os.path.join(tmp, "v.png")
        g = subprocess.run("novacom get file:///tmp/fl-video.png > %s" % png, shell=True, capture_output=True)
        subprocess.run(["novacom", "run", "file://bin/rm", "--", "-f", "/tmp/fl-video.png"], stdin=subprocess.DEVNULL, capture_output=True)
        try: Image.open(png).convert("RGB").rotate(-90, expand=True).save(os.path.join(o.dir, "%s-%s.jpg" % (o.browser, cid)), quality=80)
        except Exception as e: r["shot"] = "none: %s" % e
        res[cid] = r; json.dump(res, open(out_json, "w"), indent=1); open(out_json, "a").write("\n")
        print("%-18s %-10s %3d rows  first %2d s  colours %4d  content %5.1f/s of %d  spacing %s" % (cid, codec, rows, r["first_s"], r["colours"], r["content"], fps, r.get("spacing")), flush=True)


if __name__ == "__main__":
    main()
