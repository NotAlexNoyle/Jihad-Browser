#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# flash-serve.py -- the corpus over the local network (cavekit-flash.md R1, T-573): the cache's files as they are, and for each
# title a page holding it the same way in this browser and in the stock one, so an internet failure is never a Flash verdict.
#
#   tools/host/flash-serve.py [--port 8097]            (8078 is jihad-identity-server.py's)
#   /title/ID[?w=W&h=H&wmode=W&quality=Q]    the movie at the page's top left, fitted to the card's width, element id "m"
#                                   a video: its player with its media; the report elements of jihad-flash-instr.html
#   /slow/BPS/STALL_AT/STALL_S/PATH a cache file throttled, with one stall
#   /swf/, /game/ID.swf ...        the cache's own paths
# Run with python3 -I.
import argparse, http.server, json, os, socketserver, sys, urllib.parse

HERE = os.path.dirname(os.path.abspath(__file__))
CACHE = os.environ.get("JIHAD_FLASH_CACHE", os.path.expanduser("~/.cache/jihad-flash"))
M = {t["id"]: t for t in json.load(open(os.path.join(HERE, "flash-corpus.json")))["titles"]}
CARD_W = 768


def path_of(t): return "/%s/%s%s" % (t["class"], t["id"], os.path.splitext(urllib.parse.urlparse(t["source"]).path)[1].lower())


GALLERY_COLOURS = [(200, 40, 40), (40, 160, 60), (40, 80, 200), (220, 180, 30), (150, 60, 170), (30, 170, 180)]


def gallery_jpeg(n):
    # the postcard viewer's cards, made here (the repository holds no pictures): a flat colour and its number in bars
    from PIL import Image, ImageDraw
    import io
    im = Image.new("RGB", (400, 300), GALLERY_COLOURS[n % len(GALLERY_COLOURS)]); d = ImageDraw.Draw(im)
    for k in range(n + 1): d.rectangle((30 + 50 * k, 100, 60 + 50 * k, 200), fill=(255, 255, 255))
    b = io.BytesIO(); im.save(b, "JPEG", quality=85, progressive=False); return b.getvalue()


GALLERY_XML = ('<?xml version="1.0" encoding="UTF-8"?>\n<gallery cellDimension="400" columns="3" zoomOutPerc="15" zoomInPerc="100" frameWidth="10" '
               'frameColor="0xFFFFFF" captionColor="0xFFFFFF" enableRightClickOpen="false">\n'
               + "".join("<image><url>/gallery/%d.jpg</url><caption>card %d</caption></image>\n" % (i, i + 1) for i in range(6)) + "</gallery>\n").encode()

YUI_PAGE = """<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>yui-charts</title>
<style>body{margin:0;min-width:768px;background:#888;font:14px monospace}#m{display:block;width:500px;height:375px;background:#fff}</style>
%s</head><body>
<div id="m"></div>
<div id="count">0</div><div id="last">-</div>
<script>var n=0;function jihadFlash(s){n++;document.getElementById("count").textContent=n;document.getElementById("last").textContent=s;}
window.onerror = function(m, f, l){ jihadFlash("error " + m + " " + String(f).split("/").pop() + ":" + l); };
// YUI 2 Charts as its own page uses it: the data goes to the movie by ExternalInterface, a tapped column comes back as an event
// YUI 2 looks for Flash only in a browser its sniffing knows (Gecko/, AppleWebKit/ or Opera in the user agent). The
// page gives it no help: until the user agent carried Gecko/52.9 (2026-11-16) its detection said "no Flash" here and
// nothing was embedded, so this chart drawing at all is also the test of that token.
YAHOO.widget.Chart.SWFURL = "/app/yui-charts.swf";
var rows = [{month:"Jan",rent:880,util:894},{month:"Feb",rent:880,util:901},{month:"Mar",rent:880,util:889},{month:"Apr",rent:880,util:890},{month:"May",rent:910,util:903}];
var ds = new YAHOO.util.DataSource(rows); ds.responseType = YAHOO.util.DataSource.TYPE_JSARRAY; ds.responseSchema = {fields:["month","rent","util"]};
var chart = new YAHOO.widget.ColumnChart("m", ds, {xField:"month", series:[{displayName:"Rent",yField:"rent"},{displayName:"Utilities",yField:"util"}], wmode:"window"});
chart.subscribe("contentReady", function(){ jihadFlash("ready"); });
chart.subscribe("itemClickEvent", function(e){ jihadFlash("click " + e.index + " " + e.item.month + " series " + e.seriesIndex); });
chart.subscribe("itemMouseOverEvent", function(e){ document.title = "over " + e.index; });
</script></body></html>"""


