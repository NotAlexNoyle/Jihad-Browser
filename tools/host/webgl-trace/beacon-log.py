#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# beacon-log.py LOGFILE (T-672): a host-side log for a page's beacons on :8098 (every POST body appended to LOGFILE, one line
# per body, with the host's time) and the server of gltrace.js and console.js beside it (GET). Run with python3 -I.
import sys, time
from http.server import BaseHTTPRequestHandler, HTTPServer

OUT = sys.argv[1]

class H(BaseHTTPRequestHandler):
    def do_POST(self):
        n = int(self.headers.get("Content-Length") or 0)
        body = self.rfile.read(n).decode("utf-8", "replace")
        with open(OUT, "a") as f:
            f.write("%.3f %s\n" % (time.time(), body.replace("\n", " | ")))
        self.send_response(204)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
    def do_GET(self):
        # the tracer itself (gltrace.js beside this script), for a page that loads it with a script tag
        import os
        name = os.path.basename(self.path.split("?")[0]) or "gltrace.js"
        if name not in ("gltrace.js", "console.js"):
            self.send_response(404); self.end_headers(); return
        body = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), name), "rb").read()
        self.send_response(200)
        self.send_header("Content-Type", "application/javascript")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)
    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Headers", "*")
        self.end_headers()
    def log_message(self, *a):
        pass

HTTPServer(("192.168.1.245", 8098), H).serve_forever()
