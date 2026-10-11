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
//     Mojo : {copy, cut, findInPage, goBack, goForward, insertStringAtCursor, pageFocused,
//             reloadPage, selectAll, stopLoad}
//     Enyo : those and more (dialogs, images, printing, privacy switches)
// — no additions, no renames. (copy, cut, selectAll and insertStringAtCursor arrived in both
// shells on 2026-10-05 with the long-press menu; all four were already adapter methods.) This variant uses no palm://com.palm.browserServer/*
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
		// The same app menu as the Enyo and Mochi shells (and Atlas's, minus what this
		// browser has not got): Find on Page, Settings, Print.
		items: [
			{ label: $L("Find on Page"), command: "jihad-find" },
			{ label: $L("Settings"), command: "jihad-settings" },
			{ label: $L("Print"), command: "jihad-print" }
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
	// (default https://lite.duckduckgo.com/lite/). Never disabled — unlike back/forward
	// it does not depend on page history.
	this.homeItem = {iconPath: "images/menu-icon-home.png", command: "jihad-home"};
	this.newCardItem = {icon: "new", command: "jihad-new-card"};
	// Share and Bookmarks open their submenus (setupSubmenus): Add Bookmark · Share Link · Add to
	// Launcher, and Bookmarks · History · Downloads — the upper-right menu of the other shells.
	this.libraryItem = {iconPath: "images/menu-icon-bookmark.png", submenu: "jihad-library-menu"};
	this.shareItem = {iconPath: "images/menu-icon-share.png", submenu: "jihad-share-menu"};
	// The row scrolls now (see the jihad-cmdmenu rules), so find lives here with the rest of
	// the controls rather than being tucked into the app menu for want of space. An icon, not
	// a label: a text item is the one thing in this row that cannot be a circle, and the row
	// reads as a set of round controls.
	this.findItem = {iconPath: "images/menu-icon-find.png", command: "jihad-find"};
	this.setupSubmenus();
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
	this.applyHeaderHeight();
	var input = this.controller.get("jihad-find-input");
	if (input && input.mojo && input.mojo.focus) { input.mojo.focus(); }
};

MainAssistant.prototype.hideFind = function () {
	var bar = this.controller.get("jihad-find");
	if (bar) { bar.style.display = "none"; }
	this.applyHeaderHeight();
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
	this.handleAddressTap = this.handleAddressTap.bind(this);
	this.handleAddressSubmit = this.handleAddressSubmit.bind(this);
	this.handleRetry = this.handleRetry.bind(this);
	this.handleReloadTap = this.handleReloadTap.bind(this);
	this.handleReloadDown = this.handleReloadDown.bind(this);
	this.handleReloadUp = this.handleReloadUp.bind(this);
	this.handleDownloadFinished = this.handleDownloadFinished.bind(this);
	this.handleMimeHandoff = this.handleMimeHandoff.bind(this);
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
		// A hand-off is a DOWNLOAD the engine has already started (it saves the file itself
		// and reports it through the download callbacks — see hookDownloads), not a page that
		// failed: it used to put up "This content type can't be displayed" over a transfer
		// that was in fact under way.
		[Mojo.Event.webViewMimeHandoff, this.handleMimeHandoff],
		[Mojo.Event.webViewDownloadFinished, this.handleDownloadFinished],
		[Mojo.Event.webViewServerDisconnect, this.handleServerDisconnect]
	];
};

MainAssistant.prototype.listen = function () {
	var view = this.controller.get(this.WEB_VIEW_ID);
	var events = this.webViewEvents();
	for (var i = 0; i < events.length; i++) {
		Mojo.Event.listen(view, events[i][0], events[i][1]);
	}
	Mojo.Event.listen(this.controller.get(this.ADDRESS_ID), Mojo.Event.tap, this.handleAddressTap, true);
	Mojo.Event.listen(this.controller.get(this.ADDRESS_ID), Mojo.Event.propertyChange,
		this.handleAddressSubmit);
	Mojo.Event.listen(this.controller.get(this.RETRY_ID), Mojo.Event.tap, this.handleRetry);
	var rb = this.controller.get("jihad-reload");
	if (rb) {
		Mojo.Event.listen(rb, Mojo.Event.tap, this.handleReloadTap);
		rb.addEventListener("mousedown", this.handleReloadDown, false);
		rb.addEventListener("mouseup", this.handleReloadUp, false);
		rb.addEventListener("mouseout", this.handleReloadUp, false);
	}
};

//* `result` is what a scene we pushed handed back when it popped — the history
//* scene returns {url} for the entry the user tapped.
MainAssistant.prototype.activate = function (result) {
	this.adoptViewportModel();
	if (result && result.url) {
		this.openUrl(result.url);
	}
	this.hookDownloads();
	if (!this.checkedEngine) {
		this.checkedEngine = true;
		this.verifyEngineRouting();
	}
};

MainAssistant.prototype.deactivate = function () {};

