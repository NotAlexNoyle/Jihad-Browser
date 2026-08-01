// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// JihadUrl — address-bar input handling for the Mojo front-end.
//
// Turns what the user typed into either a navigation or a search, mirroring the
// Enyo-1.0 shell's URLSearch.go()/looksLikeHost() (../../../app/source/URLSearch.js)
// and the Mochi shell's normalizeUrl() (../../../app-mochi/source/JihadBrowser.js) so
// all three variants resolve the same input the same way. Pure string handling — it
// touches neither the adapter nor any Luna service.
var JihadUrl = {

	//* Schemes the address bar loads as typed. javascript:/vbscript: are excluded on
	//* purpose: typing one is treated as a search, never as an in-page eval.
	isNavigableScheme: function (scheme) {
		switch (scheme) {
		case "http": case "https": case "ftp": case "file":
		case "about": case "mailto": case "tel": case "data":
		case "view-source": case "ws": case "wss": case "rtsp":
			return true;
		default:
			return false;
		}
	},

	//* Heuristic from URLSearch.looksLikeHost: does the raw input denote a host
	//* (=> navigate) rather than search terms?
	looksLikeHost: function (text) {
		var v = this.trim(text);
		if (!v || /\s/.test(v)) {
			return false;
		}
		if (/^localhost(:\d+)?([\/?#].*)?$/i.test(v)) {
			return true;
		}
		if (/^\d{1,3}(\.\d{1,3}){3}(:\d+)?([\/?#].*)?$/.test(v)) {
			return true;
		}
		// host.tld[:port][/path...] — the final label is a 2+ letter TLD.
		return /^[a-z0-9]([a-z0-9\-]*[a-z0-9])?(\.[a-z0-9]([a-z0-9\-]*[a-z0-9])?)*\.[a-z]{2,}(:\d+)?([\/?#].*)?$/i.test(v);
	},

	//* Default search provider, matching the other two variants' default.
	searchUrl: function (text) {
		return "https://duckduckgo.com/?q=" + encodeURIComponent(this.trim(text));
	},

	//* Address-bar entry -> the URL to open, or "" when there is nothing to do.
	normalize: function (text) {
		var v = this.trim(text);
		if (!v) {
			return "";
		}
		var m = v.match(/^([a-z][a-z0-9+.\-]*):/i);
		if (m && this.isNavigableScheme(m[1].toLowerCase())) {
			return v;
		}
		// Note the order: an UNRECOGNISED scheme falls through to the host test rather
		// than going straight to search, exactly as URLSearch.go() does — otherwise
		// "localhost:8080" and "ftp.example.com:21" parse as scheme "localhost" /
		// "ftp.example.com" and get searched for instead of opened.
		if (this.looksLikeHost(v)) {
			// A schemeless host is a navigation, not a search; default it to http://
			// and let the site redirect to https.
			return "http://" + v;
		}
		return this.searchUrl(v);
	},

	trim: function (text) {
		return (text === undefined || text === null ? "" : String(text))
			.replace(/^\s+|\s+$/g, "");
	}
};
