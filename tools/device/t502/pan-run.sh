#!/bin/bash
# pan-run.sh <name> <url> "<env words>": the spike start with the inject channel, then a scripted
# finger pan (inject "pan 0 3000 10000": 0 -> 3000 px over 10 s), a screenshot mid-pan, the log.
set -u
S=$(dirname "$0"); R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser; D=$R/docs/rounds/t502-2026-10-17
name=$1; url=$2; extra=${3:-}
cd $R; . build/webos-oe/host-path.sh
echo "=== pan $name"
build/webos-oe/dev-sh.sh -t 90 $S/gpu-spike-start.sh "$url" "JIHAD_INJECT=1 $extra" 2>&1 | grep 'screenshot at\|FATAL' | tail -n 2
cat > $S/pan-inject.sh <<'EOS'
STATE=/var/palm/jihad/enyo
: > $STATE/daemon-gpu.pan-mark
umask 077; echo "pan 0 3000 10000" > $STATE/inject.cmd; chmod 600 $STATE/inject.cmd
sleep 5
luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/st-pan.png"}' >/dev/null 2>&1
sleep 7
echo "--- pan log"
grep -a 'inject pan\|paint pipeline\|async scroll\|scroll anchor\|FATAL' $STATE/daemon-gpu.log | tail -n 14
EOS
build/webos-oe/dev-sh.sh -t 60 $S/pan-inject.sh
novacom get file:///tmp/st-pan.png > $D/pan-$name.png 2>/dev/null
python3 -c "
from PIL import Image
Image.open('$D/pan-$name.png').rotate(-90, expand=True).convert('RGB').resize((576,768)).save('$D/pan-$name.jpg', quality=85)" 2>/dev/null && echo "screenshot $D/pan-$name.jpg"
build/webos-oe/dev-sh.sh -t 40 $S/gpu-spike-collect.sh 2>/dev/null | sed -n '/=== LOG BEGIN/,/=== LOG END/p' | grep -v 'Fontconfig warning' > $D/pan-$name-daemon-gpu.log
