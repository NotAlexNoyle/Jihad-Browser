#!/bin/bash
# Copyright 2026 the Jihad Browser project.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Compile + link + run the GoannaRenderPage backend class via its driver,
# against the built engine, under Xvfb. Same flags as build-embed-load.sh.
set -uo pipefail
DIST=/out/obj-jihad-goanna/dist
SRC=/jihad/render/goanna
OUT=/out/focus_test
[ -e "$DIST/bin/libxul.so" ] || { echo "ERROR: build the engine first"; exit 1; }

CXX=${CXX:-g++}
GTK_CFLAGS=$(pkg-config --cflags gtk+-2.0 glib-2.0)
GTK_LIBS=$(pkg-config --libs gtk+-2.0 glib-2.0)
CXXFLAGS="-std=gnu++17 -fPIC -fno-rtti -fno-exceptions -pipe -O2 -g0"
INCS="-include $DIST/include/mozilla-config.h -I$DIST/include -I$DIST/include/nspr $GTK_CFLAGS"

echo "== compiling GoannaRenderPage + focus_test =="
set -x
$CXX $CXXFLAGS $INCS -c "$SRC/EngineHost.cpp"         -o /out/EngineHost.o        || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/DialogService.cpp"      -o /out/DialogService.o     || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/DownloadService.cpp"      -o /out/DownloadService.o     || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/GoannaRenderPage.cpp"   -o /out/GoannaRenderPage.o  || exit 11
$CXX $CXXFLAGS $INCS -c "$SRC/BrowserPageGoanna.cpp"  -o /out/BrowserPageGoanna.o || exit 12
$CXX $CXXFLAGS $INCS -c "$SRC/test/focus_test.cpp"   -o /out/focus_test.o       || exit 13
$CXX /out/focus_test.o /out/BrowserPageGoanna.o /out/GoannaRenderPage.o /out/EngineHost.o /out/DialogService.o /out/DownloadService.o \
  "$DIST/sdk/lib/libxpcomglue_s.a" -L"$DIST/bin" -lxul "$DIST/sdk/lib/libmozglue.a" \
  -lnspr4 -lplc4 -lplds4 $GTK_LIBS -Wl,-rpath,"$DIST/bin" -ldl -lpthread -o "$OUT" || exit 14
set +x

# OMTC off + gfx prefs (engine patch 0003).
if ! grep -q 'jihad-embed' "$DIST/bin/goanna.js" 2>/dev/null; then
cat >> "$DIST/bin/goanna.js" <<'PREFS'
// jihad-embed
pref("layers.offmainthreadcomposition.force-disabled", true);
PREFS
fi
export JIHAD_DISABLE_OMTC=1
# This is a mobile browser: honor <meta name=viewport> (off by default on desktop
# Gecko). Append unconditionally so it wins.
echo 'pref("dom.meta-viewport.enabled", true);' >> "$DIST/bin/goanna.js"

echo "== running under Xvfb =="
LD_LIBRARY_PATH="$DIST/bin" xvfb-run -a -s "-screen 0 1024x768x24" "$OUT" "$DIST/bin"
echo "== focus_test exit: $? =="
