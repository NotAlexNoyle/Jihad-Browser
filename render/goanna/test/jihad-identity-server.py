#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. MPL-2.0.
#
# jihad-identity-server.py — a site that behaves like the bot checks the learned identity answers
# (render/goanna/components/jihadUserAgent.js), so the component can be exercised and READ without
# Cloudflare or Startpage: every request is logged with the User-Agent header it carried, and every
# page reports its navigator.userAgent back, so the header on the wire and the script's view of the
# identity can be compared for the top document, a same-site frame, an image, a script and an XHR.
#
#   python3 -I render/goanna/test/jihad-identity-server.py [port] [logfile]      (default 8078)
#   on the device: http://<host ip>:8078/
#
# Pages:
#   /plain/      an ordinary page (the baseline: the default identity everywhere)
#   /challenge/  ALWAYS answered 403 + "cf-mitigated: challenge" — the learn + one retry, then the
#                no-loop branch: the second load (with the current identity) is shown as it is
#   /sp-gate/    Startpage's gate: a 303 to /sp/captcha-block for any identity but a current
#                Firefox token, the page for that token — the second signature, learn, retry, pass
#   /site/       the learned site's page with a frame, an image, a script, an XHR and a third-party
#                frame (httpbin.org/user-agent shows the header that host received)
#   /log         the log so far, as text
# Every page POSTs its navigator.userAgent to /report?who=<page>; the log line says REPORT.

import sys, time, base64, threading
from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 8078
LOGFILE = sys.argv[2] if len(sys.argv) > 2 else None
MODERN = "Firefox/128"
PNG = base64.b64decode("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg==")
LOG = []
LOCK = threading.Lock()

def record(line):
    line = time.strftime("%H:%M:%S ") + line
    with LOCK:
        LOG.append(line)
        print(line, flush=True)
        if LOGFILE:
            with open(LOGFILE, "a") as f: f.write(line + "\n")

REPORT_JS = """<script>
(function () {
  function post(who, ua) { try { var x = new XMLHttpRequest(); x.open("POST", "/report?who=" + who, true); x.send(ua); } catch (e) {} }
  var ua = navigator.userAgent, el = document.getElementById("ua"); if (el) el.textContent = ua;
  post(%r, ua);
})();
</script>"""

def page(title, body, who, extra=""):
    return ("<!doctype html><html><head><meta charset=utf-8><meta name=viewport content='width=device-width'>"
            "<title>%s</title><style>body{font:22px/1.4 sans-serif;margin:24px}#ua,.hdr{font-family:monospace;"
            "font-size:19px;background:#eef;padding:6px;display:block;margin:6px 0}iframe{width:95%%;height:150px;border:2px solid #888}</style></head>"
            "<body><h1>%s</h1>%s<p>navigator.userAgent: <span id=ua>(script did not run)</span></p>%s%s</body></html>"
            % (title, title, body, REPORT_JS % who, extra))

class H(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"
    def log_message(self, *a): pass
    def _ua(self): return self.headers.get("User-Agent", "(none)")
    def _line(self):
        extra = []
        for h in ("Sec-Fetch-Dest", "Referer"):
            v = self.headers.get(h)
            if v: extra.append("%s=%s" % (h, v))
        record("%s %s UA=%s %s" % (self.command, self.path, self._ua(), " ".join(extra)))
    def _send(self, code, ctype, body, headers=()):
        if isinstance(body, str): body = body.encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        for k, v in headers: self.send_header(k, v)
        self.end_headers()
        self.wfile.write(body)
    def do_POST(self):
        self._line()
        n = int(self.headers.get("Content-Length") or 0)
        body = self.rfile.read(n).decode("utf-8", "replace") if n else ""
        q = parse_qs(urlparse(self.path).query)
        record("REPORT who=%s navigator.userAgent=%s" % (q.get("who", ["?"])[0], body))
        self._send(200, "text/plain", "ok")
    def do_GET(self):
        self._line()
        u = urlparse(self.path); p = u.path; q = parse_qs(u.query); ua = self._ua()
        hdr = "<p>the User-Agent header this page was served to: <span class=hdr>%s</span></p>" % ua
        if p == "/":
            self._send(200, "text/html", "<!doctype html><h1>identity harness</h1><p><a href=/plain/>/plain/</a> <a href=/challenge/>/challenge/</a> <a href=/sp-gate/>/sp-gate/</a> <a href=/site/>/site/</a> <a href=/log>/log</a></p>")
        elif p == "/plain/":
            self._send(200, "text/html", page("plain page", hdr, "plain-top",
                "<img src=/img.png?from=plain><script src=/s.js?from=plain></script><script>var x=new XMLHttpRequest();x.open('GET','/xhr?from=plain',true);x.send();</script>"
                "<p>same-site frame:</p><iframe src=/frame/?from=plain></iframe>"))
        elif p == "/challenge/":
            self._send(403, "text/html", page("CHALLENGE PAGE", "<p>served with <b>cf-mitigated: challenge</b> on EVERY load. Seen at all = the retry was made and the second challenge was left to the site (no loop).</p>" + hdr, "challenge-shown"),
                       [("cf-mitigated", "challenge"), ("Server", "cloudflare")])
        elif p == "/sp-gate/":
            if MODERN in ua:
                self._send(200, "text/html", page("SP-GATE PASSED", "<p>the gate let a current Firefox token through.</p>" + hdr, "sp-gate-passed"))
            else:
                self._send(303, "text/html", "redirecting", [("Location", "/sp/captcha-block?from=harness&had=" + ua.split()[-1])])
        elif p == "/sp/captcha-block":
            self._send(200, "text/html", page("CAPTCHA-BLOCK PAGE", "<p>Startpage's gate page. Seen = the redirect signature was NOT learned (or learning is off).</p>" + hdr, "captcha-block-shown"))
        elif p == "/site/":
            self._send(200, "text/html", page("learned site", hdr, "site-top",
                "<img src=/img.png?from=site><script src=/s.js?from=site></script><script>var x=new XMLHttpRequest();x.open('GET','/xhr?from=site',true);x.send();</script>"
                "<p>same-site frame:</p><iframe src=/frame/?from=site></iframe>"
                "<p>third-party frame (httpbin.org/user-agent shows the header it received):</p><iframe src=https://httpbin.org/user-agent></iframe>"))
        elif p == "/frame/":
            self._send(200, "text/html", page("frame", hdr, (q.get("from", ["?"])[0]) + "-frame"))
        elif p == "/img.png":
            self._send(200, "image/png", PNG)
        elif p == "/s.js":
            self._send(200, "application/javascript", "/* script */")
        elif p == "/xhr":
            self._send(200, "text/plain", "xhr ok")
        elif p == "/log":
            with LOCK: body = "\n".join(LOG) + "\n"
            self._send(200, "text/plain; charset=utf-8", body)
        else:
            self._send(404, "text/plain", "no such page")

if __name__ == "__main__":
    record("identity harness on port %d" % PORT)
    ThreadingHTTPServer(("0.0.0.0", PORT), H).serve_forever()
