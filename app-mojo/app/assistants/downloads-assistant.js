// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// Jihad Browser (Mojo) — downloads scene.
//
// The files the engine saved for this card (JihadDownloads; see that model for where the
// records come from). A finished download opens with whatever app handles it (the system's
// applicationManager open, as the Enyo shell's download list does); one in progress shows
// how far it has got; a failed one says why. Clear List empties the list (the files stay).
function DownloadsAssistant() {}

DownloadsAssistant.prototype.setup = function () {
	this.items = DownloadsAssistant.rows();
	this.controller.setupWidget("jihad-downloads-list", {
		itemTemplate: "downloads/downloads-row",
		listTemplate: "downloads/downloads-container",
		emptyTemplate: "downloads/downloads-empty",
		swipeToDelete: false,
		reorderable: false
	}, this.listModel = {items: this.items});
	this.controller.setupWidget("jihad-downloads-clear",
		{type: Mojo.Widget.defaultButton},
		{buttonLabel: $L("Clear List"), disabled: this.items.length === 0});

	this.handleTap = this.handleTap.bind(this);
	this.handleClear = this.handleClear.bind(this);
	Mojo.Event.listen(this.controller.get("jihad-downloads-list"), Mojo.Event.listTap, this.handleTap);
	Mojo.Event.listen(this.controller.get("jihad-downloads-clear"), Mojo.Event.tap, this.handleClear);
};

DownloadsAssistant.prototype.cleanup = function () {
	Mojo.Event.stopListening(this.controller.get("jihad-downloads-list"), Mojo.Event.listTap, this.handleTap);
	Mojo.Event.stopListening(this.controller.get("jihad-downloads-clear"), Mojo.Event.tap, this.handleClear);
};

//* The list rows, from the store.
DownloadsAssistant.rows = function () {
	return JihadDownloads.all().map(function (d) {
		var state;
		if (d.state === "done") {
			state = $L("Done") + (d.path ? " — " + d.path : "");
		} else if (d.state === "failed") {
			state = $L("Failed") + (d.message ? ": " + d.message : "");
		} else {
			state = (d.total > 0) ? $L("Downloading") + " " + Math.floor(100 * d.received / d.total) + "%" :
				$L("Downloading") + "…";
		}
		return {path: d.path, state: d.state, url: d.url,
			nameText: HistoryAssistant.escape(d.name || d.url), stateText: HistoryAssistant.escape(state)};
	});
};

//* A newer record arrived (the main scene calls this while the scene is up).
DownloadsAssistant.prototype.refresh = function () {
	this.items.splice(0, this.items.length);
	Array.prototype.push.apply(this.items, DownloadsAssistant.rows());
	this.controller.modelChanged(this.listModel);
};

DownloadsAssistant.prototype.activate = function () {
	DownloadsAssistant.current = this;
};

DownloadsAssistant.prototype.deactivate = function () {
	if (DownloadsAssistant.current === this) {
		DownloadsAssistant.current = null;
	}
};

DownloadsAssistant.prototype.handleTap = function (event) {
	var item = event && event.item;
	if (!item || item.state !== "done" || !item.path) {
		return;
	}
	this.controller.serviceRequest("palm://com.palm.applicationManager", {
		method: "open",
		parameters: {target: "file://" + item.path},
		onFailure: function () {
			JihadToast.show("info", $L("No app on this device opens that file."));
		}
	});
};

DownloadsAssistant.prototype.handleClear = function () {
	JihadDownloads.clear();
	this.controller.stageController.popScene();
};
