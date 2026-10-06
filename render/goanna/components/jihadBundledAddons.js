/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — the add-ons that come with the browser.
 *
 * The package carries a small number of third-party add-ons as their authors' own,
 * unmodified XPI files, in <engine dir>/bundled-addons/ (third_party/<name>/README.md in
 * the source tree says what each is, where it came from and under what licence). This
 * component OFFERS each one ONCE per profile: on the first start it installs the file
 * through the add-on manager — the same path as a user opening the file, with the same
 * compatibility check — and records that it did. After that the add-on is the user's:
 *
 *   - it is listed in the add-ons manager and can be disabled or removed there;
 *   - once offered it is never installed again, so a user who removed it keeps it removed
 *     (the record is the pref jihad.bundled-addons.<id>.offered);
 *   - a later package carrying a NEWER version upgrades a copy the user still has, and
 *     leaves a user who removed it alone.
 *
 * It is created by the daemon at start-up through its contract id and does its work a few
 * seconds later, off the start-up path; nothing depends on it having finished.
 */

"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");

// What ships. `version` is the version of the bundled FILE (the package's make-device-bundle.sh
// pins the file by SHA-256, so this cannot drift from it unnoticed).
const BUNDLED = [
  { id: "palefill@addons.martoks-place.de", file: "palefill@addons.martoks-place.de.xpi", version: "1.30" }
];
const PREF = "jihad.bundled-addons.";

let gTimer = null;

function log(s) {
  try { Services.console.logStringMessage("bundled add-ons: " + s); } catch (e) {}
}

function wasOffered(b) {
  try { return Services.prefs.getBoolPref(PREF + b.id + ".offered"); } catch (e) { return false; }
}

function markOffered(b) {
  try {
    Services.prefs.setBoolPref(PREF + b.id + ".offered", true);
    Services.prefs.savePrefFile(null);
  } catch (e) { log("could not record that " + b.id + " was offered: " + e); }
}

function bundledFile(b) {
  let f = Services.dirsvc.get("GreD", Ci.nsIFile);
  f.append("bundled-addons");
  f.append(b.file);
  return f;
}

function install(AddonManager, b, why) {
  let f = bundledFile(b);
  if (!f.exists()) { log(b.id + ": the bundled file is missing from this package"); return; }
  AddonManager.getInstallForFile(f, function(inst) {
    if (!inst) { log(b.id + ": the add-on manager would not read the bundled file"); return; }
    // The manager has read the file and judged it: one it would disable at once (the
    // engine moved on, or the add-on did) is not installed — a disabled add-on is worse
    // than none, and on an upgrade it would replace a copy that works. Not recorded as
    // offered, so a package whose engine fits again offers it then.
    if (inst.addon && inst.addon.appDisabled) {
      log(b.id + " " + b.version + ": not compatible with this engine; not installed (" + why + ")");
      try { inst.cancel(); } catch (e) {}
      return;
    }
    inst.addListener({
      onInstallEnded: function(i, addon) {
        markOffered(b);
        log(b.id + " " + addon.version + " installed (" + why + "); active=" + addon.isActive +
            (addon.isCompatible ? "" : "; NOT compatible with this engine"));
      },
      // Not recorded as offered: the next start tries again.
      onInstallFailed: function(i) { log(b.id + ": install failed, error " + i.error); },
      onDownloadFailed: function(i) { log(b.id + ": the bundled file could not be read, error " + i.error); },
      onInstallCancelled: function() { log(b.id + ": install cancelled"); }
    });
    inst.install();
  }, "application/x-xpinstall");
}

function offerAll(attempt) {
  let AddonManager = null;
  try {
    AddonManager = Cu.import("resource://gre/modules/AddonManager.jsm", {}).AddonManager;
    // Throws until the manager has started; that is the readiness test.
    AddonManager.getAddonByID(BUNDLED[0].id, function() {});
  } catch (e) {
    if (attempt < 10) { later(function() { offerAll(attempt + 1); }, 3000); }
    else { log("the add-on manager never became ready: " + e); }
    return;
  }
  BUNDLED.forEach(function(b) {
    AddonManager.getAddonByID(b.id, function(existing) {
      try {
        if (existing) {
          // The user has it. Upgrade an older copy from the package; otherwise leave it be.
          if (!wasOffered(b)) { markOffered(b); }
          if (Services.vc.compare(existing.version, b.version) < 0) { install(AddonManager, b, "upgrade from " + existing.version); }
          return;
        }
        if (wasOffered(b)) { return; }            // offered before and since removed: the user's call
        install(AddonManager, b, "first start");
      } catch (e) { log(b.id + ": " + e); }
    });
  });
}

function later(fn, ms) {
  gTimer = Cc["@mozilla.org/timer;1"].createInstance(Ci.nsITimer);
  gTimer.initWithCallback({notify: function() { gTimer = null; fn(); }}, ms, Ci.nsITimer.TYPE_ONE_SHOT);
}

function JihadBundledAddons() {
  later(function() { offerAll(0); }, 4000);
}

JihadBundledAddons.prototype = {
  classID: Components.ID("{0b9f6a4e-2f3d-4c6e-8f0a-6d2c9b1e7a55}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsISupports])
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadBundledAddons]);
