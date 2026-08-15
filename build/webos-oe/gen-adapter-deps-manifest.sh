#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Generate / verify the manifest for adapter-deps — the fourth prebuilt input the OE build
# consumes but does not build (cavekit-device-build.md R3, prebuilt-inputs carve-out).
#
# WHY THIS EXISTS: adapter-deps/ is entirely git-ignored, so a clean clone gets NOTHING of
# it, and until now nothing in the repo recorded what "it" even was. This records every
# piece with a sha256 and a re-obtain procedure, which is what the R3 criterion asks for.
#
# WHAT IS IN adapter-deps, and it is FOUR different kinds of thing, not one:
#   1. staging/lib/*.so*     ARM shared objects LIFTED OFF A TOUCHPAD. Not redistributable,
#                            not rebuildable here. Re-obtained with novacom (procedure in
#                            the manifest). These are what the adapter links against.
#   2. staging/include/      NPAPI headers. Four of the five are byte-identical to the
#                            npapi-headers checkout; nppalmdefs.h is Palm-only.
#   3. qt4-extract/          Qt4 HEADERS. NOT a device drop — see below.
#   4. libpbnjson/ yajl/     Public git checkouts, pinned by tag.
#      npapi-headers/
#
# CORRECTION THIS SCRIPT ESTABLISHED (2026-08-15): qt4-extract had been described as an HP
# build off a TouchPad. It is not. It is three stock DEBIAN JESSIE armel packages
# (qt4-x11 4:4.8.6+...+deb8u1) unpacked, proven by extracting them from archive.debian.org
# and byte-comparing. So it is re-obtainable exactly like the Jessie sysroot, and only the
# staging/lib binaries are genuinely device-gated. Note the deliberate version SKEW that
# results and that the build depends on: headers are Qt 4.8.6 (Debian), the libraries
# linked against are Qt 4.8.0 (the device's own). Qt 4.8.x keeps binary compatibility
# across the point releases, which is why this works.
#
# Usage:
#   gen-adapter-deps-manifest.sh            regenerate the manifest from what is on disk
#   gen-adapter-deps-manifest.sh --check    verify on-disk files against the manifest
#   gen-adapter-deps-manifest.sh --check-qt4-debs
#                                           re-download the three Debian qt4 .debs and
#                                           verify the pinned sha256s still hold
#
# Output is DETERMINISTIC: no timestamps, C-collation sort, so a regeneration with no input
# change produces a byte-identical file.
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
DEPS="$HERE/adapter-deps"
MANIFEST="$HERE/adapter-deps.manifest"
MIRROR="${DEBIAN_ARCHIVE_MIRROR:-http://archive.debian.org/debian}"

# ---- the three Debian jessie packages qt4-extract/ was unpacked from --------------------
# sha256 measured 2026-08-15 by downloading each from archive.debian.org; the extract was
# then proven to BE these packages by byte-comparing libQtCore.so.4.8.6 (from libqtcore4)
# and QtCore/qglobal.h (from libqt4-dev) against the tree — both identical.
QT4_SRC="qt4-x11"
QT4_VER="4.8.6+git64-g5dc8b2b+dfsg-3+deb8u1"
QT4_EPOCH_VER="4:4.8.6+git64-g5dc8b2b+dfsg-3+deb8u1"
QT4_DEBS="\
libqt4-dev|55bec20d00137291967fd0c2523aebf352ea3021e6d923ede67e71314d1396b8|890012
libqtcore4|564e1854cdb10722ae9d180645180bca425bce2ec1d8ffd31a6f1ac1c8b706b9|1420836
libqtgui4|3b6102b008a5cbe434f2394791e81452b20bb349150d4e5dc8abd0ecf8fda28a|3613208"

