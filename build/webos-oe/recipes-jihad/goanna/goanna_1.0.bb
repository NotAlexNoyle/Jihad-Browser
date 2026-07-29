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

require ../jihad-common.inc

# Both TouchPad models (TouchPad = topaz/tenderloin, TouchPad Go = opal) share ONE
# APQ8060 ARMv7 softfp libxul — there is no per-model engine variance (device-build R6).
# The machine confs (../../conf/machine/{tenderloin,opal}.conf) both resolve to
# DEFAULTTUNE "armv7a-neon" (softfp), which is exactly the mozconfig.goanna-arm target,
# so the same libxul.so installs on both. Model differences are UI-side DPI only, not
# compiled into the engine. Captured difference: none — shared ARMv7 softfp binary.
COMPATIBLE_MACHINE = "(tenderloin|opal)"

# UXP source. Default: clone the pinned commit from the in-repo submodule (third_party/uxp,
# bound in the chroot) — fast, and it IS the pin, cloned pristine (working-tree dirt ignored).
# For a fully-upstream-reproducible build instead, swap to the palemoon URL (commented) — same
# SRCREV, just a large first fetch:
#   SRC_URI = "git://repo.palemoon.org/MoonchildProductions/UXP.git;protocol=https;branch=master"
SRC_URI = "git://${JIHAD_REPO}/third_party/uxp;protocol=file;branch=master"
SRCREV = "b2594a4ace4556b0a953c079a8c1bc350fc095ec"
S = "${WORKDIR}/git"

# Modern C++14 toolchain (device-build R1) — the stock dylan/device gcc cannot build UXP.
# GTK/X/cairo/pango/nspr/nss are NOT taken from OE — they come from the Jessie armel sysroot
# (${JIHAD_ARM_SYSROOT}) via PKG_CONFIG_SYSROOT_DIR, exactly as the direct cross-build does.
DEPENDS = "jihad-cross-toolchain-native"

# The Jihad engine mods (patch queue) + the ARM mozconfig are build-control files that live
# in the repo, not upstream sources — reference them directly from ${JIHAD_REPO} (bound in the
# chroot) rather than via SRC_URI, so there is nothing to fetch/checksum for them.
JIHAD_PATCHES = "${JIHAD_REPO}/build/desktop/patches"
JIHAD_MOZCONFIG_TMPL = "${JIHAD_REPO}/build/webos-oe/mozconfig.goanna-arm"

# The cross env for mach (mirrors build-goanna-arm.sh). Exported to every task shell. The
# toolchain + Jessie sysroot are used in place from the repo; GTK/X/nspr/nss come from the
# sysroot via pkg-config, NOT from OE. CC/CXX are exported so in-tree gyp probes (e.g. NSS
# freebl __SOFTFP__) use the cross compiler, not the host cc.
export CC = "${JIHAD_TC}/bin/arm-webos-linux-gnueabi-gcc"
export CXX = "${JIHAD_TC}/bin/arm-webos-linux-gnueabi-g++"
export AR = "${JIHAD_TC}/bin/arm-webos-linux-gnueabi-ar"
export RANLIB = "${JIHAD_TC}/bin/arm-webos-linux-gnueabi-ranlib"
export STRIP = "${JIHAD_TC}/bin/arm-webos-linux-gnueabi-strip"
# OE force-exports LD as "arm-webos-linux-gnueabi-ld --sysroot=<OE target sysroot>", which
# moz.configure's linker check then tries and fails ("Cannot find link"). Point LD at our
# toolchain's own ld (no OE sysroot); mach links via CC against the Jessie sysroot anyway.
export LD = "${JIHAD_TC}/bin/arm-webos-linux-gnueabi-ld"
export AS = "${JIHAD_TC}/bin/arm-webos-linux-gnueabi-as"
export NM = "${JIHAD_TC}/bin/arm-webos-linux-gnueabi-nm"
# UXP needs a HOST compiler >= GCC 9.1 for its host tools; the rootfs default is 4.8, so use
# the gcc-9/g++-9 installed from the toolchain PPA (OE natives still use 4.8).
export HOST_CC = "gcc-9"
export HOST_CXX = "g++-9"
export PKG_CONFIG = "pkg-config"
export PKG_CONFIG_SYSROOT_DIR = "${JIHAD_ARM_SYSROOT}"
export PKG_CONFIG_LIBDIR = "${JIHAD_ARM_SYSROOT}/usr/lib/arm-linux-gnueabi/pkgconfig:${JIHAD_ARM_SYSROOT}/usr/share/pkgconfig:${JIHAD_ARM_SYSROOT}/usr/lib/pkgconfig"
export PKG_CONFIG_PATH = "${PKG_CONFIG_LIBDIR}"
export MOZCONFIG = "${B}/mozconfig.goanna-arm"
export MOZ_OBJDIR = "${B}/obj-jihad-goanna-arm"
# This UXP pin builds under PYTHON 3 (mach re-execs under python3; config/pythonpath.py is py3
# syntax). The real trap is the LOCALE: OE forces LC_ALL=C in tasks, so python3's open().read()
# on the UTF-8 .idl files dies ("UnicodeDecodeError: 'ascii' codec"). Pin python3 + a UTF-8 locale.
export PYTHON = "python3"
export LANG = "en_US.UTF-8"
export LC_ALL = "en_US.UTF-8"

