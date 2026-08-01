#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
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
#
# THREE VARIANTS, ONE SOURCE (cavekit-device-build.md R7, context/plans/plan-variant-identity.md).
# The Enyo, Mochi, and Mojo packages are three independent browsers, each with its own MIME type,
# YAP service name, shim filename, and impl path. Rather than fork the sources three ways, the
# identity is passed on the compiler command line and this script loops:
#
#   ./build-adapter-pdk.sh                 build all three variants (default)
#   ./build-adapter-pdk.sh mochi           build one variant
#   ./build-adapter-pdk.sh enyo mojo       build a subset
#
# Only BrowserAdapter.cpp and BrowserAdapterShim.cpp consume the JIHAD_* macros, so every OTHER
# translation unit (AdapterBase, BrowserClientBase, the Yap wire code, the info/scroller classes)
# is variant-INDEPENDENT and is compiled exactly once into $OUT/common. That is the difference
# between one adapter build and three. The assumption is enforced below, not assumed — see the
# "variant-macro containment" check, which fails the build if any other source starts using them.
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"          # Jihad-Browser repo root (derived, not hardcoded)
ROOT="$(cd "$REPO/.." && pwd)"             # workspace root (legacy PDK sibling location)
DEPS=$REPO/build/webos-oe/adapter-deps
# PDK (Palm SDK: gcc 4.3.3 + device sysroot). Proprietary — NOT in the repo; fetch it with
# build/webos-oe/fetch-pdk.sh <palm-sdk_*.deb> into build/webos-oe/pdk/, or set PDK_ROOT.
PDK="${PDK_ROOT:-$REPO/build/webos-oe/pdk/opt/PalmPDK}"
[ -d "$PDK/arm-gcc" ] || PDK="$ROOT/toolchains/opt/PalmPDK"   # legacy sibling location fallback
[ -d "$PDK/arm-gcc" ] || { echo "!! PDK not found. Run build/webos-oe/fetch-pdk.sh <palm-sdk_*.deb>, or set PDK_ROOT=<dir with arm-gcc/>."; exit 2; }
CXX=$PDK/arm-gcc/bin/arm-none-linux-gnueabi-g++
CC=$PDK/arm-gcc/bin/arm-none-linux-gnueabi-gcc
STRIP=$PDK/arm-gcc/bin/arm-none-linux-gnueabi-strip
SYSROOT=$PDK/arm-gcc/sysroot
JSR=$REPO/build/webos-oe/arm-sysroot/root                    # jessie: glib headers only
QT=$DEPS/qt4-extract/usr/include/qt4
PBN=$DEPS/libpbnjson                                         # checked out at submissions/10
YAPDIR=$REPO/render/browserserver
ADAPTERDIR=$REPO/render/adapter
OUT=$DEPS/build-pdk; mkdir -p "$OUT/common"; rm -f "$OUT/common"/*.o
# Remove the stale outputs from the two earlier layouts, so an incremental build can never silently
# deploy a pre-split artifact instead of the current one (Jihad review F-181, extended for the
# per-variant split): BrowserAdapter.so is the pre-shim monolith, BrowserAdapterImpl.so and the
# loose *.o are the single-variant era's (the impl now lives in $OUT/<variant>/).
rm -f "$OUT"/*.o "$OUT/BrowserAdapter.so" "$OUT/BrowserAdapterImpl.so"

# ---- variant table: the ONLY place these names appear in this script -----------------
# Must match context/plans/plan-variant-identity.md exactly. Enyo keeps the unsuffixed names it
# already has on-device — renaming the one deployment known to work would break it for no gain.
VARIANTS_ALL="enyo mochi mojo"
# Sets V_MIME / V_YAP / V_SHIM / V_APPDIR for one variant. The shim's impl path
# (/usr/lib/jihad/$V/BrowserAdapterImpl.so) and state dir (/var/palm/jihad/$V) are NOT passed:
# BrowserAdapterShim.cpp derives them from JIHAD_VARIANT by literal concatenation, so the path
# and the identity cannot drift apart no matter what this script does.
variant_set() {
  case "$1" in
    enyo)  V_MIME="application/x-jihad-browser";       V_YAP="jihad-browser";       V_SHIM="BrowserAdapterJihad.so";      V_APPDIR="app"       ;;
    mochi) V_MIME="application/x-jihad-browser-mochi"; V_YAP="jihad-browser-mochi"; V_SHIM="BrowserAdapterJihadMochi.so"; V_APPDIR="app-mochi" ;;
    mojo)  V_MIME="application/x-jihad-browser-mojo";  V_YAP="jihad-browser-mojo";  V_SHIM="BrowserAdapterJihadMojo.so";  V_APPDIR="app-mojo"  ;;
    *) echo "!! unknown variant '$1' (expected one of: $VARIANTS_ALL)" >&2; exit 3 ;;
  esac
}
VARIANTS="${*:-$VARIANTS_ALL}"
for v in $VARIANTS; do variant_set "$v"; done   # validate the whole arg list before doing any work

# ---- variant-macro containment check ------------------------------------------------
# The per-variant/common object split above is only sound while BrowserAdapter.cpp and
# BrowserAdapterShim.cpp are the ONLY translation units that reference a JIHAD_* identity macro.
# If a header or another .cpp starts using one, its object would be compiled once (into common/)
# with whichever variant happened to be first and then linked into all three — producing a Mochi
# adapter that silently reports Enyo's identity. Fail loudly instead.
STRAY=$(grep -rl -E 'JIHAD_MIME|JIHAD_YAP_NAME|JIHAD_VARIANT|JIHAD_IMPL_PATH|JIHAD_STATE_DIR' \
          "$ADAPTERDIR" "$YAPDIR/Yap" "$YAPDIR/Src" 2>/dev/null \
        | grep -v -e '/BrowserAdapter\.cpp$' -e '/BrowserAdapterShim\.cpp$')
[ -z "$STRAY" ] || { echo "!! variant macros leaked into non-variant sources (would be built once and shared):"; echo "$STRAY"; echo "!! either move the use into BrowserAdapter.cpp/BrowserAdapterShim.cpp, or compile that file per-variant below."; exit 4; }

ARM="-march=armv7-a -mfpu=neon -mfloat-abi=softfp"
COMMON="--sysroot=$SYSROOT -fno-exceptions -fno-rtti -fvisibility=hidden -fPIC -O2 -g0 -DXP_UNIX -DXP_WEBOS -DNDEBUG $ARM"
GLIBINC="-I$JSR/usr/include/glib-2.0 -I$JSR/usr/lib/arm-linux-gnueabi/glib-2.0/include"
QTINC="-I$QT -I$QT/QtCore -I$QT/QtGui -I$QT/QtNetwork"
PDKINC="-I$PDK/include"
PBNINC="-I$PBN/include/public"
NPINC="-I$DEPS/staging/include -I$DEPS/staging/include/webkit/npapi"
INC="$NPINC $PBNINC $GLIBINC $QTINC $PDKINC -I$ROOT/Jihad-Browser/render/adapter -I$YAPDIR/Yap -I$YAPDIR/Src -I$ROOT/Jihad-Browser/render/adapter"

compile(){ echo "  CXX $(basename "$1")"; $CXX $COMMON $INC -c "$1" -o "$2" || { echo "!! FAILED: $1"; exit 20; }; }

# ---- PASS 1: variant-INDEPENDENT objects, built once ---------------------------------
# Everything except BrowserAdapter.cpp (the one impl TU that reads JIHAD_MIME/JIHAD_YAP_NAME).
echo "== [pdk-adapter] AdapterBase =="
compile "$ADAPTERDIR/AdapterBase.cpp" "$OUT/common/AdapterBase.o"

echo "== [pdk-adapter] Yap (Jihad daemon's — wire-identical) =="
for f in YapClient YapPacket YapProxy YapServer OffscreenBuffer BufferLock ProcessMutex; do
  compile "$YAPDIR/Yap/$f.cpp" "$OUT/common/$f.o"
done
compile "$YAPDIR/Src/IpcBuffer.cpp" "$OUT/common/IpcBuffer.o"

echo "== [pdk-adapter] glib_compat (g_malloc0_n shim for device glib 2.16) =="
$CC --sysroot=$SYSROOT -std=gnu99 -fvisibility=hidden -fPIC -O2 -g0 $ARM $GLIBINC \
  -c "$DEPS/glib_compat.c" -o "$OUT/common/glib_compat.o" || { echo "!! FAILED glib_compat"; exit 21; }

echo "== [pdk-adapter] BrowserAdapter sources (variant-independent) =="
for f in BrowserClientBase BrowserAdapterManager Rectangle UrlInfo \
         InteractiveInfo ElementInfo ImageInfo JsonNPObject NPObjectEvent KineticScroller BrowserOffscreen; do
  compile "$ADAPTERDIR/$f.cpp" "$OUT/common/$f.o"
done

# ---- PASS 2: per-variant impl + shim -------------------------------------------------
# Inspect the ARM output with the PDK's OWN binutils: they are guaranteed present whenever this
# script can run at all. (The crosstool-NG readelf used here before belongs to a separate, optional
# toolchain and silently produced nothing for --dyn-syms, so the "shim exports" line always printed
# empty — a check that can only pass is not a check.)
RE=$PDK/arm-gcc/bin/arm-none-linux-gnueabi-readelf
SSTRINGS=$PDK/arm-gcc/bin/arm-none-linux-gnueabi-strings

# Assert an exact WHOLE STRING is present in a `strings` dump. Whole-string, not substring:
# "jihad-browser" is a PREFIX of "jihad-browser-mochi", so a substring test would pass an Enyo
# probe against a Mochi build — the precise confusion these probes exist to catch.
# The dump is captured into a variable first and matched WITHOUT `grep -q` on purpose: `-q` exits
# on the first match, which SIGPIPEs the writer, and with `set -o pipefail` (line 1 of this script)
# that 141 becomes the pipeline's status. That made these checks pass or fail at random depending
# on pipe-buffer timing — and would have made the negative /media check silently pass.
probe() { # <label> <strings-dump> <expected whole string>
  printf '%s\n' "$2" | grep -xF -- "$3" >/dev/null \
    || { echo "!! $1: identity NOT baked in — expected string '$3'"; exit 26; }
}

for V in $VARIANTS; do
  variant_set "$V"
  VOUT="$OUT/$V"; mkdir -p "$VOUT"; rm -f "$VOUT"/*.o
  echo "== [pdk-adapter] ===== variant $V (MIME $V_MIME, YAP $V_YAP) ====="

  # The identity macros, passed TOGETHER: BrowserAdapter.cpp #errors on a half-set, because a build
  # that registers as Mochi and then dials the Enyo daemon would look perfectly healthy in the logs
  # (cavekit-device-build.md R7). JIHAD_VARIANT additionally fixes the adapter's state dir
  # (/var/palm/jihad/$V — the gated paint log, R8), which it derives from the token itself.
  echo "  CXX BrowserAdapter.cpp [$V]"
  $CXX $COMMON $INC -DJIHAD_VARIANT="\"$V\"" -DJIHAD_MIME="\"$V_MIME\"" -DJIHAD_YAP_NAME="\"$V_YAP\"" \
    -c "$ADAPTERDIR/BrowserAdapter.cpp" -o "$VOUT/BrowserAdapter.o" \
    || { echo "!! FAILED: BrowserAdapter.cpp [$V]"; exit 20; }

  # The impl holds ALL the adapter logic. It is dlopen'd by the shim per card open, NOT loaded by
  # LunaSysMgr directly, and installs to /usr/lib/jihad/$V/BrowserAdapterImpl.so — the exact path
  # this variant's shim was compiled to look for. The link list is explicit (common/ + this
  # variant's BrowserAdapter.o); no glob, so a stray object can never be swept into the .so.
  echo "== [pdk-adapter] link $V/BrowserAdapterImpl.so (device libstdc++/Qt/pbnjson, dynamic) =="
  $CXX --sysroot=$SYSROOT $ARM -shared -fPIC -fvisibility=hidden \
    -Wl,--version-script="$ADAPTERDIR/BrowserAdapter.exports" \
    "$OUT/common"/*.o "$VOUT/BrowserAdapter.o" \
    -L"$DEPS/staging/lib" -lQtGui -lQtCore -lQtNetwork -lpbnjson_cpp -lpbnjson_c -lyajl -lpng12 \
    -L"$JSR/usr/lib/arm-linux-gnueabi" -lglib-2.0 -lgthread-2.0 \
    -lrt -lpthread \
    -Wl,-rpath-link,"$DEPS/staging/lib" -Wl,-rpath-link,"$JSR/usr/lib/arm-linux-gnueabi" \
    -o "$VOUT/BrowserAdapterImpl.so" || { echo "!! LINK FAILED [$V]"; exit 22; }
  $STRIP "$VOUT/BrowserAdapterImpl.so"
  echo "== built: $VOUT/BrowserAdapterImpl.so =="; ls -la "$VOUT/BrowserAdapterImpl.so" | awk '{print " size",$5}'

  # The shim is the STABLE plugin LunaSysMgr caches at boot. It contains no browser logic — only
  # NPAPI entry points + NPP_* forwarders + dlopen of the impl. Minimal deps (npapi headers +
  # libdl); no Qt/glib/pbnjson. -DJIHAD_VARIANT alone fixes the impl path and state dir (the shim
  # concatenates them from the token), so those two can never disagree with the identity.
  echo "== [pdk-adapter] shim BrowserAdapterShim.cpp -> $V_SHIM =="
  $CXX $COMMON $NPINC -DJIHAD_VARIANT="\"$V\"" -DJIHAD_MIME="\"$V_MIME\"" \
    -c "$ADAPTERDIR/BrowserAdapterShim.cpp" -o "$VOUT/BrowserAdapterShim.o" \
    || { echo "!! FAILED shim compile [$V]"; exit 23; }
  $CXX --sysroot=$SYSROOT $ARM -shared -fPIC -fvisibility=hidden \
    -Wl,--version-script="$ADAPTERDIR/BrowserAdapter.exports" \
    "$VOUT/BrowserAdapterShim.o" -ldl \
    -o "$OUT/$V_SHIM" || { echo "!! SHIM LINK FAILED [$V]"; exit 24; }
  $STRIP "$OUT/$V_SHIM"
  echo "== built: $OUT/$V_SHIM =="; ls -la "$OUT/$V_SHIM" | awk '{print " size",$5}'

  echo "== [$V] impl GLIBCXX/GLIBC floor =="; $RE -V "$VOUT/BrowserAdapterImpl.so" 2>/dev/null | grep -oE 'GLIBCXX_[0-9.]+|GLIBC_[0-9.]+' | sort -uV | tail -6
  echo "== [$V] impl NEEDED =="; $RE -d "$VOUT/BrowserAdapterImpl.so" 2>/dev/null | grep -oE '\[lib[^]]+\]' | tr '\n' ' '; echo
  echo "== [$V] shim NEEDED =="; $RE -d "$OUT/$V_SHIM" 2>/dev/null | grep -oE '\[lib[^]]+\]' | tr '\n' ' '; echo
  echo "== [$V] shim exports (should be NP_*) =="; $RE --dyn-syms "$OUT/$V_SHIM" 2>/dev/null | grep -oE 'NP_[A-Za-z]+' | sort -u | tr '\n' ' '; echo
  # The identity is only real if it is IN the binaries. Read the strings back out rather than
  # trusting that the -D reached the compiler: a shell-quoting slip would otherwise ship a
  # Mochi-named .so speaking Enyo's MIME, and nothing downstream of here would catch it.
  # Note the YAP name appears BARE in the impl — YapClient concatenates its own kSocketPathPrefix
  # ("/tmp/yapserver.") with it at runtime, so the full socket path is never a literal.
  IMPL_STR=$("$SSTRINGS" -a "$VOUT/BrowserAdapterImpl.so" 2>/dev/null)
  SHIM_STR=$("$SSTRINGS" -a "$OUT/$V_SHIM" 2>/dev/null)
  probe "[$V] impl MIME"       "$IMPL_STR" "$V_MIME::;"
  probe "[$V] impl YAP name"   "$IMPL_STR" "$V_YAP"
  probe "[$V] impl state dir"  "$IMPL_STR" "/var/palm/jihad/$V/adapter.log"
  probe "[$V] shim MIME"       "$SHIM_STR" "$V_MIME::;"
  probe "[$V] shim impl path"  "$SHIM_STR" "/usr/lib/jihad/$V/BrowserAdapterImpl.so"
  probe "[$V] shim state dir"  "$SHIM_STR" "/var/palm/jihad/$V/shim.log"
  # R8: NEITHER half may reference the user's USB mass-storage volume, in any build. This is the
  # check that keeps /media/internal from creeping back in via a new debug channel.
  for pair in "impl:$IMPL_STR" "shim:$SHIM_STR"; do
    if printf '%s\n' "${pair#*:}" | grep -F -- '/media/' >/dev/null; then
      echo "!! [$V] R8 violation: the ${pair%%:*} still references /media/"; exit 27
    fi
  done
  echo "== [$V] identity verified in both .so (MIME + YAP name + impl path + state dirs, no /media) =="

  # Stage this variant's impl INTO its app dir so `palm-package <appdir>` bundles it; the package's
  # postinst then lays it down at /usr/lib/jihad/$V/BrowserAdapterImpl.so, the root-owned rootfs
  # path the shim trusts (cryptofs reports 0777, so the bundle copy itself can never pass the
  # shim's trust check — it is a delivery vehicle, not a load path; plan-variant-identity.md rule 5).
  # Build artifact (.gitignore'd), copied fresh every build so no package ships a stale impl.
  # The shim is a SYSTEM plugin under /usr/lib/BrowserPlugins, installed separately.
  APPDIR="$REPO/$V_APPDIR"
  if [ -d "$APPDIR" ]; then
    # F-188: fail the build if staging fails — this script has no `set -e`, so an unchecked cp
    # would let palm-package ship an absent/stale <appdir>/BrowserAdapterImpl.so.
    cp "$VOUT/BrowserAdapterImpl.so" "$APPDIR/BrowserAdapterImpl.so" || { echo "!! FAILED to stage impl into $V_APPDIR/"; exit 25; }
    echo "== staged impl -> $V_APPDIR/BrowserAdapterImpl.so (palm-package will bundle it) =="
  else
    echo "-- note: $V_APPDIR/ absent; skipped impl staging for $V"
  fi
done

echo "== [pdk-adapter] DONE: variants [$VARIANTS] =="
