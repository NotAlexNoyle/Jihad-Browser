/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — the page's half of the long-press menu: Select All, Copy, Cut.
 *
 * The card shows the menu; these act on the PAGE, so they are done here. Requests come from
 * the daemon as an observer notification. It gets them from the card over the frozen adapter
 * commands selectAll / copy / cut (no-op stubs until 2026-10-05), queues them, and raises
 * them from its guarded pump tick — never from the socket callback, because two of them can
 * run page script (an `input` event, a selection listener):
 *
 *     "jihad-page-command"   subject = the page's top content window,
 *                            data = {"cmd": …, "dir": …, …}
 *
 *         holdArm     a long-press is about to be given to the page as a contextmenu event.
 *         hold        it has been (viewport x,y; `editable`: on a text field; `readOnly`: one
 *                     that takes no edits — copyable, nothing else). If the page
 *                     claimed the event (preventDefault — it has a menu of its own) nothing
 *                     more happens. Otherwise: on text, the word under the finger is
 *                     selected — unless the press is inside what is already selected, which
 *                     is then kept, as a right-click keeps it; in a field, the caret goes to
 *                     the finger on the same rule. Answers with the notification
 *                     "page-hold", {"editable":…,"readOnly":…,"selected":…}; the card builds
 *                     its menu from that.
 *         selectAll   select everything in what the last press was on: the field, or the page
 *         copy        hand the text selected there to the card
 *         cut         delete the text the last copy handed over — the SECOND half of Cut,
 *                     which the card asks for only once that text is on the clipboard. It
 *                     is refused unless the selection is still exactly what was copied, in
 *                     full, in a field that accepts edits. Either way the card is told:
 *                     "page-cut", or "page-cut-refused" with the reason.
 *
 * HOW THE TEXT REACHES THE CARD. The system clipboard belongs to LunaSysMgr, and only a card
 * can write it. The text cannot ride the hit-test reply (the platform's HitTest.schema
 * forbids extra keys) nor the notification channel (any app may subscribe to that, and it is
 * cut at 200 characters). So it is written to a file in the card's own application
 * directory — the one place a card may load a local file from — as a one-call script, and
 * the card is told its NAME over the notification channel:
 *
 *     <appdir>/clip/copy-<24 random hex>.js    if(window.jihadClip){window.jihadClip("…");}
 *     notification                              category "page-copied", text = that name
 *
 * The name is random so that a page cannot find the file by guessing (a local file:// page
 * may load another local file as a script); the text inside is plain ASCII with everything
 * else escaped, so no character set or stray surrogate can break the script.
 *
 * The card loads it with a script tag and puts the text on the clipboard. The file is removed
 * fifteen seconds later, and anything left by a daemon that died first is removed when this
 * component starts; the clipboard itself is readable by every app on the device, so this
 * exposes nothing the copy was not already going to. A password field's contents are never
 * copied or cut at all, as in any desktop browser.
 *
 * STATE IS PER PAGE. One daemon serves every card of a variant, and this component is one
 * object for all of them: what was pressed and what was copied are remembered per top window,
 * so a command from one card can never act on another card's page.
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

// --- per-page state ---------------------------------------------------------------------
//* One entry per top window: {top, win, editable, copied, truncated, arm}. `top` and `win`
//* are weak references, so a closed page costs nothing and is dropped on the next look.
let gPages = [];

