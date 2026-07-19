#!/bin/bash
# Copyright 2026 the Jihad Browser project. Apache-2.0.
#
# Register the Jihad app's db8 kinds + permissions on a connected device.
#
# WHY: Jihad's history/bookmarks/preferences live in app-OWNED db8 kinds
# (net.riverstonerelay.jihad-browser.{history,bookmarks,preferences}:1) — NOT the
# stock com.palm.browser* kinds, which are owned by com.palm.app.browser and grant
# NO access to the Jihad app id, so Jihad's writes were silently denied (device
# "History is not working"). The kind + permission JSON ships in app/db/{kinds,
# permissions}/ and a REAL ipk install registers them via the OS appinstaller.
# The SDK `palm-install` dev shortcut does NOT run that step, so after a dev
# install run this once to register them (idempotent — putKind is upsert).
#
# putKind requires the OWNER identity, so each call uses `-a <appid>`.
set -euo pipefail
APP=net.riverstonerelay.jihad-browser
HERE=$(cd "$(dirname "$0")" && pwd)
DB="$HERE/../../app/db"
TMP=$(mktemp -d)

emit() {  # $1 = kind short name
  local k=$1
  python3 -c "import json,sys;print(json.dumps(json.load(open(sys.argv[1]))))" \
    "$DB/kinds/$APP.$k" > "$TMP/kind.json"
  python3 -c "import json,sys;print(json.dumps({'permissions':json.load(open(sys.argv[1]))}))" \
    "$DB/permissions/$APP.$k" > "$TMP/perm.json"
}

{
  echo '#!/bin/sh'
  for k in history bookmarks preferences; do
    emit "$k"
    printf "luna-send -n 1 -a %s luna://com.palm.db/putKind '%s' && echo 'kind %s registered'\n" \
      "$APP" "$(cat "$TMP/kind.json")" "$k"
    printf "luna-send -n 1 -a %s luna://com.palm.db/putPermissions '%s' && echo 'perm %s registered'\n" \
      "$APP" "$(cat "$TMP/perm.json")" "$k"
  done
} > "$TMP/reg.sh"

novacom put file:///media/internal/jihad/reg-db-kinds.sh < "$TMP/reg.sh"
novacom run file://bin/sh -- /media/internal/jihad/reg-db-kinds.sh
rm -rf "$TMP"
echo "=== done — Jihad db kinds registered ==="
