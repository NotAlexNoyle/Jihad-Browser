#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# One-time purge of the LEGACY (pre-2026-07-31) on-device layout.
#
# Development deploys before the R7/R8 rework left three kinds of cruft on the device that a
# clean install must never reproduce:
#   1. /media/internal/jihad/            — the old daemon home + a session's worth of dev scripts,
#                                          logs, frame dumps and .so backups, on the USER'S vfat
#                                          USB volume. R8 forbids writing there at all.
#   2. /usr/lib/BrowserPlugins/*.prejihadshim, BrowserAdapter.so.stock
#                                        — hand-made backups from bring-up. Ours, not the system's,
#                                          and confusing to anyone auditing the plugin directory.
#   3. the old single-variant shim + /etc/event.d/jihad job, if a legacy build installed them.
#
# It refuses to remove BrowserAdapter.so.stock unless that file is byte-identical to the live
# stock BrowserAdapter.so — i.e. unless the backup is provably redundant. If they differ, the
# live one may BE a modified copy and the backup is the only original; that is a stop-and-think
# situation, not something to clean up automatically.
#
# ── WHY IT REFUSES TO RUN AGAINST A CONFIGURED INSTALL (review F-11) ──────────────────────────
# The legacy Enyo names and the CURRENT Enyo names are deliberately identical — plan-variant-
# identity.md keeps the shipping variant unsuffixed, because renaming the one deployment known
# to work buys nothing. So `/usr/lib/BrowserPlugins/BrowserAdapterJihad.so`, `/etc/event.d/
# jihad` and `/tmp/yapserver.jihad-browser` in steps 4 and 5 name the PRODUCTION Enyo variant's
# shim, upstart job and socket just as well as the legacy build's. Running this script after a
# real install therefore uninstalls half of it — the exact collateral damage the rest of the
# R7/R8 work exists to make impossible. The guard below refuses outright when any variant is
# installed; purge FIRST, install SECOND. There is no --force: the correct way to remove a
# configured variant is its own prerm.
#
# Usage: device-purge-legacy.sh [--apply]     (default is a dry run that only prints)
set -uo pipefail

APPLY=0
[ "${1:-}" = "--apply" ] && APPLY=1

nc() { timeout 90 novacom run "file://$1" -- "${@:2}" 2>/dev/null; }
run() {
	if [ "$APPLY" = 1 ]; then echo "  RUN: $*"; nc "$@"; else echo "  would run: $*"; fi
}
# Exact-path existence probe. `ls -d <path>` prints the path or nothing; no glob, no prefix
# match — the Enyo app id is a prefix of the Mochi and Mojo ones, so a prefix test here would
# report "installed" for the wrong variant.
exists() { [ -n "$(nc /bin/ls -d "$1")" ]; }

[ -n "$(novacom -l 2>/dev/null)" ] || { echo "NO DEVICE (novacom -l empty)"; exit 1; }
echo "=== device ==="; novacom -l

echo "=== 0. refuse to run against a configured install (F-11) ==="
APPS=/media/cryptofs/apps/usr/palm/applications
CONFIGURED=""
for p in \
	"$APPS/net.riverstonerelay.jihad-browser" \
	"$APPS/net.riverstonerelay.jihad-browser.mochi" \
	"$APPS/net.riverstonerelay.jihad-browser.mojo" \
	/usr/lib/jihad/enyo /usr/lib/jihad/mochi /usr/lib/jihad/mojo \
	/var/palm/jihad/enyo /var/palm/jihad/mochi /var/palm/jihad/mojo
do
	exists "$p" && CONFIGURED="$CONFIGURED
  $p"
done
if [ -n "$CONFIGURED" ]; then
	echo "!! REFUSING: a CURRENT Jihad install is present on this device:$CONFIGURED"
	echo "!! Steps 4-5 below would delete the production Enyo variant's shim, upstart job and"
	echo "!! socket — the legacy and current names are deliberately identical. Remove the"
	echo "!! installed variant(s) with their own prerm first, then re-run this purge."
	exit 1
fi
echo "  no configured install found — safe to purge the legacy layout"

echo "=== 1. stop legacy daemon + upstart job ==="
run /sbin/stop jihad
# NOT `killall jihad-browserserver` (F-11): all three variants ship a binary with that name, so
# killall is a cross-variant weapon — it would take down the Mochi and Mojo daemons of anyone
# who has them installed. Match on argv instead. The LEGACY daemon is the only one whose command
# line carries the legacy home /media/internal/jihad/hl as a whole argument; every current
# variant runs out of its own app bundle, so this cannot reach a shipping daemon even if the
# step-0 guard were somehow bypassed. The comparison is done in the SHELL, never with `grep -x`:
# this device's BusyBox 1.17.3 grep has no -x and prints its usage text instead (measured
# 2026-07-31), which would make the match silently never fire.
LEGACY_HL=/media/internal/jihad/hl
KILL_LEGACY=$(cat <<'EOS'
LEG=/media/internal/jihad/hl
for p in /proc/[0-9]*; do
	[ -r "$p/cmdline" ] || continue
	m=0
	while IFS= read -r a; do
		[ "$a" = "$LEG" ] && { m=1; break; }
	done <<EOF
$(tr '\0' '\n' < "$p/cmdline" 2>/dev/null)
EOF
	[ "$m" = 1 ] && kill "$(basename "$p")" 2>/dev/null
done
true
EOS
)
if [ "$APPLY" = 1 ]; then
	echo "  RUN: argv-exact kill of any daemon running from $LEGACY_HL"
	nc /bin/sh -c "$KILL_LEGACY"
else
	echo "  would run: argv-exact kill of any daemon running from $LEGACY_HL"
fi

echo "=== 2. /media/internal/jihad (user volume must be left clean) ==="
echo "  current size:"; nc /usr/bin/du -sh /media/internal/jihad
run /bin/rm -rf /media/internal/jihad

echo "=== 3. plugin-directory backups left by bring-up ==="
STOCK_MD5=$(nc /usr/bin/md5sum /usr/lib/BrowserPlugins/BrowserAdapter.so       | awk '{print $1}')
BACK_MD5=$( nc /usr/bin/md5sum /usr/lib/BrowserPlugins/BrowserAdapter.so.stock | awk '{print $1}')
echo "  live stock md5 : ${STOCK_MD5:-<none>}"
echo "  backup    md5 : ${BACK_MD5:-<none>}"
run /bin/mount -o remount,rw /
if [ -n "$BACK_MD5" ] && [ "$STOCK_MD5" = "$BACK_MD5" ]; then
	echo "  backup is byte-identical to the live stock adapter -> redundant, removing"
	run /bin/rm -f /usr/lib/BrowserPlugins/BrowserAdapter.so.stock
else
	echo "  !! backup differs from (or is missing against) the live adapter — NOT touching it."
	echo "  !! Investigate before proceeding: the live BrowserAdapter.so may not be stock."
fi
run /bin/rm -f /usr/lib/BrowserPlugins/BrowserAdapterJihad.so.prejihadshim

echo "=== 4. legacy single-variant shim + upstart job ==="
run /bin/rm -f /usr/lib/BrowserPlugins/BrowserAdapterJihad.so
run /bin/rm -f /etc/event.d/jihad
run /bin/rm -f /usr/lib/jihad/BrowserAdapterImpl.so
run /bin/sync
run /bin/mount -o remount,ro /

echo "=== 5. legacy sockets ==="
run /bin/rm -f /tmp/yapserver.jihad-browser

if [ "$APPLY" = 0 ]; then
	echo
	echo "DRY RUN — nothing was changed. Re-run with --apply to execute."
fi
