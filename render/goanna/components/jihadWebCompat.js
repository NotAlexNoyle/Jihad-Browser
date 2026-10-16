/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — web compatibility shims (cavekit-modern-web.md R1/R6): the few web platform
 * pieces this engine lacks that modern pages assume, supplied from JavaScript before a page's own
 * scripts run, each only where the engine has no native one, each small enough to read.
 *
 * WHY: a page tests for a feature and then uses another it assumes comes with it — Etsy's
 * components (Lit) test `replace` on CSSStyleSheet and, finding it, hand adoptedStyleSheets the
 * sheets `replace` would have built; this engine has the constructor, replaceSync and
 * adoptedStyleSheets but not replace(), so Lit's test fails and Etsy's own code then assigns
 * undefined sheets: 47 TypeErrors per search page (2026-10-16, Mochi, daemon 12). DuckDuckGo's
 * results application reaches for Intl.ListFormat, reportError, the popover attribute and inert.
 *
 * HOW: at document-element-inserted (the root element exists, no script has run) a sandbox with the
 * page's principal and the page's window as its prototype evaluates the shims, which define the
 * missing members on the PAGE's own prototypes (CSSStyleSheet, HTMLElement, Intl …). The sandbox
 * has no chrome powers; the shims are plain page-level JavaScript. The two UA-level rules the
 * popover and inert attributes need (hidden until shown; no pointer events) are one agent sheet,
 * registered once. jihad.compat.enabled turns the shims off; jihad.compat.trace (off by default,
 * a diagnosis tool) also wraps the engine calls that can throw an XPCOM result to a page and logs
 * which one threw, with the page's own stack — the console bridge sees such a throw only as
 * "NS_ERROR_NOT_INITIALIZED: " at column 0.
 */

"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/Timer.jsm");

const PREF_ENABLED = "jihad.compat.enabled";
const PREF_TRACE   = "jihad.compat.trace";
const PREF_PROBE   = "jihad.compat.probe";   // trace mode only: base64 of a script run in each top document 25 s after its root, its result logged

function log(s) { try { Services.console.logStringMessage("compat: " + s); } catch (e) {} }
function boolPref(n, d) { try { return Services.prefs.getBoolPref(n); } catch (e) { return d; } }
function charPref(n, d) { try { return Services.prefs.getCharPref(n); } catch (e) { return d; } }

// The agent sheet: what the UA stylesheet of a browser with popover and inert carries, for the
// shims below (the engine has neither attribute). Agent level, as in those browsers, so a page's
// own rules win over it exactly as they do there.
const AGENT_CSS =
  "[popover]{display:none;position:fixed;inset:0;width:-moz-fit-content;height:-moz-fit-content;margin:auto;" +
  "border:1px solid;padding:0.25em;overflow:auto;color:-moz-FieldText;background:-moz-Field;z-index:2147483646}" +
  "[popover].jihad-popover-open{display:block}" +
  "[inert],[inert] *{pointer-events:none!important;cursor:default!important;-moz-user-select:none!important}";

