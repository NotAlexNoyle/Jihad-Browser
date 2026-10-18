echo "=== LOG BEGIN"
cat /var/palm/jihad/enyo/daemon-gpu.log
echo "=== LOG END"
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
stop jihad >/dev/null 2>&1; sleep 1; start jihad >/dev/null 2>&1; sleep 4
status jihad
n=0; for p in /proc/[0-9]*; do c=$(tr '\0' ' ' < $p/cmdline 2>/dev/null); case "$c" in *jihad-browserserver*"$HL"*) n=$((n+1));; esac; done
echo "enyo daemons running: $n"

# the prefs a run added (PREF= words) go again, now that the daemon is stopped
PREFS=/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/profile/prefs.js
if [ -s /tmp/spike-prefs.txt ]; then
  while read n; do [ -n "$n" ] && sed -i "/\"$n\"/d" "$PREFS" && echo "pref stripped: $n"; done < /tmp/spike-prefs.txt
  : > /tmp/spike-prefs.txt
fi

# the default prefs a run changed (GPREF= words): the bundle's goanna.js back
HL=/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/deviceroot/hl
if [ -f "$HL/goanna.js.spike-orig" ]; then mv "$HL/goanna.js.spike-orig" "$HL/goanna.js" && echo "goanna.js restored"; fi
: > /tmp/spike-gprefs.txt
