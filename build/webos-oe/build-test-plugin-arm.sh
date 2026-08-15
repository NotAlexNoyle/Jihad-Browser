#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Build the trivial NPAPI test plugin (render/goanna/test/npapi_test_plugin.c) for the device.
#
# This is the CONTROL for cavekit-addons-extensions.md R7. The only real NPAPI binary on the
# TouchPad is the device's own libflashplayer.so, which is linked against LunaSysMgr's WebKit
# host — so when it fails there is no way to tell a broken port from a plugin that will not host
# anywhere else. This plugin has no dependencies beyond libc and does nothing but announce each
# NPAPI callback and paint a solid colour.
#
# Runs on the HOST (no container): it needs only the cross-gcc and UXP's NPAPI headers, which
# are plain C headers with no build-system dependency.
#
# Usage: build/webos-oe/build-test-plugin-arm.sh
# Output: build/webos-oe/out-arm/libjihadtestplugin.so
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
TC="${TC:-$HERE/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi}"
CC="$TC/bin/arm-webos-linux-gnueabi-gcc"
SRC="$ROOT/render/goanna/test/npapi_test_plugin.c"
INC="$ROOT/third_party/uxp/dom/plugins/base"
OUT="$HERE/out-arm/libjihadtestplugin.so"

[ -x "$CC" ] || { echo "ERROR: no cross gcc at $CC" >&2; exit 1; }
[ -f "$SRC" ] || { echo "ERROR: no source at $SRC" >&2; exit 1; }
[ -f "$INC/npapi.h" ] || { echo "ERROR: no npapi.h under $INC" >&2; exit 1; }

mkdir -p "$(dirname "$OUT")"
# -DXP_UNIX is REQUIRED, not decoration: npfunctions.h defines NP_EXPORT only inside
# `#if defined(XP_UNIX)`, so without it every entry point fails to parse.
"$CC" -shared -fPIC -O2 -Wall -Wextra -DXP_UNIX=1 \
      -I"$INC" \
      -o "$OUT" "$SRC"

"$TC/bin/arm-webos-linux-gnueabi-strip" "$OUT"

# A plugin missing any of the four entry points is silently skipped by nsPluginHost and looks
# exactly like a plugin that was never found — so assert them here rather than on the device.
missing=""
for sym in NP_Initialize NP_Shutdown NP_GetMIMEDescription NP_GetValue; do
  "$TC/bin/arm-webos-linux-gnueabi-nm" -D --defined-only "$OUT" | grep -q " $sym\$" || missing="$missing $sym"
done
[ -z "$missing" ] || { echo "ERROR: built plugin is missing entry points:$missing" >&2; exit 2; }

echo "== built: $OUT ($(stat -c%s "$OUT") bytes) =="
"$TC/bin/arm-webos-linux-gnueabi-readelf" -d "$OUT" | awk '/NEEDED/{print "   needs: " $5}'
