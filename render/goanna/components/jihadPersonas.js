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

// What a card is given: the fields it paints with, each bounded, so the message always fits the
// notification channel (DialogService passes "theme" whole up to 6000 characters) and a card
// that starts later (getTheme) sees exactly what a running card saw.
function cardTheme(t) {
  // Control characters out first: JSON writes each as six characters, which is how three short
  // fields could still overflow the channel (inspector 2026-10-06).
  let out = {}, str = function(v, max) {
    return (typeof v === "string" && v.length <= max) ? v.replace(/[\x00-\x1f\x7f]/g, " ") : undefined;
  };
  out.id = str(t.id, 200) || "theme";
  out.name = str(t.name, 200);
  out.author = str(t.author, 200);
  out.headerURL = str(t.headerURL, 2048);
  out.footerURL = str(t.footerURL, 2048);
  out.textcolor = str(t.textcolor, 40);
  out.accentcolor = str(t.accentcolor, 40);
  // Quotes and backslashes still escape to two characters: if what is left would not fit the
  // channel (6000), drop what a card can live without, in order, rather than send "null".
  let drop = ["footerURL", "author", "name", "headerURL"];
  for (let i = 0; i < drop.length && JSON.stringify(out).length > 5800; i++) { delete out[drop[i]]; }
  return out;
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
//
// The package is content: a manifest over 64 KB or an image over 4 MB is not a theme. The
// sizes a zip DECLARES are the attacker's to write, so they are only a first filter: the image
// is copied through the reader's own stream (which stops at the declared size) with a hard
// byte cap, never `extract`ed (which inflates to the end of the data and recreates symlink
// entries as symlinks — inspector 2026-10-06, P2). It lands in a PENDING file that only becomes
// the theme's after the user's yes (and is deleted on a no), and the theme is named by a hash
// of its image, so two packages that share a name cannot overwrite each other's image
// (Codex 2026-10-06).
const MAX_MANIFEST = 64 * 1024, MAX_IMAGE = 4 * 1024 * 1024;

// Copy one zip entry into `out`, at most `max` bytes, as a plain file.
function copyEntry(zr, name, out, max) {
  let bis = Cc["@mozilla.org/binaryinputstream;1"].createInstance(Ci.nsIBinaryInputStream);
  bis.setInputStream(zr.getInputStream(name));
  let fos = Cc["@mozilla.org/network/file-output-stream;1"].createInstance(Ci.nsIFileOutputStream);
  fos.init(out, 0x02 | 0x08 | 0x20, 0o644, 0);          // write, create, truncate
  let bos = Cc["@mozilla.org/binaryoutputstream;1"].createInstance(Ci.nsIBinaryOutputStream);
  bos.setOutputStream(fos);
  let total = 0;
  try {
    for (;;) {
      let n = Math.min(bis.available(), 65536);
      if (n <= 0) { break; }
      total += n;
      if (total > max) { throw new Error("the image is larger than " + max + " bytes"); }
      bos.writeByteArray(bis.readByteArray(n), n);
    }
  } finally {
    try { bos.close(); } catch (e) {}
    try { bis.close(); } catch (e) {}
  }
}

function sha256Hex(file) {
  let ch = Cc["@mozilla.org/security/hash;1"].createInstance(Ci.nsICryptoHash);
  ch.init(ch.SHA256);
  let is = Cc["@mozilla.org/network/file-input-stream;1"].createInstance(Ci.nsIFileInputStream);
  is.init(file, 0x01, 0, 0);
  ch.updateFromStream(is, -1);
  is.close();
  let raw = ch.finish(false), hex = "";
  for (let i = 0; i < raw.length; i++) { hex += ("0" + raw.charCodeAt(i).toString(16)).slice(-2); }
  return hex;
}

// Returns {theme, pending (nsIFile), finalName} or null when the file is not a theme-only package.
function staticThemeFromXpi(path, origin) {
  let file = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsIFile);
  try { file.initWithPath(path); } catch (e) { return null; }
  if (!file.exists()) { return null; }
  let zr = Cc["@mozilla.org/libjar/zip-reader;1"].createInstance(Ci.nsIZipReader);
  try { zr.open(file); } catch (e) { return null; }
  let pending = null;
  try {
    if (!zr.hasEntry("manifest.json") || zr.getEntry("manifest.json").realSize > MAX_MANIFEST) { return null; }
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
    let ext = (/\.(png|jpe?g|gif)$/i.exec(header) || [null, ""])[1].toLowerCase();
    if (!ext || !zr.hasEntry(header) || zr.getEntry(header).realSize > MAX_IMAGE) { return null; }
    pending = themesDir();
    pending.append("pending-" + Date.now() + "-" + Math.floor(Math.random() * 1e9) + "." + ext);
    copyEntry(zr, header, pending, MAX_IMAGE);
    let hash = sha256Hex(pending).slice(0, 24);
    let name = String(m.name || "Theme").replace(/[\x00-\x1f\x7f]/g, " ").slice(0, 80);
    let finalName = hash + "-header." + ext;
    let theme = {id: "static-theme:" + hash, name: name, headerURL: "resource://" + THEMES_RESOURCE + "/" + finalName};
    let accent = cssColor(colors.frame || colors.accentcolor), textc = cssColor(colors.tab_background_text || colors.textcolor || colors.toolbar_text);
    if (accent) { theme.accentcolor = accent; }
    if (textc) { theme.textcolor = textc; }
    if (m.author && typeof m.author === "string") { theme.author = m.author.slice(0, 80); }
    let out = {theme: theme, pending: pending, finalName: finalName};
    pending = null;                                   // handed over
    return out;
  } catch (e) {
    log("static theme could not be read: " + e);
    return null;
  } finally {
    try { zr.close(); } catch (e) {}
    if (pending) { try { pending.remove(false); } catch (e) {} }
  }
}

