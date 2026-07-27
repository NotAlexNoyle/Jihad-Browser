#!/bin/bash
# Copyright 2026 the Jihad Browser project.
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

echo "== done; artifacts under /out =="
