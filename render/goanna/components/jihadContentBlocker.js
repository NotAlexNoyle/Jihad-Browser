/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — ads and trackers blocked in the engine (cavekit-content-blocking.md).
 *
 * WHAT IT DOES. Two things, both with the engine's own machinery, for all three interfaces:
 *   1. Host blocking (R1): an nsIContentPolicy in the "content-policy" category. A load whose
 *      host, or a parent domain of it, is on the lists is refused before it leaves the device
 *      (REJECT_REQUEST). Never the top-level document (a listed site still opens when asked for),
 *      never a load from the page's own host. Rules marked $third-party refuse only when the
 *      request's base domain (eTLD+1) differs from the page's. @@||host^ exceptions win.
 *   2. Hiding (R3): EasyList's generic element-hiding rules that are a single class or id become
 *      one user stylesheet (nsIStyleSheetService USER_SHEET) of `sel { display:none !important }`,
 *      limited to http(s) documents with @-moz-document, so the frames adverts leave are gone.
 *
 * THE LISTS (R2, R5): EasyList and EasyPrivacy, by The EasyList authors, GPLv3+ (NOTICE,
 * third_party/easylist/README.md). The package carries them raw in <GRE>/blocklists/. They are
 * compiled HERE, in JavaScript, into the sets and the sheet — one compiler for the bundled
 * snapshot and for updates — and the compiled form is cached in <profile>/jihad-blocklists/
 * compiled.json, keyed by the lists' "! Version:" and the compiler's version. Updates come from
 * easylist.to at most every 4 days (the lists' own "! Expires"), newer version only, swapped in
 * place; on any failure what is loaded stays loaded. The bundled snapshot is the fallback for ever.
 *
 * ALSO TAKEN (2026-10-15, the first device test: google-analytics' rule is typed, MDN's own advert
 * is a path rule and a site rule): ||host^$<types>,third-party rules (a type mask per host),
 * ||host/path prefix rules, and site-specific hiding rules (a.com,b.com##sel → an @-moz-document
 * domain() block, one CSS rule per selector so an unparsable one drops only itself).
 *
 * WHAT IS SKIPPED, by design (counted, logged): rules with other options (domain=, redirect, csp,
 * popup, …), wildcard and regex rules, ~domain negations, generic cosmetic rules that are not a
 * single id or class, ids/classes that start with a digit (invalid as a CSS identifier
 * unescaped). This version is sized for a 1 GB tablet running three daemons.
 *
 * MEMORY (R6): the sets are JS Sets of hostnames (~97,000 from today's lists). Compiling is done
 * in slices of lines between timer ticks, so the daemon keeps painting while it runs.
 *
 * HOW IT IS SEEN: Services.console lines reach the daemon log as "[jihad-bs] js-console
 * (no source):0 blocker: …"; the settings page reads state() through wrappedJSObject.
 */

"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu, results: Cr } = Components;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/NetUtil.jsm");
Cu.import("resource://gre/modules/FileUtils.jsm");
Cu.import("resource://gre/modules/Timer.jsm");

const PREF_ENABLED  = "jihad.blocking.enabled";
const PREF_COSMETIC = "jihad.blocking.cosmetic";
const PREF_LOG      = "jihad.blocking.log";
const PREF_LAST     = "jihad.blocking.lastUpdate";
const PREF_VERSIONS = "jihad.blocking.versions";

const COMPILER_VERSION = 3;                 // bump when the compiled form changes
const UPDATE_EVERY_S   = 4 * 24 * 3600;     // the lists say "! Expires: 4 days"
const UPDATE_CHECK_MS  = 6 * 3600 * 1000;   // how often the timer looks at the clock
const LISTS = [
  { name: "easylist",    url: "https://easylist.to/easylist/easylist.txt" },
  { name: "easyprivacy", url: "https://easylist.to/easylist/easyprivacy.txt" }
];
const SLICE_LINES = 8000;                   // lines compiled per timer tick

const R_HOST     = /^\|\|([a-z0-9][a-z0-9.-]*[a-z0-9])\^$/;
const R_HOST_3P  = /^\|\|([a-z0-9][a-z0-9.-]*[a-z0-9])\^\$third-party$/;
// ||host^$opts with type names and/or third-party only (||google-analytics.com^$script,third-party,xmlhttprequest)
const R_HOST_OPT = /^\|\|([a-z0-9][a-z0-9.-]*[a-z0-9])\^\$([a-z,~-]+)$/;
// ||host/path (a prefix of the path, no wildcard), bare or $third-party (||developer.mozilla.org/pong/)
const R_PATH     = /^\|\|([a-z0-9][a-z0-9.-]*[a-z0-9])(\/[^$*^|]*)(?:\$third-party)?$/;
const R_EXC      = /^@@\|\|([a-z0-9][a-z0-9.-]*[a-z0-9])\^$/;
const R_COS      = /^##([#.]-?[A-Za-z_][A-Za-z0-9_-]*)$/;
const R_COS_EXC  = /^#@#([#.]-?[A-Za-z_][A-Za-z0-9_-]*)$/;
// a.com,b.com##selector — any selector without braces, quotes or a backslash, one CSS rule each
const R_SITE_COS = /^([a-z0-9.,-]+)##([^{}"'\\\n]+)$/;
const R_SITE_EXC = /^([a-z0-9.,-]+)#@#([^{}"'\\\n]+)$/;
const TYPE_BITS = {other: 1 << 1, script: 1 << 2, image: 1 << 3, stylesheet: 1 << 4, object: 1 << 5, subdocument: 1 << 7,
                   ping: 1 << 10, xmlhttprequest: 1 << 11, font: 1 << 14, media: 1 << 15, websocket: 1 << 16,
                   beacon: 1 << 19, fetch: 1 << 20};
// nsIContentPolicy types → the option a list writer would use (TYPE_FETCH behaves as xmlhttprequest)
const TYPE_BIT_OF = {1: 1 << 1, 2: 1 << 2, 3: 1 << 3, 4: 1 << 4, 5: 1 << 5, 7: 1 << 7, 10: 1 << 10, 11: 1 << 11, 14: 1 << 14,
                     15: 1 << 15, 16: 1 << 16, 19: 1 << 19, 20: 1 << 11};

const TYPE_NAMES = {1: "other", 2: "script", 3: "image", 4: "stylesheet", 5: "object", 6: "document",
                    7: "subdocument", 9: "xbl", 10: "ping", 11: "xhr", 12: "objectrequest", 13: "dtd",
                    14: "font", 15: "media", 16: "websocket", 17: "csp", 18: "xslt", 19: "beacon",
                    20: "fetch", 21: "manifest", 22: "import"};

function log(s) {
  try { Services.console.logStringMessage("blocker: " + s); } catch (e) {}
}
function boolPref(name, dflt) { try { return Services.prefs.getBoolPref(name); } catch (e) { return dflt; } }
function intPref(name, dflt)  { try { return Services.prefs.getIntPref(name); } catch (e) { return dflt; } }
function charPref(name, dflt) { try { return Services.prefs.getCharPref(name); } catch (e) { return dflt; } }

function readFileText(file) {
  let fis = Cc["@mozilla.org/network/file-input-stream;1"].createInstance(Ci.nsIFileInputStream);
  fis.init(file, -1, -1, 0);
  try { return NetUtil.readInputStreamToString(fis, fis.available(), {charset: "UTF-8"}); }
  finally { fis.close(); }
}
// A plain file stream on purpose: a "safe" (temp-and-rename) stream that is closed through the
// converter never finishes, and the file does not appear (NS_ERROR_FILE_NOT_FOUND at the sheet's
// registration, device 2026-10-15). These files are rewritten whole on every change.
function writeFileText(file, text) {
  let fos = FileUtils.openFileOutputStream(file, FileUtils.MODE_WRONLY | FileUtils.MODE_CREATE | FileUtils.MODE_TRUNCATE);
  let conv = Cc["@mozilla.org/intl/converter-output-stream;1"].createInstance(Ci.nsIConverterOutputStream);
  conv.init(fos, "UTF-8", 0, 0);
  conv.writeString(text);
  conv.close();
}
function versionOf(text) {
  let m = /^! Version:\s*(\S+)/m.exec(text.slice(0, 2000));
  return m ? m[1] : "";
}
// "! Version: 202610151429" → seconds since the epoch (UTC), or 0.
function versionToSeconds(v) {
  let m = /^(\d{4})(\d{2})(\d{2})(\d{2})(\d{2})$/.exec(v || "");
  if (!m) return 0;
  return Math.floor(Date.UTC(+m[1], +m[2] - 1, +m[3], +m[4], +m[5]) / 1000);
}
function profileDir() {
  let d = Services.dirsvc.get("ProfD", Ci.nsIFile);
  d.append("jihad-blocklists");
  if (!d.exists()) d.create(Ci.nsIFile.DIRECTORY_TYPE, 0o755);
  return d;
}
function bundledFile(name) {
  let f = Services.dirsvc.get("GreD", Ci.nsIFile);
  f.append("blocklists"); f.append(name + ".txt");
  return f;
}
function profileFile(name) { let f = profileDir(); f.append(name); return f; }

// ── the compiler ───────────────────────────────────────────────────────────────────────────────
// Feeds lines of one or more lists through the five patterns, a slice per tick, then calls back
// with the sets, the sheet text and the counts.
function compile(texts, onDone) {
  let hosts = new Set(), tp = new Set(), exc = new Set(), cos = new Set(), cosExc = new Set();
  let typed = new Map();            // host → {types: bitmask, tp: 1 third-party only | -1 first-party only | 0 any}
  let paths = new Map();            // host → ["/prefix", …]
  let site = new Map();             // "a.com,b.com" → Set(selector)
  let siteExc = [];                 // [domains, selector]
  let counts = {rules: 0, host: 0, host3p: 0, hostTyped: 0, path: 0, exc: 0, cosmetic: 0, cosmeticExc: 0,
                siteCosmetic: 0, siteCosmeticExc: 0, skipped: 0};
  let lists = texts.map(t => t.split("\n")), li = 0, lines = lists[0] || [];
  let i = 0, started = Date.now();
  function slice() {
    let end = Math.min(i + SLICE_LINES, lines.length);
    for (; i < end; i++) {
      let line = lines[i].trim();
      if (!line || line[0] === "!" || line[0] === "[") continue;
      counts.rules++;
      let m;
      if ((m = R_HOST.exec(line)))         { hosts.add(m[1]); counts.host++; }
      else if ((m = R_HOST_3P.exec(line))) { tp.add(m[1]); counts.host3p++; }
      else if ((m = R_HOST_OPT.exec(line))) {
        let types = 0, party = 0, ok = true;
        for (let o of m[2].split(",")) {
          if (o === "third-party") party = 1;
          else if (o === "~third-party") party = -1;
          else if (TYPE_BITS[o]) types |= TYPE_BITS[o];
          else { ok = false; break; }              // domain=, redirect, csp, popup, … : not this version
        }
        if (ok) { let t = typed.get(m[1]) || {types: 0, tp: party}; t.types |= types; t.tp = party; typed.set(m[1], t); counts.hostTyped++; }
        else counts.skipped++;
      }
      else if ((m = R_PATH.exec(line)))    { let a = paths.get(m[1]); if (!a) { a = []; paths.set(m[1], a); } a.push(m[2]); counts.path++; }
      else if ((m = R_EXC.exec(line)))     { exc.add(m[1]); counts.exc++; }
      else if ((m = R_COS.exec(line)))     { cos.add(m[1]); counts.cosmetic++; }
      else if ((m = R_COS_EXC.exec(line))) { cosExc.add(m[1]); counts.cosmeticExc++; }
      else if ((m = R_SITE_COS.exec(line))) {
        if (m[1].indexOf("~") >= 0) { counts.skipped++; }
        else { let k = m[1]; let set = site.get(k); if (!set) { set = new Set(); site.set(k, set); } set.add(m[2].trim()); counts.siteCosmetic++; }
      }
      else if ((m = R_SITE_EXC.exec(line))) { siteExc.push([m[1], m[2].trim()]); counts.siteCosmeticExc++; }
      else counts.skipped++;
    }
    if (i < lines.length) { setTimeout(slice, 0); return; }
    if (++li < lists.length) { lines = lists[li]; i = 0; setTimeout(slice, 0); return; }   // the next list
    lists = null; lines = null;
    for (let s of cosExc) cos.delete(s);
    for (let h of exc) { hosts.delete(h); tp.delete(h); typed.delete(h); paths.delete(h); }
    // a site exception removes that selector from every domain group that names one of its
    // domains: the groups are indexed by domain first (301 exceptions x 8,000 groups was 20 s)
    if (siteExc.length) {
      let byDomain = new Map();
      for (let [k, set] of site) for (let d of k.split(",")) { let a = byDomain.get(d); if (!a) { a = []; byDomain.set(d, a); } a.push(set); }
      for (let [doms, sel] of siteExc) for (let d of doms.split(",")) { let a = byDomain.get(d); if (a) for (let set of a) set.delete(sel); }
    }
    let out = ["@-moz-document url-prefix(http://), url-prefix(https://) {"], sels = Array.from(cos);
    for (let j = 0; j < sels.length; j += 400) out.push(sels.slice(j, j + 400).join(",") + "{display:none!important}");
    out.push("}");
    // site-specific rules: one @-moz-document block per domain group, one rule per selector so a
    // selector the engine cannot parse drops only itself
    for (let [k, set] of site) {
      if (!set.size) continue;
      out.push("@-moz-document " + k.split(",").map(d => "domain(" + d + ")").join(",") + " {");
      for (let sel of set) out.push(sel + "{display:none!important}");
      out.push("}");
    }
    let css = out.join("\n") + "\n";
    counts.ms = Date.now() - started;
    onDone({hosts: hosts, tp: tp, exc: exc, typed: typed, paths: paths, css: css, counts: counts});
  }
  setTimeout(slice, 0);
}

function packCache(r, versions) {
  let typed = {}; for (let [h, t] of r.typed) typed[h] = [t.types, t.tp];
  let paths = {}; for (let [h, a] of r.paths) paths[h] = a;
  return JSON.stringify({compiler: COMPILER_VERSION, versions: versions,
    hosts: Array.from(r.hosts).join("\n"), tp: Array.from(r.tp).join("\n"), exc: Array.from(r.exc).join("\n"),
    typed: typed, paths: paths, css: r.css, counts: r.counts});
}
function unpackCache(c) {
  let typed = new Map(); for (let h in c.typed) typed.set(h, {types: c.typed[h][0], tp: c.typed[h][1]});
  let paths = new Map(); for (let h in c.paths) paths.set(h, c.paths[h]);
  return {hosts: new Set(c.hosts.split("\n")), tp: new Set(c.tp.split("\n")), exc: new Set(c.exc.split("\n")),
          typed: typed, paths: paths, css: c.css, counts: c.counts};
}

function JihadContentBlocker() {
  this.wrappedJSObject = this;
  this._enabled = boolPref(PREF_ENABLED, true);
  this._cosmetic = boolPref(PREF_COSMETIC, true);
  this._log = boolPref(PREF_LOG, true);
  this._hosts = null; this._tp = null; this._exc = null;   // Sets, once loaded
  this._typed = new Map(); this._paths = new Map();
  this._versions = {easylist: "", easyprivacy: ""};
  this._counts = null;
  this._count = 0;                     // refusals this session
  this._sheetURI = null;
  this._updating = false;
  this._lastError = "";
  try { Services.prefs.addObserver("jihad.blocking.", this, false); } catch (e) {}
  // Off the start-up path: the cache or the compile a moment later, the update timer after that.
  setTimeout(() => this._load(), 1500);
  this._timer = setInterval(() => this._maybeUpdate(false), UPDATE_CHECK_MS);
  setTimeout(() => this._maybeUpdate(false), 90 * 1000);
}

JihadContentBlocker.prototype = {
  classID: Components.ID("{3c7e1b9a-5d24-4f6e-8a0b-9e2d4c6f1a37}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIContentPolicy, Ci.nsIObserver, Ci.nsISupportsWeakReference]),

  // ── nsIObserver: the prefs ──
  observe: function (subject, topic, data) {
    if (topic !== "nsPref:changed") return;
    if (data === PREF_ENABLED) { this._enabled = boolPref(PREF_ENABLED, true); log("blocking " + (this._enabled ? "on" : "off")); }
    else if (data === PREF_COSMETIC) { this._cosmetic = boolPref(PREF_COSMETIC, true); this._applySheet(); }
    else if (data === PREF_LOG) { this._log = boolPref(PREF_LOG, true); }
  },

  // ── nsIContentPolicy ──
  shouldLoad: function (type, location, origin, context, mime, extra, principal) {
    if (!this._enabled || !this._hosts || type === Ci.nsIContentPolicy.TYPE_DOCUMENT) return Ci.nsIContentPolicy.ACCEPT;
    let scheme;
    try { scheme = location.scheme; } catch (e) { return Ci.nsIContentPolicy.ACCEPT; }
    if (scheme !== "http" && scheme !== "https" && scheme !== "ws" && scheme !== "wss") return Ci.nsIContentPolicy.ACCEPT;
    let host;
    try { host = location.asciiHost.toLowerCase(); } catch (e) { return Ci.nsIContentPolicy.ACCEPT; }
    if (!host) return Ci.nsIContentPolicy.ACCEPT;
    if (host[host.length - 1] === ".") host = host.slice(0, -1);
    let pageHost = this._pageHost(context, origin);
    let verdict;
    if (pageHost && pageHost === host) {
      // The page's own host: only a path rule can refuse (||developer.mozilla.org/pong/ is MDN's own
      // advert server; host rules never apply to the page itself).
      verdict = this._lookupPath(host, location) ? 1 : 0;
    } else {
      verdict = this._lookup(host, pageHost, type, location);
    }
    if (verdict === 0) return Ci.nsIContentPolicy.ACCEPT;
    this._count++;
    if (this._log) log("blocked " + (TYPE_NAMES[type] || type) + " " + host + " on " + (pageHost || "?"));
    return Ci.nsIContentPolicy.REJECT_REQUEST;
  },
  shouldProcess: function () { return Ci.nsIContentPolicy.ACCEPT; },

  // 0 = load; 1 = refuse. Walks host, then each parent domain, so a rule for ads.example.com
  // covers x.ads.example.com, and ||example.com^ covers them all. An exception at any level wins.
  _lookup: function (host, pageHost, type, location) {
    let thirdParty = null;             // computed once, only if a $third-party rule is met
    let path = null;                   // read once, only if a path rule's host is met
    let h = host;
    for (;;) {
      if (this._exc.has(h)) return 0;
      if (this._hosts.has(h)) return 1;
      if (this._tp.has(h)) {
        if (thirdParty === null) thirdParty = this._isThirdParty(host, pageHost);
        if (thirdParty) return 1;
      }
      let t = this._typed.get(h);
      if (t && (t.types === 0 || (t.types & (TYPE_BIT_OF[type] || 0)))) {
        if (t.tp === 0) return 1;
        if (thirdParty === null) thirdParty = this._isThirdParty(host, pageHost);
        if (t.tp === 1 ? thirdParty : !thirdParty) return 1;
      }
      let ps = this._paths.get(h);
      if (ps) {
        if (path === null) { try { path = location.path; } catch (e) { path = ""; } }
        for (let i = 0; i < ps.length; i++) if (path.indexOf(ps[i]) === 0) return 1;
      }
      let dot = h.indexOf(".");
      if (dot < 0) return 0;
      h = h.slice(dot + 1);
      if (h.indexOf(".") < 0) return 0;  // never match a bare TLD
    }
  },
  _lookupPath: function (host, location) {
    let h = host, path = null;
    for (;;) {
      let ps = this._paths.get(h);
      if (ps) {
        if (path === null) { try { path = location.path; } catch (e) { path = ""; } }
        for (let i = 0; i < ps.length; i++) if (path.indexOf(ps[i]) === 0) return true;
      }
      let dot = h.indexOf(".");
      if (dot < 0) return false;
      h = h.slice(dot + 1);
      if (h.indexOf(".") < 0) return false;
    }
  },
  _baseDomain: function (host) {
    try { return Services.eTLD.getBaseDomainFromHost(host, 0); } catch (e) { return host; }
  },
  _isThirdParty: function (host, pageHost) {
    if (!pageHost) return true;
    return this._baseDomain(host) !== this._baseDomain(pageHost);
  },
  // The TOP document's host (EasyList's "party" is the page the user is on, not an iframe).
  _pageHost: function (context, origin) {
    let doc = null;
    try {
      if (context) {
        if (context instanceof Ci.nsIDOMDocument) doc = context;
        else if (context instanceof Ci.nsIDOMNode) doc = context.ownerDocument;
        else if (context instanceof Ci.nsIDOMWindow) doc = context.document;
      }
      if (doc && doc.defaultView && doc.defaultView.top && doc.defaultView.top.document) doc = doc.defaultView.top.document;
      if (doc && doc.documentURIObject) {
        let h = doc.documentURIObject.asciiHost;
        if (h) return h.toLowerCase();
      }
    } catch (e) {}
    try { if (origin) { let h = origin.asciiHost; if (h) return h.toLowerCase(); } } catch (e) {}
    return "";
  },

  // ── loading: the cache, or a compile of the raw lists ──
  _rawFile: function (name) {
    let f = profileFile(name + ".txt");            // an update, if one was ever kept
    if (f.exists() && f.fileSize > 1000) return f;
    return bundledFile(name);
  },
  _load: function () {
    try {
      let raw = {}, versions = {};
      for (let l of LISTS) {
        let f = this._rawFile(l.name);
        if (!f.exists()) { log("no list file: " + f.path); continue; }
        raw[l.name] = readFileText(f);
        versions[l.name] = versionOf(raw[l.name]);
      }
      if (!raw.easylist && !raw.easyprivacy) { log("no lists: nothing is blocked"); return; }
      // the cache, if it was made from exactly these versions by this compiler
      let cacheFile = profileFile("compiled.json");
      if (cacheFile.exists()) {
        try {
          let c = JSON.parse(readFileText(cacheFile));
          if (c.compiler === COMPILER_VERSION && c.versions.easylist === versions.easylist && c.versions.easyprivacy === versions.easyprivacy) {
            this._install(unpackCache(c), versions, "cache");
            return;
          }
        } catch (e) { log("cache unreadable, recompiling: " + e); }
      }
      let texts = [raw.easylist || "", raw.easyprivacy || ""];
      compile(texts, (r) => {
        this._install(r, versions, "compiled");
        try { writeFileText(cacheFile, packCache(r, versions)); } catch (e) { log("cache not written: " + e); }
      });
    } catch (e) { log("load failed: " + e); }
  },
  _install: function (r, versions, how) {
    this._hosts = r.hosts; this._tp = r.tp; this._exc = r.exc; this._typed = r.typed || new Map(); this._paths = r.paths || new Map();
    this._css = r.css; this._counts = r.counts;
    this._versions = versions;
    let v = (versions.easylist || "-") + "/" + (versions.easyprivacy || "-");
    try { Services.prefs.setCharPref(PREF_VERSIONS, v); } catch (e) {}
    if (intPref(PREF_LAST, 0) === 0) {
      // the first start: the snapshot's own date is when the lists were last current
      let s = versionToSeconds(versions.easylist) || Math.floor(Date.now() / 1000);
      try { Services.prefs.setIntPref(PREF_LAST, s); } catch (e) {}
    }
    log(how + ": EasyList " + versions.easylist + ", EasyPrivacy " + versions.easyprivacy + " - hosts " + r.hosts.size +
        ", third-party " + r.tp.size + ", typed " + this._typed.size + ", path " + r.counts.path + ", exceptions " + r.exc.size +
        ", selectors " + r.counts.cosmetic + " (less " + r.counts.cosmeticExc + "), site selectors " + r.counts.siteCosmetic +
        " (less " + r.counts.siteCosmeticExc + "), skipped " + r.counts.skipped + (r.counts.ms !== undefined ? ", " + r.counts.ms + " ms" : ""));
    this._applySheet();
  },

  // ── the cosmetic sheet ──
  _applySheet: function () {
    try {
      let sss = Cc["@mozilla.org/content/style-sheet-service;1"].getService(Ci.nsIStyleSheetService);
      if (this._sheetURI && sss.sheetRegistered(this._sheetURI, sss.USER_SHEET)) sss.unregisterSheet(this._sheetURI, sss.USER_SHEET);
      this._sheetURI = null;
      if (!this._cosmetic || !this._css) return;
      let f = profileFile("cosmetic.css");
      writeFileText(f, this._css);
      this._sheetURI = Services.io.newFileURI(f);
      sss.loadAndRegisterSheet(this._sheetURI, sss.USER_SHEET);
      log("hiding on (" + this._counts.cosmetic + " selectors)");
    } catch (e) { log("sheet failed: " + e); this._sheetURI = null; }
  },

  // ── updates ──
  _maybeUpdate: function (force) {
    if (this._updating || !this._hosts) return;
    let now = Math.floor(Date.now() / 1000);
    if (!force && now - intPref(PREF_LAST, 0) < UPDATE_EVERY_S) return;
    this._updating = true;
    this._lastError = "";
    let got = {}, pending = LISTS.length;
    let finish = () => {
      this._updating = false;
      let newer = LISTS.filter(l => got[l.name] && versionOf(got[l.name]) > (this._versions[l.name] || ""));
      if (!newer.length) {
        if (!this._lastError) { log("update: nothing newer than " + this._versions.easylist + "/" + this._versions.easyprivacy); }
        try { if (!this._lastError) Services.prefs.setIntPref(PREF_LAST, now); } catch (e) {}
        return;
      }
      let texts = {}, versions = {};
      for (let l of LISTS) {
        let t = got[l.name] && versionOf(got[l.name]) > (this._versions[l.name] || "") ? got[l.name] : readFileText(this._rawFile(l.name));
        texts[l.name] = t; versions[l.name] = versionOf(t);
      }
      compile([texts.easylist, texts.easyprivacy], (r) => {
        for (let l of newer) { try { writeFileText(profileFile(l.name + ".txt"), got[l.name]); } catch (e) { log("update: " + l.name + " not kept: " + e); } }
        this._install(r, versions, "update");
        try {
          Services.prefs.setIntPref(PREF_LAST, now);
          writeFileText(profileFile("compiled.json"), packCache(r, versions));
        } catch (e) { log("update: cache not written: " + e); }
      });
    };
    for (let l of LISTS) {
      try {
        let xhr = Cc["@mozilla.org/xmlextras/xmlhttprequest;1"].createInstance(Ci.nsIXMLHttpRequest);
        xhr.open("GET", l.url, true);
        xhr.responseType = "text";
        xhr.timeout = 120000;
        xhr.onload = () => {
          if (xhr.status === 200 && /^\[Adblock/.test(xhr.responseText.slice(0, 20)) && versionOf(xhr.responseText)) got[l.name] = xhr.responseText;
          else { this._lastError = l.name + ": HTTP " + xhr.status; log("update: " + this._lastError); }
          if (--pending === 0) finish();
        };
        xhr.onerror = xhr.ontimeout = () => { this._lastError = l.name + ": no download"; log("update: " + this._lastError); if (--pending === 0) finish(); };
        xhr.send(null);
      } catch (e) { this._lastError = l.name + ": " + e; log("update: " + this._lastError); if (--pending === 0) finish(); }
    }
  },

  // ── for the settings page (wrappedJSObject) ──
  update: function () { this._maybeUpdate(true); },
  state: function () {
    return {enabled: this._enabled, cosmetic: this._cosmetic, loaded: !!this._hosts, versions: this._versions,
            lastUpdate: intPref(PREF_LAST, 0), count: this._count, counts: this._counts, updating: this._updating,
            error: this._lastError};
  }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadContentBlocker]);
