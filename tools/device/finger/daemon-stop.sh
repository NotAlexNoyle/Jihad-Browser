# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# daemon-stop.sh VARIANT [log]: the hand daemon of daemon-start.sh stopped, the run's prefs stripped, the clock floor
# put back, the upstart job started again; with "log" the daemon log between "=== LOG BEGIN/END" marks first.
V=${1:?variant}
case $V in
  enyo)  ID=net.riverstonerelay.jihad-browser;       JOB=jihad;;
  mochi) ID=net.riverstonerelay.jihad-browser-mochi; JOB=jihad-mochi;;
  mojo)  ID=net.riverstonerelay.jihad-browser-mojo;  JOB=jihad-mojo;;
  *) echo "daemon-stop: unknown variant $V"; exit 2;;
esac
APP=/media/cryptofs/apps/usr/palm/applications/$ID; HL=$APP/deviceroot/hl; STATE=/var/palm/jihad/$V; LOG=$STATE/daemon-gpu.log
[ "${2:-}" = log ] && { echo "=== LOG BEGIN"; cat $LOG; echo "=== LOG END"; }
# a round's device script left behind when its session was cut short (2026-11-04: a walk's script looped and its
# session was killed from the host) — every /tmp/dev-sh.* script but this one
for p in /proc/[0-9]*; do [ "${p#/proc/}" = "$$" ] && continue; c=$(tr '\0' ' ' < $p/cmdline 2>/dev/null); case "$c" in *"/tmp/dev-sh."*) kill ${p#/proc/} 2>/dev/null && echo "killed a leftover device script: $(echo "$c" | cut -c1-60)";; esac; done
for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 "\"$ID\"" | grep processid | sed 's/.*"processid": *"\([0-9]*\)".*/\1/'); do
  luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'"$pid"'"}' >/dev/null 2>&1
done
gone() { for p in /proc/[0-9]*; do case "$(tr '\0' ' ' 2>/dev/null < $p/cmdline)" in *jihad-browserserver*"$HL"*) return 1;; esac; done; return 0; }
for p in /proc/[0-9]*; do case "$(tr '\0' ' ' 2>/dev/null < $p/cmdline)" in *jihad-browserserver*"$HL"*) kill ${p#/proc/} 2>/dev/null;; esac; done
i=0; while [ $i -lt 30 ] && ! gone; do sleep 1; i=$((i+1)); done
if ! gone; then for p in /proc/[0-9]*; do case "$(tr '\0' ' ' 2>/dev/null < $p/cmdline)" in *jihad-browserserver*"$HL"*) kill -9 ${p#/proc/} 2>/dev/null;; esac; done; sleep 2; fi
gone || { echo "ERROR: the $V hand daemon is still running; the profile and the job are left as they are"; exit 1; }
for c in 0 1; do d=/sys/devices/system/cpu/cpu$c/cpufreq; [ -d $d ] && cat $d/cpuinfo_min_freq > $d/scaling_min_freq 2>/dev/null; done
PREFS=$APP/profile/prefs.js
if [ -s /tmp/fl-prefs-$V.txt ]; then while read n; do [ -n "$n" ] && sed -i "/\"$n\"/d" "$PREFS" && echo "pref stripped: $n"; done < /tmp/fl-prefs-$V.txt; : > /tmp/fl-prefs-$V.txt; fi
rm -f /tmp/fl-region.last.raw
stop $JOB >/dev/null 2>&1; sleep 1; start $JOB >/dev/null 2>&1; sleep 4
n=0; for p in /proc/[0-9]*; do case "$(tr '\0' ' ' 2>/dev/null < $p/cmdline)" in *jihad-browserserver*"$HL"*) n=$((n+1));; esac; done
echo "$V: $(status $JOB 2>&1); daemons running: $n"
