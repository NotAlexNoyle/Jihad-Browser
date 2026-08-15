/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — about:preferences / about:settings.
 *
 * PANES follow Pale Moon's Preferences window (General, Content, Privacy,
 * Security, Advanced) and every row below maps to a pref Pale Moon exposes in
 * that same pane, so a Pale Moon user finds settings where they expect them.
 *
 * Pale Moon panes deliberately NOT ported, because the feature they configure
 * does not exist in this embedding:
 *   - Tabs         : this shell is one page per webOS card. There are no tabs to
 *                    configure; "open in a new card" is the card manager's job.
 *   - Applications : the handler table is driven by nsIHandlerService and a
 *                    helper-app chooser dialog, neither of which a headless
 *                    daemon can show.
 *   - Sync         : no account plumbing is built into this fork.
 * Their prefs still exist in the engine and remain reachable in about:config;
 * they are just not surfaced here.
 *
 * The two settings the SHELL owns — the start page's shortcut list and the home
 * button's target — ARE here, in the Browser pane, because this page is meant to be
 * the one settings surface (R5). They are stored in one string pref,
 * `jihad.chrome.settings`, and reach the cards through the url fragment. The default
 * SEARCH ENGINE is still not here: its list comes from a webOS service, not from an
 * engine pref, so only the card can offer it.
 */
"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;
Cu.import("resource://gre/modules/Services.jsm");

