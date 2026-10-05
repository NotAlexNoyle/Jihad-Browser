// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadPrintDialog — Print for the Mochi shell (../../app/source/BrowserApp.js printClick /
// PrintDialog parity, by another road).
//
// The Enyo shell prints through webOS's own print dialog, and that stack accepts HP printers
// only: it finds any printer on the network and throws away every one that is not HP
// (measured 2026-10-05, printmgrd: "not HP printer and doing printer filtering"). So this
// dialog does not use it. The engine daemon finds printers itself and prints over IPP
// (render/goanna/components/jihadPrint.js); this is its face:
//
//   open        ask the daemon to look (findPrinters); printers are listed as each one answers
//   a row       print the page on that printer (printPage)
//   an address  a printer discovery cannot see, by its address on this network
//
// What the daemon has to say arrives on the card's notification channel under categories that
// start with "print-"; the shell hands those to notification() instead of raising a toast.
// An overlay, like every dialog in this shell (mochi.Popup crashes the card on this engine).

enyo.kind({
	name: "JihadPrintDialog",
	classes: "jihad-modal-overlay",
	showing: false,
	events: {
		//* Look for printers. {address} when the user typed one.
		onFindPrinters: "",
		//* Print the page on screen. {printer} is a row of the daemon's list, passed back as is.
		onPrintPage: ""
	},
	components: [
		{classes: "jihad-dialog-box jihad-print", components: [
			{classes: "jihad-dialog-title", content: "Print"},
			{name: "status", classes: "jihad-sheet-sub"},
			{name: "printers", classes: "jihad-sheet-items"},
			{name: "manual", components: [
				{kind: "mochi.InputDecorator", classes: "jihad-dialog-inputbox", components: [
					{kind: "mochi.Input", name: "address", placeholder: "Printer address, e.g. 192.168.1.50",
						classes: "jihad-dialog-input", onkeydown: "addressKeydown",
						attributes: {autocapitalize: "off", autocorrect: "off", spellcheck: "false"}}
				]}
			]},
			{classes: "jihad-dialog-buttons", components: [
				{name: "searchBtn", classes: "jihad-btn jihad-btn-alt", content: "Search Again", ontap: "search"},
				{classes: "jihad-btn", content: "Close", ontap: "close"}
			]}
		]}
	],
	create: function() {
		this.inherited(arguments);
		this._printers = [];
		this._printing = false;
	},
	//* Open on a fresh search.
	open: function() {
		this._printing = false;
		this.show();
		this.search();
	},
	close: function() {
		this.hide();
		return true;
	},
	//* True while the dialog is the place to show what the daemon says about printing.
	isOpen: function() { return this.getShowing(); },

	search: function() {
		if (this._printing) { return true; }
		var address = String(this.$.address.getValue() || "").replace(/^\s+|\s+$/g, "");
		this._printers = [];
		this.renderPrinters();
		this.$.status.setContent("Looking for printers…");
		this.$.manual.show();
		this.$.searchBtn.show();
		this.doFindPrinters(address ? {address: address} : {});
		return true;
	},
	addressKeydown: function(inSender, inEvent) {
		if (inEvent && inEvent.keyCode === 13) { this.search(); return true; }
	},

	renderPrinters: function() {
		this.$.printers.destroyClientControls();
		for (var i = 0; i < this._printers.length; i++) {
			var p = this._printers[i];
			// Second line: what tells two entries apart — the model when the name is not it,
			// and where the printer is.
			var where = p.address + (p.port && p.port !== 631 ? ":" + p.port : "");
			var sub = (p.model && p.model !== p.name ? p.model + " · " : "") + where;
			this.$.printers.createComponent({
				classes: "jihad-menu-item jihad-print-row", printerIndex: i, ontap: "pick", components: [
					{classes: "jihad-print-name", content: p.name || where, allowHtml: false},
					{classes: "jihad-print-sub", content: sub, allowHtml: false}
				]
			}, {owner: this});
		}
		this.$.printers.render();
	},
	pick: function(inSender) {
		// The tap may land on a line inside the row; the row is the one carrying the index.
		var row = inSender;
		while (row && row.printerIndex === undefined) { row = row.parent; }
		var p = row && this._printers[row.printerIndex];
		if (!p || this._printing) { return true; }
		this._printing = true;
		this.$.printers.destroyClientControls();
		this.$.printers.render();
		this.$.manual.hide();
		this.$.searchBtn.hide();
		this.$.status.setContent("Sending to " + (p.name || p.address) + "…");
		this.doPrintPage({printer: p});
		return true;
	},

	//* One line from the daemon. `category` starts with "print-".
	notification: function(category, text) {
		if (category === "print-printers") {
			if (this._printing) { return; }
			var r = null;
			try { r = enyo.json.parse(text); } catch (e) { r = null; }
			var list = (r && r.printers) || [];
			this._printers = list;
			this.renderPrinters();
			if (r && r.done) {
				this.$.status.setContent(list.length ? "Choose a printer." :
					"No printer answered. If yours is on, type its address below and search again.");
			} else {
				this.$.status.setContent(list.length ? "Choose a printer. Still looking…" : "Looking for printers…");
			}
		} else if (category === "print-progress") {
			this.$.status.setContent(text);
		} else if (category === "print-done" || category === "print-error") {
			this._printing = false;
			this.$.status.setContent(text);
			this.$.searchBtn.show();
		}
	}
});
