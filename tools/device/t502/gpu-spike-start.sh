# T-502 spike: stop the Enyo job, start its daemon by hand with the GPU compositor environment,
# open a page in the card, print what the engine said.
URL="${1:-http://192.168.1.245:8077/jihad-colors.html}"
# $2: extra env for the daemon as KEY=VAL words; a word IOF=<n> sets the GPU io_fraction instead
EXTRA=""; IOF=""
PREFS_ADD=""
GPREFS_ADD=""; GPUENV="JIHAD_GPU_COMPOSITOR=1"
# T-512: the [jihad-gpu] diagnostics are off by default; a hand-started daemon has them (NOLOG=1 not)
LOGENV="JIHAD_GPU_LOG=1"
# novacom splits a quoted "$2" into separate args: every arg after the URL is a word
for w in $(shift; echo "$@"); do case "$w" in IOF=*) IOF="${w#IOF=}";; PREF=*) PREFS_ADD="$PREFS_ADD ${w#PREF=}";; GPREF=*) GPREFS_ADD="$GPREFS_ADD ${w#GPREF=}";; NOGPU=*) GPUENV="";; NOLOG=*) LOGENV="";; *) EXTRA="$EXTRA $w";; esac; done
K=/sys/devices/platform/kgsl/msm_kgsl/kgsl-3d0/io_fraction
[ -n "$IOF" ] && [ -f $K ] && { echo "$IOF" > $K; echo "io_fraction now $(cat $K)"; }
APP=/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser
HL=$APP/deviceroot/hl
STATE=/var/palm/jihad/enyo
LOG=$STATE/daemon-gpu.log
# close the card first: a launch into an open card relays the URL to it, and the old page
# renders under the new daemon before the new one (run 35's scroll page was the colours page)
for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 'net.riverstonerelay.jihad-browser"' | grep processid | sed 's/.*"processid": *"\([0-9]*\)".*/\1/'); do
  luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'"$pid"'"}' >/dev/null 2>&1; echo "closed card process $pid"
done
sleep 2
stop jihad >/dev/null 2>&1; sleep 2
for p in /proc/[0-9]*; do
  c=$(tr '\0' ' ' < $p/cmdline 2>/dev/null)
  case "$c" in *jihad-browserserver*"$HL"*) kill ${p#/proc/} 2>/dev/null;; esac
done
sleep 1
: > "$LOG"
rm -f /tmp/frame.raw /tmp/frames.raw
# T-499 off for the spike: the widget stays at the card's size (no 980x1202 layout viewport, no fit zoom)
PREFS=$APP/profile/prefs.js
grep -q 'jihad.viewport.followOverflow' "$PREFS" 2>/dev/null || echo 'user_pref("jihad.viewport.followOverflow", false);' >> "$PREFS"
# PREF=name=value words: user prefs for this run only (the stop script strips them again)
: > /tmp/spike-prefs.txt
for pv in $PREFS_ADD; do
  n="${pv%%=*}"; v="${pv#*=}"
  sed -i "/\"$n\"/d" "$PREFS"
  case "$v" in true|false|[0-9]*) echo "user_pref(\"$n\", $v);" >> "$PREFS";; *) echo "user_pref(\"$n\", \"$v\");" >> "$PREFS";; esac
  echo "$n" >> /tmp/spike-prefs.txt
  echo "pref for this run: $n = $v"
done
rm -f /tmp/yapserver.jihad-browser
# GPREF=name=value words: a DEFAULT pref (goanna.js, read before the once-only gfx prefs are
# snapshotted) for this run only; the stop script puts the bundle's line back
: > /tmp/spike-gprefs.txt
for pv in $GPREFS_ADD; do
  n="${pv%%=*}"; v="${pv#*=}"
  [ -f "$HL/goanna.js.spike-orig" ] || cp "$HL/goanna.js" "$HL/goanna.js.spike-orig"
  sed -i "/^pref(\"$n\"/d" "$HL/goanna.js"
  echo "pref(\"$n\", $v);" >> "$HL/goanna.js"
  echo "$n" >> /tmp/spike-gprefs.txt
  echo "default pref for this run: $n = $v"
done
cd "$HL" || exit 1
nohup env LD_LIBRARY_PATH="$HL" ICU_DATA="$HL" HOME="$STATE" \
  JIHAD_BS_NAME=jihad-browser JIHAD_OFFSCREEN=1 JIHAD_STATE_DIR="$STATE" \
  $GPUENV $LOGENV JIHAD_GPU_DUMP=/tmp/frame.raw JIHAD_GPU_DUMP_SEQ=/tmp/frames.raw $EXTRA \
  ./ld-2.23.so --library-path "$HL" ./jihad-browserserver "$HL" >> "$LOG" 2>&1 &
echo "daemon started by hand (pid $!)"
sleep 6
luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"net.riverstonerelay.jihad-browser","params":{"target":"'"$URL"'"}}' 2>&1 | head -2
# the screenshot once the daemon has published three content frames (the fit zoom comes late on
# this path and the frames before it are empty), 45 s at most
t=0; n=0
while [ $t -lt 45 ]; do n=$(grep -ac 'painted shmid=0x[0-9a-f]* bytes=[1-9]' "$LOG"); [ "$n" -ge 3 ] && break; sleep 1; t=$((t+1)); done
echo "screenshot at +$t s after the launch: $n content frames published"
echo "--- marker screenshot" >> $STATE/adapter.log 2>/dev/null
luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/st-gpu.png"}' >/dev/null 2>&1; sleep 3
[ $t -lt 12 ] && sleep $((12 - t))
echo "--- [jihad-gpu] lines"
grep -a 'jihad-gpu\|jihad-widget\] create\|CompositorOGL\|GLContext\|EGL\|Failed to create\|OMT compositor\|FATAL\|SIGSEGV\|crash\|clock hold' "$LOG" | head -90
echo "--- pipeline"
grep -a "paint pipeline\|gap hist\|tick period\|seq frame" "$LOG" | tail -n 24
echo "--- daemon alive?"
for p in /proc/[0-9]*; do
  c=$(tr '\0' ' ' < $p/cmdline 2>/dev/null)
  case "$c" in *jihad-browserserver*"$HL"*) echo "pid ${p#/proc/} rss $(grep VmRSS $p/status)";; esac
done
echo "--- log tail"
tail -n 25 "$LOG"
