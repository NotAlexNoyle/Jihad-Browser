# Jihad Browser — cross-toolchain edge for the ARM engine/daemon builds.
#
# The Goanna/UXP engine cannot be built by the 2013 dylan gcc (nor the device gcc 4.4) — it
# needs a modern C++14 compiler. Jihad uses a crosstool-NG GCC 9.4 targeting
# arm-webos-linux-gnueabi (glibc 2.23, softfp ARMv7) — the same toolchain the direct
# cross-build (build/webos-oe/build-goanna-arm.sh) uses, built from source by crosstool-NG.
#
# This is a `native` recipe: crosstool-NG's compiler is an x86_64 host binary that emits ARM,
# so it legitimately runs on the build host. It does not rebuild the toolchain here (that is a
# separate, heavy crosstool-NG run assembled at build/webos-oe/toolchain/) — it owns the
# DEPENDS edge for `goanna`/`jihad-browserserver` and fails early with a clear message if the
# toolchain has not been assembled. The goanna recipe references ${JIHAD_TC} in place (both the
# recipe layer and the toolchain are bound in the oe-env.sh chroot).

SUMMARY = "arm-webos-linux-gnueabi GCC 9.4 cross-toolchain edge (crosstool-NG) for Goanna"
LICENSE = "GPL-3.0-with-GCC-exception"

require ../jihad-common.inc

inherit native

# Nothing to fetch/configure/compile — the toolchain is prebuilt (from source, by crosstool-NG).
SRC_URI = ""
do_fetch[noexec]     = "1"
do_unpack[noexec]    = "1"
do_patch[noexec]     = "1"
do_configure[noexec] = "1"
do_compile[noexec]   = "1"

do_install() {
    if [ ! -x "${JIHAD_TC}/bin/arm-webos-linux-gnueabi-gcc" ]; then
        bbfatal "crosstool-NG toolchain not found at ${JIHAD_TC}. Assemble it first (crosstool-NG build under build/webos-oe/toolchain/); it is git-ignored and not vendored."
    fi
    # Record the resolved toolchain root so consumers/logs can find it; the real bits are used
    # in place from ${JIHAD_TC} (219 MB — not copied into every recipe sysroot).
    install -d ${D}${datadir}/jihad-cross-toolchain
    echo "${JIHAD_TC}" > ${D}${datadir}/jihad-cross-toolchain/toolchain-root
    "${JIHAD_TC}/bin/arm-webos-linux-gnueabi-gcc" -dumpversion > ${D}${datadir}/jihad-cross-toolchain/gcc-version || true
}
