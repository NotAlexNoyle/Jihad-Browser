// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// Jihad Browser (Mojo) — stage assistant.
//
// One card stage with a scene stack. The first "main" scene is the browser; a
// "main" scene pushed later with a pageIdentifier is a page the ENGINE created for
// a target=_blank / window.open link (see main-assistant.js openNewPage), so those
// links land on the scene stack instead of being dropped, and the back gesture pops
// back to the opener.
//
// The engine, the daemon and the adapter are installed on-device by this package's
// deviceroot bundle; nothing here touches them. The plugin MIME swap that routes
// this app's WebView to them lives in ../models/jihad-engine-override.js.
function StageAssistant() {}

StageAssistant.prototype.setup = function () {
	this.controller.pushScene("main", {url: this.launchUrl()});
};

//* The url this app was launched with, if any (another app handing off a link, or
//* a universal-search result). Guarded: off-device there is no PalmSystem.
StageAssistant.prototype.launchUrl = function () {
	var params;
	try {
		if (window.PalmSystem && PalmSystem.launchParams) {
			params = Mojo.parseJSON(PalmSystem.launchParams);
		}
	} catch (e) {
		Mojo.Log.logException(e, "StageAssistant#launchUrl");
	}
	return (params && params.url) || "";
};
