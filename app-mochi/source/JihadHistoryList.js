// Copyright 2026 the Jihad Browser project.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadHistoryList — the Enyo 2 / Mochi port of ../../app/source/HistoryList.js.
// A full-card overlay panel listing browse history (most-recent first) from the
// SAME db8 kind the Enyo 1.0 app uses
// (net.riverstonerelay.jihad-browser.history:1, via enyo.jihad.dbFind ordered by
// `date` desc). A tap selects an entry (doSelectItem -> shell navigates); a row
// Delete removes it; the header Clear empties the kind (doClearHistory + db8
// delByQuery). The Enyo 1.0 relative-date dividers used enyo.g11n (not in the
// Enyo 2 stack) — omitted here in favour of an inline date label; noted in
// PARITY.md as an intentional cosmetic simplification.

enyo.kind({
	name: "JihadHistoryList",
	kind: "FittableRows",
	classes: "jihad-panel enyo-fit",
	showing: false,
	published: {
		items: []
	},
	events: {
		onSelectItem: "",
		onDeleteHistory: "",
		onClearHistory: "",
		onClose: ""
	},
	components: [
		{kind: "FittableColumns", classes: "jihad-panel-header", components: [
			{name: "doneBtn", classes: "jihad-panel-btn jihad-panel-done", ontap: "closeTapped", content: "Done"},
			{classes: "jihad-panel-title", fit: true, content: "History"},
			{name: "clearBtn", classes: "jihad-panel-btn jihad-panel-action", ontap: "clearTapped", content: "Clear"}
		]},
		{kind: "enyo.Scroller", fit: true, touch: true, classes: "jihad-panel-scroller", components: [
			{name: "list", kind: "enyo.Repeater", onSetupItem: "setupRow", components: [
				{name: "row", classes: "jihad-row", ontap: "rowTapped", components: [
					{kind: "FittableColumns", components: [
						{fit: true, components: [
							{name: "title", classes: "jihad-row-title"},
							{name: "url", classes: "jihad-row-url"}
						]},
						{name: "date", classes: "jihad-row-date"},
						{name: "del", classes: "jihad-row-del", ontap: "deleteTapped", content: "Delete"}
					]}
				]}
			]},
			{name: "empty", classes: "jihad-empty", content: "No history yet.", showing: false}
		]}
	],
	open: function() {
		this.load();
		this.show();
	},
	load: function() {
		var self = this;
		enyo.jihad.dbFind(enyo.jihad.kinds.history, null, "date", true, function(results) {
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
	//* Short local date label for a history record's `date` (epoch ms).
	formatDate: function(ms) {
		if (!ms) {
			return "";
		}
		var d = new Date(ms);
		if (isNaN(d.getTime())) {
			return "";
		}
		return (d.getMonth() + 1) + "/" + d.getDate() + "/" + String(d.getFullYear()).slice(2);
	},
	setupRow: function(inSender, inEvent) {
		var it = this.items[inEvent.index];
		if (!it) {
			return true;
		}
		inEvent.item.$.title.setContent(it.title || it.url || "");
		inEvent.item.$.url.setContent(it.url || "");
		inEvent.item.$.date.setContent(this.formatDate(it.date));
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
			this.doDeleteHistory({item: it});
			if (it._id) {
				enyo.jihad.dbDel([it._id]);
			}
			this.items.splice(i, 1);
			this.renderList();
		}
		return true;
	},
	clearTapped: function() {
		// The shell (onClearHistory) owns the db8 delByQuery; just clear the view.
		this.doClearHistory();
		this.setItems([]);
	},
	closeTapped: function() {
		this.hide();
		this.doClose();
	}
});
