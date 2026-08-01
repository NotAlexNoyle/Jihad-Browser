#!/bin/bash
# Copyright 2026 NotAlexNoyle.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Compile + link + run the Goanna embedding smoke test (T-013) against the
# built engine. Runs inside the pinned container:
#   /src/uxp  : UXP source (for any generated headers, ro ok)
#   /jihad    : the Jihad-Browser repo (render/goanna sources)
#   /out      : the engine objdir (contains obj-jihad-goanna/dist)
set -uo pipefail

DIST=/out/obj-jihad-goanna/dist
SRC=/jihad/render/goanna
OUT=/out/embed_smoke

if [ ! -e "$DIST/bin/libxul.so" ]; then
  echo "ERROR: libxul.so not found at $DIST/bin (build the engine first)"; exit 1
fi

CXX=${CXX:-g++}
CXXFLAGS="-std=gnu++17 -fPIC -fno-rtti -fno-exceptions -pipe -O2 -g0"
INCS="-include $DIST/include/mozilla-config.h -I$DIST/include -I$DIST/include/nspr"
# Frozen/external embedding API (do not define MOZILLA_INTERNAL_API).

echo "== compiling EngineHost + embed_smoke =="
set -x
$CXX $CXXFLAGS $INCS -c "$SRC/EngineHost.cpp"      -o /out/EngineHost.o    || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/DialogService.cpp"   -o /out/DialogService.o || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/DownloadService.cpp"   -o /out/DownloadService.o || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/test/embed_smoke.cpp" -o /out/embed_smoke.o  || exit 11

echo "== linking against libxul =="
$CXX /out/embed_smoke.o /out/EngineHost.o /out/DialogService.o /out/DownloadService.o \
  "$DIST/sdk/lib/libxpcomglue_s.a" \
  -L"$DIST/bin" -lxul \
  "$DIST/sdk/lib/libmozglue.a" \
  -lnspr4 -lplc4 -lplds4 \
  -Wl,-rpath,"$DIST/bin" \
  -ldl -lpthread \
  -o "$OUT" || exit 12
set +x

echo "== running smoke test =="
LD_LIBRARY_PATH="$DIST/bin" "$OUT" "$DIST/bin"; rc=$?
echo "== embed_smoke exit code: $rc =="
exit $rc
