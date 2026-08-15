// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../LICENSE.
//
// ChromePrefs — the two settings this shell owns rather than the engine: the start
// page's shortcut list and the home button's target.
//
// SOURCE OF TRUTH IS THE ENGINE, not this card (cavekit-preferences-ui.md R5): they are
// edited in about:preferences, which stores them in one engine pref
// (`jihad.chrome.settings`). What is HERE is a CACHE of that value, kept in the card's own
// localStorage so the start page can be built synchronously on a cold start.
//
// The cache is refreshed by `adoptFromUrl()`, called with every committed url: the settings
// page publishes an edit by rewriting its own fragment, and that url is the only channel
// between the two. There is deliberately no Luna read here — the daemon does expose
// `getChromePrefs`, but it registers on the private bus and this card is on the public one,
// so the call never arrives. See cavekit-preferences-ui.md R5.
//
// The page->card leg WORKS as of 2026-08-06 (device-verified end to end: a real click on the
// page's own control -> publishToFragment -> daemon same-document location change ->
// msgTitleAndUrlChanged -> here). It needed a body in PageChrome::OnLocationChange, because a
// fragment rewrite is a SAME-DOCUMENT navigation and so produces no load lifecycle for the
// daemon's completion boundary to report from. The earlier note here — "the ref is not reaching
// the card" — was WRONG and is corrected: the ref was always delivered on a real load; the
// evidence for it was a stale daemon.log line, frozen because /var had filled up.
// Nothing here is added to the frozen callBrowserAdapter set.

enyo.jihadChrome = (function () {
	var KEY = "jihad.chromeSettings";
	var MAX = 12;
	var HOME_DEFAULT = "https://start.duckduckgo.com/";
	var cache = null;   // null until the first successful read

	function trim(s) {
		return (typeof s === "string") ? s.replace(/^\s+|\s+$/g, "") : "";
	}

	function defaultLinks() {
		return [
			{title: "DuckDuckGo",    url: "https://duckduckgo.com/"},
			{title: "Wikipedia",     url: "https://en.m.wikipedia.org/"},
			{title: "webOS Archive", url: "https://webosarchive.org/"}
		];
	}

	//* Keep only rows with both a label and a target — the stored value is user-editable.
	function cleanLinks(list) {
		var out = [], i, t, u;
		if (!list || !list.length) { return out; }
		for (i = 0; i < list.length && out.length < MAX; i++) {
			t = trim(list[i] && list[i].title);
			u = trim(list[i] && list[i].url);
			u = safeUrl(u);
			if (t && u) { out.push({title: t, url: u}); }
		}
		return out;
	}

	//* Only http(s) and our own about: pages may be navigated to from a stored setting.
	//* Anything else — javascript:, data:, file: — is dropped rather than sanitised: these
	//* values end up in the home button and the start page, which are one tap from the user.
	function safeUrl(u) {
		var v = trim(u);
		return (/^https?:\/\//i.test(v) || /^about:(preferences|settings|jihad|isis|blank)$/i.test(v)) ? v : "";
	}

	function normalize(o) {
		var links = cleanLinks(o && o.startLinks);
		return {
			homeUrl: safeUrl(o && o.homeUrl) || HOME_DEFAULT,
			startLinks: links.length ? links : defaultLinks()
		};
	}

	return {
		MAX: MAX,
		HOME_DEFAULT: HOME_DEFAULT,
		defaultLinks: defaultLinks,

		//* True when this url is the settings page (path only — see adoptFromUrl for why the
		//* scheme alone is not safe). Shared by the adopt gate and the diagnostic log so the
		//* two can never disagree about what counts as the settings page.
		isSettingsUrl: function (url) {
			var base = url ? String(url).split("#")[0] : "";
			return base === "about:preferences" || base === "about:settings";
		},

		//* How many shortcut rows the cache currently holds. For logging the SHAPE of an
		//* adopted payload without logging the user's urls.
		linkCount: function () {
			return this.load().startLinks.length;
		},

		//* Immediate, from cache. Never empty: defaults until the first read lands.
		loadLinks: function () {
			return this.load().startLinks;
		},
		loadHome: function () {
			return this.load().homeUrl;
		},

		//* Persisted here as a CACHE of what about:preferences holds — see the header.
		//* localStorage, so the start page can build synchronously on a cold start.
		load: function () {
			if (cache) { return cache; }
			var raw = null, o = null;
			try {
				if (window.localStorage) { raw = window.localStorage.getItem(KEY); }
			} catch (e) { raw = null; }
			if (raw) { try { o = enyo.json.parse(raw); } catch (e2) { o = null; } }
			cache = normalize(o);
			return cache;
		},

		//* The settings url for the one editing surface, carrying THIS card's current
		//* values so the page opens on what the user is actually using.
		settingsUrl: function () {
			var cfg = this.load();
			return "about:preferences#chrome=" +
				encodeURIComponent(enyo.json.stringify(cfg));
		},

		//* Called with every committed url. When it is the settings page carrying an edited
		//* payload, adopt it. Returns true when something changed, so the caller can redraw.
		//*
		//* This is the card's half of the sync: the page cannot reach this card's storage
		//* and the daemon's Luna service is on the private bus (an app card cannot call it),
		//* so the fragment on the url the card already receives is the channel.
		adoptFromUrl: function (url) {
			// Gate on the PATH, not the scheme. "about:" alone is not safe: about:blank is
			// content-loadable, so any visited page could do
			//     location.href = "about:blank#chrome=" + payload
			// and — because that is a cross-document navigation, so the card is told about it
			// — rewrite the home button and the start page. From there a javascript: home
			// target runs script in page context (the daemon's url fixup passes javascript:
			// through). Only the settings page may hand us settings. Caught in review
			// 2026-08-06, before the ref-delivery fix that would have armed it.
			if (!this.isSettingsUrl(url)) { return false; }
			var m = /[#&]chrome=([^&]*)/.exec(String(url));
			if (!m) { return false; }
			var o = null;
			try { o = enyo.json.parse(decodeURIComponent(m[1])); } catch (e) { return false; }
			if (!o) { return false; }
			var next = normalize(o);
			var before = enyo.json.stringify(cache);
			cache = next;
			try {
				if (window.localStorage) {
					window.localStorage.setItem(KEY, enyo.json.stringify(next));
				}
			} catch (e2) { /* a full store must not break navigation */ }
			return enyo.json.stringify(next) !== before;
		}
	};
}());
