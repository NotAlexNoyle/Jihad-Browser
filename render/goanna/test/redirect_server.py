#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0 (see ../../../LICENSE).
# Tiny redirect server for the F R4 test: GET /a -> 302 -> /b ; GET /b -> 200.
import sys
from http.server import BaseHTTPRequestHandler, HTTPServer

class H(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/a":
            self.send_response(302)
            self.send_header("Location", "/b")
            self.end_headers()
        elif self.path == "/b":
            body = b"<title>B</title><body>redirected-ok</body>"
            self.send_response(200)
            self.send_header("Content-Type", "text/html")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
        elif self.path == "/link":
            # A full-viewport anchor to /b, for the link-clicked test.
            body = (b"<body style='margin:0'>"
                    b"<a href='/b' style='display:block;width:100vw;height:100vh'>x</a>"
                    b"</body>")
            self.send_response(200)
            self.send_header("Content-Type", "text/html")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
        else:
            self.send_response(404)
            self.end_headers()
    def log_message(self, *a):
        pass

if __name__ == "__main__":
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 18080
    HTTPServer(("127.0.0.1", port), H).serve_forever()
