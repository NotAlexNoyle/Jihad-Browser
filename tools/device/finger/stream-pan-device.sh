# T-561 (device side of stream-pan.sh): on the spike daemon's page, DRAGS emulated finger drags (480,900 -> 480,250
# in 1 s, a second apart) marked in the daemon log, with the frame sampler (fbstream) reading the shown page for the
# drags' whole length when STREAM=1 (its options in FBS_OPTS), the finger's scans traced; the CPU ticks of the daemon
# and of each LunaSysMgr over the drags as finger-pan-device.sh takes them. Writes the daemon log, /tmp and the
# stream under /media/internal/jihad-stream (the 40 MB tmpfs /tmp fills with a round's dumps: 2026-11-03, the verify
# mode's wrong-frame dumps left the stream short); stream-pan.sh removes the stream directory after pulling it.
STATE=/var/palm/jihad/enyo; LOG=$STATE/daemon-gpu.log
DRAGS=${1:-8}; STREAM=${2:-0}; shift 2 2>/dev/null; FBS_OPTS="$*"
[ -x /tmp/finger ] || { echo "FATAL: no /tmp/finger (tools/device/finger/README.md)"; exit 1; }
[ "$STREAM" = 1 ] && [ ! -x /tmp/fbstream ] && { echo "FATAL: no /tmp/fbstream (tools/device/finger/README.md)"; exit 1; }
mark() { echo "[round] $1 at $(cut -d' ' -f1 /proc/uptime)" >> $LOG; }
ticks() { word=$1; for p in /proc/[0-9]*; do c=$(tr '\0' ' ' 2>/dev/null < $p/cmdline); case "$c" in *"$word"*) set -- $(cat $p/stat 2>/dev/null); echo "${p#/proc/} $((${14} + ${15})) $c" | cut -c1-90;; esac; done; }
total() { head -1 /proc/stat; }
SD=/media/internal/jihad-stream; rm -rf $SD; mkdir -p $SD
sleep 20
for w in jihad-browserserver LunaSysMgr; do ticks $w; done > /tmp/fp-t0.txt; total > /tmp/fp-s0.txt
if [ "$STREAM" = 1 ]; then
  /tmp/fbstream $SD/stream.fbs -s $((DRAGS * 2 + 3)) -f $SD/stream.fin $FBS_OPTS > /tmp/stream.out 2>&1 &
  SP=$!; sleep 1
fi
mark "finger drags"
echo "[round] finger drags at $(date +%s)000 ms" >> $LOG
i=0
while [ $i -lt $DRAGS ]; do FINGER_TRACE=$SD/stream.fin /tmp/finger drag 480 900 480 250 1000; sleep 1; i=$((i+1)); done
mark "end"
for w in jihad-browserserver LunaSysMgr; do ticks $w; done > /tmp/fp-t1.txt; total > /tmp/fp-s1.txt
[ "$STREAM" = 1 ] && { wait $SP; echo "--- fbstream"; cat /tmp/stream.out; ls -la $SD; df /tmp /media/internal | tail -2; }
echo "--- CPU ticks over the drags (pid, before, after, command)"
while read pid t0 rest; do t1=$(grep "^$pid " /tmp/fp-t1.txt | cut -d' ' -f2); echo "$pid $t0 $t1 $rest"; done < /tmp/fp-t0.txt
echo "--- /proc/stat before and after"; cat /tmp/fp-s0.txt /tmp/fp-s1.txt
rm -f /tmp/fp-t0.txt /tmp/fp-t1.txt /tmp/fp-s0.txt /tmp/fp-s1.txt /tmp/stream.out
