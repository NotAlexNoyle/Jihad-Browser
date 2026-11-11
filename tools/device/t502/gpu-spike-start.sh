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
XULDIR=""   # T-556: XULDIR=<device dir> puts another libxul.so (the -O2 build) ahead of the bundle's for this run
HLDIR=""    # T-559: HLDIR=<device dir> runs the daemon from another GRE directory (the omni.ja layout) for this run
for w in $(shift; echo "$@"); do case "$w" in HLDIR=*) HLDIR="${w#HLDIR=}";; XULDIR=*) XULDIR="${w#XULDIR=}";; IOF=*) IOF="${w#IOF=}";; PREF=*) PREFS_ADD="$PREFS_ADD ${w#PREF=}";; GPREF=*) GPREFS_ADD="$GPREFS_ADD ${w#GPREF=}";; NOGPU=*) GPUENV="";; NOLOG=*) LOGENV="";; *) EXTRA="$EXTRA $w";; esac; done
K=/sys/devices/platform/kgsl/msm_kgsl/kgsl-3d0/io_fraction
[ -n "$IOF" ] && [ -f $K ] && { echo "$IOF" > $K; echo "io_fraction now $(cat $K)"; }
APP=/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser
HL=$APP/deviceroot/hl
[ -n "$HLDIR" ] && HL=$HLDIR
STATE=/var/palm/jihad/enyo
LOG=$STATE/daemon-gpu.log
# close the card first: a launch into an open card relays the URL to it, and the old page
# renders under the new daemon before the new one (run 35's scroll page was the colours page)
for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 'net.riverstonerelay.jihad-browser"' | grep processid | sed 's/.*"processid": *"\([0-9]*\)".*/\1/'); do
  luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'"$pid"'"}' >/dev/null 2>&1; echo "closed card process $pid"
done
sleep 2
# the adapter starts the job when a card finds no daemon (T-613): not while this hand daemon serves
touch /var/palm/jihad/enyo/hand-daemon
stop jihad >/dev/null 2>&1; sleep 2
for p in /proc/[0-9]*; do
  c=$(tr '\0' ' ' 2>/dev/null < $p/cmdline)
  case "$c" in *jihad-browserserver*"$HL"*) kill ${p#/proc/} 2>/dev/null;; esac
done
# The pref edits below wait for every Enyo daemon to be gone: one still exiting saves its prefs over
# them (2026-10-19: the spike's followOverflow=false was lost that way, T-499 on in a spike). SIGTERM
# had 30 s, then SIGKILL; one still there and the run does not start.
gone() { for p in /proc/[0-9]*; do case "$(tr '\0' ' ' 2>/dev/null < $p/cmdline)" in *jihad-browserserver*"$HL"*) return 1;; esac; done; return 0; }
# /tmp/spike-stop-wait lengthens it, as in gpu-spike-stop.sh: an instrumented engine writes its profile at exit
W=30; [ -s /tmp/spike-stop-wait ] && W=$(cat /tmp/spike-stop-wait)
i=0; while [ $i -lt $W ] && ! gone; do sleep 1; i=$((i+1)); done
if ! gone; then
  echo "an Enyo daemon outlived SIGTERM by $W s: SIGKILL"
  for p in /proc/[0-9]*; do case "$(tr '\0' ' ' 2>/dev/null < $p/cmdline)" in *jihad-browserserver*"$HL"*) kill -9 ${p#/proc/} 2>/dev/null;; esac; done
  i=0; while [ $i -lt 5 ] && ! gone; do sleep 1; i=$((i+1)); done
fi
if ! gone; then echo "FATAL: an Enyo daemon is still running; the spike does not start"; exit 1; fi
: > "$LOG"
rm -f /tmp/frame.raw /tmp/frames.raw
# T-499 off for the spike: the widget stays at the card's size (no 980x1202 layout viewport, no fit zoom)
PREFS=$APP/profile/prefs.js
grep -q 'jihad.viewport.followOverflow' "$PREFS" 2>/dev/null || echo 'user_pref("jihad.viewport.followOverflow", false);' >> "$PREFS"
# PREF=name=value words: user prefs for this run only. Each name goes to /tmp/spike-prefs.txt and the line it replaces
# to /tmp/spike-prefs-orig.txt; the stop script strips the run's lines and puts those back. (Until 2026-11-09 nothing
# stripped them: every lever's pref stayed in the profile, so each interleaved control after the lever's first run
# carried the lever, and later rounds inherited ffvpx off, reduced motion, Skia and script off.) A previous run never
# stopped is undone first.
if [ -s /tmp/spike-prefs.txt ]; then
  while read -r n; do [ -n "$n" ] && sed -i "/\"$n\"/d" "$PREFS"; done < /tmp/spike-prefs.txt
  [ -s /tmp/spike-prefs-orig.txt ] && cat /tmp/spike-prefs-orig.txt >> "$PREFS"
  echo "a previous run's prefs undone: $(tr '\n' ' ' < /tmp/spike-prefs.txt)"
fi
: > /tmp/spike-prefs.txt; : > /tmp/spike-prefs-orig.txt
for pv in $PREFS_ADD; do
  n="${pv%%=*}"; v="${pv#*=}"
  grep "\"$n\"" "$PREFS" >> /tmp/spike-prefs-orig.txt 2>/dev/null
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
  # a string value is quoted here as PREF= quotes it (the Skia round of 2026-11-10 needed "skia")
  case "$v" in true|false|[0-9]*|-[0-9]*|\"*) echo "pref(\"$n\", $v);" >> "$HL/goanna.js";; *) echo "pref(\"$n\", \"$v\");" >> "$HL/goanna.js";; esac
  echo "$n" >> /tmp/spike-gprefs.txt
  echo "default pref for this run: $n = $v"
done
cd "$HL" || exit 1
nohup env LD_LIBRARY_PATH="$HL" ICU_DATA="$HL" HOME="$STATE" \
  JIHAD_BS_NAME=jihad-browser JIHAD_OFFSCREEN=1 JIHAD_STATE_DIR="$STATE" \
  $GPUENV $LOGENV JIHAD_GPU_DUMP=/tmp/frame.raw JIHAD_GPU_DUMP_SEQ=/tmp/frames.raw $EXTRA \
  ./ld-2.23.so --library-path "${XULDIR:+$XULDIR:}$HL" ./jihad-browserserver "$HL" >> "$LOG" 2>&1 &
echo "daemon started by hand (pid $!)${XULDIR:+, libxul from $XULDIR}"
# the card once the engine is up (6 s at least, as before): an instrumented PGO engine takes far longer, and a card
# launched before it connects and leaves with no page (2026-11-10: every profiling row came back empty)
t=0; while [ $t -lt 240 ] && ! grep -aq 'engine up' "$LOG"; do sleep 1; t=$((t+1)); done
[ $t -lt 6 ] && sleep $((6 - t))
if [ $t -ge 240 ]; then echo "!! the engine is not up after 240 s"; elif [ $t -ge 6 ]; then echo "engine up after $t s"; fi
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
  c=$(tr '\0' ' ' 2>/dev/null < $p/cmdline)
  case "$c" in *jihad-browserserver*"$HL"*) echo "pid ${p#/proc/} rss $(grep VmRSS $p/status)";; esac
done
echo "--- log tail"
tail -n 25 "$LOG"
