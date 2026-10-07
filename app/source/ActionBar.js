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
	name: "ActionBar",
	kind: enyo.VFlexBox,
	defaultKind: "ToolButton",
	pack: "center",
	className: "enyo-toolbar actionbar",
	published: {
		url: "",
		title: "",
		canGoBack: false,
		canGoForward: false,
		canShare: true,
		searchPreferences: {},
		defaultSearch: "",
		progress: 0,
		loading: false,
		loadFailed: false,
		pending: false
	},
	events: {
		onBack: "",
		onForward: "",
		onHome: "",
		onLoad: "",
		onStopLoad: "",
		onRefresh: "",
		onAddBookmark: "",
		onAddToLauncher: "",
		onShareLink: "",
		onOpenBookmarks: "",
		onNewCard: "",
		onSecureInfo: ""
	},
	components: [
    	{kind: "Control", showing: false, name: "title", className: "page-title enyo-text-ellipsis", content: "Untitled"},
		{kind: enyo.HFlexBox, className: "menu-container", align: "center", components: [
			{kind: "ToolButton", name: "back", className: "actionbar-tool-button", icon: "images/chrome/menu-icon-back.png", onclick: "doBack"},
			{kind: "ToolButton", name: "forward", className: "actionbar-tool-button", icon: "images/chrome/menu-icon-forward.png", onclick: "doForward"},
			// Home sits immediately right of forward and goes to the page set in
			// Preferences (enyo.jihadChrome.loadHome(), default
			// https://start.duckduckgo.com/). Never disabled: home is always reachable.
			{kind: "ToolButton", name: "home", className: "actionbar-tool-button", icon: "images/chrome/menu-icon-home.png", onclick: "doHome"},
			{kind: "ToolButton", name: "search", className: "actionbar-tool-button", flex: 1, kind: "URLSearch", onLoad: "doLoad", onStopLoad: "doStopLoad", onRefresh: "doRefresh", onSecureInfo: "doSecureInfo"},
			{kind: "ToolButton", name: "share", className: "actionbar-tool-button", icon: "images/chrome/menu-icon-share.png", onclick: "showSharePopup"},
			{kind: "ToolButton", className: "actionbar-tool-button", icon: "images/chrome/menu-icon-newcard.png", onclick: "doNewCard"},
			{kind: "ToolButton", className: "actionbar-tool-button", icon: "images/chrome/menu-icon-bookmark.png", onclick: "doOpenBookmarks"},
		]},	
		{name: "sharePopup", className: "launch-popup",  kind: "Menu", components: [
			{caption: $L("Add Bookmark"), onclick: "doAddBookmark"},
			{caption: $L("Share Link"), onclick: "doShareLink"},
			{caption: $L("Add to Launcher"), onclick: "doAddToLauncher"}
		]},
		{name: "progressBar", kind: "ProgressBar", className: "url-progress invisible", animatePosition: false},
	],
	//* @public
	resize: function() {
		this.$.search.resize();
	},
	forceFocus: function() {
		this.$.search.forceFocus();
	},
	forceBlur: function() {
		this.$.search.closeSearchPopup();
	},
	//* Paint the lightweight theme ("persona") on the top chrome, or take it off (null). As
	//* Firefox 52 / Pale Moon / Basilisk draw one: the header image anchored to the top right
	//* behind the toolbar, accentcolor where the image does not reach, textcolor on the text.
	//* The image is the file the engine persisted in the profile (inside this app's own
	//* directory, so the card may load it) or the site's URL until then; a URL that is neither
	//* is not applied. Colours are kept to CSS colour syntax (no url(), no quotes, no ';').
	applyTheme: function(theme) {
		var node = this.hasNode();
		if (!node) { this._pendingTheme = theme; return; }   // before render: applied from rendered()
		var title = this.$.title && this.$.title.hasNode();
		// The tool buttons' glyphs are background images (IconButton.applyIconImage,
		// images/chrome/menu-icon-*.png). A theme with a dark text colour gets the ink set
		// (images/chrome/dark/, same names), as Firefox 52 / Pale Moon / Basilisk switch icon sets
		// on the theme's text colour. The original image is kept on each element to put back.
		var icons = node.querySelectorAll(".enyo-button-icon"), i, ic, img;
		var restore = function() {
			for (i = 0; i < icons.length; i++) {
				ic = icons[i];
				if (ic.getAttribute("data-jihad-icon")) { ic.style.backgroundImage = ic.getAttribute("data-jihad-icon"); }
			}
		};
		if (!theme || typeof theme !== "object") {
			node.style.background = ""; node.style.color = ""; node.style.borderColor = ""; node.style.webkitBorderImage = "";
			if (title) { title.style.color = ""; }
			restore();
			this.log("theme: none");
			return;
		}
		var color = function(c) { return (typeof c === "string" && /^[#a-zA-Z0-9(),.%\s-]{1,40}$/.test(c)) ? c : ""; };
		var url = function(u) { return (typeof u === "string" && /^(https?:\/\/|file:\/\/\/)[^\s"'()]+$/.test(u)) ? u : ""; };
		var accent = color(theme.accentcolor), text = color(theme.textcolor), header = url(theme.headerURL);
		node.style.background = (accent || "") + (header ? " url(" + header + ") no-repeat right top" : "");
		// The Onyx toolbar is drawn by -webkit-border-image (toolbar.png, stretched over the whole
		// box), which paints over any background: off while a theme is in force.
		node.style.webkitBorderImage = "none";
		node.style.color = text;
		if (title) { title.style.color = text; }
		restore();
		if (this.isDarkColor(text)) {
			for (i = 0; i < icons.length; i++) {
				ic = icons[i];
				img = ic.style.backgroundImage;
				if (!img || img === "none" || !/images\/chrome\/menu-icon-/.test(img)) { continue; }
				ic.setAttribute("data-jihad-icon", img);
				ic.style.backgroundImage = img.replace(/images\/chrome\/menu-icon-/, "images/chrome/dark/menu-icon-");
			}
		}
		this.log("theme: " + (theme.name || theme.id) + (header ? " (header image)" : " (colours only)"));
	},
	rendered: function() {
		this.inherited(arguments);
		if (this._pendingTheme !== undefined) { var t = this._pendingTheme; delete this._pendingTheme; this.applyTheme(t); }
	},
	//* Whether a CSS colour (a theme's textcolor, in any form the browser accepts) is dark: the
	//* browser resolves it on a detached element. Unreadable → not dark.
	isDarkColor: function(c) {
		if (!c) { return false; }
		var d = document.createElement("div"), m;
		d.style.color = c;
		document.body.appendChild(d);
		m = /rgba?\((\d+),\s*(\d+),\s*(\d+)/.exec(window.getComputedStyle(d, null).color || "");
		document.body.removeChild(d);
		return !!m && (0.299 * m[1] + 0.587 * m[2] + 0.114 * m[3]) < 128;
	},
	//* @protected
	create: function() {
		this.inherited(arguments);
		this.urlChanged();
		this.titleChanged();
		this.canGoBackChanged();
		this.canGoForwardChanged();
		this.canShareChanged();
		this.loadingChanged();
	},
	loadFailedChanged: function() {
		this.$.search.setLoadFailed(this.loadFailed);
	},
	pendingChanged: function() {
		this.$.search.setPending(this.pending);
	},
	urlChanged: function() {
		this.$.search.setUrl(this.url);
	},
	titleChanged: function() {
		this.$.title.setContent(this.title || $L("Untitled"));
	},
	canGoBackChanged: function() {
		this.$.back.setDisabled(!this.canGoBack);
	},
	canGoForwardChanged: function() {
		this.$.forward.setDisabled(!this.canGoForward);
	},
	canShareChanged: function() {
		this.$.share.setDisabled(!this.canShare);
	},
	searchPreferencesChanged: function() {
		this.$.search.setSearchPreferences(this.searchPreferences);
	},
	defaultSearchChanged: function() {
		this.$.search.setDefaultSearch(this.defaultSearch);
	},
	showSharePopup: function(inSender, inEvent) {
		this.$.sharePopup.openAt({top: -1000});
		var pop = this.$.sharePopup.getBounds(); 
		this.$.sharePopup.close();
		this.$.sharePopup.openAtControl(this.$.share, {left: -pop.width+10, top: 26});
	},
	progressChanged: function() {
		this.$.progressBar.setPosition(this.progress);
	},
	loadingChanged: function() {
		this.$.search.setLoading(this.loading);
		if (this.loading) {
			if (this.$.progressBar.hasClass("invisible")) {
				this.$.progressBar.removeClass("invisible");
			}
		} else {
			if (!this.$.progressBar.hasClass("invisible")) {
				this.$.progressBar.addClass("invisible"); 
			}
		}
	},
});
