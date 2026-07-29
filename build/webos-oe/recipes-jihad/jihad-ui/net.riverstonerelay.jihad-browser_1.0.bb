# Jihad Browser — self-contained Goanna browser, Enyo 1.0 UI variant (.ipk #1 of 2).
#
# A single installable package that bundles the whole stack: the Enyo 1.0 front-end + the shared
# runtime bundle (Goanna engine + daemon + NPAPI adapter + bundled glibc-2.23, from jihad-deviceroot)
# + the adapter impl, with a postinst that deploys the daemon/shim/upstart on-device. Coexists with
# the stock webOS browser. See jihad-app.inc.

SUMMARY = "Jihad Browser (self-contained Goanna browser, Enyo 1.0 UI)"
LICENSE = "Apache-2.0 & MPL-2.0"

WEBOS_APP_ID = "net.riverstonerelay.jihad-browser"
SRC_URI = "file://app/"
S = "${WORKDIR}/app"

require jihad-app.inc
