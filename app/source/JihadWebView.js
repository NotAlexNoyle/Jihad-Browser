// Copyright 2026 NotAlexNoyle. Apache-2.0.
//
// The engine saves a page's download itself ([jihad-dl] in the daemon's log) and reports it over
// the adapter's download callbacks: downloadStart(url), downloadProgress(url, soFar, total),
// downloadFinished(url, mimeType, path), downloadError(url, message), invoked by name on the
// <object> node's eventListener (BrowserAdapter::InvokeEventListener). Enyo 1.0's WebView has no
// methods of those names, so this shell never heard of such a download and its Downloads tab
// stayed empty while the Mochi and Mojo shells listed theirs (found 2026-11-08 by the finger
// suite's download-list script). This kind adds them and publishes them as events; the Mochi
// shell's JihadWebView.js is the same thing in Enyo 2.
enyo.kind({
	name: "JihadWebView",
	kind: enyo.WebView,
	events: {
		//* (inUrl)
		onDownloadStart: "",
		//* (inUrl, inReceived, inTotal)
		onDownloadProgress: "",
		//* (inUrl, inMimeType, inPath)
		onDownloadFinished: "",
		//* (inUrl, inMessage)
		onDownloadError: ""
	},
	// enyo.WebView is a wrapper: the plugin node's eventListener is its INNER BasicWebView (`view`,
	// BasicWebView.rendered), so a callback the adapter invokes by name is looked up on that object,
	// never on this kind. The four are bound onto it here (found 2026-11-08: the list stayed empty
	// with the methods on the wrapper alone).
	create: function() {
		this.inherited(arguments);
		var inner = this.$.view;
		if (inner) {
			inner.downloadStart = enyo.bind(this, "downloadStart");
			inner.downloadProgress = enyo.bind(this, "downloadProgress");
			inner.downloadFinished = enyo.bind(this, "downloadFinished");
			inner.downloadError = enyo.bind(this, "downloadError");
		}
	},
	downloadStart: function(inUrl) {
		this.doDownloadStart(inUrl);
	},
	downloadProgress: function(inUrl, inReceived, inTotal) {
		this.doDownloadProgress(inUrl, inReceived, inTotal);
	},
	downloadFinished: function(inUrl, inMimeType, inPath) {
		this.doDownloadFinished(inUrl, inMimeType, inPath);
	},
	downloadError: function(inUrl, inMessage) {
		this.doDownloadError(inUrl, inMessage);
	},
	// (through flashGestureLockChange) the framework's BasicWebView answers an engaged movie with an
	// enyo Popup ("Tap outside or pinch when finished"). Open, the popup stands between the touches and
	// the movie: modal, its scrim took the release of the tap that engaged the movie (since T-289 the
	// first tap engages); non-modal, the drag that followed lost its moves (Enyo read 2 where Mochi and
	// Mojo read 16, finger suite 2026-11-08). The same words as the daemon's own toast: a plain DOM
	// node no touch can land on (pointer-events none), and nothing of enyo's dispatcher in the way.
	showFlashLockedMessage: function() {
		if (enyo.jihadToast) {
			enyo.jihadToast.show("flash", $L("Tap outside or pinch when finished"));
		}
	}
});
