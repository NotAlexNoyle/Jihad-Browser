# Jihad Browser — standalone Goanna browser, MOJO UI variant (3 of 3 independent packages).
#
# Third UI variant alongside Enyo 1.0 + Enyo 2/Mochi. It packages its OWN copy of the runtime
# bundle (engine + daemon + glibc-2.23, from jihad-deviceroot) plus its OWN adapter shim, adapter
# impl, YAP service name, socket and upstart job via jihad-app.inc — nothing is shared with the
# Enyo or Mochi package (cavekit-device-build.md R7).
#
# The front-end itself is still being brought up (cavekit-mojo-ui.md / T-059): app-mojo/ is a
# scaffold, so this recipe stays out of `bitbake world` and builds on request (`bitbake
# net.riverstonerelay.jihad-browser.mojo`).

SUMMARY = "Jihad Browser (standalone Goanna browser, Mojo UI — front-end in progress)"
LICENSE = "Apache-2.0 & MPL-2.0"

# The ONE identity fact this recipe states; app id, shim, upstart job, YAP name are all derived.
JIHAD_VARIANT = "mojo"

SRC_URI = "file://app-mojo/"
S = "${WORKDIR}/app-mojo"

require jihad-app.inc

# Skeleton — keep it out of `bitbake world` so a broad build doesn't pull the stub in.
EXCLUDE_FROM_WORLD = "1"
