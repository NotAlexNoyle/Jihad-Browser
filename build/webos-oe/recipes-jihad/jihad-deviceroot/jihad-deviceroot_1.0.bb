# Jihad Browser — the on-device runtime bundle ("deviceroot") staged for the product .ipks.
#
# Layout it stages (jihad-app.inc picks ONE variant's slice per .ipk):
#   hl/                              variant-AGNOSTIC: the daemon + libxul's transitive .so
#                                    closure + the BUNDLED glibc-2.23 loader (the daemon is
#                                    gcc9/glibc-2.23, the device has glibc 2.8 — it MUST bring
#                                    its own), the glibc NSS/resolver + NSS crypto modules
#                                    (dlopen'd, not NEEDED), and the GRE resource files
#                                    (goanna.js with the low-RAM prefs, omni.ja, …).
#   variants/<V>/BrowserPlugins/<shim .so>      that variant's NPAPI shim   (postinst → /usr/lib/BrowserPlugins)
#   variants/<V>/BrowserAdapterImpl.so          that variant's adapter impl (postinst → /usr/lib/jihad/<V>/)
#   variants/<V>/event.d/<job>                  that variant's upstart job  (postinst → /etc/event.d/)
#
# WHY hl/ IS SHARED HERE BUT NOTHING IS SHARED ON THE DEVICE (R7): hl/ is a ~40 MB build
# artifact with no variant-specific content, so assembling it three times would triple the build
# for byte-identical output. Each product .ipk still gets its OWN COPY inside its own app
# directory, run in place from there — no file on the device is co-owned, which is what R7
# actually requires. The variant-specific parts (shim/impl/job) are per-variant here so that a
# .ipk physically cannot carry another variant's adapter.
#
# The engine and daemon are NOT copied anywhere at install time (R8): the product recipe drops
# this tree into the app's own deviceroot/ on cryptofs and the upstart job execs it in place.
# Nothing is written to /media/internal. See jihad-app.inc and packaging/README.md.

SUMMARY = "Jihad Browser on-device runtime bundle (engine + daemon + per-variant adapter/upstart)"
LICENSE = "Apache-2.0 & MPL-2.0"

require ../jihad-common.inc
require ../jihad-variants.inc

COMPATIBLE_MACHINE = "(tenderloin|opal)"

# Consumes the staged artifacts of the three build recipes (all under ${datadir} in the sysroot).
DEPENDS = "goanna jihad-browserserver browser-adapter-jihad"

# It only assembles prebuilt ARM artifacts on the host — no OE cross-toolchain / sysroot needed.
INHIBIT_DEFAULT_DEPS = "1"
do_configure[noexec] = "1"

# review #8: do_compile runs the repo's bundler script and copies the repo's upstart jobs — all
# read from ${JIHAD_REPO}, outside SRC_URI, so declare them (plus the toolchain, whose sysroot
# supplies the bundled glibc-2.23 + NSS modules, and the Jessie sysroot the closure walk
# searches). ALL THREE job files are listed: this task now reads every one of them, and an
# undeclared input is exactly the stale-sstate bug #8 was about. They are named individually
# rather than globbed so adding a fourth variant fails loudly here instead of silently escaping
# the signature. The staged engine dist / daemon / adapter parts arrive through the DEPENDS
# task-hash chain. Mechanics + the JIHAD_*_SIG identity sets: ../jihad-common.inc.
do_compile[file-checksums] = "${JIHAD_REPO}/build/webos-oe/make-device-bundle.sh \
    ${JIHAD_REPO}/packaging/event.d/jihad \
    ${JIHAD_REPO}/packaging/event.d/jihad-mochi \
    ${JIHAD_REPO}/packaging/event.d/jihad-mojo \
    ${JIHAD_TC_SIG} ${JIHAD_SYS_SIG}"

