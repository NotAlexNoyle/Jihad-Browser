/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — about:preferences / about:settings.
 *
 * WHY A COMPONENT AND NOT A DAEMON CHANGE: the engine's own about: table
 * (render/goanna/BrowserPageGoanna.cpp jihadAboutPage) serves about:jihad and
 * about:isis as inline HTML, which is right for two static credit pages and
 * wrong for a settings UI that has to read and write prefs. An nsIAboutModule
 * registered from a JS component — the same hook the toolkit's own about: pages
 * use — resolves to a chrome:// document instead, which runs with the system
 * principal and can therefore reach Services.prefs directly. It also needs no
 * C++ rebuild: this file and its chrome package are plain data in the GRE
 * bundle, next to jihadInstallPrompt.js.
 *
 * WHAT IT SERVES: Pale Moon organises its settings as a prefwindow of panes
 * (General / Content / Privacy / Security / Advanced); Basilisk serves the same
 * material in-content at about:preferences. This embedding has no chrome window
 * to host a prefwindow, so it follows Basilisk's shape with Pale Moon's panes —
 * see chrome/jihad-prefs/content/preferences.js for the pane table and for which
 * of Pale Moon's panes are deliberately absent.
 *
 * about:settings is registered as a SECOND contract on the same class, so the
 * name Basilisk users type and the name Pale Moon users type both land here.
 */
"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");

const PAGE_URL = "chrome://jihad-prefs/content/preferences.html";

function JihadAboutPreferences() {}

JihadAboutPreferences.prototype = {
  classID: Components.ID("{0f2b8f4e-6c31-4a7b-9f2d-5c1e73a4b806}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIAboutModule]),

  newChannel: function(aURI, aLoadInfo) {
    let uri = Services.io.newURI(PAGE_URL, null, null);
    // newChannelFromURIWithLoadInfo keeps the caller's loadInfo, which is what
    // ESR52 expects from an about: module; the older two-argument form is gone.
    let channel = aLoadInfo
      ? Services.io.newChannelFromURIWithLoadInfo(uri, aLoadInfo)
      : Services.io.newChannelFromURI(uri);
    // So the address bar and document.location keep saying about:preferences
    // (or about:settings) rather than leaking the chrome:// path.
    channel.originalURI = aURI;
    return channel;
  },

  getURIFlags: function(aURI) {
    // Deliberately NOT URI_SAFE_FOR_UNTRUSTED_CONTENT: this page writes prefs,
    // so it must keep the chrome document's system principal, and a web page
    // must not be able to link or redirect into it.
    return Ci.nsIAboutModule.ALLOW_SCRIPT;
  }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadAboutPreferences]);
