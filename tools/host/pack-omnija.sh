#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# pack-omnija.sh <bundle dir> <out dir> : cavekit-lowpower-uxp.md R5, T-559 — the GRE packed into omni.ja (the research's
# "2126 loose files today": fewer opens and stats on the TouchPad's eMMC at start). A COPY of the assembled device bundle
# (build/webos-oe/make-device-bundle.sh) with the engine's own resources moved into $out/omni.ja: chrome/, the engine's
# components (Jihad's own jihad* stay loose), modules/, res/, defaults/ and goanna.js, with the manifest lines that
# register them; the loose chrome.manifest keeps only Jihad's lines (its components, the branding, the settings page).
# The engine reads both: the GRE directory's chrome.manifest and the jar's (nsComponentManager.cpp), and with a jar its
# default prefs come from the jar alone (Preferences.cpp). Stored, not deflated (zip -0): the Scorpion's inflate is the
# cost this lever must not buy. Dictionaries, hyphenation, the ICU data and the libraries stay loose.
set -euo pipefail
IN=${1:?bundle}; OUT=${2:?out}
[ -f "$IN/chrome.manifest" ] && [ -f "$IN/goanna.js" ] || { echo "!! $IN is not an assembled bundle" >&2; exit 2; }
rm -rf "$OUT"; cp -a "$IN" "$OUT"
cd "$OUT"
STAGE=$(mktemp -d); trap 'rm -rf "$STAGE"' EXIT
# the engine's own manifest lines (not Jihad's) go into the jar's chrome.manifest; Jihad's stay in the loose one
grep -v '^manifest jihad-\|^manifest components/jihad-' chrome.manifest > "$STAGE/chrome.manifest"
grep '^manifest jihad-\|^manifest components/jihad-' chrome.manifest > chrome.manifest.jihad
mv chrome.manifest.jihad chrome.manifest
mkdir -p "$STAGE/components"
for f in components/*; do
	case "$(basename "$f")" in jihad*) ;; *) mv "$f" "$STAGE/components/";; esac
done
mv chrome modules res defaults goanna.js "$STAGE/"
( cd "$STAGE" && zip -q -0 -X -r "$OUT/omni.ja" chrome.manifest goanna.js chrome components modules res defaults )
echo "packed: $(unzip -l omni.ja | tail -1 | awk '{print $2}') files into omni.ja ($(du -h omni.ja | cut -f1)); loose now: $(find . -type f | wc -l) files"
echo "loose chrome.manifest: $(wc -l < chrome.manifest) lines (Jihad's)"
