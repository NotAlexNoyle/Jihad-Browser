/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — about:reader?url=<encoded http(s) address> (T-642, the owner's choice from
 * the Pale Moon for Android research, 2026-11-11): the page's article alone, without the page.
 *
 * WHY THIS FILE EXISTS. UXP ships the reader's parts (toolkit/components/reader: Readability,
 * its worker, ReaderMode.jsm, AboutReader.jsm, aboutReader.html) and nothing that puts them
 * together: the about:reader module is the APPLICATION's (browser/ and mobile/ register it,
 * toolkit's redirector does not), and the page is filled by a frame script (AboutReaderListener
 * in the application's tab-content.js) over a message manager this embedding has not got. So
 * about:reader resolved to nothing. This component is both halves:
 *
 *   1. the about: module. about:reader is chrome://jihad-prefs/content/reader.html, served
 *      with URI_SAFE_FOR_UNTRUSTED_CONTENT and WITHOUT MAKE_LINKABLE — the two flags Firefox
 *      gives it. The first makes the document UNPRIVILEGED (nsAboutProtocolHandler drops the
 *      chrome channel's system owner, so the principal is about:reader's own): what is shown
 *      in it is a web page's markup, and it must not be shown in a document that can write
 *      prefs. The second keeps web content from linking or redirecting into it.
 *   2. the filler. The article is fetched and parsed HERE, with the system principal, and put
 *      into that document from outside (it has no script of its own): an XMLHttpRequest for
 *      the address as a document — which runs none of the page's script, loads none of its
 *      style sheets, frames or images; that is what makes this the fast way to read an
 *      article on this device — Readability in the toolkit's worker, and the result through
 *      the engine's sanitizer (nsIParserUtils: no script, no event handlers, no forms, no
 *      style) into the page.
 *
 * NOT ReaderMode.downloadAndParseDocument: it refuses a document of more than
 * reader.parse-node-limit elements, 3000 in this engine's defaults (Android's figure), which
 * is less than an ordinary Wikipedia article has. The same steps are done here with a limit
 * of this browser's own (jihad.reader.nodeLimit, 30000; 0 is none).
 *
 * WHERE IT IS REGISTERED: packaging/prefsui/jihad-prefsui.manifest, beside the chrome package
 * the page is in.
 */

"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu, results: Cr } = Components;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");
Cu.importGlobalProperties(["XMLHttpRequest"]);

XPCOMUtils.defineLazyModuleGetter(this, "ReaderWorker", "resource://gre/modules/reader/ReaderWorker.jsm");

const PAGE_URL = "chrome://jihad-prefs/content/reader.html";
const PREF_NODE_LIMIT = "jihad.reader.nodeLimit";
const DEFAULT_NODE_LIMIT = 30000;
const FETCH_TIMEOUT_MS = 60000;

function log(s) { try { Services.console.logStringMessage("reader: " + s); } catch (e) {} }

