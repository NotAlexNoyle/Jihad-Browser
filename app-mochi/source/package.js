// Copyright 2026 the Jihad Browser project.
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
	"JihadWebView.js",
	"JihadBrowser.js"
	// future (T-053): BookmarkList.js, HistoryList.js, DownloadList.js,
	// Preferences.js, FindBar.js, StartPage.js, dialogs...
);
