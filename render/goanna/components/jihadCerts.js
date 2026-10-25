/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — the certificates Jihad trusts beyond its own (cavekit-proxies-certs.md).
 *
 * Jihad does its own TLS with the engine's NSS and its own roots. This component adds three
 * things to that, each in memory or by the user's own act:
 *
 *  R4  webOS's certificates. At the first page (when NSS is up — the daemon raises
 *      "jihad-nss-ready"), webOS's stores are read and every CA certificate in them that HP's
 *      3.0.5 image did not carry, that Mozilla has not removed, that is not Jihad's already and
 *      has not expired, is trusted to identify websites. Those are the roots a community update
 *      (or webOS CE) brought, and the CAs a user trusted in Certificate Manager. They are
 *      NSS temporary certificates: nothing is written to the profile, the stores are read again
 *      at every start, and the switch "jihad.security.webos_certs" takes the trust away at once.
 *      Only roots are taken (rootShape): never a site's own certificate, which Trust Always puts
 *      in the same store, and never one Jihad's own SSL dialog put there.
 *
 *  R2  The Riverstone Relay's CA, carried here and pinned by SHA-256. Trusted, the same way, only
 *      while "jihad.proxy.relay.trust_ca" is on AND the Relay is the proxy in use. Found in a
 *      webOS store or offered by a link, it is never taken by those routes.
 *
 *  R6  "@mozilla.org/nsCertificateDialogs;1": opening a CA certificate link asks on the card
 *      whether to trust it to identify websites. The engine's own dialog is a XUL window that a
 *      headless daemon cannot open, so the question went nowhere and the import failed. A Yes
 *      puts it in the profile's certificate database (the engine does that); Settings lists
 *      those and removes them.
 *
 * The lists R4 checks against are certs/jihad-cert-lists.json in the bundle, made by
 * render/goanna/certs/make-cert-lists.py from HP's image and Mozilla's CCADB report.
 *
 * Settings reads the state through wrappedJSObject: report(), addedCAs(), removeAdded().
 */

"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");

const PREF_WEBOS = "jihad.security.webos_certs";
// The SHA-256s of the sites' own certificates Jihad's SSL dialog put into webOS's store (Trust
// Always and Trust Once install there too): R4 never takes one. Kept to the last MAX_ACCEPTED.
const PREF_ACCEPTED = "jihad.security.webos_certs.accepted";
const MAX_ACCEPTED = 200;
const PREF_RELAY_CA = "jihad.proxy.relay.trust_ca";
const RELAY_HOST = "proxy.riverstonerelay.org";
// The Riverstone Relay's CA (certs/riverstone-relay-ca.pem), pinned HERE, not only in the lists, so
// a missing or altered list cannot let it in by another route (the inspector, 2026-10-13); its key
// too. make-cert-lists.py refuses to write a list whose Relay certificate is not this one.
const RELAY_SHA256 = "b48f324daca7da1dd3f8b737834a6a5d66c61aabd1a0a55bd36e8cfa27fb03bb";
const RELAY_SPKI = "Myh8cRV4VewPyetVq2qa2icpMz4/kBLHBb0/LFI4YkE=";
// Every squid-for-webos install makes its CA with this subject; a re-keyed Relay CA would have it
// too. R4, which trusts without asking, never takes one; the R6 question says what it is.
const SQUID_WEBOS_ORG = "squid-sslbump-for-webos";

// webOS's stores, in the order they are read (a certificate seen twice counts once).
//   /etc/ssl/certs/trustedcerts  the system root list: HP's, and what an update put there
//   /var/ssl/trustedcerts        what webOS's OpenSSL trusts: links to the list above, and links
//                                to the certificates trusted in Certificate Manager
//   ca-certificates.crt          the bundle (some updates refresh only this)
//   /usr/lib/ssl/cert.pem        where the community OpenSSL 1.1.1 looks, if anything put one there
const STORES = [
  {path: "/etc/ssl/certs/trustedcerts", dir: true, from: "webOS's root list", only: /\.pem$/i},
  {path: "/var/ssl/trustedcerts", dir: true, from: "Certificate Manager", links: true},
  {path: "/etc/ssl/certs/ca-certificates.crt", from: "webOS's certificate bundle"},
  {path: "/usr/lib/ssl/cert.pem", from: "the community OpenSSL bundle"}
];
const MAX_FILE = 4 * 1024 * 1024;
const PEM_RE = /-----BEGIN CERTIFICATE-----([\s\S]+?)-----END CERTIFICATE-----/g;

function log(s) {
  try { Services.console.logStringMessage("certs: " + s); } catch (e) {}
}

function certDB() {
  return Cc["@mozilla.org/security/x509certdb;1"].getService(Ci.nsIX509CertDB);
}

function boolPref(name, dflt) {
  try { return Services.prefs.getBoolPref(name); } catch (e) { return dflt; }
}
function charPref(name) {
  try { return Services.prefs.getCharPref(name); } catch (e) { return ""; }
}
function intPref(name) {
  try { return Services.prefs.getIntPref(name); } catch (e) { return 0; }
}

