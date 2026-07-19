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
# Arch-independence must be DECLARED — meta-webos defaults PACKAGE_ARCH to
# ${MACHINE_ARCH}, so without this the two MACHINEs would emit two identical
# but differently-arched packages. (webos_arch_indep is the meta-webos-native
# spelling; PACKAGE_ARCH = "all" is the layer-portable one.)
PACKAGE_ARCH = "all"
WEBOS_APP_ID = "net.riverstonerelay.jihad-browser.mochi"

# Model-agnostic package (PACKAGE_ARCH = "all" above): Enyo 2/Mochi scales to MACHINE_DPI at
# runtime; both TouchPad models are 1024x768, so ONE .ipk installs on tenderloin AND opal
# (device-build R6). No COMPATIBLE_MACHINE restriction — build once, install on both.
# The Mochi package bundles Enyo 2 + layout + Mochi (vs Enyo 1.0 for the other).
RDEPENDS_${PN} = "jihad-browserserver browser-adapter-jihad"

# GAP (codex F-388): the adapter is two-piece — the /usr/lib/BrowserPlugins shim
# dlopens BrowserAdapterImpl.so from THIS app's install dir per card open, so
# the UI package must also carry BrowserAdapterImpl.so (built by
# build-adapter-pdk.sh; a git-ignored binary, hence absent from SRC_URI here).
# The working deployment pushes it via the app .ipk; this skeleton documents it.

do_install() {
    install -d ${D}${webos_applicationsdir}/${WEBOS_APP_ID}
    cp -r ${S}/* ${D}${webos_applicationsdir}/${WEBOS_APP_ID}/
}

FILES_${PN} = "${webos_applicationsdir}/${WEBOS_APP_ID}"
