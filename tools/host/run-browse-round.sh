#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# Run the pretend-user round from the host (cavekit-modern-web.md R6): serve the test pages,
# put the site list, run tools/device/browse-round.sh on the device with a long hold, pull the
# shots one session at a time, delete them from the device's 40 MB /tmp, and write the analysis.
#   tools/host/run-browse-round.sh <out dir>      (about 40 minutes; one novacom session at a time)
set -uo pipefail
cd "$(dirname "$0")/../.."
OUT=${1:?out dir}; mkdir -p "$OUT/shots"
pgrep -f 'http.server 8077' >/dev/null || ( cd render/goanna/test && python3 -m http.server 8077 --bind 0.0.0.0 >/dev/null 2>&1 & ); sleep 1
novacom put file:///tmp/round2-sites.txt < tools/device/browse-round-sites.txt
bash build/webos-oe/dev-sh.sh -t 2700 tools/device/browse-round.sh > "$OUT/round.log" 2>&1
python3 -I tools/device/browse-round-analyse.py "$OUT/round.log" > "$OUT/analysis.md" 2>&1
for n in $(cut -d'|' -f1 tools/device/browse-round-sites.txt); do
  ( sleep 20 | novacom get "file:///tmp/shot-r2-$n.png" > "$OUT/shots/$n.png" ) 2>/dev/null
done
( sleep 5 | novacom run file://bin/sh -- -c 'rm -f /tmp/shot-r2-*.png; df /tmp | tail -1' ) 2>&1 | tail -1
echo "round done: $(grep -c '^@@' "$OUT/round.log") visits; $OUT/analysis.md"
