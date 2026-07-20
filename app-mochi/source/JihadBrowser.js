// Copyright 2026 the Jihad Browser project.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// Mochi (Enyo 2) UI variant of Jihad Browser — main application kind.
//
// This is the browser SHELL: a Mochi-composed toolbar (back / forward / reload /
// stop + address input + menu), a load ProgressBar, the JihadWebView content
// region, and basic Popup scaffolding. Full feature parity with the Enyo 1.0 app
// (bookmarks / history / downloads / find / preferences / start page /
// alert-confirm-prompt-SSL dialogs) is the next-wave parity port (T-053); this
// shell is structured so those views slot into the popups / menu below.
//
// Contract invariant (cavekit-ipc-contract R1, cavekit-mochi-ui R3): the UI
// drives the engine ONLY through JihadWebView's callBrowserAdapter proxy and the
// palm://com.palm.browserServer/* Luna services — the identical method-name set
// and URIs the Enyo 1.0 app uses (../../app/source/Browser.js). See
// ../../docs/IPC-CONTRACT.md.
//
// Layout: FittableRows (toolbar / progress / view). The toolbar is a
// FittableColumns whose address box is `fit: true`, so the chrome reflows to the
// panel width with no hardcoded pixels beyond the shared 1024x768 both TouchPad
// models use. The toolbar is NOT wrapped in mochi.Header — Header reserves a
// title slot and pads its client, which would keep the address field from taking
// the free width. Nav glyphs are inline base64 SVG data URIs (no image assets),
// stroked white on the dark toolbar so they read against the chrome.



