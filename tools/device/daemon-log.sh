#!/bin/sh
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# On the TouchPad: a variant's daemon log size and its last lines matching a pattern.
#   build/webos-oe/dev-sh.sh tools/device/daemon-log.sh <enyo|mochi|mojo> <pattern> [lines]
# The size comes first so two calls show whether the log grew between them.
L=/var/palm/jihad/${1:?variant}/daemon.log
[ -f "$L" ] || { echo "no $L"; exit 1; }
echo "size $(wc -c < "$L")"
grep -nE "${2:?pattern}" "$L" | tail -n "${3:-15}" | cut -c1-260