# Generate the OE mozconfig from the repo template, retargeting the direct-build's fixed mount
# points (/tc, /sysroot, /out) at the real in-chroot paths + this recipe's object dir.
jihad_gen_mozconfig() {
    sed -e "s#/tc/#${JIHAD_TC}/#g" \
        -e "s#/sysroot#${JIHAD_ARM_SYSROOT}#g" \
        -e "s#/out/obj-jihad-goanna-arm#${MOZ_OBJDIR}#g" \
        -e 's#HOST_CC="gcc"#HOST_CC="gcc-9"#' \
        -e 's#HOST_CXX="g++"#HOST_CXX="g++-9"#' \
        "${JIHAD_MOZCONFIG_TMPL}" > "${MOZCONFIG}"
}

# Apply the Jihad patch queue in its OWN task after do_patch (which is a python task — can't
# append shell to it) and before do_configure. Runs ONCE on the freshly-unpacked pristine tree,
# so it is not re-applied onto an already-patched tree the way a do_configure loop would be.
do_apply_jihad_patches() {
    cd ${S}
    for p in ${JIHAD_PATCHES}/*.patch; do
        patch -p1 --forward < "$p" || true
    done
}
addtask apply_jihad_patches after do_patch before do_configure

do_configure() {
    jihad_gen_mozconfig
    export PATH="${JIHAD_TC}/bin:${PATH}"
    cd ${S} && ./mach configure
}

do_compile() {
    export PATH="${JIHAD_TC}/bin:${PATH}"
    cd ${S} && ./mach build
}

do_install() {
    # Stage the WHOLE engine dist to ONE datadir location — the daemon links against it AND the
    # jihad-deviceroot bundler walks its .so closure + GRE files. Under datadir (not libdir)
    # because OE libdir sysroot-staging drops static .a and would split the dist; datadir
    # plain-staging keeps the tree intact. -L/-aL dereference the objdir symlinks (incl. RELATIVE
    # ones like xpcom-config.h) so nothing dangles once staged.
    #   bin/     libxul + NSS suite (libnss3/nssutil3/smime3/ssl3/softokn3/freebl*/nssckbi) +
    #            sqlite (libmozsqlite3) + nspr + liblgpllibs + libhunspell + GRE files
    #            (goanna.js, omni.ja, dependentlibs.list, platform.ini, icu*.dat)
    #   sdk/lib/ static glue (libxpcomglue_s.a, libmozglue.a)
    #   include/ frozen embedding headers
    ED=${D}${datadir}/jihad-engine-dist
    install -d ${ED}/bin ${ED}/sdk/lib ${ED}/include
    # ONLY the .so closure + GRE files from dist/bin — NOT the x86 host build tools (nsinstall,
    # xpcshell, …) which would break OE's ARM objcopy during do_package.
    cp -aL ${MOZ_OBJDIR}/dist/bin/*.so ${ED}/bin/
    for f in goanna.js omni.ja dependentlibs.list platform.ini greprefs.js; do
        [ -e ${MOZ_OBJDIR}/dist/bin/$f ] && cp -L ${MOZ_OBJDIR}/dist/bin/$f ${ED}/bin/ || true
    done
    for f in ${MOZ_OBJDIR}/dist/bin/*.dat; do [ -e "$f" ] && cp -L "$f" ${ED}/bin/ || true; done
    cp -aL ${MOZ_OBJDIR}/dist/sdk/lib/*.a ${ED}/sdk/lib/
    cp -rL ${MOZ_OBJDIR}/dist/include/. ${ED}/include/
    # Strip the ~1.1G debug libxul to device size (dynsym kept so the daemon still links).
    ${JIHAD_TC}/bin/arm-webos-linux-gnueabi-strip --strip-debug ${ED}/bin/libxul.so
}
# The engine libs are already built/stripped by us — don't let OE re-strip/split-debug them
# (it would run objcopy over the whole prebuilt dist).
INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"

# Build-INPUT recipe: stages the engine dist to the sysroot for the daemon + deviceroot; emits no
# .ipk of its own (only the 2 UI products + Mojo skeleton are packages). populate_sysroot still runs.
do_package[noexec] = "1"
do_packagedata[noexec] = "1"
do_package_qa[noexec] = "1"
do_package_write_ipk[noexec] = "1"

FILES_${PN} = "${datadir}/jihad-engine-dist"
INSANE_SKIP_${PN} = "already-stripped dev-so staticdev arch ldflags libdir"