# ---- what each staging/lib binary IS ----------------------------------------------------
# filename|upstream version|device rootfs path|note
# The build-tree strings quoted in "note" are literally inside the binaries — they are HP's
# own OE build paths and are the strongest provenance available without a device in hand.
STAGING_LIBS="\
libQtCore.so.4.8.0|Qt 4.8.0 (HP nova qt4-4.8.0-83)|/usr/lib/libQtCore.so.4|carries /home/reviewdaemon/projects/nova/oe/BUILD-topaz/work/qt4-4.8.0-83
libQtGui.so.4.8.0|Qt 4.8.0 (HP nova)|/usr/lib/libQtGui.so.4|carries BUILD-topaz build paths
libQtNetwork.so.4.8.0|Qt 4.8.0 (HP nova)|/usr/lib/libQtNetwork.so.4|carries BUILD-topaz build paths
libpbnjson_c.so|pbnjson 1.0.1-38 (submission-38)|/usr/lib/libpbnjson_c.so|carries .../BUILD-topaz/work/pbnjson-1.0.1-38 — NEWER than the submissions/10 headers built against
libpbnjson_cpp.so|pbnjson 1.0.1-38 (submission-38)|/usr/lib/libpbnjson_cpp.so|carries .../BUILD-topaz/work/pbnjson-1.0.1-38 — NEWER than the submissions/10 headers built against
libyajl.so.1|yajl 1.0.7|/usr/lib/libyajl.so.1|SONAME libyajl.so.1, internal version string libyajl.so.1.0.7 — OLDER than the 1.0.12 checkout
libpng12.so.0|libpng 1.2.44 (2010-06-26)|/usr/lib/libpng12.so.0|version string read from the binary"

sha_of() { sha256sum "$1" | cut -d' ' -f1; }

# Deterministic digest of a whole directory tree: sha256 over a C-sorted listing of
# "<sha256|symlink target>\t<relative path>" for every entry. Independent of mtimes,
# inode order and the traversal order the filesystem happens to hand back.
tree_digest() {
  local root="$1"
  {
    while IFS= read -r p; do
      printf '%s\t%s\n' "$(sha_of "$root/$p")" "$p"
    done < <(cd "$root" && find . -type f | LC_ALL=C sort)
    while IFS= read -r p; do
      printf 'symlink:%s\t%s\n' "$(readlink "$root/$p")" "$p"
    done < <(cd "$root" && find . -type l | LC_ALL=C sort)
  } | sha256sum | cut -d' ' -f1
}

tree_counts() {
  local root="$1" f l
  f="$(cd "$root" && find . -type f | wc -l)"
  l="$(cd "$root" && find . -type l | wc -l)"
  printf '%s files, %s symlinks' "$f" "$l"
}

git_pin() {
  # $1 = checkout dir; prints "commit<TAB>describe<TAB>remote" or "" if not a checkout
  local d="$1"
  [ -e "$d/.git" ] || return 0
  printf '%s\t%s\t%s' \
    "$(git -C "$d" rev-parse HEAD)" \
    "$(git -C "$d" describe --tags --always 2>/dev/null || echo '-')" \
    "$(git -C "$d" config --get remote.origin.url 2>/dev/null || echo '-')"
}

