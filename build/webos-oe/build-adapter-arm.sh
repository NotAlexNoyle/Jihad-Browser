#!/bin/bash
# Copyright 2026 the Jihad Browser project. Apache-2.0.
#
# Cross-build the isis BrowserAdapter NPAPI plugin for the webOS-3 TouchPad using
# OUR crosstool-NG toolchain (GCC 9.4). The plugin is dlopen'd by the device's
# system webkit (glibc 2.8), so we statically link libstdc++/libgcc and keep
# visibility hidden — the resulting .so needs only GLIBC_2.4 (verified) + the
# device's Qt4/pbnjson/glib. It speaks the isis YAP command generation (0x1000
# Connect, etc.) that the Jihad Goanna daemon implements, replacing the stock
# 2011 HP BrowserAdapter.so which is a different, incompatible generation.
set -uo pipefail
REPO="$(cd "$(dirname "$0")/../.." && pwd)"     # Jihad-Browser repo root (derived)
ROOT="$(cd "$REPO/.." && pwd)"                  # workspace root (legacy PDK sibling location)
# PDK headers (Palm SDK). Proprietary — fetch with build/webos-oe/fetch-pdk.sh or set PDK_ROOT.
PDK="${PDK_ROOT:-$REPO/build/webos-oe/pdk/opt/PalmPDK}"
[ -d "$PDK/include" ] || PDK="$ROOT/toolchains/opt/PalmPDK"
DEPS=$ROOT/Jihad-Browser/build/webos-oe/adapter-deps
TCB=$ROOT/Jihad-Browser/build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin
SYSROOT=$ROOT/Jihad-Browser/build/webos-oe/arm-sysroot/root
CXX=$TCB/arm-webos-linux-gnueabi-g++
CC=$TCB/arm-webos-linux-gnueabi-gcc
AR=$TCB/arm-webos-linux-gnueabi-ar
QT=$DEPS/qt4-extract/usr/include/qt4
OUT=$DEPS/build; mkdir -p "$OUT"

ARM="-march=armv7-a -mfpu=neon -mfloat-abi=softfp"
# _GLIBCXX_USE_CXX11_ABI=0: the device's libstdc++/Qt4/pbnjson are gcc-4.x (old COW
# std::string ABI). gcc9 defaults to the C++11 SSO string ABI, which is layout-
# incompatible and crashes when a std::string crosses into device pbnjson/Qt. Force
# the old ABI so string interop with the device C++ libs is binary-compatible.
COMMON="-fno-exceptions -fno-rtti -fvisibility=hidden -fPIC -O2 -g0 -DXP_UNIX -DXP_WEBOS -DNDEBUG -D_GLIBCXX_USE_CXX11_ABI=0 $ARM -Wno-psabi"
GLIBINC="-I$SYSROOT/usr/include/glib-2.0 -I$SYSROOT/usr/lib/arm-linux-gnueabi/glib-2.0/include"
QTINC="-I$QT -I$QT/QtCore -I$QT/QtGui -I$QT/QtNetwork"
PDKINC="-I$PDK/include"
YAPDIR=$ROOT/Jihad-Browser/render/browserserver
PBN=$DEPS/libpbnjson
PBNINC="-I$PBN/include/public -I$PBN/include/public/pbnjson/cxx -I$PBN/include/public/pbnjson/c -I$PBN/src/pbnjson_cxx -I$PBN/src/pbnjson_c -I$DEPS/yajl-inc"
INC="-I$DEPS/staging/include -I$DEPS/staging/include/webkit/npapi $PBNINC $GLIBINC $QTINC $PDKINC \
 -I$ROOT/Jihad-Browser/render/adapter -I$YAPDIR/Yap -I$YAPDIR/Src \
 -I$ROOT/Jihad-Browser/render/adapter"

compile(){ # <src> <obj>
  echo "  CC $(basename "$1")"
  $CXX $COMMON $INC -c "$1" -o "$2" || { echo "!! FAILED: $1"; return 1; }
}

echo "== [adapter] AdapterBase =="
compile "$ROOT/Jihad-Browser/render/adapter/AdapterBase.cpp" "$OUT/AdapterBase.o" || exit 11

echo "== [adapter] Yap (Jihad daemon's — wire-identical, GLib YapServer, no Qt) =="
for f in YapClient YapPacket YapProxy YapServer OffscreenBuffer BufferLock ProcessMutex; do
  compile "$YAPDIR/Yap/$f.cpp" "$OUT/$f.o" || exit 12
done
echo "== [adapter] IPC support (Src) =="
for f in IpcBuffer; do
  compile "$YAPDIR/Src/$f.cpp" "$OUT/$f.o" || exit 12
done

