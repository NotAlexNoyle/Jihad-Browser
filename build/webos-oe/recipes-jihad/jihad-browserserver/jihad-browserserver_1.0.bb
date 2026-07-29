# Jihad Browser — render daemon (BrowserServer + Goanna backend) for webOS 3.
# Mirrors meta-webos/recipes-webos/browserserver but DEPENDS on goanna instead
# of webkit-webos + qt4-webos. The YAP IPC contract is byte-identical to isis.
#
# do_compile mirrors build/webos-oe/build-daemon-arm.sh (the verified device build): a plain
# g++ compile+link with the crosstool-NG toolchain against the Jessie armel sysroot and the
# staged goanna dist (libxul + libxpcomglue_s.a + libmozglue.a + nspr + headers). No cmake.

SUMMARY = "Jihad Browser render daemon (Goanna-backed BrowserServer)"
LICENSE = "Apache-2.0 & MPL-2.0"

require ../jihad-common.inc

# Model-agnostic: one ARMv7 softfp binary valid on BOTH TouchPad models (device-build R6).
COMPATIBLE_MACHINE = "(tenderloin|opal)"

# Sources live in this repo (bound in the chroot via FILESEXTRAPATHS from jihad-common.inc).
SRC_URI = "file://render/ file://packaging/"
S = "${WORKDIR}"

# goanna stages libxul + glue + headers into the sysroot; the toolchain edge validates the
# cross gcc. GTK/glib/nspr come from the Jessie sysroot via pkg-config, NOT from OE.
DEPENDS = "goanna jihad-cross-toolchain-native"
RDEPENDS_${PN} = "goanna"

# --- cross env (mirrors build-daemon-arm.sh); exported to the task shells ---
export PKG_CONFIG = "pkg-config"
export PKG_CONFIG_SYSROOT_DIR = "${JIHAD_ARM_SYSROOT}"
export PKG_CONFIG_LIBDIR = "${JIHAD_ARM_SYSROOT}/usr/lib/arm-linux-gnueabi/pkgconfig:${JIHAD_ARM_SYSROOT}/usr/share/pkgconfig:${JIHAD_ARM_SYSROOT}/usr/lib/pkgconfig"
export PKG_CONFIG_PATH = "${PKG_CONFIG_LIBDIR}"

# The staged goanna dist (goanna stages the whole tree under datadir → this recipe's sysroot).
GOANNA = "${STAGING_DATADIR}/jihad-engine-dist"
GOANNA_INC = "${GOANNA}/include"
GOANNA_LIB = "${GOANNA}/bin"
GOANNA_A = "${GOANNA}/sdk/lib"

do_compile() {
    CXX="${JIHAD_TC}/bin/arm-webos-linux-gnueabi-g++"
    export PATH="${JIHAD_TC}/bin:${PATH}"
    R="${S}/render"
    BS="${S}/render/browserserver"
    O="${B}"

    ARMFLAGS="-march=armv7-a -mfpu=neon -mfloat-abi=softfp"
    SYSINC="-I${JIHAD_ARM_SYSROOT}/usr/include -I${JIHAD_ARM_SYSROOT}/usr/include/arm-linux-gnueabi"
    SYSLIB="-L${JIHAD_ARM_SYSROOT}/usr/lib/arm-linux-gnueabi -Wl,-rpath-link,${JIHAD_ARM_SYSROOT}/usr/lib/arm-linux-gnueabi"
    # JIHAD_OFFSCREEN_ONLY: GTK-free device daemon (matches the headless libxul).
    CXXFLAGS="-std=gnu++17 -fPIC -fno-rtti -fno-exceptions -Os -g0 -DJIHAD_OFFSCREEN_ONLY ${ARMFLAGS} ${SYSINC}"
    GLIB="$(pkg-config --cflags glib-2.0 gthread-2.0)";  GLIBL="$(pkg-config --libs glib-2.0 gthread-2.0)"
    ENGINC="-include ${GOANNA_INC}/mozilla-config.h -I${GOANNA_INC} -I${GOANNA_INC}/nspr"
    YAPINC="-I${BS}/Yap -I${BS}/Src"

    echo "== [ARM] libYap + BrowserServerBase =="
    for f in YapPacket YapProxy YapServer; do
        $CXX $CXXFLAGS $YAPINC $GLIB -c "${BS}/Yap/$f.cpp" -o "${O}/arm-$f.o"
    done
    $CXX $CXXFLAGS $YAPINC $GLIB -c "${BS}/Src/BrowserServerBase.cpp" -o "${O}/arm-BrowserServerBase.o"

    echo "== [ARM] Goanna backend =="
    for f in EngineHost DialogService DownloadService GoannaRenderPage BrowserPageGoanna; do
        $CXX $CXXFLAGS $ENGINC $YAPINC $GLIB -c "${R}/goanna/$f.cpp" -o "${O}/arm-$f.o"
    done

    echo "== [ARM] JihadBrowserServer + Main =="
    $CXX $CXXFLAGS $YAPINC $GLIB -c "${BS}/JihadBrowserServer.cpp" -o "${O}/arm-JihadBrowserServer.o"
    $CXX $CXXFLAGS $YAPINC $ENGINC $GLIB -c "${BS}/Main.cpp" -o "${O}/arm-bs_main.o"

    echo "== [ARM] linking jihad-browserserver =="
    $CXX $ARMFLAGS \
        "${O}/arm-bs_main.o" "${O}/arm-JihadBrowserServer.o" "${O}/arm-BrowserServerBase.o" \
        "${O}/arm-YapPacket.o" "${O}/arm-YapProxy.o" "${O}/arm-YapServer.o" \
        "${O}/arm-BrowserPageGoanna.o" "${O}/arm-GoannaRenderPage.o" "${O}/arm-EngineHost.o" \
        "${O}/arm-DialogService.o" "${O}/arm-DownloadService.o" \
        "${GOANNA_A}/libxpcomglue_s.a" -L"${GOANNA_LIB}" -Wl,-rpath-link,"${GOANNA_LIB}" \
        -lxul "${GOANNA_A}/libmozglue.a" -lnspr4 -lplc4 -lplds4 \
        ${SYSLIB} ${GLIBL} -ldl -lpthread \
        -o "${O}/jihad-browserserver"
    ${JIHAD_TC}/bin/arm-webos-linux-gnueabi-strip "${O}/jihad-browserserver"
}

do_install() {
    # Stage the daemon binary under datadir for the jihad-deviceroot bundler (which assembles the
    # self-contained /media/internal/jihad/hl set: daemon + libxul .so closure + bundled glibc-2.23).
    install -d ${D}${datadir}/jihad-parts
    install -m 0755 ${B}/jihad-browserserver ${D}${datadir}/jihad-parts/jihad-browserserver
}

FILES_${PN} = "${datadir}/jihad-parts"
# Links the offscreen headless libxul; ARM binary staged for the deviceroot, not a device pkg itself.
INSANE_SKIP_${PN} = "already-stripped ldflags arch"

# Build-INPUT recipe: stages the daemon for the deviceroot; emits no .ipk of its own.
do_package[noexec] = "1"
do_packagedata[noexec] = "1"
do_package_qa[noexec] = "1"
do_package_write_ipk[noexec] = "1"