#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Single-entry device build (device-build R3, R11): from a clean clone to the three installable
# packages, on any x86_64 Linux with git and podman (or docker). The TouchPad family (tenderloin
# and opal) shares one ARMv7 softfp binary set — see docs/DEVICE-BUILD.md.
#
#   0. inputs     (each skipped when already present)
#                 submodules            git submodule update --init
#                 cross toolchain       toolchain/build.sh                  (about an hour, once)
#                 Palm PDK              fetch-pdk.sh                        (357 MB download, once)
#                 Qt4 headers           gen-adapter-deps-manifest.sh --fetch-qt4
#                 ARM sysroot           gen-sysroot-manifest.sh --fetch, --assemble
#                 device libraries      --fetch-device-libs of both manifests: needs a TouchPad on
#                                       USB and novacom, once (eight libraries HP never published)
#   1. engine    libxul.so            (build-goanna-arm.sh — SLOW, hours)
#   2. daemon    jihad-browserserver  (build-daemon-arm.sh)
#   3. adapter   BrowserAdapterJihad{,Mochi,Mojo}.so + per-variant BrowserAdapterImpl.so
#                                                              (build-adapter-pdk.sh)
#   4. UI ipks   the THREE self-contained packages              (build-variant-ipk.sh, which also
#                net.riverstonerelay.jihad-browser{,-mochi,-mojo}_*.ipk   runs make-device-bundle.sh)
#
# Steps 1 and 2 run in the jihad-goanna-build container, 3 and 4 in jihad-host-tools
# (in-container.sh builds both images on first use). JIHAD_NO_CONTAINER=1 runs 3 and 4 on the host
# instead, which then needs 32-bit libraries for the PDK's gcc and Java for palm-package.
#
# Default: build what is missing, rebuild the daemon and the packages, and REUSE an existing engine
# and adapter. Flags:
#   --engine    also rebuild libxul (hours)
#   --adapter   also rebuild the adapter
#   --skip-arm  skip the daemon too (packages only)
# Every produced artifact is listed with an md5 at the end.
#
# NOTE (kit deviation, documented): the Enyo package does NOT bundle Enyo 1.0 —
# app/index.html loads /usr/palm/frameworks/enyo/0.10 from the OS, exactly like
# upstream isis-browser. Bundling would duplicate the system framework and risk
# version skew. The Mochi package DOES bundle Enyo 2 + layout + Mochi (no system
# Enyo 2 exists on webOS 3) via build-mochi-ipk.sh.
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../.." && pwd)
. "$HERE/host-path.sh"   # the repository's own novacom and palm-* first (R11)
OUT="$HERE/out-all"                       # git-ignored by /build/**/out-*/
ENGINE=0 ADAPTER=0 SKIP_ARM=0
for a in "$@"; do case "$a" in
	--engine) ENGINE=1;; --adapter) ADAPTER=1;; --skip-arm) SKIP_ARM=1;;
	*) echo "unknown flag: $a" >&2; exit 2;;
esac; done

say() { echo; echo "=== $* ==="; }
have() { [ -e "$1" ]; }
host_step() { # <script> [args]: steps 3 and 4
	if [ "${JIHAD_NO_CONTAINER:-0}" = 1 ]; then bash "$@"; else bash "$HERE/in-container.sh" "$@"; fi
}

mkdir -p "$OUT"

# --- 0. inputs ------------------------------------------------------------------
say "inputs"
# The default submodules (build-webos, update = none, stays out). Only when one is missing, so a
# submodule someone is working in is never reset.
if git -C "$REPO" submodule status | grep '^-' | grep -qv ' build/webos-oe/build-webos'; then
	git -C "$REPO" submodule update --init
