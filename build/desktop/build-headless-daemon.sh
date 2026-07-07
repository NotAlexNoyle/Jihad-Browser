#!/bin/bash
# Copyright 2026 the Jihad Browser project.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Build the jihad-browserserver daemon + adapter against the TRUE HEADLESS libxul
# (obj-jihad-headless, MOZ_WIDGET_TOOLKIT=headless — libxul has NO gtk/gdk/X/pango,
# only freetype/fontconfig). Then run the full YAP round-trip with NO X SERVER at
# all (DISPLAY unset, no xvfb) via the JIHAD_OFFSCREEN PuppetWidget path. This is
# the desktop proxy for the on-device (TouchPad, no X) configuration.
#
# The daemon executable itself still links gtk on the desktop only for its unused
# on-screen fallback path in GoannaRenderPage.cpp; the engine (libxul) it drives is
# entirely gtk-free. Making the daemon gtk-free is the ARM/device build's job.
set -uo pipefail
DIST=/out/obj-jihad-headless/dist
R=/jihad/render
BS=/jihad/render/browserserver
[ -e "$DIST/bin/libxul.so" ] || { echo "ERROR: build the headless engine first (mozconfig.goanna-headless)"; exit 1; }

CXX=${CXX:-g++}
CXXFLAGS="-std=gnu++17 -fPIC -fno-rtti -fno-exceptions -O2 -g0"
GLIB=$(pkg-config --cflags glib-2.0 gthread-2.0);  GLIBL=$(pkg-config --libs glib-2.0 gthread-2.0)
GTKC=$(pkg-config --cflags gtk+-2.0);              GTKL=$(pkg-config --libs gtk+-2.0)
ENGINC="-include $DIST/include/mozilla-config.h -I$DIST/include -I$DIST/include/nspr"
YAPINC="-I$BS/Yap -I$BS/Src"
XULLINK="$DIST/sdk/lib/libxpcomglue_s.a -L$DIST/bin -lxul $DIST/sdk/lib/libmozglue.a -lnspr4 -lplc4 -lplds4"

echo "== compiling libYap (incl YapClient) + BrowserServerBase =="
for f in YapPacket YapProxy YapServer YapClient; do
  $CXX $CXXFLAGS $YAPINC $GLIB -c "$BS/Yap/$f.cpp" -o "/out/$f.o" || exit 10
done
$CXX $CXXFLAGS $YAPINC $GLIB -c "$BS/Src/BrowserServerBase.cpp" -o /out/BrowserServerBase.o || exit 11

echo "== compiling Goanna backend + daemon (against headless dist) =="
$CXX $CXXFLAGS $ENGINC $GTKC -c "$R/goanna/EngineHost.cpp"        -o /out/EngineHost.o        || exit 12
$CXX $CXXFLAGS $ENGINC $GTKC -c "$R/goanna/DialogService.cpp"     -o /out/DialogService.o     || exit 12
$CXX $CXXFLAGS $ENGINC $GTKC -c "$R/goanna/DownloadService.cpp"   -o /out/DownloadService.o   || exit 12
$CXX $CXXFLAGS $ENGINC $GTKC -c "$R/goanna/GoannaRenderPage.cpp"  -o /out/GoannaRenderPage.o  || exit 13
$CXX $CXXFLAGS $ENGINC $GTKC -c "$R/goanna/BrowserPageGoanna.cpp" -o /out/BrowserPageGoanna.o || exit 14
$CXX $CXXFLAGS $YAPINC $GLIB -c "$BS/JihadBrowserServer.cpp"      -o /out/JihadBrowserServer.o || exit 15
$CXX $CXXFLAGS $YAPINC $ENGINC $GTKC $GLIB -c "$BS/Main.cpp"      -o /out/bs_main.o           || exit 16
$CXX /out/bs_main.o /out/JihadBrowserServer.o /out/BrowserServerBase.o \
     /out/YapPacket.o /out/YapProxy.o /out/YapServer.o \
     /out/BrowserPageGoanna.o /out/GoannaRenderPage.o /out/EngineHost.o /out/DialogService.o /out/DownloadService.o \
     $XULLINK $GTKL $GLIBL -Wl,-rpath,"$DIST/bin" -ldl -lpthread -o /out/jihad-browserserver-headless || exit 17

echo "== compiling adapter client =="
$CXX $CXXFLAGS $YAPINC $GLIB -c "$BS/test/adapter_client.cpp" -o /out/adapter_client.o || exit 18
$CXX /out/adapter_client.o /out/YapClient.o /out/YapPacket.o \
     $GLIBL -ldl -lpthread -o /out/jihad-adapter || exit 19
echo "== built headless daemon + adapter =="

if ! grep -q 'offmainthreadcomposition.force-disabled' "$DIST/bin/goanna.js" 2>/dev/null; then
  echo 'pref("layers.offmainthreadcomposition.force-disabled", true);' >> "$DIST/bin/goanna.js"
fi

echo "== headless round-trip (NO X server, DISPLAY unset) =="
unset DISPLAY
export JIHAD_DISABLE_OMTC=1
export JIHAD_OFFSCREEN=1
export LD_LIBRARY_PATH="$DIST/bin"
/out/jihad-browserserver-headless "$DIST/bin" &
dpid=$!
sleep 8
timeout 40 /out/jihad-adapter
rc=$?
kill "$dpid" 2>/dev/null; wait 2>/dev/null
echo "== headless round-trip exit: $rc =="
exit $rc
