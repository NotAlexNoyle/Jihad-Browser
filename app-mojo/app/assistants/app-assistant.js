// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// Jihad Browser (Mojo) — app assistant: a RELAUNCH with a link. Without one, Mojo's framework answers a launch of the
// running app by bringing its card forward (controller_app.js handleRelaunch: "AppAssistant didn't implement
// handleLaunch so giving focus to main window") and the link is dropped: "open this in Jihad Mojo" while the card was
// open showed whatever it showed before (found 2026-11-09 by the screen-hold test). The first launch stays the stage
// assistant's (StageAssistant.launchUrl); here, a relaunch's target (or url) goes to the card's browser scene.
function AppAssistant() {}

AppAssistant.prototype.handleLaunch = function (params) {
	var url = params && (params.target || params.url);
	// the card's stage even while another app's card is in front (the ACTIVE stage controller is then none, and the
	// link was dropped with the focus; found 2026-11-09): the app's main stage, which Mojo keeps as its stageController
	var sc = this.controller.getActiveStageController(Mojo.Controller.StageType.card) ||
		Mojo.Controller.stageController || this.controller.getStageController("stage");
	if (!sc) {
		return;   // the first launch: the stage is not set up yet and reads the link itself
	}
	if (url) {
		var scenes = sc.getScenes ? sc.getScenes() : [];
		var main = scenes.length ? scenes[0].assistant : null;   // the browser: the bottom of the stack
		if (main && typeof main.openUrl === "function") {
			if (scenes.length > 1) {
				sc.popScenesTo("main");   // a page the engine opened on top goes; the browser takes the link
			}
			main.openUrl(url);
		}
	}
	sc.activate();
};
