#!/bin/bash
# imgdecode-round.sh [n=8] : cavekit-lowpower-uxp.md R4, T-558 — the engine's decode time of the same picture as JPEG XL,
# WebP and JPEG (render/goanna/test/make-image-assets.sh, served by the host's test server), each format a page of
# its own on the Enyo variant's hand-started daemon (the T-502 spike helpers), the result read from the title the
# page sets and the daemon logs. One line per format: decoded/asked, total ms, median ms a picture.
#   HOST=192.168.1.245:8077 tools/device/imgdecode-round.sh 8
set -u
R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser
cd "$R" || exit 2
. build/webos-oe/host-path.sh
T=tools/device/t502
HOST=${HOST:-192.168.1.245:8077}
N=${1:-8}
SCR=${TMPDIR:-/tmp}/imgd.$$; mkdir -p "$SCR"; trap 'rm -rf "$SCR"' EXIT
cat > "$SCR/title.sh" <<'EOS'
LOG=/var/palm/jihad/enyo/daemon-gpu.log
t=0; while [ $t -lt 120 ]; do grep -aq 'jihad-imgdecode [a-z]*: ' $LOG && break; sleep 1; t=$((t+1)); done
grep -a 'jihad-imgdecode [a-z]*: ' $LOG | tail -n 1 | sed 's/.*jihad-imgdecode //; s/\].*//'
EOS
for fmt in jpg webp jxl; do
	build/webos-oe/dev-sh.sh -t 90 $T/gpu-spike-start.sh "http://$HOST/jihad-imgdecode.html?fmt=$fmt&n=$N" JIHAD_GPU_TIMELINE=1 2>&1 | grep -a "FATAL" | head -2
	res=$(build/webos-oe/dev-sh.sh -t 140 "$SCR/title.sh" 2>&1 | tail -n 1)
	echo "$fmt: ${res:-NA}"
	build/webos-oe/dev-sh.sh -t 60 $T/gpu-spike-stop.sh 2>&1 | grep -a "FATAL\|ERROR" | head -1
done
