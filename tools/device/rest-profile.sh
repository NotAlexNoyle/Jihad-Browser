#!/bin/bash
# rest-profile.sh [label] ["<env words>"] : T-604 — what keeps the engine's main thread busy while a page sits still.
# The Enyo hand daemon (tools/device/t502/gpu-spike-start.sh) on PAGE (desktop Wikipedia) with the timeline and the
# stall sampler (every event of 5 ms or more sampled every 10 ms: the program counter), the load let finish and 10 s
# more, then 30 s at rest; the main thread's calls over the rest window from the timeline, and its samples over the
# same window as ONE profile (tools/host/stall-resolve.py --from --to --merge, against the unstripped libxul).
#   tools/device/rest-profile.sh base
#   tools/device/rest-profile.sh nojs "PREF=javascript.enabled=false"
set -u
R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser
cd "$R" || exit 2
export JIHAD_REPO=$R; . build/webos-oe/host-path.sh
T=tools/device/t502
LABEL=${1:-base}; EXTRA=${2:-}
PAGE=${PAGE:-https://en.wikipedia.org/wiki/HP_TouchPad}
D=${ROUND_DIR:-$R/docs/rounds/rest-2026-11-09}; mkdir -p "$D"
SCR=${TMPDIR:-/tmp}/rp.$$; mkdir -p "$SCR"; trap 'rm -rf "$SCR"' EXIT
cat > "$SCR/rest.sh" <<'EOS'
LOG=/var/palm/jihad/enyo/daemon-gpu.log
rm -f /tmp/jihad-stall.txt
t=0; while [ $t -lt 90 ]; do grep -aq 'load done uri=' $LOG && break; sleep 1; t=$((t+1)); done
sleep 10
echo "rest from $(date +%s)000"
sleep 30
echo "rest to $(date +%s)000"
EOS
build/webos-oe/dev-sh.sh -t 90 $T/gpu-spike-start.sh "$PAGE" JIHAD_GPU_EXPRESS=1 JIHAD_GPU_TIMELINE=1 JIHAD_STALL_SAMPLER=5 PREF=jihad.viewport.followOverflow=true $EXTRA 2>&1 | grep -a "screenshot at\|FATAL\|stall sampler" | head -3
out=$(build/webos-oe/dev-sh.sh -t 160 "$SCR/rest.sh" 2>&1)
from=$(echo "$out" | sed -n 's/^rest from \([0-9]*\)$/\1/p'); to=$(echo "$out" | sed -n 's/^rest to \([0-9]*\)$/\1/p')
echo "rest window: $from .. $to"
novacom get file:///tmp/jihad-stall.txt > "$D/stall-$LABEL.txt" 2>/dev/null
build/webos-oe/dev-sh.sh -t 40 $T/gpu-spike-collect.sh 2>/dev/null | sed -n '/=== LOG BEGIN/,/=== LOG END/p' | grep -v 'Fontconfig warning' > "$D/rest-$LABEL-daemon.log"
build/webos-oe/dev-sh.sh -t 60 $T/gpu-spike-stop.sh 2>&1 | grep -a "enyo daemons running\|FATAL"
{ echo 'rm -f /tmp/jihad-stall.txt'; sleep 3; } | novacom run file://bin/sh >/dev/null 2>&1
python3 - "$D/rest-$LABEL-daemon.log" "$from" "$to" <<'PY'
import re, sys
path, a, b = sys.argv[1], int(sys.argv[2] or 0), int(sys.argv[3] or 0)
calls = []
for line in open(path, encoding='latin1'):
    m = re.search(r'\[jihad-tl\] t=(\d+) main (.*) (\d+) ms$', line.strip())
    if m and a <= int(m.group(1)) <= b: calls.append((m.group(2), int(m.group(3))))
tot = sum(ms for _, ms in calls)
print("main thread over the rest window: %d calls of 15 ms or more, %d ms of %d (%.0f %%)" % (len(calls), tot, b - a, 100.0 * tot / max(1, b - a)))
by = {}
for k, ms in calls: by[k] = by.get(k, 0) + ms
for k, v in sorted(by.items(), key=lambda x: -x[1]): print("  %-40s %d ms" % (k, v))
PY
XUL=build/webos-oe/out-arm/obj-jihad-goanna-arm/toolkit/library/libxul.so
[ -s "$D/stall-$LABEL.txt" ] && python3 -I tools/host/stall-resolve.py "$D/stall-$LABEL.txt" --xul "$XUL" --from "$from" --to "$to" --merge --top 25 > "$D/rest-$LABEL-profile.txt" 2>&1 && head -45 "$D/rest-$LABEL-profile.txt"
