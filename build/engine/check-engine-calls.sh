#!/usr/bin/env bash
# Copyright 2026 NotAlexNoyle. MPL-2.0.
# check-engine-calls.sh — cavekit-engine-update.md R3 (T-385): the engine calls whose positional
# argument lists UXP changes between releases go through render/goanna/EngineCalls.h and nowhere
# else. Fails (exit 1) on a call to one of the wrapped methods in the daemon's code outside the
# wrapper header; comments are not calls. The method list is read from EngineCalls.h itself, so a
# method wrapped later is guarded too.
set -euo pipefail
here="$(cd "$(dirname "$0")/../.." && pwd)"
wrapper="$here/render/goanna/EngineCalls.h"
[ -f "$wrapper" ] || { echo "FAIL: $wrapper missing"; exit 1; }
methods=$(grep -o -- '->[A-Z][A-Za-z]*(' "$wrapper" | sed 's/->//; s/($//' | sort -u)
[ -n "$methods" ] || { echo "FAIL: no wrapped methods found in EngineCalls.h"; exit 1; }
pattern=$(echo "$methods" | paste -sd'|' -)
fail=0
while IFS= read -r f; do
  # strip // comments and the inside of /* */ blocks line by line (good enough: no wrapped call
  # sits on a line that also opens a block comment)
  hits=$(awk -v pat="(->|\\\\.)(${pattern})\\\\(" '
    /\/\*/ { inblock = 1 }
    { line = $0; if (!inblock) { sub(/\/\/.*/, "", line); if (line ~ pat) printf "%d: %s\n", NR, $0 } }
    /\*\// { inblock = 0 }' "$f")
  if [ -n "$hits" ]; then
    echo "FAIL: ${f#$here/} calls a wrapped engine method directly:"; echo "$hits" | sed 's/^/  /'; fail=1
  fi
done < <(find "$here/render" -name '*.cpp' -o -name '*.h' | grep -v '/EngineCalls.h$' | grep -v '/third_party/' | sort)
n=$(grep -rho 'engine_calls::[A-Za-z]*' "$here/render" --include=*.cpp --include=*.h | wc -l)
if [ "$fail" = 0 ]; then
  echo "ok: the wrapped methods ($(echo "$methods" | paste -sd' ' -)) are called only from EngineCalls.h; $n call sites use engine_calls::"
fi
exit $fail
