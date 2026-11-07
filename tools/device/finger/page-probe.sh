# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# page-probe.sh URL [SECS] — cavekit-device-headroom.md R2 on the device: the Enyo card on URL; every 5 s the load, the job
# daemon's memory and thread count, free memory and free swap, and its busiest threads over those 5 s (jiffies of 10 ms:
# 500 is a full core); the card closed at the end and the memory after. With jihad.blocking.trace on, the daemon log also
# lists every load the page asked for. Run through build/webos-oe/dev-sh.sh.
URL=$1; SECS=${2:-60}; app=net.riverstonerelay.jihad-browser
close_all() { for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 "\"$app\"" | grep processid | sed 's/.*"processid": *"\([^"]*\)".*/\1/'); do luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'"$pid"'"}' >/dev/null 2>&1; done; }
DP=$(status jihad | sed -n 's/.*process \([0-9]*\).*/\1/p'); echo "daemon $DP"
snap() { for t in /proc/$DP/task/*; do s=$(cat $t/stat 2>/dev/null) || continue; c=$(echo "$s" | sed 's/^[0-9]* (\(.*\)) .*/\1/' | tr ' ' '_'); r=$(echo "$s" | sed 's/^.*) //'); set -- $r; echo "${t##*/} $c $(( ${12} + ${13} ))"; done; }
close_all; sleep 1
snap > /tmp/yp.0
luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"'$app'","params":{"target":"'"$URL"'"}}' >/dev/null
i=0; while [ $i -lt $SECS ]; do
  sleep 5; i=$((i+5)); snap > /tmp/yp.1
  echo "t=$i load $(cut -d' ' -f1 /proc/loadavg) rss $(grep VmRSS /proc/$DP/status | tr -s ' ' | cut -d' ' -f2) threads $(ls /proc/$DP/task | wc -l) free $(grep MemFree /proc/meminfo | tr -s ' ' | cut -d' ' -f2) swapfree $(grep SwapFree /proc/meminfo | tr -s ' ' | cut -d' ' -f2)"
  # the busiest threads over the last 5 s (jiffies of 10 ms)
  awk 'NR==FNR { a[$1]=$3; next } { d=$3-a[$1]; if (d > 20) printf "   %s %s %d\n", $1, $2, d }' /tmp/yp.0 /tmp/yp.1 | sort -k3 -n -r | head -6
  mv /tmp/yp.1 /tmp/yp.0
  [ "$(cut -d. -f1 /proc/loadavg)" -ge 14 ] && { echo "load 14 or more: stopping early"; break; }
done
luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/fl-yt.png"}' >/dev/null 2>&1; usleep 800000
close_all; sleep 5
echo "after close: rss $(grep VmRSS /proc/$DP/status | tr -s ' ' | cut -d' ' -f2) load $(cut -d' ' -f1 /proc/loadavg)"
rm -f /tmp/yp.0 /tmp/yp.1
