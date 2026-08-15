// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// JihadToast (Mojo) — the card end of the daemon's NON-BLOCKING message channel
// (cavekit-gre-widgets.md R5). Same behaviour as the other two shells'
// ../../../app/source/JihadToast.js and ../../../app-mochi/source/JihadToast.js.
//
// WHY. msgDialog* parks the render daemon on a FIFO until the card answers or the deadline
// expires — the right price for a question, far too high for a statement such as
// "Cookies cleared." Statements therefore arrive on a Luna SUBSCRIPTION to this variant's own
// service (palm://net.riverstonerelay.jihadBrowserMojo/notifications, opened in
// ../assistants/main-assistant.js) and land here as a transient, non-interactive toast.
//
// CONTRACT NOTE, because this variant's invariant is the strictest of the three: this adds NO
// callBrowserAdapter method, so the set stays {goBack, goForward, reloadPage, stopLoad} — still
// a strict subset of the Enyo-1.0 shell's — and NO YAP command or message
// (cavekit-ipc-contract.md R1). It does mean this variant now uses one palm:// URI where it
// previously used none, and it is OUR OWN per-variant service, never com.palm.browserServer.
//
// NOT a Mojo.Widget: there is no framework toast/banner widget that draws inside the card
// (Mojo's banner goes to the system dashboard, which is the wrong surface for a message about
// the page you are looking at), and this is transient chrome that belongs to no scene. Plain
// DOM through the scene's own window, so it is torn down with the card.
//
// CSS is webOS-3 card WebKit only: no box-sizing and no flexbox — this WebKit needs -webkit-
// prefixes for both and silently drops the unprefixed forms.

var JihadToast = (function () {
	var HOLD_MS = 4000;
	var FADE_MS = 250;

	var doc = null;
	var win = null;
	var node = null;
	var hideTimer = null;
	var removeTimer = null;

	//* The scene hands us its own window/document (Mojo scenes are not guaranteed to be the
	//* global one, and the assistant already holds controller.window).
	function attach(aWindow) {
		win = aWindow || window;
		doc = win.document;
		node = null;
	}

	function build() {
		if (node) { return node; }
		if (!doc || !doc.body) { return null; }
		node = doc.createElement("div");
		node.id = "jihad-toast";
		var s = node.style;
		s.position = "fixed";
		// left+right rather than a width: needs no box-sizing, and padding cannot push the box
		// past the screen edge the way width:100% plus padding would.
		s.left = "24px";
		s.right = "24px";
		s.bottom = "24px";
		s.padding = "10px 16px";
		s.background = "rgba(0,0,0,0.82)";
		s.color = "#ffffff";
		s.fontSize = "20px";
		s.lineHeight = "24px";
		s.textAlign = "center";
		s.borderRadius = "8px";
		s.WebkitBorderRadius = "8px";
		s.zIndex = "10000";
		s.opacity = "0";
		s.display = "none";
		// Non-interactive: a toast must never eat a tap meant for the page underneath. The node
		// carries no listener of its own either, so even where pointer-events is not honoured
		// the worst case is one swallowed tap in the four seconds it is up.
		s.pointerEvents = "none";
		s.WebkitTransition = "opacity " + FADE_MS + "ms linear";
		s.transition = "opacity " + FADE_MS + "ms linear";
		doc.body.appendChild(node);
		return node;
	}

	function clearTimers() {
		if (hideTimer)   { win.clearTimeout(hideTimer);   hideTimer = null; }
		if (removeTimer) { win.clearTimeout(removeTimer); removeTimer = null; }
	}

	//* Show one line. A newer message REPLACES the one on screen and restarts the clock rather
	//* than queueing: these are status lines and the newest is the one that matters.
	function show(category, text) {
		if (!text || !win) { return; }
		var n = build();
		if (!n) { return; }
		clearTimers();
		// textContent, never innerHTML: the text can carry an add-on name that came out of a
		// downloaded install.rdf, i.e. from the web.
		n.textContent = String(text);
		n.style.display = "block";
		// Force layout before flipping opacity, or display+opacity coalesce into one style
		// change and there is no transition to run.
		/*jshint -W030 */
		n.offsetHeight;
		n.style.opacity = "1";
		hideTimer = win.setTimeout(function () {
			hideTimer = null;
			if (!node) { return; }
			node.style.opacity = "0";
			// A timer, not a transitionend listener: if transitions do not fire in this card a
			// listener would never run and the node would sit there forever at opacity 0.
			removeTimer = win.setTimeout(function () {
				removeTimer = null;
				if (node) { node.style.display = "none"; }
			}, FADE_MS + 50);
		}, HOLD_MS);
	}

	function detach() {
		if (win) { clearTimers(); }
		if (node && node.parentNode) { node.parentNode.removeChild(node); }
		node = null;
		doc = null;
		win = null;
	}

	return {
		attach: attach,
		show: show,
		detach: detach,
		//* Test seam: the message currently on screen, or "" when nothing is up.
		current: function () {
			return (node && node.style.display !== "none") ? node.textContent : "";
		}
	};
})();
