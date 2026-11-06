#!/bin/sh
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# sysmgr-restart.sh — LunaSysMgr restarted and the screen left as the owner wants it, in ONE device script (dev-sh.sh):
# with the USB cable in, a restart brings the system's "Connected / USB Drive" alert up in the upper right, and it stays
# until tapped. The owner has asked twice that it never be left up: the tap belongs to the restart, not to a later step.
# The alert draws in fb0, so fbstream never sees it: the screenshot at the end is the evidence (rotated, the upper right).
# Needs /tmp/finger. Usage: bash build/webos-oe/dev-sh.sh -t 120 tools/device/finger/sysmgr-restart.sh
killall -9 LunaSysMgr
n=0; while [ $n -lt 30 ]; do [ "$(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | grep -c returnValue)" = 1 ] && break; sleep 2; n=$((n+1)); done
sleep 45            # the alert comes after the manager answers; a tap before it closes nothing
/tmp/finger tap 583 232
sleep 2
/tmp/finger tap 583 232     # (on the home screen or an empty card view a second tap does nothing)
sleep 1
luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/fl-restart.png"}' >/dev/null 2>&1
echo "LunaSysMgr restarted, the alert's Close tapped; the screen: /tmp/fl-restart.png (the caller fetches and removes it)"
