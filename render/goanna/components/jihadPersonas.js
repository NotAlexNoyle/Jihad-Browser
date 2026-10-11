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
  // A colour-only Persona's drawn picture (a frame-to-toolbar gradient) is stretched over the whole
  // bar: scaled to cover, an 8x64 picture shows only its middle row (inspector 2026-10-11).
  if (t.headerFit === "stretch") { out.headerFit = "stretch"; }
  // A theme package's extras (cavekit-persona-formats.md): where the header sits, the pictures
  // under it, and the colours for the rest of the shell. Each checked again here.
  if (POSITIONS.indexOf(t.headerPosition) >= 0) { out.headerPosition = t.headerPosition; }
  if (TILINGS.indexOf(t.headerRepeat) >= 0) { out.headerRepeat = t.headerRepeat; }
  if (Array.isArray(t.layers)) {
    let ls = t.layers.slice(0, MAX_LAYERS).map(function(l) {
      let u = l && str(l.url, 2048);
      return (u && /^(file:\/\/\/|https:\/\/)[^\s"'()\\]+$/.test(u)) ?
        {url: u, position: POSITIONS.indexOf(l.position) >= 0 ? l.position : "right top",
         repeat: TILINGS.indexOf(l.repeat) >= 0 ? l.repeat : "no-repeat"} : null;
    }).filter(Boolean);
    if (ls.length) { out.layers = ls; }
  }
  let colors = slotColors(t.colors);
  if (colors) { out.colors = colors; }
  // Quotes and backslashes still escape to two characters: if what is left would not fit the
  // channel (6000), drop what a card can live without, in order, rather than send "null".
  let drop = ["colors", "layers", "footerURL", "author", "name", "headerURL"];
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

// A WebExtension colour: "#rrggbb", "rgb(...)", a name, or [r, g, b(, a)]. Only a first sieve:
// canonColor() parses what passes and is what goes anywhere ("/" is let through for the
// "rgb(1 2 3 / 50%)" form; canonColor's output never carries it).
function cssColor(c) {
  if (Array.isArray(c) && c.length >= 3) {
    let n = c.slice(0, 4).map(function(v) { return Number(v); });
    if (n.some(isNaN)) { return null; }
    return n.length === 4 ? "rgba(" + n.join(",") + ")" : "rgb(" + n.join(",") + ")";
  }
  if (typeof c === "string" && /^[#a-zA-Z0-9(),.%\/\s-]{1,40}$/.test(c.trim())) { return c.trim(); }
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
// A picture's size in pixels is bounded too, read from its own header before anything decodes it:
// a few compressed megabytes can declare a picture that decodes to gigabytes, which would kill the
// daemon on this 512 MB device before the user was even asked (Codex r13, P1). 4096x4096 (64 MB
// decoded) still takes the 3840x2160 wallpapers addons.mozilla.org serves.
const MAX_PIXELS = 4096 * 4096;
// What a theme-only package may say besides its theme. Anything else is behaviour this engine
// would silently drop (an override page, commands, a protocol handler…), so such a package is not
// treated as a Persona (Codex r13: a list of refused keys missed most of them).
const THEME_PACKAGE_KEYS = ["manifest_version", "name", "short_name", "version", "version_name", "description",
                            "author", "developer", "homepage_url", "icons", "theme", "dark_theme",
                            "browser_specific_settings", "applications", "default_locale"];

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
  return hexOf(ch.finish(false));
}

// A wallpaper-sized picture (AMO ships 3840x2160 ones) decodes to more than the engine's whole
// image cache on this device (32 MB, make-device-bundle.sh), so it is never drawn anywhere the
// engine paints it (the Personas pane, measured 2026-10-08). Kept at most WALLPAPER_WIDTH wide:
// the engine's encoder decodes it straight at that size. A picture made for a toolbar is left
// exactly as it is.
const WALLPAPER_WIDTH = 1600;
function shrinkWallpaper(file, ext) {
  if (ext === "gif") { return; }
  let sz = pictureSize(file);
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

// ---------------------------------------------------------------------------------------------
// What a theme package says beyond what the engine keeps (cavekit-persona-formats.md). The
// engine's theme manager keeps a Persona's own fields only (id, name, one header picture, an accent
// and a text colour…) and refuses one without a picture. More pictures and where they sit (R2),
// and more colours (R3), are kept BESIDE the Persona: extras-<hash of its id>.json in the
// profile's jihad-themes folder, added to what the cards are sent, and deleted with the Persona
// (forgetThemeFiles). A colour-only theme (R1) gets a small picture drawn from its colours, so the
// manager has the header it insists on.
const MAX_LAYERS = 6;                          // additional_backgrounds kept besides theme_frame
// All of one Persona's pictures together, decoded: 6 Mpx is 24 MB, under the engine's 32 MB image
// cache (a wallpaper is first shrunk to 1600 px wide). Pictures past it are left out.
const MAX_PERSONA_PIXELS = 6 * 1024 * 1024;
// Where a picture may sit and how it may repeat: Firefox's own words for these, nothing else.
const POSITIONS = ["bottom", "center", "left", "right", "top", "center bottom", "center center", "center top",
                   "left bottom", "left center", "left top", "right bottom", "right center", "right top"];
const TILINGS = ["no-repeat", "repeat", "repeat-x", "repeat-y"];
// The colour slots read beyond the frame and text colours (R3), with the older name of each.
const COLOR_SLOTS = ["toolbar", "toolbar_text", "toolbar_field", "toolbar_field_text", "toolbar_field_border",
                     "toolbar_field_focus", "toolbar_field_text_focus", "popup", "popup_text", "popup_border",
                     "popup_highlight", "popup_highlight_text", "icons", "ntp_background", "ntp_text"];
const COLOR_OLD_NAMES = {toolbar_text: "bookmark_text"};
const EXTRA_FILE = /^[0-9a-f]{24}-layer[1-9]\.(png|jpe?g|gif)$/;

function hexOf(raw) {
  let hex = "";
  for (let i = 0; i < raw.length; i++) { hex += ("0" + raw.charCodeAt(i).toString(16)).slice(-2); }
  return hex;
}

function sha256Str(s) {
  let conv = Cc["@mozilla.org/intl/scriptableunicodeconverter"].createInstance(Ci.nsIScriptableUnicodeConverter);
  conv.charset = "UTF-8";
  let data = conv.convertToByteArray(String(s), {});
  let ch = Cc["@mozilla.org/security/hash;1"].createInstance(Ci.nsICryptoHash);
  ch.init(ch.SHA256);
  ch.update(data, data.length);
  return hexOf(ch.finish(false));
}

// A colour as the engine's own parser reads it, written back as rgb() or rgba(): a slot is put
// into the shells' stylesheets, and text that merely uses colour characters ("red, 0 0 0 2000px
// black", an unclosed "rgb(") would otherwise become a shadow over the card or swallow the rest of
// the sheet (inspector 2026-10-11). Null when it is not one colour.
// A bare word must be a CSS colour name or "transparent": the parser also takes currentcolor,
// -moz-* keywords and system colours, which come out black or invisible with no element to resolve
// them against (inspector 2026-10-11, round 15).
function canonColor(c) {
  let css = cssColor(c);
  if (!css) { return null; }
  try {
    let du = Cc["@mozilla.org/inspector/dom-utils;1"].getService(Ci.inIDOMUtils);
    if (/^[a-zA-Z-]+$/.test(css) && css.toLowerCase() !== "transparent") {
      try { du.colorNameToRGB(css.toLowerCase()); } catch (e) { return null; }
    }
    let v = du.colorToRGBA(css);
    if (!v) { return null; }
    let r = Math.round(v.r), g = Math.round(v.g), b = Math.round(v.b), a = Number(v.a);
    if ([r, g, b].some(function(x) { return !(x >= 0 && x <= 255); }) || !(a >= 0 && a <= 1)) { return null; }
    return a < 1 ? "rgba(" + r + ", " + g + ", " + b + ", " + Math.round(a * 1000) / 1000 + ")" : "rgb(" + r + ", " + g + ", " + b + ")";
  } catch (e) { return null; }
}

// A theme's colour slots, each kept only if it is one colour (canonColor); null when there are none.
function slotColors(colors) {
  if (!colors || typeof colors !== "object") { return null; }
  let out = {}, n = 0;
  COLOR_SLOTS.forEach(function(k) {
    let v = canonColor(k in colors ? colors[k] : colors[COLOR_OLD_NAMES[k]]);
    if (v) { out[k] = v; n++; }
  });
  return n ? out : null;
}

// A CSS colour as {r, g, b} by the engine's own parser (names, #rgb, rgb(), hsl()…); null if none.
function rgbOf(c) {
  try {
    let v = Cc["@mozilla.org/inspector/dom-utils;1"].getService(Ci.inIDOMUtils).colorToRGBA(String(c));
    return v ? {r: v.r, g: v.g, b: v.b} : null;
  } catch (e) { return null; }
}

// A small truecolour PNG, `h` rows going from colour `top` to colour `bottom`, `w` pixels wide.
// The deflate stream is "stored" (uncompressed), so no compressor is needed: 8x64 is 1.6 KB.
let crcTable = null;
function crc32(bytes) {
  if (!crcTable) {
    crcTable = [];
    for (let n = 0; n < 256; n++) {
      let c = n;
      for (let k = 0; k < 8; k++) { c = (c & 1) ? (0xedb88320 ^ (c >>> 1)) : (c >>> 1); }
      crcTable.push(c >>> 0);
    }
  }
  let crc = 0xffffffff;
  for (let i = 0; i < bytes.length; i++) { crc = crcTable[(crc ^ bytes[i]) & 0xff] ^ (crc >>> 8); }
  return (crc ^ 0xffffffff) >>> 0;
}
function gradientPng(top, bottom, w, h) {
  let be32 = function(v) { return [(v >>> 24) & 255, (v >>> 16) & 255, (v >>> 8) & 255, v & 255]; };
  let raw = [];
  for (let y = 0; y < h; y++) {
    let f = h > 1 ? y / (h - 1) : 0;
    raw.push(0);                                       // filter: none
    for (let x = 0; x < w; x++) {
      raw.push(Math.round(top.r + (bottom.r - top.r) * f), Math.round(top.g + (bottom.g - top.g) * f),
               Math.round(top.b + (bottom.b - top.b) * f));
    }
  }
  let z = [0x78, 0x01];                                // zlib header, then stored blocks
  for (let i = 0; i < raw.length; i += 65535) {
    let n = Math.min(65535, raw.length - i);
    z.push(i + n >= raw.length ? 1 : 0, n & 255, (n >>> 8) & 255, ~n & 255, (~n >>> 8) & 255);
    for (let j = 0; j < n; j++) { z.push(raw[i + j]); }
  }
  let a = 1, b = 0;
  for (let i = 0; i < raw.length; i++) { a = (a + raw[i]) % 65521; b = (b + a) % 65521; }
  z = z.concat([(b >>> 8) & 255, b & 255, (a >>> 8) & 255, a & 255]);
  let chunk = function(type, data) {
    let body = type.split("").map(function(ch) { return ch.charCodeAt(0); }).concat(data);
    return be32(data.length).concat(body, be32(crc32(body)));
  };
  return [137, 80, 78, 71, 13, 10, 26, 10].concat(chunk("IHDR", be32(w).concat(be32(h), [8, 2, 0, 0, 0])),
                                                  chunk("IDAT", z), chunk("IEND", []));
}
function writeGradientPng(file, top, bottom, w, h) {
  let png = gradientPng(top, bottom, w, h);
  let fos = Cc["@mozilla.org/network/file-output-stream;1"].createInstance(Ci.nsIFileOutputStream);
  fos.init(file, 0x02 | 0x08 | 0x20, 0o644, 0);
  let bos = Cc["@mozilla.org/binaryoutputstream;1"].createInstance(Ci.nsIBinaryOutputStream);
  bos.setOutputStream(fos);
  try { bos.writeByteArray(png, png.length); } finally { try { bos.close(); } catch (e) {} }
}

// The extras a Persona may have, checked field by field (they are read back from the profile, and
// some come from an index): null when there is nothing.
function cleanExtras(x) {
  if (!x || typeof x !== "object") { return null; }
  let out = {}, n = 0;
  // "cover" was written only for a colour-only Persona's drawn gradient, before it was stretched
  // (inspector 2026-10-11): read as what it is now.
  if (x.fit === "cover" || x.fit === "stretch") { out.fit = "stretch"; n++; }
  if (POSITIONS.indexOf(x.position) >= 0) { out.position = x.position; n++; }
  if (TILINGS.indexOf(x.repeat) >= 0) { out.repeat = x.repeat; n++; }
  if (Array.isArray(x.layers)) {
    let ls = x.layers.slice(0, MAX_LAYERS).filter(function(l) {
      return l && typeof l.file === "string" && EXTRA_FILE.test(l.file);
    }).map(function(l) {
      return {file: l.file, position: POSITIONS.indexOf(l.position) >= 0 ? l.position : "right top",
              repeat: TILINGS.indexOf(l.repeat) >= 0 ? l.repeat : "no-repeat"};
    });
    if (ls.length) { out.layers = ls; n++; }
  }
  let c = slotColors(x.colors);
  if (c) { out.colors = c; n++; }
  return n ? out : null;
}

function extrasFile(id) {
  let f = themesDir();
  f.append("extras-" + sha256Str(String(id)).slice(0, 24) + ".json");
  return f;
}

let extrasCache = new Map();
function readExtras(id) {
  if (typeof id !== "string" || !id) { return null; }
  if (extrasCache.has(id)) { return extrasCache.get(id); }
  let x = null;
  try {
    let f = extrasFile(id);
    if (f.exists() && f.fileSize <= MAX_MANIFEST) {
      let is = Cc["@mozilla.org/network/file-input-stream;1"].createInstance(Ci.nsIFileInputStream);
      is.init(f, 0x01, 0, 0);
      let text = NetUtil.readInputStreamToString(is, is.available(), {charset: "UTF-8"});
      is.close();
      x = cleanExtras(JSON.parse(text));
    }
  } catch (e) { log("extras of a Persona could not be read: " + e); x = null; }
  extrasCache.set(id, x);
  return x;
}

// Keep (or, with nothing to keep, delete) a Persona's extras. Written before the Persona is worn:
// the styling update that follows reads them.
function writeExtras(id, x) {
  extrasCache.delete(String(id));
  let f = extrasFile(id);
  x = cleanExtras(x);
  if (!x) { try { if (f.exists()) { f.remove(false); } } catch (e) {} return; }
  let text = JSON.stringify(x);                        // ASCII only: file names, CSS words, colours
  let fos = Cc["@mozilla.org/network/file-output-stream;1"].createInstance(Ci.nsIFileOutputStream);
  fos.init(f, 0x02 | 0x08 | 0x20, 0o644, 0);
  try { fos.write(text, text.length); } finally { try { fos.close(); } catch (e) {} }
}

// A theme-only package as a Persona: {theme, files: [{pending, finalName}], extras}, or null when
// it is not one this browser can wear. Pictures (theme_frame, then additional_backgrounds in their
// order, at most MAX_LAYERS of those) are copied into PENDING files that become the Persona's only
// after the user's yes; one in a format the engine does not draw (SVG), too large, or past the
// Persona's pixel budget is left out. With no picture left, its colours make one (R1).
function staticThemeFromXpi(path, origin) {
  let file = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsIFile);
  try { file.initWithPath(path); } catch (e) { return null; }
  if (!file.exists()) { return null; }
  let zr = Cc["@mozilla.org/libjar/zip-reader;1"].createInstance(Ci.nsIZipReader);
  try { zr.open(file); } catch (e) { return null; }
  let pendings = [], handed = false;
  let pendingFile = function(ext) {
    let f = themesDir();
    f.append("pending-" + Date.now() + "-" + Math.floor(Math.random() * 1e9) + "." + ext);
    pendings.push(f);
    return f;
  };
  try {
    if (!zr.hasEntry("manifest.json") || zr.getEntry("manifest.json").realSize > MAX_MANIFEST) { return null; }
    let is = zr.getInputStream("manifest.json");
    let text = NetUtil.readInputStreamToString(is, is.available(), {charset: "UTF-8"});
    is.close();
    let m = JSON.parse(text.replace(/^﻿/, ""));
    if (!m || typeof m !== "object" || !m.theme || typeof m.theme !== "object") { return null; }
    let extra = Object.keys(m).filter(function(k) { return THEME_PACKAGE_KEYS.indexOf(k) < 0; });
    if (extra.length) { log("not a theme-only package: it also has " + extra.slice(0, 5).join(", ")); return null; }
    let obj = function(v) { return (v && typeof v === "object" && !Array.isArray(v)) ? v : {}; };
    let images = obj(m.theme.images), colors = obj(m.theme.colors), props = obj(m.theme.properties);
    // The pictures, top first: the classic theme_frame (headerURL in older manifests), then
    // additional_backgrounds — many newer AMO themes carry only those (owner 2026-10-08) — each
    // with its alignment and tiling (right top, no-repeat when the theme names none, as Firefox).
    let wanted = [];
    let frame = images.theme_frame || images.headerURL;
    if (typeof frame === "string" && frame) { wanted.push({path: frame, position: null, repeat: null}); }
    let more = Array.isArray(images.additional_backgrounds) ? images.additional_backgrounds : [];
    let al = Array.isArray(props.additional_backgrounds_alignment) ? props.additional_backgrounds_alignment : [];
    let ti = Array.isArray(props.additional_backgrounds_tiling) ? props.additional_backgrounds_tiling : [];
    for (let i = 0; i < more.length && i < MAX_LAYERS; i++) {
      if (typeof more[i] !== "string" || !more[i]) { continue; }
      wanted.push({path: more[i], position: POSITIONS.indexOf(al[i]) >= 0 ? al[i] : "right top",
                   repeat: TILINGS.indexOf(ti[i]) >= 0 ? ti[i] : "no-repeat"});
    }
    let kept = [], pixels = 0;
    wanted.forEach(function(w) {
      let p = w.path.replace(/^\.?\//, ""), shown = p.slice(0, 60);
      let ext = (/\.(png|jpe?g|gif)$/i.exec(p) || [null, ""])[1].toLowerCase();
      if (!ext) { log("picture " + shown + " left out: not a format this browser draws"); return; }
      if (!zr.hasEntry(p) || zr.getEntry(p).realSize > MAX_IMAGE) { log("picture " + shown + " left out: missing or over 4 MB"); return; }
      // Once the Persona's budget is spent, the rest are not even copied (inspector 2026-10-11).
      if (kept.length && pixels >= MAX_PERSONA_PIXELS) { log("picture " + shown + " left out: the Persona's pictures are big enough"); return; }
      let f = pendingFile(ext);
      copyEntry(zr, p, f, MAX_IMAGE);
      let sz = pictureSize(f);
      if (!sz || sz.w * sz.h > MAX_PIXELS || sz.type !== (ext === "jpeg" ? "jpg" : ext)) {
        log("the picture is " + (sz ? sz.w + "x" + sz.h + " " + sz.type : "of no size this browser can read") + ": not kept");
        return;
      }
      // What it will cost once shrunk, from its header, before anything decodes it.
      let after = (ext !== "gif" && sz.h >= WALLPAPER_MIN_HEIGHT && sz.w > WALLPAPER_WIDTH) ?
        WALLPAPER_WIDTH * Math.max(1, Math.round(sz.h * WALLPAPER_WIDTH / sz.w)) : sz.w * sz.h;
      // The first picture counts too: shrinking narrows a wallpaper but leaves a tall one tall
      // (1024x16384 passed as the first; inspector 2026-10-11, round 15).
      if (pixels + after > MAX_PERSONA_PIXELS) { log("picture " + shown + " (" + sz.w + "x" + sz.h + ") left out: over the Persona's " + MAX_PERSONA_PIXELS + " pixels"); return; }
      // A picture that cannot be shrunk is kept as it came (it may not draw in the pane; the
      // cards draw it) rather than refusing the Persona (inspector 2026-10-08).
      try { shrinkWallpaper(f, ext); }
      catch (e) {
        log("wallpaper kept at its own size: " + e);
        try { let t = f.clone(); t.leafName = f.leafName + ".scaled"; if (t.exists()) { t.remove(false); } } catch (e2) {}
      }
      sz = pictureSize(f) || sz;
      if (pixels + sz.w * sz.h > MAX_PERSONA_PIXELS) { log("picture " + shown + " left out: the Persona's pictures are big enough"); return; }
      pixels += sz.w * sz.h;
      kept.push({file: f, ext: ext, position: w.position, repeat: w.repeat, sha: sha256Hex(f)});
    });
    let accent = canonColor(colors.frame || colors.accentcolor), textc = canonColor(colors.tab_background_text || colors.textcolor || colors.toolbar_text);
    let slots = slotColors(colors);
    let name = String(m.name || "Theme").replace(/[\x00-\x1f\x7f]/g, " ").slice(0, 80);
    let extras = {};
    if (slots) { extras.colors = slots; }
    if (!kept.length) {
      // Colour-only (R1): the bar in its frame colour, going to its toolbar colour when it has one.
      let top = accent ? rgbOf(accent) : null;
      if (!top) { log("no picture and no frame colour: not a Persona"); return null; }
      let bottom = (slots && slots.toolbar && rgbOf(slots.toolbar)) || top;
      let f = pendingFile("png");
      writeGradientPng(f, top, bottom, 8, 64);
      kept.push({file: f, ext: "png", position: null, repeat: null, sha: sha256Hex(f), made: true});
      extras.fit = "stretch";
    }
    // A Persona of one picture, placed as Firefox places a header, is named by that picture as it
    // always was (the same art is the same Persona); any other by everything that makes it up.
    let head = kept[0];
    let plain = kept.length === 1 && !head.made && (!head.position || head.position === "right top") &&
                (!head.repeat || head.repeat === "no-repeat");
    let hash = plain ? head.sha.slice(0, 24) :
      sha256Str(JSON.stringify({name: name, pictures: kept.map(function(k) { return [k.sha, k.position, k.repeat]; }),
                                accent: accent, text: textc, colors: slots})).slice(0, 24);
    if (head.position && head.position !== "right top") { extras.position = head.position; }
    if (head.repeat && head.repeat !== "no-repeat") { extras.repeat = head.repeat; }
    let files = [{pending: head.file, finalName: hash + "-header." + head.ext}];
    if (kept.length > 1) {
      extras.layers = kept.slice(1).map(function(k, i) {
        let fn = hash + "-layer" + (i + 1) + "." + k.ext;
        files.push({pending: k.file, finalName: fn});
        return {file: fn, position: k.position || "right top", repeat: k.repeat || "no-repeat"};
      });
    }
    let theme = {id: "static-theme:" + hash, name: name, headerURL: "resource://" + THEMES_RESOURCE + "/" + files[0].finalName};
    if (accent) { theme.accentcolor = accent; }
    if (textc) { theme.textcolor = textc; }
    if (m.author && typeof m.author === "string") { theme.author = m.author.slice(0, 80); }
    if (typeof m.version === "string" && /^[\w.+-]{1,20}$/.test(m.version)) { theme.version = m.version; }
    // The pictures left out go now; the rest are handed over.
    pendings.forEach(function(f) {
      if (files.some(function(x) { return x.pending === f; })) { return; }
      try { if (f.exists()) { f.remove(false); } } catch (e) {}
    });
    handed = true;
    return {theme: theme, files: files, extras: Object.keys(extras).length ? extras : null};
  } catch (e) {
    log("static theme could not be read: " + e);
    return null;
  } finally {
    try { zr.close(); } catch (e) {}
    if (!handed) { pendings.forEach(function(f) { try { if (f.exists()) { f.remove(false); } } catch (e) {} }); }
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
  let dropPending = function() {
    got.files.forEach(function(x) { try { if (x.pending.exists()) { x.pending.remove(false); } } catch (e) {} });
  };
  if (!confirmTheme(host, got.theme.name)) {
    log("static theme [" + got.theme.name + "] from " + host + " declined");
    dropPending();
    bag.setPropertyAsBool("converted", true);          // it was a theme; the user said no
    return;
  }
  try {
    got.files.forEach(function(x) {
      let dest = themesDir();
      dest.append(x.finalName);
      if (dest.exists()) { x.pending.remove(false); }  // the same picture, already here
      else { x.pending.moveTo(null, x.finalName); }
    });
    writeExtras(got.theme.id, got.extras);
  } catch (e) {
    log("static theme pictures could not be kept: " + e);
    dropPending();
    return;
  }
  ensureThemesResource();
  let LWTM = manager();
  previewId = null;
  LWTM.resetPreview();
  wearTheme(got.theme);
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
  // The add-on site's permission comes from the pref xpinstall.whitelist.add (cavekit-addons-site.md
  // R11); the add-on manager imports it only when an install is checked, so it is imported here too
  // (once per run; it does nothing after that).
  try {
    Cu.import("resource://gre/modules/PermissionsUtils.jsm", {}).PermissionsUtils.importFromPrefs("xpinstall.", "install");
  } catch (e) {}
  try { return Services.perms.testPermission(uri, "install") === Services.perms.ALLOW_ACTION; }
  catch (e) { return false; }
}

// A Persona from a site or an index may name the same colour slots a theme package does
// (`colors`, cavekit-persona-formats.md R4); pictures beyond its header it may not. Kept as its
// extras — and a reinstall without them clears the old ones.
function keepIndexColors(id, json) {
  let raw = null;
  try { raw = JSON.parse(String(json || "")); } catch (e) { raw = null; }
  let colors = raw && typeof raw === "object" ? slotColors(raw.colors) : null;
  try { writeExtras(id, colors ? {colors: colors} : null); } catch (e) { log("colours of a Persona could not be kept: " + e); }
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
  keepIndexColors(theme.id, req.json);
  wearTheme(theme);               // persists, downloads the images, notifies
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
// "Get Personas" opens the browser's own gallery (about:personas), unless the pref Basilisk uses
// for its "Get More Themes" names another place.
const PREF_GET_MORE = "lightweightThemes.getMoreURL";
function getPersonasURL() {
  try {
    let u = Services.prefs.getCharPref(PREF_GET_MORE);
    if (/^(https:|about:personas$)/.test(u)) { return u; }
  } catch (e) {}
  return "about:personas";
}
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

// Width, height and kind ("png", "gif", "jpg") of a picture, read from its bytes the way the
// decoders read them; null if unknown. A GIF is as big as its first frame when that frame does not
// fit the declared screen (or always, in a GIF87a): UXP's nsGIFDecoder2 and WebKit's GIF reader both
// grow it. A JPEG's markers may be preceded by any number of 0xFF fill bytes, which libjpeg skips;
// an SOS before any SOF is not a picture libjpeg reads (inspector 2026-10-11, round 15: the old scan
// read a GIF's screen and stepped onto a fake SOF, so a 3000x3000 picture passed as 1x1).
function imageSize(file) {
  let bis = null;
  try {
    let is = Cc["@mozilla.org/network/file-input-stream;1"].createInstance(Ci.nsIFileInputStream);
    is.init(file, 0x01, 0, 0);
    bis = Cc["@mozilla.org/binaryinputstream;1"].createInstance(Ci.nsIBinaryInputStream);
    bis.setInputStream(is);
    let b = bis.readByteArray(Math.min(bis.available(), 256 * 1024));
    let be16 = function(o) { return (b[o] << 8) | b[o + 1]; };
    let le16 = function(o) { return b[o] | (b[o + 1] << 8); };
    if (b.length > 24 && b[0] === 0x89 && b[1] === 0x50 && b[2] === 0x4e && b[3] === 0x47) {   // PNG: IHDR
      return {w: be16(16) * 65536 + be16(18), h: be16(20) * 65536 + be16(22), type: "png"};
    }
    if (b.length > 13 && b[0] === 0x47 && b[1] === 0x49 && b[2] === 0x46) {                     // GIF
      let sw = le16(6), sh = le16(8), v87 = b[4] === 0x37, o = 13;
      if (b[10] & 0x80) { o += 3 * (1 << ((b[10] & 7) + 1)); }       // the global colour table
      while (o < b.length) {
        if (b[o] === 0x2c) {                                          // the first image descriptor
          if (o + 9 > b.length) { return null; }
          let fw = le16(o + 5), fh = le16(o + 7);
          if (sh < fh || sw < fw || v87) { sw = fw; sh = fh; }
          return {w: sw, h: sh, type: "gif"};
        }
        if (b[o] !== 0x21) { return null; }                           // the trailer, or not a GIF
        o += 2;                                                       // an extension: its sub-blocks
        while (o < b.length && b[o] !== 0) { o += b[o] + 1; }
        o++;
      }
      return null;
    }
    if (b.length > 4 && b[0] === 0xff && b[1] === 0xd8) {                                       // JPEG: SOFn
      let o = 2;
      while (o + 9 < b.length) {
        if (b[o] !== 0xff) { o++; continue; }                         // stray bytes: skipped, as libjpeg does
        let m = b[o + 1];
        if (m === 0xff) { o++; continue; }                            // a fill byte
        if (m === 0x00) { o += 2; continue; }
        if (m >= 0xc0 && m <= 0xcf && m !== 0xc4 && m !== 0xc8 && m !== 0xcc) {
          return {w: be16(o + 7), h: be16(o + 5), type: "jpg"};
        }
        if (m === 0xda || m === 0xd9) { return null; }                // SOS or EOI before any SOF
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

// The size the engine's own decoder gives a picture: imgITools.decodeImage on an anonymous image
// decodes only its metadata (RasterImage::OnImageDataComplete, a sync load) and allocates no
// pixels. Null when the engine cannot read it as `type`.
function decodedSize(file, type) {
  let src = null;
  try {
    src = Cc["@mozilla.org/network/file-input-stream;1"].createInstance(Ci.nsIFileInputStream);
    src.init(file, 0x01, 0, 0);
    let c = Cc["@mozilla.org/image/tools;1"].getService(Ci.imgITools).decodeImage(src, type === "jpg" ? "image/jpeg" : "image/" + type);
    return (c && c.width > 0 && c.height > 0) ? {w: c.width, h: c.height} : null;
  } catch (e) {
    return null;
  } finally {
    try { if (src) { src.close(); } } catch (e) {}
  }
}

// A theme package's picture, sized before anything keeps or shrinks it: the larger of what its bytes
// say (imageSize) and what the engine's decoder says, so neither a lying header nor a decoder quirk
// gets a picture past the bounds. A picture already over MAX_PIXELS by its bytes is not handed to
// the decoder at all. Null when either cannot read it.
function pictureSize(file) {
  let s = imageSize(file);
  if (!s || s.w < 1 || s.h < 1) { return null; }
  if (s.w * s.h > MAX_PIXELS) { return s; }
  let d = decodedSize(file, s.type);
  if (!d) { return null; }
  return {w: Math.max(s.w, d.w), h: Math.max(s.h, d.h), type: s.type};
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
                formerNames: Array.isArray(p.formerNames) ? p.formerNames.map(String) : [],
                license: String(p.license || ""), licenseURL: String(p.licenseURL || ""),
                source: String(p.source || ""), modified: p.modified ? String(p.modified) : "",
                theme: {id: "static-theme:" + hash, name: String(p.name || p.slug).slice(0, 80),
                        headerURL: "resource://" + THEMES_RESOURCE + "/" + hash + "-header." + ext,
                        accentcolor: canonColor(p.accentcolor) || undefined, textcolor: canonColor(p.textcolor) || undefined,
                        author: String(p.author || "Jihad Browser").slice(0, 80), version: String(p.version || "1.0").slice(0, 20)}});
    });
  } catch (e) { log("bundled Personas: " + e); }
  return out;
}

// Wear a Persona just installed or re-installed. One with the id already worn (the same picture
// again with other colours, a site Persona with new colour slots) needs rewear(): the manager sends
// no styling update for the id it is wearing, so the cards would keep the old look (inspector
// 2026-10-11).
function wearTheme(theme) {
  let LWTM = manager(), cur = null;
  try { cur = LWTM.currentTheme; } catch (e) { cur = null; }
  if (cur && theme && cur.id === theme.id) { rewear(theme); }
  else { LWTM.currentTheme = theme; }
}

// Put a Persona on again with new details (same id). The manager does nothing for the id already
// worn — no styling update — so the cards and jihad.theme.current would keep the old details
// (inspector 2026-10-08): the update is sent here.
function rewear(theme) {
  let LWTM = manager();
  previewId = null;
  LWTM.currentTheme = theme;
  // For the id already worn the manager neither fetches the pictures again nor tells the windows: an
  // update from the add-on site kept the old picture in the profile (measured 2026-10-11).
  // themeChanged does both: it persists the pictures and sends the styling update (twice: now, and
  // once the files are in place).
  try { LWTM.themeChanged(LWTM.currentTheme || theme); }
  catch (e) {
    log("re-wear: " + e);
    try { Services.obs.notifyObservers(null, "lightweight-theme-styling-update", JSON.stringify(LWTM.currentThemeForDisplay || theme)); } catch (e2) {}
  }
}

// How many Personas the manager keeps (it drops the oldest beyond this, as uninstalls).
function maxUsed() {
  try { return Services.prefs.getIntPref("lightweightThemes.maxUsedThemes"); } catch (e) { return 30; }
}

// A Persona in the list with the same picture as a bundled one (installed from a site before it
// was bundled, say) gets the bundled entry's name, author and version: it is that Persona.
function sameDetails(t, fresh) {
  return t.name === fresh.name && t.author === fresh.author && t.version === fresh.version &&
         t.accentcolor === fresh.accentcolor && t.textcolor === fresh.textcolor;
}
function refreshBundledDetails(all) {
  let LWTM = manager(), used = [], cur = null;
  try { used = LWTM.usedThemes || []; cur = LWTM.currentTheme; } catch (e) { return; }
  let byId = {}, changed = false, wear = null;
  all.forEach(function(b) { byId[b.theme.id] = b.theme; });
  let now = used.map(function(t) {
    let fresh = byId[t.id];
    if (!fresh || sameDetails(t, fresh)) { return t; }
    changed = true;
    if (cur && cur.id === t.id) { wear = fresh; }
    return fresh;
  });
  if (!changed) { return; }
  if (wear) { rewear(wear); }
  try { now = (LWTM.usedThemes || []).map(function(t) { return byId[t.id] || t; }); } catch (e) {}
  let str = Cc["@mozilla.org/supports-string;1"].createInstance(Ci.nsISupportsString);
  str.data = JSON.stringify(now);
  Services.prefs.getBranch("lightweightThemes.").setComplexValue("usedThemes", Ci.nsISupportsString, str);
  log("bundled Personas: details brought up to date");
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
  if (done === key) { refreshBundledDetails(all); return; }
  let offered = {};
  try { JSON.parse(Services.prefs.getCharPref(PREF_BUNDLED_NAMES)).forEach(function(n) { offered[n] = true; }); }
  catch (e) {
    // A profile offered the set before the names were kept was offered the project's own five
    // (then the whole set), under their names of the time — not Personas added since.
    if (done) {
      all.forEach(function(b) {
        if (b.theme.author === "Jihad Browser") { [b.theme.name].concat(b.formerNames).forEach(function(n) { offered[n] = true; }); }
      });
    }
  }
  let LWTM = manager();
  let used = [];
  try { used = LWTM.usedThemes || []; } catch (e) { used = []; }
  let cur = null;
  try { cur = LWTM.currentTheme; } catch (e) {}
  let have = {}, wear = null, dropped = {}, droppedIds = [], replacing = {}, renamed = false;
  used.forEach(function(t) { have[t.id] = true; });
  // An entry by "Jihad Browser" under a bundled Persona's name (or one of its earlier names) is
  // that Persona: with the same picture it is brought up to date in place (a renamed one keeps
  // its place, and stays worn if it was); with another picture it is an earlier edition, which
  // the new picture replaces.
  let newIds = {}, byName = {}, wasOffered = function(b) {
    return [b.theme.name].concat(b.formerNames).some(function(n) { return offered[n]; });
  };
  all.forEach(function(b) {
    newIds[b.theme.id] = b;
    [b.theme.name].concat(b.formerNames).forEach(function(n) { byName[n] = b; });
  });
  used = used.map(function(t) {
    let b = byName[t.name];
    if (!b || t.author !== "Jihad Browser") { return t; }
    if (newIds[t.id]) {
      let fresh = newIds[t.id].theme;
      if (t.name !== fresh.name || t.version !== fresh.version || t.accentcolor !== fresh.accentcolor ||
          t.textcolor !== fresh.textcolor) {
        renamed = true;
        if (cur && cur.id === t.id) { wear = newIds[t.id]; }
        return fresh;
      }
      return t;
    }
    dropped[t.id] = true; droppedIds.push(t.id); replacing[b.theme.name] = true;
    if (cur && cur.id === t.id) { wear = b; }
    return t;
  });
  let add = all.filter(function(b) {
    return !have[b.theme.id] && (!wasOffered(b) || replacing[b.theme.name]);
  });
  // Never past the manager's limit: beyond it the manager would drop the oldest entries — the
  // ones just added — as uninstalls, deleting their pictures (inspector 2026-10-08). What does
  // not fit is not marked as offered, so it is offered again when there is room.
  let room = maxUsed() - used.filter(function(t) { return !dropped[t.id]; }).length;
  let leftOver = add.length > Math.max(0, room);
  if (leftOver) { add = add.slice(0, Math.max(0, room)); }
  add.forEach(function(b) {
    let dest = themesDir();
    dest.append(b.finalName);
    if (!dest.exists()) { b.file.copyTo(themesDir(), b.finalName); }
  });
  ensureThemesResource();
  // The list is rewritten in place: a renamed Persona keeps its place, and a worn Persona's new
  // edition takes the old one's place at the head (the manager wears usedThemes[0] while a theme
  // is selected). Wearing the new one through the manager first grew the list for a moment,
  // which at its limit drops the oldest as uninstalls; rewriting without the old one first left
  // the manager wearing whatever came next (inspector 2026-10-08). Then the cards are told.
  if (add.length || droppedIds.length || renamed) {
    let now = [];
    try { now = LWTM.usedThemes || []; } catch (e) { now = used; }
    let present = {};
    now = now.map(function(t) {
               if (wear && cur && t.id === cur.id) { return wear.theme; }   // worn: renamed, or its new edition
               let b = newIds[t.id];
               return (b && t.author === "Jihad Browser") ? b.theme : t;   // renamed in place
             })
             .filter(function(t) { present[t.id] = true; return !dropped[t.id]; });
    add.forEach(function(b) { if (!present[b.theme.id]) { now.push(b.theme); } });
    let str = Cc["@mozilla.org/supports-string;1"].createInstance(Ci.nsISupportsString);
    str.data = JSON.stringify(now);
    Services.prefs.getBranch("lightweightThemes.").setComplexValue("usedThemes", Ci.nsISupportsString, str);
  }
  if (wear) { rewear(wear.theme); }
  droppedIds.forEach(function(id) { forgetThemeFiles(id); });
  // Offered: what is in the list now, what was just added, and what was offered before.
  let listed = {};
  try { (LWTM.usedThemes || []).forEach(function(t) { listed[t.id] = true; }); } catch (e) {}
  add.forEach(function(b) { listed[b.theme.id] = true; });
  all.forEach(function(b) {
    if (listed[b.theme.id] || wasOffered(b)) {
      [b.theme.name].concat(b.formerNames).forEach(function(n) { offered[n] = true; });
    }
  });
  Services.prefs.setCharPref(PREF_BUNDLED_NAMES, JSON.stringify(Object.keys(offered)));
  // The edition is marked done only when all of it found room: otherwise the next visit would
  // stop at the "done" check and never offer the rest once Personas are uninstalled (Codex r13).
  if (!leftOver) { Services.prefs.setCharPref(PREF_BUNDLED, key); }
  refreshBundledDetails(all);
  log("bundled Personas offered: " + add.length + ", replaced: " + droppedIds.length + (renamed ? ", renamed" : "") + " (" + key + ")");
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
  get.addEventListener("command", function() { win.location.href = getPersonasURL(); });
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
  let x = theme ? readExtras(theme.id) : null;
  item.style.setProperty("background-color", accent || "#3a3a3a", "important");
  if (header) {
    // A wallpaper (or a colour-only Persona's own picture) is shown whole across the row, a
    // toolbar picture from its top right corner (where Firefox anchors it) — both filling the
    // row. A layered Persona's other pictures go under it, each where the theme puts it.
    // A layered Persona's first picture keeps its own size and place (filling the row would hide
    // the pictures under it); a colour-only one's drawn picture is stretched over the row.
    let wall = !!(theme && isWallpaper(theme.headerURL));
    let placed = !!(x && (x.layers || x.position || (x.repeat && x.repeat !== "no-repeat")));   // a tiled pattern too
    let imgs = [header], pos = [wall ? "center center" : ((x && x.position) || "right top")];
    let rep = [(x && x.repeat) || "no-repeat"];
    let size = [(x && x.fit === "stretch") ? "100% 100%" : ((placed && !wall) ? "auto" : "cover")];
    ((x && x.layers) || []).forEach(function(l) {
      let u = cssUrl("resource://" + THEMES_RESOURCE + "/" + l.file);
      if (u) { imgs.push(u); pos.push(l.position); rep.push(l.repeat); size.push("auto"); }
    });
    item.style.setProperty("background-image", imgs.join(", "), "important");
    item.style.setProperty("background-position", pos.join(", "), "important");
    item.style.setProperty("background-size", size.join(", "), "important");
    item.style.setProperty("background-repeat", rep.join(", "), "important");
  }
  item.style.setProperty("color", text || "#ffffff", "important");
  item.style.setProperty("min-height", "96px", "important");
  item.style.MozBoxOrdinalGroup = isCurrent ? "0" : "1";
  let anon = function(attr, val) { return doc.getAnonymousElementByAttribute(item, attr, val); };
  // The text sits on a pane in the opposite of its colour, so the name reads over any picture.
  let pane = anon("class", "content-inner-container");
  if (pane) {
    let light = isLight(text || "#ffffff", doc);
    pane.style.setProperty("background-color", light ? "rgba(0,0,0,0.45)" : "rgba(255,255,255,0.72)", "important");
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

// A converted static theme's pictures are the browser's own files, and any Persona's extras are
// too: they go when the Persona does.
function forgetThemeFiles(id) {
  if (!id) { return; }
  try { let f = extrasFile(id); if (f.exists()) { f.remove(false); log("removed " + f.leafName); } } catch (e) {}
  extrasCache.delete(String(id));
  let m = /^static-theme:([0-9a-f]{24})$/.exec(String(id));
  if (!m) { return; }
  try {
    let en = themesDir().directoryEntries;
    while (en.hasMoreElements()) {
      let f = en.getNext().QueryInterface(Ci.nsIFile);
      if (f.leafName.indexOf(m[1] + "-header.") === 0 || f.leafName.indexOf(m[1] + "-layer") === 0) {
        f.remove(false); log("removed " + f.leafName);
      }
    }
  } catch (e) { log("theme files could not be removed: " + e); }
}

// about:personas (packaging/prefsui/content/personas.js) asks through "jihad-personas-gallery":
//   list    → the bundled Personas, each with its credits and whether it is installed / worn;
//   wear    → put a bundled one on (installing it if it is not in the list);
//   install → a Persona from the online index: read like a site's, confirmed like a site's.
function galleryRequest(bag) {
  let action = "";
  try { action = bag.getPropertyAsAString("action"); } catch (e) {}
  let get = function(k) { try { return bag.getPropertyAsAString(k); } catch (e) { return ""; } };
  let reply = function(v) { bag.setPropertyAsAString("result", JSON.stringify(v)); };
  let LWTM = manager(), cur = null, used = [];
  try { cur = LWTM.currentTheme; used = LWTM.usedThemes || []; } catch (e) {}
  let installed = {};
  used.forEach(function(t) { installed[t.id] = true; });
  if (action === "list") {
    reply(bundledPersonas().map(function(b) {
      return {key: b.theme.id, name: b.theme.name, author: b.theme.author, version: b.theme.version,
              license: b.license, licenseURL: b.licenseURL, source: b.source,
              accentcolor: b.theme.accentcolor, textcolor: b.theme.textcolor,
              preview: Services.io.newFileURI(b.file).spec,
              installed: !!installed[b.theme.id], worn: !!(cur && cur.id === b.theme.id)};
    }));
    return;
  }
  if (action === "wear") {
    let b = bundledPersonas().filter(function(x) { return x.theme.id === get("key"); })[0];
    if (!b) { reply({ok: false}); return; }
    let dest = themesDir();
    dest.append(b.finalName);
    if (!dest.exists()) { b.file.copyTo(themesDir(), b.finalName); }
    ensureThemesResource();
    previewId = null;
    LWTM.resetPreview();
    wearTheme(b.theme);
    notify("info", "Persona applied: " + b.theme.name);
    reply({ok: true});
    return;
  }
  if (action === "install") {
    let origin = get("origin");
    if (!/^https:\/\//.test(origin)) { reply({ok: false, message: "The gallery must be an https address."}); return; }
    let theme = null;
    try { theme = LWTM.parseTheme(get("json"), origin); } catch (e) { theme = null; }
    if (!theme) { reply({ok: false, message: "That Persona could not be read."}); return; }
    let badImage = ["headerURL", "footerURL"].some(function(k) {
      return theme[k] && !/^https:/i.test(String(theme[k]));
    });
    if (badImage) { reply({ok: false, message: "That Persona's pictures are not served over https."}); return; }
    if (!confirmTheme(hostOf(origin), String(theme.name || "Persona"))) { reply({ok: false, message: "Not installed."}); return; }
    keepIndexColors(theme.id, get("json"));
    previewId = null;
    LWTM.resetPreview();
    wearTheme(theme);
    notify("info", "Persona applied: " + theme.name);
    reply({ok: true});
    return;
  }
  // The Personas installed and the one worn, for the gallery's buttons and its update check
  // (cavekit-addons-site.md R12, R13).
  if (action === "state") {
    reply({worn: cur ? cur.id : null,
           installed: used.slice(0, 50).map(function(t) {
             return {id: t.id, name: t.name, version: t.version || "", updateURL: t.updateURL || ""};
           })});
    return;
  }
  // A newer version of an installed Persona, from its own update address: read like a site's,
  // confirmed like a site's, put in the old one's place; worn stays worn.
  if (action === "update") {
    let origin = get("origin");
    if (!/^https:\/\//.test(origin)) { reply({ok: false, message: "Updates come only over https."}); return; }
    let theme = null;
    try { theme = LWTM.parseTheme(get("json"), origin); } catch (e) { theme = null; }
    if (!theme || !installed[theme.id]) { reply({ok: false, message: "That update could not be read."}); return; }
    let badImage = ["headerURL", "footerURL"].some(function(k) { return theme[k] && !/^https:/i.test(String(theme[k])); });
    if (badImage) { reply({ok: false, message: "That Persona's pictures are not served over https."}); return; }
    if (!confirmTheme(hostOf(origin), String(theme.name || "Persona"))) { reply({ok: false, message: "Not updated."}); return; }
    keepIndexColors(theme.id, get("json"));
    if (cur && cur.id === theme.id) {
      rewear(theme);                                  // the manager re-fetches its pictures
    } else {
      let str = Cc["@mozilla.org/supports-string;1"].createInstance(Ci.nsISupportsString);
      str.data = JSON.stringify(used.map(function(t) { return t.id === theme.id ? theme : t; }));
      Services.prefs.getBranch("lightweightThemes.").setComplexValue("usedThemes", Ci.nsISupportsString, str);
    }
    log("[" + theme.name + "] updated to " + (theme.version || "?") + " from " + hostOf(origin));
    refreshPanes();
    reply({ok: true});
    return;
  }
  reply(null);
}

function JihadPersonas() {
  Services.obs.addObserver(this, "jihad-personas-gallery", false);
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
          let x = readExtras(t.id);
          if (x) {
            if (x.fit) { t.headerFit = x.fit; }
            if (x.position) { t.headerPosition = x.position; }
            if (x.repeat) { t.headerRepeat = x.repeat; }
            if (x.layers) {
              t.layers = x.layers.map(function(l) {
                return {url: resolveThemesUrl("resource://" + THEMES_RESOURCE + "/" + l.file), position: l.position, repeat: l.repeat};
              });
            }
            if (x.colors) { t.colors = x.colors; }
          }
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
    if (aTopic === "jihad-personas-gallery") {
      let bag = null;
      try { bag = aSubject.QueryInterface(Ci.nsIWritablePropertyBag2); } catch (e) { bag = null; }
      if (bag) { try { galleryRequest(bag); } catch (e) { log("gallery: " + e); } }
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
