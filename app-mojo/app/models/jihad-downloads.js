// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// Jihad Browser (Mojo) — downloads.
//
// A page that links to something the engine cannot show (an archive, a PDF, a media file)
// is saved by the ENGINE into the downloads folder (render/goanna/DownloadService.cpp), and
// the transfer is reported through the adapter's download callbacks: downloadStart(url),
// downloadProgress(url, soFar, total), downloadFinished(url, mimeType, path),
// downloadError(url, message). The Mojo WebView widget turns only downloadFinished into an
// event; the main scene adds the other three to the widget (main-assistant.js,
// hookDownloads) and records each transfer here, the Mojo equivalent of the Enyo and Mochi
// shells' download lists.
//
// localStorage, one key, newest first, capped. A record:
//   {url, name, state: "downloading" | "done" | "failed", received, total, path, mimeType,
//    message, date}
var JihadDownloads = {
	KEY: "jihad.mojo.downloads",
	LIMIT: 50,

	all: function () {
		try {
			var raw = window.localStorage && window.localStorage.getItem(this.KEY);
			var list = raw ? Mojo.parseJSON(raw) : null;
			return (list && list.length !== undefined) ? list : [];
		} catch (e) {
			Mojo.Log.error("[Jihad] downloads unreadable: %s", e);
			return [];
		}
	},

	//* The file name a url names (its last path segment, decoded), for the list.
	nameOf: function (url) {
		var path = String(url || "").split(/[?#]/)[0], name = path.substring(path.lastIndexOf("/") + 1);
		try { name = decodeURIComponent(name); } catch (e) {}
		return name || String(url || "");
	},

	//* A transfer began: a new record at the top.
	start: function (url) {
		var list = this.all();
		list.unshift({url: String(url || ""), name: this.nameOf(url), state: "downloading",
			received: 0, total: 0, date: Date.now()});
		this.save(list.slice(0, this.LIMIT));
	},

	update: function (url, fields) {
		var list = this.all(), i, k, rec = null;
		for (i = 0; i < list.length; i++) {
			if (list[i] && list[i].url === url && list[i].state === "downloading") { rec = list[i]; break; }
		}
		if (!rec) {
			// A transfer whose start this card did not see (it began before the card did).
			rec = {url: String(url || ""), name: this.nameOf(url), state: "downloading", received: 0, total: 0, date: Date.now()};
			list.unshift(rec);
		}
		for (k in fields) {
			if (fields.hasOwnProperty(k)) { rec[k] = fields[k]; }
		}
		if (rec.path) {
			rec.name = this.nameOf(rec.path);
		}
		this.save(list.slice(0, this.LIMIT));
		return rec;
	},

	clear: function () {
		try {
			if (window.localStorage) {
				window.localStorage.removeItem(this.KEY);
			}
		} catch (e) {}
	},

	save: function (list) {
		try {
			if (window.localStorage) {
				window.localStorage.setItem(this.KEY, JSON.stringify(list));
			}
		} catch (e) {
			Mojo.Log.error("[Jihad] downloads not saved: %s", e);
		}
	}
};
