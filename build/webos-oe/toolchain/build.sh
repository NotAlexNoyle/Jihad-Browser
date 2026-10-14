#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Build the webOS-3 ARM cross toolchain from a clean clone, on any Linux with podman (or docker).
#
#   build/webos-oe/toolchain/build.sh [config|build]     (default: build, about an hour)
#
# The image (Dockerfile here) carries only Ubuntu 20.04's build packages. crosstool-NG itself is
# the submodule third_party/crosstool-ng, mounted read-only and built inside the container by
# build-toolchain.sh; the toolchain lands in out-toolchain/x-tools/arm-webos-linux-gnueabi/
# (git-ignored), with the source tarballs crosstool-NG downloads cached in out-toolchain/src/.
# TOOLCHAIN_OUT=<dir> writes elsewhere (to rebuild beside a toolchain in use, and compare).
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../../.." && pwd)"
CTNG="$REPO/third_party/crosstool-ng"
MODE="${1:-build}"
ENGINE="${CONTAINER_ENGINE:-$(command -v podman || command -v docker || true)}"
[ -n "$ENGINE" ] || { echo "!! need podman or docker" >&2; exit 2; }
[ -f "$CTNG/bootstrap" ] || { echo "!! third_party/crosstool-ng is not checked out: git submodule update --init third_party/crosstool-ng" >&2; exit 2; }
# The version crosstool-NG reports (and writes into every binary the toolchain compiles, as
# "crosstool-NG <version>") comes from the submodule's tag; the container cannot run git on it.
VER="$(git -C "$CTNG" describe --tags --exact-match 2>/dev/null)" || { echo "!! third_party/crosstool-ng is not at a release tag" >&2; exit 2; }
VER="${VER#crosstool-ng-}"
OUT="${TOOLCHAIN_OUT:-$HERE/out-toolchain}"
mkdir -p "$OUT"
"$ENGINE" build -t jihad-xtool-build "$HERE"
# keep-id: the builder (uid 1000 in the image) writes /out as the invoking user. With docker, run
# as root or with a uid-1000 user.
USERNS=(); [ "$(basename "$ENGINE")" = podman ] && USERNS=(--userns=keep-id)
"$ENGINE" run --rm "${USERNS[@]}" -e CTNG_VERSION="$VER" \
  -v "$HERE:/cfg:ro" -v "$CTNG:/ctng-src:ro" -v "$OUT:/out" \
  jihad-xtool-build bash /cfg/build-toolchain.sh "$MODE"
