// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// Jihad Browser (Mojo) — main scene assistant. This is the browser.
//
// STRUCTURE (cavekit-mojo-ui.md R4): built from Mojo's own constructs — a scene
// assistant over the view template app/views/main/main-scene.html, the framework's
// WebView / TextField / ProgressBar / Button widgets, and Mojo.Menu.commandMenu for
// the always-visible navigation bar. The framework comes from the device
// (/usr/palm/frameworks/mojo); nothing is bundled.
//
// ENGINE ROUTING: Mojo.Widget.WebView renders the BrowserAdapter NPAPI <object>. Its
// plugin MIME is swapped to THIS variant's own application/x-jihad-browser-mojo by
// ../models/jihad-engine-override.js, so this card runs on the Jihad daemon
// (/tmp/yapserver.jihad-browser-mojo) and never on another variant's adapter or the
// stock one. See that file for why a framework override is needed.
//
// CONTRACT INVARIANT (cavekit-mojo-ui.md R3, cavekit-ipc-contract.md R1):
// callBrowserAdapter() below is the ONLY app-facing adapter surface, and the set of
// method-name literals routed through it is a strict SUBSET of the Enyo-1.0 app's
// set in ../../../app/source/Browser.js —
//     Mojo : {goBack, goForward, reloadPage, stopLoad}
//     Enyo : {findInPage, goBack, goForward, reloadPage, stopLoad}
// — no additions, no renames. This variant uses no palm://com.palm.browserServer/*
// URI at all (the empty set is a subset of Enyo's {clearCache, clearCookies}).
// Everything else the page needs — openURL, the connect handshake, viewport sizing,
// magnification, dialog responses — is the WebView widget's own internal traffic to
// the plugin, exactly as Enyo-1.0's BasicWebView keeps those calls out of the app's
// own call set.
function MainAssistant(params) {
	this.params = params || {};
	//* Last url the engine committed (msgLocationChanged / msgTitleAndUrlChanged).
	this.url = "";
	this.title = "";
	this.loading = false;
	this.canGoBack = false;
	this.canGoForward = false;
	//* The packaged start page; kept out of the address bar (as in both sibling
	//* variants, whose start page is likewise not a user-visible url).
	this.startUrl = "";
	//* Url of the load that last failed, so the error panel's retry can re-open it.
	this.failedUrl = "";
}

MainAssistant.prototype.WEB_VIEW_ID = "jihad-web";
MainAssistant.prototype.ADDRESS_ID = "jihad-address";
MainAssistant.prototype.PROGRESS_ID = "jihad-progress";
MainAssistant.prototype.RETRY_ID = "jihad-error-retry";

// Engine error codes worth naming, copied from the Enyo-1.0 shell's WebKitErrors
// table (../../../app/source/Browser.js). UI text only — no contract surface.
MainAssistant.prototype.ERRORS = {
	ERR_SYS_FILE_DOESNT_EXIST: 14,
	ERR_WK_FLOADER_CANCELLED: 1000,
	ERR_WK_NOINTERNET: 1005,
	ERR_CURL_FAILURE: 2000,
	ERR_CURL_COULDNT_RESOLVE_HOST: 2006,
	ERR_CURL_SSL_CACERT: 2060
};

// --- lifecycle --------------------------------------------------------------

MainAssistant.prototype.setup = function () {
	// A real page, shipped in the package, so the card opens on rendered content
	// instead of an empty surface. Mojo.appPath is the app's own file:/// root.
	this.startUrl = (window.Mojo && Mojo.appPath ? Mojo.appPath : "") + "start.html";

	this.setupWebView();
	this.setupChrome();
	this.setupCommandMenu();
	this.bindHandlers();
	this.listen();
	// No modelChanged() here: the widgets are instantiated when the scene renders,
	// after setup() returns, and they read these models then.
};

