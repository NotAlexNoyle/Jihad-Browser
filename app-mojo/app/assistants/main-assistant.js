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
// Since 2026-08-15 it does hold ONE subscription to its OWN per-variant service,
// palm://net.riverstonerelay.jihadBrowserMojo/notifications — see subscribeNotifications
// below. That is our service, not the stock daemon's, and it adds no adapter method and
// no YAP id, so neither frozen set above changes.
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
	//* Set when an adopted settings edit has not been drawn on the start page yet. The
	//* redraw is a NAVIGATION in this variant, so it has to be deferred — see
	//* adoptChromePrefs.
	this._chromePrefsStale = false;
}

//* Diagnostic, off by default — see setup(). Flip to true when changing the command row's
//* CSS; it prints one line with the row's and first button's geometry.
MainAssistant.prototype.LOG_COMMAND_ROW_GEOMETRY = false;

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
	// Dev-loop boot marker: "@DEV@" is replaced with a per-push stamp by
	// build/webos-oe/push-card-js.sh; a stale WebAppMgr JS cache shows the old stamp.
	Mojo.Log.error("[JIHAD-BOOT] stamp=%s", "@DEV@");
	// A real page, shipped in the package, so the card opens on rendered content
	// instead of an empty surface. Mojo.appPath is the app's own file:/// root.
	this.startUrl = (window.Mojo && Mojo.appPath ? Mojo.appPath : "") + "start.html";

	this.setupWebView();
	this.setupChrome();
	this.setupAppMenu();
	this.setupCommandMenu();
	this.bindHandlers();
	this.bindFindHandlers();
	this.listen();
	this.subscribeNotifications();
	// After the scene renders and the menu widget exists.
	// Command-row geometry probe: OFF by default. It was a diagnostic for the row rendering
	// empty (the .palm-menu-fade block pushing statically-positioned items out of a clipped
	// box), that bug is fixed, and it logged at ERROR level on EVERY card launch. Kept because
	// the row's layout is overridden in CSS and has broken twice; turn it on with
	//     luna-send -n 1 -a com.palm.configurator palm://com.palm.systemmanager/... (or just
	// flip this flag) when touching stylesheets/jihad-browser.css.
	if (MainAssistant.prototype.LOG_COMMAND_ROW_GEOMETRY) {
		this.controller.window.setTimeout(this.logCommandRowGeometry.bind(this), 3000);
	}
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
		attributes.url = JihadUrl.normalize(this.params.url) || this.startPageUrl();
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

// App menu (swipe down from the app name): a single "Settings" entry that opens
// about:preferences through the ordinary openUrl path, matching Enyo's app-menu entry and
// Mochi's overflow-menu entry (cavekit-preferences-ui R4). Added 2026-08-15 when the product
// owner reversed the 2026-08-05 "no settings icon in mojo" direction. A TEXT app-menu item rather
// than a command-row icon: the command row is circular icons only and already crowded, and the
// app menu is exactly where Enyo puts Settings. No new adapter call — settingsUrl() carries the
// card's current home + shortcuts in the fragment so the page opens on this card's values.
MainAssistant.prototype.setupAppMenu = function () {
	this.appMenuModel = {
		visible: true,
		items: [
			{ label: $L("Settings"), command: "jihad-settings" }
		]
	};
	this.controller.setupWidget(Mojo.Menu.appMenu, { omitDefaultItems: true }, this.appMenuModel);
};