// The shims: page-level JavaScript, run in the page's sandbox. `window` is the page's window (the
// sandbox's prototype); every member is defined on the page's own objects. Returns the names
// installed, for the log.
const SHIMS = String.raw`
(function (W) {
  "use strict";   // a page's bare call (matchMedia()) must reach a shim with this === undefined, not the sandbox global
  var installed = [];
  function lacks(o, k) { return !!o && !(k in o); }

  // CSSStyleSheet.prototype.replace — the engine has replaceSync and adoptedStyleSheets, not the
  // promise form; Lit (Etsy) tests for replace and assumes the rest. Same semantics: replaceSync,
  // then a promise of the sheet.
  if (W.CSSStyleSheet && "replaceSync" in W.CSSStyleSheet.prototype && lacks(W.CSSStyleSheet.prototype, "replace")) {
    W.CSSStyleSheet.prototype.replace = function (text) {
      var sheet = this;
      return new W.Promise(function (resolve, reject) { try { sheet.replaceSync(text); resolve(sheet); } catch (e) { reject(e); } });
    };
    installed.push("CSSStyleSheet.replace");
  }

  // window.reportError — reports as an uncaught exception would: an error event, then the console.
  if (lacks(W, "reportError")) {
    W.reportError = function (e) {
      try { W.dispatchEvent(new W.ErrorEvent("error", {error: e, message: String(e && e.message || e)})); } catch (x) {}
      try { W.console.error(e); } catch (x) {}
    };
    installed.push("reportError");
  }

  // Intl.ListFormat — the English forms only (the engine's Intl has no list patterns); other
  // locales get the English words between the items, which reads, and is better than a throw.
  if (W.Intl && lacks(W.Intl, "ListFormat")) {
    function ListFormat(locales, options) {
      options = options || {};
      this._type = options.type === "disjunction" || options.type === "unit" ? options.type : "conjunction";
      this._style = options.style === "short" || options.style === "narrow" ? options.style : "long";
      this._locale = (Array.isArray(locales) ? locales[0] : locales) || "en";
    }
    ListFormat.prototype.format = function (list) {
      var a = W.Array.prototype.map.call(list, String), n = a.length;
      if (n === 0) return "";
      if (n === 1) return a[0];
      if (this._type === "unit") return a.join(this._style === "narrow" ? " " : ", ");
      var word = this._type === "disjunction" ? "or" : (this._style === "long" ? "and" : "&");
      if (n === 2) return a[0] + " " + word + " " + a[1];
      return a.slice(0, -1).join(", ") + ", " + word + " " + a[n - 1];
    };
    ListFormat.prototype.formatToParts = function (list) { return [{type: "literal", value: this.format(list)}]; };
    ListFormat.prototype.resolvedOptions = function () { return {locale: this._locale, type: this._type, style: this._style}; };
    ListFormat.supportedLocalesOf = function (l) { return W.Array.isArray(l) ? l.slice() : (l ? [l] : []); };
    W.Intl.ListFormat = ListFormat;
    installed.push("Intl.ListFormat");
  }

  // Element.prototype.checkVisibility — connected, rendered, and (on request) visible and opaque.
  if (W.Element && lacks(W.Element.prototype, "checkVisibility")) {
    W.Element.prototype.checkVisibility = function (options) {
      if (!this.isConnected) return false;
      var cs = W.getComputedStyle(this);
      if (!cs || cs.display === "none") return false;
      if (options && (options.visibilityProperty || options.checkVisibilityCSS) && cs.visibility !== "visible") return false;
      if (options && (options.opacityProperty || options.checkOpacity) && cs.opacity === "0") return false;
      return this.getClientRects().length > 0;
    };
    installed.push("Element.checkVisibility");
  }

  // AbortSignal.any — a signal that aborts with the first of the given.
  if (W.AbortSignal && W.AbortController && lacks(W.AbortSignal, "any")) {
    W.AbortSignal.any = function (signals) {
      var c = new W.AbortController();
      W.Array.prototype.forEach.call(signals, function (s) {
        if (s.aborted) { if (!c.signal.aborted) c.abort(s.reason); return; }
        s.addEventListener("abort", function () { if (!c.signal.aborted) c.abort(s.reason); }, {once: true});
      });
      return c.signal;
    };
    installed.push("AbortSignal.any");
  }

  // URLSearchParams.prototype.size
  if (W.URLSearchParams && lacks(W.URLSearchParams.prototype, "size")) {
    W.Object.defineProperty(W.URLSearchParams.prototype, "size", {configurable: true, get: function () { var n = 0; this.forEach(function () { n++; }); return n; }});
    installed.push("URLSearchParams.size");
  }

  // Array.fromAsync
  if (W.Array && lacks(W.Array, "fromAsync")) {
    W.Array.fromAsync = function (source, mapFn, thisArg) {
      return (async function () {
        var out = new W.Array(), i = 0;
        if (source != null && (source[W.Symbol.asyncIterator] || source[W.Symbol.iterator])) {
          for await (var v of source) { out.push(mapFn ? await mapFn.call(thisArg, v, i) : v); i++; }
        } else {
          var len = W.Number(source.length) >>> 0;
          for (; i < len; i++) { var x = await source[i]; out.push(mapFn ? await mapFn.call(thisArg, x, i) : x); }
        }
        return out;
      })();
    };
    installed.push("Array.fromAsync");
  }

  // The inert attribute as a property (the agent sheet takes the pointer events away; focus is
  // not refused — a smaller inert than the real one, and the dialogs that set it still work).
  if (W.HTMLElement && lacks(W.HTMLElement.prototype, "inert")) {
    W.Object.defineProperty(W.HTMLElement.prototype, "inert", {configurable: true,
      get: function () { return this.hasAttribute("inert"); },
      set: function (v) { if (v) this.setAttribute("inert", ""); else this.removeAttribute("inert"); }});
    installed.push("inert");
  }

  // The popover attribute and its methods: hidden until shown (the agent sheet), one "auto"
  // popover open at a time, light dismiss by a click outside or Escape, popovertarget buttons,
  // beforetoggle (cancelable) and toggle events with oldState/newState. No top layer: the sheet's
  // z-index stands in for it.
  if (W.HTMLElement && lacks(W.HTMLElement.prototype, "showPopover")) {
    var OPEN = "jihad-popover-open";
    function kind(el) { var v = el.getAttribute("popover"); return v === null ? null : (v === "" || v.toLowerCase() === "auto" ? "auto" : "manual"); }
    function isOpen(el) { return el.classList.contains(OPEN); }
    function toggleEvent(el, name, from, to, cancelable) {
      var ev = new W.CustomEvent(name, {bubbles: false, cancelable: !!cancelable});
      try { W.Object.defineProperty(ev, "oldState", {value: from}); W.Object.defineProperty(ev, "newState", {value: to}); } catch (e) {}
      return el.dispatchEvent(ev);
    }
    function hide(el) {
      if (!isOpen(el)) return;
      if (!toggleEvent(el, "beforetoggle", "open", "closed", true)) return;
      el.classList.remove(OPEN);
      toggleEvent(el, "toggle", "open", "closed", false);
    }
    function hideOthers(except) {
      var open = W.document.querySelectorAll("[popover]." + OPEN);
      for (var i = 0; i < open.length; i++) if (open[i] !== except && kind(open[i]) === "auto") hide(open[i]);
    }
    function show(el) {
      if (!el.isConnected || kind(el) === null || isOpen(el)) return;
      if (!toggleEvent(el, "beforetoggle", "closed", "open", true)) return;
      if (kind(el) === "auto") hideOthers(el);
      el.classList.add(OPEN);
      toggleEvent(el, "toggle", "closed", "open", false);
    }
    W.HTMLElement.prototype.showPopover = function () { show(this); };
    W.HTMLElement.prototype.hidePopover = function () { hide(this); };
    W.HTMLElement.prototype.togglePopover = function (force) {
      var want = force === undefined ? !isOpen(this) : !!force;
      if (want) show(this); else hide(this);
      return isOpen(this);
    };
    W.Object.defineProperty(W.HTMLElement.prototype, "popover", {configurable: true,
      get: function () { return kind(this); },
      set: function (v) { if (v === null || v === undefined) this.removeAttribute("popover"); else this.setAttribute("popover", String(v)); }});
    W.document.addEventListener("click", function (e) {
      var t = e.target, button = t && t.closest ? t.closest("[popovertarget]") : null;
      if (button) {
        var target = W.document.getElementById(button.getAttribute("popovertarget") || "");
        if (target && kind(target) !== null) {
          var action = (button.getAttribute("popovertargetaction") || "toggle").toLowerCase();
          if (action === "show") show(target); else if (action === "hide") hide(target); else if (isOpen(target)) hide(target); else show(target);
          return;
        }
      }
      hideOthers(t && t.closest ? t.closest("[popover]." + OPEN) : null);
    }, true);
    W.document.addEventListener("keydown", function (e) { if (e.key === "Escape") hideOthers(null); }, true);
    installed.push("popover");
  }

  // ── the second batch (the API probe's list, 2026-10-16): small members modern bundles use ──
  // Blob.prototype.text / arrayBuffer, through FileReader
  if (W.Blob && lacks(W.Blob.prototype, "text")) {
    function readAs(blob, method) { return new W.Promise(function (resolve, reject) { var r = new W.FileReader(); r.onload = function () { resolve(r.result); }; r.onerror = function () { reject(r.error); }; r[method](blob); }); }
    W.Blob.prototype.text = function () { return readAs(this, "readAsText"); };
    W.Blob.prototype.arrayBuffer = function () { return readAs(this, "readAsArrayBuffer"); };
    installed.push("Blob.text/arrayBuffer");
  }
  // Response.json (static)
  if (W.Response && lacks(W.Response, "json")) {
    W.Response.json = function (data, init) { init = init || {}; var h = new W.Headers(init.headers || {}); if (!h.has("content-type")) h.set("content-type", "application/json"); return new W.Response(W.JSON.stringify(data), {status: init.status, statusText: init.statusText, headers: h}); };
    installed.push("Response.json");
  }
  // Promise.try
  if (W.Promise && lacks(W.Promise, "try")) {
    W.Promise.try = function (fn) { var args = W.Array.prototype.slice.call(arguments, 1); return new W.Promise(function (resolve, reject) { try { resolve(fn.apply(undefined, args)); } catch (e) { reject(e); } }); };
    installed.push("Promise.try");
  }
  // the Set methods of ES2025
  if (W.Set && lacks(W.Set.prototype, "union")) {
    var SP = W.Set.prototype;
    function toSet(o) { return o instanceof W.Set ? o : new W.Set(typeof o.keys === "function" ? o.keys() : o); }
    SP.union = function (o) { var r = new W.Set(this); toSet(o).forEach(function (v) { r.add(v); }); return r; };
    SP.intersection = function (o) { var s = toSet(o), r = new W.Set(); this.forEach(function (v) { if (s.has(v)) r.add(v); }); return r; };
    SP.difference = function (o) { var s = toSet(o), r = new W.Set(); this.forEach(function (v) { if (!s.has(v)) r.add(v); }); return r; };
    SP.symmetricDifference = function (o) { var s = toSet(o), r = new W.Set(); this.forEach(function (v) { if (!s.has(v)) r.add(v); }); s.forEach(function (v) { if (!this.has(v)) r.add(v); }, this); return r; };
    SP.isSubsetOf = function (o) { var s = toSet(o), ok = true; this.forEach(function (v) { if (!s.has(v)) ok = false; }); return ok; };
    SP.isSupersetOf = function (o) { var self = this, ok = true; toSet(o).forEach(function (v) { if (!self.has(v)) ok = false; }); return ok; };
    SP.isDisjointFrom = function (o) { var s = toSet(o), ok = true; this.forEach(function (v) { if (s.has(v)) ok = false; }); return ok; };
    installed.push("Set methods");
  }
  // the iterator helpers (map, filter, take, drop, flatMap, forEach, some, every, find, reduce,
  // toArray) on the shared %IteratorPrototype% every built-in iterator inherits
  try {
    // the PAGE's %IteratorPrototype% (an array made in the page's realm — a literal here would be
    // the sandbox's own, and the helpers went there in the third trace round)
    var IP = W.Object.getPrototypeOf(W.Object.getPrototypeOf(W.Array.prototype[W.Symbol.iterator].call(new W.Array())));
    // (not gated on a global Iterator: this engine still has old SpiderMonkey's legacy one,
    // unrelated to %IteratorPrototype% — such a guard skipped the helpers on the device)
    if (IP && lacks(IP, "map")) {
      function wrapIt(next, ret) { var it = W.Object.create(IP); it.next = next; it.return = function () { if (ret) ret(); return {value: undefined, done: true}; }; return it; }
      IP.map = function (f) { var src = this, i = 0; return wrapIt(function () { var r = src.next(); return r.done ? r : {value: f(r.value, i++), done: false}; }, function () { if (src.return) src.return(); }); };
      IP.filter = function (f) { var src = this, i = 0; return wrapIt(function () { for (;;) { var r = src.next(); if (r.done || f(r.value, i++)) return r; } }, function () { if (src.return) src.return(); }); };
      IP.take = function (n) { var src = this, left = n; return wrapIt(function () { if (left-- <= 0) { if (src.return) src.return(); return {value: undefined, done: true}; } return src.next(); }, function () { if (src.return) src.return(); }); };
      IP.drop = function (n) { var src = this, left = n; return wrapIt(function () { while (left > 0) { left--; var r = src.next(); if (r.done) return r; } return src.next(); }, function () { if (src.return) src.return(); }); };
      IP.flatMap = function (f) { var src = this, inner = null, i = 0; return wrapIt(function () { for (;;) { if (inner) { var r = inner.next(); if (!r.done) return r; inner = null; } var o = src.next(); if (o.done) return o; inner = f(o.value, i++)[W.Symbol.iterator](); } }, function () { if (src.return) src.return(); }); };
      IP.forEach = function (f) { var i = 0; for (;;) { var r = this.next(); if (r.done) return; f(r.value, i++); } };
      IP.some = function (f) { var i = 0; for (;;) { var r = this.next(); if (r.done) return false; if (f(r.value, i++)) { if (this.return) this.return(); return true; } } };
      IP.every = function (f) { var i = 0; for (;;) { var r = this.next(); if (r.done) return true; if (!f(r.value, i++)) { if (this.return) this.return(); return false; } } };
      IP.find = function (f) { var i = 0; for (;;) { var r = this.next(); if (r.done) return undefined; if (f(r.value, i++)) { if (this.return) this.return(); return r.value; } } };
      IP.reduce = function (f, init) { var acc = init, i = 0, has = arguments.length > 1; for (;;) { var r = this.next(); if (r.done) { if (!has) throw new W.TypeError("reduce of an empty iterator with no initial value"); return acc; } if (!has) { acc = r.value; has = true; i++; } else acc = f(acc, r.value, i++); } };
      IP.toArray = function () { var a = []; for (;;) { var r = this.next(); if (r.done) return a; a.push(r.value); } };
      installed.push("iterator helpers");
    }
  } catch (e) {}
  // async iteration of a ReadableStream (for await … of response.body)
  if (W.ReadableStream && W.Symbol.asyncIterator && lacks(W.ReadableStream.prototype, W.Symbol.asyncIterator)) {
    W.ReadableStream.prototype[W.Symbol.asyncIterator] = function () { var reader = this.getReader(); return {next: function () { return reader.read(); }, return: function () { reader.releaseLock(); return W.Promise.resolve({value: undefined, done: true}); }}; };
    W.ReadableStream.prototype.values = W.ReadableStream.prototype[W.Symbol.asyncIterator];
    installed.push("ReadableStream iteration");
  }
  // crypto.randomUUID (version 4, from getRandomValues)
  if (W.crypto && typeof W.crypto.getRandomValues === "function" && lacks(W.crypto, "randomUUID")) {
    W.crypto.randomUUID = function () { var b = W.crypto.getRandomValues(new W.Uint8Array(16)); b[6] = (b[6] & 0x0f) | 0x40; b[8] = (b[8] & 0x3f) | 0x80; var h = W.Array.prototype.map.call(b, function (x) { return (x + 256).toString(16).slice(1); }).join(""); return h.slice(0, 8) + "-" + h.slice(8, 12) + "-" + h.slice(12, 16) + "-" + h.slice(16, 20) + "-" + h.slice(20); };
    installed.push("crypto.randomUUID");
  }
  // scheduler.postTask / yield, on timers (no priorities)
  if (lacks(W, "scheduler")) {
    W.scheduler = {postTask: function (cb, opts) { var delay = (opts && opts.delay) || 0; return new W.Promise(function (resolve, reject) { W.setTimeout(function () { if (opts && opts.signal && opts.signal.aborted) { reject(opts.signal.reason); return; } try { resolve(cb()); } catch (e) { reject(e); } }, delay); }); }, yield: function () { return new W.Promise(function (resolve) { W.setTimeout(resolve, 0); }); }};
    installed.push("scheduler.postTask");
  }
  // HTMLVideoElement.requestVideoFrameCallback, on animation frames
  if (W.HTMLVideoElement && lacks(W.HTMLVideoElement.prototype, "requestVideoFrameCallback")) {
    W.HTMLVideoElement.prototype.requestVideoFrameCallback = function (cb) { var v = this; return W.requestAnimationFrame(function (now) { cb(now, {presentationTime: now, expectedDisplayTime: now, width: v.videoWidth, height: v.videoHeight, mediaTime: v.currentTime, presentedFrames: 0, processingDuration: 0}); }); };
    W.HTMLVideoElement.prototype.cancelVideoFrameCallback = function (h) { W.cancelAnimationFrame(h); };
    installed.push("requestVideoFrameCallback");
  }
  // CanvasRenderingContext2D.roundRect
  if (W.CanvasRenderingContext2D && lacks(W.CanvasRenderingContext2D.prototype, "roundRect")) {
    W.CanvasRenderingContext2D.prototype.roundRect = function (x, y, w, h, radii) {
      var r = radii === undefined ? [0] : (typeof radii === "number" ? [radii] : W.Array.prototype.slice.call(radii));
      var tl = r[0], tr = r.length > 1 ? r[1] : r[0], br = r.length > 2 ? r[2] : r[0], bl = r.length > 3 ? r[3] : (r.length > 1 ? r[1] : r[0]);
      this.moveTo(x + tl, y); this.lineTo(x + w - tr, y); this.arcTo(x + w, y, x + w, y + tr, tr); this.lineTo(x + w, y + h - br); this.arcTo(x + w, y + h, x + w - br, y + h, br);
      this.lineTo(x + bl, y + h); this.arcTo(x, y + h, x, y + h - bl, bl); this.lineTo(x, y + tl); this.arcTo(x, y, x + tl, y, tl); this.closePath();
    };
    installed.push("canvas.roundRect");
  }

  return installed;
})
`;