MainAssistant.prototype.setupWebView = function () {
	var attributes = {
		minFontSize: 2,
		// The shell does not interrogate every click; links navigate in place, and
		// the engine reports the committed url back through titleUrlChange/urlChange.
		interrogateClicks: false,
		showClickedLink: true
	};
	if (this.params.pageIdentifier) {
		// This scene is a page the ENGINE created (target=_blank / window.open); bind
		// to that page instead of opening a url of our own. The widget calls
		// setPageIdentifier for us once the adapter is initialised.
		attributes.pageIdentifier = this.params.pageIdentifier;
	} else {
		attributes.url = JihadUrl.normalize(this.params.url) || this.startUrl;
	}
	this.controller.setupWidget(this.WEB_VIEW_ID, attributes);
};

MainAssistant.prototype.setupChrome = function () {
	this.addressModel = {value: "", disabled: false};
	this.controller.setupWidget(this.ADDRESS_ID, {
		hintText: $L("Enter URL or search terms"),
		multiline: false,
		requiresEnterKey: true,
		changeOnKeyPress: false,
		focusMode: Mojo.Widget.focusSelectMode,
		// A url is not prose: no auto-capitalisation, no smart-text replacement.
		autoReplace: false,
		textCase: Mojo.Widget.steModeLowerCase
	}, this.addressModel);

	this.progressModel = {value: 0};
	this.controller.setupWidget(this.PROGRESS_ID, {modelProperty: "value"}, this.progressModel);

	this.retryModel = {buttonLabel: $L("Try Again"), buttonClass: "affirmative", disabled: false};
	this.controller.setupWidget(this.RETRY_ID, {type: Mojo.Widget.defaultButton}, this.retryModel);
};

MainAssistant.prototype.setupCommandMenu = function () {
	// The always-visible bottom bar: back / forward / (stop | reload). The empty
	// items are Mojo's spacers, which centre the group.
	this.backItem = {icon: "back", command: "jihad-back", disabled: true};
	this.forwardItem = {icon: "forward", command: "jihad-forward", disabled: true};
	this.reloadItem = {icon: "refresh", command: "jihad-reload"};
	this.stopItem = {icon: "stop", command: "jihad-stop"};
	this.commandMenuModel = {visible: true, items: []};
	this.syncCommandMenu();
	this.controller.setupWidget(Mojo.Menu.commandMenu, {menuClass: "no-fade"},
		this.commandMenuModel);
	// Mojo.Menu.commandMenu is a key into the scene's widget setups, NOT an element
	// id — there is nothing to look up with controller.get() — so track readiness
	// here to know when modelChanged() has a widget to notify.
	this.commandMenuReady = true;
};

MainAssistant.prototype.bindHandlers = function () {
	this.handleLoadStarted = this.handleLoadStarted.bind(this);
	this.handleLoadProgress = this.handleLoadProgress.bind(this);
	this.handleLoadStopped = this.handleLoadStopped.bind(this);
	this.handleLoadFailed = this.handleLoadFailed.bind(this);
	this.handleTitleUrlChanged = this.handleTitleUrlChanged.bind(this);
	this.handleTitleChanged = this.handleTitleChanged.bind(this);
	this.handleUrlChanged = this.handleUrlChanged.bind(this);
	this.handleCreatePage = this.handleCreatePage.bind(this);
	this.handleMimeUnsupported = this.handleMimeUnsupported.bind(this);
	this.handleServerDisconnect = this.handleServerDisconnect.bind(this);
	this.handleAddressSubmit = this.handleAddressSubmit.bind(this);
	this.handleRetry = this.handleRetry.bind(this);
};

//* WebView events are dispatched on the widget element (Mojo.Event.send(this.adapter,
//* ...) bubbles to it), so every listener attaches to the same node.
MainAssistant.prototype.webViewEvents = function () {
	return [
		[Mojo.Event.webViewLoadStarted, this.handleLoadStarted],
		[Mojo.Event.webViewLoadProgress, this.handleLoadProgress],
		[Mojo.Event.webViewLoadStopped, this.handleLoadStopped],
		[Mojo.Event.webViewDidFinishDocumentLoad, this.handleLoadStopped],
		[Mojo.Event.webViewLoadFailed, this.handleLoadFailed],
		[Mojo.Event.webViewSetMainDocumentError, this.handleLoadFailed],
		[Mojo.Event.webViewTitleUrlChanged, this.handleTitleUrlChanged],
		[Mojo.Event.webViewTitleChanged, this.handleTitleChanged],
		[Mojo.Event.webViewUrlChanged, this.handleUrlChanged],
		[Mojo.Event.webViewCreatePage, this.handleCreatePage],
		[Mojo.Event.webViewMimeNotSupported, this.handleMimeUnsupported],
		[Mojo.Event.webViewMimeHandoff, this.handleMimeUnsupported],
		[Mojo.Event.webViewServerDisconnect, this.handleServerDisconnect]
	];
};

