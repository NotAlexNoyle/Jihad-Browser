#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# The reference checkouts beside this repository brought to their upstreams' heads, and what arrived
# said per checkout (cavekit-atlas-references.md R1, R2). References are read, never built or edited:
# every fetch is a read, the default branch moves by fast-forward only, and a checkout with local
# changes or a diverged branch is reported and left alone. Nothing here touches Jihad's own repository.
#
#   tools/host/update-references.sh                       the four Atlas checkouts in ../Atlas/
#   tools/host/update-references.sh webos-docs ref-forks/lunace-webos-ports   …and these (paths under
#                                                         the workspace root, the folder above this repository, or absolute)
#
# The Atlas checkouts (R1): the app's origin is webOS-ports' repository (GitHub redirects Herrie82's old
# name there), and the two isis descendants carry their 2013 fork parents as a read-only `ancestor`
# remote for comparison. The script puts those remotes right when they are missing and says so.
#
# Afterwards: the commits printed are read for Jihad and recorded in docs/REFERENCES.md (R3).
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
WS="$(cd "$REPO/.." && pwd)"

ATLAS=(Atlas/atlas-browser-app Atlas/BrowserAdapter Atlas/BrowserServer Atlas/atlas-wpe-backend)
APP_ORIGIN=https://github.com/webOS-ports/org.webosports.app.atlas
declare -A ANCESTOR=(
	[Atlas/BrowserAdapter]=https://github.com/woce/BrowserAdapter
	[Atlas/BrowserServer]=https://github.com/isis-project/BrowserServer
)

