# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# headroom-stress.sh [SECS] — cavekit-device-headroom.md R1's stress on the device: Learn to Fly 3 (17 MB, the heaviest
# movie of the corpus, flash-serve.py on the host) played in a Mochi card, and YouTube's watch page playing in an Enyo
# card; every 5 s the available memory, the pages swapped in, the load, each daemon's and the players' memory, the
# application manager's answer time and the guards' lines from both daemons' logs. Both cards are closed at the end
# (600 s), or at once if the application manager takes over 8 s or the load reaches 14 twice running. Run it detached;
# the host times a novacom command every 10 s beside it (the USB link's half of the criterion).
SECS=${1:-600}; E=net.riverstonerelay.jihad-browser; M=net.riverstonerelay.jihad-browser-mochi
close_app() { for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 "\"$1\"" | grep processid | sed 's/.*"processid": *"\([^"]*\)".*/\1/'); do luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'"$pid"'"}' >/dev/null 2>&1; done; }
avail() { awk '/^MemFree:/{f=$2} /^Buffers:/{b=$2} /^Cached:/{c=$2} /^Shmem:/{s=$2} END{print f+b+c-s}' /proc/meminfo; }
rssp() { v=$(grep VmRSS /proc/$1/status 2>/dev/null | tr -s ' ' | cut -d' ' -f2); echo ${v:-0}; }
cs() { cut -d' ' -f1 /proc/uptime | tr -d '.'; }
EP=$(status jihad | sed -n 's/.*process \([0-9]*\).*/\1/p'); MP=$(status jihad-mochi | sed -n 's/.*process \([0-9]*\).*/\1/p')
LE=/var/palm/jihad/enyo/daemon.log; LM=/var/palm/jihad/mochi/daemon.log; ne=$(wc -l < $LE); nm=$(wc -l < $LM)
close_app $E; close_app $M; sleep 2
luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"'$M'","params":{"target":"http://192.168.1.245:8097/title/learn-to-fly-3"}}' >/dev/null
sleep 50; /tmp/finger tap 353 413; sleep 8
luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"'$E'","params":{"target":"https://m.youtube.com/watch?v=ZgZM0ewM4lU"}}' >/dev/null
sleep 30; /tmp/finger tap 380 344
in0=$(awk '/pswpin/{print $2}' /proc/vmstat); bad=0; i=0
while [ $i -lt $SECS ]; do
  sleep 5; i=$((i+5))
  in1=$(awk '/pswpin/{print $2}' /proc/vmstat)
  a=$(cs); luna-send -n 1 palm://com.palm.applicationManager/running '{}' >/dev/null 2>&1; lat=$(( ($(cs) - a) * 10 ))
  ld=$(cut -d' ' -f1 /proc/loadavg); pl=0; for p in $(pgrep -f plugin-container); do pl=$((pl + $(rssp $p))); done
  echo "t=$i avail=$(avail) swapin=$((in1 - in0)) load=$ld enyo=$(rssp $EP) mochi=$(rssp $MP) players=$pl luna=${lat}ms"
  for f in $LE $LM; do n=$(wc -l < $f); [ $f = $LE ] && o=$ne || o=$nm; [ $n -gt $o ] && tail -n +$((o + 1)) $f | grep -a -E 'memory (shed|pressure)' | sed "s|^|  ${f#/var/palm/jihad/} |"; [ $f = $LE ] && ne=$n || nm=$n; done
  if [ $lat -gt 8000 ] || [ "${ld%%.*}" -ge 14 ]; then bad=$((bad + 1)); else bad=0; fi
  [ $bad -ge 2 ] && { echo "SAFETY: the guard did not hold; closing both cards"; break; }
  in0=$in1
done
luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/fl-hstress.png"}' >/dev/null 2>&1; usleep 800000
close_app $E; close_app $M; sleep 8
echo "after: avail=$(avail) enyo=$(rssp $EP) mochi=$(rssp $MP) load=$(cut -d' ' -f1 /proc/loadavg)"
echo DONE
