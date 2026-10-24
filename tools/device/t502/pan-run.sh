#!/bin/bash
# pan-run.sh <name> <url> "<env words>": the spike start with the inject channel, then a scripted
# finger pan (inject "pan 0 3000 10000": 0 -> 3000 px over 10 s), a screenshot mid-pan, the log.
set -u
S=$(dirname "$0"); R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser; D=${ROUND_DIR:-$R/docs/rounds/t502-2026-10-17}   # ROUND_DIR=<dir> for a later round
name=$1; url=$2; extra=${3:-}
# $4: the inject command (default a 0 -> 3000 px pan over 10 s); "none" measures the page as it is
# PAN_TOP=1: the processes' CPU over 4 s of the pan (BusyBox top), after the pan log
pancmd=${4:-pan 0 3000 10000}
cd $R; . build/webos-oe/host-path.sh
echo "=== pan $name"
build/webos-oe/dev-sh.sh -t 90 $S/gpu-spike-start.sh "$url" "JIHAD_INJECT=1 JIHAD_GPU_DUMP_EVERY=60 $extra" 2>&1 | grep 'screenshot at\|FATAL' | tail -n 2
cat > $S/pan-inject.sh <<'EOS'
STATE=/var/palm/jihad/enyo
rm -f /tmp/frame.raw-*
sleep PANDELAY   # PAN_DELAY seconds for a page that is still loading
for p in /proc/[0-9]*; do c=$(tr '\0' ' ' 2>/dev/null < $p/cmdline); case "$c" in *jihad-browserserver*) echo "daemon env: $(tr '\0' '\n' < $p/environ 2>/dev/null | grep 'DUMP\|INJECT' | tr '\n' ' ')";; esac; done
: > $STATE/daemon-gpu.pan-mark
[ "PANCMD" != "none" ] && { umask 077; echo "PANCMD" > $STATE/inject.cmd; }   # the daemon consumes the file at once
if [ "PANTOP" = 1 ]; then
  sleep 1; top -b -n 2 -d 4 > /tmp/top-pan.txt 2>&1 &
  for p in /proc/[0-9]*; do c=$(tr '\0' ' ' 2>/dev/null < $p/cmdline); case "$c" in *jihad-browserserver*) DP=${p#/proc/};; esac; done
  for t in /proc/$DP/task/*; do echo "$(cat $t/comm) $(sed 's/.*) //' $t/stat | cut -d' ' -f12,13)"; done > /tmp/ts0.txt
  ( sleep 4; for t in /proc/$DP/task/*; do echo "$(cat $t/comm) $(sed 's/.*) //' $t/stat | cut -d' ' -f12,13)"; done > /tmp/ts1.txt ) &
fi
sleep 5
luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/st-pan.png"}' >/dev/null 2>&1
sleep 7
echo "--- pan log"
grep -a 'inject pan\|paint pipeline\|scroll anchor\|FATAL' $STATE/daemon-gpu.log | tail -n 12
if [ "PANTOP" = 1 ]; then
  sleep 1; echo "--- top over 4 s of the pan"; awk '/^Mem:/{n++} n==2' /tmp/top-pan.txt | sed -n '3,14p' | cut -c1-100
  echo "--- the daemon's threads over 4 s of the pan (% of a core, user+system)"
  awk 'NR==FNR {a[$1]+=$2+$3; next} {b[$1]+=$2+$3} END {for (k in b) if (b[k]-a[k] > 4) printf "%5.1f %s\n", (b[k]-a[k])/4, k}' /tmp/ts0.txt /tmp/ts1.txt | sort -rn | head -n 10
  rm -f /tmp/top-pan.txt /tmp/ts0.txt /tmp/ts1.txt
fi
echo "--- bands"
grep -a 'shifted by\|compositor thread: compose' $STATE/daemon-gpu.log | tail -n 6 | cut -c1-200
EOS
sed -i "s|PANCMD|$pancmd|g" $S/pan-inject.sh
sed -i "s|PANDELAY|${PAN_DELAY:-0}|g" $S/pan-inject.sh
sed -i "s|PANTOP|${PAN_TOP:-0}|g" $S/pan-inject.sh
build/webos-oe/dev-sh.sh -t $((60 + ${PAN_DELAY:-0})) $S/pan-inject.sh
novacom get file:///tmp/st-pan.png > $D/pan-$name.png 2>/dev/null
python3 -c "
from PIL import Image
Image.open('$D/pan-$name.png').rotate(-90, expand=True).convert('RGB').resize((576,768)).save('$D/pan-$name.jpg', quality=85)" 2>/dev/null && echo "screenshot $D/pan-$name.jpg"
for n in 60 120 180; do
  novacom get file:///tmp/frame.raw-$n > $S/pan-frame-$n.bgra 2>/dev/null || true
  if [ -s $S/pan-frame-$n.bgra ]; then
    python3 -c "
from PIL import Image
d = open('$S/pan-frame-$n.bgra','rb').read()
Image.frombytes('RGBA', (768, len(d) // 3072), d, 'raw', 'BGRA').convert('RGB').resize((384, len(d) // 6144)).save('$D/pan-$name-frame-$n.jpg', quality=85)" && echo "frame $n: $D/pan-$name-frame-$n.jpg"
  fi
done
build/webos-oe/dev-sh.sh -t 40 $S/gpu-spike-collect.sh 2>/dev/null | sed -n '/=== LOG BEGIN/,/=== LOG END/p' | grep -v 'Fontconfig warning' > $D/pan-$name-daemon-gpu.log
