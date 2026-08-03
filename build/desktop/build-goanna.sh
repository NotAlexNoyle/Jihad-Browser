#!/bin/bash
# Copyright 2026 NotAlexNoyle.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Build the UXP/Goanna engine inside the pinned container (see Dockerfile).
# Runs as the unprivileged `builder` user against a read-only UXP source mount.
#
#   /src/uxp  : UXP source (mounted read-WRITE — autoconf2.13 generates the
#               `configure` script into the source tree at build time)
#   /out      : object dir + build artifacts (mounted read-write)
#   /cfg      : this directory (mozconfig.goanna, mounted read-only)
#
# Usage (inside the container): /cfg/build-goanna.sh [configure|build|all]
set -euo pipefail

SRC=${UXP_SRC:-/src/uxp}
export MOZCONFIG=${MOZCONFIG:-/cfg/mozconfig.goanna}
STAGE=${1:-all}

echo "== Jihad Goanna build =="
echo "src:      $SRC"
echo "mozconfig:$MOZCONFIG"
echo "python3:  $(python3 --version 2>&1)"
echo "python2:  $(python2 --version 2>&1)"
echo "autoconf: $(command -v autoconf2.13 || echo MISSING)"
echo "gcc:      $(gcc -dumpfullversion 2>/dev/null || gcc -dumpversion)"

if [ ! -d "$SRC" ]; then echo "ERROR: UXP source not mounted at $SRC"; exit 1; fi

# Apply Jihad build patches to the UXP source (idempotent: skip if already in).
# These adapt UXP to the modern toolchain (e.g. GCC 9 -Wformat-overflow).
if [ -d /cfg/patches ]; then
  for p in /cfg/patches/*.patch; do
    [ -e "$p" ] || continue
    name=$(basename "$p")
    if patch -p1 -d "$SRC" --forward --dry-run < "$p" >/dev/null 2>&1; then
      patch -p1 -d "$SRC" --forward < "$p" && echo "applied patch: $name"
    else
      echo "patch already applied/skipped: $name"
    fi
  done
fi

# Jihad branding strip (cavekit-licensing-branding R3) is now patches/0010-branding-strip.patch
# (applied by the patch loop above), not an inline sed — so it is captured in the patch queue
# against the pristine third_party/uxp submodule like every other engine modification. See docs/UXP.md.

# mach builds into MOZ_OBJDIR (mozconfig → /out); autoconf2.13 also writes the
# generated `configure` into the source tree, so $SRC is mounted read-write.
cd "$SRC"

# ./mach is a shell/python polyglot that re-execs itself under python3.
case "$STAGE" in
  configure) ./mach configure ;;
  build)     ./mach build ;;
  all)       ./mach configure && ./mach build ;;
  *)         echo "unknown stage: $STAGE (use configure|build|all)"; exit 2 ;;
esac

# --- our own chrome://branding/ package, into the DESKTOP dist -----------------------------------
# The device bundle installs this (make-device-bundle.sh) because toolkit's about:addons opens with
#   <!ENTITY % brandDTD SYSTEM "chrome://branding/locale/brand.dtd" > %brandDTD;
# and a DTD that fails to load is a hard XML parse error. The desktop dist needs it for the same
# reason: without it about:addons parses to a <parsererror> document, and every chrome/XUL probe run
# against it silently tests an error page instead of the add-ons manager (measured 2026-08-03 —
# clicks resolved to <parsererror>/<sourcetext>, which reads like "the popup path is broken" when
# the page simply never existed). A `mach build` rewrites dist/bin, so this must run after it, on
# every build, not once by hand.
BRANDING=/jihad/packaging/branding
DISTBIN=/out/obj-jihad-goanna/dist/bin
if [ -d "$BRANDING" ] && [ -d "$DISTBIN" ]; then
  rm -rf "$DISTBIN/branding"
  mkdir -p "$DISTBIN/branding"
  cp -R "$BRANDING/content" "$BRANDING/locale" "$DISTBIN/branding/"
  cp -L "$BRANDING/jihad-branding.manifest" "$DISTBIN/"
  # Idempotent: a duplicate `manifest` line re-registers the package and warns on every start.
  if ! grep -q '^manifest jihad-branding.manifest$' "$DISTBIN/chrome.manifest" 2>/dev/null; then
    echo 'manifest jihad-branding.manifest' >> "$DISTBIN/chrome.manifest"
  fi
  [ -s "$DISTBIN/branding/locale/en-US/brand.dtd" ] || {
    echo "ERROR: branding brand.dtd missing from the desktop dist" >&2; exit 1; }
  echo "  branding: chrome://branding/ registered in the desktop dist (about:addons brand.dtd)"
fi

echo "== done; artifacts under /out =="
