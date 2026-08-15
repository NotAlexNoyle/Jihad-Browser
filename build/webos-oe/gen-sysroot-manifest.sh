#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Generate / verify / re-fetch the manifest for the Debian Jessie armel sysroot.
#
# WHY THIS EXISTS (cavekit-device-build.md R3): the OE build consumes four prebuilt inputs
# it does not build. Three of the four must be "obtainable from a clean clone: an in-repo
# recipe, or a documented procedure". The Jessie sysroot had neither — its ONLY manifest
# was the set of .deb files sitting in the git-ignored build/webos-oe/arm-sysroot/debs/,
# which a clean clone does not get. This script turns that directory into a checked-in,
# checksummed, re-fetchable manifest, so the sysroot becomes reproducible from the repo
# plus archive.debian.org.
#
# The sysroot ITSELF (arm-sysroot/root/, 127 MB) is just these .debs unpacked; it stays
# git-ignored and is re-derivable from them.
#
# No dpkg is required: a .deb is an `ar` archive and the metadata lives in its
# control.tar.gz member (same trick fetch-pdk.sh uses on the PDK .deb).
#
# Usage:
#   gen-sysroot-manifest.sh              regenerate the manifest from the .debs on disk
#   gen-sysroot-manifest.sh --check      verify the on-disk .debs against the manifest
#   gen-sysroot-manifest.sh --fetch      download every .deb in the manifest from
#                                        archive.debian.org and verify its sha256
#   gen-sysroot-manifest.sh --check-urls [N]
#                                        HTTP-HEAD N manifest URLs (default 5) to confirm
#                                        they still resolve on archive.debian.org
#
# Output is DETERMINISTIC: no timestamps, C-collation sort, so a regeneration with no
# input change produces a byte-identical file. Git records when it changed; the file does
# not claim a date of its own.
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
DEBS="$HERE/arm-sysroot/debs"
MANIFEST="$HERE/arm-sysroot-debs.manifest"
MIRROR="${DEBIAN_ARCHIVE_MIRROR:-http://archive.debian.org/debian}"

# Debian pool prefix: source packages starting with "lib" live under lib<x>/, all others
# under <x>/ where <x> is the first letter.
pool_prefix() {
  case "$1" in
    lib?*) printf '%s' "${1:0:4}" ;;
    *)     printf '%s' "${1:0:1}" ;;
  esac
}

# The pool filename drops the epoch that the apt cache filename URL-encodes as %3a:
# apt cache  libgcc1_1%3a4.9.2-10+deb8u1_armel.deb
# pool       libgcc1_4.9.2-10+deb8u1_armel.deb        <- the only one that 404s otherwise
strip_epoch() { printf '%s' "${1#*:}"; }

read_control_field() {
  # $1 = deb path, $2 = field name
  ar p "$1" control.tar.gz 2>/dev/null | tar -xzO ./control 2>/dev/null \
    | sed -n "s/^$2: //p" | head -1
}

generate() {
  [ -d "$DEBS" ] || { echo "!! no $DEBS — nothing to manifest" >&2; exit 2; }
  local count
  count="$(find "$DEBS" -maxdepth 1 -name '*.deb' | wc -l)"
  [ "$count" -gt 0 ] || { echo "!! no .deb files in $DEBS" >&2; exit 2; }

  {
    cat <<'HDR'
# Debian Jessie (8) armel sysroot — package manifest
#
# This is the declared, re-obtainable form of one of the four prebuilt inputs the OE build
# consumes but does not build (cavekit-device-build.md R3, prebuilt-inputs carve-out). The
# sysroot supplies the GTK2/X11/cairo/pango/fontconfig/glib that the engine links against;
# PKG_CONFIG_LIBDIR points at the .pc files these packages install.
#
# HOW TO REBUILD THE SYSROOT FROM A CLEAN CLONE
#   1. build/webos-oe/gen-sysroot-manifest.sh --fetch
#        downloads every .deb below from archive.debian.org into
#        build/webos-oe/arm-sysroot/debs/ and verifies each sha256.
#   2. unpack each .deb's data member into build/webos-oe/arm-sysroot/root/
#        for d in build/webos-oe/arm-sysroot/debs/*.deb; do
#          m=$(ar t "$d" | grep '^data\.tar'); ar p "$d" "$m" | tar -C root -xf -
#        done
#   3. relativize the absolute symlinks and the .pc/.la prefixes as the sysroot needs.
#
# Debian 8 "jessie" is END OF LIFE, so these exact versions are frozen on
# archive.debian.org rather than moving under security updates — which is what makes a
# checksum manifest a durable pin instead of a snapshot that rots. armel (soft-float
# ARM EABI) is the right Debian port for this target: webOS 3 is a softfp userland.
#
# NOTE ON THE PACKAGE SET: this records what the sysroot was actually assembled from. It
# is the transitive closure apt resolved for the engine's build-dependencies, not a
# hand-curated list, so it includes base packages (dpkg, coreutils, perl-base) that came
# along as dependencies. Do not prune it by eye — regenerate it from a working sysroot.
# The set can legitimately GROW when the engine gains a new link-time dependency (an ALSA
# addition was in flight when this was first generated); regenerate after any such change
# rather than hand-editing rows.
#
# The URL column is the archive.debian.org pool path. It is NOT always
# <mirror>/pool/main/<x>/<package>/<on-disk filename>: the pool is keyed by SOURCE package,
# and the pool filename DROPS the epoch that the apt cache filename encodes as %3a. Both
# forms are recorded so neither has to be re-derived.
#
# Regenerate with: build/webos-oe/gen-sysroot-manifest.sh
# Verify on-disk:  build/webos-oe/gen-sysroot-manifest.sh --check
#
# TSV columns:
#   package  version  architecture  sha256  size_bytes  source_package  filename  url
HDR
    printf '#\n'
    printf '# package\tversion\tarchitecture\tsha256\tsize_bytes\tsource_package\tfilename\turl\n'

    local f base pkg ver arch src srcname size sum pfx poolver poolfile url
    # LC_ALL=C sort on the filename is what makes reruns byte-identical.
    while IFS= read -r f; do
      base="$(basename "$f")"
      pkg="$(read_control_field "$f" Package)"
      ver="$(read_control_field "$f" Version)"
      arch="$(read_control_field "$f" Architecture)"
      src="$(read_control_field "$f" Source)"
      [ -n "$pkg" ] || { echo "!! $base: no Package field — not a .deb?" >&2; exit 3; }
      # "Source: glib2.0 (2.42.1-1)" -> glib2.0 ; absent Source means source == package
      srcname="${src%% *}"
      [ -n "$srcname" ] || srcname="$pkg"
      size="$(stat -c %s "$f")"
      sum="$(sha256sum "$f" | cut -d' ' -f1)"
      pfx="$(pool_prefix "$srcname")"
      poolver="$(strip_epoch "$ver")"
      poolfile="${pkg}_${poolver}_${arch}.deb"
      url="$MIRROR/pool/main/$pfx/$srcname/$poolfile"
      printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
        "$pkg" "$ver" "$arch" "$sum" "$size" "$srcname" "$base" "$url"
    done < <(find "$DEBS" -maxdepth 1 -name '*.deb' | LC_ALL=C sort)
  } > "$MANIFEST.tmp"

  mv "$MANIFEST.tmp" "$MANIFEST"
  echo "wrote $MANIFEST ($count packages)"
}

