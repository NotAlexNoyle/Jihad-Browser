#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0 (see ../../../LICENSE).
# Tiny self-signed HTTPS server for the G R5 TLS test. GET / -> 200 "tls-ok".
# Args: <port> <certfile> <keyfile>
import sys, ssl
from http.server import BaseHTTPRequestHandler, HTTPServer

class H(BaseHTTPRequestHandler):
    def do_GET(self):
        body = b"<title>TLS</title><body>tls-ok</body>"
        self.send_response(200)
        self.send_header("Content-Type", "text/html")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)
    def log_message(self, *a):
        pass

if __name__ == "__main__":
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 18443
    cert = sys.argv[2] if len(sys.argv) > 2 else "cert.pem"
    key = sys.argv[3] if len(sys.argv) > 3 else "key.pem"
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    ctx.load_cert_chain(cert, key)
    srv = HTTPServer(("127.0.0.1", port), H)
    srv.socket = ctx.wrap_socket(srv.socket, server_side=True)
    srv.serve_forever()