fi
TC="$HERE/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin/arm-webos-linux-gnueabi-gcc"
have "$TC" || { say "toolchain: toolchain/build.sh (about an hour)"; bash "$HERE/toolchain/build.sh" build; }
have "$HERE/pdk/opt/PalmPDK/arm-gcc" || { say "PDK: fetch-pdk.sh"; bash "$HERE/fetch-pdk.sh"; }
have "$HERE/adapter-deps/qt4-extract/usr" || bash "$HERE/gen-adapter-deps-manifest.sh" --fetch-qt4
NEED_ADAPTER_LIBS=0 NEED_SYSROOT_LIBS=0
have "$HERE/adapter-deps/staging/lib/libQtCore.so.4.8.0" || NEED_ADAPTER_LIBS=1
have "$HERE/arm-sysroot/root/usr" || have "$HERE/arm-sysroot/device/libpulse.so.0" || NEED_SYSROOT_LIBS=1
if [ "$NEED_ADAPTER_LIBS" = 1 ] || [ "$NEED_SYSROOT_LIBS" = 1 ]; then
	if ! command -v novacom >/dev/null || ! novacom -l 2>/dev/null | grep -q .; then
		echo "!! eight libraries come off a TouchPad, once: connect one over USB with novacomd running" >&2
		echo "   (tools/host/build-novacom.sh builds novacom and novacomd from the submodule)" >&2
		exit 3
	fi
	[ "$NEED_ADAPTER_LIBS" = 0 ] || bash "$HERE/gen-adapter-deps-manifest.sh" --fetch-device-libs
	[ "$NEED_SYSROOT_LIBS" = 0 ] || bash "$HERE/gen-sysroot-manifest.sh" --fetch-device-libs
fi
if ! have "$HERE/arm-sysroot/root/usr"; then
	bash "$HERE/gen-sysroot-manifest.sh" --fetch
	bash "$HERE/gen-sysroot-manifest.sh" --assemble
fi

# --- 1. engine ------------------------------------------------------------------
LIBXUL="$HERE/out-arm/obj-jihad-goanna-arm/dist/bin/libxul.so"
if [ "$ENGINE" = 1 ] || ! have "$LIBXUL"; then
	say "engine: build-goanna-arm.sh build (slow)"
	bash "$HERE/in-container.sh" --arm "$HERE/build-goanna-arm.sh" build
else
	say "engine: reusing $LIBXUL (pass --engine to rebuild)"
fi

# --- 2. daemon ------------------------------------------------------------------
DAEMON="$HERE/out-arm/jihad-browserserver-arm"
if [ "$SKIP_ARM" = 1 ]; then
	say "daemon: skipped (--skip-arm)"
else
	say "daemon: build-daemon-arm.sh"
	bash "$HERE/in-container.sh" --arm "$HERE/build-daemon-arm.sh"
fi

# --- 3. adapter -----------------------------------------------------------------
ADAPTER_SO="$HERE/adapter-deps/build-pdk/BrowserAdapterJihad.so"
ADAPTER_IMPL="$REPO/app/BrowserAdapterImpl.so"
if [ "$ADAPTER" = 1 ] || ! have "$ADAPTER_SO" || ! have "$ADAPTER_IMPL"; then
	say "adapter: build-adapter-pdk.sh"
	host_step "$HERE/build-adapter-pdk.sh"
else
	say "adapter: reusing existing shim/impl (pass --adapter to rebuild)"
fi

# --- 4. the three SELF-CONTAINED UI ipks ----------------------------------------
# build-variant-ipk.sh produces, per variant, the same payload shape the OE product recipe does:
# the app tree + deviceroot/{hl, BrowserPlugins/<shim>, event.d/<job>} + the adapter impl +
# LICENSE/NOTICE, and it repacks the .ipk so control.tar.gz carries that variant's
# packaging/<V>/{postinst,prerm}. A plain `palm-package app/` cannot do this — it emits a UI-only
# package whose control.tar.gz holds nothing but ./control, so the daemon/shim/upstart would
# never be installed. Install the results via Preware or WebOS Quick Install, NOT palm-install.
say "ipks: build-variant-ipk.sh (enyo mochi mojo — self-contained, postinst/prerm injected)"
host_step "$HERE/build-variant-ipk.sh" enyo mochi mojo
cp "$HERE"/out-ipk/net.riverstonerelay.jihad-browser*_all.ipk "$OUT/"

# --- report ---------------------------------------------------------------------
say "artifacts"
for f in "$LIBXUL" "$DAEMON" "$ADAPTER_SO" "$ADAPTER_IMPL" \
         "$OUT"/net.riverstonerelay.jihad-browser_*_all.ipk \
         "$OUT"/net.riverstonerelay.jihad-browser-mochi_*_all.ipk \
         "$OUT"/net.riverstonerelay.jihad-browser-mojo_*_all.ipk; do
	if have "$f"; then
		printf '  %s  %s\n' "$(md5sum "$f" | cut -c1-8)" "$f"
	else
		printf '  MISSING   %s\n' "$f"; MISS=1
	fi
done
[ "${MISS:-0}" = 0 ] && echo "=== ALL ARTIFACTS PRESENT ===" || {
	echo "=== INCOMPLETE (missing artifacts above) ==="; exit 1; }
