# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# yt-play.sh LITE [URL] [ROWS] — cavekit-device-headroom.md R2/R4 on the device, Enyo: YouTube's watch page with jihad.youtube.lite
# LITE (1 or 0) and jihad.youtube.maxRows ROWS (480), the play button tapped, then every 5 s for 60 s the daemon's main-thread and total CPU (jiffies of 10
# ms per 5 s) and memory; the format the player fetched (itag, mime) from the blocker's load trace; and at 75 s after
# the page's root the video element's size, time and frame counts (the compat probe). The prefs are set for the run
# and cleared after (set-prefs.sh restarts the daemon each time); the card is closed by the script. Needs
# /tmp/set-prefs.sh (tools/device/set-prefs.sh) beside it.
LITE=${1:-1}; URL=${2:-https://m.youtube.com/watch?v=ZgZM0ewM4lU}; ROWS=${3:-480}; app=net.riverstonerelay.jihad-browser; L=/var/palm/jihad/enyo/daemon.log
close_all() { for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 "\"$app\"" | grep processid | sed 's/.*"processid": *"\([^"]*\)".*/\1/'); do luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'"$pid"'"}' >/dev/null 2>&1; done; }
# the probe (a script run in the page 75 s after its root), base64 on the HOST — the device has no base64 (2026-11-08):
#   (function(){var v=document.querySelector('video');if(!v)return 'no video';var q=v.getVideoPlaybackQuality?v.getVideoPlaybackQuality():null;return JSON.stringify({w:v.videoWidth,h:v.videoHeight,t:Math.
PROBE=KGZ1bmN0aW9uKCl7dmFyIHY9ZG9jdW1lbnQucXVlcnlTZWxlY3RvcigndmlkZW8nKTtpZighdilyZXR1cm4gJ25vIHZpZGVvJzt2YXIgcT12LmdldFZpZGVvUGxheWJhY2tRdWFsaXR5P3YuZ2V0VmlkZW9QbGF5YmFja1F1YWxpdHkoKTpudWxsO3JldHVybiBKU09OLnN0cmluZ2lmeSh7dzp2LnZpZGVvV2lkdGgsaDp2LnZpZGVvSGVpZ2h0LHQ6TWF0aC5yb3VuZCh2LmN1cnJlbnRUaW1lKSxwYXVzZWQ6di5wYXVzZWQsdG90YWw6cT9xLnRvdGFsVmlkZW9GcmFtZXM6LTEsZHJvcHBlZDpxP3EuZHJvcHBlZFZpZGVvRnJhbWVzOi0xLHNyYzoodi5jdXJyZW50U3JjfHwnJykuc2xpY2UoMCw0MCl9KTt9KSgp
P='jihad\.(youtube\.(lite|maxRows)|compat\.(trace|probe|probeDelay)|blocking\.trace)'
printf 'user_pref("jihad.youtube.lite", %s);\nuser_pref("jihad.youtube.maxRows", %s);\nuser_pref("jihad.compat.trace", true);\nuser_pref("jihad.compat.probe", "%s");\nuser_pref("jihad.compat.probeDelay", 75000);\nuser_pref("jihad.blocking.trace", true);\n' "$([ $LITE = 1 ] && echo true || echo false)" "$ROWS" "$PROBE" > /tmp/jihad-newprefs.txt
sh /tmp/set-prefs.sh enyo "$P" /tmp/jihad-newprefs.txt >/dev/null 2>&1; sleep 8
DP=$(status jihad | sed -n 's/.*process \([0-9]*\).*/\1/p'); n0=$(wc -l < $L)
cpu() { set -- $(sed 's/^.*) //' /proc/$DP/task/$DP/stat); m=$(( ${12} + ${13} )); set -- $(sed 's/^.*) //' /proc/$DP/stat); echo "$m $(( ${12} + ${13} ))"; }
close_all; sleep 1
luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"'$app'","params":{"target":"'"$URL"'"}}' >/dev/null
sleep 25; /tmp/finger tap 380 344; echo "lite=$LITE: play tapped at 25 s"
set -- $(cpu); m0=$1; a0=$2; i=0
while [ $i -lt 60 ]; do
  sleep 5; i=$((i+5)); set -- $(cpu)
  echo "play+$i main=$(( $1 - m0 )) all=$(( $2 - a0 )) rss=$(grep VmRSS /proc/$DP/status | tr -s ' ' | cut -d' ' -f2) load=$(cut -d' ' -f1 /proc/loadavg)"
  m0=$1; a0=$2
done
sleep 5
mkdir -p /media/internal/jihad-stream; luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/media/internal/jihad-stream/fl-ytplay.png"}' >/dev/null 2>&1; usleep 800000   # not /tmp: the device check counts a png there as a dump left behind
tail -n +$n0 $L | grep -a -o 'trace taken [a-z]* https://[^ ]*googlevideo.com/videoplayback[^ ]*' | grep -o -E '(itag|mime)=[^&]*' | sort | uniq -c | head -8
tail -n +$n0 $L | grep -a 'probe \[' | sed 's/.*probe /probe /' | head -3
tail -n +$n0 $L | grep -a 'youtube: ' | head -2 | cut -c1-160
close_all; sleep 3
sh /tmp/set-prefs.sh enyo "$P" >/dev/null 2>&1
echo DONE
