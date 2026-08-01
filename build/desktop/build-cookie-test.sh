#!/bin/bash
# Copyright 2026 NotAlexNoyle.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Cookie PERSISTENCE test (cavekit-browser-services R2): a local HTTP server
# hands out a PERSISTENT cookie (future Max-Age); the engine is started, shut
# down cleanly, and started AGAIN against the same profile dir. The cookie must
# come back on the second run and cookies.sqlite must exist in the profile.
# This is the desktop reproduction of the device gap logged 2026-07-19 s4.
set -uo pipefail
DIST=/out/obj-jihad-goanna/dist
SRC=/jihad/render/goanna
OUT=/out/cookie_test
PROF=/out/cookieprofile
PORT=${JIHAD_COOKIE_PORT:-8138}
[ -e "$DIST/bin/libxul.so" ] || { echo "ERROR: build the engine first"; exit 1; }

CXX=${CXX:-g++}
GTK_CFLAGS=$(pkg-config --cflags gtk+-2.0 glib-2.0); GTK_LIBS=$(pkg-config --libs gtk+-2.0 glib-2.0)
CXXFLAGS="-std=gnu++17 -fPIC -fno-rtti -fno-exceptions -O2 -g0"
INCS="-include $DIST/include/mozilla-config.h -I$DIST/include -I$DIST/include/nspr $GTK_CFLAGS"

$CXX $CXXFLAGS $INCS -c "$SRC/EngineHost.cpp"        -o /out/EngineHost.o       || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/DialogService.cpp"     -o /out/DialogService.o    || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/DownloadService.cpp"   -o /out/DownloadService.o  || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/GoannaRenderPage.cpp"  -o /out/GoannaRenderPage.o || exit 11
$CXX $CXXFLAGS $INCS -c "$SRC/test/cookie_test.cpp"  -o /out/cookie_test.o      || exit 12
$CXX /out/cookie_test.o /out/GoannaRenderPage.o /out/EngineHost.o /out/DialogService.o /out/DownloadService.o \
  "$DIST/sdk/lib/libxpcomglue_s.a" -L"$DIST/bin" -lxul "$DIST/sdk/lib/libmozglue.a" \
  -lnspr4 -lplc4 -lplds4 $GTK_LIBS -Wl,-rpath,"$DIST/bin" -ldl -lpthread -o "$OUT" || exit 13
echo "== built $OUT =="

cat > /out/cookiesrv.py <<'PYEOF'
import sys
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

PORT = int(sys.argv[1])

class H(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"
    def log_message(self, *a): pass
    def _html(self, body, extra=None):
        data = body.encode()
        self.send_response(200)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(data)))
        if extra:
            for k, v in extra:
                self.send_header(k, v)
        self.end_headers()
        self.wfile.write(data)
    def do_GET(self):
        if self.path.startswith("/set"):
            # PERSISTENT cookie: a session cookie is not meant to survive a restart,
            # so only a Max-Age/Expires cookie proves persistence.
            self._html("<title>SET</title>set",
                       [("Set-Cookie", "jihadpersist=1; Max-Age=3600; Path=/")])
        elif self.path.startswith("/echo"):
            c = self.headers.get("Cookie", "")
            self._html("<title>COOKIE[%s]</title>echo" % c)
        else:
            self._html("<title>404</title>no")

ThreadingHTTPServer.daemon_threads = True
ThreadingHTTPServer(("127.0.0.1", PORT), H).serve_forever()
PYEOF

python3 /out/cookiesrv.py "$PORT" &
httpd=$!
sleep 1

# F-8: the idempotence guard greps for a 'jihad-embed' marker, so the line it
# appends has to CARRY that marker. It did not, so the guard never matched its
# own output and every run of this script appended another copy of the pref to
# $DIST/bin/goanna.js — a SHARED build output that every other desktop test reads,
# growing without bound. (Same defect in a dozen sibling scripts; fixed the same
# way. The scripts that already wrote the marker — download/touch/dialog — were
# the only reason the growth ever stopped.)
if ! grep -q 'jihad-embed' "$DIST/bin/goanna.js" 2>/dev/null; then
  echo 'pref("layers.offmainthreadcomposition.force-disabled", true); // jihad-embed' >> "$DIST/bin/goanna.js"
fi

rm -rf "$PROF"
export JIHAD_DISABLE_OMTC=1
export JIHAD_PROFILE_DIR="$PROF"
export LD_LIBRARY_PATH="$DIST/bin"
BASE="http://127.0.0.1:$PORT"

echo "== run 1: set the cookie =="
xvfb-run -a -s "-screen 0 1024x768x24" "$OUT" "$DIST/bin" set "$BASE"
rc1=$?
echo "== profile after run 1 =="
ls -l "$PROF" || true

echo "== run 2: RESTART, cookie must still be sent =="
xvfb-run -a -s "-screen 0 1024x768x24" "$OUT" "$DIST/bin" check "$BASE"
rc2=$?

kill "$httpd" 2>/dev/null
echo "== run1=$rc1 run2=$rc2 =="
if [ "$rc1" = 0 ] && [ "$rc2" = 0 ]; then
  echo "COOKIE-PERSISTENCE PASS"; exit 0
else
  echo "COOKIE-PERSISTENCE FAIL"; exit 1
fi
