/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — XPI web-install confirmation (browser-services R3).
 *
 * The toolkit's default install confirm is a modal XUL chrome window
 * (amWebInstallListener.js), which the headless daemon cannot open — and its
 * failure path CANCELS every install. amWebInstallListener checks for an
 * application-provided "@mozilla.org/addons/web-install-prompt;1" FIRST, so this
 * component takes that hook and routes the decision out to the CARD:
 *
 *   confirm() → notifyObservers("jihad-xpi-confirm", bag)      (synchronous)
 *     → DialogService's C++ observer → DialogSink (YAP msgDialogConfirm)
 *     → the variant's own card dialog (Enyo popup / Mochi kind / Mojo dialog)
 *     → reply written back into the property bag → install() or cancel().
 *
 * The daemon side stays framework-agnostic (impl-r8-palemoon-basilisk.md,
 * "follow Atlas: prompts are card-side dialogs").
 */
"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");

// R3, the "clear reason" half. An XPI whose targetApplication does not match this app never
// reaches confirm() at all: amWebInstallListener.checkAllDownloaded classifies it as failed on
// `install.addon.appDisabled` (:113), calls install.cancel() on it (:147), and only THEN notifies
// "addon-install-failed" (:150). cancel() from STATE_DOWNLOADED (XPIProvider.jsm:5014-5019) fires
// onDownloadCancelled SYNCHRONOUSLY on the per-install listener addonManager.js:110-113 attached
// for the page callback, which maps it unconditionally to the legacy XPInstall code
// USER_CANCELLED (-210). So the number the PAGE gets is decided one step BEFORE the notification
// that carries the reason, and it cannot be changed from here.
//
// READ out of the source, not assumed: this UXP tree has NO incompatibility error enum at all.
// AddonManager.jsm:2563-2574 defines only ERROR_NETWORK_FAILURE(-1), ERROR_INCORRECT_HASH(-2),
// ERROR_CORRUPT_FILE(-3), ERROR_FILE_ACCESS(-4), ERROR_JETPACKSDK_FILE(-8), ERROR_WEBEXT_FILE(-9),
// and install.error stays 0 for an appDisabled add-on; Mozilla's ERROR_INCOMPATIBLE postdates this
// fork. The only signal is on the ADDON: appDisabled / isCompatible / isPlatformCompatible.
// Upstream Firefox reports the same -210 for this case and gets its "clear reason" entirely from
// the addon-install-failed observer, which its chrome renders as a notification bar. We have no
// chrome and nothing observed that topic at all, so the refusal was silent to the USER as well as
// generic to the page. This observer is that missing half.
function JihadInstallPrompt() {
  // Registered in the CONSTRUCTOR, not lazily on the first confirm(): the incompatible case is
  // refused before any prompt is reached, so nothing would ever construct this component in time.
  // DialogService.cpp forces it into existence at daemon start for exactly that reason.
  try {
    Services.obs.addObserver(this, "addon-install-failed", false);
    // "add-on installed" — cavekit-gre-widgets.md R5's first named example, and it had no
    // emitter at all before 2026-08-15. amWebInstallListener raises this once every download
    // in a batch has finished installing, with info.installs = the ones that SUCCEEDED
    // (amWebInstallListener.js:220, `this.installed`). Same constructor for the same reason:
    // an install started from a page must not depend on something else having built us first.
    Services.obs.addObserver(this, "addon-install-complete", false);
  } catch (e) {}
}

