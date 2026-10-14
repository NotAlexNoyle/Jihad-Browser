#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# The source tarballs crosstool-NG needs (tarballs.manifest), fetched by the repository and checked
# by sha256 into the directory ct-ng reads first (CT_LOCAL_TARBALLS_DIR, out-toolchain/src/), BEFORE
# ct-ng runs: ct-ng 1.25.0's own mirror lists have aged (zlib.net keeps only its newest release;
# the clean-clone VM run of 2026-10-14 died there), and the toolchain must be rebuildable from the
# repository on any host (R11). Several URLs per tarball, the sha256 decides; a file already present
# and matching is left alone.
#
#   toolchain/fetch-tarballs.sh [dir]        default: out-toolchain/src beside this script
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
. "$HERE/../fetch-lib.sh"
DIR="${1:-$HERE/out-toolchain/src}"
mkdir -p "$DIR"
n=0 fail=0
while IFS=$'\t' read -r file sum size rest; do
  case "$file" in ''|'#'*) continue;; esac
  n=$((n + 1))
  if [ -f "$DIR/$file" ] && [ "$(sha256sum "$DIR/$file" | cut -d' ' -f1)" = "$sum" ]; then continue; fi
  echo "fetching $file"
  ok=0
  for url in $rest; do
    if fetch_url "$url" "$DIR/$file.part" "$sum"; then mv "$DIR/$file.part" "$DIR/$file"; ok=1; break; fi
    echo "   not from $url" >&2
  done
  [ "$ok" = 1 ] || { echo "!! $file: no candidate URL gave the pinned file" >&2; rm -f "$DIR/$file.part"; fail=$((fail + 1)); }
done < "$HERE/tarballs.manifest"
echo "tarballs: $n in the manifest, $fail missing"
[ "$fail" -eq 0 ]
