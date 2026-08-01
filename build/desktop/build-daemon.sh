#!/bin/bash
# Copyright 2026 NotAlexNoyle.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Build the real jihad-browserserver daemon on desktop: libYap (Qt-free) +
# BrowserServerBase (YAP dispatch) + JihadBrowserServer + the Goanna backend,
# linked against libxul. Then a smoke start (comes up + listens, then exits).
set -uo pipefail
DIST=/out/obj-jihad-goanna/dist
R=/jihad/render
BS=/jihad/render/browserserver
OUT=/out/jihad-browserserver
[ -e "$DIST/bin/libxul.so" ] || { echo "ERROR: build the engine first"; exit 1; }

CXX=${CXX:-g++}
CXXFLAGS="-std=gnu++17 -fPIC -fno-rtti -fno-exceptions -O2 -g0"
GLIB=$(pkg-config --cflags glib-2.0 gthread-2.0)
GLIBL=$(pkg-config --libs glib-2.0 gthread-2.0)
GTK_CFLAGS=$(pkg-config --cflags gtk+-2.0)
GTK_LIBS=$(pkg-config --libs gtk+-2.0)
ENGINC="-include $DIST/include/mozilla-config.h -I$DIST/include -I$DIST/include/nspr"
YAPINC="-I$BS/Yap -I$BS/Src"

echo "== compiling libYap (core, Qt-free) + BrowserServerBase =="
set -x
for f in YapPacket YapProxy YapServer; do
  $CXX $CXXFLAGS $YAPINC $GLIB -c "$BS/Yap/$f.cpp" -o "/out/$f.o" || exit 10
done
$CXX $CXXFLAGS $YAPINC $GLIB -c "$BS/Src/BrowserServerBase.cpp" -o /out/BrowserServerBase.o || exit 11

echo "== compiling Goanna backend =="
$CXX $CXXFLAGS $ENGINC $GTK_CFLAGS -c "$R/goanna/EngineHost.cpp"        -o /out/EngineHost.o        || exit 12
$CXX $CXXFLAGS $ENGINC $GTK_CFLAGS -c "$R/goanna/DialogService.cpp"     -o /out/DialogService.o     || exit 12
$CXX $CXXFLAGS $ENGINC $GTK_CFLAGS -c "$R/goanna/DownloadService.cpp"     -o /out/DownloadService.o     || exit 12
$CXX $CXXFLAGS $ENGINC $GTK_CFLAGS -c "$R/goanna/GoannaRenderPage.cpp"  -o /out/GoannaRenderPage.o  || exit 13
$CXX $CXXFLAGS $ENGINC $GTK_CFLAGS -c "$R/goanna/BrowserPageGoanna.cpp" -o /out/BrowserPageGoanna.o || exit 14

echo "== compiling JihadBrowserServer + Main =="
$CXX $CXXFLAGS $YAPINC $GLIB -c "$BS/JihadBrowserServer.cpp" -o /out/JihadBrowserServer.o || exit 15
$CXX $CXXFLAGS $YAPINC $ENGINC $GTK_CFLAGS $GLIB -c "$BS/Main.cpp" -o /out/bs_main.o || exit 16

echo "== linking jihad-browserserver =="
$CXX /out/bs_main.o /out/JihadBrowserServer.o /out/BrowserServerBase.o \
     /out/YapPacket.o /out/YapProxy.o /out/YapServer.o \
     /out/BrowserPageGoanna.o /out/GoannaRenderPage.o /out/EngineHost.o /out/DialogService.o /out/DownloadService.o \
     "$DIST/sdk/lib/libxpcomglue_s.a" -L"$DIST/bin" -lxul "$DIST/sdk/lib/libmozglue.a" \
     -lnspr4 -lplc4 -lplds4 $GTK_LIBS $GLIBL -Wl,-rpath,"$DIST/bin" -ldl -lpthread \
     -o "$OUT" || exit 17
set +x
echo "== built: $OUT ($(du -h "$OUT" | cut -f1)) =="

# F-8: the guard greps for a 'jihad-embed' marker, so the appended line has to
# CARRY it — without it the guard never matched its own output and every run
# appended another copy of the pref to the SHARED build output goanna.js.
if ! grep -q 'jihad-embed' "$DIST/bin/goanna.js" 2>/dev/null; then
  echo 'pref("layers.offmainthreadcomposition.force-disabled", true); // jihad-embed' >> "$DIST/bin/goanna.js"
fi
export JIHAD_DISABLE_OMTC=1

echo "== smoke start (engine up + listening; auto-stop) =="
LD_LIBRARY_PATH="$DIST/bin" timeout 12 xvfb-run -a -s "-screen 0 1024x768x24" "$OUT" "$DIST/bin" &
pid=$!
sleep 9
if kill -0 "$pid" 2>/dev/null; then echo "DAEMON_UP (listening on YAP socket)"; else echo "DAEMON_EXITED_EARLY"; fi
kill "$pid" 2>/dev/null; wait 2>/dev/null
