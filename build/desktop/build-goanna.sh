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

# --- XPI web-install confirm component, into the DESKTOP dist ------------------------------------
# Same reason as the branding package above: the device bundle installs this, and the desktop dist
# needs it to be able to exercise the install flow at all (without it the toolkit falls back to a
# modal XUL window that cannot open here, and every install is cancelled).
PROMPT_JS=/jihad/render/goanna/components/jihadInstallPrompt.js
if [ -f "$PROMPT_JS" ] && [ -d "$DISTBIN/components" ]; then
  cp -L "$PROMPT_JS" "$DISTBIN/components/jihadInstallPrompt.js"
  cat > "$DISTBIN/components/jihad-install-prompt.manifest" <<'MANIFEST'
component {5cf8e2a6-91a1-44f5-9d33-8ab6f2c40d17} jihadInstallPrompt.js
contract @mozilla.org/addons/web-install-prompt;1 {5cf8e2a6-91a1-44f5-9d33-8ab6f2c40d17}
MANIFEST
  if ! grep -q '^manifest components/jihad-install-prompt.manifest$' "$DISTBIN/chrome.manifest" 2>/dev/null; then
    echo 'manifest components/jihad-install-prompt.manifest' >> "$DISTBIN/chrome.manifest"
  fi
  echo "  xpi prompt: registered in the desktop dist"
fi

# --- about:preferences / about:settings, into the DESKTOP dist ------------------------------------
# Mirrors the block in build/webos-oe/make-device-bundle.sh. Without this the desktop dist has no
# about:preferences at all, so EVERY criterion in cavekit-preferences-ui.md was device-only —
# which is the divergence cavekit-gre-widgets.md R7 exists to prevent (found in review 2026-08-06).
PREFSUI=/jihad/packaging/prefsui
ABOUT_JS=/jihad/render/goanna/components/jihadAboutPreferences.js
if [ -d "$PREFSUI" ] && [ -f "$ABOUT_JS" ] && [ -d "$DISTBIN/components" ]; then
  rm -rf "$DISTBIN/prefsui"
  mkdir -p "$DISTBIN/prefsui"
  cp -R "$PREFSUI/content" "$DISTBIN/prefsui/"
  cp -L "$PREFSUI/jihad-prefsui.manifest" "$DISTBIN/"
  cp -L "$ABOUT_JS" "$DISTBIN/components/jihadAboutPreferences.js"
  cat > "$DISTBIN/components/jihad-about-preferences.manifest" <<'MANIFEST'
component {0f2b8f4e-6c31-4a7b-9f2d-5c1e73a4b806} jihadAboutPreferences.js
contract @mozilla.org/network/protocol/about;1?what=preferences {0f2b8f4e-6c31-4a7b-9f2d-5c1e73a4b806}
contract @mozilla.org/network/protocol/about;1?what=settings {0f2b8f4e-6c31-4a7b-9f2d-5c1e73a4b806}
MANIFEST
  if ! grep -q '^manifest jihad-prefsui.manifest$' "$DISTBIN/chrome.manifest" 2>/dev/null; then
    echo 'manifest jihad-prefsui.manifest' >> "$DISTBIN/chrome.manifest"
  fi
  if ! grep -q '^manifest components/jihad-about-preferences.manifest$' "$DISTBIN/chrome.manifest" 2>/dev/null; then
    echo 'manifest components/jihad-about-preferences.manifest' >> "$DISTBIN/chrome.manifest"
  fi
  for f in jihad-prefsui.manifest prefsui/content/preferences.html prefsui/content/preferences.js \
           prefsui/content/preferences.css components/jihadAboutPreferences.js \
           components/jihad-about-preferences.manifest; do
    [ -s "$DISTBIN/$f" ] || { echo "ERROR: prefs-ui file missing/empty in desktop dist: $f" >&2; exit 1; }
  done
  echo "  prefs ui: about:preferences + about:settings registered in the desktop dist"
else
  # FAIL LOUD. A silent skip here is what let the two builds drift in the first place.
  echo "ERROR: prefs-ui inputs missing ($PREFSUI / $ABOUT_JS) — desktop dist would have no about:preferences" >&2
  exit 1
fi

# --- add-on prefs, from the SAME file the device bundle uses --------------------------------------
# Without these the desktop dist runs on engine defaults, where xpinstall.whitelist.required is TRUE
# and an install from a page throws NS_ERROR_UNEXPECTED — indistinguishable from a broken install
# path. Sharing the file is what keeps the two builds from drifting.
ADDON_PREFS=/jihad/packaging/prefs/jihad-addon-prefs.js
if [ -f "$ADDON_PREFS" ] && [ -f "$DISTBIN/goanna.js" ]; then
  grep -q 'JIHAD add-on prefs' "$DISTBIN/goanna.js" 2>/dev/null || \
    cat "$ADDON_PREFS" >> "$DISTBIN/goanna.js"
  echo "  add-on prefs: appended to the desktop goanna.js"
fi

PLATFORM_PREFS=/jihad/packaging/prefs/jihad-platform-prefs.js
if [ -f "$PLATFORM_PREFS" ] && [ -f "$DISTBIN/goanna.js" ]; then
  grep -q 'JIHAD platform prefs' "$DISTBIN/goanna.js" 2>/dev/null || \
    cat "$PLATFORM_PREFS" >> "$DISTBIN/goanna.js"
  echo "  platform prefs: appended to the desktop goanna.js"
fi

echo "== done; artifacts under /out =="