generate() {
  [ -d "$DEPS" ] || { echo "!! no $DEPS — nothing to manifest" >&2; exit 2; }

  {
    cat <<'HDR'
# adapter-deps — prebuilt input manifest
#
# The NPAPI BrowserAdapter is built by build-adapter-pdk.sh (PDK gcc 4.3.3) and
# build-adapter-arm.sh (crosstool gcc 9.4). Both link against the libraries and headers in
# build/webos-oe/adapter-deps/, which is git-ignored in full. This file is the declared,
# checksummed form of that directory: cavekit-device-build.md R3 requires every prebuilt
# input to be obtainable from a clean clone by an in-repo recipe or a documented procedure.
#
# THE SPLIT THAT MATTERS. Only ONE of the four parts below is genuinely device-gated:
#
#   part                        obtainable from a clean clone?
#   staging/lib/*.so*           NO — needs a TouchPad. Procedure below; not redistributable.
#   staging/include/webkit/     MOSTLY — 4 of 5 headers are the npapi-headers checkout.
#                               nppalmdefs.h is Palm-only and travels with the device drop.
#   qt4-extract/                YES — three stock Debian jessie .debs, pinned below.
#   libpbnjson/ yajl/ npapi-headers/
#                               YES — public git, pinned by tag+commit below.
#
# ============================================================================
# PROCEDURE: RE-OBTAIN staging/lib FROM A TOUCHPAD
# ============================================================================
# These are HP's own builds off the device rootfs (webOS 3.0.5, ARMv7 softfp, glibc 2.8).
# They cannot be vendored — they are HP proprietary — and they cannot be rebuilt, because
# nothing reproduces HP's "nova" OE tree. With a device attached over novacom:
#
#   mkdir -p build/webos-oe/adapter-deps/staging/lib
#   cd build/webos-oe/adapter-deps/staging/lib
#   for f in libQtCore.so.4 libQtGui.so.4 libQtNetwork.so.4 \
#            libpbnjson_c.so libpbnjson_cpp.so libyajl.so.1 libpng12.so.0; do
#     novacom get file:///usr/lib/$f > $f
#   done
#
# Then rename each to the versioned name in the table below (the device ships the SONAME
# path; the versioned filename is what the linker table here records) and recreate the
# development symlinks listed in the SYMLINKS section — `ld -lQtGui` needs libQtGui.so to
# exist, and the device rootfs has no -dev symlinks. Verify with --check afterwards: a
# device that is not webOS 3.0.5 will produce different checksums, and that is the point.
#
# CAVEAT, stated rather than glossed: the /usr/lib paths above are the standard webOS 3
# locations for these libraries and are how the set was originally obtained, but they were
# NOT re-confirmed against hardware when this manifest was written (no device was attached).
# If a path is wrong, `novacom get` fails loudly rather than silently producing a bad file.
# [human-review on device]
#
# ============================================================================
# PROCEDURE: RE-OBTAIN qt4-extract FROM archive.debian.org
# ============================================================================
# Debian 8 "jessie" is end-of-life, so these versions are frozen rather than moving under
# security updates. For each package in the QT4-DEBIAN-PACKAGES table:
#
#   curl -fsSLO <url>
#   ar p <pkg>.deb data.tar.xz | tar -C build/webos-oe/adapter-deps/qt4-extract -xJf -
#
# ============================================================================
# PROCEDURE: RE-OBTAIN THE GIT CHECKOUTS
# ============================================================================
# For each row of GIT-CHECKOUTS: git clone <remote> <dir> && git -C <dir> checkout <commit>.
# Pin by COMMIT, not by tag — openwebos tags are mutable in principle and the commit is not.
#
# Regenerate with: build/webos-oe/gen-adapter-deps-manifest.sh
# Verify on-disk:  build/webos-oe/gen-adapter-deps-manifest.sh --check
HDR
    printf '#\n'

    # ---- device ARM shared objects ----
    cat <<'S1'
# ============================================================================
# SECTION: DEVICE-ARM-SHARED-OBJECTS  (staging/lib, regular files)
# TSV: filename  sha256  size_bytes  soname  upstream_version  device_path  note
# ============================================================================
S1
    local row fn upv devp note p sum size soname
    while IFS='|' read -r fn upv devp note; do
      [ -n "$fn" ] || continue
      p="$DEPS/staging/lib/$fn"
      if [ ! -f "$p" ]; then
        printf '# MISSING\t%s\t(not on disk when this manifest was generated)\n' "$fn"
        continue
      fi
      sum="$(sha_of "$p")"
      size="$(stat -c %s "$p")"
      soname="$(readelf -d "$p" 2>/dev/null | sed -n 's/.*Library soname: \[\(.*\)\]/\1/p' | head -1)"
      [ -n "$soname" ] || soname='-'
      printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$fn" "$sum" "$size" "$soname" "$upv" "$devp" "$note"
    done <<< "$STAGING_LIBS"

    # ---- symlinks ----
    printf '#\n'
    cat <<'S2'
# ============================================================================
# SECTION: SYMLINKS  (staging/lib — created HERE for the linker, not device artifacts)
# The device rootfs ships no -dev symlinks; `-lQtGui` needs libQtGui.so to resolve.
# TSV: link  target
# ============================================================================
S2
    if [ -d "$DEPS/staging/lib" ]; then
      while IFS= read -r l; do
        printf '%s\t%s\n' "$l" "$(readlink "$DEPS/staging/lib/$l")"
      done < <(cd "$DEPS/staging/lib" && find . -maxdepth 1 -type l -printf '%f\n' | LC_ALL=C sort)
    fi

    # ---- npapi headers ----
    printf '#\n'
    cat <<'S3'
# ============================================================================
# SECTION: NPAPI-HEADERS  (staging/include/webkit/npapi)
# origin=npapi-headers means byte-identical to the pinned npapi-headers checkout, so a
# clean clone regenerates it. origin=palm-only means it exists nowhere else in this repo
# and travels with the device drop.
# TSV: path  sha256  size_bytes  origin
# ============================================================================
S3
    if [ -d "$DEPS/staging/include" ]; then
      while IFS= read -r h; do
        local hp match origin
        hp="$DEPS/staging/include/$h"
        origin='palm-only'
        # Compare against EVERY same-named file in the checkout, not just the first. The
        # checkout carries a debhelper staging copy (debian/npapiheaders/usr/include/) that
        # sorts ahead of the real header and does NOT match — taking only the first hit
        # misfiled npapi.h as palm-only.
        while IFS= read -r match; do
          [ -n "$match" ] || continue
          if cmp -s "$hp" "$match"; then origin='npapi-headers'; break; fi
        done < <(find "$DEPS/npapi-headers" -name "$(basename "$h")" -not -path '*/.git/*' 2>/dev/null | LC_ALL=C sort)
        printf '%s\t%s\t%s\t%s\n' "$h" "$(sha_of "$hp")" "$(stat -c %s "$hp")" "$origin"
      done < <(cd "$DEPS/staging/include" && find . -type f | LC_ALL=C sort | sed 's|^\./||')
    fi

    # ---- qt4 debian packages ----
    printf '#\n'
    cat <<'S4'
# ============================================================================
# SECTION: QT4-DEBIAN-PACKAGES  (what qt4-extract/ was unpacked from)
# sha256 is of the .deb as served by archive.debian.org, verified by download.
# TSV: package  version  architecture  sha256  size_bytes  source_package  url
# ============================================================================
S4
    local qp qsum qsize
    while IFS='|' read -r qp qsum qsize; do
      [ -n "$qp" ] || continue
      printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
        "$qp" "$QT4_EPOCH_VER" "armel" "$qsum" "$qsize" "$QT4_SRC" \
        "$MIRROR/pool/main/q/$QT4_SRC/${qp}_${QT4_VER}_armel.deb"
    done <<< "$QT4_DEBS"

    # ---- tree digests ----
    printf '#\n'
    cat <<'S5'
# ============================================================================
# SECTION: TREE-DIGESTS
# A whole-tree fingerprint for the parts too large to list file-by-file: sha256 over a
# C-sorted "<sha256|symlink target>\t<relpath>" listing of every entry. Independent of
# mtimes and traversal order, so it is stable across machines and re-extractions.
# TSV: tree  digest  contents
# ============================================================================
S5
    local t
    for t in qt4-extract staging/lib staging/include yajl-inc; do
      if [ -d "$DEPS/$t" ]; then
        printf '%s\t%s\t%s\n' "$t" "$(tree_digest "$DEPS/$t")" "$(tree_counts "$DEPS/$t")"
      else
        printf '# ABSENT\t%s\n' "$t"
      fi
    done

    # ---- git checkouts ----
    printf '#\n'
    cat <<'S6'
# ============================================================================
# SECTION: GIT-CHECKOUTS  (public, fully reproducible — pin by commit)
# TSV: dir  commit  describe  remote
# ============================================================================
S6
    local d pin
    for d in libpbnjson yajl npapi-headers; do
      pin="$(git_pin "$DEPS/$d")"
      if [ -n "$pin" ]; then printf '%s\t%s\n' "$d" "$pin"; else printf '# NOT-A-CHECKOUT\t%s\n' "$d"; fi
    done
    cat <<'S7'
#
# yajl-inc/ is NOT a checkout: it is yajl/src/api/*.h copied flat under a yajl/ directory
# so `#include <yajl/yajl_parse.h>` resolves. Three of its four headers are byte-identical
# to the yajl checkout; yajl_version.h is absent from it because cmake GENERATES that one
# from yajl_version.h.cmake at configure time. Regenerate yajl-inc by copying the three and
# configuring yajl once for the fourth, or keep it as the small vendored drop it is — the
# TREE-DIGESTS row above pins it either way.
#
# VERSION SKEW, recorded because it is load-bearing and easy to "fix" wrongly:
#   pbnjson  headers submissions/10 (c4b0611)  vs  device library submission-38
#   yajl     checkout tag 1.0.12               vs  device library 1.0.7
#   Qt       headers 4.8.6 (Debian)            vs  device libraries 4.8.0 (HP)
# In each case the HEADERS are what this repo pins and the LIBRARY is whatever the device
# has. build-adapter-pdk.sh chose submissions/10 deliberately ("the device era") so gcc4
# instantiates what the device libpbnjson_cpp exports. Do not bump these to match each
# other without re-testing the adapter on hardware: the C++ ones are template/ABI-sensitive.
S7
  } > "$MANIFEST.tmp"

  mv "$MANIFEST.tmp" "$MANIFEST"
  echo "wrote $MANIFEST"
}

