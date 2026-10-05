/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — the page's half of the long-press menu: Select All, Copy, Cut.
 *
 * The card shows the menu; these three act on the PAGE, so they are done here. Requests come
 * from the daemon as an observer notification (it gets them from the card over the frozen
 * adapter commands selectAll / copy / cut, which were no-op stubs until 2026-10-05):
 *
 *     "jihad-page-command"   subject = the content window, data = {"cmd": …, "dir": …}
 *         hold        a long-press landed at viewport x,y (`editable`: on a text field;
 *                     `focusedNow`: which the press itself has just focused).
 *                     On text it selects the word under the finger — unless the press is
 *                     inside what is already selected, which is then kept, as a right-click
 *                     keeps it. Answers with the notification "page-hold",
 *                     {"editable":…,"selected":…}; the card builds its menu from that.
 *         selectAll   select everything in what the last press was on: the field, or the page
 *         copy        hand the text selected there to the card
 *         cut         the same, then delete it (in a field; elsewhere it is a copy)
 *
 * HOW THE TEXT REACHES THE CARD. The system clipboard belongs to LunaSysMgr, and only a card
 * can write it. The text cannot ride the hit-test reply (the platform's HitTest.schema
 * forbids extra keys) nor the notification channel (any app may subscribe to that, and it is
 * cut at 200 characters). So it is written to a file in the card's own application
 * directory — the one place a card may load a local file from — as a one-call script, and
 * the card is told its NAME over the notification channel:
 *
 *     <appdir>/clip/copy-<n>.js      if(window.jihadClip){window.jihadClip("…");}
 *     notification                    category "page-copied", text "copy-<n>.js"
 *
 * The card loads it with a script tag, puts the text on the clipboard and says "Copied.".
 * The file is removed fifteen seconds later (and the directory emptied at start-up); the
 * clipboard itself is readable by every app on the device, so this exposes nothing the copy
 * was not already going to.
 */

"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");

const MAX_COPY = 1024 * 1024;       // characters; a page is text, not a file transfer
let gSeq = 0;
let gTimers = [];

function log(s) {
  try { Services.console.logStringMessage("page: " + s); } catch (e) {}
}

function notify(category, text) {
  try {
    let bag = Cc["@mozilla.org/hash-property-bag;1"].createInstance(Ci.nsIWritablePropertyBag2);
    bag.setPropertyAsAString("category", category);
    bag.setPropertyAsAString("text", String(text));
    Services.obs.notifyObservers(bag, "jihad-notify", null);
  } catch (e) { log("notify failed: " + e); }
}

//* The window that has the focus inside `win` (frames followed), and its focused element.
function focusOf(win) {
  let w = win, el = null;
  for (let depth = 0; depth < 8 && w && w.document; depth++) {
    el = w.document.activeElement;
    let inner = null;
    try { inner = el && el.contentWindow; } catch (e) { inner = null; }
    if (!inner) { break; }
    w = inner;
  }
  return {win: w, el: el};
}

function isTextField(el) {
  if (!el || !el.localName) { return false; }
  let n = el.localName.toLowerCase();
  return n === "textarea" || n === "input";
}

function selectedText(f) {
  // A field keeps its own selection; types without one (checkbox, number in some builds)
  // throw on selectionStart, and those have nothing to copy.
  if (isTextField(f.el)) {
    try {
      let a = f.el.selectionStart, b = f.el.selectionEnd;
      if (typeof a === "number" && typeof b === "number" && b > a) {
        return String(f.el.value).substring(a, b);
      }
    } catch (e) {}
    return "";
  }
  try { return String(f.win.getSelection() || ""); } catch (e) { return ""; }
}

//* What the last long-press was on: {win: weak reference to its window, editable}. The menu's
//* commands act on THAT — Select All after a press on the page's text selects the page even
//* while a text field has the focus, and Copy copies the page's selection, not the field's.
let gHold = null;

function plainSelection(w) {
  try {
    let sel = w.getSelection();
    return (sel && !sel.isCollapsed) ? String(sel) : "";
  } catch (e) { return ""; }
}

//* True when one of the selection's boxes is under (or within a fingertip's slop of) x,y.
function selectionIsAt(sel, x, y) {
  const SLOP = 12;
  for (let i = 0; i < sel.rangeCount; i++) {
    let rects = sel.getRangeAt(i).getClientRects();
    for (let k = 0; k < rects.length; k++) {
      let r = rects[k];
      if (x >= r.left - SLOP && x <= r.right + SLOP && y >= r.top - SLOP && y <= r.bottom + SLOP) {
        return true;
      }
    }
  }
  return false;
}