// "AB:CD:…" (nsIX509Cert's form) or "abcd…" -> "abcd…".
function normFp(fp) {
  return String(fp || "").replace(/:/g, "").toLowerCase();
}
// "abcd…" -> "AB:CD:…", as Settings shows it.
function showFp(hex) {
  return String(hex).toUpperCase().replace(/(..)(?!$)/g, "$1:");
}
// The same in groups of eight bytes, so a card's dialog can wrap it (one 95-character run
// went off the dialog's edge on the device).
function showFpGrouped(hex) {
  return showFp(hex).replace(/((?:[0-9A-F]{2}:){7}[0-9A-F]{2}):/g, "$1 ");
}

// --- reading -------------------------------------------------------------------------------------
const B64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
function base64Bytes(b64) {
  let out = [], buf = 0, bits = 0;
  for (let i = 0; i < b64.length; i++) {
    let v = B64.indexOf(b64.charAt(i));
    if (v < 0) { continue; }                 // "=" and anything stray
    buf = (buf << 6) | v; bits += 6;
    if (bits >= 8) { bits -= 8; out.push((buf >> bits) & 0xff); }
  }
  return out;
}

function sha256Hex(bytes) {
  let h = Cc["@mozilla.org/security/hash;1"].createInstance(Ci.nsICryptoHash);
  h.init(Ci.nsICryptoHash.SHA256);
  h.update(bytes, bytes.length);
  let bin = h.finish(false), hex = "";
  for (let i = 0; i < bin.length; i++) { hex += ("0" + bin.charCodeAt(i).toString(16)).slice(-2); }
  return hex;
}

function readText(file) {
  let is = Cc["@mozilla.org/network/file-input-stream;1"].createInstance(Ci.nsIFileInputStream);
  is.init(file, 0x01, 0, 0);
  try {
    let n = is.available();
    if (n > MAX_FILE) { throw new Error("too large"); }
    let ss = Cc["@mozilla.org/scriptableinputstream;1"].createInstance(Ci.nsIScriptableInputStream);
    ss.init(is);
    return ss.read(n);
  } finally {
    try { is.close(); } catch (e) {}
  }
}

// Every certificate in one file: [{b64, der, sha256}]. A file with none is "unreadable".
function certsIn(file) {
  let text = readText(file), out = [], m;
  PEM_RE.lastIndex = 0;
  while ((m = PEM_RE.exec(text)) !== null) {
    let b64 = m[1].replace(/[^A-Za-z0-9+/=]/g, "");
    let bytes = base64Bytes(b64);
    if (bytes.length > 64) { out.push({b64: b64, der: bytes, sha256: sha256Hex(bytes)}); }
  }
  return out;
}

