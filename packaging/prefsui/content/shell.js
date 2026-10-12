/* Copyright 2026 NotAlexNoyle. MPL-2.0 (matches the GRE this is registered into).
 *
 * Which card a Jihad page is shown in (cavekit-chrome-look.md R1). Each variant's daemon is started
 * by its own upstart job, which names it in JIHAD_BS_NAME; nothing else tells the three apart (the
 * app info and the user agent are the same in all of them). Loaded in <head>, so the page's root
 * carries data-shell="enyo" | "mochi" | "mojo" before anything is drawn and shell.css draws each
 * shell's buttons from the first paint. Elsewhere (the desktop build) there is no attribute and the
 * pages keep their own look.
 */
"use strict";
(function () {
	let shell = "";
	try {
		let name = Components.classes["@mozilla.org/process/environment;1"]
			.getService(Components.interfaces.nsIEnvironment).get("JIHAD_BS_NAME");
		shell = {"jihad-browser": "enyo", "jihad-browser-mochi": "mochi", "jihad-browser-mojo": "mojo"}[name] || "";
	} catch (e) {
		shell = "";
	}
	if (shell) { document.documentElement.setAttribute("data-shell", shell); }
})();
