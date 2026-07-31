# Jihad Browser — the self-contained on-device runtime bundle ("deviceroot"), shared by both
# UI product .ipks (Enyo + Mochi). It is assembled ONCE here and staged for the product recipes.
#
# Contents (mirrors the direct-build make-device-bundle.sh + packaging/):
#   hl/            the daemon + libxul's transitive .so closure + the BUNDLED glibc-2.23 loader
#                  (the daemon is gcc9/glibc-2.23; the device has glibc 2.8 — it MUST bring its own),
#                  the glibc NSS/resolver + NSS crypto modules (dlopen'd, not NEEDED), and the GRE
#                  resource files (goanna.js with the low-RAM prefs, omni.ja, …).
#   BrowserPlugins/BrowserAdapterJihad.so   the NPAPI shim (postinst → /usr/lib/BrowserPlugins).
#   event.d/jihad                           the upstart job (postinst → /etc/event.d).
# The product recipe drops this whole tree into the app's deviceroot/ and its postinst lays the
# pieces into place (see jihad-app.inc).

SUMMARY = "Jihad Browser self-contained on-device runtime bundle (engine + daemon + adapter shim)"
LICENSE = "Apache-2.0 & MPL-2.0"

require ../jihad-common.inc

COMPATIBLE_MACHINE = "(tenderloin|opal)"

# Consumes the staged artifacts of the three build recipes (all under ${datadir} in the sysroot).
DEPENDS = "goanna jihad-browserserver browser-adapter-jihad"

# It only assembles prebuilt ARM artifacts on the host — no OE cross-toolchain / sysroot needed.
INHIBIT_DEFAULT_DEPS = "1"
do_configure[noexec] = "1"

# review #8: do_compile runs the repo's bundler script and copies the repo's upstart job — both
# read from ${JIHAD_REPO}, outside SRC_URI, so declare them (plus the toolchain, whose sysroot
# supplies the bundled glibc-2.23 + NSS modules, and the Jessie sysroot the closure walk searches).
# The staged engine dist / daemon / adapter parts arrive through the DEPENDS task-hash chain.
# Mechanics + the JIHAD_*_SIG identity sets: ../jihad-common.inc.
do_compile[file-checksums] = "${JIHAD_REPO}/build/webos-oe/make-device-bundle.sh \
    ${JIHAD_REPO}/packaging/event.d/jihad \
    ${JIHAD_TC_SIG} ${JIHAD_SYS_SIG}"

do_compile() {
    DROOT=${B}/deviceroot
    rm -rf ${DROOT}; install -d ${DROOT}/hl ${DROOT}/BrowserPlugins ${DROOT}/event.d

    # hl/: daemon + libxul .so closure + bundled glibc-2.23 + NSS + GRE (the proven assembler,
    # pointed at the bitbake-staged engine dist + daemon binary + our crosstool-NG toolchain).
    DIST="${STAGING_DATADIR}/jihad-engine-dist" \
    DAEMON="${STAGING_DATADIR}/jihad-parts/jihad-browserserver" \
    SYS="${JIHAD_ARM_SYSROOT}" \
    JIHAD_TC="${JIHAD_TC}" \
    OUT="${DROOT}/hl" \
        bash ${JIHAD_REPO}/build/webos-oe/make-device-bundle.sh

    # The NPAPI shim (→ /usr/lib/BrowserPlugins at postinst) + the upstart job (→ /etc/event.d).
    install -m 0755 ${STAGING_DATADIR}/jihad-parts/BrowserAdapterJihad.so ${DROOT}/BrowserPlugins/BrowserAdapterJihad.so
    install -m 0644 ${JIHAD_REPO}/packaging/event.d/jihad ${DROOT}/event.d/jihad
}

do_install() {
    # Stage the whole assembled deviceroot for the product recipes (datadir → sysroot).
    install -d ${D}${datadir}/jihad-deviceroot
    cp -a ${B}/deviceroot/. ${D}${datadir}/jihad-deviceroot/
    # The adapter impl the app bundle carries (dlopen'd by the shim from the app dir).
    install -m 0755 ${STAGING_DATADIR}/jihad-parts/BrowserAdapterImpl.so ${D}${datadir}/jihad-deviceroot/BrowserAdapterImpl.so
}

FILES_${PN} = "${datadir}/jihad-deviceroot"
INSANE_SKIP_${PN} = "already-stripped arch ldflags libdir textrel"

# Build-INPUT recipe: it stages the bundle to the sysroot for the product .ipks and emits NO .ipk
# of its own (only the 2 UI products + Mojo skeleton are packages). populate_sysroot still runs.
do_package[noexec] = "1"
do_packagedata[noexec] = "1"
do_package_qa[noexec] = "1"
do_package_write_ipk[noexec] = "1"
