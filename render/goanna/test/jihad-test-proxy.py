#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
"""Jihad Browser — a logging HTTP proxy for the device checks of cavekit-proxies-certs.md.

It prints one line per request it carries (the method, the address, the User-Agent, whether a
login came with it), so a test can tell that a page went through it and what Jihad sent on a
CONNECT. Plain HTTP is forwarded; a CONNECT is tunnelled untouched, or, with --mitm, ended here
and the site re-signed with a test CA the way a bumping Squid or WebOne does.

  jihad-test-proxy.py --port 8899                       log and pass everything through
  jihad-test-proxy.py --port 8899 --auth user:secret     ask for a login first (407)
  jihad-test-proxy.py --port 8899 --mitm DIR             re-sign https with DIR/ca.{pem,key}
                                                         (made there if missing)

Asked directly (not as a proxy), it serves:
  /test-ca.crt   the test CA, DER, application/x-x509-ca-cert (the CA question, R6)
  /test-ca.pem   the same, PEM
  /proxy.pac     a PAC file naming this proxy

Test use only: it listens on the LAN and does no access control beyond --auth.
"""
import argparse
import base64
import os
import select
import socket
import ssl
import subprocess
import sys
import threading
import time
import http.client
import urllib.parse

ARGS = None
LOCK = threading.Lock()


def log(line):
    with LOCK:
        sys.stdout.write(time.strftime("%H:%M:%S ") + line + "\n")
        sys.stdout.flush()


def read_head(sock):
    data = b""
    while b"\r\n\r\n" not in data:
        chunk = sock.recv(4096)
        if not chunk:
            break
        data += chunk
        if len(data) > 65536:
            break
    head, _, rest = data.partition(b"\r\n\r\n")
    lines = head.decode("latin-1").split("\r\n")
    hdrs = {}
    for l in lines[1:]:
        k, _, v = l.partition(":")
        hdrs[k.strip().lower()] = v.strip()
    return lines[0] if lines else "", hdrs, rest


def ca_paths():
    d = ARGS.mitm
    return os.path.join(d, "ca.pem"), os.path.join(d, "ca.key")


def ensure_ca():
    pem, key = ca_paths()
    if os.path.exists(pem) and os.path.exists(key):
        return
    os.makedirs(ARGS.mitm, exist_ok=True)
    subprocess.run(["openssl", "req", "-x509", "-newkey", "rsa:2048", "-nodes", "-sha256", "-days", "30",
                    "-subj", "/O=Jihad test/CN=Jihad Test Proxy CA", "-keyout", key, "-out", pem,
                    "-addext", "basicConstraints=critical,CA:TRUE", "-addext", "keyUsage=critical,keyCertSign,cRLSign"],
                   check=True, capture_output=True)


LEAVES = {}


def leaf_for(host):
    with LOCK:
        if host in LEAVES:
            return LEAVES[host]
        pem, key = ca_paths()
        d = os.path.join(ARGS.mitm, "leaf")
        os.makedirs(d, exist_ok=True)
        safe = "".join(c if c.isalnum() or c in ".-" else "_" for c in host)
        lk, lc, csr, ext = (os.path.join(d, safe + s) for s in (".key", ".pem", ".csr", ".ext"))
        with open(ext, "w") as f:
            f.write("subjectAltName=DNS:%s\nbasicConstraints=CA:FALSE\nextendedKeyUsage=serverAuth\n" % host)
        subprocess.run(["openssl", "req", "-newkey", "rsa:2048", "-nodes", "-subj", "/CN=" + host,
                        "-keyout", lk, "-out", csr], check=True, capture_output=True)
        subprocess.run(["openssl", "x509", "-req", "-in", csr, "-CA", pem, "-CAkey", key, "-CAcreateserial",
                        "-days", "7", "-sha256", "-extfile", ext, "-out", lc], check=True, capture_output=True)
        LEAVES[host] = (lc, lk)
        return LEAVES[host]


def serve_local(client, path):
    if path in ("/test-ca.crt", "/test-ca.pem") and ARGS.mitm:
        ensure_ca()
        pem = ca_paths()[0]
        if path.endswith(".crt"):
            body = subprocess.run(["openssl", "x509", "-in", pem, "-outform", "DER"], capture_output=True, check=True).stdout
            ctype = "application/x-x509-ca-cert"
        else:
            body = open(pem, "rb").read()
            ctype = "text/plain"
    elif path == "/proxy.pac":
        host = client.getsockname()[0]
        body = ('function FindProxyForURL(url, host) { return "PROXY %s:%d"; }\n' % (host, ARGS.port)).encode()
        ctype = "application/x-ns-proxy-autoconfig"
    else:
        body, ctype = b"jihad-test-proxy\n", "text/plain"
    client.sendall(b"HTTP/1.1 200 OK\r\nContent-Type: " + ctype.encode() + b"\r\nContent-Length: "
                   + str(len(body)).encode() + b"\r\nConnection: close\r\n\r\n" + body)
    log("SERVED %s (%d bytes, %s)" % (path, len(body), ctype))


