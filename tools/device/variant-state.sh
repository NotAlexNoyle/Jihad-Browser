#!/bin/sh
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# On the TouchPad: the state of the three Jihad variants at a glance — leftover test hooks, proxy
# prefs, the daemon's md5, the adapter paint-log gates, the upstart jobs, the root mount, and
# Certificate Manager's store.
#   build/webos-oe/dev-sh.sh tools/device/variant-state.sh
A=/media/cryptofs/apps/usr/palm/applications
for v in enyo mochi mojo; do
	case $v in enyo) id=net.riverstonerelay.jihad-browser; job=jihad ;;
	           mochi) id=net.riverstonerelay.jihad-browser-mochi; job=jihad-mochi ;;
	           mojo) id=net.riverstonerelay.jihad-browser-mojo; job=jihad-mojo ;; esac
	D=$A/$id
	[ -d "$D" ] || { echo "$v: not installed"; continue; }
	hooks=$(find "$D" -iname '*selftest*' 2>/dev/null | wc -l)
	proxy=$(grep -c 'user_pref("\(network\|jihad\)\.proxy' "$D/profile/prefs.js" 2>/dev/null)
	daemon=$(md5sum "$D/deviceroot/hl/jihad-browserserver" 2>/dev/null | cut -c1-8)
	gate=$(ls /var/palm/jihad/$v/ 2>/dev/null | grep -c '^adapterlog$')
	echo "$v: hooks=$hooks proxyprefs=${proxy:-0} daemon=${daemon:--} adapterlog-gate=$gate $(status $job 2>&1)"
done
echo "root: $(grep ' / ' /proc/mounts | grep -v rootfs | cut -d' ' -f4 | cut -d, -f1)"
echo "certificate store: $(ls /var/ssl/certs 2>/dev/null | wc -l) files, $(ls /var/ssl/trustedcerts 2>/dev/null | wc -l) trusted links"