# The list: the Atlas four, then the arguments (relative to the workspace root, or absolute).
CHECKOUTS=()
for c in "${ATLAS[@]}"; do CHECKOUTS+=("$WS/$c"); done
for a in "$@"; do
	case "$a" in /*) CHECKOUTS+=("$a") ;; *) CHECKOUTS+=("$WS/$a") ;; esac
done

say() { echo; echo "##### $*"; }
rel() { echo "${1#$WS/}"; }

# Every checkout must be a git checkout with a clean tree before anything is fetched (R2).
dirty=()
for c in "${CHECKOUTS[@]}"; do
	[ -d "$c/.git" ] || [ -f "$c/.git" ] || { echo "!! not a git checkout: $c" >&2; exit 2; }
	if [ -n "$(git -C "$c" status --porcelain --untracked-files=no)" ]; then
		dirty+=("$(rel "$c")")
		echo "!! uncommitted changes in $(rel "$c"):" >&2
		git -C "$c" status --porcelain --untracked-files=no | sed 's/^/     /' >&2
	fi
done
if [ ${#dirty[@]} -gt 0 ]; then
	echo "!! refusing: ${#dirty[@]} checkout(s) with uncommitted changes (above); nothing fetched" >&2
	exit 2
fi

# R1: the Atlas remotes as they should be (a local config change, not a network write).
app="$WS/Atlas/atlas-browser-app"
if [ -d "$app" ]; then
	cur="$(git -C "$app" remote get-url origin)"
	if [ "$cur" != "$APP_ORIGIN" ] && [ "$cur" != "$APP_ORIGIN.git" ]; then
		git -C "$app" remote set-url origin "$APP_ORIGIN"
		echo "atlas-browser-app: origin was $cur, now $APP_ORIGIN (the app moved to webOS-ports)"
	fi
fi
for k in "${!ANCESTOR[@]}"; do
	c="$WS/$k"; [ -d "$c" ] || continue
	if ! git -C "$c" remote get-url ancestor >/dev/null 2>&1; then
		git -C "$c" remote add ancestor "${ANCESTOR[$k]}"
		git -C "$c" remote set-url --push ancestor DISABLED
		echo "$k: ancestor remote added (${ANCESTOR[$k]}, read-only): the 2013 fork parent, never merged"
	fi
done

status=0
for c in "${CHECKOUTS[@]}"; do
	name="$(rel "$c")"
	say "$name"
	# The default branch: origin's HEAD as fetched; asked of the remote when the checkout has none recorded.
	def="$(git -C "$c" symbolic-ref -q --short refs/remotes/origin/HEAD 2>/dev/null | sed 's#^origin/##' || true)"
	if [ -z "$def" ]; then
		def="$(git -C "$c" ls-remote --symref origin HEAD 2>/dev/null | sed -n 's#^ref: refs/heads/\([^\t]*\)\tHEAD$#\1#p')"
		[ -n "$def" ] && git -C "$c" remote set-head origin "$def" >/dev/null 2>&1 || true
	fi
	[ -n "$def" ] || { echo "!! $name: cannot tell origin's default branch"; status=1; continue; }

	before_refs="$(git -C "$c" for-each-ref --format='%(refname)' refs/remotes refs/tags | sort)"
	old_remote="$(git -C "$c" rev-parse -q --verify "refs/remotes/origin/$def" 2>/dev/null || true)"
	git -C "$c" fetch --all --tags --prune --quiet
	after_refs="$(git -C "$c" for-each-ref --format='%(refname)' refs/remotes refs/tags | sort)"
	new_remote="$(git -C "$c" rev-parse -q --verify "refs/remotes/origin/$def")"

	# New branches and tags (origin's and the ancestor's alike).
	new_refs="$(comm -13 <(echo "$before_refs") <(echo "$after_refs") | sed 's#^refs/remotes/##; s#^refs/tags/#tag #')"
	gone_refs="$(comm -23 <(echo "$before_refs") <(echo "$after_refs") | sed 's#^refs/remotes/##; s#^refs/tags/#tag #')"

	# The default branch, fast-forward only.
	branch="$(git -C "$c" symbolic-ref -q --short HEAD || echo DETACHED)"
	head_before="$(git -C "$c" rev-parse HEAD)"
	if [ "$branch" != "$def" ]; then
		echo "   on '$branch', not origin's default '$def': left where it is"
		status=1
	elif [ "$head_before" = "$new_remote" ]; then
		:
	elif git -C "$c" merge-base --is-ancestor "$head_before" "$new_remote"; then
		git -C "$c" merge --ff-only --quiet "origin/$def"
	else
		echo "   DIVERGED from origin/$def (local $(git -C "$c" rev-list --count "origin/$def..HEAD") ahead, $(git -C "$c" rev-list --count "HEAD..origin/$def") behind): left alone, not reset"
		status=1
	fi
	head_after="$(git -C "$c" rev-parse HEAD)"

	# What arrived on the default branch (from the head before this run to the new head, newest first).
	if [ -n "$old_remote" ] && [ "$old_remote" != "$new_remote" ]; then
		n="$(git -C "$c" rev-list --count "$old_remote..$new_remote")"
		echo "   $n commit(s) arrived on $def ($(git -C "$c" rev-parse --short "$old_remote") -> $(git -C "$c" rev-parse --short "$new_remote")):"
		git -C "$c" log --format='     %h %cs %s' "$old_remote..$new_remote"
	else
		echo "   nothing new on $def"
	fi
	[ -n "$new_refs" ] && { echo "   new branches/tags:"; echo "$new_refs" | sed 's/^/     /'; }
	[ -n "$gone_refs" ] && { echo "   pruned:"; echo "$gone_refs" | sed 's/^/     /'; }
	if [ "$head_before" != "$head_after" ]; then
		echo "   $branch fast-forwarded $(git -C "$c" rev-parse --short "$head_before") -> $(git -C "$c" rev-parse --short "$head_after")"
	fi
	echo "   head: $(git -C "$c" rev-parse --short HEAD) $(git -C "$c" log -1 --format='%cs %s' | cut -c1-90) [$(git -C "$c" remote get-url origin)]"
done
exit $status