MainAssistant.prototype.listen = function () {
	var view = this.controller.get(this.WEB_VIEW_ID);
	var events = this.webViewEvents();
	for (var i = 0; i < events.length; i++) {
		Mojo.Event.listen(view, events[i][0], events[i][1]);
	}
	Mojo.Event.listen(this.controller.get(this.ADDRESS_ID), Mojo.Event.propertyChange,
		this.handleAddressSubmit);
	Mojo.Event.listen(this.controller.get(this.RETRY_ID), Mojo.Event.tap, this.handleRetry);
};

MainAssistant.prototype.activate = function () {
	if (!this.checkedEngine) {
		this.checkedEngine = true;
		this.verifyEngineRouting();
	}
};

MainAssistant.prototype.deactivate = function () {};

//* This card must run on THIS variant's adapter and no other (cavekit-mojo-ui.md R3).
//* Mojo.Widget.WebView hard-codes the stock plugin MIME and ../models/
//* jihad-engine-override.js rewrites it before the plugin goes live; if that ever
//* stopped taking effect the card would silently render on the stock Mojo browser
//* engine instead, which looks like success. So check the live <object> once the
//* scene is up and surface it as an error rather than let it pass.
MainAssistant.prototype.verifyEngineRouting = function () {
	var view = this.controller.get(this.WEB_VIEW_ID);
	var plugin = (view && view.querySelector) ? view.querySelector("object") : null;
	if (!plugin) {
		// Off-device (Mojo.Host.browser) the widget substitutes an <img> placeholder.
		return;
	}
	if (!JihadEngine.isOurs(plugin)) {
		Mojo.Log.error("[Jihad] WebView is bound to '%s', not '%s'",
			plugin.getAttribute("type"), JihadEngine.MIME);
		this.showError(null, $L("The browser engine could not be started for this app."));
	}
};

MainAssistant.prototype.cleanup = function () {
	var view = this.controller.get(this.WEB_VIEW_ID);
	var events = this.webViewEvents();
	for (var i = 0; i < events.length; i++) {
		Mojo.Event.stopListening(view, events[i][0], events[i][1]);
	}
	Mojo.Event.stopListening(this.controller.get(this.ADDRESS_ID), Mojo.Event.propertyChange,
		this.handleAddressSubmit);
	Mojo.Event.stopListening(this.controller.get(this.RETRY_ID), Mojo.Event.tap, this.handleRetry);
};

// --- the single app-facing adapter surface (FROZEN SET) ---------------------

//* Proxy a BrowserAdapter method call to the plugin through the WebView widget's own
//* exposed method of the same name (widget.goBack() -> adapter.goBack(), and so on).
//* The set of method-name LITERALS passed here is grep-audited against the Enyo-1.0
//* app and must stay a subset of it — see the file header.
MainAssistant.prototype.callBrowserAdapter = function (method, args) {
	var view = this.controller.get(this.WEB_VIEW_ID);
	var widget = view && view.mojo;
	if (widget && typeof widget[method] === "function") {
		return widget[method].apply(widget, args || []);
	}
	Mojo.Log.warn("[Jihad] BrowserAdapter method unavailable: %s", method);
};

// --- navigation -------------------------------------------------------------

//* Open what the user typed. JihadUrl decides url-vs-search exactly as the Enyo and
//* Mochi shells do. Routed through the widget's own openURL — the WebView-internal
//* plugin path — NOT through callBrowserAdapter, mirroring Enyo-1.0's BasicWebView
//* (urlChanged -> openURL), so it does not widen the audited app-facing set.
MainAssistant.prototype.openUrl = function (text) {
	var url = JihadUrl.normalize(text);
	if (!url) {
		return;
	}
	var view = this.controller.get(this.WEB_VIEW_ID);
	this.hideError();
	if (view && view.mojo && view.mojo.openURL) {
		view.mojo.openURL(url);
	}
};

