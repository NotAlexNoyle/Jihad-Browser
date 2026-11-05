#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# fingerlib-check.sh [enyo|mochi|mojo] [plant]: the check library's own round (cavekit-finger-testing.md R1, R3; T-563)
# on one shell: the hand daemon on jihad-long-static.html, the calibration tap, every oracle and frame check once
# (a deliberate failure among them: the frame it keeps is pulled), then desktop Wikipedia at its fit zoom (the zoom
# in the geometry measured by a second calibration tap). "plant": the table's page-top constant 20 px wrong — the
# calibration must stop the round before any check (exit 3). Output: $ROUND_DIR/fl-check-<shell>[-plant].txt, the
# kept frames beside it as fl-kept-<shell>-<check>.raw (a portrait page of /dev/fb1: check-frame.py reads one).
set -u
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; ROOT="$(cd "$HERE/../../.." && pwd)"
. "$ROOT/build/webos-oe/host-path.sh"
V=${1:-enyo}; PLANT=${2:-}
D=${ROUND_DIR:-$ROOT/docs/rounds/finger-$(date +%F)}; mkdir -p "$D"
LONG=${LONG_URL:-http://192.168.1.245:8077/jihad-long-static.html}; WIKI=${WIKI_URL:-https://en.wikipedia.org/wiki/HP_TouchPad}
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT
# the sampler from its source (no binary in the tree), pushed every run: 48 KB
TC=$ROOT/build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin
$TC/arm-webos-linux-gnueabi-gcc -O2 -march=armv7-a -mfpu=neon -mfloat-abi=softfp -std=gnu99 -o "$TMP/fbstream" "$HERE/fbstream.c" || exit 1
novacom put file:///tmp/fbstream < "$TMP/fbstream"
"$ROOT/build/webos-oe/dev-sh.sh" -t 90 "$HERE/daemon-start.sh" "$V" "$LONG" 2>&1 | tail -n 3
{ echo "chmod +x /tmp/fbstream /tmp/finger 2>/dev/null"; [ -n "$PLANT" ] && echo "FL_PLANT_TOP=20"; cat "$HERE/fingerlib.sh"; cat <<'BODY'
fl_init $1
fl_calibrate "sel:#content p"
fl_check_zoom zoom-1 1.0 0.01
fl_check_url url-long jihad-long-static
fl_check_text_has p1-text "sel:#content p" "1: the quick brown fox"
fl_check_content content-top 0,150,768,400 "sel:#content p" "quick brown"
fl_check_element_at p1-at "sel:#content p" 384 $((FL_TOP + 90))
fl_sample header header
fl_sample content 0,300,756,900
fl_check_scroll scroll-0 0 0 5
fl_drag 480 900 480 250
fl_check_card_scroll scroll-650 650 80
fl_check_region_equal header-still header header
fl_check_region_changed content-moved 0,300,756,900 content
fl_check_top top-after-scroll "sel:#content p:nth-child(11)"
fl_tap "sel:#content p:nth-child(12)" && fl_check_hit hit-p12 "sel:#content p:nth-child(12)"
fl_chrome_tap address; fl_check_keyboard kb-up up 8
fl_tap_at 384 300; fl_check_keyboard kb-down down 8
fl_check_text deliberate-fail "sel:#header" "not the header"
fl_load WIKI 60 && fl_check_url url-wiki HP_TouchPad
fl_check_zoom zoom-fit 0.69 0.06
m=$(fl_measure_top "sel:#firstHeading") && echo "wikipedia at the fit zoom $(fl_zoom): the page's top measured $m (calibrated $FL_TOP on the long page)"
fl_check_content wiki-title 0,$((FL_TOP)),768,$((FL_TOP + 300)) "sel:#firstHeading" "HP TouchPad"
fl_summary
BODY
} | sed "s|WIKI|$WIKI|" > "$TMP/body.sh"
OUT="$D/fl-check-$V${PLANT:+-plant}.txt"
"$ROOT/build/webos-oe/dev-sh.sh" -t 400 "$TMP/body.sh" "$V" 2>&1 | tee "$OUT"; RC=${PIPESTATUS[0]}
# the kept frames of the failed checks
for f in $(grep -a -o '/media/internal/jihad-finger/[^ ]*\.last\.raw' "$OUT" | sort -u); do
  novacom get "file://$f" > "$D/fl-kept-$V-$(basename "$f" .last.raw).raw" && echo "pulled $f"
done
novacom run file://bin/rm -- -rf /media/internal/jihad-finger </dev/null >/dev/null 2>&1
"$ROOT/build/webos-oe/dev-sh.sh" -t 90 "$HERE/daemon-stop.sh" "$V" 2>&1 | tail -n 2
echo "exit $RC: $OUT"; exit $RC
