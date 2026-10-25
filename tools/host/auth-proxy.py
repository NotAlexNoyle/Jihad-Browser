#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
"""A small HTTP proxy that asks for a login (407 Proxy-Authenticate: Basic) — the bench for
cavekit-proxies-certs.md R5 / docs/FINGER-TESTS.md ("a proxy that asks for a login"): the card's
dialog must take the user name and password and the page must load. Plain GET/HEAD are forwarded;
CONNECT tunnels (https) as well. Nothing is cached or logged beyond one line per request.

  tools/host/auth-proxy.py [--port 3129] [--user jihad --password touchpad]
"""
import argparse, base64, http.server, select, socket, socketserver, sys, urllib.request

class Handler(http.server.BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"
    user = "jihad"; password = "touchpad"

    def _authed(self):
        h = self.headers.get("Proxy-Authorization", "")
        want = "Basic " + base64.b64encode(("%s:%s" % (self.user, self.password)).encode()).decode()
        return h.strip() == want

    def _ask(self):
        self.send_response(407, "Proxy Authentication Required")
        self.send_header("Proxy-Authenticate", 'Basic realm="Jihad test proxy"')
        self.send_header("Content-Length", "0"); self.send_header("Connection", "close")
        self.end_headers()
        self.log_message("407 asked %s %s", self.command, self.path)

    def do_CONNECT(self):
        if not self._authed(): return self._ask()
        host, _, port = self.path.partition(":")
        try: up = socket.create_connection((host, int(port or 443)), timeout=20)
        except OSError as e:
            self.send_error(502, str(e)); return
        self.send_response(200, "Connection established"); self.end_headers()
        self.log_message("CONNECT %s ok", self.path)
        a, b = self.connection, up
        try:
            while True:
                r, _, _ = select.select([a, b], [], [], 60)
                if not r: break
                for s in r:
                    d = s.recv(65536)
                    if not d: return
                    (b if s is a else a).sendall(d)
        finally:
            up.close()

    def _forward(self):
        if not self._authed(): return self._ask()
        req = urllib.request.Request(self.path, method=self.command)
        for k, v in self.headers.items():
            if k.lower() not in ("proxy-authorization", "proxy-connection", "connection", "host"): req.add_header(k, v)
        try:
            with urllib.request.urlopen(req, timeout=20) as r:
                body = r.read()
                self.send_response(r.status)
                for k, v in r.headers.items():
                    if k.lower() not in ("transfer-encoding", "connection", "content-length"): self.send_header(k, v)
                self.send_header("Content-Length", str(len(body))); self.send_header("Via", "1.1 jihad-auth-proxy")
                self.end_headers(); self.wfile.write(body)
        except urllib.error.HTTPError as e:
            body = e.read(); self.send_response(e.code); self.send_header("Content-Length", str(len(body))); self.end_headers(); self.wfile.write(body)
        except Exception as e:
            self.send_error(502, str(e))
        self.log_message("%s %s forwarded", self.command, self.path)

    do_GET = _forward; do_HEAD = _forward; do_POST = _forward

class Server(socketserver.ThreadingMixIn, http.server.HTTPServer):
    daemon_threads = True; allow_reuse_address = True

if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", type=int, default=3129); ap.add_argument("--user", default="jihad"); ap.add_argument("--password", default="touchpad")
    a = ap.parse_args()
    Handler.user, Handler.password = a.user, a.password
    print("auth-proxy on 0.0.0.0:%d, login %s / %s" % (a.port, a.user, a.password), flush=True)
    Server(("0.0.0.0", a.port), Handler).serve_forever()
