/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — JihadEngine.jsm: the engine's JS surface Jihad's components rely on, in one place
 * (cavekit-engine-update.md R4b, T-386).
 *
 * The engine modules the components import (LightweightThemeManager, AddonManager,
 * PermissionsUtils) and the about:addons markup the Personas pane decorates (element ids, the
 * anonids of an add-on's row, classes, the view controller) change between UXP releases without the
 * compiler saying so. Every component reaches them through here, and
 * build/engine/check-engine-api.py reads the SURFACE block below (the same JSON) to check a
 * candidate engine against exactly what the components use: one list, not two.
 *
 * Installed as resource://gre/modules/JihadEngine.jsm (modules/ in the GRE).
 */
"use strict";

this.EXPORTED_SYMBOLS = ["JihadEngine"];

const Cu = Components.utils;

// SURFACE-BEGIN (JSON: check-engine-api.py parses the text between the markers)
const SURFACE = {
  "modules": {
    "LightweightThemeManager": {
      "url": "resource://gre/modules/LightweightThemeManager.jsm",
      "source": "toolkit/mozapps/extensions/LightweightThemeManager.jsm",
      "needs": ["parseTheme", "previewTheme", "resetPreview", "currentTheme", "usedThemes", "forgetUsedTheme",
                "\"lightweightThemes.\"", "lightweight-theme-styling-update", "@personas.mozilla.org"]
    },
    "AddonManager": {
      "url": "resource://gre/modules/AddonManager.jsm",
      "source": "toolkit/mozapps/extensions/AddonManager.jsm",
      "needs": ["getInstallForFile", "getAddonByID", "addAddonListener"]
    },
    "PermissionsUtils": {
      "url": "resource://gre/modules/PermissionsUtils.jsm",
      "source": "toolkit/modules/PermissionsUtils.jsm",
      "needs": ["importFromPrefs"]
    }
  },
  "addons": {
    "urls": ["about:addons", "chrome://mozapps/content/extensions/extensions.xul"],
    "ids": {
      "page": "addons-page",
      "listView": "list-view",
      "addonList": "addon-list",
      "addonListEmpty": "addon-list-empty",
      "detailView": "detail-view",
      "detailRows": "detail-rows",
      "categoryTheme": "category-theme"
    },
    "themeView": "addons://list/theme",
    "anonids": ["name", "description", "date-updated", "version", "details-btn", "enable-btn", "disable-btn", "remove-btn"],
    "classes": ["content-inner-container", "disabled-postfix"],
    "markup": {
      "toolkit/mozapps/extensions/content/extensions.xul": ["id=\"addons-page\"", "id=\"list-view\"", "id=\"addon-list\"",
        "id=\"addon-list-empty\"", "id=\"detail-view\"", "id=\"detail-rows\""],
      "toolkit/mozapps/extensions/content/extensions.xml": ["anonid=\"name\"", "anonid=\"description\"", "anonid=\"date-updated\"",
        "anonid=\"version\"", "anonid=\"details-btn\"", "anonid=\"enable-btn\"", "anonid=\"disable-btn\"", "anonid=\"remove-btn\"",
        "anonid=\"control-container\"", "class=\"content-inner-container\"", "class=\"disabled-postfix\""],
      "toolkit/mozapps/extensions/content/extensions.js": ["gViewController", "currentViewId", "viewPort", "\"ViewChanged\"",
        "extensions.ui.lastCategory", "\"category-\" + aId", "addons://list/"]
    }
  }
};
// SURFACE-END

this.JihadEngine = {
  surface: SURFACE,

  // An engine module by its name in SURFACE.modules (throws for a name not listed there, so a new
  // dependency is added to the surface, and so to the checker, first).
  module(name) {
    const m = SURFACE.modules[name];
    if (!m) { throw new Error("JihadEngine: " + name + " is not in the surface"); }
    return Cu.import(m.url, {})[name];
  },
  get LightweightThemeManager() { return this.module("LightweightThemeManager"); },
  get AddonManager() { return this.module("AddonManager"); },
  get PermissionsUtils() { return this.module("PermissionsUtils"); },

  // about:addons
  addons: {
    ids: SURFACE.addons.ids,
    themeView: SURFACE.addons.themeView,
    // is this document the add-on manager?
    isDocument(doc) {
      let u = "";
      try { u = String(doc.documentURI || ""); } catch (e) { return false; }
      return SURFACE.addons.urls.some(function(p) { return u.indexOf(p) === 0; });
    },
    // an element by its key in SURFACE.addons.ids
    element(doc, key) {
      const id = SURFACE.addons.ids[key];
      if (!id) { throw new Error("JihadEngine: no about:addons id " + key); }
      return doc.getElementById(id);
    },
    // "#id" for a style sheet
    selector(key) {
      const id = SURFACE.addons.ids[key];
      if (!id) { throw new Error("JihadEngine: no about:addons id " + key); }
      return "#" + id;
    },
    // a row's part by anonid or class (both listed in the surface)
    part(doc, item, attr, value) {
      const listed = attr === "anonid" ? SURFACE.addons.anonids : attr === "class" ? SURFACE.addons.classes : [];
      if (listed.indexOf(value) < 0) { throw new Error("JihadEngine: about:addons " + attr + " " + value + " is not in the surface"); }
      return doc.getAnonymousElementByAttribute(item, attr, value);
    },
    // the manager's view controller and the view shown
    viewController(win) { return win.gViewController || null; },
    currentViewId(win) {
      const c = win.gViewController;
      return String((c && c.currentViewId) || "");
    },
    viewChangedEvent: "ViewChanged",
  },
};