// What a certificate (its DER bytes) says about itself:
//   ca           a v3 certificate whose basicConstraints says cA=TRUE. The engine's own test
//                (CERT_IsCACert) also counts any self-signed v1 certificate as a CA, and mozilla::pkix
//                accepts a v1 trust anchor, so only the certificate's own word counts (the inspector,
//                2026-10-13).
//   keyCertSign  keyUsage is present and allows signing certificates.
//   san, eku     it names sites (subjectAltName) or limits its key to uses (extendedKeyUsage):
//                a site's own certificate does; a root does not (the CA/Browser Forum's rules).
//   self         its issuer and subject are the same name: a root, not an intermediate.
// Malformed or v1/v2 DER says nothing: every fact false.
function derFacts(b) {
  const NONE = {ca: false, keyCertSign: false, san: false, eku: false, self: false};
  let f = {ca: false, keyCertSign: false, san: false, eku: false, self: false};
  function tlv(i) {
    if (i + 2 > b.length) { return null; }
    let tag = b[i], len = b[i + 1], p = i + 2;
    if (len & 0x80) {
      let n = len & 0x7f;
      if (n < 1 || n > 3 || p + n > b.length) { return null; }
      len = 0;
      for (let k = 0; k < n; k++) { len = (len << 8) | b[p++]; }
    }
    if (p + len > b.length) { return null; }
    return {tag: tag, start: p, end: p + len};
  }
  function extIs(oid, last) {          // 2.5.29.<last>: 55 1d <last>
    return oid.end - oid.start === 3 && b[oid.start] === 0x55 && b[oid.start + 1] === 0x1d && b[oid.start + 2] === last;
  }
  let cert = tlv(0);
  if (!cert || cert.tag !== 0x30) { return NONE; }
  let tbs = tlv(cert.start);
  if (!tbs || tbs.tag !== 0x30) { return NONE; }
  let ver = tlv(tbs.start);
  if (!ver || ver.tag !== 0xa0) { return NONE; }               // v1: no version field
  let n = tlv(ver.start);
  if (!n || n.tag !== 0x02 || n.end - n.start !== 1 || b[n.start] !== 2) { return NONE; }
  // serial, signature, issuer, validity, subject, key, then [1] [2] [3] if present
  let fields = [];
  for (let i = ver.end; i < tbs.end;) {
    let t = tlv(i);
    if (!t) { return NONE; }
    fields.push(t);
    i = t.end;
  }
  if (fields.length < 6 || fields[2].tag !== 0x30 || fields[4].tag !== 0x30) { return NONE; }
  let is = fields[2], su = fields[4];
  f.self = is.end - is.start === su.end - su.start;
  for (let k = 0; f.self && k < is.end - is.start; k++) { f.self = b[is.start + k] === b[su.start + k]; }
  let exts = null;
  fields.forEach(function (t) { if (t.tag === 0xa3) { exts = t; } });
  if (!exts) { return f; }
  let seq = tlv(exts.start);
  if (!seq || seq.tag !== 0x30) { return NONE; }
  for (let j = seq.start; j < seq.end;) {
    let ext = tlv(j);
    if (!ext || ext.tag !== 0x30) { return NONE; }
    let oid = tlv(ext.start);
    if (!oid || oid.tag !== 0x06) { return NONE; }
    let v = tlv(oid.end);
    if (v && v.tag === 0x01) { v = tlv(v.end); }               // critical
    if (!v || v.tag !== 0x04) { return NONE; }                 // extnValue
    if (extIs(oid, 0x13)) {                                    // basicConstraints
      let bc = tlv(v.start);
      if (!bc || bc.tag !== 0x30) { return NONE; }
      let ca = bc.start < bc.end ? tlv(bc.start) : null;       // cA absent: FALSE
      f.ca = !!ca && ca.tag === 0x01 && ca.end - ca.start === 1 && b[ca.start] !== 0;
    } else if (extIs(oid, 0x0f)) {                             // keyUsage: BIT STRING, keyCertSign is bit 5
      let ku = tlv(v.start);
      f.keyCertSign = !!ku && ku.tag === 0x03 && ku.end - ku.start >= 2 && (b[ku.start + 1] & 0x04) !== 0;
    } else if (extIs(oid, 0x11)) {
      f.san = true;
    } else if (extIs(oid, 0x25)) {
      f.eku = true;
    }
    j = ext.end;
  }
  return f;
}
function derIsV3CA(b) { return derFacts(b).ca; }
// Removed roots are matched by their own fingerprint, not by name and key: Mozilla removed old
// issues of roots it still trusts under the same name and key (GTS Root R1-R4, GlobalSign,
// Firmaprofesional; measured against CCADB and webOS CE's store, round 24), so that match would
// refuse current roots. A CA under a removed root is no root, and is refused as notRoot.
// What R4 may trust without asking: a root that says its key signs certificates and names no site.
// A site's own certificate a user accepted with Trust Always (here, in webOS's browser or in an app)
// goes into Certificate Manager's store, and the usual router or NAS one (openssl req -x509) also says
// cA=TRUE: taken as a root, its key could vouch for any site (the inspector, round 24).
function rootShape(f) {
  if (!f.ca) { return "notCA"; }
  if (!f.self) { return "notRoot"; }
  if (f.san || f.eku) { return "siteLike"; }
  // A CA made without keyUsage (openssl's default) cannot be told from a router's certificate:
  // not taken silently. Opened in Jihad, it is asked about (R6).
  if (!f.keyCertSign) { return "noCertSign"; }
  return "";
}

function rawDER(cert) {
  try { return cert.getRawDER({}); } catch (e) { return []; }
}

// The Relay's, by fingerprint or key; or any squid-for-webos CA (subject), as the caller asks.
function isRelayCert(cert) {
  let fp = "", spki = "";
  try { fp = normFp(cert.sha256Fingerprint); } catch (e) {}
  try { spki = cert.sha256SubjectPublicKeyInfoDigest; } catch (e) {}
  return fp === RELAY_SHA256 || spki === RELAY_SPKI;
}
function isSquidWebosCert(cert) {
  try { return cert.organization === SQUID_WEBOS_ORG; } catch (e) { return false; }
}

// A name out of a certificate, fit to show: no control characters (a newline could fake a second
// line of the question), and not endless.
function shownName(s) {
  s = String(s || "").replace(/[\u0000-\u001f\u007f-\u009f\u2028\u2029]/g, " ").replace(/\s+/g, " ").trim();
  return s.length > 64 ? s.slice(0, 61) + "..." : s;
}

// isCertTrusted throws (NS_ERROR_FAILURE) for a certificate with no trust record at all, which is
// what a certificate fresh from a webOS store is (measured on the device, 2026-10-13): no record
// is not trusted.
function trustedNow(db, cert) {
  try { return db.isCertTrusted(cert, Ci.nsIX509Cert.CA_CERT, Ci.nsIX509CertDB.TRUSTED_SSL); }
  catch (e) { return false; }
}

