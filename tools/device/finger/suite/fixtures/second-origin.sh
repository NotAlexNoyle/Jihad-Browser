#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# second-origin.sh up|down SHELL (host side): the test pages (render/goanna/test) served a second time, on port 8079 of
# the host, so a page on :8077 can frame a page of another origin (cavekit-site-data.md R1: a login form in a frame is
# filled only for the frame's own origin). 8078 is the identity harness's; 8079 is this fixture's alone. The server's
# pid is kept in a file and stopped by its number, never by a pattern.
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; ROOT="$(cd "$HERE/../../../../.." && pwd)"
PIDF="${XDG_RUNTIME_DIR:-/tmp}/jihad-second-origin.pid"
up() {
  if [ -s "$PIDF" ] && kill -0 "$(cat "$PIDF")" 2>/dev/null; then echo "second origin: already on :8079 (pid $(cat "$PIDF"))"; return 0; fi
  cd "$ROOT/render/goanna/test" || exit 1
  setsid nohup python3 -I -m http.server 8079 --bind 192.168.1.245 >/dev/null 2>&1 < /dev/null &
  echo $! > "$PIDF"
  for i in 1 2 3 4 5 6 7 8 9 10; do
    curl -s -o /dev/null -m 2 http://192.168.1.245:8079/jihad-login.html && { echo "second origin: up on :8079 (pid $(cat "$PIDF")) ($2)"; return 0; }
    sleep 0.5
  done
  echo "second origin: :8079 did not answer"; return 1
}
down() {
  [ -s "$PIDF" ] && kill "$(cat "$PIDF")" 2>/dev/null
  rm -f "$PIDF"; echo "second origin: down ($2)"
}
case "$1" in up) up "$@";; down) down "$@";; *) echo "up|down?"; exit 2;; esac
