#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Run one of this repository's build scripts in a container, so the host needs only podman (or
# docker) and git (cavekit-device-build.md R11).
#
#   build/webos-oe/in-container.sh <script> [args...]          the packaging/adapter host
#   build/webos-oe/in-container.sh --arm <script> [args...]    the engine/daemon cross host
#
# Default: the jihad-host-tools image (host-tools/Dockerfile: 32-bit libc for the PDK's gcc 4.3.3,
# Java for palm-package, binutils). The repository is mounted at its own path, so every path the
# scripts derive from their own location works unchanged.
#
# --arm: the jihad-goanna-build image (build/desktop/Dockerfile) with the mounts that
# build-goanna-arm.sh and build-daemon-arm.sh are written against: /src/uxp (the engine, read-write),
# /jihad (the repository), /tc (the crosstool-NG toolchain), /sysroot (arm-sysroot/root), /cfg
# (build/desktop), /armcfg (build/webos-oe), /out (build/webos-oe/out-arm).
#
# Each image is built on first use; JIHAD_REBUILD_IMAGES=1 builds it again. CONTAINER_ENGINE picks
# podman or docker (default: podman if installed).
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
ARM=0
if [ "${1:-}" = "--arm" ]; then ARM=1; shift; fi
SCRIPT="${1:?usage: in-container.sh [--arm] <script> [args...]}"; shift
SCRIPT="$(cd "$(dirname "$SCRIPT")" && pwd)/$(basename "$SCRIPT")"
case "$SCRIPT" in "$REPO"/*) ;; *) echo "!! $SCRIPT is not in this repository" >&2; exit 2 ;; esac

ENGINE="${CONTAINER_ENGINE:-$(command -v podman || command -v docker || true)}"
[ -n "$ENGINE" ] || { echo "!! need podman or docker" >&2; exit 2; }
ensure_image() { # <name> <context dir>
	if [ "${JIHAD_REBUILD_IMAGES:-0}" = 1 ] || ! "$ENGINE" image inspect "$1" >/dev/null 2>&1; then
		echo "== building image $1 from ${2#"$REPO"/} =="
		"$ENGINE" build -t "$1" "$2"
	fi
}
# Run as the invoking user, so what the build writes is the user's own; no SELinux relabelling of
# the repository (label=disable), which would otherwise walk the whole tree on Fedora-like hosts.
RUNOPTS=(--rm --security-opt label=disable -e HOME=/tmp -e JIHAD_IN_CONTAINER=1)
if [ "$(basename "$ENGINE")" = podman ]; then RUNOPTS+=(--userns=keep-id); else RUNOPTS+=(--user "$(id -u):$(id -g)"); fi
[ -t 0 ] && [ -t 1 ] && RUNOPTS+=(-t)

if [ "$ARM" = 0 ]; then
	ensure_image jihad-host-tools "$HERE/host-tools"
	WD="$PWD"; case "$WD" in "$REPO"|"$REPO"/*) ;; *) WD="$REPO" ;; esac
	exec "$ENGINE" run "${RUNOPTS[@]}" -v "$REPO:$REPO" -w "$WD" jihad-host-tools bash "$SCRIPT" "$@"
fi

TC="$HERE/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi"
[ -x "$TC/bin/arm-webos-linux-gnueabi-gcc" ] || { echo "!! no toolchain: build/webos-oe/toolchain/build.sh" >&2; exit 2; }
[ -d "$HERE/arm-sysroot/root/usr" ] || { echo "!! no ARM sysroot: build/webos-oe/gen-sysroot-manifest.sh --fetch, --fetch-device-libs, --assemble" >&2; exit 2; }
[ -f "$REPO/third_party/uxp/mach" ] || { echo "!! third_party/uxp is not checked out: git submodule update --init third_party/uxp" >&2; exit 2; }
mkdir -p "$HERE/out-arm"
ensure_image jihad-goanna-build "$REPO/build/desktop"
case "$SCRIPT" in
	"$HERE"/*) INNER="/armcfg/${SCRIPT#"$HERE"/}" ;;
	*) INNER="/jihad/${SCRIPT#"$REPO"/}" ;;
esac
exec "$ENGINE" run "${RUNOPTS[@]}" \
	-v "$REPO/third_party/uxp:/src/uxp" -v "$REPO:/jihad" -v "$TC:/tc" \
	-v "$HERE/arm-sysroot/root:/sysroot" -v "$REPO/build/desktop:/cfg" \
	-v "$HERE:/armcfg" -v "$HERE/out-arm:/out" \
	jihad-goanna-build bash "$INNER" "$@"
