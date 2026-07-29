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
}

FILES_${PN} = "${webos_applicationsdir}"
# The app carries the ARM engine/daemon/adapter — an "all"-arch webOS package legitimately holds
# ARM binaries; skip the arch/QA checks that would reject them.
INSANE_SKIP_${PN} = "arch already-stripped ldflags libdir textrel staticdev"

# ---- on-device install/removal (opkg runs these; guarded so a rootfs build never runs them) ----
pkg_postinst_${PN} () {
if [ -n "$D" ]; then exit 1; fi   # defer to first boot if installed into an image
APP=/media/cryptofs/apps/usr/palm/applications/${WEBOS_APP_ID}
DR=$APP/deviceroot
# 1. daemon + its bundled libs (incl. glibc-2.23) -> /media/internal/jihad/hl
mkdir -p /media/internal/jihad/hl
cp -a "$DR/hl/." /media/internal/jihad/hl/
chmod 755 /media/internal/jihad/hl/jihad-browserserver /media/internal/jihad/hl/ld-2.23.so 2>/dev/null || true
# 2. adapter shim + upstart job live on the read-only rootfs — one rw window.
mount -o remount,rw / 2>/dev/null || true
mkdir -p /usr/lib/BrowserPlugins
cp -a "$DR/BrowserPlugins/BrowserAdapterJihad.so" /usr/lib/BrowserPlugins/BrowserAdapterJihad.so
chmod 755 /usr/lib/BrowserPlugins/BrowserAdapterJihad.so
cp -a "$DR/event.d/jihad" /etc/event.d/jihad
chmod 755 /etc/event.d/jihad
sync
mount -o remount,ro / 2>/dev/null || true
# 3. start the daemon + reload LunaSysMgr (registers the new NPAPI plugin at next scan/boot).
start jihad 2>/dev/null || true
killall LunaSysMgr 2>/dev/null || true
true
}

pkg_prerm_${PN} () {
if [ -n "$D" ]; then exit 1; fi
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
