// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadChromePrefs — the settings this front-end owns by itself: the start page's
// shortcut list, the home button's target and (since 2026-10-05) the default search engine.
// All three are EDITED on the settings page (about:preferences); what is here is the card's
// cache of them, refreshed by adoptFromUrl.
//
// Storage is the CARD's own localStorage, not the db8 preferences kind this
// package owns: those two settings are pure chrome state, they are read while
// the start page is being built (db8 is asynchronous, localStorage is not), and
// keeping them here registers no new kind and adds nothing to the frozen
// BrowserAdapter method set. Deliberately a SEPARATE copy from the Enyo and Mojo
// variants' — the three shells are not a shared installation.

enyo.jihadChrome = (function () {
	var LINKS_KEY = "jihad.startPageLinks";
	var HOME_KEY  = "jihad.homeUrl";
	var SEARCH_KEY = "jihad.searchEngine";
	//* Where the card's own settings panel kept the choice before the panel was merged into
	//* the settings page. Read once as the starting value, never written again.
	var SEARCH_KEY_OLD = "jihad.mochi.searchEngine";
	var LITE_MIGRATED_KEY = "jihad.searchEngine.liteMigrated";
	//* The ids the settings page offers (packaging/prefsui/content/preferences.js
	//* SEARCH_ENGINES); the first is the default. enyo.jihad.searchEngines maps each to a url.
	var SEARCH_IDS = ["duckduckgo-lite", "duckduckgo-html", "duckduckgo", "google", "wikipedia"];
	function safeEngine(id) {
		for (var i = 0; i < SEARCH_IDS.length; i++) { if (SEARCH_IDS[i] === id) { return id; } }
		return "";
	}
	// A start page with a hundred rows is not a start page; the cap also bounds
	// what a corrupt or hand-edited store can make the page render.
	var MAX = 12;
	// The home button's default. A real url, deliberately: about:home is not a page
	// this engine serves, so a button pointing at it would go nowhere.
	var HOME_DEFAULT = "https://start.duckduckgo.com/";

	function trim(s) {
		return (typeof s === "string") ? s.replace(/^\s+|\s+$/g, "") : "";
	}

	function read(key) {
		try {
			return window.localStorage ? window.localStorage.getItem(key) : null;
		} catch (e) { return null; }
	}

	function write(key, value) {
		try {
			if (!window.localStorage) { return; }
			if (value === null) { window.localStorage.removeItem(key); }
			else { window.localStorage.setItem(key, value); }
		} catch (e) { /* a full or disabled store must not break preferences */ }
	}

	//* The out-of-the-box shortcut list. Returned as a fresh copy so a caller that
	//* edits the result (the preferences editor does) cannot mutate the defaults.
	function defaultLinks() {
		return [
			{title: "DuckDuckGo",    url: "https://duckduckgo.com/"},
			{title: "Wikipedia",     url: "https://en.m.wikipedia.org/"},
			{title: "webOS Archive", url: "https://webosarchive.org/"}
		];
	}

	//* Only http(s) and our own about: pages may be navigated to from a stored setting.
	//* Anything else — javascript:, data:, file: — is DROPPED rather than sanitised: these
	//* values end up in the home button and the start page's shortcut list, one tap from the
	//* user, and a javascript: target runs script in page context (the daemon's url fixup
	//* passes javascript: through). Byte-identical allowlist to the Enyo and Mojo shells'
	//* safeUrl and to the settings page's own safeUrl (packaging/prefsui/content/preferences.js);
	//* the four must stay in step, or a value one accepts and another drops reads to the user as
	//* "settings did not save". Applied on the way OUT (loadLinks/loadHome) as well as in, so a
	//* store written before this allowlist existed, or hand-edited, is neutralised on read.
	function safeUrl(u) {
		var v = trim(u);
		return (/^https?:\/\//i.test(v) || /^about:(preferences|settings|jihad|isis|blank)$/i.test(v)) ? v : "";
	}

	//* Keep only rows that have both a label and a target. Anything else came from
	//* a half-finished edit or a corrupt store and must not reach the page.
	function cleanLinks(list) {
		var out = [], i, t, u;
		if (!list || !list.length) { return out; }
		for (i = 0; i < list.length && out.length < MAX; i++) {
			t = trim(list[i] && list[i].title);
			u = safeUrl(list[i] && list[i].url);
			if (t && u) { out.push({title: t, url: u}); }
		}
		return out;
	}

	return {
		MAX: MAX,
		HOME_DEFAULT: HOME_DEFAULT,
		defaultLinks: defaultLinks,
		cleanLinks: cleanLinks,

		//* Never throws and never returns empty: a missing, unreadable or empty store
		//* yields the defaults, so the start page always has something.
		loadLinks: function () {
			var raw = read(LINKS_KEY), list = null;
			if (raw) {
				try { list = JSON.parse(raw); } catch (e) { list = null; }
			}
			list = cleanLinks(list);
			return list.length ? list : defaultLinks();
		},

		//* The settings url for the one editing surface, carrying THIS card's values.
		settingsUrl: function () {
			return "about:settings#chrome=" + encodeURIComponent(JSON.stringify({
				searchEngine: this.loadSearchEngine(), homeUrl: this.loadHome(), startLinks: this.loadLinks()
			}));
		},

		//* Adopt an edit published by the settings page. Gated on the PATH, not the scheme:
		//* about:blank is content-loadable, so accepting any "about:" url would let a visited
		//* page rewrite the home button. Same rule as the Enyo shell's ChromePrefs.
		adoptFromUrl: function (url) {
			var base = url ? String(url).split("#")[0] : "";
			if (base !== "about:preferences" && base !== "about:settings") { return false; }
			var m = /[#&]chrome=([^&]*)/.exec(String(url));
			if (!m) { return false; }
			var o = null;
			try { o = JSON.parse(decodeURIComponent(m[1])); } catch (e) { return false; }
			if (!o) { return false; }
			var before = JSON.stringify(this.loadLinks()) + this.loadHome() + this.loadSearchEngine();
			this.saveHome(o.homeUrl);
			this.saveLinks(o.startLinks);
			// A page older than the search row sends no engine: keep the card's.
			// From the settings page the engine is the user's choice, never the pre-Lite default.
			if (safeEngine(o.searchEngine)) { write(SEARCH_KEY, o.searchEngine); write(LITE_MIGRATED_KEY, "1"); }
			return (JSON.stringify(this.loadLinks()) + this.loadHome() + this.loadSearchEngine()) !== before;
		},

		//* The id of the engine the address bar searches with: the one adopted from the
		//* settings page, else the one the old card panel stored, else the default.
		//* A stored "duckduckgo" from before 2026-10-06 (when it was the only DuckDuckGo and the
		//* default) is the old default, not a choice against Lite: moved to Lite once, the move
		//* recorded so a later deliberate choice of the full site stays.
		loadSearchEngine: function () {
			var id = safeEngine(read(SEARCH_KEY)) || safeEngine(read(SEARCH_KEY_OLD));
			if (!read(LITE_MIGRATED_KEY)) {
				if (id === "duckduckgo") { id = "duckduckgo-lite"; write(SEARCH_KEY, id); }
				write(LITE_MIGRATED_KEY, "1");
			}
			return id || SEARCH_IDS[0];
		},

		//* Returns the cleaned list actually stored. Saving an empty list clears the
		//* override, so the page falls back to the defaults rather than showing nothing.
		saveLinks: function (list) {
			var kept = cleanLinks(list);
			write(LINKS_KEY, kept.length ? JSON.stringify(kept) : null);
			return kept.length ? kept : defaultLinks();
		},

		//* Always a loadable url: an unset, blanked or DISALLOWED home falls back to the
		//* default. safeUrl on READ as well as write, so a store written before the allowlist
		//* existed cannot leave a javascript: target on the home button.
		loadHome: function () {
			return safeUrl(read(HOME_KEY)) || HOME_DEFAULT;
		},

		//* An empty value clears the override rather than storing a home that goes
		//* nowhere. Returns what is now in effect. A value outside safeUrl's allowlist is
		//* treated as empty — dropped, not sanitised.
		saveHome: function (url) {
			var u = safeUrl(url);
			write(HOME_KEY, u || null);
			return u || HOME_DEFAULT;
		}
	};
}());