// --- the WebView as a screen-sized view (2026-10-07) ----------------------------------------
// Mojo.Widget.WebView was written for an adapter that is PAGE-sized: it sizes the plugin node
// to page x scale, pans by scrolling the scene, and pinches by resizing that node and scrolling
// the scene again. This browser's adapter is the Enyo one, which is VIEWPORT-sized and pans and
// zooms by itself (as it does in the Enyo and Mochi shells, where pinch is smooth and sharp).
// The two models met in every pinch: a grey card after pinching in and out, pages shrinking to
// a stripe (owner reports 2026-10-06/07). So this shell keeps the node the size of the visible
// area and hands every pinch step to the adapter, exactly as the other shells' views do:
//   _setDimensions   — whatever the widget asks (page size, a pinch step, a resize), the node
//                      stays the visible area, and the adapter fit-zooms the page into it;
//   pageDimensions / metaViewportSet — recorded, nothing resized (the adapter keeps the page's
//                      viewport limits itself);
//   gesture events   — the widget's own handlers are replaced by ones that pass start / change /
//                      end to the adapter in the node's own pixels, and keep a drag from
//                      scrolling the scene while two fingers are down.
// The node is the WebView's <object>; the widget instance is element._mojoController.assistant.
MainAssistant.prototype.adoptViewportModel = function () {
	var view = this.controller.get(this.WEB_VIEW_ID);
	var wa = view && view._mojoController && view._mojoController.assistant;
	if (!wa || wa._jihadViewport) {
		return;
	}
	wa._jihadViewport = true;
	var self = this, doc = this.controller.document;
	wa._setDimensions = function () { self.fitWebViewToScreen(); };
	wa.pageDimensions = function (width, height) {
		this.pageGotSize = width > 0 && height > 0;
		this.currPageWidth = width;
		this.currPageHeight = height;
		self.fitWebViewToScreen();
	};
	wa.metaViewportSet = function (initialScale, minimumScale, maximumScale, width, height, userScalable) {
		this.metaViewport = new Mojo.WebView.MetaViewport(initialScale, minimumScale, maximumScale,
			width, height, userScalable);
	};
	var at = function (event) {
		var node = wa.adapter, r = node.getBoundingClientRect();
		return {x: Math.round(event.centerX - r.left), y: Math.round(event.centerY - r.top)};
	};
	var start = function (event) {
		event.stop();
		if (!wa.pageGotSize) {
			return;
		}
		if (wa._removeElementHighlight) { wa._removeElementHighlight(); }
		wa.inPinchZoom = true;      // the widget's drag handler stops the scene scrolling on this
		wa.pageManipulated = true;
		var c = at(event);
		wa.adapter.gestureStart(c.x, c.y, event.scale, event.rotation || 0, c.x, c.y);
	};
	var change = function (event) {
		event.stop();
		if (!wa.inPinchZoom) {
			return;
		}
		var c = at(event);
		wa.adapter.gestureChange(c.x, c.y, event.scale, event.rotation || 0, c.x, c.y);
	};
	var end = function (event) {
		event.stop();
		if (!wa.inPinchZoom) {
			return;
		}
		wa.inPinchZoom = false;
		var c = at(event);
		wa.adapter.gestureEnd(c.x, c.y, event.scale, event.rotation || 0, c.x, c.y);
	};
	// Off with the widget's (it listens on the document from its _activate), on with ours — and
	// ours under the widget's own handler names, so its later _activate/_deactivate add and
	// remove these instead.
	Mojo.Event.stopListening(doc, "gesturestart", wa._gestureStartHandler, false);
	Mojo.Event.stopListening(doc, "gesturechange", wa._gestureChangeHandler, false);
	Mojo.Event.stopListening(doc, "gestureend", wa._gestureEndHandler, false);
	wa._gestureStartHandler = start;
	wa._gestureChangeHandler = change;
	wa._gestureEndHandler = end;
	Mojo.Event.listen(doc, "gesturestart", start, false);
	Mojo.Event.listen(doc, "gesturechange", change, false);
	Mojo.Event.listen(doc, "gestureend", end, false);
	// A drag on the page belongs to the adapter, which pans the page itself. Left to the scene
	// scroller, the drag is CLAIMED there and the finger's moves are cancelled before they reach
	// the plugin, so nothing pans (owner 2026-10-07: "scrolling does not work"). Stopping the
	// drag at the WebView keeps the scroller out of it.
	var dragStart = function (event) {
		event.stop();
	};
	Mojo.Event.stopListening(wa.controller.element, Mojo.Event.dragStart, wa._dragStartHandler, false);
	wa._dragStartHandler = dragStart;
	Mojo.Event.listen(wa.controller.element, Mojo.Event.dragStart, dragStart, false);
	// A tap is the adapter's too: it takes the finger's own tap (as in the Enyo and Mochi shells)
	// and the widget's single-tap handler sent a SECOND click for every one (two identical
	// clickAt per tap in the daemon log, 2026-10-08). That handler stands down. Its plain tap
	// handler stays: it clicks nothing, and it keeps the tap that linkClicked reports with
	// webViewLinkClicked (Codex review 2026-10-08). The double-tap stays too (it is what asks
	// the engine for the block to zoom to).
	Mojo.Event.stopListening(wa.adapter, "singletap", wa._singleTapHandler, false);
	wa._singleTapHandler = function () {};
	// The widget remembers each page's zoom at load start and puts it back at load stop. It
	// keeps that zoom in currScale, which nothing maintains under this model (it stays 1), so
	// a reload, Back/Forward or a revisit of any page whose fit is not 1 was forced to 1.0
	// behind the adapter's back - drawn upscaled, the right and bottom of the page out of
	// reach (inspector 2026-10-08, P1). The adapter keeps the page's zoom itself.
	wa._saveCurrentPagePosition = function () {};
	wa._restoreCurrentPagePosition = function () {};
	// The adapter reports every scroll of the page to the widget it calls back (scrolledTo with
	// the negated scroll); the top strip follows it.
	wa.scrolledTo = function (x, y) { self.onPageScrolled(-y); };
	this.fitWebViewToScreen();
	this.applyHeaderHeight();
	if (!this._fitOnResize) {
		this._fitOnResize = function () { self.fitWebViewToScreen(); self.applyHeaderHeight(); };
		this.controller.window.addEventListener("resize", this._fitOnResize, false);
	}
	Mojo.Log.error("[Jihad] WebView in the screen-sized model");
};

//* The adapter keeps a header strip as tall as the floating top strip above the page, so the
//* page starts below it and the strip can scroll away with it. Re-sent whenever the strip's
//* height changes (find bar, rotation); the error panel is pushed below the strip too.
MainAssistant.prototype.applyHeaderHeight = function () {
	var bar = this.controller.get("jihad-topbar"), view = this.controller.get(this.WEB_VIEW_ID);
	var wa = view && view._mojoController && view._mojoController.assistant;
	if (!bar || !wa || !wa.adapter) {
		return;
	}
	var h = bar.offsetHeight;
	var err = this.controller.get("jihad-error");
	if (err) { err.style.marginTop = h + "px"; }
	if (h === this._headerHeight) {
		return;
	}
	try {
		wa.adapter.setHeaderHeight(h);
		this._headerHeight = h;
	} catch (e) {
		Mojo.Log.error("[Jihad] setHeaderHeight not ready: %s", e);
	}
};

