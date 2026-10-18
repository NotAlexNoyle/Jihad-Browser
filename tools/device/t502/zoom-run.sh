#!/bin/bash
# zoom-run.sh <name> <url> "<env words>" "<zoom in>" "<zoom out>": T-504 (cavekit-gpu-rendering R6).
# The spike start with the inject channel, then after PAN_DELAY seconds the card's own zoom command
# as the daemon receives it after a pinch ("zoom <z> <x> <y>" = setZoomAndScroll), screenshots 1 s
# and 5 s after it, the zoom back, two more screenshots, and the timeline: each zoom command and the
# frames after it (zoom, the paint pipeline's ms), the tile uploads between, the displayport lines.
# e.g. zoom-run.sh wiki https://en.m.wikipedia.org/wiki/HP_TouchPad "" "zoom 1.6918 504 590" "zoom 0.6882 0 0"
set -u
S=$(dirname "$0"); R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser; D=$R/docs/rounds/t502-2026-10-17
name=$1; url=$2; extra=${3:-}; zin=${4:-zoom 1.6918 504 590}; zout=${5:-zoom 0.6882 0 0}
cd $R; . build/webos-oe/host-path.sh
echo "=== zoom $name"
build/webos-oe/dev-sh.sh -t 90 $S/gpu-spike-start.sh "$url" "JIHAD_INJECT=1 $extra" 2>&1 | grep 'screenshot at\|FATAL' | tail -n 2
cat > $S/zoom-inject.sh <<'EOS'
STATE=/var/palm/jihad/enyo
sleep PANDELAY
: > $STATE/daemon-gpu.pan-mark
shot() { luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot "{\"file\":\"/tmp/st-zoom-$1.png\"}" >/dev/null 2>&1; }
umask 077
echo "ZOOMIN" > $STATE/inject.cmd; sleep 1; shot in1; sleep 4; shot in5
echo "ZOOMOUT" > $STATE/inject.cmd; sleep 1; shot out1; sleep 4; shot out5
sleep 2
EOS
sed -i "s|ZOOMIN|$zin|; s|ZOOMOUT|$zout|; s|PANDELAY|${PAN_DELAY:-0}|" $S/zoom-inject.sh
build/webos-oe/dev-sh.sh -t $((60 + ${PAN_DELAY:-0})) $S/zoom-inject.sh
for s in in1 in5 out1 out5; do
  novacom get file:///tmp/st-zoom-$s.png > $D/zoom-$name-$s.png 2>/dev/null
  python3 -c "
from PIL import Image
Image.open('$D/zoom-$name-$s.png').rotate(-90, expand=True).convert('RGB').resize((384,512)).save('$D/zoom-$name-$s.jpg', quality=85)" 2>/dev/null && echo "screenshot $s: $D/zoom-$name-$s.jpg"
done
build/webos-oe/dev-sh.sh -t 40 $S/gpu-spike-collect.sh 2>/dev/null | sed -n '/=== LOG BEGIN/,/=== LOG END/p' | grep -v 'Fontconfig warning' > $D/zoom-$name-daemon-gpu.log
python3 - "$D/zoom-$name-daemon-gpu.log" <<'PYEOF'
import re, sys
lines = open(sys.argv[1], encoding="latin1").read().splitlines()
zi = [i for i, l in enumerate(lines) if "setZoomAndScroll" in l and "(was -1" not in l]
for k, i in enumerate(zi):
    end = zi[k + 1] if k + 1 < len(zi) else len(lines)
    seg = lines[i:end]
    z = re.search(r"zoom=([0-9.]+)", lines[i]).group(1)
    painted = [l for l in seg if "painted shmid" in l]
    at = [l for l in painted if "mZoom=" + z in l]
    ups = [float(m.group(1)) for l in seg for m in [re.search(r"upload call \S+ .*: ([0-9.]+) ms", l)] if m]
    print("--- " + lines[i].strip()[:110])
    print("    frames after it: %d (at the new zoom: %d); uploads: %d, %.0f ms in all" % (len(painted), len(at), len(ups), sum(ups)))
    for l in painted[:6]:
        m = re.search(r"mZoom=([0-9.]+).*ms=(\d+)", l)
        if m: print("    frame zoom %s, %s ms" % (m.group(1), m.group(2)))
    for l in [l for l in seg if "displayport:" in l or "viewport: scale" in l][:3]:
        print("    " + l.strip()[:150])
PYEOF
build/webos-oe/dev-sh.sh -t 60 $S/gpu-spike-collect-stop.sh > /dev/null 2>&1
