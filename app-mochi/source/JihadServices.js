// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadServices — the Enyo 2 equivalent of the Enyo 1.0 app's DbService /
// enyo.PalmService kinds, which the Mochi framework stack does NOT bundle. It is
// a thin wrapper over the platform's raw PalmServiceBridge (the same low-level
// Luna transport the shell already uses for clearCookies/clearCache in
// JihadBrowser.js), exposing helpers for the two Luna surfaces the parity views
// need:
//
//   * db8 (palm://com.palm.db/*) against THIS PACKAGE'S OWN kinds,
//     net.riverstonerelay.jihad-browser-mochi.{history,bookmarks,preferences}:1
//     — declared in ../db/kinds/, owned by this app id, and shipped inside this
//     .ipk. NOT the stock com.palm.* kinds, and (since review F-1) NOT the Enyo
//     variant's kinds either: see the ownership note above scope.kinds below.
//   * the download manager (palm://com.palm.downloadmanager/*) and the app
//     launcher (palm://com.palm.applicationManager/*), matching
//     BrowserApp.js downloadService / launchApplicationService.
//
// This adds NO engine (BrowserAdapter) methods — it only touches Luna services,
// exactly the set the Enyo 1.0 app touches. Off-device (no PalmServiceBridge) the
// calls are inert and report {returnValue:false, offline:true} so callers never
// hang; live behaviour is DEVICE-GATED.

enyo.jihad = enyo.jihad || {};

