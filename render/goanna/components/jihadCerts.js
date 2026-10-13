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
 *      A site's own certificate found there is not taken: the engine never uses one as a root.
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

// Is this certificate (its DER bytes) a v3 certificate whose basicConstraints says cA=TRUE?
// The engine's own test (CERT_IsCACert) also counts any self-signed v1 certificate as a CA, and
// mozilla::pkix accepts a v1 trust anchor, so a router's or a NAS box's own v1 certificate that a
// user once accepted in webOS would become a root able to vouch for any site (the inspector,
// 2026-10-13). Only the certificate's own word that it is a CA counts here.
function derIsV3CA(b) {
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
  function basicConstraintsCA(exts) {
    let seq = tlv(exts.start);
    if (!seq || seq.tag !== 0x30) { return false; }
    for (let j = seq.start; j < seq.end;) {
      let ext = tlv(j);
      if (!ext || ext.tag !== 0x30) { return false; }
      let oid = tlv(ext.start);
      if (!oid || oid.tag !== 0x06) { return false; }
      if (oid.end - oid.start === 3 && b[oid.start] === 0x55 && b[oid.start + 1] === 0x1d && b[oid.start + 2] === 0x13) {
        let v = tlv(oid.end);
        if (v && v.tag === 0x01) { v = tlv(v.end); }           // critical
        if (!v || v.tag !== 0x04) { return false; }            // extnValue
        let bc = tlv(v.start);
        if (!bc || bc.tag !== 0x30 || bc.start === bc.end) { return false; }   // cA absent: FALSE
        let ca = tlv(bc.start);
        return !!ca && ca.tag === 0x01 && ca.end - ca.start === 1 && b[ca.start] !== 0;
      }
      j = ext.end;
    }
    return false;
  }
  let cert = tlv(0);
  if (!cert || cert.tag !== 0x30) { return false; }
  let tbs = tlv(cert.start);
  if (!tbs || tbs.tag !== 0x30) { return false; }
  let ver = tlv(tbs.start);
  if (!ver || ver.tag !== 0xa0) { return false; }               // v1: no version field
  let n = tlv(ver.start);
  if (!n || n.tag !== 0x02 || n.end - n.start !== 1 || b[n.start] !== 2) { return false; }
  for (let i = ver.end; i < tbs.end;) {
    let t = tlv(i);
    if (!t) { return false; }
    if (t.tag === 0xa3) { return basicConstraintsCA(t); }
    i = t.end;
  }
  return false;
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
      } else if (topic === "jihad-nss-ready") {
        this._ready = true;
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

  // A CA certificate over HTTP. In a frame (or any load that is not the page itself) it is stopped:
  // a hidden frame must never put "Trust ...?" over the page the user is looking at. The page
  // itself: the offering host is remembered for the question, and a download marking is taken off.
  _caLoad: function (subject) {
    let ch = subject.QueryInterface(Ci.nsIHttpChannel);
    let type = "";
    try { type = String(ch.contentType || "").toLowerCase(); } catch (e) { return; }
    if (type !== "application/x-x509-ca-cert") { return; }
    let top = false;
    try { top = ch.loadInfo.externalContentPolicyType === Ci.nsIContentPolicy.TYPE_DOCUMENT; } catch (e) {}
    if (!top) {
      try { ch.cancel(Components.results.NS_BINDING_ABORTED); } catch (e) {}
      log("a CA certificate in a frame or a sub-load was stopped (" + ch.URI.prePath.slice(0, 80) + ")");
      return;
    }
    let host = "";
    try { host = ch.URI.hostPort; } catch (e) {}
    this._offer = {host: host, at: Date.now()};
    let disp = "";
    try { disp = ch.getResponseHeader("Content-Disposition"); } catch (e) { return; }   // none: already inline
    if (!/^\s*attachment/i.test(disp)) { return; }
    ch.setResponseHeader("Content-Disposition", "", false);
    log("a CA certificate sent as a download is opened, so its question is asked (" + ch.URI.prePath.slice(0, 80) + ")");
  },

  // Who offered the certificate now being asked about: the page's host if one was opened in the
  // last minute, otherwise a file or page on this device.
  offeredBy: function () {
    let o = this._offer;
    this._offer = null;
    return (o && Date.now() - o.at < 60000 && o.host) ? o.host : "";
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
    let skipped = {stock: 0, removed: 0, builtin: 0, expired: 0, notCA: 0, already: 0, relay: 0, unreadable: 0};
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

    let db = certDB(), nowUs = Date.now() * 1000, trusted = [];
    found.forEach(function (c) {
      if (c.sha256 === RELAY_SHA256) { skipped.relay++; return; }
      if (lists.stock[c.sha256]) { skipped.stock++; return; }
      if (lists.removed[c.sha256]) { skipped.removed++; return; }
      if (!derIsV3CA(c.der)) { skipped.notCA++; return; }
      let cert;
      try { cert = db.constructX509FromBase64(c.b64); } catch (e) { skipped.unreadable++; return; }
      try {
        if (isRelayCert(cert) || isSquidWebosCert(cert)) { skipped.relay++; return; }
        if (cert.isBuiltInRoot) { skipped.builtin++; return; }
        if (cert.validity.notAfter < nowUs) { skipped.expired++; return; }
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
    let from = svc ? svc.offeredBy() : "";
    if (isRelayCert(cert)) {
      p.alert("Certificate", "This is the Riverstone Relay's certificate. Jihad trusts it only through " +
              "Settings \u2192 Advanced \u2192 Proxy, while the Relay is in use, and only if you turn that on.");
      log("refused the Relay's certificate offered by a link");
      return false;
    }
    // The engine also lets a self-signed v1 site certificate through as a "CA" (CERT_IsCACert).
    if (!derIsV3CA(rawDER(cert))) {
      p.alert("Certificate", "\"" + name + "\" is not a certificate authority's certificate, so Jihad does not " +
              "take it as one. A site's own certificate is accepted when the site is opened.");
      log("refused a certificate that is not a v3 CA (" + fp.slice(0, 16) + "...)");
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

// --- R6: a certificate file in a frame never reaches the question -----------------------------------
// An HTTP one is stopped by JihadCerts._caLoad. A data: or file: frame is not an HTTP load, so it is
// refused here, by its address, before it loads (a page could otherwise raise the question from a
// hidden frame with a data: address carrying a CA certificate).
function JihadCertPolicy() {}

JihadCertPolicy.prototype = {
  classID: Components.ID("{5b7e2c94-1d3a-4f68-8e0b-a49c6d2f7e13}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIContentPolicy]),

  shouldLoad: function (type, location) {
    if (type !== Ci.nsIContentPolicy.TYPE_SUBDOCUMENT || !location) { return Ci.nsIContentPolicy.ACCEPT; }
    let refuse = false;
    try {
      if (location.schemeIs("data")) {
        refuse = /^data:\s*application\/(x-x509-ca-cert|pkix-cert)/i.test(location.spec);
      } else if (location.schemeIs("file")) {
        let ext = "";
        try { ext = location.QueryInterface(Ci.nsIURL).fileExtension; } catch (e) {}
        refuse = /^(crt|cer|der|pem)$/i.test(ext);
      }
    } catch (e) {}
    if (refuse) { log("a certificate file in a frame was not loaded (" + location.scheme + ":)"); }
    return refuse ? Ci.nsIContentPolicy.REJECT_REQUEST : Ci.nsIContentPolicy.ACCEPT;
  },

  shouldProcess: function () { return Ci.nsIContentPolicy.ACCEPT; }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadCerts, JihadCertDialogs, JihadCertPolicy]);
