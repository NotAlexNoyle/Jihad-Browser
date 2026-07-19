#!/bin/bash
# Copyright 2026 the Jihad Browser project. Apache-2.0.
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
# Requires: palm-package (webOS SDK, on PATH), rsync, ar, tar.
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../.." && pwd)          # Jihad-Browser repo (or worktree) root
APP="$REPO/app-mochi"                     # in-repo Mochi front-end

# --- external framework sources (bundled at build time, never vendored) --------
# Workspace roots. Override any of the *_SRC vars directly to relocate a source.
WORKSPACE="${WORKSPACE:-/home/notalexnoyle/eclipse-workspace}"
JIHAD_ROOT="${JIHAD_ROOT:-$WORKSPACE/Jihad}"

# Enyo 2 core.
ENYO_SRC="${ENYO_SRC:-$JIHAD_ROOT/mochi-sampler/enyo}"
# Mochi widget library (LG, Apache-2.0).
MOCHI_SRC="${MOCHI_SRC:-$JIHAD_ROOT/mochi}"
# Enyo 2 "layout" library. Resolved from the first candidate that actually holds a
# package.js: the mochi-sampler slot (per the build packet) is an empty dir in this
# checkout, so the matched copy from the webos-stacks Mochi demo assembly is used.
LAYOUT_SRC="${LAYOUT_SRC:-}"
LAYOUT_CANDIDATES=(
	"$LAYOUT_SRC"
	"$JIHAD_ROOT/mochi-sampler/lib/layout"
	"$WORKSPACE/webos-stacks/mochi/lib/layout"
)

# --- output (git-ignored) ------------------------------------------------------
OUT="$HERE/out-mochi"
STAGE="$OUT/staging"                      # palm-package APP_DIR
IPK_OUT="$OUT/ipk"                        # palm-package OUTPUT_DIR
APP_ID="net.riverstonerelay.jihad-browser.mochi"

# Dev-only dirs pruned from the bundled frameworks: never loaded at runtime (no
# package.js depends on them) and they carry the bulk (enyo tools/node_modules,
# per-lib .git and samples). Runtime code, css, fonts, images, and LICENSE stay.
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
rm -rf "$OUT"
mkdir -p "$STAGE" "$IPK_OUT"

# App: appinfo.json, index.html, icons, source/. Exclude any local framework dirs
# and dev docs so the frameworks below are the single source of those trees.
rsync -a "${PRUNE[@]}" --exclude=enyo --exclude=lib --exclude=CLAUDE.md \
	--exclude=README.md "$APP/" "$STAGE/"

mkdir -p "$STAGE/enyo" "$STAGE/lib/layout" "$STAGE/lib/mochi"
rsync -a "${PRUNE[@]}" "$ENYO_SRC/"         "$STAGE/enyo/"
rsync -a "${PRUNE[@]}" "$LAYOUT_RESOLVED/"  "$STAGE/lib/layout/"
rsync -a "${PRUNE[@]}" "$MOCHI_SRC/"        "$STAGE/lib/mochi/"

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

# --- package -------------------------------------------------------------------
echo "=== palm-package ==="
palm-package "$STAGE" -o "$IPK_OUT"

IPK=$(ls -1 "$IPK_OUT/${APP_ID}"_*_all.ipk 2>/dev/null | head -n1)
[ -n "$IPK" ] || die "palm-package did not produce ${APP_ID}_*_all.ipk"

# --- self-inspect: confirm the bundled trees + appinfo id are inside the .ipk ---
echo "=== inspecting $IPK ==="
TMP=$(mktemp -d)
( cd "$TMP" && ar x "$IPK" )
DATA=$(ls "$TMP"/data.tar.* 2>/dev/null | head -n1)
[ -n "$DATA" ] || die "no data.tar.* inside the .ipk"
MANIFEST=$(tar tf "$DATA")
for need in appinfo.json index.html enyo/enyo.js lib/layout/package.js \
            lib/mochi/package.js source/package.js; do
	echo "$MANIFEST" | grep -q "/$need$" || echo "$MANIFEST" | grep -q "^\./.*/$need$" \
		|| die "expected $need missing from .ipk payload"
done
for d in enyo/ lib/layout/ lib/mochi/ source/; do
	echo "$MANIFEST" | grep -q "$d" || die "expected dir $d missing from .ipk payload"
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
