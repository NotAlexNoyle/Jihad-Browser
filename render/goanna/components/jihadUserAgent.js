/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — the browser's identity, adjusted to the needs of the page (cavekit-modern-web.md
 * R3; the owner, 2026-10-16: "the user agent should be 52.9 by default but adjust based on the
 * needs of the page").
 *
 * THE DEFAULT is the engine's own identity (JihadUserAgent.h: Goanna, UXP, Firefox/52.9, the
 * browser's name). THE ADJUSTMENT has two parts:
 *   1. The daemon's hand-curated table (render/goanna/EngineHost.cpp, ported from Basilisk's):
 *      sites that serve a working page only to a newer Firefox token — the header only.
 *   1b. This component's own RULES (jihad.ua.sites, "site=identity;…"), header and script
 *      together: youtube.com gets a mobile Firefox identity, so YouTube serves its mobile site —
 *      measured 2026-10-16 (T-497): the desktop watch page hands the engine 13.7 MB of script and
 *      stalls the main thread for 24 s, still grey after a minute; the mobile site 3.0 MB, a 5 s
 *      worst stall, the home feed rendered in 18 s (the owner: "speed up youtube").
 *   2. This component's LEARNED list: a site whose bot check turned the page away — Cloudflare marks
 *      such responses "cf-mitigated: challenge" — goes onto jihad.ua.modernHosts (its base domain,
 *      persisted), and the page is loaded again, once, with a current Firefox ESR identity. From
 *      then on every request to that site, and every document of it, carries that identity: the
 *      header (http-on-modify-request, after the daemon's observer, so this wins) AND the docShell's
 *      customUserAgent (document-element-inserted; frames inherit), so header and script agree —
 *      measured 2026-10-16: with the two apart the challenge failed; with the two together w3.org,
 *      Britannica, npm and Medium pass Cloudflare's check (Medium on the learned retry).
 * A site that still challenges after the retry is left alone (no loop); the user can forget the
 * learned list in Settings → General. jihad.ua.learn turns the learning off.
 * THE SIGNATURES of a check that turned the page away (_challengeOf): Cloudflare's documented
 * "cf-mitigated: challenge" header, and Startpage's own gate — the page answered with a redirect to
 * its /sp/captcha-block page (measured from the host 2026-10-16: the 52.9 token gets the redirect
 * on every page, a current token gets the page; the search then meets Anubis's proof of work,
 * which is the same for every browser and no identity's business).
 * Tested without the real sites by render/goanna/test/jihad-identity-server.py.
 */

"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/Timer.jsm");

const PREF_HOSTS  = "jihad.ua.modernHosts";
const PREF_LEARN  = "jihad.ua.learn";
const PREF_MODERN = "jihad.ua.modern";
const PREF_SITES  = "jihad.ua.sites";
const DEFAULT_MODERN = "Mozilla/5.0 (X11; Linux armv7l; rv:128.0) Gecko/20100101 Firefox/128.0";

function log(s) { try { Services.console.logStringMessage("identity: " + s); } catch (e) {} }
function boolPref(n, d) { try { return Services.prefs.getBoolPref(n); } catch (e) { return d; } }
function charPref(n, d) { try { return Services.prefs.getCharPref(n); } catch (e) { return d; } }
function windowOf(ch) {
  let cb = null;
  try { cb = ch.notificationCallbacks; } catch (e) {}
  if (!cb) { try { cb = ch.loadGroup && ch.loadGroup.notificationCallbacks; } catch (e) {} }
  if (!cb) return null;
  try { return cb.getInterface(Ci.nsILoadContext).associatedWindow; } catch (e) {}
  try { return cb.getInterface(Ci.nsIDOMWindow); } catch (e) {}
  return null;
}
function baseDomain(host) { try { return Services.eTLD.getBaseDomainFromHost(host, 0); } catch (e) { return host; } }
// "site=identity;site=identity" → Map; a site is a base domain, matched with its subdomains
function parseSites(s) {
  let m = new Map();
  for (let rule of (s || "").split(";")) {
    let i = rule.indexOf("="); if (i <= 0) continue;
    let site = rule.slice(0, i).trim().toLowerCase(), ua = rule.slice(i + 1).trim();
    if (site && ua) m.set(site, ua);
  }
  return m;
}

function JihadUserAgent() {
  this.wrappedJSObject = this;
  this._hosts = new Set(charPref(PREF_HOSTS, "").split(",").map(x => x.trim()).filter(Boolean));
  this._learn = boolPref(PREF_LEARN, true);
  this._modern = charPref(PREF_MODERN, DEFAULT_MODERN) || DEFAULT_MODERN;
  this._sites = parseSites(charPref(PREF_SITES, ""));
  this._retried = new Map();          // base domain → when the one retry was made (ms)
  this._learnedThisSession = 0;
  try { Services.prefs.addObserver("jihad.ua.", this, false); } catch (e) {}
  try { Services.obs.addObserver(this, "http-on-modify-request", false); } catch (e) {}
  try { Services.obs.addObserver(this, "http-on-examine-response", false); } catch (e) {}
  try { Services.obs.addObserver(this, "document-element-inserted", false); } catch (e) {}
  // logged a moment later: at creation the daemon's console bridge is not up yet and the line is
  // lost — and the profile's prefs.js is not read yet either (the daemon creates this component
  // before the profile is selected; the pref observer above brings the learned list in when it is)
  let self = this;
  setTimeout(function () { log("ready: " + self._hosts.size + " learned site(s), " + self._sites.size + " site rule(s)" + (self._learn ? "" : ", learning off")); }, 4000);
}

JihadUserAgent.prototype = {
  classID: Components.ID("{8d4f2c6a-1b7e-4e3a-9f5c-2a6d8e0b4c71}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIObserver, Ci.nsISupportsWeakReference]),

  observe: function (subject, topic, data) {
    if (topic === "nsPref:changed") {
      if (data === PREF_HOSTS) this._hosts = new Set(charPref(PREF_HOSTS, "").split(",").map(x => x.trim()).filter(Boolean));
      else if (data === PREF_LEARN) this._learn = boolPref(PREF_LEARN, true);
      else if (data === PREF_MODERN) this._modern = charPref(PREF_MODERN, DEFAULT_MODERN) || DEFAULT_MODERN;
      else if (data === PREF_SITES) this._sites = parseSites(charPref(PREF_SITES, ""));
      return;
    }
    if (topic === "http-on-modify-request") { this._onRequest(subject); return; }
    if (topic === "http-on-examine-response") { this._onResponse(subject); return; }
    if (topic === "document-element-inserted") { this._onDocument(subject); return; }
  },

  _isModern: function (host) {
    if (!host || !this._hosts.size) return false;
    for (let d of this._hosts) if (host === d || host.endsWith("." + d)) return true;
    return false;
  },
  // the identity a host gets: the learned current one, or a rule's, or "" for the default
  _identityFor: function (host) {
    if (!host) return "";
    if (this._isModern(host)) return this._modern;
    for (let [site, ua] of this._sites) if (host === site || host.endsWith("." + site)) return ua;
    return "";
  },
  // the page's host for a channel: the top document's, so a learned site's frames and
  // subresources carry the identity too
  _pageHostOf: function (ch) {
    try {
      let li = ch.loadInfo, d = li && li.loadingDocument;
      if (d && d.defaultView && d.defaultView.top) return d.defaultView.top.document.documentURIObject.asciiHost.toLowerCase();
    } catch (e) {}
    return "";
  },

  _onRequest: function (subject) {
    if (!this._hosts.size && !this._sites.size) return;
    try {
      let ch = subject.QueryInterface(Ci.nsIHttpChannel);
      let host = ch.URI.asciiHost.toLowerCase();
      let ua = this._identityFor(host) || this._identityFor(this._pageHostOf(ch));
      if (ua) ch.setRequestHeader("User-Agent", ua, false);
    } catch (e) {}
  },

  // What a bot check's answer looks like, or "" for an ordinary response (the header comment lists
  // the signatures and where each was measured)
  _challengeOf: function (ch) {
    try { if (ch.getResponseHeader("cf-mitigated").toLowerCase() === "challenge") return "cf-mitigated: challenge"; } catch (e) {}
    try {
      let status = ch.responseStatus;
      if (status >= 300 && status < 400) {
        let path = ch.getResponseHeader("Location").replace(/^[a-z][a-z0-9+.-]*:\/\/[^\/]*/i, "");
        if (path.indexOf("/sp/captcha-block") === 0) return "a redirect to /sp/captcha-block";
      }
    } catch (e) {}
    return "";
  },

  // A top-level document answered with a bot challenge: learn the site, pin its docShell, load
  // it again once.
  _onResponse: function (subject) {
    if (!this._learn) return;
    try {
      let ch = subject.QueryInterface(Ci.nsIHttpChannel);
      let li = ch.loadInfo;
      if (!li || li.externalContentPolicyType !== Ci.nsIContentPolicy.TYPE_DOCUMENT) return;
      let why = this._challengeOf(ch);
      if (!why) return;
      let host = ch.URI.asciiHost.toLowerCase(), base = baseDomain(host);
      let now = Date.now();
      if (this._retried.has(base) && now - this._retried.get(base) < 10 * 60 * 1000) { log(host + ": challenged again with the current identity; the page is left to the site"); return; }
      this._retried.set(base, now);
      if (!this._hosts.has(base)) {
        this._hosts.add(base); this._learnedThisSession++;
        try { Services.prefs.setCharPref(PREF_HOSTS, Array.from(this._hosts).join(",")); Services.prefs.savePrefFile(null); } catch (e) {}
        log(host + ": the site's bot check turned the page away (" + why + "); learned " + base + ", loading again with a current Firefox identity");
      } else {
        log(host + ": challenged on a learned site; loading again once");
      }
      // The retry: the challenge page is not shown at all — the channel is cancelled as a user's
      // stop is — the docShell pinned so navigator.userAgent agrees with the header, the page loaded
      // again. A document load has no loadingDocument; its window comes from the channel's callbacks.
      let win = windowOf(ch);
      if (!win) { log("no window to retry " + host + "; the challenge page is left as it is"); return; }
      let self = this, url = ch.URI.spec;
      try { ch.cancel(Components.results.NS_BINDING_ABORTED); } catch (e) {}
      setTimeout(function () {
        try {
          let nav = win.QueryInterface(Ci.nsIInterfaceRequestor).getInterface(Ci.nsIWebNavigation);
          nav.QueryInterface(Ci.nsIDocShell).customUserAgent = self._modern;
          nav.loadURI(url, Ci.nsIWebNavigation.LOAD_FLAGS_BYPASS_CACHE | Ci.nsIWebNavigation.LOAD_FLAGS_REPLACE_HISTORY, null, null, null);
        } catch (e) { log("retry failed for " + host + ": " + e); }
      }, 300);
    } catch (e) {}
  },

  // every document of a learned site or a rule's site (and of its frames): the docShell pinned,
  // so navigator.userAgent is the identity the headers carry
  _onDocument: function (doc) {
    if (!this._hosts.size && !this._sites.size) return;
    try {
      let uri = doc.documentURIObject;
      if (!uri || (uri.scheme !== "http" && uri.scheme !== "https")) return;
      let win = doc.defaultView;
      if (!win) return;
      let host = uri.asciiHost.toLowerCase(), top = host;
      try { top = win.top.document.documentURIObject.asciiHost.toLowerCase(); } catch (e) {}
      let ua = this._identityFor(host) || this._identityFor(top);
      if (!ua) return;
      let ds = win.QueryInterface(Ci.nsIInterfaceRequestor).getInterface(Ci.nsIWebNavigation).QueryInterface(Ci.nsIDocShell);
      if (ds.customUserAgent !== ua) ds.customUserAgent = ua;
    } catch (e) {}
  },

  // for the settings page (wrappedJSObject)
  state: function () { return {hosts: Array.from(this._hosts), learn: this._learn, modern: this._modern, learnedThisSession: this._learnedThisSession, sites: Array.from(this._sites.keys())}; },
  forget: function () {
    this._hosts = new Set(); this._retried = new Map();
    try { Services.prefs.setCharPref(PREF_HOSTS, ""); Services.prefs.savePrefFile(null); } catch (e) {}
    log("the learned sites forgotten");
  }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadUserAgent]);
