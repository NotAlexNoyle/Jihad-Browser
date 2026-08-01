#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Cross-build the isis BrowserAdapter NPAPI plugin for the webOS-3 TouchPad using
# OUR crosstool-NG toolchain (GCC 9.4). The plugin is dlopen'd by the device's
# system webkit (glibc 2.8), so we statically link libstdc++/libgcc and keep
# visibility hidden — the resulting .so needs only GLIBC_2.4 (verified) + the
# device's Qt4/pbnjson/glib. It speaks the isis YAP command generation (0x1000
# Connect, etc.) that the Jihad Goanna daemon implements, replacing the stock
# 2011 HP BrowserAdapter.so which is a different, incompatible generation.
#
# NOTE this is the ALTERNATE toolchain path. build-adapter-pdk.sh (gcc 4.3.3) is what actually
# ships, because a gcc9-built plugin dlopen'd into the gcc4 WebKit host hits a static-libstdc++
# ABI collision that crashes LunaSysMgr. This script is kept for ABI/dependency comparison and as
# the fallback if the proprietary PDK is unavailable; its flags are deliberately NOT modernized.
#
# THREE VARIANTS, ONE SOURCE (cavekit-device-build.md R7, context/plans/plan-variant-identity.md),
# same model as build-adapter-pdk.sh:
#
#   ./build-adapter-arm.sh                 build all three variants (default)
#   ./build-adapter-arm.sh mochi           build one variant
#   ./build-adapter-arm.sh enyo mojo       build a subset
#
# It also now emits BOTH halves of the adapter (shim + impl), not just the pre-split monolith it
# used to: the shim is what LunaSysMgr caches for a MIME type, so a build that produces only the
# monolith cannot express per-variant identity at all — LunaSysMgr would cache one .so per boot
# and every variant would resolve to it.
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
ADAPTERDIR=$ROOT/Jihad-Browser/render/adapter
OUT=$DEPS/build; mkdir -p "$OUT/common"; rm -f "$OUT/common"/*.o
# Drop the pre-split artifacts so an incremental build can never ship a stale monolith instead of
# the current shim+impl pair (the same trap as Jihad review F-181 on the PDK path). The loose *.o
# and BrowserAdapter.so are the single-variant era's; the impl now lives in $OUT/<variant>/.
rm -f "$OUT"/*.o "$OUT/BrowserAdapter.so"

# ---- variant table: the ONLY place these names appear in this script -----------------
# Must match context/plans/plan-variant-identity.md exactly — and build-adapter-pdk.sh's copy of
# the same table. Enyo keeps the unsuffixed names it already has on-device.
VARIANTS_ALL="enyo mochi mojo"
# Sets V_MIME / V_YAP / V_SHIM for one variant. The impl path (/usr/lib/jihad/$V/...) and the state
# dir (/var/palm/jihad/$V) are NOT passed: both adapter halves derive them from JIHAD_VARIANT by
# literal concatenation, so a path can never drift away from the identity it belongs to.
variant_set() {
  case "$1" in
    enyo)  V_MIME="application/x-jihad-browser";       V_YAP="jihad-browser";       V_SHIM="BrowserAdapterJihad.so"      ;;
    mochi) V_MIME="application/x-jihad-browser-mochi"; V_YAP="jihad-browser-mochi"; V_SHIM="BrowserAdapterJihadMochi.so" ;;
    mojo)  V_MIME="application/x-jihad-browser-mojo";  V_YAP="jihad-browser-mojo";  V_SHIM="BrowserAdapterJihadMojo.so"  ;;
    *) echo "!! unknown variant '$1' (expected one of: $VARIANTS_ALL)" >&2; exit 3 ;;
  esac
}
VARIANTS="${*:-$VARIANTS_ALL}"
for v in $VARIANTS; do variant_set "$v"; done   # validate the whole arg list before doing any work

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

# ---- variant-macro containment check ------------------------------------------------
# BrowserAdapter.cpp and BrowserAdapterShim.cpp must be the ONLY translation units that reference a
# JIHAD_* identity macro — everything else is compiled ONCE into common/ and linked into all three
# variants. If a header or another .cpp starts using one, its object would carry whichever variant
# happened to be built first and the other two would silently inherit it. Fail loudly instead.
STRAY=$(grep -rl -E 'JIHAD_MIME|JIHAD_YAP_NAME|JIHAD_VARIANT|JIHAD_IMPL_PATH|JIHAD_STATE_DIR' \
          "$ADAPTERDIR" "$YAPDIR/Yap" "$YAPDIR/Src" 2>/dev/null \
        | grep -v -e '/BrowserAdapter\.cpp$' -e '/BrowserAdapterShim\.cpp$')
[ -z "$STRAY" ] || { echo "!! variant macros leaked into non-variant sources (would be built once and shared):"; echo "$STRAY"; exit 4; }

# ---- PASS 1: variant-INDEPENDENT objects, built once ---------------------------------
echo "== [adapter] AdapterBase =="
compile "$ADAPTERDIR/AdapterBase.cpp" "$OUT/common/AdapterBase.o" || exit 11

echo "== [adapter] Yap (Jihad daemon's — wire-identical, GLib YapServer, no Qt) =="
for f in YapClient YapPacket YapProxy YapServer OffscreenBuffer BufferLock ProcessMutex; do
  compile "$YAPDIR/Yap/$f.cpp" "$OUT/common/$f.o" || exit 12
done
echo "== [adapter] IPC support (Src) =="
for f in IpcBuffer; do
  compile "$YAPDIR/Src/$f.cpp" "$OUT/common/$f.o" || exit 12
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
  $CC $PBNCFLAGS $PBNC_INC -c "$PBN/src/pbnjson_c/$f.c" -o "$OUT/common/pbn_$(basename $f).o" || { echo "!! FAILED pbn-c $f"; exit 12; }
done
echo "  CC (c) glib_compat.c (g_malloc0_n shim for device glib 2.16)"
$CC $PBNCFLAGS $GLIBINC -c "$DEPS/glib_compat.c" -o "$OUT/common/glib_compat.o" || { echo "!! FAILED glib_compat"; exit 12; }

echo "== [adapter] pbnjson C++ layer from source (submissions/10, gcc9-consistent) =="
# pbnjson C++ uses exceptions + RTTI internally; compile those TUs with them enabled
# (the plugin's own code stays -fno-exceptions/-fno-rtti).
PBNFLAGS="-fexceptions -frtti -fvisibility=hidden -fPIC -O2 -g0 -DXP_UNIX -DXP_WEBOS -DNDEBUG -DPJSON_EXPORT -DPJSON_NO_LOGGING -D_GLIBCXX_USE_CXX11_ABI=0 $ARM -Wno-psabi"
for f in JValue JDomParser JGenerator JParser JSchema JSchemaFragment JSchemaFile \
         JResolver JErrorHandler JOutputStream; do
  echo "  CC (exc) $f.cpp"
  $CXX $PBNFLAGS $INC -c "$PBN/src/pbnjson_cxx/$f.cpp" -o "$OUT/common/pbn_$f.o" || { echo "!! FAILED pbn $f"; exit 12; }
done

echo "== [adapter] BrowserAdapter sources (variant-independent) =="
for f in BrowserClientBase BrowserAdapterManager Rectangle UrlInfo \
         InteractiveInfo ElementInfo ImageInfo JsonNPObject NPObjectEvent KineticScroller BrowserOffscreen; do
  compile "$ADAPTERDIR/$f.cpp" "$OUT/common/$f.o" || exit 13
done

# ---- PASS 2: per-variant impl + shim -------------------------------------------------
RE=$TCB/arm-webos-linux-gnueabi-readelf
SSTRINGS=$TCB/arm-webos-linux-gnueabi-strings

# Assert an exact WHOLE STRING is present in a `strings` dump — whole-string, not substring, since
# "jihad-browser" is a PREFIX of "jihad-browser-mochi" and a substring test would pass an Enyo probe
# against a Mochi build. No `grep -q`: it exits on the first match, SIGPIPEs the writer, and with
# `set -o pipefail` (top of this script) that 141 becomes the pipeline's status, making the check
# pass or fail at random on pipe-buffer timing.
probe() { # <label> <strings-dump> <expected whole string>
  printf '%s\n' "$2" | grep -xF -- "$3" >/dev/null \
    || { echo "!! $1: identity NOT baked in — expected string '$3'"; exit 26; }
}

for V in $VARIANTS; do
  variant_set "$V"
  VOUT="$OUT/$V"; mkdir -p "$VOUT"; rm -f "$VOUT"/*.o
  echo "== [adapter] ===== variant $V (MIME $V_MIME, YAP $V_YAP) ====="

  # The identity macros, passed TOGETHER: BrowserAdapter.cpp #errors on a half-set, because a build
  # that registers as Mochi and then dials the Enyo daemon looks perfectly healthy in the logs
  # (cavekit-device-build.md R7). JIHAD_VARIANT also fixes the adapter's state dir (R8).
  echo "  CC BrowserAdapter.cpp [$V]"
  $CXX $COMMON $INC -DJIHAD_VARIANT="\"$V\"" -DJIHAD_MIME="\"$V_MIME\"" -DJIHAD_YAP_NAME="\"$V_YAP\"" \
    -c "$ADAPTERDIR/BrowserAdapter.cpp" -o "$VOUT/BrowserAdapter.o" \
    || { echo "!! FAILED: BrowserAdapter.cpp [$V]"; exit 13; }

  echo "== [adapter] link $V/BrowserAdapterImpl.so (static libstdc++, hidden vis) =="
  $CXX $ARM -shared -fPIC -fvisibility=hidden -static-libstdc++ -static-libgcc \
    -Wl,--version-script="$ADAPTERDIR/BrowserAdapter.exports" \
    "$OUT/common"/*.o "$VOUT/BrowserAdapter.o" \
    -L"$DEPS/staging/lib" -lQtGui -lQtCore -lQtNetwork -lyajl -lpng12 \
    -L"$SYSROOT/usr/lib/arm-linux-gnueabi" -lglib-2.0 -lgthread-2.0 \
    -lrt -lpthread \
    -Wl,-rpath-link,"$DEPS/staging/lib" -Wl,-rpath-link,"$SYSROOT/usr/lib/arm-linux-gnueabi" -Wl,-rpath-link,"$SYSROOT/lib/arm-linux-gnueabi" \
    -o "$VOUT/BrowserAdapterImpl.so" || { echo "!! LINK FAILED [$V]"; exit 14; }
  $TCB/arm-webos-linux-gnueabi-strip "$VOUT/BrowserAdapterImpl.so"
  echo "== built: $VOUT/BrowserAdapterImpl.so =="; ls -la "$VOUT/BrowserAdapterImpl.so" | awk '{print " size",$5}'

  # The shim is what LunaSysMgr actually caches for this variant's MIME type; the impl above is
  # dlopen'd out of /usr/lib/jihad/$V/ per card open. Building only the monolith (as this script
  # used to) cannot express per-variant identity at all. Minimal deps: npapi headers + libdl.
  echo "== [adapter] shim BrowserAdapterShim.cpp -> $V_SHIM =="
  $CXX $COMMON -I"$DEPS/staging/include" -I"$DEPS/staging/include/webkit/npapi" \
    -DJIHAD_VARIANT="\"$V\"" -DJIHAD_MIME="\"$V_MIME\"" \
    -c "$ADAPTERDIR/BrowserAdapterShim.cpp" -o "$VOUT/BrowserAdapterShim.o" \
    || { echo "!! FAILED shim compile [$V]"; exit 15; }
  $CXX $ARM -shared -fPIC -fvisibility=hidden -static-libstdc++ -static-libgcc \
    -Wl,--version-script="$ADAPTERDIR/BrowserAdapter.exports" \
    "$VOUT/BrowserAdapterShim.o" -ldl \
    -o "$OUT/$V_SHIM" || { echo "!! SHIM LINK FAILED [$V]"; exit 16; }
  $TCB/arm-webos-linux-gnueabi-strip "$OUT/$V_SHIM"
  echo "== built: $OUT/$V_SHIM =="; ls -la "$OUT/$V_SHIM" | awk '{print " size",$5}'

  echo "== [$V] impl glibc floor (need <= 2.8) =="; $RE -V "$VOUT/BrowserAdapterImpl.so" 2>/dev/null | grep -oE 'GLIBC_[0-9.]+' | sort -uV | tail -3
  echo "== [$V] impl NEEDED =="; $RE -d "$VOUT/BrowserAdapterImpl.so" 2>/dev/null | grep -oE '\[lib[^]]+\]' | tr '\n' ' '; echo
  echo "== [$V] shim NEEDED =="; $RE -d "$OUT/$V_SHIM" 2>/dev/null | grep -oE '\[lib[^]]+\]' | tr '\n' ' '; echo

  # Read the identity back OUT of the binaries: trusting that the -D reached the compiler is how a
  # Mochi-named .so ends up speaking Enyo's MIME with nothing downstream to catch it. The YAP name
  # appears BARE — YapClient concatenates its own "/tmp/yapserver." prefix at runtime.
  IMPL_STR=$("$SSTRINGS" -a "$VOUT/BrowserAdapterImpl.so" 2>/dev/null)
  SHIM_STR=$("$SSTRINGS" -a "$OUT/$V_SHIM" 2>/dev/null)
  probe "[$V] impl MIME"       "$IMPL_STR" "$V_MIME::;"
  probe "[$V] impl YAP name"   "$IMPL_STR" "$V_YAP"
  probe "[$V] impl state dir"  "$IMPL_STR" "/var/palm/jihad/$V/adapter.log"
  probe "[$V] shim MIME"       "$SHIM_STR" "$V_MIME::;"
  probe "[$V] shim impl path"  "$SHIM_STR" "/usr/lib/jihad/$V/BrowserAdapterImpl.so"
  probe "[$V] shim state dir"  "$SHIM_STR" "/var/palm/jihad/$V/shim.log"
  # R8: neither half may reference the user's USB mass-storage volume, in any build.
  for pair in "impl:$IMPL_STR" "shim:$SHIM_STR"; do
    if printf '%s\n' "${pair#*:}" | grep -F -- '/media/' >/dev/null; then
      echo "!! [$V] R8 violation: the ${pair%%:*} still references /media/"; exit 27
    fi
  done
  echo "== [$V] identity verified in both .so (MIME + YAP name + impl path + state dirs, no /media) =="
done

echo "== [adapter] DONE: variants [$VARIANTS] =="