JihadInstallPrompt.prototype = {
  classID: Components.ID("{5cf8e2a6-91a1-44f5-9d33-8ab6f2c40d17}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.amIWebInstallPrompt, Ci.nsIObserver]),

  // One line of text to the card over the NON-BLOCKING channel (cavekit-gre-widgets.md R5).
  //
  // ~~This used to go through "@mozilla.org/prompter;1" -> JihadPrompter::Alert -> DialogSink
  // -> msgDialogAlert -> the variant's own modal dialog.~~ MOVED 2026-08-15 (T-148). That path
  // BLOCKS the daemon in awaitDialogReply until the card answers or the deadline expires, and
  // both messages this component raises are statements of fact with exactly one possible
  // response — an add-on either installed or it did not, and no answer changes anything. A
  // modal was buying nothing and could stall a render for the length of the dialog deadline.
  //
  // "jihad-notify" is observed by DialogService.cpp, which hands the pair to
  // jihad::PostNotification. That returns immediately whether or not any card is listening, so
  // this function cannot park the install flow. Still deliberately NOT Services.prompt
  // ("@mozilla.org/embedcomp/prompt-service;1", nsPrompter.js) — that is not overridden and
  // would try to open a chrome window this embedding cannot show. Nothing is added to the YAP
  // contract by this; Luna carries it.
  _notify: function(aCategory, aText) {
    try {
      let bag = Cc["@mozilla.org/hash-property-bag;1"]
                  .createInstance(Ci.nsIWritablePropertyBag2);
      bag.setPropertyAsAString("category", aCategory);
      bag.setPropertyAsAString("text", aText);
      Services.obs.notifyObservers(bag, "jihad-notify", null);
    } catch (e) {}
  },

  observe: function(aSubject, aTopic, aData) {
    if (aTopic != "addon-install-failed" && aTopic != "addon-install-complete")
      return;
    // The subject is amWebInstallListener's plain JS info object. wrappedJSObject is the idiom
    // Firefox's own XPInstallObserver uses for it; the QI is the fallback, since the object does
    // declare amIWebInstallInfo.
    let info = null;
    try { info = aSubject.wrappedJSObject; } catch (e) {}
    if (!info || !info.installs) {
      try { info = aSubject.QueryInterface(Ci.amIWebInstallInfo); } catch (e) { return; }
    }
    let installs = (info && info.installs) ? info.installs : [];
    let complete = (aTopic == "addon-install-complete");
    // On the FAILED topic, ONLY the incompatibility case is claimed here. Every other failure
    // on that topic already reports a DISTINCT legacy code to the page (-207 corrupt archive,
    // -228 download), so surfacing those as well would change behaviour outside this criterion.
    // On the COMPLETE topic there is nothing to filter: amWebInstallListener only puts an
    // install in `this.installed` once it actually installed.
    let names = [];
    for (let install of installs) {
      try {
        if (!complete && (!install.addon || !install.addon.appDisabled))
          continue;
        names.push((install.addon && install.addon.name) || install.name || "The add-on");
      } catch (e) {}
    }
    if (names.length == 0)
      return;
    // Same sanitising, and the same reason, as DialogService.cpp's confirm text: the name comes
    // out of install.rdf, i.e. from content, so it can carry control characters or be long enough
    // to push the real message off the card. (The C++ side sanitises again — this is content
    // text and the two budgets are independent; neither is allowed to be the only one.)
    let label = names.join(", ").replace(/[\x00-\x1f\x7f]/g, " ");
    if (label.length > 160)
      label = label.substr(0, 157) + "...";
    if (complete) {
      this._notify("addon", label + " installed.");
      return;
    }
    let version = "";
    try { version = Services.appinfo.version; } catch (e) {}
    this._notify("addon", label + " could not be installed because it is not compatible with " +
                          "Jihad Browser" + (version ? " " + version : "") + ".");
  },

  confirm: function(aBrowser, aUri, aInstalls, aCount) {
    let names = [];
    for (let install of aInstalls) {
      try {
        names.push(install.name ||
                   (install.addon && install.addon.name) ||
                   (install.sourceURI && install.sourceURI.spec) ||
                   "add-on");
      } catch (e) {
        names.push("add-on");
      }
    }
    let bag = Cc["@mozilla.org/hash-property-bag;1"]
                .createInstance(Ci.nsIWritablePropertyBag2);
    // Default DENY: if the daemon has no observer (or no card is connected to
    // answer), an unattended install must not slip through.
    bag.setPropertyAsBool("accept", false);
    let host = "";
    try { host = aUri ? aUri.host : ""; } catch (e) {}
    bag.setPropertyAsAString("host", host);
    bag.setPropertyAsAString("names", names.join(", "));
    // SYNCHRONOUS round-trip: the daemon's observer blocks on the card's YAP
    // reply (same blocking model as the SSL confirm) and rewrites "accept".
    try {
      Services.obs.notifyObservers(bag, "jihad-xpi-confirm", null);
    } catch (e) {}
    let accept = false;
    try { accept = bag.getPropertyAsBool("accept"); } catch (e) {}
    for (let install of aInstalls) {
      try {
        if (accept) {
          install.install();
        } else {
          install.cancel();
        }
      } catch (e) {}
    }
    if (!accept) {
      // Observers are how failure surfaces at all in this embedding (F6).
      try {
        Services.obs.notifyObservers(aBrowser, "addon-install-cancelled", null);
      } catch (e) {}
    }
  }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadInstallPrompt]);
