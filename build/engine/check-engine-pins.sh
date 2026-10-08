#!/bin/bash
# Copyright 2026 NotAlexNoyle. Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Every engine pin and version string in the repo agrees with build/engine/engine-version.env.
# Run after changing the engine (docs/ENGINE.md); exit 1 names each place that disagrees.
set -uo pipefail
HERE=$(cd "$(dirname "$0")" && pwd); REPO=$(cd "$HERE/../.." && pwd)
# shellcheck disable=SC1091
. "$HERE/engine-version.env"
bad=0
say() { printf '  %-4s %s\n' "$1" "$2"; }
need() {  # need <file> <fixed string that must be in it> <what it is>
  if grep -qF -- "$2" "$REPO/$1" 2>/dev/null; then say ok "$3 ($1)"; else say FAIL "$3: $1 does not say '$2'"; bad=1; fi
}
echo "Engine pins and version strings vs build/engine/engine-version.env"
gl=$(git -C "$REPO" ls-tree HEAD third_party/uxp | awk '{print $3}')
if [ "$gl" = "$UXP_PIN" ]; then say ok "third_party/uxp gitlink is UXP_PIN"; else say FAIL "gitlink is $gl, UXP_PIN is $UXP_PIN"; bad=1; fi
co=$(git -C "$REPO/third_party/uxp" rev-parse HEAD 2>/dev/null)
if [ "$co" = "$UXP_PIN" ]; then say ok "third_party/uxp is checked out at UXP_PIN"; else say WARN "third_party/uxp is checked out at $co (git submodule update?)"; fi
need build/webos-oe/recipes-jihad/goanna/goanna_1.0.bb "SRCREV = \"$UXP_PIN\"" "OE recipe SRCREV"
if git -C "$REPO/third_party/uxp" merge-base --is-ancestor "$UXP_BASE" "$UXP_PIN" 2>/dev/null; then
  say ok "UXP_BASE is an ancestor of UXP_PIN"; else say FAIL "UXP_BASE is not an ancestor of UXP_PIN"; bad=1; fi
case "$UXP_BASE" in "$UXP_SHORT"*) say ok "UXP_SHORT is UXP_BASE's prefix";; *) say FAIL "UXP_SHORT $UXP_SHORT is not UXP_BASE's prefix"; bad=1;; esac
ua="Goanna/$GOANNA_VERSION UXP/$UXP_SHORT"
need render/goanna/JihadUserAgent.h "$ua" "user agent"
need app/source/StartPage.js "$ua" "Enyo start page"
need app-mochi/source/JihadBrowser.js "$ua" "Mochi start page"
need app-mojo/start.html "$ua" "Mojo start page"
need build/webos-oe/make-device-bundle.sh "$ICU_DATA" "ICU data file (bundler)"
need packaging/gen-variant-scripts.sh "$ICU_DATA" "ICU data file (launch scripts)"
if [ -e "$REPO/third_party/uxp/config/external/icu/data/$ICU_DATA" ]; then say ok "the engine ships $ICU_DATA"; else say FAIL "the engine tree has no $ICU_DATA"; bad=1; fi
[ "$bad" = 0 ] && echo "all agree" || echo "DISAGREEMENT — see FAIL lines"
exit $bad
