// Copyright 2026 the Jihad Browser project.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// Mochi (Enyo 2) UI variant of Jihad Browser — main application kind.
//
// This is the browser SHELL: a Mochi-composed toolbar (back / forward / reload /
// stop + address input + menu), a load ProgressBar, the JihadWebView content
// region, and basic Popup scaffolding. Full feature parity with the Enyo 1.0 app
// (bookmarks / history / downloads / find / preferences / start page /
// alert-confirm-prompt-SSL dialogs) is the next-wave parity port (T-053); this
// shell is structured so those views slot into the popups / menu below.
//
// Contract invariant (cavekit-ipc-contract R1, cavekit-mochi-ui R3): the UI
// drives the engine ONLY through JihadWebView's callBrowserAdapter proxy and the
// palm://com.palm.browserServer/* Luna services — the identical method-name set
// and URIs the Enyo 1.0 app uses (../../app/source/Browser.js). See
// ../../docs/IPC-CONTRACT.md.
//
// Layout: FittableRows (toolbar / progress / view). The toolbar is a
// FittableColumns whose address box is `fit: true`, so the chrome reflows to the
// panel width with no hardcoded pixels beyond the shared 1024x768 both TouchPad
// models use. The toolbar is NOT wrapped in mochi.Header — Header reserves a
// title slot and pads its client, which would keep the address field from taking
// the free width. Nav glyphs are inline base64 SVG data URIs (no image assets),
// stroked white on the dark toolbar so they read against the chrome.