check() {
  [ -f "$MANIFEST" ] || { echo "!! no $MANIFEST — run without arguments first" >&2; exit 2; }
  local fail=0 ok=0

  # device .so rows: filename sha256 size soname ...
  local fn sum rest p have
  while IFS=$'\t' read -r fn sum rest; do
    p="$DEPS/staging/lib/$fn"
    if [ ! -f "$p" ]; then echo "MISSING  staging/lib/$fn"; fail=$((fail + 1)); continue; fi
    have="$(sha_of "$p")"
    if [ "$have" = "$sum" ]; then ok=$((ok + 1)); else
      echo "MISMATCH staging/lib/$fn"; echo "  manifest $sum"; echo "  on disk  $have"; fail=$((fail + 1))
    fi
  done < <(sed -n '/^# SECTION: DEVICE-ARM-SHARED-OBJECTS/,/^# SECTION: SYMLINKS/p' "$MANIFEST" | grep -v '^#')

  # header rows: path sha256 size origin
  local h
  while IFS=$'\t' read -r h sum rest; do
    p="$DEPS/staging/include/$h"
    if [ ! -f "$p" ]; then echo "MISSING  staging/include/$h"; fail=$((fail + 1)); continue; fi
    have="$(sha_of "$p")"
    if [ "$have" = "$sum" ]; then ok=$((ok + 1)); else
      echo "MISMATCH staging/include/$h"; fail=$((fail + 1))
    fi
  done < <(sed -n '/^# SECTION: NPAPI-HEADERS/,/^# SECTION: QT4-DEBIAN-PACKAGES/p' "$MANIFEST" | grep -v '^#')

  # tree digests
  local t dg
  while IFS=$'\t' read -r t dg rest; do
    if [ ! -d "$DEPS/$t" ]; then echo "MISSING  tree $t"; fail=$((fail + 1)); continue; fi
    have="$(tree_digest "$DEPS/$t")"
    if [ "$have" = "$dg" ]; then ok=$((ok + 1)); else
      echo "MISMATCH tree $t"; echo "  manifest $dg"; echo "  on disk  $have"; fail=$((fail + 1))
    fi
  done < <(sed -n '/^# SECTION: TREE-DIGESTS/,/^# SECTION: GIT-CHECKOUTS/p' "$MANIFEST" | grep -v '^#')

  # git pins
  local d c
  while IFS=$'\t' read -r d c rest; do
    if [ ! -e "$DEPS/$d/.git" ]; then echo "MISSING  checkout $d"; fail=$((fail + 1)); continue; fi
    have="$(git -C "$DEPS/$d" rev-parse HEAD)"
    if [ "$have" = "$c" ]; then ok=$((ok + 1)); else
      echo "MISMATCH checkout $d: manifest $c, on disk $have"; fail=$((fail + 1))
    fi
  done < <(sed -n '/^# SECTION: GIT-CHECKOUTS/,$p' "$MANIFEST" | grep -v '^#')

  echo "checked: $ok ok, $fail bad"
  [ "$fail" -eq 0 ]
}

check_qt4_debs() {
  local tmp; tmp="$(mktemp -d)"
  # shellcheck disable=SC2064
  trap "rm -rf '$tmp'" EXIT
  local qp qsum qsize url got fail=0
  while IFS='|' read -r qp qsum qsize; do
    [ -n "$qp" ] || continue
    url="$MIRROR/pool/main/q/$QT4_SRC/${qp}_${QT4_VER}_armel.deb"
    if ! curl -fsSL -m 300 -o "$tmp/$qp.deb" "$url"; then
      echo "DOWNLOAD-FAIL $qp  $url"; fail=$((fail + 1)); continue
    fi
    got="$(sha_of "$tmp/$qp.deb")"
    if [ "$got" = "$qsum" ]; then echo "MATCH    $qp"; else
      echo "MISMATCH $qp: pinned $qsum, archive $got"; fail=$((fail + 1))
    fi
  done <<< "$QT4_DEBS"
  echo "qt4 debs: $fail bad"
  [ "$fail" -eq 0 ]
}

case "${1:-}" in
  "")                generate ;;
  --check)           check ;;
  --check-qt4-debs)  check_qt4_debs ;;
  *) echo "usage: $0 [--check | --check-qt4-debs]" >&2; exit 1 ;;
esac
