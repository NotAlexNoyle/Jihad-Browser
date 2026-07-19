# Jihad Browser — Goanna/UXP engine for webOS 3 ARMv7 (OpenEmbedded recipe).
# The heavy recipe: cross-compiles UXP with the embedding config against the
# device sysroot. Replaces the isis webkit-webos + qt4-webos engine deps.
#
# STATUS: skeleton. Requires the modern ARMv7 cross-toolchain (device-build R1)
# and the UXP source; the actual cross-build is gated on the device sysroot.
# See ../../../docs/DEVICE-BUILD.md and ../../mozconfig.goanna-arm.

SUMMARY = "Goanna (UXP) web engine, embedding build for Jihad Browser"
HOMEPAGE = "https://repo.palemoon.org/MoonchildProductions/UXP"
LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=<fill-from-uxp>"

# Both TouchPad models (TouchPad = topaz/tenderloin, TouchPad Go = opal) share ONE
# APQ8060 ARMv7 softfp libxul — there is no per-model engine variance (device-build R6).
# The machine confs (../../conf/machine/{tenderloin,opal}.conf) both resolve to
# DEFAULTTUNE "armv7a-neon" (softfp), which is exactly the mozconfig.goanna-arm target,
# so the same libxul.so installs on both. Model differences are UI-side DPI only, not
# compiled into the engine. Captured difference: none — shared ARMv7 softfp binary.
COMPATIBLE_MACHINE = "(tenderloin|opal)"

# UXP is fetched, not vendored. Pin the same revision documented in
# docs/ENGINE-SOURCE.md (b2594a4...).
SRC_URI = "git://repo.palemoon.org/MoonchildProductions/UXP.git;protocol=https;branch=master"
SRCREV = "b2594a4ace4556b0a953c079a8c1bc350fc095ec"
S = "${WORKDIR}/git"

# Modern C++14 toolchain (device-build R1) — the stock gcc 4.4 cannot build UXP.
DEPENDS = "jihad-cross-toolchain-native gtk+ nspr nss"

# The Jihad build patches + branding strip (same as desktop) are applied here.
SRC_URI += "file://patches/ file://mozconfig.goanna-arm"

do_configure() {
    # Apply build/desktop/patches/* + the branding strip (see build-goanna.sh),
    # then run mach configure with the ARM mozconfig against the sysroot.
    export MOZCONFIG="${WORKDIR}/mozconfig.goanna-arm"
    cd ${S} && ./mach configure
}

do_compile() {
    cd ${S} && ./mach build
}

do_install() {
    # Stage libxul.so + the frozen embedding headers the daemon links against.
    install -d ${D}${libdir}/goanna
    install -m 0755 ${S}/../obj-jihad-goanna-arm/dist/bin/libxul.so ${D}${libdir}/goanna/
    install -d ${D}${includedir}/goanna
    cp -r ${S}/../obj-jihad-goanna-arm/dist/include/* ${D}${includedir}/goanna/
}

FILES:${PN} = "${libdir}/goanna/libxul.so"
FILES:${PN}-dev = "${includedir}/goanna"
INSANE_SKIP:${PN} = "already-stripped dev-so"
