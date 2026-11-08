#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# yt-levers.sh [-n RUNS] [-s SECS] [-u URL] CONFIG... — cavekit-device-headroom.md R3 from the host: each lever alone
# against the baseline on the Enyo job daemon, RUNS (3) runs of page-probe.sh (SECS, 60 s) each, the prefs set for the
# config by tools/device/set-prefs.sh (the daemon restarted) and stripped after the last config. Per run: the main
# thread's jiffies over the minute (10 ms each; 6000 is a core), the peak resident memory and the memory after the
# card closed. The configs:
#   base        nothing changed
#   fonts       gfx.downloadable_fonts.enabled=false (no web fonts anywhere, the measurement; a kept lever would be per site)
#   telemetry   jihad.blocking.extra: YouTube's log_event, stats, ptracking, csi and gen_204 paths refused
#   heap        javascript.options.mem.max=131072 (the JS heap capped at 128 MB; -1 is the default)
#   parsing     javascript.options.parallel_parsing=false (ON is the default: the lever is the other way)
#   ion         javascript.options.ion=false (the optimizing JIT off; 2026-11-07 measured no gain, kept for the table)
#   nolite      jihad.youtube.lite=false (the format lever off, for reference)
# Output: one line per run, then the medians per config, under docs/rounds/headroom-<date>/levers-<date>.txt.
set -uo pipefail
HERE=$(cd "$(dirname "$0")" && pwd); ROOT=$(cd "$HERE/../../.." && pwd)
RUNS=3; SECS=60; URL="https://m.youtube.com/watch?v=ZgZM0ewM4lU"
while [ $# -gt 0 ]; do case "$1" in -n) RUNS=$2; shift 2;; -s) SECS=$2; shift 2;; -u) URL=$2; shift 2;; *) break;; esac; done
[ $# -gt 0 ] || { echo "usage: yt-levers.sh [-n RUNS] [-s SECS] [-u URL] base|fonts|telemetry|heap|parsing|ion|nolite ..."; exit 2; }
RE='gfx\.downloadable_fonts\.enabled|jihad\.blocking\.extra|javascript\.options\.mem\.max|javascript\.options\.parallel_parsing|javascript\.options\.ion|jihad\.youtube\.lite'
prefs_of() { case "$1" in
  base) ;;
  fonts) echo 'user_pref("gfx.downloadable_fonts.enabled", false);';;
  telemetry) echo 'user_pref("jihad.blocking.extra", "youtube.com/youtubei/v1/log_event;youtube.com/api/stats/;youtube.com/ptracking;youtube.com/csi_204;youtube.com/gen_204;youtube.com/youtubei/v1/att/log;youtube.com/pagead/");';;
  heap) echo 'user_pref("javascript.options.mem.max", 131072);';;
  parsing) echo 'user_pref("javascript.options.parallel_parsing", false);';;
  ion) echo 'user_pref("javascript.options.ion", false);';;
  nolite) echo 'user_pref("jihad.youtube.lite", false);';;
  *) echo "unknown config $1" >&2; return 1;; esac; }
DAY=$(date +%Y-%m-%d); OUT="$ROOT/docs/rounds/headroom-$DAY"; mkdir -p "$OUT"; LOG="$OUT/levers-$(date +%H%M).txt"
echo "# yt-levers $(date -Is): $URL, $SECS s, $RUNS runs each; main = the main thread's jiffies over the run, peak/after in kB" | tee "$LOG"
for cfg in "$@"; do
  prefs_of "$cfg" > /tmp/jihad-levers-prefs.txt || exit 2
  novacom put file:///tmp/jihad-newprefs.txt < /tmp/jihad-levers-prefs.txt
  "$ROOT/build/webos-oe/dev-sh.sh" -t 60 "$ROOT/tools/device/set-prefs.sh" enyo "$RE" /tmp/jihad-newprefs.txt > /tmp/jihad-levers-set.txt 2>&1 || { echo "set-prefs failed for $cfg"; cat /tmp/jihad-levers-set.txt; exit 1; }
  sleep 6
  for r in $(seq 1 "$RUNS"); do
    "$ROOT/build/webos-oe/dev-sh.sh" -t $((SECS + 90)) "$HERE/page-probe.sh" "$URL" "$SECS" > /tmp/jihad-levers-run.txt 2>&1
    dp=$(sed -n 's/^daemon \([0-9]*\)$/\1/p' /tmp/jihad-levers-run.txt | head -1)
    main=$(awk -v d="$dp" '$1 == d && $2 == "ld-2.23.so" { s += $3 } END { print s + 0 }' /tmp/jihad-levers-run.txt)
    peak=$(sed -n 's/.* rss \([0-9]*\) .*/\1/p' /tmp/jihad-levers-run.txt | sort -n | tail -1)
    after=$(sed -n 's/^after close: rss \([0-9]*\).*/\1/p' /tmp/jihad-levers-run.txt)
    load=$(sed -n 's/^t=[0-9]* load \([0-9.]*\) .*/\1/p' /tmp/jihad-levers-run.txt | sort -n | tail -1)
    echo "$cfg run$r main=$main peak=${peak:-?} after=${after:-?} maxload=${load:-?}" | tee -a "$LOG"
    cp /tmp/jihad-levers-run.txt "$OUT/levers-$cfg-run$r.txt"
  done
done
# the prefs stripped: the device as found
: > /tmp/jihad-levers-prefs.txt; "$ROOT/build/webos-oe/dev-sh.sh" -t 60 "$ROOT/tools/device/set-prefs.sh" enyo "$RE" > /dev/null 2>&1
echo "# medians" | tee -a "$LOG"
for cfg in "$@"; do
  grep "^$cfg run" "$LOG" | awk -v c="$cfg" '{ for (i = 3; i <= NF; i++) { split($i, kv, "="); v[kv[1]][NR] = kv[2] + 0 } }
    END { line = c; for (k in v) { n = asort(v[k], s); m = (n % 2) ? s[(n + 1) / 2] : (s[n / 2] + s[n / 2 + 1]) / 2; line = line " " k "=" m } print line }' | tee -a "$LOG"
done
echo "$LOG"
