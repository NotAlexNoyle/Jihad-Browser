#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# lowpower-bench.sh [runs] : the low-power baseline (cavekit-lowpower-uxp.md R1, T-555), on the Enyo variant's
# shipped build. One row per run and measurement in $ROUND_DIR/lowpower-bench.tsv, the daemon logs beside it, and the
# medians with their spread at the end. Every lever of that kit is judged against these rows, taken the same way.
#
# What a run measures (each from the daemon's timeline, JIHAD_GPU_TIMELINE=1, wall-clock ms on one clock):
#   wiki    desktop Wikipedia (HP_TouchPad): the first content frame to the load's end ("load done"), the main thread's
#           calls of 15 ms or more in that window and in the 10 s after it (count, those of 100 ms or more, the longest,
#           their sum: the page's script and paints), and in the 30 s at rest after that (the page's own churn); the
#           daemon's RSS and PSS (/proc/<pid>/smaps) after the load and 30 s later; then the scripted pan (inject
#           "pan 0 3000 10000"): its published frames over its ms (tools/host/gap-trace.py) and the daemon's paint
#           pipeline rate over the pan's 2 s windows (median)
#   jsbench jihad-jsbench.html's five timings (the page writes them into its title; the daemon logs titles)
#   long    the long static page's scripted pan rate
#   start   the daemon's start to its engine up ("daemon main" to "engine up" on the timeline): cold (page cache dropped
#           first) and warm (started again at once); and the card's connection to its first content frame ("card
#           connected" to "first painted frame") on the colours page
#
# The device is driven as the T-502 rounds drive it (tools/device/t502: gpu-spike-start.sh starts the variant's own
# binaries by hand with the round's environment, gpu-spike-stop.sh puts the upstart job back). The bench rules: the
# sensors run, nothing is written to cpufreq, one novacom session at a time, the owner not at the device.
#
#   RUNS=5 (default) tools/device/lowpower-bench.sh
#   ROUND_DIR=docs/rounds/lowpower-<date>   where the rows and logs go (default: a dated dir under docs/rounds)
#   HOST=192.168.1.245:8077                 the host's test page server (render/goanna/test, python3 -m http.server 8077)
#   ONLY="wiki jsbench long start"          a subset
#   LEVER=<name> EXTRA_ENV="<env words>"    a lever's rows (R2-R5): the name in the rows' lever column, the words
#                                           added to the daemon's environment (and PREF=/GPREF= words for prefs)
#   LEVER_B=<name> EXTRA_ENV_B="<words>"    a second setting run in the same pass, the two alternating run by run
#                                           (an interleaved control: LEVER_B=baseline EXTRA_ENV_B="" — the levers
#                                           of 2026-11-09 measured after the baseline read 2.5 s slower loads
#                                           whatever the words, so a lever is read against a control of its own hour)
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
# The shipped job's switches (packaging/event.d/jihad: the GPU path with the express lane; the spike start adds the
# compositor word itself) and the shipped viewport pref (the spike start forces followOverflow off unless told), plus
# the inject channel and the timeline this bench reads.
BASE_ENV="JIHAD_INJECT=1 JIHAD_GPU_EXPRESS=1 JIHAD_GPU_TIMELINE=1 PREF=jihad.viewport.followOverflow=true"
ENV="$BASE_ENV ${EXTRA_ENV:-}"
LEVER=${LEVER:-baseline}   # the rows' lever column: baseline, or the switch words under test (R2-R5)
LEVER_A=$LEVER; ENV_A=$ENV
WIKI=${PAGE:-https://en.wikipedia.org/wiki/HP_TouchPad}   # PAGE=<url>: another page through the "wiki" measurement (R4's animated sites)
ONLY=${ONLY:-wiki jsbench long start}
SCR=${TMPDIR:-/tmp}/lpb.$$; mkdir -p "$SCR"; trap 'rm -rf "$SCR"' EXIT
STATE=/var/palm/jihad/enyo
LOG=$STATE/daemon-gpu.log

say() { echo; echo "##### $(date +%H:%M:%S) $*"; }
row() { printf '%s\t%s\t%s\n' "$(date +%Y-%m-%dT%H:%M:%S)" "$LEVER" "$*" >> "$TSV"; echo "   row: $LEVER $*"; }
[ -s "$TSV" ] || printf 'when\tlever\trun\tmeasure\tfields...\n' > "$TSV"

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
def first(name):
    xs = [t for t, r in ev if r == name]
    return xs[0] if xs else None
if mode == 'start':
    main, up, conn, painted = first('daemon main'), first('engine up'), first('card connected'), first('first painted frame')
    print(up - main if main and up else 'NA', painted - conn if conn and painted else 'NA')
elif mode == 'load':
    # the load's end: the timeline's time at the daemon's "load done" line (the line after it on the timeline)
    t0 = first('first painted frame')
    tl_after_done = None
    seen = False
    for line in open(path, encoding='latin1'):
        if 'load done uri=' in line: seen = True; continue
        m = re.search(r'\[jihad-tl\] t=(\d+) ', line)
        if seen and m: tl_after_done = int(m.group(1)); break
    if t0 is None or tl_after_done is None: print('NA NA NA NA NA NA NA NA NA NA'); sys.exit()
    mains = [(t, int(re.search(r'(\d+) ms$', r).group(1))) for t, r in ev if r.startswith('main ') and t >= t0]
    def stats(lo, hi):
        w = [ms for t, ms in mains if lo <= t < hi]
        return [len(w), sum(1 for ms in w if ms >= 100), max(w) if w else 0, sum(w)]
    load = stats(t0, tl_after_done + 10000)          # the load and its first 10 s
    rest = stats(tl_after_done + 10000, tl_after_done + 40000)   # the 30 s at rest after
    # the layer paints alone over the load and its 10 s (T-559's content backend lever reads on these)
    paints = [int(re.search(r'(\d+) ms$', r).group(1)) for t, r in ev if r.startswith('main layer paint') and t0 <= t < tl_after_done + 10000]
    print(tl_after_done - t0, *load, rest[0], rest[3], rest[2], sum(paints), len(paints))
elif mode == 'pipe':
    # the paint pipeline's rate over the pan: the "(N fps)" of its 2 s windows after the pan mark line, median
    rates = []; on = False
    for line in open(path, encoding='latin1'):
        if 'inject pan' in line and 'done' not in line: on = True
        if 'inject pan done' in line: on = False
        m = re.search(r'paint pipeline: .* in (\d+)ms \(([\d.]+) fps\)', line)
        if on and m and int(m.group(1)) >= 1500: rates.append(float(m.group(2)))
    rates.sort()
    print('%.1f' % rates[len(rates)//2] if rates else 'NA')
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
t=0
while [ $t -lt 90 ]; do grep -aq 'load done uri=' $LOG && break; sleep 1; t=$((t+1)); done
sleep 10
echo "load done after $t s (main lines $(grep -ac 'jihad-tl\] t=[0-9]* main ' $LOG)); load done lines: $(grep -ac 'load done uri=' $LOG)"
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

# The USB drop (cavekit-device-headroom.md: eMMC saturation under a heavy page takes the link down for a while): a
# measurement starts only with the device on novacom, and after a drop the system's "Connected" alert is closed (the
# owner's standing rule) before anything else; the rows of a run the drop interrupted come out NA and are left out
# of the medians (the summary skips a row whose values do not match its names).
ready() {   # the device on novacom, and the Connected alert closed by LOOKING (tools/device/close-usb-alert.sh): a
	# drop inside a command can come back before this poll, so "was the device away" is not enough (2026-11-09)
	local out
	out=$("$R/tools/device/close-usb-alert.sh" 300) || { echo "!! $out"; return 1; }
	case "$out" in *"came back"*|*"was up"*) echo "   $out" | tr '\n' ' '; echo ;; esac
	return 0
}
spike() {   # spike <url> -> the hand-started daemon with the timeline, the card on the page
	ready || return 1
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
  for side in A B; do
	if [ "$side" = B ]; then
		[ -n "${LEVER_B:-}" ] || continue
		LEVER=$LEVER_B; ENV="$BASE_ENV ${EXTRA_ENV_B:-}"
	else
		LEVER=$LEVER_A; ENV=$ENV_A
	fi
	# each side's logs under its own name (the lever's, made safe for a file name), or the second overwrites the first
	LV=$(printf '%s' "$LEVER" | tr -c 'A-Za-z0-9.-' '_' | cut -c1-48)
	case " $ONLY " in *" wiki "*)
	say "run $r: wiki"
	spike "$WIKI"
	idle=$(dev "$SCR/idle.sh" 140 | tail -n 1); echo "   $idle"
	mem1=$(dev "$SCR/mem.sh" 20 | tail -n 1)
	sleep 30
	mem2=$(dev "$SCR/mem.sh" 20 | tail -n 1)
	dev "$SCR/pan.sh" 40 | tail -n 1 >/dev/null
	collect "$D/lp-r$r-$LV-wiki.log"
	load=$(tl load "$D/lp-r$r-$LV-wiki.log")
	rate=$(panrate "$D/lp-r$r-$LV-wiki.log"); pipe=$(tl pipe "$D/lp-r$r-$LV-wiki.log")
	row "$r" wiki "load_ms main_calls main_ge100 main_max_ms main_sum_ms rest_calls rest_sum_ms rest_max_ms paint_sum_ms paint_calls rss_kb pss_kb rss30_kb pss30_kb pan_frames pan_ms pan_fps pipe_fps" $load $mem1 $mem2 $rate $pipe
	stopspike
	;; esac
	case " $ONLY " in *" jsbench "*)
	say "run $r: jsbench"
	spike "http://$HOST/jihad-jsbench.html"
	res=$(dev "$SCR/title.sh" 170 | tail -n 1)
	collect "$D/lp-r$r-$LV-jsbench.log"
	nums=$(echo "$res" | grep -oE '[0-9]+ ms' | awk '{printf "%s ", $1}')
	row "$r" jsbench "loop50M_ms objects2M_ms strings200k_ms regex100k_ms dom20k_ms" ${nums:-NA NA NA NA NA}
	stopspike
	;; esac
	case " $ONLY " in *" long "*)
	say "run $r: long page pan"
	spike "http://$HOST/jihad-long-static.html"
	dev "$SCR/idle.sh" 140 | tail -n 1 >/dev/null
	dev "$SCR/pan.sh" 40 | tail -n 1 >/dev/null
	collect "$D/lp-r$r-$LV-long.log"
	rate=$(panrate "$D/lp-r$r-$LV-long.log"); pipe=$(tl pipe "$D/lp-r$r-$LV-long.log")
	row "$r" long "pan_frames pan_ms pan_fps pipe_fps" $rate $pipe
	stopspike
	;; esac
	case " $ONLY " in *" start "*)
	say "run $r: daemon start, cold then warm"
	dev "$SCR/drop.sh" 30 | tail -n 1 >/dev/null
	spike "http://$HOST/jihad-colors.html"
	collect "$D/lp-r$r-$LV-start-cold.log"; cold=$(tl start "$D/lp-r$r-$LV-start-cold.log")
	stopspike
	spike "http://$HOST/jihad-colors.html"
	collect "$D/lp-r$r-$LV-start-warm.log"; warm=$(tl start "$D/lp-r$r-$LV-start-warm.log")
	stopspike
	row "$r" start "cold_up_ms cold_card_ms warm_up_ms warm_card_ms" $cold $warm
	;; esac
  done
done

say "summary (median [min..max] over the runs; $TSV)"
python3 - "$TSV" <<'PY'
import sys, collections
rows = [l.rstrip('\n').split('\t') for l in open(sys.argv[1])][1:]
by = collections.OrderedDict()
for when, lever, rest in rows:
    words = rest.split()          # run, measure, the field names, then as many values
    run, meas, fields = words[0], words[1], words[2:]
    k = len(fields) // 2
    names = [f for f in fields if not f.replace('.', '').replace('-', '').isdigit() and f != 'NA']
    if len(fields) != 2 * len(names): continue   # a run a drop interrupted: fewer values than names
    k = len(names)
    for n, v in zip(fields[:k], fields[k:]):
        try: by.setdefault((lever, meas, n), []).append(float(v))
        except ValueError: pass
for (lever, meas, n), vs in by.items():
    vs.sort(); med = vs[len(vs)//2] if len(vs) % 2 else (vs[len(vs)//2-1] + vs[len(vs)//2]) / 2
    print(f"{lever:14s} {meas:8s} {n:16s} {med:10.1f}  [{vs[0]:.1f}..{vs[-1]:.1f}]  n={len(vs)}")
PY
