#!/bin/bash
# Copyright 2026 the Jihad Browser project. Apache-2.0.
#
# Single-entry device build (device-build R3): produces ALL FOUR artifact
# classes for the TouchPad family (tenderloin + opal share one ARMv7 softfp
# binary set — see docs/DEVICE-BUILD.md):
#
#   1. engine    libxul.so            (build-goanna-arm.sh — SLOW, container)
#   2. daemon    jihad-browserserver  (build-daemon-arm.sh — container)
#      + bundle  device-bundle/       (make-device-bundle.sh)
#   3. adapter   BrowserAdapterJihad.so + BrowserAdapterImpl.so (build-adapter-pdk.sh)
#   4. UI ipks   net.riverstonerelay.jihad-browser_*.ipk        (palm-package app/)
#                net.riverstonerelay.jihad-browser.mochi_*.ipk  (build-mochi-ipk.sh)
#
# Default: rebuild the FAST parts (daemon, bundle, both ipks) and REUSE existing
# slow artifacts (libxul, adapter) when present — pass flags to force:
#   --engine    also rebuild libxul (hours)
#   --adapter   also rebuild the adapter (needs PDK toolchain)
#   --skip-arm  skip daemon+bundle too (ipks only; no podman needed)
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
OUT="$HERE/out-all"                       # git-ignored by /build/**/out-*/
ENGINE=0 ADAPTER=0 SKIP_ARM=0
for a in "$@"; do case "$a" in
	--engine) ENGINE=1;; --adapter) ADAPTER=1;; --skip-arm) SKIP_ARM=1;;
	*) echo "unknown flag: $a" >&2; exit 2;;
esac; done

say() { echo; echo "=== $* ==="; }
have() { [ -e "$1" ]; }

mkdir -p "$OUT"

# --- 1. engine ------------------------------------------------------------------
LIBXUL="$HERE/out-arm/obj-jihad-goanna-arm/dist/bin/libxul.so"
if [ "$ENGINE" = 1 ] || ! have "$LIBXUL"; then
	say "engine: build-goanna-arm.sh build (slow)"
	bash "$HERE/build-goanna-arm.sh" build
else
	say "engine: reusing $LIBXUL (pass --engine to rebuild)"
fi

# --- 2. daemon + bundle ---------------------------------------------------------
DAEMON="$HERE/out-arm/jihad-browserserver-arm"
if [ "$SKIP_ARM" = 1 ]; then
	say "daemon+bundle: skipped (--skip-arm)"
else
	say "daemon: build-daemon-arm.sh"
	bash "$HERE/build-daemon-arm.sh"
	say "bundle: make-device-bundle.sh"
	bash "$HERE/make-device-bundle.sh"
fi

# --- 3. adapter -----------------------------------------------------------------
ADAPTER_SO="$HERE/adapter-deps/build-pdk/BrowserAdapterJihad.so"
ADAPTER_IMPL="$REPO/app/BrowserAdapterImpl.so"
if [ "$ADAPTER" = 1 ]; then
	say "adapter: build-adapter-pdk.sh"
	bash "$HERE/build-adapter-pdk.sh"
else
	say "adapter: reusing existing shim/impl (pass --adapter to rebuild)"
fi

# --- 4. the two UI ipks ---------------------------------------------------------
# Enyo ipk: stage app/ minus repo-docs (a shipped ipk once carried stray
# CLAUDE.md/README.md — keep them out of the payload), then palm-package.
say "Enyo ipk: palm-package (staged app/)"
ESTAGE="$OUT/app-stage"
rm -rf "$ESTAGE"; mkdir -p "$ESTAGE"
rsync -a --exclude=CLAUDE.md --exclude=README.md "$REPO/app/" "$ESTAGE/"
palm-package "$ESTAGE" -o "$OUT"

say "Mochi ipk: build-mochi-ipk.sh"
bash "$HERE/build-mochi-ipk.sh"
cp "$HERE"/out-mochi/ipk/net.riverstonerelay.jihad-browser.mochi_*_all.ipk "$OUT/"

# --- report ---------------------------------------------------------------------
say "artifacts"
for f in "$LIBXUL" "$DAEMON" "$ADAPTER_SO" "$ADAPTER_IMPL" \
         "$OUT"/net.riverstonerelay.jihad-browser_*_all.ipk \
         "$OUT"/net.riverstonerelay.jihad-browser.mochi_*_all.ipk; do
	if have "$f"; then
		printf '  %s  %s\n' "$(md5sum "$f" | cut -c1-8)" "$f"
	else
		printf '  MISSING   %s\n' "$f"; MISS=1
	fi
done
[ "${MISS:-0}" = 0 ] && echo "=== ALL ARTIFACTS PRESENT ===" || {
	echo "=== INCOMPLETE (missing artifacts above) ==="; exit 1; }
