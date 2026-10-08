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
  if (t.headerFit === "cover") { out.headerFit = "cover"; }      // a wallpaper: fitted, not its corner
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

// The file a resource://jihad-themes/… URL stands for. The mapping is made here when missing:
// the component starts before the profile directory is known, so the constructor's attempt can
// fail and leave it unset until a static theme is installed (measured 2026-10-08: after a daemon
// restart every converted theme's picture failed to resolve, NS_ERROR_NOT_AVAILABLE).
function resolveThemesUrl(u) {
  ensureThemesResource();
  let rph = Services.io.getProtocolHandler("resource").QueryInterface(Ci.nsIResProtocolHandler);
  return rph.resolveURI(Services.io.newURI(u, null, null));
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

// A wallpaper-sized picture (AMO ships 3840x2160 ones) decodes to more than the engine's whole
// image cache on this device (32 MB, make-device-bundle.sh), so it is never drawn anywhere the
// engine paints it (the Personas pane, measured 2026-10-08). Kept at most WALLPAPER_WIDTH wide:
// the engine's encoder decodes it straight at that size. A picture made for a toolbar is left
// exactly as it is.
const WALLPAPER_WIDTH = 1600;
function shrinkWallpaper(file, ext) {
  if (ext === "gif") { return; }
  let sz = imageSize(file);
  if (!sz || sz.h < WALLPAPER_MIN_HEIGHT || sz.w <= WALLPAPER_WIDTH) { return; }
  let w = WALLPAPER_WIDTH, h = Math.max(1, Math.round(sz.h * WALLPAPER_WIDTH / sz.w));
  let mime = ext === "png" ? "image/png" : "image/jpeg";
  let tools = Cc["@mozilla.org/image/tools;1"].getService(Ci.imgITools);
  let src = Cc["@mozilla.org/network/file-input-stream;1"].createInstance(Ci.nsIFileInputStream);
  src.init(file, 0x01, 0, 0);
  let container = null;
  try { container = tools.decodeImage(src, mime); } finally { try { src.close(); } catch (e) {} }
  let scaled = tools.encodeScaledImage(container, mime, w, h, ext === "png" ? "" : "quality=88");
  let tmp = file.clone();
  tmp.leafName = file.leafName + ".scaled";
  let bis = Cc["@mozilla.org/binaryinputstream;1"].createInstance(Ci.nsIBinaryInputStream);
  bis.setInputStream(scaled);
  let fos = Cc["@mozilla.org/network/file-output-stream;1"].createInstance(Ci.nsIFileOutputStream);
  fos.init(tmp, 0x02 | 0x08 | 0x20, 0o644, 0);
  let bos = Cc["@mozilla.org/binaryoutputstream;1"].createInstance(Ci.nsIBinaryOutputStream);
  bos.setOutputStream(fos);
  try {
    for (;;) {
      let n = Math.min(bis.available(), 65536);
      if (n <= 0) { break; }
      bos.writeByteArray(bis.readByteArray(n), n);
    }
  } finally {
    try { bos.close(); } catch (e) {}
    try { bis.close(); } catch (e) {}
  }
  tmp.moveTo(null, file.leafName);
  log("wallpaper " + sz.w + "x" + sz.h + " kept at " + w + "x" + h);
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
    // The header art: the classic theme_frame (headerURL in older manifests), or — what many
    // newer AMO themes carry instead — the first of additional_backgrounds (owner 2026-10-08,
    // a gallery of AMO themes for the README: three of five had only that).
    let header = images.theme_frame || images.headerURL ||
      (Array.isArray(images.additional_backgrounds) ? images.additional_backgrounds[0] : null);
    if (typeof header !== "string") { return null; }
    header = header.replace(/^\.?\//, "");
    let ext = (/\.(png|jpe?g|gif)$/i.exec(header) || [null, ""])[1].toLowerCase();
    if (!ext || !zr.hasEntry(header) || zr.getEntry(header).realSize > MAX_IMAGE) { return null; }
    pending = themesDir();
    pending.append("pending-" + Date.now() + "-" + Math.floor(Math.random() * 1e9) + "." + ext);
    copyEntry(zr, header, pending, MAX_IMAGE);
    // A picture that cannot be shrunk is kept as it came (it may not draw in the pane; the
    // cards draw it) rather than refusing the Persona (inspector 2026-10-08).
    try { shrinkWallpaper(pending, ext); }
    catch (e) {
      log("wallpaper kept at its own size: " + e);
      try { let t = pending.clone(); t.leafName = pending.leafName + ".scaled"; if (t.exists()) { t.remove(false); } } catch (e2) {}
    }
    let hash = sha256Hex(pending).slice(0, 24);
    let name = String(m.name || "Theme").replace(/[\x00-\x1f\x7f]/g, " ").slice(0, 80);
    let finalName = hash + "-header." + ext;
    let theme = {id: "static-theme:" + hash, name: name, headerURL: "resource://" + THEMES_RESOURCE + "/" + finalName};
    let accent = cssColor(colors.frame || colors.accentcolor), textc = cssColor(colors.tab_background_text || colors.textcolor || colors.toolbar_text);
    if (accent) { theme.accentcolor = accent; }
    if (textc) { theme.textcolor = textc; }
    if (m.author && typeof m.author === "string") { theme.author = m.author.slice(0, 80); }
    if (typeof m.version === "string" && /^[\w.+-]{1,20}$/.test(m.version)) { theme.version = m.version; }
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
function handleStaticTheme(bag, hostLabel) {
  let path = "", origin = "";
  try { path = bag.getPropertyAsAString("path"); } catch (e) {}
  try { origin = bag.getPropertyAsAString("origin"); } catch (e) {}
  let got = staticThemeFromXpi(String(path), String(origin));
  if (!got) { return; }
  let host = hostLabel || hostOf(origin);
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
  notify("info", "Persona applied: " + got.theme.name);
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
    if (req.kind === "install") { notify("info", "That Persona could not be read."); }
    return;
  }
  if (req.kind === "preview") {
    previewId = theme.id; previewUntil = Date.now() + 30000;   // the manager's own preview limit
    LWTM.previewTheme(theme);     // the manager times a preview out on its own
    return;
  }
  // install
  previewId = null;
  LWTM.resetPreview();
  let host = hostOf(req.origin), name = String(theme.name || theme.id || "Persona");
  if (!confirmTheme(host, name)) { log("install of [" + name + "] from " + host + " declined"); return; }
  LWTM.currentTheme = theme;      // persists, downloads the images, notifies
  log("[" + name + "] from " + host + " is the theme now");
  notify("info", "Persona applied: " + name);
}

// ---------------------------------------------------------------------------------------------
// The Themes pane of about:addons, made the home of personas (cavekit-personas-ui.md). The page
// is the engine's own add-ons manager; this decorates it from here rather than changing it, so
// the manager, its add-on model and the install path above stay exactly as they are:
//   - each theme row shows the theme: its header over its accent colour, the name in its text
//     colour, the one in use marked and first (R1);
//   - Wear / Take off on the rows, a "Default look" entry above them, finger-sized controls (R2);
//   - after any change of look, an Undo bar for 30 s brings the previous one back (R3; owner
//     2026-10-08: "try and wear seem duplicative" — one Wear, made safe by Undo);
//   - every row says who made the theme and its version (owner: "author and version fields
//     should be shown for all personas", "default version 1.0");
//   - a picture made as a wallpaper (taller than a toolbar by far) is fitted, not cropped to its
//     corner (owner: "blue nebula looks like it isnt working properly");
//   - Get Personas, and Install from file — the .xpi files already on the device (R4);
//   - the Personas that come with Jihad (a Frutiger Aero set, owner 2026-10-08: "add fruiter
//     aero style themes") join the list once, as ordinary rows;
//   - a few sentences on what a persona is when there are none yet (R5).
const XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";
const GET_THEMES_URL = "https://addons.mozilla.org/firefox/themes/";
const UNDO_MS = 30000;
// A theme picture this tall was made as a wallpaper (AMO's 3840x2160 ones): drawn at its own
// size, a toolbar would show only its corner. Header art made for a toolbar is 100-300 px tall.
const WALLPAPER_MIN_HEIGHT = 400;
const ID_SUFFIX = "@personas.mozilla.org";   // the manager's add-on id is the theme's id + this
function themeIdOf(addonId) {
  let s = String(addonId || "");
  return s.slice(-ID_SUFFIX.length) === ID_SUFFIX ? s.slice(0, -ID_SUFFIX.length) : s;
}
// Where a theme the user downloaded is: the browser's downloads folder, and the USB drive's top.
const XPI_DIRS = ["/media/internal/downloads", "/media/internal"];
const PANE_CSS =
  "@namespace url(\"" + XUL_NS + "\");\n" +
  "#addons-page button { min-height: 44px !important; min-width: 64px !important; }\n" +
  "#addons-page .category { min-height: 52px; }\n" +
  "#addons-page[jihad-themes-view=\"true\"] #addon-list-empty { display: none; }\n" +
  "#jihad-themes-box { padding: 10px 12px 4px; }\n" +
  "#jihad-themes-box .jihad-themes-intro { margin: 0 0 8px; font-size: 1.05em; }\n" +
  "#jihad-themes-box .jihad-row { min-height: 64px; padding: 6px 10px; margin: 0 0 8px;\n" +
  "  border: 1px solid rgba(0,0,0,0.25); border-radius: 6px; }\n" +
  "#jihad-themes-box .jihad-row-name { font-weight: bold; font-size: 1.15em; }\n" +
  "#jihad-themes-box .jihad-in-use { font-weight: bold; }\n" +
  "#jihad-themes-box .jihad-files button { -moz-box-pack: start; }\n" +
  "#jihad-default-row { background: #e8e8e8; color: #222; }\n" +
  "#jihad-undo-bar { background: #2c5f8a; color: #fff; }\n";

let paneWindows = new Set();

// A callback on this component's own timer. The page's window.setTimeout, called from here, never
// ran its callback (measured 2026-10-08: the pane was never redrawn after a Persona change).
let timers = new Set();
function later(ms, fn) {
  let t = Cc["@mozilla.org/timer;1"].createInstance(Ci.nsITimer);
  timers.add(t);                                     // held until it fires (or it may be collected)
  t.initWithCallback({notify: function() { timers.delete(t); try { fn(); } catch (e) { log("persona timer: " + e); } }},
                     Math.max(0, ms), Ci.nsITimer.TYPE_ONE_SHOT);
  return t;
}

function isAddonsDocument(doc) {
  let u = "";
  try { u = String(doc.documentURI || ""); } catch (e) { return false; }
  return u.indexOf("about:addons") === 0 || u.indexOf("chrome://mozapps/content/extensions/extensions.xul") === 0;
}

function xul(doc, tag, attrs) {
  let el = doc.createElementNS(XUL_NS, tag);
  for (let k in attrs || {}) { el.setAttribute(k, attrs[k]); }
  return el;
}

// Is this CSS colour light? (A theme's text colour decides the pane behind the row's text.)
function isLight(color, doc) {
  try {
    let probe = xul(doc, "box");
    probe.style.color = String(color || "");
    doc.documentElement.appendChild(probe);
    let c = doc.defaultView.getComputedStyle(probe, null).color;
    probe.remove();
    let m = /rgba?\((\d+),\s*(\d+),\s*(\d+)/.exec(c);
    if (!m) { return false; }
    return (0.299 * m[1] + 0.587 * m[2] + 0.114 * m[3]) > 150;
  } catch (e) { return false; }
}

// A url() a stylesheet can take from a theme: only schemes a theme image can have, and nothing
// that could end the url() early. A converted static theme's resource://jihad-themes/ picture is
// given as the file it stands for (the page draws no resource: background; measured 2026-10-08).
function cssUrl(u) {
  if (typeof u === "string" && u.indexOf("resource://" + THEMES_RESOURCE + "/") === 0) {
    try { u = resolveThemesUrl(u); } catch (e) { return null; }
  }
  if (typeof u !== "string" || u.length > 2048 || /["'\\()\s]/.test(u)) { return null; }
  if (!/^(https?|resource|file):/i.test(u)) { return null; }
  return "url(\"" + u + "\")";
}

// The look before the last change, for Undo: {theme (null = default look), name, until}.
let undo = null;
let lastSeen = undefined;     // the Persona worn as of the last change seen (undefined = not yet)

// Called when the worn Persona may have changed (the manager's styling update), so Undo's 30 s
// run from the change itself, wherever it was made — not from when a pane was next opened.
function noteCurrent(cur) {
  let id = cur ? cur.id : null;
  if (lastSeen !== undefined && (lastSeen ? lastSeen.id : null) !== id) {
    undo = {theme: lastSeen, name: cur ? String(cur.name || "a Persona") : "the default look", until: Date.now() + UNDO_MS};
  }
  lastSeen = cur || null;
  if (undo && Date.now() > undo.until) { undo = null; }
}

// A change arrives as more than one update (wearing a Persona takes the old one off first: the
// manager says "none", then the new one), so the change is noted once they have settled —
// otherwise Undo would go back to "none" instead of the Persona worn before.
let notePending = null;
let undoTimer = null;          // one redraw when the Undo bar's time is up
function scheduleNote() {
  if (notePending) { return; }
  notePending = later(150, function() {
    notePending = null;
    try { noteCurrent(manager().currentTheme); } catch (e) {}
    refreshPanes();
  });
}

function undoChange() {
  let u = undo;
  undo = null;
  if (!u) { return; }
  lastSeen = u.theme;                     // the change Undo makes is not itself undoable
  manager().currentTheme = u.theme;
  notify("info", u.theme ? "Persona applied: " + u.theme.name : "Persona taken off.");
  refreshPanes();
}

// Width and height of a PNG, GIF or JPEG file, read from its header; null if unknown.
function imageSize(file) {
  let bis = null;
  try {
    let is = Cc["@mozilla.org/network/file-input-stream;1"].createInstance(Ci.nsIFileInputStream);
    is.init(file, 0x01, 0, 0);
    bis = Cc["@mozilla.org/binaryinputstream;1"].createInstance(Ci.nsIBinaryInputStream);
    bis.setInputStream(is);
    let b = bis.readByteArray(Math.min(bis.available(), 256 * 1024));
    let be16 = function(o) { return (b[o] << 8) | b[o + 1]; };
    if (b.length > 24 && b[0] === 0x89 && b[1] === 0x50) {                       // PNG: IHDR
      return {w: be16(16) * 65536 + be16(18), h: be16(20) * 65536 + be16(22)};
    }
    if (b.length > 10 && b[0] === 0x47 && b[1] === 0x49) {                       // GIF
      return {w: b[6] | (b[7] << 8), h: b[8] | (b[9] << 8)};
    }
    if (b.length > 4 && b[0] === 0xff && b[1] === 0xd8) {                        // JPEG: SOFn
      let o = 2;
      while (o + 9 < b.length) {
        if (b[o] !== 0xff) { o++; continue; }
        let m = b[o + 1];
        if (m >= 0xc0 && m <= 0xcf && m !== 0xc4 && m !== 0xc8 && m !== 0xcc) {
          return {w: be16(o + 7), h: be16(o + 5)};
        }
        if (m === 0xd8 || m === 0x01 || (m >= 0xd0 && m <= 0xd7)) { o += 2; continue; }
        o += 2 + be16(o + 2);
      }
    }
  } catch (e) {
  } finally {
    try { if (bis) { bis.close(); } } catch (e) {}
  }
  return null;
}

// Whether a theme's picture (a file: or resource://jihad-themes/ URL) is a wallpaper. Sizes are
// remembered by file (a picture's file never changes: its name is its hash), so a pane redraw
// does not read every picture again on the main thread (inspector 2026-10-08).
let sizeCache = new Map();
function isWallpaper(u) {
  try {
    if (typeof u !== "string") { return false; }
    if (u.indexOf("resource://" + THEMES_RESOURCE + "/") === 0) { u = resolveThemesUrl(u); }
    if (u.indexOf("file:") !== 0) { return false; }
    let f = Services.io.newURI(u, null, null).QueryInterface(Ci.nsIFileURL).file;
    let sz = sizeCache.has(f.path) ? sizeCache.get(f.path) : imageSize(f);
    sizeCache.set(f.path, sz);
    return !!(sz && sz.h >= WALLPAPER_MIN_HEIGHT);
  } catch (e) { return false; }
}

function xpiFilesOnDevice() {
  let out = [];
  XPI_DIRS.forEach(function(p) {
    try {
      let d = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsIFile);
      d.initWithPath(p);
      if (!d.exists() || !d.isDirectory()) { return; }
      let en = d.directoryEntries;
      while (en.hasMoreElements() && out.length < 40) {
        let f = en.getNext().QueryInterface(Ci.nsIFile);
        try { if (f.isFile() && /\.xpi$/i.test(f.leafName)) { out.push(f); } } catch (e) {}
      }
    } catch (e) {}
  });
  return out;
}

// The Personas that come with the browser (render/goanna/personas/: a Frutiger Aero set drawn
// for this project), shipped in the bundle's personas/ folder beside components/. Wearing one
// installs it exactly as a downloaded one: its picture copied into the profile, named by its hash.
let bundledCache = null;
function bundledPersonas() {
  if (bundledCache) { return bundledCache; }
  let out = bundledCache = [];
  try {
    let dir = __LOCATION__.parent.parent;            // <bundle>/components/jihadPersonas.js → <bundle>
    dir.append("personas");
    let listFile = dir.clone();
    listFile.append("personas.json");
    if (!listFile.exists()) { return out; }
    let is = Cc["@mozilla.org/network/file-input-stream;1"].createInstance(Ci.nsIFileInputStream);
    is.init(listFile, 0x01, 0, 0);
    let text = NetUtil.readInputStreamToString(is, is.available(), {charset: "UTF-8"});
    is.close();
    let d = JSON.parse(text);
    (d.personas || []).forEach(function(p) {
      if (!p || typeof p.file !== "string" || !/^[\w.-]+\.(jpg|png)$/.test(p.file)) { return; }
      let f = dir.clone();
      f.append(p.file);
      if (!f.exists()) { return; }
      let hash = sha256Hex(f).slice(0, 24), ext = /\.png$/.test(p.file) ? "png" : "jpg";
      out.push({file: f, finalName: hash + "-header." + ext, collection: String(d.collection || ""),
                edition: String(d.edition || 1),
                theme: {id: "static-theme:" + hash, name: String(p.name || p.slug).slice(0, 80),
                        headerURL: "resource://" + THEMES_RESOURCE + "/" + hash + "-header." + ext,
                        accentcolor: cssColor(p.accentcolor) || undefined, textcolor: cssColor(p.textcolor) || undefined,
                        author: String(p.author || "Jihad Browser").slice(0, 80), version: String(p.version || "1.0").slice(0, 20)}});
    });
  } catch (e) { log("bundled Personas: " + e); }
  return out;
}

// The bundled set joins the Persona list once per profile, NOT worn: each is then an ordinary
// row of about:addons → Personas — the same picture, Wear, Uninstall, author and version as the
// others (owner 2026-10-08: "the aero themes dont display consistently in the persona preview
// in about:addons like the others"). One the user uninstalls stays uninstalled, through later
// editions too: the names already offered are remembered, and a later edition adds only a
// Persona new to the set, or one replacing its own earlier picture still in the list.
const PREF_BUNDLED = "jihad.personas.bundled";
const PREF_BUNDLED_NAMES = "jihad.personas.bundledNames";
function offerBundledOnce() {
  let done = "";
  try { done = Services.prefs.getCharPref(PREF_BUNDLED); } catch (e) {}
  let all = bundledPersonas();
  if (!all.length) { return; }
  let key = (all[0].collection || "bundled") + "/" + all[0].edition;
  if (done === key) { return; }
  let offered = {};
  try { JSON.parse(Services.prefs.getCharPref(PREF_BUNDLED_NAMES)).forEach(function(n) { offered[n] = true; }); }
  catch (e) {
    // A profile offered the set before the names were kept was offered every one of them.
    if (done) { all.forEach(function(b) { offered[b.theme.name] = true; }); }
  }
  let LWTM = manager();
  let used = [];
  try { used = LWTM.usedThemes || []; } catch (e) { used = []; }
  let cur = null;
  try { cur = LWTM.currentTheme; } catch (e) {}
  let have = {}, wear = null, dropped = {}, droppedIds = [], replacing = {};
  used.forEach(function(t) { have[t.id] = true; });
  // A newer edition replaces its own older pictures: an entry by "Jihad Browser" with a bundled
  // Persona's name but not its picture is that Persona's earlier edition.
  let newIds = {}, byName = {};
  all.forEach(function(b) { newIds[b.theme.id] = true; byName[b.theme.name] = b; });
  used.forEach(function(t) {
    let b = byName[t.name];
    if (!b || t.author !== "Jihad Browser" || newIds[t.id]) { return; }
    dropped[t.id] = true; droppedIds.push(t.id); replacing[t.name] = true;
    if (cur && cur.id === t.id) { wear = b; }
  });
  let add = all.filter(function(b) {
    return !have[b.theme.id] && (!offered[b.theme.name] || replacing[b.theme.name]);
  });
  add.forEach(function(b) {
    let dest = themesDir();
    dest.append(b.finalName);
    if (!dest.exists()) { b.file.copyTo(themesDir(), b.finalName); }
  });
  ensureThemesResource();
  // The new edition of a worn Persona is worn FIRST (the manager puts it at the head of its list,
  // still "selected"), and only then is the list rewritten without the old edition — rewriting
  // first left the manager briefly wearing whatever came next (inspector 2026-10-08).
  if (wear) { previewId = null; LWTM.currentTheme = wear.theme; }
  if (add.length || droppedIds.length) {
    let now = [];
    try { now = LWTM.usedThemes || []; } catch (e) { now = used; }
    let present = {};
    now = now.filter(function(t) { present[t.id] = true; return !dropped[t.id]; });
    add.forEach(function(b) { if (!present[b.theme.id]) { now.push(b.theme); } });
    let str = Cc["@mozilla.org/supports-string;1"].createInstance(Ci.nsISupportsString);
    str.data = JSON.stringify(now);
    Services.prefs.getBranch("lightweightThemes.").setComplexValue("usedThemes", Ci.nsISupportsString, str);
  }
  droppedIds.forEach(function(id) { forgetThemeFiles(id); });
  all.forEach(function(b) { offered[b.theme.name] = true; });
  Services.prefs.setCharPref(PREF_BUNDLED_NAMES, JSON.stringify(Object.keys(offered)));
  Services.prefs.setCharPref(PREF_BUNDLED, key);
  log("bundled Personas offered: " + add.length + ", replaced: " + droppedIds.length + " (" + key + ")");
}

function installThemeFile(file) {
  let bag = Cc["@mozilla.org/hash-property-bag;1"].createInstance(Ci.nsIWritablePropertyBag2);
  bag.setPropertyAsAString("path", file.path);
  bag.setPropertyAsAString("origin", "");
  bag.setPropertyAsBool("converted", false);
  handleStaticTheme(bag, "a file on this device");
  let converted = false;
  try { converted = bag.getPropertyAsBool("converted"); } catch (e) {}
  if (!converted) { notify("info", file.leafName + " is not a Persona this browser can wear."); }
}

function buildBox(win) {
  let doc = win.document;
  let box = xul(doc, "vbox", {id: "jihad-themes-box"});
  let intro = xul(doc, "description", {"class": "jihad-themes-intro"});
  box.appendChild(intro);

  let undoBar = xul(doc, "hbox", {id: "jihad-undo-bar", "class": "jihad-row", align: "center", hidden: "true"});
  let undoLabel = xul(doc, "label", {flex: "1"});
  let undoBtn = xul(doc, "button", {label: "Undo"});
  undoBtn.addEventListener("command", function() { undoChange(); });
  undoBar.appendChild(undoLabel); undoBar.appendChild(undoBtn);
  box.appendChild(undoBar);

  let row = xul(doc, "hbox", {id: "jihad-default-row", "class": "jihad-row", align: "center"});
  let names = xul(doc, "vbox", {flex: "1"});
  names.appendChild(xul(doc, "label", {"class": "jihad-row-name", value: "Default look"}));
  names.appendChild(xul(doc, "label", {value: "The browser's own colours, no Persona."}));
  let inUse = xul(doc, "label", {"class": "jihad-in-use", value: "In use"});
  let wear = xul(doc, "button", {label: "Wear"});
  wear.addEventListener("command", function() {
    manager().currentTheme = null;
    notify("info", "Persona taken off.");
  });
  row.appendChild(names); row.appendChild(inUse); row.appendChild(wear);
  box.appendChild(row);

  let actions = xul(doc, "hbox", {align: "center"});
  let get = xul(doc, "button", {label: "Get Personas"});
  get.addEventListener("command", function() { win.location.href = GET_THEMES_URL; });
  let fromFile = xul(doc, "button", {label: "Install from file\u2026"});
  let files = xul(doc, "vbox", {"class": "jihad-files", hidden: "true"});
  fromFile.addEventListener("command", function() {
    while (files.firstChild) { files.firstChild.remove(); }
    let found = xpiFilesOnDevice();
    if (!found.length) {
      files.appendChild(xul(doc, "label", {value: "No .xpi files in " + XPI_DIRS.join(" or ") + "."}));
    }
    found.forEach(function(f) {
      let b = xul(doc, "button", {label: f.leafName});
      b.addEventListener("command", function() { files.hidden = true; installThemeFile(f); });
      files.appendChild(b);
    });
    files.hidden = !files.hidden;
  });
  actions.appendChild(get); actions.appendChild(fromFile);
  box.appendChild(actions);
  box.appendChild(files);

  box._jihad = {intro: intro, undoBar: undoBar, undoLabel: undoLabel, inUse: inUse, wear: wear};
  return box;
}

function decorateRow(doc, item, theme, isCurrent) {
  item.setAttribute("jihad-theme-row", "true");
  let accent = theme && theme.accentcolor ? String(theme.accentcolor) : "";
  let text = theme && theme.textcolor ? String(theme.textcolor) : "";
  let header = theme ? cssUrl(theme.headerURL) : null;
  item.style.setProperty("background-color", accent || "#3a3a3a", "important");
  if (header) {
    // A wallpaper is shown whole across the row, a toolbar picture from its top right corner
    // (where Firefox anchors it) — both filling the row.
    let wall = !!(theme && isWallpaper(theme.headerURL));
    item.style.setProperty("background-image", header, "important");
    item.style.setProperty("background-position", wall ? "center center" : "right top", "important");
    item.style.setProperty("background-size", "cover", "important");
    item.style.setProperty("background-repeat", "no-repeat", "important");
  }
  item.style.setProperty("color", text || "#ffffff", "important");
  item.style.setProperty("min-height", "96px", "important");
  item.style.MozBoxOrdinalGroup = isCurrent ? "0" : "1";
  let anon = function(attr, val) { return doc.getAnonymousElementByAttribute(item, attr, val); };
  // The text sits on a pane in the opposite of its colour, so the name reads over any picture.
  let pane = anon("class", "content-inner-container");
  if (pane) {
    let light = isLight(text || "#ffffff", doc);
    pane.style.setProperty("background-color", light ? "rgba(0,0,0,0.38)" : "rgba(255,255,255,0.5)", "important");
    pane.style.setProperty("border-radius", "6px", "important");
    pane.style.setProperty("padding", "4px 8px", "important");
  }
  ["name", "description", "date-updated", "version"].forEach(function(a) {
    let el = anon("anonid", a);
    if (el) { el.style.setProperty("color", text || "#ffffff", "important"); }
  });
  // Every Persona says its version (1.0 when it names none) and, the one worn, "In use"; the
  // manager's own "(disabled)" says the same thing less kindly and goes.
  let version = anon("anonid", "version");
  if (version) {
    let v = (theme && typeof theme.version === "string" && theme.version.trim()) ? theme.version.trim().slice(0, 20) : "1.0";
    version.setAttribute("value", v + (isCurrent ? "  \u2014 In use" : ""));
    version.hidden = false;                           // the manager hides it when a theme names none
    version.style.setProperty("font-weight", isCurrent ? "bold" : "normal", "important");
  }
  let postfix = anon("class", "disabled-postfix");
  if (postfix) { postfix.style.setProperty("display", "none", "important"); }
  // And who made it, under the name.
  let desc = anon("anonid", "description");
  if (desc) {
    let who = (theme && typeof theme.author === "string" && theme.author.trim()) ? theme.author.trim().slice(0, 80) : "";
    desc.setAttribute("value", who ? "by " + who : "Author unknown");
    desc.hidden = false;                              // the manager hides an empty description
  }
  ["enable-btn", "disable-btn", "remove-btn"].forEach(function(a) {
    let b = anon("anonid", a);
    if (b) { b.style.setProperty("min-height", "44px", "important"); b.style.setProperty("min-width", "64px", "important"); }
  });
  let en = anon("anonid", "enable-btn"), dis = anon("anonid", "disable-btn");
  if (en) { en.setAttribute("label", "Wear"); }
  if (dis) { dis.setAttribute("label", "Take off"); }
}

function decoratePane(win) {
  let doc = win.document;
  let gVC = win.gViewController;
  let viewId = String((gVC && gVC.currentViewId) || "");
  let isThemes = viewId.indexOf("addons://list/theme") === 0;
  doc.documentElement.setAttribute("jihad-themes-view", isThemes ? "true" : "false");
  // The category is called Personas, as everywhere else in this browser (owner 2026-10-08).
  let cat = doc.getElementById("category-theme");
  if (cat && cat.getAttribute("name") !== "Personas") { cat.setAttribute("name", "Personas"); }
  let listView = doc.getElementById("list-view"), list = doc.getElementById("addon-list");
  let box = doc.getElementById("jihad-themes-box");
  // "Uninstall", not "Remove", on every add-on, in every category (owner 2026-10-08).
  if (list) {
    for (let i = 0; i < list.children.length; i++) {
      let rm = doc.getAnonymousElementByAttribute(list.children[i], "anonid", "remove-btn");
      if (rm && rm.getAttribute("label") !== "Uninstall") { rm.setAttribute("label", "Uninstall"); }
    }
  }
  if (!isThemes || !listView || !list) { if (box) { box.hidden = true; } return; }
  if (!box) { box = buildBox(win); listView.insertBefore(box, list); }
  box.hidden = false;
  let LWTM = manager(), cur = LWTM.currentTheme, used = LWTM.usedThemes || [];
  if (lastSeen === undefined) { lastSeen = cur || null; }
  if (undo && Date.now() > undo.until) { undo = null; }
  let parts = box._jihad;
  parts.intro.textContent = used.length ?
    "A Persona gives the browser's toolbar and buttons a picture and colours, in every card of this browser." :
    "A Persona gives the browser's toolbar and buttons a picture and colours, in every card of this browser. " +
    "You have none yet: tap Get Personas, pick one you like, and say yes when the browser asks. " +
    "A Persona you downloaded as an .xpi file installs with Install from file.";
  parts.inUse.hidden = !!cur;
  parts.wear.hidden = !cur;
  parts.undoBar.hidden = !undo;
  if (undo) {
    parts.undoLabel.setAttribute("value", "Now wearing " + undo.name + ".");
    if (!undoTimer) {
      undoTimer = later(undo.until - Date.now() + 50, function() { undoTimer = null; refreshPanes(); });
    }
  }
  let byId = {}, unbound = 0;
  used.forEach(function(t) { byId[t.id] = t; });
  for (let i = 0; i < list.children.length; i++) {
    let item = list.children[i], a = item.mAddon;
    // A row just added (a Persona installed while the pane is open) gets its add-on when its
    // binding attaches, a moment after it is inserted: come back for it (owner 2026-10-08:
    // "dark fox preview in about:addons not working").
    if (!a) { unbound++; continue; }
    if (a.type !== "theme" || String(a.id).slice(-ID_SUFFIX.length) !== ID_SUFFIX) { continue; }   // a complete theme is not a Persona
    let tid = themeIdOf(a.id);
    try { decorateRow(doc, item, byId[tid] || null, !!(cur && cur.id === tid)); }
    catch (e) { log("persona row: " + e); }
  }
  win._jihadRetries = unbound ? (win._jihadRetries || 0) + 1 : 0;
  if (unbound && win._jihadRetries <= 20) {
    later(150, function() { if (paneWindows.has(win)) { decoratePane(win); } });
  }
}

function refreshPanes() {
  paneWindows.forEach(function(win) {
    try {
      if (win.closed) { paneWindows.delete(win); return; }
      later(60, function() { if (paneWindows.has(win)) { decoratePane(win); } });
    } catch (e) { paneWindows.delete(win); }
  });
}

function attachPane(win) {
  if (paneWindows.has(win)) { return; }
  paneWindows.add(win);
  let doc = win.document;
  try {
    win.QueryInterface(Ci.nsIInterfaceRequestor).getInterface(Ci.nsIDOMWindowUtils)
      .loadSheetUsingURIString("data:text/css," + encodeURIComponent(PANE_CSS), Ci.nsIDOMWindowUtils.AUTHOR_SHEET);
  } catch (e) { log("themes pane style: " + e); }
  let again = function() { try { decoratePane(win); } catch (e) { log("themes pane: " + e); } };
  doc.addEventListener("ViewChanged", again, true);
  let list = doc.getElementById("addon-list");
  if (list) {
    let pending = 0;
    new win.MutationObserver(function() {
      if (pending) { return; }
      pending = later(30, function() { pending = 0; again(); });
    }).observe(list, {childList: true});
  }
  win.addEventListener("unload", function() {
    paneWindows.delete(win);
  });
  again();
}

// A converted static theme's picture is the browser's own file: it goes when the theme does.
function forgetThemeFiles(id) {
  let m = /^static-theme:([0-9a-f]{24})$/.exec(String(id || ""));
  if (!m) { return; }
  try {
    let en = themesDir().directoryEntries;
    while (en.hasMoreElements()) {
      let f = en.getNext().QueryInterface(Ci.nsIFile);
      if (f.leafName.indexOf(m[1] + "-header.") === 0) { f.remove(false); log("removed " + f.leafName); }
    }
  } catch (e) { log("theme files could not be removed: " + e); }
}

function JihadPersonas() {
  Services.obs.addObserver(this, "content-document-global-created", false);
  Services.obs.addObserver(this, "chrome-document-global-created", false);
  try {
    let AM = Cu.import("resource://gre/modules/AddonManager.jsm", {}).AddonManager;
    AM.addAddonListener({
      onUninstalled: function(addon) {
        if (!addon || addon.type !== "theme") { return; }
        let id = themeIdOf(addon.id);
        // Undo must not bring back a Persona that is gone (its picture is deleted next).
        if (undo && undo.theme && undo.theme.id === id) { undo = null; }
        if (lastSeen && lastSeen.id === id) { lastSeen = null; }
        forgetThemeFiles(id);
        refreshPanes();
      },
      onEnabled: function() { refreshPanes(); },
      onDisabled: function() { refreshPanes(); }
    });
  } catch (e) { log("add-on listener: " + e); }
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
    if (aTopic === "content-document-global-created" || aTopic === "chrome-document-global-created") {
      // Every new window: about:addons is a XUL document, which never sends
      // document-element-inserted, so the page is recognised when it has loaded.
      let win = aSubject;
      try {
        // The Persona worn when the first page opens is the baseline Undo compares against.
        if (lastSeen === undefined) { try { lastSeen = manager().currentTheme || null; } catch (e) {} }
        // Before the page lists its add-ons: the bundled Personas join the list first.
        try { if (isAddonsDocument(win.document)) { offerBundledOnce(); } } catch (e) { log("bundled Personas: " + e); }
        win.addEventListener("load", function() {
          try { if (isAddonsDocument(win.document)) { attachPane(win); } } catch (e) { log("themes pane: " + e); }
        });
      } catch (e) {}
      return;
    }
    if (aTopic === "lightweight-theme-styling-update") {
      scheduleNote();
      refreshPanes();
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
              t[k] = resolveThemesUrl(t[k]); changed = true;
            }
          });
          if (isWallpaper(t.headerURL)) { t.headerFit = "cover"; }
          json = JSON.stringify(cardTheme(t));
        } catch (e) { log("theme could not be read: " + e); json = ""; }
      }
      // Only the INSTALLED theme is remembered (a preview is not: it would otherwise survive a
      // restart as if chosen), and the pref file is written only when it actually changes.
      // Decided on the theme's OWN id (cardTheme may have replaced an oversized one), and never
      // for an update that is the page's preview.
      let previewing = previewId !== null && rawId === previewId && Date.now() < previewUntil;
      if (previewId !== null && rawId !== previewId) { previewId = null; }   // something else is shown: the preview is over
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
