# Jihad Browser — render daemon (BrowserServer + Goanna backend) for webOS 3.
# Mirrors meta-webos/recipes-webos/browserserver but DEPENDS on goanna instead
# of webkit-webos + qt4-webos. The YAP IPC contract is byte-identical to isis.
#
# STATUS: skeleton. Builds render/browserserver + render/goanna against the
# cross goanna recipe. Gated on device-build R1/R2. See ../../../docs/DEVICE-BUILD.md.

SUMMARY = "Jihad Browser render daemon (Goanna-backed BrowserServer)"
LICENSE = "Apache-2.0 & MPL-2.0"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=<fill>"

# Model-agnostic: the daemon is one ARMv7 softfp binary (build-daemon-arm.sh,
# -mfloat-abi=softfp) valid on BOTH TouchPad models — tenderloin and opal share the
# APQ8060 SoC + 1024x768 render buffer, so the shmem/offscreen contract is identical.
# Captured difference: none — shared ARMv7 softfp binary (device-build R6).
COMPATIBLE_MACHINE = "(tenderloin|opal)"

# The daemon sources live in this repo (not fetched).
SRC_URI = "file://render/ file://build/desktop/ file://packaging/"
S = "${WORKDIR}"

DEPENDS = "goanna glib-2.0 gtk+ luna-service2"
RDEPENDS_${PN} = "goanna"

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
    # Upstart job: without it nothing starts the daemon and the adapter's
    # /tmp/yapserver.jihad-browser socket never exists (codex F-389). Same job
    # the working packaging/postinst deploys.
    install -d ${D}${sysconfdir}/event.d
    install -m 0644 ${WORKDIR}/packaging/event.d/jihad ${D}${sysconfdir}/event.d/jihad
}

FILES_${PN} += "${bindir}/jihad-browserserver ${sysconfdir}/palm/services ${sysconfdir}/event.d/jihad"
