# Jihad Browser — coexisting NPAPI adapter (BrowserAdapterJihad.so) for webOS 3.
# The self-contained architecture's adapter: MIME application/x-jihad-browser,
# YAP client name "jihad-browser" (socket /tmp/yapserver.jihad-browser). Installs
# ALONGSIDE the stock BrowserAdapter.so (application/x-palm-browser) — nothing
# stock is replaced. See docs/DEVICE-HANDOFF.md (2026-07-07) and
# jihad-self-contained-arch.md.
#
# STATUS: skeleton. The working build is build/webos-oe/build-adapter-pdk.sh
# (PDK gcc 4.x against ref-BrowserAdapter/ with the two Jihad edits: MIME in
# AdapterGetMIMEDescription + BrowserClientBase("jihad-browser", ctxt)); this
# recipe packages that artifact for the Full-OE path. Gated on device-build R3.

SUMMARY = "Jihad Browser coexisting NPAPI adapter (BrowserAdapterJihad.so)"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=<fill>"

# Model-agnostic: one ARMv7 softfp .so for both TouchPad models (same SoC family,
# same 1024x768 buffer). Captured difference: none (device-build R6).
COMPATIBLE_MACHINE = "(tenderloin|opal)"

SRC_URI = "file://ref-BrowserAdapter/"
S = "${WORKDIR}/ref-BrowserAdapter"

DEPENDS = "glib-2.0"
# Talks YAP to the Jihad daemon; useless without it.
RDEPENDS:${PN} = "jihad-browserserver"

do_compile() {
    # Cross-build BrowserAdapter.cpp with the Jihad MIME/YAP-name edits →
    # BrowserAdapterJihad.so (same object recipe as build-adapter-pdk.sh).
    :
}

do_install() {
    # NPAPI plugins register from /usr/lib/BrowserPlugins at boot (full reboot
    # required — LunaSysMgr restart does NOT rescan).
    install -d ${D}${libdir}/BrowserPlugins
    install -m 0644 ${B}/BrowserAdapterJihad.so ${D}${libdir}/BrowserPlugins/
}

FILES:${PN} = "${libdir}/BrowserPlugins/BrowserAdapterJihad.so"
