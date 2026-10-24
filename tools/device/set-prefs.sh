#!/bin/sh
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# On the TouchPad: change a variant's profile prefs with its daemon stopped — drop every user_pref
# whose name matches a pattern, then append the user_pref lines of a file (optional), then start
# the daemon again and show what matches.
#   novacom put file:///tmp/jihad-newprefs.txt < my-prefs.txt        (optional)
#   build/webos-oe/dev-sh.sh tools/device/set-prefs.sh <enyo|mochi|mojo> '<name regex>' [/tmp/jihad-newprefs.txt]
# e.g. the proxy prefs: 'network\.proxy\.|jihad\.proxy\.'; the accepted-certificate list:
# 'jihad\.security\.webos_certs\.accepted'. The file is deleted afterwards.
case "${1:?variant}" in
	enyo) id=net.riverstonerelay.jihad-browser; job=jihad ;;
	mochi) id=net.riverstonerelay.jihad-browser-mochi; job=jihad-mochi ;;
	mojo) id=net.riverstonerelay.jihad-browser-mojo; job=jihad-mojo ;;
	*) echo "unknown variant $1"; exit 2 ;;
esac
RE="${2:?name regex}"; NEW="${3:-}"
P=/media/cryptofs/apps/usr/palm/applications/$id/profile/prefs.js
[ -f "$P" ] || { echo "no $P"; exit 1; }
stop $job >/dev/null
# The daemon's clean shutdown saves its prefs when they changed: an edit made before it has exited
# can be written over, so wait for the process to go (the fixed 3 s did not always cover it). NB a
# cleared jihad.blocking.lastUpdate comes back by design: the blocker sets it from EasyList's
# version when it is 0; force an update with the value 1.
i=0
while [ $i -lt 30 ]; do
	alive=0
	for q in /proc/[0-9]*; do case "$(tr '\0' ' ' 2>/dev/null < $q/cmdline)" in *"applications/$id/"*jihad-browserserver*) alive=1;; esac; done
	[ $alive = 0 ] && break
	sleep 1; i=$((i+1))
done
[ $alive = 0 ] || { echo "the $job daemon is still running after 30 s: prefs not changed"; start $job >/dev/null; exit 1; }
# BusyBox 1.17 sed knows -r (not -E); the name is matched inside user_pref("…").
sed -i -r "/user_pref\(\"($RE)/d" "$P"
[ -n "$NEW" ] && [ -f "$NEW" ] && { cat "$NEW" >> "$P"; rm -f "$NEW"; }
grep -E "user_pref\(\"($RE)" "$P"
echo "matching prefs now: $(grep -cE "user_pref\(\"($RE)" "$P")"
start $job >/dev/null; sleep 3; status $job
