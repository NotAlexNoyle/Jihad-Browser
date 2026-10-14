#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# Serve certificate files over plain HTTP as application/x-x509-ca-cert, the way a router or a
# proxy's setup page offers its CA, so a card can open one as a page (cavekit-proxies-certs.md R6).
#   tools/test-servers/cert-server.py [--port 8896] /ca.crt=path/to/ca.crt /router.crt=router.crt ...
# Binds every address, so the TouchPad on the LAN reaches it; stop it when the test is done.
import argparse, http.server, os

ap = argparse.ArgumentParser()
ap.add_argument("--port", type=int, default=8896)
ap.add_argument("files", nargs="+", help="URL path=file, e.g. /ca.crt=testca.crt")
args = ap.parse_args()
FILES = {}
for spec in args.files:
    path, _, f = spec.partition("=")
    if not path.startswith("/") or not f:
        ap.error(f"not URL-path=file: {spec}")
    FILES[path] = os.path.abspath(f)

class H(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        f = FILES.get(self.path)
        if not f:
            self.send_response(404); self.end_headers(); return
        with open(f, "rb") as fh:
            b = fh.read()
        self.send_response(200)
        self.send_header("Content-Type", "application/x-x509-ca-cert")
        self.send_header("Content-Length", str(len(b)))
        self.end_headers()
        self.wfile.write(b)

http.server.HTTPServer(("0.0.0.0", args.port), H).serve_forever()
