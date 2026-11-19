/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — logins saved and filled (cavekit-site-data.md R1, T-661).
 *
 * WHY THIS AND NOT THE TOOLKIT'S OWN: the password manager's content half (LoginManagerContent.jsm) reaches its parent
 * through the docshell's content frame message manager, which a page in an nsIWebBrowser embedding does not have, and
 * its parent half (LoginManagerParent.jsm) is started by a browser's glue that this embedding has no part of. So in this
 * browser no login was ever saved or filled, whatever Settings' "Remember logins for sites" said. This component does
 * the two jobs in-process, through the same storage (nsILoginManager: logins.json in the variant's profile, encrypted
 * by NSS), the same two prefs and the same never-save list:
 *   SAVE: the engine's "earlyformsubmit" notification (nsIFormSubmitObserver) gives the form before it leaves. The
 *     username and password are read there and the card is asked, then and there, whether to remember them (or to
 *     update a saved password), as a page's own confirm() in its submit handler would be; the submission goes on when
 *     the question is answered, whatever the answer. Asked a moment later, the question met the next page's load:
 *     Mochi's card never took it up, Mojo's answered Cancel at once, and a dialog shown late held every tap that came
 *     after it (finger round 2026-11-18).
 *   FILL: at each page's DOMContentLoaded (and again at its load, for forms a script adds late), every form with one
 *     password field whose fields are empty gets the saved login of the page's own origin (a frame: the frame's), when
 *     there is exactly one, or the one whose username the page already shows. The page's input and change events fire,
 *     so a script that watches the fields sees the values as if typed.
 * The card's dialog has OK and Cancel only (DialogService.cpp): OK remembers, Cancel does not. A site goes onto the
 * never-save list from about:logins (cavekit-site-data.md R2), and a site on it is never asked about.
 * A master password set and not entered: nothing is filled and nothing is asked (no prompt the card cannot answer);
 * about:logins asks for it in its own page.
 */
"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/Timer.jsm");

function log(s) { try { Services.console.logStringMessage("logins: " + s); } catch (e) {} }
function boolPref(n, d) { try { return Services.prefs.getBoolPref(n); } catch (e) { return d; } }

const USER_TYPES = ["text", "email", "tel", "url", "number", ""];
const PREF_TEST_MASTER = "jihad.test.masterPassword";

function originOf(spec) {
  try { return Services.io.newURI(spec, null, null).prePath; } catch (e) { return ""; }
}

// The NSS key store is locked behind a master password that was not entered: no login can be read without a prompt
function locked() {
  try {
    let token = Cc["@mozilla.org/security/pk11tokendb;1"].getService(Ci.nsIPK11TokenDB).getInternalKeyToken();
    return token.needsLogin() && !token.isLoggedIn();
  } catch (e) { return false; }
}

// A form's login fields: the one password field and the user field before it (the last text-like field), or null.
// A form with two or three password fields is a change of password: the new one is the last, the user field the same.
// A field that was a password field at any time it was seen (focused, typed in, or when the page was read): a page's
// "show password" turns its type to text, and its value must stay out of the form history all the same (Codex review
// 2026-11-19). Weak, so a document's fields go with it.
const gWasPassword = new WeakSet();
function notePassword(e) { try { if (e && e.localName === "input" && e.type === "password") gWasPassword.add(e); } catch (x) {} }
// what a field is for, by the page's own words: a credential, a one-time code or a card's security number
const SECRET_AUTOCOMPLETE = /\b(current-password|new-password|one-time-code|cc-csc|cc-number)\b/i;
const SECRET_NAME = /pass(word|wd|code)?|pwd|\bpin\b|otp|cvv|cvc|csc|secret|token/i;
function isSecretField(e) {
  if (gWasPassword.has(e)) return true;
  if (SECRET_AUTOCOMPLETE.test(String(e.getAttribute("autocomplete") || ""))) return true;
  return SECRET_NAME.test(String(e.name || "") + " " + String(e.id || ""));
}

