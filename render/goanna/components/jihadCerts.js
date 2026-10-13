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

// Every certificate in one file: [{b64, sha256}]. A file with none is "unreadable".
function certsIn(file) {
  let text = readText(file), out = [], m;
  PEM_RE.lastIndex = 0;
  while ((m = PEM_RE.exec(text)) !== null) {
    let b64 = m[1].replace(/[^A-Za-z0-9+/=]/g, "");
    let bytes = base64Bytes(b64);
    if (bytes.length > 64) { out.push({b64: b64, sha256: sha256Hex(bytes)}); }
  }
  return out;
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
// 2026-10-13: a page signed under a root just switched off still loaded). The session cache is
// cleared and the idle connections closed, so the next request is checked afresh. A page shown
// again from the HTTP cache is not a new connection, as in any browser.
function dropSessions() {
  try { Cc["@mozilla.org/security/sdr;1"].getService(Ci.nsISecretDecoderRing).logoutAndTeardown(); }
  catch (e) { log("clearing the TLS sessions: " + e); }
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
  Services.obs.addObserver(this, "jihad-nss-ready", false);
  // A CA certificate opened as a page is the R6 question even when the server marks it as a
  // download: WebOne sends its CA with "Content-Disposition: attachment", and an attachment
  // skips every content listener, the engine's certificate import among them (nsURILoader's
  // forceExternalHandling), so the link only downloaded a file. The header outranks any hint
  // set on the channel, so it is taken off — for a top-level load of a CA certificate only.
  Services.obs.addObserver(this, "http-on-examine-response", false);
  Services.obs.addObserver(this, "http-on-examine-cached-response", false);
  Services.prefs.addObserver(PREF_WEBOS, this, false);
  Services.prefs.addObserver(PREF_RELAY_CA, this, false);
  Services.prefs.addObserver("network.proxy.", this, false);
}

JihadCerts.prototype = {
  classID: Components.ID("{3c8e2f1a-5d47-4b9e-a2c6-7f0b1d9e4a63}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIObserver]),

  observe: function (subject, topic, data) {
    try {
      if (topic === "http-on-examine-response" || topic === "http-on-examine-cached-response") {
        this._caInline(subject);
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

  _caInline: function (subject) {
    let ch = subject.QueryInterface(Ci.nsIHttpChannel);
    if (!(ch.loadFlags & Ci.nsIChannel.LOAD_DOCUMENT_URI)) { return; }
    let type = "";
    try { type = String(ch.contentType || "").toLowerCase(); } catch (e) { return; }
    if (type !== "application/x-x509-ca-cert") { return; }
    let disp = "";
    try { disp = ch.getResponseHeader("Content-Disposition"); } catch (e) { return; }   // none: already inline
    if (!/^\s*attachment/i.test(disp)) { return; }
    ch.setResponseHeader("Content-Disposition", "", false);
    log("a CA certificate sent as a download is opened, so its question is asked (" + ch.URI.spec.slice(0, 80) + ")");
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
    let l = this.lists();
    return !!(l.relay && normFp(sha256) === l.relay.sha256);
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
      files.forEach(function (file) {
        let certs = [];
        try { certs = certsIn(file); } catch (e) { certs = []; }
        if (!certs.length) { skipped.unreadable++; return; }
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
      if (lists.relay && c.sha256 === lists.relay.sha256) { skipped.relay++; return; }
      if (lists.stock[c.sha256]) { skipped.stock++; return; }
      if (lists.removed[c.sha256]) { skipped.removed++; return; }
      let cert;
      try { cert = db.constructX509FromBase64(c.b64); } catch (e) { skipped.unreadable++; return; }
      try {
        if (cert.isBuiltInRoot) { skipped.builtin++; return; }
        if (cert.validity.notAfter < nowUs) { skipped.expired++; return; }
        if (cert.certType !== Ci.nsIX509Cert.CA_CERT) { skipped.notCA++; return; }
        // A CA the user already trusts in Jihad's own database (R6) is theirs; leave it.
        if (trustedNow(db, cert)) { skipped.already++; return; }
        db.setCertTrust(cert, Ci.nsIX509Cert.CA_CERT, Ci.nsIX509CertDB.TRUSTED_SSL);
        trusted.push({cert: cert, sha256: c.sha256, from: c.from,
                      name: cert.commonName || cert.organization || cert.subjectName});
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
        if (normFp(this._relayCert.sha256Fingerprint) !== lists.relay.sha256) {
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
    let l = this.lists();
    if (l.relay) { mine[l.relay.sha256] = true; }
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
          out.push({name: c.commonName || c.organization || c.subjectName, org: c.organization || "",
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
      trusted: this._webos.map(function (e) {
        return {name: e.name, from: e.from, sha256: e.sha256, fingerprint: showFp(e.sha256)};
      }),
      skipped: this._skipped,
      stores: this._stores,
      relay: {inUse: this.relayInUse(), wanted: boolPref(PREF_RELAY_CA, false), trusted: this._relayTrusted,
              fingerprint: this.lists().relay ? showFp(this.lists().relay.sha256) : ""},
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
    let name = cert.commonName || cert.organization || cert.subjectName;
    if (svc && svc.isRelayFp(fp)) {
      p.alert("Certificate", "This is the Riverstone Relay's certificate. Jihad trusts it only through " +
              "Settings \u2192 Advanced \u2192 Proxy, while the Relay is in use, and only if you turn that on.");
      log("refused the Relay's certificate offered by a link");
      return false;
    }
    if (svc && svc.report().trusted.some(function (t) { return t.sha256 === fp; })) {
      p.alert("Certificate", "Jihad already trusts \"" + name + "\": it is one of the certificates webOS has.");
      return false;
    }
    let org = cert.organization && cert.organization !== name ? " (" + cert.organization + ")" : "";
    let ok = p.confirm("Trust this certificate authority?",
      "Trust \"" + name + "\"" + org + " to identify websites?\n\n" +
      "SHA-256 " + showFpGrouped(fp) + "\n\n" +
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

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadCerts, JihadCertDialogs]);