MainAssistant.prototype.handleAddressSubmit = function (event) {
	// requiresEnterKey is set, so this fires only when the user commits the field.
	var value = (event && event.value !== undefined) ? event.value : this.addressModel.value;
	var address = this.controller.get(this.ADDRESS_ID);
	if (address && address.mojo && address.mojo.blur) {
		address.mojo.blur();
	}
	this.openUrl(value);
};

MainAssistant.prototype.handleRetry = function () {
	this.hideError();
	// Fall back to the start page so the button is never a dead end, even when the
	// failure arrived before any url was committed.
	this.openUrl(this.failedUrl || this.url || this.startUrl);
};

//* Menu commands and the back gesture (Mojo's chain of command).
MainAssistant.prototype.handleCommand = function (event) {
	if (event.type === Mojo.Event.command) {
		switch (event.command) {
		case "jihad-back":
			this.callBrowserAdapter("goBack");
			break;
		case "jihad-forward":
			this.callBrowserAdapter("goForward");
			break;
		case "jihad-reload":
			this.hideError();
			this.callBrowserAdapter("reloadPage");
			break;
		case "jihad-stop":
			this.callBrowserAdapter("stopLoad");
			this.setLoading(false);
			break;
		}
		// The command event is deliberately NOT stopped: nothing further up the chain
		// claims these commands, and the menu widget still wants to see it (this is
		// what the framework's own WebView sample does).
		return;
	}
	if (event.type === Mojo.Event.back && this.canGoBack) {
		// The back gesture walks page history first; only when there is none does
		// Mojo get the event and pop this scene (or minimise the card, for the first
		// one on the stack).
		this.callBrowserAdapter("goBack");
		this.stopEvent(event);
	}
};

//* Chain-of-command events are DOM events, but guard anyway: an event object without
//* Prototype's stop() must not take the whole handler down with a TypeError.
MainAssistant.prototype.stopEvent = function (event) {
	if (event && event.stop) {
		event.stop();
	}
};

//* The engine created a page for a link with a target / window.open. Push another
//* browser scene bound to that page identifier, so the link is not dropped and the
//* back gesture returns to the opener.
MainAssistant.prototype.handleCreatePage = function (event) {
	var identifier = event && event.pageIdentifier;
	if (!identifier) {
		return;
	}
	this.controller.stageController.pushScene("main", {pageIdentifier: identifier});
};

// --- load state -------------------------------------------------------------

MainAssistant.prototype.handleLoadStarted = function () {
	this.hideError();
	this.setLoading(true);
	this.setProgress(0);
};

MainAssistant.prototype.handleLoadProgress = function (event) {
	var percent = (event && typeof event.progress === "number") ? event.progress : 0;
	if (percent < 0) {
		percent = 0;
	}
	if (percent > 100) {
		percent = 100;
	}
	this.setProgress(percent);
};

MainAssistant.prototype.handleLoadStopped = function () {
	this.setProgress(100);
	this.setLoading(false);
};

MainAssistant.prototype.handleLoadFailed = function (event) {
	var code = event && event.errorCode;
	this.setLoading(false);
	if (code === this.ERRORS.ERR_WK_FLOADER_CANCELLED) {
		// The user (or a redirect) cancelled this load; not an error to report.
		return;
	}
	this.failedUrl = (event && event.failingURL) || this.url;
	this.showError(code, event && event.message);
};

MainAssistant.prototype.handleServerDisconnect = function () {
	// The daemon went away (restart or crash). Say so instead of leaving a card that
	// silently stops responding; the widget reconnects on its own timer.
	this.setLoading(false);
	this.showError(null, $L("Lost the connection to the browser engine. Reconnecting..."));
};

MainAssistant.prototype.handleMimeUnsupported = function (event) {
	this.setLoading(false);
	this.showError(null, $L("This content type can't be displayed: ") +
		((event && event.mimeType) || $L("unknown")));
};

