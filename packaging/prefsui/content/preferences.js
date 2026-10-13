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
 * What the SHELL owns is here too, in the Browser pane, because this page is the one
 * settings surface (R5) — since 2026-10-05 the ONLY one: the cards' own "Browser
 * Settings" panels are gone (owner: "merge browser settings into settings").
 *   - the home button's target, the start page's shortcut list and the default search
 *     engine are stored in one string pref, `jihad.chrome.settings`, and reach the cards
 *     through the url fragment;
 *   - Clear Bookmarks and Clear History act on lists the CARD keeps, so the page asks
 *     the cards to do it, over the daemon's notification channel (see tellCards);
 *     Clear Cookies and Clear Cache are the engine's and are done here.
 * The Flash switch the card panels carried is the "Adobe Flash" row in Content.
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
					// The JavaScript row. Absent from 2026-08-05 to 2026-10-01 on a wrong diagnosis
					// ("the page cannot reach a docShell"). Measured on the device 2026-10-01: this
					// page's own write DOES stop script on the next page loaded; what undid it was
					// the card, which re-sent its own value to the USER branch on every launch. The
					// daemon now takes the card's value as the default, like the three settings
					// below, and re-reads the effective pref at every load it starts.
					{pref: "javascript.enabled", type: "bool", label: "Enable JavaScript",
					 help: "Applies to pages opened after the change."},
					{pref: "dom.disable_open_during_load", type: "bool", label: "Block pop-up windows"},
					{pref: "plugin.default.state", type: "choice", label: "Plugins",
					 options: [[0, "Never activate"], [1, "Ask to activate"], [2, "Always activate"]]},
					// The same stored value as the card's Flash switch (the daemon's getFlash /
					// enableFlash / disableFlash): one value, two places to change it.
					{pref: "plugin.state.flash", type: "choice", label: "Adobe Flash",
					 options: [[2, "On"], [0, "Off"]],
					 unlisted: {"1": "Ask to Activate (does not play here)"},
					 help: "Used when the HP Flash plugin is installed on this device. Applies to pages opened after the change."},
					{pref: "media.autoplay.enabled", type: "bool", label: "Allow media to play automatically"},
					// NO WebGL row. `webgl.disabled` exists, but WebGL is never available on this device
					// (the offscreen surface has no GL: getContext("webgl") is null with the pref off —
					// device 2026-10-06), so the row could not change anything a page sees.
					{pref: "dom.storage.enabled", type: "bool", label: "Allow sites to store data locally"},
					// CSS :has() (UXP RB_20261006). Upstream ships it off in releases: on some pages it slows
					// typing and scrolling, and this device is slower than any desktop. Off here too, with
					// the choice left to the user (the owner, 2026-10-12: "supporting modern web standards").
					{pref: "layout.css.has-pseudo.enabled", type: "bool", label: "Support the CSS :has() selector",
					 help: "Newer sites use it in their layout. Off by default, as in Pale Moon: on some pages it slows typing and scrolling. Applies to pages opened after the change."}
				]
			},
			{
				// The fonts per script are a form of their own (buildFontsSection, R9); these rows
				// follow them.
				title: "Fonts & Colors",
				build: buildFontsSection,
				reset: resetFonts,
				rows: [
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
			}
			// NO zoom row. `browser.zoom.full` ("zoom text and images together") is read by a
			// browser front-end's zoom code, not by the engine; this browser's zoom is the
			// daemon's own render-time scale of the laid-out page, so the pref changed nothing
			// (2026-10-06).
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
			},
			{
				// The certificates Jihad trusts beyond its own (buildCertsSection, cavekit-proxies-certs.md
				// R4, R6): webOS's added ones, and the ones the user added from a link.
				title: "Certificates",
				build: buildCertsSection,
				reset: resetCerts,
				rows: [
					{pref: "jihad.security.webos_certs", type: "bool", label: "Also trust certificates added to webOS",
					 help: "The roots a community update or webOS CE added, and the authorities you trusted in webOS's " +
					       "Certificate Manager. Never HP's original roots and never a root Mozilla has removed. Read when " +
					       "Jihad starts; turning it off stops the trust at once."}
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
					{pref: "network.prefetch-next", type: "bool", label: "Prefetch links the page suggests"}
				]
			},
			{
				// How Jihad reaches the web: a form of its own (buildProxySection, cavekit-proxies-certs.md
				// R1-R3), over the engine's network.proxy.* prefs and jihad.proxy.*.
				title: "Proxy",
				build: buildProxySection,
				reset: resetProxy,
				rows: []
			},
			{
				title: "Images",
				rows: [
					{pref: "image.animation_mode", type: "choice", label: "Animated images",
					 options: [["normal", "Play as the image says"], ["once", "Play once"], ["none", "Do not animate"]]}
				]
			}
		]
	},
	{
		id: "about",
		title: "About",
		about: true    // rendered by renderAboutPane(): facts read at run time, no prefs
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

// The engines a card's address bar can search with. The page stores only the ID; each shell
// maps it to its own search url (app/source/BrowserApp.js defaultWebSearchEngines,
// app-mochi/source/JihadServices.js searchEngines, app-mojo/app/models/jihad-url.js) — keep
// the ids in step with those three. Not the webOS "Just Type" list: that is the SYSTEM's
// setting, and the Enyo shell used to change it for every app on the device whenever the
// browser's own choice was changed.
// The first is the default. DuckDuckGo Lite first (owner, 2026-10-06: "add lite/html duckduckgo
// as a default search engine option and make it the default") — the plain-HTML results pages,
// which this engine renders fast and whose links work without script; the full DuckDuckGo stays
// an option. Every shell keeps its own id → url map in step with this list.
const SEARCH_ENGINES = [
	["duckduckgo-lite", "DuckDuckGo Lite"],
	["duckduckgo-html", "DuckDuckGo HTML"],
	["duckduckgo",      "DuckDuckGo"],
	["google",          "Google"],
	["wikipedia",       "Wikipedia"]
];
// "duckduckgo" was the only DuckDuckGo and the default until 2026-10-06, so a stored
// "duckduckgo" is the old default, not a choice against Lite: moved to Lite once (the
// flag records that the move was made, so a later deliberate choice of the full site stays).
function migrateEngine(cfg) {
	var moved = false;
	if (cfg && cfg.homeUrl === "https://start.duckduckgo.com/" && !cfg.homeLiteMigrated) {
		cfg.homeUrl = "https://lite.duckduckgo.com/lite/"; moved = true;   // the old default home (2026-10-07)
	}
	if (cfg) { cfg.homeLiteMigrated = true; }
	if (cfg && cfg.searchEngine === "duckduckgo" && !cfg.liteMigrated) {
		cfg.searchEngine = "duckduckgo-lite"; cfg.liteMigrated = true; return true;
	}
	if (cfg && !cfg.liteMigrated) { cfg.liteMigrated = true; }
	return moved;
}

function safeEngine(id) {
	for (var i = 0; i < SEARCH_ENGINES.length; i++) {
		if (SEARCH_ENGINES[i][0] === id) { return id; }
	}
	return "";
}

function chromeDefaults() {
	return {
		searchEngine: SEARCH_ENGINES[0][0],
		homeUrl: "https://lite.duckduckgo.com/lite/",
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
		// location.replace, NOT `location.hash = …`: assigning the hash PUSHES a session-history
		// entry per edit. Measured on the device 2026-10-01 (Mojo card): after two edits Back did
		// not leave this page — it stepped to the previous fragment, the card was handed that
		// OLDER payload as a committed url and adopted it, silently undoing the second edit.
		window.location.replace("#chrome=" + encodeURIComponent(JSON.stringify(cfg)));
	} catch (e) { /* never let publishing break the editor */ }
}

function loadChrome() {
	var d = chromeDefaults(), raw = null, o = chromeFromFragment();
	// The card's copy WINS on arrival: it is what the shells are actually using, and the
	// engine pref is this page's own store for when it is opened by typing the url.
	// A card's payload is already migrated (each shell moves its own pre-Lite default once and
	// records it), so it is taken as the user's choice — re-migrating here turned a deliberate
	// "full DuckDuckGo" back into Lite every time the page was opened (Codex 2026-10-06).
	if (o) { return normalizeChrome(o, d); }
	try { raw = Services.prefs.getCharPref(CHROME_PREF); } catch (e) { raw = null; }
	if (raw) { try { o = JSON.parse(raw); } catch (e2) { o = null; } }
	if (!o) { return d; }
	migrateEngine(o);                                  // only this page's own stored copy
	return normalizeChrome(o, d);
}

function normalizeChrome(o, d) {
	var links = cleanLinks(o.startLinks);
	return {
		searchEngine: safeEngine(o.searchEngine) || d.searchEngine,
		homeUrl: safeUrl(o.homeUrl) || d.homeUrl,
		startLinks: links.length ? links : d.startLinks,
		liteMigrated: true,
		homeLiteMigrated: true
	};
}

//* Writes the cleaned value and returns what is now in effect. Saved immediately rather
//* than at shutdown, so a card reading over Luna a moment later sees it.
function saveChrome(o) {
	var d = chromeDefaults();
	var kept = {
		searchEngine: safeEngine(o.searchEngine) || d.searchEngine,
		homeUrl: safeUrl(o.homeUrl) || d.homeUrl,
		startLinks: cleanLinks(o.startLinks),
		liteMigrated: true,
		homeLiteMigrated: true
	};
	if (!kept.startLinks.length) { kept.startLinks = d.startLinks; }
	try {
		Services.prefs.setCharPref(CHROME_PREF, JSON.stringify(kept));
		Services.prefs.savePrefFile(null);
	} catch (e) { /* a read-only profile must not break the rest of the page */ }
	publishToFragment(kept);
	return kept;
}

// --- clear data --------------------------------------------------------------
// Four actions the cards' own panels used to carry. Two are the engine's and are done right
// here; two act on lists the CARD keeps (its bookmarks and its history list live in the
// card's store, which this page cannot reach), so the page asks the cards to do them.
//
// THE ASKING goes over the daemon's notification channel — the same door a toast uses
// (DialogService's "jihad-notify" observer -> the variant's Luna subscription -> every card
// of this variant). A card treats a "chrome-…" category as a request, carries it out and
// shows `text` as its toast; any other category is just shown. Only chrome code can raise
// this observer topic, so a web page cannot ask a card to clear anything.
//
// Why not the url fragment the settings above travel in: that carries STATE, and a card
// adopts whatever state a settings url shows it — including an old url reached again with
// Back. An action must happen once.
function tellCards(category, text) {
	try {
		let bag = Cc["@mozilla.org/hash-property-bag;1"].createInstance(Ci.nsIWritablePropertyBag2);
		bag.setPropertyAsAString("category", category);
		bag.setPropertyAsAString("text", text);
		Services.obs.notifyObservers(bag, "jihad-notify", null);
		return true;
	} catch (e) {
		return false;
	}
}

const CLEAR_ACTIONS = [
	{id: "bookmarks", label: "Bookmarks", ask: "Clear all your bookmarks?",
	 help: "The bookmarks saved in this browser.",
	 run: function () { return tellCards("chrome-clear-bookmarks", "Bookmarks cleared."); }},
	{id: "history", label: "History", ask: "Clear your browsing history?",
	 help: "The history list, each card's back and forward pages, and the engine's own record of visited pages.",
	 run: function () {
		// The engine's Places record first; the cards then drop their own lists.
		try {
			Cc["@mozilla.org/browser/nav-history-service;1"].getService(Ci.nsIBrowserHistory).removeAllPages();
		} catch (e) { /* no Places in this build: the cards' lists are the history */ }
		return tellCards("chrome-clear-history", "History cleared.");
	 }},
	{id: "cookies", label: "Cookies", ask: "Clear all cookies? You will be signed out of sites.",
	 help: "Every cookie sites have stored.",
	 run: function () {
		try {
			Cc["@mozilla.org/cookiemanager;1"].getService(Ci.nsICookieManager).removeAll();
		} catch (e) { return false; }
		tellCards("privacy", "Cookies cleared.");
		return true;
	 }},
	{id: "cache", label: "Cache", ask: "Clear the cache of downloaded pages and images?",
	 help: "Copies of pages and images kept to load them faster.",
	 run: function () {
		try {
			Cc["@mozilla.org/netwerk/cache-storage-service;1"].getService(Ci.nsICacheStorageService).clear();
		} catch (e) { return false; }
		tellCards("privacy", "Cache cleared.");
		return true;
	 }}
];

//* One row per action. A tap on Clear does not clear: it turns the row into the question,
//* with Clear and Cancel — on the page, not as an engine dialog, which would block the
//* daemon until a card answered it.
function buildClearRow(act) {
	var row = el("div", "row clear-row");
	var label = el("label", "row-label");
	var title = document.createTextNode(act.label);
	label.appendChild(title);
	var help = el("div", "row-help", act.help);
	label.appendChild(help);
	var go = el("button", "btn", "Clear");
	go.setAttribute("type", "button");
	go.id = "clear-" + act.id;
	var yes = el("button", "btn btn-go", "Clear");
	yes.setAttribute("type", "button");
	yes.id = "clear-" + act.id + "-yes";
	var no = el("button", "btn", "Cancel");
	no.setAttribute("type", "button");
	no.id = "clear-" + act.id + "-no";

	function asking(on) {
		go.style.display = on ? "none" : "";
		yes.style.display = on ? "" : "none";
		no.style.display = on ? "" : "none";
		help.textContent = on ? act.ask : act.help;
		row.className = on ? "row clear-row asking" : "row clear-row";
	}
	go.addEventListener("click", function () { asking(true); }, false);
	no.addEventListener("click", function () { asking(false); }, false);
	yes.addEventListener("click", function () {
		var ok = act.run();
		asking(false);
		if (ok) {
			status(act.label + " cleared.");
		} else {
			announce("clear-failed", act.label + " could not be cleared.", true);
		}
	}, false);
	asking(false);

	row.appendChild(label);
	row.appendChild(go);
	row.appendChild(yes);
	row.appendChild(no);
	return row;
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
		// A value set elsewhere (about:addons, about:config) that this row does not offer is shown as it
		// is, not as an empty list; choosing an offered one replaces it (inspector 2026-10-12).
		if (value !== null && !row.options.some(function (o) { return String(o[0]) === String(value); })) {
			let opt = document.createElement("option");
			opt.setAttribute("value", String(value));
			opt.appendChild(document.createTextNode((row.unlisted && row.unlisted[String(value)]) || "Set elsewhere: " + value));
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

// --- Fonts (R9) -----------------------------------------------------------------------------------
// The fonts pages are drawn in, per script, as Pale Moon's and Basilisk's Fonts & Colors and their
// Advanced fonts dialog offer them (the owner, 2026-10-12). Only the fonts already on the device are
// offered, as the engine enumerates them: this browser ships no font, so no font's redistribution
// terms are engaged (the device's fonts — HP's Prelude, the Microsoft core fonts, the CJK fonts webOS
// carries — are licensed with the device and are only used where they are). The lists follow UXP's
// FontBuilder (toolkit/mozapps/preferences/fontbuilder.js): "Default (<font>)" first, then the fonts
// of that kind, then every other installed font.
const FONT_GROUPS = [
	["x-western", "Latin (Western)", "The quick brown fox jumps over the lazy dog. 0123456789"],
	["x-cyrillic", "Cyrillic", "Съешь же ещё этих мягких французских булок"],
	["el", "Greek", "Ξεσκεπάζω την ψυχοφθόρα βδελυγμία"],
	["he", "Hebrew", "דג סקרן שט בים מאוכזב"],
	["ar", "Arabic", "نص حكيم له سر قاطع وذو شأن عظيم"],
	["th", "Thai", "เป็นมนุษย์สุดประเสริฐเลิศคุณค่า"],
	["ja", "Japanese", "いろはにほへと ちりぬるを 日本語"],
	["ko", "Korean", "다람쥐 헌 쳇바퀴에 타고파"],
	["zh-CN", "Chinese (Simplified)", "天地玄黄 宇宙洪荒 日月盈昃"],
	["zh-TW", "Chinese (Traditional, Taiwan)", "天地玄黃 宇宙洪荒 日月盈昃"],
	["zh-HK", "Chinese (Traditional, Hong Kong)", "天地玄黃 宇宙洪荒 日月盈昃"],
	["x-unicode", "Other writing systems", "Aa Бб Γγ Ωω"]
];
const FONT_SIZES = [9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 40, 44, 48, 56, 64, 72];
const FONT_MIN_SIZES = [0, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 20, 22, 24];
let gFontGroup = null;

function fontEnumerator() {
	try { return Cc["@mozilla.org/gfx/fontenumerator;1"].createInstance(Ci.nsIFontEnumerator); }
	catch (e) { return null; }
}

// What the engine would use for a kind of font in a script, the fonts of that kind, and the rest.
function fontChoices(fe, group, generic) {
	let kind = [], all = [], dflt = "";
	try { kind = fe.EnumerateFonts(group, generic, {}) || []; } catch (e) {}
	try { dflt = fe.getDefaultFont(group, kind.length ? generic : "") || ""; } catch (e) {}
	try { all = fe.EnumerateAllFonts({}) || []; } catch (e) {}
	// Only fonts that are installed: fontconfig's generic aliases (serif, sans-serif, monospace — what
	// Default stands for) are left out, and a font is offered when the engine lists it for Latin text
	// or for the script chosen, so one that cannot draw the script is not (the device's Noto Sans Thai,
	// in /usr/share/fonts/HeiT_nb.ttf, draws Thai only). "Other writing systems" (x-unicode) names no
	// language, so the engine lists every font there, as Pale Moon does.
	let scripts = {};
	[["x-western", ""], [group, ""]].forEach(function (q) {
		try { (fe.EnumerateFonts(q[0], q[1], {}) || []).forEach(function (n) { scripts[n] = true; }); } catch (e) {}
	});
	let installed = function (n) {
		return !!n && !/^(serif|sans-serif|monospace|cursive|fantasy)$/i.test(n) && !!scripts[n];
	};
	kind = kind.filter(installed);
	let seen = {};
	kind.forEach(function (n) { seen[n] = true; });
	return {dflt: installed(dflt) ? dflt : "", kind: kind, rest: all.filter(function (n) { return !seen[n] && installed(n); })};
}

// Font names can be in any script (fontconfig gives localized family names), and this engine's
// setCharPref keeps only the low byte of each character: names go through nsISupportsString, which
// stores UTF-8, as UXP's own font preference does (toolkit/content/widgets/preferences.xml).
function unicodePref(pref) {
	return Services.prefs.getComplexValue(pref, Ci.nsISupportsString).data;
}

function fontPrefValue(pref, type) {
	try {
		if (Services.prefs.getPrefType(pref) === Ci.nsIPrefBranch.PREF_INVALID) { return null; }
		return type === "int" ? Services.prefs.getIntPref(pref) : unicodePref(pref);
	} catch (e) { return null; }
}

// "" clears the setting (the engine's own choice comes back); anything else is written.
function fontWrite(pref, type, value) {
	try {
		if (value === "") {
			if (Services.prefs.prefHasUserValue(pref)) { Services.prefs.clearUserPref(pref); }
		} else if (type === "int") {
			Services.prefs.setIntPref(pref, parseInt(value, 10) || 0);
		} else {
			let str = Cc["@mozilla.org/supports-string;1"].createInstance(Ci.nsISupportsString);
			str.data = String(value);
			Services.prefs.setComplexValue(pref, Ci.nsISupportsString, str);
		}
		Services.prefs.savePrefFile(null);
		status(pref + " = " + (value === "" ? "(default)" : value));
		return true;
	} catch (e) {
		announce("pref-write-failed", "Could not save " + pref + ": the profile may be read-only, or that setting locked.", true);
		return false;
	}
}

function option(sel, value, text, disabled) {
	let o = document.createElement("option");
	o.setAttribute("value", String(value));
	o.appendChild(document.createTextNode(text));
	if (disabled) { o.disabled = true; }
	sel.appendChild(o);
	return o;
}

// One labelled control row, in the same shape as buildRow's.
function fontRow(label, pref, control, help) {
	let wrap = el("div", "row");
	let l = el("label", "row-label");
	l.appendChild(document.createTextNode(label));
	l.appendChild(el("div", "row-pref", pref));
	if (help) { l.appendChild(el("div", "row-help", help)); }
	wrap.appendChild(l);
	wrap.appendChild(control);
	return wrap;
}

function fontSampleStyle(node, name, generic) {
	node.style.fontFamily = name ? "\"" + String(name).replace(/["\\]/g, "") + "\", " + generic : generic;
}

// Every script's font settings back to the engine's (the pane's Reset); how many were set.
function resetFonts() {
	let n = 0;
	FONT_GROUPS.forEach(function (g) {
		["font.default.", "font.size.variable.", "font.size.fixed.", "font.minimum-size.",
		 "font.name.serif.", "font.name.sans-serif.", "font.name.monospace."].forEach(function (pre) {
			try { if (Services.prefs.prefHasUserValue(pre + g[0])) { Services.prefs.clearUserPref(pre + g[0]); n++; } } catch (e) {}
		});
	});
	return n;
}

function buildFontsSection(group) {
	let box = el("section", "group");
	box.id = "section-fonts";
	box.appendChild(el("h2", "group-title", group.title));
	let fe = fontEnumerator();
	if (!gFontGroup) {
		let lg = "";
		try { lg = Services.prefs.getComplexValue("font.language.group", Ci.nsIPrefLocalizedString).data; } catch (e) {}
		gFontGroup = FONT_GROUPS.some(function (g) { return g[0] === lg; }) ? lg : "x-western";
	}
	let which = document.createElement("select");
	which.className = "sel";
	FONT_GROUPS.forEach(function (g) { option(which, g[0], g[1]); });
	which.value = gFontGroup;
	box.appendChild(fontRow("Fonts for", "font.*.<script>", which));
	let body = el("div", "fonts-body");
	box.appendChild(body);

	function fill() {
		while (body.firstChild) { body.removeChild(body.firstChild); }
		let g = gFontGroup, sample = FONT_GROUPS.filter(function (x) { return x[0] === g; })[0][2];
		if (!fe) {
			body.appendChild(el("div", "row-help", "This build cannot list the fonts installed."));
			return;
		}
		// Proportional: serif or sans-serif.
		let prop = document.createElement("select");
		prop.className = "sel";
		option(prop, "serif", "Serif");
		option(prop, "sans-serif", "Sans-serif");
		prop.value = fontPrefValue("font.default." + g, "char") === "sans-serif" ? "sans-serif" : "serif";
		prop.addEventListener("change", function () { fontWrite("font.default." + g, "char", prop.value); }, false);
		body.appendChild(fontRow("Proportional", "font.default." + g, prop));
		body.appendChild(sizeRow("Size", "font.size.variable." + g, FONT_SIZES));
		[["serif", "Serif"], ["sans-serif", "Sans-serif"], ["monospace", "Monospace"]].forEach(function (k) {
			body.appendChild(fontNameRow(g, k[0], k[1], sample));
			if (k[0] === "monospace") { body.appendChild(sizeRow("Monospace size", "font.size.fixed." + g, FONT_SIZES)); }
		});
		body.appendChild(sizeRow("Minimum font size", "font.minimum-size." + g, FONT_MIN_SIZES));
		let reset = el("button", "btn", "Restore default fonts");
		reset.setAttribute("type", "button");
		reset.addEventListener("click", function () {
			["font.default.", "font.size.variable.", "font.size.fixed.", "font.minimum-size.",
			 "font.name.serif.", "font.name.sans-serif.", "font.name.monospace."].forEach(function (pre) {
				try { if (Services.prefs.prefHasUserValue(pre + g)) { Services.prefs.clearUserPref(pre + g); } } catch (e) {}
			});
			try { Services.prefs.savePrefFile(null); } catch (e) {}
			status("The " + which.options[which.selectedIndex].textContent + " fonts are back to their defaults.");
			fill();
		}, false);
		let actions = el("div", "row");
		actions.appendChild(reset);
		body.appendChild(actions);
	}

	function sizeRow(label, pref, sizes) {
		let sel = document.createElement("select");
		sel.className = "sel";
		let cur = fontPrefValue(pref, "int");
		// A size not in the list (the card sets a minimum of 2 px) takes its place in it.
		let list = sizes.slice();
		if (cur !== null && list.indexOf(cur) < 0) { list.push(cur); list.sort(function (a, b) { return a - b; }); }
		list.forEach(function (n) { option(sel, n, n === 0 ? "None" : n + " px"); });
		if (cur !== null) { sel.value = String(cur); }
		sel.addEventListener("change", function () { fontWrite(pref, "int", sel.value); }, false);
		return fontRow(label, pref, sel);
	}

	function fontNameRow(g, generic, label, sample) {
		let pref = "font.name." + generic + "." + g;
		let c = fontChoices(fe, g, generic);
		let sel = document.createElement("select");
		sel.className = "sel";
		option(sel, "", c.dflt ? "Default (" + c.dflt + ")" : "Default");
		c.kind.forEach(function (n) { option(sel, n, n); });
		if (c.rest.length) {
			option(sel, "-", "Other fonts", true);
			c.rest.forEach(function (n) { option(sel, n, n); });
		}
		// The setting's own value only when the user chose it (the shipped "serif"/"sans-serif" generic
		// names are the default); one no longer installed is still shown, as itself.
		let cur = "";
		try { if (Services.prefs.prefHasUserValue(pref)) { cur = unicodePref(pref); } } catch (e) {}
		if (cur && c.kind.indexOf(cur) < 0 && c.rest.indexOf(cur) < 0) { option(sel, cur, cur + " (not installed)"); }
		sel.value = cur;
		let sampleLine = el("div", "font-sample", sample);
		fontSampleStyle(sampleLine, cur || c.dflt, generic);
		sel.addEventListener("change", function () {
			if (sel.value === "-") { sel.value = cur; return; }
			if (fontWrite(pref, "char", sel.value)) { cur = sel.value; }
			fontSampleStyle(sampleLine, sel.value || c.dflt, generic);
		}, false);
		let row = fontRow(label, pref, sel);
		row.className = "row has-sample";
		row.appendChild(sampleLine);
		return row;
	}

	which.addEventListener("change", function () { gFontGroup = which.value; fill(); }, false);
	fill();
	for (let r = 0; r < group.rows.length; r++) { box.appendChild(buildRow(group.rows[r])); }
	return box;
}

// --- Proxy (cavekit-proxies-certs.md R1-R3) -------------------------------------------------------
// One choice of how Jihad reaches the web. What the section shows is read back from the engine's own
// network.proxy.* prefs (jihad.proxy.mode only says which of the choices that share a pref shape the
// user picked), so a value set in about:config shows here and nothing claims a proxy not in use.
const RELAY_HOST = "proxy.riverstonerelay.org";
const RELAY_PORT = 3128;
const PROXY_MODES = [
	["none", "No proxy"],
	["webos", "Use webOS's proxy setting"],
	["relay", "Riverstone Relay"],
	["webone", "WebOne"],
	["manual", "My own proxy"],
	["pac", "Automatic configuration URL"],
	["wpad", "Find the proxy on the network (WPAD)"]
];
const PROXY_PREFS = ["network.proxy.type", "network.proxy.http", "network.proxy.http_port", "network.proxy.ssl",
	"network.proxy.ssl_port", "network.proxy.socks", "network.proxy.socks_port", "network.proxy.socks_version",
	"network.proxy.socks_remote_dns", "network.proxy.no_proxies_on", "network.proxy.autoconfig_url",
	"network.proxy.share_proxy_settings", "jihad.proxy.mode", "jihad.proxy.webone.host", "jihad.proxy.webone.port",
	"jihad.proxy.relay.trust_ca"];
const PROXY_HOST_RE = /^(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)(?:\.[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)*$|^\d{1,3}(?:\.\d{1,3}){3}$|^[0-9A-Fa-f]*:[0-9A-Fa-f:.]+$/;

function pInt(name, dflt) { try { return Services.prefs.getIntPref(name); } catch (e) { return dflt; } }
function pChar(name) { try { return Services.prefs.getCharPref(name); } catch (e) { return ""; } }
function pBool(name, dflt) { try { return Services.prefs.getBoolPref(name); } catch (e) { return dflt; } }

function jihadService(contract) {
	try { return Cc[contract].getService().wrappedJSObject || null; } catch (e) { return null; }
}

// Which choice the engine's prefs amount to.
function proxyMode() {
	let type = pInt("network.proxy.type", 0), mode = pChar("jihad.proxy.mode");
	if (mode === "webos") { return "webos"; }
	if (type === 1) {
		let http = pChar("network.proxy.http"), httpPort = pInt("network.proxy.http_port", 0);
		if (http.toLowerCase() === RELAY_HOST && httpPort === RELAY_PORT && pChar("network.proxy.ssl").toLowerCase() === RELAY_HOST &&
		    pInt("network.proxy.ssl_port", 0) === RELAY_PORT && !pChar("network.proxy.socks")) { return "relay"; }
		// WebOne only while the engine really goes to the WebOne this section names; an about:config
		// edit since makes it the user's own proxy again (the inspector, 2026-10-13).
		if (mode === "webone" && http && http === pChar("jihad.proxy.webone.host") &&
		    httpPort === pInt("jihad.proxy.webone.port", 8080)) { return "webone"; }
		return "manual";
	}
	if (type === 2) { return "pac"; }
	if (type === 4) { return "wpad"; }
	return "none";        // 0, and 5 (the engine's "system proxy", which goes direct in this build)
}

function validHost(h) { return PROXY_HOST_RE.test(String(h || "").trim()); }
function validPort(p) { p = String(p).trim(); return /^\d{1,5}$/.test(p) && Number(p) >= 1 && Number(p) <= 65535; }

// Write a set of prefs as one change; an empty string or null clears a char pref's user value.
function proxyWrite(values, mode, said) {
	// All or nothing: a locked pref (an administrator's) stops the change before anything is written,
	// so the engine is never left with half a proxy.
	let names = Object.keys(values).concat(["jihad.proxy.mode"]);
	for (let i = 0; i < names.length; i++) {
		let locked = false;
		try { locked = Services.prefs.prefIsLocked(names[i]); } catch (e) {}
		if (locked) {
			announce("pref-write-failed", "Could not change the proxy: " + names[i] + " is locked on this device.", true);
			return false;
		}
	}
	try {
		for (let name in values) {
			let v = values[name];
			if (typeof v === "number") { Services.prefs.setIntPref(name, v); }
			else if (typeof v === "boolean") { Services.prefs.setBoolPref(name, v); }
			else { Services.prefs.setCharPref(name, String(v)); }
		}
		Services.prefs.setCharPref("jihad.proxy.mode", mode);
		Services.prefs.savePrefFile(null);
		status(said);
		return true;
	} catch (e) {
		announce("pref-write-failed", "Could not save the proxy: the profile may be read-only, or a setting locked.", true);
		return false;
	}
}

function resetProxy() {
	let n = 0;
	PROXY_PREFS.forEach(function (name) {
		try { if (Services.prefs.prefHasUserValue(name)) { Services.prefs.clearUserPref(name); n++; } } catch (e) {}
	});
	return n;
}

function textField(value, placeholder, kind) {
	let t = document.createElement("input");
	t.setAttribute("type", kind || "text");
	t.className = "txt";
	t.value = value === null || value === undefined ? "" : String(value);
	if (placeholder) { t.setAttribute("placeholder", placeholder); }
	t.setAttribute("autocapitalize", "off");
	t.setAttribute("spellcheck", "false");
	if (kind === "port") {
		// A text field, checked by validPort: a number field turns letters into "" without a word.
		t.setAttribute("type", "text");
		t.setAttribute("inputmode", "numeric");
		t.setAttribute("maxlength", "5");
	}
	return t;
}

function helpRow(text) {
	let r = el("div", "row");
	r.appendChild(el("div", "row-help", text));
	return r;
}

function buildProxySection(group) {
	let box = el("section", "group");
	box.id = "section-proxy";
	box.appendChild(el("h2", "group-title", group.title));
	let current = proxyMode();
	let which = document.createElement("select");
	which.className = "sel";
	PROXY_MODES.forEach(function (m) {
		// WPAD is offered only when it is what the engine is doing (webOS or about:config set it).
		if (m[0] === "wpad" && current !== "wpad") { return; }
		option(which, m[0], m[1]);
	});
	which.value = current;
	box.appendChild(fontRow("Connect through", "network.proxy.type", which));
	let body = el("div", "proxy-body");
	box.appendChild(body);

	function fill(mode, askWebos) {
		while (body.firstChild) { body.removeChild(body.firstChild); }
		let pending = mode !== proxyMode();
		if (mode === "none") {
			body.appendChild(helpRow(pInt("network.proxy.type", 0) === 5
				? "Jihad connects to sites directly. (This profile was set to the engine's \"system proxy\", which this " +
				  "build cannot read, so it was direct already.)"
				: "Jihad connects to sites directly."));
		} else if (mode === "webos") {
			fillWebos(askWebos);
		} else if (mode === "relay") {
			fillRelay();
		} else if (mode === "webone") {
			fillWebOne(pending);
		} else if (mode === "manual") {
			fillManual(pending);
		} else if (mode === "pac") {
			fillPac(pending);
		} else if (mode === "wpad") {
			body.appendChild(helpRow("The engine looks for a proxy auto-config file on the network (WPAD), as webOS's " +
				"setting or about:config asked."));
		}
		// The Relay can be in use without being chosen here (webOS's setting, or a proxy typed by hand):
		// its certificate switch is shown wherever it is (the inspector, 2026-10-13).
		let certs = jihadService("@riverstonerelay.net/jihad/certs;1");
		if (mode !== "relay" && !pending && certs && certs.report().relay.inUse) {
			body.appendChild(helpRow("Jihad is going through the Riverstone Relay (" + RELAY_HOST + ")."));
			body.appendChild(relayTrustRow());
		}
	}

	function fillWebos(ask) {
		let svc = jihadService("@riverstonerelay.net/jihad/proxy;1");
		// Read again whenever the section is shown, and draw it again once the answer is in.
		if (ask && svc) {
			svc.refresh();
			window.setTimeout(function () { if (which.value === "webos") { fill("webos", false); } }, 1500);
		}
		body.appendChild(helpRow("Follows the proxy set for the whole TouchPad, which apps such as webOS Archive Proxy " +
			"set. Read when Jihad starts and whenever a card opens."));
		let st = svc ? svc.state() : null;
		let line = !svc ? "This build cannot read webOS's proxy setting."
			: st.error || st.said || "Reading webOS's setting…";
		let now = el("div", "row");
		now.appendChild(el("div", "row-help proxy-now", "Now: " + line));
		body.appendChild(now);
		let again = el("button", "btn", "Read webOS's setting again");
		again.setAttribute("type", "button");
		again.addEventListener("click", function () {
			if (svc) { svc.refresh(); }
			status("Asked webOS for its proxy setting.");
			window.setTimeout(function () { if (which.value === "webos") { fill("webos", false); } }, 1500);
		}, false);
		let actions = el("div", "row");
		actions.appendChild(again);
		body.appendChild(actions);
	}

	function fillRelay() {
		body.appendChild(helpRow("The Riverstone Relay is a proxy run by Jihad's author at " + RELAY_HOST + ":" + RELAY_PORT +
			". Pages travel through it. Secure (https) pages pass through it still encrypted, checked by Jihad as they " +
			"would be without it."));
		body.appendChild(relayTrustRow());
	}

	function relayTrustRow() {
		let certs = jihadService("@riverstonerelay.net/jihad/certs;1");
		let rep = certs ? certs.report() : null;
		let cb = document.createElement("input");
		cb.setAttribute("type", "checkbox");
		cb.className = "cb";
		cb.checked = pBool("jihad.proxy.relay.trust_ca", false);
		cb.addEventListener("change", function () {
			try {
				Services.prefs.setBoolPref("jihad.proxy.relay.trust_ca", cb.checked);
				Services.prefs.savePrefFile(null);
				status(cb.checked ? "Jihad trusts the Relay's certificate while the Relay is in use."
				                  : "Jihad no longer trusts the Relay's certificate.");
			} catch (e) {
				announce("pref-write-failed", "Could not save that: the profile may be read-only.", true);
			}
		}, false);
		return fontRow("Trust the Relay's certificate", "jihad.proxy.relay.trust_ca", cb,
			"Off unless you turn it on. Only needed if the Relay re-signs secure pages with its own certificate " +
			"(squid-sslbump-for-webos), which it does for webOS's own older browsers. While it is on and the Relay is " +
			"in use, Jihad takes that certificate for every secure site, so the Relay can read and change the secure " +
			"pages it carries." + (rep && rep.relay.fingerprint ? " SHA-256 " + rep.relay.fingerprint + "." : ""));
	}

	function hostPortRows(hostLabel, hostPref, host, port, portDefault, portPref) {
		let h = textField(host, "host name or address");
		// A port left empty takes the usual one, shown as the placeholder.
		let p = textField(port || "", String(portDefault || ""), "port");
		body.appendChild(fontRow(hostLabel, hostPref, h));
		body.appendChild(fontRow("Port", portPref || hostPref + "_port", p));
		return [h, p];
	}

	function useButton(text, go) {
		let b = el("button", "btn", text);
		b.setAttribute("type", "button");
		b.addEventListener("click", go, false);
		let r = el("div", "row");
		r.appendChild(b);
		body.appendChild(r);
	}

	function pendingNote(pending) {
		if (pending) { body.appendChild(helpRow("Not in use until you tap the button below.")); }
	}

	function fillWebOne(pending) {
		body.appendChild(helpRow("WebOne (github.com/atauenis/webone) is a proxy you run, made for old browsers: it " +
			"converts pages and re-signs secure (https) pages with its own certificate. For secure pages through it, " +
			"open its certificate once (below) and say Yes."));
		let f = hostPortRows("WebOne at", "jihad.proxy.webone.host", pChar("jihad.proxy.webone.host"),
			pInt("jihad.proxy.webone.port", 8080), 8080, "jihad.proxy.webone.port");
		pendingNote(pending);
		useButton("Use this WebOne", function () {
			let host = f[0].value.trim(), port = f[1].value.trim() || "8080";
			if (!validHost(host)) { announce("proxy-bad", "WebOne's address must be a host name or an IP address.", true); return; }
			if (!validPort(port)) { announce("proxy-bad", "The port must be a number from 1 to 65535.", true); return; }
			port = Number(port);
			if (proxyWrite({"jihad.proxy.webone.host": host, "jihad.proxy.webone.port": port,
			                "network.proxy.http": host, "network.proxy.http_port": port,
			                "network.proxy.ssl": host, "network.proxy.ssl_port": port,
			                "network.proxy.socks": "", "network.proxy.share_proxy_settings": true,
			                "network.proxy.type": 1}, "webone", "Jihad goes through WebOne at " + host + ":" + port + ".")) {
				fill("webone");
			}
		});
		let host = pChar("jihad.proxy.webone.host");
		if (host && validHost(host)) {
			let port = pInt("jihad.proxy.webone.port", 8080);
			let open = el("button", "btn", "Open this WebOne's certificate");
			open.setAttribute("type", "button");
			open.addEventListener("click", function () {
				window.location.href = "http://" + (host.indexOf(":") >= 0 ? "[" + host + "]" : host) + ":" + port + "/weboneca.crt";
			}, false);
			let r = el("div", "row");
			r.appendChild(open);
			body.appendChild(r);
		}
	}

	function fillManual(pending) {
		body.appendChild(helpRow("A proxy you run or were given: an HTTP proxy (which can also carry secure pages), " +
			"and a SOCKS proxy if you have one."));
		let http = hostPortRows("HTTP proxy", "network.proxy.http", pChar("network.proxy.http"), pInt("network.proxy.http_port", 0), 3128);
		let share = document.createElement("input");
		share.setAttribute("type", "checkbox");
		share.className = "cb";
		let sslHost = pChar("network.proxy.ssl"), sslPort = pInt("network.proxy.ssl_port", 0);
		share.checked = !sslHost || (sslHost === pChar("network.proxy.http") && sslPort === pInt("network.proxy.http_port", 0));
		body.appendChild(fontRow("Use it for secure (https) pages too", "network.proxy.ssl", share));
		let sslBox = el("div", "proxy-ssl");
		body.appendChild(sslBox);
		let ssl = [textField(sslHost, "host name or address"), textField(sslPort || "", "3128", "port")];
		sslBox.appendChild(fontRow("HTTPS proxy", "network.proxy.ssl", ssl[0]));
		sslBox.appendChild(fontRow("Port", "network.proxy.ssl_port", ssl[1]));
		sslBox.style.display = share.checked ? "none" : "";
		share.addEventListener("change", function () { sslBox.style.display = share.checked ? "none" : ""; }, false);
		let socks = hostPortRows("SOCKS proxy", "network.proxy.socks", pChar("network.proxy.socks"), pInt("network.proxy.socks_port", 0), 1080);
		let ver = document.createElement("select");
		ver.className = "sel";
		option(ver, 5, "SOCKS 5");
		option(ver, 4, "SOCKS 4");
		ver.value = String(pInt("network.proxy.socks_version", 5) === 4 ? 4 : 5);
		body.appendChild(fontRow("SOCKS version", "network.proxy.socks_version", ver));
		let dns = document.createElement("input");
		dns.setAttribute("type", "checkbox");
		dns.className = "cb";
		dns.checked = pBool("network.proxy.socks_remote_dns", false);
		body.appendChild(fontRow("Look up site names through the SOCKS 5 proxy", "network.proxy.socks_remote_dns", dns));
		let direct = textField(pChar("network.proxy.no_proxies_on") || "localhost, 127.0.0.1", "localhost, 127.0.0.1");
		body.appendChild(fontRow("Go direct for", "network.proxy.no_proxies_on", direct,
			"Host names, domains (.example.com) or networks (192.168.1.0/24), separated by commas."));
		pendingNote(pending);
		useButton("Use this proxy", function () {
			let hh = http[0].value.trim(), hp = http[1].value.trim() || "3128";
			let sh = share.checked ? hh : ssl[0].value.trim(), sp = share.checked ? hp : (ssl[1].value.trim() || "3128");
			let xh = socks[0].value.trim(), xp = socks[1].value.trim() || "1080";
			let bad = function (what) { announce("proxy-bad", what, true); };
			if (!hh && !sh && !xh) { bad("Give at least one proxy: an HTTP, HTTPS or SOCKS host and its port."); return; }
			let pairs = [[hh, hp, "HTTP"], [sh, sp, "HTTPS"], [xh, xp, "SOCKS"]];
			for (let i = 0; i < pairs.length; i++) {
				if (!pairs[i][0]) { continue; }
				if (!validHost(pairs[i][0])) { bad("The " + pairs[i][2] + " proxy must be a host name or an IP address."); return; }
				if (!validPort(pairs[i][1])) { bad("The " + pairs[i][2] + " proxy's port must be a number from 1 to 65535."); return; }
			}
			if (proxyWrite({"network.proxy.http": hh, "network.proxy.http_port": hh ? Number(hp) : 0,
			                "network.proxy.ssl": sh, "network.proxy.ssl_port": sh ? Number(sp) : 0,
			                "network.proxy.socks": xh, "network.proxy.socks_port": xh ? Number(xp) : 0,
			                "network.proxy.socks_version": Number(ver.value),
			                "network.proxy.socks_remote_dns": dns.checked,
			                "network.proxy.share_proxy_settings": share.checked,
			                "network.proxy.no_proxies_on": direct.value.trim(),
			                "network.proxy.type": 1}, "manual",
			               "Jihad goes through " + (hh ? hh + ":" + hp : sh ? sh + ":" + sp : xh + ":" + xp) + ".")) {
				fill("manual");
			}
		});
	}

	function fillPac(pending) {
		body.appendChild(helpRow("A proxy auto-config (PAC) file decides the proxy for each address. WebOne serves one " +
			"at http://<its address>:8080/auto.pac."));
		let url = textField(pChar("network.proxy.autoconfig_url"), "http://…/proxy.pac", "url");
		body.appendChild(fontRow("Configuration URL", "network.proxy.autoconfig_url", url));
		pendingNote(pending);
		useButton("Use this configuration", function () {
			let u = url.value.trim();
			if (!/^(https?|file):\/\/\S+$/i.test(u)) { announce("proxy-bad", "The configuration URL must start with http://, https:// or file://.", true); return; }
			if (proxyWrite({"network.proxy.autoconfig_url": u, "network.proxy.type": 2}, "pac",
			               "Jihad asks " + u + " which proxy to use.")) {
				fill("pac");
			}
		});
	}

	which.addEventListener("change", function () {
		let m = which.value;
		// The choices with nothing to type take effect at once; the others when their button is tapped.
		if (m === "none") {
			proxyWrite({"network.proxy.type": 0}, "none", "Jihad connects to sites directly.");
		} else if (m === "webos") {
			// components/jihadProxy.js reads webOS's setting again when the mode changes.
			proxyWrite({}, "webos", "Jihad follows webOS's proxy setting.");
			window.setTimeout(function () { if (which.value === "webos") { fill("webos"); } }, 1500);
		} else if (m === "relay") {
			proxyWrite({"network.proxy.http": RELAY_HOST, "network.proxy.http_port": RELAY_PORT,
			            "network.proxy.ssl": RELAY_HOST, "network.proxy.ssl_port": RELAY_PORT,
			            "network.proxy.socks": "", "network.proxy.share_proxy_settings": true,
			            "network.proxy.type": 1}, "relay", "Jihad goes through the Riverstone Relay.");
		}
		fill(m);
	}, false);
	fill(current, true);
	return box;
}

// --- Certificates (cavekit-proxies-certs.md R4, R6) ----------------------------------------------
function resetCerts() {
	// The switch is the group's one row (resetPane clears it); the CAs the user added are theirs,
	// and are removed one by one, never by a pane reset.
	return 0;
}

function buildCertsSection(group) {
	let box = el("section", "group");
	box.id = "section-certs";
	box.appendChild(el("h2", "group-title", group.title));
	for (let r = 0; r < group.rows.length; r++) { box.appendChild(buildRow(group.rows[r])); }
	let svc = jihadService("@riverstonerelay.net/jihad/certs;1");
	if (!svc) {
		box.appendChild(helpRow("This build cannot add certificates beyond the engine's own."));
		return box;
	}
	let rep = svc.report();
	let lines = [];
	if (rep.error) { lines.push(rep.error); }
	else if (!rep.ready) { lines.push("webOS's certificates are read when the first page opens."); }
	else if (!rep.enabled) { lines.push("Off: Jihad trusts only its own roots and the ones you added below."); }
	else {
		let found = rep.stores.filter(function (s) { return s.present; }).length;
		lines.push(found ? "Trusted from webOS: " + rep.trusted.length + "." : "No webOS certificate store was found.");
		let k = rep.skipped || {}, parts = [];
		if (k.builtin) { parts.push(k.builtin + " Jihad has already"); }
		if (k.stock) { parts.push(k.stock + " of HP's original roots"); }
		if (k.removed) { parts.push(k.removed + " Mozilla removed"); }
		if (k.expired) { parts.push(k.expired + " expired"); }
		if (k.notCA) { parts.push(k.notCA + " site certificates (Jihad asks about those per site)"); }
		if (k.already) { parts.push(k.already + " you added here as well"); }
		if (k.relay) { parts.push("the Riverstone Relay's (see Advanced → Proxy)"); }
		if (k.unreadable) { parts.push(k.unreadable + " unreadable files"); }
		if (parts.length) { lines.push("Not taken: " + parts.join(", ") + "."); }
	}
	lines.forEach(function (t) { box.appendChild(helpRow(t)); });
	rep.trusted.forEach(function (t) {
		let r = el("div", "row cert-row");
		let l = el("div", "row-label", t.name);
		l.appendChild(el("div", "row-pref", t.from + " · SHA-256 " + t.fingerprint));
		r.appendChild(l);
		box.appendChild(r);
	});

	box.appendChild(el("h3", "group-subtitle", "Certificates you added"));
	let added = svc.addedCAs();
	if (!added.length) {
		box.appendChild(helpRow("None. Opening a certificate authority's certificate (a .crt link, such as a proxy's) " +
			"asks whether to trust it to identify websites."));
	}
	added.forEach(function (c) {
		let r = el("div", "row cert-row");
		let l = el("div", "row-label", c.name);
		l.appendChild(el("div", "row-pref", (c.org && c.org !== c.name ? c.org + " · " : "") + "SHA-256 " + c.fingerprint));
		r.appendChild(l);
		let rm = el("button", "btn", "Remove");
		rm.setAttribute("type", "button");
		rm.addEventListener("click", function () {
			if (svc.removeAdded(c.sha256)) {
				status("\"" + c.name + "\" is no longer trusted.");
				box.parentNode.replaceChild(buildCertsSection(group), box);
			} else {
				announce("cert-remove-failed", "Could not remove \"" + c.name + "\".", true);
			}
		}, false);
		r.appendChild(rm);
		box.appendChild(r);
	});
	return box;
}

//* The Browser pane. Not built from the pref table: these two settings live in one JSON
//* pref and are edited as a form, so they get their own renderer.
function themeManager() {
	try { return Cu.import("resource://gre/modules/LightweightThemeManager.jsm", {}).LightweightThemeManager; }
	catch (e) { return null; }
}

// about:addons opens on the list it last showed: that list is set first, then the page opened.
function openAddons(category) {
	try { Services.prefs.setCharPref("extensions.ui.lastCategory", category); } catch (e) {}
	window.location.href = "about:addons";
}

function goButton(text, id, go) {
	var b = el("button", "btn btn-nav", text);
	b.setAttribute("type", "button");
	b.id = id;
	b.addEventListener("click", go, false);
	return b;
}

// A plugin's state, as the add-on manager and the Content pane's Flash row set it. "Ask to Activate"
// (click to play, state 1) is offered by about:addons, but this browser has nothing that activates a
// plugin afterwards: set to it, Flash does not play at all, and it is said so (inspector 2026-10-12).
function pluginState(tag) {
	if (tag.blocklisted) { return "blocked"; }
	if (tag.disabled) { return "off"; }
	return tag.clicktoplay ? "set to Ask to Activate" : "on";
}

// The plugins the engine found (the device's own Flash among them), from its plugin host.
function pluginTags() {
	try {
		return Cc["@mozilla.org/plugin/host;1"].getService(Ci.nsIPluginHost).getPluginTags() || [];
	} catch (e) {
		return null;
	}
}

function isFlash(tag) {
	try { if (tag.niceName === "flash") { return true; } } catch (e) {}
	try { return tag.getMimeTypes().indexOf("application/x-shockwave-flash") >= 0; } catch (e) { return false; }
}

// "Adobe Flash Player 10.3 d185, on. From the HP Flash plugin installed on this device." The HP
// plugin leaves the version field empty and puts it in its description ("Shockwave Flash 10.3 d185").
function flashSummary(tags) {
	var flash = tags.filter(isFlash)[0];
	if (!flash) {
		return "Adobe Flash is not installed on this device. HP's Flash plugin for webOS puts it where this browser finds it." +
			(tags.length ? " " + tags.length + (tags.length === 1 ? " other plugin." : " other plugins.") : "");
	}
	var version = String(flash.version || "");
	if (!version) {
		var m = /(\d+(?:\.\d+)+(?:\s+[a-z]\d+)?)/i.exec(String(flash.description || ""));
		version = m ? m[1] : "";
	}
	var path = String(flash.fullpath || "");
	var from = path.indexOf("/media/cryptofs/apps/usr/lib/BrowserServerPlugins/") === 0
		? "the HP Flash plugin installed on this device"
		: path ? "this browser's own plugins folder" : "";
	var others = tags.length - 1;
	// Named as people know it: the HP plugin describes itself as "Shockwave Flash".
	return "Adobe Flash Player" + (version ? " " + version.slice(0, 30) : "") + ", " + pluginState(flash) +
		(flash.clicktoplay && !flash.disabled && !flash.blocklisted
			? ", which this browser cannot do, so it does not play: choose Always Activate in Manage Plugins." : ".") +
		(from ? " From " + from + "." : "") +
		(others > 0 ? " " + others + (others === 1 ? " other plugin." : " other plugins.") : "");
}

// Personas, Extensions and Plugins (R8): what the browser wears and runs, at the top of the pane the
// settings page opens on, one button each (the owner, 2026-10-12): about:personas, where Personas are
// found and worn; about:addons on its Extensions list, where they are turned on and off and removed;
// about:addons on its Plugins list, where Flash shows its version and is turned on or off. Everything
// shown is text.
function buildAddonsSection() {
	var box = el("section", "group");
	box.id = "addons-group";
	box.appendChild(el("h2", "group-title", "Personas, Extensions and Plugins"));

	var LWTM = themeManager(), current = null, used = [];
	if (LWTM) {
		try { current = LWTM.currentTheme; } catch (e) { current = null; }
		try { used = LWTM.usedThemes || []; } catch (e) { used = []; }
	}
	var prow = el("div", "row go-row");
	var plabel = el("label", "row-label");
	plabel.appendChild(document.createTextNode("Personas"));
	plabel.appendChild(el("div", "row-help", !LWTM ? "Personas are not available in this build." :
		(current ? "Wearing " + String(current.name || current.id).slice(0, 60) +
		           (current.author ? " by " + String(current.author).slice(0, 40) : "") + ". "
		         : (used.length ? "None worn. " : "")) +
		(used.length ? used.length + " installed."
		             : "None installed yet: a Persona gives the toolbar and buttons a picture and colours.")));
	prow.appendChild(plabel);
	prow.appendChild(goButton("Browse Personas", "personas-open", function () { window.location.href = "about:personas"; }));
	box.appendChild(prow);

	// Extensions are described, not named (the owner, 2026-10-12: the one that comes with the
	// browser is not what this row is about); the list is one tap away.
	var xrow = el("div", "row go-row");
	var xlabel = el("label", "row-label");
	xlabel.appendChild(document.createTextNode("Extensions"));
	var xdesc = "Add features to web pages and to the browser.";
	var xhelp = el("div", "row-help", xdesc);
	xlabel.appendChild(xhelp);
	xrow.appendChild(xlabel);
	xrow.appendChild(goButton("Manage Extensions", "extensions-manage", function () { openAddons("addons://list/extension"); }));
	box.appendChild(xrow);
	try {
		var AM = Cu.import("resource://gre/modules/AddonManager.jsm", {}).AddonManager;
		AM.getAddonsByTypes(["extension"], function (list) {
			list = list || [];
			var off = list.filter(function (a) { return !a.isActive; }).length;
			xhelp.textContent = xdesc + " " + (!list.length ? "None installed." : list.length + " installed" +
				(off ? ", " + off + " turned off." : "."));
		});
	} catch (e) {
		// The add-on manager not started yet, or not in this build: the button still goes there.
	}

	var grow = el("div", "row go-row");
	var glabel = el("label", "row-label");
	glabel.appendChild(document.createTextNode("Plugins"));
	var tags = pluginTags();
	glabel.appendChild(el("div", "row-help", tags ? flashSummary(tags) : "The plugin list is not available."));
	grow.appendChild(glabel);
	grow.appendChild(goButton("Manage Plugins", "plugins-manage", function () { openAddons("addons://list/plugin"); }));
	box.appendChild(grow);
	return box;
}

function renderChromePane(host) {
	var cfg = loadChrome();

	// First: the way to Personas, extensions and plugins (R8).
	host.appendChild(buildAddonsSection());

	var search = el("section", "group");
	search.appendChild(el("h2", "group-title", "Search"));
	var srow = el("div", "row");
	var slabel = el("label", "row-label");
	slabel.appendChild(document.createTextNode("Default search engine"));
	slabel.appendChild(el("div", "row-help",
		"Used when you type words instead of an address into a card's address bar."));
	var ssel = document.createElement("select");
	ssel.className = "sel";
	ssel.id = "search-engine";
	for (var e = 0; e < SEARCH_ENGINES.length; e++) {
		var opt = document.createElement("option");
		opt.setAttribute("value", SEARCH_ENGINES[e][0]);
		opt.appendChild(document.createTextNode(SEARCH_ENGINES[e][1]));
		ssel.appendChild(opt);
	}
	ssel.value = cfg.searchEngine;
	ssel.addEventListener("change", function () {
		cfg.searchEngine = ssel.value;
		var now = saveChrome(cfg);
		ssel.value = now.searchEngine;
		status("Search engine saved.");
	}, false);
	srow.appendChild(slabel);
	srow.appendChild(ssel);
	search.appendChild(srow);
	host.appendChild(search);

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

	// Before the link editor, not after it: that list can be twelve rows long, and these
	// four are what people come to this page for.
	var clear = el("section", "group");
	clear.appendChild(el("h2", "group-title", "Clear Data"));
	for (var c = 0; c < CLEAR_ACTIONS.length; c++) {
		clear.appendChild(buildClearRow(CLEAR_ACTIONS[c]));
	}
	host.appendChild(clear);

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
		"The search engine, the home button and the start page are the app's, not the " +
		"engine's: an open card picks up a change here at once. Flash is in Content. " +
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

	if (pane.about) {
		renderAboutPane(host);
		markTab(id);
		status("");
		return;
	}

	// The Browser pane is a form over one JSON pref, not a table of prefs.
	if (pane.chrome) {
		renderChromePane(host);
		markTab(id);
		status("");
		return;
	}

	for (let g = 0; g < pane.groups.length; g++) {
		let group = pane.groups[g];
		if (group.build) { host.appendChild(group.build(group)); continue; }
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

// --- About (owner 2026-10-07: "add an about page to the settings page") -----------------------
// Facts read at run time, so the page can never disagree with what is installed: which of the
// three apps this is and its version (the app's own appinfo.json, beside this variant's profile),
// the engine underneath, and the Flash plugin the engine found.
function readAppInfo() {
	try {
		let f = Services.dirsvc.get("ProfD", Ci.nsIFile).parent;   // <app>/profile -> <app>
		f.append("appinfo.json");
		if (!f.exists()) { return null; }
		let is = Cc["@mozilla.org/network/file-input-stream;1"].createInstance(Ci.nsIFileInputStream);
		is.init(f, 0x01, 0, 0);
		let ss = Cc["@mozilla.org/scriptableinputstream;1"].createInstance(Ci.nsIScriptableInputStream);
		ss.init(is);
		let text = ss.read(Math.min(is.available(), 65536));
		ss.close();
		return JSON.parse(text);
	} catch (e) {
		return null;
	}
}

function renderAboutPane(host) {
	let app = readAppInfo() || {};
	let shells = {
		"net.riverstonerelay.jihad-browser": "Enyo 1 (classic)",
		"net.riverstonerelay.jihad-browser-mochi": "Enyo 2 / Mochi",
		"net.riverstonerelay.jihad-browser-mojo": "Mojo"
	};
	// From the plugin host, as the Browser pane reads it: navigator.plugins leaves out a plugin that is
	// turned off, which read as "not installed" (inspector 2026-10-12).
	let flash = "", tags = pluginTags(), ft = tags ? tags.filter(isFlash)[0] : null;
	if (ft) { flash = String(ft.description || ft.name || "Flash").slice(0, 60) + " (" + pluginState(ft) + ")"; }

	let head = el("section", "group about-head");
	head.appendChild(el("h2", "about-name", "Jihad Browser"));
	head.appendChild(el("div", "about-tag", "Goanna inside, webOS alive."));
	head.appendChild(el("p", "about-blurb",
		"A modern web browser for the HP TouchPad and webOS 3: the UXP / Goanna engine under the " +
		"webOS browser you know, with Flash, real add-ons and themes, modern TLS and printing to " +
		"today's network printers. It installs beside the stock browser and never replaces it."));
	host.appendChild(head);

	let facts = el("section", "group");
	facts.appendChild(el("h2", "group-title", "This browser"));
	let rows = [
		["Interface", shells[app.id] || (app.title || "unknown")],
		["Version", app.version || "unknown"],
		["App id", app.id || "unknown"],
		["Engine", "Goanna " + Services.appinfo.platformVersion + " (UXP)"],
		["Engine build", Services.appinfo.platformBuildID],
		["Flash", flash || "not installed"],
		["User agent", navigator.userAgent]
	];
	for (let i = 0; i < rows.length; i++) {
		let r = el("div", "about-row");
		r.appendChild(el("span", "about-key", rows[i][0]));
		r.appendChild(el("span", "about-val", String(rows[i][1])));
		facts.appendChild(r);
	}
	host.appendChild(facts);

	let lic = el("section", "group");
	lic.appendChild(el("h2", "group-title", "Licences and credits"));
	let credits = [
		"The browser's interface and its IPC daemon: Apache License 2.0 — built on HP/Palm's isis-browser for webOS.",
		"The engine (UXP / Goanna) and its Jihad backend: Mozilla Public License 2.0. Pale Moon, Basilisk and Moonchild trademarks are removed from this build.",
		"Palefill (web technologies polyfill), by Martok and contributors: Mozilla Public License 2.0, shipped unmodified.",
		"Adobe Flash Player is the device's own plugin; Jihad loads it, it does not ship it.",
		"webOS, Enyo and Mojo are the frameworks of Palm / HP webOS; Mochi is Enyo 2's webOS-styled library."
	];
	for (let i = 0; i < credits.length; i++) { lic.appendChild(el("p", "about-credit", credits[i])); }
	host.appendChild(lic);
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
		announce("prefs-reset", "Search engine, home page and start page links restored to their defaults.", false);
		return;
	}
	let n = 0;
	for (let g = 0; g < pane.groups.length; g++) {
		if (pane.groups[g].reset) { n += pane.groups[g].reset(); }
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

// about:settings#pane=content&section=fonts opens a pane, and a section of it, directly. A fragment
// without pane= (the chrome= one the settings write) changes nothing here.
function showHashTarget() {
	let hash = String(window.location.hash || "");
	let want = /(?:^#|&)pane=([a-z]+)/.exec(hash);
	if (!want || !PANES.some(function (p) { return p.id === want[1]; })) { return; }
	if (gPane !== want[1]) { renderPane(want[1]); }
	let section = /(?:^#|&)section=([a-z]+)/.exec(hash);
	let target = section ? document.getElementById("section-" + section[1]) : null;
	if (target) { target.scrollIntoView(true); }
	// The same link followed again while it is the page's address changes nothing (an unchanged
	// fragment fires no hashchange). The link is not taken out of the address to make room for that:
	// replaceState refuses an about: page (UXP's docshell cannot read a user:password part of one),
	// and location.replace to an empty fragment scrolls to the top (inspector, round 22; noted).
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
	showHashTarget();
	// The same page asked for again with another fragment is not loaded again.
	window.addEventListener("hashchange", showHashTarget, false);
	document.getElementById("reset-pane").addEventListener("click", resetPane, false);
}

window.addEventListener("DOMContentLoaded", init, false);