// --- the pane table ---------------------------------------------------------
// type: "bool" | "int" | "string" | "choice"
//   choice rows carry `options: [[value, label], ...]`; values are ints when the
//   pref is an int pref, strings when it is a string pref.
// Every `pref` below is a real engine pref; nothing here is invented, so a row
// that shows a value is a row that is actually in effect.
const PANES = [
	{
		id: "browser",
		title: "Browser",
		chrome: true   // rendered by renderChromePane(), not from a pref table
	},
	{
		id: "general",
		title: "General",
		groups: [
			{
				title: "Browsing",
				rows: [
					{pref: "general.smoothScroll", type: "bool", label: "Use smooth scrolling"},
					{pref: "browser.sessionhistory.max_entries", type: "int",
					 label: "Back/forward entries to keep per page"},
					{pref: "layout.frame_rate", type: "int",
					 label: "Frame-rate cap (Hz)",
					 help: "-1 leaves it uncapped. This build ships 30 for the TouchPad."},
					{pref: "accessibility.browsewithcaret", type: "bool",
					 label: "Always use the caret to navigate pages"},
					{pref: "general.warnOnAboutConfig", type: "bool",
					 label: "Warn me before opening about:config"}
				]
			},
			// NO user-agent row. `general.useragent.override` looks editable and is not:
			// EngineHost re-writes it to JIHAD_USER_AGENT on EVERY daemon start
			// (render/goanna/EngineHost.cpp), so an edit here survives to prefs.js and is
			// then silently reverted — the same decorative-control trap as the JavaScript
			// toggle. The build owns the user agent.
		]
	},
	{
		id: "content",
		title: "Content",
		groups: [
			{
				title: "Content",
				rows: [
					{pref: "permissions.default.image", type: "choice", label: "Load images",
					 options: [[1, "Always"], [2, "Never"], [3, "Only from the originating site"]]},
					// NO JavaScript toggle here, deliberately. `javascript.enabled` is written by
					// this page fine, but script still ran on a real page afterwards
					// (device-measured 2026-08-05): the daemon gates script per-PAGE on the
					// docShell as well — `GoannaRenderPage::SetJavaScriptEnabled` does
					// `ds->SetAllowJavascript()` alongside the pref — and this page cannot reach
					// a docShell. A control that does nothing is worse than an absent one
					// (cavekit-preferences-ui.md R2), so it is absent until the page can drive
					// the daemon (the R5 merge), at which point it belongs here.
					{pref: "dom.disable_open_during_load", type: "bool", label: "Block pop-up windows"},
					{pref: "plugin.default.state", type: "choice", label: "Plugins",
					 options: [[0, "Never activate"], [1, "Ask to activate"], [2, "Always activate"]]},
					{pref: "media.autoplay.enabled", type: "bool", label: "Allow media to play automatically"},
					{pref: "webgl.disabled", type: "bool", label: "Disable WebGL"},
					{pref: "dom.storage.enabled", type: "bool", label: "Allow sites to store data locally"}
				]
			},
			{
				title: "Fonts & Colors",
				rows: [
					{pref: "font.size.variable.x-western", type: "int", label: "Default font size (px)"},
					{pref: "font.minimum-size.x-western", type: "int", label: "Minimum font size (px)"},
					{pref: "browser.display.use_document_fonts", type: "choice", label: "Page fonts",
					 options: [[1, "Allow pages to choose their own fonts"], [0, "Always use my fonts"]]},
					{pref: "browser.display.document_color_use", type: "choice", label: "Page colors",
					 options: [[0, "Use the page's colors"], [1, "Always use my colors"], [2, "Use my colors only with high-contrast themes"]]},
					{pref: "browser.display.background_color", type: "string", label: "My background color"},
					{pref: "browser.display.foreground_color", type: "string", label: "My text color"}
				]
			},
			{
				title: "Languages",
				rows: [
					{pref: "intl.accept_languages", type: "string",
					 label: "Preferred languages for web pages",
					 help: "A comma-separated list, most preferred first — for example: en-us,en"}
				]
			},
			{
				title: "Zoom",
				rows: [
					{pref: "browser.zoom.full", type: "bool", label: "Zoom text and images together"}
				]
			}
		]
	},
	{
		id: "privacy",
		title: "Privacy",
		groups: [
			{
				title: "Tracking",
				rows: [
					{pref: "privacy.donottrackheader.enabled", type: "bool",
					 label: "Tell sites I do not want to be tracked",
					 help: "Sends the DNT header. Honored by the network layer directly, so it works with no browser app above the engine."},
					{pref: "geo.enabled", type: "bool", label: "Allow sites to ask for my location"},
					{pref: "network.http.sendRefererHeader", type: "choice", label: "Send the referer header",
					 options: [[0, "Never"], [1, "Only when following a link"], [2, "For links and images"]]},
					{pref: "network.http.referer.spoofSource", type: "bool",
					 label: "Send the target page as the referer instead of the real one"}
				]
			},
			{
				title: "History",
				rows: [
					{pref: "places.history.enabled", type: "bool", label: "Remember my browsing history",
					 help: "The engine's own Places history. Each card shell also keeps its own list, cleared from its menu."},
					{pref: "browser.formfill.enable", type: "bool", label: "Remember form entries"},
					{pref: "browser.formfill.expire_days", type: "int", label: "Keep form entries for (days)"}
				]
			},
			{
				title: "Cookies",
				rows: [
					{pref: "network.cookie.cookieBehavior", type: "choice", label: "Accept cookies",
					 options: [[0, "From all sites"], [1, "From the originating site only"], [2, "Never"], [3, "From visited sites only"]]},
					{pref: "network.cookie.lifetimePolicy", type: "choice", label: "Keep cookies",
					 options: [[0, "Until they expire"], [2, "Until the browser closes"], [3, "For a set number of days"]]},
					{pref: "network.cookie.lifetime.days", type: "int", label: "Cookie lifetime (days)"},
					{pref: "network.cookie.thirdparty.sessionOnly", type: "bool",
					 label: "Third-party cookies last only for the session"}
				]
			}
		]
	},
	{
		id: "security",
		title: "Security",
		groups: [
			{
				title: "Add-ons",
				rows: [
					{pref: "xpinstall.whitelist.required", type: "bool",
					 label: "Warn me when sites try to install add-ons"},
					{pref: "extensions.blocklist.enabled", type: "bool", label: "Use the add-on blocklist"},
					{pref: "extensions.update.enabled", type: "bool", label: "Check for add-on updates"},
					{pref: "extensions.strictCompatibility", type: "bool",
					 label: "Only run add-ons that declare compatibility with this version"}
				]
			},
			{
				title: "Passwords",
				rows: [
					{pref: "signon.rememberSignons", type: "bool", label: "Remember logins for sites"},
					{pref: "signon.autofillForms", type: "bool", label: "Fill in logins automatically"}
				]
			},
			{
				title: "Connection Security",
				rows: [
					{pref: "security.tls.version.min", type: "choice", label: "Minimum TLS version",
					 options: [[1, "TLS 1.0"], [2, "TLS 1.1"], [3, "TLS 1.2"], [4, "TLS 1.3"]]},
					{pref: "security.tls.version.max", type: "choice", label: "Maximum TLS version",
					 options: [[1, "TLS 1.0"], [2, "TLS 1.1"], [3, "TLS 1.2"], [4, "TLS 1.3"]]},
					{pref: "security.mixed_content.block_active_content", type: "bool",
					 label: "Block insecure scripts on secure pages"},
					{pref: "security.mixed_content.block_display_content", type: "bool",
					 label: "Block insecure images on secure pages"},
					{pref: "security.OCSP.enabled", type: "choice", label: "Check certificate revocation (OCSP)",
					 options: [[0, "Never"], [1, "For certificates that say where to ask"]]},
					{pref: "security.default_personal_cert", type: "choice",
					 label: "When a site asks for my certificate",
					 options: [["Select Automatically", "Select one automatically"], ["Ask Every Time", "Ask me every time"]]}
				]
			}
		]
	},
	{
		id: "advanced",
		title: "Advanced",
		groups: [
			{
				title: "Cache",
				rows: [
					{pref: "browser.cache.disk.enable", type: "bool", label: "Use a disk cache"},
					{pref: "browser.cache.disk.capacity", type: "int", label: "Disk cache limit (KB)"},
					{pref: "browser.cache.disk.smart_size.enabled", type: "bool",
					 label: "Let the engine choose the disk cache size"},
					{pref: "browser.cache.memory.capacity", type: "int", label: "Memory cache limit (KB)"},
					{pref: "browser.cache.offline.enable", type: "bool", label: "Allow sites to store an offline copy"}
				]
			},
			{
				title: "Network",
				rows: [
					{pref: "network.http.max-connections", type: "int", label: "Maximum connections"},
					{pref: "network.http.pipelining", type: "bool", label: "Use HTTP pipelining"},
					{pref: "network.http.speculative-parallel-limit", type: "int",
					 label: "Speculative connections", help: "0 turns speculative connecting off."},
					{pref: "network.dns.disablePrefetch", type: "bool", label: "Disable DNS prefetching"},
					{pref: "network.dns.disableIPv6", type: "bool", label: "Disable IPv6"},
					{pref: "network.prefetch-next", type: "bool", label: "Prefetch links the page suggests"},
					{pref: "network.proxy.type", type: "choice", label: "Proxy",
					 options: [[0, "No proxy"], [1, "Manual (set in about:config)"], [2, "Automatic proxy configuration URL"], [4, "Auto-detect"], [5, "Use the system proxy"]]}
				]
			},
			{
				title: "Images",
				rows: [
					{pref: "image.animation_mode", type: "choice", label: "Animated images",
					 options: [["normal", "Play as the image says"], ["once", "Play once"], ["none", "Do not animate"]]}
				]
			}
		]
	}
];

