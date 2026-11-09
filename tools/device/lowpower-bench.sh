#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# lowpower-bench.sh [runs] : the low-power baseline (cavekit-lowpower-uxp.md R1, T-555), on the Enyo variant's
# shipped build. One row per run and measurement in $ROUND_DIR/lowpower-bench.tsv, the daemon logs beside it, and the
# medians with their spread at the end. Every lever of that kit is judged against these rows, taken the same way.
#
# What a run measures (each from the daemon's timeline, JIHAD_GPU_TIMELINE=1, wall-clock ms on one clock):
#   wiki    desktop Wikipedia (HP_TouchPad): the first content frame to the page idle (no main-thread call of 15 ms or
#           more for 5 s), the main thread's calls in that window (count, those of 100 ms or more, the longest, their
#           sum), "load done" seen; the daemon's RSS and PSS (/proc/<pid>/smaps) at idle and 30 s later; then the
#           scripted pan (inject "pan 0 3000 10000") and its rate (tools/host/gap-trace.py: the pan's frames over its ms)
#   jsbench jihad-jsbench.html's five timings (the page writes them into its title; the daemon logs titles)
#   long    the long static page's scripted pan rate
#   start   daemon start to first content frame: cold (page cache dropped first) and warm (started again at once), on
#           the colours page; "daemon main" to "first painted frame" on the timeline
#
# The device is driven as the T-502 rounds drive it (tools/device/t502: gpu-spike-start.sh starts the variant's own
# binaries by hand with the round's environment, gpu-spike-stop.sh puts the upstart job back). The bench rules: the
# sensors run, nothing is written to cpufreq, one novacom session at a time, the owner not at the device.
#
#   RUNS=5 (default) tools/device/lowpower-bench.sh
#   ROUND_DIR=docs/rounds/lowpower-<date>   where the rows and logs go (default: a dated dir under docs/rounds)
#   HOST=192.168.1.245:8077                 the host's test page server (render/goanna/test, python3 -m http.server 8077)
#   ONLY="wiki jsbench long start"          a subset
set -u
R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser
cd "$R" || exit 2
. build/webos-oe/host-path.sh
T=tools/device/t502
RUNS=${RUNS:-${1:-5}}
HOST=${HOST:-192.168.1.245:8077}
D=${ROUND_DIR:-docs/rounds/lowpower-$(date +%Y-%m-%d)}
mkdir -p "$D"
TSV=$D/lowpower-bench.tsv
ENV="JIHAD_INJECT=1 JIHAD_GPU_TIMELINE=1"
WIKI=https://en.wikipedia.org/wiki/HP_TouchPad
ONLY=${ONLY:-wiki jsbench long start}
SCR=${TMPDIR:-/tmp}/lpb.$$; mkdir -p "$SCR"; trap 'rm -rf "$SCR"' EXIT
STATE=/var/palm/jihad/enyo
LOG=$STATE/daemon-gpu.log

say() { echo; echo "##### $(date +%H:%M:%S) $*"; }
row() { printf '%s\t%s\n' "$(date +%Y-%m-%dT%H:%M:%S)" "$*" >> "$TSV"; echo "   row: $*"; }
[ -s "$TSV" ] || printf 'when\trun\tmeasure\tfields...\n' > "$TSV"

# A device script run through dev-sh.sh; $1 the file, $2 the time limit.
dev() { build/webos-oe/dev-sh.sh -t "${2:-60}" "$1" 2>&1; }

