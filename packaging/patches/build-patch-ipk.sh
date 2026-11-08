#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Build a webOS patch package (an .ipk Preware or palm-install can install) from a directory in the shape of the
# community's patch sources:
#
#   packaging/patches/<name>/control/{control,postinst,prerm}   the package metadata and its install/remove scripts
#   packaging/patches/<name>/data/<name>.patch                  the unified diff (.orig/path -> /path, applied with -p1 from /)
#   packaging/patches/<name>/README.md                          what it does, how it was made, how to check it
#
#   packaging/patches/build-patch-ipk.sh <name> [out dir]       -> <out dir>/<Package>_<Version>_all.ipk (default /tmp/ipk)
#
# The .ipk is the ordinary ipkg form (an `ar` archive: debian-binary, control.tar.gz with ./control ./postinst ./prerm,
# data.tar.gz with the files as ./usr/palm/applications/<Package>/…), built fresh with `ar rc` and read back (the
# Jihad variants' packaging follows the same rule). The data paths are RELATIVE TO THE INSTALL ROOT: Preware and
# palm-install run `ipkg -o /media/cryptofs/apps`, which puts ./usr/palm/applications/<Package>/ at
# /media/cryptofs/apps/usr/palm/applications/<Package>/ (a path written as ./media/cryptofs/apps/... lands doubled,
# 2026-11-08). The data goes under the package's own app directory so ipkg removes it with the package; the scripts
# do the system-file work. ipkg -o runs no maintainer script itself (an offline root): Preware's service and
# palm-install run postinst after the install and prerm before the removal.
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
NAME="${1:?usage: build-patch-ipk.sh <name> [out dir]}"; OUT="${2:-/tmp/ipk}"
SRC="$HERE/$NAME"
[ -d "$SRC/control" ] || { echo "no $SRC/control" >&2; exit 2; }
PKG=$(sed -n 's/^Package: *//p' "$SRC/control/control"); VER=$(sed -n 's/^Version: *//p' "$SRC/control/control")
[ -n "$PKG" ] && [ -n "$VER" ] || { echo "control needs Package and Version" >&2; exit 2; }
for t in ar tar gzip md5sum; do command -v "$t" >/dev/null || { echo "$t not on PATH" >&2; exit 2; }; done
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
mkdir -p "$T/c" "$T/d/usr/palm/applications/$PKG"
cp "$SRC/control/control" "$T/c/control"
for s in postinst prerm; do [ -f "$SRC/control/$s" ] && { cp "$SRC/control/$s" "$T/c/$s"; chmod 755 "$T/c/$s"; }; done
[ -d "$SRC/data" ] && cp -R "$SRC/data/." "$T/d/usr/palm/applications/$PKG/"
[ -f "$SRC/README.md" ] && cp "$SRC/README.md" "$T/d/usr/palm/applications/$PKG/"
# the package's own md5sums: what ipkg records, and what a patch's bookkeeping may read
( cd "$T/d" && find . -type f | sed 's|^\./||' | sort | xargs md5sum ) > "$T/c/md5sums"
printf '2.0\n' > "$T/debian-binary"
( cd "$T/c" && tar --owner=0 --group=0 -czf "$T/control.tar.gz" ./control ./md5sums $(ls postinst prerm 2>/dev/null | sed 's|^|./|') )
( cd "$T/d" && tar --owner=0 --group=0 -czf "$T/data.tar.gz" . )
mkdir -p "$OUT"; IPK="$OUT/${PKG}_${VER}_all.ipk"; rm -f "$IPK"
( cd "$T" && ar rc "$IPK" debian-binary control.tar.gz data.tar.gz )
echo "built $IPK"; ar t "$IPK"; tar -tzf <(ar p "$IPK" control.tar.gz); tar -tzf <(ar p "$IPK" data.tar.gz) | grep -v '/$'
