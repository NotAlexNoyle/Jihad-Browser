# T-502 spike: kill the hand-started Enyo daemon and put the upstart job back.
APP=/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser
HL=$APP/deviceroot/hl
for p in /proc/[0-9]*; do
  c=$(tr '\0' ' ' < $p/cmdline 2>/dev/null)
  case "$c" in *jihad-browserserver*"$HL"*) kill ${p#/proc/} 2>/dev/null; echo "killed ${p#/proc/}";; esac
done
sleep 2
# a daemon killed by hand leaves the clock floor at the top: put it back
for c in 0 1; do d=/sys/devices/system/cpu/cpu$c/cpufreq; [ -d $d ] && cat $d/cpuinfo_min_freq > $d/scaling_min_freq 2>/dev/null; done
sed -i '/jihad.viewport.followOverflow/d' $APP/profile/prefs.js 2>/dev/null
# the run's PREF= lines stripped and the lines they replaced put back (gpu-spike-start.sh); GPREF= words: the bundle's
# goanna.js put back (2026-11-09: neither was done, and every lever's pref stayed for the runs after it)
if [ -s /tmp/spike-prefs.txt ]; then
  while read -r n; do [ -n "$n" ] && sed -i "/\"$n\"/d" $APP/profile/prefs.js; done < /tmp/spike-prefs.txt
  [ -s /tmp/spike-prefs-orig.txt ] && cat /tmp/spike-prefs-orig.txt >> $APP/profile/prefs.js
  echo "prefs of the run stripped: $(tr '\n' ' ' < /tmp/spike-prefs.txt)"
fi
rm -f /tmp/spike-prefs.txt /tmp/spike-prefs-orig.txt
[ -f $HL/goanna.js.spike-orig ] && mv -f $HL/goanna.js.spike-orig $HL/goanna.js && echo "the bundle's goanna.js put back"
rm -f /tmp/spike-gprefs.txt
stop jihad >/dev/null 2>&1; sleep 1; start jihad >/dev/null 2>&1; sleep 4
status jihad
n=0; for p in /proc/[0-9]*; do c=$(tr '\0' ' ' < $p/cmdline 2>/dev/null); case "$c" in *jihad-browserserver*"$HL"*) n=$((n+1));; esac; done
echo "enyo daemons running: $n"
