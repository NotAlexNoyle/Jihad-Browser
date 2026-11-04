#!/bin/bash
# stream-pan.sh <name> <url> [drags] ["<env words>"] [stream: 0|1] ["<fbstream options>"]: T-561 (cavekit-finger-testing
# R2). finger-pan.sh with the frame sampler: the spike daemon with the timeline on the page, the emulated finger's drags
# (stream-pan-device.sh) with fbstream reading the shown page while they run (stream 1) or not (stream 0, the cost's
# control), the daemon log to $ROUND_DIR/fp-<name>-daemon-gpu.log and the stream to $ROUND_DIR/fs-<name>.fbs/.fin.
# Read the rate with tools/host/flow-fps.py and the stream with tools/host/finger-stream.py.
set -u
T=$(cd "$(dirname "$0")" && pwd); R=$(cd "$T/../../.." && pwd); D=${ROUND_DIR:-$R/docs/rounds/finger-$(date +%F)}
T5=$R/tools/device/t502
OUT=${OUT_DIR:-/tmp}
name=$1; url=$2; drags=${3:-8}; extra=${4:-}; stream=${5:-1}; fbs=${6:-}
mkdir -p "$D"
cd $R; . build/webos-oe/host-path.sh
build/webos-oe/dev-sh.sh -t 90 $T5/gpu-spike-start.sh "$url" "JIHAD_INJECT=1 JIHAD_GPU_EXPRESS=1 JIHAD_GPU_TIMELINE=1 JIHAD_GPU_PHASES=1 PREF=jihad.viewport.followOverflow=true $extra" 2>&1 | grep 'screenshot at\|FATAL'
# T-567: THREADSTAT=1 samples the daemon's threads over the drags (threadstat.c on the device as /tmp/threadstat)
if [ "${THREADSTAT:-0}" = 1 ]; then novacom put file:///tmp/threadstat.on < /dev/null; else novacom run file://bin/rm -- -f /tmp/threadstat.on </dev/null >/dev/null 2>&1; fi
# T-567 (a test lever): RENICE_MAIN=<n> renices the daemon's main thread over the drags
if [ -n "${RENICE_MAIN:-}" ]; then echo "$RENICE_MAIN" | novacom put file:///tmp/renice.main; else novacom run file://bin/rm -- -f /tmp/renice.main </dev/null >/dev/null 2>&1; fi
build/webos-oe/dev-sh.sh -t 200 $T/stream-pan-device.sh $drags $stream $fbs > $D/fp-$name-cpu.txt 2>&1
if [ "$stream" = 1 ]; then
  SD=/media/internal/jihad-stream
  novacom get file://$SD/stream.fbs > $D/fs-$name.fbs; novacom get file://$SD/stream.fin > $D/fs-$name.fin
  [ "${THREADSTAT:-0}" = 1 ] && novacom get file://$SD/threads.txt > $D/ft-$name.txt
  # the event shots (-U): the whole page the sampler kept when a one-colour run appeared where content was
  for f in $(grep -a -o "$SD/stream.fbs.event[0-9]*.raw" $D/fp-$name-cpu.txt | sort -u); do novacom get file://$f > $D/fs-$name.$(basename $f .raw | sed 's/stream.fbs.//').raw; done
  # a pull is whole when its size is the device's (the ls -la line of stream-pan-device.sh)
  for f in fbs fin; do want=$(grep -a "^-" $D/fp-$name-cpu.txt | awk -v n="stream.$f" '$NF == n {print $5}' | head -1); got=$(wc -c < $D/fs-$name.$f); [ "$got" = "$want" ] || echo "!! fs-$name.$f: pulled $got bytes, the device had ${want:-?}"; done
  novacom run file://bin/rm -- -rf $SD </dev/null >/dev/null 2>&1
fi
build/webos-oe/dev-sh.sh -t 60 $T5/gpu-spike-collect-stop.sh 2>/dev/null > $OUT/fp-collect.out
sed -n '/=== LOG BEGIN/,/=== LOG END/p' $OUT/fp-collect.out | grep -v 'Fontconfig warning' > $D/fp-$name-daemon-gpu.log
grep -a 'enyo daemons running\|ERROR' $OUT/fp-collect.out
grep -a 'fbstream:' $D/fp-$name-cpu.txt
