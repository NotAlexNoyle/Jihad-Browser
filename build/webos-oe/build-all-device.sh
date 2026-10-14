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
# What every later step needs, checked before anything slow starts.
if [ "${JIHAD_NO_CONTAINER:-0}" != 1 ] && ! command -v "${CONTAINER_ENGINE:-podman}" >/dev/null && ! command -v docker >/dev/null; then
	echo "!! need podman or docker (README.md, Building)" >&2; exit 2
fi
# Only the submodules that are missing, by path, so a submodule someone is working in is never
# reset; build-webos (update = none) stays out. The listing is captured first: a grep -q on the
# pipeline would exit early and hand git SIGPIPE under pipefail.
MISSING="$(git -C "$REPO" submodule status | awk '$1 ~ /^-/ && $2 != "build/webos-oe/build-webos" {print $2}')"
# A submodule at its pin with an EMPTY working tree counts as missing too: an interrupted
# `git clone --recurse-submodules` leaves exactly that (2026-10-14, the clean-clone run: four of
# them, and the adapter failed on the NPAPI headers' dangling symlinks). --force restores it.
for p in $(git -C "$REPO" submodule status | awk '$1 !~ /^-/ && $2 != "build/webos-oe/build-webos" {print $2}'); do
	[ "$(find "$REPO/$p" -mindepth 1 -maxdepth 1 ! -name .git | head -1)" ] || MISSING="$MISSING $p"
done
if [ -n "${MISSING// /}" ]; then
	echo "submodules to check out:$MISSING"
	# shellcheck disable=SC2086
	git -C "$REPO" submodule update --init --force -- $MISSING
fi
# A device library counts as present only with the sha256 the recipe records for it: a file an
# interrupted novacom transfer left behind must not pass as fetched.
device_lib_ok() { # <file> <recipe>
	[ -f "$1" ] || return 1
	local want; want="$(awk -F'\t' -v n="$(basename "$1")" '$1=="device" && $3 ~ ("/" n "$") {print $4; exit}' "$2")"
	[ -n "$want" ] && [ "$(sha256sum "$1" | cut -d' ' -f1)" = "$want" ]
}
NEED_ADAPTER_LIBS=0 NEED_SYSROOT_LIBS=0
have "$HERE/adapter-deps/staging/lib/libQtCore.so.4.8.0" || NEED_ADAPTER_LIBS=1
if ! have "$HERE/arm-sysroot/root/usr" && ! device_lib_ok "$HERE/arm-sysroot/device/libpulse.so.0" "$HERE/arm-sysroot.recipe"; then
	NEED_SYSROOT_LIBS=1
fi
# Say now, before the hour-long toolchain build, when a TouchPad is needed.
if [ "$NEED_ADAPTER_LIBS" = 1 ] || [ "$NEED_SYSROOT_LIBS" = 1 ]; then
	DEVICES="$( { command -v novacom >/dev/null && novacom -l; } 2>/dev/null || true)"
	if [ -z "$DEVICES" ]; then
		echo "!! eight libraries come off a TouchPad, once: connect one over USB with novacomd running" >&2
		echo "   (tools/host/build-novacom.sh builds novacom and novacomd from the submodule)" >&2
		exit 3
	fi
fi
# The inputs that need no device.
TC="$HERE/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin/arm-webos-linux-gnueabi-gcc"
have "$TC" || { say "toolchain: toolchain/build.sh (about an hour)"; bash "$HERE/toolchain/build.sh" build; }
have "${PDK_ROOT:-$HERE/pdk/opt/PalmPDK}/arm-gcc" || { say "PDK: fetch-pdk.sh"; bash "$HERE/fetch-pdk.sh"; }
have "$HERE/adapter-deps/qt4-extract/usr" || bash "$HERE/gen-adapter-deps-manifest.sh" --fetch-qt4
have "$HERE/arm-sysroot/root/usr" || bash "$HERE/gen-sysroot-manifest.sh" --fetch
# Then the eight libraries off the TouchPad, and the sysroot from all of it.
[ "$NEED_ADAPTER_LIBS" = 0 ] || bash "$HERE/gen-adapter-deps-manifest.sh" --fetch-device-libs
[ "$NEED_SYSROOT_LIBS" = 0 ] || bash "$HERE/gen-sysroot-manifest.sh" --fetch-device-libs
have "$HERE/arm-sysroot/root/usr" || bash "$HERE/gen-sysroot-manifest.sh" --assemble

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
# Only this run's three packages go to out-all (each variant's appinfo.json names its version);
# older versions left in out-ipk stay there, and stale copies in out-all go.
# One process, no pipe: `| head -1` here would hand sed SIGPIPE under pipefail (F-13 again).
ver_of() { awk 'match($0, /"version"[[:space:]]*:[[:space:]]*"[^"]*"/) { s = substr($0, RSTART, RLENGTH); sub(/^"version"[[:space:]]*:[[:space:]]*"/, "", s); sub(/"$/, "", s); print s; exit }' "$1"; }
IPKS=("$HERE/out-ipk/net.riverstonerelay.jihad-browser_$(ver_of "$REPO/app/appinfo.json")_all.ipk"
      "$HERE/out-ipk/net.riverstonerelay.jihad-browser-mochi_$(ver_of "$REPO/app-mochi/appinfo.json")_all.ipk"
      "$HERE/out-ipk/net.riverstonerelay.jihad-browser-mojo_$(ver_of "$REPO/app-mojo/appinfo.json")_all.ipk")
rm -f "$OUT"/*.ipk
OUT_IPKS=()
for f in "${IPKS[@]}"; do
	if have "$f"; then cp "$f" "$OUT/"; fi
	OUT_IPKS+=("$OUT/$(basename "$f")")
done

# --- report ---------------------------------------------------------------------
say "artifacts"
for f in "$LIBXUL" "$DAEMON" "$ADAPTER_SO" "$ADAPTER_IMPL" "${OUT_IPKS[@]}"; do
	if have "$f"; then
		printf '  %s  %s\n' "$(md5sum "$f" | cut -c1-8)" "$f"
	else
		printf '  MISSING   %s\n' "$f"; MISS=1
	fi
done
[ "${MISS:-0}" = 0 ] && echo "=== ALL ARTIFACTS PRESENT ===" || {
	echo "=== INCOMPLETE (missing artifacts above) ==="; exit 1; }
