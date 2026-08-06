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

enyo.kind({
	name: "StartPage",
	kind: enyo.VFlexBox,
	className: "startpage",
	published: {
		url: "",
		searchPreferences: {},
		defaultSearch: ""
	},
	events: {
		onUrlChange: "",
		onOpenBookmarks: "",
		onNewCard: ""
	},
	components: [
		{name: "actionbar", kind: "ActionBar", canShare: false, onLoad: "addressSelect", onOpenBookmarks: "doOpenBookmarks", onNewCard: "doNewCard"},
		{name: "tall", flex: 1, className: "startpage-placeholder-tall", components: [
			// Centred brand block — matches the Mochi variant's start page (logo +
			// name + "<UI> UI ★ <engine>"). The logo is sized to match Mochi's.
			{name: "brand", className: "startpage-brand", components: [
				{kind: "Image", className: "startpage-logo", src: "images/startpage-placeholder.png"},
				{content: "Jihad Browser", className: "startpage-title", allowHtml: false},
				{content: "Enyo UI ★ Goanna/6.9 UXP/b2594a4", className: "startpage-sub", allowHtml: false},
				{content: $L("Type a web address or a search in the bar above, then press Enter."),
					className: "startpage-hint", allowHtml: false},
				// Shortcuts, filled in at create() from enyo.jihadChrome so the list
				// the user edited in Preferences is what shows. Taps go through
				// doUrlChange — the identical event the address bar raises — rather
				// than a new path.
				{name: "links", className: "startpage-links"}
			]},
			{name: "placeholder", className: "startpage-placeholder"}
		]}
	],
	//* @protected
	create: function() {
		this.inherited(arguments);
		this.buildLinks();
		this.searchPreferencesChanged();
		this.defaultSearchChanged();
	},
	//* Render the shortcut row from the stored list. Called again by the shell after
	//* Preferences saves, so an edit shows without relaunching the card.
	buildLinks: function() {
		var list = enyo.jihadChrome.loadLinks(), i;
		this.$.links.destroyControls();
		for (i = 0; i < list.length; i++) {
			this.$.links.createComponent({
				content: list[i].title,
				className: "startpage-link",
				// Enyo 1.0's onclick hands back the sender, so the target rides on the
				// control itself rather than needing a handler per row.
				linkUrl: list[i].url,
				onclick: "linkTapped"
			}, {owner: this});
		}
		if (this.hasNode()) {
			this.$.links.render();
		}
	},
	addressSelect: function(inSender, inUrl) {
		this.doUrlChange(inUrl);
	},
	linkTapped: function(inSender) {
		if (inSender && inSender.linkUrl) {
			this.doUrlChange(inSender.linkUrl);
		}
	},
	showingChanged: function() {
		this.inherited(arguments);
		// Always focus the action bar when start page is shown.
		if (this.showing) {
			this.$.actionbar.forceFocus();
		} else {
			this.$.actionbar.forceBlur();
		}
	},
	urlChanged: function() {
		this.$.actionbar.setUrl(this.url);
	},
	searchPreferencesChanged: function() {
		this.$.actionbar.setSearchPreferences(this.searchPreferences);
	},
	defaultSearchChanged: function() {
		this.$.actionbar.setDefaultSearch(this.defaultSearch);
	},
	//* @public
	resize: function() {
		this.$.actionbar.resize();
		// Workaround a repaint issue where the area behind
		// the keyboard does not get repainted when switch cards, if the
		// keyboard resizes the window. This is temporary until the native
		// issue is fixed. 
		var b = enyo.calcModalControlBounds(this.$.tall);
		this.$.placeholder.applyStyle("height", b.height + "px");
	},
	getUrl: function() {
		return this.$.actionBar.getUrl();
	}
});
