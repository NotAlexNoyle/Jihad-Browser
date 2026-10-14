#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Every submodule pin must be fetchable from its .gitmodules URL by someone without this machine's
# objects (cavekit-device-build.md R11, "pins are public"): a clone elsewhere stops at the first pin
# its remote does not have, and git's submodule loop stops with it, so the ones after it stay
# missing too. This fetches each pinned commit into a throwaway repository (shallow, and without
# blobs where the server allows) and reports. Run it before a tier that moves a pin is called done.
#
#   tools/host/check-pins.sh            every submodule
#   tools/host/check-pins.sh <path>...  some of them
set -euo pipefail
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
git -C "$TMP" init -q
# <name> <path> per submodule, from .gitmodules.
list() { git -C "$REPO" config -f .gitmodules --get-regexp '^submodule\..*\.path$' | sed -e 's/^submodule\.//' -e 's/\.path / /'; }
fail=0
if [ $# -gt 0 ]; then want=("$@"); else mapfile -t want < <(list | awk '{print $2}'); fi
for p in "${want[@]}"; do
	name="$(list | awk -v p="$p" '$2==p {print $1; exit}')"
	[ -n "$name" ] || { echo "!! $p is not a submodule" >&2; fail=1; continue; }
	url="$(git -C "$REPO" config -f .gitmodules --get "submodule.$name.url")"
	sha="$(git -C "$REPO" ls-tree HEAD "$p" | awk '$2=="commit" {print $3}')"
	[ -n "$sha" ] || { echo "!! $p: no gitlink in HEAD" >&2; fail=1; continue; }
	if git -C "$TMP" fetch -q --depth 1 --filter=blob:none "$url" "$sha" 2>"$TMP/err" \
	   || git -C "$TMP" fetch -q --depth 1 "$url" "$sha" 2>"$TMP/err"; then
		echo "ok    $p  ${sha:0:12}  $url"
	else
		echo "FAIL  $p  ${sha:0:12}  $url"; sed 's/^/        /' "$TMP/err" | head -3; fail=1
	fi
done
if [ "$fail" = 0 ]; then echo "every pin is public"; else
	echo "!! a pin above is not on its remote: push it, or a clone elsewhere cannot build" >&2; exit 1
fi
