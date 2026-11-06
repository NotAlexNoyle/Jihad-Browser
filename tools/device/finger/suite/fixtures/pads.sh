#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# pads.sh up|down SHELL (host side): twelve test extensions sideloaded in the shell's profile for a script (up, before
# the hand daemon starts: they are found at its start, listed disabled), and removed after (down, with the job stopped
# so nothing writes them back; the next start prunes them from the add-on database).
set -u
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; ROOT="$(cd "$HERE/../../../../.." && pwd)"; . "$ROOT/build/webos-oe/host-path.sh"
ACT=$1; V=$2
case $V in enyo) ID=net.riverstonerelay.jihad-browser; JOB=jihad;; mochi) ID=net.riverstonerelay.jihad-browser-mochi; JOB=jihad-mochi;; mojo) ID=net.riverstonerelay.jihad-browser-mojo; JOB=jihad-mojo;; esac
EXT=/media/cryptofs/apps/usr/palm/applications/$ID/profile/extensions
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT
if [ "$ACT" = up ]; then
  python3 -I "$HERE/make-pad-xpi.py" "$TMP/x" 12
  for f in "$TMP"/x/*.xpi; do novacom put "file://$EXT/$(basename "$f")" < "$f"; done
  echo "pads: 12 put in $EXT"
else
  cat > "$TMP/d.sh" <<EOS
stop $JOB >/dev/null 2>&1; sleep 2
rm -f $EXT/pad*@jihad.test.xpi
start $JOB >/dev/null 2>&1; sleep 4; echo "pads: removed; left: \$(ls $EXT | grep -c '@jihad.test'); \$(status $JOB 2>&1)"
EOS
  "$ROOT/build/webos-oe/dev-sh.sh" -t 40 "$TMP/d.sh" 2>&1 | tail -1
fi
