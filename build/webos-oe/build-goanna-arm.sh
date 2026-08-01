#!/bin/bash
# Copyright 2026 NotAlexNoyle.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Cross-build libxul for the webOS-3 TouchPad (ARMv7 softfp). Runs in the
# jihad-goanna-build container (host build tools) with the crosstool-NG toolchain
# and the Jessie armel GTK/X sysroot mounted.
#
# Mounts:
#   /src/uxp  UXP source (RW; shared with desktop build — patches already applied)
#   /tc       crosstool-NG toolchain root (.../x-tools/arm-webos-linux-gnueabi)
#   /sysroot  Debian-Jessie armel sysroot (GTK/X/cairo/pango headers + libs)
#   /cfg      build/desktop (for patches/)      /armcfg  build/webos-oe (mozconfig-arm)
#   /out      ARM object dir + artifacts
#
# Usage: /armcfg/build-goanna-arm.sh [configure|build|all]
set -euo pipefail
SRC=/src/uxp
STAGE=${1:-all}
export MOZCONFIG=/armcfg/mozconfig.goanna-arm

# --- cross environment ---
export PATH=/tc/bin:$PATH
# Export CC/CXX so in-tree gyp checks (e.g. NSS freebl.gyp's __SOFTFP__ probe that
# runs `${CC:-cc} ${CFLAGS}`) use the cross compiler, not the host `cc` (which
# rejects -mfloat-abi=softfp). mozbuild still reads target CC/CXX from the mozconfig.
export CC="/tc/bin/arm-webos-linux-gnueabi-gcc"
export CXX="/tc/bin/arm-webos-linux-gnueabi-g++"
export PKG_CONFIG=pkg-config
export PKG_CONFIG_SYSROOT_DIR=/sysroot
export PKG_CONFIG_LIBDIR=/sysroot/usr/lib/arm-linux-gnueabi/pkgconfig:/sysroot/usr/share/pkgconfig:/sysroot/usr/lib/pkgconfig
export PKG_CONFIG_PATH=$PKG_CONFIG_LIBDIR

echo "== Jihad Goanna ARM cross-build =="
echo "cross gcc: $(/tc/bin/arm-webos-linux-gnueabi-gcc -dumpversion 2>&1) ($(/tc/bin/arm-webos-linux-gnueabi-gcc -dumpmachine))"
echo "sysroot gtk: $(PKG_CONFIG_SYSROOT_DIR=/sysroot pkg-config --modversion gtk+-2.0 2>&1)"
echo "sysroot glib: $(PKG_CONFIG_SYSROOT_DIR=/sysroot pkg-config --modversion glib-2.0 2>&1)"

# Patches + branding strip are idempotent and normally already applied by the
# desktop build on the shared source tree; re-run for standalone safety.
if [ -d /cfg/patches ]; then
  for p in /cfg/patches/*.patch; do
    [ -e "$p" ] || continue; name=$(basename "$p")
    if patch -p1 -d "$SRC" --forward --dry-run < "$p" >/dev/null 2>&1; then
      patch -p1 -d "$SRC" --forward < "$p" && echo "applied patch: $name"
    else echo "patch already applied/skipped: $name"; fi
  done
fi

cd "$SRC"
case "$STAGE" in
  configure) ./mach configure ;;
  build)     ./mach build ;;
  all)       ./mach configure && ./mach build ;;
  *)         echo "unknown stage: $STAGE"; exit 2 ;;
esac
echo "== ARM build stage '$STAGE' done; artifacts under /out =="
