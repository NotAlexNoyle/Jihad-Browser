# Jihad Browser — standalone Goanna browser, Enyo 2 + Mochi UI variant (2 of 3 independent
# packages).
#
# Same complete stack as the Enyo variant, with the Enyo 2 + Mochi front-end and its OWN app id,
# MIME, adapter shim, adapter impl, YAP service name, socket and upstart job — nothing is shared
# with the Enyo or Mojo package (cavekit-device-build.md R7). See jihad-app.inc; every name comes
# from ../jihad-variants.inc.
#
# The filename carries the app id with a HYPHEN, not a dot: PN comes from the filename and
# jihad-app.inc bb.fatal's unless PN == WEBOS_APP_ID, and the id must not be a dot-child of the
# Enyo id or ipkg's `<pkgid>.*` removal glob eats this package's metadata when the Enyo package is
# removed (../../../../context/impl/impl-ipkg-prefix-collision.md).

SUMMARY = "Jihad Browser (standalone Goanna browser, Enyo 2 + Mochi UI)"
LICENSE = "Apache-2.0 & MPL-2.0"

# The ONE identity fact this recipe states; app id, shim, upstart job, YAP name are all derived.
JIHAD_VARIANT = "mochi"

SRC_URI = "file://app-mochi/"
S = "${WORKDIR}/app-mochi"

require jihad-app.inc

# review #1: unlike the Enyo-1 variant (which loads its framework from the OS path
# /usr/palm/frameworks/enyo/0.10), app-mochi/index.html loads BUNDLED Enyo 2 + layout + Mochi
# (enyo/enyo.js, lib/layout/package.js, lib/mochi/package.js) that are NOT in app-mochi/. Stage
# them from the third_party submodules (as build-mochi-ipk.sh does) or the app fails before
# creating JihadBrowser. Prune build tooling/VCS so the .ipk stays lean.
JIHAD_ENYO   = "${JIHAD_REPO}/third_party/mochi-sampler/enyo"
JIHAD_LAYOUT = "${JIHAD_REPO}/third_party/enyo-layout"
JIHAD_MOCHI  = "${JIHAD_REPO}/third_party/mochi"

# review #8: those three framework trees are shipped payload read straight from ${JIHAD_REPO}
# (submodules, not SRC_URI), so add them to do_install's signature on top of the LICENSE/NOTICE
# set jihad-app.inc declares. Passed as bare directories (bitbake 1.18 os.walk's them; ~13 MB /
# 1590 files, md5-cached by mtime) — NOT as globs, which must never match a directory. See
# ../jihad-common.inc.
do_install[file-checksums] += "${JIHAD_ENYO} ${JIHAD_LAYOUT} ${JIHAD_MOCHI}"

do_install_append() {
    A=${D}${webos_applicationsdir}/${WEBOS_APP_ID}
    install -d ${A}/enyo ${A}/lib/layout ${A}/lib/mochi
    cp -a ${JIHAD_ENYO}/.   ${A}/enyo/
    cp -a ${JIHAD_LAYOUT}/. ${A}/lib/layout/
    cp -a ${JIHAD_MOCHI}/.  ${A}/lib/mochi/
    # drop VCS + node build tooling from the shipped payload
    find ${A}/enyo ${A}/lib -depth \( -name .git -o -name .github -o -name node_modules -o -name tools \) \
        -exec rm -rf {} + 2>/dev/null || true
    # fatal QA (matches the direct build): every index.html script reference must be staged.
    for s in $(grep -oE 'src="[^"]+"' ${A}/index.html | sed -e 's/^src="//' -e 's/"$//'); do
        case "$s" in http*://*) continue ;; esac
        [ -e "${A}/$s" ] || bbfatal "mochi index.html references a missing script: $s"
    done
}
