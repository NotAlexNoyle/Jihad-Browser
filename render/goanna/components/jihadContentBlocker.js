/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — ads and trackers blocked in the engine (cavekit-content-blocking.md).
 *
 * WHAT IT DOES, for all three interfaces, with the engine's own machinery:
 *   1. Host blocking (R1): an nsIContentPolicy in the "content-policy" category refuses a load
 *      whose host, or a parent domain of it, is on the lists (REJECT_REQUEST), and an
 *      http-on-modify-request observer refuses the same on a REDIRECTED channel, which the policy
 *      never sees again. Never the top-level document (a listed site still opens when asked for).
 *      A load from the page's own host is refused only by a path rule (||host/path): host rules
 *      never apply to the page itself. $third-party rules refuse only when the request's base
 *      domain (eTLD+1) differs from the page's. Typed rules (||host^$script,third-party) refuse
 *      those types under that party rule. EXCEPTIONS WIN AND ARE NEVER DROPPED: @@||host^ and its
 *      optioned forms (a path, types, $domain=, party) are checked, at every level of the domain
 *      walk, before any block.
 *   2. Hiding (R3): EasyList's generic hiding rules that are a single id or class are one user
 *      stylesheet (nsIStyleSheetService USER_SHEET) of `sel{display:none!important}`, limited to
 *      http(s) documents with @-moz-document; a generic selector that a site excepts is scoped
 *      away from that site; $generichide/$elemhide/$document exceptions take a whole site out of
 *      it. Site-specific rules (a.com,b.com##sel) are kept per domain and loaded into a document
 *      of that domain only (nsIDOMWindowUtils.loadSheet), one CSS rule per selector, so no page
 *      pays for the 8,000 rules it cannot match.
 *
 * THE LISTS (R2, R5): EasyList and EasyPrivacy, by The EasyList authors, GPLv3+ (NOTICE,
 * third_party/easylist/README.md). The package carries them raw in <GRE>/blocklists/. They are
 * compiled HERE, in JavaScript, in slices between timer ticks — one compiler for the bundled
 * snapshot and for updates — and the compiled form is cached in <profile>/jihad-blocklists/
 * (hosts.txt, tp.txt, exc.txt, cosmetic.css, meta.json), keyed by the lists' "! Version:" and
 * the compiler's version. Whichever of the
 * bundled snapshot and the profile's downloaded copy is NEWER is used. Updates come from
 * easylist.to at most every 4 days (plus jitter, so three daemons do not fetch together), newer
 * version only, complete only (a list that lost more than a quarter of its rules is a truncated
 * download), through an anonymous, uncached, background request; on any failure what is loaded
 * stays loaded and the next try waits a day.
 *
 * WHAT IS SKIPPED, by design (counted, logged): rules with other options ($redirect, $csp,
 * $popup, $removeparam, …), wildcard and regex rules, ~domain negations in blocks, generic
 * cosmetic rules that are not a single id or class, selectors with procedural pseudo-classes or
 * unbalanced brackets, ids/classes that start with a digit. Sized for a 1 GB tablet running
 * three daemons: the three host sets are HostSets (one string and two typed arrays each, so a
 * full GC marks a handful of things, not 100,000 strings), the small maps stay Maps, the raw
 * list text is not kept (everything is built from the cache's own files), and the lists are
 * held only while a card is connected (loaded on the first card, dropped after ten idle minutes).
 *
 * LOGGING: a refusal logs the LIST ENTRY that matched (never the page-chosen request host), and
 * only when jihad.blocking.log is on (off by default: hundreds of lines a page churn the capped
 * device log). Services.console lines reach the daemon log as "[jihad-bs] js-console (no
 * source):0 blocker: …"; the settings page reads state() through wrappedJSObject.
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
const PREF_LAST     = "jihad.blocking.lastUpdate";      // seconds: when the lists were last current
const PREF_ATTEMPT  = "jihad.blocking.lastAttempt";     // seconds: the last update attempt
const PREF_VERSIONS = "jihad.blocking.versions";

const COMPILER_VERSION = 9;                 // bump when the compiled form changes
const UPDATE_EVERY_S   = 4 * 24 * 3600;     // the lists say "! Expires: 4 days"
const RETRY_AFTER_S    = 24 * 3600;         // after a failed attempt
const UPDATE_CHECK_MS  = 6 * 3600 * 1000;   // how often the timer looks at the clock
const QUIET_AFTER_DOC_MS = 20 * 1000;       // no update work within 20 s of a page starting
const UNLOAD_AFTER_IDLE_MS = 10 * 60 * 1000; // the lists are dropped ten minutes after the last card leaves
const LISTS = [
  { name: "easylist",    url: "https://easylist.to/easylist/easylist.txt" },
  { name: "easyprivacy", url: "https://easylist.to/easylist/easyprivacy.txt" }
];
const SLICE_LINES = 8000;                   // lines compiled per timer tick

const HOST = "([a-z0-9][a-z0-9.-]*[a-z0-9])";
const R_HOST      = new RegExp("^\\|\\|" + HOST + "\\^$");
const R_HOST_OPT  = new RegExp("^\\|\\|" + HOST + "\\^\\$([a-z,~=|.-]+)$");          // ||host^$opts
const R_PATH      = new RegExp("^\\|\\|" + HOST + "(/[^$*^|]*)(?:\\$([a-z,~=|.-]+))?$"); // ||host/path[$opts]
const R_EXC_HOST  = new RegExp("^@@\\|\\|" + HOST + "\\^(?:\\$([a-z,~=|.-]+))?$");      // @@||host^[$opts]
const R_EXC_PATH  = new RegExp("^@@\\|\\|" + HOST + "(/[^$*^|]*)(?:\\$([a-z,~=|.-]+))?$");
const R_EXC_PFX   = /^@@:\/\/([0-9.]+)\$([a-z,~=|.-]+)$/;                              // @@://192.168.$generichide
const R_COS       = /^##([#.]-?[A-Za-z_][A-Za-z0-9_-]*)$/;
const R_COS_EXC   = /^#@#([#.]-?[A-Za-z_][A-Za-z0-9_-]*)$/;
const R_SITE_COS  = /^([a-z0-9.,-]+)##([^{}"'\\\n]+)$/;
const R_SITE_EXC  = /^([a-z0-9.,-]+)#@#([^{}"'\\\n]+)$/;
const R_PROCEDURAL = /:(?:-abp-|has\(|has-text|matches-css|style\(|xpath\(|if\(|if-not\(|contains\(|nth-ancestor|upward|remove\(|watch-attr|min-text-length|matches-path|matches-media|matches-prop|others|not\(:has)/;

// ABP's type options → a bit each; the engine's external content types → the bit they fall under
const TYPE_BITS = {other: 1 << 1, script: 1 << 2, image: 1 << 3, stylesheet: 1 << 4, object: 1 << 5,
                   subdocument: 1 << 7, ping: 1 << 10, xmlhttprequest: 1 << 11, font: 1 << 14,
                   media: 1 << 15, websocket: 1 << 16};
const TYPE_BIT_OF = {1: 1 << 1, 2: 1 << 2, 3: 1 << 3, 4: 1 << 4, 5: 1 << 5, 7: 1 << 7, 10: 1 << 10,
                     11: 1 << 11, 12: 1 << 5, 14: 1 << 14, 15: 1 << 15, 16: 1 << 16, 19: 1 << 10,
                     20: 1 << 11, 21: 1 << 3};
const TYPE_NAMES = {1: "other", 2: "script", 3: "image", 4: "stylesheet", 5: "object", 6: "document",
                    7: "subdocument", 9: "xbl", 10: "ping", 11: "xhr", 12: "object-subrequest", 13: "dtd",
                    14: "font", 15: "media", 16: "websocket", 17: "csp-report", 18: "xslt", 19: "beacon",
                    20: "fetch", 21: "imageset", 22: "web-manifest", 42: "saveas-download"};
const NEVER_TYPES = {6: true, 42: true};    // the document itself; a download the user asked for

function log(s) { try { Services.console.logStringMessage("blocker: " + s); } catch (e) {} }
function boolPref(name, dflt) { try { return Services.prefs.getBoolPref(name); } catch (e) { return dflt; } }
function intPref(name, dflt)  { try { return Services.prefs.getIntPref(name); } catch (e) { return dflt; } }
function setIntPref(name, v)  { try { Services.prefs.setIntPref(name, v); Services.prefs.savePrefFile(null); } catch (e) {} }
function now() { return Math.floor(Date.now() / 1000); }

function readFileText(file, max) {
  let fis = Cc["@mozilla.org/network/file-input-stream;1"].createInstance(Ci.nsIFileInputStream);
  fis.init(file, -1, -1, 0);
  try { return NetUtil.readInputStreamToString(fis, max ? Math.min(max, fis.available()) : fis.available(), {charset: "UTF-8"}); }
  finally { fis.close(); }
}
// The list's "! Version:" from its first 2 KB — never the whole file for that (inspector F-4).
function versionOfFile(file) { try { return versionOf(readFileText(file, 2048)); } catch (e) { return ""; } }
function versionOf(text) {
  let m = /^! Version:\s*(\S+)/m.exec(text.slice(0, 2048));
  return m ? m[1] : "";
}
// Atomic: a safe (temp-and-rename) stream, FLUSHED through the converter and FINISHED through
// FileUtils. Closing the converter instead of finishing the safe stream deletes the temp file —
// which IS the target when the target is new (nsFileStreams.cpp) — so the file never appeared
// (device 2026-10-15; the inspector named the mechanism).
function writeFileText(file, text) {
  let fos = FileUtils.openSafeFileOutputStream(file);
  let conv = Cc["@mozilla.org/intl/converter-output-stream;1"].createInstance(Ci.nsIConverterOutputStream);
  conv.init(fos, "UTF-8", 0, 0);
  conv.writeString(text);
  conv.flush();
  FileUtils.closeSafeFileOutputStream(fos);
}
function versionToSeconds(v) {
  let m = /^(\d{4})(\d{2})(\d{2})(\d{2})(\d{2})$/.exec(v || "");
  return m ? Math.floor(Date.UTC(+m[1], +m[2] - 1, +m[3], +m[4], +m[5]) / 1000) : 0;
}
function profileDir() {
  let d = Services.dirsvc.get("ProfD", Ci.nsIFile);
  d.append("jihad-blocklists");
  if (!d.exists()) d.create(Ci.nsIFile.DIRECTORY_TYPE, 0o755);
  return d;
}
function profileFile(name) { let f = profileDir(); f.append(name); return f; }
function bundledFile(name) {
  let f = Services.dirsvc.get("GreD", Ci.nsIFile);
  f.append("blocklists"); f.append(name + ".txt");
  return f;
}
function escapeRe(s) { return s.replace(/[.*+?^${}()|[\]\\]/g, "\\$&"); }

// ── option parsing (shared by blocks and exceptions) ────────────────────────────────────────
// Returns {types, party, domains, notDomains, generichide, document} or null when an option is
// not one this version understands.
function parseOptions(opts, forException) {
  let o = {types: 0, party: 0, domains: null, notDomains: null, generichide: false, document: false};
  if (!opts) return o;
  for (let tok of opts.split(",")) {
    if (tok === "third-party") o.party = 1;
    else if (tok === "~third-party") o.party = -1;
    else if (tok.indexOf("domain=") === 0) {
      for (let d of tok.slice(7).split("|")) {
        if (!d) continue;
        if (d[0] === "~") { (o.notDomains = o.notDomains || []).push(d.slice(1)); }
        else { (o.domains = o.domains || []).push(d); }
      }
    }
    else if (TYPE_BITS[tok]) o.types |= TYPE_BITS[tok];
    else if (tok[0] === "~" && TYPE_BITS[tok.slice(1)]) { /* a negated type: treat as "all but": not this version */ return null; }
    else if (forException && (tok === "generichide" || tok === "elemhide")) o.generichide = true;
    else if (forException && tok === "document") { o.document = true; o.generichide = true; }
    else return null;
  }
  if (!forException && (o.notDomains)) return null;   // ~domain negations on blocks: not this version
  return o;
}
function domainMatches(list, pageHost) {
  if (!list) return true;
  if (!pageHost) return false;
  for (let d of list) { if (pageHost === d || (pageHost.length > d.length && pageHost.endsWith("." + d))) return true; }
  return false;
}

// ── the compiler ────────────────────────────────────────────────────────────────────────────
function validSelector(sel) {
  if (R_PROCEDURAL.test(sel) || sel.indexOf("/*") >= 0 || sel.indexOf("@") >= 0 || sel.indexOf(";") >= 0) return false;
  let depth = 0;
  for (let i = 0; i < sel.length; i++) {
    let c = sel[i];
    if (c === "(" || c === "[") depth++;
    else if (c === ")" || c === "]") { if (--depth < 0) return false; }
  }
  return depth === 0;
}
function compile(texts, onDone) {
  let hosts = new Set(), tp = new Set(), excHosts = new Set();
  let typed = new Map();            // host → [{types, party}]
  let paths = new Map();            // host → [{p, party}]
  let excRules = new Map();         // host → [{p, types, party, domains, notDomains}]
  let generichide = [];             // [host or prefix regexp source]
  let cos = new Set(), cosExc = new Set();
  let siteGroups = new Map();       // "a.com,b.com" → Set(selector)
  let siteExc = [];                 // [domains, selector] — against site groups AND generic rules
  let counts = {rules: 0, host: 0, host3p: 0, hostTyped: 0, path: 0, exc: 0, excTyped: 0, generichide: 0,
                cosmetic: 0, cosmeticExc: 0, siteCosmetic: 0, siteCosmeticExc: 0, badSelector: 0, skipped: 0};
  let lists = texts.map(t => t.split("\n")), li = 0, lines = lists[0] || [];
  let i = 0, started = Date.now();
  function addExc(host, p, o) {
    if (!p && o.types === 0 && o.party === 0 && !o.domains && !o.notDomains) { excHosts.add(host); counts.exc++; return; }
    let a = excRules.get(host); if (!a) { a = []; excRules.set(host, a); }
    a.push({p: p ? p.toLowerCase() : "", types: o.types, party: o.party, domains: o.domains, notDomains: o.notDomains});
    counts.excTyped++;
  }
  function slice() {
    let end = Math.min(i + SLICE_LINES, lines.length);
    for (; i < end; i++) {
      let line = lines[i].trim();
      if (!line || line[0] === "!" || line[0] === "[") continue;
      counts.rules++;
      let m, o;
      if ((m = R_HOST.exec(line)))         { hosts.add(m[1]); counts.host++; }
      else if ((m = R_HOST_OPT.exec(line))) {
        o = parseOptions(m[2], false);
        if (!o) { counts.skipped++; continue; }
        if (o.domains) { counts.skipped++; continue; }      // $domain= on a block: not this version
        if (o.types === 0 && o.party === 1) { tp.add(m[1]); counts.host3p++; }
        else { let a = typed.get(m[1]); if (!a) { a = []; typed.set(m[1], a); } a.push({types: o.types, party: o.party}); counts.hostTyped++; }
      }
      else if ((m = R_PATH.exec(line))) {
        o = parseOptions(m[3], false);
        if (!o || o.domains || o.types) { counts.skipped++; continue; }
        let a = paths.get(m[1]); if (!a) { a = []; paths.set(m[1], a); }
        a.push({p: m[2].toLowerCase(), party: o.party}); counts.path++;
      }
      else if ((m = R_EXC_HOST.exec(line))) {
        o = parseOptions(m[2], true);
        if (!o) { counts.skipped++; continue; }
        if (o.generichide) { generichide.push(escapeRe(m[1])); counts.generichide++; if (!o.document && o.types === 0 && !o.domains) continue; }
        addExc(m[1], "", o);
      }
      else if ((m = R_EXC_PATH.exec(line))) {
        o = parseOptions(m[3], true);
        if (!o) { counts.skipped++; continue; }
        addExc(m[1], m[2], o);
      }
      else if ((m = R_EXC_PFX.exec(line))) {
        o = parseOptions(m[2], true);
        if (o && o.generichide) { generichide.push(escapeRe(m[1]) + "[^/]*"); counts.generichide++; } else counts.skipped++;
      }
      else if ((m = R_COS.exec(line)))     { cos.add(m[1]); counts.cosmetic++; }
      else if ((m = R_COS_EXC.exec(line))) { cosExc.add(m[1]); counts.cosmeticExc++; }
      else if ((m = R_SITE_COS.exec(line))) {
        let sel = m[2].trim();
        if (m[1].indexOf("~") >= 0) { counts.skipped++; }
        else if (!validSelector(sel)) { counts.badSelector++; }
        else { let set = siteGroups.get(m[1]); if (!set) { set = new Set(); siteGroups.set(m[1], set); } set.add(sel); counts.siteCosmetic++; }
      }
      else if ((m = R_SITE_EXC.exec(line))) { siteExc.push([m[1], m[2].trim()]); counts.siteCosmeticExc++; }
      else counts.skipped++;
    }
    if (i < lines.length) { setTimeout(slice, 0); return; }
    if (++li < lists.length) { lines = lists[li]; i = 0; setTimeout(slice, 0); return; }
    lists = null; lines = null;
    setTimeout(finish, 0);
  }
  function finish() {
    for (let s of cosExc) cos.delete(s);
    for (let h of excHosts) { hosts.delete(h); tp.delete(h); typed.delete(h); paths.delete(h); }
    // site exceptions: against the site groups naming one of their domains, and against the
    // generic rules (the generic selector is then scoped away from those domains, inspector F-2)
    let byDomain = new Map(), genericExcDomains = new Map();   // selector → [domains]
    for (let [k, set] of siteGroups) for (let d of k.split(",")) { let a = byDomain.get(d); if (!a) { a = []; byDomain.set(d, a); } a.push(set); }
    for (let [doms, sel] of siteExc) {
      for (let d of doms.split(",")) {
        let a = byDomain.get(d); if (a) for (let set of a) set.delete(sel);
        if (cos.has(sel)) { let g = genericExcDomains.get(sel); if (!g) { g = []; genericExcDomains.set(sel, g); } g.push(d); }
      }
    }
    // the generic sheet: scoped to http(s) and away from the generichide sites; a selector with
    // site exceptions gets its own block that excludes those sites
    let scope = "url-prefix(http://), url-prefix(https://)";
    if (generichide.length) scope = 'regexp("^https?://(?!([^/]*\\\\.)?(' + generichide.join("|") + ')[:/]).*")';
    let out = ["@-moz-document " + scope + " {"], plain = [];
    for (let sel of cos) if (!genericExcDomains.has(sel)) plain.push(sel);
    for (let j = 0; j < plain.length; j += 400) out.push(plain.slice(j, j + 400).join(",") + "{display:none!important}");
    out.push("}");
    for (let [sel, doms] of genericExcDomains) {
      let ex = doms.map(escapeRe).concat(generichide).join("|");
      out.push('@-moz-document regexp("^https?://(?!([^/]*\\\\.)?(' + ex + ')[:/]).*") {');
      out.push(sel + "{display:none!important}");
      out.push("}");
    }
    let css = out.join("\n") + "\n";
    let siteByDomain = {};
    for (let [k, set] of siteGroups) {
      if (!set.size) continue;
      let sels = Array.from(set);
      for (let d of k.split(",")) { (siteByDomain[d] = siteByDomain[d] || []).push(...sels); }
    }
    counts.ms = Date.now() - started;
    onDone({hosts: hosts, tp: tp, excHosts: excHosts, typed: typed, paths: paths, excRules: excRules,
            css: css, site: siteByDomain, counts: counts});
  }
  setTimeout(slice, 0);
}
// ── HostSet: ~100,000 hostnames as ONE string and two typed arrays ──────────────────────────
// A JS Set of 100,000 strings is 100,000 GC things: every full collection marks them all, and on
// the device that made the first full GC 3.9 s instead of 1.1 s (the GC log, 2026-10-15). Here
// the names live in one "\n"-joined string (one GC thing); a hash table of 32-bit FNV-1a hashes
// (Uint32Array) and start offsets (Int32Array) finds a name without allocating: the probe
// compares in place with startsWith and the terminator. Memory: the string plus 8 bytes per
// slot at a load factor under 0.5.
function fnv1a(str) {
  let h = 0x811c9dc5;
  for (let i = 0; i < str.length; i++) { h ^= str.charCodeAt(i); h = Math.imul(h, 0x01000193) >>> 0; }
  return h >>> 0;
}
function HostSet(blob) {
  this.blob = blob || "";
  this.size = 0;
  let n = 0;
  for (let i = 0; i < this.blob.length; i++) if (this.blob.charCodeAt(i) === 10) n++;
  if (this.blob.length && this.blob[this.blob.length - 1] !== "\n") n++;
  let cap = 1; while (cap < n * 2 + 8) cap <<= 1;
  this.mask = cap - 1;
  this.hashes = new Uint32Array(cap);
  this.offsets = new Int32Array(cap).fill(-1);
  let start = 0, len = this.blob.length;
  for (let i = 0; i <= len; i++) {
    if (i === len || this.blob.charCodeAt(i) === 10) {
      if (i > start) {
        let name = this.blob.slice(start, i), h = fnv1a(name), slot = h & this.mask;
        while (this.offsets[slot] !== -1) slot = (slot + 1) & this.mask;
        this.hashes[slot] = h; this.offsets[slot] = start; this.size++;
      }
      start = i + 1;
    }
  }
}
HostSet.prototype.has = function (name) {
  if (!this.size) return false;
  let h = fnv1a(name), slot = h & this.mask, off;
  while ((off = this.offsets[slot]) !== -1) {
    if (this.hashes[slot] === h && this.blob.startsWith(name, off)) {
      let end = off + name.length;
      if (end === this.blob.length || this.blob.charCodeAt(end) === 10) return true;
    }
    slot = (slot + 1) & this.mask;
  }
  return false;
};

function mapToObj(m) { let o = {}; for (let [k, v] of m) o[k] = v; return o; }
function objToMap(o) { let m = new Map(); for (let k in o) m.set(k, o[k]); return m; }
// The cache: the three host blobs as their own files, read straight into the strings the HostSets
// keep (a 3 MB JSON of everything cost a 3 MB string, its parse and what the allocator kept of
// both: +13 MB resident on the device, 2026-10-15), and the small maps, the CSS and the counts in
// meta.json.
function writeCache(r, versions) {
  writeFileText(profileFile("hosts.txt"), Array.from(r.hosts).join("\n"));
  writeFileText(profileFile("tp.txt"), Array.from(r.tp).join("\n"));
  writeFileText(profileFile("exc.txt"), Array.from(r.excHosts).join("\n"));
  writeFileText(profileFile("cosmetic.css"), r.css);
  writeFileText(profileFile("meta.json"), JSON.stringify({compiler: COMPILER_VERSION, versions: versions,
    typed: mapToObj(r.typed), paths: mapToObj(r.paths), excRules: mapToObj(r.excRules), site: r.site, counts: r.counts}));
}
// null when there is no cache for these versions and this compiler
function readCache(versions) {
  let mf = profileFile("meta.json");
  if (!mf.exists()) return null;
  let c = JSON.parse(readFileText(mf));
  if (c.compiler !== COMPILER_VERSION || c.versions.easylist !== versions.easylist || c.versions.easyprivacy !== versions.easyprivacy) return null;
  for (let n of ["hosts.txt", "tp.txt", "exc.txt", "cosmetic.css"]) if (!profileFile(n).exists()) return null;
  return {hosts: new HostSet(readFileText(profileFile("hosts.txt"))), tp: new HostSet(readFileText(profileFile("tp.txt"))),
          excHosts: new HostSet(readFileText(profileFile("exc.txt"))),
          typed: objToMap(c.typed || {}), paths: objToMap(c.paths || {}), excRules: objToMap(c.excRules || {}),
          css: null, site: c.site || {}, counts: c.counts};
}

function JihadContentBlocker() {
  this.wrappedJSObject = this;
  this._enabled = boolPref(PREF_ENABLED, true);
  this._cosmetic = boolPref(PREF_COSMETIC, true);
  this._log = boolPref(PREF_LOG, false);
  this._hosts = null; this._tp = null; this._excHosts = null;
  this._typed = new Map(); this._paths = new Map(); this._excRules = new Map(); this._site = {};
  this._versions = {easylist: "", easyprivacy: ""};
  this._counts = null;
  this._count = 0;                     // refusals this session
  this._sheetURI = null;
  this._updating = false;
  this._lastError = "";
  this._lastDocAt = 0;
  this._noLists = false;
  try { Services.prefs.addObserver("jihad.blocking.", this, false); } catch (e) {}
  try { Services.obs.addObserver(this, "document-element-inserted", false); } catch (e) {}
  try { Services.obs.addObserver(this, "http-on-modify-request", false); } catch (e) {}
  // ON DEMAND (R6, 2026-10-15): every variant's daemon runs from boot, and three of them holding
  // the lists cost ~80 MB of a 1 GB device that then refused new cards. The daemon says when a
  // card connects and when the last one leaves (jihad-client-connected / -disconnected, the
  // data the count of clients); the lists load on the first card and are dropped after ten idle
  // minutes. The first page after a long idle gets its first seconds without blocking.
  try { Services.obs.addObserver(this, "jihad-client-connected", false); } catch (e) {}
  try { Services.obs.addObserver(this, "jihad-client-disconnected", false); } catch (e) {}
  this._clients = 0; this._unloadTimer = null; this._loading = false;
  this._timer = setInterval(() => this._maybeUpdate(false), UPDATE_CHECK_MS);
  setTimeout(() => this._maybeUpdate(false), (90 + Math.floor(Math.random() * 120)) * 1000);
}

JihadContentBlocker.prototype = {
  classID: Components.ID("{3c7e1b9a-5d24-4f6e-8a0b-9e2d4c6f1a37}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIContentPolicy, Ci.nsIObserver, Ci.nsISupportsWeakReference]),

  // ── nsIObserver ──
  observe: function (subject, topic, data) {
    if (topic === "jihad-client-connected") {
      this._clients = parseInt(data, 10) || (this._clients + 1);
      if (this._unloadTimer) { clearTimeout(this._unloadTimer); this._unloadTimer = null; }
      if (!this._hosts && !this._loading) { this._loading = true; this._load(); }
      return;
    }
    if (topic === "jihad-client-disconnected") {
      this._clients = parseInt(data, 10); if (isNaN(this._clients)) this._clients = Math.max(0, this._clients - 1);
      if (this._clients === 0) {
        if (!this._unloadTimer) this._unloadTimer = setTimeout(() => { this._unloadTimer = null; if (this._clients === 0) this._unload(); }, UNLOAD_AFTER_IDLE_MS);
        // The last card gone (R6, 2026-10-16): after three heavy pages the daemon kept ~190 MB with
        // nothing open, the daemon's purge notwithstanding. A closed window's compartment is freed
        // only after a cycle collection, a GC and a cycle collection again; done here from chrome
        // once the card's teardown has settled, with the compartment count logged so a zombie
        // window shows as "user compartments > 0" with no card.
        setTimeout(() => this._collectAfterLastCard(), 3000);
      }
      return;
    }
    if (topic === "document-element-inserted") { this._lastDocAt = Date.now(); this._siteSheetFor(subject); return; }
    if (topic === "http-on-modify-request") { this._onRequest(subject); return; }
    if (topic !== "nsPref:changed") return;
    if (data === PREF_ENABLED) { this._enabled = boolPref(PREF_ENABLED, true); log("blocking " + (this._enabled ? "on" : "off")); this._applySheet(); }
    else if (data === PREF_COSMETIC) { this._cosmetic = boolPref(PREF_COSMETIC, true); this._applySheet(); }
    else if (data === PREF_LOG) { this._log = boolPref(PREF_LOG, false); }
  },

  // ── nsIContentPolicy ──
  shouldLoad: function (type, location, origin, context, mime, extra, principal) {
    if (!this._enabled || !this._hosts || NEVER_TYPES[type]) return Ci.nsIContentPolicy.ACCEPT;
    let page = null, self = this;
    let pageHost = () => { if (page === null) page = self._pageHost(context, origin); return page; };
    return this._decide(type, location, pageHost) ? Ci.nsIContentPolicy.REJECT_REQUEST : Ci.nsIContentPolicy.ACCEPT;
  },
  shouldProcess: function () { return Ci.nsIContentPolicy.ACCEPT; },

  // A redirected channel: the policy saw the first address only (inspector F-18).
  _onRequest: function (subject) {
    if (!this._enabled || !this._hosts) return;
    try {
      let ch = subject.QueryInterface(Ci.nsIHttpChannel);
      if (!(ch.loadFlags & Ci.nsIChannel.LOAD_REPLACE)) return;
      let li = ch.loadInfo;
      if (!li) return;
      let type = li.externalContentPolicyType;
      if (NEVER_TYPES[type]) return;
      let self = this, page = null;
      let pageHost = () => {
        if (page === null) {
          page = "";
          try { let d = li.loadingDocument; if (d && d.defaultView && d.defaultView.top) d = d.defaultView.top.document; if (d && d.documentURIObject) page = d.documentURIObject.asciiHost.toLowerCase(); } catch (e) {}
          if (!page) { try { page = li.loadingPrincipal.URI.asciiHost.toLowerCase(); } catch (e) {} }
        }
        return page;
      };
      if (this._decide(type, ch.URI, pageHost)) ch.cancel(Cr.NS_BINDING_ABORTED);
    } catch (e) {}
  },

  // true = refuse. pageHost is a function (computed only when a rule needs it, inspector F-17).
  _decide: function (type, location, pageHost) {
    let scheme;
    try { scheme = location.scheme; } catch (e) { return false; }
    if (scheme !== "http" && scheme !== "https" && scheme !== "ws" && scheme !== "wss") return false;
    let host;
    try { host = location.asciiHost.toLowerCase(); } catch (e) { return false; }
    if (!host) return false;
    if (host[host.length - 1] === ".") host = host.slice(0, -1);
    let bit = TYPE_BIT_OF[type] || 0, path = null, thirdParty = null, self = this;
    let getPath = () => { if (path === null) { try { path = location.path.toLowerCase(); } catch (e) { path = ""; } } return path; };
    let isTP = () => { if (thirdParty === null) thirdParty = self._isThirdParty(host, pageHost()); return thirdParty; };
    let partyOk = (p) => p === 0 || (p === 1 ? isTP() : !isTP());
    // 1. exceptions, at every level, first (inspector F-2, F-9)
    for (let h = host; ;) {
      if (this._excHosts.has(h)) return false;
      let ex = this._excRules.get(h);
      if (ex) for (let r of ex) {
        if (r.p && getPath().indexOf(r.p) !== 0) continue;
        if (r.types && !(r.types & bit)) continue;
        if (!partyOk(r.party)) continue;
        if (r.domains && !domainMatches(r.domains, pageHost())) continue;
        if (r.notDomains && domainMatches(r.notDomains, pageHost())) continue;
        return false;
      }
      let dot = h.indexOf("."); if (dot < 0) break; h = h.slice(dot + 1); if (h.indexOf(".") < 0) break;
    }
    // 2. path rules (they apply on the page's own host too: MDN's /pong/ is first-party)
    for (let h = host; ;) {
      let ps = this._paths.get(h);
      if (ps) for (let r of ps) { if (getPath().indexOf(r.p) === 0 && partyOk(r.party)) return this._refused("path " + h + r.p, type); }
      let dot = h.indexOf("."); if (dot < 0) break; h = h.slice(dot + 1); if (h.indexOf(".") < 0) break;
    }
    // 3. host rules: never for the page's own host
    if (pageHost() === host) return false;
    for (let h = host; ;) {
      if (this._hosts.has(h)) return this._refused(h, type);
      if (this._tp.has(h) && isTP()) return this._refused(h + "$third-party", type);
      let ts = this._typed.get(h);
      if (ts) for (let r of ts) { if ((r.types === 0 || (r.types & bit)) && partyOk(r.party)) return this._refused(h + "$typed", type); }
      let dot = h.indexOf("."); if (dot < 0) break; h = h.slice(dot + 1); if (h.indexOf(".") < 0) break;
    }
    return false;
  },
  _refused: function (rule, type) {
    this._count++;
    if (this._log) log("blocked " + (TYPE_NAMES[type] || type) + " by " + rule);
    return true;
  },
  _baseDomain: function (host) { try { return Services.eTLD.getBaseDomainFromHost(host, 0); } catch (e) { return host; } },
  _isThirdParty: function (host, pageHost) { return !pageHost || this._baseDomain(host) !== this._baseDomain(pageHost); },
  _pageHost: function (context, origin) {
    let doc = null;
    try {
      if (context) {
        if (context instanceof Ci.nsIDOMDocument) doc = context;
        else if (context instanceof Ci.nsIDOMNode) doc = context.ownerDocument;
        else if (context instanceof Ci.nsIDOMWindow) doc = context.document;
      }
      if (doc && doc.defaultView && doc.defaultView.top && doc.defaultView.top.document) doc = doc.defaultView.top.document;
      if (doc && doc.documentURIObject) { let h = doc.documentURIObject.asciiHost; if (h) return h.toLowerCase(); }
    } catch (e) {}
    try { if (origin) { let h = origin.asciiHost; if (h) return h.toLowerCase(); } } catch (e) {}
    return "";
  },

  // ── loading: the cache, or a compile of the raw lists ──
  // Whichever copy is newer: the bundled snapshot (a package upgrade may carry a newer one) or
  // the profile's download (inspector F-14).
  _rawFile: function (name) {
    let b = bundledFile(name), p = profileFile(name + ".txt");
    let bv = b.exists() ? versionOfFile(b) : "", pv = (p.exists() && p.fileSize > 1000) ? versionOfFile(p) : "";
    if (pv && pv >= bv) return p;
    return b.exists() ? b : (p.exists() ? p : b);
  },
  _load: function () {
    try {
      let files = {}, versions = {};
      for (let l of LISTS) {
        let f = this._rawFile(l.name);
        if (!f.exists()) { log("no list file: " + f.path); continue; }
        files[l.name] = f; versions[l.name] = versionOfFile(f);
      }
      if (!files.easylist && !files.easyprivacy) { this._noLists = true; this._loading = false; log("no lists: nothing is blocked"); return; }
      try {
        let c = readCache(versions);
        if (c) { this._loading = false; this._install(c, versions, "cache"); return; }
      } catch (e) { log("cache unreadable, recompiling: " + e); }
      let texts = [files.easylist ? readFileText(files.easylist) : "", files.easyprivacy ? readFileText(files.easyprivacy) : ""];
      compile(texts, (r) => this._afterCompile(r, versions, "compiled"));
    } catch (e) { this._loading = false; log("load failed: " + e); }
  },
  // Written out, then installed from the cache's OWN files, so the sets hold fresh strings and the
  // raw list text can be collected (SpiderMonkey keeps split() pieces as dependent strings,
  // inspector F-5).
  _afterCompile: function (r, versions, how) {
    try { writeCache(r, versions); } catch (e) { log("cache not written: " + e); }
    let c = null;
    try { c = readCache(versions); } catch (e) { log("cache unreadable after writing: " + e); }
    if (!c) {   // keep working from the compile's own sets, as a last resort
      c = {hosts: new HostSet(Array.from(r.hosts).join("\n")), tp: new HostSet(Array.from(r.tp).join("\n")), excHosts: new HostSet(Array.from(r.excHosts).join("\n")),
           typed: r.typed, paths: r.paths, excRules: r.excRules, css: null, site: r.site, counts: r.counts};
    }
    r = null;
    this._loading = false;
    this._install(c, versions, how);
  },
  _collectAfterLastCard: function () {
    if (this._clients !== 0) return;
    try { Cu.forceCC(); Cu.forceShrinkingGC(); Cu.forceCC(); Cu.forceShrinkingGC(); } catch (e) { log("collect: " + e); }
    try { Services.obs.notifyObservers(null, "jihad-blocker-installed", null); } catch (e) {}   // the daemon trims again
    setTimeout(() => {
      try {
        let mr = Cc["@mozilla.org/memory-reporter-manager;1"].getService(Ci.nsIMemoryReporterManager);
        let fig = (name) => { try { return (mr[name] / 1048576).toFixed(1) + " MB"; } catch (e) { return "n/a"; } };
        let comps = "n/a"; try { comps = mr.JSMainRuntimeCompartmentsSystem + "+" + mr.JSMainRuntimeCompartmentsUser; } catch (e) {}
        let ghosts = "n/a"; try { ghosts = String(mr.ghostWindows); } catch (e) {}
        log("after the last card: resident " + fig("residentFast") + ", JS GC heap " + fig("JSMainRuntimeGCHeap") + ", compartments " + comps + " (system+user; user > 0 with no card = a zombie window), ghost windows " + ghosts);
        // The full report, for the investigation of what holds a closed window (R6, 2026-10-16):
        // <profile>/jihad-blocklists/memory-last-card.json.gz, as about:memory would show it.
        if (boolPref("jihad.blocking.memoryDump", false)) {
          let f = profileFile("memory-last-card.json.gz");
          let dumper = Cc["@mozilla.org/memory-info-dumper;1"].getService(Ci.nsIMemoryInfoDumper);
          dumper.dumpMemoryReportsToNamedFile(f.path, () => log("memory report written: " + f.path), null, false);
        }
      } catch (e) { log("memory reporter: " + e); }
    }, 2000);
  },
  _unload: function () {
    if (!this._hosts) return;
    this._hosts = null; this._tp = null; this._excHosts = null;
    this._typed = new Map(); this._paths = new Map(); this._excRules = new Map(); this._site = {};
    this._applySheet();
    log("idle ten minutes with no card: the lists dropped (they load again with the next card)");
    try { Services.obs.notifyObservers(null, "jihad-blocker-installed", null); } catch (e) {}   // the daemon trims
    setTimeout(() => { try { Services.obs.notifyObservers(null, "memory-pressure", "heap-minimize"); } catch (e) {} }, 500);
  },
  _install: function (r, versions, how) {
    this._hosts = r.hosts; this._tp = r.tp; this._excHosts = r.excHosts; this._typed = r.typed; this._paths = r.paths;
    this._excRules = r.excRules; this._site = r.site; this._counts = r.counts;
    this._versions = versions;
    try { Services.prefs.setCharPref(PREF_VERSIONS, (versions.easylist || "-") + "/" + (versions.easyprivacy || "-")); } catch (e) {}
    if (intPref(PREF_LAST, 0) === 0) setIntPref(PREF_LAST, versionToSeconds(versions.easylist) || now());
    let c = r.counts;
    log(how + ": EasyList " + versions.easylist + ", EasyPrivacy " + versions.easyprivacy + " - hosts " + r.hosts.size +
        ", third-party " + r.tp.size + ", typed " + r.typed.size + ", path " + c.path + ", exceptions " + c.exc + "+" + c.excTyped +
        ", generichide " + c.generichide + ", selectors " + c.cosmetic + " (less " + c.cosmeticExc + "), site selectors " + c.siteCosmetic +
        " (less " + c.siteCosmeticExc + ", per document), bad selectors " + c.badSelector + ", skipped " + c.skipped +
        (c.ms !== undefined ? ", " + c.ms + " ms" : ""));
    // the generic sheet is the cache's cosmetic.css, registered as it is
    this._applySheet();
    // The parse of a 3 MB cache (or a compile) leaves the JS heap and the allocator holding what
    // it no longer needs; on a 1 GB tablet that showed as +36 MB of RSS at idle (device
    // 2026-10-15, the stylesheet itself cost nothing). Ask for a collection and a purge once the
    // sets are in place: at start nothing is cached yet, after an update (every 4 days) the
    // engine's caches refill as the user browses.
    setTimeout(() => {
      try { Services.obs.notifyObservers(null, "memory-pressure", "heap-minimize"); } catch (e) {}
      try { Services.obs.notifyObservers(null, "jihad-blocker-installed", null); } catch (e) {}   // the daemon: malloc_trim
      // Where the memory sits (R6): the JS GC heap and the allocator's heap, in MB, after the purge.
      setTimeout(() => {
        try {
          let mr = Cc["@mozilla.org/memory-reporter-manager;1"].getService(Ci.nsIMemoryReporterManager);
          // each figure on its own: heapAllocated is jemalloc's and this build has none (NS_ERROR_NOT_AVAILABLE)
          let fig = (name) => { try { return (mr[name] / 1048576).toFixed(1) + " MB"; } catch (e) { return "n/a"; } };
          log("memory after install: resident " + fig("residentFast") + ", heap-allocated " + fig("heapAllocated") +
              ", JS GC heap " + fig("JSMainRuntimeGCHeap") + ", JS temporary peak " + fig("JSMainRuntimeTemporaryPeak") +
              ", JS compartments " + (function () { try { return mr.JSMainRuntimeCompartmentsSystem + "+" + mr.JSMainRuntimeCompartmentsUser; } catch (e) { return "n/a"; } })());
        } catch (e) { log("memory reporter: " + e); }
      }, 3000);
    }, 2000);
  },

  // ── hiding ──
  _applySheet: function () {
    try {
      let sss = Cc["@mozilla.org/content/style-sheet-service;1"].getService(Ci.nsIStyleSheetService);
      if (this._sheetURI && sss.sheetRegistered(this._sheetURI, sss.USER_SHEET)) sss.unregisterSheet(this._sheetURI, sss.USER_SHEET);
      this._sheetURI = null;
      if (!this._cosmetic || !this._enabled || !this._hosts) return;   // the master switch gates hiding too (F-15)
      let f = profileFile("cosmetic.css");
      if (!f.exists()) return;
      this._sheetURI = Services.io.newFileURI(f);
      sss.loadAndRegisterSheet(this._sheetURI, sss.USER_SHEET);
      log("hiding on (" + (this._counts ? this._counts.cosmetic : "?") + " generic selectors)");
    } catch (e) { log("sheet failed: " + e); this._sheetURI = null; }
  },
  _siteSheetFor: function (doc) {
    try {
      if (!this._cosmetic || !this._enabled || !this._hosts) return;
      let uri = doc.documentURIObject;
      if (!uri || (uri.scheme !== "http" && uri.scheme !== "https")) return;
      let win = doc.defaultView;
      if (!win) return;
      let h = uri.asciiHost.toLowerCase(), sels = [];
      for (;;) {
        let a = this._site[h];
        if (a) sels.push(...a);
        let dot = h.indexOf("."); if (dot < 0) break; h = h.slice(dot + 1); if (h.indexOf(".") < 0) break;
      }
      if (!sels.length) return;
      let css = sels.map(sel => sel + "{display:none!important}").join("\n");
      let utils = win.QueryInterface(Ci.nsIInterfaceRequestor).getInterface(Ci.nsIDOMWindowUtils);
      utils.loadSheetUsingURIString("data:text/css;charset=utf-8," + encodeURIComponent(css), utils.USER_SHEET);
    } catch (e) { log("site sheet: " + e); }
  },

  // ── updates ──
  _maybeUpdate: function (force) {
    if (this._updating || !this._hosts) return;
    let t = now(), last = intPref(PREF_LAST, 0), attempt = intPref(PREF_ATTEMPT, 0);
    if (last > t) { last = t; setIntPref(PREF_LAST, t); }                 // a clock corrected backwards
    if (!force) {
      if (t - last < UPDATE_EVERY_S) return;
      if (t - attempt < RETRY_AFTER_S) return;                             // a failed try: a day's rest
      if (Date.now() - this._lastDocAt < QUIET_AFTER_DOC_MS) return;        // not while a page starts (R5)
    }
    this._updating = true; this._lastError = "";
    setIntPref(PREF_ATTEMPT, t);
    let got = {}, pending = LISTS.length, self = this;
    function done() {
      try {
        let newer = LISTS.filter(l => got[l.name] && versionOf(got[l.name]) > (self._versions[l.name] || ""));
        if (!newer.length) {
          if (!self._lastError) { log("update: nothing newer than " + self._versions.easylist + "/" + self._versions.easyprivacy); setIntPref(PREF_LAST, t); }
          self._updating = false; return;
        }
        let texts = {}, versions = {};
        for (let l of LISTS) {
          let t2 = (got[l.name] && versionOf(got[l.name]) > (self._versions[l.name] || "")) ? got[l.name] : readFileText(self._rawFile(l.name));
          texts[l.name] = t2; versions[l.name] = versionOf(t2);
        }
        compile([texts.easylist, texts.easyprivacy], (r) => {
          try {
            for (let l of newer) { try { writeFileText(profileFile(l.name + ".txt"), got[l.name]); } catch (e) { log("update: " + l.name + " not kept: " + e); } }
            self._afterCompile(r, versions, "update");
            if (!self._lastError) setIntPref(PREF_LAST, t);
          } catch (e) { log("update: " + e); }
          self._updating = false;
        });
      } catch (e) { log("update: " + e); self._updating = false; }
    }
    for (let l of LISTS) {
      try {
        let xhr = Cc["@mozilla.org/xmlextras/xmlhttprequest;1"].createInstance(Ci.nsIXMLHttpRequest);
        xhr.mozBackgroundRequest = true;                                      // no auth or cert prompts
        xhr.open("GET", l.url, true);
        xhr.responseType = "text";
        xhr.timeout = 120000;
        xhr.channel.loadFlags |= Ci.nsIRequest.LOAD_BYPASS_CACHE | Ci.nsIRequest.LOAD_ANONYMOUS | Ci.nsIRequest.INHIBIT_CACHING;
        xhr.onload = () => {
          let txt = xhr.responseText || "";
          let ok = xhr.status === 200 && /^\[Adblock/.test(txt.slice(0, 20)) && versionOf(txt);
          // a truncated download has far fewer rules than what it would replace
          if (ok) { let n = (txt.match(/\n/g) || []).length, cur = self._counts ? self._counts.rules : 0; if (cur && n < cur * 0.5) { ok = false; self._lastError = l.name + ": truncated (" + n + " lines)"; } }
          if (ok) got[l.name] = txt;
          else { if (!self._lastError) self._lastError = l.name + ": HTTP " + xhr.status; log("update: " + self._lastError); }
          if (--pending === 0) done();
        };
        xhr.onerror = xhr.ontimeout = () => { self._lastError = l.name + ": no download"; log("update: " + self._lastError); if (--pending === 0) done(); };
        xhr.send(null);
      } catch (e) { self._lastError = l.name + ": " + e; log("update: " + self._lastError); if (--pending === 0) done(); }
    }
  },

  // ── for the settings page (wrappedJSObject) ──
  update: function () { this._maybeUpdate(true); },
  state: function () {
    return {enabled: this._enabled, cosmetic: this._cosmetic, loaded: !!this._hosts, noLists: this._noLists, versions: this._versions,
            lastUpdate: intPref(PREF_LAST, 0), count: this._count, counts: this._counts, updating: this._updating, error: this._lastError};
  }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadContentBlocker]);
