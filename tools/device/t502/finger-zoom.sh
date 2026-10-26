#!/bin/bash
# finger-zoom.sh <name> <url> "<env words>": T-527 (cavekit-gpu-rendering R6). The spike daemon (the express
# lane, the timeline, JIHAD_GPU_ZOOM_DUMP=log) on the page, then the emulated finger's double tap in and out
# and a pinch in and out (finger-zoom-device.sh; /tmp/finger on the device, tools/device/finger), the six
# screenshots as one contact sheet, the daemon log in $ROUND_DIR. Per gesture the log has the first frame at
# the new zoom ("zoom frame written (express)", the old tiles scaled) and the first frame of the paint at its
# resolution (the express lane publishing the main lane's frame, "publishes main-lane frame").
set -u
T=$(cd "$(dirname "$0")" && pwd); R=$(cd "$T/../../.." && pwd); D=${ROUND_DIR:-$R/docs/rounds/t527-2026-10-25}
OUT=${OUT_DIR:-/tmp}   # the screenshots and the sheet (judged, then deleted)
name=$1; url=$2; extra=${3:-}
cd $R; . build/webos-oe/host-path.sh
build/webos-oe/dev-sh.sh -t 90 $T/gpu-spike-start.sh "$url" "JIHAD_INJECT=1 JIHAD_GPU_EXPRESS=1 JIHAD_GPU_TIMELINE=1 JIHAD_GPU_ZOOM_DUMP=log PREF=jihad.viewport.followOverflow=true $extra" 2>&1 | grep 'screenshot at\|FATAL'
build/webos-oe/dev-sh.sh -t 150 $T/finger-zoom-device.sh > $OUT/fz-$name.out 2>&1
for i in 1 2 3 4 5 6; do novacom get file:///tmp/fz-$i.png > $OUT/fz-$i.png 2>/dev/null; done
build/webos-oe/dev-sh.sh -t 60 $T/gpu-spike-collect-stop.sh 2>/dev/null > $OUT/fz-collect.out
sed -n '/=== LOG BEGIN/,/=== LOG END/p' $OUT/fz-collect.out | grep -v 'Fontconfig warning' > $D/fz-$name-daemon-gpu.log
python3 - "$OUT" "$name" <<'PY'
import sys
from PIL import Image
o, n = sys.argv[1], sys.argv[2]
ims = [Image.open('%s/fz-%d.png' % (o, i)).rotate(-90, expand=True).convert('RGB').resize((384, 512)) for i in range(1, 7)]
W = Image.new('RGB', (384 * 3, 512 * 2), 'white')
for k, im in enumerate(ims):
    W.paste(im, ((k % 3) * 384, (k // 3) * 512))
W.save('%s/fz-sheet-%s.jpg' % (o, n), quality=85)
print('sheet: %s/fz-sheet-%s.jpg' % (o, n))
PY
cat $OUT/fz-$name.out