# Emit the data rows of the manifest (drops comments and the column-header line).
manifest_rows() {
  [ -f "$MANIFEST" ] || { echo "!! no $MANIFEST — run without arguments first" >&2; exit 2; }
  grep -v '^#' "$MANIFEST" | grep -v '^[[:space:]]*$'
}

check() {
  local pkg ver arch sum size src base url fail=0 ok=0 missing=0
  while IFS=$'\t' read -r pkg ver arch sum size src base url; do
    if [ ! -f "$DEBS/$base" ]; then
      echo "MISSING  $base"; missing=$((missing + 1)); continue
    fi
    local have
    have="$(sha256sum "$DEBS/$base" | cut -d' ' -f1)"
    if [ "$have" = "$sum" ]; then
      ok=$((ok + 1))
    else
      echo "MISMATCH $base"; echo "  manifest $sum"; echo "  on disk  $have"
      fail=$((fail + 1))
    fi
  done < <(manifest_rows)

  local extra=0
  if [ -d "$DEBS" ]; then
    while IFS= read -r f; do
      grep -q -F "	$(basename "$f")	" "$MANIFEST" || { echo "UNLISTED $(basename "$f")"; extra=$((extra + 1)); }
    done < <(find "$DEBS" -maxdepth 1 -name '*.deb' | LC_ALL=C sort)
  fi

  echo "checked: $ok ok, $fail mismatched, $missing missing, $extra unlisted"
  [ "$fail" -eq 0 ] && [ "$missing" -eq 0 ] && [ "$extra" -eq 0 ]
}

fetch() {
  mkdir -p "$DEBS"
  local pkg ver arch sum size src base url got n=0 fail=0
  while IFS=$'\t' read -r pkg ver arch sum size src base url; do
    n=$((n + 1))
    if [ -f "$DEBS/$base" ] && [ "$(sha256sum "$DEBS/$base" | cut -d' ' -f1)" = "$sum" ]; then
      continue
    fi
    echo "fetching $base"
    if ! curl -fsSL -o "$DEBS/$base.part" "$url"; then
      echo "!! download failed: $url" >&2; rm -f "$DEBS/$base.part"; fail=$((fail + 1)); continue
    fi
    got="$(sha256sum "$DEBS/$base.part" | cut -d' ' -f1)"
    if [ "$got" != "$sum" ]; then
      echo "!! sha256 mismatch for $base (want $sum, got $got)" >&2
      rm -f "$DEBS/$base.part"; fail=$((fail + 1)); continue
    fi
    mv "$DEBS/$base.part" "$DEBS/$base"
  done < <(manifest_rows)
  echo "fetch: $n packages processed, $fail failed"
  [ "$fail" -eq 0 ]
}

check_urls() {
  local want="${1:-5}" n=0 fail=0 code
  local pkg ver arch sum size src base url
  while IFS=$'\t' read -r pkg ver arch sum size src base url; do
    [ "$n" -lt "$want" ] || break
    n=$((n + 1))
    code="$(curl -sS -o /dev/null -w '%{http_code}' -I -m 30 "$url" || echo 000)"
    printf '%s  %s\n' "$code" "$url"
    [ "$code" = "200" ] || fail=$((fail + 1))
  done < <(manifest_rows)
  echo "url check: $n probed, $fail not 200"
  [ "$fail" -eq 0 ]
}

case "${1:-}" in
  "")           generate ;;
  --check)      check ;;
  --fetch)      fetch ;;
  --check-urls) check_urls "${2:-5}" ;;
  *) echo "usage: $0 [--check | --fetch | --check-urls [N]]" >&2; exit 1 ;;
esac
