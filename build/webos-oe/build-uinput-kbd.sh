#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Build the synthetic hardware keyboard (render/goanna/test/uinput_kbd.c) for the device.
#
# This exists because the keyboard-arbitration criterion in cavekit-addons-extensions.md R7
# CANNOT BE TESTED without it. The chrome half of that criterion lives in
# BrowserAdapter::handleKeyDown, which runs inside LunaSysMgr, so only a key that LunaSysMgr
# itself dispatches exercises it — and the daemon's `key` inject command goes straight to the
# page, bypassing the adapter entirely. Meanwhile the TouchPad has no keyboard to press:
# /proc/bus/input/devices lists only gpio-keys, the PMIC power key and headset detect, and the
# on-screen keyboard is drawn by LunaSysMgr in-process rather than through /dev/input.
#
# Built with the PDK (gcc 4.3.3 + the device sysroot), NOT the crosstool gcc9, so it links
# against the DEVICE's glibc 2.8 and runs directly — no bundled ld-2.23.so, and none of the
# cross-ABI traps in cavekit-device-build.md R9 apply to it.
#
# Runs on the HOST (no container). Usage: build/webos-oe/build-uinput-kbd.sh
# Output: build/webos-oe/out-arm/uinput_kbd
#
# Deploy + drive:
#   novacom put file:///tmp/uinput_kbd < build/webos-oe/out-arm/uinput_kbd
#   # then, on device: chmod 755 and start it BEFORE restarting LunaSysMgr, so LunaSysMgr
#   # enumerates the new input device at startup:
#   /tmp/uinput_kbd /tmp/uinput.cmd > /tmp/uinput.log 2>&1 &
#   killall LunaSysMgr            # let it pick the device up
#   echo "key 108 5" > /tmp/uinput.cmd    # 5x KEY_DOWN;  30=A 103=UP 105=LEFT 106=RIGHT 108=DOWN
#   echo quit       > /tmp/uinput.cmd     # destroy the device and exit
#
# STATUS 2026-08-10: the device registers correctly (`N: Name="Jihad Test Keyboard" ... event3`)
# and BOTH /usr/bin/hidd and LunaSysMgr open /dev/input/event3 — but no key ever reaches the
# browser card. hidd reads it and does not dispatch. See R7 for where that trail continues.
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)

# Same PDK discovery as build-adapter-pdk.sh — proprietary, not in the repo.
PDK="${PDK_ROOT:-$HERE/pdk/opt/PalmPDK}"
[ -d "$PDK/arm-gcc" ] || PDK="$ROOT/toolchains/opt/PalmPDK"
[ -d "$PDK/arm-gcc" ] || {
  echo "!! PDK not found. Run build/webos-oe/fetch-pdk.sh <palm-sdk_*.deb>, or set PDK_ROOT=<dir with arm-gcc/>." >&2
  exit 2
}

CC="$PDK/arm-gcc/bin/arm-none-linux-gnueabi-gcc"
SYSROOT="$PDK/arm-gcc/sysroot"
SRC="$ROOT/render/goanna/test/uinput_kbd.c"
OUT="$HERE/out-arm/uinput_kbd"

mkdir -p "$(dirname "$OUT")"
# --sysroot is REQUIRED: without it this gcc finds no headers at all, not even stdio.h.
"$CC" --sysroot="$SYSROOT" -O2 -std=gnu99 -Wall -o "$OUT" "$SRC"

file "$OUT"
echo "== built $OUT ($(md5sum "$OUT" | cut -d' ' -f1)) =="
