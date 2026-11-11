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

enyo.kind({
	name: "Browser",
	kind: enyo.VFlexBox,
	className: "basic-back",
	published: {
		url: "",
		searchPreferences: {},
		defaultSearch: ""
	},
	events: {
		onPageTitleChanged: "",
		onPageLoadStopped: "",
		onFileLoad: "",
		onAddBookmark: "",
		onAddToLauncher: "",
		onShareLink: "",
		onOpenBookmarks: "",
		onPrint: "",
		onUrlRedirected: "",
		//* The engine's own downloads (JihadWebView.js): (inUrl), (inUrl, inReceived, inTotal),
		//* (inUrl, inMimeType, inPath), (inUrl, inMessage)
		onEngineDownloadStart: "",
		onEngineDownloadProgress: "",
		onEngineDownloadFinished: "",
		onEngineDownloadError: "",
		// called when user wants to leave the browser
		onClose: ""
	},
	components: [
		{name: "launchApplicationService", kind: enyo.PalmService, service: enyo.palmServices.application, method: "open"},
		{name: "importWallpaperService", kind: enyo.PalmService, service: enyo.palmServices.system, method: "wallpaper/importWallpaper", onSuccess: "importedWallpaper", onFailure: "wallpaperError"},
		{name: "setWallpaperService", kind: enyo.PalmService, service: enyo.palmServices.system, method: "setPreferences", onFailure: "wallpaperError"},
		{name: "actionbar", kind: "ActionBar",
			onBack: "goBack",
			onForward: "goForward",
			onHome: "goHome",
			onLoad: "goClick",
			onStopLoad: "stopClick",
			onRefresh: "reloadClick",
			onAddBookmark: "doAddBookmark",
			onAddToLauncher: "doAddToLauncher",
			onShareLink: "doShareLink",
			onOpenBookmarks: "doOpenBookmarks",
			onNewCard: "openNewCard",
			onSecureInfo: "showPageCertificate"
		},
		// The padlock: this browser's own daemon says whose certificate the page was served
		// with (its pageCertificate method answers this variant's own card only).
		{name: "certService", kind: enyo.PalmService, service: "palm://net.riverstonerelay.jihadBrowser/",
			method: "pageCertificate", onSuccess: "gotPageCertificate", onFailure: "gotPageCertificate"},
		// T-600: the card's own page for a failed load, over the view (LoadErrorView.js)
		{name: "loadError", kind: "LoadErrorView", onRetry: "loadErrorRetry", onOpenSettings: "loadErrorSettings"},
		// T-602 (from the Atlas app's 0.9.6 `6aa556d`): the screen lock while a video plays — a button at the view's top
		// right; locked, a shield over the view takes every tap, a tap shows "Screen locked", a tap on that asks
		// "Unlock controls?", a second tap unlocks. Gone 3 s after the video stops or when the card leaves the front.
		{name: "lockShield", className: "jihad-lock-shield", showing: false, onclick: "lockShieldClick", components: [
			{name: "lockNotice", className: "jihad-lock-notice", showing: false, onclick: "lockNoticeClick"}
		]},
		{name: "lockButton", className: "jihad-lock-button", showing: false, onclick: "lockButtonClick", components: [
			{className: "jihad-lock-shackle"}, {className: "jihad-lock-body"}
		]},
		{name: "view", kind: "JihadWebView", flex: 1, height: "100%",
			onDownloadStart: "engineDownloadStart",
			onDownloadProgress: "engineDownloadProgress",
			onDownloadFinished: "engineDownloadFinished",
			onDownloadError: "engineDownloadError",
			onMousehold: "openContextMenu",
			onPageTitleChanged: "pageTitleChanged",
			onUrlRedirected: "doUrlRedirected",
			onLoadStarted: "loadStarted",
			onLoadProgress: "loadProgress",
			onLoadStopped: "loadStopped",
			onLoadComplete: "loadCompleted",
			onFileLoad: "doFileLoad",
			onError: "browserError",
			onSingleTap: "browserTap",
			onAlertDialog: "showAlertDialog",
			onConfirmDialog: "showConfirmDialog",
			onPromptDialog: "showPromptDialog",
			onSSLConfirmDialog: "showSSLConfirmDialog",
			onUserPasswordDialog: "showUserPasswordDialog",
			onNewPage: "openNewCardWithIdentifier",
			onPrint: "doPrint",
			// <select> dropdowns need NO app handler: enyo.WebView consumes the inner
			// BasicWebView's onOpenSelect itself (showSelect -> createSelectPopup ->
			// PopupList) and replies via selectPopupMenuItem. It never re-publishes the
			// event, so an onOpenSelect mapping here is dead code — the daemon just has
			// to emit the isis JSON shape (items[].text/isEnabled + selectedIdx).
			minFontSize: 2,
		},
		{kind: "FindBar", showing: false, onFind: "find", onGoToPrevious: "goToPrevious", onGoToNext: "goToNext"},
		{name: "context", kind: "BrowserContextMenu", onItemClick: "contextItemClick"},
		{name: "dialog", kind: "VerticalAcceptCancelPopup", cancelCaption: "", components: [
			{name: "dialogTitle", className: "enyo-dialog-prompt-title"},
			{name: "dialogMessage", className: "browser-dialog-body enyo-text-body "}
		]},
		{name: "alertDialog", kind: "AcceptCancelPopup", cancelCaption: "", onResponse: "sendDialogResponse", components: [
			{name: "alertMessage", className: "browser-dialog-body enyo-text-body "}
		]},
		{name: "confirmDialog", kind: "VerticalAcceptCancelPopup", onResponse: "sendDialogResponse", components: [
			{name: "confirmMessage", className: "browser-dialog-body enyo-text-body "}
		]},
		{name: "promptDialog", kind: "AcceptCancelPopup", cancelCaption: "", onResponse: "promptResponse", onClose: "closePrompt", components: [
			{name: "promptMessage", className: "browser-dialog-body enyo-text-body "},
			{name: "promptInput", kind: "Input", spellcheck: false, autocorrect: false, autoCapitalize: "lowercase"}
		]},
		{name: "shareLinkDialog", kind: "ShareLinkDialog"},
		{name: "loginDialog", kind: "AcceptCancelPopup", onResponse: "loginResponse", onClose: "closeLogin", components: [
			{name: "loginMessage", className: "browser-dialog-body enyo-text-body "},
			{name: "userInput", kind: "Input", spellcheck: false, autocorrect: false, autoCapitalize: "lowercase", hint: $L("Username...")},
			{name: "passwordInput", kind: "PasswordInput", hint: $L("Password...")}
		]},
		{name: "sslDialog", kind: "Popup", onClose: "sslConfirmResponse", components: [
			{name: "sslConfirmMessage", className: "browser-dialog-body enyo-text-body "},
			{kind: enyo.HFlexBox, components: [
				{kind: "Button", name: "viewCertButton", flex: 1, caption: $L("View Certificate"), className: "enyo-button-dark", onclick: "viewSSLCertificate"},
				{kind: "Button", flex: 1, caption: $L("Trust Always"), response: "1", className: "enyo-button-dark", onclick: "closeSSLConfirmBox"},
				{kind: "Button", flex: 1, caption: $L("Trust Once"), response: "2", className: "enyo-button-dark", onclick: "closeSSLConfirmBox"},
				{kind: "Button", flex: 1, caption: $L("Don't Trust"), response: "0", className: "enyo-button-dark", onclick: "closeSSLConfirmBox"}
			]}
		]},
        {name: "sslCertDialog", kind: "CertificateDialog", onCertLoad: "enableViewSSLCertificate", onClose: "closeSSLCertificate"}
    ],
	WebKitErrors: {
		ERR_SYS_FILE_DOESNT_EXIST: 14,
		ERR_WK_FLOADER_CANCELLED: 1000,
		ERR_WK_NOINTERNET:1005,
		ERR_CURL_FAILURE: 2000,
		ERR_CURL_COULDNT_RESOLVE_HOST: 2006,
		ERR_CURL_SSL_CACERT: 2060
	},
	// Dev-loop boot marker: "@DEV@" is replaced with a per-push stamp by
	// build/webos-oe/push-card-js.sh; a stale WebAppMgr JS cache shows the old stamp.
	jihadBuildStamp: "@DEV@",
	create: function() {
		this.inherited(arguments);
		enyo.log("[JIHAD-BOOT] stamp=" + this.jihadBuildStamp);
		try { window.console.error("[JIHAD-BOOT-ERR] stamp=" + this.jihadBuildStamp); } catch (e) {}
		this.$.context.setView(this.$.view);
		this.urlChanged();
		this.searchPreferencesChanged();
		this.defaultSearchChanged();
		if (window.PalmSystem) {
			this.$.view.setIdentifier(enyo.windowParams.webviewId);
		}
	},
	resize: function() {
		this.$.actionbar.resize();
		this.$.view.resize();
	},
	showingChanged: function() {
		this.inherited(arguments);
		if (!this.showing) {
			this.$.actionbar.forceBlur();
		}
	},
	//* @public
	printFrame: function(inJobID, inPrintParams) {
		this.viewCall("printFrame", ["", inJobID, inPrintParams.width, inPrintParams.height, inPrintParams.pixelUnits, false, inPrintParams.renderInReverseOrder]);
	},
	showFind: function() {
		this.$.findBar.show();
	},
	//* @protected
	find: function(inSender, inString) {
		this.log(inString);
		//* Remembered so "next" can re-issue the same query — see goToNext.
		this._findString = inString;
		this.$.view.callBrowserAdapter("findInPage", [inString]);
	},
	//* The engine's finder continues from the current match, so re-issuing the SAME query is
	//* what "next" means. That is the whole of it — there is no separate find-again command.
	goToNext: function() {
		if (this._findString) {
			this.$.view.callBrowserAdapter("findInPage", [this._findString]);
		}
	},
	//* Backwards search is NOT reachable from here. The frozen scriptable surface exposes
	//* `findInPage(string)` with no direction argument, and the adapter hardcodes forward
	//* (`asyncCmdFindString(str, true)`, BrowserAdapter.cpp). The daemon and the YAP command
	//* both carry the flag, so the capability exists — only the app-facing call cannot express
	//* it, and widening that call set is exactly what cavekit-ui-shell.md R2 forbids. The
	//* FindBar's prev button therefore stays disabled rather than silently searching forward.
	goToPrevious: function() {
	},
	setEnableJavascript: function(inEnable) {
		this.viewCall("setEnableJavascript", [inEnable]);
	},
	setBlockPopups: function(inBlock) {
		this.viewCall("setBlockPopups", [inBlock]);
	},
	setAcceptCookies: function(inAccept) {
		this.viewCall("setAcceptCookies", [inAccept]);
	},
	clearHistory: function() {
		this.viewCall("clearHistory");
	},
	// OUR OWN service, not palm://com.palm.browserServer/ — see below.
	//
	// Upstream calls the stock browser daemon's name. That daemon is still running on this
	// device, because Jihad coexists with it rather than replacing it, so the upstream URI
	// SUCCEEDS and clears the STOCK browser's data while this browser's is untouched
	// (measured 2026-08-04: `{"returnValue":true}` came back and Jihad's cookies were all
	// still there). Silent, and it only became visible once cookie persistence worked.
	//
	// The daemon registers `palm://net.riverstonerelay.jihad-browser<variant>/` itself — the
	// same per-variant identity as the NPAPI MIME, the YAP socket, the upstart job and the
	// state dir. This is a SERVICE name, not part of the frozen BrowserAdapter call set, so
	// the contract cavekit-ui-shell.md R2 protects is untouched.
	clearCookies: function() {
		new PalmServiceBridge().call('palm://net.riverstonerelay.jihadBrowser/clearCookies', '{}');
	},
	clearCache: function() {
		new PalmServiceBridge().call('palm://net.riverstonerelay.jihadBrowser/clearCache', '{}');
	},
	isLoading: function() {
		return this.$.actionbar.getProgress() != 0;
	},
	viewCall: function(inMethod, inArgs) {
		if (window.PalmSystem) {
			var v = this.$.view;
			if (v[inMethod]) {
				v[inMethod].apply(v, inArgs);
			} else {
				v.callBrowserAdapter(inMethod, inArgs);
			}
		}
	},
	urlChanged: function() {
		this.log(this.url);
		// Jihad's own pages open with the keyboard away, so the whole page shows: the address field
		// (focused on the start page, or by typing) gives up the focus (the owner, 2026-10-12).
		if (/^about:(settings|preferences|personas|addons)(?:[#?]|$)/.test(this.url || "") &&
				document.activeElement && document.activeElement.blur) {
			document.activeElement.blur();
		}
		this.$.view.setUrl(this.url);
		this.$.actionbar.setLoading(true);
		this.$.actionbar.setUrl(this.url);
		// Until the engine commits this url the page on screen is the previous one; a
		// padlock for the new address would be the old page's certificate under it.
		this.$.actionbar.setPending(this.url !== this._committedUrl);
	},
	searchPreferencesChanged: function() {
		this.$.actionbar.setSearchPreferences(this.searchPreferences);
	},
	defaultSearchChanged: function() {
		this.$.actionbar.setDefaultSearch(this.defaultSearch);
	},
	pageTitleChanged: function(inSender, inTitle, inUrl, inBack, inForward) {
		this.log(inUrl, inTitle, inBack, inForward);
		// T-600: did this load put a real page up (not the interim about:blank of the engine's www. retry)?
		this._loadCommitted = !!inUrl && inUrl.indexOf("about:blank") !== 0;
		this._committedUrl = inUrl;
		this.$.actionbar.setPending(false);
		this.url = inUrl;
		this.title = inTitle || $L("Untitled");
		if (!this.$.dialog.isOpen) {
			this.$.actionbar.setUrl(this.url);
			this.$.actionbar.setTitle(this.title);
		}
		this.gotHistoryState(inBack, inForward);
		this.doPageTitleChanged(this.title, this.url);
	},
	gotHistoryState: function(inBack, inForward) {
		this.canGoBack = inBack;
		this.canGoForward = inForward;
		this.$.actionbar.setCanGoBack(inBack);
		this.$.actionbar.setCanGoForward(inForward);
	},
	//* T-642 / T-645: Reader View and Desktop Site, the app menu's two page entries
	//* (BrowserApp.js). Both are asked of the engine by opening an address — about:reader
	//* and about:desktopsite, see packaging/prefsui/content/jihadAboutReader.js and
	//* jihadSiteMode.js — so nothing is added to the adapter's call set.
	//*
	//* The address an about:reader url shows as an article, or "" for any other url.
	readerOriginal: function(inUrl) {
		var m = /^about:reader\?(?:[^#]*&)?url=([^&#]*)/i.exec(inUrl || ""), u = "";
		try { u = m ? decodeURIComponent(m[1]) : ""; } catch (e) { u = ""; }
		return /^https?:\/\//i.test(u) ? u : "";
	},
	//* What the app menu offers for the page the engine has committed: reader is "enter" on
	//* an http(s) page, "leave" in the reader, "" anywhere else; desktop is "on"/"off" on an
	//* http(s) page (this card's state), "" anywhere else.
	pageToolState: function() {
		var url = this._committedUrl || "";
		var web = /^https?:\/\//i.test(url);
		return {
			reader: web ? "enter" : (this.readerOriginal(url) ? "leave" : ""),
			desktop: web ? (this._desktopSite ? "on" : "off") : ""
		};
	},
	//* Into the reader from an http(s) page; out of it to the page it shows. Out is a step
	//* Back when that is where the page is (the reader was entered from it in this card),
	//* so the page comes back as it was left; otherwise the page is opened.
	toggleReader: function() {
		var url = this._committedUrl || "", original = this.readerOriginal(url);
		if (original) {
			if (this._readerFrom === original && this.canGoBack) {
				this.$.view.callBrowserAdapter("goBack");
			} else {
				this.setUrl(original);
			}
		} else if (/^https?:\/\//i.test(url)) {
			this._readerFrom = url;
			this.setUrl("about:reader?url=" + encodeURIComponent(url));
		}
	},
	//* Desktop site for THIS card, on or off, and the page loaded again with it. The engine
	//* loads the page itself under its own address (the about: address is never committed),
	//* so the address bar is left alone: only the view is pointed at the request.
	toggleDesktopSite: function() {
		var url = this._committedUrl || "";
		if (!/^https?:\/\//i.test(url)) { return; }
		this._desktopSite = !this._desktopSite;
		this.$.view.setUrl("about:desktopsite?on=" + (this._desktopSite ? "1" : "0") + "&url=" + encodeURIComponent(url));
		this.$.actionbar.setLoading(true);
	},
	goClick: function(inSender, inUrl) {
		//this.$.popup.openAtTap({centerX: 100, centerY: 100});
		this.setUrl(inUrl);
		//this.showHideTitle(true);
	},
	browserTap: function(inSender, inPosition, inEvent, inTapInfo) {
	},
	showPopup: function(inPopup) {
		var w = enyo.fetchControlSize(this).w;
		inPopup.applyStyle("max-width", w - 100);
		inPopup.openPopup();
	},
	showAlertDialog: function(inSender, inMsg) {
		this.$.alertDialog.validateComponents();
		this.$.alertMessage.setContent(inMsg);
		this.showPopup(this.$.alertDialog);
	},
	showConfirmDialog: function(inSender, inMsg) {
		this.$.confirmDialog.validateComponents();
		this.$.confirmMessage.setContent(inMsg);
		this.showPopup(this.$.confirmDialog);
	},
	showPromptDialog: function(inSender, inMsg, inDefaultValue) {
		this.$.promptDialog.validateComponents();
		this.$.promptMessage.setContent(inMsg);
		this.$.promptInput.setValue("");
		this.$.promptInput.setHint(inDefaultValue);
		this.showPopup(this.$.promptDialog);
	},
    showShareLinkDialog: function(inUrl, inTitle) {
        this.$.shareLinkDialog.init(inUrl, inTitle);
        this.showPopup(this.$.shareLinkDialog);
    },
	promptResponse: function(inAccept) {
		this.sendDialogResponse(this, inAccept, this.$.promptInput.getValue() || this.$.promptInput.getHint());
	},
	closePrompt: function() {
		this.$.promptInput.forceBlur();
	},
	showSSLConfirmDialog: function(inSender, inHost, inErrorCode, inCertFile) {
		this.$.sslDialog.validateComponents();
		this.$.viewCertButton.setDisabled(true);
		this.$.sslCertDialog.setCertFile(inCertFile);
		var msg;
		if (inErrorCode == 0) {
			msg = $L("The security certificate #{websiteName} sent is expired. Connecting to this site might put your confidential information at risk.");
		} else if (inErrorCode >= 2 && inErrorCode < 5) {
			msg = $L("The website #{websiteName} didn't send a security certificate to identify itself. Connecting to this site might put your confidential information at risk.");
		} else if (inErrorCode >= 5 && inErrorCode < 10) {
			msg = $L("The security certificate #{websiteName} sent could not be read completely. Connecting to this site might put your confidential information at risk.");
		} else if (inErrorCode >= 10 && inErrorCode < 18) {
			msg = $L("The security certificate #{websiteName} sent has some invalid information. Connecting to this site might put your confidential information at risk.");
		} else if (inErrorCode >= 18 && inErrorCode < 24) {
			msg = $L("The security certificate #{websiteName} sent has questionable signatures. Connecting to this site might put your confidential information at risk.");
		} else if (inErrorCode >= 24 && inErrorCode < 30) {
			msg = $L("The security certificate #{websiteName} sent is invalid. Connecting to this site might put your confidential information at risk.");
		} else if (inErrorCode == 30 || inErrorCode == 31 || inErrorCode == 50) {
			msg = $L("The security certificate #{websiteName} sent has inconsistent information in it. Connecting to this site might put your confidential information at risk.");
		}
		if (msg) {
			var m = msg.replace("#{websiteName}", inHost);
			this.$.sslConfirmMessage.setContent(m);
		}
		this.$.sslDialog.response = "0";
		this.$.sslDialog.openAtCenter();
	},
	closeSSLConfirmBox: function(inSender) {
		this.$.sslDialog.response = inSender.response;
		this.$.sslDialog.close();
	},
	sslConfirmResponse: function(inSender) {
		this.viewCall("sendDialogResponse", [inSender.response]);
	},
	enableViewSSLCertificate: function() {
		this.$.viewCertButton.setDisabled(false);
	},
	viewSSLCertificate: function(inSender) {
		this.$.sslCertDialog.validateComponents();
		this.$.sslCertDialog.openAtCenter();
	},
	//* The address bar's padlock was tapped. pageFocused first: the question goes over Luna,
	//* which cannot say which card is asking, and the daemon answers for the focused one.
	showPageCertificate: function() {
		this.$.view.callBrowserAdapter("pageFocused", [true]);
		this.$.certService.call({});
	},
	gotPageCertificate: function(inSender, inResponse) {
		if (inResponse && inResponse.returnValue && inResponse.secure && inResponse.subject) {
			this.$.sslCertDialog.showCertificate(inResponse);
			this.$.sslCertDialog.openAtCenter();
		} else {
			enyo.jihadToast.show("info", $L("No certificate information is available for this page."));
		}
	},
	closeSSLCertificate: function(inSender) {
		this.$.sslCertDialog.close();
	},
	showUserPasswordDialog: function(inSender, inMsg) {
		this.$.loginDialog.validateComponents();
		// A proxy's login (a 407) comes as "[proxy] <host:port> - <realm>" (cavekit-proxies-certs.md R5).
		var proxy = /^\[proxy\] /.test(String(inMsg || ""));
		var msg = proxy ? $L("The proxy {$serverName} requires a username and password")
		                : $L("The server {$serverName} requires a username and password");
		msg = enyo.macroize(msg, {serverName: proxy ? String(inMsg).slice(8) : inMsg});
		this.$.loginMessage.setContent(msg);
		this.showPopup(this.$.loginDialog);
	},
	loginResponse: function(inSender, inAccept) {
		this.sendDialogResponse(this, inAccept, this.$.userInput.getValue(), this.$.passwordInput.getValue());
	},
	sendDialogResponse: function(inSender, inAccepted) {
		this.log(inAccepted);
		if (inAccepted) {
			this.viewCall("acceptDialog", [].slice.call(arguments, 2));
		} else {
			this.viewCall("cancelDialog");
		}
	},
	closeLogin: function() {
		this.$.userInput.forceBlur();
		this.$.passwordInput.forceBlur();
	},
	openContextMenu: function(inSender, inEvent, inTapInfo) {
		if (inTapInfo.isLink || inTapInfo.isImage) {
			this.$.context.openAtTap(inEvent, inTapInfo);
			return true;
		}
		// Anything else is the page's own long-press. Not claimed: the engine then gets the
		// hold (the page's contextmenu event; a word selected under the finger, or a text
		// field focused) and reports what it found as a "page-hold" notification, which is
		// when the menu opens — see pageHold. Several cards share one daemon and all of them
		// hear that report, so only the card that is waiting for one acts on it.
		this._pageHold = {left: inEvent.pageX, top: inEvent.pageY, at: Date.now()};
	},
	//* The engine's answer to a long-press this card passed on: {editable, selected}. The
	//* answer is broadcast to every card of this browser and names no card, so it is taken
	//* only by the one that is on screen and passed a hold on within the last four seconds —
	//* which, a finger being in one place, is one card. (The answer normally takes 0.1-0.5 s;
	//* once, on a card started seconds earlier, it was not acted on at 2.5 s.)
	pageHold: function(inText) {
		var h = this._pageHold, s = null;
		this._pageHold = null;
		if (!h || Date.now() - h.at > 4000 || (window.PalmSystem && PalmSystem.isActivated === false)) {
			return;
		}
		try { s = enyo.json.parse(inText); } catch (e) { s = null; }
		s = s || {};
		this.$.context.openForPage({left: h.left, top: h.top}, {
			editable: !!s.editable, readOnly: !!s.readOnly, selected: !!s.selected,
			canGoBack: !!this.canGoBack, canGoForward: !!this.canGoForward
		});
	},
	pageSelectAllClick: function() {
		this.$.view.callBrowserAdapter("selectAll");
	},
	//* Copy asks the engine for the selected text; it comes back by way of pageCopied. What
	//* the engine says about a copy is broadcast too, so the card remembers that it asked.
	pageCopyClick: function() {
		this._copyAsked = {at: Date.now(), cut: false};
		this.$.view.callBrowserAdapter("copy");
	},
	//* Cut is a copy, and then — only once the text is on the clipboard — the adapter's cut,
	//* which tells the engine to delete what it handed over (see pageCopied).
	pageCutClick: function() {
		this._copyAsked = {at: Date.now(), cut: true};
		this.$.view.callBrowserAdapter("copy");
	},
	//* The lightweight theme ("persona") on this view's action bar (ActionBar.applyTheme).
	applyTheme: function(theme) {
		this.$.actionbar.applyTheme(theme);
	},
	//* The copy this card asked for within the last five seconds, once.
	takeCopyAsked: function() {
		var a = this._copyAsked;
		this._copyAsked = null;
		return (a && Date.now() - a.at < 5000) ? a : null;
	},
	//* "Nothing is selected." and the like are for the card that asked.
	pageCopyEmpty: function() {
		return !!this.takeCopyAsked();
	},
	//* So is how a cut ended ("Cut.", or why not).
	pageCutResult: function() {
		var at = this._cutAsked;
		this._cutAsked = 0;
		return !!(at && Date.now() - at < 5000);
	},
	//* The engine has written the selected text where this card may load it: a one-call
	//* script in the app's own clip/ directory (a card can load a local file from nowhere
	//* else, and the clipboard is the card's to write). `inName` is that file's name.
	pageCopied: function(inName) {
		var asked = this.takeCopyAsked(), view = this.$.view, self = this, done = false;
		if (!asked || !/^copy-[0-9a-f-]+\.js$/.test(String(inName))) {
			return;
		}
		var s = document.createElement("script");
		var failed = function() {
			if (done) { return; }
			done = true;
			window.jihadClip = null;
			enyo.jihadToast.show("info", $L("The text could not be copied."));
		};
		window.jihadClip = function(inText) {
			if (done) { return; }
			done = true;
			window.jihadClip = null;
			if (!enyo.jihadSetClipboard(String(inText))) {
				enyo.jihadToast.show("info", $L("The text could not be copied."));
				return;
			}
			// True from this moment whatever the cut does; "Cut." replaces it if the
			// engine confirms the deletion (BrowserApp.gotNotification, "page-cut").
			enyo.jihadToast.show("info", $L("Copied."));
			if (asked.cut) {
				self._cutAsked = Date.now();
				view.callBrowserAdapter("cut");
			}
		};
		s.onload = function() {
			if (s.parentNode) { s.parentNode.removeChild(s); }
			failed();                       // it loaded and never called: not a clip script
		};
		s.onerror = function() {
			if (s.parentNode) { s.parentNode.removeChild(s); }
			failed();
		};
		s.src = "clip/" + inName;
		document.body.appendChild(s);
	},
	//* Paste: the clipboard is read here (only a card can) and typed into the focused field.
	//* Another app decides what is on the clipboard, so its size is bounded here: more than
	//* a quarter of a million characters is not something to push through a text field.
	//* One message to the engine holds 16 KB, and a longer string is dropped
	//* whole, so the text goes in pieces of at most 12000 bytes of UTF-8, never cut inside a
	//* surrogate pair.
	pagePasteClick: function() {
		var view = this.$.view;
		enyo.jihadGetClipboard(function(inText) {
			var parts, i;
			if (!inText) {
				enyo.jihadToast.show("info", $L("The clipboard is empty."));
			} else if (inText.length > 262144) {
				enyo.jihadToast.show("info", $L("The clipboard holds too much text to paste here."));
			} else {
				parts = enyo.jihadPasteChunks(String(inText));
				for (i = 0; i < parts.length; i++) {
					view.callBrowserAdapter("insertStringAtCursor", [parts[i]]);
				}
			}
		});
	},
	pageBookmarkClick: function() {
		this.doAddBookmark();
	},
	contextItemClick: function(inSender, inValue, inTapInfo, inPosition) {
		if (this[inValue]) {
			this[inValue](inTapInfo, inPosition);
		}
	},
	//* A link the PAGE showed, opened in a new card: web addresses only, as window.open is. The
	//* new card opens it as a typed address would (no page behind it), so a page's file: or
	//* data: link must not get there by a long-press (inspector 2026-10-07).
	newCardClick: function(inTapInfo) {
		var url = String((inTapInfo && inTapInfo.linkUrl) || "");
		if (!/^(https?|ftp):/i.test(url)) {
			this.warn("not opened in a new card (not a web address): " + url.slice(0, 80));
			return;
		}
		enyo.windows.openWindow("index.html", null, {url: url});
	},
	openNewCard: function() {
		enyo.windows.openWindow("index.html", null, null);
	},
	openNewCardWithIdentifier: function(inSender, inIdentifier) {
		enyo.windows.openWindow("index.html", null, {webviewId: inIdentifier});
	},
	copyLinkClick: function(inTapInfo) {
		enyo.dom.setClipboard(inTapInfo.linkUrl);
		var params = enyo.json.stringify({dontLaunch:true});
		enyo.windows.addBannerMessage($L("Link Copied to clipboard"), params);
	},
    //handler for the context menu shareLinkClick in BrowserContextMenu.js.
    //TODO: refactor these for a clearer abstraction. So you don't have to hunt
    //to see where it is being called.
	shareLinkClick: function(inTapInfo) {
		this.shareLink(inTapInfo.linkUrl, inTapInfo.linkText || inTapInfo.linkUrl);
	},
	shareLink: function(inUrl, inTitle) {
        this.showShareLinkDialog(inUrl, inTitle);
	},
	copyToPhotosClick: function(inTapInfo, inPosition) {
		this.viewCall("saveImageAtPoint", [inPosition.left, inPosition.top, "/media/internal",
			enyo.hitch(this, "finishCopyToPhotos", inTapInfo)]);
	},
	shareImageClick: function(inTapInfo, inPosition) {
		this.viewCall("saveImageAtPoint", [inPosition.left, inPosition.top, "/tmp",
			enyo.hitch(this, "finishShareImage", inTapInfo)]);
	},
	setWallpaperClick: function(inTapInfo, inPosition) {
		this.viewCall("saveImageAtPoint", [inPosition.left, inPosition.top, "/media/internal",
			enyo.hitch(this, "finishSetWallpaper", inTapInfo)]);
	},
	openDialog: function(inTitle, inMessage) {
		this.$.dialog.validateComponents();
		this.$.dialogTitle.setContent(inTitle);
		this.$.dialogMessage.setContent(inMessage);
		this.$.dialog.openPopup();
	},
	finishCopyToPhotos: function(inTapInfo, inSuccess, inPath) {
		var params = enyo.json.stringify({dontLaunch:true});
		if (inSuccess) {
			enyo.windows.addBannerMessage($L("Image Saved to Photos"),params);
		} else {
			enyo.windows.addBannerMessage($L("Error Saving Image"),params);
		}
	},
	finishShareImage: function(inTapInfo, inSuccess, inPath) {
		if (inSuccess) {
			var url = inTapInfo.imageUrl;
			var defaultTitle = url.indexOf("data:") >= 0 ? $L("Picture Link") : $L("Picture at ") + url;
			var title = inTapInfo.title || inTapInfo.altText || defaultTitle;
			var msg = $L("Here's a picture I think you'll like: <a href='{$src}'>{$title}</a>");
			msg = enyo.macroize(msg, {src: url, title: title});
			var s = url.lastIndexOf("/") + 1;
			var params = {
				summary: $L("Check out this picture..."),
				text: msg,
				attachments: [{name: url.substring(s), path: inPath}]
			};
			this.$.launchApplicationService.call({id: "com.palm.app.email", params: params});
		} else {
			var p = enyo.json.stringify({dontLaunch:true});
			enyo.windows.addBannerMessage($L("Error Sharing Image"),p);
		}
	},
	finishSetWallpaper: function(inTapInfo, inSuccess, inPath) {
		if (inSuccess) {
			this.$.importWallpaperService.call({target: inPath, scale: 1.0});
		} else {
			var p = enyo.json.stringify({dontLaunch:true});
			enyo.windows.addBannerMessage($L("Error Setting Wallpaper"),p);
		}
	},
	importedWallpaper: function(inSender, inResponse) {
		this.$.setWallpaperService.call({wallpaper: inResponse.wallpaper});
	},
	wallpaperError: function(inSender, inResponse) {
		this.openDialog($L("Error"), $L("Failed to set wallpaper"));
	},
	goBack: function() {
		if (this.canGoBack) {
			this.$.view.callBrowserAdapter("goBack");
		} else {
			this.doClose();
		}
	},
	//* Home goes to the page configured in Preferences (default
	//* https://lite.duckduckgo.com/lite/), by exactly the path a typed address takes
	//* (ActionBar onLoad -> goClick).
	goHome: function() {
		this.setUrl(enyo.jihadChrome.loadHome());
	},
	goForward: function() {
		this.$.view.callBrowserAdapter("goForward");
	},
	reloadClick: function() {
	if(this.isErrorLoadFailed === true) {
		this.isErrorLoadFailed = false;
		this.setUrl(this.failedLoadUrl);
	}
	else
		this.$.view.callBrowserAdapter("reloadPage");
		//this.$.view.setZoom(this.$.view.getZoom() + 0.1);
	},
	stopClick: function() {
		this.log();
		this.$.view.callBrowserAdapter("stopLoad");
		this.$.actionbar.setProgress(0);
	},
	// The engine's own downloads, passed up to the app (BrowserApp.js keeps the list).
	engineDownloadStart: function(inSender, inUrl) { this.doEngineDownloadStart(inUrl); },
	engineDownloadProgress: function(inSender, inUrl, inReceived, inTotal) { this.doEngineDownloadProgress(inUrl, inReceived, inTotal); },
	engineDownloadFinished: function(inSender, inUrl, inMimeType, inPath) { this.doEngineDownloadFinished(inUrl, inMimeType, inPath); },
	engineDownloadError: function(inSender, inUrl, inMessage) { this.doEngineDownloadError(inUrl, inMessage); },
	loadStarted: function() {
	   this._lastProgress = 0;
	   if (this._timeoutHandle != null) {
		   clearTimeout(this._timeoutHandle);
		   this._timeoutHandle = null;
	   }
	   this.isErrorLoadFailed = false;
	   this._loadCommitted = false;
	   this.$.actionbar.setLoadFailed(false);
	   this.$.actionbar.setLoading(true);
	},
	loadProgress: function(inSender, inProgress) {
		if (this._lastProgress < inProgress) {
			this.$.actionbar.setProgress(inProgress);
			this._lastProgress = inProgress;

			if (inProgress === 100) {
				this._timeoutHandle = setTimeout(enyo.hitch(this, "clearProgress"), 1000);
			}
		}
	},
	loadStopped: function() {
		this.doPageLoadStopped(this.url);
		// T-600: a load that ended on a real page without an error took the page view back: the error panel leaves.
		// Not on the engine's own www. retry, whose first stop is an empty about:blank (the panel blinked off for
		// the retry's lookup and a tap on it fell through; found 2026-11-09)
		if (!this.isErrorLoadFailed && this._loadCommitted && this.$.loadError.showing) { this.$.loadError.hide(); }
	},
	//* T-600: the panel's Try Again — the load that failed, again; the panel stays until the result is known.
	loadErrorRetry: function() {
		this.reloadClick();
	},
	loadErrorSettings: function(inSender, inAppId) {
		this.$.launchApplicationService.call({id: inAppId});
	},
	//* T-602: a video started or stopped (BrowserApp.videoPlaying). The button shows while one plays; 3 s after it
	//* stops (the next episode, a seek) the lock and the button go.
	setVideoPlaying: function(inOn) {
		if (this._lockOffTimer) { clearTimeout(this._lockOffTimer); this._lockOffTimer = null; }
		if (inOn) {
			this.placeLockParts();
			this.$.lockButton.show();
		} else {
			this._lockOffTimer = setTimeout(enyo.bind(this, function() { this._lockOffTimer = null; this.dropScreenLock(); }), 3000);
		}
	},
	dropScreenLock: function() {
		this._locked = false;
		this.$.lockShield.hide(); this.$.lockNotice.hide(); this.$.lockButton.hide();
		this.$.lockButton.removeClass("jihad-lock-on");
	},
	placeLockParts: function() {
		var b = this.$.view.hasNode() ? this.$.view.getBounds() : null;
		if (!b) { return; }
		this.$.lockButton.applyStyle("top", (b.top + 12) + "px");
		this.$.lockShield.applyStyle("top", b.top + "px");
		this.$.lockShield.applyStyle("height", b.height + "px");
	},
	lockButtonClick: function(inSender, inEvent) {
		if (this._locked) { return this.lockShieldClick(inSender, inEvent); }
		this._locked = true;
		this.placeLockParts();
		this.$.lockButton.addClass("jihad-lock-on");
		this.$.lockShield.show();
		this.showLockNotice($L("Screen locked"), "locked");
		return true;
	},
	lockShieldClick: function(inSender, inEvent) {
		if (!this._locked) { return true; }
		if (this._noticeStage !== "ask") { this.showLockNotice($L("Screen locked"), "locked"); }
		return true;
	},
	lockNoticeClick: function(inSender, inEvent) {
		if (this._noticeStage === "locked") { this.showLockNotice($L("Unlock controls? Tap again"), "ask"); }
		else if (this._noticeStage === "ask") {
			this._locked = false;
			this.$.lockShield.hide(); this.$.lockNotice.hide();
			this.$.lockButton.removeClass("jihad-lock-on");
		}
		return true;
	},
	showLockNotice: function(inText, inStage) {
		this._noticeStage = inStage;
		this.$.lockNotice.setContent(inText);
		this.$.lockNotice.show();
		if (this._noticeTimer) { clearTimeout(this._noticeTimer); }
		this._noticeTimer = setTimeout(enyo.bind(this, function() { this._noticeTimer = null; this._noticeStage = null; this.$.lockNotice.hide(); }), 3000);
	},
	//* T-600: the error panel over the view, sized to it.
	showLoadError: function(inCode, inUrl) {
		var b = this.$.view.hasNode() ? this.$.view.getBounds() : null;
		this.$.loadError.setError(inCode, inUrl, b);
	},
	loadCompleted: function() {
		// empty
	},
	clearProgress: function() {
		this.$.actionbar.setProgress(0);
		this.$.actionbar.setLoading(false);
		this._timeoutHandle = null;
	},
	browserError: function(inSender, inErrorCode, inMsg) {
		// What is on screen now is an error, not the site in the address bar: no padlock.
		// (A cancelled load is not an error and leaves the page that was there.)
		if (inErrorCode !== this.WebKitErrors.ERR_WK_FLOADER_CANCELLED) {
			this.$.actionbar.setLoadFailed(true);
		}
		
		// T-600 (2026-11-09): in place of isis's modal dialog over whatever was on screen, the card's own panel takes
		// the page view's place with the cause in plain words and Try Again (LoadErrorView.js). A cancelled load
		// is not an error. The failed address is kept so Try Again loads it, not what the view shows.
		if (inErrorCode !== this.WebKitErrors.ERR_WK_FLOADER_CANCELLED) {
			// the failing address rides in the message (BasicWebView: "<message>: <failingURL>"): a card opened on a
			// link that fails has no address of its own yet (this.url is still about:blank; found 2026-11-09)
			var m = /^[^:]*: (.+)$/.exec(inMsg || "");
			var failing = m ? m[1] : "";
			var own = this.url && this.url.indexOf("about:") !== 0 ? this.url : "";
			var bare = function(u) { return u.replace(/^(\w+:\/\/)www\./i, "$1").replace(/\/$/, ""); };
			this.isErrorLoadFailed = true;
			// Try Again: the failing address (a failed link leaves this.url on the page it came from), unless it is
			// the card's own address with the engine's www. guess added: then what the user asked for
			this.failedLoadUrl = failing ? (own && bare(own) === bare(failing) ? own : failing) : own;
			this.showLoadError(inErrorCode, failing || own);
			this.log("load failed: " + inErrorCode + " " + inMsg);
		}
		this.clearProgress();
	},
	createPageImages: function() {
		var t = (new Date()).getTime();
		var w = 90;
		var h = 120;
		var p = "/var/luna/data/browser/icons/";
		var thumbnail = p + "thumbnail-" + t + ".png";
		var icon32 = p + "icon32-" + t + ".png";
		var icon64 = p + "icon64-" + t + ".png";
		this.viewCall("saveViewToFile", [thumbnail, 0, 0, w, h]);
		this.viewCall("generateIconFromFile", [thumbnail, icon64, 0, 0, w, h]);
		//FIXME: resize the icon or thumbnail?
		this.viewCall("resizeImage", [icon64, icon32, 32, 32]);
		return {thumbnailFile: thumbnail, iconFile32: icon32, iconFile64: icon64};
		return {};
	},
	deleteImages: function(inImages) {
		for (var i=0, image; image=inImages[i]; i++) {
			this.log(image);
			this.viewCall("deleteImage", [image]);
		}
	}
})
