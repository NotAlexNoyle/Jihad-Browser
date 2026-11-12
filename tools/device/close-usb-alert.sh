#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# close-usb-alert.sh [wait-seconds] : the system's "Connected / USB Drive" alert (upper right) closed if it is up. The
# owner's standing rule, asked four times: after ANY USB drop or replug and after any LunaSysMgr restart the alert is
# closed, never left on the screen. A drop can come in the middle of any device script (the eMMC saturated by a heavy
# page, cavekit-device-headroom.md), so this LOOKS rather than infers: a screenshot, the alert's two opaque buttons
# (USB Drive and Close: dark gray, uniform but for their gradient) read at their place in the portrait screen; when both
# are there, the finger taps Close (583,232) and a second screenshot proves it gone. A screen without the alert is left
# alone (no tap). Every device script calls this at its end and after the device comes back from a drop.
#   tools/device/close-usb-alert.sh          now
#   tools/device/close-usb-alert.sh 300      wait up to 300 s for the device first (after a drop)
set -u
R=$(cd "$(dirname "$0")/../.." && pwd)
export JIHAD_REPO=$R; . "$R/build/webos-oe/host-path.sh"
W=${1:-0}; t=0
while ! novacom -l 2>/dev/null | grep -q topaz; do
	[ $t -ge "$W" ] && { echo "usb-alert: no device"; exit 1; }
	sleep 5; t=$((t+5))
done
[ $t -gt 0 ] && { echo "usb-alert: the device came back after $t s"; sleep 20; }   # the alert comes a while after the link
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT
shot() {   # shot FILE: the screen, portrait, to FILE (png)
	{ echo "luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{\"file\":\"/tmp/ua.png\"}' >/dev/null 2>&1; sleep 2; ls /tmp/ua.png >/dev/null"; sleep 5; } | novacom run file://bin/sh >/dev/null 2>&1
	novacom get file:///tmp/ua.png > "$1" 2>/dev/null
	{ echo 'rm -f /tmp/ua.png'; sleep 2; } | novacom run file://bin/sh >/dev/null 2>&1
}
up() {   # up FILE: 0 when the alert is on it
	python3 - "$1" <<'PY' 2>/dev/null
import sys
from PIL import Image
try:
    im = Image.open(sys.argv[1]).rotate(-90, expand=True).convert('RGB')
except Exception:
    sys.exit(2)
def region(x0, y0, x1, y1):
    px = list(im.crop((x0, y0, x1, y1)).getdata()); n = len(px)
    m = [sum(p[c] for p in px) / n for c in range(3)]
    v = sum(sum((p[c] - m[c]) ** 2 for c in range(3)) for p in px) / n
    return m, v
def bright(x0, y0, x1, y1):
    return sum(1 for p in im.crop((x0, y0, x1, y1)).getdata() if min(p) > 200)
ok = 0
for box in ((440, 224, 520, 240), (440, 177, 520, 193)):   # the Close and USB Drive buttons' text-free left parts
    m, v = region(*box)
    gray = abs(m[0] - m[1]) < 6 and abs(m[1] - m[2]) < 6
    if gray and 30 <= m[1] <= 62 and v < 700: ok += 1
# and their white labels ("Close", "USB Drive"): Mojo's error panel is uniform dark gray too, and has no such text there
# (checked on every saved screenshot in docs/rounds, 2026-11-09: 8 alerts found, all real, no look-alike passed)
ok += 40 <= bright(562, 224, 604, 241) <= 400
ok += 60 <= bright(545, 177, 620, 195) <= 600
sys.exit(0 if ok == 4 else 1)
PY
}
shot "$TMP/a.png"
if up "$TMP/a.png"; then
	# /tmp/finger is gone after a reboot (the device rebooted itself on 2026-11-12 and both taps went nowhere): build it
	if ! { echo '[ -x /tmp/finger ] && echo HAVE'; sleep 3; } | novacom run file://bin/sh 2>/dev/null | grep -q HAVE; then
		TC=$R/build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin/arm-webos-linux-gnueabi-gcc
		"$TC" -O2 -march=armv7-a -mfpu=neon -mfloat-abi=softfp -std=gnu99 -o "$TMP/finger" "$R/tools/device/finger/finger.c" &&
			novacom put file:///tmp/finger < "$TMP/finger" && { echo 'chmod +x /tmp/finger'; sleep 2; } | novacom run file://bin/sh >/dev/null 2>&1
		echo "usb-alert: /tmp/finger was missing, pushed"
	fi
	for try in 1 2; do   # a second tap when the first met the alert while it was still sliding in
		{ echo '/tmp/finger tap 583 232'; sleep 3; } | novacom run file://bin/sh >/dev/null 2>&1
		sleep 2; shot "$TMP/b.png"
		up "$TMP/b.png" || { echo "usb-alert: was up, closed"; exit 0; }
	done
	echo "usb-alert: STILL UP after two taps"; exit 1
fi
echo "usb-alert: none"
