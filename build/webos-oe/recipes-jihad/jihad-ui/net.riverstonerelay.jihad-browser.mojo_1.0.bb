# Jihad Browser — self-contained Goanna browser, MOJO UI variant. SKELETON for a future port.
#
# Third UI variant alongside Enyo 1.0 + Enyo 2/Mochi. It packages the SAME self-contained runtime
# bundle (engine + daemon + adapter + glibc-2.23, from jihad-deviceroot) via jihad-app.inc, so it
# installs and the engine works — only the Mojo UI is a stub (see app-mojo/README.md for the TODOs:
# a WebView with the application/x-jihad-browser plugin MIME + the browser chrome).
#
# It is NOT one of the two shipping variants; it builds on request (`bitbake
# net.riverstonerelay.jihad-browser.mojo`) as a starting point for the port.

SUMMARY = "Jihad Browser (self-contained Goanna browser, Mojo UI — SKELETON)"
LICENSE = "Apache-2.0 & MPL-2.0"

WEBOS_APP_ID = "net.riverstonerelay.jihad-browser.mojo"
SRC_URI = "file://app-mojo/"
S = "${WORKDIR}/app-mojo"

require jihad-app.inc

# Skeleton — keep it out of `bitbake world` so a broad build doesn't pull the stub in.
EXCLUDE_FROM_WORLD = "1"
