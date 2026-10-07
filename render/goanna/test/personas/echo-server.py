#!/usr/bin/env python3
# Tiny HTTP server for the preferences effect audit: /effects serves a page whose title reports
# what the engine let it do; /echo reports the request headers in its title. Titles reach the
# daemon log as "titleAndUrl title=[...]", so no card hook is needed.
import http.server, sys, urllib.parse
EFFECTS = r"""<!doctype html><html><head><meta charset="utf-8"><title>effects: no script</title>
<style>p{font-family:serif} .tiny{font-size:6px}</style></head>
<body><p id="p">The quick brown fox.</p><span id="t" class="tiny">tiny</span>
<img id="im" src="/dot.png" alt="">
<canvas id="c" width="8" height="8"></canvas>
<script>
function go(){
  var r=[];
  var im=document.getElementById('im'); r.push('img:'+(im.complete&&im.naturalWidth>0?'ok':'blocked'));
  var gl=null; try{gl=document.getElementById('c').getContext('webgl')||document.getElementById('c').getContext('experimental-webgl');}catch(e){} r.push('webgl:'+(gl?'yes':'no'));
  var st='no'; try{localStorage.setItem('jihad','1'); st=localStorage.getItem('jihad')==='1'?'yes':'no';}catch(e){st='no'} r.push('storage:'+st);
  r.push('fs:'+getComputedStyle(document.getElementById('p')).fontSize);
  r.push('tinyH:'+Math.round(document.getElementById('t').getBoundingClientRect().height));
  r.push('lang:'+navigator.language);
  r.push('dnt:'+navigator.doNotTrack);
  r.push('geo:'+('geolocation' in navigator && navigator.geolocation?'yes':'no'));
  var ck='no'; try{document.cookie='jihadck=1; path=/'; ck=document.cookie.indexOf('jihadck=1')>=0?'yes':'no';}catch(e){} r.push('cookie:'+ck);
  var w=null; try{w=window.open('/echo?popup','_blank');}catch(e){} r.push('popup:'+(w?'opened':'null'));
  var fl='no'; for(var i=0;i<navigator.plugins.length;i++){ if(/flash/i.test(navigator.plugins[i].name)) fl='yes'; } r.push('plugins:'+navigator.plugins.length+' flash:'+fl);
  document.title='effects: '+r.join(' ');
  setTimeout(function(){ location.href='/echo'; }, 2500);
}
window.addEventListener('load', function(){ setTimeout(go, 400); });
</script></body></html>"""
DOT = bytes.fromhex("89504e470d0a1a0a0000000d4948445200000002000000020802000000fdd49a730000001649444154789c63fccfc0c0c0c0c0c4c0c0c0c0c000000d1d01036ac29be90000000049454e44ae426082")
class H(http.server.BaseHTTPRequestHandler):
    def log_message(self, f, *a): sys.stderr.write("%s %s\n" % (self.address_string(), f % a)); sys.stderr.flush()
    def do_GET(self):
        p = urllib.parse.urlparse(self.path).path
        if p == "/water-like-header.jpg":
            data = open("water-like-header.jpg","rb").read()
            self.send_response(200); self.send_header("Content-Type", "image/jpeg"); self.send_header("Content-Length", str(len(data))); self.end_headers(); self.wfile.write(data); return
        if p == "/bomb.xpi":
            # A theme XPI whose image entry LIES: 6 MB of zeros deflated, the directory claiming
            # 1000 bytes (inspector 2026-10-06, P2). The theme code must not unpack more than it allows.
            import io, zipfile, struct as _st
            buf = io.BytesIO()
            with zipfile.ZipFile(buf, "w", zipfile.ZIP_DEFLATED) as z:
                z.writestr("manifest.json", '{"manifest_version":2,"name":"Bomb","version":"1","theme":{"images":{"theme_frame":"h.png"},"colors":{"frame":"#ff0000","tab_background_text":"#000000"}}}')
                z.writestr("h.png", b"\0" * (6 * 1024 * 1024))
            data = bytearray(buf.getvalue())
            # patch h.png's uncompressed size in its local header (offset 22) and central entry (offset 24)
            i = data.find(b"PK\x03\x04", data.find(b"h.png") - 40)
            li = data.rfind(b"PK\x03\x04", 0, data.find(b"h.png") + 1)
            _st.pack_into("<I", data, li + 22, 1000)
            ci = data.find(b"PK\x01\x02", data.rfind(b"PK\x01\x02") - 1)
            cd = data.find(b"PK\x01\x02")
            while cd >= 0:
                nlen = _st.unpack_from("<H", data, cd + 28)[0]
                if bytes(data[cd + 46: cd + 46 + nlen]) == b"h.png":
                    _st.pack_into("<I", data, cd + 24, 1000)
                cd = data.find(b"PK\x01\x02", cd + 4)
            data = bytes(data)
            self.send_response(200); self.send_header("Content-Type", "application/x-xpinstall"); self.send_header("Content-Length", str(len(data))); self.end_headers(); self.wfile.write(data); return
        if p == "/water-like.xpi":
            data = open("water_like-2.0.xpi","rb").read()
            self.send_response(200); self.send_header("Content-Type", "application/x-xpinstall"); self.send_header("Content-Length", str(len(data))); self.end_headers(); self.wfile.write(data); return
        if p == "/persona-header.png":
            data = open("persona-header.png","rb").read()
            self.send_response(200); self.send_header("Content-Type", "image/png"); self.send_header("Content-Length", str(len(data))); self.end_headers(); self.wfile.write(data); return
        if p == "/persona":
            import json as _j
            q = urllib.parse.parse_qs(urllib.parse.urlparse(self.path).query); v = (q.get("v") or ["1"])[0]
            if v == "water":
                # Water-Like by stick177 (addons.mozilla.org/firefox/addon/water-like/), as the persona it was:
                # AMO's converted static theme carries theme_frame + frame #00ccff + tab_background_text #000000.
                theme = {"id": "water-like@stick177", "name": "Water-Like", "author": "stick177",
                         "headerURL": "http://192.168.1.245:8091/water-like-header.jpg", "textcolor": "#000000", "accentcolor": "#00ccff"}
            else:
                theme = {"id": "jihad-test-persona-" + v, "name": "Jihad Test Persona " + v, "author": "the test server",
                         "headerURL": "http://192.168.1.245:8091/persona-header.png", "textcolor": "#ffffff", "accentcolor": "#1e2f5c" if v == "1" else "#5c1e2f"}
            body = ("""<!doctype html><html><head><meta charset="utf-8"><title>persona test site</title><style>body{margin:0}</style></head><body>
<!-- The buttons are big blocks at fixed places from the top, so a scripted tap lands on them at any zoom:
     install 0-150, xpi 150-300, preview 300-400, reset 400-500 (css px). -->
<a id="install" href="#" data-browsertheme='%s' style="display:block;height:150px;line-height:150px;margin:0;background:#1e2f5c;color:#fff;font-size:28px;text-align:center;text-decoration:none">Add this persona</a>
<a id="xpi" href="%s" style="display:block;height:150px;line-height:150px;margin:0;background:#06a;color:#fff;font-size:28px;text-align:center;text-decoration:none">Install the AMO static-theme XPI</a>
<a id="preview" href="#" data-browsertheme='%s' style="display:block;height:100px;line-height:100px;margin:0;background:#555;color:#fff;font-size:24px;text-align:center;text-decoration:none">Preview</a>
<a id="reset" href="#" style="display:block;height:100px;line-height:100px;margin:0;background:#333;color:#fff;font-size:24px;text-align:center;text-decoration:none">Reset preview</a>
<h1>A theme site, as Firefox 52 / Pale Moon / Basilisk expect one</h1>
<script>
function fire(node, name){ var e=document.createEvent("Events"); e.initEvent(name, true, false); node.dispatchEvent(e); }
document.getElementById("install").addEventListener("click", function(ev){ ev.preventDefault(); fire(this, "InstallBrowserTheme"); document.title="persona: install asked"; }, false);
document.getElementById("preview").addEventListener("click", function(ev){ ev.preventDefault(); fire(this, "PreviewBrowserTheme"); document.title="persona: preview asked"; }, false);
document.getElementById("reset").addEventListener("click", function(ev){ ev.preventDefault(); fire(this, "ResetBrowserThemePreview"); document.title="persona: reset asked"; }, false);
</script></body></html>""" % (_j.dumps(theme), "/bomb.xpi" if v == "bomb" else "/water-like.xpi", _j.dumps(theme))).encode()
            self.send_response(200); self.send_header("Content-Type", "text/html; charset=utf-8"); self.send_header("Content-Length", str(len(body))); self.send_header("Cache-Control", "no-store"); self.end_headers(); self.wfile.write(body); return
        if p == "/titleflood":
            # A page that rewrites its title every 20 ms for 6 s, 5000 characters with control
            # characters in it: the daemon must coalesce (<= 2 sends a second) and bound (<= 1024
            # bytes, no control characters) what it forwards (Codex 2026-10-06).
            body = b"""<!doctype html><html><head><meta charset="utf-8"><title>flood start</title></head><body><p>title flood</p>
<script>var n=0,pad=new Array(5001).join("x");var t=setInterval(function(){n++;document.title="flood "+n+"\\u0001\\u0007|"+pad;if(n>=300){clearInterval(t);document.title="flood done "+n;}},20);</script></body></html>"""
            self.send_response(200); self.send_header("Content-Type", "text/html; charset=utf-8"); self.send_header("Content-Length", str(len(body))); self.send_header("Cache-Control", "no-store"); self.end_headers(); self.wfile.write(body); return
        if p == "/ref":
            # Referrer cases (Codex 2026-10-06): one link per page, a 300px block at the top, so a
            # scripted tap at any zoom lands on it; "js" variants navigate by script instead
            # (the engine starts that load itself: the daemon's content-nav re-drive).
            v = (urllib.parse.parse_qs(urllib.parse.urlparse(self.path).query).get("v") or ["plain"])[0]
            # "file" / "jsdata" (inspector 2026-10-06, P1): a page must not reach file: or a
            # top-level data: document through the daemon's re-drive by withholding its referrer.
            meta = '<meta name="referrer" content="no-referrer">' if v in ("meta", "jsmeta", "jsdata") else ""
            attrs = {"noref": ' rel="noreferrer"', "origin": ' referrerpolicy="origin"', "file": ' rel="noreferrer"'}.get(v, "")
            href = "file:///tmp/jihad-plain.html" if v == "file" else "/echo?%s" % v
            target = "data:text/html,<title>DATA-REACHED</title>pwned" if v == "jsdata" else "/echo?%s" % v
            js = "<script>window.addEventListener('load',function(){setTimeout(function(){location.href='%s';},800);});</script>" % target if v.startswith("js") else ""
            body = ("""<!doctype html><html><head><meta charset="utf-8">%s<title>ref test %s</title><style>body{margin:0}</style></head><body>
<a id="go" href="%s"%s style="display:block;height:300px;line-height:300px;margin:0;background:#06a;color:#fff;font-size:28px;text-align:center;text-decoration:none">Follow (%s)</a>
%s</body></html>""" % (meta, v, href, attrs, v, js)).encode()
            self.send_response(200); self.send_header("Content-Type", "text/html; charset=utf-8"); self.send_header("Content-Length", str(len(body))); self.send_header("Cache-Control", "no-store"); self.end_headers(); self.wfile.write(body); return
        if p == "/dot.png":
            self.send_response(200); self.send_header("Content-Type", "image/png"); self.send_header("Content-Length", str(len(DOT))); self.end_headers(); self.wfile.write(DOT); return
        if p == "/echo":
            h = self.headers
            t = "echo: Q=[%s] AL=[%s] DNT=[%s] REF=[%s] UA=[%s]" % (urllib.parse.urlparse(self.path).query, h.get("Accept-Language"), h.get("DNT"), h.get("Referer"), (h.get("User-Agent") or "")[:60])
            body = ("<!doctype html><html><head><meta charset='utf-8'><title>%s</title></head><body><p>%s</p></body></html>" % (t.replace("<","&lt;"), t.replace("<","&lt;"))).encode()
        else:
            body = EFFECTS.encode()
        self.send_response(200); self.send_header("Content-Type", "text/html; charset=utf-8"); self.send_header("Content-Length", str(len(body))); self.send_header("Cache-Control", "no-store"); self.end_headers(); self.wfile.write(body)
http.server.HTTPServer(("0.0.0.0", int(sys.argv[1]) if len(sys.argv) > 1 else 8091), H).serve_forever()
