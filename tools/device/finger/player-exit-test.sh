# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# player-exit-test.sh VARIANT URL [HOLD] — cavekit-device-headroom.md R1 on the device: a card on URL (a page with a Flash
# movie), its player awaited and its priority read, HOLD s (20) of play, then the card closed and the player watched
# for 45 s: when it exits (the engine's unload delay is 30 s, dom.ipc.plugins.unloadTimeoutSecs) and the CPU it used
# between the card's close and its exit (jiffies of 10 ms). Through build/webos-oe/dev-sh.sh.
V=${1:?variant}; URL=${2:?url}; HOLD=${3:-20}
case $V in enyo) ID=net.riverstonerelay.jihad-browser;; mochi) ID=net.riverstonerelay.jihad-browser-mochi;; mojo) ID=net.riverstonerelay.jihad-browser-mojo;; *) exit 2;; esac
close_app() { for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 "\"$ID\"" | grep processid | sed 's/.*"processid": *"\([^"]*\)".*/\1/'); do luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'"$pid"'"}' >/dev/null 2>&1; done; }
cs() { cut -d' ' -f1 /proc/uptime | tr -d '.'; }
jif() { awk '{print $14+$15}' /proc/$1/stat 2>/dev/null; }
DP=$(status jihad$( [ $V = enyo ] || echo -$V ) | sed -n 's/.*process \([0-9]*\).*/\1/p')
kids() { for p in /proc/[0-9]*; do [ "$(awk '{print $4}' $p/stat 2>/dev/null)" = "$DP" ] && echo ${p#/proc/}; done; }
close_app; sleep 2
[ -n "$(kids)" ] && echo "a player was already alive before the card: $(kids)"
luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"'$ID'","params":{"target":"'"$URL"'"}}' >/dev/null
t=0; while [ $t -lt 60 ] && [ -z "$(kids)" ]; do sleep 1; t=$((t+1)); done
K=$(kids); [ -n "$K" ] || { echo "no player within 60 s"; close_app; exit 1; }
for k in $K; do echo "player $k up at +$t s: nice=$(awk '{print $19}' /proc/$k/stat) oom_adj=$(cat /proc/$k/oom_adj)"; done
sleep $HOLD
for k in $K; do echo "player $k after $HOLD s of the movie: $(jif $k) jiffies, rss $(grep VmRSS /proc/$k/status | tr -s ' ' | cut -d' ' -f2) kB"; done
c0=$(cs); close_app
for k in $K; do
  j0=$(jif $k); j0=${j0:-0}; j1=$j0; w=0
  while [ -d /proc/$k ] && [ $w -lt 45 ]; do j=$(jif $k); [ -n "$j" ] && j1=$j; sleep 1; w=$((w+1)); done
  if [ -d /proc/$k ]; then echo "player $k STILL ALIVE 45 s after the card closed: $((j1 - j0)) jiffies meanwhile"
  else echo "player $k exited $(( ($(cs) - c0) / 100 )) s after the card closed: $((j1 - j0)) jiffies meanwhile"; fi
done
echo DONE
