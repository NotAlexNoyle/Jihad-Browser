#!/bin/bash
# Copyright 2026 the Jihad Browser project.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Download LIFECYCLE test (cavekit-browser-services R4): builds the real
# jihad-browserserver daemon + a YAP adapter stand-in and drives two downloads
# from a local HTTP server, asserting the frozen daemon->adapter messages:
#   msgDownloadStart (0x2010) -> msgDownloadProgress (0x2011) ->
#   msgDownloadFinished (0x2013, mime + temp path), and
#   cancelDownload (0x1015) aborting an in-progress download (msgDownloadError,
#   never msgDownloadFinished).
# Sibling of build-download-test.sh, which covers only the MIME handoff capture.
set -uo pipefail
DIST=/out/obj-jihad-goanna/dist
R=/jihad/render
BS=/jihad/render/browserserver
[ -e "$DIST/bin/libxul.so" ] || { echo "ERROR: build the engine first"; exit 1; }

CXX=${CXX:-g++}
CXXFLAGS="-std=gnu++17 -fPIC -fno-rtti -fno-exceptions -O2 -g0"
GLIB=$(pkg-config --cflags glib-2.0 gthread-2.0);  GLIBL=$(pkg-config --libs glib-2.0 gthread-2.0)
GTKC=$(pkg-config --cflags gtk+-2.0);              GTKL=$(pkg-config --libs gtk+-2.0)
ENGINC="-include $DIST/include/mozilla-config.h -I$DIST/include -I$DIST/include/nspr"
YAPINC="-I$BS/Yap -I$BS/Src"
XULLINK="$DIST/sdk/lib/libxpcomglue_s.a -L$DIST/bin -lxul $DIST/sdk/lib/libmozglue.a -lnspr4 -lplc4 -lplds4"

echo "== compiling libYap (incl YapClient) + BrowserServerBase =="
for f in YapPacket YapProxy YapServer YapClient; do
  $CXX $CXXFLAGS $YAPINC $GLIB -c "$BS/Yap/$f.cpp" -o "/out/$f.o" || exit 10
done
$CXX $CXXFLAGS $YAPINC $GLIB -c "$BS/Src/BrowserServerBase.cpp" -o /out/BrowserServerBase.o || exit 11

echo "== compiling Goanna backend + daemon =="
$CXX $CXXFLAGS $ENGINC $GTKC -c "$R/goanna/EngineHost.cpp"        -o /out/EngineHost.o        || exit 12
$CXX $CXXFLAGS $ENGINC $GTKC -c "$R/goanna/DialogService.cpp"     -o /out/DialogService.o     || exit 12
$CXX $CXXFLAGS $ENGINC $GTKC -c "$R/goanna/DownloadService.cpp"   -o /out/DownloadService.o   || exit 12
$CXX $CXXFLAGS $ENGINC $GTKC -c "$R/goanna/GoannaRenderPage.cpp"  -o /out/GoannaRenderPage.o  || exit 13
$CXX $CXXFLAGS $ENGINC $GTKC -c "$R/goanna/BrowserPageGoanna.cpp" -o /out/BrowserPageGoanna.o || exit 14
$CXX $CXXFLAGS $YAPINC $GLIB -c "$BS/JihadBrowserServer.cpp"      -o /out/JihadBrowserServer.o || exit 15
$CXX $CXXFLAGS $YAPINC $ENGINC $GTKC $GLIB -c "$BS/Main.cpp"      -o /out/bs_main.o           || exit 16
$CXX /out/bs_main.o /out/JihadBrowserServer.o /out/BrowserServerBase.o \
     /out/YapPacket.o /out/YapProxy.o /out/YapServer.o \
     /out/BrowserPageGoanna.o /out/GoannaRenderPage.o /out/EngineHost.o /out/DialogService.o /out/DownloadService.o \
     $XULLINK $GTKL $GLIBL -Wl,-rpath,"$DIST/bin" -ldl -lpthread -o /out/jihad-browserserver || exit 17

echo "== compiling download adapter stand-in =="
$CXX $CXXFLAGS $YAPINC $GLIB -c "$BS/test/download_client.cpp" -o /out/download_client.o || exit 18
$CXX /out/download_client.o /out/YapClient.o /out/YapPacket.o \
     $GLIBL -ldl -lpthread -o /out/jihad-download-client || exit 19

# --- local HTTP server: one fast attachment + one deliberately slow one -------
PORT=${JIHAD_DL_PORT:-8137}
SIZE=${JIHAD_DL_SIZE:-524288}
cat > /out/dlserver.py <<'PYEOF'
import sys, time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

PORT = int(sys.argv[1]); SIZE = int(sys.argv[2])
CHUNK = 32768
SLOW_SIZE = 32 * 1024 * 1024      # never actually completes within the test

class H(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"
    def log_message(self, *a): pass

    def _attach(self, total, name):
        self.send_response(200)
        self.send_header("Content-Type", "application/octet-stream")
        self.send_header("Content-Disposition", 'attachment; filename="%s"' % name)
        self.send_header("Content-Length", str(total))
        self.end_headers()

    def do_GET(self):
        try:
            if self.path.startswith("/big"):
                self._attach(SIZE, "jihad-payload.bin")
                sent = 0
                while sent < SIZE:
                    n = min(CHUNK, SIZE - sent)
                    self.wfile.write(b"J" * n); self.wfile.flush()
                    sent += n
                    time.sleep(0.03)          # several progress ticks
            elif self.path.startswith("/slow"):
                self._attach(SLOW_SIZE, "jihad-slow.bin")
                sent = 0
                while sent < SLOW_SIZE:
                    self.wfile.write(b"S" * CHUNK); self.wfile.flush()
                    sent += CHUNK
                    time.sleep(0.25)          # ~4 min: cancelled long before this ends
            else:
                self.send_response(404); self.send_header("Content-Length", "0"); self.end_headers()
        except (BrokenPipeError, ConnectionResetError):
            pass                              # the cancel closed the socket: expected

ThreadingHTTPServer.daemon_threads = True
ThreadingHTTPServer(("127.0.0.1", PORT), H).serve_forever()
PYEOF

rm -rf /out/dltmp && mkdir -p /out/dltmp
python3 /out/dlserver.py "$PORT" "$SIZE" &
httpd=$!
sleep 1

echo "== download lifecycle round-trip under Xvfb =="
export DIST PORT SIZE
xvfb-run -a -s "-screen 0 1024x768x24" bash -c '
  export JIHAD_DISABLE_OMTC=1
  export JIHAD_OFFSCREEN=1
  export JIHAD_DOWNLOAD_DIR=/out/dltmp
  export JIHAD_PROFILE_DIR=/out/dlprofile
  export LD_LIBRARY_PATH="$DIST/bin"
  /out/jihad-browserserver "$DIST/bin" &
  dpid=$!
  sleep 8                       # let the engine come up + open the socket
  JIHAD_DL_URL="http://127.0.0.1:$PORT/big" \
  JIHAD_DL_SLOW="http://127.0.0.1:$PORT/slow" \
  JIHAD_DL_SIZE="$SIZE" \
    timeout 70 /out/jihad-download-client
  rc=$?
  kill "$dpid" 2>/dev/null; wait 2>/dev/null
  exit $rc
'
rc=$?
kill "$httpd" 2>/dev/null
echo "== files left in /out/dltmp: $(ls -l /out/dltmp | tail -n +2 | wc -l) =="
ls -l /out/dltmp || true
echo "== download2 exit: $rc =="
exit $rc
