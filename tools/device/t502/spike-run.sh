#!/bin/bash
# spike-run.sh <name> <url> [extra env words]: one device run of the hand-started Enyo daemon on
# the compositor path, with the frame dump checked for real content, the screenshot and the
# daemon log saved under docs/rounds/t502-2026-10-17/ as spike-<name>-*.
set -u
S=$(dirname "$0"); R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser; D=$R/docs/rounds/t502-2026-10-17
name=$1; url=$2; extra=${3:-}
cd $R; . build/webos-oe/host-path.sh
echo "=== $name"
build/webos-oe/dev-sh.sh -t 90 $S/gpu-spike-start.sh "$url" "$extra" 2>&1 | grep -v "can't open /proc" | grep 'compositor thread.*frame 1[0-9],\|paint pipeline\|FATAL\|execute\|probe:\|screenshot at\|seq frame' | tail -n 8
novacom get file:///tmp/frame.raw > $S/frame-$name.raw 2>/dev/null
python3 $S/check-frame.py $S/frame-$name.raw 768 942 $D/spike-$name-frame.jpg
novacom get file:///tmp/st-gpu.png > $D/spike-$name.png 2>/dev/null
novacom get file:///tmp/frames.raw > $S/frames-$name.raw 2>/dev/null
python3 $S/check-seq.py $S/frames-$name.raw 768 942 $D/spike-$name-seq.jpg
python3 -c "
from PIL import Image
Image.open('$D/spike-$name.png').rotate(-90, expand=True).convert('RGB').resize((576,768)).save('$D/spike-$name.jpg', quality=85)"
build/webos-oe/dev-sh.sh -t 40 $S/gpu-spike-collect.sh 2>/dev/null | sed -n '/=== LOG BEGIN/,/=== LOG END/p' | grep -v 'Fontconfig warning' > $D/spike-$name-daemon-gpu.log
python3 - "$D/spike-$name-daemon-gpu.log" <<'PY'
import re, sys
L = open(sys.argv[1]).read()
def st(a): a=sorted(a); return "n=%d min=%.1f med=%.1f p90=%.1f max=%.1f" % (len(a), a[0], a[len(a)//2], a[int(len(a)*0.9)], a[-1]) if a else "none"
print("  compose", st([float(m) for m in re.findall(r"compositor thread: compose ([0-9.]+) ms", L)]))
print("  read   ", st([float(m) for m in re.findall(r"ms, read ([0-9.]+) ms", L)]))
print("  layers ", st([float(m) for m in re.findall(r"layers ([0-9.]+) ms", L)]))
print("  fps    ", " ".join(re.findall(r"in 20[0-9][0-9]ms \(([0-9.]+) fps\)", L)))
print("  stages ", re.findall(r"daemon frame: (.*)", L)[:3])
PY
