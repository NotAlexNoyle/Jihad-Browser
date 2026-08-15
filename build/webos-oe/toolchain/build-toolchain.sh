#!/usr/bin/env bash
# Jihad Browser — build the webOS-3 ARM cross toolchain with crosstool-NG.
#
# Runs INSIDE the jihad-xtool-build image (see Dockerfile). The toolchain is
# written to /out (bind-mount a persistent host dir there so it survives).
#
# Usage (from the host):
#   podman run --rm -v <hostdir>:/out:Z jihad-xtool-build \
#       bash /out/../build-toolchain.sh [config|build]
# In practice we bind the toolchain/ dir read-only at /cfg and an output dir at /out:
#   podman run --rm \
#       -v $PWD:/cfg:ro \
#       -v $PWD/out-toolchain:/out:Z \
#       jihad-xtool-build bash /cfg/build-toolchain.sh build
#
# Modes:
#   config  - generate + validate .config only (fast; catches symbol errors)
#   build   - full ct-ng build (GCC + glibc + binutils; ~45-90 min)
set -euo pipefail

MODE="${1:-config}"
WORK=/home/builder/ctng-work
mkdir -p "$WORK"
cd "$WORK"

# Start from ct-ng's arm softfp glibc sample, then overlay our defconfig fragment.
# The sample gives valid, version-correct symbol names for this ct-ng release;
# the fragment (jihad.defconfig) overrides only what the device demands.
echo "== loading base sample: arm-unknown-linux-gnueabi (softfp glibc) =="
ct-ng arm-unknown-linux-gnueabi

echo "== overlaying jihad.defconfig fragment =="
# Merge fragment on top of the sample's .config, then normalise.
cat /cfg/jihad.defconfig >> .config
ct-ng olddefconfig

echo "== effective target tuple + key choices =="
grep -E '^CT_TARGET(_VENDOR)?=|^CT_ARCH_ARCH=|^CT_ARCH_FLOAT|^CT_ARCH_FPU=|^CT_GCC_VERSION=|^CT_GLIBC_VERSION=|^CT_GLIBC_MIN_KERNEL|^CT_LINUX_VERSION=' .config || true

if [ "$MODE" = "config" ]; then
    echo "== CONFIG-ONLY: validated. Not building. =="
    cp .config /out/jihad-arm.config
    exit 0
fi

echo "== building toolchain (this takes a while) =="
# Helper: set a kconfig string reliably (replace if present, else append).
set_cfg() { # $1=SYMBOL $2=value(with quotes already)
    if grep -q "^$1=" .config; then
        sed -i "s#^$1=.*#$1=$2#" .config
    else
        echo "$1=$2" >> .config
    fi
}
# Install prefix + local tarball cache inside the persistent /out mount
# (${CT_TARGET} = arm-webos-linux-gnueabi). Pre-seeded tarballs (e.g. the kernel
# headers that kernel.org blocks here) live in /out/src and are used as-is.
mkdir -p /out/src
set_cfg CT_PREFIX_DIR '"/out/x-tools/${CT_TARGET}"'
set_cfg CT_LOCAL_TARBALLS_DIR '"/out/src"'
set_cfg CT_SAVE_TARBALLS 'y'
ct-ng olddefconfig
ct-ng "build.$(nproc)"

echo "== DONE. Toolchain in /out/x-tools =="
ls -la /out/x-tools/*/bin/*gcc 2>&1 || true
