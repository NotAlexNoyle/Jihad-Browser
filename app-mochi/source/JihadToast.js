// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadToast (Mochi) — the card end of the daemon's NON-BLOCKING message channel
// (cavekit-gre-widgets.md R5). Behaviourally identical to the Enyo-1.0 variant's
// ../../app/source/JihadToast.js; see that file for the full rationale.
//
// In one line: msgDialog* parks the render daemon on a FIFO until this card answers, which is
// the right price for a question and much too high for a statement like "Cookies cleared." So
// statements arrive over a Luna SUBSCRIPTION instead — palm://net.riverstonerelay.
// jihadBrowserMochi/notifications, this variant's OWN service, opened once at create() — and
// land here as a transient, non-interactive toast.
//
// NOTHING IS ADDED TO THE FROZEN CONTRACTS. Luna is not YAP (cavekit-ipc-contract.md R1) and
// this adds no callBrowserAdapter method, so this shell's adapter call set is unchanged and
// still a subset of the Enyo-1.0 app's.
//
// PLAIN DOM rather than an Enyo 2 control, for the same reasons as the Enyo-1.0 variant: it is
// transient, owns no state, must float above the WebView, and belongs to no view. CSS is
// webOS-3 card WebKit only — no box-sizing and no flexbox, both of which this WebKit needs
// -webkit- prefixes for and silently drops unprefixed.

enyo.jihad = enyo.jihad || {};

(function(scope) {
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
		// also carries no listener of its own, so even where pointer-events is not honoured the
		// worst case is one swallowed tap during the four seconds it is up.
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

	//* Show one line. A newer message REPLACES the one on screen and restarts the clock rather
	//* than queueing behind it: these are status lines and the newest is the one that matters.
	scope.toast = function(category, text) {
		if (!text) { return; }
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
		hideTimer = window.setTimeout(function() {
			hideTimer = null;
			if (!node) { return; }
			node.style.opacity = "0";
			// A timer, not a transitionend listener: if transitions do not fire in this card a
			// listener would never run and the node would sit there forever at opacity 0.
			removeTimer = window.setTimeout(function() {
				removeTimer = null;
				if (node) { node.style.display = "none"; }
			}, FADE_MS + 50);
		}, HOLD_MS);
	};

	//* Test seam: the message currently on screen, or "" when nothing is up.
	scope.toastCurrent = function() {
		return (node && node.style.display !== "none") ? node.textContent : "";
	};
})(enyo.jihad);