do_compile() {
    DROOT=${B}/deviceroot
    rm -rf ${DROOT}; install -d ${DROOT}/hl

    # hl/: daemon + libxul .so closure + bundled glibc-2.23 + NSS + GRE (the proven assembler,
    # pointed at the bitbake-staged engine dist + daemon binary + our crosstool-NG toolchain).
    DIST="${STAGING_DATADIR}/jihad-engine-dist" \
    DAEMON="${STAGING_DATADIR}/jihad-parts/jihad-browserserver" \
    SYS="${JIHAD_ARM_SYSROOT}" \
    JIHAD_TC="${JIHAD_TC}" \
    OUT="${DROOT}/hl" \
        bash ${JIHAD_REPO}/build/webos-oe/make-device-bundle.sh

    # Per-variant parts. Each variant gets its own subtree so a product .ipk can copy exactly
    # one slice and physically cannot pick up a sibling's adapter.
    PARTS=${STAGING_DATADIR}/jihad-parts
    for row in ${JIHAD_VARIANT_TABLE}; do
        V=$(echo "$row" | cut -d: -f1)
        SHIM=$(echo "$row" | cut -d: -f3)
        JOB=$(echo "$row" | cut -d: -f4)
        VD=${DROOT}/variants/$V
        install -d $VD/BrowserPlugins $VD/event.d

        # Adapter: prefer the per-variant staging layout, which browser-adapter-jihad produces
        # by building the adapter once per variant (its MIME, YAP name and impl path are
        # compile-time constants — T-055). The FLAT legacy layout is accepted ONLY for enyo,
        # whose identity IS the unsuffixed one; accepting it for mochi/mojo would hand them an
        # adapter registered for the ENYO MIME and pointing at the ENYO impl — review finding #5
        # all over again. A missing variant is warned about here and made FATAL in the product
        # recipe's do_install, so one un-built variant cannot fail the other two's builds.
        if [ -f "$PARTS/$V/$SHIM" ] && [ -f "$PARTS/$V/BrowserAdapterImpl.so" ]; then
            install -m 0755 "$PARTS/$V/$SHIM" "$VD/BrowserPlugins/$SHIM"
            install -m 0755 "$PARTS/$V/BrowserAdapterImpl.so" "$VD/BrowserAdapterImpl.so"
        elif [ "$V" = "enyo" ] && [ -f "$PARTS/$SHIM" ] && [ -f "$PARTS/BrowserAdapterImpl.so" ]; then
            install -m 0755 "$PARTS/$SHIM" "$VD/BrowserPlugins/$SHIM"
            install -m 0755 "$PARTS/BrowserAdapterImpl.so" "$VD/BrowserAdapterImpl.so"
        else
            bbwarn "jihad-deviceroot: no adapter staged for variant '$V' (looked for $PARTS/$V/$SHIM + $PARTS/$V/BrowserAdapterImpl.so); its .ipk will fail in do_install"
        fi

        # That variant's upstart job, straight from packaging/ (generated by
        # packaging/gen-variant-scripts.sh; declared in do_compile[file-checksums] above).
        install -m 0644 ${JIHAD_REPO}/packaging/event.d/$JOB "$VD/event.d/$JOB"
    done
}

do_install() {
    # Stage the whole assembled deviceroot for the product recipes (datadir → sysroot). The
    # adapter impls now live under variants/<V>/ (staged by do_compile), so there is no separate
    # variant-agnostic impl to install here any more.
    install -d ${D}${datadir}/jihad-deviceroot
    cp -a ${B}/deviceroot/. ${D}${datadir}/jihad-deviceroot/
}

FILES_${PN} = "${datadir}/jihad-deviceroot"
INSANE_SKIP_${PN} = "already-stripped arch ldflags libdir textrel"

# Build-INPUT recipe: it stages the bundle to the sysroot for the product .ipks and emits NO .ipk
# of its own (only the 3 UI products are packages). populate_sysroot still runs.
do_package[noexec] = "1"
do_packagedata[noexec] = "1"
do_package_qa[noexec] = "1"
do_package_write_ipk[noexec] = "1"
