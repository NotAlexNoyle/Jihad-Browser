echo "=== LOG BEGIN"
cat /var/palm/jihad/enyo/daemon-gpu.log
echo "=== LOG END"
# T-502 spike: kill the hand-started Enyo daemon and put the upstart job back.
APP=/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser
HL=$APP/deviceroot/hl
PREFS=$APP/profile/prefs.js
gone() { for p in /proc/[0-9]*; do case "$(tr '\0' ' ' 2>/dev/null < $p/cmdline)" in *jihad-browserserver*"$HL"*) return 1;; esac; done; return 0; }
killall_hl() { for p in /proc/[0-9]*; do case "$(tr '\0' ' ' 2>/dev/null < $p/cmdline)" in *jihad-browserserver*"$HL"*) kill $1 ${p#/proc/} 2>/dev/null; echo "killed ${p#/proc/} $1";; esac; done; }
killall_hl
# The daemon saves its prefs on the way out: the profile is touched only once it is gone, or the
# spike's followOverflow=false came back and stayed in the shipped profile (found 2026-10-19: T-499
# was off on the Enyo card for hours after a spike). SIGTERM first (a daemon in the GPU driver), then
# SIGKILL; still there, the profile is left alone and the upstart job is not started over it.
i=0; while [ $i -lt 30 ] && ! gone; do sleep 1; i=$((i+1)); done
if ! gone; then echo "the hand-started daemon outlived SIGTERM by 30 s: SIGKILL"; killall_hl -9; i=0; while [ $i -lt 5 ] && ! gone; do sleep 1; i=$((i+1)); done; fi
if ! gone; then echo "ERROR: the hand-started daemon is still running; the profile and the upstart job are left as they are"; exit 1; fi
# a daemon killed by hand leaves the clock floor at the top: put it back
for c in 0 1; do d=/sys/devices/system/cpu/cpu$c/cpufreq; [ -d $d ] && cat $d/cpuinfo_min_freq > $d/scaling_min_freq 2>/dev/null; done
# the run's own settings go before the upstart daemon starts (one that starts on them saves them back)
sed -i '/jihad.viewport.followOverflow/d' "$PREFS" 2>/dev/null
# the prefs a run added (PREF= words)
if [ -s /tmp/spike-prefs.txt ]; then
  while read n; do [ -n "$n" ] && sed -i "/\"$n\"/d" "$PREFS" && echo "pref stripped: $n"; done < /tmp/spike-prefs.txt
  : > /tmp/spike-prefs.txt
fi
# the default prefs a run changed (GPREF= words): the bundle's goanna.js back
if [ -f "$HL/goanna.js.spike-orig" ]; then mv "$HL/goanna.js.spike-orig" "$HL/goanna.js" && echo "goanna.js restored"; fi
: > /tmp/spike-gprefs.txt
stop jihad >/dev/null 2>&1; sleep 1; start jihad >/dev/null 2>&1; sleep 4
status jihad
n=0; for p in /proc/[0-9]*; do c=$(tr '\0' ' ' 2>/dev/null < $p/cmdline); case "$c" in *jihad-browserserver*"$HL"*) n=$((n+1));; esac; done
echo "enyo daemons running: $n"
