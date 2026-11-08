#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# detached-run.sh [-t MAXSECONDS] [-o HOSTLOG] <device script> [args...] — run a heavy device test DETACHED from the
# novacom session that starts it, and measure the USB link beside it (cavekit-device-headroom.md R1: "a novacom
# command every 10 s answers within 3 s, the USB link never drops"). The script goes to /tmp on the device and runs
# under nohup with its output in /media/internal/jihad-stream/<name>.<time>.log (never /tmp: a 40 MB tmpfs the
# daemon's dumps share); it must end with a line "DONE". Every 10 s this probes the link with a 3 s ceiling
# (`novacom run true`: nothing to print, so the stdin end-of-file race cannot eat its answer) and every 30 s reads
# the log's last line; it stops at DONE or at MAXSECONDS (default 900), pulls the log to HOSTLOG (default
# ./<name>.log) and prints the probes' count, failures and worst latency. A test killed by a dropped link keeps
# running on the device and its log is still there afterwards; this is why the heavy tests are never run through
# dev-sh.sh (2026-11-07: a USB drop under swap took the foreground session with it).
set -uo pipefail
. "$(dirname "$0")/../../../build/webos-oe/host-path.sh"
MAX=900; HOSTLOG=""
while [ $# -gt 0 ]; do case "$1" in -t) MAX=$2; shift 2;; -o) HOSTLOG=$2; shift 2;; *) break;; esac; done
SRC="${1:?usage: detached-run.sh [-t MAX] [-o HOSTLOG] <device script> [args...]}"; shift
[ -f "$SRC" ] || { echo "detached-run: no such script: $SRC" >&2; exit 2; }
NAME=$(basename "$SRC" .sh); TS=$(date +%Y%m%d-%H%M%S)
DEVSCRIPT=/tmp/$NAME.sh; DEVLOG=/media/internal/jihad-stream/$NAME.$TS.log
[ -n "$HOSTLOG" ] || HOSTLOG=./$NAME.$TS.log
novacom put "file://$DEVSCRIPT" < "$SRC" || exit 1
# the launcher: one short session; the test outlives it
L=$(mktemp); trap 'rm -f "$L"' EXIT
{ echo "mkdir -p /media/internal/jihad-stream"
  printf 'nohup sh %s' "$DEVSCRIPT"; for a in "$@"; do printf " '%s'" "$a"; done; printf ' > %s 2>&1 &\n' "$DEVLOG"
  echo 'sleep 1; echo "started $!"'; } > "$L"
# `novacom run sh -- script` reads the script from a DEVICE path: push the launcher first
novacom put "file:///tmp/$NAME.launch.sh" < "$L"
START=$(sleep 2 | novacom run file://bin/sh -- "/tmp/$NAME.launch.sh" 2>&1)
echo "detached-run: $START; log $DEVLOG"
n=0; bad=0; worst=0; t0=$(date +%s); last=""
while :; do
  now=$(( $(date +%s) - t0 ))
  s=$(date +%s%N)
  if timeout 3 novacom run file://bin/true </dev/null >/dev/null 2>&1; then ok=ok; else ok=FAILED; bad=$((bad+1)); fi
  ms=$(( ($(date +%s%N) - s) / 1000000 )); n=$((n+1)); [ $ms -gt $worst ] && worst=$ms
  echo "usb t=${now}s ${ms}ms $ok"
  if [ $((n % 3)) = 0 ]; then
    # `tail -1`, not `-n 1`: novacom takes a `-n` after the `--` as its own (the read came back empty, 2026-11-08)
    last=$(timeout 10 bash -c "sleep 4 | novacom run file://bin/tail -- -1 $DEVLOG" 2>/dev/null | tr -d '\r')
    [ -n "$last" ] && echo "  log: ${last:0:160}"
    case "$last" in DONE*) break;; esac
  fi
  [ $now -ge $MAX ] && { echo "detached-run: $MAX s reached, the test may still be running"; break; }
  sleep 10
done
sleep 5 | novacom run file://bin/cat -- "$DEVLOG" > "$HOSTLOG" 2>/dev/null
echo "detached-run: probes $n, failed $bad, worst ${worst} ms; the log is $HOSTLOG ($(wc -l < "$HOSTLOG") lines)"
[ $bad = 0 ] && [ $worst -le 3000 ]