// --- chrome-owned settings ------------------------------------------------
// The home button's target and the start page's shortcut list belong to the CARD
// shells, not to the engine — but they are edited HERE, because this page is the one
// settings surface (cavekit-preferences-ui.md R5). They are stored in a single string
// pref, `jihad.chrome.settings`, which the daemon serves to the card over Luna
// (`getChromePrefs`). One writer (this page), many readers (the three shells).
//
// One pref carrying JSON rather than a pref per setting: the set is owned end to end by
// this project, it is read and written whole, and it keeps the Luna surface to one method.

const CHROME_PREF = "jihad.chrome.settings";
const CHROME_MAX_LINKS = 12;

function chromeDefaults() {
	return {
		homeUrl: "https://start.duckduckgo.com/",
		startLinks: [
			{title: "DuckDuckGo",    url: "https://duckduckgo.com/"},
			{title: "Wikipedia",     url: "https://en.m.wikipedia.org/"},
			{title: "webOS Archive", url: "https://webosarchive.org/"}
		]
	};
}

function trimStr(v) { return (typeof v === "string") ? v.replace(/^\s+|\s+$/g, "") : ""; }

//* Only http(s) and our own about: pages. These values become the home button and the start
//* page's links in every shell — one tap from the user — so javascript:, data: and file: are
//* dropped rather than sanitised. The card applies the identical rule on its side.
function safeUrl(u) {
	var v = trimStr(u);
	return (/^https?:\/\//i.test(v) || /^about:(preferences|settings|jihad|isis|blank)$/i.test(v)) ? v : "";
}

//* Keep only rows with both a label and a target; anything else came from a half-finished
//* edit or a hand-edited pref and must not reach a start page.
function cleanLinks(list) {
	var out = [], i, t, u;
	if (!list || !list.length) { return out; }
	for (i = 0; i < list.length && out.length < CHROME_MAX_LINKS; i++) {
		t = trimStr(list[i] && list[i].title);
		u = safeUrl(list[i] && list[i].url);
		if (t && u) { out.push({title: t, url: u}); }
	}
	return out;
}

//* The CARD's copy arrives in the url fragment and the edited copy goes back the same way.
//*
//* Why not the Luna service the daemon exposes: the daemon registers on the PRIVATE bus, so
//* `luna-send -a <privileged>` reaches `getChromePrefs` but an app CARD, which is on the
//* public bus, does not (measured 2026-08-05 — the card's call never arrived). Making it
//* public would mean an LS2 role file in a system path, and this project does not modify
//* system files. The fragment needs no channel at all: the card already receives every
//* committed url, and fragments are known to survive that round-trip.
function chromeFromFragment() {
	var m = /(?:^|[#&])chrome=([^&]*)/.exec(window.location.hash || "");
	if (!m) { return null; }
	try { return JSON.parse(decodeURIComponent(m[1])); } catch (e) { return null; }
}

function publishToFragment(cfg) {
	try {
		// Replaces the fragment in place: same document, no reload, and the card sees the
		// new url. Written even when unchanged so a card that missed one still catches up.
		window.location.hash = "chrome=" + encodeURIComponent(JSON.stringify(cfg));
	} catch (e) { /* never let publishing break the editor */ }
}

function loadChrome() {
	var d = chromeDefaults(), raw = null, o = chromeFromFragment();
	// The card's copy WINS on arrival: it is what the shells are actually using, and the
	// engine pref is this page's own store for when it is opened by typing the url.
	if (o) { return normalizeChrome(o, d); }
	try { raw = Services.prefs.getCharPref(CHROME_PREF); } catch (e) { raw = null; }
	if (raw) { try { o = JSON.parse(raw); } catch (e2) { o = null; } }
	if (!o) { return d; }
	return normalizeChrome(o, d);
}

function normalizeChrome(o, d) {
	var links = cleanLinks(o.startLinks);
	return {
		homeUrl: safeUrl(o.homeUrl) || d.homeUrl,
		startLinks: links.length ? links : d.startLinks
	};
}

//* Writes the cleaned value and returns what is now in effect. Saved immediately rather
//* than at shutdown, so a card reading over Luna a moment later sees it.
function saveChrome(o) {
	var d = chromeDefaults();
	var kept = {
		homeUrl: safeUrl(o.homeUrl) || d.homeUrl,
		startLinks: cleanLinks(o.startLinks)
	};
	if (!kept.startLinks.length) { kept.startLinks = d.startLinks; }
	try {
		Services.prefs.setCharPref(CHROME_PREF, JSON.stringify(kept));
		Services.prefs.savePrefFile(null);
	} catch (e) { /* a read-only profile must not break the rest of the page */ }
	publishToFragment(kept);
	return kept;
}

// --- pref plumbing ----------------------------------------------------------
// Every read is guarded: a pref a given build does not ship must leave the row
// disabled rather than throwing and taking the rest of the pane down with it.

function prefExists(name) {
	try {
		return Services.prefs.getPrefType(name) !== Ci.nsIPrefBranch.PREF_INVALID;
	} catch (e) {
		return false;
	}
}

function readPref(row) {
	try {
		switch (row.type) {
		case "bool":   return Services.prefs.getBoolPref(row.pref);
		case "int":    return Services.prefs.getIntPref(row.pref);
		case "choice": return (Services.prefs.getPrefType(row.pref) === Ci.nsIPrefBranch.PREF_INT)
			? Services.prefs.getIntPref(row.pref)
			: Services.prefs.getCharPref(row.pref);
		default:
			// Some string prefs are LOCALIZED: their stored value is a chrome:// properties
			// URL, not the text. intl.accept_languages is one (all.js ships
			// "chrome://global/locale/intl.properties"), so a plain getCharPref would show
			// that URL as if it were the user's language list. Ask for the resolved value
			// first and fall back for ordinary string prefs.
			try {
				return Services.prefs.getComplexValue(row.pref, Ci.nsIPrefLocalizedString).data;
			} catch (e2) {
				return Services.prefs.getCharPref(row.pref);
			}
		}
	} catch (e) {
		return null;
	}
}

function writePref(row, value) {
	try {
		switch (row.type) {
		case "bool":   Services.prefs.setBoolPref(row.pref, !!value); break;
		case "int":    Services.prefs.setIntPref(row.pref, parseInt(value, 10) || 0); break;
		case "choice":
			if (Services.prefs.getPrefType(row.pref) === Ci.nsIPrefBranch.PREF_INT) {
				Services.prefs.setIntPref(row.pref, parseInt(value, 10) || 0);
			} else {
				Services.prefs.setCharPref(row.pref, String(value));
			}
			break;
		default:       Services.prefs.setCharPref(row.pref, String(value)); break;
		}
		// Settings that survive a crash are the whole point of a settings page.
		Services.prefs.savePrefFile(null);
		return true;
	} catch (e) {
		return false;
	}
}

// --- rendering --------------------------------------------------------------

let gPane = PANES[0].id;

function el(tag, cls, text) {
	let n = document.createElement(tag);
	if (cls) { n.className = cls; }
	if (text !== undefined) { n.appendChild(document.createTextNode(text)); }
	return n;
}

function status(msg) {
	let s = document.getElementById("status");
	while (s.firstChild) { s.removeChild(s.firstChild); }
	s.appendChild(document.createTextNode(msg || ""));
}

// --- the message bar (the GRE's notificationbox) -----------------------------
// The one thing this page could not do before: say something that outlives a glance,
// without a modal. The footer #status line is transient and is wiped on every pane
// switch, and msgDialog* BLOCKS the daemon until the card answers, which is far too
// heavy for "I just reset seven settings" or "that write did not take".
//
// This is a XUL element inside an HTML document, built from JS because HTML markup has
// no way to spell a namespace. Read out of the sheets, NOT measured here:
// minimal-xul.css is already loaded for every HTML document and gives every XUL element
// `display: -moz-box`, so the box lays out; xul.css carries the -moz-binding (:182) and
// is pulled into a non-XUL document on demand when the first non-minimal XUL element is
// bound (third_party/uxp/dom/xul/nsXULElement.cpp, BindToTree). Pulling it in cannot
// restyle this page: its default namespace is XUL and its only html|-prefixed rules are
// two textbox-internal ones. The cost is that sheet's memory on a 512 MB device, paid
// only while this page is open.
const NOTIFY_XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";
let gNotifyBox = null;

//* Built at load, not at first message: the XBL binding is installed during FRAME
//* CONSTRUCTION, not during appendChild, so a box created and used in the same turn can
//* still have no appendNotification. getBoundingClientRect() forces the layout flush that
//* runs that construction; notification.xml is a chrome URI and
//* nsXBLService::FetchBindingDocument forces a SYNC load for those (source read, not run),
//* so the implementation should be present by the time the flush returns.
function initNotifyBox() {
	let host = document.getElementById("msgbar");
	if (!host) { return; }
	try {
		let box = document.createElementNS(NOTIFY_XUL_NS, "notificationbox");
		host.appendChild(box);
		box.getBoundingClientRect();
		gNotifyBox = box;
	} catch (e) {
		gNotifyBox = null;
	}
}

//* The honest test for "the binding attached" is the METHOD, not the element: an unbound
//* <notificationbox> is a styled box with no implementation at all. Re-tested on every
//* use rather than latched at startup, because it can attach later than init().
function notifyBox() {
	return (gNotifyBox && typeof gNotifyBox.appendNotification === "function") ? gNotifyBox : null;
}

//* Remove a bar and MEAN it. removeNotification(item, true) is the skipAnimation path and
//* calls _removeNotificationElement() straight away. The default path only sets
//* margin-top/opacity and finishes in a transitionend handler, and nothing in this
//* embedding has ever shown that a transition fires in an offscreen, daemon-driven
//* document. If it does not, _animating stays true and the bar stays in the DOM at
//* opacity 0: dismissed to the eye, present to the DOM. cavekit-gre-widgets.md R5 judges
//* this by the DOM, so the parentNode sweep below is the real test, not a nicety.
function dismissNotification(item) {
	if (!item) { return; }
	let box = notifyBox();
	try {
		if (box) { box.removeNotification(item, true); }
	} catch (e) { /* fall through; the DOM check below is what decides */ }
	if (item.parentNode) {
		try { item.parentNode.removeChild(item); } catch (e2) {}
	}
}

function dismissAllNotifications() {
	let box = notifyBox();
	if (!box) { return; }
	try { box.removeAllNotifications(true); } catch (e) {}
	// Sweep by tag afterwards rather than trusting that call, for the same
	// DOM-not-appearance reason as above. The collection is live, so walk it backwards.
	let left = box.getElementsByTagNameNS(NOTIFY_XUL_NS, "notification");
	for (let i = left.length - 1; i >= 0; i--) {
		try { left[i].parentNode.removeChild(left[i]); } catch (e2) {}
	}
}

//* Say something that outlives a glance. Mirrored into the footer line unconditionally,
//* so a build where the binding never attaches is no worse off than before the bar
//* existed: this is additive, not a replacement.
function announce(value, label, isWarning) {
	status(label);
	let box = notifyBox();
	if (!box) { return false; }
	try {
		dismissNotification(box.getNotificationWithValue(value));   // replace, never stack
		let item = box.appendNotification(label, value, null,
			isWarning ? box.PRIORITY_WARNING_HIGH : box.PRIORITY_INFO_MEDIUM,
			[{
				label: "Dismiss",
				// No accessKey: this device has no keyboard, and an underlined letter that
				// nothing can press is a lie about the control.
				// Returning TRUE suppresses the binding's own close(), which is the animated
				// path; we remove synchronously instead.
				callback: function (n) { dismissNotification(n); return true; }
			}]);
		// The binding's built-in X routes through dismiss() -> close() ->
		// removeNotification(item) with NO skipAnimation, i.e. the transitionend path above.
		// Shipping a control that may not do what it says is the decorative-control trap
		// this file already names for the user-agent row, so it is hidden and Dismiss is the
		// only affordance. Un-hide it the day somebody MEASURES that transitions fire here.
		item.setAttribute("hideclose", "true");
		// Tap anywhere on the bar to dismiss. Touch affordance and belt-and-braces at once:
		// the XUL <button> command path has never been exercised in this shell, and this is a
		// plain DOM listener. Both ends call the same idempotent remover.
		item.addEventListener("click", function () { dismissNotification(item); }, false);
		return true;
	} catch (e) {
		// Never leave a half-inserted bar behind: that is exactly the present-but-invisible
		// state the DOM test exists to catch.
		dismissAllNotifications();
		return false;
	}
}

//* Echo one row's write. SUCCESS is a transient echo and stays in the footer; FAILURE
//* gets the bar, because the control is now showing a value that is NOT in effect and a
//* footer line the user may have scrolled past is not enough to say so.
function echoWrite(row, value) {
	if (writePref(row, value)) {
		status(row.pref + " = " + value);
		return;
	}
	announce("pref-write-failed",
	         "Could not save " + row.pref + ". The control above is showing a value that is "
	         + "not in effect: the profile may be read-only, or that setting locked.",
	         true);
}

function buildRow(row) {
	let wrap = el("div", "row");
	let known = prefExists(row.pref);
	let value = known ? readPref(row) : null;

	let label = el("label", "row-label");
	let labelText = (row.invert && row.labelWhenInverted) ? row.labelWhenInverted : row.label;
	label.appendChild(document.createTextNode(labelText));
	let name = el("div", "row-pref", row.pref);
	label.appendChild(name);
	if (row.help) { label.appendChild(el("div", "row-help", row.help)); }

	let control;
	if (row.type === "bool") {
		control = document.createElement("input");
		control.setAttribute("type", "checkbox");
		control.className = "cb";
		control.checked = row.invert ? !value : !!value;
		control.addEventListener("change", function () {
			let v = row.invert ? !control.checked : control.checked;
			echoWrite(row, v);
		}, false);
	} else if (row.type === "choice") {
		control = document.createElement("select");
		control.className = "sel";
		for (let i = 0; i < row.options.length; i++) {
			let opt = document.createElement("option");
			opt.setAttribute("value", String(row.options[i][0]));
			opt.appendChild(document.createTextNode(row.options[i][1]));
			control.appendChild(opt);
		}
		if (value !== null) { control.value = String(value); }
		control.addEventListener("change", function () {
			echoWrite(row, control.value);
		}, false);
	} else {
		control = document.createElement("input");
		control.setAttribute("type", row.type === "int" ? "number" : "text");
		control.className = "txt";
		control.value = (value === null) ? "" : String(value);
		control.addEventListener("change", function () {
			echoWrite(row, control.value);
		}, false);
	}

	if (!known) {
		// Shown but inert: the pref is not in this build, and pretending otherwise
		// would mean a control that silently does nothing.
		control.disabled = true;
		wrap.className = "row unavailable";
		label.appendChild(el("div", "row-help", "Not available in this build."));
	}

	wrap.appendChild(label);
	wrap.appendChild(control);
	return wrap;
}

//* The Browser pane. Not built from the pref table: these two settings live in one JSON
//* pref and are edited as a form, so they get their own renderer.
function renderChromePane(host) {
	var cfg = loadChrome();

	var home = el("section", "group");
	home.appendChild(el("h2", "group-title", "Home Page"));
	var hrow = el("div", "row");
	var hlabel = el("label", "row-label");
	hlabel.appendChild(document.createTextNode("Home button target"));
	hlabel.appendChild(el("div", "row-help",
		"Where the home button in every card goes. Blank it to restore the default."));
	var hinput = document.createElement("input");
	hinput.setAttribute("type", "url");
	hinput.className = "txt";
	hinput.value = cfg.homeUrl;
	hinput.addEventListener("change", function () {
		cfg.homeUrl = hinput.value;
		var now = saveChrome(cfg);
		hinput.value = now.homeUrl;
		status("Home page saved.");
	}, false);
	hrow.appendChild(hlabel);
	hrow.appendChild(hinput);
	home.appendChild(hrow);
	host.appendChild(home);

	var box = el("section", "group");
	box.appendChild(el("h2", "group-title", "Start Page Links"));
	var rows = el("div", "");
	box.appendChild(rows);

	// The editor's working copy is NOT cleaned as you type, so a half-finished row (a name
	// typed, no address yet) does not vanish under the cursor; only what is saved is cleaned.
	var working = cfg.startLinks.slice();

	function commit() {
		var i, out = [];
		for (i = 0; i < working.length; i++) {
			out.push({title: working[i].title, url: working[i].url});
		}
		cfg.startLinks = out;
		saveChrome(cfg);
		status("Start page links saved.");
	}

	function draw() {
		while (rows.firstChild) { rows.removeChild(rows.firstChild); }
		for (var i = 0; i < working.length; i++) {
			(function (idx) {
				var r = el("div", "row link-row");
				var name = document.createElement("input");
				name.setAttribute("type", "text");
				name.className = "txt link-name";
				name.value = working[idx].title;
				name.setAttribute("placeholder", "Name");
				name.addEventListener("change", function () {
					working[idx].title = name.value; commit();
				}, false);
				var url = document.createElement("input");
				url.setAttribute("type", "url");
				url.className = "txt link-url";
				url.value = working[idx].url;
				url.setAttribute("placeholder", "Address");
				url.addEventListener("change", function () {
					working[idx].url = url.value; commit();
				}, false);
				var rm = el("button", "btn link-remove", "Remove");
				rm.setAttribute("type", "button");
				rm.addEventListener("click", function () {
					working.splice(idx, 1); commit(); draw();
				}, false);
				r.appendChild(name);
				r.appendChild(url);
				r.appendChild(rm);
				rows.appendChild(r);
			}(i));
		}
	}
	draw();

	var actions = el("div", "row");
	var add = el("button", "btn", "Add Link");
	add.setAttribute("type", "button");
	add.addEventListener("click", function () {
		if (working.length >= CHROME_MAX_LINKS) { status("That is the maximum."); return; }
		working.push({title: "", url: ""});
		draw();
	}, false);
	var reset = el("button", "btn", "Restore Defaults");
	reset.setAttribute("type", "button");
	reset.addEventListener("click", function () {
		working = chromeDefaults().startLinks;
		commit();
		draw();
	}, false);
	actions.appendChild(add);
	actions.appendChild(reset);
	box.appendChild(actions);
	host.appendChild(box);

	var note = el("section", "group");
	note.appendChild(el("h2", "group-title", "Where these apply"));
	var n = el("div", "row");
	n.appendChild(el("div", "row-help",
		"These two are the card shell's, not the engine's — the home button and the start " +
		"page live in the app, so they are read back over this variant's own Luna service. " +
		"Every other pane on this page changes the engine directly."));
	note.appendChild(n);
	host.appendChild(note);
}

function renderPane(id) {
	gPane = id;
	let pane = null;
	for (let i = 0; i < PANES.length; i++) {
		if (PANES[i].id === id) { pane = PANES[i]; }
	}
	let host = document.getElementById("content");
	while (host.firstChild) { host.removeChild(host.firstChild); }
	// A message raised about the pane you just left must not survive into the next one,
	// the same reason the footer line is cleared below. This is also the second dismissal
	// path, and like the button one it does not wait on a transition.
	dismissAllNotifications();
	if (!pane) { return; }

	// The Browser pane is a form over one JSON pref, not a table of prefs.
	if (pane.chrome) {
		renderChromePane(host);
		markTab(id);
		status("");
		return;
	}

	for (let g = 0; g < pane.groups.length; g++) {
		let group = pane.groups[g];
		let box = el("section", "group");
		box.appendChild(el("h2", "group-title", group.title));
		for (let r = 0; r < group.rows.length; r++) {
			box.appendChild(buildRow(group.rows[r]));
		}
		host.appendChild(box);
	}

	markTab(id);
	status("");
}

function markTab(id) {
	let tabs = document.getElementById("panes").childNodes;
	for (let i = 0; i < tabs.length; i++) {
		if (tabs[i].nodeType === 1) {
			tabs[i].className = (tabs[i].getAttribute("data-pane") === id) ? "pane-tab on" : "pane-tab";
		}
	}
}

function buildTabs() {
	let nav = document.getElementById("panes");
	for (let i = 0; i < PANES.length; i++) {
		let b = el("button", "pane-tab", PANES[i].title);
		b.setAttribute("type", "button");
		b.setAttribute("data-pane", PANES[i].id);
		b.addEventListener("click", function () { renderPane(b.getAttribute("data-pane")); }, false);
		nav.appendChild(b);
	}
}

//* Clear the user value on every pref in the current pane, so each falls back to
//* what the build ships. Only prefs shown in this pane — a settings page must not
//* reset things the user cannot see from here.
function resetPane() {
	let pane = null;
	for (let i = 0; i < PANES.length; i++) {
		if (PANES[i].id === gPane) { pane = PANES[i]; }
	}
	if (!pane) { return; }
	if (pane.chrome) {
		saveChrome(chromeDefaults());
		renderPane(gPane);
		announce("prefs-reset", "Home page and start page links restored to their defaults.", false);
		return;
	}
	let n = 0;
	for (let g = 0; g < pane.groups.length; g++) {
		for (let r = 0; r < pane.groups[g].rows.length; r++) {
			let name = pane.groups[g].rows[r].pref;
			try {
				if (Services.prefs.prefHasUserValue(name)) {
					Services.prefs.clearUserPref(name);
					n++;
				}
			} catch (e) { /* a pref this build lacks is nothing to reset */ }
		}
	}
	try { Services.prefs.savePrefFile(null); } catch (e) {}
	renderPane(gPane);
	if (n) {
		// A bulk reset is the one thing on this page the user cannot see the result of by
		// looking, so it gets the bar rather than a line that the next tap wipes.
		announce("prefs-reset",
		         "Restored " + n + " setting" + (n === 1 ? "" : "s") + " in this pane to the default.",
		         false);
	} else {
		status("Everything in this pane was already at its default.");
	}
}

function init() {
	// about:settings and about:preferences are the same document; say which one
	// was asked for so the page does not look like it redirected somewhere else.
	let asked = String(document.location.href || "");
	if (asked.indexOf("about:settings") === 0) {
		document.title = "Settings";
		document.getElementById("page-title").textContent = "Settings";
	}
	initNotifyBox();
	buildTabs();
	renderPane(PANES[0].id);
	document.getElementById("reset-pane").addEventListener("click", resetPane, false);
}

window.addEventListener("DOMContentLoaded", init, false);
