# Shared definitions for the Jihad Browser OE recipes.
#
# The recipe SOURCES live at the REPO ROOT (this layer is build/webos-oe/ *inside* the
# repo, while app/, render/, packaging/, LICENSE, … are at the top). Under oe-env.sh the
# repo is bind-mounted at /oe/Jihad-Browser inside the chroot, so pointing the file://
# fetcher's search path (FILESEXTRAPATHS) there lets `file://app/`, `file://render/`, etc.
# resolve to the real trees. Override JIHAD_REPO if you build outside oe-env.sh.
JIHAD_REPO ?= "/oe/Jihad-Browser"
FILESEXTRAPATHS_prepend := "${JIHAD_REPO}:"

# One license anchor for all Jihad recipes (the composite Apache-2.0 + MPL-2.0 text at the
# repo root). Recipes still set their own LICENSE string; this is the change-detection file.
LIC_FILES_CHKSUM ?= "file://${JIHAD_REPO}/LICENSE;md5=be353ef4e243581e528e4524ffa92d97"

# webOS on-device install locations (this 2014 meta-webos has no webos-app class; the app
# dir is the standard webOS 3 path).
webos_applicationsdir ?= "/usr/palm/applications"

# The native cross-toolchain + target sysroot for the ARM engine/daemon builds. Both are
# assembled in-repo (gitignored, 219M/189M) and bound in the chroot, so recipes reference them
# in place. The toolchain is crosstool-NG GCC 9.4 (arm-webos-linux-gnueabi, glibc 2.23 → runs
# on the device 2.6.35 kernel); the sysroot is Debian-Jessie armel (GTK/X/cairo/pango/fontconfig).
JIHAD_TC ?= "${JIHAD_REPO}/build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi"
JIHAD_ARM_SYSROOT ?= "${JIHAD_REPO}/build/webos-oe/arm-sysroot/root"
