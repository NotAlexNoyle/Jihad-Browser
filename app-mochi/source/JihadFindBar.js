// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadFindBar — the Enyo 2 / Mochi port of ../../app/source/FindBar.js. A slim
// overlay bar (toggled by showing) with a text field and prev / next / done
// controls. Typing (>= 2 chars) fires onFind(value); the shell forwards it to the
// engine as callBrowserAdapter("findInPage", [value]) — the ONLY find entry point
// and part of the frozen adapter method set (cavekit-ipc-contract R1). The engine
// exposes only findInPage (no separate next/prev), exactly as in the Enyo 1.0 app
// where goToNext/goToPrevious were no-ops; here Next/Prev simply re-issue the
// current query, which advances the match in the BrowserAdapter search.

enyo.kind({
	name: "JihadFindBar",
	kind: "FittableColumns",
	classes: "jihad-findbar",
	showing: false,
	events: {
		onFind: "",
		onClose: ""
	},
	components: [
		{kind: "mochi.InputDecorator", fit: true, classes: "jihad-findbar-box", components: [
			{kind: "mochi.Input", name: "input", classes: "jihad-findbar-input", placeholder: "Find in page",
				oninput: "inputChange", onchange: "inputChange",
				attributes: {autocapitalize: "off", autocorrect: "off", spellcheck: "false"}}
		]},
		{name: "prev", classes: "jihad-findbar-btn disabled", ontap: "findPrevious", content: "Prev"},
		{name: "next", classes: "jihad-findbar-btn disabled", ontap: "findNext", content: "Next"},
		{name: "done", classes: "jihad-findbar-btn", ontap: "close", content: "Done"}
	],
	showingChanged: function() {
		this.inherited(arguments);
		if (this.showing) {
			var self = this;
			enyo.asyncMethod(this, function() {
				if (self.$.input.hasNode() && self.$.input.node.focus) {
					self.$.input.node.focus();
				}
			});
		}
	},
	//* Current query, or "" if too short to search.
	currentValue: function() {
		var v = this.$.input.getValue() || "";
		return v.length < 2 ? "" : v;
	},
	inputChange: function() {
		var v = this.currentValue();
		var disabled = !v;
		this.$.prev.addRemoveClass("disabled", disabled);
		this.$.next.addRemoveClass("disabled", disabled);
		if (v) {
			this.doFind({value: v});
		}
	},
	findPrevious: function() {
		var v = this.currentValue();
		if (v) {
			this.doFind({value: v});
		}
	},
	findNext: function() {
		var v = this.currentValue();
		if (v) {
			this.doFind({value: v});
		}
	},
	close: function() {
		if (this.$.input.hasNode() && this.$.input.node.blur) {
			this.$.input.node.blur();
		}
		this.hide();
		this.doClose();
	}
});
