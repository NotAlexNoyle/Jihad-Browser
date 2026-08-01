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

	//* App id for download ownership (BrowserApp.js enyo.fetchAppId()). Enyo 2
	//* has no fetchAppId(); read PalmSystem.identifier ("<appid> <processid>").
	scope.appId = function() {
		if (window.PalmSystem && window.PalmSystem.identifier) {
			return String(window.PalmSystem.identifier).split(" ")[0];
		}
		return "net.riverstonerelay.jihad-browser-mochi";
	};
})(enyo.jihad);
