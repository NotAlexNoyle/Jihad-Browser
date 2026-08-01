#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Register a Jihad variant's db8 kinds + permissions on a connected device.
#
# WHY: Jihad's history/bookmarks/preferences live in app-OWNED db8 kinds — NOT the
# stock com.palm.browser* kinds, which are owned by com.palm.app.browser and grant
# NO access to any Jihad app id, so Jihad's writes were silently denied (device
# "History is not working"). A REAL .ipk install registers the kinds via the OS
# appinstaller, which reads db/{kinds,permissions}/ out of the installed app dir.
# The SDK `palm-install` dev shortcut does NOT run that step, so after a dev
# install run this once (idempotent — putKind is an upsert).
#
# ── ONE VARIANT, ONE NAMESPACE, ONE OWNER (review F-1 / R7) ──────────────────────────────────
# Each variant declares, ships and owns ITS OWN kinds, under its own app id:
#     enyo   app/db/        net.riverstonerelay.jihad-browser.{history,bookmarks,preferences}:1
#     mochi  app-mochi/db/  net.riverstonerelay.jihad-browser-mochi.{…}:1
# They used to be co-owned — the Enyo package declared all three kinds and app/db/permissions/*
# granted the .mochi/.mojo app ids CRUD on them — which is exactly the shared-lifetime coupling
# R7 exists to remove: Mochi installed alone had no registered kinds at all, and removing the
# Enyo package destroyed Mochi's data layer. db8 requires a kind's `owner` to equal the app id
# that registers it, so independence means separate namespaces, not broader grants. There are
# deliberately NO cross-variant permissions here; do not add any.
#
# MOJO HAS NO db8 LAYER AT ALL, on purpose: app-mojo/ ships no history, bookmarks or
# preferences view and makes no com.palm.db call (grep-verified), so it declares no kinds and
# needs no registration. If a Mojo front-end ever grows persistence it gets app-mojo/db/ with
# `net.riverstonerelay.jihad-browser-mojo.*` owned by its own app id — never a grant on another
# variant's kinds.
#
# putKind requires the OWNER identity, so each call uses `-a <that variant's app id>`.
#
# Usage:
#   register-db-kinds.sh              both variants that have a db layer (enyo mochi)
#   register-db-kinds.sh mochi        just this one
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../.." && pwd)

# <variant>:<app id>:<db dir, relative to the repo root>
variant_row() {
  case "$1" in
    enyo)  V_APP=net.riverstonerelay.jihad-browser;       V_DB=$REPO/app/db       ;;
    mochi) V_APP=net.riverstonerelay.jihad-browser-mochi; V_DB=$REPO/app-mochi/db ;;
    mojo)  echo "ERROR: the mojo variant has no db8 layer (see the header)" >&2; exit 2 ;;
    *)     echo "ERROR: unknown variant '$1' (expected: enyo mochi)" >&2; exit 2 ;;
  esac
}

VARIANTS="${*:-enyo mochi}"
for v in $VARIANTS; do variant_row "$v"; done   # validate the whole list before touching the device

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

{
  echo '#!/bin/sh'
  for v in $VARIANTS; do
    variant_row "$v"
    for k in history bookmarks preferences; do
      # The kind/permission FILES are named "<app id>.<kind>", so the Mochi files sit under
      # net.riverstonerelay.jihad-browser-mochi.<kind> — a different namespace from Enyo's, not
      # a prefix collision: db8 matches the `owner`/`object` strings exactly. (The app id gained
      # its hyphen on 2026-08-01 for a reason that is NOT db8's: ipkg's removal glob treated the
      # dotted form as a child of the Enyo package — impl-ipkg-prefix-collision.md.)
      kf=$V_DB/kinds/$V_APP.$k
      pf=$V_DB/permissions/$V_APP.$k
      [ -f "$kf" ] && [ -f "$pf" ] || { echo "ERROR: missing $kf or $pf" >&2; exit 1; }
      python3 -c "import json,sys;print(json.dumps(json.load(open(sys.argv[1]))))" "$kf" > "$TMP/kind.json"
      python3 -c "import json,sys;print(json.dumps({'permissions':json.load(open(sys.argv[1]))}))" "$pf" > "$TMP/perm.json"
      printf "luna-send -n 1 -a %s luna://com.palm.db/putKind '%s' && echo 'kind %s.%s registered'\n" \
        "$V_APP" "$(cat "$TMP/kind.json")" "$v" "$k"
      printf "luna-send -n 1 -a %s luna://com.palm.db/putPermissions '%s' && echo 'perm %s.%s registered'\n" \
        "$V_APP" "$(cat "$TMP/perm.json")" "$v" "$k"
    done
  done
} > "$TMP/reg.sh"

# Staged in /tmp on the DEVICE, not /media/internal: R8 keeps our files off the user's volume,
# and that applies to a dev helper's scratch file as much as to the package's own state.
novacom put file:///tmp/jihad-reg-db-kinds.sh < "$TMP/reg.sh"
novacom run file://bin/sh -- /tmp/jihad-reg-db-kinds.sh
novacom run file://bin/rm -- -f /tmp/jihad-reg-db-kinds.sh
echo "=== done — db kinds registered for: $VARIANTS ==="
