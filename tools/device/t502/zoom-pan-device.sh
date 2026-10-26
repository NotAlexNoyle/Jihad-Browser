# T-504 (device side of zoom-pan.sh): on the spike daemon's page, the emulated finger's double tap in, DRAGS
# drags while zoomed in (the card's view leaves the frame kept at the zoom in), and a double tap out, each marked
# in the daemon log, a screenshot after each step. The daemon dumps the overview (JIHAD_GPU_OVERVIEW_DUMP) and the
# frame published at the zoom out (JIHAD_GPU_ZOOM_BACK_DUMP); the host fetches and deletes them.
STATE=/var/palm/jihad/enyo; LOG=$STATE/daemon-gpu.log
DRAGS=${1:-1}
PRE=${2:-0}   # drags at the fit zoom before the double tap in (the zoom-in point further down the page)
[ -x /tmp/finger ] || { echo "FATAL: no /tmp/finger (tools/device/finger/README.md)"; exit 1; }
mark() { echo "[round] $1 at $(cut -d' ' -f1 /proc/uptime)" >> $LOG; }
shot() { luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/zp-'$1'.png"}' >/dev/null 2>&1; }
sleep 20
i=0
while [ $i -lt $PRE ]; do mark "pre-drag $i"; /tmp/finger drag 480 800 480 350 1000; sleep 2; i=$((i+1)); done
shot 0
zooms() { grep -ac 'setZoomAndScroll' $LOG; }
# a double tap the card did not take (the page's script busy after its load: no smartZoom) is tried again
dtap() { n0=$(zooms); /tmp/finger dtap 384 600; sleep 4; [ "$(zooms)" -gt "$n0" ] || { mark "$1 again"; /tmp/finger dtap 384 600; sleep 4; }; }
mark "dtap in"; dtap "dtap in"; shot 1
i=0
while [ $i -lt $DRAGS ]; do mark "drag $i"; /tmp/finger drag 480 800 480 350 1000; sleep 2; i=$((i+1)); done
shot 2
mark "dtap out"; dtap "dtap out"; shot 3
mark "end"
grep -a '\[round\]\|setZoomAndScroll\|smartZoom\|zoom frame written\|zoom [0-9.]*: \|overview\|resolution [0-9.]* ->\|publishes main-lane\|margins follow' $LOG | cut -c1-280
