#!/bin/sh
# Copyright 2026 NotAlexNoyle. MPL-2.0.
# install-components.sh DEST REPO — cavekit-engine-update.md R6 (T-387): Jihad's own XPCOM
# components into an engine dist, the one script both the device bundle (make-device-bundle.sh)
# and the desktop dist (build/desktop/build-goanna.sh) use. Every render/goanna/components/*.manifest
# is copied with the JS files its "component" lines name and registered from DEST/chrome.manifest
# (idempotent); the engine shim the components import goes to DEST/modules/JihadEngine.jsm. A
# manifest naming a file that is not there fails the build, not the device.
set -eu
DEST="${1:?usage: install-components.sh DEST REPO}"
SRC="${2:?usage: install-components.sh DEST REPO}/render/goanna"
mkdir -p "$DEST/components" "$DEST/modules"
touch "$DEST/chrome.manifest"
n=0
for m in "$SRC"/components/*.manifest; do
	[ -f "$m" ] || continue
	name=$(basename "$m")
	for js in $(awk '$1 == "component" { print $3 }' "$m" | sort -u); do
		[ -s "$SRC/components/$js" ] || { echo "ERROR: $name names components/$js, which is missing" >&2; exit 1; }
		cp -L "$SRC/components/$js" "$DEST/components/$js"
	done
	cp -L "$m" "$DEST/components/$name"
	grep -q "^manifest components/$name\$" "$DEST/chrome.manifest" || echo "manifest components/$name" >> "$DEST/chrome.manifest"
	n=$((n + 1))
done
[ "$n" -gt 0 ] || { echo "ERROR: no component manifests in $SRC/components" >&2; exit 1; }
cp -L "$SRC/modules/JihadEngine.jsm" "$DEST/modules/JihadEngine.jsm"
echo "  components: $n manifests registered, modules/JihadEngine.jsm"
