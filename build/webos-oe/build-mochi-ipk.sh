#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Build the Mochi (Enyo 2) UI variant .ipk:
#     net.riverstonerelay.jihad-browser.mochi_<version>_all.ipk
#
# The in-repo front-end lives in app-mochi/. Its index.html loads the framework
# from these paths (relative to the app root):
#     enyo/enyo.js            <- Enyo 2 core
#     lib/layout/package.js   <- Enyo 2 "layout" library (FittableRows/Panels/List)
#     lib/mochi/package.js    <- Mochi widget library (LG, Apache-2.0)
#     source/package.js       <- the app itself (in-repo)
#
# Enyo 2 core, the layout library, and Mochi are NOT vendored in this repo; they
# are BUNDLED at build time from external source trees (see the *_SRC vars below,
# each overridable via the environment). This script stages app-mochi/ + those
# three trees into a matching directory layout, then runs palm-package to emit
# the .ipk. Both the staging tree and the .ipk land under out-mochi/ (git-ignored
# by /build/**/out-*/ and the global *.ipk rule), so nothing bundled is committed.
#
# Usage:  build/webos-oe/build-mochi-ipk.sh
#         build/webos-oe/build-mochi-ipk.sh --stage-only <dir>
#
# --stage-only stages app-mochi/ + the three bundled framework trees into <dir> and STOPS
# (no palm-package, no .ipk). It exists so build-variant-ipk.sh — which has to add the
# deviceroot runtime bundle and repack the .ipk to inject postinst/prerm — reuses THIS
# script's framework resolution/pruning/provenance/QA instead of duplicating it. There is
# exactly one implementation of "what goes in a Mochi app tree", and it is here.
#
# Requires: palm-package (webOS SDK, on PATH), rsync, ar, tar.
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../.." && pwd)          # Jihad-Browser repo (or worktree) root
APP="$REPO/app-mochi"                     # in-repo Mochi front-end

STAGE_ONLY=""
if [ "${1:-}" = "--stage-only" ]; then
	[ -n "${2:-}" ] || { echo "ERROR: --stage-only needs a directory" >&2; exit 2; }
	mkdir -p "$2"
	STAGE_ONLY=$(cd "$2" && pwd)
fi

# --- UI framework sources: in-repo submodules (third_party/), bundled at build ----
# Populate with `git submodule update --init`. Override any *_SRC to relocate a source.
# Enyo 2 core (github.com/webOSArchive/mochi-sampler submodule).
ENYO_SRC="${ENYO_SRC:-$REPO/third_party/mochi-sampler/enyo}"
# Mochi widget library (github.com/webOSArchive/mochi submodule; LG, Apache-2.0).
MOCHI_SRC="${MOCHI_SRC:-$REPO/third_party/mochi}"
# Enyo 2 "layout" library (github.com/enyojs/layout @ 2.5.2 submodule). mochi-sampler's own
# lib/layout slot is an empty placeholder, so the layout submodule supplies it. Legacy
# fallbacks (a sibling mochi-sampler checkout / a webos-stacks assembly) are kept last.
LAYOUT_SRC="${LAYOUT_SRC:-}"
LAYOUT_CANDIDATES=(
	"$LAYOUT_SRC"
	"$REPO/third_party/enyo-layout"
	"${JIHAD_ROOT:-$REPO/..}/mochi-sampler/lib/layout"
	"${WORKSPACE:-$REPO/../..}/webos-stacks/mochi/lib/layout"
)

# --- output (git-ignored) ------------------------------------------------------
OUT="$HERE/out-mochi"
STAGE="${STAGE_ONLY:-$OUT/staging}"       # palm-package APP_DIR (or the caller's --stage-only dir)
IPK_OUT="$OUT/ipk"                        # palm-package OUTPUT_DIR
APP_ID="net.riverstonerelay.jihad-browser.mochi"

