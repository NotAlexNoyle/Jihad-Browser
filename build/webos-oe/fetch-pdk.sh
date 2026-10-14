#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Fetch the Palm PDK (CodeSourcery arm-none-linux-gnueabi gcc 4.3.3 + the webOS-3 device
# sysroot/headers) needed to build the NPAPI BrowserAdapter with the device-host-native
# toolchain. The PDK is HP/Palm PROPRIETARY and is NOT redistributable, so it is not vendored in
# this repo. HP's SDK package (palm-sdk_3.0.5-svn528736-pho676_i386.deb) is preserved on the
# Internet Archive (item webOSLegacySDK); with no argument this script downloads it from there
# (357 MB, once, kept in build/webos-oe/pdk/.download/) and checks its sha256 before using it
# (cavekit-device-build.md R11). It then extracts /opt/PalmPDK into build/webos-oe/pdk/
# (git-ignored), where build-adapter-{pdk,arm}.sh find it (or set PDK_ROOT to another copy).
#
# The .deb is an `ar` archive whose payload is data.tar.{gz,xz}; no dpkg is required.
#
# Usage: build/webos-oe/fetch-pdk.sh                  download, check, extract
#        build/webos-oe/fetch-pdk.sh <path/to/.deb>   use a copy you have (checked the same way)
# A different SDK build fails the check; PDK_ACCEPT_ANY=1 extracts it anyway (the adapter it
# builds is then not the one this repository's adapter checksums were taken from).
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
DEST="$HERE/pdk"
DEB_NAME="palm-sdk_3.0.5-svn528736-pho676_i386.deb"
DEB_URL="${PDK_DEB_URL:-https://archive.org/download/webOSLegacySDK/$DEB_NAME}"
DEB_SHA256="4bbc0e614204232f386a5f338d06af7168be25dc0a8046dc256ea96455efaaf8"
DEB="${1:-}"

if [ -z "$DEB" ]; then
  DEB="$DEST/.download/$DEB_NAME"
  if [ ! -f "$DEB" ] || [ "$(sha256sum "$DEB" | cut -d' ' -f1)" != "$DEB_SHA256" ]; then
    mkdir -p "$DEST/.download"
    echo "downloading $DEB_NAME (357 MB) from the Internet Archive ..."
    curl -fL --retry 3 -o "$DEB.part" "$DEB_URL"
    mv "$DEB.part" "$DEB"
  fi
elif [ ! -f "$DEB" ]; then
  echo "usage: $0 [path/to/$DEB_NAME]" >&2
  exit 1
fi

have="$(sha256sum "$DEB" | cut -d' ' -f1)"
if [ "$have" != "$DEB_SHA256" ]; then
  if [ "${PDK_ACCEPT_ANY:-0}" = 1 ]; then
    echo "WARNING: $(basename "$DEB") is not the pinned SDK build (sha256 $have); extracting anyway"
  else
    echo "!! $(basename "$DEB"): sha256 $have, expected $DEB_SHA256 (PDK_ACCEPT_ANY=1 to use it anyway)" >&2
    exit 2
  fi
fi

mkdir -p "$DEST"
member="$(ar t "$DEB" | grep -E '^data\.tar' | head -1)"
[ -n "$member" ] || { echo "!! no data.tar member in $DEB — not a Debian package?"; exit 2; }
echo "extracting $member from $(basename "$DEB") -> $DEST/ ..."
case "$member" in
  *.gz)  ar p "$DEB" "$member" | tar -C "$DEST" -xzf - ./opt/PalmPDK ;;
  *.xz)  ar p "$DEB" "$member" | tar -C "$DEST" -xJf - ./opt/PalmPDK ;;
  *.bz2) ar p "$DEB" "$member" | tar -C "$DEST" -xjf - ./opt/PalmPDK ;;
  *)     ar p "$DEB" "$member" | tar -C "$DEST" -xf - ./opt/PalmPDK ;;
esac

if [ -d "$DEST/opt/PalmPDK/arm-gcc" ]; then
  echo "OK: PDK ready at $DEST/opt/PalmPDK"
  # The compilers are 32-bit x86 programs; a host without 32-bit libraries cannot run them, and
  # build-adapter-pdk.sh then runs inside the build container instead (build/webos-oe/in-container.sh).
  "$DEST/opt/PalmPDK/arm-gcc/bin/arm-none-linux-gnueabi-gcc" --version 2>/dev/null | head -1 \
    || echo "(this host cannot run the PDK's 32-bit compiler itself; the container will)"
else
  echo "!! extraction did not yield opt/PalmPDK/arm-gcc — check the .deb layout"; exit 3
fi
