/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — following webOS's own proxy setting (cavekit-proxies-certs.md R1).
 *
 * webOS keeps one proxy setting for the whole device in com.palm.connectionmanager
 * (getNwProxiesConfig, since webOS 2.2.4; apps such as webOS Archive Proxy set it). The engine's
 * own "system proxy" reads GNOME's settings, which this headless build does not have, so it went
 * direct. When the user chooses "Use webOS's proxy setting" (jihad.proxy.mode = "webos"), this
 * component writes what webOS says into the engine's network.proxy.* prefs:
 *
 *     manualProxy            that host and port, for HTTP and HTTPS
 *     autoConfigUrl          that PAC file
 *     autoDetectFromNetwork  the engine's WPAD
 *     noProxy, or nothing    direct
 *
 * The daemon reads the setting on its own Luna connection when it starts and whenever a card
 * opens, and hands the service's JSON over as the observer topic "jihad-webos-proxy"; this
 * component asks for a fresh read with "jihad-webos-proxy-refresh" (Settings does, through
 * refresh()). If the service cannot be reached, Jihad goes direct and Settings says why.
 *
 * Every other mode is written by the settings page itself; this component leaves the prefs alone
 * unless the mode is "webos".
 */

"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");

const PREF_MODE = "jihad.proxy.mode";

function log(s) {
  try { Services.console.logStringMessage("proxy: " + s); } catch (e) {}
}

function charPref(name) {
  try { return Services.prefs.getCharPref(name); } catch (e) { return ""; }
}

// The entry that applies to this device. A TouchPad has only Wi-Fi; webOS writes "default" for
// the whole device and may also keep per-technology entries, so "default" wins, then "wifi",
// then the first there is.
function pickEntry(list) {
  if (!Array.isArray(list) || !list.length) { return null; }
  let by = function (t) {
    return list.filter(function (e) { return e && String(e.networkTechnology || "default") === t; })[0];
  };
  return by("default") || by("wifi") || list[0];
}

const HOST_RE = /^(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)(?:\.[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)*$|^\d{1,3}(?:\.\d{1,3}){3}$|^\[?[0-9A-Fa-f:]+\]?$/;

// What to set, from one entry: {type, host, port, pac, said} or {error}.
function plan(entry) {
  if (!entry) { return {type: 0, said: "webOS has no proxy set: direct."}; }
  let kind = String(entry.proxyConfigType || "noProxy");
  if (kind === "manualProxy") {
    let host = String(entry.proxyServer || "").trim(), port = parseInt(entry.proxyPort, 10);
    if (!HOST_RE.test(host) || !(port >= 1 && port <= 65535)) {
      return {error: "webOS's proxy setting names no usable host and port (" + host + ":" + entry.proxyPort + ")."};
    }
    return {type: 1, host: host.replace(/^\[|\]$/g, ""), port: port, said: "webOS's proxy: " + host + ":" + port + "."};
  }
  if (kind === "autoConfigUrl") {
    let url = String(entry.proxyAutoConfigUrl || "").trim();
    if (!/^(https?|file):\/\//i.test(url)) { return {error: "webOS's automatic proxy setting has no usable address."}; }
    return {type: 2, pac: url, said: "webOS's automatic proxy configuration: " + url + "."};
  }
  if (kind === "autoDetectFromNetwork") { return {type: 4, said: "webOS finds the proxy on the network (WPAD)."}; }
  return {type: 0, said: "webOS has no proxy set: direct."};
}

function JihadProxy() {
  this.wrappedJSObject = this;
  this._answer = null;       // the service's last JSON, parsed
  this._said = "";           // what Settings shows
  this._error = "";
  this._at = 0;
  Services.obs.addObserver(this, "jihad-webos-proxy", false);
  Services.prefs.addObserver(PREF_MODE, this, false);
}

JihadProxy.prototype = {
  classID: Components.ID("{d47b9c02-6e15-4a8f-93b1-5c2e8f0a7d14}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIObserver]),

  observe: function (subject, topic, data) {
    try {
      if (topic === "jihad-webos-proxy") {
        this._at = Date.now();
        try { this._answer = JSON.parse(String(data || "")); } catch (e) { this._answer = {returnValue: false, errorText: "an answer that is not JSON"}; }
        this.apply();
      } else if (topic === "nsPref:changed" && charPref(PREF_MODE) === "webos") {
        this.apply();
        this.refresh();
      }
    } catch (e) {
      log(topic + " threw: " + e);
    }
  },

  refresh: function () {
    Services.obs.notifyObservers(null, "jihad-webos-proxy-refresh", null);
  },

  apply: function () {
    let a = this._answer;
    let p;
    if (!a) {
      p = {type: 0, said: "", error: "webOS's proxy setting has not been read yet."};
    } else if (a.returnValue === false) {
      p = {type: 0, error: "webOS's proxy setting could not be read (" + String(a.errorText || "no reason given") + "), so Jihad goes direct."};
    } else {
      p = plan(pickEntry(a.proxyInfoList));
      if (p.error) { p.type = 0; p.error += " Jihad goes direct."; }
    }
    this._said = p.said || "";
    this._error = p.error || "";
    if (charPref(PREF_MODE) !== "webos") { return; }
    this._write(p);
  },

  // Only what changes is written, so a re-read that says the same thing leaves prefs.js alone.
  _write: function (p) {
    let P = Services.prefs, changed = false;
    let setInt = function (n, v) { let cur; try { cur = P.getIntPref(n); } catch (e) { cur = null; } if (cur !== v) { P.setIntPref(n, v); changed = true; } };
    let setChar = function (n, v) { if (charPref(n) !== v) { P.setCharPref(n, v); changed = true; } };
    if (p.type === 1) {
      setChar("network.proxy.http", p.host); setInt("network.proxy.http_port", p.port);
      setChar("network.proxy.ssl", p.host); setInt("network.proxy.ssl_port", p.port);
      setChar("network.proxy.socks", "");
    } else if (p.type === 2) {
      setChar("network.proxy.autoconfig_url", p.pac);
    }
    setInt("network.proxy.type", p.type);
    if (changed) { log("following webOS: " + (p.said || p.error)); }
  },

  state: function () {
    return {mode: charPref(PREF_MODE), said: this._said, error: this._error, at: this._at};
  }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadProxy]);