# The timeline read on the host: python over the fetched log.
tl() { python3 - "$@" <<'PY'
import re, sys
mode, path = sys.argv[1], sys.argv[2]
ev = []
for line in open(path, encoding='latin1'):
    m = re.search(r'\[jihad-tl\] t=(\d+) (.*)', line)
    if m: ev.append((int(m.group(1)), m.group(2).strip()))
if mode == 'start':
    main = [t for t, r in ev if r == 'daemon main']
    first = [t for t, r in ev if r == 'first painted frame']
    print(first[0] - main[0] if main and first else 'NA')
elif mode == 'load':
    first = [t for t, r in ev if r == 'first painted frame']
    if not first: print('NA NA NA NA NA'); sys.exit()
    t0 = first[0]
    mains = [(t, int(re.search(r'(\d+) ms$', r).group(1))) for t, r in ev if r.startswith('main ') and t >= t0]
    # idle: the first 5 s gap after t0 with no main-thread call ending in it (the last call before it marks the idle)
    idle = t0
    for t, ms in mains:
        if t - idle > 5000: break
        idle = t
    inwin = [ms for t, ms in mains if t <= idle]
    print(idle - t0, len(inwin), sum(1 for ms in inwin if ms >= 100), max(inwin) if inwin else 0, sum(inwin))
PY
}

# The daemon's RSS and PSS on the device (kB), of the hand-started Enyo daemon.
cat > "$SCR/mem.sh" <<'EOS'
HL=/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/deviceroot/hl
for p in /proc/[0-9]*; do c=$(tr '\0' ' ' 2>/dev/null < $p/cmdline); case "$c" in *jihad-browserserver*"$HL"*) awk '/^Rss:/{r+=$2} /^Pss:/{s+=$2} END{print r, s}' $p/smaps;; esac; done
EOS
# Wait for the page to go idle (no main-thread timeline call for 5 s after the first content frame, cap 120 s).
cat > "$SCR/idle.sh" <<'EOS'
LOG=/var/palm/jihad/enyo/daemon-gpu.log
t=0; last=$(grep -ac 'jihad-tl\] t=[0-9]* main ' $LOG); q=0
while [ $t -lt 120 ]; do
  sleep 1; t=$((t+1)); n=$(grep -ac 'jihad-tl\] t=[0-9]* main ' $LOG)
  if grep -aq 'first painted frame' $LOG; then
    if [ "$n" = "$last" ]; then q=$((q+1)); else q=0; last=$n; fi
    [ $q -ge 5 ] && break
  fi
done
echo "idle after $t s (main lines $n); load done: $(grep -ac 'load done uri=' $LOG)"
EOS
# The scripted pan and its log lines.
cat > "$SCR/pan.sh" <<'EOS'
STATE=/var/palm/jihad/enyo
: > $STATE/daemon-gpu.pan-mark
umask 077; echo "pan 0 3000 10000" > $STATE/inject.cmd
sleep 13
echo "pan done"
EOS
cat > "$SCR/title.sh" <<'EOS'
LOG=/var/palm/jihad/enyo/daemon-gpu.log
t=0; while [ $t -lt 150 ]; do grep -aq 'jihad-jsbench: ' $LOG && break; sleep 1; t=$((t+1)); done
grep -a 'jihad-jsbench: ' $LOG | tail -n 1 | sed 's/.*jihad-jsbench: //; s/\].*//'
EOS
cat > "$SCR/drop.sh" <<'EOS'
sync; echo 3 > /proc/sys/vm/drop_caches; echo "page cache dropped"
EOS

spike() {   # spike <url> -> the hand-started daemon with the timeline, the card on the page
	dev_out=$(build/webos-oe/dev-sh.sh -t 90 $T/gpu-spike-start.sh "$1" $ENV 2>&1)
	echo "$dev_out" | grep 'screenshot at\|FATAL\|daemon started' | head -3
}
collect() {   # collect <file>: the daemon log to the host, the device's dumps removed
	build/webos-oe/dev-sh.sh -t 40 $T/gpu-spike-collect.sh 2>/dev/null | sed -n '/=== LOG BEGIN/,/=== LOG END/p' | grep -v 'Fontconfig warning' > "$1"
}
stopspike() { build/webos-oe/dev-sh.sh -t 60 $T/gpu-spike-stop.sh 2>&1 | grep 'enyo daemons running\|FATAL' ; }
panrate() {   # panrate <log> -> "frames ms fps"
	python3 tools/host/gap-trace.py "$1" 2>/dev/null | sed -n 's/^the pan: \([0-9]*\) frames over \([0-9]*\) ms.*/\1 \2/p' | awk '{ if ($2 > 0) printf "%d %d %.1f\n", $1, $2, $1 * 1000 / $2; else print "NA NA NA" }' | head -1
}