// Trace mode: the engine calls that can throw an XPCOM result to a page, wrapped so the throw is
// logged with its name and the page's own stack, then thrown on unchanged. __jihadTrace is the
// component's logger, exported into the sandbox only (the page never sees it).
const TRACE = String.raw`
(function (W) {
  "use strict";   // same reason: third trace round, 2026-10-16 — without it this was the sandbox global and the binding refused it
  function report(name, e) {
    var code = (e && typeof e.result === "number") ? " (0x" + (e.result >>> 0).toString(16) + ")" : "";
    var stack = ""; try { stack = new W.Error("trace").stack || ""; } catch (x) {}
    try { __jihadTrace(name + " threw " + String(e) + code + "\n" + stack); } catch (x) {}
  }
  // A function the page calls without a receiver (matchMedia(), structuredClone()) arrives with
  // this === undefined; the engine's method then wants its object back (first trace round,
  // 2026-10-16: the wrapper itself threw "called on an object that does not implement Window").
  function wrapMethod(obj, name, label) {
    if (!obj || typeof obj[name] !== "function") return;
    var orig = obj[name];
    obj[name] = function () { var self = (this === undefined || this === null) ? obj : this; try { return orig.apply(self, arguments); } catch (e) { report(label || name, e); throw e; } };
  }
  function wrapCtor(name) {
    var orig = W[name]; if (typeof orig !== "function") return;
    var wrapped = function () { try { return W.Reflect.construct(orig, arguments, new.target || orig); } catch (e) { report("new " + name, e); throw e; } };
    wrapped.prototype = orig.prototype;
    try { W.Object.getOwnPropertyNames(orig).forEach(function (k) { if (!(k in wrapped)) try { wrapped[k] = orig[k]; } catch (x) {} }); } catch (x) {}
    W[name] = wrapped;
  }
  function wrapGetter(obj, name, label) {
    var d = null, o = obj;
    while (o && !d) { d = W.Object.getOwnPropertyDescriptor(o, name); if (!d) o = W.Object.getPrototypeOf(o); }
    if (!d || !d.get) return;
    var get = d.get;
    W.Object.defineProperty(obj, name, {configurable: true, enumerable: d.enumerable, get: function () { try { return get.call(this); } catch (e) { report(label || name, e); throw e; } }, set: d.set});
  }
  // A promise-returning call: a rejection with an XPCOM result is logged too (the page's chain is
  // untouched — the original promise is returned; its own rejection handler, if any, still runs).
  function wrapAsync(obj, name, label) {
    if (!obj || typeof obj[name] !== "function") return;
    var orig = obj[name];
    obj[name] = function () {
      var self = (this === undefined || this === null) ? obj : this, stack = "";
      try { stack = new W.Error("trace").stack || ""; } catch (x) {}
      var r;
      try { r = orig.apply(self, arguments); } catch (e) { report(label || name, e); throw e; }
      if (r && typeof r.then === "function") {
        try { r.then(null, function (e) { if (e && (typeof e.result === "number" || /NS_ERROR/.test(String(e)))) { try { __jihadTrace((label || name) + " REJECTED with " + String(e) + "\n" + stack); } catch (x) {} } }); } catch (x) {}
      }
      return r;
    };
  }
  var X = W.XMLHttpRequest && W.XMLHttpRequest.prototype;
  wrapMethod(X, "open", "XMLHttpRequest.open"); wrapMethod(X, "send", "XMLHttpRequest.send"); wrapMethod(X, "setRequestHeader", "XMLHttpRequest.setRequestHeader");
  wrapMethod(X, "getAllResponseHeaders", "XMLHttpRequest.getAllResponseHeaders"); wrapMethod(X, "overrideMimeType", "XMLHttpRequest.overrideMimeType");
  wrapGetter(X, "responseText", "XMLHttpRequest.responseText"); wrapGetter(X, "response", "XMLHttpRequest.response");
  wrapCtor("WebSocket"); wrapCtor("BroadcastChannel"); wrapCtor("Notification"); wrapCtor("Worker"); wrapCtor("SharedWorker");
  wrapCtor("EventSource"); wrapCtor("MediaSource"); wrapCtor("AudioContext"); wrapCtor("PerformanceObserver"); wrapCtor("FileReader");
  wrapMethod(W.navigator, "sendBeacon", "navigator.sendBeacon");
  wrapGetter(W, "localStorage", "window.localStorage"); wrapGetter(W, "sessionStorage", "window.sessionStorage"); wrapGetter(W, "indexedDB", "window.indexedDB");
  if (W.Storage) { wrapMethod(W.Storage.prototype, "getItem", "Storage.getItem"); wrapMethod(W.Storage.prototype, "setItem", "Storage.setItem"); wrapMethod(W.Storage.prototype, "removeItem", "Storage.removeItem"); }
  if (W.IDBFactory) { wrapMethod(W.IDBFactory.prototype, "open", "indexedDB.open"); wrapMethod(W.IDBFactory.prototype, "deleteDatabase", "indexedDB.deleteDatabase"); }
  if (W.crypto && W.crypto.subtle) { ["digest", "importKey", "sign", "verify", "encrypt", "decrypt", "generateKey", "deriveBits"].forEach(function (m) { wrapMethod(W.crypto.subtle, m, "crypto.subtle." + m); }); }
  wrapMethod(W.crypto, "getRandomValues", "crypto.getRandomValues");
  wrapGetter(W.Document.prototype, "cookie", "document.cookie");
  wrapMethod(W, "matchMedia", "matchMedia"); wrapMethod(W, "structuredClone", "structuredClone"); wrapMethod(W, "requestIdleCallback", "requestIdleCallback");
  wrapMethod(W.history, "pushState", "history.pushState"); wrapMethod(W.history, "replaceState", "history.replaceState");
  wrapMethod(W.performance, "getEntriesByType", "performance.getEntriesByType"); wrapMethod(W.performance, "mark", "performance.mark"); wrapMethod(W.performance, "measure", "performance.measure");
  if (W.PerformanceObserver) wrapMethod(W.PerformanceObserver.prototype, "observe", "PerformanceObserver.observe");
  if (W.Notification) { try { wrapGetter(W.Notification, "permission", "Notification.permission"); } catch (e) {} wrapMethod(W.Notification, "requestPermission", "Notification.requestPermission"); }
  if (W.navigator.serviceWorker) wrapMethod(W.navigator.serviceWorker, "register", "serviceWorker.register");
  wrapMethod(W.URL, "createObjectURL", "URL.createObjectURL"); wrapMethod(W, "open", "window.open"); wrapMethod(W, "postMessage", "window.postMessage");
  if (W.Element) { wrapMethod(W.Element.prototype, "attachShadow", "Element.attachShadow"); wrapMethod(W.Element.prototype, "requestFullscreen", "Element.requestFullscreen"); }
  if (W.HTMLMediaElement) { wrapMethod(W.HTMLMediaElement.prototype, "play", "HTMLMediaElement.play"); wrapMethod(W.HTMLMediaElement.prototype, "canPlayType", "HTMLMediaElement.canPlayType"); }
  if (W.customElements) wrapMethod(W.customElements, "define", "customElements.define");
  wrapMethod(W.document, "execCommand", "document.execCommand");
  // promise-returning calls (second trace pass, 2026-10-16: Vimeo's five throws had no trace line)
  wrapAsync(W, "fetch", "fetch"); wrapAsync(W, "createImageBitmap", "createImageBitmap");
  if (W.crypto && W.crypto.subtle) { ["digest", "importKey", "exportKey", "sign", "verify", "encrypt", "decrypt", "generateKey", "deriveBits", "deriveKey", "wrapKey", "unwrapKey"].forEach(function (m) { wrapAsync(W.crypto.subtle, m, "crypto.subtle." + m); }); }
  if (W.caches) { ["open", "match", "has", "keys", "delete"].forEach(function (m) { wrapAsync(W.caches, m, "caches." + m); }); }
  if (W.navigator.storage) { ["estimate", "persist", "persisted", "getDirectory"].forEach(function (m) { wrapAsync(W.navigator.storage, m, "navigator.storage." + m); }); }
  if (W.navigator.permissions) wrapAsync(W.navigator.permissions, "query", "navigator.permissions.query");
  if (W.navigator.mediaDevices) { ["enumerateDevices", "getUserMedia"].forEach(function (m) { wrapAsync(W.navigator.mediaDevices, m, "mediaDevices." + m); }); }
  if (W.navigator.locks) wrapAsync(W.navigator.locks, "request", "navigator.locks.request");
  if (W.navigator.clipboard) { ["readText", "writeText", "read", "write"].forEach(function (m) { wrapAsync(W.navigator.clipboard, m, "clipboard." + m); }); }
  wrapAsync(W.navigator, "getBattery", "navigator.getBattery"); wrapAsync(W.navigator, "share", "navigator.share"); wrapAsync(W.navigator, "requestMediaKeySystemAccess", "requestMediaKeySystemAccess");
  if (W.document.fonts) { wrapAsync(W.document.fonts, "load", "document.fonts.load"); }
  wrapAsync(W.document, "requestStorageAccess", "document.requestStorageAccess"); wrapAsync(W.document, "hasStorageAccess", "document.hasStorageAccess");
  if (W.HTMLMediaElement) wrapAsync(W.HTMLMediaElement.prototype, "play", "HTMLMediaElement.play (promise)");
  if (W.Response) { ["json", "text", "arrayBuffer", "blob", "formData"].forEach(function (m) { wrapAsync(W.Response.prototype, m, "Response." + m); }); }
  if (W.Blob) { ["text", "arrayBuffer"].forEach(function (m) { wrapAsync(W.Blob.prototype, m, "Blob." + m); }); }
  if (W.AudioContext) wrapAsync(W.AudioContext.prototype, "decodeAudioData", "decodeAudioData");
  if (W.Notification) wrapAsync(W.Notification, "requestPermission", "Notification.requestPermission (promise)");
  if (W.ServiceWorkerContainer) wrapAsync(W.ServiceWorkerContainer.prototype, "register", "serviceWorker.register (promise)");
  if (W.IDBFactory) wrapMethod(W.IDBFactory.prototype, "databases", "indexedDB.databases");
  // getters on the navigator, window and document that an engine may refuse
  ["connection", "mediaDevices", "permissions", "storage", "credentials", "locks", "geolocation", "wakeLock", "userAgentData", "serviceWorker", "mediaSession", "presentation", "bluetooth", "usb", "hid", "xr", "gpu", "ink", "virtualKeyboard", "windowControlsOverlay", "scheduling", "login", "keyboard", "contacts", "share", "plugins", "mimeTypes", "doNotTrack", "globalPrivacyControl", "deviceMemory", "hardwareConcurrency", "maxTouchPoints", "onLine", "language", "languages", "cookieEnabled", "webdriver", "pdfViewerEnabled"].forEach(function (g) { try { wrapGetter(W.Navigator.prototype, g, "navigator." + g); } catch (e) {} });
  ["caches", "speechSynthesis", "visualViewport", "navigation", "scheduler", "crypto", "indexedDB", "localStorage", "sessionStorage", "screen", "devicePixelRatio", "performance", "customElements", "trustedTypes", "launchQueue", "cookieStore", "documentPictureInPicture", "external", "applicationCache"].forEach(function (g) { try { wrapGetter(W.Window.prototype, g, "window." + g); } catch (e) {} });
  ["fonts", "featurePolicy", "permissionsPolicy", "visibilityState", "hidden", "referrer", "domain", "timeline", "pictureInPictureEnabled", "fullscreenEnabled", "adoptedStyleSheets"].forEach(function (g) { try { wrapGetter(W.Document.prototype, g, "document." + g); } catch (e) {} });
  if (W.Screen) { ["orientation", "colorDepth", "pixelDepth", "availWidth", "availHeight", "isExtended"].forEach(function (g) { try { wrapGetter(W.Screen.prototype, g, "screen." + g); } catch (e) {} }); }
})
`;

