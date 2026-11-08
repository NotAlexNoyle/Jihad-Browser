# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# atlas-probe.sh URL [SECS] — cavekit-device-headroom.md R2's reference on the device: Atlas (org.webosports.app.atlas,
# WPE WebKit) on URL for SECS (60); every 5 s the load, free memory and swap, and for Atlas's engine processes
# (BrowserServer-atlas and its children: the WPE web and network processes) their resident memory and CPU over the
# last 5 s (jiffies of 10 ms; 500 is a core); the card closed at the end. The same numbers page-probe.sh gives for
# Jihad's Enyo daemon. Through build/webos-oe/dev-sh.sh.
URL=$1; SECS=${2:-60}; app=org.webosports.app.atlas
close_all() { for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 "\"$app\"" | grep processid | sed 's/.*"processid": *"\([^"]*\)".*/\1/'); do luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'"$pid"'"}' >/dev/null 2>&1; done; }
AP=$(pgrep -f BrowserServer-atlas | head -1); echo "atlas daemon $AP"
procs() { echo $AP; for p in /proc/[0-9]*; do [ "$(awk '{print $4}' $p/stat 2>/dev/null)" = "$AP" ] && echo ${p#/proc/}; done; }
snap() { for p in $(procs); do s=$(cat /proc/$p/stat 2>/dev/null) || continue; c=$(echo "$s" | sed 's/^[0-9]* (\(.*\)) .*/\1/' | tr ' ' '_'); r=$(echo "$s" | sed 's/^.*) //'); set -- $r; echo "$p $c $(( ${12} + ${13} )) $(grep VmRSS /proc/$p/status 2>/dev/null | tr -s ' ' | cut -d' ' -f2)"; done; }
close_all; sleep 2
snap > /tmp/ap.0
luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"'$app'","params":{"target":"'"$URL"'"}}' >/dev/null
i=0; while [ $i -lt $SECS ]; do
  sleep 5; i=$((i+5)); snap > /tmp/ap.1
  rss=$(awk '{s+=$4} END{print s+0}' /tmp/ap.1)
  echo "t=$i load $(cut -d' ' -f1 /proc/loadavg) rss $rss procs $(wc -l < /tmp/ap.1) free $(grep MemFree /proc/meminfo | tr -s ' ' | cut -d' ' -f2) swapfree $(grep SwapFree /proc/meminfo | tr -s ' ' | cut -d' ' -f2)"
  awk 'NR==FNR { a[$1]=$3; next } { d=$3-(($1 in a)?a[$1]:0); if (d > 20) printf "   %s %s %d rss %s\n", $1, $2, d, $4 }' /tmp/ap.0 /tmp/ap.1 | sort -k3 -n -r | head -5
  mv /tmp/ap.1 /tmp/ap.0
  [ "$(cut -d. -f1 /proc/loadavg)" -ge 14 ] && { echo "load 14 or more: stopping early"; break; }
done
luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/fl-atlas.png"}' >/dev/null 2>&1; usleep 800000
close_all; sleep 5
echo "after close: $(snap | awk '{s+=$4} END{print s+0}') kB in $(procs | wc -l) processes, load $(cut -d' ' -f1 /proc/loadavg)"
rm -f /tmp/ap.0 /tmp/ap.1