# pbnjson from source, tag submissions/10 — the pre-schema-rewrite era matching the
# TouchPad's 2011 shipped libpbnjson (device libpbnjson_c has no jsaxparser_*/no
# validation subsystem). Compiling BOTH C and C++ layers from source makes the plugin
# self-contained for JSON, depending on the device only for stable libyajl (1.x, its
# API matched by the bundled yajl 1.0.7 headers) + glib. 6 generated feature headers
# (pjson_syslog/regexp/strnlen/sys_malloc/isatty/assert_compat) live beside the sources.
echo "== [adapter] pbnjson C layer from source (submissions/10) =="
PBNCFLAGS="-std=gnu99 -fvisibility=hidden -fPIC -O2 -g0 -DNDEBUG -DPJSON_EXPORT -DPJSON_NO_LOGGING $ARM -Wno-psabi -Wno-deprecated-declarations"
# C-only includes: the pbnjson/c japi.h must win over the C++ one (which has `namespace`).
PBNC_INC="-I$PBN/include/public -I$PBN/include/public/pbnjson/c -I$PBN/src/pbnjson_c -I$DEPS/yajl-inc $GLIBINC"
for f in debugging jgen_stream jobject jparse_stream jschema jvalue/num_conversion; do
  echo "  CC (c) $f.c"
  $CC $PBNCFLAGS $PBNC_INC -c "$PBN/src/pbnjson_c/$f.c" -o "$OUT/pbn_$(basename $f).o" || { echo "!! FAILED pbn-c $f"; exit 12; }
done
echo "  CC (c) glib_compat.c (g_malloc0_n shim for device glib 2.16)"
$CC $PBNCFLAGS $GLIBINC -c "$DEPS/glib_compat.c" -o "$OUT/glib_compat.o" || { echo "!! FAILED glib_compat"; exit 12; }

echo "== [adapter] pbnjson C++ layer from source (submissions/10, gcc9-consistent) =="
# pbnjson C++ uses exceptions + RTTI internally; compile those TUs with them enabled
# (the plugin's own code stays -fno-exceptions/-fno-rtti).
PBNFLAGS="-fexceptions -frtti -fvisibility=hidden -fPIC -O2 -g0 -DXP_UNIX -DXP_WEBOS -DNDEBUG -DPJSON_EXPORT -DPJSON_NO_LOGGING -D_GLIBCXX_USE_CXX11_ABI=0 $ARM -Wno-psabi"
for f in JValue JDomParser JGenerator JParser JSchema JSchemaFragment JSchemaFile \
         JResolver JErrorHandler JOutputStream; do
  echo "  CC (exc) $f.cpp"
  $CXX $PBNFLAGS $INC -c "$PBN/src/pbnjson_cxx/$f.cpp" -o "$OUT/pbn_$f.o" || { echo "!! FAILED pbn $f"; exit 12; }
done

echo "== [adapter] BrowserAdapter sources =="
for f in BrowserClientBase BrowserAdapter BrowserAdapterManager Rectangle UrlInfo \
         InteractiveInfo ElementInfo ImageInfo JsonNPObject NPObjectEvent KineticScroller BrowserOffscreen; do
  compile "$ROOT/Jihad-Browser/render/adapter/$f.cpp" "$OUT/$f.o" || exit 13
done

echo "== [adapter] link BrowserAdapter.so (static libstdc++, hidden vis) =="
$CXX $ARM -shared -fPIC -fvisibility=hidden -static-libstdc++ -static-libgcc \
  -Wl,--version-script="$ROOT/Jihad-Browser/render/adapter/BrowserAdapter.exports" \
  "$OUT"/*.o \
  -L"$DEPS/staging/lib" -lQtGui -lQtCore -lQtNetwork -lyajl -lpng12 \
  -L"$SYSROOT/usr/lib/arm-linux-gnueabi" -lglib-2.0 -lgthread-2.0 \
  -lrt -lpthread \
  -Wl,-rpath-link,"$DEPS/staging/lib" -Wl,-rpath-link,"$SYSROOT/usr/lib/arm-linux-gnueabi" -Wl,-rpath-link,"$SYSROOT/lib/arm-linux-gnueabi" \
  -o "$OUT/BrowserAdapter.so" || { echo "!! LINK FAILED"; exit 14; }

$TCB/arm-webos-linux-gnueabi-strip "$OUT/BrowserAdapter.so"
echo "== built: $OUT/BrowserAdapter.so =="
ls -la "$OUT/BrowserAdapter.so" | awk '{print " size",$5}'
echo "== glibc floor (need <= 2.8) =="; $TCB/arm-webos-linux-gnueabi-readelf -V "$OUT/BrowserAdapter.so" 2>/dev/null | grep -oE 'GLIBC_[0-9.]+' | sort -uV | tail -3
echo "== NEEDED =="; $TCB/arm-webos-linux-gnueabi-readelf -d "$OUT/BrowserAdapter.so" 2>/dev/null | grep -oE '\[lib[^]]+\]' | tr '\n' ' '; echo