function pageState(top, create) {
  let found = null, live = [];
  for (let i = 0; i < gPages.length; i++) {
    let t = null;
    try { t = gPages[i].top.get(); } catch (e) { t = null; }
    if (!t) { continue; }
    live.push(gPages[i]);
    if (t === top) { found = gPages[i]; }
  }
  gPages = live;
  if (!found && create) {
    found = {top: Cu.getWeakReference(top), win: null, editable: false,
             copied: null, truncated: false, arm: null};
    gPages.push(found);
    if (gPages.length > 16) { gPages.shift(); }
  }
  return found;
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

//* Something typing could go into: a text control, or an element inside an editing host.
function isEditingTarget(el) {
  if (isTextField(el)) { return true; }
  try { return !!el && !!el.isContentEditable; } catch (e) { return false; }
}

function isPassword(el) {
  try { return !!el && String(el.type).toLowerCase() === "password"; } catch (e) { return false; }
}

function fieldSelection(el) {
  // A field keeps its own selection; types without one (checkbox, number in some builds)
  // throw on selectionStart, and those have nothing to copy.
  try {
    let a = el.selectionStart, b = el.selectionEnd;
    if (typeof a === "number" && typeof b === "number" && b > a) { return {a: a, b: b}; }
  } catch (e) {}
  return null;
}

//* Walk the text a selection covers: fn(node, from, to) for each run of characters, in
//* order, until fn returns false or `maxNodes` text nodes have been looked at. Returns false
//* if it was cut short. No string is built here (a plain-text page of many megabytes is ONE
//* node), and only the selection is walked: from where each range starts to where it ends,
//* not the whole subtree they share.
function walkSelection(w, sel, maxNodes, fn) {
  let visited = 0;
  for (let i = 0; i < sel.rangeCount; i++) {
    let r = sel.getRangeAt(i), root = r.commonAncestorContainer;
    if (root.nodeType === 3) {
      if (r.endOffset > r.startOffset && fn(root, r.startOffset, r.endOffset) === false) { return false; }
      continue;
    }
    let walker = w.document.createTreeWalker(root, 4 /* SHOW_TEXT */, null, false);
    // Begin AT the range's start: the text node itself, or the first one inside or after
    // the element the range starts in.
    let n = null;
    if (r.startContainer.nodeType === 3) {
      n = r.startContainer;
      walker.currentNode = n;
    } else {
      let at = r.startContainer.childNodes[r.startOffset] || null;
      if (at) {
        walker.currentNode = at;
        n = (at.nodeType === 3) ? at : walker.nextNode();
      } else {
        // The range starts after the last child: the next text node past this subtree.
        let last = r.startContainer;
        while (last.lastChild) { last = last.lastChild; }
        walker.currentNode = last;
        n = walker.nextNode();
      }
    }
    for (; n; n = walker.nextNode()) {
      // Past the range's end: this range is done (not cut short).
      let rel = 0;
      try { rel = r.comparePoint(n, 0); } catch (e) { rel = 1; }
      if (rel > 0) { break; }
      if (++visited > maxNodes) { return false; }
      // What a selection's text leaves out: the source of scripts and styles.
      let pn = n.parentNode ? String(n.parentNode.localName || "").toLowerCase() : "";
      let skip = pn === "script" || pn === "style" || pn === "noscript" || pn === "template";
      let a = (n === r.startContainer) ? r.startOffset : 0;
      let b = (n === r.endContainer) ? r.endOffset : n.length;
      if (!skip && b > a && fn(n, a, b) === false) { return false; }
      if (n === r.endContainer) { break; }
    }
  }
  return true;
}

//* The selected text of a page's own content, at most `max` characters, and whether that is
//* all of it. A selection is read whole, with the line breaks the layout gives it — unless
//* it is far more than could be copied anyway, where building that string would stop the
//* daemon for seconds (or exhaust it) only to throw most of it away. Its size is measured
//* first, without building anything; an oversized one is read by walking its text and
//* stopping at the bound.
function docSelection(w, max) {
  let sel = null;
  try { sel = w.getSelection(); } catch (e) { sel = null; }
  if (!sel || sel.isCollapsed) { return {text: "", whole: true}; }
  let size = 0;
  let measured = walkSelection(w, sel, 400000, function(n, a, b) { size += b - a; return size <= 2 * max; });
  if (measured) {
    let s = String(sel);
    return s.length > max ? {text: s.substr(0, max), whole: false} : {text: s, whole: true};
  }
  let out = "";
  let whole = walkSelection(w, sel, 400000, function(n, a, b) {
    out += n.data.substring(a, b > a + (max - out.length) ? a + (max - out.length) : b);
    return out.length < max;
  });
  return {text: out, whole: whole};
}

function hasDocSelection(w) {
  try {
    let sel = w.getSelection();
    return !!(sel && !sel.isCollapsed);
  } catch (e) { return false; }
}

//* True when one of the selection's boxes is under (or within a fingertip's slop of) x,y.
//* Only ever asked of a selection a press has just made — one word.
function selectionIsAt(sel, x, y) {
  const SLOP = 12;
  for (let i = 0; i < sel.rangeCount; i++) {
    let rects = sel.getRangeAt(i).getClientRects();
    for (let k = 0; k < rects.length && k < 64; k++) {
      let r = rects[k];
      if (x >= r.left - SLOP && x <= r.right + SLOP && y >= r.top - SLOP && y <= r.bottom + SLOP) {
        return true;
      }
    }
  }
  return false;
}

//* The long-press on plain content. Returns true when something is selected afterwards.
function holdSelect(st, top, x, y) {
  let w = top;
  for (let depth = 0; depth < 8; depth++) {
    let el = w.document.elementFromPoint(x, y), inner = null;
    try { inner = el && el.contentWindow; } catch (e) { inner = null; }
    if (!inner || !inner.document) { break; }
    // Into the frame's own coordinates: past its border AND its padding.
    let r = el.getBoundingClientRect(), cs = null;
    try { cs = w.getComputedStyle(el, null); } catch (e) { cs = null; }
    x -= r.left + el.clientLeft + (cs ? (parseFloat(cs.paddingLeft) || 0) : 0);
    y -= r.top + el.clientTop + (cs ? (parseFloat(cs.paddingTop) || 0) : 0);
    w = inner;
  }
  st.win = Cu.getWeakReference(w);
  st.editable = false;
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
    if (sel && !sel.isCollapsed) {
      // What the engine has just selected is a word or two; reading it is cheap.
      let got = String(sel);
      // A press on the last half-letter of a word puts the caret at the word's END, and
      // the engine then takes that word and the next one ("bravo charlie", measured). One
      // press is one word: when two came back, keep the one the caret is in or at the end of.
      if (/\S\s+\S/.test(got) && pos && pos.offsetNode && pos.offsetNode.nodeType === 3) {
        let d = pos.offsetNode.data, a = pos.offset, b = pos.offset;
        while (a > 0 && /\S/.test(d.charAt(a - 1))) { a--; }
        while (b < d.length && /\S/.test(d.charAt(b))) { b++; }
        if (b > a) {
          let r = w.document.createRange();
          r.setStart(pos.offsetNode, a);
          r.setEnd(pos.offsetNode, b);
          sel.removeAllRanges();
          sel.addRange(r);
          got = d.substring(a, b);
        }
      }
      // A press on empty space selects the nearest word, wherever that is. Nothing was
      // pressed, so nothing is selected.
      if (!/\S/.test(got) || !selectionIsAt(sel, x, y)) { sel.removeAllRanges(); }
    }
  } catch (e) { log("hold: narrowing threw: " + e); }
  return hasDocSelection(w);
}

