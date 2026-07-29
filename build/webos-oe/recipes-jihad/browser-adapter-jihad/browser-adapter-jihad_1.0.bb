# Jihad Browser — coexisting NPAPI adapter (BrowserAdapterJihad.so) for webOS 3.
# The self-contained architecture's adapter: MIME application/x-jihad-browser,
# YAP client name "jihad-browser" (socket /tmp/yapserver.jihad-browser). Installs
# ALONGSIDE the stock BrowserAdapter.so (application/x-palm-browser) — nothing
# stock is replaced. See docs/DEVICE-HANDOFF.md (2026-07-07) and
# jihad-self-contained-arch.md.
#
# STATUS: NON-RUNNABLE skeleton — packaging documentation of record, NOT a
# buildable recipe (codex F-385..F-388). The working build is
# build/webos-oe/build-adapter-pdk.sh (PDK gcc 4.x against ref-BrowserAdapter/
# with the two Jihad edits: MIME in AdapterGetMIMEDescription +
# BrowserClientBase("jihad-browser", ctxt)). Known gaps to make this real:
#   - LIC_FILES_CHKSUM is a placeholder and ref-BrowserAdapter carries no
#     LICENSE file (use the repo LICENSE + real md5).
#   - SRC_URI points at a sibling checkout outside FILESPATH; the source would
#     need mirroring into the layer (or a git:// fetch).
#   - do_compile is a stub; do_install would fail wanting ${B}/BrowserAdapterJihad.so.
#   - Two-piece adapter: this shim (/usr/lib/BrowserPlugins) dlopens
#     BrowserAdapterImpl.so FROM THE APP BUNDLE per card open — the Impl ships
#     inside the UI .ipk (see jihad-ui recipes), not here.

SUMMARY = "Jihad Browser coexisting NPAPI adapter (BrowserAdapterJihad.so)"
LICENSE = "Apache-2.0"

require ../jihad-common.inc

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

do_compile() {
    # The two-piece adapter (stable NPAPI shim BrowserAdapterJihad.so + logic BrowserAdapterImpl.so)
    # is built by the proven build-adapter-pdk.sh: PDK gcc 4.3.3 (build/webos-oe/pdk/) against the
    # vendored adapter-deps device libs (Qt4/pbnjson/yajl/npapi). It derives its own paths and, in
    # the chroot, sees the repo at ${JIHAD_REPO}; outputs land in adapter-deps/build-pdk/ and the
    # impl is staged into app/ (bundled by the UI .ipk).
    bash ${JIHAD_REPO}/build/webos-oe/build-adapter-pdk.sh
}

do_install() {
    # Stage both pieces under datadir for the product .ipk: the deviceroot puts the shim under
    # deviceroot/BrowserPlugins (→ /usr/lib/BrowserPlugins at postinst) and the impl in the app dir
    # (dlopen'd by the shim from there).
    install -d ${D}${datadir}/jihad-parts
    install -m 0755 ${JIHAD_REPO}/build/webos-oe/adapter-deps/build-pdk/BrowserAdapterJihad.so \
        ${D}${datadir}/jihad-parts/BrowserAdapterJihad.so
    install -m 0755 ${JIHAD_REPO}/build/webos-oe/adapter-deps/build-pdk/BrowserAdapterImpl.so \
        ${D}${datadir}/jihad-parts/BrowserAdapterImpl.so
}

FILES_${PN} = "${datadir}/jihad-parts"
INSANE_SKIP_${PN} = "already-stripped ldflags arch"

# Build-INPUT recipe: stages the shim + impl for the deviceroot/products; emits no .ipk of its own.
do_package[noexec] = "1"
do_packagedata[noexec] = "1"
do_package_qa[noexec] = "1"
do_package_write_ipk[noexec] = "1"
