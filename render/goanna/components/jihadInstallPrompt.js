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

function JihadInstallPrompt() {}

JihadInstallPrompt.prototype = {
  classID: Components.ID("{5cf8e2a6-91a1-44f5-9d33-8ab6f2c40d17}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.amIWebInstallPrompt]),

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