//* The address an about:reader url asks for, or "" unless it is an http(s) one.
function originalOf(spec) {
  let m = /^about:reader\?(?:[^#]*&)?url=([^&#]*)/i.exec(String(spec || ""));
  if (!m) { return ""; }
  let url = "";
  try { url = decodeURIComponent(m[1]); } catch (e) { return ""; }
  return /^https?:\/\//i.test(url) ? url : "";
}

//* The address as a document. Nothing in it runs or loads: an XMLHttpRequest's document is data.
//* `identity` is the tab's own (its navigator.userAgent): a request with no window would go out
//* with the engine's bare default, which is no identity of this browser's (read on the wire,
//* host build, 2026-11-11). The per-site identities still apply after it: they go by the host.
function download(url, identity) {
  return new Promise(function (resolve, reject) {
    let xhr = new XMLHttpRequest();
    xhr.open("GET", url, true);
    if (identity) { try { xhr.setRequestHeader("User-Agent", identity); } catch (e) {} }
    // no window to ask for a password or about a certificate: such a page fails here, and the
    // reader says so, with the link to the page itself
    xhr.mozBackgroundRequest = true;
    xhr.responseType = "document";
    xhr.timeout = FETCH_TIMEOUT_MS;
    xhr.onerror = function () { reject(new Error("the page could not be loaded")); };
    xhr.ontimeout = function () { reject(new Error("the page took too long to load")); };
    xhr.onload = function () {
      if (xhr.status < 200 || xhr.status > 299) { reject(new Error("the site answered " + xhr.status)); return; }
      let doc = xhr.responseXML;
      if (!doc || !doc.documentElement) { reject(new Error("the address is not a web page")); return; }
      resolve(doc);
    };
    xhr.send();
  });
}

//* Readability over the document: in the toolkit's worker; on this thread if the worker cannot
//* be had (the same script, on the document itself).
function parse(doc) {
  let limit = DEFAULT_NODE_LIMIT;
  try { limit = Services.prefs.getIntPref(PREF_NODE_LIMIT); } catch (e) {}
  if (limit > 0 && doc.getElementsByTagName("*").length > limit) {
    return Promise.reject(new Error("the page is too large for the reader"));
  }
  let base = doc.baseURIObject;
  let uri = {
    spec: base.spec, host: base.host, prePath: base.prePath, scheme: base.scheme,
    pathBase: Services.io.newURI(".", null, base).spec
  };
  let text = Cc["@mozilla.org/xmlextras/xmlserializer;1"].createInstance(Ci.nsIDOMSerializer).serializeToString(doc);
  return ReaderWorker.post("parseDocument", [uri, text, {}]).then(null, function (e) {
    log("the worker failed (" + e + "); parsing on the main thread");
    let scope = {};
    Services.scriptloader.loadSubScript("resource://gre/modules/reader/Readability.js", scope);
    return new scope.Readability(doc, {}).parse();
  });
}

function setText(doc, id, text) {
  let el = doc.getElementById(id);
  if (el) { el.textContent = text || ""; }
  return el;
}

function fail(doc, why) {
  setText(doc, "reader-status", "This page could not be shown in the reader: " + why + ".");
  doc.body.className = "failed";
}

//* Put the article into the about:reader document. The markup goes through the sanitizer, with
//* the article's own address as its base; everything else is set as text.
function show(doc, article, url) {
  let uri = Services.io.newURI(url, null, null);
  doc.title = article.title || uri.host;
  setText(doc, "reader-title", article.title);
  setText(doc, "reader-credits", article.byline);
  let content = doc.getElementById("reader-content");
  if (article.dir) { content.setAttribute("dir", article.dir); }
  let utils = Cc["@mozilla.org/parserutils;1"].getService(Ci.nsIParserUtils);
  content.appendChild(utils.parseFragment(article.content, Ci.nsIParserUtils.SanitizerDropForms, false, uri, content));
  setText(doc, "reader-status", "");
  doc.body.className = "loaded";
}

function fill(doc) {
  let url = originalOf(doc.documentURI);
  if (!url) { fail(doc, "the reader shows http and https pages only"); return; }
  let host = "";
  try { host = Services.io.newURI(url, null, null).host; } catch (e) {}
  for (let id of ["reader-domain", "reader-original"]) {
    let a = doc.getElementById(id);
    if (a) { a.setAttribute("href", url); }
  }
  setText(doc, "reader-domain", host);
  let started = Date.now();
  let final = url;
  // the document may be gone before the article is here (the user left the page)
  let gone = function () { try { return Cu.isDeadWrapper(doc) || !doc.defaultView; } catch (e) { return true; } };
  let identity = "";
  try { identity = doc.defaultView.navigator.userAgent; } catch (e) {}
  download(url, identity).then(function (page) {
    try { final = page.documentURIObject.specIgnoringRef || url; } catch (e) {}
    return parse(page);
  }).then(function (article) {
    if (gone()) { return; }
    if (!article || !article.content) { fail(doc, "no article was found on it"); return; }
    show(doc, article, /^https?:\/\//i.test(final) ? final : url);
    log(host + ": " + (article.length || 0) + " characters in " + (Date.now() - started) + " ms");
  }).then(null, function (e) {
    log(host + ": " + e);
    if (!gone()) { fail(doc, (e && e.message) || "it could not be read"); }
  });
}

let gWatching = false;
let gObserver = {
  observe: function (doc, topic) {
    try {
      if (!doc || !doc.documentURI || doc.documentURI.slice(0, 13).toLowerCase() !== "about:reader?") { return; }
      // only the page this module serves: the unprivileged about:reader document
      if (!doc.nodePrincipal || Services.scriptSecurityManager.isSystemPrincipal(doc.nodePrincipal)) { return; }
      doc.addEventListener("DOMContentLoaded", function once() {
        doc.removeEventListener("DOMContentLoaded", once, false);
        try { fill(doc); } catch (e) { log("fill: " + e); }
      }, false);
    } catch (e) {}
  }
};

function JihadAboutReader() {}

JihadAboutReader.prototype = {
  classID: Components.ID("{b7a1c3e2-5d94-4f08-8a6b-3e2f9c7d1a54}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIAboutModule]),

  newChannel: function (aURI, aLoadInfo) {
    // The filler is installed by the first about:reader load: nothing of the reader is
    // loaded, and nothing observes documents, in a session that never opens it.
    if (!gWatching) { Services.obs.addObserver(gObserver, "document-element-inserted", false); gWatching = true; }
    let uri = Services.io.newURI(PAGE_URL, null, null);
    let channel = aLoadInfo ? Services.io.newChannelFromURIWithLoadInfo(uri, aLoadInfo)
                            : Services.io.newChannelFromURI(uri);
    // the document's address and its principal are about:reader's, not the chrome file's
    channel.originalURI = aURI;
    return channel;
  },

  getURIFlags: function (aURI) {
    return Ci.nsIAboutModule.ALLOW_SCRIPT | Ci.nsIAboutModule.URI_SAFE_FOR_UNTRUSTED_CONTENT |
           Ci.nsIAboutModule.HIDE_FROM_ABOUTABOUT;
  }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadAboutReader]);
