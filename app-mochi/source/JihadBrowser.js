// Copyright 2026 the Jihad Browser project.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// Mochi (Enyo 2) UI variant of Jihad Browser — main application kind.
//
// This is the browser SHELL: a Mochi-composed action bar (back / forward /
// reload / stop + address input + menu), a load ProgressBar, the JihadWebView
// content region, and basic Popup scaffolding. Full feature parity with the
// Enyo 1.0 app (bookmarks / history / downloads / find / preferences / start
// page / alert-confirm-prompt-SSL dialogs) is the next-wave parity port (T-053);
// this shell is structured so those views slot into the popups / menu below.
//
// Contract invariant (cavekit-ipc-contract R1, cavekit-mochi-ui R3): the UI
// drives the engine ONLY through JihadWebView's callBrowserAdapter proxy and the
// palm://com.palm.browserServer/* Luna services — the identical method-name set
// and URIs the Enyo 1.0 app uses (../../app/source/Browser.js). See
// ../../docs/IPC-CONTRACT.md.
//
// Layout: FittableRows (action bar / progress / view) + a FittableColumns
// toolbar whose address box is `fit: true`, so the chrome reflows to the panel
// width with no hardcoded pixels beyond the shared 1024x768 both TouchPad models
// use. Nav glyphs are inline base64 SVG data URIs (no bundled image assets).

// --- toolbar glyphs: monochrome SVG, base64 data URIs (self-contained) -------
enyo.JihadIcons = {
	back:    "data:image/svg+xml;base64,PHN2ZyB4bWxucz0naHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmcnIHdpZHRoPSczMicgaGVpZ2h0PSczMicgdmlld0JveD0nMCAwIDMyIDMyJz48cGF0aCBkPSdNMjAgNiBMMTAgMTYgTDIwIDI2JyBmaWxsPSdub25lJyBzdHJva2U9J3doaXRlJyBzdHJva2Utd2lkdGg9JzMnIHN0cm9rZS1saW5lY2FwPSdyb3VuZCcgc3Ryb2tlLWxpbmVqb2luPSdyb3VuZCcvPjwvc3ZnPg==",
	forward: "data:image/svg+xml;base64,PHN2ZyB4bWxucz0naHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmcnIHdpZHRoPSczMicgaGVpZ2h0PSczMicgdmlld0JveD0nMCAwIDMyIDMyJz48cGF0aCBkPSdNMTIgNiBMMjIgMTYgTDEyIDI2JyBmaWxsPSdub25lJyBzdHJva2U9J3doaXRlJyBzdHJva2Utd2lkdGg9JzMnIHN0cm9rZS1saW5lY2FwPSdyb3VuZCcgc3Ryb2tlLWxpbmVqb2luPSdyb3VuZCcvPjwvc3ZnPg==",
	reload:  "data:image/svg+xml;base64,PHN2ZyB4bWxucz0naHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmcnIHdpZHRoPSczMicgaGVpZ2h0PSczMicgdmlld0JveD0nMCAwIDMyIDMyJz48cGF0aCBkPSdNMjQgMTYgYTggOCAwIDEgMSAtMi41IC01LjgnIGZpbGw9J25vbmUnIHN0cm9rZT0nd2hpdGUnIHN0cm9rZS13aWR0aD0nMycgc3Ryb2tlLWxpbmVjYXA9J3JvdW5kJyBzdHJva2UtbGluZWpvaW49J3JvdW5kJy8+PHBhdGggZD0nTTIzIDUgTDIzIDExIEwxNyAxMScgZmlsbD0nbm9uZScgc3Ryb2tlPSd3aGl0ZScgc3Ryb2tlLXdpZHRoPSczJyBzdHJva2UtbGluZWNhcD0ncm91bmQnIHN0cm9rZS1saW5lam9pbj0ncm91bmQnLz48L3N2Zz4=",
	stop:    "data:image/svg+xml;base64,PHN2ZyB4bWxucz0naHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmcnIHdpZHRoPSczMicgaGVpZ2h0PSczMicgdmlld0JveD0nMCAwIDMyIDMyJz48cGF0aCBkPSdNOSA5IEwyMyAyMyBNMjMgOSBMOSAyMycgZmlsbD0nbm9uZScgc3Ryb2tlPSd3aGl0ZScgc3Ryb2tlLXdpZHRoPSczJyBzdHJva2UtbGluZWNhcD0ncm91bmQnIHN0cm9rZS1saW5lam9pbj0ncm91bmQnLz48L3N2Zz4=",
	menu:    "data:image/svg+xml;base64,PHN2ZyB4bWxucz0naHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmcnIHdpZHRoPSczMicgaGVpZ2h0PSczMicgdmlld0JveD0nMCAwIDMyIDMyJz48cGF0aCBkPSdNNyAxMSBIMjUgTTcgMTYgSDI1IE03IDIxIEgyNScgZmlsbD0nbm9uZScgc3Ryb2tlPSd3aGl0ZScgc3Ryb2tlLXdpZHRoPSczJyBzdHJva2UtbGluZWNhcD0ncm91bmQnIHN0cm9rZS1saW5lam9pbj0ncm91bmQnLz48L3N2Zz4="
};

