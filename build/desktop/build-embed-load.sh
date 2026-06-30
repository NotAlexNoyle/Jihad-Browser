#!/bin/bash
# Copyright 2026 the Jihad Browser project.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Compile + link + run the Goanna page-load test (T-019) against the built
# engine, under a virtual X display (Xvfb). Runs inside the pinned container.
set -uo pipefail

DIST=/out/obj-jihad-goanna/dist
SRC=/jihad/render/goanna
OUT=/out/embed_load

[ -e "$DIST/bin/libxul.so" ] || { echo "ERROR: build the engine first"; exit 1; }

CXX=${CXX:-g++}
GTK_CFLAGS=$(pkg-config --cflags gtk+-2.0 glib-2.0)
GTK_LIBS=$(pkg-config --libs gtk+-2.0 glib-2.0)
CXXFLAGS="-std=gnu++17 -fPIC -fno-rtti -fno-exceptions -pipe -O2 -g0"
INCS="-include $DIST/include/mozilla-config.h -I$DIST/include -I$DIST/include/nspr $GTK_CFLAGS"

echo "== compiling =="
set -x
$CXX $CXXFLAGS $INCS -c "$SRC/EngineHost.cpp"       -o /out/EngineHost.o   || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/test/embed_load.cpp"  -o /out/embed_load.o   || exit 11
echo "== linking =="
$CXX /out/embed_load.o /out/EngineHost.o \
  "$DIST/sdk/lib/libxpcomglue_s.a" \
  -L"$DIST/bin" -lxul \
  "$DIST/sdk/lib/libmozglue.a" \
  -lnspr4 -lplc4 -lplds4 \
  $GTK_LIBS \
  -Wl,-rpath,"$DIST/bin" -ldl -lpthread \
  -o "$OUT" || exit 12
set +x

echo "== running under Xvfb =="
LD_LIBRARY_PATH="$DIST/bin" xvfb-run -a -s "-screen 0 1024x768x24" "$OUT" "$DIST/bin"
rc=$?
echo "== embed_load exit code: $rc =="
exit $rc
