#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Fetch the Palm PDK (CodeSourcery arm-none-linux-gnueabi gcc 4.3.3 + the webOS-3 device
# sysroot/headers) needed to build the NPAPI BrowserAdapter with the device-host-native
# toolchain. The PDK is HP/Palm PROPRIETARY and is NOT redistributable, so it is not
# vendored in this repo. Supply the SDK .deb yourself (e.g. palm-sdk_3.0.5-*_i386.deb) and
# this extracts /opt/PalmPDK into build/webos-oe/pdk/ (git-ignored). Then
# build-adapter-{pdk,arm}.sh find it there automatically (or set PDK_ROOT to an existing copy).
#
# The .deb is an `ar` archive whose payload is data.tar.{gz,xz}; no dpkg is required.
#
# Usage: build/webos-oe/fetch-pdk.sh <path/to/palm-sdk_3.0.5-*_i386.deb>
set -euo pipefail
DEB="${1:-}"
HERE="$(cd "$(dirname "$0")" && pwd)"
DEST="$HERE/pdk"

if [ -z "$DEB" ] || [ ! -f "$DEB" ]; then
  echo "usage: $0 <path/to/palm-sdk_3.0.5-*_i386.deb>"
  echo "  (the SDK .deb is proprietary; obtain it from an HP webOS SDK archive)"
  exit 1
fi

mkdir -p "$DEST"
member="$(ar t "$DEB" | grep -E '^data\.tar' | head -1)"
[ -n "$member" ] || { echo "!! no data.tar member in $DEB — not a Debian package?"; exit 2; }
echo "extracting $member from $(basename "$DEB") -> $DEST/ ..."
case "$member" in
  *.gz)  ar p "$DEB" "$member" | tar -C "$DEST" -xzf - ;;
  *.xz)  ar p "$DEB" "$member" | tar -C "$DEST" -xJf - ;;
  *.bz2) ar p "$DEB" "$member" | tar -C "$DEST" -xjf - ;;
  *)     ar p "$DEB" "$member" | tar -C "$DEST" -xf - ;;
esac

if [ -d "$DEST/opt/PalmPDK/arm-gcc" ]; then
  echo "OK: PDK ready at $DEST/opt/PalmPDK"
  "$DEST/opt/PalmPDK/arm-gcc/bin/arm-none-linux-gnueabi-gcc" --version 2>/dev/null | head -1 || true
else
  echo "!! extraction did not yield opt/PalmPDK/arm-gcc — check the .deb layout"; exit 3
fi
