/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — "Desktop site" for one tab (T-645, the owner's choice from the Pale Moon for
 * Android research, 2026-11-11).
 *
 * WHAT IT DOES. A tab in desktop mode says it is a desktop Firefox to every site, whatever the
 * site's own rule or learned identity says (components/jihadUserAgent.js: YouTube is given a
 * MOBILE identity by rule, and the default identity names webOS and the TouchPad, which some
 * sites answer with their phone pages). The identity is the one a learned site gets
 * (jihad.ua.modern), or jihad.ua.desktop if that is set. A tab that is not in desktop mode is
 * as it always was.
 *
 * HOW A CARD ASKS, WITHOUT A NEW ADAPTER CALL OR YAP MESSAGE. By opening an address, the one
 * thing every shell already does:
 *
 *     about:desktopsite?on=1&url=<encoded http(s) address>      (on=0 leaves desktop mode)
 *
 * This component is that about: module. The load's nsILoadInfo names the tab (the outer window
 * the document is loading into), so the mode is recorded for THAT tab, and the channel handed
 * back is the address's own, marked LOAD_REPLACE — the way the toolkit's redirector serves an
 * about: page that lives on the web. The tab therefore loads the address itself: the document,
 * its principal, the address the card is told and the session-history entry are all the
 * site's, and nothing of about:desktopsite is left to go Back to. Going Back or reloading
 * later loads the entry's own address (docshell keeps the about: address only as the entry's
 * original URI), so an old entry cannot switch the mode again.
 *
 * It is not URI_SAFE_FOR_UNTRUSTED_CONTENT: a page cannot link or redirect to it.
 *
 * THE TAB'S IDENTITY, header and script together (as jihadUserAgent.js keeps them):
 *   - http-on-modify-request: every request of a desktop tab — the document, its frames and
 *     what they load — carries the desktop identity.
 *   - document-element-inserted: the document's docShell is given, as its customUserAgent
 *     (what navigator.userAgent answers), the User-Agent its own request carried. Read off
 *     the request, not assumed: the daemon's per-site table (EngineHost.cpp) may still have
 *     the last word on the header for the hosts it lists, all of them desktop Firefox tokens,
 *     and the page's script must see what the server saw.
 *     The first document after LEAVING desktop mode is treated the same way, once: the
 *     daemon pinned the docShell for the about: address it was asked to open, not for the
 *     site's, and its table's hosts would otherwise read the default identity in script.
 *
 * ORDER. The observer service notifies the LATEST registered first. These observers are
 * added the first time a tab enters desktop mode, so they would be told before
 * jihadUserAgent.js's and its site rule would then overwrite what they set. That component's
 * two observers are therefore taken off and put back on at that moment: later registered,
 * earlier told, and this one's word stands for a desktop tab.
 *
 * The mode lives with the daemon and ends with the tab (outer-window-destroyed).
 *
 * WHERE IT IS REGISTERED: packaging/prefsui/jihad-prefsui.manifest.
 */

"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu, results: Cr } = Components;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");

const DEFAULT_DESKTOP = "Mozilla/5.0 (X11; Linux armv7l; rv:128.0) Gecko/20100101 Firefox/128.0";

function log(s) { try { Services.console.logStringMessage("site mode: " + s); } catch (e) {} }
function charPref(n) { try { return Services.prefs.getCharPref(n); } catch (e) { return ""; } }
function desktopIdentity() { return charPref("jihad.ua.desktop") || charPref("jihad.ua.modern") || DEFAULT_DESKTOP; }

let gDesktop = new Set();   // outer window ids of the tabs in desktop mode
let gResync = new Set();    // tabs that just left it: their next document's script identity is re-read
let gWatching = false;

function idOf(win) {
  return win.QueryInterface(Ci.nsIInterfaceRequestor).getInterface(Ci.nsIDOMWindowUtils).outerWindowID;
}
function docShellOf(win) {
  return win.QueryInterface(Ci.nsIInterfaceRequestor).getInterface(Ci.nsIWebNavigation).QueryInterface(Ci.nsIDocShell);
}
//* The tab a request belongs to (its top window's id), or 0: a request with no window is no tab's.
function tabOf(ch) {
  try {
    let cb = ch.notificationCallbacks || (ch.loadGroup && ch.loadGroup.notificationCallbacks);
    if (cb) { return idOf(cb.getInterface(Ci.nsILoadContext).topWindow); }
  } catch (e) {}
  try {
    let doc = ch.loadInfo && ch.loadInfo.loadingDocument;
    if (doc && doc.defaultView) { return idOf(doc.defaultView.top); }
  } catch (e) {}
  return 0;
}

