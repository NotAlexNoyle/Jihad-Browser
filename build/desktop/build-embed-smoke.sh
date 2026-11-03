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
GTK_CFLAGS=$(pkg-config --cflags gtk+-2.0 glib-2.0); GTK_LIBS=$(pkg-config --libs gtk+-2.0 glib-2.0)
CXXFLAGS="-std=gnu++17 -fPIC -fno-rtti -fno-exceptions -pipe -O2 -g0"
# 2026-11-03: EngineHost needs GDK and GoannaRenderPage (the stall sampler) since T-515; as build-tls-test.sh
INCS="-include $DIST/include/mozilla-config.h -I$DIST/include -I$DIST/include/nspr -I$DIST/include/nss -I/jihad/render/browserserver/Src $GTK_CFLAGS"
# Frozen/external embedding API (do not define MOZILLA_INTERNAL_API).

echo "== compiling EngineHost + embed_smoke =="
set -x
$CXX $CXXFLAGS $INCS -c "$SRC/EngineHost.cpp"      -o /out/EngineHost.o    || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/DialogService.cpp"   -o /out/DialogService.o || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/DownloadService.cpp"   -o /out/DownloadService.o || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/GoannaRenderPage.cpp"  -o /out/GoannaRenderPage.o || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/test/embed_smoke.cpp" -o /out/embed_smoke.o  || exit 11

echo "== linking against libxul =="
$CXX /out/embed_smoke.o /out/EngineHost.o /out/DialogService.o /out/DownloadService.o /out/GoannaRenderPage.o \
  "$DIST/sdk/lib/libxpcomglue_s.a" \
  -L"$DIST/bin" -lxul \
  "$DIST/sdk/lib/libmozglue.a" \
  -lnspr4 -lplc4 -lplds4 $GTK_LIBS \
  -Wl,-rpath,"$DIST/bin" \
  -ldl -lpthread \
  -o "$OUT" || exit 12
set +x

echo "== running smoke test =="
LD_LIBRARY_PATH="$DIST/bin" xvfb-run -a -s "-screen 0 1024x768x24" "$OUT" "$DIST/bin"; rc=$?   # under Xvfb as the other tests (GTK needs a display)
echo "== embed_smoke exit code: $rc =="
exit $rc
