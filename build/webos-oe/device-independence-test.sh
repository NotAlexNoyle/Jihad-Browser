#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# The R7 acceptance harness — proves the three UI variants are INDEPENDENT packages
# (cavekit-device-build.md R7) and that each install/remove is footprint-exact (R8).
#
# Why a script and not a checklist: R7's claims are about interactions between packages
# ("installing B does not disturb A", "removing B leaves A working"), which is a matrix, and a
# matrix executed by hand is a matrix executed inconsistently. Every assertion here maps to a
# specific acceptance criterion, named in the output so a run doubles as the evidence record.
#
# postinst is what deploys the shim/impl/upstart, and `palm-install` runs as a non-root user and
# does NOT execute postinst (webOS packaging rule). So installs go through /usr/bin/ipkg, which
# does.
#
# STAGING LOCATION. Not /var/tmp: that is a 32 MB tmpfs on this device and a variant .ipk is
# ~36 MB, so the push dies with ENOSPC partway through (measured 2026-07-31). Not
# /media/internal either — R8 forbids us writing to the user's USB volume. So the .ipk lands in
# a dot-directory on the app partition, which is the same filesystem opkg unpacks into and has
# ~10 GB free, and it is deleted immediately after the install either way.
#
# Usage:
#   device-independence-test.sh install <variant> <ipk>
#   device-independence-test.sh remove  <variant>
#   device-independence-test.sh check   <variant>          assert this variant is alive + serving
#   device-independence-test.sh matrix  <enyo.ipk> <mochi.ipk> <mojo.ipk>
set -uo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
AUDIT="$HERE/device-citizen-audit.sh"

# Identity table — MUST match context/plans/plan-variant-identity.md. Kept as parallel lookups
# rather than an array-of-structs because this has to run under both bash and scrutiny.
appid()   { case "$1" in enyo) echo net.riverstonerelay.jihad-browser;; mochi) echo net.riverstonerelay.jihad-browser.mochi;; mojo) echo net.riverstonerelay.jihad-browser.mojo;; *) return 1;; esac; }
yapname() { case "$1" in enyo) echo jihad-browser;;                    mochi) echo jihad-browser-mochi;;                    mojo) echo jihad-browser-mojo;;                    *) return 1;; esac; }
job()     { case "$1" in enyo) echo jihad;;                            mochi) echo jihad-mochi;;                            mojo) echo jihad-mojo;;                            *) return 1;; esac; }
shim()    { case "$1" in enyo) echo BrowserAdapterJihad.so;;           mochi) echo BrowserAdapterJihadMochi.so;;            mojo) echo BrowserAdapterJihadMojo.so;;            *) return 1;; esac; }

nc()   { timeout 120 novacom run "file://$1" -- "${@:2}" 2>/dev/null; }
pass() { echo "  PASS  $*"; }
fail() { echo "  FAIL  $*"; FAILED=$((FAILED+1)); }
FAILED=0

need_device() { [ -n "$(novacom -l 2>/dev/null)" ] || { echo "NO DEVICE"; exit 1; }; }

do_install() {
	local v="$1" ipk="$2"
	[ -f "$ipk" ] || { echo "no such ipk: $ipk"; exit 2; }
	echo "=== install $v ($(basename "$ipk")) ==="
	local stage=/media/cryptofs/apps/.jihad-stage
	nc /bin/mkdir -p "$stage"
	timeout 900 novacom put "file://$stage/jihad-$v.ipk" < "$ipk" || { echo "push failed"; exit 3; }
	# ipkg (not palm-install) so postinst actually runs as root.
	#
	# `-o /media/cryptofs/apps` is REQUIRED, not optional. A bare `ipkg install` uses / as the
	# root, so the package's ./usr/palm/applications/... payload lands on the ROOTFS at
	# /usr/palm/applications/... — ~36 MB into a 559 MB system partition, where the app is not
	# an app and postinst (which looks under /media/cryptofs/apps) fails. Measured the hard way
	# on 2026-07-31. That offline root is also where the device's app-partition package database
	# lives (/media/cryptofs/apps/usr/lib/ipkg), which is the one Preware maintains.
	nc /usr/bin/ipkg -o /media/cryptofs/apps install "$stage/jihad-$v.ipk"
	nc /bin/rm -f "$stage/jihad-$v.ipk"
	nc /bin/rmdir "$stage"
}

do_remove() {
	local v="$1"
	echo "=== remove $v ==="
	nc /usr/bin/ipkg -o /media/cryptofs/apps remove "$(appid "$v")"
}

