#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# engine-checks.sh <round dir>: the areas an engine update touches, on the Enyo variant by a hand-started daemon
# with the inject channel (cavekit-engine-release-2026-10.md R4): the daemon's engine-exports line; Jihad's own
# pages through jar:/resource: (about:settings, about:personas, about:addons) and an SVG filter page
# (render/goanna/test/jihad-svg-filter.html, served by the test server), each with a screenshot to look at; a
# long press on the long page (selection); the cookie store across the update (a cookie set, counted again
# after a daemon restart). The Enyo job is put back after. Then run tools/device/cert-checks.sh.
set -u
R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser; cd $R; . build/webos-oe/host-path.sh
D=${1:?round dir}; mkdir -p "$D"
FINGER=${FINGER:-}   # a host path of the cross-built finger tool to push (tools/device/finger/README.md)
[ -n "$FINGER" ] && [ -f "$FINGER" ] && novacom put file:///tmp/finger < "$FINGER"
build/webos-oe/dev-sh.sh -t 90 tools/device/t502/gpu-spike-start.sh "https://en.wikipedia.org/wiki/HP_TouchPad" "JIHAD_INJECT=1" 2>&1 | grep 'screenshot at\|FATAL' | tail -n 1
S=/tmp/claude-engine-checks.sh
cat > $S <<'EOS'
STATE=/var/palm/jihad/enyo
LOG=$STATE/daemon-gpu.log
[ -f /tmp/finger ] && chmod +x /tmp/finger
sleep 6
echo "== exports: $(grep -a 'exports\|export' $LOG | grep -a -i 'engine\|jihad_' | head -n 2 | tr '\n' '|' | cut -c1-300)"
echo "== roots/tls: $(grep -a 'roots:\|TLS\|tls' $LOG | head -n 2 | tr '\n' '|' | cut -c1-200)"
shot() { sleep 7; luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/ec-'$1'.png"}' >/dev/null 2>&1; }
inj() { umask 077; echo "$1" > $STATE/inject.cmd; }
mark=$(wc -c < $LOG)
inj "cookie set en.wikipedia.org jihadupd rb27"; sleep 2
inj "cookie count"; sleep 2
echo "== cookie: $(tail -c +$((mark+1)) $LOG | grep -a 'inject cookie' | tr '\n' '|')"
for p in about:settings about:personas about:addons; do
  mark=$(wc -c < $LOG); inj "url $p"; shot ${p#about:}
  echo "== $p -> $(tail -c +$((mark+1)) $LOG | grep -a 'load done\|loaderr\|js-console.*rror' | head -n 3 | tr '\n' '|' | cut -c1-300)"
done
mark=$(wc -c < $LOG); inj "url http://192.168.1.245:8077/jihad-svg-filter.html"; shot svg
echo "== svg -> $(tail -c +$((mark+1)) $LOG | grep -a 'load done\|loaderr\|FATAL' | head -n 2 | tr '\n' '|' | cut -c1-200)"
mark=$(wc -c < $LOG); inj "url http://192.168.1.245:8077/jihad-long-static.html"; sleep 8
[ -x /tmp/finger ] && /tmp/finger hold 300 400 1500; sleep 3
luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/ec-select.png"}' >/dev/null 2>&1
echo "== select -> $(tail -c +$((mark+1)) $LOG | grep -a 'selection\|long\|FATAL' | head -n 3 | tr '\n' '|' | cut -c1-200)"
echo "== faults: $(grep -a -c 'FATAL SIGNAL' $LOG), js errors: $(grep -a -c 'js-console.*JavaScript Error' $LOG), sqlite: $(grep -a -i -c 'sqlite.*error\|corrupt' $LOG)"
EOS
build/webos-oe/dev-sh.sh -t 300 $S | tee "$D/engine-checks.txt"
rm -f $S
for n in settings personas addons svg select; do
  novacom get file:///tmp/ec-$n.png > "$D/ec-$n.png" 2>/dev/null
  python3 -c "
from PIL import Image
Image.open('$D/ec-$n.png').rotate(-90, expand=True).convert('RGB').resize((576,768)).save('$D/ec-$n.jpg', quality=85)" 2>/dev/null && rm -f "$D/ec-$n.png" && echo "screenshot $D/ec-$n.jpg"
done
novacom run file:///bin/rm -- -f /tmp/ec-settings.png /tmp/ec-personas.png /tmp/ec-addons.png /tmp/ec-svg.png /tmp/ec-select.png
# the cookie across a restart: the job's daemon, then the count under the inject channel again is the next round's;
# here the restart is the job's (enyo-restore-job.sh) and the count is read from the job's log at the next start
build/webos-oe/dev-sh.sh -t 90 tools/device/t502/enyo-restore-job.sh 2>&1 | grep 'running\|killing' | sed 's/^/restore: /'