//* The long-press in a text field: the caret goes where the finger is, as a click puts it —
//* unless the press is inside what is selected there, which is then kept.
function holdInField(st, f, x, y) {
  st.win = Cu.getWeakReference(f.win);
  st.editable = true;
  if (isTextField(f.el)) {
    try {
      let pos = f.win.document.caretPositionFromPoint(x, y);
      if (pos && pos.offsetNode === f.el) {
        let s = fieldSelection(f.el);
        if (!s || pos.offset < s.a || pos.offset > s.b) { f.el.setSelectionRange(pos.offset, pos.offset); }
      }
    } catch (e) {}
    return !isPassword(f.el) && !!fieldSelection(f.el);   // nothing in a password is copyable
  }
  return hasDocSelection(f.win);          // a contentEditable region
}

//* What a command acts on: {field: focusOf() result} for a text field or editable region,
//* or {doc: window} for a page's own text.
function target(st, top) {
  let f = focusOf(top);
  if (st && st.win) {
    let hw = null;
    try { hw = st.win.get(); } catch (e) { hw = null; }
    try {
      if (hw && !hw.closed && hw.document && hw.top === top) {
        return st.editable ? {field: f} : {doc: hw};
      }
    } catch (e) {}
  }
  // No press to go by (a command from somewhere other than the long-press menu).
  return isTextField(f.el) ? {field: f} : {doc: f.win};
}

