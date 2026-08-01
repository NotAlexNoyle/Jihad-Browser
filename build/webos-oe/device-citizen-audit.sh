#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Citizen audit — the evidence tool for cavekit-device-build.md R7/R8.
#
# R8 requires proof, not assertion: that installing/removing a Jihad package modifies NO stock
# file, writes NOTHING to the user's /media/internal volume, and leaves NO residue behind. That
# is a before/after comparison, so this script captures one side of it into a snapshot file and
# `--diff` compares two snapshots.
#
# Usage:
#   device-citizen-audit.sh snap <name>      capture a snapshot into out-audit/<name>.txt
#   device-citizen-audit.sh diff <a> <b>     diff two snapshots (exit 1 if they differ)
#
# Typical acceptance run:
#   snap baseline  ->  install variant  ->  snap installed  ->  diff baseline installed
#     (expect: ONLY this variant's own files appear, all under the enumerated paths)
#   remove variant ->  snap removed    ->  diff baseline removed
#     (expect: EMPTY — exact reversal, no residue, no collateral deletion)
set -uo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
OUT="$HERE/out-audit"

nc() { timeout 60 novacom run "file://$1" -- "${@:2}" 2>/dev/null; }

snap() {
	local name="${1:?snapshot name required}"
	mkdir -p "$OUT"
	local f="$OUT/$name.txt"
	{
		echo "## snapshot: $name"
		# NOTE: no date/uptime here on purpose — a snapshot must be byte-comparable to another
		# snapshot, so every line has to be a property of the system, not of when we looked.

		echo "## --- stock files that must never change (md5) ---"
		# The stock browser stack. If any of these md5s move, we modified a system file and R8 fails.
		nc /usr/bin/md5sum \
			/usr/lib/BrowserPlugins/BrowserAdapter.so \
			/usr/lib/BrowserPlugins/BrowserAdapterMojo.so \
			/usr/lib/BrowserPlugins/RemoteAdapter.so \
			/etc/event.d/browserserver \
			/etc/event.d/browserservermojo \
			/usr/bin/BrowserServer \
			/usr/bin/BrowserServerMojo | sort

		echo "## --- /usr/lib/BrowserPlugins (names + sizes) ---"
		nc /bin/ls -la /usr/lib/BrowserPlugins | awk '{print $5, $9}' | sort

		echo "## --- /usr/lib/jihad tree ---"
		nc /usr/bin/find /usr/lib/jihad -type f | sort

		echo "## --- /etc/event.d ---"
		nc /bin/ls /etc/event.d | sort

		echo "## --- /var/palm/jihad tree (dirs only; logs churn) ---"
		nc /usr/bin/find /var/palm/jihad -type d | sort

		echo "## --- /media/internal top level (MUST be untouched by us) ---"
		nc /bin/ls -a /media/internal | sort

		echo "## --- installed jihad apps ---"
		nc /bin/ls /media/cryptofs/apps/usr/palm/applications | grep -i jihad | sort

		echo "## --- our yap sockets ---"
		nc /bin/ls /tmp | grep -E '^yapserver\.' | sort
	} > "$f"
	echo "wrote $f ($(wc -l < "$f") lines)"
}

case "${1:-}" in
	snap) snap "${2:-}" ;;
	diff)
		a="$OUT/${2:?}.txt"; b="$OUT/${3:?}.txt"
		[ -f "$a" ] && [ -f "$b" ] || { echo "missing snapshot: $a or $b" >&2; exit 2; }
		# Drop the "## snapshot: <name>" header before comparing. It is the one line that is a
		# property of the CAPTURE rather than of the system, so leaving it in made every
		# comparison report DIFFERS — including the R8 residue check in
		# device-independence-test.sh, whose whole job is to assert two snapshots are equal.
		# A check that can never pass is worse than no check: it reads as a real failure.
		if diff -u <(grep -v '^## snapshot: ' "$a") <(grep -v '^## snapshot: ' "$b"); then
			echo "IDENTICAL: $2 == $3"
		else
			echo "DIFFERS: $2 vs $3"; exit 1
		fi
		;;
	*) echo "usage: $0 snap <name> | diff <a> <b>" >&2; exit 2 ;;
esac