// After a trust is taken away: a resumed TLS session and an idle kept-alive connection were each
// checked under the old trust and are reused without a new check (measured on the device,
// 2026-10-13: a page signed under a root just switched off still loaded). The daemon clears NSS's
// session cache ("jihad-clear-tls-sessions", DialogService.cpp) and idle connections are closed, so
// the next request is checked afresh; transfers under way and every Trust Once are left alone,
// which nsISecretDecoderRing.logoutAndTeardown (used at first) did not do. A page shown again from
// the HTTP cache is not a new connection, as in any browser.
function dropSessions() {
  try { Services.obs.notifyObservers(null, "jihad-clear-tls-sessions", null); } catch (e) {}
  try { Services.obs.notifyObservers(null, "net:prune-all-connections", null); } catch (e) {}
}

function fileAt(path) {
  let f = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsIFile);
  f.initWithPath(path);
  return f;
}

// --- the component -------------------------------------------------------------------------------
function JihadCerts() {
  this.wrappedJSObject = this;
  this._lists = null;          // jihad-cert-lists.json, parsed
  this._ready = false;         // NSS is up and the stores have been read
  this._webos = [];            // [{cert, name, sha256, from}] trusted from webOS's stores
  this._skipped = {};
  this._stores = [];
  this._relayCert = null;      // the Relay's CA, once constructed
  this._relayTrusted = false;
  this._error = "";
  this._offer = null;          // the last top-level CA certificate load: {host, at}
  this._unreadLogged = {};
  Services.obs.addObserver(this, "jihad-nss-ready", false);
  Services.obs.addObserver(this, "jihad-platform-cert", false);
  // A CA certificate opened as a page is the R6 question even when the server marks it as a
  // download: WebOne sends its CA with "Content-Disposition: attachment", and an attachment
  // skips every content listener, the engine's certificate import among them (nsURILoader's
  // forceExternalHandling), so the link only downloaded a file. The header outranks any hint
  // set on the channel, so it is taken off — for a top-level load of a CA certificate only. A CA
  // certificate in a frame never reaches the question at all (see _caLoad and JihadCertPolicy).
  Services.obs.addObserver(this, "http-on-examine-response", false);
  Services.obs.addObserver(this, "http-on-examine-cached-response", false);
  Services.obs.addObserver(this, "http-on-examine-merged-response", false);
  Services.prefs.addObserver(PREF_WEBOS, this, false);
  Services.prefs.addObserver(PREF_RELAY_CA, this, false);
  Services.prefs.addObserver("network.proxy.", this, false);
}

