/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — printing (cavekit-mochi-ui.md R2 "Print").
 *
 * WHY THIS EXISTS AND DOES NOT USE THE PLATFORM'S PRINT MANAGER. webOS ships a print stack
 * (com.palm.printmgr / printmgrd, HP's wprint) and the stock browser prints through it. It
 * accepts HP printers ONLY: on the test network it found an "EPSON ET-2800 Series" over mDNS
 * and logged `_resolve_reply: not HP printer and doing printer filtering` — the printer is
 * seen and then discarded (measured 2026-10-05). So this component prints by itself, to any
 * printer that speaks IPP and takes JPEG, which is what driverless printing (IPP Everywhere /
 * AirPrint / Mopria) requires of a printer:
 *
 *   find    mDNS browse for _ipp._tcp / _ipps._tcp (asked from port 5353 — see mdnsBrowse for
 *           why the engine's own DNS-SD component cannot be used here), or a printer address
 *           typed by hand, each confirmed with an IPP Get-Printer-Attributes;
 *   render  the page is drawn into page-shaped slices with canvas.drawWindow — the engine's
 *           own "render this part of the document" call, so what is printed is what the page
 *           looks like — and each slice is encoded as a JPEG;
 *   send    one IPP Print-Job per slice, over TLS when the printer wants it (the Epson answers
 *           plain HTTP on 631 with "426 Upgrade Required"). A printer's certificate is
 *           self-signed by nature; it is accepted for THAT printer for the session, the same
 *           trust-on-first-use every driverless print client applies.
 *
 * THE DOORS. Requests arrive as observer notifications from the daemon (which gets them from
 * the card over the variant's own Luna service):
 *     "jihad-print-find"  data = {"address": optional}
 *     "jihad-print-job"   subject = the content window to print, data = {"printer": {...}}
 * Results leave through the daemon's notification channel ("jihad-notify", a property bag
 * with `category` and `text`), which the card is already subscribed to:
 *     print-printers  text = JSON {printers: [{name, model, address, port, path, tls, media, fit}],
 *                     done: bool} — pushed each time the list grows and once more, with
 *                     done:true, when the search is over
 *     print-progress / print-done / print-error   text = one line for the user
 *
 * A printer address is only ever a literal on the local network (private IPv4, link-local):
 * this component will not post a picture of the user's page to an arbitrary host.
 */

"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");

const MAX_PAGES = 30;
const FIND_MS = 3500;
const ERROR_UNTRUSTED = 1, ERROR_MISMATCH = 2, ERROR_TIME = 4;

// Through the console service: the daemon bridges that to its own log (EngineHost's console
// listener), which is the only place a device run can read what happened here.
function log(s) {
  try { Services.console.logStringMessage("print: " + s); } catch (e) {}
}

function notify(category, text) {
  try {
    let bag = Cc["@mozilla.org/hash-property-bag;1"].createInstance(Ci.nsIWritablePropertyBag2);
    bag.setPropertyAsAString("category", category);
    bag.setPropertyAsAString("text", String(text));
    Services.obs.notifyObservers(bag, "jihad-notify", null);
  } catch (e) { log("notify failed: " + e); }
}

// ── which addresses may be printers ─────────────────────────────────────────────────────────
function isLocalAddress(a) {
  let m = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/.exec(String(a || ""));
  if (!m) { return false; }
  let o = m.slice(1).map(Number);
  if (o.some(function(x) { return x > 255; })) { return false; }
  return o[0] === 10 || (o[0] === 172 && o[1] >= 16 && o[1] <= 31) ||
         (o[0] === 192 && o[1] === 168) || (o[0] === 169 && o[1] === 254);
}

function cleanPrinter(p) {
  if (!p || !isLocalAddress(p.address)) { return null; }
  let port = parseInt(p.port, 10);
  if (!(port > 0 && port < 65536)) { port = 631; }
  let path = String(p.path || "ipp/print").replace(/^\/+/, "");
  if (!/^[A-Za-z0-9._~\/-]{1,120}$/.test(path)) { path = "ipp/print"; }
  return {
    name: String(p.name || p.model || p.address).replace(/[\x00-\x1f\x7f]/g, " ").substr(0, 80),
    model: String(p.model || "").replace(/[\x00-\x1f\x7f]/g, " ").substr(0, 80),
    address: p.address, port: port, path: path, tls: p.tls !== false
  };
}

// ── IPP, the little of it that is needed ─────────────────────────────────────────────────────
function ippAttr(tag, name, value) {
  let n = [], v = [], i;
  for (i = 0; i < name.length; i++) { n.push(name.charCodeAt(i) & 0xff); }
  if (typeof value === "number") {
    v = [(value >>> 24) & 0xff, (value >>> 16) & 0xff, (value >>> 8) & 0xff, value & 0xff];
  } else {
    let s = unescape(encodeURIComponent(String(value)));     // UTF-8 bytes
    for (i = 0; i < s.length; i++) { v.push(s.charCodeAt(i)); }
  }
  return [tag, (n.length >> 8) & 0xff, n.length & 0xff].concat(n, [(v.length >> 8) & 0xff, v.length & 0xff], v);
}

function ippRequest(op, requestId, printerUri, opAttrs, jobAttrs, doc) {
  let b = [2, 0, (op >> 8) & 0xff, op & 0xff,
           (requestId >>> 24) & 0xff, (requestId >>> 16) & 0xff, (requestId >>> 8) & 0xff, requestId & 0xff, 0x01];
  b = b.concat(ippAttr(0x47, "attributes-charset", "utf-8"),
               ippAttr(0x48, "attributes-natural-language", "en"),
               ippAttr(0x45, "printer-uri", printerUri));
  (opAttrs || []).forEach(function(a) { b = b.concat(ippAttr(a[0], a[1], a[2])); });
  if (jobAttrs && jobAttrs.length) {
    b.push(0x02);
    jobAttrs.forEach(function(a) { b = b.concat(ippAttr(a[0], a[1], a[2])); });
  }
  b.push(0x03);
  let out = new Uint8Array(b.length + (doc ? doc.length : 0));
  out.set(b, 0);
  if (doc) { out.set(doc, b.length); }
  return out;
}

//* Parse an IPP response: {status, attrs: {name: [values]}}. Values are strings, numbers or
//* booleans; anything else is given as a string of its bytes.
function ippParse(buf) {
  let d = new Uint8Array(buf), out = {status: -1, attrs: {}};
  if (d.length < 9) { return out; }
  out.status = (d[2] << 8) | d[3];
  let i = 8, last = null;
  while (i < d.length) {
    let tag = d[i++];
    if (tag < 0x10) { if (tag === 3) { break; } continue; }
    if (i + 2 > d.length) { break; }
    let nl = (d[i] << 8) | d[i + 1]; i += 2;
    let name = "";
    for (let k = 0; k < nl && i + k < d.length; k++) { name += String.fromCharCode(d[i + k]); }
    i += nl;
    if (i + 2 > d.length) { break; }
    let vl = (d[i] << 8) | d[i + 1]; i += 2;
    let v;
    if ((tag === 0x21 || tag === 0x23) && vl === 4) {
      v = (d[i] << 24) | (d[i + 1] << 16) | (d[i + 2] << 8) | d[i + 3];
    } else if (tag === 0x22 && vl === 1) {
      v = d[i] !== 0;
    } else {
      v = "";
      for (let k = 0; k < vl && i + k < d.length; k++) { v += String.fromCharCode(d[i + k]); }
    }
    i += vl;
    if (name) { last = name; out.attrs[last] = []; }
    if (last) { out.attrs[last].push(v); }
  }
  return out;
}

// ── transport: POST application/ipp, over TLS when asked, trusting the printer's own cert ───
function ippPost(printer, body, onDone, retried) {
  let url = (printer.tls ? "https" : "http") + "://" + printer.address + ":" + printer.port + "/" + printer.path;
  let xhr = Cc["@mozilla.org/xmlextras/xmlhttprequest;1"].createInstance(Ci.nsIXMLHttpRequest);
  try {
    xhr.open("POST", url, true);
    xhr.responseType = "arraybuffer";
    xhr.setRequestHeader("Content-Type", "application/ipp");
    xhr.timeout = 90000;
    xhr.channel.loadFlags |= Ci.nsIRequest.LOAD_BYPASS_CACHE | Ci.nsIRequest.LOAD_ANONYMOUS |
                             Ci.nsIRequest.INHIBIT_CACHING;
  } catch (e) { onDone(null, "could not open " + url + ": " + e); return; }
  let finished = false;
  function fail(why) {
    if (finished) { return; }
    finished = true;
    // The engine's own reason, for the log: "could not be reached" covers a refused
    // connection, a failed handshake and a reset alike.
    try { why += " (0x" + (xhr.channel.status >>> 0).toString(16) + ")"; } catch (e) {}
    // A printer's certificate is self-signed. If that is what stopped us, accept THIS
    // printer's certificate for the session and try once more.
    if (printer.tls && !retried && trustPrinterCert(xhr, printer)) {
      ippPost(printer, body, onDone, true);
      return;
    }
    onDone(null, why);
  }
  xhr.onload = function() {
    if (finished) { return; }
    finished = true;
    if (xhr.status !== 200) { onDone(null, "the printer answered HTTP " + xhr.status); return; }
    onDone(ippParse(xhr.response), null);
  };
  xhr.onerror = function() { fail("the printer could not be reached"); };
  xhr.ontimeout = function() { fail("the printer did not answer in time"); };
  try { xhr.send(body); } catch (e) { fail("send failed: " + e); }
}

function trustPrinterCert(xhr, printer) {
  try {
    let info = xhr.channel.securityInfo;
    if (!info) { return false; }                     // not a TLS failure at all
    let status = info.QueryInterface(Ci.nsISSLStatusProvider).SSLStatus;
    if (!status || !status.serverCert) { return false; }   // the handshake never got as far as a certificate
    let flags = 0;
    if (status.isUntrusted) { flags |= ERROR_UNTRUSTED; }
    if (status.isDomainMismatch) { flags |= ERROR_MISMATCH; }
    if (status.isNotValidAtThisTime) { flags |= ERROR_TIME; }
    if (!flags) { return false; }
    Cc["@mozilla.org/security/certoverride;1"].getService(Ci.nsICertOverrideService)
      .rememberValidityOverride(printer.address, printer.port, status.serverCert, flags, true /* this session only */);
    log("accepted the printer's own certificate for " + printer.address + ":" + printer.port + " (flags " + flags + ", session only)");
    return true;
  } catch (e) {
    log("certificate override failed: " + e);
    return false;
  }
}

function printerUri(p) { return (p.tls ? "ipps" : "ipp") + "://" + p.address + ":" + p.port + "/" + p.path; }

//* Ask a would-be printer what it is. onDone(printer-with-details) or onDone(null).
function probePrinter(p, onDone) {
  // Only what is used: an embedded printer takes seconds to list everything it knows.
  let body = ippRequest(0x000B, 1, printerUri(p), [
    [0x44, "requested-attributes", "printer-make-and-model"], [0x44, "", "document-format-supported"],
    [0x44, "", "media-default"], [0x44, "", "print-scaling-supported"]], null, null);
  ippPost(p, body, function(resp, err) {
    if (!resp || resp.status >= 0x0100) {
      // A printer that does not want TLS: try the same address in the clear, once.
      if (p.tls) {
        let q = {name: p.name, model: p.model, address: p.address, port: p.port, path: p.path, tls: false};
        probePrinter(q, onDone);
        return;
      }
      log("probe " + p.address + " failed: " + (err || ("ipp status 0x" + (resp ? resp.status.toString(16) : "?"))));
      onDone(null);
      return;
    }
    let a = resp.attrs;
    let formats = a["document-format-supported"] || [];
    p.model = String((a["printer-make-and-model"] || [p.model])[0] || "");
    if (!p.name || p.name === p.address) { p.name = p.model || p.address; }
    p.jpeg = formats.indexOf("image/jpeg") >= 0;
    p.media = String((a["media-default"] || ["na_letter_8.5x11in"])[0]);
    p.fit = (a["print-scaling-supported"] || []).indexOf("fit") >= 0;
    onDone(p);
  });
}

// ── finding printers ─────────────────────────────────────────────────────────────────────────
// mDNS, asked FROM PORT 5353. The engine has a DNS-SD component, and it does not work on this
// platform: it asks from an ephemeral port, a printer answers such a query to that port
// directly, and the device firewall drops it — INPUT is DROP except established connections
// and `udp dpt:5353` (measured 2026-10-05: the query was seen on the LAN, the printer and a
// print server both answered a host that asked the same way, and the device heard nothing).
// A query sent from 5353 is an ordinary multicast query: the answers come to the group on
// 5353, which the firewall lets in.
const MDNS_GROUP = "224.0.0.251", MDNS_PORT = 5353;

//* A DNS name at `off` in packet `d` (compression pointers followed): {name, next}.
function dnsName(d, off) {
  let labels = [], next = -1, hops = 0;
  while (off < d.length && hops < 32) {
    let len = d[off];
    if (len === 0) { off++; break; }
    if ((len & 0xc0) === 0xc0) {
      if (next < 0) { next = off + 2; }
      off = ((len & 0x3f) << 8) | d[off + 1];
      hops++;
      continue;
    }
    let str = "";
    for (let k = 1; k <= len && off + k < d.length; k++) { str += String.fromCharCode(d[off + k]); }
    try { str = decodeURIComponent(escape(str)); } catch (e) {}     // labels are UTF-8
    labels.push(str);
    off += len + 1;
  }
  return {name: labels.join("."), next: next < 0 ? off : next};
}

//* The answer records of an mDNS response that matter here, by type:
//* {ptr: [{name, target}], srv: {name: {host, port}}, txt: {name: {key: value}}, a: {name: "1.2.3.4"}}.
//* Written out rather than borrowed from the engine's DNS packet helper, which gives up on
//* the first record type it does not know — and a printer's own answer carries several.
function mdnsParse(d) {
  let out = {ptr: [], srv: {}, txt: {}, a: {}};
  if (d.length < 12 || !(d[2] & 0x80)) { return out; }          // answers only
  let qd = (d[4] << 8) | d[5];
  let rr = ((d[6] << 8) | d[7]) + ((d[8] << 8) | d[9]) + ((d[10] << 8) | d[11]);
  let off = 12;
  for (let q = 0; q < qd && off < d.length; q++) { off = dnsName(d, off).next + 4; }
  for (let n = 0; n < rr && off + 10 <= d.length; n++) {
    let nm = dnsName(d, off); off = nm.next;
    if (off + 10 > d.length) { break; }
    let type = (d[off] << 8) | d[off + 1];
    let rdlen = (d[off + 8] << 8) | d[off + 9];
    let rd = off + 10; off = rd + rdlen;
    if (off > d.length) { break; }
    if (type === 12) {
      out.ptr.push({name: nm.name, target: dnsName(d, rd).name});
    } else if (type === 33 && rdlen >= 7) {
      out.srv[nm.name] = {port: (d[rd + 4] << 8) | d[rd + 5], host: dnsName(d, rd + 6).name};
    } else if (type === 16) {
      let kv = {}, p = rd;
      while (p < rd + rdlen) {
        let len = d[p], str = "";
        for (let k = 1; k <= len && p + k < rd + rdlen; k++) { str += String.fromCharCode(d[p + k]); }
        let eq = str.indexOf("=");
        if (eq > 0) { kv[str.substr(0, eq)] = str.substr(eq + 1); }
        p += len + 1;
      }
      out.txt[nm.name] = kv;
    } else if (type === 1 && rdlen === 4) {
      out.a[nm.name] = d[rd] + "." + d[rd + 1] + "." + d[rd + 2] + "." + d[rd + 3];
    }
  }
  return out;
}

//* A PTR question for `name`. `unicast` sets the unicast-response bit: the answer then comes
//* straight back to this socket (port 5353, which the firewall admits) instead of to the
//* group, where a sleeping radio can miss it.
function mdnsQuery(name, unicast) {
  let b = [0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0];
  name.split(".").forEach(function(label) {
    b.push(label.length);
    for (let i = 0; i < label.length; i++) { b.push(label.charCodeAt(i) & 0xff); }
  });
  b.push(0, 0, 12, unicast ? 0x80 : 0, 1);
  return new Uint8Array(b);
}

function mdnsBrowse(types, ms, onService, onDone) {
  let sock = null, timers = [], seen = {};
  function finish() {
    try { if (sock) { sock.close(); } } catch (e) {}
    sock = null;
    onDone();
  }
  function ask(unicast) {
    if (!sock) { return; }
    types.forEach(function(t) {
      let q = mdnsQuery(t + ".local", unicast);
      try { sock.send(MDNS_GROUP, MDNS_PORT, q, q.length); } catch (e) { log("mdns: send failed (" + e + ")"); }
    });
  }
  function after(delay, fn) {
    let timer = Cc["@mozilla.org/timer;1"].createInstance(Ci.nsITimer);
    timer.initWithCallback({notify: fn}, delay, Ci.nsITimer.TYPE_ONE_SHOT);
    timers.push(timer);                 // kept alive until the browse object is collected
  }
  try {
    sock = Cc["@mozilla.org/network/udp-socket;1"].createInstance(Ci.nsIUDPSocket);
    sock.init2("0.0.0.0", MDNS_PORT, Services.scriptSecurityManager.getSystemPrincipal(), true /* address reuse */);
    sock.asyncListen({
      QueryInterface: XPCOMUtils.generateQI([Ci.nsIUDPSocketListener]),
      onStopListening: function() {},
      onPacketReceived: function(aSocket, aMessage) {
        let r;
        try { r = mdnsParse(aMessage.rawData); } catch (e) { return; }
        r.ptr.forEach(function(p) {
          let type = null;
          for (let t of types) { if (p.name === t + ".local") { type = t; } }
          let svc = r.srv[p.target];
          if (!type || !svc || !svc.port || seen[p.target]) { return; }
          // The address record of the host that serves it; failing that, whoever sent the
          // packet (a printer answers for itself).
          let address = r.a[svc.host] || (aMessage.fromAddr && aMessage.fromAddr.address);
          if (!address) { return; }
          seen[p.target] = true;
          log("mdns: " + p.target + " -> " + address + ":" + svc.port + " (answer from " +
              (aMessage.fromAddr ? aMessage.fromAddr.address : "?") + ")");
          onService({
            name: String(p.target).replace("." + type + ".local", ""),
            type: type, address: address, port: svc.port, attributes: r.txt[p.target] || {}
          });
        });
      }
    });
    sock.joinMulticast(MDNS_GROUP, "0.0.0.0");
    // Asked three times, as a resolver does: once for a direct answer, twice more on the
    // group for anything that missed it.
    ask(true);
    after(500, function() { ask(false); });
    after(1500, function() { ask(false); });
    log("mdns: asked for " + types.join(", ") + " from port " + MDNS_PORT);
  } catch (e) {
    log("mdns: could not ask (" + e + ")");
    finish();
    return;
  }
  after(ms, finish);
  mdnsBrowse._timers = timers;
}

function findPrinters(request) {
  let found = {}, pending = 0, closed = false;
  // The list is pushed every time it grows, and once more when the search is over: a printer
  // that answers in a second is on screen in a second, not after the slowest one.
  function report() {
    let list = [];
    for (let k in found) { if (found[k]) { list.push(found[k]); } }
    list.sort(function(a, b) { return a.name < b.name ? -1 : a.name > b.name ? 1 : 0; });
    notify("print-printers", JSON.stringify({printers: list, done: closed && pending === 0}));
  }
  function add(p) {
    p = cleanPrinter(p);
    // One entry per queue: the same printer can be reached directly and through a print
    // server, and a print server can share several printers at one address.
    let key = p ? (p.address + ":" + p.port + "/" + p.path) : null;
    if (!p || found[key] !== undefined) { return; }
    found[key] = null;
    pending++;
    probePrinter(p, function(q) {
      pending--;
      if (q && q.jpeg) {
        found[key] = {name: q.name, model: q.model, address: q.address, port: q.port,
                      path: q.path, tls: q.tls, media: q.media, fit: q.fit};
        log("printer: " + q.name + " at " + q.address + ":" + q.port + "/" + q.path + (q.tls ? " (TLS)" : ""));
        report();
      } else {
        delete found[key];
        if (q) { log("printer " + q.address + "/" + q.path + " does not take JPEG; left out"); }
        if (closed && pending === 0) { report(); }
      }
    });
  }
  // An address typed by hand goes first: it works on a network where discovery does not.
  // "a.b.c.d" or "a.b.c.d:port".
  if (request && request.address) {
    let m = /^\s*([0-9.]+)(?::(\d{1,5}))?\s*$/.exec(String(request.address));
    if (m) { add({address: m[1], port: m[2] || 631, name: ""}); }
  }

  mdnsBrowse(["_ipps._tcp", "_ipp._tcp"], FIND_MS, function(svc) {
    let a = svc.attributes || {};
    // TLS is tried first whichever service type announced the queue (a printer that
    // announces plain IPP may still refuse it: the Epson answers "426 Upgrade Required");
    // probePrinter falls back to the clear by itself.
    add({name: svc.name, model: a.ty || "", address: svc.address, port: svc.port,
         path: a.rp || "ipp/print", tls: true});
  }, function() {
    closed = true;
    if (pending === 0) { report(); }
  });
}

// ── rendering and sending ────────────────────────────────────────────────────────────────────
// Paper shapes by IPP media name; anything unknown is treated as Letter.
function paperAspect(media) {
  let m = /_(\d+(?:\.\d+)?)x(\d+(?:\.\d+)?)(in|mm)$/.exec(String(media || ""));
  if (m && Number(m[1]) > 0) { return Number(m[2]) / Number(m[1]); }
  return 11 / 8.5;
}

function printWindow(win, printer, dryRun) {
  let doc = win.document, root = doc.documentElement, body = doc.body;
  let width = Math.max(root.scrollWidth, body ? body.scrollWidth : 0, win.innerWidth);
  let height = Math.max(root.scrollHeight, body ? body.scrollHeight : 0, win.innerHeight);
  if (!(width > 0 && height > 0)) { notify("print-error", "There is no page to print."); return; }
  // One slice is one sheet: as wide as the page's layout, as tall as the paper's shape allows.
  let sliceH = Math.floor(width * paperAspect(printer.media));
  let pages = Math.ceil(height / sliceH);
  let truncated = pages > MAX_PAGES;
  if (truncated) { pages = MAX_PAGES; }
  // Drawn at up to twice the layout size (about 190 dpi on Letter for a 768 px layout): sharp
  // text without a canvas so large the device cannot hold it.
  let scale = Math.min(2, Math.max(1, 1600 / width));
  let title = String(doc.title || win.location.href || "Web page").substr(0, 100);
  let canvas = doc.createElementNS("http://www.w3.org/1999/xhtml", "canvas");
  canvas.width = Math.round(width * scale);
  canvas.height = Math.round(sliceH * scale);
  let ctx = canvas.getContext("2d");
  let uri = printerUri(printer);
  log("job: \"" + title + "\" " + width + "x" + height + " css px -> " + pages + " sheet(s) of " +
      canvas.width + "x" + canvas.height + " to " + uri);

  let page = 0;
  function next() {
    if (page >= pages) {
      notify("print-done", (dryRun ? "Dry run: would send " : "Sent ") + pages + (pages === 1 ? " page" : " pages") + " to " + printer.name + "." +
             (truncated ? " The page is longer; only the first " + MAX_PAGES + " sheets were printed." : ""));
      return;
    }
    notify("print-progress", "Printing page " + (page + 1) + " of " + pages + "…");
    let bytes, t0 = Date.now(), tDrawn = 0;
    try {
      ctx.setTransform(1, 0, 0, 1, 0, 0);
      ctx.fillStyle = "#ffffff";
      ctx.fillRect(0, 0, canvas.width, canvas.height);
      ctx.scale(scale, scale);
      let h = Math.min(sliceH, height - page * sliceH);
      ctx.drawWindow(win, 0, page * sliceH, width, h, "rgb(255,255,255)");
      tDrawn = Date.now();
      let url = canvas.toDataURL("image/jpeg", 0.9);
      let bin = win.atob(url.substr(url.indexOf(",") + 1));
      bytes = new Uint8Array(bin.length);
      for (let i = 0; i < bin.length; i++) { bytes[i] = bin.charCodeAt(i); }
    } catch (e) {
      log("render of sheet " + (page + 1) + " threw: " + e);
      notify("print-error", "The page could not be drawn for printing.");
      return;
    }
    let jobAttrs = [[0x21, "copies", 1]];
    if (printer.fit) { jobAttrs.push([0x44, "print-scaling", "fit"]); }
    let body = ippRequest(0x0002, 100 + page, uri, [
      [0x42, "requesting-user-name", "Jihad Browser"],
      [0x42, "job-name", title + (pages > 1 ? " (" + (page + 1) + "/" + pages + ")" : "")],
      [0x49, "document-format", "image/jpeg"]
    ], jobAttrs, bytes);
    let tSent = Date.now();
    log("sheet " + (page + 1) + "/" + pages + ": " + bytes.length + " bytes of JPEG, drawn in " + (tDrawn - t0) +
        " ms, encoded in " + (tSent - tDrawn) + " ms" + (dryRun ? " (dry run, not sent)" : ""));
    if (dryRun) { page++; next(); return; }
    ippPost(printer, body, function(resp, err) {
      if (!resp || resp.status >= 0x0100) {
        let why = err || ("the printer refused the job (IPP status 0x" + resp.status.toString(16) + ")");
        log("sheet " + (page + 1) + " failed: " + why);
        notify("print-error", "Printing stopped at page " + (page + 1) + ": " + why + ".");
        return;
      }
      log("sheet " + (page + 1) + " accepted after " + (Date.now() - tSent) + " ms, job-id " + ((resp.attrs["job-id"] || ["?"])[0]));
      page++;
      next();
    });
  }
  next();
}

function JihadPrint() {
  Services.obs.addObserver(this, "jihad-print-find", false);
  Services.obs.addObserver(this, "jihad-print-job", false);
}

JihadPrint.prototype = {
  classID: Components.ID("{b7f1a6f0-0c5e-4d0b-9a55-3f6f2f7c1d42}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIObserver]),

  observe: function(aSubject, aTopic, aData) {
    let req = null;
    try { req = JSON.parse(aData || "{}"); } catch (e) { req = {}; }
    try {
      if (aTopic === "jihad-print-find") {
        if (req.address && !isLocalAddress(String(req.address).replace(/:\d+\s*$/, "").trim())) {
          notify("print-error", "That is not an address on this network.");
          req.address = "";
        }
        findPrinters(req);
      } else if (aTopic === "jihad-print-job") {
        let printer = cleanPrinter(req.printer);
        if (!printer) { notify("print-error", "No printer was chosen."); return; }
        let win = aSubject;
        if (!win || !win.document) { notify("print-error", "There is no page to print."); return; }
        let dryRun = req.dryRun === true;
        log("request: " + (dryRun ? "DRY RUN (nothing is sent)" : "print") + " to " + printerUri(printer));
        // Ask the printer again rather than trust what the card sent back: its paper and
        // whether it scales to fit decide how the sheets are cut.
        probePrinter(printer, function(p) {
          if (!p || !p.jpeg) { notify("print-error", "The printer did not answer."); return; }
          // `dryRun` draws and encodes every sheet and sends none: the way to check how a
          // page is cut without using paper.
          printWindow(win, p, dryRun);
        });
      }
    } catch (e) {
      log(aTopic + " threw: " + e);
      notify("print-error", "Printing failed.");
    }
  }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadPrint]);
