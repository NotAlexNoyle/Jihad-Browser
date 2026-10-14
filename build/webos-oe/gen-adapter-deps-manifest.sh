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
# WHAT IS IN adapter-deps (since 2026-10-14, cavekit-device-build.md R11, only two things):
#   1. staging/lib/*.so*     ARM shared objects LIFTED OFF A TOUCHPAD. Not redistributable,
#                            not rebuildable here. --fetch-device-libs gets them over novacom.
#                            These are what the adapter links against.
#   2. qt4-extract/          Qt4 HEADERS. NOT a device drop — see below. --fetch-qt4 gets them.
# What used to be here is now in git: the public checkouts libpbnjson, yajl and npapi-headers
# are submodules under third_party/ (the parent repository pins their commits), and the headers
# Jihad itself made (npapi.h with the device's touch event, the nppalmdefs.h shim, yajl's
# generated yajl_version.h, six pbnjson_c compat headers) are in build/webos-oe/adapter-include/.
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
#   gen-adapter-deps-manifest.sh --fetch-qt4
#                                           download the three .debs, check them, and unpack
#                                           qt4-extract/ (a clean clone's first build)
#   gen-adapter-deps-manifest.sh --fetch-device-libs
#                                           copy the seven libraries off a TouchPad over
#                                           novacom ($NOVACOM, else novacom on PATH), name them,
#                                           make the linker symlinks, and check them
#
# Output is DETERMINISTIC: no timestamps, C-collation sort, so a regeneration with no input
# change produces a byte-identical file.
set -euo pipefail
. "$(dirname "$0")/host-path.sh"   # the repository's own novacom and palm-* first (R11)

HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
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

. "$(dirname "$0")/fetch-lib.sh"   # fetch_url: pinned downloads with the CDN retry

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

