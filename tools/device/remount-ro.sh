#!/bin/sh
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# On the TouchPad: put / back to read-only after an install or a test left it read-write. It can
# stay busy for a while (a library that was replaced is still mapped until LunaSysMgr restarts),
# so it retries for 30 s.
#   build/webos-oe/dev-sh.sh tools/device/remount-ro.sh
i=0
while [ $i -lt 30 ]; do
	if mount -o remount,ro / 2>/tmp/jihad-ro.err; then echo "read-only after $i retries"; break; fi
	i=$((i + 1)); sync; sleep 1
done
cat /tmp/jihad-ro.err; rm -f /tmp/jihad-ro.err
grep ' / ' /proc/mounts | grep -v rootfs
