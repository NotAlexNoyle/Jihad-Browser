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

# NC_TIMEOUT lets one call buy more wall-clock than the default without loosening it for all of
# them: the recursive /media/internal walk below can legitimately take minutes on a device with a
# populated user volume, and a `timeout`-truncated listing would be BOTH wrong and unstable
# between runs (which is the one thing a byte-comparable snapshot must never be).
nc() { timeout "${NC_TIMEOUT:-60}" novacom run "file://$1" -- "${@:2}" 2>/dev/null; }

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

		# F-14: `-type f` missed a leftover EMPTY DIRECTORY, which is residue too — prerm only
		# `rmdir`s /usr/lib/jihad and /usr/lib/jihad/<V>, so a failed rmdir is exactly the
		# outcome this snapshot has to be able to see. Every entry, no type filter.
		echo "## --- /usr/lib/jihad tree (every entry) ---"
		nc /usr/bin/find /usr/lib/jihad | sort

		echo "## --- /etc/event.d ---"
		nc /bin/ls /etc/event.d | sort

		# F-14: was `-type d`, so a residual FILE under /var/palm/jihad was invisible — the
		# inverse blind spot, on the tree prerm `rm -rf`s. Every entry.
		# `find` prints NAMES only: the daemon log's existence shows up (which is what the R8
		# residue check needs) while its contents, size and mtime do not (which is what keeps two
		# snapshots byte-comparable). Do not "improve" this into `ls -l`.
		echo "## --- /var/palm/jihad tree (every entry; names only, so log CONTENT stays out) ---"
		nc /usr/bin/find /var/palm/jihad | sort

		# F-14: this was `ls -a /media/internal`, i.e. the TOP LEVEL ONLY — anything written into
		# an already-existing subdirectory (which is where a stray write would land) was invisible
		# to the one assertion R8 exists to enforce. Walk the whole volume.
		# NOTE the one deliberate exception, /media/internal/downloads: that is where FINISHED
		# USER DOWNLOADS go (F-10), by webOS convention and by the user's own request. Install and
		# removal still write nothing there, so an install->remove snapshot pair is unaffected;
		# but a snapshot taken across a browsing session legitimately shows the user's files.
		echo "## --- /media/internal (whole tree; MUST be untouched by install/removal) ---"
		NC_TIMEOUT=600 nc /usr/bin/find /media/internal | sort

		echo "## --- installed jihad apps ---"
		nc /bin/ls /media/cryptofs/apps/usr/palm/applications | grep -i jihad | sort

		# The engine's profile lives in each app's OWN cryptofs directory, both halves:
		#   $APP/profile   ProfD  — cookies.sqlite, prefs.js, permissions
		#   $APP/cache     ProfLD — cache2, startupCache
		# (isis and Atlas both put cookies+cache on cryptofs on this device; /var has only
		# 49.6 MB free and is shared with system state.) The DAEMON creates both at runtime, so
		# ipkg never tracked them and would leave the trees behind on removal — residue that R8's
		# exact-reversal criterion forbids and that this snapshot is the evidence for. `prerm`
		# removes them explicitly; this makes that visible.
		#
		# Directory names only: cache2 fans out into thousands of entry files and cookies.sqlite
		# is rewritten on every page load, so listing FILES would make two snapshots incomparable
		# — the one thing this file must never be. A leftover tree still shows up as a directory.
		echo "## --- per-app engine profile + cache trees (dirs only; contents churn) ---"
		for a in net.riverstonerelay.jihad-browser \
		         net.riverstonerelay.jihad-browser.mochi \
		         net.riverstonerelay.jihad-browser.mojo; do
			for d in profile cache; do
				nc /usr/bin/find "/media/cryptofs/apps/usr/palm/applications/$a/$d" -type d
			done
		done | sort

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
