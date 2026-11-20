//   Copyright 2012 Hewlett-Packard Development Company, L.P.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
// Modified by the Jihad Browser project (2026), per Apache-2.0 section 4(b): this file differs from
// HP's isis-browser original. The changes are this repository's git history for this file.

// Find on page (cavekit-find-on-page.md, T-675): found from the first letter, Next and Previous both ways (the adapter's
// findInPage takes a direction since T-674), the engine's count shown ("n of m", "No matches"), going round said, Enter is
// Next, Done clears the page's match; opened again, the last word is offered selected so typing replaces it.
enyo.kind({
	name: "FindBar",
	kind: enyo.HFlexBox,
	align: "center",
	className: "jihad-findbar",
	events: {
		onFind: "",
		onGoToPrevious: "",
		onGoToNext: "",
		onClose: ""
	},
	components: [
		// changeOnInput: a change for every letter (found as you type). The bar this replaces asked changeOnKeypress, which
		// this Enyo's Input does not have: it searched on Enter and on blur only.
		{kind: "Input", name: "input", flex: 2, autoCapitalize: "lowercase", changeOnInput: true, onchange: "inputChange",
			onkeypress: "inputKeypress", hint: $L("Find in page")},
		{name: "count", className: "jihad-findbar-count"},
		{flex: 1},
		{kind: "NoFocusButton", name: "prev", caption: $L("prev"), disabled: true, onclick: "findPrevious"},
		{kind: "NoFocusButton", name: "next", caption: $L("next"), disabled: true, onclick: "findNext"},
		{kind: "NoFocusButton", caption: $L("done"), onclick: "close"}
	],
	//* @protected
	showingChanged: function() {
		this.inherited(arguments);
		if (this.showing) {
			this._sent = "";
			if (this._last) {
				// the last word again, selected: typing replaces it, Next searches it (R5)
				this.$.input.setValue(this._last);
				this.$.input.forceSelect();
			}
			this.setResult(null);
			this.$.input.forceFocus();
		}
	},
	inputChange: function() {
		var value = this.$.input.getValue();
		// the field's change comes again on its blur and with Enter: the same word again would be "next" to the engine
		if (value === this._sent) { return; }
		this._sent = value;
		this._dir = 1;
		this.setResult(null);
		this.doFind(value);   // an empty word clears the page's match
	},
	inputKeypress: function(inSender, inEvent) {
		if (inEvent && inEvent.keyCode === 13) {
			this.findNext();
			return true;
		}
	},
	//* A step; a word in the field not searched yet (the last one offered again, R5) is searched instead, once: a step on
	//* top of it skipped its first match (Codex review 2026-11-19).
	step: function(inBackwards) {
		var value = this.$.input.getValue() || "";
		if (!value) { return; }
		if (value !== this._sent) { this.inputChange(); return; }
		this._dir = inBackwards ? -1 : 1;
		if (inBackwards) { this.doGoToPrevious(); } else { this.doGoToNext(); }
	},
	findPrevious: function() {
		if (this.$.prev.getDisabled()) { return; }
		this.step(true);
	},
	findNext: function() {
		if (this.$.next.getDisabled()) { return; }
		this.step(false);
	},
	//* @public
	//* What the engine said of the last search: {index, total, more, wrapped}, or null while none is in. The answer
	//* carries no text (Browser.findResult takes it only while one of this card's searches is out, in order).
	setResult: function(r) {
		var c = this.$.count, none = false, text = "";
		if (r && this._sent) {
			if (!r.total) {
				none = true;
				text = $L("No matches");
			} else {
				var of = r.total + (r.more ? "+" : "");
				text = r.index ? r.index + " " + $L("of") + " " + of : of + " " + $L("found");
				if (r.wrapped) { text += " · " + (this._dir < 0 ? $L("from the bottom") : $L("from the top")); }
			}
		}
		c.setContent(text);
		this.$.input.addRemoveClass("jihad-findbar-none", none);
		// Prev and Next work from the first letter, and on a word offered again: only an empty field or a "No matches"
		// turns them off, never a result not in yet (a Mochi run lost one, find round 2026-11-19)
		var off = !(this._sent || this.$.input.getValue()) || none;
		this.$.prev.setDisabled(off);
		this.$.next.setDisabled(off);
	},
	refocus: function() {
		this.$.input.forceSelect();
		this.$.input.forceFocus();
	},
	close: function() {
		this.log();
		this._last = this.$.input.getValue();
		this.$.input.forceBlur();
		this.hide();
		this.doClose();
	}
});
