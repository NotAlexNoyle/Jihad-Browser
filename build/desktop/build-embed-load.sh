#!/bin/bash
# Copyright 2026 NotAlexNoyle.
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

# The headless embedder has no compositor process; force the in-process
# BasicLayerManager via a default pref read at GRE init (gfxPlatform caches the
# OMTC decision at init, so it must be set before XRE_InitEmbedding2).
if ! grep -q 'jihad-embed' "$DIST/bin/goanna.js" 2>/dev/null; then
cat >> "$DIST/bin/goanna.js" <<'PREFS'
// jihad-embed: force in-process BasicLayerManager (no compositor) for headless
pref("layers.offmainthreadcomposition.enabled", false);
pref("layers.offmainthreadcomposition.force-disabled", true);
pref("layers.acceleration.disabled", true);
pref("gfx.xrender.enabled", false);
PREFS
fi
echo "== ensured embed prefs in goanna.js (OMTC off) =="

echo "== running under Xvfb =="
# JIHAD_DISABLE_OMTC forces the in-process BasicLayerManager (CPU paint), which
# the headless render path needs (engine patch 0003).
export JIHAD_DISABLE_OMTC=1
LD_LIBRARY_PATH="$DIST/bin" xvfb-run -a -s "-screen 0 1024x768x24" "$OUT" "$DIST/bin"
rc=$?
echo "== embed_load exit code: $rc =="
exit $rc