# Assert a variant is fully alive: its own daemon on its own socket, its own upstart job, its own
# shim + impl on the rootfs, and its app dir carrying the in-place runtime (R7 + R8).
do_check() {
	local v="$1" a s j
	a=$(appid "$v"); s="/tmp/yapserver.$(yapname "$v")"; j=$(job "$v")
	echo "=== check $v ==="

	nc /bin/ls "/media/cryptofs/apps/usr/palm/applications/$a/appinfo.json" >/dev/null \
		&& pass "app installed ($a)" || fail "app dir missing ($a)"

	# R8: the engine must live in the app bundle and run from there — not from /media/internal.
	nc /bin/ls "/media/cryptofs/apps/usr/palm/applications/$a/deviceroot/hl/jihad-browserserver" >/dev/null \
		&& pass "runtime in app bundle (runs in place)" || fail "deviceroot/hl/jihad-browserserver missing"

	nc /bin/ls "/usr/lib/BrowserPlugins/$(shim "$v")" >/dev/null \
		&& pass "own shim installed ($(shim "$v"))" || fail "shim missing"

	nc /bin/ls "/usr/lib/jihad/$v/BrowserAdapterImpl.so" >/dev/null \
		&& pass "own impl installed (/usr/lib/jihad/$v)" || fail "impl missing"

	nc /bin/ls "/etc/event.d/$j" >/dev/null \
		&& pass "own upstart job ($j)" || fail "upstart job missing"

	# The daemon: its own process, its own socket.
	nc /bin/ls "$s" >/dev/null && pass "own socket ($s)" || fail "socket missing ($s)"
	# Capture then match — never `… | grep -q` under `set -o pipefail`: -q exits at the first
	# match, ps upstream takes SIGPIPE, and pipefail turns that 141 into a failed test. It
	# reported "no daemon process" for a daemon that was demonstrably running (2026-07-31).
	local ps_out
	ps_out=$(nc /bin/ps aux)
	case "$ps_out" in
		*"$a/deviceroot"*) pass "own daemon process running (in place from the app bundle)" ;;
		*) fail "no daemon process for $v" ;;
	esac

	# R8: nothing of ours on the user's volume.
	if nc /bin/ls /media/internal | grep -qi jihad; then
		fail "R8 VIOLATION: something jihad-named on /media/internal"
	else
		pass "R8: /media/internal clean"
	fi
}

# The full R7 matrix. Each phase snapshots, so a residue/collateral diff is available afterwards.
do_matrix() {
	local enyo="$1" mochi="$2" mojo="$3"
	"$AUDIT" snap m0-baseline

	do_install enyo  "$enyo";  "$AUDIT" snap m1-enyo;        do_check enyo
	do_install mochi "$mochi"; "$AUDIT" snap m2-enyo-mochi
	echo "--- R7: installing mochi must not disturb enyo ---"; do_check enyo; do_check mochi
	do_install mojo  "$mojo";  "$AUDIT" snap m3-all
	echo "--- R7: installing mojo must not disturb the other two ---"
	do_check enyo; do_check mochi; do_check mojo

	echo "--- R7: removing the MIDDLE variant must leave the others working ---"
	do_remove mochi; "$AUDIT" snap m4-enyo-mojo
	do_check enyo; do_check mojo
	nc /bin/ls "/usr/lib/BrowserPlugins/$(shim mochi)" >/dev/null \
		&& fail "mochi shim survived removal" || pass "mochi shim removed"

	echo "--- R8: remove everything, expect an EXACT reversal to baseline ---"
	do_remove enyo; do_remove mojo; "$AUDIT" snap m5-removed
	if "$AUDIT" diff m0-baseline m5-removed >/dev/null 2>&1; then
		pass "R8: full install/remove cycle left NO residue and deleted nothing else"
	else
		fail "R8: post-removal state differs from baseline — see: $AUDIT diff m0-baseline m5-removed"
	fi

	echo; echo "=== $FAILED failure(s) ==="; [ "$FAILED" -eq 0 ]
}

need_device
case "${1:-}" in
	install) do_install "${2:?variant}" "${3:?ipk}" ;;
	remove)  do_remove  "${2:?variant}" ;;
	check)   do_check   "${2:?variant}"; echo "=== $FAILED failure(s) ==="; [ "$FAILED" -eq 0 ] ;;
	matrix)  do_matrix  "${2:?enyo ipk}" "${3:?mochi ipk}" "${4:?mojo ipk}" ;;
	*) echo "usage: $0 {install <v> <ipk>|remove <v>|check <v>|matrix <enyo> <mochi> <mojo>}" >&2; exit 2 ;;
esac
