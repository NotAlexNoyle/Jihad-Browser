# Jihad Browser — Mochi UI package (.ipk) for webOS 3. From app-mochi/.
# The second front-end variant (device-build R3): bundles Enyo 2 + layout +
# Mochi instead of Enyo 1.0. Shares the same daemon + adapter as the Enyo variant
# and can coexist on-device (distinct app id).

SUMMARY = "Jihad Browser (Mochi UI)"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/../LICENSE;md5=<fill>"
SECTION = "webos/apps"

SRC_URI = "file://app-mochi/"
S = "${WORKDIR}/app-mochi"

inherit webos-app
WEBOS_APP_ID = "net.riverstonerelay.jihad-browser.mochi"

# The Mochi package bundles Enyo 2 + layout + Mochi (vs Enyo 1.0 for the other).
RDEPENDS:${PN} = "jihad-browserserver browser-adapter"

do_install() {
    install -d ${D}${webos_applicationsdir}/${WEBOS_APP_ID}
    cp -r ${S}/* ${D}${webos_applicationsdir}/${WEBOS_APP_ID}/
}

FILES:${PN} = "${webos_applicationsdir}/${WEBOS_APP_ID}"
