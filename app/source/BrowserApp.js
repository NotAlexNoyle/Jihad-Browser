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
	name: "enyo.BrowserApp",
	kind: enyo.VFlexBox,
	className: "basic-back",
	published: {
		url: "",
		systemPreferences: {},
		browserPreferences: {},
		searchPreferences: [],
		defaultSearch: ""
	},
	components: [
		{kind: "ApplicationEvents", onWindowActivated: "windowActivatedHandler", onWindowDeactivated: "windowDeactivatedHandler", onApplicationRelaunch: "applicationRelaunchHandler"},
		{name: "downloadService", kind: enyo.PalmService, service: "palm://com.palm.downloadmanager/", method: "download", onSuccess: "gotDownloadStatus", onFailure: "gotDownloadFailure", subscribe: true, resubscribe: true},
		{name: "cancelDownloadService", kind: enyo.PalmService, service: "palm://com.palm.downloadmanager/", method: "cancelDownload"},
		{name: "downloadHistoryService", kind: enyo.PalmService, service: "palm://com.palm.downloadmanager/", method: "getAllHistory", onSuccess: "gotDownloadHistory"},
		{name: "clearDownloadsService", kind: enyo.PalmService, service: "palm://com.palm.downloadmanager/", method: "clearHistory"},
		{name: "bookmarksService", kind: "DbService", dbKind: "net.riverstonerelay.jihad-browser.bookmarks:1", reCallWatches: true},
		{name: "historyService", kind: "DbService", dbKind: "net.riverstonerelay.jihad-browser.history:1", reCallWatches: true},
		{name: "systemPrefsService", kind: "SystemService"},
		// This variant's own daemon service, for the calls that are not subscriptions (print).
		{name: "ownService", kind: enyo.PalmService, service: "palm://net.riverstonerelay.jihadBrowser/"},
		{name: "browserPrefsService", kind: "DbService", dbKind: "net.riverstonerelay.jihad-browser.preferences:1", reCallWatches: true},
		// NO com.palm.universalsearch services any more (2026-10-05). Upstream took the search
		// engine list from the system's "Just Type" settings, WROTE the system-wide default
		// whenever the browser's choice changed, and wiped the system's optional engine list
		// whenever browser history was cleared. The choice is now this browser's own, made on
		// the settings page and stored with the rest of its settings (ChromePrefs.js).
		{name: "stService", kind: "PalmService", service: "palm://com.palm.stservice/", timeout: 500},
		{name: "launchApplicationService", kind: "PalmService", service: enyo.palmServices.application, method: "open", onFailure: "gotResourceError"},
		{name: "addToLauncherService", kind: "PalmService", service: enyo.palmServices.application, method: "addLaunchPoint"},
		{name: "resourceInfoService", kind: "PalmService", service: enyo.palmServices.application, method: "getResourceInfo", onSuccess: "gotResourceInfo", onFailure: "gotResourceError"},
		// The daemon's NON-BLOCKING message channel (cavekit-gre-widgets.md R5). One
		// subscription, opened at create() and left open for the life of the card; every
		// informational line the daemon raises arrives on it and becomes a transient toast
		// (source/JihadToast.js). OUR OWN service, per-variant, exactly like clearCookies /
		// clearCache below in Browser.js — never palm://com.palm.browserServer/, which is the
		// stock daemon we coexist with. resubscribe:true so a daemon restart re-arms it
		// instead of leaving the card permanently deaf.
		// The lightweight theme ("persona") in force, asked once at start; changes come on the
		// notification channel as category "theme" (cavekit-addons-extensions.md R10).
		{name: "themeService", kind: enyo.PalmService, service: "palm://net.riverstonerelay.jihadBrowser/",
			method: "getTheme", onSuccess: "gotTheme"},
		{name: "notificationService", kind: enyo.PalmService, service: "palm://net.riverstonerelay.jihadBrowser/", method: "notifications", onSuccess: "gotNotification", onFailure: "notificationServiceFailed", subscribe: true, resubscribe: true},
		{kind: enyo.Pane, flex: 1, height: "100%", onSelectView: "paneViewSelected", lazyViews: [
			{name: "startPage", kind: "StartPage", 
				onUrlChange: "processUrlChange",
				onOpenBookmarks: "bookmarksClick",
				onNewCard: "newCardClick",
				onShow: "startPageShown"
			},
			{name: "browser", kind: "Browser",
				onPageTitleChanged: "pageTitleChanged",
				onPageLoadStopped: "pageLoadStopped",
				onFileLoad: "handleResource",
				onAddBookmark: "addBookmark",
				onAddToLauncher: "showAddtoLauncherDialog",
				onShareLink: "shareClick",
				onOpenBookmarks: "bookmarksClick",
				onPrint: "printClick",
				onUrlRedirected: "openResource",
				onClose: "browserClosed",
				onShow: "browserShown",
				onHide: "browserHidden"
			},
			// NO "preferences" view any more (2026-10-05, owner: "merge browser settings into
			// settings"). What it held — the search engine, the Flash switch, the four clear
			// actions — is on the settings page, which is the "Settings" menu entry.
		]},
		{kind: enyo.Toaster, flyInFrom: "right", style: "top: 0px; bottom: 0px", lazy: false, components: [
			{className: "enyo-sliding-view-shadow"},
			{kind: enyo.VFlexBox, flex: 1, width: "320px", height: "100%", components: [
				{kind: "Header", className: "enyo-header-dark", components: [
					{kind: "RadioGroup", flex: 1, components: [
						{kind: "RadioButton", value: "bookmarks", className: "enyo-radiobutton-dark", icon: "images/chrome/toaster-icon-bookmarks.png", onclick: "showBookmarks"},
						{kind: "RadioButton", value: "history", className: "enyo-radiobutton-dark", icon: "images/chrome/toaster-icon-history.png", onclick: "showHistory"},
						{kind: "RadioButton", value: "downloads", className: "enyo-radiobutton-dark", icon: "images/chrome/toaster-icon-downloads.png", onclick: "showDownloads"}
					]}
				]},
				{name: "drawerPane", kind: "Pane", flex: 1, lazyViews: [
					{name: "bookmarks", kind: "BookmarkList",
						onSelectItem: "selectItem",
						onEditItem: "showEditBookmarkDialog",
						onDeleteItem: "deleteBookmark",
						onAddBookmark: "addBookmark",
						onClose: "closeToaster"
					},
					{name: "history", kind: "HistoryList",
						onSelectItem: "selectItem",
						onDeleteItem: "deleteHistory",
						onClose: "closeToaster",
					},
					{name: "downloads", kind: "DownloadList",
						onOpenItem: "openDownloadedFile",
						onCancelItem: "cancelDownload",
						onRetryItem: "retryDownload",
						onDeleteItem: "deleteDownload",
						onClearAll: "showClearDownloadsDialog",
						onClose: "closeToaster",
						onShow: "downloadListShown"
					}
				]}
			]}
		]},
		{kind: "BookmarkDialog", onAccept: "bookmarkAccept", onBeforeOpen: "setupBookmarkDialog"},
		{name: "downloadError", kind: "BrowserPrompt", caption: $L("Cannot open MIME type"), message: ""},
		{name: "clearDownloadsDialog", kind: "BrowserPrompt",
			caption: $L("Are you sure you want to clear the Downloads list?"),
			message: $L("This will not delete the file(s). Plug your device into a computer and use USB mode."),
			acceptCaption: $L("Clear List"),
			onAccept: "clearDownloads"
		},
		// Print. Upstream used the system PrintDialog here; it accepts HP printers only, so
		// the engine daemon prints by itself and this is its dialog (JihadPrintDialog.js).
		{name: "printDialog", kind: "JihadPrintDialog", lazy: false, onFindPrinters: "printFind", onPrintPage: "printPage"},
		{name: "printNothing", kind: "BrowserPrompt", caption: $L("Print"),
			message: $L("Open a page first: there is nothing to print on the start page."), cancelCaption: ""},
		{kind: "AppMenu", components: [
			// Upstream isis shipped this COMMENTED OUT, and this fork inherited it — so the
			// FindBar, its handlers and the frozen findInPage command were all present and
			// wired, with nothing anywhere to open them. Re-enabled 2026-08-05, now that
			// findInPage actually searches (it used to abort the daemon: patch 0015).
			{caption: $L("Find on Page"), onclick: "showFindOnPage"},
			// ONE settings entry (cavekit-preferences-ui.md R5, finished 2026-10-05): the engine's
			// about:preferences, the surface the three shells share. The card's own "Browser
			// Settings" panel is gone; so is Help, which launched the system help app at an HP
			// address that no longer exists.
			{caption: $L("Settings"), onclick: "openSettings"},
			// Never greyed out. Upstream disabled it while a page was loading and off a page;
			// a slow page then left Print dead for as long as its last image took, with nothing
			// to say why (owner, 2026-10-05: "print is greyed out"). What is on screen can be
			// printed as it is, and off a page printClick says so.
			{name: "printMenuItem", caption: $L("Print"), onclick: "printClick"}
		]}
	],
	//* @protected
	constructor: function() {
		this.inherited(arguments);
		this.downloads = [];
		this.preferences = {};
	},
	create: function() {
		this.inherited(arguments);
		this.fetchInitialPreferences();
		this.subscribeNotifications();
		this.log(enyo.json.stringify(enyo.windowParams));
	},
	//* Open the daemon's message subscription (cavekit-gre-widgets.md R5). Off device there is
	//* no Luna bus at all, so this is skipped rather than left to fail asynchronously.
	subscribeNotifications: function() {
		if (!window.PalmSystem) { return; }
		// Empty params on purpose. The component's own `subscribe: true` is what puts
		// "subscribe":true into the request AND marks the request persistent, and the
		// framework's own docs say passing subscribe in params directly is not supported
		// (enyo/1.0/framework/source/palm/services/PalmService.js, makeRequestProps).
		this.$.notificationService.call({});
		this.$.themeService.call({});
	},
	//* The theme at start (getTheme): null when there is none.
	gotTheme: function(inSender, inResponse) {
		this.setTheme(inResponse && inResponse.theme ? inResponse.theme : null);
	},
	//* The theme in force, painted on whichever of the pane's views exist. The views are LAZY
	//* (the browser view does not exist while the start page shows), so the theme is kept and
	//* given to a view again whenever the pane selects one (paneViewSelected).
	setTheme: function(theme) {
		this._theme = theme;
		if (this.$.browser) { this.$.browser.applyTheme(theme); }
		if (this.$.startPage && this.$.startPage.applyTheme) { this.$.startPage.applyTheme(theme); }
	},
	paneViewSelected: function(inSender, inView) {
		if (this._theme !== undefined && inView && inView.applyTheme) { inView.applyTheme(this._theme); }
	},
	//* Every reply on that subscription lands here — the immediate
	//* {"returnValue":true,"subscribed":true} handshake as well as each later push. Only a
	//* payload carrying `text` is a message; the handshake is not one, and showing it would
	//* put an empty toast on screen at every card launch.
	gotNotification: function(inSender, inResponse) {
		if (!inResponse || !inResponse.text) { return; }
		// What the daemon says about printing belongs in the print dialog while it is open.
		// With it closed, a job still running reports by toast; a list of printers nobody is
		// looking at is dropped (it is JSON, not a sentence).
		if (String(inResponse.category).indexOf("print-") === 0) {
			if (this.$.printDialog.isOpen) { this.$.printDialog.notification(inResponse.category, inResponse.text); return; }
			if (inResponse.category === "print-printers") { return; }
		}
		// The settings page asking this card to clear a list the card keeps. The request and
		// the sentence to show arrive together; the lists are db8 kinds this app owns.
		if (inResponse.category === "chrome-clear-bookmarks") { this.clearBookmarks(); }
		if (inResponse.category === "chrome-clear-history") { this.clearHistory(); }
		// The long-press menu's traffic with the engine: what is under the finger, and where
		// the copied text was put. Neither is a sentence to show.
		if (inResponse.category === "theme") {
			var t = null;
			try { t = enyo.json.parse(inResponse.text); } catch (e) { t = null; }
			this.setTheme(t);
			return;
		}
		if (inResponse.category === "page-hold") { this.$.browser.pageHold(inResponse.text); return; }
		if (inResponse.category === "page-copied") { this.$.browser.pageCopied(inResponse.text); return; }
		// What the engine says about a copy or a cut is broadcast to every card; it is
		// shown by the one that asked.
		if (inResponse.category === "page-copy-empty" && !this.$.browser.pageCopyEmpty()) { return; }
		if ((inResponse.category === "page-cut" || inResponse.category === "page-cut-refused") && !this.$.browser.pageCutResult()) { return; }
		enyo.jihadToast.show(inResponse.category, inResponse.text);
	},
	//* A failed subscription is not worth a dialog: it costs the user their status toasts and
	//* nothing else, and the browser is fully usable without them. Logged so "no toasts" can
	//* be told apart from "the daemon never sent one".
	notificationServiceFailed: function(inSender, inResponse) {
		this.log("notifications subscription failed: " + enyo.json.stringify(inResponse || {}));
	},
	applicationRelaunchHandler: function(inSender) {
		var c = enyo.windows.getActiveWindow();
		var params = enyo.windowParams;
		// squelch needless banner clicks
		if (params.dontLaunch) return true;
		if (params.sendDataToShare !== undefined) {
			this.$.stService.call({data: {target: c.enyo.$.browserApp.getUrl(), type: "rawdata", mimetype: "text/html"}}, {method: "shareData"});
			return true;
		} else if (params.toasterOpen) {
			this.$.toaster.open();
			this.$.radioGroup.setValue(params.toasterOpen);
			this.$.drawerPane.selectViewByPane(params.toasterOpen);
		} else {
			enyo.windows.openWindow("index.html", null, params);
			return true;
		}
	},
	rendered: function() {
		this.inherited(arguments);
		this.log();
		var p = window.PalmSystem ? enyo.windowParams : this.processQueryString();
		var url = p.target || p.url;
		if (url) {
			this.setUrl(url); // this opens the browser view
		} else if (p.webviewId) {
			this.$.pane.selectViewByName("browser");
		} else {
			this.$.pane.selectViewByName("startPage");
		}
		this.showBookmarks();
		//this.showAppMenu();
		//this.resize();
	},
	processQueryString: function() {
		var q = location.search.slice(1), queryArgs = {};
		if (q) {
			var args = q.split("&");
			for (var i=0, a, nv; a=args[i]; i++) {
				var nv = args[i] = a.split("=");
				if (nv) {
					queryArgs[nv[0]] = unescape(nv[1]);
				}
			}
		}
		if (queryArgs.query) {
			queryArgs.url = "https://duckduckgo.com/?q=" + queryArgs.query;
		}
		return queryArgs;
	},
	isBrowserShowing: function() {
		return this.$.pane.getViewName() === "browser";
	},
	isStartPageShowing: function() {
		return this.$.pane.getViewName() === "startPage";
	},
	isDownloadListShowing: function() {
		return this.$.toaster.isOpen && this.$.drawerPane.getViewName() === "downloads";
	},
	isBrowserLoading: function() {
		if (this.$.browser) {
			return this.$.browser.isLoading();
		} else {
			return false;
		}
	},
	resizeHandler: function() {
		this.inherited(arguments);
		if (this.isBrowserShowing()) {
			this.$.browser.resize();
		} else if (this.isStartPageShowing()) {
			this.$.startPage.resize();
		}
	},
	windowActivatedHandler: function() {
		//this.$.browser.hasKind();
		if (this.isBrowserShowing()) {
			this.$.browser.viewCall("activate");
		}
		this.refreshDownloads();
	},
	windowDeactivatedHandler: function() {
		if (this.isBrowserShowing()) {
			this.$.browser.viewCall("deactivate");
		}
	},
	refreshPanes: function() {
		this.inherited(arguments);
		var i = this.$.drawerPane.getViewByIndex();
		this.$.drawerPane.selectViewByIndex(i);
	},
	browserShown: function(inSender, inShowing, inRefresh) {
		enyo.keyboard.setResizesWindow(false);
		// FIXME: there's a transition skag. trying to eliminate.
		setTimeout(enyo.hitch(this, function() {
			this.$.browser.viewCall("activate");
			if (inRefresh || !this.$.browser.getUrl()) {
				this.$.browser.setUrl(this.url);
			}
		}), 1);
	},
	browserHidden: function() {
		enyo.keyboard.setResizesWindow(true);
		this.$.browser.viewCall("deactivate");
	},
	browserClosed: function() {
		this.gotoView("startPage");
	},
	downloadListShown: function(inSender) {
		this.$.downloads.setDownloads(this.downloads);
	},
	//* Settings live in ONE place now (cavekit-preferences-ui.md R5): the engine's own
	//* about:preferences. Opened through the ordinary navigation path — the same setUrl a
	//* typed address takes — so no adapter call is added. The url carries THIS card's
	//* current chrome settings, so the page opens on what the user is actually using.
	openSettings: function() {
		this.gotoView("browser");
		this.$.browser.setUrl(enyo.jihadChrome.settingsUrl());
	},
	//* Every committed url passes here. When it is the settings page carrying an edited
	//* payload, adopt it and redraw the start page.
	noteUrlForChromePrefs: function(inUrl) {
		// startPage is a LAZY pane view: a card launched straight at a url selects "browser"
		// and never instantiates it, so this must not assume it exists.
		if (enyo.jihadChrome.adoptFromUrl(inUrl)) {
			if (this.$.startPage) { this.$.startPage.buildLinks(); }
			// The search engine is one of the adopted settings: the address bars follow it.
			this.setDefaultSearch(enyo.jihadChrome.loadSearchEngine());
		}
		// COUNTS ONLY, never the values: this carries the user's home page and shortcut
		// list, and /var/log/messages is world-readable. The count is what tells you the
		// page->card leg fired at all, which is the leg that was dead until 2026-08-06.
		if (enyo.jihadChrome.isSettingsUrl(inUrl)) {
			enyo.log("jihadChrome: settings url seen, links=" +
				enyo.jihadChrome.linkCount() + " startPage=" + (this.$.startPage ? 1 : 0));
		}
	},
	startPageShown: function() {
		this.$.startPage.setUrl("");
	},
	// OWNERSHIP (cavekit-preferences-ui.md R2/R5). blockPopups, acceptCookies and
	// enableJavascript are owned by the SETTINGS PAGE. Since 2026-10-02 (owner decision) this
	// card has no rows for them and no longer re-applies the copy an older version stored in
	// db8 — which is why the three names are gone from the map below: a stored
	// `blockPopups: false` with no row left to change it would otherwise keep being sent at
	// every launch. What still reaches the engine from the card side is enyo's own
	// BasicWebView.initView(), which sends its published defaults (all true) on every adapter
	// connect; the daemon files those on the pref DEFAULT branch, so the page's value wins
	// (jihad::SetBlockPopups / SetJavaScriptEnabled in render/goanna/GoannaRenderPage.cpp).
	// NB minFontSize was never in this map nor in db8 — its only source is `minFontSize: 2` on
	// the WebView in Browser.js.
	applyPreference: function(inPreference, inValue) {
		this.log(inPreference, inValue);
		var preferenceMap = {
		}
		if (inPreference == "enableJavascript" || inPreference == "blockPopups" || inPreference == "acceptCookies") {
			return;
		}
		this.$.pane.viewByName("browser");
		var o = inPreference == "clearHistory" || inPreference == "clearBookmarks" ? this : this.$.browser;
		this.log(preferenceMap[inPreference] || inPreference);
		this.log(o);
		this.log(o[preferenceMap[inPreference] || inPreference]);
		if (o[preferenceMap[inPreference] || inPreference]) {
			this.log(preferenceMap[inPreference || inPreference], inValue);
			o[preferenceMap[inPreference] || inPreference](inValue);
		}
	},
	systemPreferencesChanged: function() {
		for (var key in this.systemPreferences) {
			this.log(key, this.systemPreferences[key]);
			this.applyPreference(key, this.systemPreferences[key]);
		}
	},
	browserPreferencesChanged: function() {
		for (var key in this.browserPreferences) {
			this.applyPreference(key, this.browserPreferences[key]);
		}
	},
	searchPreferencesChanged: function() {
		var views = ["browser", "startPage"];
		for (var i=0, v;v=views[i];i++) {
			this.$.pane.viewByName(v).setSearchPreferences(this.searchPreferences);
		}
	},
	defaultSearchChanged: function() {
		this.log(this.defaultSearch);
		var views = ["browser", "startPage"];
		for (var i=0, v;v=views[i];i++) {
			this.$.pane.viewByName(v).setDefaultSearch(this.defaultSearch);
		}
	},
	gotSystemPreferences: function(inSender, inResponse) {
		delete inResponse.returnValue;
		this.setSystemPreferences(inResponse);
	},
	gotBrowserPreferences: function(inSender, inResponse) {
		for (var i=0,p;p=inResponse.results[i];i++) {
			this.browserPreferences[p.key] = p.value;
		}
		this.browserPreferencesChanged();
	},
	gotInitialBrowserPreferences: function(inSender, inResponse) {
		var kind = this.$.browserPrefsService.dbKind;
		var defaultBrowserPreferences = [
			{_kind: kind, key: "rememberPasswords", value: true}
		];
		if (inResponse.results.length == 0) {
			this.$.browserPrefsService.call({objects: defaultBrowserPreferences}, {method: "put", onSuccess: "fetchPreferences"});
		} else {
			this.fetchPreferences();
		}
	},
	fetchInitialPreferences: function() {
		this.$.browserPrefsService.call(undefined, {method: "find", onSuccess: "gotInitialBrowserPreferences"});
	},
	fetchPreferences: function(inSuccessCallback, inSubscribe) {
		var systemPreferences = [
			"flashplugins",
			"click2play"
		];
		this.$.systemPrefsService.call({keys: systemPreferences}, {method: "getPreferences", onSuccess: "gotSystemPreferences", subscribe: true});
		this.$.browserPrefsService.call(undefined, {method: "find", onSuccess: "gotBrowserPreferences", subscribe: true});
		this.applySearchEngines();
	},
	//* The engines the address bar can search with, and which one is the default. The list
	//* is this browser's own; the default is the one chosen on the settings page
	//* (ChromePrefs.loadSearchEngine), which falls back to the first when nothing is stored.
	applySearchEngines: function() {
		this.searchPreferences = this.defaultWebSearchEngines();
		this.searchPreferencesChanged();
		this.setDefaultSearch(enyo.jihadChrome.loadSearchEngine());
	},
	// The ids are the ones the settings page offers (packaging/prefsui/content/preferences.js
	// SEARCH_ENGINES) — keep the two in step. Also mirrored by URLSearch.defaultSearchPreferences.
	defaultWebSearchEngines: function() {
		return [
			{id: "duckduckgo-lite", type: "web", enabled: true, displayName: "DuckDuckGo Lite",
			 url: "https://lite.duckduckgo.com/lite/?q={$query}", iconFilePath: ""},
			{id: "duckduckgo-html", type: "web", enabled: true, displayName: "DuckDuckGo HTML",
			 url: "https://html.duckduckgo.com/html/?q={$query}", iconFilePath: ""},
			{id: "duckduckgo", type: "web", enabled: true, displayName: "DuckDuckGo",
			 url: "https://duckduckgo.com/?q={$query}", iconFilePath: ""},
			{id: "google", type: "web", enabled: true, displayName: "Google",
			 url: "https://www.google.com/search?q={$query}", iconFilePath: ""},
			{id: "wikipedia", type: "web", enabled: true, displayName: "Wikipedia",
			 url: "https://en.m.wikipedia.org/wiki/Special:Search?search={$query}", iconFilePath: ""}
		];
	},
	processUrlChange: function(inSender, inUrl) {
		this.setUrl(inUrl);
	},
	urlChanged: function() {
		if (this.isBrowserShowing()) {
			this.browserShown(this, true, true);
		} else {
			this.gotoView("browser");
		}
	},
	selectItem: function(inSender, inItem) {
		this.setUrl(inItem.url);
		this.closeToaster();
	},
	gotoView: function(inName) {
		this.closeToaster();
		this.$.pane.selectViewByName(inName);
	},
	pageTitleChanged: function(inSender, inTitle, inUrl) {
		this.url = inUrl;
		this.title = inTitle;
		// The settings page publishes an edit by rewriting its own fragment; this is where
		// the card learns about it (cavekit-preferences-ui.md R5).
		this.noteUrlForChromePrefs(inUrl);
	},
	pageLoadStopped: function(inSender, inUrl) {
		this.updateHistory(this.title, this.url);
	},
	handleResource: function(inSender, inMimeType, inUrl) {
		this.$.resourceInfoService.call({uri: inUrl, mime: inMimeType});
	},
	gotResourceInfo: function(inSender, inResponse, inRequest) {
		var uri = enyo.uri.parseUri(inResponse.uri);
		if (inResponse.appIdByExtension == enyo.fetchAppId()) {
			this.downloadResource(inResponse.uri, inRequest.params.mime);
		} else if (inResponse.canStream) {
			this.openResourceWithApp(inResponse.appIdByExtension, inResponse.uri, inRequest.params.mime);
		} else if (uri.scheme != "http" && uri.scheme != "https" && uri.scheme != "ftp") {
			this.openResource(inResponse.uri);
		} else {
			this.downloadResource(inResponse.uri, inRequest.params.mime);
		}
	},
	gotResourceError: function(inSender, inResponse) {
		this.$.toaster.close();
		this.$.downloadError.openPopup();
	},
	downloadResource: function(inUrl, inMime) {
		this.downloads.unshift({url: inUrl, mimetype: inMime});
		this.$.downloadService.call({target: inUrl, mime: inMime});
		this.showDownloads();
		this.$.toaster.open();
	},
	openDownloadedFile: function(inSender, inIndex) {
		var d = this.downloads[inIndex];
		if (d.completed && !d.aborted && !d.interrupted) {
			this.$.launchApplicationService.call({target: d.destPath + d.destFile});
		}
	},
	cancelDownload: function(inSender, inIndex) {
		var d = this.downloads[inIndex];
		this.$.cancelDownloadService.call({ticket: d.ticket});
	},
	retryDownload: function(inSender, inIndex) {
		var u = this.downloads[inIndex].url;
		this.downloads[inIndex] = {url: u};
		this.$.downloadService.call({target: u});
	},
	deleteDownload: function(inSender, inIndex) {
	},
	showClearDownloadsDialog: function() {
		this.$.clearDownloadsDialog.openAtCenter();
	},
	clearDownloads: function() {
		var newDownloads = [];
		for (var i=0,d;d=this.downloads[i];i++) {
			if (!d.completed) {
				newDownloads.push(d);
			}
		}
		this.downloads = newDownloads;
		this.$.downloads.setDownloads(this.downloads);
		this.$.clearDownloadsService.call({owner: enyo.fetchAppId()});
	},
	findDownload: function(inObj, inSkipUrl) {
		for (var i=0, d; d=this.downloads[i]; i++) {
			if (d.ticket && d.ticket == inObj.ticket) {
				return d;
			}
			if (!inSkipUrl && d.url && d.url == inObj.url) {
				return d;
			}
		}
	},
	gotDownloadStatus: function(inSender, inResponse) {
		var d = this.findDownload(inResponse);
		if (d) {
			// download manager returns the wrong mimetype
			inResponse.mimetype = d.mimetype;
			enyo.mixin(d, inResponse);
			if (!this.$.downloads.showing && d.completed) {
				var filename = d.destFile.replace(/%20/g, " ");
				var params = enyo.json.stringify({toasterOpen:"downloads"});
				if (d.completionStatusCode == 200) {
					enyo.windows.addBannerMessage(filename + ' ' + $L("finished downloading"), params);
				} else {
					enyo.windows.addBannerMessage($L("There was a problem downloading ") + filename, params);
				}
			}
			if (this.$.downloads.showing) {
				this.$.downloads.setDownloads(this.downloads);
			}
		}
	},
	gotDownloadFailure: function(inSender, inResponse) {
		this.log(inResponse);
	},
	refreshDownloads: function() {
		this.$.downloadHistoryService.call({owner: enyo.fetchAppId()});
	},
	gotDownloadHistory: function(inSender, inResponse) {
		// inResponse is sorted by ticket in ascending order
		var iidx = this.downloads.length;
		for (var i=0,d;d=inResponse.items[i];i++) {
			if (d.state == "completed" && d.fileExistsOnFilesys) {
				iidx = this.insertIntoDownloads(enyo.json.parse(d.recordString), iidx);
			}
		}
		if (this.isDownloadListShowing()) {
			this.$.downloads.setDownloads(this.downloads);
		}
	},
	insertIntoDownloads: function(inObj, inStartIndex) {
		for (var i=inStartIndex,d;d=this.downloads[i-1];i--) {
			if (d.ticket == inObj.ticket) {
				return i;
			}
			if (d.ticket > inObj.ticket) {
				this.downloads.splice(i, 0, inObj);
				return i;
			}
		}
		this.downloads.unshift(inObj);
		return 0;
	},
	openResource: function(inSender, inUrl) {
		this.$.launchApplicationService.call({target: inUrl});
	},
	openResourceWithApp: function(inAppId, inUrl, inMime) {
		this.$.launchApplicationService.call({id: inAppId, params: {target: inUrl, mime: inMime}});
	},
	updateHistory: function(inTitle, inUrl) {
		this.$.historyService.call(
			{query: {where: [{prop:"url",op:"=",val:inUrl}]}},
			{method: "delByQuery"});
		var history = {
			_kind: this.$.historyService.dbKind,
			url: inUrl,
			title: inTitle,
			date: (new Date()).getTime()
		}
		this.$.historyService.call({objects: [history]}, {method: "put"});
	},
	deleteHistory: function(inSender, inHistory) {
		this.$.historyService.call({ids: [inHistory._id]}, {method: "del"});
	},
	clearHistory: function() {
		this.$.historyService.call(undefined, {method: "delByQuery"});
		if (this.$.browser) {
			this.$.browser.clearHistory();
		}
	},
	showFindOnPage: function() {
		if (this.isBrowserShowing()) {
			this.$.browser.showFind();
		}
	},
	printClick: function() {
		if (!this.isBrowserShowing()) { this.$.printNothing.openAtCenter(); return; }
		this.$.printDialog.openPrint();
	},
	//* The print dialog asks for printers / for the page to be printed. Both go to this
	//* variant's own daemon service; the answers come back as "print-…" notifications.
	printFind: function(inSender, inEvent) {
		this.$.ownService.call(inEvent && inEvent.address ? {address: inEvent.address} : {},
			{method: "findPrinters", onSuccess: "printAsked", onFailure: "printUnavailable"});
	},
	printPage: function(inSender, inEvent) {
		// The request goes over Luna and cannot say which page it means; the daemon prints
		// the page of the card that last said it has the user's attention.
		if (this.isBrowserShowing()) { this.$.browser.viewCall("pageFocused", [true]); }
		this.$.ownService.call({printer: inEvent.printer},
			{method: "printPage", onSuccess: "printAsked", onFailure: "printRefused"});
	},
	printAsked: function() {},
	printUnavailable: function() {
		this.$.printDialog.notification("print-error", $L("Printing is not available in this build."));
	},
	printRefused: function() {
		this.$.printDialog.notification("print-error", $L("The page could not be sent to the printer."));
	},
	setupBookmarkDialog: function() {
	},
	showAddtoLauncherDialog: function() {
		this.$.bookmarkDialog.setTitle(this.$.browser.title);
		this.$.bookmarkDialog.setUrl(this.$.browser.url);
		var imgs = {};
		try { imgs = this.$.browser.createPageImages(); } catch (e) {}
		this.$.bookmarkDialog.setIcons(imgs);
		this.$.bookmarkDialog.setAcceptCaption($L("Add to Launcher"));
		this.$.bookmarkDialog.acceptAction = "addToLauncher";
		this.$.bookmarkDialog.openAtCenter();
	},
	addToLauncher: function(inTitle, inUrl, inIcons, inId) {
		this.$.addToLauncherService.call({id: enyo.fetchAppId(), icon: inIcons.iconFile64, title: inTitle, params: {url: inUrl}});
	},
	showEditBookmarkDialog: function(inSender, inBookmark) {
		this.$.bookmarkDialog.setTitle(inBookmark.title);
		this.$.bookmarkDialog.setUrl(inBookmark.url);
		this.$.bookmarkDialog.setIcons({thumbnailFile: inBookmark.thumbnailFile, iconFile32: inBookmark.iconFile32, iconFile64: inBookmark.iconFile64});
		this.$.bookmarkDialog.setId(inBookmark._id);
		this.$.bookmarkDialog.setAcceptCaption($L("Save"));
		this.$.bookmarkDialog.acceptAction = "editBookmark";
		this.$.bookmarkDialog.openAtCenter();
	},
	bookmarkAccept: function(inSender, inTitle, inUrl, inIcons, inId) {
		var a = inSender.acceptAction;
		if (this[a]) {
			this[a](inTitle, inUrl, inIcons, inId);
		}
	},
	addBookmark: function() {
		var date = (new Date()).getTime();
		var b = {
			_kind: this.$.bookmarksService.dbKind,
			title: this.$.browser.title,
			url: this.$.browser.url,
			date: date,
			lastVisited: date,
			defaultEntry: false,
			visitCount: 0,
			idx: null
		};
		// Thumbnail generation (createPageImages -> saveViewToFile/generateIconFromFile
		// NPAPI calls, writing under /var/luna/data/browser/icons which the app user
		// can't write) is best-effort: a throw here must NOT abort the bookmark write,
		// or the bookmark never lands in the db (device: "adding bookmarks via share
		// menu doesn't show up"). The essential record saves regardless.
		try {
			enyo.mixin(b, this.$.browser.createPageImages());
		} catch (e) {
			this.log && this.log("createPageImages failed, saving bookmark without images: " + e);
		}
		this.$.bookmarksService.call({objects: [b]}, {method: "put"});
	},
	editBookmark: function(inTitle, inUrl, inIcons, inId) {
		var date = (new Date()).getTime();
		var b = {
			_id: inId,
			title: inTitle,
			url: inUrl,
			date: date,
		};
		enyo.mixin(b, inIcons);
		this.$.bookmarksService.call({objects: [b]}, {method: "merge"});
	},
	deleteBookmark: function(inSender, inBookmark) {
		this.$.bookmarksService.call({ids: [inBookmark._id]}, {method: "del"});
	},
	clearBookmarks: function() {
		this.$.bookmarksService.call(undefined,{method:"delByQuery"});
	},
	newCardClick: function() {
		enyo.windows.openWindow("index.html");
	},
	shareClick: function() {
		this.$.browser.shareLink(this.url, this.title);
	},
	bookmarksClick: function() {
		this.$.toaster.open();
		this.$.drawerPane.selectView(this.$.drawerPane.getView());
	},
	showBookmarks: function() {
		this.$.radioGroup.setValue("bookmarks");
		this.$.drawerPane.selectViewByName("bookmarks");
	},
	showHistory: function() {
		this.$.radioGroup.setValue("history");
		this.$.drawerPane.selectViewByName("history");
	},
	showDownloads: function() {
		this.$.radioGroup.setValue("downloads");
		this.$.drawerPane.selectViewByName("downloads");
	},
	closeToaster: function() {
		this.$.toaster.close();
	},
	backHandler: function(inSender, e) {
		var n = this.$.pane.getViewName();
		switch (n) {
			case "browser":
				this.$.browser.goBack();
				e.preventDefault();
				break;
			case "startPage":
				break;
			default:
				this.$.pane.back(e);
		}
	}
});
