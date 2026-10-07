/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
 * Jihad Browser — lightweight themes ("personas"), the browser's half.
 *
 * Firefox 52, Pale Moon and Basilisk personas are one format: a JSON object (id, name,
 * headerURL, footerURL, textcolor, accentcolor, …) that a theme site hands to the browser by
 * dispatching InstallBrowserTheme / PreviewBrowserTheme / ResetBrowserThemePreview on an
 * element whose data-browsertheme attribute carries the JSON. The engine already has the
 * manager (resource://gre/modules/LightweightThemeManager.jsm): it sanitises the JSON, keeps
 * the current theme and the recently used ones in prefs, downloads the header/footer images
 * into the profile (lightweighttheme-header / -footer) and tells its observers with
 * "lightweight-theme-styling-update". What a bare engine has not got is a browser window to
 * catch the events and paint the theme — that is the three card shells here.
 *
 *   daemon  ──"jihad-page-command" {cmd:"theme", kind, json, origin}──▶  this component
 *             (GoannaRenderPage saw the event on the page's document)
 *   this    ──"jihad-xpi-confirm" {kind:"theme", host, names}──▶ the card's own confirm dialog
 *   this    ──LightweightThemeManager.currentTheme = theme──▶ engine persists + notifies
 *   engine  ──"lightweight-theme-styling-update" (JSON | "null")──▶ this
 *   this    ──pref jihad.theme.current (the JSON) + "jihad-notify" {category:"theme"}──▶ cards
 *             (a card starting later asks the daemon's Luna service: getTheme reads the pref)
 *
 * The cards paint the header image (anchored top right, as Firefox did) behind their top
 * chrome in accentcolor, and set their text to textcolor. The image URL is the persisted
 * profile file once the engine has it (a card may load a file from its own application
 * directory, which the profile is in), the site's URL until then.
 *
 * Nothing here touches the YAP contract; Luna carries the card's half.
 */

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;
Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/NetUtil.jsm");

const PREF_CURRENT = "jihad.theme.current";
// Where a static theme's images are unpacked (under the profile), served to the theme manager
// as resource://jihad-themes/… — the manager takes http, https and resource URLs, not file.
const THEMES_DIR = "jihad-themes";
const THEMES_RESOURCE = "jihad-themes";

function log(s) {
  try { Services.console.logStringMessage("theme: " + s); } catch (e) {}
}

function notify(category, text) {
  try {
    let bag = Cc["@mozilla.org/hash-property-bag;1"].createInstance(Ci.nsIWritablePropertyBag2);
    bag.setPropertyAsAString("category", category);
    bag.setPropertyAsAString("text", String(text));
    Services.obs.notifyObservers(bag, "jihad-notify", null);
  } catch (e) {}
}

function manager() {
  return Cu.import("resource://gre/modules/LightweightThemeManager.jsm", {}).LightweightThemeManager;
}

function hostOf(url) {
  try { return Services.io.newURI(url, null, null).host; } catch (e) { return ""; }
}

// The user's yes or no, through the daemon's blocking confirm (the same door the XPI install
// uses — DialogService.cpp answers it on the asking card in that card's own idiom). Default no.
function confirmTheme(host, name) {
  let bag = Cc["@mozilla.org/hash-property-bag;1"].createInstance(Ci.nsIWritablePropertyBag2);
  bag.setPropertyAsBool("accept", false);
  bag.setPropertyAsAString("kind", "theme");
  bag.setPropertyAsAString("host", host);
  bag.setPropertyAsAString("names", name);
  try { Services.obs.notifyObservers(bag, "jihad-xpi-confirm", null); } catch (e) {}
  try { return bag.getPropertyAsBool("accept"); } catch (e) { return false; }
}

function themesDir() {
  let d = Services.dirsvc.get("ProfD", Ci.nsIFile);
  d.append(THEMES_DIR);
  if (!d.exists()) { d.create(Ci.nsIFile.DIRECTORY_TYPE, 0o755); }
  return d;
}

function ensureThemesResource() {
  let rph = Services.io.getProtocolHandler("resource").QueryInterface(Ci.nsIResProtocolHandler);
  if (!rph.hasSubstitution(THEMES_RESOURCE)) { rph.setSubstitution(THEMES_RESOURCE, Services.io.newFileURI(themesDir())); }
}

// A WebExtension colour: "#rrggbb", "rgb(...)", a name, or [r, g, b(, a)].
function cssColor(c) {
  if (Array.isArray(c) && c.length >= 3) {
    let n = c.slice(0, 4).map(function(v) { return Number(v); });
    if (n.some(isNaN)) { return null; }
    return n.length === 4 ? "rgba(" + n.join(",") + ")" : "rgb(" + n.join(",") + ")";
  }
  if (typeof c === "string" && /^[#a-zA-Z0-9(),.%\s-]{1,40}$/.test(c.trim())) { return c.trim(); }
  return null;
}

// AMO now ships every former persona as a WebExtension "static theme": a manifest.json with a
// `theme` key (images.theme_frame or images.headerURL; colors.frame / accentcolor, colors.
// tab_background_text / textcolor) and the image beside it. This engine installs no
// WebExtension — the XPI fails as corrupt — but a theme-ONLY package describes exactly a
// lightweight theme, so it is read here and handed to the manager as one. Anything that would
// need WebExtension machinery (scripts, actions, permissions) is left to fail as before.
// Returns the lightweight theme, or null when the file is not a theme-only package.
function staticThemeFromXpi(path, origin) {
  let file = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsIFile);
  try { file.initWithPath(path); } catch (e) { return null; }
  if (!file.exists()) { return null; }
  let zr = Cc["@mozilla.org/libjar/zip-reader;1"].createInstance(Ci.nsIZipReader);
  try { zr.open(file); } catch (e) { return null; }
  try {
    if (!zr.hasEntry("manifest.json")) { return null; }
    let is = zr.getInputStream("manifest.json");
    let text = NetUtil.readInputStreamToString(is, is.available(), {charset: "UTF-8"});
    is.close();
    let m = JSON.parse(text.replace(/^\uFEFF/, ""));
    if (!m || typeof m !== "object" || !m.theme || typeof m.theme !== "object") { return null; }
    let notATheme = ["background", "content_scripts", "browser_action", "page_action", "sidebar_action", "permissions", "web_accessible_resources"];
    for (let i = 0; i < notATheme.length; i++) { if (notATheme[i] in m) { return null; } }
    let images = m.theme.images || {}, colors = m.theme.colors || {};
    let header = images.theme_frame || images.headerURL;
    if (typeof header !== "string") { return null; }
    header = header.replace(/^\.?\//, "");
    if (!zr.hasEntry(header)) { return null; }
    let name = String(m.name || "Theme").replace(/[\x00-\x1f\x7f]/g, " ").slice(0, 80);
    let host = hostOf(origin) || "local";
    let id = "static-theme:" + name.replace(/[^A-Za-z0-9._-]+/g, "-").toLowerCase() + "@" + host;
    let ext = (/\.(png|jpe?g|gif|webp|bmp)$/i.exec(header) || [null, "img"])[1].toLowerCase();
    let out = themesDir();
    out.append(id.replace(/[^A-Za-z0-9._-]+/g, "_") + "-header." + ext);
    if (out.exists()) { out.remove(false); }
    zr.extract(header, out);
    ensureThemesResource();
    let theme = {id: id, name: name, headerURL: "resource://" + THEMES_RESOURCE + "/" + out.leafName};
    let accent = cssColor(colors.frame || colors.accentcolor), textc = cssColor(colors.tab_background_text || colors.textcolor || colors.toolbar_text);
    if (accent) { theme.accentcolor = accent; }
    if (textc) { theme.textcolor = textc; }
    if (typeof m.description === "string") { theme.description = m.description.slice(0, 200); }
    if (m.author && typeof m.author === "string") { theme.author = m.author.slice(0, 80); }
    return theme;
  } catch (e) {
    log("static theme could not be read: " + e);
    return null;
  } finally {
    try { zr.close(); } catch (e) {}
  }
}

// "jihad-static-theme" (from components/jihadInstallPrompt.js, with the failed install's file
// still on disk): a theme-only static theme becomes the lightweight theme, with the user's
// yes; `converted` goes back true so the install's own failure is not reported as one.
function handleStaticTheme(bag) {
  let path = "", origin = "";
  try { path = bag.getPropertyAsAString("path"); } catch (e) {}
  try { origin = bag.getPropertyAsAString("origin"); } catch (e) {}
  let theme = staticThemeFromXpi(String(path), String(origin));
  if (!theme) { return; }
  let host = hostOf(origin);
  if (!confirmTheme(host, theme.name)) {
    log("static theme [" + theme.name + "] from " + host + " declined");
    bag.setPropertyAsBool("converted", true);          // it was a theme; the user said no
    return;
  }
  let LWTM = manager();
  LWTM.resetPreview();
  LWTM.currentTheme = theme;
  bag.setPropertyAsBool("converted", true);
  log("static theme [" + theme.name + "] from " + host + " is the theme now");
  notify("info", "Theme applied: " + theme.name);
}

function handle(req) {
  let LWTM = manager();
  if (req.kind === "reset") { LWTM.resetPreview(); return; }
  let theme = null;
  try { theme = LWTM.parseTheme(String(req.json || ""), req.origin ? String(req.origin) : null); }
  catch (e) { theme = null; }
  if (!theme) {
    log("the page's theme could not be read (" + (req.kind || "?") + ")");
    if (req.kind === "install") { notify("info", "That theme could not be read."); }
    return;
  }
  if (req.kind === "preview") {
    LWTM.previewTheme(theme);     // the manager times a preview out on its own
    return;
  }
  // install
  LWTM.resetPreview();
  let host = hostOf(req.origin), name = String(theme.name || theme.id || "theme");
  if (!confirmTheme(host, name)) { log("install of [" + name + "] from " + host + " declined"); return; }
  LWTM.currentTheme = theme;      // persists, downloads the images, notifies
  log("[" + name + "] from " + host + " is the theme now");
  notify("info", "Theme applied: " + name);
}

function JihadPersonas() {
  Services.obs.addObserver(this, "jihad-page-command", false);
  Services.obs.addObserver(this, "lightweight-theme-styling-update", false);
  Services.obs.addObserver(this, "jihad-static-theme", false);
  // The resource:// root for themes already on disk — a current theme's image may be one.
  try { ensureThemesResource(); } catch (e) {}
}

JihadPersonas.prototype = {
  classID: Components.ID("{6f0d2b1c-7a3e-4b58-9c21-3e9a5d7f1b02}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIObserver, Ci.nsISupportsWeakReference]),

  observe: function(aSubject, aTopic, aData) {
    if (aTopic === "lightweight-theme-styling-update") {
      // aData: the theme's JSON (with the persisted file URIs once the engine has the images),
      // or "null" when the theme was removed. Remembered for cards that start later; sent
      // to the ones listening now.
      let json = (aData && aData !== "null") ? String(aData) : "";
      // A converted static theme's images are served to the manager as resource://jihad-themes/…
      // (it takes no file: URL from us); the cards cannot load resource:, so they get the file
      // that URL stands for — inside this app's directory, which a card may read.
      if (json) {
        try {
          let t = JSON.parse(json), changed = false;
          ["headerURL", "footerURL"].forEach(function(k) {
            if (typeof t[k] === "string" && t[k].indexOf("resource://" + THEMES_RESOURCE + "/") === 0) {
              let rph = Services.io.getProtocolHandler("resource").QueryInterface(Ci.nsIResProtocolHandler);
              t[k] = rph.resolveURI(Services.io.newURI(t[k], null, null)); changed = true;
            }
          });
          if (changed) { json = JSON.stringify(t); }
        } catch (e) { log("theme image could not be resolved: " + e); }
      }
      try {
        if (json) { Services.prefs.setCharPref(PREF_CURRENT, json); }
        else if (Services.prefs.prefHasUserValue(PREF_CURRENT)) { Services.prefs.clearUserPref(PREF_CURRENT); }
        Services.prefs.savePrefFile(null);
      } catch (e) { log("could not remember the theme: " + e); }
      notify("theme", json || "null");
      return;
    }
    if (aTopic === "jihad-static-theme") {
      let bag = null;
      try { bag = aSubject.QueryInterface(Ci.nsIWritablePropertyBag2); } catch (e) { bag = null; }
      if (bag) { try { handleStaticTheme(bag); } catch (e) { log("static theme threw: " + e); } }
      return;
    }
    if (aTopic !== "jihad-page-command") { return; }
    let req = null;
    try { req = JSON.parse(aData || "{}"); } catch (e) { req = null; }
    if (!req || req.cmd !== "theme") { return; }
    try { handle(req); } catch (e) { log("theme command threw: " + e); }
  }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadPersonas]);