//* The long-press on plain content. Returns true when something is selected afterwards.
function holdSelect(win, x, y) {
  let w = win;
  for (let depth = 0; depth < 8; depth++) {
    let el = w.document.elementFromPoint(x, y), inner = null;
    try { inner = el && el.contentWindow; } catch (e) { inner = null; }
    if (!inner || !inner.document) { break; }
    let r = el.getBoundingClientRect();
    x -= r.left + el.clientLeft;
    y -= r.top + el.clientTop;
    w = inner;
  }
  gHold = {win: Cu.getWeakReference(w), editable: false};
  let pos = null, sel = null;
  try { pos = w.document.caretPositionFromPoint(x, y); } catch (e) { pos = null; }
  try {
    sel = w.getSelection();
    if (sel && !sel.isCollapsed && pos && pos.offsetNode) {
      for (let i = 0; i < sel.rangeCount; i++) {
        if (sel.getRangeAt(i).isPointInRange(pos.offsetNode, pos.offset)) { return true; }
      }
    }
    // Not inside it: this press replaces it. (The engine's own call ADDS a range, which
    // left every word ever pressed highlighted at once.)
    if (sel) { sel.removeAllRanges(); }
  } catch (e) {}
  try {
    let utils = w.QueryInterface(Ci.nsIInterfaceRequestor).getInterface(Ci.nsIDOMWindowUtils);
    utils.selectAtPoint(x, y, Ci.nsIDOMWindowUtils.SELECT_WORDNOSPACE);
  } catch (e) { log("hold: selectAtPoint threw: " + e); }
  try {
    sel = w.getSelection();
    // A press on the last half-letter of a word puts the caret at the word's END, and the
    // engine then takes that word and the next one ("bravo charlie", measured). One press
    // is one word: when two came back, keep the one the caret is in or at the end of.
    if (/\S\s+\S/.test(plainSelection(w)) && pos && pos.offsetNode && pos.offsetNode.nodeType === 3) {
      let d = pos.offsetNode.data, a = pos.offset, b = pos.offset;
      while (a > 0 && /\S/.test(d.charAt(a - 1))) { a--; }
      while (b < d.length && /\S/.test(d.charAt(b))) { b++; }
      if (b > a) {
        let r = w.document.createRange();
        r.setStart(pos.offsetNode, a);
        r.setEnd(pos.offsetNode, b);
        sel.removeAllRanges();
        sel.addRange(r);
      }
    }
    // A press on empty space selects the nearest word, wherever that is. Nothing was
    // pressed, so nothing is selected.
    if (sel && !sel.isCollapsed && !selectionIsAt(sel, x, y)) { sel.removeAllRanges(); }
  } catch (e) { log("hold: narrowing threw: " + e); }
  return /\S/.test(plainSelection(w));
}

//* What a command acts on: {field: focusOf() result} for a text field or editable region,
//* or {doc: window} for a page's own text.
function target(win) {
  let f = focusOf(win);
  if (gHold) {
    let hw = null;
    try { hw = gHold.win.get(); } catch (e) { hw = null; }
    try {
      if (hw && !hw.closed && hw.document) { return gHold.editable ? {field: f} : {doc: hw}; }
    } catch (e) {}
  }
  // No press to go by (a command from somewhere other than the long-press menu).
  return isTextField(f.el) ? {field: f} : {doc: f.win};
}

function selectAll(t) {
  if (t.doc) {
    let d = t.doc.document;
    t.doc.getSelection().selectAllChildren(d.body || d.documentElement);
    return;
  }
  if (isTextField(t.field.el)) {
    try { t.field.el.select(); return; } catch (e) {}
  }
  docShellOf(t.field.win).doCommand("cmd_selectAll");
}

function deleteSelection(f) {
  if (isTextField(f.el)) {
    try {
      let a = f.el.selectionStart, b = f.el.selectionEnd;
      if (typeof a === "number" && b > a) {
        f.el.setRangeText("", a, b, "end");
        // What typing would have told the page.
        let ev = f.el.ownerDocument.createEvent("Event");
        ev.initEvent("input", true, false);
        f.el.dispatchEvent(ev);
      }
      return;
    } catch (e) {}
  }
  docShellOf(f.win).doCommand("cmd_delete");
}

function docShellOf(win) {
  return win.QueryInterface(Ci.nsIInterfaceRequestor)
            .getInterface(Ci.nsIWebNavigation)
            .QueryInterface(Ci.nsIDocShell);
}

