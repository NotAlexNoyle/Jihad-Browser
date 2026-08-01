# Shared logic for the three INDEPENDENT Jihad Browser UI product .ipks (Enyo, Mochi, Mojo).
#
# Each variant .ipk is a complete, standalone Goanna browser: its own webOS app + its own copy of
# the runtime bundle (engine + daemon + bundled glibc-2.23, from jihad-deviceroot) + its OWN
# NPAPI shim, adapter impl and upstart job, plus a postinst that deploys exactly those and a
# prerm that removes exactly those. Installing, upgrading or removing one variant cannot touch
# another (cavekit-device-build.md R7), and the install footprint outside the app directory is
# bounded, variant-namespaced and exactly reversed (R8).
#
# This retires the 2026-07-29 shipping model: the shared /media/internal/jihad/hl runtime, the
# shared /usr/lib/jihad/BrowserAdapterImpl.so, the shared /etc/event.d/jihad, and the pkg_prerm
# sibling REFCOUNT those forced (review findings #4/#5). Nothing is co-owned, so nothing is
# counted. The engine now RUNS IN PLACE from the app's own cryptofs deviceroot — /media/internal
# is the user's vfat USB storage and is never written. That install model (run in place from the
# app's deviceroot; one remount-rw window for the plugin + upstart job; teardown that leaves
# /media/internal alone because nothing was written there) is adapted from the Atlas browser's
# atlas-browser-app/packaging/{ipk-postinst.sh,ipk-prerm.sh} (Apache-2.0, Herman van Hazendonk /
# Herrie82).
#
# A variant recipe sets exactly one identity fact — JIHAD_VARIANT — plus its app source
# (SRC_URI + S). Every name is derived from ../jihad-variants.inc.

require ../jihad-common.inc
require ../jihad-variants.inc
inherit allarch

# Per-variant identity, derived from the one table (see ../jihad-variants.inc for why).
WEBOS_APP_ID      = "${@jihad_variant_field(d, 1)}"
JIHAD_SHIM_SO     = "${@jihad_variant_field(d, 2)}"
JIHAD_UPSTART_JOB = "${@jihad_variant_field(d, 3)}"
JIHAD_YAP_NAME    = "${@jihad_variant_field(d, 4)}"

# webOS user apps (installed via Preware/opkg) live under /media/cryptofs/apps; the postinst reads
# the bundle from the installed app's deviceroot/ there.
webos_applicationsdir = "/media/cryptofs/apps/usr/palm/applications"
APPINSTDIR = "${webos_applicationsdir}/${WEBOS_APP_ID}"

# Pull in the runtime bundle (engine+daemon+glibc + the per-variant adapter/upstart parts).
DEPENDS += "jihad-deviceroot"
SECTION = "webos/apps"

# Parse-time guards for R7's "no two variants resolve to the same X". Cheap, and they fail the
# BUILD rather than letting a duplicated row ship two packages that fight over one file.
python () {
    pn = d.getVar('PN', True)
    appid = d.getVar('WEBOS_APP_ID', True)
    if pn != appid:
        bb.fatal("recipe %s derives WEBOS_APP_ID '%s' from JIHAD_VARIANT '%s' — the recipe "
                 "filename and the variant table disagree" % (pn, appid, d.getVar('JIHAD_VARIANT', True)))
    rows = [r.split(':') for r in (d.getVar('JIHAD_VARIANT_TABLE', True) or '').split()]
    for col, what in ((0, 'variant token'), (1, 'app id'), (2, 'adapter shim'),
                      (3, 'upstart job'), (4, 'YAP service name')):
        vals = [r[col] for r in rows]
        if len(set(vals)) != len(vals):
            bb.fatal("JIHAD_VARIANT_TABLE: two variants share a %s (%s) — R7 requires all of "
                     "them distinct" % (what, ', '.join(vals)))
}