function JihadWebCompat() {
  this.wrappedJSObject = this;
  this._enabled = boolPref(PREF_ENABLED, true);
  this._trace = boolPref(PREF_TRACE, false);
  this._probe = charPref(PREF_PROBE, "");
  this._documents = 0;
  this._installed = new Map();          // shim name → documents it was installed in
  this._sheet = null;
  try { Services.prefs.addObserver("jihad.compat.", this, false); } catch (e) {}
  try { Services.obs.addObserver(this, "document-element-inserted", false); } catch (e) {}
  this._registerSheet();
  let self = this;
  setTimeout(function () { log("ready: shims " + (self._enabled ? "on" : "off") + (self._trace ? ", trace on" : "")); }, 4000);
}

JihadWebCompat.prototype = {
  classID: Components.ID("{5a1c7e2d-9b3f-4c6e-8d21-7f0a4b9c2e13}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIObserver, Ci.nsISupportsWeakReference]),

  _registerSheet: function () {
    try {
      let sss = Cc["@mozilla.org/content/style-sheet-service;1"].getService(Ci.nsIStyleSheetService);
      let uri = Services.io.newURI("data:text/css;charset=utf-8," + encodeURIComponent(AGENT_CSS), null, null);
      if (!sss.sheetRegistered(uri, sss.AGENT_SHEET)) sss.loadAndRegisterSheet(uri, sss.AGENT_SHEET);
      this._sheet = uri;
    } catch (e) { log("agent sheet not registered: " + e); }
  },

  observe: function (subject, topic, data) {
    if (topic === "nsPref:changed") {
      if (data === PREF_ENABLED) this._enabled = boolPref(PREF_ENABLED, true);
      else if (data === PREF_TRACE) this._trace = boolPref(PREF_TRACE, false);
      else if (data === PREF_PROBE) this._probe = charPref(PREF_PROBE, "");
      return;
    }
    if (topic === "document-element-inserted") this._onDocument(subject);
  },

  _onDocument: function (doc) {
    if (!this._enabled && !this._trace) return;
    let uri = null, win = null;
    try { uri = doc.documentURIObject; win = doc.defaultView; } catch (e) { return; }
    if (!uri || !win || (uri.scheme !== "http" && uri.scheme !== "https")) return;
    try {
      // The page's own window object, handed to the shims as W: the sandbox has the page's
      // principal, so W is the page's window itself, its members the page's own (no prototype
      // proxy in between — with sandboxPrototype, a method taken from `window` came bound to the
      // proxy and refused the page's window as its receiver, first trace round 2026-10-16).
      let raw = ("wrappedJSObject" in win) ? win.wrappedJSObject : win;
      let sandbox = Cu.Sandbox(win, {wantXrays: false, sameZoneAs: win, sandboxName: "jihad-web-compat"});
      if (this._trace) {
        let host = uri.asciiHost;
        Cu.exportFunction(function (s) { log("trace [" + host + "] " + s); }, sandbox, {defineAs: "__jihadTrace"});
        Cu.evalInSandbox(TRACE, sandbox, "latest", "jihad-web-compat-trace.js", 1)(raw);
        // the probe: a diagnosis script from the pref, in the top document, once the page has had
        // 25 s; what it returns is logged (page-controlled text — trace mode only). Its sandbox
        // has the window as prototype so the script's bare names (document, getComputedStyle)
        // resolve as in a page.
        if (this._probe && win === win.top) {
          let src = "";
          try { src = atob(this._probe); } catch (e) { log("probe: the pref is not base64"); }
          if (src) setTimeout(function () {
            try {
              if (win.closed) return;
              let psb = Cu.Sandbox(win, {sandboxPrototype: win, wantXrays: false, sameZoneAs: win, sandboxName: "jihad-web-compat-probe"});
              let r = Cu.evalInSandbox(src, psb, "latest", "jihad-web-compat-probe.js", 1);
              log("probe [" + host + "] " + String(r).slice(0, 6000));
            } catch (e) { log("probe [" + host + "] threw " + e); }
          }, 25000);
        }
      }
      if (this._enabled) {
        let installed = Cu.evalInSandbox(SHIMS, sandbox, "latest", "jihad-web-compat.js", 1)(raw);
        this._documents++;
        let names = [];
        try { names = Array.from(installed || []); } catch (e) {}
        for (let n of names) this._installed.set(n, (this._installed.get(n) || 0) + 1);
        if (this._documents <= 3 || this._trace) log(uri.asciiHost + ": " + (names.length ? names.join(", ") : "nothing needed"));
      }
    } catch (e) { log("shims failed on " + uri.spec.slice(0, 100) + ": " + e); }
  },

  // for the settings page (wrappedJSObject)
  state: function () {
    let counts = {}; for (let [k, v] of this._installed) counts[k] = v;
    return {enabled: this._enabled, trace: this._trace, documents: this._documents, installed: counts};
  }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadWebCompat]);
