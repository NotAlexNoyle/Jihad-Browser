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

	//* Only http(s) and our own about: pages may be navigated to from a stored setting.
	//* Anything else — javascript:, data:, file: — is DROPPED rather than sanitised: these
	//* values end up in the home button and the start page's shortcut list, one tap from the
	//* user, and a javascript: target runs script in page context (the daemon's url fixup
	//* passes javascript: through).
	//*
	//* Byte-identical allowlist to the Enyo shell's ChromePrefs.safeUrl and to the settings
	//* page's own safeUrl (packaging/prefsui/content/preferences.js:277). The three must stay
	//* in step: a value the page accepts but a shell silently drops reads to the user as
	//* "settings did not save".
	//*
	//* Applied on the way OUT as well as in — cleanLinks backs loadLinks as well as saveLinks
	//* — so a store written before this allowlist existed, or hand-edited, is neutralised on
	//* read and not only on write.
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

		//* Returns the cleaned list actually stored. Saving an empty list clears the
		//* override, so the page falls back to the defaults rather than showing nothing.
		saveLinks: function (list) {
			var kept = cleanLinks(list);
			write(LINKS_KEY, kept.length ? JSON.stringify(kept) : null);
			return kept.length ? kept : defaultLinks();
		},

		//* Always a loadable url: an unset, blanked or DISALLOWED home falls back to the
		//* default. safeUrl on READ as well as write, so a store written before the allowlist
		//* existed cannot leave a javascript: target on the home button — handleCommand's
		//* "jihad-home" feeds this value straight into openUrl.
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
		},

		//* The settings-page url carrying THIS card's current home + shortcut list in the
		//* fragment, so about:preferences opens on this card's values (an engine-rendered
		//* document cannot read the card's localStorage directly — the values must travel in the
		//* url, exactly as the Enyo/Mochi shells do). The fragment key contract is
		//* {homeUrl, startLinks} — the same shape adoptFromUrl reads back and preferences.js
		//* consumes, so the card->page->card round-trip stays consistent. Added 2026-08-15 with
		//* the Settings command menu entry (user reversed the 2026-08-05 "no settings icon in
		//* mojo" direction).
		settingsUrl: function () {
			var cfg = { homeUrl: this.loadHome(), startLinks: this.loadLinks() };
			return "about:preferences#chrome=" + encodeURIComponent(JSON.stringify(cfg));
		},

		//* True when this url is the settings page. PATH ONLY — see adoptFromUrl for why the
		//* scheme alone is not safe. Factored out so the gate and any future caller can never
		//* disagree about what counts as the settings page, the same reason the Enyo shell
		//* factored it out (../../../app/source/ChromePrefs.js).
		isSettingsUrl: function (url) {
			var base = url ? String(url).split("#")[0] : "";
			return base === "about:preferences" || base === "about:settings";
		},

		//* Adopt an edit published by the settings page. Called with EVERY committed url
		//* (main-assistant.js adoptChromePrefs) and returns true only when something actually
		//* changed, so the caller knows whether the start page needs redrawing.
		//*
		//* This is the card's half of the sync: the page cannot reach this card's localStorage,
		//* and the daemon's chrome-prefs Luna service registers on the PRIVATE bus while an app
		//* card is on the public one (measured 2026-08-05 — the card's call never arrived), so
		//* the fragment on the url the card already receives is the only channel there is.
		//*
		//* GATE ON THE PATH, NOT THE SCHEME. "about:" alone is not safe: about:blank IS
		//* content-loadable, so any visited page could do
		//*     location.href = "about:blank#chrome=" + payload
		//* and — that being a cross-document navigation, so the card is told about it — rewrite
		//* the home button and the start page's shortcut list. about:preferences itself is NOT
		//* reachable that way, and that is what makes the path test a real gate rather than a
		//* naming convention: its about: module deliberately withholds
		//* URI_SAFE_FOR_UNTRUSTED_CONTENT (render/goanna/components/jihadAboutPreferences.js,
		//* getURIFlags — read, not assumed). Ported from the Enyo shell's ChromePrefs.adoptFromUrl
		//* where this was caught in review 2026-08-06. Do NOT relax it to a scheme test.
		//*
		//* safeUrl() is the SECOND line of defence and is not redundant with the gate: it runs on
		//* every value that comes out of the payload, so even a payload that reached here cannot
		//* put a javascript: url one tap from the user.
		adoptFromUrl: function (url) {
			if (!this.isSettingsUrl(url)) { return false; }
			var m = /[#&]chrome=([^&]*)/.exec(String(url));
			if (!m) { return false; }
			var o = null;
			try { o = JSON.parse(decodeURIComponent(m[1])); } catch (e) { return false; }
			if (!o) { return false; }
			// Compare what is actually STORED, before and after — saveLinks/saveHome clean, clamp
			// and drop, so a payload that differs only in rows the allowlist rejects is correctly
			// reported as "no change" and costs no start-page redraw. Links and home are compared
			// separately so a concatenation can never make two different states look equal.
			var beforeLinks = JSON.stringify(this.loadLinks());
			var beforeHome = this.loadHome();
			this.saveHome(o.homeUrl);
			this.saveLinks(o.startLinks);
			return JSON.stringify(this.loadLinks()) !== beforeLinks || this.loadHome() !== beforeHome;
		}
	};
}());

