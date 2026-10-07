// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// Jihad Browser (Mojo) — Print.
//
// The Mojo face of the browser's own printing (render/goanna/components/jihadPrint.js: the
// daemon finds IPP printers by mDNS or a typed address and prints the page as JPEG sheets),
// as the Enyo and Mochi shells' Print dialogs are (app-mochi/source/JihadPrintDialog.js is
// the model for this one). webOS's own print stack keeps HP printers only, so it is not used.
//
//   on open    ask the daemon to look (findPrinters); printers are listed as each answers
//   a row      print the page on that printer (printPage)
//   an address a printer discovery cannot see, by its address on this network
//
// What the daemon says arrives on the card's notification channel under "print-…"
// categories; the main scene hands them here while this scene is up (notification()).
function PrintAssistant(args) {
	this.main = args && args.main;
	this.printers = [];
	this.printing = false;
}

PrintAssistant.prototype.setup = function () {
	this.listModel = {items: []};
	this.controller.setupWidget("jihad-print-list", {
		itemTemplate: "print/print-row",
		listTemplate: "print/print-container",
		emptyTemplate: "print/print-empty",
		swipeToDelete: false,
		reorderable: false
	}, this.listModel);
	this.controller.setupWidget("jihad-print-address", {
		hintText: $L("Printer address, e.g. 192.168.1.50"),
		autoReplace: false, textCase: Mojo.Widget.steModeLowerCase, enterSubmits: true
	}, this.addressModel = {value: ""});
	this.controller.setupWidget("jihad-print-search", {type: Mojo.Widget.defaultButton},
		this.searchModel = {buttonLabel: $L("Search Again"), disabled: false});
	this.controller.setupWidget("jihad-print-close", {type: Mojo.Widget.defaultButton},
		{buttonLabel: $L("Close")});

	// The address is for the rare printer discovery cannot see: no focus (and no keyboard
	// over the list) until the user taps the field.
	this.controller.setInitialFocusedElement(null);

	this.handleTap = this.handleTap.bind(this);
	this.search = this.search.bind(this);
	this.close = this.close.bind(this);
	this.handleAddressKey = this.handleAddressKey.bind(this);
	Mojo.Event.listen(this.controller.get("jihad-print-list"), Mojo.Event.listTap, this.handleTap);
	Mojo.Event.listen(this.controller.get("jihad-print-search"), Mojo.Event.tap, this.search);
	Mojo.Event.listen(this.controller.get("jihad-print-close"), Mojo.Event.tap, this.close);
	Mojo.Event.listen(this.controller.get("jihad-print-address"), Mojo.Event.propertyChange, this.handleAddressKey);
};

PrintAssistant.prototype.cleanup = function () {
	Mojo.Event.stopListening(this.controller.get("jihad-print-list"), Mojo.Event.listTap, this.handleTap);
	Mojo.Event.stopListening(this.controller.get("jihad-print-search"), Mojo.Event.tap, this.search);
	Mojo.Event.stopListening(this.controller.get("jihad-print-close"), Mojo.Event.tap, this.close);
	Mojo.Event.stopListening(this.controller.get("jihad-print-address"), Mojo.Event.propertyChange, this.handleAddressKey);
	if (PrintAssistant.current === this) {
		PrintAssistant.current = null;
	}
};

PrintAssistant.prototype.activate = function () {
	PrintAssistant.current = this;
	if (!this.started) {
		this.started = true;
		this.search();
	}
};

PrintAssistant.prototype.status = function (text) {
	var el = this.controller.get("jihad-print-status");
	if (el) {
		el.innerText = String(text || "");
	}
};

//* Enter in the address field searches, like the Search button.
PrintAssistant.prototype.handleAddressKey = function (event) {
	if (event && event.originalEvent && event.originalEvent.keyCode === Mojo.Char.enter) {
		this.search();
	}
};

PrintAssistant.prototype.search = function () {
	if (this.printing) {
		return;
	}
	var address = String(this.addressModel.value || "").replace(/^\s+|\s+$/g, "");
	this.printers = [];
	this.render();
	this.status($L("Looking for printers…"));
	if (this.main) {
		this.main.findPrinters(address);
	}
};

PrintAssistant.prototype.render = function () {
	var items = this.listModel.items, i, p, where, sub;
	items.splice(0, items.length);
	for (i = 0; i < this.printers.length; i++) {
		p = this.printers[i];
		where = p.address + (p.port && p.port !== 631 ? ":" + p.port : "");
		sub = (p.model && p.model !== p.name ? p.model + " · " : "") + where;
		items.push({index: i, nameText: HistoryAssistant.escape(p.name || where), subText: HistoryAssistant.escape(sub)});
	}
	this.controller.modelChanged(this.listModel);
};

PrintAssistant.prototype.handleTap = function (event) {
	var item = event && event.item, p = item && this.printers[item.index];
	if (!p || this.printing) {
		return;
	}
	this.printing = true;
	this.printers = [];
	this.render();
	this.status($L("Sending to ") + (p.name || p.address) + "…");
	if (this.main) {
		this.main.printPage(p);
	}
};

PrintAssistant.prototype.close = function () {
	this.controller.stageController.popScene();
};

//* One line from the daemon; `category` starts with "print-".
PrintAssistant.prototype.notification = function (category, text) {
	if (category === "print-printers") {
		if (this.printing) {
			return;
		}
		var r = null;
		try { r = JSON.parse(text); } catch (e) { r = null; }
		this.printers = (r && r.printers) || [];
		this.render();
		if (r && r.done) {
			this.status(this.printers.length ? $L("Choose a printer.") :
				$L("No printer answered. If yours is on, type its address below and search again."));
		} else {
			this.status(this.printers.length ? $L("Choose a printer. Still looking…") : $L("Looking for printers…"));
		}
	} else if (category === "print-progress") {
		this.status(text);
	} else if (category === "print-done" || category === "print-error") {
		this.printing = false;
		this.status(text);
	}
};
