#!/bin/bash
# [PRE=<n>] zoom-pan.sh <name> <url> <drags> "<env words>": T-504 (cavekit-gpu-rendering R6, the owner's bounded overview).
# The spike daemon (the express lane, the timeline, the overview and zoom-back dumps) on the page, then the emulated
# finger's double tap in, <drags> drags while zoomed in and a double tap out (zoom-pan-device.sh). The daemon log
# goes to $ROUND_DIR; the three screenshots, the overview and the published frame (as PNGs) to $OUT_DIR, to be
# looked at and deleted.
set -u
T=$(cd "$(dirname "$0")" && pwd); R=$(cd "$T/../../.." && pwd); D=${ROUND_DIR:-$R/docs/rounds/t504-2026-10-26}
OUT=${OUT_DIR:-/tmp}
name=$1; url=$2; drags=${3:-1}; extra=${4:-}; pre=${PRE:-0}
mkdir -p "$D"
cd $R; . build/webos-oe/host-path.sh
build/webos-oe/dev-sh.sh -t 90 $T/gpu-spike-start.sh "$url" "JIHAD_INJECT=1 JIHAD_GPU_EXPRESS=1 JIHAD_GPU_TIMELINE=1 JIHAD_GPU_ZOOM_DUMP=log JIHAD_GPU_OVERVIEW_DUMP=/tmp/zp-ov.raw JIHAD_GPU_ZOOM_BACK_DUMP=/tmp/zp-back.raw PREF=jihad.viewport.followOverflow=true $extra" 2>&1 | grep 'screenshot at\|FATAL'
build/webos-oe/dev-sh.sh -t 150 $T/zoom-pan-device.sh $drags $pre > $OUT/zp-$name.out 2>&1
for i in 0 1 2 3; do novacom get file:///tmp/zp-$i.png > $OUT/zp-$name-$i.png 2>/dev/null; done
novacom get file:///tmp/zp-ov.raw > $OUT/zp-$name-ov.raw 2>/dev/null
novacom get file:///tmp/zp-back.raw > $OUT/zp-$name-back.raw 2>/dev/null
novacom run file:///bin/rm -- -f /tmp/zp-0.png /tmp/zp-1.png /tmp/zp-2.png /tmp/zp-3.png /tmp/zp-ov.raw /tmp/zp-back.raw
build/webos-oe/dev-sh.sh -t 60 $T/gpu-spike-collect-stop.sh 2>/dev/null > $OUT/zp-collect.out
sed -n '/=== LOG BEGIN/,/=== LOG END/p' $OUT/zp-collect.out | grep -v 'Fontconfig warning' > $D/zp-$name-daemon-gpu.log
grep -a 'enyo daemons running\|ERROR' $OUT/zp-collect.out
# the raw dumps as PNGs: the overview's size from its log line, the frame the card's (768 wide, B8G8R8A8)
python3 - "$OUT" "$name" "$D/zp-$name-daemon-gpu.log" <<'PY'
import sys, re, os
from PIL import Image
o, n, log = sys.argv[1:4]
txt = open(log, errors='replace').read()
m = re.search(r'overview: dumped (\d+)x(\d+)', txt)
def save(raw, w, out):
    data = open(raw, 'rb').read()
    h = len(data) // (w * 4)
    if h == 0: return print('empty:', raw)
    im = Image.frombuffer('RGBA', (w, h), data[:w * h * 4], 'raw', 'BGRA', 0, 1).convert('RGB')
    im.save(out); print('%s: %dx%d' % (out, w, h))
ov = '%s/zp-%s-ov.raw' % (o, n)
if m and os.path.getsize(ov) > 0: save(ov, int(m.group(1)), '%s/zp-%s-ov.png' % (o, n))
else: print('no overview dump')
bk = '%s/zp-%s-back.raw' % (o, n)
if os.path.getsize(bk) > 0: save(bk, 768, '%s/zp-%s-back.png' % (o, n))
else: print('no zoom-back dump')
for f in (ov, bk):
    if os.path.exists(f): os.remove(f)
PY
cat $OUT/zp-$name.out
