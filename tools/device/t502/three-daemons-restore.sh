# T-509: the hand-started daemons of three-daemons.sh stopped and the three upstart jobs put back.
for V in enyo mochi mojo; do
  case $V in enyo) ID=net.riverstonerelay.jihad-browser; JOB=jihad;; mochi) ID=net.riverstonerelay.jihad-browser-mochi; JOB=jihad-mochi;; mojo) ID=net.riverstonerelay.jihad-browser-mojo; JOB=jihad-mojo;; esac
  HL=/media/cryptofs/apps/usr/palm/applications/$ID/deviceroot/hl
  for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 "\"$ID\"" | grep processid | sed 's/.*"processid": *"\([0-9]*\)".*/\1/'); do
    luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'"$pid"'"}' >/dev/null 2>&1
  done
  for p in /proc/[0-9]*; do c=$(tr '\0' ' ' < $p/cmdline 2>/dev/null); case "$c" in *jihad-browserserver*"$HL"*) kill ${p#/proc/} 2>/dev/null;; esac; done
done
sleep 2
for c in 0 1; do d=/sys/devices/system/cpu/cpu$c/cpufreq; [ -d $d ] && cat $d/cpuinfo_min_freq > $d/scaling_min_freq 2>/dev/null; done
for JOB in jihad jihad-mochi jihad-mojo; do stop $JOB >/dev/null 2>&1; start $JOB >/dev/null 2>&1; done
sleep 4
for JOB in jihad jihad-mochi jihad-mojo; do echo "$(status $JOB)"; done