let gObserver = {
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIObserver]),
  observe: function (subject, topic) {
    if (topic === "http-on-modify-request") {
      if (!gDesktop.size) { return; }
      try {
        let ch = subject.QueryInterface(Ci.nsIHttpChannel);
        if (gDesktop.has(tabOf(ch))) { ch.setRequestHeader("User-Agent", desktopIdentity(), false); }
      } catch (e) {}
    } else if (topic === "document-element-inserted") {
      if (!gDesktop.size && !gResync.size) { return; }
      try {
        let uri = subject.documentURIObject, win = subject.defaultView;
        if (!win || !uri || (uri.scheme !== "http" && uri.scheme !== "https")) { return; }
        let tab = idOf(win.top);
        let on = gDesktop.has(tab);
        let resync = !on && win === win.top && gResync.delete(tab);
        if (!on && !resync) { return; }
        let ds = docShellOf(win), ua = "";
        try { ua = ds.currentDocumentChannel.QueryInterface(Ci.nsIHttpChannel).getRequestHeader("User-Agent"); } catch (e) {}
        if (!ua && on) { ua = desktopIdentity(); }
        if (ua && ds.customUserAgent !== ua) { ds.customUserAgent = ua; }
      } catch (e) {}
    } else if (topic === "outer-window-destroyed") {
      try {
        let id = subject.QueryInterface(Ci.nsISupportsPRUint64).data;
        gDesktop.delete(id); gResync.delete(id);
      } catch (e) {}
    }
  }
};

//* See ORDER in the header.
function watch() {
  if (gWatching) { return; }
  gWatching = true;
  for (let topic of ["http-on-modify-request", "document-element-inserted", "outer-window-destroyed"]) {
    Services.obs.addObserver(gObserver, topic, false);
  }
  let identity = null;
  try { identity = Cc["@riverstonerelay.net/jihad/user-agent;1"].getService(Ci.nsISupports).wrappedJSObject; } catch (e) {}
  if (!identity) { return; }
  for (let topic of ["http-on-modify-request", "document-element-inserted"]) {
    try { Services.obs.removeObserver(identity, topic); } catch (e) { continue; }   // it was not observing: leave it so
    try { Services.obs.addObserver(identity, topic, false); } catch (e) { log("could not put the identity's observer back (" + topic + "): " + e); }
  }
}

function setMode(tab, on) {
  if (on) {
    watch();
    gResync.delete(tab);
    gDesktop.add(tab);
  } else if (gDesktop.delete(tab)) {
    gResync.add(tab);
  }
  log("tab " + tab + (on ? ": desktop site" : ": the site's own choice") + " (" + gDesktop.size + " desktop tab(s))");
}

function JihadSiteMode() { this.wrappedJSObject = this; }

JihadSiteMode.prototype = {
  classID: Components.ID("{2c9e6f41-7b3a-4d85-9e10-6a4b8d2f5c37}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIAboutModule]),

  newChannel: function (aURI, aLoadInfo) {
    let query = String(aURI.path || "").replace(/^[^?]*\??/, "").replace(/#.*$/, "");
    let on = /(?:^|&)on=1(?:&|$)/.test(query);
    let m = /(?:^|&)url=([^&]*)/.exec(query), url = "";
    try { url = m ? decodeURIComponent(m[1]) : ""; } catch (e) { url = ""; }
    if (!/^https?:\/\//i.test(url)) { throw Cr.NS_ERROR_MALFORMED_URI; }
    let target = Services.io.newURI(url, null, null);
    // Only the tab's own load switches the tab: not a frame's, not a request with no window.
    let tab = 0;
    try {
      if (aLoadInfo && aLoadInfo.externalContentPolicyType === Ci.nsIContentPolicy.TYPE_DOCUMENT) { tab = aLoadInfo.outerWindowID; }
    } catch (e) {}
    if (tab) { setMode(tab, on); } else { log("no tab for this load; the address is opened as it is"); }
    let channel = aLoadInfo ? Services.io.newChannelFromURIWithLoadInfo(target, aLoadInfo)
                            : Services.io.newChannelFromURI(target);
    // the document is the site's, under the site's address (see the header)
    channel.loadFlags |= Ci.nsIChannel.LOAD_REPLACE;
    channel.originalURI = aURI;
    return channel;
  },

  getURIFlags: function (aURI) {
    return Ci.nsIAboutModule.ALLOW_SCRIPT | Ci.nsIAboutModule.HIDE_FROM_ABOUTABOUT;
  },

  // for a settings page or a test (wrappedJSObject)
  state: function () { return {desktopTabs: Array.from(gDesktop), identity: desktopIdentity()}; }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadSiteMode]);
