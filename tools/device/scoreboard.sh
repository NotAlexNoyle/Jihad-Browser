#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# scoreboard.sh [runs] (T-629, cavekit-modern-web.md R1; the owner's goal of 2026-11-11: "beat both webkit and
# chromium ... users will pick jihad browser and global jihad over atlas and webkit + chromium"): the same pages through
# Jihad (the Enyo shell), Atlas (org.webosports.app.atlas, WPE WebKit) and the stock browser (HP's isis on its own
# BrowserServer, installed for the round under a test id by tools/device/finger/flash/stock-bench.sh), read the same way
# for each: the screen itself (fbstream), a finger's drags, every process's memory, a screenshot at rest for the eye.
# Rows in $ROUND_DIR/scoreboard.tsv: browser, page, run, first_ms, settled_ms, pan_fps, pan_moved, growth_kb.
#
#   RUNS=3 (default) ROUND_DIR=docs/rounds/scoreboard-<date> PAGES="<url> ..." (default: five real sites, below) BROWSERS="jihad atlas stock"
# The browsers alternate within each run, so each is read in the same hour as the others. The stock copy is removed at
# the end. One novacom session at a time; the owner not at the device (/tmp/jihad-owner-testing refuses the round).
set -u
R=$(cd "$(dirname "$0")/../.." && pwd)
cd "$R" || exit 2
. build/webos-oe/host-path.sh
RUNS=${RUNS:-${1:-3}}
HOST=${HOST:-192.168.1.245:8077}
D=${ROUND_DIR:-docs/rounds/scoreboard-$(date +%Y-%m-%d)}; mkdir -p "$D"
TSV=$D/scoreboard.tsv
[ -s "$TSV" ] || printf 'when\tbrowser\tpage\trun\tfirst_ms\tsettled_ms\tpan_fps\tpan_moved\tgrowth_kb\n' > "$TSV"
# Real sites (the owner, 2026-11-12: "use real websites for the scoreboard"): a reference article, a light text page, a
# heavy news page, an app-like page, a CSS-heavy docs page and a Flash page ("include a flash player page": Orisinal's
# Winterbells, real Flash in the three shells by T-574). A browser that cannot load one gets its row anyway (NA).
PAGES=${PAGES:-"https://en.wikipedia.org/wiki/HP_TouchPad https://news.ycombinator.com/ https://www.bbc.com/news https://github.com/webOS-ports/org.webosports.app.atlas https://developer.mozilla.org/en-US/docs/Web/CSS/display http://www.ferryhalim.com/orisinal/g3/bells.htm"}
BROWSERS=${BROWSERS:-"jihad atlas stock"}
idof() { case $1 in jihad) echo net.riverstonerelay.jihad-browser;; atlas) echo org.webosports.app.atlas;; stock) echo net.riverstonerelay.stockbench;; esac; }
say() { echo; echo "##### $(date +%H:%M:%S) $*"; }
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT

out=$(printf '[ -e /tmp/jihad-owner-testing ] && echo LOCKED || echo free\nls -d /media/cryptofs/apps/usr/palm/applications/org.webosports.app.atlas 2>/dev/null || echo "no atlas"\n' > "$T/c.sh"; build/webos-oe/dev-sh.sh -t 30 "$T/c.sh" 2>&1)
case "$out" in *LOCKED*) echo "REFUSED: the owner is testing (/tmp/jihad-owner-testing)"; exit 3;; esac
case "$out" in *"no atlas"*) echo "Atlas is not installed: left out"; BROWSERS=$(echo $BROWSERS | sed 's/atlas//');; esac
TC=build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin/arm-webos-linux-gnueabi-gcc
novacom put file:///tmp/scoreboard-device.sh < tools/device/scoreboard-device.sh
for b in fbstream finger; do
  "$TC" -O2 -march=armv7-a -mfpu=neon -mfloat-abi=softfp -std=gnu99 -o "$T/$b" tools/device/finger/$b.c && novacom put file:///tmp/$b < "$T/$b"
done
printf 'chmod +x /tmp/fbstream /tmp/finger; echo ok\n' > "$T/x.sh"; build/webos-oe/dev-sh.sh -t 20 "$T/x.sh" >/dev/null 2>&1
case " $BROWSERS " in *" stock "*) say "the stock browser under its test id"; bash tools/device/finger/flash/stock-bench.sh up;; esac

