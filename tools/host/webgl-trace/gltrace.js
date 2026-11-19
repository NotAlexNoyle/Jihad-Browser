// Copyright 2026 NotAlexNoyle. Apache-2.0.
// gltrace.js (T-672): every WebGLRenderingContext call (and the methods of the extensions getExtension hands out), the page's
// console and its errors, sent as beacons to beacon-log.py on the host (:8098) before each call runs, so a call that never
// returns (a GPU hang) is the last one in the log. Loaded by a probe as a script tag from that logger:
//   probe gltrace '(function(){var s=document.createElement("script");s.src="http://192.168.1.245:8098/gltrace.js";(document.head||document.documentElement).appendChild(s);return "gl trace loading";})()'
// (a probe's text rides the inject channel, which cuts a long one: this file is too long to be a probe itself).
(function(){
var P=WebGLRenderingContext.prototype; if(P.__jgt) return "gl trace already";
P.__jgt=1; var seq=0, buf=[], last="", rep=0;
var BIG={texImage2D:1,texSubImage2D:1,generateMipmap:1,bufferData:1,bufferSubData:1,readPixels:1,copyTexImage2D:1,copyTexSubImage2D:1,renderbufferStorage:1,framebufferTexture2D:1,framebufferRenderbuffer:1,checkFramebufferStatus:1,compileShader:1,linkProgram:1,viewport:1,clear:1,finish:1,flush:1,deleteTexture:1,createTexture:1,drawElements:1,drawArrays:1,useProgram:1,bindFramebuffer:1,getExtension:1,bindVertexArrayOES:1,createVertexArrayOES:1,deleteVertexArrayOES:1};
function arg(a){ try{ if(a===null||a===undefined) return String(a); var t=typeof a; if(t==="number"||t==="boolean") return String(a); if(t==="string") return "s:"+a.slice(0,40);
  if(a.BYTES_PER_ELEMENT) return "TA["+a.length+"]"; if(t==="object"&&"byteLength" in a) return "AB["+a.byteLength+"]";
  if(t==="object"&&"width" in a&&"height" in a) return "img("+a.width+"x"+a.height+")";
  return (a.constructor&&a.constructor.name)||t; }catch(e){ return "?"; } }
function flush(){ if(!buf.length) return; try{ navigator.sendBeacon("http://192.168.1.245:8098/gl", buf.join("\n")); }catch(e){} buf=[]; }
function note(n,args){ try{ seq++; var s=n+"("+Array.prototype.map.call(args,arg).join(",")+")";
    if(s===last){ rep++; } else { if(rep>1) buf.push(last+" x"+rep); else if(last) buf.push(last); last=s; rep=1; }
    if(BIG[n]||buf.length>=60){ if(rep>1) buf.push(last+" x"+rep); else buf.push(last); last=""; rep=0; buf.unshift("#"+seq+" "+Math.round(performance.now()/1000)+"s"); flush(); } }catch(e){} }
function wrap(obj,n){ var d=Object.getOwnPropertyDescriptor(obj,n); if(!d||typeof d.value!=="function"||n==="constructor") return; var f=d.value;
  obj[n]=function(){ note(n,arguments); var r=f.apply(this,arguments);
    if(n==="getExtension"&&r&&!r.__jgt){ try{ r.__jgt=1; var pr=Object.getPrototypeOf(r); if(pr&&!pr.__jgt){ pr.__jgt=1; Object.getOwnPropertyNames(pr).forEach(function(m){ wrap(pr,m); }); } }catch(e){} }
    return r; }; }
Object.getOwnPropertyNames(P).forEach(function(n){ wrap(P,n); });
["log","info","warn","error"].forEach(function(k){ var o=console[k]; console[k]=function(){ try{ var t=Array.prototype.map.call(arguments,function(x){ return x&&x.stack?String(x)+" @"+String(x.stack).slice(0,300):String(x); }).join(" ").slice(0,500); navigator.sendBeacon("http://192.168.1.245:8098/gl","CONSOLE "+Math.round(performance.now()/1000)+"s "+k+": "+t); }catch(e){} try{ return o.apply(console,arguments); }catch(e){} }; });
window.addEventListener("error",function(e){ try{ navigator.sendBeacon("http://192.168.1.245:8098/gl","ONERROR "+e.message+" "+e.filename+":"+e.lineno); }catch(x){} },true);
try{navigator.sendBeacon("http://192.168.1.245:8098/gl","gl trace installed");}catch(e){}
return "gl trace on";
})()
