#!/bin/bash
# T-505, the scaler (2026-11-09): desktop Wikipedia by finger, 16 drags with the stream sampler, the control and the
# scaler (JIHAD_GPU_CPU_SCALE=1) interleaved three times, then one verify round with the scaler; the table by
# tools/host/wiki-rounds-table.py (rate, express p90, the bottom band), the census line (how many leaves the scaler
# drew), the daemon's memory over the drags (stream-pan-device.sh's lines).
#   docs/rounds/t505-2026-11-09/rounds-scaler.sh [rounds=3]
set -u
R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser
cd "$R" || exit 2
export ROUND_DIR=$R/docs/rounds/t505-2026-11-09
N=${1:-3}
WIKI=https://en.wikipedia.org/wiki/HP_TouchPad
say() { echo; echo "##### $(date +%H:%M:%S) $*"; }
export JIHAD_REPO=$R; . build/webos-oe/host-path.sh
# The USB drop (cavekit-device-headroom.md: eMMC saturation under a heavy round takes the link down for a while): before
# a round the device must be back on novacom, and after any drop the system's "Connected" alert is closed (the owner's
# standing rule) before the finger moves again. A round whose stream came back empty is run once more.
ready() {
	local t=0
	while ! novacom -l 2>/dev/null | grep -q topaz; do sleep 5; t=$((t+5)); [ $t -ge 300 ] && { echo "!! the device did not come back in 300 s"; return 1; }; done
	if [ $t -gt 0 ]; then echo "   the device was away $t s (a USB drop): closing the Connected alert"; sleep 20; { echo '/tmp/finger tap 583 232; rm -rf /media/internal/jihad-stream'; sleep 4; } | novacom run file://bin/sh >/dev/null 2>&1; fi
	return 0
}
round() {   # round <name> <env words> <stream 0|1>
	local n=$1 env=$2 st=$3 try
	for try in 1 2; do
		ready || return 1
		tools/device/finger/stream-pan.sh "$n" "$WIKI" 16 "$env" "$st" 2>&1 | tail -4
		if [ "$st" = 1 ] && [ ! -s "$ROUND_DIR/fs-$n.fbs" ]; then echo "   !! $n: no stream (a drop?); again"; continue; fi
		[ -s "$ROUND_DIR/fp-$n-daemon-gpu.log" ] && return 0
		echo "   !! $n: no daemon log (a drop?); again"
	done
}
verify_round() {   # the verify round with the scaler, the first wrong frame dumped and fetched, compared on the host
	say "verify round with the scaler (no sampler; the first frame with > 50 wrong rows dumped)"
	round wiki-sv "JIHAD_GPU_CPU_SCALE=1 JIHAD_GPU_VERIFY=1 JIHAD_GPU_VERIFY_DUMP_WRONG=1" 0
	for f in vw-slot vw-whole; do novacom get file:///tmp/$f.raw > "$ROUND_DIR/$f.raw" 2>/dev/null; echo "   $f.raw: $(stat -c %s "$ROUND_DIR/$f.raw") bytes"; done
	{ echo 'rm -f /tmp/vw-slot.raw /tmp/vw-whole.raw /tmp/vs-slot.raw /tmp/vs-whole.raw'; sleep 3; } | novacom run file://bin/sh >/dev/null 2>&1
	if [ -s "$ROUND_DIR/vw-slot.raw" ] && [ -s "$ROUND_DIR/vw-whole.raw" ]; then
		python3 "${VERIFY_DIFF:-/tmp/claude-1000/-home-notalexnoyle-eclipse-workspace-Jihad/a4af0791-1f3d-4e1d-a2f0-982ce7d3a95a/scratchpad/verify-diff.py}" "$ROUND_DIR/vw-slot.raw" "$ROUND_DIR/vw-whole.raw" 2>&1 | head -24
	fi
}
if [ "${VERIFY_FIRST:-0}" = 1 ]; then verify_round; fi
for i in $(seq 1 "$N"); do
	say "control $i"; round wiki-c$i "" 1
	say "scaler $i";  round wiki-s$i "JIHAD_GPU_CPU_SCALE=1" 1
done
[ "${VERIFY_FIRST:-0}" = 1 ] || verify_round
say "table"
names=""; for i in $(seq 1 "$N"); do names="$names wiki-c$i wiki-s$i"; done
python3 tools/host/wiki-rounds-table.py "$ROUND_DIR" $names 2>&1 | tail -20
say "census (the last 'cpu bands' line of each round) and verify"
for n in $names wiki-sv; do echo "$n: $(grep -a 'cpu bands:' $ROUND_DIR/fp-$n-daemon-gpu.log | tail -n 1 | sed 's/.*cpu bands: //' | cut -c1-200)"; done
echo "verify frames: $(grep -ac '\] verify .*slot' $ROUND_DIR/fp-wiki-sv-daemon-gpu.log), with rows wrong: $(grep -a '\] verify .*slot' $ROUND_DIR/fp-wiki-sv-daemon-gpu.log | grep -avc '; 0 rows wrong:'), structurally wrong: $(grep -a '\] verify .*slot' $ROUND_DIR/fp-wiki-sv-daemon-gpu.log | grep -avc '; 0 rows structurally wrong')"
grep -a '\] verify .*slot' $ROUND_DIR/fp-wiki-sv-daemon-gpu.log | grep -av '; 0 rows wrong:' | head -3 | cut -c1-220
say "the daemon's memory over the drags (pid VmHWM VmRSS VmData kB) before/after, per round"
for n in $names; do echo "-- $n: $(sed -n '/daemon memory before and after/,/^---/p' $ROUND_DIR/fp-$n-cpu.txt | awk 'NF >= 4 && $3 > 100000 {print "VmRSS " $3 " kB (HWM " $2 ")"}' | head -2 | tr '\n' ' ')"; done
echo DONE
