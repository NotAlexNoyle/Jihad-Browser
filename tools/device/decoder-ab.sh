#!/bin/bash
# decoder-ab.sh [runs=3] : cavekit-lowpower-uxp.md R4, T-558 — the VP9 decoder A/B: ffvpx (the default) against libvpx
# (media.ffvpx.enabled=false), each 30 s clip of make-media-assets.sh at 360p and 480p played to its end on the Enyo
# hand daemon, the frames painted and dropped read from the page's title (jihad-media-test.html: "ended ... painted= dropped=
# total="); then, with ffvpx off, an MP3 and a FLAC played to their end (the lever must not lose them). One line a run.
#   tools/device/decoder-ab.sh 3
set -u
R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser
cd "$R" || exit 2
export JIHAD_REPO=$R; . build/webos-oe/host-path.sh
T=tools/device/t502
N=${1:-3}; HOST=${HOST:-192.168.1.245:8077}
SCR=${TMPDIR:-/tmp}/dab.$$; mkdir -p "$SCR"; trap 'rm -rf "$SCR"' EXIT
cat > "$SCR/end.sh" <<'EOS'
LOG=/var/palm/jihad/enyo/daemon-gpu.log
t=0; while [ $t -lt 75 ]; do grep -aq 'titleAndUrl title=\[ended\|title changed by the page: \[ended\|title=\[error\|page: \[error' $LOG && break; sleep 1; t=$((t+1)); done
grep -a 'title=\[ended\|page: \[ended\|title=\[error\|page: \[error\|play refused' $LOG | tail -n 1 | sed 's/.*\[\(ended[^]]*\|error[^]]*\|play refused[^]]*\)\].*/\1/'
EOS
one() {   # one LABEL SRC [PREF words]
	local label=$1 src=$2; shift 2
	build/webos-oe/dev-sh.sh -t 90 $T/gpu-spike-start.sh "http://$HOST/jihad-media-test.html?src=$src&wide=1" JIHAD_GPU_EXPRESS=1 "$@" 2>&1 | grep -a "FATAL" | head -1
	echo "$label $src: $(build/webos-oe/dev-sh.sh -t 90 "$SCR/end.sh" 2>&1 | tail -n 1)"
	build/webos-oe/dev-sh.sh -t 60 $T/gpu-spike-stop.sh 2>&1 | grep -a "FATAL" | head -1
}
for r in $(seq 1 "$N"); do
	for clip in clip30vp9-360.webm clip30vp9-480.webm; do
		one "run $r ffvpx " "$clip"
		one "run $r libvpx" "$clip" PREF=media.ffvpx.enabled=false
	done
done
echo "--- with ffvpx off, the other formats"
for f in tone10.mp3 tone10.flac; do one "libvpx-config" "$f&audio=1" PREF=media.ffvpx.enabled=false; done
