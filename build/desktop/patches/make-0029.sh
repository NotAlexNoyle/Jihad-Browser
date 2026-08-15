#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Generate build/desktop/patches/0029-npapi-plugin-pull-frame.patch by diffing the LIVE
# third_party/uxp working tree against the pre-0029 baseline in .baseline-pre-0029/.
#
# WHY A BASELINE DIR. The patch series is applied IN PLACE to the shared third_party/uxp tree,
# so by the time you author 0029 the same four files already carry 0008/0017/0020/0022/0024/
# 0026/0028. Diffing against `git show HEAD:` would fold all of those into 0029; diffing against
# the working tree gives nothing. The correct baseline is "HEAD plus every EARLIER patch", which
# is exactly what .baseline-pre-0029/ holds — and what --rebuild-baseline reconstructs.
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
UXP=$(cd "$HERE/../../../third_party/uxp" && pwd)
BASE="$HERE/.baseline-pre-0029"
OUT="$HERE/0029-npapi-plugin-pull-frame.patch"
FILES="PPluginInstance.ipdl PluginInstanceChild.h PluginInstanceChild.cpp PluginInstanceParent.cpp"
# Every patch that touches one of $FILES, in the order the build's *.patch glob applies them.
EARLIER="0008 0017 0020 0022 0024 0026 0028"

if [ "${1:-}" = "--rebuild-baseline" ]; then
  rm -rf "$BASE"; mkdir -p "$BASE/dom/plugins/ipc"
  # PluginMessageUtils.h is seeded only so 0008's hunk for it lands somewhere instead of
  # prompting; it is not part of the 0029 diff.
  for f in $FILES PluginMessageUtils.h; do
    (cd "$UXP" && git show "HEAD:dom/plugins/ipc/$f") > "$BASE/dom/plugins/ipc/$f"
  done
  for p in $EARLIER; do
    (cd "$BASE" && patch -p1 --batch --forward --no-backup-if-mismatch < "$HERE/$p"*.patch) \
      >/dev/null 2>&1 || true
  done
  rm -f "$BASE/dom/plugins/ipc/PluginMessageUtils.h" "$BASE/dom/plugins/ipc/JihadNPNInterpose.cpp" "$BASE"/dom/plugins/ipc/*.orig
  echo "baseline rebuilt: $BASE"
  exit 0
fi

[ -d "$BASE/dom/plugins/ipc" ] || { echo "ERROR: no baseline — run $0 --rebuild-baseline" >&2; exit 1; }

: > "$OUT.body"
for f in $FILES; do
  diff -u --label "a/dom/plugins/ipc/$f" --label "b/dom/plugins/ipc/$f" \
    "$BASE/dom/plugins/ipc/$f" "$UXP/dom/plugins/ipc/$f" >> "$OUT.body" || true
done
[ -s "$OUT.body" ] || { echo "ERROR: no differences — nothing to capture" >&2; rm -f "$OUT.body"; exit 1; }

# Header first (patch(1) ignores everything before the first ---), then the hunks.
{ [ -f "$HERE/0029.header.txt" ] && cat "$HERE/0029.header.txt"; cat "$OUT.body"; } > "$OUT"
rm -f "$OUT.body"

# Believe the artifact: the patch must apply to the baseline it was cut from, and must be
# detected as already-applied against the live tree (which is what the build's dry-run gate
# will see). Both checks, or this script fails rather than shipping an unverified patch.
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
cp -r "$BASE/dom" "$T/"
(cd "$T" && patch -p1 --batch --forward --dry-run < "$OUT" >/dev/null) \
  || { echo "ERROR: $OUT does not apply to the baseline" >&2; exit 1; }
if (cd "$UXP" && patch -p1 --batch --forward --dry-run < "$OUT" >/dev/null 2>&1); then
  echo "ERROR: $OUT still applies to the live tree — the tree does not carry these edits" >&2
  exit 1
fi
echo "wrote $OUT ($(grep -c '^@@' "$OUT") hunks); applies to baseline, already-applied on the tree"
