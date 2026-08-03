// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// Jihad Browser (Mojo) — history scene.
//
// Pushed by the main scene's command-menu history button. A Mojo List over the
// JihadHistory store (see ../models/jihad-history.js): tapping an entry pops back
// to the browser and opens it, the Clear button empties the store.
//
// Titles and urls come from loaded PAGES, so they are rendered through the list's
// own text substitution with html-escaped values — never as markup.
function HistoryAssistant() {}

HistoryAssistant.prototype.setup = function () {
	this.items = JihadHistory.all().map(function (entry) {
		return {
			url: entry.url,
			titleText: HistoryAssistant.escape(entry.title || entry.url),
			urlText: HistoryAssistant.escape(entry.url)
		};
	});
	this.controller.setupWidget("jihad-history-list", {
		itemTemplate: "history/history-row",
		listTemplate: "history/history-container",
		emptyTemplate: "history/history-empty",
		swipeToDelete: false,
		reorderable: false
	}, {items: this.items});

	this.controller.setupWidget("jihad-history-clear",
		{type: Mojo.Widget.defaultButton},
		{buttonLabel: $L("Clear History"), disabled: this.items.length === 0});

	this.handleTap = this.handleTap.bind(this);
	this.handleClear = this.handleClear.bind(this);
	Mojo.Event.listen(this.controller.get("jihad-history-list"), Mojo.Event.listTap, this.handleTap);
	Mojo.Event.listen(this.controller.get("jihad-history-clear"), Mojo.Event.tap, this.handleClear);
};

HistoryAssistant.prototype.cleanup = function () {
	Mojo.Event.stopListening(this.controller.get("jihad-history-list"), Mojo.Event.listTap, this.handleTap);
	Mojo.Event.stopListening(this.controller.get("jihad-history-clear"), Mojo.Event.tap, this.handleClear);
};

//* Open the tapped entry in the browser scene we came from (pop, don't stack a
//* second browser: the card would otherwise grow a scene per history visit).
HistoryAssistant.prototype.handleTap = function (event) {
	var item = event && event.item;
	if (!item || !item.url) {
		return;
	}
	this.controller.stageController.popScene({url: item.url});
};

HistoryAssistant.prototype.handleClear = function () {
	JihadHistory.clear();
	this.items.length = 0;
	this.controller.modelChanged({items: this.items});
	this.controller.stageController.popScene();
};

//* Escape for the list template (the row template inserts these as html).
HistoryAssistant.escape = function (text) {
	return String(text === undefined || text === null ? "" : text)
		.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;")
		.replace(/"/g, "&quot;");
};
