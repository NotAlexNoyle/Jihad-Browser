#!/bin/sh
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# On the TouchPad: a variant's adapter paint log (the gate file the adapter checks once per
# LunaSysMgr, so restart Luna after "on" before trusting it).
#   build/webos-oe/dev-sh.sh tools/device/adapter-log.sh <enyo|mochi|mojo> on|off|show
S=/var/palm/jihad/${1:?variant}
G=$S/adapterlog; L=$S/adapter.log
case "${2:-show}" in
	on) touch "$G"; : > "$L"; chmod 600 "$L" ;;
	off) rm -f "$G" "$L" ;;
	show) ;;
	*) echo "usage: adapter-log.sh <variant> on|off|show"; exit 2 ;;
esac
ls -la "$S/" | grep adapter
[ -f "$L" ] && echo "adapter.log lines: $(wc -l < "$L")"
exit 0
