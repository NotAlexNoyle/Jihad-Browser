# Jihad Browser — Enyo 1.0 UI package (.ipk) for webOS 3. From app/.
# Mirrors meta-webos/recipes-webos/com.palm.app.browser. Device-build R3.
# The second UI variant is net.riverstonerelay.jihad.mochi_1.0.bb (from app-mochi/).

SUMMARY = "Jihad Browser (Enyo UI)"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/../LICENSE;md5=<fill>"
SECTION = "webos/apps"

SRC_URI = "file://app/"
S = "${WORKDIR}/app"

# webOS app package: bundles the Enyo 1.0 framework + the forked isis UI, and
# talks to jihad-browserserver over the unchanged callBrowserAdapter + Luna URIs.
inherit webos-app
WEBOS_APP_ID = "net.riverstonerelay.jihad"

RDEPENDS:${PN} = "jihad-browserserver browser-adapter"

do_install() {
    install -d ${D}${webos_applicationsdir}/${WEBOS_APP_ID}
    cp -r ${S}/* ${D}${webos_applicationsdir}/${WEBOS_APP_ID}/
}

FILES:${PN} = "${webos_applicationsdir}/${WEBOS_APP_ID}"
