# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# daemon-start.sh VARIANT URL [env words]: the shell's daemon started by hand for a finger round (T-563), as the
# shipped job starts it (the GPU path with the express lane) plus the inject channel and the [jihad-gpu] log, on
# /var/palm/jihad/$V/daemon-gpu.log; its card launched on URL and three content frames awaited. PREF=name=value
# words are prefs for this run; the profile's prefs.js is copied first and daemon-stop.sh puts the copy back whole.
# Runs on the device (build/webos-oe/dev-sh.sh).
V=${1:?variant}; URL=${2:?url}; shift 2
case $V in
  enyo)  ID=net.riverstonerelay.jihad-browser;       JOB=jihad;       NAME=jihad-browser;;
  mochi) ID=net.riverstonerelay.jihad-browser-mochi; JOB=jihad-mochi; NAME=jihad-browser-mochi;;
  mojo)  ID=net.riverstonerelay.jihad-browser-mojo;  JOB=jihad-mojo;  NAME=jihad-browser-mojo;;
  *) echo "daemon-start: unknown variant $V"; exit 2;;
esac
APP=/media/cryptofs/apps/usr/palm/applications/$ID; HL=$APP/deviceroot/hl; STATE=/var/palm/jihad/$V; LOG=$STATE/daemon-gpu.log
EXTRA=""; PREFS_ADD=""
for w in "$@"; do case "$w" in PREF=*) PREFS_ADD="$PREFS_ADD ${w#PREF=}";; *) EXTRA="$EXTRA $w";; esac; done
luna-send -n 1 palm://com.palm.display/control/setState '{"state":"on"}' >/dev/null 2>&1
for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 "\"$ID\"" | grep processid | sed 's/.*"processid": *"\([0-9]*\)".*/\1/'); do
  luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'"$pid"'"}' >/dev/null 2>&1; echo "closed card process $pid"
done
sleep 2
stop $JOB >/dev/null 2>&1; sleep 2
gone() { for p in /proc/[0-9]*; do case "$(tr '\0' ' ' 2>/dev/null < $p/cmdline)" in *jihad-browserserver*"$HL"*) return 1;; esac; done; return 0; }
for p in /proc/[0-9]*; do case "$(tr '\0' ' ' 2>/dev/null < $p/cmdline)" in *jihad-browserserver*"$HL"*) kill ${p#/proc/} 2>/dev/null;; esac; done
i=0; while [ $i -lt 30 ] && ! gone; do sleep 1; i=$((i+1)); done
if ! gone; then for p in /proc/[0-9]*; do case "$(tr '\0' ' ' 2>/dev/null < $p/cmdline)" in *jihad-browserserver*"$HL"*) kill -9 ${p#/proc/} 2>/dev/null;; esac; done; sleep 2; fi
gone || { echo "FATAL: a $V daemon is still running"; exit 1; }
PREFS=$APP/profile/prefs.js
rm -f "$STATE"/dialog-*.fifo   # a daemon killed with a dialog up leaves its reply pipe, and the dialog check reads those
if [ "${KEEP:-0}" != 1 ]; then
  : > "$LOG"
  # the profile's prefs as found, put back whole by daemon-stop.sh: a round leaves them as they were whatever it did
  cp "$PREFS" /tmp/fl-prefs-before-$V 2>/dev/null
else echo "--- restart (the profile and the log kept)" >> "$LOG"; fi
# what this start waits for, read from this start's lines only: with KEEP the last daemon's lines read as ready at +0 s,
# and a script's next inject then blocked on a pipe with no reader (settings-persist on Mochi, 417 s, 2026-11-05)
LOG0=$(($(wc -l < "$LOG") + 1)); since() { tail -n +$LOG0 "$LOG"; }
: > /tmp/fl-prefs-$V.txt
for pv in $PREFS_ADD; do
  n="${pv%%=*}"; v="${pv#*=}"; sed -i "/\"$n\"/d" "$PREFS"
  # bare only for a boolean or an all-digit number: an address like 192.168.1.245 is a string (it broke prefs.js, 2026-11-05)
  case "$v" in true|false) echo "user_pref(\"$n\", $v);" >> "$PREFS";; ""|*[!0-9]*) echo "user_pref(\"$n\", \"$v\");" >> "$PREFS";; *) echo "user_pref(\"$n\", $v);" >> "$PREFS";; esac
  echo "$n" >> /tmp/fl-prefs-$V.txt; echo "pref for this run: $n = $v"
done
rm -f /tmp/yapserver.$NAME
cd "$HL" || exit 1
nohup env LD_LIBRARY_PATH="$HL" ICU_DATA="$HL" HOME="$STATE" JIHAD_BS_NAME=$NAME JIHAD_OFFSCREEN=1 JIHAD_DISABLE_OMTC=1 \
  JIHAD_STATE_DIR="$STATE" JIHAD_GPU_COMPOSITOR=1 JIHAD_GPU_EXPRESS=1 JIHAD_GPU_LOG=1 JIHAD_INJECT=1 $EXTRA \
  ./ld-2.23.so --library-path "$HL" ./jihad-browserserver "$HL" >> "$LOG" 2>&1 &
echo "$V daemon started by hand (pid $!)"
# the card only once the daemon serves YAP (a card that connects while the engine is still coming up — 5-10 s, longer
# with another shell's job daemon starting beside it — loses its handshake and shows no page: 2026-11-04)
t=0; while [ $t -lt 60 ] && ! since | grep -aq 'engine up; serving YAP'; do sleep 1; t=$((t+1)); done
since | grep -aq 'engine up; serving YAP' || { echo "FATAL: the $V daemon did not serve YAP within 60 s"; tail -n 5 "$LOG"; exit 1; }
echo "$V daemon serving YAP at +$t s"
luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"'"$ID"'","params":{"target":"'"$URL"'"}}' >/dev/null 2>&1
t=0; while [ $t -lt 20 ] && ! since | grep -aq 'client connected'; do sleep 1; t=$((t+1)); done
since | grep -aq 'client connected' || echo "WARNING: no card connected to the $V daemon within 20 s"
# the page's load done (a card beside another shell's job daemon coming up loads slowly), then three content frames
t2=0; while [ $t2 -lt 60 ] && ! since | grep -aq 'load done uri='; do sleep 1; t2=$((t2+1)); done
since | grep -aq 'load done uri=' || echo "WARNING: no load done within 60 s"
t3=0; n=0
while [ $t3 -lt 30 ]; do n=$(since | grep -ac 'painted shmid=0x[0-9a-f]* bytes=[1-9][0-9]'); [ "$n" -ge 3 ] && break; sleep 1; t3=$((t3+1)); done
echo "$V card on $URL: connected at +$t s, load done at +$((t + t2)) s, $n content frames at +$((t + t2 + t3)) s; $(grep -a 'rendering:' $LOG | tail -1 | sed 's/.*rendering: //')"
grep -a 'FATAL\|SIGSEGV' "$LOG" | head -3