# Dev-only dirs pruned from the bundled frameworks: never loaded at runtime (no
# package.js depends on them) and they carry the bulk (enyo tools/node_modules,
# per-lib .git and samples). Runtime code, css, fonts, images, and LICENSE stay.
# KNOWN DEBT (codex F-369): the trees ship unminified and their root package.js
# graphs load every module (~200 JS files). A minify/tree-shake pass (enyo
# deploy tooling, or pruning to the app's real depends() closure once T-053
# fixes the final control set) is deferred until the parity port lands.
PRUNE=(--exclude=.git --exclude=node_modules --exclude=tools
       --exclude=docs --exclude=samples --exclude=minify)

die() { echo "ERROR: $*" >&2; exit 1; }

# --- resolve sources -----------------------------------------------------------
[ -f "$APP/appinfo.json" ] || die "app-mochi not found at $APP"
[ -f "$ENYO_SRC/enyo.js" ] || die "Enyo core (enyo.js) not found at ENYO_SRC=$ENYO_SRC"
[ -f "$MOCHI_SRC/package.js" ] || die "Mochi lib (package.js) not found at MOCHI_SRC=$MOCHI_SRC"

LAYOUT_RESOLVED=""
for c in "${LAYOUT_CANDIDATES[@]}"; do
	[ -n "$c" ] && [ -f "$c/package.js" ] && { LAYOUT_RESOLVED="$c"; break; }
done
[ -n "$LAYOUT_RESOLVED" ] || die "layout lib (package.js) not found in: ${LAYOUT_CANDIDATES[*]}"

echo "=== sources ==="
echo "  app-mochi : $APP"
echo "  enyo core : $ENYO_SRC"
echo "  layout    : $LAYOUT_RESOLVED"
echo "  mochi lib : $MOCHI_SRC"

# --- stage ---------------------------------------------------------------------
# In --stage-only mode the caller owns the staging dir (and out-mochi/ must be left alone).
if [ -n "$STAGE_ONLY" ]; then rm -rf "$STAGE"; else rm -rf "$OUT"; fi
mkdir -p "$STAGE" "$IPK_OUT"

# App: appinfo.json, index.html, icons, source/. Exclude any local framework dirs
# and dev docs so the frameworks below are the single source of those trees.
rsync -a "${PRUNE[@]}" --exclude=enyo --exclude=lib --exclude=CLAUDE.md \
	--exclude=README.md "$APP/" "$STAGE/"

mkdir -p "$STAGE/enyo" "$STAGE/lib/layout" "$STAGE/lib/mochi"
rsync -a "${PRUNE[@]}" "$ENYO_SRC/"         "$STAGE/enyo/"
rsync -a "${PRUNE[@]}" "$LAYOUT_RESOLVED/"  "$STAGE/lib/layout/"
rsync -a "${PRUNE[@]}" "$MOCHI_SRC/"        "$STAGE/lib/mochi/"

# License + attribution ship IN the ipk (mochi-ui R5 / codex F-368): app source
# headers point at LICENSE, and the bundled Enyo 2 + Mochi attribution lives in
# NOTICE. Both go to the app root of the package.
install -m 0644 "$REPO/LICENSE" "$STAGE/LICENSE"
install -m 0644 "$REPO/NOTICE"  "$STAGE/NOTICE"

