#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# flash-serve.py -- the corpus over the local network (cavekit-flash.md R1, T-573): the cache's files as they are, and for each
# title a page holding it the same way in this browser and in the stock one, so an internet failure is never a Flash verdict.
#
#   tools/host/flash-serve.py [--port 8097]            (8078 is jihad-identity-server.py's)
#   /title/ID[?w=W&h=H&wmode=W]    the movie at the page's top left, fitted to the card's width, element id "m"
#                                   a video: its player with its media; the report elements of jihad-flash-instr.html
#   /swf/, /game/ID.swf ...        the cache's own paths
# Run with python3 -I.
import argparse, http.server, json, os, socketserver, sys, urllib.parse

HERE = os.path.dirname(os.path.abspath(__file__))
CACHE = os.environ.get("JIHAD_FLASH_CACHE", os.path.expanduser("~/.cache/jihad-flash"))
M = {t["id"]: t for t in json.load(open(os.path.join(HERE, "flash-corpus.json")))["titles"]}
CARD_W = 768


def path_of(t): return "/%s/%s%s" % (t["class"], t["id"], os.path.splitext(urllib.parse.urlparse(t["source"]).path)[1].lower())


def page(tid, q, host):
    t = M[tid]; movie = path_of(t); fv = ""; sw, sh = t.get("width") or 640, t.get("height") or 480
    if t.get("player"):                         # a video: its player, the media by FlashVars as that player takes them
        p = M[t["player"]]; media = "http://%s%s" % (host, path_of(t)); movie = path_of(p)
        if t["player"] == "flowplayer": fv = "config=" + urllib.parse.quote(json.dumps({"clip": {"url": media, "autoPlay": True}}))
        elif t["player"] == "mediaelement": fv = "file=" + urllib.parse.quote(media) + "&autoplay=true&isvideo=true&timerrate=250"
        else: fv = "src=" + urllib.parse.quote(media)
        sw, sh = t.get("width") or 640, t.get("height") or 360
    if t["id"] == "mp3-player-multi": fv = "mp3=" + urllib.parse.quote("http://%s%s" % (host, path_of(M["sample-mp3"])))
    k = min(1.0, CARD_W / float(sw)) if "w" not in q else 1.0
    w = int(q.get("w", [round(sw * k)])[0]); h = int(q.get("h", [round(sh * k)])[0])
    wmode = ('wmode="%s"' % q["wmode"][0]) if "wmode" in q else ""
    return ("""<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>%s</title>
<style>body{margin:0;min-width:768px;background:#888;font:14px monospace}#m{display:block}</style></head><body>
<embed id="m" src="%s" type="application/x-shockwave-flash" width="%d" height="%d" allowScriptAccess="always" allowFullScreen="true" flashvars="%s" %s>
<div id="count">0</div><div id="last">-</div>
<script>var n=0;function jihadFlash(s){n++;document.getElementById("count").textContent=n;document.getElementById("last").textContent=s;}</script>
</body></html>""" % (tid, movie, w, h, fv, wmode)).encode()


class H(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *a, **k): super().__init__(*a, directory=CACHE, **k)
    def log_message(self, *a): pass
    def end_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*"); super().end_headers()
    def do_GET(self):
        u = urllib.parse.urlparse(self.path)
        if u.path.startswith("/title/"):
            tid = u.path[7:]
            if tid not in M: self.send_error(404); return
            b = page(tid, urllib.parse.parse_qs(u.query), self.headers.get("Host", "192.168.1.245:8097"))
            self.send_response(200); self.send_header("Content-Type", "text/html; charset=utf-8"); self.send_header("Content-Length", str(len(b))); self.end_headers(); self.wfile.write(b); return
        if u.path == "/crossdomain.xml":
            b = b'<?xml version="1.0"?><cross-domain-policy><allow-access-from domain="*"/></cross-domain-policy>'
            self.send_response(200); self.send_header("Content-Type", "text/xml"); self.send_header("Content-Length", str(len(b))); self.end_headers(); self.wfile.write(b); return
        super().do_GET()
    def guess_type(self, p):
        if p.endswith(".swf"): return "application/x-shockwave-flash"
        if p.endswith(".flv"): return "video/x-flv"
        return super().guess_type(p)


class S(socketserver.ThreadingMixIn, http.server.HTTPServer):
    daemon_threads = True; allow_reuse_address = True


def main():
    ap = argparse.ArgumentParser(); ap.add_argument("--port", type=int, default=8097); o = ap.parse_args()
    S(("0.0.0.0", o.port), H).serve_forever()


if __name__ == "__main__":
    main()
