#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# auth-proxy.sh up|down SHELL (host side): the proxy that asks for a login (tools/host/auth-proxy.py, port 3129, jihad / touchpad)
# running on the host for a script.
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; ROOT="$(cd "$HERE/../../../../.." && pwd)"; PID=/tmp/jihad-auth-proxy.pid
stop() { [ -s "$PID" ] && kill "$(cat "$PID")" 2>/dev/null; rm -f "$PID"; }
if [ "$1" = up ]; then
  stop
  nohup python3 -I "$ROOT/tools/host/auth-proxy.py" --port 3129 >/dev/null 2>&1 &
  echo $! > "$PID"; disown; sleep 1; echo "auth proxy: running"
else stop; echo "auth proxy: stopped"; fi