def pipe(a, b):
    socks = [a, b]
    try:
        while True:
            r, _, x = select.select(socks, [], socks, 120)
            if x or not r:
                return
            for s in r:
                data = s.recv(65536)
                if not data:
                    return
                (b if s is a else a).sendall(data)
    except OSError:
        return


def handle(client, addr):
    try:
        line, hdrs, rest = read_head(client)
        if not line:
            return
        method, target, _ = (line.split(" ") + ["", ""])[:3]
        ua = hdrs.get("user-agent", "-")
        login = "login" if "proxy-authorization" in hdrs else "no login"
        if method != "CONNECT" and target.startswith("/"):
            serve_local(client, target)
            return
        log("%s %s from %s [%s] UA=%s" % (method, target, addr[0], login, ua))
        if ARGS.auth:
            want = "Basic " + base64.b64encode(ARGS.auth.encode()).decode()
            if hdrs.get("proxy-authorization") != want:
                client.sendall(b"HTTP/1.1 407 Proxy Authentication Required\r\n"
                               b"Proxy-Authenticate: Basic realm=\"jihad-test\"\r\nContent-Length: 0\r\n"
                               b"Connection: close\r\n\r\n")
                log("  -> 407 (login %s)" % ("wrong" if "proxy-authorization" in hdrs else "missing"))
                return
        if method == "CONNECT":
            host, _, port = target.rpartition(":")
            if ARGS.mitm:
                client.sendall(b"HTTP/1.1 200 Connection established\r\n\r\n")
                ensure_ca()
                lc, lk = leaf_for(host)
                ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
                ctx.load_cert_chain(lc, lk)
                try:
                    tls = ctx.wrap_socket(client, server_side=True)
                except ssl.SSLError as e:
                    log("  -> re-signed handshake refused by the browser (%s)" % e.reason)
                    return
                up = ssl.create_default_context().wrap_socket(socket.create_connection((host, int(port)), 20),
                                                              server_hostname=host)
                log("  -> re-signed %s with the test CA" % host)
                pipe(tls, up)
                return
            up = socket.create_connection((host, int(port)), 20)
            client.sendall(b"HTTP/1.1 200 Connection established\r\n\r\n")
            log("  -> tunnel to %s" % target)
            pipe(client, up)
            return
        u = urllib.parse.urlsplit(target)
        conn = http.client.HTTPConnection(u.hostname, u.port or 80, timeout=20)
        body = rest
        n = int(hdrs.get("content-length", "0") or 0)
        while len(body) < n:
            body += client.recv(65536)
        fwd = {k: v for k, v in hdrs.items() if k not in ("proxy-connection", "proxy-authorization", "connection")}
        fwd["connection"] = "close"
        conn.request(method, (u.path or "/") + ("?" + u.query if u.query else ""), body or None, fwd)
        r = conn.getresponse()
        data = r.read()
        out = "HTTP/1.1 %d %s\r\n" % (r.status, r.reason)
        for k, v in r.getheaders():
            if k.lower() in ("transfer-encoding", "connection", "content-length"):
                continue
            out += "%s: %s\r\n" % (k, v)
        out += "Via: 1.1 jihad-test-proxy\r\nContent-Length: %d\r\nConnection: close\r\n\r\n" % len(data)
        client.sendall(out.encode("latin-1") + data)
        log("  -> %d, %d bytes" % (r.status, len(data)))
    except Exception as e:
        log("  !! %s" % e)
    finally:
        try:
            client.close()
        except OSError:
            pass


def main():
    global ARGS
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", type=int, default=8899)
    ap.add_argument("--auth", help="user:password; ask every request for it (407)")
    ap.add_argument("--mitm", help="a directory for the test CA; re-sign https with it")
    ARGS = ap.parse_args()
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(("0.0.0.0", ARGS.port))
    s.listen(64)
    log("listening on :%d%s%s" % (ARGS.port, " (login required)" if ARGS.auth else "",
                                  " (re-signing https)" if ARGS.mitm else ""))
    while True:
        c, a = s.accept()
        threading.Thread(target=handle, args=(c, a), daemon=True).start()


if __name__ == "__main__":
    main()
