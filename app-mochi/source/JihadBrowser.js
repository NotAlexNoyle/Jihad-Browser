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

// Start-page logo (compact base64 PNG, bundled inline so the start page needs no asset).
enyo.JihadLogo = "data:image/png;base64,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";

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
			{name: "back",    classes: "jihad-nav-btn disabled", content: "❮", ontap: "goBack"},
			{name: "forward", classes: "jihad-nav-btn disabled", content: "❯", ontap: "goForward"},
			{name: "reload",  classes: "jihad-nav-btn", content: "↻", ontap: "reloadPage"},
			{name: "stop",    classes: "jihad-nav-btn", content: "✕", ontap: "stopLoad", showing: false},
			{kind: "mochi.InputDecorator", name: "addressBox", classes: "jihad-address-box", fit: true, components: [
				{kind: "mochi.Input", name: "address", classes: "jihad-address", placeholder: "Search or type a URL", onchange: "addressEntered", onkeydown: "addressKeydown"}
			]},
			{name: "menu",    classes: "jihad-nav-btn", content: "☰", ontap: "openMenu"}
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
		p = p || {};
		if (p.webviewId) {
			this.$.view.setIdentifier(p.webviewId);
		}
		var url = p.target || p.url;
		// Launched from the icon with no URL: show the Mochi start page so the card
		// presents content instead of a blank/forever-loading view (device: "the
		// mochi app just loads forever and doesn't open"). Only on the FIRST apply
		// (a later relaunch with no URL must not wipe the current page).
		if (url) {
			this.setUrl(url);
		} else if (!this._startedOnce) {
			this.showStartPage();
		}
		this._startedOnce = true;
	},
	//* Mochi-styled start page: a dark card matching the shell chrome (#1c1c1c),
	//* loaded via a data: URL so its background is our own, not a generic about:.
	showStartPage: function() {
		var html = "<!doctype html><html><head><meta charset=utf-8>"
			+ "<meta name=viewport content='width=device-width,initial-scale=1'>"
			+ "<style>html,body{margin:0;height:100%}"
			+ "body{background:#1c1c1c;color:#eaeaea;font-family:sans-serif;"
			+ "display:flex;flex-direction:column;align-items:center;justify-content:center}"
			+ "h1{margin:0;font-size:30px;letter-spacing:.5px}"
			+ ".sub{color:#8cafbe;margin-top:8px;font-size:15px}"
			+ ".logo{width:96px;height:96px;margin-bottom:20px;"
			+ "filter:drop-shadow(0 8px 24px rgba(0,0,0,.5))}"
			+ "</style></head><body><img class=logo src='" + enyo.JihadLogo + "'>"
			+ "<h1>Jihad Browser</h1>"
			+ "<div class=sub>Mochi &middot; UXP/Goanna engine</div></body></html>";
		this.url = "about:jihad";
		if (this.$.view.setHtml) {
			this.$.view.setHtml("about:jihad", html);
		} else {
			this.$.view.setUrl("data:text/html," + encodeURIComponent(html));
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
	goBack:     function() { if (!this.$.back.hasClass("disabled")) this.$.view.callBrowserAdapter("goBack"); },
	goForward:  function() { if (!this.$.forward.hasClass("disabled")) this.$.view.callBrowserAdapter("goForward"); },
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
			this.$.back.addRemoveClass("disabled", !inEvent.canGoBack);
		}
		if (typeof inEvent.canGoForward === "boolean") {
			this.$.forward.addRemoveClass("disabled", !inEvent.canGoForward);
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
