// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// JihadChromePrefs — the settings this front-end owns by itself: the start page's
// shortcut list and the home button's target.
//
// Storage is the CARD's own localStorage, the same store jihad-history.js uses,
// because this package registers no db8 kinds. NOTE the start page is a DOCUMENT
// rendered by the engine (start.html), not card chrome, so it cannot read this —
// the scene hands it the list in the url fragment instead (see main-assistant's
// startPageUrl()).

var JihadChromePrefs = (function () {
	var LINKS_KEY = "jihad.startPageLinks";
	var HOME_KEY  = "jihad.homeUrl";
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

	//* Keep only rows that have both a label and a target. Anything else came from
	//* a half-finished edit or a corrupt store and must not reach the page.
	function cleanLinks(list) {
		var out = [], i, t, u;
		if (!list || !list.length) { return out; }
		for (i = 0; i < list.length && out.length < MAX; i++) {
			t = trim(list[i] && list[i].title);
			u = trim(list[i] && list[i].url);
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

		//* Returns the cleaned list actually stored. Saving an empty list clears the
		//* override, so the page falls back to the defaults rather than showing nothing.
		saveLinks: function (list) {
			var kept = cleanLinks(list);
			write(LINKS_KEY, kept.length ? JSON.stringify(kept) : null);
			return kept.length ? kept : defaultLinks();
		},

		//* Always a loadable url: an unset or blanked home falls back to the default.
		loadHome: function () {
			return trim(read(HOME_KEY)) || HOME_DEFAULT;
		},

		//* An empty value clears the override rather than storing a home that goes
		//* nowhere. Returns what is now in effect.
		saveHome: function (url) {
			var u = trim(url);
			write(HOME_KEY, u || null);
			return u || HOME_DEFAULT;
		}
	};
}());

