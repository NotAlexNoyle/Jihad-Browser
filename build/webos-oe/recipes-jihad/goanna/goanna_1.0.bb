# Jihad Browser — Goanna/UXP engine for webOS 3 ARMv7 (OpenEmbedded recipe).
# The heavy recipe: cross-compiles UXP with the embedding config against the
# device sysroot. Replaces the isis webkit-webos + qt4-webos engine deps.
#
# STATUS (re-read 2026-08-10, T-115): "skeleton … gated on the device sysroot" was STALE. The body
# below is a full cross-build (mach configure/build under ${JIHAD_TC} against ${JIHAD_ARM_SYSROOT},
# staged as jihad-engine-dist), and both prerequisites it named exist: the crosstool-NG toolchain
# (R1, on-device verified) and the Jessie armel sysroot — both are declared prebuilt inputs, see
# R3's prebuilt-inputs carve-out. CODE READ ONLY: not bitbake-run since 2026-07-29.
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

# UXP source. Clones the jihad-engine-mods commit from the in-repo submodule (third_party/uxp,
# bound in the chroot). That commit is pristine UXP b2594a4 (upstream master) + the full jihad
# engine delta captured as one durable commit — it IS the exact tree the desktop host build
# (build-goanna-arm.sh) compiles, so the OE clone builds identical source. nobranch=1: SRCREV
# alone pins the checkout (the commit lives on the jihad-engine-mods branch, not master, and a
# file:// mirror need not track a branch). The patch queue (do_apply_jihad_patches) is already
# baked into this commit, so it now dry-run-gate skips as a redundant safety net.
# For a fully-upstream base instead, clone pristine b2594a4 from palemoon (commented) and let the
# patch queue rebuild the delta — but the queue drifted from pristine UXP, so that path is stale:
#   SRC_URI = "git://repo.palemoon.org/MoonchildProductions/UXP.git;protocol=https;branch=master"
SRC_URI = "git://${JIHAD_REPO}/third_party/uxp;protocol=file;nobranch=1"
SRCREV = "07259a27f058fd042849bc4379bdfa886b338694"
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

# review #8: because those files are NOT in SRC_URI, nothing would invalidate sstate when they
# change — declare them in the signature of each task that reads them (mechanics + the
# JIHAD_*_SIG identity sets are documented in ../jihad-common.inc). The UXP tree itself IS a
# SRC_URI (git, SRCREV-pinned) so it is already covered by do_fetch.
#   apply_jihad_patches  the patch queue (glob: adding/removing/editing a patch moves the hash).
#   configure            the mozconfig template + the toolchain + the Jessie sysroot it retargets.
#   compile              the toolchain + sysroot used in place by mach.
#   install              the toolchain (its `strip` shrinks the staged libxul).
# NOTE: apply_jihad_patches re-runs on the EXISTING ${S} when the queue changes; `patch --forward`
# skips already-applied hunks, so the re-run is safe (a full re-unpack still needs cleansstate).
do_apply_jihad_patches[file-checksums] = "${JIHAD_PATCHES}/*.patch"
do_configure[file-checksums] = "${JIHAD_MOZCONFIG_TMPL} ${JIHAD_TC_SIG} ${JIHAD_SYS_SIG}"
do_compile[file-checksums] = "${JIHAD_TC_SIG} ${JIHAD_SYS_SIG}"
# Wipe + recreate the mozilla objdir before every configure so each build starts clean. A
# do_configure that failed AFTER `mach configure` (e.g. the do_qa_configure LIC check) leaves a
# populated objdir; on the next build `mach build`'s AUTOCLOBBER then chokes on it with
# `rm: cannot remove 'dist/bin': Directory not empty`. A fresh objdir means AUTOCLOBBER never has a
# stale tree to clobber. (The host dev-loop build-goanna-arm.sh keeps its objdir for incremental
# rebuilds; the OE recipe does a clean build, which is the OE norm anyway.) 2026-08-15, T-154.
do_configure[cleandirs] = "${MOZ_OBJDIR}"
do_install[file-checksums] = "${JIHAD_TC_SIG}"

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
# append shell to it) and before do_configure. The SRCREV commit already has the whole queue
# baked in, so every patch is already applied here; the dry-run gate (mirroring the host build's
# build-goanna-arm.sh loop) skips a patch that does not apply cleanly instead of half-applying its
# hunks with `|| true`. A stale/partially-drifted patch therefore leaves the baked-in source
# untouched rather than corrupting it. If SRCREV is ever swapped back to pristine b2594a4, this
# same loop applies the queue for real.
do_apply_jihad_patches() {
    cd ${S}
    for p in ${JIHAD_PATCHES}/*.patch; do
        if patch -p1 --forward --dry-run < "$p" >/dev/null 2>&1; then
            patch -p1 --forward < "$p" || true
        fi
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
# .ipk of its own (only the THREE UI products are packages — Mojo stopped being a skeleton on
# 2026-08-05, cavekit-mojo-ui.md). populate_sysroot still runs.
do_package[noexec] = "1"
do_packagedata[noexec] = "1"
do_package_qa[noexec] = "1"
do_package_write_ipk[noexec] = "1"

FILES_${PN} = "${datadir}/jihad-engine-dist"
INSANE_SKIP_${PN} = "already-stripped dev-so staticdev arch ldflags libdir"
