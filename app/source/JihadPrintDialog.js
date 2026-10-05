// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadPrintDialog — Print for the Enyo shell.
//
// Upstream printed through webOS's own print dialog ($enyo-lib/printdialog) and the frozen
// printFrame command. That stack accepts HP printers only: its daemon finds every printer on
// the network and discards each one that is not HP (measured 2026-10-05, printmgrd: "not HP
// printer and doing printer filtering" about an Epson it had just resolved). So the engine
// daemon finds printers itself and prints over IPP (render/goanna/components/jihadPrint.js),
// and this dialog is its face — the same one, in the same words, as the Mochi shell's
// (../../app-mochi/source/JihadPrintDialog.js):
//
//   open        ask the daemon to look; printers are listed as each one answers
//   a row       print the page on that printer
//   an address  a printer discovery cannot see, by its address on this network
//
// What the daemon says arrives on the card's notification subscription under categories that
// start with "print-"; BrowserApp hands those to notification() instead of raising a toast.

enyo.kind({
	name: "JihadPrintDialog",
	kind: "ModalDialog",
	caption: $L("Print"),
	events: {
		//* Look for printers. {address} when the user typed one.
		onFindPrinters: "",
		//* Print the page on screen. {printer} is a row of the daemon's list, passed back as is.
		onPrintPage: ""
	},
	components: [
		{name: "status", className: "jihad-print-status"},
		{name: "printers", className: "jihad-print-list"},
		{name: "manual", kind: "RowGroup", components: [
			{name: "address", kind: "Input", hint: $L("Printer address, e.g. 192.168.1.50"),
				autoCapitalize: "lowercase", autocorrect: false, spellcheck: false, onkeydown: "addressKeydown"}
		]},
		{kind: "HFlexBox", components: [
			{name: "searchBtn", kind: "NoFocusButton", flex: 1, caption: $L("Search Again"), onclick: "search"},
			{kind: "NoFocusButton", className: "enyo-button-dark", flex: 1, caption: $L("Close"), onclick: "closeClick"}
		]}
	],
	//* Open on a fresh search.
	openPrint: function() {
		this._printing = false;
		this._printers = [];
		this.openAtCenter();
		this.search();
	},
	closeClick: function() {
		this.close();
	},
	search: function() {
		if (this._printing) { return; }
		var address = String(this.$.address.getValue() || "").replace(/^\s+|\s+$/g, "");
		this._printers = [];
		this.renderPrinters();
		this.$.status.setContent($L("Looking for printers…"));
		this.$.manual.show();
		this.$.searchBtn.show();
		this.doFindPrinters(address ? {address: address} : {});
	},
	addressKeydown: function(inSender, inEvent) {
		if (inEvent && inEvent.keyCode === 13) { this.search(); }
	},
	renderPrinters: function() {
		this.$.printers.destroyControls();
		for (var i = 0; i < this._printers.length; i++) {
			var p = this._printers[i];
			// Second line: what tells two entries apart — the model when the name is not it,
			// and where the printer is.
			var where = p.address + (p.port && p.port !== 631 ? ":" + p.port : "");
			var sub = (p.model && p.model !== p.name ? p.model + " · " : "") + where;
			this.$.printers.createComponent({
				kind: "Item", tapHighlight: true, className: "jihad-print-row", printerIndex: i, onclick: "pick", components: [
					{className: "jihad-print-name", content: enyo.string.escapeHtml(p.name || where)},
					{className: "jihad-print-sub", content: enyo.string.escapeHtml(sub)}
				]
			}, {owner: this});
		}
		this.$.printers.render();
	},
	pick: function(inSender) {
		var p = this._printers[inSender.printerIndex];
		if (!p || this._printing) { return; }
		this._printing = true;
		this.$.printers.destroyControls();
		this.$.printers.render();
		this.$.manual.hide();
		this.$.searchBtn.hide();
		this.$.status.setContent($L("Sending to ") + enyo.string.escapeHtml(p.name || p.address) + "…");
		this.doPrintPage({printer: p});
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
				this.$.status.setContent(list.length ? $L("Choose a printer.") :
					$L("No printer answered. If yours is on, type its address below and search again."));
			} else {
				this.$.status.setContent(list.length ? $L("Choose a printer. Still looking…") : $L("Looking for printers…"));
			}
		} else if (category === "print-progress") {
			this.$.status.setContent(enyo.string.escapeHtml(text));
		} else if (category === "print-done" || category === "print-error") {
			this._printing = false;
			this.$.status.setContent(enyo.string.escapeHtml(text));
			this.$.searchBtn.show();
		}
	}
});