// Toolbar icons: crisp PNGs (this engine renders neither the mochi.IconButton
// sprites, CSS-background SVGs, nor the needed font glyphs; and border-radius:50%
// is not a circle here). A persona recolours them via CSS `filter` or swaps a URI.
enyo.JihadIcons = {
	back: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAABZklEQVR4nO3WP27CMBQG8M8kbHSBofeoxNLLdKJqL9MD9Bxdegek/jkDA8hKHGyTENvwdYHKrQStlAQJtW+zHfuXp8R+Bv6j5RBNFyDZA0AAEEKw8Rv9AovbjRM4hqUAsFgsrquqepJSXnSG7jEp5biqKkmSWut3kkOS4hjaOzRwDBNCBCnleDAYPG82m+F2uwWA1/l8XqOF/+ILBnxmlltrNyS5XC4fo2faAX/CSCZni/VOjhVFMS7LMl+tViRJY0z72B4k2c/z/CWEwLIsvbX2oQl2cFvsFiOAVAgxcs5tkySBc+5tP7f1o4xkAgB5nt+EEKi1DiEEZll2CwDT6bTfKhijSqmJ957GGO+9p1JqshtPu0DTGLXWur+DOuditLtKEX/Tuq5ZFMU9yfR7jWwdreua3ntqrQtjzOVP5akxmmXZ3Xq9nimlrnb97WcYoT0AmM1mo127uytGhIoYP0mcJLOziQ/XcMKJTkZM8wAAAABJRU5ErkJggg==",
	forward: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAABF0lEQVR4nO3WP07DMBQG8Je4IhJHYWdm4BxsbdeepyzciClnyJI/z46fHBU7fCyxqAoCocZBiH6TZUv+ObGtZ6JLfjMAcgBqMeyzdlKs67o7rfV26lslwwBkzHzvnGMAYOZ1MjTumdZ6CwDW2oP3Hsy8SYnmRERt266997DWviyBroiImHnjvYeI/F90/itzjIYQICLjYqgx5sE5J9baUURC3/c3X6FnrySE8ExEBwCvRVEopdT1uXN+SPy6uq5vh2HQIjICgNZ6X5bl1ay/9ATrImaMeZzGMwDZUpj6s1hORFRV1TJYrBbDMLQiEpJhcUKi92oxVYx9EuwIzYmImqbZWWufYl8S7BSd2vMd/e/Q5O+ZS36aN0Yfu5TQd0NKAAAAAElFTkSuQmCC",
	reload: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAC6UlEQVR4nO1Wv2/bRhh935GSLMmujDgIiiAFOhhFgSJtFgOdAiRA4axJ1g6Bkal/RecOBfw3dGk7dOmSqVOHBNZkZElaF4EROEEtnEmKFO9IHl8HnwDaleRocKa8ibyP373v3veDB3zAEiAZkJT3RfZ+iJpkWuvvtNYDAHJpAZBUJEVr/VVVVYzj+JFfDy6LMAAArfUfdV0ziqLnJDvTQC6FbDQaPSjLklEUWZLUWn/btDexMAKSCgBEpJ5hEwBI0/Qayf1Wq3XVGOO63W7LGPOmqqrPNzY2xt6fUz+1iFBE6llkDV+p6/qbXq/3uixLpZRqTSaTst/v/91ut295osWyTk+ltb5ZFMXvo9Ho63n9RVJItgAgSZJd5xyLonBJkjz09osLh2QIAFEU/UCSSZLoLMuu+81nRusLJIiiaN/ncM+v/U/BWZI6kiHJOwCcc27Y7/ePAEgzF+f3EREH4GeSCMPwszzPb4hIfZ70zAtJERHGcbwG4FMAAYCnC4JruFIA7JVliU6n85G1dtPbzqgybxOZfigi9l36SURY13XhnyEiM/dW551IymAwmIjIvwBQ1/WXXsp5cgJ+lCmlvgjDENZao5Q68rYzfrOiUCJiADwDwCAIbvv5yFlF4E8v/oT3lVKsqurV2traXz5FZ9pqbl5I/lSWpfR6vY+VUj96Rw6Hw5Zvk8BXdCAi5cnJyU63272L01T8IiIlTmvgYjTm468kOZlMmCTJ7rTnziNN08d5nmfOOcZx/A/JwbxZOq+vBIDEcbweBMGT1dXVraIoYIzZJ/kbgKckKwA32+32PQDbKysrsNbqoii219fXhyTVgik1k1QBwOHh4ZUsy56wAWstjTHNJeZ5/uL4+HirqdDSaBZJmqY7WZb9OR6PI2strbUcj8d5nuf7eZ5/f3BwMHgXsgv7a5qH6ZQh+YkxZjMMQzjn3nY6nZd+ymBpGS8gnntBWubytPQf2cs89SMALpixH3D5+A9A2ig29VlyAwAAAABJRU5ErkJggg==",
	stop: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAAB/klEQVR4nO2VsY7TQBCGx75Ag2ig4ClokGhoKHgNkCguEVS8CdTQIPEMPEaEBLpHiBJFjj3r3bXX3l3zX3FeMEvCxVYKivzV2v6934x3Zkx01lkjldxmAJASUZIkSXeED3RjxqRo+k3+Wv/L11/fmsjBTYqieC6EWPT3Znt8MyKi7Xb7zBjzNcuy+6OhAFIACTO/qOuaAYCZL2NoWGdZ9tQYkwGAlPIHgAcAkqOhAC6IiIQQCwBQSrXOOTDzPIAiWKG17rqug5Tyy2azuReCHpUlEVGe55fOOSil7BAawwCgLMtPg/cnneOMiIiZ5845aK2t9x5lWb4siuKxMYZjGICLSbB9UO89qqpCURSdEEJVVYX+k58GFkPLsnxljNFSyk4IAWOM11q/HwM72Fv75L3/BqBNkuQnEdk0Tcla+z3sNbnhh4qqUVRVhbqu0TQNpJSd9x55ni+IiJbL5Z1Twn5VoxDiAzO/7qvXxS0zFZYSEa3X6z9goUCIbvrUWguttbXWDqHjCmc4aYwxudbax6V/dXV1l+h3yyilXNu2EEK87QfD8fURT5o+s4/hWcggnBkzz9u2hXMOUkqhlHo0arSFLImIdrvdO6XU52HmkW9GRJTn+ZumaVbM/GT4/ihFv6eDEQffarV6GLyjYcPNjok2QCZlNlUnGWtn/fe6BjOqYn5X7L9uAAAAAElFTkSuQmCC",
	share: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAACiUlEQVR4nN2WvW7UQBDHZ9b2+YSOi51nQKKiTJOSjxfgBahpKFEaSsQ7pEJAw0tAGRFFgoLqpERKpEhEjrxjZ5Wzz/b+KbIXWSLnu3OChJjGXzPz2/nv7qyJ/nfjPkEAPBfbMDPudkh/wrjreZn564KYGXmePw/DcPPo6OgzM18AUMxs1wEvhTkZKcuyXTgzxnw7OTnZdD7qLmGqDRORmda6tNbCGLN/fHwc3wm0DRORXQDQWs9EBFmWIU1T2zQNjDH7aZpuzGNuA1QAPBF57yqzxhhora3WuhERMcagrmtkWfZTRDbdIBdCV5EAAH64+19VVb0OggDD4ZCttS/quv7ieR4BOJhOpyX13Gpz0vViEZGXSZJspWn6qKoqVFUFEdk6Pz8f53n+ph3TG3iTicjTsixRliW01k/Wje+UFMAQQJym6QaA0M1p03KxToVBkiT3AcRnZ2ejtYEAfFfNKyI6ZOaD09PTkdvc15L5vk/MDGaeBUHwgYgOwzD86HLcmLuz0yil7hFRzMzEzHMQWlW2++gGEcXuutA6gS4xAFTzd03TBGEYekRERVEELd/a+da9gXQlHxMRAwARked5SVmWX4mIfN9PALCTlYmIW0r0Al7baDSyRERxHH8nosfz926uVj6iVu59SikfgA9gMJlMwslkEgIIiMh3i2yl/bdqhRiPx0mXg9Z6dmugtZaJyDLzUGv9jpmn1lpWSrUlZLqS9IG1dumZ2AlkZkVXso+jKNrp8r28vCSlFAEY9gFaAJzn+aeiKJ4NBoOHIlLTgnliZvi+r8qyvGDmt51FdH2c+wCIRASLljwARFHEe3t7xfb29nSFnDdbn86/7NRfmnBd6F//bfzn7TdXVcWzva+xYwAAAABJRU5ErkJggg==",
	newtab: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAADCUlEQVR4nO1VsY7kRBB9z257Znc8zLDHIRFBBiHB3okQhAj5AhKEiC8guABICfkJwk0RIKILCViJiA0uOV2EtLtMe8f23NjjrkeA57S3O+uZWwjvSS1L5ap6Xa+7q4BX+J/BbQ6Stvq8kJDUrXcjKb5NTN8mXV8wySBpD8Awz3OR3JhIkqbTKbz3InnRm/OGBATAoii+TpLky6Zp9sysV1qScs6J5M8hhIfj8fjvtb0v7rmMeZ5/I0lt22pXmJkkaTab/SopkhT1VrjW/vz8PHPOPc6y7G5Zln+Q/FFSRNI617b7RgAikpEkA/B+mqafRlHkQgj3R6PR75JikuGm6thVd+C9P5Ok+Xz+sFeSSzg7O7vXNI3qujbv/ceXFVvjWslrbgDWBexLct03mc/nd/M8fyKp6GRPJGXHx8dJkiQTSWvlNlZ1E+E6CJKMZAvgGclVXddLM5sAyADEJFcAnh0eHq4khavxL0P4AoqiuJPn+evOuTsk2+6CDL33UwBvdNJtffS977BDCwBmduyc21+tVgRwUJYlST4A8IWZxfP5/BOSZRRFMLMbk+1CCADY399/O0kSAEBRFAghIMuykXNuBABRFO2ZWb0tzy6EEQAsFotvnXNxCGEo6cFwOBxVVfUIwKPxeJw0TfM0TdN3ukvz3wmn0+l3a4P3/vPhcLjXNM1Pk8nk+0v297YRbr003aOOJWWSBqenp28BOAAQSXpT0kDSa5Icdpg+fRUKAMxs0XWKsrP/NZvN7gNIST4hWQOoASDP8zlJmJnatt1IvpFQ/+riQgiB5Afe+49IxpKCc04hhOri4sJIvuu9H6z/SfqwbVtzzkWDwWBzO9tAFkui9/4HSSqKQsvlUsvlUnVdq65rrVYrtW2rro09X1VVSZLyPH8saSSJV2fjpjM0AIjj+KvFYvFLkiS4OgbNDCEErC+IJEiCcw51Xf8J4DOSFQBeHU9bD7mqqntpmmZlWfb6OecQx3E4OTn5rWtz18h6sUmOl4jduWVew9HRUdyd667rVpt8hVvjH9tHTdc+UNHdAAAAAElFTkSuQmCC",
	bookmarks: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAYAAAByDd+UAAADYklEQVR4nO1Uz2skRRT+XndnZ8Nkus2sCktugsJ6CcgYyGnFk3gUBG+i4EXQgxePc/QHeBEXdMHD/gNijusGcwkuBDzoxRCCIyFhdHamq6qnO2lMd30eUiWTZMZN0IuQBw1V1e+973tfvVfAlf3fTf6rRCSjacciUl8miZAMSUb+63a7wb8hNrNCkuEsdiQDnLAnAOzt7c3Hcfx+GIbzdV0TAFutVmCM6bXb7XskxftOkwHdbjcQkbrf7zdbrdZtAJ0oimit/c1auy4ifQ8sIjaO43kR+XhhYeFsqh8B3HOFTQf0lRlj3m40Gh81Go2nJ/+XZVlprT9PkuRDALWLsSQf5Xm+WFUVAdhWqxUCUGfznwJ0jOvRaHQnjuN38zw3h4eHn4rIwyAIjq21z83Nzb2TJMkHWZatxHH8CoBCRAIAkYhErhIrIiGAcJqCf1cGAGmavkmSeZ7/MBgMbgKA1npNa73m/CJjzB2STNP0a5Kyv79/Q2ut8jynUsoqpaq6rpmm6fe+kLNgQlL6/X7TGGOKohgeHBw86QG01j9prX8mec23f5ZlDxzoMslIKZVdBNAvAhFho9F4KY7juCzLu0tLS8Ner3ddRCqSJclcRP4EYAHAWvuJk+4Nd5cXmml/hwIAQRB0cNLu90mKSwQRuRZFUay1ftEYs7uxsTFOkmTLGAMAqwBCEbEXATylrYiIk/fYzQ1dNe8BuB4EwVaz2fxjeXl5VUQyF3+p1+oUYF3XPRFBGIYvTJ632+3NsixvAbgF4HVr7a/D4fD5ZrNpSe7iRGYPXLv9bHPyIcuyp8bjcW2M+cWdB+5JO9ferkOplHqZZKCUGo/H47qqKhZFwaqq7Gg02vB5poH6sfiMJLMs+2IWQaXUW9Zaaq0f7uzsNJRSTyilHjkCX2qt1936/j8BiqsoNMZskqQx5oEx5lWtdTtN00RrvWKMuUuSR0dHQ631M45kUhQFsyxbA4DhcBgXRfG7UmprJuAkqAv4is78bPl9nuebg8HgWR+X5/lNrfW6MebGhFK3tdbf+hn3vuc6bPJlL4piBcBr1tpVEYGI7JL8Znt7+7tOp3PsH2+S8zgZp9InFxGmaZosLi6OHzsyZ1nN8Dkn02TMLBkflzR0Pp6h97dehUmwi5xd2ZVd2v4ClC2pxjl9PaQAAAAASUVORK5CYII="
};

