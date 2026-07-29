# Shared logic for the SELF-CONTAINED Jihad Browser UI product .ipks (Enyo, Mochi, [Mojo]).
#
# Each variant .ipk installs a complete, coexisting Goanna browser: its own webOS app + the shared
# runtime bundle (engine + daemon + adapter + bundled glibc-2.23, from jihad-deviceroot) + the
# adapter impl, and a postinst that lays the daemon/shim/upstart into place on-device (the same
# additive, self-contained model as packaging/postinst — nothing stock is touched).
#
# A variant recipe sets: WEBOS_APP_ID and the app source (SRC_URI + S).

require ../jihad-common.inc
inherit allarch

# webOS user apps (installed via Preware/opkg) live under /media/cryptofs/apps; the postinst reads
# the bundle from the installed app's deviceroot/ there.
webos_applicationsdir = "/media/cryptofs/apps/usr/palm/applications"
APPINSTDIR = "${webos_applicationsdir}/${WEBOS_APP_ID}"

# Pull in the shared runtime bundle (engine+daemon+adapter+glibc, staged under datadir).
DEPENDS += "jihad-deviceroot"
SECTION = "webos/apps"

do_install() {
    install -d ${D}${APPINSTDIR}
    # the UI app (Enyo/Mochi/Mojo JS/CSS/images)
    cp -r ${S}/* ${D}${APPINSTDIR}/
    # the self-contained runtime bundle → app/deviceroot/{hl, BrowserPlugins, event.d}
    cp -a ${STAGING_DATADIR}/jihad-deviceroot ${D}${APPINSTDIR}/deviceroot
    # the adapter impl belongs at the APP ROOT (the shim dlopen's it from there), not in deviceroot
    if [ -e ${D}${APPINSTDIR}/deviceroot/BrowserAdapterImpl.so ]; then
        mv ${D}${APPINSTDIR}/deviceroot/BrowserAdapterImpl.so ${D}${APPINSTDIR}/BrowserAdapterImpl.so
    fi
    # License/attribution payload (review #12): the app bundles MPL-2.0 Goanna + Apache-2.0 parts,
    # so ship the composite LICENSE + NOTICE (+ the full texts) in each package, like the direct build.
    install -m 0644 ${JIHAD_REPO}/LICENSE ${D}${APPINSTDIR}/LICENSE
    install -m 0644 ${JIHAD_REPO}/NOTICE  ${D}${APPINSTDIR}/NOTICE
    if [ -d ${JIHAD_REPO}/licenses ]; then
        install -d ${D}${APPINSTDIR}/licenses
        install -m 0644 ${JIHAD_REPO}/licenses/*.txt ${D}${APPINSTDIR}/licenses/
    fi
}

FILES_${PN} = "${webos_applicationsdir}"
# The app carries the ARM engine/daemon/adapter — an "all"-arch webOS package legitimately holds
# ARM binaries; skip the arch/QA checks that would reject them.
INSANE_SKIP_${PN} = "arch already-stripped ldflags libdir textrel staticdev"

# ---- on-device install/removal (opkg runs these; guarded so a rootfs build never runs them) ----
pkg_postinst_${PN} () {
if [ -n "$D" ]; then exit 1; fi   # defer to first boot if installed into an image
set -e                            # review #6: a failed deploy must NOT report success
APP=/media/cryptofs/apps/usr/palm/applications/${WEBOS_APP_ID}
DR=$APP/deviceroot
# preflight: the bundle payload must be present before we touch the system.
[ -x "$DR/hl/jihad-browserserver" ] && [ -f "$DR/hl/ld-2.23.so" ] \
  && [ -f "$DR/BrowserPlugins/BrowserAdapterJihad.so" ] && [ -f "$DR/event.d/jihad" ] || {
    echo "jihad-postinst: bundle payload missing under $DR — aborting" >&2; exit 1; }
# 1. daemon + its bundled libs (incl. glibc-2.23) -> /media/internal/jihad/hl (fatal on failure).
mkdir -p /media/internal/jihad/hl
cp -a "$DR/hl/." /media/internal/jihad/hl/
chmod 755 /media/internal/jihad/hl/jihad-browserserver /media/internal/jihad/hl/ld-2.23.so
# 2. adapter shim + upstart job live on the read-only rootfs — one rw window (restore ro on exit).
trap 'mount -o remount,ro / 2>/dev/null || true' EXIT
mount -o remount,rw /
mkdir -p /usr/lib/BrowserPlugins
cp -a "$DR/BrowserPlugins/BrowserAdapterJihad.so" /usr/lib/BrowserPlugins/BrowserAdapterJihad.so
chmod 755 /usr/lib/BrowserPlugins/BrowserAdapterJihad.so
cp -a "$DR/event.d/jihad" /etc/event.d/jihad
chmod 755 /etc/event.d/jihad
sync
mount -o remount,ro / 2>/dev/null || true
trap - EXIT
# verify the critical files actually landed (BusyBox cp can fail with EBADF across mounts).
[ -x /media/internal/jihad/hl/jihad-browserserver ] \
  && [ -f /usr/lib/BrowserPlugins/BrowserAdapterJihad.so ] && [ -f /etc/event.d/jihad ] || {
    echo "jihad-postinst: deploy verification failed" >&2; exit 1; }
# 3. optional runtime kicks — non-fatal (a fresh boot picks them up anyway).
set +e
start jihad 2>/dev/null || true
killall LunaSysMgr 2>/dev/null || true
true
}

pkg_prerm_${PN} () {
if [ -n "$D" ]; then exit 1; fi
APP=/media/cryptofs/apps/usr/palm/applications/${WEBOS_APP_ID}
# Coexistence refcount (review #4): the daemon (/media/internal/jihad/hl), the system shim
# (/usr/lib/BrowserPlugins/BrowserAdapterJihad.so) and the upstart job (/etc/event.d/jihad) are
# SHARED by all Jihad UI variants. Only tear them down when THIS is the LAST variant installed;
# otherwise leave them for the survivor(s) (the package manager still removes this app's own dir).
others=0
for d in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser*; do
  [ -d "$d" ] || continue
  [ "$d" = "$APP" ] && continue
  others=$((others + 1))
done
if [ "$others" -gt 0 ]; then
  echo "jihad-prerm: $others sibling variant(s) remain — keeping the shared runtime" >&2
  exit 0
fi
stop jihad 2>/dev/null || true
killall jihad-browserserver 2>/dev/null || true
rm -f /tmp/yapserver.jihad-browser*
mount -o remount,rw / 2>/dev/null || true
rm -f /usr/lib/BrowserPlugins/BrowserAdapterJihad.so
rm -f /etc/event.d/jihad
sync
mount -o remount,ro / 2>/dev/null || true
rm -rf /media/internal/jihad/hl
killall LunaSysMgr 2>/dev/null || true
true
}
