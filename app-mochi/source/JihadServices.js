// Copyright 2026 the Jihad Browser project.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadServices — the Enyo 2 equivalent of the Enyo 1.0 app's DbService /
// enyo.PalmService kinds, which the Mochi framework stack does NOT bundle. It is
// a thin wrapper over the platform's raw PalmServiceBridge (the same low-level
// Luna transport the shell already uses for clearCookies/clearCache in
// JihadBrowser.js), exposing helpers for the two Luna surfaces the parity views
// need:
//
//   * db8 (palm://com.palm.db/*) against the SAME Jihad-owned kinds the Enyo 1.0
//     app uses (net.riverstonerelay.jihad-browser.{history,bookmarks,preferences}:1
//     — see ../../app/source/BrowserApp.js and ../../app/db/). NOT the stock
//     com.palm.* kinds.
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

	// db8 kinds — identical to the Enyo 1.0 app (do not swap to com.palm.*).
	scope.kinds = {
		history:     "net.riverstonerelay.jihad-browser.history:1",
		bookmarks:   "net.riverstonerelay.jihad-browser.bookmarks:1",
		preferences: "net.riverstonerelay.jihad-browser.preferences:1"
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
		return "net.riverstonerelay.jihad-browser.mochi";
	};
})(enyo.jihad);
