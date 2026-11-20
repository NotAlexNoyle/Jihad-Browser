#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# kernel-kbd.sh up|down SHELL (host side): an emulated hardware keyboard (render/goanna/test/uinput_kbd.c). LunaSysMgr enumerates
# its inputs at its start, so the device has to stand before a restart, and go before another one; the restarts close the USB alert.
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; ROOT="$(cd "$HERE/../../../../.." && pwd)"
. "$ROOT/build/webos-oe/host-path.sh" 2>/dev/null
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
if [ "$1" = up ]; then
  novacom put file:///tmp/uinput_kbd < "$ROOT/build/webos-oe/out-arm/uinput_kbd"
  # one keyboard only: a second, or one a failed `down` left (2026-11-19: it stood through later scripts, the system
  # keyboard stayed hidden for it and every on-screen tap of theirs fell on the page), is ended by its name first
  printf 'for p in $(pidof uinput_kbd); do kill $p; done; chmod 755 /tmp/uinput_kbd; : > /tmp/uinput.cmd; /tmp/uinput_kbd /tmp/uinput.cmd >/dev/null 2>&1 &\nsleep 2\n' > "$T/u.sh"
  bash "$ROOT/build/webos-oe/dev-sh.sh" -t 20 "$T/u.sh" >/dev/null 2>&1
else
  printf 'echo quit > /tmp/uinput.cmd; sleep 2; for p in $(pidof uinput_kbd); do kill $p; done; rm -f /tmp/uinput_kbd /tmp/uinput.cmd\n' > "$T/u.sh"
  bash "$ROOT/build/webos-oe/dev-sh.sh" -t 20 "$T/u.sh" >/dev/null 2>&1
fi
bash "$ROOT/build/webos-oe/dev-sh.sh" -t 120 "$ROOT/tools/device/finger/sysmgr-restart.sh" >/dev/null 2>&1
novacom run file://bin/rm -- -f /tmp/fl-restart.png < /dev/null
echo "kernel keyboard: $1"