//* The page's scroll (from the adapter, header included): the strip slides up with the first
//* `header` pixels of it and stays away below that; back at the top it is whole again.
MainAssistant.prototype.onPageScrolled = function (y) {
	var bar = this.controller.get("jihad-topbar");
	if (!bar) {
		return;
	}
	var h = this._headerHeight || bar.offsetHeight, off = Math.max(0, Math.min(h, y || 0));
	if (off === this._topbarOff) {
		return;
	}
	this._topbarOff = off;
	bar.style.webkitTransform = off ? "translate3d(0,-" + off + "px,0)" : "";
};

//* The node is the whole card below the status bar: the top strip floats over its head and
//* the command row over its foot. The scene then has nothing to scroll, so a drag reaches the
//* adapter, which pans the page itself.
MainAssistant.prototype.fitWebViewToScreen = function () {
	var view = this.controller.get(this.WEB_VIEW_ID);
	var wa = view && view._mojoController && view._mojoController.assistant;
	var node = wa && wa.adapter, win = this.controller.window;
	if (!node) {
		return;
	}
	var top = Math.max(0, Math.round(node.getBoundingClientRect().top));
	var w = win.innerWidth, h = Math.max(100, win.innerHeight - top);
	if (node.style.width !== w + "px") { node.style.width = w + "px"; }
	if (node.style.height !== h + "px") { node.style.height = h + "px"; }
	if (wa.sceneScroller && wa.sceneScroller.scrollTo) {
		var p = wa.sceneScroller.getScrollPosition && wa.sceneScroller.getScrollPosition();
		if (p && (p.left || p.top)) { wa.sceneScroller.scrollTo(0, 0); }
	}
};

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
	// The lightweight theme ("persona") in force, once at start; changes come on the
	// notification channel as category "theme".
	this.controller.serviceRequest("palm://net.riverstonerelay.jihadBrowserMojo", {
		method: "getTheme",
		parameters: {},
		onSuccess: function (r) { this.applyTheme(r && r.theme ? r.theme : null); }.bind(this),
		onFailure: function () {}
	});
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
//* Paint the lightweight theme ("persona") on the toolbar, or take it off (null): the header
//* image anchored top right behind the bar, accentcolor where it does not reach, textcolor on
//* the text — as Firefox 52 / Pale Moon / Basilisk drew one. The image is the file the engine
//* persisted in the profile (inside this app's own directory, so the card may load it), or the
//* site's URL until then. Colours are kept to CSS colour syntax.
MainAssistant.prototype.applyTheme = function (theme) {
	var bar = this.controller.get("jihad-toolbar");
	if (!bar) {
		return;
	}
	this.theme = theme || null;
	if (!theme || typeof theme !== "object") {
		bar.style.background = ""; bar.style.color = "";
		this.themeCommandRow(false, "");
		return;
	}
	var color = function (c) { return (typeof c === "string" && /^[#a-zA-Z0-9(),.%\s-]{1,40}$/.test(c)) ? c : ""; };
	var url = function (u) { return (typeof u === "string" && /^(https?:\/\/|file:\/\/\/)[^\s"'()]+$/.test(u)) ? u : ""; };
	var accent = color(theme.accentcolor), text = color(theme.textcolor);
	var bg = this.themeBackground(theme, url, color);
	bar.style.background = "";
	bar.style.backgroundColor = bg.color;
	bar.style.backgroundImage = bg.image;
	bar.style.backgroundPosition = bg.position;
	bar.style.backgroundRepeat = bg.repeat;
	bar.style.backgroundSize = bg.size;
	bar.style.color = text;
	this.themeCommandRow(this.isDarkColor(text), accent);
};

//* The bar's background for a Persona, as CSS longhands with one entry per picture, top first:
//* the header (a wallpaper-sized one fitted to the bar, as the Persona component marks it with
//* headerFit; one made for a toolbar at its own size, top right as Firefox anchors it, unless the
//* theme places it), then a layered theme's other pictures (cavekit-persona-formats.md R2), all
//* over the accent colour. Positions and tilings are Firefox's own words only.
MainAssistant.prototype.themeBackground = function (theme, url, color) {
	var POS = /^(left|center|right|top|bottom)( (left|center|right|top|bottom))?$/, REP = /^(no-repeat|repeat|repeat-x|repeat-y)$/;
	var img = [], pos = [], rep = [], size = [], i, u;
	var header = url(theme.headerURL), fit = !!header && theme.headerFit === "cover";
	if (header) {
		img.push("url(" + header + ")");
		pos.push(fit ? "center center" : (POS.test(theme.headerPosition) ? theme.headerPosition : "right top"));
		rep.push(REP.test(theme.headerRepeat) ? theme.headerRepeat : "no-repeat");
		size.push(fit ? "cover" : "auto");
	}
	var layers = (Object.prototype.toString.call(theme.layers) === "[object Array]") ? theme.layers.slice(0, 6) : [];   // parsed in another window: not this one's Array
	for (i = 0; i < layers.length; i++) {
		u = layers[i] && url(layers[i].url);
		if (!u) {
			continue;
		}
		img.push("url(" + u + ")");
		pos.push(POS.test(layers[i].position) ? layers[i].position : "right top");
		rep.push(REP.test(layers[i].repeat) ? layers[i].repeat : "no-repeat");
		size.push("auto");
	}
	return {color: color(theme.accentcolor), image: img.length ? img.join(", ") : "none",
		position: pos.join(", "), repeat: rep.join(", "), size: size.join(", ")};
};

//* Whether a CSS colour (a theme's textcolor, in any form the browser accepts) is dark: the
//* browser resolves it on a detached element. Unreadable → not dark.
MainAssistant.prototype.isDarkColor = function (c) {
	if (!c) {
		return false;
	}
	var doc = this.controller.document, d = doc.createElement("div"), m;
	d.style.color = c;
	doc.body.appendChild(d);
	m = /rgba?\((\d+),\s*(\d+),\s*(\d+)/.exec(this.controller.window.getComputedStyle(d, null).color || "");
	doc.body.removeChild(d);
	return !!m && (0.299 * m[1] + 0.587 * m[2] + 0.114 * m[3]) < 128;
};

//* The command row's glyphs: the framework's sprites (back, forward, refresh, stop, new) and
//* this shell's own (home, history, share, find), each a background-image sprite on the button.
//* With a dark theme text colour they switch to the ink set in images/dark/ (same file names;
//* the framework's five drawn from isis-browser's Apache-2.0 art), as Firefox 52 / Pale Moon /
//* Basilisk switch icon sets on the theme's text colour. The sprite position the framework
//* slides for the pressed state is left alone. The framework redraws the row when its model
//* changes, so updateCommandMenu re-applies.
MainAssistant.prototype.themeCommandRow = function (ink, accent) {
	// All of it is STYLESHEET, keyed off classes on <body> plus one generated colour rule: the
	// menu widget rebuilds the row's buttons whenever its model changes (every load start and
	// stop), and theming set on the buttons themselves was wiped by each rebuild and put back a
	// tick later — the circles flashed at every load (owner 2026-10-07). Rules apply to the
	// rebuilt buttons as they appear. See "theme on the command row" in jihad-browser.css.
	// Without an accent colour the row keeps the framework's own dark circles, so it keeps their
	// light glyphs too: ink glyphs on those circles would not read (Codex r13 asked for ink there).
	var doc = this.controller.document, body = doc.body;
	body.className = body.className.replace(/\s*jihad-themed(-ink)?/g, "") +
		(accent ? " jihad-themed" + (ink ? " jihad-themed-ink" : "") : "");
	var css = doc.getElementById("jihad-theme-css");
	if (!css) {
		css = doc.createElement("style");
		css.id = "jihad-theme-css";
		doc.getElementsByTagName("head")[0].appendChild(css);
	}
	css.textContent = accent ? "body.jihad-themed .palm-menu.command-menu.jihad-cmdmenu .palm-menu-button { background-color: " + accent + "; }" : "";
};

MainAssistant.prototype.handleNotification = function (response) {
	if (!response || !response.text) {
		return;
	}
	// The settings page asking this card to clear the history list it keeps (this shell has
	// no bookmarks). The request and the sentence to show arrive together.
	if (response.category === "chrome-clear-history") {
		JihadHistory.clear();
	}
	if (response.category === "chrome-clear-bookmarks") {
		JihadBookmarks.clear();
	}
	// What the daemon says about printing belongs to the Print scene while it is up; with it
	// closed a job still running reports by toast, and a printer list nobody is looking at is
	// dropped (it is JSON, not a sentence).
	if (String(response.category).indexOf("print-") === 0) {
		if (typeof PrintAssistant !== "undefined" && PrintAssistant.current) {
			PrintAssistant.current.notification(response.category, response.text);
			return;
		}
		if (response.category === "print-printers") {
			return;
		}
	}
	// The long-press menu's traffic with the engine: what is under the finger, and where the
	// copied text was put. Neither is a sentence to show.
	if (response.category === "theme") {
		var theme = null;
		try { theme = JSON.parse(response.text); } catch (e) { theme = null; }
		this.applyTheme(theme);
		return;
	}
	if (response.category === "page-hold") {
		this.handlePageHold(response.text);
		return;
	}
	if (response.category === "page-copied") {
		this.handlePageCopied(response.text);
		return;
	}
	// What the engine says about a copy or a cut is broadcast to every card; it is shown by
	// the one that asked.
	if (response.category === "page-copy-empty" && !this.takeCopyAsked()) {
		return;
	}
	if ((response.category === "page-cut" || response.category === "page-cut-refused") && !this.takeCutAsked()) {
		return;
	}
	JihadToast.show(response.category, response.text);
};

// --- the padlock ---------------------------------------------------------------------------
//* On a page that came over TLS the address field carries a padlock at its left edge (drawn
//* as the field's background: the field is a framework widget and redraws its own children).
MainAssistant.prototype.updateLock = function () {
	var field = this.controller.get(this.ADDRESS_ID);
	this.secure = !this.loadFailed && /^https:\/\//i.test(this.url || "");
	if (field) {
		if (this.secure) {
			field.addClassName("jihad-secure");
		} else {
			field.removeClassName("jihad-secure");
		}
	}
};

//* A tap on the padlock (the left 40 px of a field that shows one) says whose certificate the
//* page was served with, instead of starting an edit. Asked of this browser's own daemon,
//* which answers this variant's own card only; pageFocused first, because the question goes
//* over Luna and cannot say which card is asking.
MainAssistant.prototype.handleAddressTap = function (event) {
	var field = this.controller.get(this.ADDRESS_ID), down = event && event.down, rect;
	if (!this.secure || !field || !down || down.pageX === undefined) {
		return;
	}
	rect = field.getBoundingClientRect();
	if (down.pageX - rect.left > 40) {
		return;
	}
	event.stop();
	this.callBrowserAdapter("pageFocused", [true]);
	this.controller.serviceRequest("palm://net.riverstonerelay.jihadBrowserMojo", {
		method: "pageCertificate",
		parameters: {},
		onSuccess: this.showPageCertificate.bind(this),
		onFailure: this.showPageCertificate.bind(this)
	});
};

MainAssistant.prototype.showPageCertificate = function (r) {
	var lines = [], c = r && r.connection, bits = [];
	function party(label, p) {
		var out = [], keys = ["commonname", "organization", "unit"], i;
		if (!p) {
			return;
		}
		for (i = 0; i < keys.length; i++) {
			if (p[keys[i]]) {
				out.push(String(p[keys[i]]));
			}
		}
		if (out.length) {
			lines.push(label + ": " + out.join(", "));
		}
	}
	if (r && r.returnValue && r.secure) {
		party($L("Issued to"), r.subject);
		party($L("Issued by"), r.issuer);
		if (r.startdate) { lines.push($L("Valid from") + ": " + r.startdate); }
		if (r.expiredate) { lines.push($L("Valid until") + ": " + r.expiredate); }
		if (r.serialNumber) { lines.push($L("Serial number") + ": " + r.serialNumber); }
		if (r.sha256) { lines.push($L("SHA-256 fingerprint") + ": " + r.sha256); }
		if (c) {
			if (c.protocol) { bits.push(String(c.protocol)); }
			if (c.cipher) { bits.push(String(c.cipher)); }
			if (c.keyLength) { bits.push(c.keyLength + $L("-bit keys")); }
		}
		if (bits.length) { lines.push($L("Connection") + ": " + bits.join(", ")); }
		// Whether the certificate was believed, and whether the page counts as secure.
		if (r.overridden || r.untrusted || r.mismatch || r.badTime) {
			lines.push((r.mismatch ? $L("This certificate was issued for a different site.") :
				r.badTime ? $L("This certificate is not valid at this time.") :
				$L("This certificate could not be verified.")) + (r.overridden ? " " + $L("You chose to trust it.") : ""));
		}
		if (r.broken) { lines.push($L("This page is not fully secure: part of it, or the connection itself, is not protected.")); }
	}
	this.controller.showAlertDialog({
		title: $L("Certificate"),
		// The dialog takes text, not markup: the certificate's own strings are shown as they are.
		message: lines.length ? lines.join("\n") : $L("No certificate information is available for this page."),
		allowHTMLMessage: false,
		choices: [{label: $L("OK"), value: "ok"}],
		onChoose: function () {}
	});
};

// --- long-press menu -----------------------------------------------------------------------
//* A finger held on the page. Mojo.Widget.WebView claims no hold, so every one goes to the
//* engine, which gives the page its contextmenu event, selects the word under the finger (or
//* focuses the text field) and says what it found: {editable, selected}. The menu is what a
//* desktop browser offers on a right-click. Every card of this variant hears the report; only
//* the one the user is in acts on it.
MainAssistant.prototype.handlePageHold = function (text) {
	var sc = this.controller.stageController, s = null, items = [];
	var active = sc.isActiveAndHasScenes ? sc.isActiveAndHasScenes() : sc.active;
	if (!active) {
		return;
	}
	try { s = JSON.parse(text); } catch (e) { s = null; }
	s = s || {};
	if (s.editable) {
		// A field that takes no edits (read-only, disabled) is for copying only.
		if (!s.readOnly) {
			items.push({label: $L("Paste"), command: "page-paste"});
		}
		if (s.selected) {
			items.push({label: $L("Copy"), command: "page-copy"});
			if (!s.readOnly) {
				items.push({label: $L("Cut"), command: "page-cut"});
			}
		}
		items.push({label: $L("Select All"), command: "page-select-all"});
	} else {
		if (s.selected) {
			items.push({label: $L("Copy"), command: "page-copy"});
		}
		items.push({label: $L("Select All"), command: "page-select-all"});
		if (this.canGoBack) {
			items.push({label: $L("Back"), command: "page-back"});
		}
		if (this.canGoForward) {
			items.push({label: $L("Forward"), command: "page-forward"});
		}
		items.push({label: $L("Reload"), command: "page-reload"});
		items.push({label: $L("Find on Page"), command: "page-find"});
	}
	this.controller.popupSubmenu({
		onChoose: this.handlePageMenuChoice.bind(this),
		items: items
	});
};

MainAssistant.prototype.handlePageMenuChoice = function (command) {
	switch (command) {
	case "page-select-all":
		this.callBrowserAdapter("selectAll");
		break;
	case "page-copy":
		// The text comes back by way of handlePageCopied. What the engine says about a copy
		// is broadcast to every card, so the card remembers that it asked.
		this.copyAsked = {at: Date.now(), cut: false};
		this.askCopy();
		break;
	case "page-cut":
		// A copy, and then — only once the text is on the clipboard — the adapter's cut,
		// which tells the engine to delete what it handed over (handlePageCopied).
		this.copyAsked = {at: Date.now(), cut: true};
		this.askCopy();
		break;
	case "page-paste":
		this.pasteIntoPage();
		break;
	case "page-back":
		this.callBrowserAdapter("goBack");
		break;
	case "page-forward":
		this.callBrowserAdapter("goForward");
		break;
	case "page-reload":
		this.hideError();
		this.callBrowserAdapter("reloadPage");
		break;
	case "page-find":
		this.showFind();
		break;
	}
};

//* Ask the engine to copy the selection. The WebView widget's copy(callback) hands its one
//* argument to the adapter as given, and the adapter takes one argument only when it is an
//* object: called with none, the widget passes `undefined`, the adapter answers "Bad
//* arguments" and nothing is copied (device 2026-10-06). So the call always carries a
//* function. The engine's yes/no arrives here; the text itself comes by handlePageCopied. A
//* no means nothing is coming: if the engine's own word on why ("Nothing is selected.", on the
//* notification channel, which may land before or after this) has not shown by then, the
//* card says so itself, so the user is not left waiting on a Copy that did nothing.
MainAssistant.prototype.askCopy = function () {
	var self = this, win = this.controller.window;
	this.callBrowserAdapter("copy", [function (ok) {
		if (ok) {
			return;
		}
		Mojo.Log.error("[Jihad] copy: the engine handed nothing over");
		win.setTimeout(function () {
			if (self.takeCopyAsked()) {
				JihadToast.show("info", $L("The text could not be copied."));
			}
		}, 1500);
	}]);
};

//* The copy this card asked for within the last five seconds, once.
MainAssistant.prototype.takeCopyAsked = function () {
	var a = this.copyAsked;
	this.copyAsked = null;
	return (a && Date.now() - a.at < 5000) ? a : null;
};

//* How a cut this card asked for ended ("Cut.", or why not): its to show, nobody else's.
MainAssistant.prototype.takeCutAsked = function () {
	var at = this.cutAsked;
	this.cutAsked = 0;
	return !!(at && Date.now() - at < 5000);
};

//* The engine has written the selected text where this card may load it: a one-call script in
//* the app's own clip/ directory (a card can load a local file from nowhere else, and the
//* clipboard is the card's to write). `name` is that file's name.
MainAssistant.prototype.handlePageCopied = function (name) {
	var win = this.controller.window, doc = this.controller.document, sc = this.controller.stageController;
	var script, asked = this.takeCopyAsked(), self = this, done = false;
	if (!asked || !/^copy-[0-9a-f-]+\.js$/.test(String(name))) {
		return;
	}
	function failed() {
		if (done) {
			return;
		}
		done = true;
		win.jihadClip = null;
		JihadToast.show("info", $L("The text could not be copied."));
	}
	script = doc.createElement("script");
	win.jihadClip = function (text) {
		if (done) {
			return;
		}
		done = true;
		win.jihadClip = null;
		try {
			sc.setClipboard(String(text), false);
		} catch (e) {
			JihadToast.show("info", $L("The text could not be copied."));
			return;
		}
		// True from this moment whatever the cut does; "Cut." replaces it if the engine
		// confirms the deletion (the "page-cut" notification).
		JihadToast.show("info", $L("Copied."));
		if (asked.cut) {
			self.cutAsked = Date.now();
			self.callBrowserAdapter("cut");
		}
	};
	script.onload = function () {
		if (script.parentNode) {
			script.parentNode.removeChild(script);
		}
		failed();                           // it loaded and never called: not a clip script
	};
	script.onerror = function () {
		if (script.parentNode) {
			script.parentNode.removeChild(script);
		}
		failed();
	};
	script.src = "clip/" + name;
	doc.body.appendChild(script);
};

//* `text` as the pieces a paste is sent in. One message to the engine holds 16 KB and a longer
//* string is dropped whole (the IPC's own limit), so each piece is at most 12000 bytes of
//* UTF-8, and no piece ends inside a surrogate pair.
MainAssistant.prototype.pasteChunks = function (inText) {
	var out = [], start = 0, bytes = 0, i, c, n;
	for (i = 0; i < inText.length; i++) {
		c = inText.charCodeAt(i);
		n = c < 0x80 ? 1 : (c < 0x800 ? 2 : 3);
		// A low surrogate stays with the high one before it.
		if (bytes + n > 12000 && !(c >= 0xDC00 && c <= 0xDFFF)) {
			out.push(inText.substring(start, i));
			start = i;
			bytes = 0;
		}
		bytes += n;
	}
	if (start < inText.length) {
		out.push(inText.substring(start));
	}
	return out;
};

//* Paste: the clipboard is read here (only a card can) — the system's paste into a throwaway
//* textarea — and typed into the page's focused field. The paste is not synchronous and
//* takes anything from a few milliseconds to more than 60 (measured in the Mochi shell, same
//* platform call), so the textarea is watched for up to 600 ms before the clipboard is
//* called empty.
MainAssistant.prototype.pasteIntoPage = function () {
	var doc = this.controller.document, win = this.controller.window, self = this, tries = 0;
	var ta = doc.createElement("textarea"), parts, i;
	ta.style.position = "absolute";
	ta.style.left = "-2000px";
	ta.style.top = "0";
	ta.value = "";
	doc.body.appendChild(ta);
	try {
		ta.select();
		if (win.PalmSystem && win.PalmSystem.paste) {
			win.PalmSystem.paste();
		} else {
			doc.execCommand("paste");
		}
	} catch (e) {
		Mojo.Log.error("[Jihad] paste: %s", e);
	}
	// Another app decides what is on the clipboard, so its size is bounded here.
	function look() {
		var text = ta.value;
		if (!text && ++tries < 15) {
			win.setTimeout(look, 40);
			return;
		}
		try { ta.blur(); } catch (e) {}
		if (ta.parentNode) {
			ta.parentNode.removeChild(ta);
		}
		if (!text) {
			JihadToast.show("info", $L("The clipboard is empty."));
		} else if (text.length > 262144) {
			JihadToast.show("info", $L("The clipboard holds too much text to paste here."));
		} else {
			parts = self.pasteChunks(String(text));
			for (i = 0; i < parts.length; i++) {
				self.callBrowserAdapter("insertStringAtCursor", [parts[i]]);
			}
		}
	}
	win.setTimeout(look, 20);
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
	if (this._fitOnResize) {
		this.controller.window.removeEventListener("resize", this._fitOnResize, false);
		this._fitOnResize = null;
	}
	var view = this.controller.get(this.WEB_VIEW_ID);
	var events = this.webViewEvents();
	for (var i = 0; i < events.length; i++) {
		Mojo.Event.stopListening(view, events[i][0], events[i][1]);
	}
	Mojo.Event.stopListening(this.controller.get(this.ADDRESS_ID), Mojo.Event.tap, this.handleAddressTap, true);
	Mojo.Event.stopListening(this.controller.get(this.ADDRESS_ID), Mojo.Event.propertyChange,
		this.handleAddressSubmit);
	Mojo.Event.stopListening(this.controller.get(this.RETRY_ID), Mojo.Event.tap, this.handleRetry);
	var rbc = this.controller.get("jihad-reload");
	if (rbc) {
		Mojo.Event.stopListening(rbc, Mojo.Event.tap, this.handleReloadTap);
		rbc.removeEventListener("mousedown", this.handleReloadDown, false);
		rbc.removeEventListener("mouseup", this.handleReloadUp, false);
		rbc.removeEventListener("mouseout", this.handleReloadUp, false);
	}
	// The toast holds a node in this scene's document and a pending timer in its window; both
	// have to go with the scene or the timer fires against a torn-down document.
	JihadToast.detach();
};

// --- the single app-facing adapter surface (FROZEN SET) ---------------------

//* Proxy a BrowserAdapter method call to the plugin through the WebView widget's own
//* exposed method of the same name (widget.goBack() -> adapter.goBack(), and so on).
//* The set of method-name LITERALS passed here is grep-audited against the Enyo-1.0
//* app and must stay a subset of it — see the file header.
//*
//* The widget wraps the adapter methods of its day and no others: it has no cut and no
//* pageFocused (both existed in the stock adapter; the widget simply never exposed them),
//* so those two calls went to the warning below and nothing happened — Cut copied and
//* deleted nothing, and the padlock's question was asked without saying whose card it was
//* (device 2026-10-06). A method the widget does not wrap is called on the plugin element
//* the widget made — the one <object> inside the widget's element, carrying this variant's
//* MIME type — which is what every wrapper does inside (`element.mojo` is the widget's
//* exposed face, not the widget, and does not carry its `adapter`). The literal set above is
//* unchanged by this. On the device's framework build the widget wraps copy, selectAll,
//* insertStringAtCursor, goBack, goForward, reloadPage and stopLoad of this set; cut,
//* pageFocused AND findInPage take the plugin road — so Find on Page, too, had never reached
//* the engine from this shell. A plugin method answers a failure by throwing its message
//* (the widget's wrappers catch theirs); caught here and logged, the same as a missing one.
MainAssistant.prototype.callBrowserAdapter = function (method, args) {
	var view = this.controller.get(this.WEB_VIEW_ID);
	var widget = view && view.mojo, plugin = view && view.querySelector("object");
	if (widget && typeof widget[method] === "function") {
		return widget[method].apply(widget, args || []);
	}
	if (plugin && typeof plugin[method] === "function") {
		try {
			return plugin[method].apply(plugin, args || []);
		} catch (e) {
			Mojo.Log.error("[Jihad] BrowserAdapter %s: %s", method, e);
			return undefined;
		}
	}
	// Error level: warn is below the device log's threshold, and this line silent is how
	// a Cut that did nothing passed for a day.
	Mojo.Log.error("[Jihad] BrowserAdapter method unavailable: %s", method);
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
		case "share-bookmark":
		case "share-link":
		case "share-launcher":
			this.handleShareChoice(event.command);
			break;
		case "lib-bookmarks":
		case "lib-history":
		case "lib-downloads":
			this.openLibrary(event.command);
			break;
		case "jihad-find":
			this.showFind();
			break;
		case "jihad-settings":
			this.openUrl(JihadChromePrefs.settingsUrl());
			break;
		case "jihad-print":
			this.openPrint();
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

// --- Reload / Stop, in the address field ------------------------------------------
// The same command the row's circle used to send; the glyph says which (syncCommandMenu).

MainAssistant.prototype.handleReloadTap = function () {
	this.handleCommand({type: Mojo.Event.command, command: this.loading ? "jihad-stop" : "jihad-reload"});
};

MainAssistant.prototype.handleReloadDown = function (event) {
	var el = event && event.currentTarget;
	if (el && el.className.indexOf("jihad-urlbar-pressed") < 0) { el.className += " jihad-urlbar-pressed"; }
};

MainAssistant.prototype.handleReloadUp = function (event) {
	var el = event && event.currentTarget;
	if (el) { el.className = el.className.replace(/\s*jihad-urlbar-pressed/g, ""); }
};

// --- Share ▾ and Bookmarks ▾ on the command row ------------------------------------
// The same options as the Enyo shell's and Atlas's upper-right menu (and the Mochi shell's),
// the owner's request: "the upper right menu has the same options on all 3 variants". In this
// shell they live on the bottom row's circles — "mojo should have no top toolbar icons next to
// url. bottom circular buttons only" (owner, 2026-10-06): the Share circle and the Bookmarks
// circle are the framework's own SUBMENU items (setupSubmenus), which pop their menu at the
// tapped circle and send the choice back here as a command (handleCommand).

MainAssistant.prototype.setupSubmenus = function () {
	this.controller.setupWidget("jihad-share-menu", undefined, {items: [
		{label: $L("Add Bookmark"), command: "share-bookmark"},
		{label: $L("Share Link"), command: "share-link"},
		{label: $L("Add to Launcher"), command: "share-launcher"}
	]});
	this.controller.setupWidget("jihad-library-menu", undefined, {items: [
		{label: $L("Bookmarks"), command: "lib-bookmarks"},
		{label: $L("History"), command: "lib-history"},
		{label: $L("Downloads"), command: "lib-downloads"}
	]});
};

MainAssistant.prototype.handleShareChoice = function (command) {
	var title = this.title || this.url;
	if (command === "share-bookmark") {
		JihadToast.show("info", JihadBookmarks.add(this.url, title) ? $L("Bookmark added.") : $L("The bookmark could not be saved."));
	} else if (command === "share-link") {
		this.controller.popupSubmenu({
			onChoose: this.handleShareLinkChoice.bind(this),
			items: [
				{label: $L("Email"), command: "link-email"},
				{label: $L("Text Message"), command: "link-message"},
				{label: $L("Copy Address"), command: "link-copy"}
			]
		});
	} else if (command === "share-launcher") {
		this.addToLauncher(this.url, title);
	}
};

MainAssistant.prototype.handleShareLinkChoice = function (command) {
	if (command === "link-email") {
		this.shareCurrentPage();
	} else if (command === "link-message") {
		this.controller.serviceRequest("palm://com.palm.applicationManager", {
			method: "launch",
			parameters: {id: "com.palm.app.messaging", params: {compose: {messageText: $L("Check out this web page: ") + this.url}}},
			onFailure: function () { JihadToast.show("info", $L("Messaging could not be opened.")); }
		});
	} else if (command === "link-copy") {
		try {
			this.controller.stageController.setClipboard(this.url, false);
			JihadToast.show("info", $L("Address copied."));
		} catch (e) {
			JihadToast.show("info", $L("The address could not be copied."));
		}
	}
};

//* A launcher entry that opens `url` in THIS variant (the Enyo shell's addToLauncher): the
//* launch point carries {url}, which the stage reads like any other launch.
MainAssistant.prototype.addToLauncher = function (url, title) {
	var base = String(Mojo.appPath || "").replace(/^file:\/\//, "");
	this.controller.serviceRequest("palm://com.palm.applicationManager", {
		method: "addLaunchPoint",
		parameters: {id: Mojo.appInfo.id, title: String(title || url), icon: base + "icon.png", params: {url: url}},
		onSuccess: function () { JihadToast.show("info", $L("Added to Launcher.")); },
		onFailure: function () { JihadToast.show("info", $L("The page could not be added to the Launcher.")); }
	});
};

MainAssistant.prototype.openLibrary = function (command) {
	if (command === "lib-bookmarks") {
		this.controller.stageController.pushScene("bookmarks");
	} else if (command === "lib-history") {
		this.controller.stageController.pushScene("history", {history: JihadHistory.all()});
	} else if (command === "lib-downloads") {
		this.controller.stageController.pushScene("downloads");
	}
};

// --- Print (the app menu) --------------------------------------------------------

MainAssistant.prototype.openPrint = function () {
	if (this.isStartPage(this.url)) {
		JihadToast.show("info", $L("Open a page first: there is nothing to print on the start page."));
		return;
	}
	this.controller.stageController.pushScene("print", {main: this});
};

//* The daemon looks for printers (mDNS, or `address`); the result comes back on the
//* notification channel as "print-printers".
MainAssistant.prototype.findPrinters = function (address) {
	this.controller.serviceRequest("palm://net.riverstonerelay.jihadBrowserMojo", {
		method: "findPrinters",
		parameters: address ? {address: address} : {},
		onFailure: function () {
			if (PrintAssistant.current) { PrintAssistant.current.notification("print-error", $L("Printing is not available in this build.")); }
		}
	});
};

//* Print this card's page on `printer`. The request goes over Luna, which cannot say which
//* card is asking: pageFocused first, as for the padlock.
MainAssistant.prototype.printPage = function (printer) {
	this.callBrowserAdapter("pageFocused", [true]);
	this.controller.serviceRequest("palm://net.riverstonerelay.jihadBrowserMojo", {
		method: "printPage",
		parameters: {printer: printer},
		onFailure: function () {
			if (PrintAssistant.current) { PrintAssistant.current.notification("print-error", $L("The page could not be sent to the printer.")); }
		}
	});
};

// --- downloads the page started ------------------------------------------------
// The engine saves the file and reports it through the adapter's download callbacks on the
// plugin's event listener — the WebView widget (the <object>'s `eventListener`). The widget
// implements downloadFinished (it fires webViewDownloadFinished) and none of the other three,
// so they are added to it here, once, calling into this scene.

MainAssistant.prototype.hookDownloads = function () {
	var view = this.controller.get(this.WEB_VIEW_ID), obj = view && view.querySelector("object");
	var widget = obj && obj.eventListener, self = this;
	if (!widget || widget.__jihadDownloads) {
		return;
	}
	widget.__jihadDownloads = true;
	if (typeof widget.downloadStart !== "function") {
		widget.downloadStart = function (url) { self.handleDownloadStart(url); };
	}
	if (typeof widget.downloadProgress !== "function") {
		widget.downloadProgress = function (url, soFar, total) { self.handleDownloadProgress(url, soFar, total); };
	}
	if (typeof widget.downloadError !== "function") {
		widget.downloadError = function (url, message) { self.handleDownloadError(url, message); };
	}
};

MainAssistant.prototype.refreshDownloads = function () {
	if (typeof DownloadsAssistant !== "undefined" && DownloadsAssistant.current) {
		DownloadsAssistant.current.refresh();
	}
};

MainAssistant.prototype.handleDownloadStart = function (url) {
	JihadDownloads.start(url);
	JihadToast.show("info", $L("Downloading ") + JihadDownloads.nameOf(url) + "…");
	this.refreshDownloads();
};

MainAssistant.prototype.handleDownloadProgress = function (url, soFar, total) {
	// Only the numbers; storing on every chunk would write localStorage hundreds of times
	// for a large file, so it is written at most every 5% — per transfer: two downloads at
	// once must not throttle each other.
	var pct = total > 0 ? Math.floor(100 * soFar / total) : 0, key = String(url || "");
	this._dlPct = this._dlPct || {};
	if (pct - (this._dlPct[key] || 0) < 5 && pct < 100) {
		return;
	}
	this._dlPct[key] = pct;
	JihadDownloads.update(url, {received: soFar, total: total});
	this.refreshDownloads();
};

MainAssistant.prototype.handleDownloadFinished = function (event) {
	var rec = JihadDownloads.update(event && event.url, {state: "done", path: event && event.tmpFilePath, mimeType: event && event.mimeType});
	if (this._dlPct) { delete this._dlPct[String((event && event.url) || "")]; }
	JihadToast.show("info", $L("Downloaded ") + (rec ? rec.name : "") + ".");
	this.refreshDownloads();
};

MainAssistant.prototype.handleDownloadError = function (url, message) {
	JihadDownloads.update(url, {state: "failed", message: String(message || "")});
	if (this._dlPct) { delete this._dlPct[String(url || "")]; }
	JihadToast.show("info", $L("The download failed: ") + JihadDownloads.nameOf(url));
	this.refreshDownloads();
};

//* webViewMimeHandoff: the engine is downloading what it cannot show (see webViewEvents).
MainAssistant.prototype.handleMimeHandoff = function (event) {
	this.setLoading(false);
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
	this.loadFailed = false;
	this.updateLock();
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
	// What is on screen is an error now, not the site in the address field: no padlock.
	this.loadFailed = true;
	this.updateLock();
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
	if (loading && !this.loading) {
		this.applyHeaderHeight();   // the adapter may have come up since; a new page starts at the top
		this.onPageScrolled(0);
	}
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
	this.updateLock();
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
	this.updateLock();
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
		this.newCardItem,
		this.libraryItem,
		this.shareItem,
		this.findItem,
		{}
	];
	// Reload/Stop is in the address field, not this row: its glyph follows the load the same way.
	var rb = this.controller.get("jihad-reload");
	if (rb) {
		rb.className = rb.className.replace(/\s*jihad-urlbar-(refresh|stop)/g, "") +
			(this.loading ? " jihad-urlbar-stop" : " jihad-urlbar-refresh");
	}
	if (this.commandMenuReady) {
		this.controller.modelChanged(this.commandMenuModel);   // the theme is stylesheet: survives the redraw
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
