# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# headroom-test.sh VARIANT URL [SECS] — cavekit-device-headroom.md R1 on the device: a card on URL (the memory test
# page, render/goanna/test/jihad-memhog.html?mb=700&rate=10, or any heavy page), and every 2 s the available memory,
# the pages swapped in, the load, the daemon's and its players' memory and how long the application manager takes to
# answer; the guard's own lines from the daemon's log as they come. It stops 20 s after the guard stopped the page,
# or after SECS (240). Safety: if the application manager takes over 8 s or the load reaches 14 twice running, the
# guard has failed and this closes the card itself, so the device is never left starving. Run it detached
# (tools/device/finger/detached-run.sh) and read its output afterwards: a USB drop must not take the test with it.
# With &swf=URL on the memory page a Flash player runs in the same card: the guard's first step ends it (logged) before
# the page is stopped; the first player's nice and oom_adj are printed when it appears, and after the card closes the
# players are watched for 45 s (R1: a player with no movie left exits within the engine's 30 s unload delay, using no
# CPU meanwhile).
V=${1:?variant}; URL=${2:?url}; SECS=${3:-240}
case $V in enyo) ID=net.riverstonerelay.jihad-browser;; mochi) ID=net.riverstonerelay.jihad-browser-mochi;; mojo) ID=net.riverstonerelay.jihad-browser-mojo;; *) exit 2;; esac
L=/var/palm/jihad/$V/daemon.log
close_app() { for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 "\"$ID\"" | grep processid | sed 's/.*"processid": *"\([^"]*\)".*/\1/'); do luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'"$pid"'"}' >/dev/null 2>&1; done; }
avail() { awk '/^MemFree:/{f=$2} /^Buffers:/{b=$2} /^Cached:/{c=$2} /^Shmem:/{s=$2} END{print f+b+c-s}' /proc/meminfo; }
rss() { r=0; for p in $1; do v=$(grep VmRSS /proc/$p/status 2>/dev/null | tr -s ' ' | cut -d' ' -f2); r=$((r + ${v:-0})); done; echo $r; }
cs() { cut -d' ' -f1 /proc/uptime | tr -d '.'; }
DP=$(status jihad$( [ $V = enyo ] || echo -$V ) | sed -n 's/.*process \([0-9]*\).*/\1/p')
close_app; sleep 2
n0=$(wc -l < $L)
luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"'$ID'","params":{"target":"'"$URL"'"}}' >/dev/null
echo "daemon $DP nice=$(awk '{print $19}' /proc/$DP/stat) oom_adj=$(cat /proc/$DP/oom_adj)"
in0=$(awk '/pswpin/{print $2}' /proc/vmstat); bad=0; i=0; stopAt=0; shown=$n0; seen=""
while [ $i -lt $SECS ]; do
  sleep 2; i=$((i+2))
  in1=$(awk '/pswpin/{print $2}' /proc/vmstat)
  a=$(cs); luna-send -n 1 palm://com.palm.applicationManager/running '{}' >/dev/null 2>&1; lat=$(( ($(cs) - a) * 10 ))
  ld=$(cut -d' ' -f1 /proc/loadavg)
  kids=$(for p in /proc/[0-9]*; do [ "$(awk '{print $4}' $p/stat 2>/dev/null)" = "$DP" ] && echo ${p#/proc/}; done)
  for k in $kids; do case " $seen " in *" $k "*) ;; *) seen="$seen $k"; echo "  player $k at t=$i: nice=$(awk '{print $19}' /proc/$k/stat) oom_adj=$(cat /proc/$k/oom_adj) $(tr '\0' ' ' < /proc/$k/cmdline | cut -c1-60)";; esac; done
  echo "t=$i avail=$(avail) swapin=$((in1 - in0)) load=$ld daemon=$(rss $DP) players=$(rss "$kids") luna=${lat}ms"
  n=$(wc -l < $L); [ $n -gt $shown ] && { tail -n +$((shown + 1)) $L | grep -a -E 'memory (shed|pressure)' | sed 's/^/  LOG /'; shown=$n; }
  [ $stopAt = 0 ] && tail -n +$n0 $L | grep -a -q 'memory shed: stopping' && stopAt=$i
  [ $stopAt != 0 ] && [ $i -ge $((stopAt + 20)) ] && break
  if [ $lat -gt 8000 ] || [ "${ld%%.*}" -ge 14 ]; then bad=$((bad + 1)); else bad=0; fi
  [ $bad -ge 2 ] && { echo "SAFETY: the guard did not hold; closing the card"; break; }
  in0=$in1
done
luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/fl-headroom.png"}' >/dev/null 2>&1; usleep 800000
kids=$(for p in /proc/[0-9]*; do [ "$(awk '{print $4}' $p/stat 2>/dev/null)" = "$DP" ] && echo ${p#/proc/}; done)
c0=$(cs); close_app
# the players left after the card: gone within the engine's 30 s unload delay, and their CPU meanwhile (jiffies of 10 ms)
for k in $kids; do
  j0=$(awk '{print $14+$15}' /proc/$k/stat 2>/dev/null); j0=${j0:-0}; w=0
  while [ -d /proc/$k ] && [ $w -lt 45 ]; do j1=$(awk '{print $14+$15}' /proc/$k/stat 2>/dev/null); sleep 1; w=$((w+1)); done
  if [ -d /proc/$k ]; then echo "  player $k STILL ALIVE 45 s after the card closed, $(( ${j1:-$j0} - j0 )) jiffies meanwhile"
  else echo "  player $k exited $(( ($(cs) - c0) / 100 )) s after the card closed, $(( ${j1:-$j0} - j0 )) jiffies meanwhile"; fi
done
sleep 2
echo "after: avail=$(avail) daemon=$(rss $DP) load=$(cut -d' ' -f1 /proc/loadavg) stopped_at=${stopAt}s"
echo DONE