function selectedOf(t, max) {
  if (t.doc) { return docSelection(t.doc, max); }
  if (isTextField(t.field.el)) {
    let s = isPassword(t.field.el) ? null : fieldSelection(t.field.el);
    if (!s) { return {text: "", whole: true}; }
    let n = s.b - s.a;
    return {text: String(t.field.el.value).substr(s.a, n > max ? max : n), whole: n <= max};
  }
  return docSelection(t.field.win, max);
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
    if (f.el.readOnly || f.el.disabled) { return false; }
    let s = fieldSelection(f.el);
    if (!s) { return false; }
    f.el.setRangeText("", s.a, s.b, "end");
    // What typing would have told the page.
    let ev = f.el.ownerDocument.createEvent("Event");
    ev.initEvent("input", true, false);
    f.el.dispatchEvent(ev);
    return true;
  }
  // Only an editing host can lose its selection; the body of a page cannot, and saying
  // "Cut." over it would be a lie.
  if (!isEditingTarget(f.el)) { return false; }
  if (!hasDocSelection(f.win)) { return false; }
  docShellOf(f.win).doCommand("cmd_delete");
  return !hasDocSelection(f.win);
}

function docShellOf(win) {
  return win.QueryInterface(Ci.nsIInterfaceRequestor)
            .getInterface(Ci.nsIWebNavigation)
            .QueryInterface(Ci.nsIDocShell);
}

// --- the clip directory -------------------------------------------------------------------
function randomName() {
  let hex = "";
  try {
    let bytes = Cc["@mozilla.org/security/random-generator;1"]
                  .createInstance(Ci.nsIRandomGenerator).generateRandomBytes(12);
    for (let i = 0; i < bytes.length; i++) { hex += ("0" + (bytes[i] & 0xff).toString(16)).slice(-2); }
  } catch (e) { hex = ""; }
  if (hex.length !== 24) {
    hex = "";
    while (hex.length < 24) { hex += Math.floor(Math.random() * 16).toString(16); }
  }
  return "copy-" + hex + ".js";
}

//* `text` as a JavaScript string literal in plain ASCII: JSON's own escapes, and \uXXXX for
//* every other character — the two line separators a script cannot hold bare, anything a
//* character set could mangle, and half surrogate pairs a page is free to put in its text.
function asciiLiteral(text) {
  return JSON.stringify(text).replace(/[\u0080-\uffff]/g, function(c) {
    return "\\u" + ("0000" + c.charCodeAt(0).toString(16)).slice(-4);
  });
}

function writeClip(dir, text) {
  let d = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsIFile);
  d.initWithPath(dir);
  if (!d.exists()) { d.create(Ci.nsIFile.DIRECTORY_TYPE, 0o755); }
  let name = randomName();
  let f = d.clone();
  f.append(name);
  let body = "if(window.jihadClip){window.jihadClip(" + asciiLiteral(text) + ");}\n";
  let out = Cc["@mozilla.org/network/file-output-stream;1"].createInstance(Ci.nsIFileOutputStream);
  out.init(f, 0x02 | 0x08 | 0x20, 0o644, 0);            // write | create | truncate
  let ok = false;
  try {
    // ASCII by construction, so the bytes are the characters.
    let left = body;
    while (left.length) {
      let n = out.write(left, left.length);
      if (n <= 0) { throw new Error("short write"); }
      left = left.substr(n);
    }
    ok = true;
  } finally {
    try { out.close(); } catch (e) {}
    if (!ok) { try { f.remove(false); } catch (e) {} }   // never leave half a script
  }
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

//* Empty the clip directory: whatever is in it was written by a daemon that is gone (its
//* fifteen-second timers went with it). The directory sits beside the profile, in the app's
//* own directory; done once, when this component is created at daemon start.
function sweepClipDir() {
  try {
    let d = Services.dirsvc.get("ProfD", Ci.nsIFile).parent;
    d.append("clip");
    if (!d.exists() || !d.isDirectory()) { return; }
    let it = d.directoryEntries;
    while (it.hasMoreElements()) {
      let f = it.getNext().QueryInterface(Ci.nsIFile);
      if (/^copy-[0-9a-f-]+\.js$/.test(f.leafName)) { try { f.remove(false); } catch (e) {} }
    }
  } catch (e) { log("clip sweep: " + e); }
}

