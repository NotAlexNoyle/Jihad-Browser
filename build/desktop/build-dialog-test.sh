#!/bin/bash
# Copyright 2026 the Jihad Browser project.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
# Build + run the JS-dialog interception test (domain G R3) against the engine.
set -uo pipefail
DIST=/out/obj-jihad-goanna/dist
SRC=/jihad/render/goanna
OUT=/out/dialog_test
[ -e "$DIST/bin/libxul.so" ] || { echo "ERROR: build the engine first"; exit 1; }

CXX=${CXX:-g++}
GTK_CFLAGS=$(pkg-config --cflags gtk+-2.0 glib-2.0); GTK_LIBS=$(pkg-config --libs gtk+-2.0 glib-2.0)
CXXFLAGS="-std=gnu++17 -fPIC -fno-rtti -fno-exceptions -O2 -g0"
INCS="-include $DIST/include/mozilla-config.h -I$DIST/include -I$DIST/include/nspr $GTK_CFLAGS"

set -x
$CXX $CXXFLAGS $INCS -c "$SRC/EngineHost.cpp"       -o /out/EngineHost.o       || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/GoannaRenderPage.cpp" -o /out/GoannaRenderPage.o || exit 11
$CXX $CXXFLAGS $INCS -c "$SRC/DialogService.cpp"    -o /out/DialogService.o    || exit 12
$CXX $CXXFLAGS $INCS -c "$SRC/test/dialog_test.cpp" -o /out/dialog_test.o      || exit 13
$CXX /out/dialog_test.o /out/GoannaRenderPage.o /out/DialogService.o /out/EngineHost.o \
  "$DIST/sdk/lib/libxpcomglue_s.a" -L"$DIST/bin" -lxul "$DIST/sdk/lib/libmozglue.a" \
  -lnspr4 -lplc4 -lplds4 $GTK_LIBS -Wl,-rpath,"$DIST/bin" -ldl -lpthread -o "$OUT" || exit 14
set +x

# OMTC off (engine patch 0003).
grep -q 'jihad-embed' "$DIST/bin/goanna.js" 2>/dev/null || \
  echo 'pref("layers.offmainthreadcomposition.force-disabled", true); // jihad-embed' >> "$DIST/bin/goanna.js"
export JIHAD_DISABLE_OMTC=1
LD_LIBRARY_PATH="$DIST/bin" xvfb-run -a -s "-screen 0 1024x768x24" "$OUT" "$DIST/bin"
echo "== dialog_test exit: $? =="
