// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// The settings page's Plugins wordings (cavekit-preferences-ui.md R8), checked on this PC with
// stand-in plugin entries: `node render/goanna/test/prefs-addons-rows.test.js`. The functions are
// taken from packaging/prefsui/content/preferences.js as they are, not copied, so the test follows
// the page. What the device shows for the real Flash is checked there.
"use strict";
const fs = require("fs");
const path = require("path");
const assert = require("assert");

const src = fs.readFileSync(path.join(__dirname, "../../../packaging/prefsui/content/preferences.js"), "utf8");
function grab(name) {
	const start = src.indexOf("\nfunction " + name + "(");
	assert(start >= 0, "no function " + name + " in preferences.js");
	const end = src.indexOf("\n}\n", start);
	return src.slice(start, end + 3);
}
// eslint-disable-next-line no-eval
const { pluginState, isFlash, flashSummary } = eval("(function () {" + grab("pluginState") + grab("isFlash") +
	grab("flashSummary") + "return { pluginState, isFlash, flashSummary }; })()");

const SYSTEM = "/media/cryptofs/apps/usr/lib/BrowserServerPlugins/libflashplayer.so";
function flash(o) {
	return Object.assign({
		name: "Adobe Flash", description: "Shockwave Flash 10.3 d185", version: "", fullpath: SYSTEM,
		niceName: "flash", disabled: false, clicktoplay: false, blocklisted: false,
		getMimeTypes: function () { return ["application/x-shockwave-flash", "application/futuresplash"]; }
	}, o);
}
const other = { name: "Test", description: "", niceName: "test", getMimeTypes: function () { return ["application/x-test"]; } };

const cases = [
	["on, from the HP plugin", [flash({})],
	 "Adobe Flash Player 10.3 d185, on. From the HP Flash plugin installed on this device."],
	["off, from the profile", [flash({ disabled: true, fullpath: "/media/cryptofs/apps/usr/palm/applications/x/profile/plugins/libflashplayer.so" })],
	 "Adobe Flash Player 10.3 d185, off. From this browser's own plugins folder."],
	["ask to activate, said not to play", [flash({ clicktoplay: true })],
	 "Adobe Flash Player 10.3 d185, set to Ask to Activate, which this browser cannot do, so it does not play: " +
	 "choose Always Activate in Manage Plugins. From the HP Flash plugin installed on this device."],
	["blocked", [flash({ blocklisted: true, disabled: true })],
	 "Adobe Flash Player 10.3 d185, blocked. From the HP Flash plugin installed on this device."],
	["the version field when it has one; found by MIME type; no path", [flash({ version: "11.1.115.81", niceName: "", fullpath: "" }), other],
	 "Adobe Flash Player 11.1.115.81, on. 1 other plugin."],
	["no version anywhere", [flash({ description: "Shockwave Flash" })],
	 "Adobe Flash Player, on. From the HP Flash plugin installed on this device."],
	["not installed", [],
	 "Adobe Flash is not installed on this device. HP's Flash plugin for webOS puts it where this browser finds it."],
	["not installed, another plugin", [other],
	 "Adobe Flash is not installed on this device. HP's Flash plugin for webOS puts it where this browser finds it. 1 other plugin."]
];
let failed = 0;
for (const [what, tags, want] of cases) {
	const got = flashSummary(tags);
	if (got === want) { console.log("ok   " + what); } else { failed++; console.log("FAIL " + what + "\n  got:  " + got + "\n  want: " + want); }
}
assert.strictEqual(isFlash(other), false);
assert.strictEqual(pluginState(flash({ clicktoplay: true })), "set to Ask to Activate");
console.log(failed ? failed + " failed" : "all " + cases.length + " passed");
process.exit(failed ? 1 : 0);
