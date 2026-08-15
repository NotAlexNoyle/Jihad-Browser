// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadToast — the card end of the daemon's NON-BLOCKING message channel
// (cavekit-gre-widgets.md R5).
//
// WHY THIS EXISTS. Until 2026-08-15 the only way the daemon could tell the user anything was
// msgDialog*, which parks the render daemon on a FIFO until this card answers or the deadline
// expires (render/goanna/BrowserPageGoanna.cpp, awaitDialogReply). That is the right price for
// a question — "may this site install an add-on?" — and far too high for a statement such as
// "Cookies cleared." So statements now arrive over a Luna SUBSCRIPTION instead, and land here.
//
// The daemon side is render/browserserver/JihadLunaService.cpp (`notifications`); the wire is
//     -> {"subscribe":true}
//     <- {"returnValue":true,"subscribed":true}
//     <- {"category":"privacy","text":"Cookies cleared."}          (zero or more, later)
// NOTHING IS ADDED TO THE FROZEN CONTRACTS BY THIS. It is a Luna service — not a YAP command
// or message (cavekit-ipc-contract.md R1), and not a callBrowserAdapter method
// (cavekit-ui-shell.md R2). This card gains no adapter call.
//
// PLAIN DOM, NOT AN ENYO KIND, deliberately. The toast has to sit above the WebView wherever
// the card happens to be — start page or browser pane — and it is transient, non-interactive
// and owns no state, so a kind in the component tree would buy nothing and would have to be
// added to two panes. The precedent that this composites at all is the `<select>` popup, which
// is card-side HTML drawn over the same NPAPI plugin surface and is device-proven
// (context/impl/impl-select-popup-2026-08-03.md).
//
// CSS: webOS-3 card WebKit only. No box-sizing and no flexbox — this WebKit needs -webkit-
// prefixes for both and silently drops the unprefixed forms, so the width comes from `left` +
// `right` on a positioned block, which needs neither.

enyo.jihadToast = (function() {

	//* How long a message stays up. Long enough to read one line, short enough that it is gone
	//* before the user's next tap lands anywhere near it.
	var HOLD_MS = 4000;
	var FADE_MS = 250;

	var node = null;
	var hideTimer = null;
	var removeTimer = null;

	function build() {
		if (node) { return node; }
		if (!document || !document.body) { return null; }
		node = document.createElement("div");
		node.id = "jihad-toast";
		var s = node.style;
		s.position = "fixed";
		// left+right instead of a width: no box-sizing needed, and the padding cannot push the
		// box past the screen edge the way `width:100%` plus padding would.
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
		// Non-interactive: a toast must never eat a tap meant for the page underneath. Belt and
		// braces, because this card's WebKit is old enough that pointer-events cannot simply be
		// assumed — the node also carries no listener of its own, so the worst case is one
		// swallowed tap in the four seconds it is up, not a broken control.
		s.pointerEvents = "none";
		s.WebkitTransition = "opacity " + FADE_MS + "ms linear";
		s.transition = "opacity " + FADE_MS + "ms linear";
		document.body.appendChild(node);
		return node;
	}

	function clearTimers() {
		if (hideTimer)   { window.clearTimeout(hideTimer);   hideTimer = null; }
		if (removeTimer) { window.clearTimeout(removeTimer); removeTimer = null; }
	}

	//* Show one line. A second message while the first is up REPLACES it and restarts the
	//* clock rather than queueing: these are status lines, and the newest is the one that
	//* matters. `category` is carried for future styling and is not used to filter — a message
	//* the daemon bothered to send is a message the user should see.
	function show(category, text) {
		if (!text) { return; }
		var n = build();
		if (!n) { return; }
		clearTimers();
		// textContent, never innerHTML: the text can carry an add-on name that came out of a
		// downloaded install.rdf, i.e. from the web.
		n.textContent = String(text);
		n.style.display = "block";
		// Force layout before flipping opacity, or the browser coalesces display+opacity into
		// one style change and there is no transition to run (the element just appears).
		/*jshint -W030 */
		n.offsetHeight;
		n.style.opacity = "1";
		hideTimer = window.setTimeout(function() {
			hideTimer = null;
			if (!node) { return; }
			node.style.opacity = "0";
			// Not a transitionend listener: if transitions do not fire in this card the node
			// would stay in the DOM forever with opacity 0, invisible and still stealing the
			// bottom of the screen from anything that does hit-test it. A timer cannot hang.
			removeTimer = window.setTimeout(function() {
				removeTimer = null;
				if (node) { node.style.display = "none"; }
			}, FADE_MS + 50);
		}, HOLD_MS);
	}

	return {
		show: show,
		//* Test seam: the message currently on screen, or "" when nothing is up.
		current: function() {
			return (node && node.style.display !== "none") ? node.textContent : "";
		}
	};
})();