JihadCerts.prototype = {
  classID: Components.ID("{3c8e2f1a-5d47-4b9e-a2c6-7f0b1d9e4a63}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIObserver]),

  observe: function (subject, topic, data) {
    try {
      if (topic === "http-on-examine-response" || topic === "http-on-examine-cached-response" ||
          topic === "http-on-examine-merged-response") {
        this._caLoad(subject);
      } else if (topic === "jihad-platform-cert") {
        this._platformCert(String(data || ""));
      } else if (topic === "jihad-nss-ready") {
        this._ready = true;
        this._logBuiltins();
        this._applyWebos();
        this._applyRelay();
      } else if (topic === "nsPref:changed" && this._ready) {
        if (data === PREF_WEBOS) { this._applyWebos(); }
        else { this._applyRelay(); }
      }
    } catch (e) {
      log(topic + " threw: " + e);
    }
  },

  // The built-in roots the engine ships (libnssckbi.so), counted at every start so the daemon's log says
  // which list this engine carries (cavekit-certificate-trust.md R2: 169 certificates, 118 trusted for
  // sites, from UXP release of 2026-10-24; the June 2025 list before it had 177 and 150).
  _logBuiltins: function () {
    let n = 0, sites = 0, removed = 0, names = [];
    try {
      let db = certDB(), all = db.getCerts(), en = all.getEnumerator(), lists = this.lists();
      while (en.hasMoreElements()) {
        let c = en.getNext().QueryInterface(Ci.nsIX509Cert);
        if (c.tokenName !== "Builtin Object Token") { continue; }
        n++;
        if (db.isCertTrusted(c, Ci.nsIX509Cert.CA_CERT, Ci.nsIX509CertDB.TRUSTED_SSL)) { sites++; }
        // The same report the platform import checks against (R3): a built-in root Mozilla has
        // removed is the audit's first finding, counted here so the device says so at every start.
        if (lists.removed[normFp(c.sha256Fingerprint)]) { removed++; if (names.length < 6) { names.push(c.commonName); } }
      }
      log("built-in roots: " + n + " certificates, " + sites + " trusted for sites, " + removed +
          " on Mozilla's removed report" + (removed ? " (" + names.join("; ") + ")" : ""));
    } catch (e) {
      log("built-in roots could not be counted: " + e);
    }
  },

  // A CA certificate over HTTP. In a frame it is stopped: a hidden frame must never put "Trust ...?"
  // over the page the user is looking at. A script's fetch or an image never reaches the question,
  // so those are left alone (a router's page may fetch its CA to show it). The page itself: it may
  // be asked about once (offer), and a download marking is taken off.
  _caLoad: function (subject) {
    let ch = subject.QueryInterface(Ci.nsIHttpChannel);
    let type = "";
    try { type = String(ch.contentType || "").toLowerCase(); } catch (e) { return; }
    if (type !== "application/x-x509-ca-cert") { return; }
    let kind = -1;
    try { kind = ch.loadInfo.externalContentPolicyType; } catch (e) {}
    if (kind === Ci.nsIContentPolicy.TYPE_SUBDOCUMENT || kind === Ci.nsIContentPolicy.TYPE_OBJECT) {
      try { ch.cancel(Components.results.NS_BINDING_ABORTED); } catch (e) {}
      log("a CA certificate in a frame was stopped (" + ch.URI.prePath.slice(0, 80) + ")");
      return;
    }
    if (kind !== Ci.nsIContentPolicy.TYPE_DOCUMENT) { return; }
    let host = "";
    try { host = ch.URI.hostPort; } catch (e) {}
    this.offer(host);
    let disp = "";
    try { disp = ch.getResponseHeader("Content-Disposition"); } catch (e) { return; }   // none: already inline
    if (!/^\s*attachment/i.test(disp)) { return; }
    ch.setResponseHeader("Content-Disposition", "", false);
    log("a CA certificate sent as a download is opened, so its question is asked (" + ch.URI.prePath.slice(0, 80) + ")");
  },

  // The question may be asked only for a CA certificate opened as the page itself, once: an HTTP one
  // offers here from _caLoad (with its host), a data: or file: one from JihadCertPolicy (no host).
  // Anything else that reaches the engine's certificate import (a blob: frame, which no address
  // check can see the type of) finds no offer and is refused (the inspector, round 24).
  offer: function (host) {
    this._offer = {host: String(host || ""), at: Date.now()};
  },
  // The offer, used up: null when there is none from the last 20 seconds.
  takeOffer: function () {
    let o = this._offer;
    this._offer = null;
    return (o && Date.now() - o.at < 20000) ? o : null;
  },

  lists: function () {
    if (this._lists) { return this._lists; }
    try {
      let f = __LOCATION__.parent.parent;            // <bundle>/components/jihadCerts.js -> <bundle>
      f.append("certs");
      f.append("jihad-cert-lists.json");
      let j = JSON.parse(readText(f));
      j.stock = {}; j.removed = {};
      (j.hpStock.sha256 || []).forEach(function (h) { j.stock[h] = true; });
      (j.mozillaRemoved.sha256 || []).forEach(function (h) { j.removed[h] = true; });
      this._lists = j;
    } catch (e) {
      // Without the lists nothing from webOS can be told apart from HP's or Mozilla's removed
      // roots, so nothing is taken; the Relay's pin is missing too, so its setting does nothing.
      this._error = "The certificate lists are missing from this build (" + e + ").";
      log(this._error);
      this._lists = {stock: {}, removed: {}, relay: null, missing: true};
    }
    return this._lists;
  },

  isRelayFp: function (sha256) {
    return normFp(sha256) === RELAY_SHA256;
  },

  // --- R4 ---
  // The daemon's SSL dialog put a site's own certificate (PEM) into webOS's store: remember it, and
  // if it is trusted from there now, stop at once.
  _platformCert: function (pem) {
    PEM_RE.lastIndex = 0;
    let m = PEM_RE.exec(pem);
    if (!m) { return; }
    let fp = sha256Hex(base64Bytes(m[1].replace(/[^A-Za-z0-9+/=]/g, "")));
    let list = this.acceptedList().filter(function (h) { return h !== fp; });
    list.push(fp);
    try { Services.prefs.setCharPref(PREF_ACCEPTED, list.slice(-MAX_ACCEPTED).join(",")); } catch (e) {}
    let db = certDB(), was = this._webos.length;
    this._webos = this._webos.filter(function (e) {
      if (e.sha256 !== fp) { return true; }
      try { db.setCertTrust(e.cert, Ci.nsIX509Cert.CA_CERT, 0); } catch (x) {}
      return false;
    });
    if (this._webos.length !== was) { dropSessions(); }
    log("a site's certificate went into webOS's store from the SSL dialog; never taken as a root (" + fp.slice(0, 16) + "...)");
  },

  acceptedList: function () {
    return charPref(PREF_ACCEPTED).split(",").filter(function (h) { return /^[0-9a-f]{64}$/.test(h); });
  },

  _applyWebos: function () {
    let on = boolPref(PREF_WEBOS, true);
    if (!on) { this._dropWebos(); return; }
    if (this._webos.length) { this._retrust(this._webos); return; }
    this._readWebos();
  },

  _retrust: function (list) {
    let db = certDB();
    list.forEach(function (e) {
      try { db.setCertTrust(e.cert, Ci.nsIX509Cert.CA_CERT, Ci.nsIX509CertDB.TRUSTED_SSL); } catch (x) {}
    });
    log("trusting " + list.length + " certificate(s) from webOS again");
  },

  _dropWebos: function () {
    if (!this._webos.length) { return; }
    let db = certDB();
    // A trust of 0 for a CA keeps it a valid CA but no longer an anchor; the engine then treats a
    // site that chains only to it as untrusted (its certificate question comes back).
    this._webos.forEach(function (e) {
      try { db.setCertTrust(e.cert, Ci.nsIX509Cert.CA_CERT, 0); } catch (x) {}
    });
    dropSessions();
    log("stopped trusting " + this._webos.length + " certificate(s) from webOS (switched off)");
  },

  _readWebos: function () {
    let lists = this.lists();
    let skipped = {stock: 0, removed: 0, builtin: 0, expired: 0, notCA: 0, notRoot: 0, siteLike: 0, noCertSign: 0,
                   accepted: 0, already: 0, relay: 0, unreadable: 0};
    let stores = [], seen = {}, found = [];
    if (lists.missing) { this._skipped = skipped; this._stores = stores; return; }

    STORES.forEach(function (s) {
      let st = {path: s.path, from: s.from, present: false, certs: 0};
      stores.push(st);
      let files = [];
      try {
        let f = fileAt(s.path);
        if (!f.exists()) { return; }
        st.present = true;
        if (s.dir) {
          let en = f.directoryEntries;
          while (en.hasMoreElements()) {
            let e = en.getNext().QueryInterface(Ci.nsIFile);
            if (s.only && !s.only.test(e.leafName)) { continue; }
            if (s.links) {
              // A link into the system root list is that list's, read above; the others are the
              // certificates trusted in Certificate Manager. Read through the link itself.
              let to = "";
              try { if (e.isSymlink()) { to = e.target; } } catch (x) {}
              if (to.indexOf("/etc/ssl/certs/trustedcerts/") >= 0) { continue; }
            }
            files.push(e);
          }
        } else {
          files.push(f);
        }
      } catch (e) {
        log(s.path + ": cannot be read (" + e + ")");
        return;
      }
      let self = this;
      files.forEach(function (file) {
        let certs = [], why = "no certificate in it";
        try { certs = certsIn(file); } catch (e) { certs = []; why = String(e); }
        if (!certs.length) {
          skipped.unreadable++;
          if (!self._unreadLogged[file.path]) {
            self._unreadLogged[file.path] = true;
            log(file.path + ": skipped (" + why + ")");
          }
          return;
        }
        certs.forEach(function (c) {
          st.certs++;
          if (seen[c.sha256]) { return; }
          seen[c.sha256] = true;
          c.from = s.from;
          found.push(c);
        });
      });
    }, this);

    let db = certDB(), nowUs = Date.now() * 1000, trusted = [], accepted = {};
    this.acceptedList().forEach(function (h) { accepted[h] = true; });
    found.forEach(function (c) {
      if (c.sha256 === RELAY_SHA256) { skipped.relay++; return; }
      if (lists.stock[c.sha256]) { skipped.stock++; return; }
      if (lists.removed[c.sha256]) { skipped.removed++; return; }
      if (accepted[c.sha256]) { skipped.accepted++; return; }
      let cert;
      try { cert = db.constructX509FromBase64(c.b64); } catch (e) { skipped.unreadable++; return; }
      try {
        if (isRelayCert(cert) || isSquidWebosCert(cert)) { skipped.relay++; return; }
        if (cert.isBuiltInRoot) { skipped.builtin++; return; }
        if (cert.validity.notAfter < nowUs) { skipped.expired++; return; }
        // Only a root: a v3 CA, its own issuer, whose key signs certificates and which names no
        // site. An intermediate (a CA under a root Mozilla removed, say) or a site's own
        // certificate is never made an anchor here.
        let shape = rootShape(derFacts(c.der));
        if (shape) { skipped[shape]++; return; }
        if (cert.certType !== Ci.nsIX509Cert.CA_CERT) { skipped.notCA++; return; }
        // A CA the user already trusts in Jihad's own database (R6) is theirs; leave it.
        if (trustedNow(db, cert)) { skipped.already++; return; }
        db.setCertTrust(cert, Ci.nsIX509Cert.CA_CERT, Ci.nsIX509CertDB.TRUSTED_SSL);
        trusted.push({cert: cert, sha256: c.sha256, from: c.from,
                      name: shownName(cert.commonName || cert.organization || cert.subjectName)});
      } catch (e) {
        skipped.unreadable++;
        log("a certificate from " + c.from + " could not be taken: " + e);
      }
    });
    this._webos = trusted;
    this._skipped = skipped;
    this._stores = stores;
    log("webOS: " + found.length + " distinct certificate(s) in " +
        stores.filter(function (s) { return s.present; }).length + " store(s); trusting " + trusted.length +
        (trusted.length ? " (" + trusted.map(function (t) { return t.name; }).join(", ") + ")" : "") +
        "; skipped " + JSON.stringify(skipped));
  },

  // --- R2 ---
  relayInUse: function () {
    if (intPref("network.proxy.type") !== 1) { return false; }
    let h = charPref("network.proxy.ssl").toLowerCase(), g = charPref("network.proxy.http").toLowerCase();
    return h === RELAY_HOST || g === RELAY_HOST;
  },

  _applyRelay: function () {
    let lists = this.lists();
    let want = boolPref(PREF_RELAY_CA, false) && this.relayInUse() && !!lists.relay;
    if (want === this._relayTrusted) { return; }
    let db = certDB();
    try {
      if (!this._relayCert) {
        PEM_RE.lastIndex = 0;
        let m = PEM_RE.exec(String(lists.relay.pem));
        if (!m) { throw new Error("the list carries no certificate for the Relay"); }
        let b64 = m[1].replace(/[^A-Za-z0-9+/=]/g, "");
        this._relayCert = db.constructX509FromBase64(b64);
        if (normFp(this._relayCert.sha256Fingerprint) !== RELAY_SHA256) {
          this._relayCert = null;
          throw new Error("the carried certificate does not match its pin");
        }
      }
      db.setCertTrust(this._relayCert, Ci.nsIX509Cert.CA_CERT, want ? Ci.nsIX509CertDB.TRUSTED_SSL : 0);
      this._relayTrusted = want;
      if (!want) { dropSessions(); }
      log("the Riverstone Relay's certificate is " + (want ? "trusted (the user chose it; the Relay is in use)" : "not trusted"));
    } catch (e) {
      log("the Relay's certificate: " + e);
    }
  },

  // --- R6 ---
  // The CAs the user added through a link: in the profile's database, trusted for websites, not
  // built in, and not one this component trusts in memory.
  addedCAs: function () {
    let out = [], mine = {};
    this._webos.forEach(function (e) { mine[e.sha256] = true; });
    mine[RELAY_SHA256] = true;
    let db = certDB();
    try {
      let en = db.getCerts().getEnumerator();
      while (en.hasMoreElements()) {
        let c = en.getNext().QueryInterface(Ci.nsIX509Cert);
        try {
          if (c.isBuiltInRoot || c.certType !== Ci.nsIX509Cert.CA_CERT) { continue; }
          let fp = normFp(c.sha256Fingerprint);
          if (mine[fp]) { continue; }
          if (!trustedNow(db, c)) { continue; }
          out.push({name: shownName(c.commonName || c.organization || c.subjectName), org: shownName(c.organization),
                    sha256: fp, fingerprint: showFp(fp)});
        } catch (e) {}
      }
    } catch (e) {
      log("listing the added certificates: " + e);
    }
    return out;
  },

  removeAdded: function (sha256) {
    let want = normFp(sha256), db = certDB();
    let en = db.getCerts().getEnumerator();
    while (en.hasMoreElements()) {
      let c = en.getNext().QueryInterface(Ci.nsIX509Cert);
      if (normFp(c.sha256Fingerprint) !== want || c.isBuiltInRoot) { continue; }
      // Distrust first: the trust is what matters, and it goes even if the delete does not.
      try { db.setCertTrust(c, Ci.nsIX509Cert.CA_CERT, 0); } catch (e) {}
      try { db.deleteCertificate(c); } catch (e) { log("removing " + want.slice(0, 16) + ": " + e); }
      dropSessions();
      log("removed " + (c.commonName || want.slice(0, 16)));
      return true;
    }
    return false;
  },

  // --- for Settings ---
  report: function () {
    return {
      ready: this._ready,
      enabled: boolPref(PREF_WEBOS, true),
      error: this._error,
      // Switched off, nothing from webOS is trusted, so none is listed.
      trusted: !boolPref(PREF_WEBOS, true) ? [] : this._webos.map(function (e) {
        return {name: e.name, from: e.from, sha256: e.sha256, fingerprint: showFp(e.sha256)};
      }),
      skipped: this._skipped,
      stores: this._stores,
      relay: {inUse: this.relayInUse(), wanted: boolPref(PREF_RELAY_CA, false), trusted: this._relayTrusted,
              fingerprint: showFp(RELAY_SHA256)},
      lists: this.lists().missing ? null : {
        stock: this.lists().hpStock.count, removed: this.lists().mozillaRemoved.count,
        fetched: this.lists().mozillaRemoved.fetched
      }
    };
  }
};

// --- R6: the question when a CA certificate is opened --------------------------------------------
function prompterFor(ctx) {
  try { if (ctx) { return ctx.getInterface(Ci.nsIPrompt); } } catch (e) {}
  return Cc["@mozilla.org/prompter;1"].getService(Ci.nsIPromptFactory).getPrompt(null, Ci.nsIPrompt);
}

function JihadCertDialogs() {}

JihadCertDialogs.prototype = {
  classID: Components.ID("{8a1d5e3b-2c94-4f07-b6e1-0d7c3a9f5b28}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsICertificateDialogs]),

  confirmDownloadCACert: function (ctx, cert, trust) {
    trust.value = 0;
    let p = prompterFor(ctx);
    let svc = null;
    try { svc = Cc["@riverstonerelay.net/jihad/certs;1"].getService().wrappedJSObject; } catch (e) {}
    let fp = normFp(cert.sha256Fingerprint);
    let name = shownName(cert.commonName || cert.organization || cert.subjectName);
    let offer = svc ? svc.takeOffer() : null;
    if (!offer) {
      // Not opened as a page: a frame or something else a page did. Nothing is asked or shown.
      log("refused a CA certificate that was not opened as a page (" + fp.slice(0, 16) + "...)");
      return false;
    }
    let from = offer.host;
    if (isRelayCert(cert)) {
      p.alert("Certificate", "This is the Riverstone Relay's certificate. Jihad trusts it only through " +
              "Settings \u2192 Advanced \u2192 Proxy, while the Relay is in use, and only if you turn that on.");
      log("refused the Relay's certificate offered by a link");
      return false;
    }
    // The engine also lets a self-signed v1 site certificate through as a "CA" (CERT_IsCACert); and a
    // site's own certificate that also says cA=TRUE (a router's, from openssl req -x509) names the
    // site it is for. Neither is a certificate authority's.
    let facts = derFacts(rawDER(cert));
    if (!facts.ca || facts.san || facts.eku) {
      p.alert("Certificate", "\"" + name + "\" is not a certificate authority's certificate, so Jihad does not " +
              "take it as one. A site's own certificate is accepted when the site is opened.");
      log("refused a certificate that is not a CA's (" + fp.slice(0, 16) + "...)");
      return false;
    }
    let lists = svc ? svc.lists() : null;
    if (lists && lists.removed && lists.removed[fp]) {
      p.alert("Certificate", "\"" + name + "\" is a root Mozilla has removed from its trusted list, so Jihad " +
              "does not take it.");
      log("refused a root Mozilla removed (" + fp.slice(0, 16) + "...)");
      return false;
    }
    if (svc && svc.report().trusted.some(function (t) { return t.sha256 === fp; })) {
      p.alert("Certificate", "Jihad already trusts \"" + name + "\": it is one of the certificates webOS has.");
      return false;
    }
    let org = cert.organization && shownName(cert.organization) !== name ? " (" + shownName(cert.organization) + ")" : "";
    let squid = isSquidWebosCert(cert)
      ? "This is a squid-for-webos proxy's certificate. If it is the Riverstone Relay's, say No: the Relay's is " +
        "trusted only through Settings \u2192 Advanced \u2192 Proxy.\n\n" : "";
    let ok = p.confirm("Trust this certificate authority?",
      "Trust \"" + name + "\"" + org + " to identify websites?\n\n" +
      "Offered by " + (from || "a file or page on this device") + ".\n" +
      "SHA-256 " + showFpGrouped(fp) + "\n\n" + squid +
      "A proxy that holds this certificate can read and change every secure page it carries. " +
      "Trust it only if you run that proxy, or trust whoever does.");
    if (ok) { trust.value = Ci.nsIX509CertDB.TRUSTED_SSL; }
    log((ok ? "trusted " : "declined ") + name + " (" + fp.slice(0, 16) + "...) from a link");
    return ok;
  },

  // A PKCS#12 backup and the certificate viewer have no place on this device.
  setPKCS12FilePassword: function (ctx, password) { password.value = ""; return false; },
  getPKCS12FilePassword: function (ctx, password) { password.value = ""; return false; },
  viewCert: function (ctx, cert) {}
};

// --- R6: which certificate files may be asked about -------------------------------------------------
// An HTTP one is sorted by JihadCerts._caLoad. A data: or file: one is not an HTTP load, so it is
// sorted here, by its address, before it loads: in a frame it is refused (a page could otherwise
// raise the question from a hidden frame with a data: address carrying a CA certificate); opened as
// the page itself it may be asked about once. Anything else (a blob: frame) never has an offer.
function JihadCertPolicy() {}

JihadCertPolicy.prototype = {
  classID: Components.ID("{5b7e2c94-1d3a-4f68-8e0b-a49c6d2f7e13}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIContentPolicy]),

  shouldLoad: function (type, location) {
    let frame = type === Ci.nsIContentPolicy.TYPE_SUBDOCUMENT, page = type === Ci.nsIContentPolicy.TYPE_DOCUMENT;
    if ((!frame && !page) || !location) { return Ci.nsIContentPolicy.ACCEPT; }
    let certFile = false;
    try {
      if (location.schemeIs("data")) {
        certFile = /^data:\s*application\/(x-x509-ca-cert|pkix-cert)/i.test(location.spec);
      } else if (location.schemeIs("file")) {
        let ext = "";
        try { ext = location.QueryInterface(Ci.nsIURL).fileExtension; } catch (e) {}
        certFile = /^(crt|cer|der|pem)$/i.test(ext);
      }
    } catch (e) {}
    if (!certFile) { return Ci.nsIContentPolicy.ACCEPT; }
    if (page) {
      try { Cc["@riverstonerelay.net/jihad/certs;1"].getService().wrappedJSObject.offer(""); } catch (e) {}
      return Ci.nsIContentPolicy.ACCEPT;
    }
    log("a certificate file in a frame was not loaded (" + location.scheme + ":)");
    return Ci.nsIContentPolicy.REJECT_REQUEST;
  },

  shouldProcess: function () { return Ci.nsIContentPolicy.ACCEPT; }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadCerts, JihadCertDialogs, JihadCertPolicy]);
