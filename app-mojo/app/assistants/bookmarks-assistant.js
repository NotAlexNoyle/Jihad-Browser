// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// Jihad Browser (Mojo) — bookmarks scene.
//
// Pushed from the toolbar's Bookmarks menu (the same three entries as the Enyo and Mochi
// shells: Bookmarks, History, Downloads). A Mojo List over JihadBookmarks: a tap pops back to
// the browser and opens the page, a swipe deletes the bookmark. Titles and urls come from
// pages, so they go through the list's text substitution html-escaped, never as markup.
function BookmarksAssistant() {}

BookmarksAssistant.prototype.setup = function () {
	this.items = JihadBookmarks.all().map(function (b) {
		return {
			url: b.url,
			titleText: HistoryAssistant.escape(b.title || b.url),
			urlText: HistoryAssistant.escape(b.url)
		};
	});
	this.controller.setupWidget("jihad-bookmarks-list", {
		itemTemplate: "bookmarks/bookmarks-row",
		listTemplate: "bookmarks/bookmarks-container",
		emptyTemplate: "bookmarks/bookmarks-empty",
		swipeToDelete: true,
		reorderable: false
	}, {items: this.items});

	this.handleTap = this.handleTap.bind(this);
	this.handleDelete = this.handleDelete.bind(this);
	Mojo.Event.listen(this.controller.get("jihad-bookmarks-list"), Mojo.Event.listTap, this.handleTap);
	Mojo.Event.listen(this.controller.get("jihad-bookmarks-list"), Mojo.Event.listDelete, this.handleDelete);
};

BookmarksAssistant.prototype.cleanup = function () {
	Mojo.Event.stopListening(this.controller.get("jihad-bookmarks-list"), Mojo.Event.listTap, this.handleTap);
	Mojo.Event.stopListening(this.controller.get("jihad-bookmarks-list"), Mojo.Event.listDelete, this.handleDelete);
};

//* Open the bookmark in the browser scene we came from (pop, never stack a second browser).
BookmarksAssistant.prototype.handleTap = function (event) {
	var item = event && event.item;
	if (!item || !item.url) {
		return;
	}
	this.controller.stageController.popScene({url: item.url});
};

BookmarksAssistant.prototype.handleDelete = function (event) {
	var item = event && event.item;
	if (item && item.url) {
		JihadBookmarks.remove(item.url);
	}
};
