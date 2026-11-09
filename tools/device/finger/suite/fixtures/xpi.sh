#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# xpi.sh up|down SHELL (host side): the test extension jihad-effect@riverstonerelay.net (the one T-249 installed over the
# FIFO) built into the served test directory for the xpi-confirm script (up), and taken out of the shell's profile and
# the served directory after (down, with the job stopped so nothing writes it back; the next start prunes the database).
set -u
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; ROOT="$(cd "$HERE/../../../../.." && pwd)"; . "$ROOT/build/webos-oe/host-path.sh"
ACT=$1; V=$2
case $V in enyo) ID=net.riverstonerelay.jihad-browser; JOB=jihad;; mochi) ID=net.riverstonerelay.jihad-browser-mochi; JOB=jihad-mochi;; mojo) ID=net.riverstonerelay.jihad-browser-mojo; JOB=jihad-mojo;; *) echo "xpi.sh: shell?"; exit 2;; esac
EXT=/media/cryptofs/apps/usr/palm/applications/$ID/profile/extensions
SERVED="$ROOT/render/goanna/test/jihad-effect.xpi"
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT
if [ "$ACT" = up ]; then
  python3 -I "$HERE/make-pad-xpi.py" --effect "$SERVED"
  echo "xpi: $(stat -c %s "$SERVED") bytes served as jihad-effect.xpi"
else
  rm -f "$SERVED"
  cat > "$TMP/d.sh" <<EOS
stop $JOB >/dev/null 2>&1; sleep 2
rm -f $EXT/jihad-effect@riverstonerelay.net.xpi; rm -rf $EXT/jihad-effect@riverstonerelay.net
start $JOB >/dev/null 2>&1; sleep 4; echo "xpi: removed; left: \$(ls $EXT 2>/dev/null | grep -c jihad-effect); \$(status $JOB 2>&1)"
EOS
  "$ROOT/build/webos-oe/dev-sh.sh" -t 40 "$TMP/d.sh" 2>&1 | tail -1
fi
