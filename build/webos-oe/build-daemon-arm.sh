#!/bin/bash
# Copyright 2026 NotAlexNoyle.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Cross-build the jihad-browserserver daemon for the webOS-3 TouchPad (ARMv7 softfp).
# Same sources as build-daemon.sh, cross-compiled with the crosstool-NG toolchain
# against the ARM libxul dist + the Jessie armel GTK sysroot. No smoke run here
# (the binary is ARM — it runs on the device, launched via the bundled ld-2.23.so).
#
# Mounts (same as build-goanna-arm.sh) + /jihad = Jihad-Browser repo.
set -uo pipefail
DIST=/out/obj-jihad-goanna-arm/dist
R=/jihad/render
BS=/jihad/render/browserserver
OUT=/out/jihad-browserserver-arm
[ -e "$DIST/bin/libxul.so" ] || { echo "ERROR: cross-build libxul first (build-goanna-arm.sh build)"; exit 1; }

export PATH=/tc/bin:$PATH
export PKG_CONFIG_SYSROOT_DIR=/sysroot
export PKG_CONFIG_LIBDIR=/sysroot/usr/lib/arm-linux-gnueabi/pkgconfig:/sysroot/usr/share/pkgconfig:/sysroot/usr/lib/pkgconfig
export PKG_CONFIG_PATH=$PKG_CONFIG_LIBDIR

CXX=/tc/bin/arm-webos-linux-gnueabi-g++
ARMFLAGS="-march=armv7-a -mfpu=neon -mfloat-abi=softfp"
SYSINC="-I/sysroot/usr/include -I/sysroot/usr/include/arm-linux-gnueabi"
SYSLIB="-L/sysroot/usr/lib/arm-linux-gnueabi -Wl,-rpath-link,/sysroot/usr/lib/arm-linux-gnueabi"
# JIHAD_OFFSCREEN_ONLY: GTK-free device daemon (matches the headless libxul). Only
# the PuppetWidget offscreen render path is compiled; no gtk/gdk/pango/cairo/X needed.
CXXFLAGS="-std=gnu++17 -fPIC -fno-rtti -fno-exceptions -Os -g0 -DJIHAD_OFFSCREEN_ONLY $ARMFLAGS $SYSINC"
GLIB=$(pkg-config --cflags glib-2.0 gthread-2.0);  GLIBL=$(pkg-config --libs glib-2.0 gthread-2.0)
ENGINC="-include $DIST/include/mozilla-config.h -I$DIST/include -I$DIST/include/nspr"
YAPINC="-I$BS/Yap -I$BS/Src"

# BrowserServerBase.cpp is GENERATED and its 0x1600 arm is hand-written, so a regeneration
# silently drops it and the third framebuffer stops being accepted. Nothing about that fails
# to compile, so it has to be checked as text. Exit 9 is otherwise unused here.
echo "== [ARM] YAP wire-contract guard =="
bash /jihad/build/webos-oe/check-yap-contract.sh /jihad || exit 9

echo "== [ARM] libYap + BrowserServerBase =="
for f in YapPacket YapProxy YapServer; do
  $CXX $CXXFLAGS $YAPINC $GLIB -c "$BS/Yap/$f.cpp" -o "/out/arm-$f.o" || exit 10
done
$CXX $CXXFLAGS $YAPINC $GLIB -c "$BS/Src/BrowserServerBase.cpp" -o /out/arm-BrowserServerBase.o || exit 11

echo "== [ARM] Goanna backend =="
for f in EngineHost DialogService DownloadService JihadCertStore GoannaRenderPage BrowserPageGoanna; do
  $CXX $CXXFLAGS $ENGINC $YAPINC $GLIB -c "$R/goanna/$f.cpp" -o "/out/arm-$f.o" || exit 12
done

echo "== [ARM] PmLogLib named-semaphore interposition =="
# Plain C, and it must live in the PROGRAM IMAGE rather than in any library: a plugin
# dlopened later resolves its PLT against the global scope, which the executable heads.
# Without it the daemon's plugin scan dlopens the device's Flash, PmLogLib's constructor
# sem_wait()s a glibc-2.8-created named semaphore our glibc 2.23 reads as empty, and the
# daemon deadlocks before it has logged a single line. See render/goanna/JihadPmLogSem.c.
CC=/tc/bin/arm-webos-linux-gnueabi-gcc
$CC -std=gnu99 -fPIC -Os -g0 $ARMFLAGS -c "$R/goanna/JihadPmLogSem.c" -o /out/arm-JihadPmLogSem.o || exit 14

echo "== [ARM] JihadBrowserServer + Main =="
$CXX $CXXFLAGS $YAPINC $GLIB -c "$BS/JihadBrowserServer.cpp" -o /out/arm-JihadBrowserServer.o || exit 15
$CXX $CXXFLAGS $YAPINC $ENGINC $GLIB -c "$BS/JihadLunaService.cpp" -o /out/arm-JihadLunaService.o || exit 15
$CXX $CXXFLAGS $YAPINC $ENGINC $GLIB -c "$BS/Main.cpp" -o /out/arm-bs_main.o || exit 16

echo "== [ARM] linking jihad-browserserver-arm =="
$CXX $ARMFLAGS /out/arm-bs_main.o /out/arm-JihadBrowserServer.o /out/arm-JihadLunaService.o /out/arm-BrowserServerBase.o \
     /out/arm-JihadPmLogSem.o \
     /out/arm-YapPacket.o /out/arm-YapProxy.o /out/arm-YapServer.o \
     /out/arm-BrowserPageGoanna.o /out/arm-GoannaRenderPage.o /out/arm-EngineHost.o \
     /out/arm-DialogService.o /out/arm-DownloadService.o /out/arm-JihadCertStore.o \
     "$DIST/sdk/lib/libxpcomglue_s.a" -L"$DIST/bin" -Wl,-rpath-link,"$DIST/bin" -lxul "$DIST/sdk/lib/libmozglue.a" \
     -lnspr4 -lplc4 -lplds4 $SYSLIB $GLIBL -ldl -lpthread \
     -o "$OUT" || exit 17
echo "== built: $OUT =="
/tc/bin/arm-webos-linux-gnueabi-strip "$OUT" 2>/dev/null || true
file "$OUT" 2>/dev/null || true