def page(tid, q, host):
    if tid == "yui-charts":
        return (YUI_PAGE % "".join('<script src="/asset/%s.js"></script>' % a for a in ("yui-yahoo-dom-event", "yui-element", "yui-json", "yui-datasource", "yui-swfdetect", "yui-swf", "yui-charts-js"))).encode()
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
    if "quality" in q: wmode += ' quality="%s"' % q["quality"][0]
    return ("""<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>%s</title>
<style>body{margin:0;min-width:768px;background:#888;font:14px monospace}#m{display:block}</style></head><body>
<embed id="m" src="%s" type="application/x-shockwave-flash" width="%d" height="%d" allowScriptAccess="always" allowFullScreen="true" flashvars="%s" %s>
<div id="count">0</div><div id="last">-</div>
<script>var n=0;function jihadFlash(s){n++;document.getElementById("count").textContent=n;document.getElementById("last").textContent=s;}</script>
</body></html>""" % (tid, movie, w, h, fv, wmode)).encode()


class H(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *a, **k): super().__init__(*a, directory=CACHE, **k)
    def log_message(self, *a): pass
    def log_request(self, code="-", size="-"):
        # a movie asking for a file the cache does not hold (its next scene, its level data) is a limit of the corpus: logged
        if str(code)[:1] in "45": sys.stderr.write("MISS %s %s\n" % (code, self.path)); sys.stderr.flush()
    def end_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*"); super().end_headers()
    def do_GET(self):
        u = urllib.parse.urlparse(self.path)
        if u.path.endswith("/gallery.xml"):     # the postcard viewer asks for it beside its page
            self.send_response(200); self.send_header("Content-Type", "text/xml"); self.send_header("Content-Length", str(len(GALLERY_XML))); self.end_headers(); self.wfile.write(GALLERY_XML); return
        if u.path.startswith("/gallery/") and u.path.endswith(".jpg") and u.path[9:-4].isdigit():
            b = gallery_jpeg(int(u.path[9:-4]))
            self.send_response(200); self.send_header("Content-Type", "image/jpeg"); self.send_header("Content-Length", str(len(b))); self.end_headers(); self.wfile.write(b); return
        if u.path.startswith("/title/"):
            tid = u.path[7:]
            if tid not in M: self.send_error(404); return
            b = page(tid, urllib.parse.parse_qs(u.query), self.headers.get("Host", "192.168.1.245:8097"))
            self.send_response(200); self.send_header("Content-Type", "text/html; charset=utf-8"); self.send_header("Content-Length", str(len(b))); self.end_headers(); self.wfile.write(b); return
        if u.path.startswith("/slow/"):         # /slow/BPS/STALL_AT/STALL_S/<a cache path>: a download held to BPS bytes a second
            # that stops for STALL_S seconds once STALL_AT bytes are out (R8: the throttled, then stalled, progressive download)
            try:
                _, _, bps, at, secs, rest = u.path.split("/", 5); bps, at, secs = int(bps), int(at), float(secs); rest = urllib.parse.unquote(rest)
                f = open(os.path.join(CACHE, os.path.normpath("/" + rest).lstrip("/")), "rb")
            except (ValueError, OSError): self.send_error(404); return
            import time
            with f:
                size = os.fstat(f.fileno()).st_size
                self.send_response(200); self.send_header("Content-Type", self.guess_type(rest)); self.send_header("Content-Length", str(size)); self.end_headers()
                sent = 0; stalled = False; step = max(1, bps // 10)
                try:
                    while True:
                        b = f.read(step)
                        if not b: break
                        self.wfile.write(b); self.wfile.flush(); sent += len(b)
                        if not stalled and sent >= at:
                            stalled = True; sys.stderr.write("STALL %s at %d for %g s\n" % (rest, sent, secs)); sys.stderr.flush(); time.sleep(secs)
                        else: time.sleep(0.1)
                except (BrokenPipeError, ConnectionResetError): pass
            return
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