function fieldsOf(form) {
  let els = [];
  try { els = Array.prototype.slice.call(form.elements); } catch (e) { return null; }
  let pws = els.filter(e => e.localName === "input" && e.type === "password" && !e.disabled);
  if (!pws.length || pws.length > 3) return null;
  let first = els.indexOf(pws[0]), user = null;
  for (let i = first - 1; i >= 0; i--) {
    let e = els[i];
    if (e.localName === "input" && USER_TYPES.indexOf(e.type) >= 0 && !e.disabled && e.type !== "hidden") { user = e; break; }
  }
  return { user: user, pw: pws[pws.length - 1], all: pws };
}

function JihadLogins() {
  this.wrappedJSObject = this;
  try { Services.obs.addObserver(this, "earlyformsubmit", false); } catch (e) { log("no earlyformsubmit: " + e); }
  try { Services.obs.addObserver(this, "document-element-inserted", false); } catch (e) {}
  try { Services.prefs.addObserver(PREF_TEST_MASTER, this, true); } catch (e) {}
  // said a moment later: the component is made before the console's bridge to the daemon's log is on
  setTimeout(() => log("ready: remember " + boolPref("signon.rememberSignons", true) + ", fill " + boolPref("signon.autofillForms", true)), 5000);
}

JihadLogins.prototype = {
  classID: Components.ID("{8e3b1f6a-2c47-4d9e-a51b-7c0d93e4f2a8}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIObserver, Ci.nsIFormSubmitObserver, Ci.nsISupportsWeakReference]),

  observe: function (subject, topic, data) {
    if (topic === "document-element-inserted") this._onDocument(subject);
    else if (topic === "nsPref:changed" && data === PREF_TEST_MASTER) this._testMaster();
  },

  // FOR THE FINGER SUITE ONLY (site-logins-master.test), and only in trace mode (jihad.compat.trace, which a round
  // turns on and nothing else does): a master password set and the key store locked, or taken off again. This browser
  // has no page that sets one (cavekit-site-data.md: out of scope), and about:logins' locked state cannot be reached
  // by a script otherwise. "set:<pw>" sets it over an empty one and locks the store; "clear:<pw>" puts the empty one back.
  _testMaster: function () {
    let v = "";
    try { v = Services.prefs.getCharPref(PREF_TEST_MASTER); } catch (e) {}
    if (!v) return;
    // read once and cleared at once: the password is never kept in prefs.js, nor written to the log (Codex review
    // 2026-11-19)
    try { Services.prefs.clearUserPref(PREF_TEST_MASTER); } catch (e) {}
    if (!boolPref("jihad.compat.trace", false)) return;
    try {
      let t = Cc["@mozilla.org/security/pk11tokendb;1"].getService(Ci.nsIPK11TokenDB).getInternalKeyToken();
      let m = /^(set|clear):(.+)$/.exec(v);
      if (!m) { log("test: master password: the command is neither set:<pw> nor clear:<pw>"); return; }
      if (m[1] === "set") {
        if (t.needsUserInit) t.initPassword(m[2]); else t.changePassword("", m[2]);
        t.logoutSimple();
        log("test: master password set, the store locked: " + locked());
      } else {
        t.changePassword(m[2], "");
        log("test: master password taken off: " + !t.needsLogin());
      }
    } catch (e) { log("test: master password: " + e); }
  },

  // nsIFormSubmitObserver: the form about to be submitted. Read now, ask later; never cancel the submission.
  notify: function (form, win, actionURI, cancelSubmit) {
    if (cancelSubmit && typeof cancelSubmit === "object") cancelSubmit.value = false;
    log("a form submitted on " + (form && form.ownerDocument ? String(form.ownerDocument.documentURI).replace(/[?#].*$/, "") : "?"));
    try { this._keepFormData(form); } catch (e) { log("form data: " + e); }
    try { this._onSubmit(form, win, actionURI); } catch (e) { log("submit: " + e); }
    return false;
  },
  notifyInvalidSubmit: function () {},

  // FORM DATA (cavekit-site-data.md R3, about:data's Form data): what was typed into a submitted form's text fields goes
  // into the engine's form history (satchel), as the browser's own form-submit listener does where there is one, and
  // only when browser.formfill.enable is on: not a password, not a field marked autocomplete=off, not a long text, not
  // a number shaped like a card's.
  _keepFormData: function (form) {
    if (!boolPref("browser.formfill.enable", true)) return;
    let doc = form.ownerDocument;
    if (!doc || doc.nodePrincipal.isSystemPrincipal || !/^https?:/.test(doc.documentURI || "")) return;
    if (String(form.getAttribute("autocomplete") || "").toLowerCase() === "off") return;
    let changes = [];
    Array.prototype.forEach.call(form.elements, function (e) {
      if (e.localName !== "input" || ["text", "search", "email", "tel", "url", ""].indexOf(e.type) < 0) return;
      if (String(e.getAttribute("autocomplete") || "").toLowerCase() === "off") return;
      if (isSecretField(e)) return;
      let name = e.name || e.id, v = String(e.value || "").trim();
      if (!name || !v || v.length > 200 || /^[\d\s-]{13,19}$/.test(v)) return;
      changes.push({ op: "bump", fieldname: name, value: v });
    });
    if (!changes.length) return;
    // One write at a time: two bumps of the same new value, each searching before either wrote, added it twice, and
    // FormHistory then refuses every later bump of that pair ("multiple entries with the same fieldname/value pair").
    // A pair found twice is put back as one entry.
    let FormHistory = Cu.import("resource://gre/modules/FormHistory.jsm", {}).FormHistory;
    let write = (list, then) => {
      let twice = false;
      FormHistory.update(list, {
        handleError: function (x) { if (/multiple entries/.test(x.message)) twice = true; else log("form data: " + x.message); },
        handleCompletion: function () { then(twice); }
      });
    };
    this._formQueue = (this._formQueue || Promise.resolve()).then(() => new Promise(done => {
      write(changes, twice => {
        if (!twice) { done(); return; }
        let again = changes.map(c => ({ op: "remove", fieldname: c.fieldname, value: c.value }))
          .concat(changes.map(c => ({ op: "add", fieldname: c.fieldname, value: c.value })));
        write(again, still => { if (still) log("form data: a pair kept twice could not be put back as one"); done(); });
      });
    }));
  },

  _onSubmit: function (form, win, actionURI) {
    if (!boolPref("signon.rememberSignons", true)) return;
    let f = fieldsOf(form);
    if (!f || !f.pw.value) { log("no login in the form (" + (f ? "an empty password" : "no password field") + ")"); return; }
    let doc = form.ownerDocument;
    if (!doc || doc.nodePrincipal.isSystemPrincipal) return;
    let hostname = originOf(doc.documentURI);
    if (!/^https?:/.test(hostname)) return;
    let action = actionURI ? actionURI.prePath : hostname;
    if (!Services.logins.getLoginSavingEnabled(hostname)) { log("never-save list: " + hostname); return; }
    if (locked()) { log("the master password was not entered: not asking for " + hostname); return; }
    let username = f.user ? f.user.value : "", password = f.pw.value;
    let usernameField = f.user ? (f.user.name || f.user.id || "") : "", passwordField = f.pw.name || f.pw.id || "";
    let saved = [];
    try { saved = Services.logins.findLogins({}, hostname, "", null); } catch (e) { return; }
    // the login of this form's action (or one saved with no action) is the one this submission can update: the same
    // username on another action origin of the page is another account and is left alone (Codex review 2026-11-19)
    let same = saved.filter(l => l.username === username && (l.formSubmitURL === action || !l.formSubmitURL));
    same.sort((a, b) => (b.formSubmitURL === action) - (a.formSubmitURL === action));
    if (same.length && same[0].password === password) {
      try {
        let bag = Cc["@mozilla.org/hash-property-bag;1"].createInstance(Ci.nsIWritablePropertyBag);
        bag.setProperty("timeLastUsed", Date.now());
        bag.setProperty("timesUsedIncrement", 1);
        Services.logins.modifyLogin(same[0], bag);
      } catch (e) {}
      return;
    }
    let host = hostname.replace(/^https?:\/\//, "");
    let who = username ? "“" + username + "”" : "the login";
    let update = same.length > 0;
    // now, before the submission leaves (the file's head says why not later)
    let ok = 1;
    try {
      // the card's own dialog (DialogService.cpp's prompter), never Services.prompt: nsPrompter.js would open a chrome
      // window this embedding cannot show and wait on it for ever
      // the submitting window's own prompt, so the card that asks is the page's (DialogService routes by window)
      let prompt = Cc["@mozilla.org/prompter;1"].getService(Ci.nsIPromptFactory).getPrompt(win && win.top || null, Ci.nsIPrompt);
      log("asking about " + hostname + (update ? " (an update)" : ""));
      ok = prompt.confirmEx( update ? "Update password" : "Remember login",
        update ? "Update the saved password for " + who + " on " + host + "?" : "Remember " + who + " for " + host + "?",
        Ci.nsIPrompt.STD_YES_NO_BUTTONS, update ? "Update" : "Remember", "Not now", null, null, {});
    } catch (e) { log("ask: " + e); return; }
    if (ok !== 0) { log("not remembered for " + hostname); return; }
    try {
      let li = Cc["@mozilla.org/login-manager/loginInfo;1"].createInstance(Ci.nsILoginInfo);
      li.init(hostname, action, null, username, password, usernameField, passwordField);
      if (update) Services.logins.modifyLogin(same[0], li);
      else Services.logins.addLogin(li);
      log((update ? "updated" : "remembered") + " for " + hostname);
    } catch (e) { log("store: " + e); }
  },

  _onDocument: function (doc) {
    let win = doc && doc.defaultView;
    if (!win || doc.nodePrincipal.isSystemPrincipal || !/^https?:/.test(doc.documentURI || "")) return;
    let fill = () => { try { this._fill(doc); } catch (e) { log("fill: " + e); } };
    // every password field seen (isSecretField): at the page's load, and at each focus or typing, in the system group
    let seen = ev => notePassword(ev.target);
    try {
      Services.els.addSystemEventListener(doc, "focus", seen, true);
      Services.els.addSystemEventListener(doc, "input", seen, true);
    } catch (e) {}
    win.addEventListener("DOMContentLoaded", () => { try { Array.prototype.forEach.call(doc.querySelectorAll("input[type=password]"), notePassword); } catch (e) {} }, { once: true });
    win.addEventListener("DOMContentLoaded", fill, { once: true });
    win.addEventListener("load", fill, { once: true });
  },

  _fill: function (doc) {
    if (!boolPref("signon.autofillForms", true) || !boolPref("signon.rememberSignons", true)) return;
    let forms = doc.forms;
    if (!forms || !forms.length) return;
    let hostname = originOf(doc.documentURI);
    if (locked()) return;
    let saved = null;
    for (let i = 0; i < forms.length; i++) {
      let f = fieldsOf(forms[i]);
      if (!f || f.all.length !== 1 || f.pw.value) continue;
      if (saved === null) {
        try { saved = Services.logins.findLogins({}, hostname, "", null); } catch (e) { saved = []; }
        if (!saved.length) return;
      }
      let action = originOf(forms[i].action || doc.documentURI);
      let fit = saved.filter(l => !l.formSubmitURL || l.formSubmitURL === action);
      if (f.user && f.user.value) fit = fit.filter(l => l.username === f.user.value);
      if (fit.length !== 1) continue;
      let l = fit[0];
      let put = (el, v) => {
        el.value = v;
        el.dispatchEvent(new doc.defaultView.Event("input", { bubbles: true }));
        el.dispatchEvent(new doc.defaultView.Event("change", { bubbles: true }));
      };
      if (f.user && !f.user.value && l.username) put(f.user, l.username);
      put(f.pw, l.password);
      log("filled for " + hostname);
    }
  }
};

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([JihadLogins]);
