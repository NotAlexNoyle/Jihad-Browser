#!/bin/sh
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# enyo-restore-job.sh (run with build/webos-oe/dev-sh.sh): the Enyo variant back on its upstart job after a
# hand-started (spike) daemon, the round dumps removed, the three jobs reported.
# the Enyo variant back on its upstart job after a hand-started (spike) daemon
HL=/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/deviceroot/hl
stop jihad >/dev/null 2>&1; sleep 2
for p in /proc/[0-9]*; do c=$(tr '\0' ' ' 2>/dev/null < $p/cmdline); case "$c" in *jihad-browserserver*"$HL"*) echo "killing hand daemon ${p#/proc/}"; kill ${p#/proc/} 2>/dev/null;; esac; done
sleep 3
for p in /proc/[0-9]*; do c=$(tr '\0' ' ' 2>/dev/null < $p/cmdline); case "$c" in *jihad-browserserver*"$HL"*) kill -9 ${p#/proc/} 2>/dev/null; echo "SIGKILL ${p#/proc/}";; esac; done
rm -f /tmp/frame.raw /tmp/frames.raw /tmp/frame.raw-* /tmp/st-pan.png /tmp/vs-*.raw /tmp/vw-*.raw
start jihad; sleep 10
status jihad; status jihad-mochi; status jihad-mojo
ls /var/palm/jihad/enyo/ | grep -v 'log\|prefs' | head
df /var | tail -1
