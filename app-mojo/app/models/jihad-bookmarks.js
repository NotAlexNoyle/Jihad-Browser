// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// Jihad Browser (Mojo) — bookmarks.
//
// The Enyo and Mochi shells keep bookmarks in their own db8 kinds; this package registers
// no db8 kinds (see jihad-history.js for why), so Mojo keeps its list in the card's
// localStorage under one key, newest first, capped so a long-lived card cannot grow it
// without bound. Same rules as the history store: this variant sees only its own
// bookmarks, writes nothing outside its app storage, and removing the package removes them.
//
// Added 2026-10-06 with the upper-right menu every shell now shares (owner: "the upper right
// menu has the same options on all 3 variants"): Share ▸ Add Bookmark writes here, Bookmarks
// ▸ Bookmarks lists it, and the settings page's "Clear Bookmarks" empties it.
var JihadBookmarks = {
	KEY: "jihad.mojo.bookmarks",
	LIMIT: 500,

	//* All bookmarks, newest first: [{url, title, date}, ...]. Always an array.
	all: function () {
		try {
			var raw = window.localStorage && window.localStorage.getItem(this.KEY);
			var list = raw ? Mojo.parseJSON(raw) : null;
			return (list && list.length !== undefined) ? list : [];
		} catch (e) {
			Mojo.Log.error("[Jihad] bookmarks unreadable: %s", e);
			return [];
		}
	},

	//* Bookmark a page. The same url again MOVES it to the top with the new title rather
	//* than adding a duplicate. Returns true when it was written.
	add: function (url, title) {
		if (!url) {
			return false;
		}
		var list = this.all().filter(function (b) { return b && b.url !== url; });
		list.unshift({url: String(url), title: String(title || url), date: Date.now()});
		return this.save(list.slice(0, this.LIMIT));
	},

	has: function (url) {
		return this.all().some(function (b) { return b && b.url === url; });
	},

	remove: function (url) {
		return this.save(this.all().filter(function (b) { return b && b.url !== url; }));
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
			if (!window.localStorage) {
				return false;
			}
			// JSON.stringify, not Mojo.stringifyJSON: see jihad-history.js.
			window.localStorage.setItem(this.KEY, JSON.stringify(list));
			return true;
		} catch (e) {
			Mojo.Log.error("[Jihad] bookmarks not saved: %s", e);
			return false;
		}
	}
};