for r in $(seq 1 "$RUNS"); do
  for page in $PAGES; do
    pn=$(basename "$page" | sed 's/\.html$//' | tr -c 'A-Za-z0-9_-' '_' | cut -c1-24)
    for b in $BROWSERS; do
      say "run $r: $b on $pn"
      tools/device/close-usb-alert.sh 300 >/dev/null
      tag=$b-$pn-r$r
      # Detached on the device, its output to a file and a done marker beside it: the first round (chain 19) lost every
      # row whose novacom session dropped under a heavy page (no fbstream line, no growth, no stream), so the host only
      # starts it and then polls the marker, a short session at a time.
      printf 'rm -f /media/internal/jihad-stream/sb-%s.done; (trap "" HUP; exec sh /tmp/scoreboard-device.sh %s %s %s > /media/internal/jihad-stream/sb-%s.log 2>&1 < /dev/null) &\necho started\n' \
        "$tag" "$(idof $b)" "$page" "$tag" "$tag" > "$T/go.sh"
      build/webos-oe/dev-sh.sh -t 20 "$T/go.sh" >/dev/null 2>&1
      w=0; while [ $w -lt 24 ]; do
        sleep 10; w=$((w + 1))
        printf '[ -e /media/internal/jihad-stream/sb-%s.done ] && echo DONE || echo running\n' "$tag" > "$T/p.sh"
        case "$(build/webos-oe/dev-sh.sh -t 20 "$T/p.sh" 2>/dev/null)" in *DONE*) break;; esac
      done
      tools/device/close-usb-alert.sh 300 >/dev/null
      novacom get file:///media/internal/jihad-stream/sb-$tag.log > "$T/s.log" 2>/dev/null
      res=$(cat "$T/s.log"); [ $w -lt 24 ] || echo "   !! no done marker after 240 s"
      echo "$res" | grep -a 'growth\|FATAL\|samples\|fbstream' | tail -3
      novacom get file:///media/internal/jihad-stream/sb-$tag.fbs > "$T/s.fbs" 2>/dev/null
      novacom get file:///media/internal/jihad-stream/sb-$tag.fin > "$T/s.fin" 2>/dev/null
      novacom get file:///tmp/sb-$tag.png > "$D/rest-$tag.png" 2>/dev/null
      cp "$T/s.log" "$D/device-$tag.log" 2>/dev/null
      printf 'rm -f /media/internal/jihad-stream/sb-%s.* /tmp/sb-%s.png\n' "$tag" "$tag" > "$T/rm.sh"; build/webos-oe/dev-sh.sh -t 20 "$T/rm.sh" >/dev/null 2>&1
      m=$(python3 tools/host/scoreboard-read.py "$T/s.fbs" --trace "$T/s.fin" 2>&1 | tail -1); echo "   $m"
      g=$(echo "$res" | sed -n 's/^growth \([0-9]*\) kB/\1/p' | tail -1)
      set -- $m
      printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$(date +%Y-%m-%dT%H:%M:%S)" "$b" "$pn" "$r" "${2:-NA}" "${4:-NA}" "${6:-NA}" "${8:-NA}" "${g:-NA}" >> "$TSV"
    done
  done
done
case " $BROWSERS " in *" stock "*) say "the stock copy removed"; bash tools/device/finger/flash/stock-bench.sh down;; esac
tools/device/close-usb-alert.sh 300
say "medians"
python3 - "$TSV" <<'PY'
import sys, statistics, collections
d = collections.defaultdict(lambda: collections.defaultdict(list))
rows = [l.rstrip('\n').split('\t') for l in open(sys.argv[1])][1:]
for w, b, p, r, *v in rows:
    for k, x in zip(['first_ms', 'settled_ms', 'pan_fps', 'pan_moved', 'growth_kb'], v):
        try: d[(p, b)][k].append(float(x))
        except ValueError: pass
for (p, b), m in sorted(d.items()):
    print("%-24s %-6s " % (p, b) + ' '.join('%s=%.6g' % (k, statistics.median(xs)) for k, xs in m.items() if xs))
PY
