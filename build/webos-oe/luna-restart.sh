#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Restart LunaSysMgr on the device and hand back a screen with nothing on top of it.
#
#   build/webos-oe/luna-restart.sh            restart, wait for the system UI, close the USB alert
#   build/webos-oe/luna-restart.sh --close    do not restart; just close the USB alert if it is up
#
# WHY THIS IS A SCRIPT. Every LunaSysMgr restart with the USB cable in (which is every restart
# done from a host) ends with the system UI's "Connected — Tap USB Drive…" alert in the top
# right corner, exactly over the Enyo toolbar's Share / New Card / Bookmarks icons. It stays
# until somebody taps Close. Screenshots taken to check the toolbar show the alert instead, and
# until 2026-10-05 the only answer was to wait and hope.
#
# HOW THE ALERT IS CLOSED. The system UI closes it itself when the storage daemon signals that
# USB drive mode is no longer on offer: /usr/lib/luna/system/luna-systemui/data/StoragedService.js
# listens for the bus signal /storaged MSMAvail and calls closeUSBAlerts() on {"mode-avail":false}.
# So that signal is sent by hand:
#
#   luna-send -m com.palm.lunasend -s palm://com.palm.lunasend/storaged/MSMAvail '{"mode-avail":false}'
#
# Two things about that line were found the hard way:
#   -m com.palm.lunasend   the hub forwards a signal only from a sender whose bus name starts
#                          with "com.palm." (ls-hubd security.c, "Only Palm services are allowed
#                          to send signals"). Without -m the signal shows up in ls-monitor and
#                          is delivered to nobody. luna-send's own role file allows it exactly
#                          that one name.
#   the address            the path after the host IS the signal's category and method; the host
#                          part is ignored.
# It changes nothing about the cable: com.palm.storage still answers hostIsConnected:true, and
# the next real plug or unplug puts the system UI back in step. What it does take away with the
# alert is the "USB Drive" entry in the notification area, so USB drive mode has to be entered
# by replugging the cable.
#
# WHEN. The signal only closes an alert that is already open — sent early, the system UI's own
# start-up check (a hostIsConnected call from com.palm.systemui, about 23 s after the restart)
# opens the alert afterwards regardless. So this watches the bus for that call with ls-monitor
# and sends the signal five seconds after it. About 30 s in all, where the blind wait was 75.
set -euo pipefail
MODE="${1:-restart}"

DEV=$(mktemp); trap 'rm -f "$DEV"' EXIT
cat > "$DEV" <<'DEVICE'
CLOSE="luna-send -m com.palm.lunasend -s palm://com.palm.lunasend/storaged/MSMAvail {\"mode-avail\":false}"
if [ "$1" = "--close" ]; then
  $CLOSE >/dev/null 2>&1; sleep 2; echo "luna-restart: USB alert close signal sent"; exit 0
fi
ls-monitor > /tmp/luna-restart.mon 2>&1 &
M=$!; sleep 1
T0=$(date +%s)
killall LunaSysMgr
n=0; seen=0
while [ $n -lt 120 ]; do
  if grep -q 'com.palm.systemui.*hostIsConnected' /tmp/luna-restart.mon; then seen=1; break; fi
  sleep 1; n=$((n+1))
done
kill $M 2>/dev/null; rm -f /tmp/luna-restart.mon
if [ $seen = 1 ]; then
  sleep 5; $CLOSE >/dev/null 2>&1; sleep 2
  echo "luna-restart: system UI up $(( $(date +%s) - T0 )) s after the restart; USB alert closed"
else
  echo "luna-restart: the system UI never checked the cable in 120 s — is LunaSysMgr up? (no close signal sent)"
  exit 1
fi
DEVICE
novacom put file:///tmp/luna-restart.sh < "$DEV"
# novacom stops at once, output lost, if its stdin is at end-of-file — hence the habit of
# `sleep N | novacom run`. But a pipeline lasts as long as its sleep. Holding stdin open with a
# sleep that can be killed gets both: novacom returns the moment the device script ends.
run_held() { # run_held <max seconds> <args for the device script...>
  local max=$1; shift
  exec 3< <(sleep "$max"); local holder=$!
  novacom run file://bin/sh -- /tmp/luna-restart.sh "$@" <&3 2>&1 | tail -1
  exec 3<&-; kill "$holder" 2>/dev/null || true
}
case "$MODE" in
  --close) run_held 15 --close ;;
  *)       run_held 150 ;;
esac
novacom run file://bin/rm -- -f /tmp/luna-restart.sh </dev/null >/dev/null 2>&1 || true