function writeClip(dir, text) {
  let d = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsIFile);
  d.initWithPath(dir);
  if (!d.exists()) { d.create(Ci.nsIFile.DIRECTORY_TYPE, 0o755); }
  let name = "copy-" + Date.now() + "-" + (++gSeq) + ".js";
  let f = d.clone();
  f.append(name);
  // JSON is a JavaScript string literal except for two line separators, which end a line in
  // a script and must be escaped there.
  let lit = JSON.stringify(text).replace(/\u2028/g, "\\u2028").replace(/\u2029/g, "\\u2029");
  let body = "if(window.jihadClip){window.jihadClip(" + lit + ");}\n";
  let out = Cc["@mozilla.org/network/file-output-stream;1"].createInstance(Ci.nsIFileOutputStream);
  out.init(f, 0x02 | 0x08 | 0x20, 0o644, 0);            // write | create | truncate
  let conv = Cc["@mozilla.org/intl/converter-output-stream;1"].createInstance(Ci.nsIConverterOutputStream);
  conv.init(out, "UTF-8", 0, 0);
  conv.writeString(body);
  conv.close();
  // Not kept: the card reads it within a second.
  let timer = Cc["@mozilla.org/timer;1"].createInstance(Ci.nsITimer);
  timer.initWithCallback({notify: function() {
    try { if (f.exists()) { f.remove(false); } } catch (e) {}
    let i = gTimers.indexOf(timer);
    if (i >= 0) { gTimers.splice(i, 1); }
  }}, 15000, Ci.nsITimer.TYPE_ONE_SHOT);
  gTimers.push(timer);
  return name;
}

//* Empty the clip directory: whatever is in it was written by a daemon that is gone.
function sweep(dir) {
  try {
    let d = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsIFile);
    d.initWithPath(dir);
    if (!d.exists() || !d.isDirectory()) { return; }
    let it = d.directoryEntries;
    while (it.hasMoreElements()) {
      let f = it.getNext().QueryInterface(Ci.nsIFile);
      if (/^copy-[0-9-]+\.js$/.test(f.leafName)) { try { f.remove(false); } catch (e) {} }
    }
  } catch (e) {}
}

let gSwept = false;

function JihadPageCommands() {
  Services.obs.addObserver(this, "jihad-page-command", false);
}

JihadPageCommands.prototype = {
  classID: Components.ID("{4e0c1b7a-6a53-4a5e-9d0f-0f6a3d1b8c27}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIObserver]),

  observe: function(aSubject, aTopic, aData) {
    if (aTopic !== "jihad-page-command") { return; }
    let req = null;
    try { req = JSON.parse(aData || "{}"); } catch (e) { req = {}; }
    let win = aSubject;
    if (!win || !win.document) { return; }
    try {
      if (req.dir && !gSwept) { gSwept = true; sweep(req.dir); }
      if (req.cmd === "hold") {
        let x = Number(req.x) || 0, y = Number(req.y) || 0, selected = false;
        if (req.editable) {
          let f = focusOf(win);
          gHold = {win: Cu.getWeakReference(f.win), editable: true};
          // A field the press has just focused: the caret goes where the finger is, as a
          // click puts it (focusing alone leaves it at the start, and Paste would land there).
          if (req.focusedNow && isTextField(f.el)) {
            try {
              let pos = f.win.document.caretPositionFromPoint(x, y);
              if (pos && pos.offsetNode === f.el) { f.el.setSelectionRange(pos.offset, pos.offset); }
            } catch (e) {}
          }
          selected = !!selectedText(f);
        } else {
          selected = holdSelect(win, x, y);
        }
        notify("page-hold", JSON.stringify({editable: !!req.editable, selected: selected}));
        log("hold: editable=" + !!req.editable + " selected=" + selected);
      } else if (req.cmd === "selectAll") {
        selectAll(target(win));
      } else if (req.cmd === "copy" || req.cmd === "cut") {
        let t = target(win);
        let text = t.doc ? plainSelection(t.doc) : selectedText(t.field);
        if (!text) { notify("page-copy-empty", "Nothing is selected."); return; }
        if (text.length > MAX_COPY) { text = text.substr(0, MAX_COPY); }
        if (!req.dir) { notify("page-copy-empty", "Copying is not available."); return; }
        notify("page-copied", writeClip(req.dir, text));
        log(req.cmd + ": " + text.length + " characters handed to the card");
        // Cut on a page's own text is a copy: there is nothing there to delete.
        if (req.cmd === "cut" && t.field) { deleteSelection(t.field); }
      }
    } catch (e) {
      log(String(req.cmd) + " threw: " + e);
      if (req.cmd === "copy" || req.cmd === "cut") {
        notify("page-copy-empty", "The text could not be copied.");
      }
    }
  }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadPageCommands]);
