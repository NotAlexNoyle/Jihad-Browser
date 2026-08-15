#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Build the PIpc/Piranha interpose shim (render/goanna/test/pipc_shim.c) for the device.
#
# Diagnostic-only: LD_PRELOADed into the plugin-container, it logs (and forwards, unchanged)
# every call to Flash's surface-factory dependencies — PIpcClient ctor, PIpcBuffer::create/data,
# PGContext::create, PGThreadGlobalContext::graphicsContext — to answer whether Flash's own
# render-surface factory (libflashplayer.so 0x56390) ever runs, and where it fails if it does.
# See the header of pipc_shim.c for deployment.
#
# Usage: build/webos-oe/build-pipc-shim-arm.sh
# Output: build/webos-oe/out-arm/jihad-pipc-shim.so
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
TC="${TC:-$HERE/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi}"
CC="$TC/bin/arm-webos-linux-gnueabi-gcc"
SRC="$ROOT/render/goanna/test/pipc_shim.c"
OUT="$HERE/out-arm/jihad-pipc-shim.so"

[ -x "$CC" ] || { echo "ERROR: no cross gcc at $CC" >&2; exit 1; }
[ -f "$SRC" ] || { echo "ERROR: no source at $SRC" >&2; exit 1; }

mkdir -p "$(dirname "$OUT")"
# NOT stripped: the shim's own frames should stay legible in a crash backtrace.
"$CC" -shared -fPIC -O1 -g -Wall -Wextra -o "$OUT" "$SRC" -ldl

# Assert the interposed symbols are exported with default visibility — a hidden symbol here
# means the shim silently intercepts nothing (the exact trap NP_EXPORT hit with gcc_hidden.h).
missing=""
for sym in _ZN10PIpcClientC2ERKSsS1_P10_GMainLoop _ZN10PIpcClientC1ERKSsS1_P10_GMainLoop \
           _ZN10PIpcBuffer6createEi _ZNK10PIpcBuffer4dataEv _ZN9PGContext6createEv \
           _ZN9PGSurfaceC1Ev _ZN11PSoftPixmap3SetE7PFormatPKvjb \
           _ZN21PGThreadGlobalContext15graphicsContextEv; do
  "$TC/bin/arm-webos-linux-gnueabi-nm" -D --defined-only "$OUT" | grep -q " $sym\$" || missing="$missing $sym"
done
[ -z "$missing" ] || { echo "ERROR: shim is missing exports:$missing" >&2; exit 2; }

echo "== built: $OUT ($(stat -c%s "$OUT") bytes) =="
"$TC/bin/arm-webos-linux-gnueabi-readelf" -d "$OUT" | awk '/NEEDED/{print "   needs: " $5}'