// --- the page's own contextmenu handling ---------------------------------------------------
//* Before the daemon dispatches contextmenu: listen for it in the system group, which runs
//* after the page's own listeners, and note whether the page claimed it.
function arm(st, top) {
  disarm(st);
  let a = {prevented: false, targets: [], handler: null};
  a.handler = function(ev) { if (ev.defaultPrevented) { a.prevented = true; } };
  let add = function(t) {
    if (!t) { return; }
    try { Services.els.addSystemEventListener(t, "contextmenu", a.handler, true); a.targets.push(t); } catch (e) {}
  };
  add(top);
  // Events in a frame do not pass through the top window; they do pass the docshell's
  // chrome event handler, where this embedding has one.
  try { add(docShellOf(top).chromeEventHandler); } catch (e) {}
  st.arm = a;
}

function disarm(st) {
  let a = st && st.arm;
  if (!a) { return false; }
  st.arm = null;
  for (let i = 0; i < a.targets.length; i++) {
    try { Services.els.removeSystemEventListener(a.targets[i], "contextmenu", a.handler, true); } catch (e) {}
  }
  return a.prevented;
}

function JihadPageCommands() {
  Services.obs.addObserver(this, "jihad-page-command", false);
  sweepClipDir();
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
      if (req.cmd === "holdArm") {
        arm(pageState(win, true), win);
      } else if (req.cmd === "hold") {
        let st = pageState(win, true);
        if (disarm(st)) {
          // The page has a context menu of its own; a second one over it helps nobody.
          log("hold: the page handled the contextmenu event itself, no menu");
          return;
        }
        let x = Number(req.x) || 0, y = Number(req.y) || 0;
        // The editing commands act on the FOCUSED field. If the page kept the press from
        // focusing the field it landed on (a custom editor that cancels mousedown), that is
        // some other field — or nothing at all, with <body> as the active element, which
        // contains everything and is not a field: offer the page's menu rather than edit
        // the wrong thing.
        let f = req.editable ? focusOf(win) : null, editable = false;
        if (f && f.el && f.win === win && isEditingTarget(f.el)) {
          let at = null;
          try { at = win.document.elementFromPoint(x, y); } catch (e) { at = null; }
          editable = !!at && (at === f.el || f.el.contains(at));
        }
        let selected = editable ? holdInField(st, f, x, y) : holdSelect(st, win, x, y);
        let readOnly = editable && !!req.readOnly;
        notify("page-hold", JSON.stringify({editable: editable, readOnly: readOnly, selected: selected}));
        log("hold: editable=" + editable + " readOnly=" + readOnly + " selected=" + selected);
      } else if (req.cmd === "selectAll") {
        selectAll(target(pageState(win, false), win));
      } else if (req.cmd === "copy") {
        let st = pageState(win, true);
        let got = selectedOf(target(st, win), MAX_COPY);
        st.copied = null; st.truncated = false;
        if (!got.text) { notify("page-copy-empty", "Nothing is selected."); return; }
        if (!req.dir) { notify("page-copy-empty", "Copying is not available."); return; }
        let name = writeClip(req.dir, got.text);
        st.copied = got.text; st.truncated = !got.whole;
        notify("page-copied", name);
        log("copy: " + got.text.length + " characters handed to the card" + (got.whole ? "" : " (the selection is longer)"));
      } else if (req.cmd === "cut") {
        // Delete what the card has just put on the clipboard — and only that.
        let st = pageState(win, false);
        let t = target(st, win);
        let was = st ? st.copied : null, cut = st ? st.truncated : false;
        if (st) { st.copied = null; st.truncated = false; }
        if (!t.field || was === null) {                 // a page's own text: Cut is a copy
          notify("page-cut-refused", "Copied. This text cannot be removed from the page.");
          return;
        }
        if (cut) { notify("page-cut-refused", "Too much text to cut at once. It was copied in part and left in place."); return; }
        if (selectedOf(t, MAX_COPY + 1).text !== was) {
          notify("page-cut-refused", "Copied. The selection changed, so nothing was removed.");
          return;
        }
        if (deleteSelection(t.field)) { notify("page-cut", "Cut."); }
        else { notify("page-cut-refused", "Copied. This field does not accept changes."); }
      }
    } catch (e) {
      log(String(req.cmd) + " threw: " + e);
      if (req.cmd === "copy") { notify("page-copy-empty", "The text could not be copied."); }
      if (req.cmd === "cut") { notify("page-cut-refused", "Copied. The text could not be removed."); }
    }
  }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadPageCommands]);
