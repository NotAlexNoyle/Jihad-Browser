// Copyright 2026 the Jihad Browser project.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadBookmarkList — the Enyo 2 / Mochi port of ../../app/source/BookmarkList.js.
// A full-card overlay panel (enyo-fit, toggled by showing) listing the saved
// bookmarks from the SAME db8 kind the Enyo 1.0 app uses
// (net.riverstonerelay.jihad-browser.bookmarks:1, via enyo.jihad.dbFind). A tap
// selects a bookmark (doSelectItem -> shell navigates); a row Delete removes it
// (doDeleteBookmark + db8 del); the header + button adds the current page
// (doAddBookmark -> shell). enyo.Repeater (not enyo.List) is used because these
// lists are small and the flyweight virtual list is fragile on this engine.

enyo.kind({
	name: "JihadBookmarkList",
	kind: "FittableRows",
	classes: "jihad-panel enyo-fit",
	showing: false,
	published: {
		//* Bookmark records ({_id, title, url, ...}) currently displayed.
		items: []
	},
	events: {
		onSelectItem: "",
		onDeleteBookmark: "",
		onAddBookmark: "",
		onClose: ""
	},
	components: [
		{kind: "FittableColumns", classes: "jihad-panel-header", components: [
			{name: "doneBtn", classes: "jihad-panel-btn jihad-panel-done", ontap: "closeTapped", content: "Done"},
			{classes: "jihad-panel-title", fit: true, content: "Bookmarks"},
			{name: "addBtn", classes: "jihad-panel-btn jihad-panel-action", ontap: "addTapped", content: "Add"}
		]},
		{kind: "enyo.Scroller", fit: true, touch: true, classes: "jihad-panel-scroller", components: [
			{name: "list", kind: "enyo.Repeater", onSetupItem: "setupRow", components: [
				{name: "row", classes: "jihad-row", ontap: "rowTapped", components: [
					{kind: "FittableColumns", components: [
						{fit: true, components: [
							{name: "title", classes: "jihad-row-title"},
							{name: "url", classes: "jihad-row-url"}
						]},
						{name: "del", classes: "jihad-row-del", ontap: "deleteTapped", content: "Delete"}
					]}
				]}
			]},
			{name: "empty", classes: "jihad-empty", content: "No bookmarks yet.", showing: false}
		]}
	],
	create: function() {
		this.inherited(arguments);
		// `published` defaults live on the PROTOTYPE, so the literal [] above is one
		// array shared by every instance — and deleteTapped splices `items` in
		// place. Give each instance its own copy.
		this.items = this.items ? this.items.slice() : [];
	},
	//* Load from db8 and present.
	open: function() {
		this.load();
		this.show();
	},
	load: function() {
		var self = this;
		enyo.jihad.dbFind(enyo.jihad.kinds.bookmarks, null, "date", true, function(results) {
			self.setItems(results || []);
		});
	},
	itemsChanged: function() {
		this.renderList();
	},
	renderList: function() {
		this.$.list.setCount(this.items.length);
		this.$.empty.setShowing(this.items.length === 0);
	},
	setupRow: function(inSender, inEvent) {
		var it = this.items[inEvent.index];
		if (!it) {
			return true;
		}
		inEvent.item.$.title.setContent(it.title || it.url || "");
		inEvent.item.$.url.setContent(it.url || "");
		return true;
	},
	rowTapped: function(inSender, inEvent) {
		var it = this.items[inEvent.index];
		if (it && it.url) {
			this.doSelectItem({url: it.url, item: it});
		}
	},
	deleteTapped: function(inSender, inEvent) {
		var i = inEvent.index, it = this.items[i];
		if (it) {
			this.doDeleteBookmark({item: it});
			if (it._id) {
				enyo.jihad.dbDel([it._id]);
			}
			this.items.splice(i, 1);
			this.renderList();
		}
		return true;
	},
	addTapped: function() {
		this.doAddBookmark();
	},
	closeTapped: function() {
		this.hide();
		this.doClose();
	}
});
