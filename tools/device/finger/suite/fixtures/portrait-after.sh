#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# portrait-after.sh up|down SHELL (host side): the player's full screen turns the whole system to landscape, in the stock browser
# as well, and leaving it, or the card, does not turn it back: only the accelerometer does, and a device on a bench does not move.
# A restart of the system manager does (and closes the USB alert: sysmgr-restart.sh).
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; ROOT="$(cd "$HERE/../../../../.." && pwd)"
. "$ROOT/build/webos-oe/host-path.sh" 2>/dev/null
[ "$1" = down ] || exit 0
bash "$ROOT/build/webos-oe/dev-sh.sh" -t 120 "$ROOT/tools/device/finger/sysmgr-restart.sh" >/dev/null 2>&1
novacom run file://bin/rm -- -f /tmp/fl-restart.png < /dev/null
echo "portrait: the system manager restarted"