(function(scope) {
	var DB_URI = "palm://com.palm.db/";
	var DL_URI = "palm://com.palm.downloadmanager/";
	var APP_URI = "palm://com.palm.applicationManager/";

	// db8 kinds — THIS PACKAGE'S OWN, namespaced under its own app id.
	//
	// Review F-1 (cavekit-device-build.md R7): these used to be the Enyo variant's
	// kinds — `owner: net.riverstonerelay.jihad-browser`, declared in ../../app/db/
	// and shipped ONLY inside the Enyo .ipk — with app/db/permissions/* extended to
	// grant this app id CRUD. That made the three variants co-own their data layer,
	// which R7 forbids and which broke both ways: install Mochi ALONE and the kinds
	// were never registered, so every db8 call failed; remove the Enyo package and
	// Mochi's data layer went with a package it does not own.
	//
	// A db8 kind's `owner` MUST equal the app id that registers it, so the fix is
	// not a permission grant (that is the co-ownership, not a cure) but a separate
	// namespace: each variant declares, ships, owns and — on uninstall — takes with
	// it exactly its own kinds. Nothing is shared, so nothing can be broken by a
	// sibling's install or removal. Do not swap these back to the Enyo ids, and do
	// not add a cross-variant caller grant to make them "shareable".
	//
	// Consequence, by design: history/bookmarks/preferences are PER VARIANT. The
	// Mochi browser does not see the Enyo browser's history. Three independent
	// browsers have three independent profiles — the same way each has its own
	// engine profile under /var/palm/jihad/<variant>/ (R8).
	//
	// The namespace is HYPHENATED (`…jihad-browser-mochi.*`) because the app id is,
	// and a kind's `owner` must equal the registering app id. The app id lost its
	// dot on 2026-08-01 for a packaging reason, not a db8 one: ipkg removes a
	// package by globbing its metadata as `<pkgid>.*`, which also matched
	// `<pkgid>.mochi.control`, so removing the Enyo package destroyed this
	// package's control scripts (context/impl/impl-ipkg-prefix-collision.md).
	scope.kinds = {
		history:     "net.riverstonerelay.jihad-browser-mochi.history:1",
		bookmarks:   "net.riverstonerelay.jihad-browser-mochi.bookmarks:1",
		preferences: "net.riverstonerelay.jihad-browser-mochi.preferences:1"
	};

	// Keep in-flight bridges referenced so the native side isn't GC'd mid-call.
	var pending = [];

	//* Low-level Luna call. `uri` is a full palm://service/method URI; `params`
	//* is a plain object (JSON-encoded here); `onResult(response)` receives the
	//* parsed reply object. Returns the bridge (or null off-device).
	function lunaCall(uri, params, onResult) {
		if (typeof PalmServiceBridge === "undefined") {
			if (onResult) { onResult({returnValue: false, offline: true}); }
			return null;
		}
		var bridge = new PalmServiceBridge();
		pending.push(bridge);
		bridge.onservicecallback = function(payload) {
			var resp;
			try {
				resp = enyo.json.parse(payload);
			} catch (e) {
				resp = {returnValue: false, parseError: true};
			}
			var i = enyo.indexOf(bridge, pending);
			if (i >= 0) { pending.splice(i, 1); }
			if (onResult) { onResult(resp); }
		};
		bridge.call(uri, enyo.json.stringify(params || {}));
		return bridge;
	}
	scope.luna = lunaCall;

	// Kept forever on purpose — a subscription bridge that is collected stops delivering.
	var subscriptions = [];

	//* Low-level Luna SUBSCRIPTION. Same shape as lunaCall, with the two differences that make
	//* a subscription work at all: `subscribe:true` goes on the request, and the bridge is
	//* NEVER spliced out of the keep-alive list — lunaCall removes it on the first reply,
	//* which for a subscription is the handshake, after which the native side would be
	//* collected and every later push dropped. `onResult` therefore fires MANY times: once for
	//* {"returnValue":true,"subscribed":true} and again for each pushed payload.
	//*
	//* There is no unsubscribe: the only subscriber in this shell lives for the life of the
	//* card, and the hub drops the subscription when the card goes away.
	function lunaSubscribe(uri, params, onResult) {
		if (typeof PalmServiceBridge === "undefined") {
			if (onResult) { onResult({returnValue: false, offline: true}); }
			return null;
		}
		var bridge = new PalmServiceBridge();
		subscriptions.push(bridge);
		bridge.onservicecallback = function(payload) {
			var resp;
			try {
				resp = enyo.json.parse(payload);
			} catch (e) {
				resp = {returnValue: false, parseError: true};
			}
			if (onResult) { onResult(resp); }
		};
		var p = params || {};
		p.subscribe = true;
		bridge.call(uri, enyo.json.stringify(p));
		return bridge;
	}
	scope.lunaSubscribe = lunaSubscribe;

	// --- db8 -----------------------------------------------------------------
	scope.db = function(method, params, onResult) {
		return lunaCall(DB_URI + method, params, onResult);
	};
	//* find: onResult receives the results array (or [] on failure), matching how
	//* the Enyo 1.0 lists consume queryResponse.
	scope.dbFind = function(kind, where, orderBy, desc, onResult) {
		var q = {from: kind};
		if (where)   { q.where = where; }
		if (orderBy) { q.orderBy = orderBy; }
		if (desc)    { q.desc = true; }
		return scope.db("find", {query: q}, function(resp) {
			if (onResult) { onResult((resp && resp.results) || [], resp); }
		});
	};
	scope.dbPut = function(objects, onResult) {
		return scope.db("put", {objects: objects}, onResult);
	};
	scope.dbMerge = function(objects, onResult) {
		return scope.db("merge", {objects: objects}, onResult);
	};
	//* merge-by-query: set `props` on every object matching `where` in `kind`
	//* (BrowserApp.js preferenceChanged uses this for toggles).
	scope.dbMergeQuery = function(kind, where, props, onResult) {
		return scope.db("merge", {query: {from: kind, where: where}, props: props}, onResult);
	};
	scope.dbDel = function(ids, onResult) {
		return scope.db("del", {ids: ids}, onResult);
	};
	scope.dbDelByQuery = function(kind, where, onResult) {
		var q = {from: kind};
		if (where) { q.where = where; }
		return scope.db("del", {query: q}, onResult);
	};

	// --- download manager ----------------------------------------------------
	scope.download        = function(params, onResult) { return lunaCall(DL_URI + "download", params, onResult); };
	scope.cancelDownload  = function(ticket, onResult) { return lunaCall(DL_URI + "cancelDownload", {ticket: ticket}, onResult); };
	scope.downloadHistory = function(owner, onResult)  { return lunaCall(DL_URI + "getAllHistory", {owner: owner}, onResult); };
	scope.clearDownloads  = function(owner, onResult)  { return lunaCall(DL_URI + "clearHistory", {owner: owner}, onResult); };

	// --- app launcher (open a downloaded file / resource) --------------------
	scope.launch = function(params, onResult) { return lunaCall(APP_URI + "open", params, onResult); };

	// --- this variant's own daemon service ------------------------------------
	//* The daemon's Luna service for THIS variant (never com.palm.browserServer — that is the
	//* stock browser's). `method` is one of its method names; replies are parsed objects.
	var OWN_URI = "palm://net.riverstonerelay.jihadBrowserMochi/";
	scope.own = function(method, params, onResult) { return lunaCall(OWN_URI + method, params, onResult); };

	// --- launcher / certificate manager / system services ---------------------
	scope.addLaunchPoint = function(params, onResult) { return lunaCall(APP_URI + "addLaunchPoint", params, onResult); };
	//* The details of the certificate an SSL warning is about. For as long as the warning is
	//* open the daemon keeps them in THIS app's own directory, as certinfo/<name of the
	//* certificate file>.js — one call to window.jihadCertInfo carrying one object. (The Enyo
	//* shell asks com.palm.certificatemanager; on webOS CE that service does not exist for a
	//* card, and while the warning is up the daemon answers nothing over Luna. LunaSysMgr
	//* lets a card load local files only from its own directory, and its WebKit refuses a
	//* local file over XMLHttpRequest — hence a script, here.) onResult gets the object, or
	//* {returnValue:false} if nothing arrived in three seconds.
	scope.certDetails = function(file, onResult) {
		var done = false, tag = null, timer = null;
		function finish(o) {
			if (done) { return; }
			done = true;
			if (timer) { window.clearTimeout(timer); }
			window.jihadCertInfo = null;
			if (tag && tag.parentNode) { tag.parentNode.removeChild(tag); }
			if (onResult) { onResult(o || {returnValue: false}); }
		}
		try {
			window.jihadCertInfo = function(o) { finish(o); };
			tag = document.createElement("script");
			tag.onerror = function() { finish(null); };
			tag.src = "certinfo/" + String(file).replace(/^.*\//, "") + ".js";
			document.getElementsByTagName("head")[0].appendChild(tag);
			timer = window.setTimeout(function() { finish(null); }, 3000);
		} catch (e) { finish(null); }
	};
	scope.importWallpaper = function(path, onResult) {
		return lunaCall("palm://com.palm.systemservice/wallpaper/importWallpaper", {target: path}, onResult);
	};
	scope.setSystemPreferences = function(prefs, onResult) {
		return lunaCall("palm://com.palm.systemservice/setPreferences", prefs, onResult);
	};
	//* Fetch `url` into the user's downloads folder with the SYSTEM download manager and report
	//* progress to onStatus (many calls; the last carries `completed`). Used for images picked
	//* from a page: the download manager does the writing, into the one place on the user
	//* volume this browser is allowed to add files (cavekit-device-build.md R8).
	scope.DOWNLOAD_DIR = "/media/internal/downloads";
	scope.fetchToDownloads = function(url, onStatus) {
		return lunaSubscribe(DL_URI + "download",
			{target: url, targetDir: scope.DOWNLOAD_DIR, keepFilenameOnRedirect: true}, onStatus);
	};

	// --- small platform helpers the Enyo 1 framework used to supply -----------
	//* A system banner (enyo.windows.addBannerMessage in Enyo 1 wraps this same call).
	scope.banner = function(text) {
		try {
			if (window.PalmSystem && window.PalmSystem.addBannerMessage) {
				window.PalmSystem.addBannerMessage(String(text), "{}", "");
				return true;
			}
		} catch (e) {}
		return false;
	};
	//* Put `text` on the clipboard: a throwaway textarea and the cut command, which is what
	//* enyo.dom.setClipboard does in Enyo 1. Returns false where the command is unavailable.
	scope.setClipboard = function(text) {
		var ok = false, ta = null;
		try {
			ta = document.createElement("textarea");
			ta.style.position = "absolute"; ta.style.left = "-2000px"; ta.style.top = "0";
			ta.value = String(text);
			document.body.appendChild(ta);
			ta.select();
			ok = document.execCommand("cut");
		} catch (e) { ok = false; }
		if (ta && ta.parentNode) { ta.parentNode.removeChild(ta); }
		return ok;
	};

	// --- default search engine -------------------------------------------------
	// The Enyo shell asks com.palm.universalsearch for the system list and writes the choice
	// back system-wide; when that list is empty it falls back to exactly these three. This
	// shell keeps the choice in the card's own store instead: a browser variant changing the
	// search engine Just Type uses is not something its settings panel should do quietly.
	scope.searchEngines = [
		{id: "duckduckgo", name: "DuckDuckGo", url: "https://duckduckgo.com/?q={q}"},
		{id: "google",     name: "Google",     url: "https://www.google.com/search?q={q}"},
		{id: "wikipedia",  name: "Wikipedia",  url: "https://en.m.wikipedia.org/wiki/Special:Search?search={q}"}
	];
	var SEARCH_KEY = "jihad.mochi.searchEngine";
	scope.loadSearchEngine = function() {
		var id = null;
		try { id = window.localStorage ? window.localStorage.getItem(SEARCH_KEY) : null; } catch (e) { id = null; }
		for (var i = 0; i < scope.searchEngines.length; i++) {
			if (scope.searchEngines[i].id === id) { return scope.searchEngines[i]; }
		}
		return scope.searchEngines[0];
	};
	scope.saveSearchEngine = function(id) {
		try { if (window.localStorage) { window.localStorage.setItem(SEARCH_KEY, id); } } catch (e) {}
		return scope.loadSearchEngine();
	};
	scope.searchUrlFor = function(text) {
		return scope.loadSearchEngine().url.replace("{q}", encodeURIComponent(text));
	};

	//* App id for download ownership (BrowserApp.js enyo.fetchAppId()). Enyo 2
	//* has no fetchAppId(); read PalmSystem.identifier ("<appid> <processid>").
	scope.appId = function() {
		if (window.PalmSystem && window.PalmSystem.identifier) {
			return String(window.PalmSystem.identifier).split(" ")[0];
		}
		return "net.riverstonerelay.jihad-browser-mochi";
	};
})(enyo.jihad);
