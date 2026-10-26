#!/bin/bash
# finger-pan.sh <name> <url> [drags] ["<env words>"]: T-505 (cavekit-gpu-rendering R3), the owner's "measure first,
# read only" (2026-10-26). The spike daemon with the timeline and the compose phases on the page, the emulated
# finger's drags (finger-pan-device.sh) with the CPU ticks of the daemon and LunaSysMgr; the log to $ROUND_DIR.
# Break it down with tools/host/express-jobs.py.
set -u
T=$(cd "$(dirname "$0")" && pwd); R=$(cd "$T/../../.." && pwd); D=${ROUND_DIR:-$R/docs/rounds/t505-2026-10-26}
OUT=${OUT_DIR:-/tmp}
name=$1; url=$2; drags=${3:-8}; extra=${4:-}
mkdir -p "$D"
cd $R; . build/webos-oe/host-path.sh
build/webos-oe/dev-sh.sh -t 90 $T/gpu-spike-start.sh "$url" "JIHAD_INJECT=1 JIHAD_GPU_EXPRESS=1 JIHAD_GPU_TIMELINE=1 JIHAD_GPU_PHASES=1 PREF=jihad.viewport.followOverflow=true $extra" 2>&1 | grep 'screenshot at\|FATAL'
build/webos-oe/dev-sh.sh -t 150 $T/finger-pan-device.sh $drags > $D/fp-$name-cpu.txt 2>&1
build/webos-oe/dev-sh.sh -t 60 $T/gpu-spike-collect-stop.sh 2>/dev/null > $OUT/fp-collect.out
sed -n '/=== LOG BEGIN/,/=== LOG END/p' $OUT/fp-collect.out | grep -v 'Fontconfig warning' > $D/fp-$name-daemon-gpu.log
grep -a 'enyo daemons running\|ERROR' $OUT/fp-collect.out
cat $D/fp-$name-cpu.txt