MainAssistant.prototype.setupCommandMenu = function () {
	// The always-visible bottom bar: back / forward / (stop | reload) / new card /
	// history / share. The empty items are Mojo's spacers, which centre the group.
	// Icons are the framework's own menu-icon set (images/menu-icon-<name>.png);
	// history and share have no stock icon, so they carry an app-shipped one via
	// iconPath, which the menu widget turns into the button's background-image.
	this.backItem = {icon: "back", command: "jihad-back", disabled: true};
	this.forwardItem = {icon: "forward", command: "jihad-forward", disabled: true};
	// Home: immediately right of forward, goes to the page JihadChromePrefs holds
	// (default https://start.duckduckgo.com/). Never disabled — unlike back/forward
	// it does not depend on page history.
	this.homeItem = {iconPath: "images/menu-icon-home.png", command: "jihad-home"};
	this.reloadItem = {icon: "refresh", command: "jihad-reload"};
	this.stopItem = {icon: "stop", command: "jihad-stop"};
	this.newCardItem = {icon: "new", command: "jihad-new-card"};
	this.historyItem = {iconPath: "images/menu-icon-history.png", command: "jihad-history"};
	this.shareItem = {iconPath: "images/menu-icon-share.png", command: "jihad-share"};
	// The row scrolls now (see the jihad-cmdmenu rules), so find lives here with the rest of
	// the controls rather than being tucked into the app menu for want of space. An icon, not
	// a label: a text item is the one thing in this row that cannot be a circle, and the row
	// reads as a set of round controls.
	this.findItem = {iconPath: "images/menu-icon-find.png", command: "jihad-find"};
	this.commandMenuModel = {visible: true, items: []};
	this.syncCommandMenu();
	// jihad-cmdmenu carries the horizontal-scroll rules in stylesheets/jihad-browser.css, so
	// the row can hold as many controls as the Enyo and Mochi shells rather than overflowing.
	this.controller.setupWidget(Mojo.Menu.commandMenu, {menuClass: "no-fade jihad-cmdmenu"},
		this.commandMenuModel);
	// Mojo.Menu.commandMenu is a key into the scene's widget setups, NOT an element
	// id — there is nothing to look up with controller.get() — so track readiness
	// here to know when modelChanged() has a widget to notify.
	this.commandMenuReady = true;

	// ── find-in-page ────────────────────────────────────────────────────────────────────────
	// Opened from the command row (see findItem above), which scrolls horizontally now.
	this.controller.setupWidget("jihad-find-input",
		{hintText: "Find in page", multiline: false, enterSubmits: true,
		 changeOnKeyPress: true, focusMode: Mojo.Widget.focusSelectMode},
		{});
	this.controller.setupWidget("jihad-find-next", {type: Mojo.Widget.defaultButton}, {label: "Next"});
	this.controller.setupWidget("jihad-find-done", {type: Mojo.Widget.defaultButton}, {label: "Done"});
};

//* Show the find bar and put the cursor in it.
MainAssistant.prototype.showFind = function () {
	var bar = this.controller.get("jihad-find");
	if (!bar) { return; }
	bar.style.display = "";
	var input = this.controller.get("jihad-find-input");
	if (input && input.mojo && input.mojo.focus) { input.mojo.focus(); }
};

MainAssistant.prototype.hideFind = function () {
	var bar = this.controller.get("jihad-find");
	if (bar) { bar.style.display = "none"; }
};

//* Issue the search. The engine's finder continues from the current match, so pressing Next
//* with the same text is what "find again" means — there is no separate command, and the
//* frozen findInPage(string) carries no direction (backwards is not reachable from the app,
//* see cavekit-ui-shell.md R4).
MainAssistant.prototype.runFind = function () {
	var input = this.controller.get("jihad-find-input");
	var text = input && input.mojo && input.mojo.getValue ? input.mojo.getValue() : "";
	if (text && text.length >= 2) {
		this.callBrowserAdapter("findInPage", [text]);
	}
};

//* One-shot geometry probe for the command row, in the spirit of the Mochi address-bar fix:
//* a card-WebKit layout bug is invisible to inspection and obvious to arithmetic. If
//* scrollWidth exceeds clientWidth the row is genuinely overflowing AND scrollable, which is
//* the whole point of the jihad-cmdmenu rules; if the items were still absolutely positioned
//* they would overlap instead and scrollWidth would equal clientWidth.
//* One compact line about the command row, at ERROR level: Mojo.Log.warn is BELOW
//* this card's log threshold and never reaches /var/log/messages, which is why this
//* probe looked like it produced no output for weeks. Kept because the row's layout
//* is overridden in stylesheets/jihad-browser.css and has broken twice — once
//* running off the right edge, once clipped out of sight by the fade block.
MainAssistant.prototype.logCommandRowGeometry = function () {
	try {
		var row = document.querySelector(".palm-menu.command-menu.jihad-cmdmenu");
		if (!row) { Mojo.Log.error("[JIHAD-CMDGEOM] command row not in the document"); return; }
		var s = window.getComputedStyle ? window.getComputedStyle(row) : null;
		var r = row.getBoundingClientRect ? row.getBoundingClientRect() : null;
		var btns = row.querySelectorAll(".palm-menu-button");
		var b0 = btns.length && btns[0].getBoundingClientRect ? btns[0].getBoundingClientRect() : null;
		// A button top that is not the row top means the items have fallen out of the
		// row's box and overflow-y: hidden is clipping them (the 2026-08-05 bug).
		Mojo.Log.error("[JIHAD-CMDGEOM] buttons=" + btns.length +
			" row=" + (r ? Math.round(r.top) + "+" + Math.round(r.height) : "?") +
			" btn0=" + (b0 ? Math.round(b0.left) + "," + Math.round(b0.top) + " " +
				Math.round(b0.width) + "x" + Math.round(b0.height) : "?") +
			" client=" + row.clientWidth + " scroll=" + row.scrollWidth +
			" overflowX=" + (s && s.overflowX));
	} catch (e) { Mojo.Log.error("[JIHAD-CMDGEOM] " + e); }
};

