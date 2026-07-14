#!/bin/bash
# Copyright 2026 the Jihad Browser project. Apache-2.0.
#
# Cross-build the isis BrowserAdapter NPAPI plugin with the PDK's GCC 4.3.3 — the
# HOST-NATIVE toolchain for the device WebKit process. The stock adapter is built
# this way; matching it avoids the gcc9 static-libstdc++ ABI collision that crashes
# LunaSysMgr when a gcc9-built plugin is dlopen'd into the gcc4 WebKit host.
#
# vs build-adapter-arm.sh (gcc9): NO static libstdc++, NO from-source pbnjson, NO
# _GLIBCXX_USE_CXX11_ABI juggling. We link the device's own libstdc++/Qt4/pbnjson_cpp
# dynamically (same libs the host already has loaded). pbnjson headers are taken from
# tag submissions/10 (the device era) so gcc4 instantiates exactly what the device
# libpbnjson_cpp exports. It speaks the isis YAP generation the Jihad daemon implements.
set -uo pipefail
ROOT=/home/notalexnoyle/eclipse-workspace/Jihad
DEPS=$ROOT/Jihad-Browser/build/webos-oe/adapter-deps
PDK=$ROOT/toolchains/opt/PalmPDK
CXX=$PDK/arm-gcc/bin/arm-none-linux-gnueabi-g++
CC=$PDK/arm-gcc/bin/arm-none-linux-gnueabi-gcc
STRIP=$PDK/arm-gcc/bin/arm-none-linux-gnueabi-strip
SYSROOT=$PDK/arm-gcc/sysroot
JSR=$ROOT/Jihad-Browser/build/webos-oe/arm-sysroot/root     # jessie: glib headers only
QT=$DEPS/qt4-extract/usr/include/qt4
PBN=$DEPS/libpbnjson                                         # checked out at submissions/10
YAPDIR=$ROOT/Jihad-Browser/render/browserserver
OUT=$DEPS/build-pdk; mkdir -p "$OUT"; rm -f "$OUT"/*.o
# Remove the stale monolithic output from before the shim/impl split, so an incremental build can
# never silently deploy the old single BrowserAdapter.so instead of the shim (Jihad review F-181).
rm -f "$OUT/BrowserAdapter.so"

ARM="-march=armv7-a -mfpu=neon -mfloat-abi=softfp"
COMMON="--sysroot=$SYSROOT -fno-exceptions -fno-rtti -fvisibility=hidden -fPIC -O2 -g0 -DXP_UNIX -DXP_WEBOS -DNDEBUG $ARM"
GLIBINC="-I$JSR/usr/include/glib-2.0 -I$JSR/usr/lib/arm-linux-gnueabi/glib-2.0/include"
QTINC="-I$QT -I$QT/QtCore -I$QT/QtGui -I$QT/QtNetwork"
PDKINC="-I$PDK/include"
PBNINC="-I$PBN/include/public"
NPINC="-I$DEPS/staging/include -I$DEPS/staging/include/webkit/npapi"
INC="$NPINC $PBNINC $GLIBINC $QTINC $PDKINC -I$ROOT/Jihad-Browser/render/adapter -I$YAPDIR/Yap -I$YAPDIR/Src -I$ROOT/Jihad-Browser/render/adapter"

compile(){ echo "  CXX $(basename "$1")"; $CXX $COMMON $INC -c "$1" -o "$2" || { echo "!! FAILED: $1"; exit 20; }; }

echo "== [pdk-adapter] AdapterBase =="
compile "$ROOT/Jihad-Browser/render/adapter/AdapterBase.cpp" "$OUT/AdapterBase.o"

echo "== [pdk-adapter] Yap (Jihad daemon's — wire-identical) =="
for f in YapClient YapPacket YapProxy YapServer OffscreenBuffer BufferLock ProcessMutex; do
  compile "$YAPDIR/Yap/$f.cpp" "$OUT/$f.o"
done
compile "$YAPDIR/Src/IpcBuffer.cpp" "$OUT/IpcBuffer.o"

echo "== [pdk-adapter] glib_compat (g_malloc0_n shim for device glib 2.16) =="
$CC --sysroot=$SYSROOT -std=gnu99 -fvisibility=hidden -fPIC -O2 -g0 $ARM $GLIBINC \
  -c "$DEPS/glib_compat.c" -o "$OUT/glib_compat.o" || { echo "!! FAILED glib_compat"; exit 21; }

echo "== [pdk-adapter] BrowserAdapter sources =="
for f in BrowserClientBase BrowserAdapter BrowserAdapterManager Rectangle UrlInfo \
         InteractiveInfo ElementInfo ImageInfo JsonNPObject NPObjectEvent KineticScroller BrowserOffscreen; do
  compile "$ROOT/Jihad-Browser/render/adapter/$f.cpp" "$OUT/$f.o"
done

# The impl (BrowserAdapterImpl.so) holds ALL the adapter logic. It is loaded by the
# shim from the app bundle per card open, NOT by LunaSysMgr directly. NOTE: $OUT/*.o must
# be the impl objects only — the shim object is built into $OUT/shim/ so this glob skips it.
echo "== [pdk-adapter] link BrowserAdapterImpl.so (device libstdc++/Qt/pbnjson, dynamic) =="
$CXX --sysroot=$SYSROOT $ARM -shared -fPIC -fvisibility=hidden \
  -Wl,--version-script="$ROOT/Jihad-Browser/render/adapter/BrowserAdapter.exports" \
  "$OUT"/*.o \
  -L"$DEPS/staging/lib" -lQtGui -lQtCore -lQtNetwork -lpbnjson_cpp -lpbnjson_c -lyajl -lpng12 \
  -L"$JSR/usr/lib/arm-linux-gnueabi" -lglib-2.0 -lgthread-2.0 \
  -lrt -lpthread \
  -Wl,-rpath-link,"$DEPS/staging/lib" -Wl,-rpath-link,"$JSR/usr/lib/arm-linux-gnueabi" \
  -o "$OUT/BrowserAdapterImpl.so" || { echo "!! LINK FAILED"; exit 22; }
$STRIP "$OUT/BrowserAdapterImpl.so"
echo "== built: $OUT/BrowserAdapterImpl.so =="; ls -la "$OUT/BrowserAdapterImpl.so" | awk '{print " size",$5}'

# The shim (BrowserAdapterJihad.so) is the STABLE plugin LunaSysMgr caches at boot. It
# contains no browser logic — only NPAPI entry points + NPP_* forwarders + dlopen of the
# impl. Minimal deps (npapi headers + libdl); no Qt/glib/pbnjson. See BrowserAdapterShim.cpp.
echo "== [pdk-adapter] shim BrowserAdapterShim.cpp -> BrowserAdapterJihad.so =="
mkdir -p "$OUT/shim"
$CXX $COMMON $NPINC -c "$ROOT/Jihad-Browser/render/adapter/BrowserAdapterShim.cpp" \
  -o "$OUT/shim/BrowserAdapterShim.o" || { echo "!! FAILED shim compile"; exit 23; }
$CXX --sysroot=$SYSROOT $ARM -shared -fPIC -fvisibility=hidden \
  -Wl,--version-script="$ROOT/Jihad-Browser/render/adapter/BrowserAdapter.exports" \
  "$OUT/shim/BrowserAdapterShim.o" -ldl \
  -o "$OUT/BrowserAdapterJihad.so" || { echo "!! SHIM LINK FAILED"; exit 24; }
$STRIP "$OUT/BrowserAdapterJihad.so"
echo "== built: $OUT/BrowserAdapterJihad.so =="; ls -la "$OUT/BrowserAdapterJihad.so" | awk '{print " size",$5}'

RE=$ROOT/Jihad-Browser/build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin/arm-webos-linux-gnueabi-readelf
echo "== impl GLIBCXX/GLIBC floor =="; $RE -V "$OUT/BrowserAdapterImpl.so" 2>/dev/null | grep -oE 'GLIBCXX_[0-9.]+|GLIBC_[0-9.]+' | sort -uV | tail -6
echo "== impl NEEDED =="; $RE -d "$OUT/BrowserAdapterImpl.so" 2>/dev/null | grep -oE '\[lib[^]]+\]' | tr '\n' ' '; echo
echo "== shim NEEDED =="; $RE -d "$OUT/BrowserAdapterJihad.so" 2>/dev/null | grep -oE '\[lib[^]]+\]' | tr '\n' ' '; echo
echo "== shim exports (should be NP_*) =="; $RE --dyn-syms "$OUT/BrowserAdapterJihad.so" 2>/dev/null | grep -E ' NP_' | awk '{print $8}' | tr '\n' ' '; echo

# Stage the impl INTO the Enyo app so `palm-package app/` bundles it — a packaged install then
# lands it at /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/
# BrowserAdapterImpl.so, exactly the trusted path the shim loads (Jihad review F-162). It is a
# build artifact (.gitignore'd), copied fresh on every build so the package never ships a stale
# impl. The shim (BrowserAdapterJihad.so) is a SYSTEM plugin under /usr/lib/BrowserPlugins and is
# installed separately (see make-device-bundle.sh / DEVICE-HANDOFF.md) — not part of the app ipk.
APPDIR="$ROOT/Jihad-Browser/app"
if [ -d "$APPDIR" ]; then
  # F-188: fail the build if staging the impl fails — otherwise the echo below would still succeed
  # (the script has no `set -e`) and palm-package would ship an absent/stale app/BrowserAdapterImpl.so.
  cp "$OUT/BrowserAdapterImpl.so" "$APPDIR/BrowserAdapterImpl.so" || { echo "!! FAILED to stage impl into app/"; exit 25; }
  echo "== staged impl -> app/BrowserAdapterImpl.so (palm-package will bundle it) =="
fi
