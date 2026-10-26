# T-505 (device side of finger-pan.sh): on the spike daemon's page, DRAGS emulated finger drags (480,900 ->
# 480,250 in 1 s, a second apart), marked in the daemon log, with the CPU ticks of the daemon, of each LunaSysMgr
# (the system UI and, on CE, the web app manager's second instance, which draws the card) and of the whole
# device over the drags. Read only: nothing on the device is changed.
STATE=/var/palm/jihad/enyo; LOG=$STATE/daemon-gpu.log
DRAGS=${1:-8}
[ -x /tmp/finger ] || { echo "FATAL: no /tmp/finger (tools/device/finger/README.md)"; exit 1; }
mark() { echo "[round] $1 at $(cut -d' ' -f1 /proc/uptime)" >> $LOG; }
# utime+stime (fields 14, 15) of the processes whose command line has the word, one line per process
ticks() { word=$1; for p in /proc/[0-9]*; do c=$(tr '\0' ' ' 2>/dev/null < $p/cmdline); case "$c" in *"$word"*) set -- $(cat $p/stat 2>/dev/null); echo "${p#/proc/} $((${14} + ${15})) $c" | cut -c1-90;; esac; done; }
total() { head -1 /proc/stat; }
sleep 20
for w in jihad-browserserver LunaSysMgr; do ticks $w; done > /tmp/fp-t0.txt; total > /tmp/fp-s0.txt
mark "finger drags"
i=0
while [ $i -lt $DRAGS ]; do /tmp/finger drag 480 900 480 250 1000; sleep 1; i=$((i+1)); done
mark "end"
for w in jihad-browserserver LunaSysMgr; do ticks $w; done > /tmp/fp-t1.txt; total > /tmp/fp-s1.txt
echo "--- CPU ticks over the drags (pid, before, after, command)"
while read pid t0 rest; do t1=$(grep "^$pid " /tmp/fp-t1.txt | cut -d' ' -f2); echo "$pid $t0 $t1 $rest"; done < /tmp/fp-t0.txt
echo "--- /proc/stat before and after"; cat /tmp/fp-s0.txt /tmp/fp-s1.txt
rm -f /tmp/fp-t0.txt /tmp/fp-t1.txt /tmp/fp-s0.txt /tmp/fp-s1.txt
