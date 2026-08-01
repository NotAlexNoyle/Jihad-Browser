# Jihad Browser — standalone Goanna browser, Enyo 1.0 UI variant (1 of 3 independent packages).
#
# A single installable package that bundles the whole stack: the Enyo 1.0 front-end + its own
# copy of the runtime bundle (Goanna engine + daemon + bundled glibc-2.23, from jihad-deviceroot)
# + its own NPAPI shim, adapter impl and upstart job, with a postinst that deploys exactly those
# and a prerm that removes exactly those. Coexists with the stock webOS browser AND with the
# Mochi/Mojo variants — no file is shared with either (cavekit-device-build.md R7). See
# jihad-app.inc; every name comes from ../jihad-variants.inc.

SUMMARY = "Jihad Browser (standalone Goanna browser, Enyo 1.0 UI)"
LICENSE = "Apache-2.0 & MPL-2.0"

# The ONE identity fact this recipe states; app id, shim, upstart job, YAP name are all derived.
JIHAD_VARIANT = "enyo"

SRC_URI = "file://app/"
S = "${WORKDIR}/app"

require jihad-app.inc