gitlink() {
  # $1 = submodule path; the commit the parent repository's index records for it
  git -C "$REPO" ls-files -s -- "$1" | awk '$1=="160000"{print $2}'
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
#   part                        obtainable from a clean clone?
#   staging/lib/*.so*           NO — needs a TouchPad (--fetch-device-libs); not redistributable.
#   qt4-extract/                YES — three stock Debian jessie .debs, pinned below (--fetch-qt4).
#
# Everything else the adapter compiles against is in git: libpbnjson, yajl and npapi-headers
# are submodules (third_party/, listed in SUBMODULES below), and Jihad's own headers are in
# build/webos-oe/adapter-include/.
#
# ============================================================================
# PROCEDURE: RE-OBTAIN staging/lib FROM A TOUCHPAD
# ============================================================================
# These are HP's own builds off the device rootfs (webOS 3.0.5, ARMv7 softfp, glibc 2.8).
# They cannot be vendored — they are HP proprietary — and they cannot be rebuilt, because
# nothing reproduces HP's "nova" OE tree. With a TouchPad attached:
#
#   build/webos-oe/gen-adapter-deps-manifest.sh --fetch-device-libs
#
# copies each from the device path in the table below to its versioned name, makes the
# development symlinks of the SYMLINKS section (`ld -lQtGui` needs libQtGui.so, and the
# device rootfs has none), and checks every sha256 against this file: a device that is not
# webOS 3.0.5 / CE 3.1.0 gives different checksums and the fetch fails, which is the point.
# The device paths were confirmed on a TouchPad on CE 3.1.0 on 2026-10-01.
#
# ============================================================================
# PROCEDURE: RE-OBTAIN qt4-extract FROM archive.debian.org
# ============================================================================
# Debian 8 "jessie" is end-of-life, so these versions are frozen rather than moving under
# security updates. `build/webos-oe/gen-adapter-deps-manifest.sh --fetch-qt4` downloads each
# package in the QT4-DEBIAN-PACKAGES table, checks its sha256 and unpacks it into qt4-extract/.
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
    for t in qt4-extract staging/lib; do
      if [ -d "$DEPS/$t" ]; then
        printf '%s\t%s\t%s\n' "$t" "$(tree_digest "$DEPS/$t")" "$(tree_counts "$DEPS/$t")"
      else
        printf '# ABSENT\t%s\n' "$t"
      fi
    done

    # ---- submodules ----
    printf '#
'
    cat <<'S6'
# ============================================================================
# SECTION: SUBMODULES  (public git, pinned by the parent repository's gitlinks)
# TSV: path  commit  describe  url
# ============================================================================
S6
    local d
    for d in third_party/libpbnjson third_party/yajl third_party/npapi-headers; do
      printf '%s	%s	%s	%s
' "$d" "$(gitlink "$d")" \
        "$(git -C "$REPO/$d" describe --tags --always 2>/dev/null || echo '-')" \
        "$(git -C "$REPO" config -f .gitmodules --get "submodule.$d.url")"
    done
    cat <<'S7'
#
# VERSION SKEW, recorded because it is load-bearing and easy to "fix" wrongly:
#   pbnjson  headers submissions/10 (c4b0611)  vs  device library submission-38
#   yajl     submodule tag 1.0.12              vs  device library 1.0.7
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

  # tree digests
  local t dg
  while IFS=$'\t' read -r t dg rest; do
    if [ ! -d "$DEPS/$t" ]; then echo "MISSING  tree $t"; fail=$((fail + 1)); continue; fi
    have="$(tree_digest "$DEPS/$t")"
    if [ "$have" = "$dg" ]; then ok=$((ok + 1)); else
      echo "MISMATCH tree $t"; echo "  manifest $dg"; echo "  on disk  $have"; fail=$((fail + 1))
    fi
  done < <(sed -n '/^# SECTION: TREE-DIGESTS/,/^# SECTION: SUBMODULES/p' "$MANIFEST" | grep -v '^#')

  # submodules: checked out, at the commit the manifest and the parent repository record
  local d c
  while IFS=$'	' read -r d c rest; do
    if [ ! -e "$REPO/$d/.git" ]; then echo "MISSING  submodule $d (git submodule update --init $d)"; fail=$((fail + 1)); continue; fi
    have="$(git -C "$REPO/$d" rev-parse HEAD)"
    if [ "$have" = "$c" ] && [ "$(gitlink "$d")" = "$c" ]; then ok=$((ok + 1)); else
      echo "MISMATCH submodule $d: manifest $c, gitlink $(gitlink "$d"), checked out $have"; fail=$((fail + 1))
    fi
  done < <(sed -n '/^# SECTION: SUBMODULES/,/^# VERSION SKEW/p' "$MANIFEST" | grep -v '^#')

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
    if ! fetch_url "$url" "$tmp/$qp.deb"; then   # no pin here: a mismatch is reported below, not retried
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

# Unpack one .deb's data member into a directory, whatever its compression (no dpkg needed).
deb_extract() { # <deb> <dest>
  local member
  member="$(ar t "$1" | grep -E '^data\.tar' | head -1)"
  [ -n "$member" ] || { echo "!! no data.tar member in $1" >&2; return 1; }
  case "$member" in
    *.xz)  ar p "$1" "$member" | tar -C "$2" -xJf - ;;
    *.gz)  ar p "$1" "$member" | tar -C "$2" -xzf - ;;
    *.bz2) ar p "$1" "$member" | tar -C "$2" -xjf - ;;
    *)     ar p "$1" "$member" | tar -C "$2" -xf - ;;
  esac
}

