#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# title-shots.py DIR [enyo] [--only id,id] -- each game's first screen, and the screen after a tap on the movie's middle, as pictures: what
# a script for it has to start from (cavekit-flash.md R6, T-576). HOST side; flash-serve.py running. Run with python3 -I.
import argparse, json, os, subprocess, tempfile
from PIL import Image
HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(HERE))))
DEVSH = os.path.join(ROOT, "build", "webos-oe", "dev-sh.sh"); MANIFEST = os.path.join(ROOT, "tools", "host", "flash-corpus.json")
APP = "net.riverstonerelay.jihad-browser"; HOST = "192.168.1.245:8097"; TOP = 82


def shot(name):
    return f"""luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{{"file":"/tmp/fl-{name}.png"}}' >/dev/null 2>&1; usleep 700000"""


def main():
    ap = argparse.ArgumentParser(); ap.add_argument("dir"); ap.add_argument("--only", default=""); o = ap.parse_args()
    os.makedirs(o.dir, exist_ok=True); m = json.load(open(MANIFEST)); only = set(x for x in o.only.split(",") if x); tmp = tempfile.mkdtemp()
    for t in m["titles"]:
        if t["class"] != "game" or (only and t["id"] not in only): continue
        sw, sh = t.get("width") or 640, t.get("height") or 480; k = min(1.0, 768.0 / sw); w, h = round(sw * k), round(sh * k)
        load = 14 + min(30, t["size"] // 400000)
        p = os.path.join(tmp, "s.sh"); open(p, "w").write(f"""
close_all() {{ for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{{}}' 2>/dev/null | tr ',' '\\n' | grep -A1 '"{APP}"' | grep processid | sed 's/.*"processid": *"\\([^"]*\\)".*/\\1/'); do luna-send -n 1 palm://com.palm.applicationManager/close '{{"processId":"'"$pid"'"}}' >/dev/null 2>&1; done; }}
close_all; usleep 500000
luna-send -n 1 palm://com.palm.applicationManager/launch '{{"id":"{APP}","params":{{"target":"http://{HOST}/title/{t['id']}"}}}}' >/dev/null
sleep {load}; {shot('a')}
/tmp/finger tap {w // 2} {TOP + h // 2}; sleep 5; {shot('b')}
close_all
""")
        subprocess.run([DEVSH, "-t", str(70 + load), p], capture_output=True)
        for k2 in "ab":
            raw = subprocess.run(["novacom", "get", f"file:///tmp/fl-{k2}.png"], capture_output=True).stdout
            q = os.path.join(tmp, "x.png"); open(q, "wb").write(raw)
            subprocess.run(["novacom", "run", "file://bin/rm", "--", "-f", f"/tmp/fl-{k2}.png"], stdin=subprocess.DEVNULL, capture_output=True)
            try:
                im = Image.open(q).convert("RGB").rotate(-90, expand=True).crop((0, TOP, min(768, w), TOP + min(h, 700)))
                im.thumbnail((256, 256)); im.save(os.path.join(o.dir, f"{t['id']}-{k2}.jpg"), quality=60)
            except Exception as e:
                print(t["id"], k2, "no picture", e, flush=True)
        print(t["id"], w, h, flush=True)


if __name__ == "__main__":
    main()
