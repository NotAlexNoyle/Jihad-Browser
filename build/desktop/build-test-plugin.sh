#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Build the trivial NPAPI test plugin (render/goanna/test/npapi_test_plugin.c) for the DESKTOP
# x86_64 build. Same control as the ARM build (build/webos-oe/build-test-plugin-arm.sh) — see
# that script and the plugin's own header comment for why it exists.
#
# Usage: build/desktop/build-test-plugin.sh
# Output: build/desktop/out/libjihadtestplugin.so
#         Drop it in the desktop profile's plugins/ dir to use it.
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
CC="${CC:-gcc}"
SRC="$ROOT/render/goanna/test/npapi_test_plugin.c"
INC="$ROOT/third_party/uxp/dom/plugins/base"
OUT="$HERE/out/libjihadtestplugin.so"

[ -f "$SRC" ] || { echo "ERROR: no source at $SRC" >&2; exit 1; }
[ -f "$INC/npapi.h" ] || { echo "ERROR: no npapi.h under $INC" >&2; exit 1; }

mkdir -p "$(dirname "$OUT")"
# -DXP_UNIX is REQUIRED: npfunctions.h defines NP_EXPORT only inside `#if defined(XP_UNIX)`.
"$CC" -shared -fPIC -O2 -Wall -Wextra -DXP_UNIX=1 -I"$INC" -o "$OUT" "$SRC"

missing=""
for sym in NP_Initialize NP_Shutdown NP_GetMIMEDescription NP_GetValue; do
  nm -D --defined-only "$OUT" | grep -q " $sym\$" || missing="$missing $sym"
done
[ -z "$missing" ] || { echo "ERROR: built plugin is missing entry points:$missing" >&2; exit 2; }

echo "== built: $OUT ($(stat -c%s "$OUT") bytes) =="
