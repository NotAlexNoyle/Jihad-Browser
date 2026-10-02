// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadPreferences — the Enyo 2 / Mochi port of ../../app/source/Preferences.js.
// A full-card overlay panel holding what the CARD itself owns:
//
//   * the default search engine (kept in the card's own store, enyo.jihad.*SearchEngine);
//   * the Flash switch — the daemon's stored value, read and written through this variant's
//     own Luna service (getFlash / enableFlash / disableFlash), the same value the settings
//     page shows;
//   * an entry that opens the settings page;
//   * the clear-data actions, each behind a confirmation.
//
// NO Block Popups / Accept Cookies / Enable JavaScript switches (owner decision 2026-10-02,
// cavekit-preferences-ui.md R2). The settings page owns those three values. The switches
// that used to be here were stored in db8 and never reached the engine, so they changed
// nothing — and had they been wired up they would have set a default the page's value
// silently overrode. Either way: a control that can lose without saying so.
//
// Clear actions bubble to the shell (onClearBookmarks / onClearHistory / onClearCookies /
// onClearCache), which owns the db8 delByQuery and the daemon calls.

enyo.kind({
	name: "JihadPreferences",
	kind: "FittableRows",
	classes: "jihad-panel enyo-fit",
	showing: false,
	events: {
		onPreferenceChanged: "",
		//* The user asked for the settings page.
		onOpenSettings: "",
		onClearBookmarks: "",
		onClearHistory: "",
		onClearCookies: "",
		onClearCache: "",
		onClose: ""
	},
	//* What each clear action asks before it runs, and the event it fires on OK. The wording
	//* follows the Enyo shell's prompts.
	clearActions: {
		bookmarks: {ask: "Would you like to clear your bookmarks?",                              fire: "doClearBookmarks"},
		history:   {ask: "Would you like to clear your browser history?",                        fire: "doClearHistory"},
		cookies:   {ask: "Would you like to clear your browser cookies and local plug-in data?", fire: "doClearCookies"},
		cache:     {ask: "Would you like to clear your browser cache?",                          fire: "doClearCache"}
	},
	components: [
		{kind: "FittableColumns", classes: "jihad-panel-header", components: [
			{name: "doneBtn", classes: "jihad-panel-btn jihad-panel-done", ontap: "closeTapped", content: "Done"},
			{classes: "jihad-panel-title", fit: true, content: "Preferences"},
			{classes: "jihad-panel-btn jihad-panel-spacer"}
		]},
		{kind: "enyo.Scroller", fit: true, touch: true, classes: "jihad-panel-scroller", components: [
			{classes: "jihad-prefs-group", components: [
				{classes: "jihad-prefs-caption", content: "Default Web Search Engine"},
				//* One row per engine, built in create(); the chosen one carries a tick.
				{name: "engines"}
			]},
			{classes: "jihad-prefs-group", components: [
				{classes: "jihad-prefs-caption", content: "Content"},
				{kind: "FittableColumns", classes: "jihad-prefs-row", components: [
					{fit: true, components: [
						{classes: "jihad-prefs-label", content: "Enable Flash"},
						{name: "flashNote", classes: "jihad-prefs-note", showing: false}
					]},
					{kind: "mochi.ToggleButton", name: "flash", onChange: "flashChanged"}
				]},
				{name: "moreSettings", classes: "jihad-prefs-btn", ontap: "moreSettingsTapped", content: "More Settings"}
			]},
			{classes: "jihad-prefs-group", components: [
				{classes: "jihad-prefs-caption", content: "Clear Data"},
				{name: "clearBookmarks", classes: "jihad-prefs-btn", ontap: "clearTapped", clearKey: "bookmarks", content: "Clear Bookmarks"},
				{name: "clearHistory", classes: "jihad-prefs-btn", ontap: "clearTapped", clearKey: "history", content: "Clear History"},
				{name: "clearCookies", classes: "jihad-prefs-btn", ontap: "clearTapped", clearKey: "cookies", content: "Clear Cookies"},
				{name: "clearCache", classes: "jihad-prefs-btn", ontap: "clearTapped", clearKey: "cache", content: "Clear Cache"}
			]}
		]},
		// The confirmation. Its own overlay: JihadDialogs' confirm answers the ENGINE (its
		// buttons write to the page's dialog pipe), so it cannot be borrowed for a question
		// the card is asking.
		{name: "ask", classes: "jihad-modal-overlay", showing: false, components: [
			{classes: "jihad-dialog-box", components: [
				{name: "askMsg", classes: "jihad-dialog-message"},
				{classes: "jihad-dialog-buttons", components: [
					{classes: "jihad-btn jihad-btn-alt", content: "Cancel", ontap: "askCancel"},
					{name: "askOk", classes: "jihad-btn", content: "Clear", ontap: "askOk"}
				]}
			]}
		]}
	],
	create: function() {
		this.inherited(arguments);
		// mochi.ToggleButton fires onChange from valueChanged, and its rendered() calls
		// valueChanged twice — so the switch emits onChange with its default the moment
		// this (initially hidden) panel is rendered at app start. Nothing may be SENT from
		// a change until the stored value has been read back and shown.
		this._applying = true;
		this._pendingClear = null;
		this.buildEngines();
	},
	open: function() {
		this.buildEngines();
		this.loadFlash();
		this.show();
		this.resize();      // see JihadBookmarkList.open
	},

	// --- default search engine ----------------------------------------------
	buildEngines: function() {
		var list = enyo.jihad.searchEngines, cur = enyo.jihad.loadSearchEngine().id;
		this.$.engines.destroyClientControls();
		for (var i = 0; i < list.length; i++) {
			this.$.engines.createComponent({
				classes: "jihad-prefs-row jihad-prefs-choice" + (list[i].id === cur ? " chosen" : ""),
				content: list[i].name, engineId: list[i].id, ontap: "engineTapped"
			}, {owner: this});
		}
		if (this.generated) { this.$.engines.render(); }
	},
	engineTapped: function(inSender) {
		if (!inSender || !inSender.engineId) { return true; }
		enyo.jihad.saveSearchEngine(inSender.engineId);
		this.buildEngines();
		this.doPreferenceChanged({preference: "defaultSearch", value: inSender.engineId});
		return true;
	},

	// --- Flash switch ---------------------------------------------------------
	//* Show the daemon's value. `available: false` (no Flash plugin on the device) leaves the
	//* switch showing the stored value and says why it has no effect — hiding it would make
	//* the panel differ between devices for no reason a user could see.
	loadFlash: function() {
		var self = this;
		enyo.jihad.own("getFlash", {}, function(resp) { self.showFlash(resp); });
	},
	showFlash: function(resp) {
		var ok = Boolean(resp && resp.returnValue);
		this._applying = true;
		if (ok || (resp && typeof resp.enabled === "boolean")) {
			this.$.flash.setValue(Boolean(resp.enabled));
		}
		var note = "";
		if (!resp || resp.offline || (!ok && typeof resp.enabled !== "boolean")) {
			note = "The browser engine is not answering; the switch cannot be changed now.";
		} else if (resp.available === false) {
			note = "No Flash plugin is installed on this device.";
		}
		this.$.flashNote.setContent(note);
		this.$.flashNote.setShowing(Boolean(note));
		var self = this;
		enyo.asyncMethod(this, function() { self._applying = false; });
	},
	flashChanged: function(inSender, inEvent) {
		if (this._applying) { return; }
		var on = inEvent && typeof inEvent.value === "boolean" ? inEvent.value : inSender.getValue();
		var self = this;
		// The reply carries the value now in effect, so a refused or failed change puts the
		// switch back instead of leaving it showing something that is not true.
		enyo.jihad.own(on ? "enableFlash" : "disableFlash", {}, function(resp) {
			self.showFlash(resp);
			self.doPreferenceChanged({preference: "flash", value: Boolean(resp && resp.enabled)});
		});
	},

	moreSettingsTapped: function() {
		this.hide();
		this.doOpenSettings();
	},

	// --- clear data, behind a confirmation -------------------------------------
	clearTapped: function(inSender) {
		var act = inSender && this.clearActions[inSender.clearKey];
		if (!act) { return true; }
		this._pendingClear = act;
		this.$.askMsg.setContent(act.ask);
		this.$.ask.show();
		return true;
	},
	askCancel: function() {
		this._pendingClear = null;
		this.$.ask.hide();
		return true;
	},
	askOk: function() {
		var act = this._pendingClear;
		this._pendingClear = null;
		this.$.ask.hide();
		if (act && this[act.fire]) { this[act.fire](); }
		return true;
	},
	closeTapped: function() {
		this.askCancel();
		this.hide();
		this.doClose();
	}
});