// "jihad-static-theme" (from components/jihadInstallPrompt.js, with the failed install's file
// still on disk): a theme-only static theme becomes the lightweight theme, with the user's
// yes; `converted` goes back true so the install's own failure is not reported as one.
function handleStaticTheme(bag) {
  let path = "", origin = "";
  try { path = bag.getPropertyAsAString("path"); } catch (e) {}
  try { origin = bag.getPropertyAsAString("origin"); } catch (e) {}
  let got = staticThemeFromXpi(String(path), String(origin));
  if (!got) { return; }
  let host = hostOf(origin);
  if (!confirmTheme(host, got.theme.name)) {
    log("static theme [" + got.theme.name + "] from " + host + " declined");
    try { got.pending.remove(false); } catch (e) {}
    bag.setPropertyAsBool("converted", true);          // it was a theme; the user said no
    return;
  }
  let dest = themesDir();
  dest.append(got.finalName);
  try {
    if (dest.exists()) { got.pending.remove(false); }  // the same image, already here
    else { got.pending.moveTo(null, got.finalName); }
  } catch (e) {
    log("static theme image could not be kept: " + e);
    try { got.pending.remove(false); } catch (e2) {}
    return;
  }
  ensureThemesResource();
  let LWTM = manager();
  previewId = null;
  LWTM.resetPreview();
  LWTM.currentTheme = got.theme;
  bag.setPropertyAsBool("converted", true);
  log("static theme [" + got.theme.name + "] from " + host + " is the theme now");
  notify("info", "Theme applied: " + got.theme.name);
}

// Whether a page may PREVIEW a theme (repaint the browser's chrome without asking): Firefox
// 52's rule (LightWeightThemeWebInstaller._isAllowed), which Pale Moon and Basilisk keep — an
// https page whose site holds the "install" permission. Anything else is ignored: a page must
// not be able to repaint the chrome around it at will (Codex 2026-10-06: chrome spoofing).
// An INSTALL is always put to the user instead (confirmTheme), wherever it comes from.
// The theme a page is previewing (its id) and until when: an update carrying that id is not
// remembered as installed — a preview may reuse the installed theme's id (inspector
// 2026-10-06). The manager ends a preview by itself; the time bound keeps a stale mark from
// blocking a later, real choice of that theme for long.
let previewId = null, previewUntil = 0;

