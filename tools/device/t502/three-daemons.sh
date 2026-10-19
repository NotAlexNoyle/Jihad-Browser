# T-509 (cavekit-gpu-rendering R10): the three variants' daemons started by hand at once, on the
# GPU path ($1 = gpu) or the software path ($1 = soft), each card on a real page; then the device's
# free memory, each daemon's resident memory and its GPU mappings (/dev/kgsl in smaps), the frames
# each published, its CPU over 30 s of an idle page, and whether LunaSysMgr kept its EGL.
# three-daemons-restore.sh puts the upstart jobs back.
MODE=${1:-gpu}
for V in enyo mochi mojo; do
  case $V in
    enyo) ID=net.riverstonerelay.jihad-browser; JOB=jihad; NAME=jihad-browser; URL="https://en.wikipedia.org/wiki/HP_TouchPad";;
    mochi) ID=net.riverstonerelay.jihad-browser-mochi; JOB=jihad-mochi; NAME=jihad-browser-mochi; URL="https://news.ycombinator.com/";;
    mojo) ID=net.riverstonerelay.jihad-browser-mojo; JOB=jihad-mojo; NAME=jihad-browser-mojo; URL="http://192.168.1.245:8077/jihad-long-static.html";;
  esac
  APP=/media/cryptofs/apps/usr/palm/applications/$ID; HL=$APP/deviceroot/hl; STATE=/var/palm/jihad/$V
  for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 "\"$ID\"" | grep processid | sed 's/.*"processid": *"\([0-9]*\)".*/\1/'); do
    luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'"$pid"'"}' >/dev/null 2>&1
  done
  stop $JOB >/dev/null 2>&1
  for p in /proc/[0-9]*; do c=$(tr '\0' ' ' < $p/cmdline 2>/dev/null); case "$c" in *jihad-browserserver*"$HL"*) kill ${p#/proc/} 2>/dev/null;; esac; done
  sleep 1
  : > $STATE/daemon-gpu.log
  if [ "$MODE" = gpu ]; then GPUENV="JIHAD_GPU_COMPOSITOR=1"; else GPUENV="JIHAD_DISABLE_OMTC=1"; fi
  ( cd "$HL" && nohup env LD_LIBRARY_PATH="$HL" ICU_DATA="$HL" HOME="$STATE" JIHAD_BS_NAME=$NAME JIHAD_OFFSCREEN=1 \
      JIHAD_STATE_DIR="$STATE" $GPUENV ./ld-2.23.so --library-path "$HL" ./jihad-browserserver "$HL" >> $STATE/daemon-gpu.log 2>&1 & )
done
sleep 6
for V in enyo mochi mojo; do
  case $V in
    enyo) ID=net.riverstonerelay.jihad-browser; URL="https://en.wikipedia.org/wiki/HP_TouchPad";;
    mochi) ID=net.riverstonerelay.jihad-browser-mochi; URL="https://news.ycombinator.com/";;
    mojo) ID=net.riverstonerelay.jihad-browser-mojo; URL="http://192.168.1.245:8077/jihad-long-static.html";;
  esac
  luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"'"$ID"'","params":{"target":"'"$URL"'"}}' >/dev/null 2>&1
  sleep 4
done
sleep 45
echo "mode $MODE"
grep 'MemTotal\|MemFree\|^Cached\|SwapFree' /proc/meminfo | tr -s ' ' | tr '\n' ' '; echo
for V in enyo mochi mojo; do
  case $V in enyo) ID=net.riverstonerelay.jihad-browser;; mochi) ID=net.riverstonerelay.jihad-browser-mochi;; mojo) ID=net.riverstonerelay.jihad-browser-mojo;; esac
  HL=/media/cryptofs/apps/usr/palm/applications/$ID/deviceroot/hl; STATE=/var/palm/jihad/$V
  for p in /proc/[0-9]*; do c=$(tr '\0' ' ' < $p/cmdline 2>/dev/null); case "$c" in *jihad-browserserver*"$HL"*) PID=${p#/proc/};; esac; done
  [ -z "$PID" ] && { echo "$V: NO DAEMON"; continue; }
  RSS=$(grep VmRSS /proc/$PID/status | tr -s ' ' | cut -d' ' -f2)
  KG=$(awk '/kgsl/ {k=1; next} /^[0-9a-f]+-[0-9a-f]+ / {k=0} k && /^Rss:/ {s+=$2} END {print s+0}' /proc/$PID/smaps)
  T0=$(awk '{print $14+$15}' /proc/$PID/stat); sleep 30; T1=$(awk '{print $14+$15}' /proc/$PID/stat)
  echo "$V pid $PID: rss ${RSS} kB, kgsl mappings ${KG} kB, cpu over 30 s idle $((T1 - T0)) ticks, frames $(grep -ac 'painted shmid' $STATE/daemon-gpu.log), path: $(grep -a 'daemon frame path\|GPU path is off' $STATE/daemon-gpu.log | tail -n 1 | cut -c1-60)"
  PID=""
done
grep 'MemFree\|^Cached' /proc/meminfo | tr -s ' ' | tr '\n' ' '; echo
echo "LunaSysMgr: $(pidof LunaSysMgr | wc -w) process(es); EGL errors in the system log: $(grep -c 'EGL error' /var/log/messages 2>/dev/null)"
