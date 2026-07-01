# Jihad Browser — render daemon (BrowserServer + Goanna backend) for webOS 3.
# Mirrors meta-webos/recipes-webos/browserserver but DEPENDS on goanna instead
# of webkit-webos + qt4-webos. The YAP IPC contract is byte-identical to isis.
#
# STATUS: skeleton. Builds render/browserserver + render/goanna against the
# cross goanna recipe. Gated on device-build R1/R2. See ../../../docs/DEVICE-BUILD.md.

SUMMARY = "Jihad Browser render daemon (Goanna-backed BrowserServer)"
LICENSE = "Apache-2.0 & MPL-2.0"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=<fill>"

# The daemon sources live in this repo (not fetched).
SRC_URI = "file://render/ file://build/desktop/"
S = "${WORKDIR}"

DEPENDS = "goanna glib-2.0 gtk+ luna-service2"
RDEPENDS:${PN} = "goanna"

# On device the LunaService surface (clearCache/clearCookies) is compiled IN
# (unlike the desktop PoC where it is compiled out) — IPC-contract R4.
EXTRA_OECMAKE = "-DJIHAD_ENABLE_LUNASERVICE=ON"

do_compile() {
    # Compile the engine-agnostic IPC layer (Yap/, BrowserServerBase) + the
    # Goanna backend (JihadBrowserServer, BrowserPageGoanna, GoannaRenderPage,
    # EngineHost, DialogService, DownloadService) + Main.cpp, linking libxul.
    # (Same object set as build/desktop/build-daemon.sh, cross-targeted.)
    :
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/jihad-browserserver ${D}${bindir}/
    # LunaService role/permission files for palm://com.palm.browserServer/*.
    install -d ${D}${sysconfdir}/palm/services
}

FILES:${PN} += "${bindir}/jihad-browserserver ${sysconfdir}/palm/services"