// --- toolbar glyphs: monochrome SVG (white stroke), base64 data URIs ---------
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
		//* Current shell URL (mirrors what the address field shows).
		url: ""
	},
	components: [
		// Toolbar built from Mochi controls, laid out with FittableColumns so the
		// address box (fit: true) absorbs the free width; the buttons keep their
		// intrinsic size, so the bar reflows on both TouchPad models. Back/forward
		// start disabled — enabled from page history state (onPageInfoChanged).
		{kind: "FittableColumns", name: "actionBar", classes: "jihad-actionbar", components: [
			{kind: "mochi.IconButton", name: "back",    classes: "jihad-nav-btn", src: enyo.JihadIcons.back,    ontap: "goBack",     disabled: true},
			{kind: "mochi.IconButton", name: "forward", classes: "jihad-nav-btn", src: enyo.JihadIcons.forward, ontap: "goForward",  disabled: true},
			{kind: "mochi.IconButton", name: "reload",  classes: "jihad-nav-btn", src: enyo.JihadIcons.reload,  ontap: "reloadPage"},
			{kind: "mochi.IconButton", name: "stop",    classes: "jihad-nav-btn", src: enyo.JihadIcons.stop,    ontap: "stopLoad",   showing: false},
			{kind: "mochi.InputDecorator", name: "addressBox", classes: "jihad-address-box", fit: true, components: [
				{kind: "mochi.Input", name: "address", classes: "jihad-address", placeholder: "Search or type a URL", onchange: "addressEntered", onkeydown: "addressKeydown"}
			]},
			{kind: "mochi.IconButton", name: "menu", classes: "jihad-nav-btn", src: enyo.JihadIcons.menu, ontap: "openMenu"}
		]},
		// Load progress; shown only during navigation.
		{kind: "mochi.ProgressBar", name: "progress", classes: "jihad-progress", progress: 0, showing: false},
		// Rendered web content: the NPAPI-bound Enyo 2 WebView (JihadWebView.js).
		{kind: "JihadWebView", name: "view", fit: true, classes: "jihad-view",
			onLoadStarted:     "loadStarted",
			onLoadProgress:    "loadProgress",
			onLoadStopped:     "loadStopped",
			onPageInfoChanged: "pageInfoChanged"
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

	// --- init + launch parameters -------------------------------------------
	create: function() {
		this.inherited(arguments);
		// webOS relaunch (a second launch of the running app, e.g. an external
		// link tap) redelivers launch params. Register best-effort listeners for
		// the platform relaunch events; harmless where they never fire.
		// [device-gated: relaunch delivery is verified on hardware.]
		if (window.PalmSystem) {
			this._relaunchBind = enyo.bind(this, "relaunchHandler");
			if (document.addEventListener) {
				document.addEventListener("webOSRelaunch", this._relaunchBind, false);
			}
			if (window.addEventListener) {
				window.addEventListener("mojo-relaunch", this._relaunchBind, false);
			}
		}
		this.applyLaunchParams(this.readLaunchParams());
	},
	//* Read the initial launch parameters. Enyo 2 does NOT populate the Enyo-1
	//* `enyo.windowParams`; the webOS way is to parse PalmSystem.launchParams
	//* (a JSON string). Fall back to enyo.windowParams only if some bootplate
	//* set it.
	readLaunchParams: function() {
		var p = null;
		if (window.PalmSystem && window.PalmSystem.launchParams) {
			try {
				p = enyo.json.parse(window.PalmSystem.launchParams);
			} catch (e) {
				p = null;
			}
		}
		if (!p && window.enyo && enyo.windowParams) {
			p = enyo.windowParams;
		}
		return p || {};
	},
	//* Apply launch params, matching app/source/BrowserApp.js: `target` (external
	//* link / new-card launches) takes precedence over `url`; `webviewId` binds
	//* an engine-created card to this view.
	applyLaunchParams: function(p) {
		if (!p) {
			return;
		}
		if (p.webviewId) {
			this.$.view.setIdentifier(p.webviewId);
		}
		var url = p.target || p.url;
		if (url) {
			this.setUrl(url);
		}
	},
	relaunchHandler: function() {
		this.applyLaunchParams(this.readLaunchParams());
		return true;
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
	// mochi.Input onchange fires on blur/commit; Enter does not blur on its own,
	// so also submit on the Enter keydown (keyCode 13). Both funnel through
	// submitAddress, which de-dupes so an Enter that also blurs won't double-load.
	addressEntered: function(inSender) {
		this.submitAddress();
	},
	addressKeydown: function(inSender, inEvent) {
		var code = inEvent && (inEvent.keyCode || inEvent.which);
		if (code === 13) {
			this.submitAddress();
			if (inSender.hasNode()) {
				inSender.node.blur();
			}
			return true;
		}
	},
	submitAddress: function() {
		var text = this.$.address.getValue();
		if (!text || text === this._lastSubmit) {
			return;
		}
		this._lastSubmit = text;
		this.openUrl(text);
	},
	openUrl: function(text) {
		var url = this.normalizeUrl(text);
		if (!url) {
			return;
		}
		this.url = url;
		this.$.view.setUrl(url);
	},
	//* Turn an address-bar entry into a URL, mirroring app/source/URLSearch.js
	//* (go / looksLikeHost): pass known navigational schemes through untouched
	//* (so about:blank and mailto: are not corrupted), treat a host-looking token
	//* (localhost, host:port, IPv4, host.tld[/path]) as an http:// navigation,
	//* and hand everything else to the default search.
	normalizeUrl: function(text) {
		var v = (text || "").replace(/^\s+|\s+$/g, "");
		if (!v) {
			return v;
		}
		var m = v.match(/^([a-z][a-z0-9+.\-]*):/i);
		if (m) {
			if (this.isNavigableScheme(m[1].toLowerCase())) {
				return v;
			}
			// Unknown / non-navigational scheme (e.g. javascript:) -> search.
			return this.searchUrl(v);
		}
		if (this.looksLikeHost(v)) {
			return "http://" + v;
		}
		return this.searchUrl(v);
	},
	//* Schemes the address bar loads as-is. javascript:/vbscript: are excluded on
	//* purpose (typing them is treated as a search, not an in-page eval).
	isNavigableScheme: function(scheme) {
		switch (scheme) {
			case "http": case "https": case "ftp": case "file":
			case "about": case "mailto": case "tel": case "data":
			case "view-source": case "ws": case "wss": case "rtsp":
				return true;
			default:
				return false;
		}
	},
	//* Heuristic from URLSearch.looksLikeHost: does the raw input denote a host
	//* (=> navigate) rather than search terms?
	looksLikeHost: function(text) {
		var v = (text || "").replace(/^\s+|\s+$/g, "");
		if (!v || /\s/.test(v)) {
			return false;
		}
		if (/^localhost(:\d+)?([\/?#].*)?$/i.test(v)) {
			return true;
		}
		if (/^\d{1,3}(\.\d{1,3}){3}(:\d+)?([\/?#].*)?$/.test(v)) {
			return true;
		}
		return /^[a-z0-9]([a-z0-9\-]*[a-z0-9])?(\.[a-z0-9]([a-z0-9\-]*[a-z0-9])?)*\.[a-z]{2,}(:\d+)?([\/?#].*)?$/i.test(v);
	},
	searchUrl: function(text) {
		return "https://duckduckgo.com/?q=" + encodeURIComponent(text);
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
	//* Page info from the engine: {title, url, canGoBack, canGoForward}. Reflect
	//* the URL in the address bar and the history state on the nav buttons. A new
	//* address entry clears the de-dupe guard so the same URL can be retyped.
	pageInfoChanged: function(inSender, inEvent) {
		if (!inEvent) {
			return;
		}
		if (inEvent.url) {
			this.url = inEvent.url;
			this.$.address.setValue(inEvent.url);
			this._lastSubmit = null;
		}
		if (typeof inEvent.canGoBack === "boolean") {
			this.$.back.setDisabled(!inEvent.canGoBack);
		}
		if (typeof inEvent.canGoForward === "boolean") {
			this.$.forward.setDisabled(!inEvent.canGoForward);
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
