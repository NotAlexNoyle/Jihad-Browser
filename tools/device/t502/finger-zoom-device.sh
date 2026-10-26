# T-527 (device side of finger-zoom.sh): on the spike daemon's page, the emulated finger's double tap in
# and out and a pinch in and out, each marked in the daemon log, a screenshot after each
STATE=/var/palm/jihad/enyo; LOG=$STATE/daemon-gpu.log
mark() { echo "[round] $1 at $(cut -d' ' -f1 /proc/uptime)" >> $LOG; }
shot() { luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/fz-'$1'.png"}' >/dev/null 2>&1; }
sleep 20
mark "dtap in"; /tmp/finger dtap 384 600; sleep 1; shot 1; sleep 3; shot 2
mark "dtap out"; /tmp/finger dtap 384 600; sleep 4; shot 3
mark "pinch in"; /tmp/finger pinch 384 500 200 420 600; sleep 1; shot 4; sleep 3; shot 5
mark "pinch out"; /tmp/finger pinch 384 500 420 200 600; sleep 4; shot 6
mark "end"
grep -a '\[round\]\|setZoomAndScroll\|smartZoom\|zoom frame written\|zoom [0-9.]*: the \|resolution [0-9.]* ->\|publishes main-lane\|margins follow\|past half\|main layer paint\|layer transaction\|main pump\|frame dropped' $LOG | cut -c1-230
