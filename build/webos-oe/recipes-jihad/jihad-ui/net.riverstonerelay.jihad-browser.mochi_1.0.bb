# Jihad Browser — self-contained Goanna browser, Enyo 2 + Mochi UI variant (.ipk #2 of 2).
#
# Same self-contained stack as the Enyo variant (shared jihad-deviceroot runtime bundle), with the
# Enyo 2 + Mochi front-end and its own app id so both variants coexist on-device. See jihad-app.inc.

SUMMARY = "Jihad Browser (self-contained Goanna browser, Enyo 2 + Mochi UI)"
LICENSE = "Apache-2.0 & MPL-2.0"

WEBOS_APP_ID = "net.riverstonerelay.jihad-browser.mochi"
SRC_URI = "file://app-mochi/"
S = "${WORKDIR}/app-mochi"

require jihad-app.inc