enyo.kind({
	name: "JihadBrowser",
	kind: "FittableRows",
	classes: "jihad enyo-fit",
	published: {
		//* Start URL; the app card is opened with one via window params.
		url: ""
	},
	components: [
		// Top action bar built from Mochi controls. A FittableColumns row lets
		// the address box (fit: true) absorb the free width; the buttons keep
		// their intrinsic size, so the bar reflows on both TouchPad models.
		{kind: "mochi.Header", name: "actionBar", classes: "jihad-actionbar", components: [
			{kind: "FittableColumns", classes: "jihad-toolbar", components: [
				{kind: "mochi.IconButton", name: "back",    classes: "jihad-nav-btn", src: enyo.JihadIcons.back,    ontap: "goBack"},
				{kind: "mochi.IconButton", name: "forward", classes: "jihad-nav-btn", src: enyo.JihadIcons.forward, ontap: "goForward"},
				{kind: "mochi.IconButton", name: "reload",  classes: "jihad-nav-btn", src: enyo.JihadIcons.reload,  ontap: "reloadPage"},
				{kind: "mochi.IconButton", name: "stop",    classes: "jihad-nav-btn", src: enyo.JihadIcons.stop,    ontap: "stopLoad", showing: false},
				{kind: "mochi.InputDecorator", name: "addressBox", classes: "jihad-address-box", fit: true, components: [
					{kind: "mochi.Input", name: "address", classes: "jihad-address", placeholder: "Search or type a URL", onchange: "addressEntered"}
				]},
				{kind: "mochi.IconButton", name: "menu", classes: "jihad-nav-btn", src: enyo.JihadIcons.menu, ontap: "openMenu"}
			]}
		]},
		// Load progress; shown only during navigation.
		{kind: "mochi.ProgressBar", name: "progress", classes: "jihad-progress", progress: 0, showing: false},
		// Rendered web content: the NPAPI-bound Enyo 2 WebView (JihadWebView.js).
		{kind: "JihadWebView", name: "view", fit: true, classes: "jihad-view",
			onLoadStarted:      "loadStarted",
			onLoadProgress:     "loadProgress",
			onLoadStopped:      "loadStopped",
			onPageTitleChanged: "pageTitleChanged",
			onUrlChanged:       "viewUrlChanged"
		},
		// Basic Popup scaffolding. The overflow menu and a generic modal dialog
		// are structured here; their contents (bookmarks / history / downloads /
		// preferences / find / alert-confirm-prompt-SSL) are the T-053 port.
		{kind: "mochi.Popup", name: "menuPopup", classes: "jihad-menu-popup", floating: true, components: [
			// T-053: menu items (New Card, Bookmarks, History, Downloads,
			// Find, Share, Add to Launcher, Preferences) slot in here.
		]},
		{kind: "mochi.Popup", name: "dialog", classes: "jihad-dialog", floating: true, modal: true, centered: true, components: [
			{name: "dialogTitle",   classes: "jihad-dialog-title"},
			{name: "dialogMessage", classes: "jihad-dialog-message"}
		]}
	],

	// --- init ---------------------------------------------------------------
	create: function() {
		this.inherited(arguments);
		// Carry the launch URL / webview identifier through, matching the Enyo
		// 1.0 app's create() (Browser.js). PalmSystem is present only on device.
		if (window.PalmSystem && enyo.windowParams) {
			if (enyo.windowParams.webviewId) {
				this.$.view.setIdentifier(enyo.windowParams.webviewId);
			}
			if (enyo.windowParams.url) {
				this.setUrl(enyo.windowParams.url);
			}
		}
		this.urlChanged();
	},
	urlChanged: function() {
		if (this.url) {
			this.$.view.setUrl(this.url);
			this.$.address.setValue(this.url);
		}
	},

	// --- navigation: forwarded verbatim to the frozen adapter contract -------
	// These four are the exact callBrowserAdapter method names the Enyo 1.0 app
	// uses (Browser.js). Do not add or rename (cavekit-ipc-contract R1).
	goBack:     function() { this.$.view.callBrowserAdapter("goBack"); },
	goForward:  function() { this.$.view.callBrowserAdapter("goForward"); },
	reloadPage: function() { this.$.view.callBrowserAdapter("reloadPage"); },
	stopLoad:   function() { this.$.view.callBrowserAdapter("stopLoad"); },
	//* Find-in-page: the frozen adapter method. The FindBar UI that calls this
	//* is the T-053 parity port; the method is present now so the app's
	//* callBrowserAdapter set matches the Enyo 1.0 app exactly.
	find:       function(str) { this.$.view.callBrowserAdapter("findInPage", [str]); },
	//* Luna services the Enyo 1.0 app invokes from Preferences (Browser.js). The
	//* Preferences UI is the T-053 port; these are present now so the
	//* palm://com.palm.browserServer/* URI set matches the Enyo 1.0 app exactly.
	clearCookies: function() { new PalmServiceBridge().call('palm://com.palm.browserServer/clearCookies', '{}'); },
	clearCache:   function() { new PalmServiceBridge().call('palm://com.palm.browserServer/clearCache', '{}'); },

	// --- address bar --------------------------------------------------------
	addressEntered: function(inSender) {
		var text = this.$.address.getValue();
		if (text) {
			this.openUrl(text);
		}
	},
	openUrl: function(text) {
		var url = this.normalizeUrl(text);
		this.url = url;
		this.$.view.setUrl(url);
	},
	//* Turn an address-bar entry into a URL: keep an explicit scheme, treat a
	//* dotted single token as a host, otherwise hand it to the default search.
	normalizeUrl: function(text) {
		text = (text || "").replace(/^\s+|\s+$/g, "");
		if (!text) {
			return text;
		}
		if (/^[a-z][a-z0-9+.-]*:\/\//i.test(text)) {
			return text;
		}
		if (/\s/.test(text) || text.indexOf(".") === -1) {
			return "https://duckduckgo.com/?q=" + encodeURIComponent(text);
		}
		return "http://" + text;
	},

	// --- overflow menu (scaffold) -------------------------------------------
	openMenu: function(inSender) {
		// T-053 populates the menu; for now just present the (empty) popup.
		this.$.menuPopup.show();
	},

	// --- WebView (JihadWebView) callbacks -> chrome state -------------------
	loadStarted: function() {
		this._lastProgress = 0;
		this.$.progress.setProgress(0);
		this.$.progress.setShowing(true);
		this.setStopVisible(true);
	},
	loadProgress: function(inSender, inEvent) {
		var p = (inEvent && typeof inEvent.progress === "number") ? inEvent.progress : 0;
		if (p >= this._lastProgress) {
			this.$.progress.setProgress(p);
			this._lastProgress = p;
		}
	},
	loadStopped: function() {
		this.$.progress.setShowing(false);
		this.$.progress.setProgress(0);
		this.setStopVisible(false);
	},
	pageTitleChanged: function(inSender, inEvent) {
		if (inEvent && inEvent.url) {
			this.url = inEvent.url;
			this.$.address.setValue(inEvent.url);
		}
	},
	viewUrlChanged: function(inSender, inEvent) {
		if (inEvent && inEvent.url) {
			this.url = inEvent.url;
			this.$.address.setValue(inEvent.url);
		}
	},
	//* Swap the reload/stop affordance during a load.
	setStopVisible: function(loading) {
		this.$.stop.setShowing(loading);
		this.$.reload.setShowing(!loading);
	},

	// --- generic dialog (scaffold for the T-053 dialog set) -----------------
	openDialog: function(inTitle, inMessage) {
		this.$.dialogTitle.setContent(inTitle || "");
		this.$.dialogMessage.setContent(inMessage || "");
		this.$.dialog.show();
	}
});
