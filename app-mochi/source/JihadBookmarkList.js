// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadBookmarkList — the Enyo 2 / Mochi port of ../../app/source/BookmarkList.js.
// A full-card overlay panel (enyo-fit, toggled by showing) listing the saved
// bookmarks from this package's own db8 kind
// (net.riverstonerelay.jihad-browser-mochi.bookmarks:1, via enyo.jihad.dbFind). A tap
// selects a bookmark (doSelectItem -> shell navigates); a row Edit opens the edit
// dialog (title + address, saved with a db8 merge — BookmarkDialog.js parity); a row
// Delete removes it (doDeleteBookmark + db8 del); the header + button adds the current
// page (doAddBookmark -> shell). A row shows the page thumbnail when the record has
// one. enyo.Repeater (not enyo.List) is used because these lists are small and the
// flyweight virtual list is fragile on this engine.

enyo.kind({
	name: "JihadBookmarkList",
	kind: "FittableRows",
	classes: "jihad-panel enyo-fit",
	showing: false,
	published: {
		//* Bookmark records ({_id, title, url, thumbnailFile?, ...}) currently displayed.
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
						// The image sits in a fixed box: a bare <img> as a fittable column is
						// stretched to the row's height by the layout (seen on the device).
						{name: "thumbBox", classes: "jihad-row-thumbbox", showing: false, components: [
							{name: "thumb", tag: "img", classes: "jihad-row-thumb"}
						]},
						{fit: true, components: [
							{name: "title", classes: "jihad-row-title"},
							{name: "url", classes: "jihad-row-url"}
						]},
						{name: "edit", classes: "jihad-row-act", ontap: "editTapped", content: "Edit"},
						{name: "del", classes: "jihad-row-del", ontap: "deleteTapped", content: "Delete"}
					]}
				]}
			]},
			{name: "empty", classes: "jihad-empty", content: "No bookmarks yet.", showing: false}
		]},
		// Edit dialog (an overlay, like every dialog in this shell: mochi.Popup crashes the
		// card on this engine).
		{name: "editor", classes: "jihad-modal-overlay", showing: false, components: [
			{classes: "jihad-dialog-box", components: [
				{classes: "jihad-dialog-title", content: "Edit Bookmark"},
				{kind: "mochi.InputDecorator", classes: "jihad-dialog-inputbox", components: [
					{kind: "mochi.Input", name: "editTitle", placeholder: "Title", classes: "jihad-dialog-input",
						attributes: {autocorrect: "off", spellcheck: "false"}}
				]},
				{kind: "mochi.InputDecorator", classes: "jihad-dialog-inputbox", components: [
					{kind: "mochi.Input", name: "editUrl", placeholder: "Address", classes: "jihad-dialog-input",
						attributes: {autocapitalize: "off", autocorrect: "off", spellcheck: "false", type: "url"}}
				]},
				{name: "editError", classes: "jihad-prefs-note", showing: false},
				{classes: "jihad-dialog-buttons", components: [
					{classes: "jihad-btn jihad-btn-alt", content: "Cancel", ontap: "editCancel"},
					{classes: "jihad-btn", content: "Save", ontap: "editSave"}
				]}
			]}
		]}
	],
	create: function() {
		this.inherited(arguments);
		// `published` defaults live on the PROTOTYPE, so the literal [] above is one
		// array shared by every instance — and deleteTapped splices `items` in
		// place. Give each instance its own copy.
		this.items = this.items ? this.items.slice() : [];
		this._editing = -1;
	},
	//* Load from db8 and present.
	open: function() {
		this.load();
		this.show();
		// A panel that was hidden when the app was laid out has never had its fittable
		// rows and columns measured; without this the header packs to the left and the
		// scroller gets no height, so the list is there but nothing of it is visible.
		// (`resize()` is this Enyo's name for it; `resized()` does not exist here and throws.)
		this.resize();
	},
	load: function() {
		var self = this;
		// Newest first by REVISION, not by `date`: the bookmarks kind has no index on `date`,
		// and db8 refuses an ordered query it has no index for ("db: no index for query",
		// measured on the device 2026-10-02) — so ordering by date returned nothing at all
		// and this list was always empty. `_rev` is indexed and grows with every put/merge.
		enyo.jihad.dbFind(enyo.jihad.kinds.bookmarks, null, "_rev", true, function(results) {
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
		var thumb = it.thumbnailFile || "";
		inEvent.item.$.thumbBox.setShowing(Boolean(thumb));
		if (thumb) { inEvent.item.$.thumb.setAttribute("src", "file://" + thumb); }
		return true;
	},
	rowTapped: function(inSender, inEvent) {
		var it = this.items[inEvent.index];
		if (it && it.url) {
			this.doSelectItem({url: it.url, item: it});
		}
	},

	// --- edit -----------------------------------------------------------------
	editTapped: function(inSender, inEvent) {
		var it = this.items[inEvent.index];
		if (!it) { return true; }
		this._editing = inEvent.index;
		this.$.editTitle.setValue(it.title || "");
		this.$.editUrl.setValue(it.url || "");
		this.$.editError.hide();
		this.$.editor.show();
		return true;                      // not a row tap
	},
	editCancel: function() {
		this._editing = -1;
		this.$.editor.hide();
		return true;
	},
	//* Save the edited title and address. Both are stored AS TYPED: the Enyo dialog runs
	//* them through an HTML escape first, which turns `?a=1&b=2` into `&amp;` in the stored
	//* address — that is a bug in it, not a behaviour to keep. Rows are filled with
	//* setContent on a control whose allowHtml is off, so nothing needs escaping to be safe.
	editSave: function() {
		var i = this._editing, it = this.items[i];
		if (!it) { return this.editCancel(); }
		var title = String(this.$.editTitle.getValue() || "").replace(/^\s+|\s+$/g, "");
		var url = String(this.$.editUrl.getValue() || "").replace(/^\s+|\s+$/g, "");
		if (!url) {
			this.$.editError.setContent("A bookmark needs an address.");
			this.$.editError.show();
			return true;
		}
		// Same rule the address bar applies to a bare host, so "example.com" stays loadable.
		if (!/^[a-z][a-z0-9+.\-]*:/i.test(url)) { url = "http://" + url; }
		it.title = title || url;
		it.url = url;
		if (it._id) {
			enyo.jihad.dbMerge([{_id: it._id, title: it.title, url: it.url, date: (new Date()).getTime()}]);
		}
		this._editing = -1;
		this.$.editor.hide();
		this.renderList();
		return true;
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
		this.editCancel();
		this.hide();
		this.doClose();
	}
});