do_install() {
    install -d ${D}${APPINSTDIR}
    # the UI app (Enyo/Mochi/Mojo JS/CSS/images)
    cp -r ${S}/* ${D}${APPINSTDIR}/

    # THIS variant's runtime bundle → app/deviceroot/{hl, BrowserPlugins, event.d}. Only this
    # variant's shim and upstart job are packaged, so the .ipk cannot carry, overwrite or remove
    # a sibling variant's adapter (R7). jihad-deviceroot warns when a variant's adapter is not
    # staged; here it is FATAL, because shipping the .ipk without it (or, worse, with another
    # variant's) is exactly review finding #5.
    VSRC=${STAGING_DATADIR}/jihad-deviceroot/variants/${JIHAD_VARIANT}
    [ -f $VSRC/BrowserPlugins/${JIHAD_SHIM_SO} ] || \
        bbfatal "no ${JIHAD_SHIM_SO} staged for variant ${JIHAD_VARIANT} — build the adapter for this variant (browser-adapter-jihad must emit jihad-parts/${JIHAD_VARIANT}/)"
    [ -f $VSRC/BrowserAdapterImpl.so ] || \
        bbfatal "no BrowserAdapterImpl.so staged for variant ${JIHAD_VARIANT}"
    DR=${D}${APPINSTDIR}/deviceroot
    install -d $DR/BrowserPlugins $DR/event.d
    # hl/ is byte-identical across variants but each package ships its OWN copy and runs it in
    # place from its own app dir — that is what makes the variants independent on the device.
    cp -a ${STAGING_DATADIR}/jihad-deviceroot/hl $DR/hl
    install -m 0755 $VSRC/BrowserPlugins/${JIHAD_SHIM_SO} $DR/BrowserPlugins/${JIHAD_SHIM_SO}
    install -m 0644 $VSRC/event.d/${JIHAD_UPSTART_JOB}    $DR/event.d/${JIHAD_UPSTART_JOB}
    # The adapter impl rides in the APP ROOT; postinst copies it to the ROOT-OWNED rootfs path
    # /usr/lib/jihad/<variant>/, because the shim dlopens it into privileged LunaSysMgr and
    # rejects anything not root-owned / group-or-world-writable — and cryptofs reports every
    # file as 777, so an impl left in the app bundle could never pass that check.
    install -m 0644 $VSRC/BrowserAdapterImpl.so ${D}${APPINSTDIR}/BrowserAdapterImpl.so

    # License/attribution payload (review #12): the app bundles MPL-2.0 Goanna + Apache-2.0 parts,
    # so ship the composite LICENSE + NOTICE (+ the full texts) in each package, like the direct build.
    install -m 0644 ${JIHAD_REPO}/LICENSE ${D}${APPINSTDIR}/LICENSE
    install -m 0644 ${JIHAD_REPO}/NOTICE  ${D}${APPINSTDIR}/NOTICE
    if [ -d ${JIHAD_REPO}/licenses ]; then
        install -d ${D}${APPINSTDIR}/licenses
        install -m 0644 ${JIHAD_REPO}/licenses/*.txt ${D}${APPINSTDIR}/licenses/
    fi
}

# review #8: the app payload itself comes from SRC_URI (covered by oe-core's
# do_fetch[file-checksums]) and the runtime bundle — including the per-variant shim, impl and
# upstart job — through the DEPENDS chain (jihad-deviceroot declares the packaging/event.d/*
# files in ITS do_compile signature), but the license/attribution files are read straight from
# ${JIHAD_REPO} — declare them so editing LICENSE/NOTICE/licenses/ actually reissues the .ipk
# instead of hitting stale sstate.
# (licenses/*.txt is a glob: it matches files only, never the directory — see ../jihad-common.inc.)
do_install[file-checksums] = "${JIHAD_REPO}/LICENSE ${JIHAD_REPO}/NOTICE ${JIHAD_REPO}/licenses/*.txt"

FILES_${PN} = "${webos_applicationsdir}"
# The app carries the ARM engine/daemon/adapter — an "all"-arch webOS package legitimately holds
# ARM binaries; skip the arch/QA checks that would reject them.
INSANE_SKIP_${PN} = "arch already-stripped ldflags libdir textrel staticdev"

# ---- on-device install/removal (opkg runs these; guarded so a rootfs build never runs them) ----
#
# These are the OE mirror of packaging/${JIHAD_VARIANT}/{postinst,prerm}, which the direct build
# ships and packaging/README.md enumerates. Keep the two in step: same footprint, same order,
# same failure behaviour.
#
# THE FOOTPRINT, and nothing else (R8):
#   /usr/lib/BrowserPlugins/${JIHAD_SHIM_SO}                the NPAPI shim — the only path webOS
#                                                           WebKit scans for plugins, at BOOT
#   /usr/lib/jihad/${JIHAD_VARIANT}/BrowserAdapterImpl.so    root-owned impl (shim trust check)
#   /etc/event.d/${JIHAD_UPSTART_JOB}                        this variant's own upstart job
#   /var/palm/jihad/${JIHAD_VARIANT}/                        root-owned 0755 runtime state
# NOT written: anything under /media/internal (the user's vfat USB volume — the engine, daemon
# and bundled glibc run IN PLACE from $APP/deviceroot/hl on cryptofs, proven executable there
# 2026-07-31), and no stock file (BrowserAdapter.so, BrowserAdapterMojo.so, /etc/event.d/
# browserserver keep their stock bytes).

pkg_postinst_${PN} () {
if [ -n "$D" ]; then exit 1; fi   # defer to first boot if installed into an image
set -e                            # review #6: a failed deploy must NOT report success
APP=/media/cryptofs/apps/usr/palm/applications/${WEBOS_APP_ID}
DR=$APP/deviceroot
HL=$DR/hl
SHIM=/usr/lib/BrowserPlugins/${JIHAD_SHIM_SO}
IMPLDIR=/usr/lib/jihad/${JIHAD_VARIANT}
IMPL=$IMPLDIR/BrowserAdapterImpl.so
JOB=/etc/event.d/${JIHAD_UPSTART_JOB}
STATE=/var/palm/jihad/${JIHAD_VARIANT}
# preflight: the whole payload must be present before we touch the system.
[ -f "$HL/jihad-browserserver" ] && [ -f "$HL/ld-2.23.so" ] \
  && [ -f "$DR/BrowserPlugins/${JIHAD_SHIM_SO}" ] && [ -f "$DR/event.d/${JIHAD_UPSTART_JOB}" ] \
  && [ -f "$APP/BrowserAdapterImpl.so" ] || {
    echo "jihad-postinst(${JIHAD_VARIANT}): bundle payload missing under $APP — aborting" >&2; exit 1; }
# 1. engine + daemon RUN IN PLACE on cryptofs — nothing is copied anywhere. Just make the two
#    things the upstart job execs executable (non-fatal: cryptofs reports every file 777 anyway,
#    and step 3 verifies for real).
chmod 755 "$HL/jihad-browserserver" "$HL/ld-2.23.so" 2>/dev/null || true
# 2. the rootfs writes — ONE rw window, closed by the trap on EVERY exit path incl. failure (R8).
#    /var/palm is created inside it too, since it may sit on the read-only rootfs.
trap 'mount -o remount,ro / 2>/dev/null || true' EXIT
mount -o remount,rw /
mkdir -p /usr/lib/BrowserPlugins "$IMPLDIR" "$STATE"
chown 0:0 /var/palm/jihad "$STATE" 2>/dev/null || true
chmod 0755 /var/palm/jihad "$STATE"
cp -a "$DR/BrowserPlugins/${JIHAD_SHIM_SO}" "$SHIM"
chmod 0755 "$SHIM"
cp -a "$APP/BrowserAdapterImpl.so" "$IMPL"
# cp -a preserves cryptofs's 777, which the shim's trust check REJECTS — set both explicitly.
chown 0:0 "$IMPL" 2>/dev/null || true
chmod 0644 "$IMPL"
cp -a "$DR/event.d/${JIHAD_UPSTART_JOB}" "$JOB"
chmod 0755 "$JOB"
sync
mount -o remount,ro / 2>/dev/null || true
trap - EXIT
# 3. verify the critical files actually landed (BusyBox cp can fail with EBADF across mounts).
[ -f "$SHIM" ] && [ -f "$IMPL" ] && [ -f "$JOB" ] && [ -d "$STATE" ] \
  && [ -x "$HL/jihad-browserserver" ] && [ -x "$HL/ld-2.23.so" ] || {
    echo "jihad-postinst(${JIHAD_VARIANT}): deploy verification failed" >&2; exit 1; }
# 4. runtime kicks — never fatal (a fresh boot picks them up anyway). `stop` first: on an UPGRADE
#    the previous daemon still holds the socket and the replaced files, so a bare `start` fails.
#    Only this variant's job name is ever named. NOTE a NEW plugin enters WebKit's MIME database
#    only on a full BOOT — killall LunaSysMgr refreshes the UI but does not re-scan.
set +e
stop ${JIHAD_UPSTART_JOB} 2>/dev/null
start ${JIHAD_UPSTART_JOB} 2>/dev/null
killall LunaSysMgr 2>/dev/null
exit 0
}

pkg_prerm_${PN} () {
if [ -n "$D" ]; then exit 1; fi
APP=/media/cryptofs/apps/usr/palm/applications/${WEBOS_APP_ID}
HL=$APP/deviceroot/hl
SHIM=/usr/lib/BrowserPlugins/${JIHAD_SHIM_SO}
IMPLDIR=/usr/lib/jihad/${JIHAD_VARIANT}
IMPL=$IMPLDIR/BrowserAdapterImpl.so
JOB=/etc/event.d/${JIHAD_UPSTART_JOB}
SOCK=/tmp/yapserver.${JIHAD_YAP_NAME}
STATE=/var/palm/jihad/${JIHAD_VARIANT}
# NO REFCOUNT (retires review #4): nothing this package installed is shared, so this removes its
# own files unconditionally and no surviving variant can be affected.
# NO CROSS-VARIANT GLOB: `rm -f /tmp/yapserver.jihad-browser*` would delete the Mochi/Mojo
# sockets and `killall jihad-browserserver` would kill their daemons (all three variants ship a
# binary with that name). Every operation below names one exact path or one exact process.
stop ${JIHAD_UPSTART_JOB} 2>/dev/null || true
# Fallback for a daemon upstart is not supervising: the job execs
# `ld-2.23.so --library-path $HL ./jihad-browserserver $HL`, so only THIS variant's processes
# carry THIS app's bundle directory as an exact argv entry.
for p in /proc/[0-9]*; do
  [ -r "$p/cmdline" ] || continue
  if tr '\0' '\n' < "$p/cmdline" 2>/dev/null | grep -qxF "$HL"; then
    kill "$(basename "$p")" 2>/dev/null || true
  fi
done
rm -f "$SOCK"
# One rw window, closed on every exit path (R8) — /var/palm may sit on the read-only rootfs.
trap 'mount -o remount,ro / 2>/dev/null || true' EXIT
mount -o remount,rw / 2>/dev/null || true
rm -f "$SHIM"
rm -f "$IMPL"
rmdir "$IMPLDIR" 2>/dev/null || true
rmdir /usr/lib/jihad 2>/dev/null || true    # succeeds only once the LAST variant is gone
rm -f "$JOB"
rm -rf "$STATE"
rmdir /var/palm/jihad 2>/dev/null || true   # ditto
sync
mount -o remount,ro / 2>/dev/null || true
trap - EXIT
# The engine/daemon live in $APP/deviceroot and go with the app dir the installer removes.
# There is nothing to clean under /media/internal because nothing was ever written there.
killall LunaSysMgr 2>/dev/null || true
true
}
