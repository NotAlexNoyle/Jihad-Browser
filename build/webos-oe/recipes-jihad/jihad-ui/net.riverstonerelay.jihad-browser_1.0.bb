# Jihad Browser — Enyo 1.0 UI package (.ipk) for webOS 3. From app/.
# Mirrors meta-webos/recipes-webos/com.palm.app.browser. Device-build R3.
# The second UI variant is net.riverstonerelay.jihad-browser.mochi_1.0.bb (from app-mochi/).

SUMMARY = "Jihad Browser (Enyo UI)"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${S}/../LICENSE;md5=<fill>"
SECTION = "webos/apps"

SRC_URI = "file://app/"
S = "${WORKDIR}/app"

# webOS app package: bundles the Enyo 1.0 framework + the forked isis UI, and
# talks to jihad-browserserver over the unchanged callBrowserAdapter + Luna URIs.
inherit webos-app
# Arch-independence must be DECLARED — meta-webos defaults PACKAGE_ARCH to
# ${MACHINE_ARCH}, so without this the two MACHINEs would emit two identical
# but differently-arched packages. (webos_arch_indep is the meta-webos-native
# spelling; PACKAGE_ARCH = "all" is the layer-portable one.)
PACKAGE_ARCH = "all"
WEBOS_APP_ID = "net.riverstonerelay.jihad-browser"

# Model-agnostic package (PACKAGE_ARCH = "all" above): the Enyo UI is density-independent
# (scales to MACHINE_DPI at runtime), and both TouchPad models are 1024x768, so ONE .ipk
# installs on tenderloin AND opal. No COMPATIBLE_MACHINE restriction — build once, install
# on both (device-build R6). DPI capture lives in the machine confs, not here.
RDEPENDS:${PN} = "jihad-browserserver browser-adapter-jihad"

do_install() {
    install -d ${D}${webos_applicationsdir}/${WEBOS_APP_ID}
    cp -r ${S}/* ${D}${webos_applicationsdir}/${WEBOS_APP_ID}/
}

FILES:${PN} = "${webos_applicationsdir}/${WEBOS_APP_ID}"
