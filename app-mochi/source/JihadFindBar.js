// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadFindBar — the Enyo 2 / Mochi port of ../../app/source/FindBar.js, an overlay strip below the toolbar.
// cavekit-find-on-page.md (T-676): found from the first letter; Next and Previous both ways (the adapter's findInPage
// takes a direction since T-674, sent in the IPC's existing FindString field); the engine's count shown ("n of m",
// "No matches"), going round said; Enter is Next; Done clears the page's match (an empty word); opened again, the last
// word is offered selected so typing replaces it.

enyo.kind({
	name: "JihadFindBar",
	kind: "FittableColumns",
	classes: "jihad-findbar",
	showing: false,
	events: {
		onFind: "",     // {value, backwards}
		onClose: ""
	},
	components: [
		{kind: "mochi.InputDecorator", fit: true, classes: "jihad-findbar-box", components: [
			{kind: "mochi.Input", name: "input", classes: "jihad-findbar-input", placeholder: "Find in page",
				oninput: "inputChange", onchange: "inputChange", onkeydown: "inputKeydown",
				attributes: {autocapitalize: "off", autocorrect: "off", spellcheck: "false"}}
		]},
		// A fixed width: FittableColumns does not reflow when the count's text grows, so a growing count pushed the
		// buttons right (Done half off the screen) until the field's blur reflowed the bar under the finger and the
		// tap was lost (find round 2026-11-19). The count's text is numbers and set words only (setResult).
		{name: "count", classes: "jihad-findbar-count", allowHtml: true},
		// Prev and Next keep the field's focus (Enyo's NoFocusButton): the keyboard stays up and the card keeps its size,
		// for a resize while the match scrolls put the page back where the card last had it (find round 2026-11-19)
		{name: "prev", classes: "jihad-findbar-btn disabled", ontap: "findPrevious", onmousedown: "keepFocus", content: "Prev"},
		{name: "next", classes: "jihad-findbar-btn disabled", ontap: "findNext", onmousedown: "keepFocus", content: "Next"},
		{name: "done", classes: "jihad-findbar-btn", ontap: "close", content: "Done"}
	],
	showingChanged: function() {
		this.inherited(arguments);
		if (this.showing) {
			this._sent = "";
			if (this._last) { this.$.input.setValue(this._last); }
			this.setResult(null);
			this.refocus();
		}
	},
	//* The field focused and its word selected (R1, R5): typing replaces it.
	refocus: function() {
		var self = this;
		enyo.asyncMethod(this, function() {
			var n = self.$.input.hasNode();
			if (n && n.focus) { n.focus(); }
			if (n && n.select) { n.select(); }
		});
	},
	inputChange: function() {
		var v = this.$.input.getValue() || "";
		// The field's change event comes on its blur too: the same word again is "next", which a tap
		// on the page did not ask for (review 2026-11-06).
		if (v === this._sent) { return; }
		this._sent = v;
		this._dir = 1;
		this.setResult(null);
		this.doFind({value: v});   // an empty word clears the page's match
	},
	inputKeydown: function(inSender, inEvent) {
		if (inEvent && inEvent.keyCode === 13) {
			this.findNext();
			return true;
		}
	},
	keepFocus: function(inSender, inEvent) {
		if (inEvent && inEvent.preventDefault) { inEvent.preventDefault(); }
	},
	//* A step; a word in the field not searched yet (the last one offered again, R5) is searched instead, once: a step on
	//* top of it skipped its first match (Codex review 2026-11-19).
	step: function(backwards) {
		var v = this.$.input.getValue() || "";
		if (!v) { return; }
		if (v !== this._sent) { this.inputChange(); return; }
		this._dir = backwards ? -1 : 1;
		this.doFind(backwards ? {value: this._sent, backwards: true} : {value: this._sent});
	},
	findPrevious: function() {
		if (this.$.prev.hasClass("disabled")) { return; }
		this.step(true);
	},
	findNext: function() {
		if (this.$.next.hasClass("disabled")) { return; }
		this.step(false);
	},
	//* What the engine said of the last search: {index, total, more, wrapped}, or null while none is in. The answer
	//* carries no text (JihadBrowser.findResult takes it only while one of this card's searches is out, in order).
	setResult: function(r) {
		var none = false, text = "", note = "";
		if (r && this._sent) {
			if (!r.total) {
				none = true;
				text = "No matches";
			} else {
				var of = Number(r.total) + (r.more ? "+" : "");
				text = r.index ? Number(r.index) + " of " + of : of + " found";
				if (r.wrapped) { note = this._dir < 0 ? "from the bottom" : "from the top"; }
			}
		}
		// went round: a second, smaller line under the count, inside its fixed width
		this.$.count.setContent(note ? text + '<span class="jihad-findbar-note">' + note + "</span>" : text);
		this.$.count.addRemoveClass("jihad-findbar-twoline", !!note);
		this.addRemoveClass("jihad-findbar-none", none);
		// Prev and Next work from the first letter: only an empty field or a "No matches" turns them off, never a result not
		// in yet (a Mochi run lost one: Next stayed off and its tap did nothing, find round 2026-11-19)
		var off = !(this._sent || this.$.input.getValue()) || none;
		this.$.prev.addRemoveClass("disabled", off);
		this.$.next.addRemoveClass("disabled", off);
	},
	close: function() {
		this._last = this.$.input.getValue() || "";
		var n = this.$.input.hasNode();
		if (n && n.blur) { n.blur(); }
		this.hide();
		this.doClose();
	}
});
