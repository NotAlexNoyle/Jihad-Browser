// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// Jihad Browser (Mojo) — browsing history.
//
// PER-VARIANT AND SELF-CONTAINED, like everything else in this package. The Enyo
// and Mochi variants keep their history in their own db8 kinds; this package
// registers no db8 kinds, so Mojo keeps its own list in the card's localStorage.
// Same rule holds either way: this variant sees only its OWN history, it writes
// nothing outside its app storage, and removing the package takes the history with
// it. Nothing here touches a system file or another variant's data.
//
// The store is a single JSON array under one key, newest first, capped at LIMIT
// entries so a long-running card cannot grow it without bound (localStorage is a
// small, shared-per-origin quota — an unbounded list would eventually throw on
// write and take the chrome down with it).
var JihadHistory = {
	KEY: "jihad.mojo.history",
	LIMIT: 200,

	//* All entries, newest first: [{url, title, date}, ...]. Always an array.
	all: function () {
		try {
			var raw = window.localStorage && window.localStorage.getItem(this.KEY);
			var list = raw ? Mojo.parseJSON(raw) : null;
			return (list && list.length !== undefined) ? list : [];
		} catch (e) {
			// Corrupt or unavailable storage must not break browsing.
			Mojo.Log.warn("[Jihad] history unreadable: %s", e);
			return [];
		}
	},

	//* Record a visit. Re-visiting a url MOVES its entry to the top (and refreshes
	//* the title) instead of appending a duplicate, so a reload or a back/forward
	//* walk does not bury the rest of the list.
	add: function (url, title) {
		if (!url) { return; }
		try {
			var list = this.all(), i;
			for (i = 0; i < list.length; i++) {
				if (list[i].url === url) { list.splice(i, 1); break; }
			}
			list.unshift({url: url, title: title || url, date: (new Date()).getTime()});
			if (list.length > this.LIMIT) { list.length = this.LIMIT; }
			this.write(list);
		} catch (e) {
			Mojo.Log.warn("[Jihad] history not recorded: %s", e);
		}
	},

	clear: function () {
		this.write([]);
	},

	write: function (list) {
		try {
			if (window.localStorage) {
				window.localStorage.setItem(this.KEY, Mojo.stringifyJSON(list));
			}
		} catch (e) {
			// Quota exceeded (or storage disabled): drop the oldest half and retry
			// once, then give up quietly — history is a convenience, not the browser.
			try {
				if (list.length > 1) {
					list.length = Math.floor(list.length / 2);
					window.localStorage.setItem(this.KEY, Mojo.stringifyJSON(list));
				}
			} catch (e2) {
				Mojo.Log.warn("[Jihad] history not saved: %s", e2);
			}
		}
	}
};
