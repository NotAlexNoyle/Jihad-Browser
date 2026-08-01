// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// Enyo 2 package manifest for the Mochi UI variant of Jihad Browser.
// Declares the shell: styles, the NPAPI-bound WebView control, and the main app
// kind. JihadWebView loads before JihadBrowser (which references it as a kind).
// The full feature-parity port (bookmarks / history / downloads / find /
// preferences / start page / dialogs) is the next wave (cavekit-mochi-ui.md,
// T-053) and slots additional source files in here.
enyo.depends(
	"JihadBrowser.css",
	// Luna-service helpers (db8 / download manager / launcher) — the Enyo 2
	// stand-in for the Enyo 1.0 DbService / enyo.PalmService kinds.
	"JihadServices.js",
	"JihadWebView.js",
	// Parity views + dialog set (T-053) are appended here as they land, ahead of
	// JihadBrowser (which references them as kinds).
	"JihadBookmarkList.js",
	"JihadHistoryList.js",
	"JihadDownloadList.js",
	"JihadFindBar.js",
	"JihadPreferences.js",
	"JihadDialogs.js",
	"JihadBrowser.js"
);