# Provenance manifest (codex F-370): record WHERE each bundled tree came from and
# its git revision (when the source is a git checkout) so a given ipk's framework
# inputs are identifiable. Full checksum ENFORCEMENT (pinning expected revs) is
# deliberately not done yet — sources are local sibling checkouts, not fetches.
{
	echo "# Bundled framework provenance — generated by build-mochi-ipk.sh"
	echo "built: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
	for entry in "enyo:$ENYO_SRC" "layout:$LAYOUT_RESOLVED" "mochi:$MOCHI_SRC"; do
		name=${entry%%:*}; src=${entry#*:}
		rev=$(git -C "$src" rev-parse HEAD 2>/dev/null || echo "not-a-git-checkout")
		dirty=$(git -C "$src" status --porcelain 2>/dev/null | grep -q . && echo "-dirty" || true)
		# Workspace-relative path only — absolute host paths would leak the
		# builder's username/layout into every distributed ipk (codex F-392).
		echo "$name: ${src#"${WORKSPACE:-}"/} @ $rev$dirty"
	done
} > "$STAGE/BUNDLED-VERSIONS"
cat "$STAGE/BUNDLED-VERSIONS"

# --- sanity: every <script src> in index.html resolves in the staging tree -----
echo "=== resolving index.html script srcs ==="
missing=0
while IFS= read -r src; do
	if [ -f "$STAGE/$src" ]; then
		echo "  OK   $src"
	else
		echo "  MISS $src"; missing=1
	fi
done < <(grep -oE 'src="[^"]+"' "$STAGE/index.html" | sed -e 's/^src="//' -e 's/"$//')
[ "$missing" -eq 0 ] || die "index.html references a script that is not staged"

# --- --stage-only: hand the staged app tree back to the caller and stop ---------
# (build-variant-ipk.sh adds deviceroot/ + LICENSE/NOTICE and does the palm-package +
# control-script repack itself; everything above is the part that must not be duplicated.)
if [ -n "$STAGE_ONLY" ]; then
	echo "=== staged only (no .ipk): $STAGE ==="
	exit 0
fi

# --- package -------------------------------------------------------------------
echo "=== palm-package ==="
palm-package "$STAGE" -o "$IPK_OUT"

# `|| true` keeps set -e/pipefail from killing the script on a missing glob so
# the die() diagnostic is actually reachable (codex F-373).
IPK=$(ls -1 "$IPK_OUT/${APP_ID}"_*_all.ipk 2>/dev/null | head -n1 || true)
[ -n "$IPK" ] || die "palm-package did not produce ${APP_ID}_*_all.ipk"

# --- self-inspect: confirm the bundled trees + appinfo id are inside the .ipk ---
echo "=== inspecting $IPK ==="
TMP=$(mktemp -d)
( cd "$TMP" && ar x "$IPK" )
DATA=$(ls "$TMP"/data.tar.* 2>/dev/null | head -n1 || true)
[ -n "$DATA" ] || die "no data.tar.* inside the .ipk"
MANIFEST=$(tar tf "$DATA")
# Captured + matched without a pipe. `echo "$MANIFEST" | grep -q …` is unsafe here: -q exits at
# the first match, echo upstream takes SIGPIPE, and `set -o pipefail` promotes that to a failed
# check — so a payload that DOES contain the file gets reported missing, non-deterministically,
# depending on where in the manifest the match lands. Bit the .ipk verifier for real on
# 2026-07-31 once the payload grew past ~2400 entries.
for need in appinfo.json index.html LICENSE NOTICE BUNDLED-VERSIONS \
            enyo/enyo.js lib/layout/package.js \
            lib/mochi/package.js source/package.js; do
	case $'\n'"$MANIFEST"$'\n' in
		*"/$need"$'\n'*) : ;;
		*) die "expected $need missing from .ipk payload" ;;
	esac
done
for d in enyo/ lib/layout/ lib/mochi/ source/; do
	case "$MANIFEST" in
		*"$d"*) : ;;
		*) die "expected dir $d missing from .ipk payload" ;;
	esac
done
# appinfo id inside the packaged appinfo.json
AIFILE=$(echo "$MANIFEST" | grep '/appinfo.json$' | head -n1)
tar xf "$DATA" -C "$TMP" "$AIFILE"
grep -q "\"$APP_ID\"" "$TMP/$AIFILE" || die "appinfo.json in .ipk does not declare id $APP_ID"
rm -rf "$TMP"

echo "=== OK ==="
echo "  ipk     : $IPK"
echo "  size    : $(( $(stat -c %s "$IPK") / 1024 )) KiB"
echo "  payload : $(echo "$MANIFEST" | grep -c . ) entries"