fetch_qt4() {
  local tmp qp qsum qsize url got
  tmp="$(mktemp -d)"
  # shellcheck disable=SC2064
  trap "rm -rf '$tmp'" EXIT
  while IFS='|' read -r qp qsum qsize; do
    [ -n "$qp" ] || continue
    url="$MIRROR/pool/main/q/$QT4_SRC/${qp}_${QT4_VER}_armel.deb"
    echo "fetch $qp"
    fetch_url "$url" "$tmp/$qp.deb" "$qsum" || { echo "!! download failed, or not the pinned sha256: $url" >&2; exit 3; }
    got="$(sha_of "$tmp/$qp.deb")"
    [ "$got" = "$qsum" ] || { echo "!! $qp: sha256 $got, pinned $qsum" >&2; exit 3; }
  done <<< "$QT4_DEBS"
  rm -rf "${DEPS:?}/qt4-extract.new"; mkdir -p "$DEPS/qt4-extract.new"
  while IFS='|' read -r qp qsum qsize; do
    [ -n "$qp" ] || continue
    deb_extract "$tmp/$qp.deb" "$DEPS/qt4-extract.new"
  done <<< "$QT4_DEBS"
  if [ -f "$MANIFEST" ]; then
    local want have
    want="$(sed -n '/^# SECTION: TREE-DIGESTS/,/^# SECTION: SUBMODULES/p' "$MANIFEST" | awk -F'\t' '$1=="qt4-extract"{print $2}')"
    have="$(tree_digest "$DEPS/qt4-extract.new")"
    [ -z "$want" ] || [ "$want" = "$have" ] || { echo "!! qt4-extract digest $have, manifest $want" >&2; exit 4; }
  fi
  rm -rf "${DEPS:?}/qt4-extract"; mv "$DEPS/qt4-extract.new" "$DEPS/qt4-extract"
  echo "qt4-extract ready ($(tree_counts "$DEPS/qt4-extract"))"
}

# novacom stops, dropping what the device had not yet sent, when its stdin reaches end-of-file;
# hold stdin open with a sleep that is killed when the transfer ends (as build/webos-oe/dev-sh.sh).
novacom_get() { # <device path> <local file>
  local holder rc
  exec 3< <(exec sleep 600 2>/dev/null); holder=$!
  set +e; "${NOVACOM:-novacom}" get "file://$1" > "$2" <&3; rc=$?; set -e
  exec 3<&-; kill "$holder" 2>/dev/null || true
  return $rc
}

fetch_device_libs() {
  [ -f "$MANIFEST" ] || { echo "!! no $MANIFEST" >&2; exit 2; }
  command -v "${NOVACOM:-novacom}" >/dev/null || { echo "!! novacom not found (tools/host/build-novacom.sh, or set NOVACOM)" >&2; exit 2; }
  local new="$DEPS/staging/lib.new" fn sum size soname upv devp note have fail=0
  rm -rf "${new:?}"; mkdir -p "$new"
  while IFS=$'\t' read -r fn sum size soname upv devp note; do
    [ -n "$fn" ] || continue
    echo "get $devp -> $fn"
    novacom_get "$devp" "$new/$fn" || { echo "!! novacom get $devp failed" >&2; exit 3; }
    have="$(sha_of "$new/$fn")"
    if [ "$have" != "$sum" ]; then echo "MISMATCH $fn: device $have, manifest $sum"; fail=$((fail + 1)); fi
  done < <(sed -n '/^# SECTION: DEVICE-ARM-SHARED-OBJECTS/,/^# SECTION: SYMLINKS/p' "$MANIFEST" | grep -v '^#')
  [ "$fail" -eq 0 ] || { echo "!! $fail libraries differ from the manifest; left in $new" >&2; exit 4; }
  local link target
  while IFS=$'\t' read -r link target; do
    [ -n "$link" ] || continue
    ln -sfn "$target" "$new/$link"
  done < <(sed -n '/^# SECTION: SYMLINKS/,/^# SECTION: QT4-DEBIAN-PACKAGES/p' "$MANIFEST" | grep -v '^#')
  rm -rf "${DEPS:?}/staging/lib"; mv "$new" "$DEPS/staging/lib"
  echo "staging/lib ready ($(tree_counts "$DEPS/staging/lib"))"
}

case "${1:-}" in
  "")                  generate ;;
  --check)             check ;;
  --check-qt4-debs)    check_qt4_debs ;;
  --fetch-qt4)         fetch_qt4 ;;
  --fetch-device-libs) fetch_device_libs ;;
  *) echo "usage: $0 [--check | --check-qt4-debs | --fetch-qt4 | --fetch-device-libs]" >&2; exit 1 ;;
esac