MainAssistant.prototype.bindFindHandlers = function () {
	var next = this.controller.get("jihad-find-next");
	var done = this.controller.get("jihad-find-done");
	var input = this.controller.get("jihad-find-input");
	this.handleFindNext = this.runFind.bind(this);
	this.handleFindDone = this.hideFind.bind(this);
	if (next)  { next.addEventListener(Mojo.Event.tap, this.handleFindNext); }
	if (done)  { done.addEventListener(Mojo.Event.tap, this.handleFindDone); }
	// enterSubmits on the field means Enter fires propertyChange with the committed value.
	if (input) { input.addEventListener(Mojo.Event.propertyChange, this.handleFindNext); }
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

//* `result` is what a scene we pushed handed back when it popped — the history
//* scene returns {url} for the entry the user tapped.
MainAssistant.prototype.activate = function (result) {
	if (result && result.url) {
		this.openUrl(result.url);
	}
	if (!this.checkedEngine) {
		this.checkedEngine = true;
		this.verifyEngineRouting();
	}
};

MainAssistant.prototype.deactivate = function () {};

//* THE DAEMON'S NON-BLOCKING MESSAGE CHANNEL (cavekit-gre-widgets.md R5).
//*
//* One subscription to THIS VARIANT'S OWN Luna service, opened at setup() and left open for
//* the life of the scene; every informational line the daemon raises arrives on it and becomes
//* a transient toast (../models/jihad-toast.js). Statements — "Cookies cleared.", "<add-on>
//* installed." — used to have no route but msgDialog*, which stops the render daemon until
//* this card answers; a message with no possible answer must not be able to do that.
//*
//* controller.serviceRequest rather than a bare Mojo.Service.Request: the controller owns the
//* request's lifetime and cancels the subscription when the scene is popped, which is exactly
//* the lifetime this subscription wants.
//*
//* CONTRACT: this is the first palm:// URI this variant uses, and it is our own per-variant
//* service name — never com.palm.browserServer, which is the stock daemon we coexist with and
//* whose clearCookies would clear the wrong browser. No adapter method and no YAP id is added;
//* the frozen sets in cavekit-mojo-ui.md R3 and cavekit-ipc-contract.md R1 are untouched.
MainAssistant.prototype.subscribeNotifications = function () {
	JihadToast.attach(this.controller.window);
	// Off device (Mojo.Host.browser) there is no Luna bus; serviceRequest would fail
	// asynchronously and log noise on every desktop preview run.
	if (!window.PalmSystem) {
		return;
	}
	this.notificationRequest = this.controller.serviceRequest(
		"palm://net.riverstonerelay.jihadBrowserMojo", {
			method: "notifications",
			// subscribe goes in the request PARAMETERS — that is what travels to the daemon and
			// what makes Mojo hold the request open for repeated onSuccess calls. resubscribe
			// re-arms it if the daemon restarts, so a card does not go permanently deaf.
			parameters: {subscribe: true},
			resubscribe: true,
			onSuccess: this.handleNotification.bind(this),
			onFailure: function (response) {
				// Mojo.Log.error, not .warn: warn does not reach the device log in this
				// framework build, and a subscription that never opened is exactly the thing
				// somebody will be looking for when no toast ever appears.
				Mojo.Log.error("[Jihad] notifications subscription failed: %j", response);
			}
		});
};

//* Every reply on that subscription: the immediate {"returnValue":true,"subscribed":true}
//* handshake as well as each later push. Only a payload carrying `text` is a message — the
//* handshake is not one, and showing it would put an empty toast up at every card launch.
MainAssistant.prototype.handleNotification = function (response) {
	if (!response || !response.text) {
		return;
	}
	JihadToast.show(response.category, response.text);
};

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
	// The toast holds a node in this scene's document and a pending timer in its window; both
	// have to go with the scene or the timer fires against a torn-down document.
	JihadToast.detach();
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
	// startPageUrl(), not the bare startUrl: the start page is a DOCUMENT rendered by the
	// engine and gets its shortcut list from the url fragment, so retrying with the bare url
	// would silently drop the user's links back to the built-in defaults.
	this.openUrl(this.failedUrl || this.url || this.startPageUrl());
};