enyo.kind({
	name: "JihadBrowser",
	kind: "FittableRows",
	classes: "jihad enyo-fit",
	published: {
		//* Current shell URL (mirrors what the address field shows).
		url: ""
	},
	components: [
		// Toolbar built from Mochi controls, laid out with FittableColumns so the
		// address box (fit: true) absorbs the free width; the buttons keep their
		// intrinsic size, so the bar reflows on both TouchPad models. Back/forward
		// start disabled — enabled from page history state (onPageInfoChanged).
		{kind: "FittableColumns", name: "actionBar", classes: "jihad-actionbar", components: [
			{name: "back",    classes: "jihad-nav-btn disabled", allowHtml: true, content: "<img class=jihad-nav-img src='" + enyo.JihadIcons.back + "'>", ontap: "goBack"},
			{name: "forward", classes: "jihad-nav-btn disabled", allowHtml: true, content: "<img class=jihad-nav-img src='" + enyo.JihadIcons.forward + "'>", ontap: "goForward"},
			// Address field with the reload/stop control INSIDE it (Enyo-parity).
			{kind: "mochi.InputDecorator", name: "addressBox", classes: "jihad-address-box", fit: true, components: [
				{kind: "mochi.Input", name: "address", classes: "jihad-address", placeholder: "Search or type a URL", onchange: "addressEntered", onkeydown: "addressKeydown"},
				{name: "reloadStop", classes: "jihad-inline-btn", allowHtml: true, content: "<img class=jihad-nav-img src='" + enyo.JihadIcons.reload + "'>", ontap: "reloadOrStop"}
			]},
			{name: "share",     classes: "jihad-nav-btn", allowHtml: true, content: "<img class=jihad-nav-img src='" + enyo.JihadIcons.share + "'>", ontap: "doShare"},
			{name: "newtab",    classes: "jihad-nav-btn", allowHtml: true, content: "<img class=jihad-nav-img src='" + enyo.JihadIcons.newtab + "'>", ontap: "doNewCard"},
			{name: "bookmarks", classes: "jihad-nav-btn", allowHtml: true, content: "<img class=jihad-nav-img src='" + enyo.JihadIcons.bookmarks + "'>", ontap: "doBookmarks"}
		]},
		// Load progress; shown only during navigation.
		{kind: "mochi.ProgressBar", name: "progress", classes: "jihad-progress", progress: 0, showing: false},
		// Rendered web content: the NPAPI-bound Enyo 2 WebView (JihadWebView.js).
		{kind: "JihadWebView", name: "view", fit: true, classes: "jihad-view",
			onLoadStarted:     "loadStarted",
			onLoadProgress:    "loadProgress",
			onLoadStopped:     "loadStopped",
			onPageInfoChanged: "pageInfoChanged"
		},
		// Start page rendered as APP CHROME (not a size-limited WebView data: URL),
		// so the logo is the crisp bundled asset — matches the Enyo variant. Shown
		// over the (idle) WebView until the first navigation; hidden on loadStarted.
		{name: "startPage", classes: "jihad-startpage", components: [
			{tag: "img", name: "spLogo", classes: "jihad-sp-logo", attributes: {src: "icon-256x256.png"}},
			{content: "Jihad Browser", classes: "jihad-sp-title", allowHtml: false},
			{content: "Mochi UI \u2605 Goanna/6.9 UXP/b2594a4", classes: "jihad-sp-sub", allowHtml: false}
		]},
		// Basic Popup scaffolding. The overflow menu and a generic modal dialog
		// are structured here; their contents (bookmarks / history / downloads /
		// preferences / find / alert-confirm-prompt-SSL) are the T-053 port.
		{kind: "mochi.Popup", name: "menuPopup", classes: "jihad-menu-popup", floating: true, components: [
			// T-053: menu items (New Card, Bookmarks, History, Downloads,
			// Find, Share, Add to Launcher, Preferences) slot in here.
		]},
		{kind: "mochi.Popup", name: "dialog", classes: "jihad-dialog", floating: true, modal: true, centered: true, components: [
			{name: "dialogTitle",   classes: "jihad-dialog-title"},
			{name: "dialogMessage", classes: "jihad-dialog-message"}
		]}
	],

	// --- init + launch parameters -------------------------------------------
	create: function() {
		this.inherited(arguments);
		// webOS relaunch (a second launch of the running app, e.g. an external
		// link tap) redelivers launch params. Register best-effort listeners for
		// the platform relaunch events; harmless where they never fire.
		// [device-gated: relaunch delivery is verified on hardware.]
		if (window.PalmSystem) {
			this._relaunchBind = enyo.bind(this, "relaunchHandler");
			if (document.addEventListener) {
				document.addEventListener("webOSRelaunch", this._relaunchBind, false);
			}
			if (window.addEventListener) {
				window.addEventListener("mojo-relaunch", this._relaunchBind, false);
			}
		}
		this.applyLaunchParams(this.readLaunchParams());
		// Start with the address bar focused so the keyboard is up (all variants).
		enyo.asyncMethod(this, function() { if (this.$.address && this.$.address.focus) { this.$.address.focus(); } });
	},
	//* Read the initial launch parameters. Enyo 2 does NOT populate the Enyo-1
	//* `enyo.windowParams`; the webOS way is to parse PalmSystem.launchParams
	//* (a JSON string). Fall back to enyo.windowParams only if some bootplate
	//* set it.
	readLaunchParams: function() {
		var p = null;
		if (window.PalmSystem && window.PalmSystem.launchParams) {
			try {
				p = enyo.json.parse(window.PalmSystem.launchParams);
			} catch (e) {
				p = null;
			}
		}
		if (!p && window.enyo && enyo.windowParams) {
			p = enyo.windowParams;
		}
		return p || {};
	},
	//* Apply launch params, matching app/source/BrowserApp.js: `target` (external
	//* link / new-card launches) takes precedence over `url`; `webviewId` binds
	//* an engine-created card to this view.
	applyLaunchParams: function(p) {
		p = p || {};
		if (p.webviewId) {
			this.$.view.setIdentifier(p.webviewId);
		}
		var url = p.target || p.url;
		// Launched from the icon with no URL: show the Mochi start page so the card
		// presents content instead of a blank/forever-loading view (device: "the
		// mochi app just loads forever and doesn't open"). Only on the FIRST apply
		// (a later relaunch with no URL must not wipe the current page).
		if (url) { this.setUrl(url); }   // else the app-chrome start page overlay stays visible
	},
	relaunchHandler: function() {
		this.applyLaunchParams(this.readLaunchParams());
		return true;
	},
	urlChanged: function() {
		if (this.url) {
			this.$.view.setUrl(this.url);
			this.$.address.setValue(this.url);
		}
	},

	// --- navigation: forwarded verbatim to the frozen adapter contract -------
	// These four are the exact callBrowserAdapter method names the Enyo 1.0 app
	// uses (Browser.js). Do not add or rename (cavekit-ipc-contract R1).
	goBack:     function() { if (!this.$.back.hasClass("disabled")) this.$.view.callBrowserAdapter("goBack"); },
	goForward:  function() { if (!this.$.forward.hasClass("disabled")) this.$.view.callBrowserAdapter("goForward"); },
	reloadPage: function() { this.$.view.callBrowserAdapter("reloadPage"); },
	stopLoad:   function() { this.$.view.callBrowserAdapter("stopLoad"); },
	//* Find-in-page: the frozen adapter method. The FindBar UI that calls this
	//* is the T-053 parity port; the method is present now so the app's
	//* callBrowserAdapter set matches the Enyo 1.0 app exactly.
	find:       function(str) { this.$.view.callBrowserAdapter("findInPage", [str]); },
	//* Luna services the Enyo 1.0 app invokes from Preferences (Browser.js). The
	//* Preferences UI is the T-053 port; these are present now so the
	//* palm://com.palm.browserServer/* URI set matches the Enyo 1.0 app exactly.
	clearCookies: function() { new PalmServiceBridge().call('palm://com.palm.browserServer/clearCookies', '{}'); },
	clearCache:   function() { new PalmServiceBridge().call('palm://com.palm.browserServer/clearCache', '{}'); },

	// --- address bar --------------------------------------------------------
	// mochi.Input onchange fires on blur/commit; Enter does not blur on its own,
	// so also submit on the Enter keydown (keyCode 13). Both funnel through
	// submitAddress, which de-dupes so an Enter that also blurs won't double-load.
	addressEntered: function(inSender) {
		this.submitAddress();
	},
	addressKeydown: function(inSender, inEvent) {
		var code = inEvent && (inEvent.keyCode || inEvent.which);
		if (code === 13) {
			this.submitAddress();
			if (inSender.hasNode()) {
				inSender.node.blur();
			}
			return true;
		}
	},
	submitAddress: function() {
		var text = this.$.address.getValue();
		if (!text || text === this._lastSubmit) {
			return;
		}
		this._lastSubmit = text;
		this.openUrl(text);
	},
	openUrl: function(text) {
		var url = this.normalizeUrl(text);
		if (!url) {
			return;
		}
		this.url = url;
		this.$.view.setUrl(url);
	},
	//* Turn an address-bar entry into a URL, mirroring app/source/URLSearch.js
	//* (go / looksLikeHost): pass known navigational schemes through untouched
	//* (so about:blank and mailto: are not corrupted), treat a host-looking token
	//* (localhost, host:port, IPv4, host.tld[/path]) as an http:// navigation,
	//* and hand everything else to the default search.
	normalizeUrl: function(text) {
		var v = (text || "").replace(/^\s+|\s+$/g, "");
		if (!v) {
			return v;
		}
		var m = v.match(/^([a-z][a-z0-9+.\-]*):/i);
		if (m) {
			if (this.isNavigableScheme(m[1].toLowerCase())) {
				return v;
			}
			// Unknown / non-navigational scheme (e.g. javascript:) -> search.
			return this.searchUrl(v);
		}
		if (this.looksLikeHost(v)) {
			return "http://" + v;
		}
		return this.searchUrl(v);
	},
	//* Schemes the address bar loads as-is. javascript:/vbscript: are excluded on
	//* purpose (typing them is treated as a search, not an in-page eval).
	isNavigableScheme: function(scheme) {
		switch (scheme) {
			case "http": case "https": case "ftp": case "file":
			case "about": case "mailto": case "tel": case "data":
			case "view-source": case "ws": case "wss": case "rtsp":
				return true;
			default:
				return false;
		}
	},
	//* Heuristic from URLSearch.looksLikeHost: does the raw input denote a host
	//* (=> navigate) rather than search terms?
	looksLikeHost: function(text) {
		var v = (text || "").replace(/^\s+|\s+$/g, "");
		if (!v || /\s/.test(v)) {
			return false;
		}
		if (/^localhost(:\d+)?([\/?#].*)?$/i.test(v)) {
			return true;
		}
		if (/^\d{1,3}(\.\d{1,3}){3}(:\d+)?([\/?#].*)?$/.test(v)) {
			return true;
		}
		return /^[a-z0-9]([a-z0-9\-]*[a-z0-9])?(\.[a-z0-9]([a-z0-9\-]*[a-z0-9])?)*\.[a-z]{2,}(:\d+)?([\/?#].*)?$/i.test(v);
	},
	searchUrl: function(text) {
		return "https://duckduckgo.com/?q=" + encodeURIComponent(text);
	},

	// --- overflow menu (scaffold) -------------------------------------------
	openMenu: function(inSender) {
		// T-053 populates the menu; for now just present the (empty) popup.
		this.$.menuPopup.show();
	},

	// --- WebView (JihadWebView) callbacks -> chrome state -------------------
	hideStartPage: function() { if (this.$.startPage && this.$.startPage.hide) this.$.startPage.hide(); },
	loadStarted: function() {
		this.hideStartPage();
		this._lastProgress = 0;
		this.$.progress.setProgress(0);
		this.$.progress.setShowing(true);
		this.setStopVisible(true);
	},
	loadProgress: function(inSender, inEvent) {
		var p = (inEvent && typeof inEvent.progress === "number") ? inEvent.progress : 0;
		if (p >= this._lastProgress) {
			this.$.progress.setProgress(p);
			this._lastProgress = p;
		}
	},
	loadStopped: function() {
		this.$.progress.setShowing(false);
		this.$.progress.setProgress(0);
		this.setStopVisible(false);
	},
	//* Page info from the engine: {title, url, canGoBack, canGoForward}. Reflect
	//* the URL in the address bar and the history state on the nav buttons. A new
	//* address entry clears the de-dupe guard so the same URL can be retyped.
	pageInfoChanged: function(inSender, inEvent) {
		if (!inEvent) {
			return;
		}
		if (inEvent.url) {
			this.url = inEvent.url;
			this.$.address.setValue(inEvent.url);
			this._lastSubmit = null;
		}
		if (typeof inEvent.canGoBack === "boolean") {
			this.$.back.addRemoveClass("disabled", !inEvent.canGoBack);
		}
		if (typeof inEvent.canGoForward === "boolean") {
			this.$.forward.addRemoveClass("disabled", !inEvent.canGoForward);
		}
	},
	//* Swap the reload/stop affordance during a load.
	setStopVisible: function(loading) {
		this._loading = loading;
		this.$.reloadStop.setContent("<img class=jihad-nav-img src='" + (loading ? enyo.JihadIcons.stop : enyo.JihadIcons.reload) + "'>");
	},
	reloadOrStop: function() {
		if (this._loading) { this.$.view.callBrowserAdapter("stopLoad"); }
		else { this.$.view.callBrowserAdapter("reloadPage"); }
	},
	//* New card (Enyo-parity "new tab"). T-053 fills the other overflow actions.
	doNewCard: function() { if (window.enyo && enyo.windows) { enyo.windows.openWindow("index.html"); } },
	doShare: function() { this.openDialog("Share", "Sharing is part of the feature-parity port (T-053)."); },
	doBookmarks: function() { this.$.menuPopup.show(); },

	// --- generic dialog (scaffold for the T-053 dialog set) -----------------
	openDialog: function(inTitle, inMessage) {
		this.$.dialogTitle.setContent(inTitle || "");
		this.$.dialogMessage.setContent(inMessage || "");
		this.$.dialog.show();
	}
});
