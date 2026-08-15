# Jihad Browser — coexisting NPAPI adapter (BrowserAdapterJihad.so) for webOS 3.
# The self-contained architecture's adapter: MIME application/x-jihad-browser,
# YAP client name "jihad-browser" (socket /tmp/yapserver.jihad-browser). Installs
# ALONGSIDE the stock BrowserAdapter.so (application/x-palm-browser) — nothing
# stock is replaced. See docs/DEVICE-HANDOFF.md (2026-07-07) and
# jihad-self-contained-arch.md.
#
# STATUS (re-read 2026-08-10, T-115): the "NON-RUNNABLE skeleton, do_compile is a stub" header
# that stood here was STALE and actively harmful — it told the reader to skip a recipe that had
# since been made real. Each of the four gaps it listed is contradicted by this file's own body:
#   - LIC_FILES_CHKSUM now comes from ../jihad-common.inc (repo LICENSE + real md5).
#   - SRC_URI is file://render/adapter/, which resolves via that file's FILESEXTRAPATHS at
#     ${JIHAD_REPO} — no sibling checkout, nothing to mirror.
#   - do_compile shells out to the proven build-adapter-pdk.sh.
#   - do_install stages per-variant from adapter-deps/build-pdk/, not ${B}/BrowserAdapterJihad.so.
# CODE READ ONLY — "the body is correct" is NOT "the task succeeds". The last recorded bitbake run
# is 2026-07-29 (cavekit-device-build.md R3) and it predates the 2026-07-31 per-variant split
# (e36c8cc), so the do_install loop below has never been executed by bitbake.
#   - Two-piece adapter (unchanged and still true): this shim (/usr/lib/BrowserPlugins) dlopens
#     BrowserAdapterImpl.so FROM THE APP BUNDLE per card open — the Impl ships inside the UI .ipk
#     (see jihad-ui recipes), not here.

SUMMARY = "Jihad Browser coexisting NPAPI adapter (BrowserAdapterJihad.so)"
LICENSE = "Apache-2.0"

require ../jihad-common.inc
require ../jihad-variants.inc

# Model-agnostic: one ARMv7 softfp .so for both TouchPad models (same SoC family,
# same 1024x768 buffer). Captured difference: none (device-build R6).
COMPATIBLE_MACHINE = "(tenderloin|opal)"

# Built by the PDK (gcc 4.3.3), NOT the crosstool-NG toolchain — matching the stock adapter's
# ABI avoids the gcc9 static-libstdc++ collision that crashes LunaSysMgr when the plugin is
# dlopen'd into the gcc4 WebKit host. Sources + device libs are all vendored (render/adapter/ +
# adapter-deps/ + the fetched PDK), so no OE DEPENDS. Talks YAP to the Jihad daemon at runtime.
SRC_URI = "file://render/adapter/"
S = "${WORKDIR}/render/adapter"
DEPENDS = ""
RDEPENDS_${PN} = "jihad-browserserver"

# No OE toolchain/sysroot here — the PDK build is fully self-contained.
INHIBIT_DEFAULT_DEPS = "1"
do_configure[noexec] = "1"

# review #8: do_compile shells out to build-adapter-pdk.sh, which compiles from the LIVE repo
# (${JIHAD_REPO}/render/...) — NOT from ${S}/${WORKDIR} — and links against the PDK + the
# vendored device libs. None of that reaches the task hash through SRC_URI (the file://render/adapter/
# entry only covers the unpacked copy the script does not use), so declare every real input here.
# See ../jihad-common.inc for the bitbake-1.18 mechanics and the JIHAD_*_SIG identity sets.
#   the script itself + render/adapter (adapter sources, exports map, shim)
#   render/browserserver/{Yap,Src} — the YAP wire code the script compiles straight out of the repo
#   ${JIHAD_PDK_SIG}   the PDK gcc 4.3.3 drivers          ${JIHAD_DEPS_SIG} the vendored device libs/headers
#   ${JIHAD_SYS_SIG}   the Jessie sysroot's glib (headers + libs are found via its pkg-config set)
# do_install is deliberately NOT given file-checksums: the files it installs
# (adapter-deps/build-pdk/*.so) are do_compile's OUTPUTS, and task hashes are computed before the
# build runs — hashing an output would bake in the previous run's artifact.
do_compile[file-checksums] = "${JIHAD_REPO}/build/webos-oe/build-adapter-pdk.sh \
    ${JIHAD_REPO}/render/adapter \
    ${JIHAD_REPO}/render/browserserver/Yap \
    ${JIHAD_REPO}/render/browserserver/Src \
    ${JIHAD_PDK_SIG} ${JIHAD_DEPS_SIG} ${JIHAD_SYS_SIG}"

do_compile() {
    # The two-piece adapter (stable NPAPI shim BrowserAdapterJihad.so + logic BrowserAdapterImpl.so)
    # is built by the proven build-adapter-pdk.sh: PDK gcc 4.3.3 (build/webos-oe/pdk/) against the
    # vendored adapter-deps device libs (Qt4/pbnjson/yajl/npapi). It derives its own paths and, in
    # the chroot, sees the repo at ${JIHAD_REPO}; outputs land in adapter-deps/build-pdk/ and the
    # impl is staged into app/ (bundled by the UI .ipk).
    bash ${JIHAD_REPO}/build/webos-oe/build-adapter-pdk.sh
}

do_install() {
    # Stage the adapter PER VARIANT (R7): build-adapter-pdk.sh builds the shim + impl once per
    # variant with that variant's MIME / YAP name / impl path compiled in, emitting the shim flat
    # as build-pdk/<shim>.so (the names are already unique) and the impl as
    # build-pdk/<variant>/BrowserAdapterImpl.so. Regroup both under jihad-parts/<variant>/ so the
    # deviceroot and the product .ipk can copy exactly one variant's slice and cannot pick up a
    # sibling's adapter — the shape review finding #5 (one shared impl for all variants) required.
    install -d ${D}${datadir}/jihad-parts
    PDK=${JIHAD_REPO}/build/webos-oe/adapter-deps/build-pdk
    for row in ${JIHAD_VARIANT_TABLE}; do
        V=$(echo "$row" | cut -d: -f1)
        SHIM=$(echo "$row" | cut -d: -f3)
        if [ -f "$PDK/$SHIM" ] && [ -f "$PDK/$V/BrowserAdapterImpl.so" ]; then
            install -d ${D}${datadir}/jihad-parts/$V
            install -m 0755 "$PDK/$SHIM" ${D}${datadir}/jihad-parts/$V/$SHIM
            install -m 0755 "$PDK/$V/BrowserAdapterImpl.so" ${D}${datadir}/jihad-parts/$V/BrowserAdapterImpl.so
        else
            bbwarn "browser-adapter-jihad: variant '$V' not built (missing $PDK/$SHIM or $PDK/$V/BrowserAdapterImpl.so)"
        fi
    done
}

FILES_${PN} = "${datadir}/jihad-parts"
INSANE_SKIP_${PN} = "already-stripped ldflags arch"

# Build-INPUT recipe: stages the shim + impl for the deviceroot/products; emits no .ipk of its own.
do_package[noexec] = "1"
do_packagedata[noexec] = "1"
do_package_qa[noexec] = "1"
do_package_write_ipk[noexec] = "1"