for r in $(seq 1 "$RUNS"); do
	case " $ONLY " in *" wiki "*)
	say "run $r: wiki"
	spike "$WIKI"
	idle=$(dev "$SCR/idle.sh" 140 | tail -n 1); echo "   $idle"
	mem1=$(dev "$SCR/mem.sh" 20 | tail -n 1)
	sleep 30
	mem2=$(dev "$SCR/mem.sh" 20 | tail -n 1)
	dev "$SCR/pan.sh" 40 | tail -n 1 >/dev/null
	collect "$D/lp-r$r-wiki.log"
	load=$(tl load "$D/lp-r$r-wiki.log")
	rate=$(panrate "$D/lp-r$r-wiki.log")
	row "$r" wiki "load_to_idle_ms main_calls main_ge100 main_max_ms main_sum_ms rss_kb pss_kb rss30_kb pss30_kb pan_frames pan_ms pan_fps" $load $mem1 $mem2 $rate
	stopspike
	;; esac
	case " $ONLY " in *" jsbench "*)
	say "run $r: jsbench"
	spike "http://$HOST/jihad-jsbench.html"
	res=$(dev "$SCR/title.sh" 170 | tail -n 1)
	collect "$D/lp-r$r-jsbench.log"
	nums=$(echo "$res" | grep -oE '[0-9]+ ms' | awk '{printf "%s ", $1}')
	row "$r" jsbench "loop50M_ms objects2M_ms strings200k_ms regex100k_ms dom20k_ms" ${nums:-NA NA NA NA NA}
	stopspike
	;; esac
	case " $ONLY " in *" long "*)
	say "run $r: long page pan"
	spike "http://$HOST/jihad-long-static.html"
	dev "$SCR/idle.sh" 140 | tail -n 1 >/dev/null
	dev "$SCR/pan.sh" 40 | tail -n 1 >/dev/null
	collect "$D/lp-r$r-long.log"
	rate=$(panrate "$D/lp-r$r-long.log")
	row "$r" long "pan_frames pan_ms pan_fps" $rate
	stopspike
	;; esac
	case " $ONLY " in *" start "*)
	say "run $r: daemon start, cold then warm"
	dev "$SCR/drop.sh" 30 | tail -n 1 >/dev/null
	spike "http://$HOST/jihad-colors.html"
	collect "$D/lp-r$r-start-cold.log"; cold=$(tl start "$D/lp-r$r-start-cold.log")
	stopspike
	spike "http://$HOST/jihad-colors.html"
	collect "$D/lp-r$r-start-warm.log"; warm=$(tl start "$D/lp-r$r-start-warm.log")
	stopspike
	row "$r" start "cold_ms warm_ms" "$cold" "$warm"
	;; esac
done

say "summary (median [min..max] over the runs; $TSV)"
python3 - "$TSV" <<'PY'
import sys, collections
rows = [l.rstrip('\n').split('\t') for l in open(sys.argv[1])][1:]
by = collections.OrderedDict()
for when, run, meas, *rest in rows:
    names = rest[0].split(); vals = rest[1:]
    for n, v in zip(names, vals):
        try: by.setdefault((meas, n), []).append(float(v))
        except ValueError: pass
for (meas, n), vs in by.items():
    vs.sort(); med = vs[len(vs)//2] if len(vs) % 2 else (vs[len(vs)//2-1] + vs[len(vs)//2]) / 2
    print(f"{meas:8s} {n:16s} {med:10.1f}  [{vs[0]:.1f}..{vs[-1]:.1f}]  n={len(vs)}")
PY
