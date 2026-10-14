#!/usr/bin/env bash
# Jihad Browser — build the webOS-3 ARM cross toolchain with crosstool-NG.
#
# Runs INSIDE the jihad-xtool-build image (see Dockerfile); start it with build.sh beside this
# file, which mounts this directory at /cfg, the crosstool-NG submodule (third_party/crosstool-ng)
# at /ctng-src and out-toolchain/ at /out, and passes CTNG_VERSION (the submodule's tag).
#
# Modes:
#   config  - generate + validate .config only (fast; catches symbol errors)
#   build   - full ct-ng build (GCC + glibc + binutils; ~45-90 min)
set -euo pipefail

MODE="${1:-config}"
WORK=/home/builder/ctng-work

# crosstool-NG from the submodule: copied (bootstrap writes into its tree), told its version (no
# git in here; git-version-gen reads .tarball-version), bootstrapped and installed under $HOME.
: "${CTNG_VERSION:?CTNG_VERSION is not set; run build.sh}"
[ -f /ctng-src/bootstrap ] || { echo "!! /ctng-src is not the crosstool-NG submodule"; exit 2; }
echo "== crosstool-NG $CTNG_VERSION from the submodule =="
rm -rf /home/builder/ctng-src /home/builder/ctng
cp -a /ctng-src /home/builder/ctng-src
rm -f /home/builder/ctng-src/.git
echo "$CTNG_VERSION" > /home/builder/ctng-src/.tarball-version
( cd /home/builder/ctng-src && ./bootstrap && ./configure --prefix=/home/builder/ctng \
  && make -j"$(nproc)" && make install ) > /home/builder/ctng-install.log 2>&1 \
  || { tail -30 /home/builder/ctng-install.log; echo "!! crosstool-NG did not build"; exit 2; }
export PATH="/home/builder/ctng/bin:$PATH"
ct-ng version > /home/builder/ctng-version.txt
sed -n 1p /home/builder/ctng-version.txt

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
