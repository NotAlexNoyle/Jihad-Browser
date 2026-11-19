// Copyright 2026 NotAlexNoyle. Apache-2.0.
// console.js (T-672): only the page's console and its errors, sent as beacons to beacon-log.py (:8098), for a run that
// must not carry gltrace.js's cost. Loaded by a probe as a script tag from that logger (http://192.168.1.245:8098/console.js).
(function(){
if (window.__jcb) return; window.__jcb = 1;
function send(s){ try{ navigator.sendBeacon("http://192.168.1.245:8098/log", s); }catch(e){} }
["log","info","warn","error"].forEach(function(k){ var o=console[k]; console[k]=function(){ try{ var t=Array.prototype.map.call(arguments,function(x){ return x&&x.stack?String(x)+" @"+String(x.stack).slice(0,300):String(x); }).join(" ").slice(0,500); send("CONSOLE "+Math.round(performance.now()/1000)+"s "+k+": "+t); }catch(e){} try{ return o.apply(console,arguments); }catch(e){} }; });
window.addEventListener("error",function(e){ send("ONERROR "+Math.round(performance.now()/1000)+"s "+e.message+" "+e.filename+":"+e.lineno); },true);
send("console beacons installed");
})();