//* Menu commands and the back gesture (Mojo's chain of command).
MainAssistant.prototype.handleCommand = function (event) {
	if (event.type === Mojo.Event.command) {
		switch (event.command) {
		case "jihad-back":
			this.callBrowserAdapter("goBack");
			break;
		case "jihad-home":
			this.openUrl(JihadChromePrefs.loadHome());
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
		case "jihad-new-card":
			this.openNewCard();
			break;
		case "jihad-history":
			this.controller.stageController.pushScene("history", {history: JihadHistory.all()});
			break;
		case "jihad-share":
			this.shareCurrentPage();
			break;
		case "jihad-find":
			this.showFind();
			break;
		case "jihad-settings":
			this.openUrl(JihadChromePrefs.settingsUrl());
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
//* New card: a second STAGE of this same app (the Enyo shell's
//* enyo.windows.openWindow equivalent), so the user gets a real second browser card
//* in the card stack rather than another scene on this one's stack.
MainAssistant.prototype.openNewCard = function (url) {
	var name = "jihad-card-" + (new Date()).getTime();
	var params = url ? {url: url} : {};
	try {
		Mojo.Controller.getAppController().createStageWithCallback(
			{name: name, lightweight: false},
			function (stageController) {
				stageController.pushScene("main", params);
			});
	} catch (e) {
		Mojo.Log.logException(e, "MainAssistant#openNewCard");
	}
};

//* Share the current page by handing it to the mail app, exactly as the Enyo
//* shell's ShareLinkDialog does (same launch id and params) — no new adapter or
//* service surface of our own.
MainAssistant.prototype.shareCurrentPage = function () {
	if (this.isStartPage(this.url)) {
		return;
	}
	var body = $L("Here's a website I think you'll like: ") + this.url;
	this.controller.serviceRequest("palm://com.palm.applicationManager", {
		method: "launch",
		parameters: {
			id: "com.palm.app.email",
			params: {summary: $L("Check out this web page..."), text: body}
		},
		onFailure: function (response) {
			Mojo.Log.warn("[Jihad] share failed: %j", response);
		}
	});
};

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
	this.recordHistory();
	this.syncChrome();
	// After this.url and the chrome are up to date — adoptChromePrefs reads this.url as the
	// COMMITTED url and may navigate.
	this.adoptChromePrefs();
};

//* Record the committed page in this variant's own history store. Never the
//* app-shipped start page (it is chrome, not a visited site), and never an
//* unchanged url twice in a row.
MainAssistant.prototype.recordHistory = function () {
	if (!this.url || this.isStartPage(this.url) || this.url === this._historyUrl) {
		return;
	}
	this._historyUrl = this.url;
	JihadHistory.add(this.url, this.title);
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
	// BOTH committed-url events, not just titleUrlChange. The Enyo and Mochi shells hook only
	// their title+url event, because that is the one a same-document fragment rewrite was
	// device-observed to produce (app/source/ChromePrefs.js header, 2026-08-06). This widget
	// dispatches webViewUrlChanged separately and WHICH of the two carries a fragment-only
	// change here is not verified — no device. adoptFromUrl is idempotent (a second call sees
	// no change and returns false), so hooking both costs one comparison and removes the guess.
	this.adoptChromePrefs();
};

//* Reflect the committed url + reported title, and the history state, in the UI.
MainAssistant.prototype.syncChrome = function () {
	var isStart = this.isStartPage(this.url);
	// No title row: the card's own title bar and the address field already say what
	// this is, and a third line of the same text just ate vertical space.
	// The start page is app-shipped chrome, so its file:/// path is not shown; the
	// address bar falls back to its hint, as in both sibling variants.
	this.addressModel.value = isStart ? "" : (this.url || "");
	this.controller.modelChanged(this.addressModel);
	this.syncCommandMenu();
};

//* Every committed url passes here (cavekit-mojo-ui.md R7, cavekit-preferences-ui.md R5).
//* When it is the settings page carrying an edited payload, adopt it: the page publishes an
//* edit by rewriting its own fragment, and the url the card already receives is the only
//* channel between the two. The GATE and the url allowlist are JihadChromePrefs' — path-only,
//* never scheme-only — and are deliberately not second-guessed here; see adoptFromUrl for the
//* about:blank attack the path test is there to stop.
//*
//* RE-RENDERING IS NOT THE JOB IT IS IN THE OTHER TWO SHELLS, and the obvious port of it is
//* DEAD CODE. Enyo and Mochi rebuild the start page IN PLACE (BrowserApp.noteUrlForChromePrefs
//* -> startPage.buildLinks; JihadBrowser.pageInfoChanged -> buildStartLinks) because their
//* start page is card CHROME. Ours is a DOCUMENT whose shortcut list travels in the url
//* fragment, so the only way to redraw it is to re-open startPageUrl() — a NAVIGATION. And
//* "re-open it if it is showing" can never fire: the url that just passed the adopt gate IS
//* about:preferences, so the start page is by construction not what is showing.
//* The redraw is therefore DEFERRED and spent the next time the start page commits. That is
//* also the case that actually breaks without it: a Back gesture lands on the start page's own
//* history entry, which still carries the STALE `#links=` fragment it was opened with, so the
//* user sees the edit they just made silently missing.
//*
//* The flag is cleared BEFORE the re-open, so this cannot loop even if the engine hands the
//* fragment back re-encoded and the fresh url never compares equal to the committed one. That
//* is why the condition is a one-shot flag and not a fragment comparison.
//*
//* Called AFTER this.url is updated, deliberately: the test below reads this.url as the
//* COMMITTED url. Called before it — which is where both sibling shells call theirs, because
//* neither has anything to navigate — this.url would still be the PREVIOUS page, and a card
//* that reached the settings page from the start page would immediately navigate itself back
//* off the settings page.
//*
//* NOT DEVICE-VERIFIED: no device was available for this port. What IS verified is the code
//* path, the gate's premise (jihadAboutPreferences.js getURIFlags withholds
//* URI_SAFE_FOR_UNTRUSTED_CONTENT, so content cannot navigate into about:preferences) and
//* parity of the gate + allowlist with the Enyo shell.
MainAssistant.prototype.adoptChromePrefs = function () {
	if (JihadChromePrefs.adoptFromUrl(this.url)) {
		this._chromePrefsStale = true;
	}
	if (this._chromePrefsStale && this.isStartPage(this.url)) {
		this._chromePrefsStale = false;
		this.openUrl(this.startPageUrl());
	}
};

//* The start page is a DOCUMENT rendered by the engine, so it cannot read the
//* card's localStorage the way the Enyo and Mochi start pages (which are app
//* chrome) read theirs. The shortcut list therefore travels in the url fragment,
//* which start.html parses. A fragment costs no extra request and keeps the page
//* a plain packaged file that talks to nothing.
MainAssistant.prototype.startPageUrl = function () {
	var links = JihadChromePrefs.loadLinks();
	return this.startUrl + "#links=" + encodeURIComponent(JSON.stringify(links));
};

//* Compare without the fragment: startPageUrl() appends one, and the engine reports
//* the committed url back with it, so a bare === would stop recognising our own
//* start page (which is what keeps it out of the address bar).
MainAssistant.prototype.isStartPage = function (url) {
	if (!url) { return true; }
	return String(url).split("#")[0] === this.startUrl;
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
	this.shareItem.disabled = this.isStartPage(this.url);   // nothing to share yet
	this.commandMenuModel.items = [
		{},
		this.backItem,
		this.forwardItem,
		this.homeItem,
		this.loading ? this.stopItem : this.reloadItem,
		this.newCardItem,
		this.historyItem,
		this.shareItem,
		this.findItem,
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
