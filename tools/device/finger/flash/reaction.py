#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# reaction.py -- tap to first changed frame inside a movie, in a browser (cavekit-flash.md R5, T-575): the movie that changes when
# pressed, once a load, each tap against the pictures just before it, by the frames. The stock browser needs stock-bench.sh up first.
#   reaction.py stock|enyo|mochi|mojo [N]    prints the milliseconds and the median
# Run with python3 -I.
import json, os, re, statistics, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(HERE))))
DEVSH = os.path.join(ROOT, "build", "webos-oe", "dev-sh.sh"); F = os.path.join(ROOT, "tools", "device", "finger")
APPS = {"stock": ("net.riverstonerelay.stockbench", 82), "enyo": ("net.riverstonerelay.jihad-browser", 82),
        "mochi": ("net.riverstonerelay.jihad-browser-mochi", 78), "mojo": ("net.riverstonerelay.jihad-browser-mojo", 73)}
URL = "http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-reacts.swf"


def main():
    who = sys.argv[1]; n = int(sys.argv[2]) if len(sys.argv) > 2 else 8
    app, top = APPS[who]
    # the page's movie stands at 40,40, 400x300
    r = "40,%d,440,%d" % (top + 42, top + 338); x, y = 240, top + 190
    body = f"""
close_all() {{ for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{{}}' 2>/dev/null | tr ',' '\\n' | grep -A1 '"{app}"' | grep processid | sed 's/.*"processid": *"\\([^"]*\\)".*/\\1/'); do luna-send -n 1 palm://com.palm.applicationManager/close '{{"processId":"'"$pid"'"}}' >/dev/null 2>&1; done; }}
i=0; while [ $i -lt {n} ]; do
  close_all; usleep 500000
  luna-send -n 1 palm://com.palm.applicationManager/launch '{{"id":"{app}","params":{{"target":"{URL}"}}}}' >/dev/null
  sleep 12
  /tmp/fbstream /tmp/fl-rc -C {r} -s 1 -P 2000 >/dev/null 2>&1
  /tmp/fbstream /tmp/fl-ra -C {r} -s 2 -P 2000 -B /tmp/fl-rc > /tmp/fl-ra.out 2>&1 &
  usleep 500000; /tmp/finger tap {x} {y}; wait
  echo "TRIAL $(cat /tmp/fl-ra.out)"
  i=$((i+1))
done
close_all; rm -f /tmp/fl-rc /tmp/fl-ra /tmp/fl-ra.out
"""
    p = os.path.join(tempfile.mkdtemp(), "b.sh"); open(p, "w").write(body)
    out = subprocess.run([DEVSH, "-t", str(40 + n * 22), p], capture_output=True, text=True, errors="replace").stdout
    ms = [max(0, int(m) - 500) for m in re.findall(r"first (\d+) ms", out)]
    print(who, "ms:", ms, "median:", statistics.median(ms) if ms else None, "of", len(re.findall(r"^TRIAL", out, re.M)))


if __name__ == "__main__":
    main()
