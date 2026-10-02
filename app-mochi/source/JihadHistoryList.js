// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadHistoryList — the Enyo 2 / Mochi port of ../../app/source/HistoryList.js.
// A full-card overlay panel listing browse history (most-recent first) from the
// SAME db8 kind the Enyo 1.0 app uses
// (net.riverstonerelay.jihad-browser-mochi.history:1, via enyo.jihad.dbFind ordered by
// `date` desc). A tap selects an entry (doSelectItem -> shell navigates); a row
// Delete removes it; the header Clear empties the kind (doClearHistory + db8
// delByQuery), after a confirmation. Entries are grouped under date headings —
// Today, Yesterday, a weekday name, Last Week / N Weeks Ago, a month name, a year —
// the same buckets as HistoryList.getDivider. The Enyo view builds its labels with
// enyo.g11n, which the Enyo 2 stack does not have, so the labels are built here; and
// its week bucket never appears (it multiplies by a misspelt `dayinMilli`, which is
// undefined), which is a bug and is not reproduced.

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
				// The heading of a date group, shown on the first entry of each group.
				{name: "divider", classes: "jihad-row-divider", showing: false},
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
		]},
		// Confirmation before Clear (its own overlay — see JihadPreferences for why the
		// engine-dialog confirm cannot be reused).
		{name: "ask", classes: "jihad-modal-overlay", showing: false, components: [
			{classes: "jihad-dialog-box", components: [
				{classes: "jihad-dialog-message", content: "Would you like to clear your browser history?"},
				{classes: "jihad-dialog-buttons", components: [
					{classes: "jihad-btn jihad-btn-alt", content: "Cancel", ontap: "askCancel"},
					{classes: "jihad-btn", content: "Clear", ontap: "askOk"}
				]}
			]}
		]}
	],
	weekdays: ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"],
	months: ["January", "February", "March", "April", "May", "June", "July", "August",
		"September", "October", "November", "December"],
	create: function() {
		this.inherited(arguments);
		// `published` defaults live on the PROTOTYPE, so the literal [] above is one
		// array shared by every instance — and deleteTapped splices `items` in
		// place. Give each instance its own copy.
		this.items = this.items ? this.items.slice() : [];
	},
	open: function() {
		this.load();
		this.show();
		this.resize();      // see JihadBookmarkList.open
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
		this._renderNow = (new Date()).getTime();
		this.$.list.setCount(this.items.length);
		this.$.empty.setShowing(this.items.length === 0);
	},
	//* Time of day for a record's `date` (epoch ms) — the day is in the group heading.
	formatDate: function(ms) {
		if (!ms) {
			return "";
		}
		var d = new Date(ms);
		if (isNaN(d.getTime())) {
			return "";
		}
		var h = d.getHours(), m = d.getMinutes();
		return ((h % 12) || 12) + ":" + (m < 10 ? "0" : "") + m + (h < 12 ? " am" : " pm");
	},
	//* The heading a record belongs under, relative to `nowMs` (injectable so the buckets
	//* can be checked without waiting for a date). Days are CALENDAR days, compared at local
	//* midnight, so an entry from 23:50 yesterday is "Yesterday" ten minutes later.
	groupLabel: function(ms, nowMs) {
		var d = new Date(ms);
		if (!ms || isNaN(d.getTime())) {
			return "Earlier";
		}
		var now = new Date(nowMs === undefined ? (new Date()).getTime() : nowMs);
		var day0 = new Date(now.getFullYear(), now.getMonth(), now.getDate()).getTime();
		var dDay = new Date(d.getFullYear(), d.getMonth(), d.getDate()).getTime();
		var days = Math.round((day0 - dDay) / 86400000);
		if (days <= 0) { return "Today"; }
		if (days === 1) { return "Yesterday"; }
		if (days < 7) { return this.weekdays[d.getDay()]; }
		if (days < 14) { return "Last Week"; }
		if (days < 31) { return Math.floor(days / 7) + " Weeks Ago"; }
		if (d.getFullYear() === now.getFullYear()) { return this.months[d.getMonth()]; }
		if (days < 365) { return this.months[d.getMonth()] + " " + d.getFullYear(); }
		return String(d.getFullYear());
	},
	setupRow: function(inSender, inEvent) {
		var it = this.items[inEvent.index];
		if (!it) {
			return true;
		}
		inEvent.item.$.title.setContent(it.title || it.url || "");
		inEvent.item.$.url.setContent(it.url || "");
		inEvent.item.$.date.setContent(this.formatDate(it.date));
		// A heading where the group changes. Items are newest first, so comparing with the
		// previous item is enough; `now` is fixed for one render pass so a list drawn across
		// midnight cannot head two groups "Today".
		var label = this.groupLabel(it.date, this._renderNow);
		var prev = inEvent.index > 0 ? this.items[inEvent.index - 1] : null;
		var first = !prev || this.groupLabel(prev.date, this._renderNow) !== label;
		inEvent.item.$.divider.setShowing(first);
		if (first) { inEvent.item.$.divider.setContent(label); }
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
		if (this.items.length) { this.$.ask.show(); }
		return true;
	},
	askCancel: function() {
		this.$.ask.hide();
		return true;
	},
	askOk: function() {
		this.$.ask.hide();
		// The shell (onClearHistory) owns the db8 delByQuery and the engine's own history;
		// just clear the view.
		this.doClearHistory();
		this.setItems([]);
		return true;
	},
	closeTapped: function() {
		this.$.ask.hide();
		this.hide();
		this.doClose();
	}
});
