// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadDownloadList — the Enyo 2 / Mochi port of ../../app/source/DownloadList.js.
// A full-card overlay panel listing downloads. The shell (JihadBrowser) owns the
// downloads array (session downloads it started via palm://com.palm.downloadmanager
// plus getAllHistory results) and hands it here via setDownloads(); this view just
// renders it and emits intent events. A completed row Opens (doOpenItem -> shell
// launches it), an in-progress download-manager row Cancels (doCancelItem), a failed
// row Retries (doRetryItem), and the header Clear empties the list (doClearAll).
//
// TWO KINDS OF RECORD arrive here. Download-manager records (a `ticket`, `destPath` +
// `destFile`, `amountReceived`/`amountTotal`) come from files the card asked the system
// to fetch, e.g. an image saved from the long-press menu. ENGINE records (`engine: true`,
// `url`, `path`, `received`/`total`, `failed`) come from a download the page itself
// started: the engine saves such a file and reports its progress over the adapter's
// download callbacks. An engine download has no cancel — the adapter has no command for
// it — so its row offers none rather than a button that does nothing.

enyo.kind({
	name: "JihadDownloadList",
	kind: "FittableRows",
	classes: "jihad-panel enyo-fit",
	showing: false,
	published: {
		//* Download records (as returned by the download manager, mixed with the
		//* {url, mimetype} stubs the shell seeds when a download starts).
		downloads: []
	},
	events: {
		onOpenItem: "",
		onCancelItem: "",
		//* {index, item} — fetch a failed download again.
		onRetryItem: "",
		onClearAll: "",
		onClose: ""
	},
	components: [
		{kind: "FittableColumns", classes: "jihad-panel-header", components: [
			{name: "doneBtn", classes: "jihad-panel-btn jihad-panel-done", ontap: "closeTapped", content: "Done"},
			{classes: "jihad-panel-title", fit: true, content: "Downloads"},
			{name: "clearBtn", classes: "jihad-panel-btn jihad-panel-action", ontap: "clearTapped", content: "Clear"}
		]},
		{kind: "enyo.Scroller", fit: true, touch: true, classes: "jihad-panel-scroller", components: [
			{name: "list", kind: "enyo.Repeater", onSetupItem: "setupRow", components: [
				{name: "row", classes: "jihad-row", components: [
					{kind: "FittableColumns", components: [
						{fit: true, components: [
							{name: "filename", classes: "jihad-row-title"},
							{name: "status", classes: "jihad-row-url"}
						]},
						{name: "open", classes: "jihad-row-act", ontap: "openTapped", content: "Open", showing: false},
						{name: "retry", classes: "jihad-row-act", ontap: "retryTapped", content: "Retry", showing: false},
						{name: "cancel", classes: "jihad-row-del", ontap: "cancelTapped", content: "Cancel", showing: false}
					]}
				]}
			]},
			{name: "empty", classes: "jihad-empty", content: "No downloads.", showing: false}
		]}
	],
	create: function() {
		this.inherited(arguments);
		// `published` defaults live on the PROTOTYPE — give each instance its own
		// array rather than sharing the literal [] above.
		this.downloads = this.downloads ? this.downloads.slice() : [];
	},
	open: function() {
		this.show();
		this.resize();      // see JihadBookmarkList.open
	},
	downloadsChanged: function() {
		this.renderList();
	},
	renderList: function() {
		var n = (this.downloads && this.downloads.length) || 0;
		this.$.list.setCount(n);
		this.$.empty.setShowing(n === 0);
		this.$.clearBtn.setShowing(n > 0);
	},
	//* Derive a display filename from a download record (target/url/destFile).
	fileNameOf: function(d) {
		if (d.destFile) {
			return String(d.destFile).replace(/%20/g, " ");
		}
		var c = d.path || d.target || d.url || "";
		c = c.split("#")[0].split("?")[0];
		var s = c.lastIndexOf("/") + 1;
		return c.substring(s).replace(/%20/g, " ") || c;
	},
	//* One of: "downloading NN%", "completed", "failed", or "queued".
	statusOf: function(d) {
		if (this.isFailed(d)) {
			return d.error ? ("failed: " + d.error) : "failed";
		}
		if (d.completed) {
			return "completed";
		}
		if (d.engine) {
			return d.total > 0 ? ("downloading " + Math.round((d.received || 0) * 100 / d.total) + "%") : "downloading";
		}
		if (d.ticket && d.amountTotal) {
			var pct = Math.round((d.amountReceived || 0) * 100 / d.amountTotal);
			return "downloading " + pct + "%";
		}
		return "queued";
	},
	isFailed: function(d) {
		return Boolean(d.failed || d.aborted || (d.completed && d.completionStatusCode && d.completionStatusCode != 200));
	},
	isComplete: function(d) {
		return Boolean(d.completed && !this.isFailed(d));
	},
	isActive: function(d) {
		return Boolean(d.ticket && !d.completed && !d.aborted);
	},
	setupRow: function(inSender, inEvent) {
		var d = this.downloads[inEvent.index];
		if (!d) {
			return true;
		}
		var it = inEvent.item;
		it.$.filename.setContent(this.fileNameOf(d));
		it.$.status.setContent(this.statusOf(d));
		it.$.open.setShowing(this.isComplete(d));
		it.$.retry.setShowing(this.isFailed(d) && Boolean(d.url || d.target));
		it.$.cancel.setShowing(this.isActive(d));
		return true;
	},
	openTapped: function(inSender, inEvent) {
		this.doOpenItem({index: inEvent.index, item: this.downloads[inEvent.index]});
		return true;
	},
	retryTapped: function(inSender, inEvent) {
		this.doRetryItem({index: inEvent.index, item: this.downloads[inEvent.index]});
		return true;
	},
	cancelTapped: function(inSender, inEvent) {
		this.doCancelItem({index: inEvent.index, item: this.downloads[inEvent.index]});
		return true;
	},
	clearTapped: function() {
		this.doClearAll();
	},
	closeTapped: function() {
		this.hide();
		this.doClose();
	}
});
