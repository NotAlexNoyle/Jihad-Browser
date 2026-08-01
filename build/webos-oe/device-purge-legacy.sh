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
# Usage: device-purge-legacy.sh [--apply]     (default is a dry run that only prints)
set -uo pipefail

APPLY=0
[ "${1:-}" = "--apply" ] && APPLY=1

nc() { timeout 90 novacom run "file://$1" -- "${@:2}" 2>/dev/null; }
run() {
	if [ "$APPLY" = 1 ]; then echo "  RUN: $*"; nc "$@"; else echo "  would run: $*"; fi
}

[ -n "$(novacom -l 2>/dev/null)" ] || { echo "NO DEVICE (novacom -l empty)"; exit 1; }
echo "=== device ==="; novacom -l

echo "=== 1. stop legacy daemon + upstart job ==="
run /sbin/stop jihad
run /usr/bin/killall jihad-browserserver

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