function previewAllowed(origin) {
  let uri = null;
  try { uri = Services.io.newURI(String(origin || ""), null, null); } catch (e) { return false; }
  if (!uri.schemeIs("https")) { return false; }
  try { return Services.perms.testPermission(uri, "install") === Services.perms.ALLOW_ACTION; }
  catch (e) { return false; }
}

function handle(req) {
  let LWTM = manager();
  if (req.kind === "reset") { previewId = null; LWTM.resetPreview(); return; }
  // A theme comes from a SITE: a request from a document with none (about:blank, about:srcdoc,
  // data:) would put a nameless "Use the theme" on the confirm (inspector 2026-10-06).
  if (!hostOf(req.origin)) {
    log("theme request from a document with no site ignored (" + (req.kind || "?") + ")");
    return;
  }
  if (req.kind === "preview" && !previewAllowed(req.origin)) {
    log("preview from " + hostOf(req.origin) + " ignored (not an allowed theme site)");
    return;
  }
  let theme = null;
  // Relative image URLs resolve against the dispatching document's base, captured with the event.
  try { theme = LWTM.parseTheme(String(req.json || ""), req.base ? String(req.base) : (req.origin ? String(req.origin) : null)); }
  catch (e) { theme = null; }
  if (!theme) {
    log("the page's theme could not be read (" + (req.kind || "?") + ")");
    if (req.kind === "install") { notify("info", "That theme could not be read."); }
    return;
  }
  if (req.kind === "preview") {
    previewId = theme.id; previewUntil = Date.now() + 60000;
    LWTM.previewTheme(theme);     // the manager times a preview out on its own
    return;
  }
  // install
  previewId = null;
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
  // Images unpacked for a theme the user never answered (the daemon stopped mid-question).
  try {
    let en = themesDir().directoryEntries;
    while (en.hasMoreElements()) {
      let f = en.getNext().QueryInterface(Ci.nsIFile);
      if (/^pending-/.test(f.leafName)) { try { f.remove(false); } catch (e) {} }
    }
  } catch (e) {}
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
      let LWTM = null, cur = null;
      try { LWTM = manager(); cur = LWTM.currentTheme; } catch (e) { cur = null; }
      // A converted static theme's images are served to the manager as resource://jihad-themes/…
      // (it takes no file: URL from us); the cards cannot load resource:, so they get the file
      // that URL stands for — inside this app's directory, which a card may read.
      let rawId = null;
      if (json) {
        try {
          let t = JSON.parse(json), changed = false;
          rawId = t.id;
          ["headerURL", "footerURL"].forEach(function(k) {
            if (typeof t[k] === "string" && t[k].indexOf("resource://" + THEMES_RESOURCE + "/") === 0) {
              let rph = Services.io.getProtocolHandler("resource").QueryInterface(Ci.nsIResProtocolHandler);
              t[k] = rph.resolveURI(Services.io.newURI(t[k], null, null)); changed = true;
            }
          });
          json = JSON.stringify(cardTheme(t));
        } catch (e) { log("theme could not be read: " + e); json = ""; }
      }
      // Only the INSTALLED theme is remembered (a preview is not: it would otherwise survive a
      // restart as if chosen), and the pref file is written only when it actually changes.
      // Decided on the theme's OWN id (cardTheme may have replaced an oversized one), and never
      // for an update that is the page's preview.
      let previewing = previewId !== null && rawId === previewId && Date.now() < previewUntil;
      let isCurrent = json ? !!(cur && rawId === cur.id && !previewing) : !cur;
      if (isCurrent) {
        try {
          let had = Services.prefs.prefHasUserValue(PREF_CURRENT) ? Services.prefs.getCharPref(PREF_CURRENT) : "";
          if (had !== json) {
            if (json) { Services.prefs.setCharPref(PREF_CURRENT, json); }
            else { Services.prefs.clearUserPref(PREF_CURRENT); }
            Services.prefs.savePrefFile(null);
          }
        } catch (e) { log("could not remember the theme: " + e); }
      }
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
