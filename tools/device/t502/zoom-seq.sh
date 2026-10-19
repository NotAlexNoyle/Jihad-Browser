#!/bin/bash
# zoom-seq.sh <name> <url> "<env words>" [count]: T-515 (cavekit-gpu-rendering R6). The spike start
# with the inject channel and JIHAD_GPU_ZOOM_DUMP=log (the zoom timeline without frame files), then
# <count> zooms (default 8) through the card's path, in and out in turn, 2 s apart, each sent
# 0, 150, 300, 450 or 600 ms after the daemon reads it (the inject file is read only when the main
# thread is free: the delay lands the zoom at every point of a page script's hold, as a card's
# pinch does). Then, per zoom, the first frame at it from the zoom's arrival, whichever lane made
# it (the express lane's "N ms after the zoom", the main lane's "N ms after it arrived"). Needs
# JIHAD_GPU_EXPRESS=1 in the env words (the card's path is the express lane's reader thread).
set -u
S=$(dirname "$0"); R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser; D=$R/docs/rounds/t502-2026-10-17
name=$1; url=$2; extra=${3:-}; count=${4:-8}
cd $R; . build/webos-oe/host-path.sh
echo "=== zoom sequence $name"
build/webos-oe/dev-sh.sh -t 90 $S/gpu-spike-start.sh "$url" "JIHAD_INJECT=1 JIHAD_GPU_ZOOM_DUMP=log $extra" 2>&1 | grep 'screenshot at\|FATAL' | tail -n 2
{
  echo 'STATE=/var/palm/jihad/enyo'
  echo "sleep ${PAN_DELAY:-0}"
  echo 'umask 077'
  for i in $(seq 0 $((count - 1))); do
    d=$(( (i % 5) * 150 ))
    if [ $((i % 2)) -eq 0 ]; then z="zoom 1.6918 504 590 $d"; else z="zoom 0.6882 0 0 $d"; fi
    echo "echo '$z' > \$STATE/inject.cmd; sleep 2"
  done
  echo 'sleep 2'
} > $S/zoom-seq-inject.sh
build/webos-oe/dev-sh.sh -t $((60 + ${PAN_DELAY:-0} + count * 2)) $S/zoom-seq-inject.sh
build/webos-oe/dev-sh.sh -t 40 $S/gpu-spike-collect.sh 2>/dev/null | sed -n '/=== LOG BEGIN/,/=== LOG END/p' | grep -v 'Fontconfig warning' > $D/zoomseq-$name-daemon-gpu.log
python3 - "$D/zoomseq-$name-daemon-gpu.log" <<'PYEOF'
import re, sys
lines = open(sys.argv[1], encoding="latin1").read().splitlines()
res = []
for i, l in enumerate(lines):
    m = re.search(r"inject zoom ([0-9.]+) at .* in (\d+) ms", l)
    if not m:
        continue
    z, d = m.group(1), m.group(2)
    got = None
    for l2 in lines[i + 1:]:
        if "inject zoom" in l2:
            break
        if "zoom frame written" in l2 and ("at zoom " + z) in l2:
            mx = re.search(r"\(express\).* (\d+) ms after the zoom", l2)
            mm = re.search(r"(\d+) ms after it arrived", l2)
            if mx: got = ("express", int(mx.group(1)))
            elif mm: got = ("main", int(mm.group(1)))
            if got: break
    res.append((z, d, got))
for z, d, got in res:
    print("zoom %s sent %s ms in: %s" % (z, d, "%s lane, %d ms after it arrived" % got if got else "no frame at it logged"))
ms = [g[1] for _, _, g in res if g]
if ms:
    print("first frames: %d of %d, max %d ms, median %d ms" % (len(ms), len(res), max(ms), sorted(ms)[len(ms) // 2]))
PYEOF
