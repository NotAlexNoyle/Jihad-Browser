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
            body = ("""<!doctype html><html><head><meta charset="utf-8"><title>persona test site</title></head><body>
<h1>A theme site, as Firefox 52 / Pale Moon / Basilisk expect one</h1>
<p><a id="install" href="#" data-browsertheme='%s' style="display:inline-block;padding:20px 30px;background:#1e2f5c;color:#fff;font-size:20px">Add this persona</a></p>
<p><a id="xpi" href="/water-like.xpi" style="display:inline-block;padding:20px 30px;background:#06a;color:#fff;font-size:20px">Install the AMO static-theme XPI</a></p>
<p><a id="preview" href="#" data-browsertheme='%s' style="display:inline-block;padding:20px 30px;background:#555;color:#fff;font-size:20px">Preview</a>
<a id="reset" href="#" style="display:inline-block;padding:20px 30px;background:#555;color:#fff;font-size:20px">Reset preview</a></p>
<script>
function fire(node, name){ var e=document.createEvent("Events"); e.initEvent(name, true, false); node.dispatchEvent(e); }
document.getElementById("install").addEventListener("click", function(ev){ ev.preventDefault(); fire(this, "InstallBrowserTheme"); document.title="persona: install asked"; }, false);
document.getElementById("preview").addEventListener("click", function(ev){ ev.preventDefault(); fire(this, "PreviewBrowserTheme"); document.title="persona: preview asked"; }, false);
document.getElementById("reset").addEventListener("click", function(ev){ ev.preventDefault(); fire(this, "ResetBrowserThemePreview"); document.title="persona: reset asked"; }, false);
</script></body></html>""" % (_j.dumps(theme), _j.dumps(theme))).encode()
            self.send_response(200); self.send_header("Content-Type", "text/html; charset=utf-8"); self.send_header("Content-Length", str(len(body))); self.send_header("Cache-Control", "no-store"); self.end_headers(); self.wfile.write(body); return
        if p == "/dot.png":
            self.send_response(200); self.send_header("Content-Type", "image/png"); self.send_header("Content-Length", str(len(DOT))); self.end_headers(); self.wfile.write(DOT); return
        if p == "/echo":
            h = self.headers
            t = "echo: AL=[%s] DNT=[%s] REF=[%s] UA=[%s]" % (h.get("Accept-Language"), h.get("DNT"), h.get("Referer"), (h.get("User-Agent") or "")[:60])
            body = ("<!doctype html><html><head><meta charset='utf-8'><title>%s</title></head><body><p>%s</p></body></html>" % (t.replace("<","&lt;"), t.replace("<","&lt;"))).encode()
        else:
            body = EFFECTS.encode()
        self.send_response(200); self.send_header("Content-Type", "text/html; charset=utf-8"); self.send_header("Content-Length", str(len(body))); self.send_header("Cache-Control", "no-store"); self.end_headers(); self.wfile.write(body)
http.server.HTTPServer(("0.0.0.0", int(sys.argv[1]) if len(sys.argv) > 1 else 8091), H).serve_forever()