MainAssistant.prototype.setLoading = function (loading) {
	this.loading = !!loading;
	var progress = this.controller.get(this.PROGRESS_ID);
	if (progress) {
		if (this.loading) {
			progress.show();
		} else {
			progress.hide();
		}
	}
	this.syncCommandMenu();
};

MainAssistant.prototype.setProgress = function (percent) {
	// The ProgressBar model runs 0..1; the engine reports 0..100.
	this.progressModel.value = percent / 100;
	this.controller.modelChanged(this.progressModel);
};

// --- title / url ------------------------------------------------------------

MainAssistant.prototype.handleTitleUrlChanged = function (event) {
	if (!event) {
		return;
	}
	this.url = event.url || this.url;
	this.title = event.title || this.title;
	this.canGoBack = !!event.canGoBack;
	this.canGoForward = !!event.canGoForward;
	this.syncChrome();
};

MainAssistant.prototype.handleTitleChanged = function (event) {
	this.title = (event && event.title) || this.title;
	this.syncChrome();
};

MainAssistant.prototype.handleUrlChanged = function (event) {
	if (!event) {
		return;
	}
	this.url = event.url || this.url;
	this.canGoBack = !!event.canGoBack;
	this.canGoForward = !!event.canGoForward;
	this.syncChrome();
};

//* Reflect the committed url + reported title, and the history state, in the UI.
MainAssistant.prototype.syncChrome = function () {
	var isStart = this.isStartPage(this.url);
	this.setText("jihad-title",
		isStart ? $L("Jihad Browser") : (this.title || this.url || ""));
	// The start page is app-shipped chrome, so its file:/// path is not shown; the
	// address bar falls back to its hint, as in both sibling variants.
	this.addressModel.value = isStart ? "" : (this.url || "");
	this.controller.modelChanged(this.addressModel);
	this.syncCommandMenu();
};

MainAssistant.prototype.isStartPage = function (url) {
	return !url || url === this.startUrl;
};

//* Put plain text into an element. Titles and engine error strings originate with
//* the loaded page, so they are set as TEXT — never as markup, and never through a
//* Prototype String extension that would have to escape them correctly.
MainAssistant.prototype.setText = function (elementId, text) {
	var element = this.controller.get(elementId);
	if (element) {
		element.textContent = (text === undefined || text === null) ? "" : String(text);
	}
};

MainAssistant.prototype.syncCommandMenu = function () {
	this.backItem.disabled = !this.canGoBack;
	this.forwardItem.disabled = !this.canGoForward;
	this.commandMenuModel.items = [
		{},
		this.backItem,
		this.forwardItem,
		this.loading ? this.stopItem : this.reloadItem,
		{}
	];
	if (this.commandMenuReady) {
		this.controller.modelChanged(this.commandMenuModel);
	}
};

// --- error panel ------------------------------------------------------------

//* A failed load must not leave a blank card (cavekit-mojo-ui.md R2), so the scene
//* shows an error panel with the reason and a retry button instead.
MainAssistant.prototype.showError = function (code, message) {
	var text = this.errorText(code, message);
	var panel = this.controller.get("jihad-error");
	this.setText("jihad-error-message", text);
	if (panel) {
		panel.show();
	}
	Mojo.Log.warn("[Jihad] load failed (%s): %s", code, text);
};

MainAssistant.prototype.hideError = function () {
	var panel = this.controller.get("jihad-error");
	if (panel) {
		panel.hide();
	}
};

//* Same code-to-message mapping the Enyo-1.0 shell uses (Browser.browserError).
MainAssistant.prototype.errorText = function (code, message) {
	switch (code) {
	case this.ERRORS.ERR_SYS_FILE_DOESNT_EXIST:
		return $L("File does not exist.");
	case this.ERRORS.ERR_CURL_COULDNT_RESOLVE_HOST:
		return $L("Unable to resolve host.");
	case this.ERRORS.ERR_WK_NOINTERNET:
		return $L("No Internet connection.");
	case this.ERRORS.ERR_CURL_SSL_CACERT:
		return $L("The site's security certificate could not be verified.");
	default:
		return message || $L("Unable to load page.");
	}
};
