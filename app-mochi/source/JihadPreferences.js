// Copyright 2026 the Jihad Browser project.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadPreferences — the Enyo 2 / Mochi port of ../../app/source/Preferences.js.
// A full-card overlay panel with mochi.ToggleButton content switches and the
// clear-data actions. Preferences are read from and written to the SAME db8 kind
// the Enyo 1.0 app uses (net.riverstonerelay.jihad-browser.preferences:1, via
// enyo.jihad.db*), seeded with the same defaults BrowserApp.js seeds. Clear
// actions bubble to the shell (onClearBookmarks/onClearHistory/onClearCookies/
// onClearCache) so the shell drives the db8 delByQuery + the
// palm://com.palm.browserServer/{clearCookies,clearCache} URIs it already owns.
//
// INTENTIONAL OMISSION (see PARITY.md): pushing toggle values into the live engine
// (the Enyo 1.0 setEnableJavascript/setBlockPopups/setAcceptCookies adapter calls)
// is NOT done here — that would widen the frozen callBrowserAdapter method set
// (cavekit-ipc-contract R1, verified {findInPage,goBack,goForward,reloadPage,
// stopLoad}). Preferences are persisted to db8; engine application is deferred to
// a contract-scope change.

enyo.kind({
	name: "JihadPreferences",
	kind: "FittableRows",
	classes: "jihad-panel enyo-fit",
	showing: false,
	events: {
		onPreferenceChanged: "",
		onClearBookmarks: "",
		onClearHistory: "",
		onClearCookies: "",
		onClearCache: "",
		onClose: ""
	},
	//* Default browser preferences, identical to BrowserApp.gotInitialBrowserPreferences.
	defaults: [
		{key: "blockPopups", value: true},
		{key: "acceptCookies", value: true},
		{key: "enableJavascript", value: true},
		{key: "rememberPasswords", value: true}
	],
	components: [
		{kind: "FittableColumns", classes: "jihad-panel-header", components: [
			{name: "doneBtn", classes: "jihad-panel-btn jihad-panel-done", ontap: "closeTapped", content: "Done"},
			{classes: "jihad-panel-title", fit: true, content: "Preferences"},
			{classes: "jihad-panel-btn jihad-panel-spacer"}
		]},
		{kind: "enyo.Scroller", fit: true, touch: true, classes: "jihad-panel-scroller", components: [
			{classes: "jihad-prefs-group", components: [
				{classes: "jihad-prefs-caption", content: "Content"},
				{kind: "FittableColumns", classes: "jihad-prefs-row", components: [
					{fit: true, classes: "jihad-prefs-label", content: "Block Popups"},
					{kind: "mochi.ToggleButton", name: "blockPopups", onChange: "toggleChanged", preference: "blockPopups"}
				]},
				{kind: "FittableColumns", classes: "jihad-prefs-row", components: [
					{fit: true, classes: "jihad-prefs-label", content: "Accept Cookies"},
					{kind: "mochi.ToggleButton", name: "acceptCookies", onChange: "toggleChanged", preference: "acceptCookies"}
				]},
				{kind: "FittableColumns", classes: "jihad-prefs-row", components: [
					{fit: true, classes: "jihad-prefs-label", content: "Enable JavaScript"},
					{kind: "mochi.ToggleButton", name: "enableJavascript", onChange: "toggleChanged", preference: "enableJavascript"}
				]}
			]},
			{classes: "jihad-prefs-group", components: [
				{classes: "jihad-prefs-caption", content: "Clear Data"},
				{name: "clearBookmarks", classes: "jihad-prefs-btn", ontap: "clearBookmarksTapped", content: "Clear Bookmarks"},
				{name: "clearHistory", classes: "jihad-prefs-btn", ontap: "clearHistoryTapped", content: "Clear History"},
				{name: "clearCookies", classes: "jihad-prefs-btn", ontap: "clearCookiesTapped", content: "Clear Cookies"},
				{name: "clearCache", classes: "jihad-prefs-btn", ontap: "clearCacheTapped", content: "Clear Cache"}
			]}
		]}
	],
	create: function() {
		this.inherited(arguments);
		// Persistence is SUPPRESSED until the stored prefs have been read back
		// (load -> applyPrefs / seedDefaults clears this). mochi.ToggleButton fires
		// onChange from valueChanged, and its rendered() calls valueChanged TWICE —
		// so every toggle emits onChange with its default `false` the moment this
		// (initially hidden) panel is rendered at app start. Without this gate that
		// startup burst ran toggleChanged and merged value:false over the user's
		// stored preferences on every single launch.
		this._applying = true;
	},
	open: function() {
		this.load();
		this.show();
	},
	//* Read prefs from db8; seed defaults if the kind is empty (BrowserApp parity).
	load: function() {
		var self = this;
		enyo.jihad.dbFind(enyo.jihad.kinds.preferences, null, null, false, function(results) {
			if (results && results.length) {
				self.applyPrefs(results);
			} else {
				self.seedDefaults();
			}
		});
	},
	seedDefaults: function() {
		var self = this, kind = enyo.jihad.kinds.preferences;
		var objs = [];
		for (var i = 0, d; (d = this.defaults[i]); i++) {
			objs.push({_kind: kind, key: d.key, value: d.value});
		}
		enyo.jihad.dbPut(objs, function() {
			self.applyPrefs(objs);
		});
	},
	//* Reflect stored values onto the toggles WITHOUT firing persistence (the
	//* mochi toggle emits onChange even on programmatic set — guard it).
	applyPrefs: function(results) {
		this._applying = true;
		for (var i = 0, p; (p = results[i]); i++) {
			var t = this.$[p.key];
			if (t && t.setValue) {
				t.setValue(Boolean(p.value));
			}
		}
		var self = this;
		enyo.asyncMethod(this, function() { self._applying = false; });
	},
	toggleChanged: function(inSender, inEvent) {
		if (this._applying) {
			return;
		}
		var key = inSender.preference;
		var value = inEvent && typeof inEvent.value === "boolean" ? inEvent.value : inSender.getValue();
		// Persist to db8 (same shape as BrowserApp.preferenceChanged merge).
		enyo.jihad.dbMergeQuery(enyo.jihad.kinds.preferences,
			[{prop: "key", op: "=", val: key}], {value: value});
		this.doPreferenceChanged({preference: key, value: value});
	},
	clearBookmarksTapped: function() { this.doClearBookmarks(); },
	clearHistoryTapped:   function() { this.doClearHistory(); },
	clearCookiesTapped:   function() { this.doClearCookies(); },
	clearCacheTapped:     function() { this.doClearCache(); },
	closeTapped: function() {
		this.hide();
		this.doClose();
	}
});
