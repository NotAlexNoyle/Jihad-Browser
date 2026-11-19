/* Copyright 2026 NotAlexNoyle. MPL-2.0 (matches the GRE this is registered into).
 *
 * about:data (cavekit-site-data.md R3, T-663): what the browser keeps about each site, one site at a time, and the form
 * history: SeaMonkey's Data Manager made a finger's page in the look of this browser's own pages.
 *   SITES: every base domain the engine keeps something for: cookies (nsICookieManager2), permissions
 *     (nsIPermissionManager), saved logins (nsILoginManager), stored data (the quota manager's origins: localStorage,
 *     IndexedDB, the cache API). A site's page lists its cookies (each removable, all removable), its permissions
 *     (Allow, Block or the default, per origin, for the kinds the engine keeps), its logins (a link to about:logins
 *     narrowed to it), its stored data with its size (clearable), and Forget this site, which removes all of those.
 *   FORM DATA: the engine's form history (satchel's FormHistory: what was typed into a page's fields, recorded by
 *     components/jihadLogins.js), searchable, each entry removable, all removable.
 * about:data?site=<domain> opens a site's page; about:data?pane=forms the form history (#forms too). Confirms are asked in the page, as in Settings.
 */
"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu, results: Cr } = Components;
Cu.import("resource://gre/modules/Services.jsm");
let FormHistory = null;
try { FormHistory = Cu.import("resource://gre/modules/FormHistory.jsm", {}).FormHistory; } catch (e) {}

// The kinds of permission shown for a site: the engine's own type names, as Pale Moon's Page Info lists them
const PERMS = [
	["cookie", "Cookies", true], ["image", "Images"], ["popup", "Pop-up windows"], ["plugin:flash", "Adobe Flash"],
	["install", "Add-on installs"], ["geo", "Location"], ["desktop-notification", "Notifications"]
];

function el(tag, cls, text) {
	let n = document.createElement(tag);
	if (cls) { n.className = cls; }
	if (text !== undefined) { n.appendChild(document.createTextNode(text)); }
	return n;
}
function button(text, cls, go) {
	let b = el("button", "btn " + (cls || ""), text);
	b.setAttribute("type", "button");
	b.addEventListener("click", go, false);
	return b;
}
function say(msg) { let s = document.getElementById("status"); if (s) { s.textContent = msg || ""; } }
function base(h) {
	h = String(h || "").replace(/^\./, "").toLowerCase();
	try { return Services.eTLD.getBaseDomainFromHost(h); } catch (e) { return h; }   // an address, localhost
}
function hostOfOrigin(o) {
	let m = /^[a-z-]+:\/\/\[?([^\/\]:^]+)/i.exec(String(o || ""));
	return m ? m[1] : "";
}
function size(n) {
	if (n < 1024) { return n + " bytes"; }
	if (n < 1048576) { return (n / 1024).toFixed(n < 10240 ? 1 : 0) + " KB"; }
	return (n / 1048576).toFixed(1) + " MB";
}
function plural(n, one, many) { return n + " " + (n === 1 ? one : many); }
function principalOf(origin) {
	return Services.scriptSecurityManager.createCodebasePrincipal(Services.io.newURI(origin, null, null), {});
}
function locked() {
	try {
		let t = Cc["@mozilla.org/security/pk11tokendb;1"].getService(Ci.nsIPK11TokenDB).getInternalKeyToken();
		return t.needsLogin() && !t.isLoggedIn();
	} catch (e) { return false; }
}

// Everything kept, by base domain: {cookies: [], perms: [], logins: [], storage: [{origin, usage}], origins: Set}
let gSites = new Map(), gStorage = [], gStorageRead = false;
function siteOf(d) {
	if (!gSites.has(d)) { gSites.set(d, {domain: d, cookies: [], perms: [], logins: [], storage: [], origins: new Set()}); }
	return gSites.get(d);
}
function collect() {
	gSites = new Map();
	try {
		let e = Services.cookies.enumerator;
		while (e.hasMoreElements()) {
			let c = e.getNext().QueryInterface(Ci.nsICookie2);
			let s = siteOf(base(c.host));
			s.cookies.push(c);
			let h = c.host.replace(/^\./, "");
			s.origins.add((c.isSecure ? "https://" : "http://") + h);
		}
	} catch (e) { say("Cookies: " + e); }
	try {
		let e = Services.perms.enumerator;
		while (e.hasMoreElements()) {
			let p = e.getNext().QueryInterface(Ci.nsIPermission);
			let u = null;
			try { u = p.principal.URI; } catch (x) {}
			if (!u || !/^https?$/.test(u.scheme)) { continue; }
			let s = siteOf(base(u.host));
			s.perms.push(p);
			s.origins.add(u.prePath);
		}
	} catch (e) { say("Permissions: " + e); }
	if (!locked()) {
		try {
			Services.logins.getAllLogins({}).forEach(function (l) {
				let h = hostOfOrigin(l.hostname);
				if (!h) { return; }
				let s = siteOf(base(h));
				s.logins.push(l);
				s.origins.add(l.hostname);
			});
		} catch (e) {}
	}
	gStorage.forEach(function (r) {
		let h = hostOfOrigin(r.origin);
		if (!h || !/^https?:/.test(r.origin) || !r.usage) { return; }
		let s = siteOf(base(h));
		s.storage.push(r);
		s.origins.add(r.origin.replace(/\^.*$/, ""));
	});
}
// localStorage is not the quota manager's in this engine (UXP keeps it in the profile's webappsstore.sqlite, one row per
// key, the origin as its reversed host, "." and ":scheme[:port]"), so it is read there: what each origin keeps, in
// characters as the engine counts it against its quota. A page's new data reaches the file within 5 s (the storage
// thread writes in batches), and a clear the same.
function readLocal() {
	let out = [];
	try {
		let f = Services.dirsvc.get("ProfD", Ci.nsIFile);
		f.append("webappsstore.sqlite");
		if (!f.exists()) { return out; }
		let db = Services.storage.openUnsharedDatabase(f);
		try {
			let st = db.createStatement("SELECT originAttributes AS a, originKey AS k, SUM(LENGTH(key) + LENGTH(value)) AS n FROM webappsstore2 GROUP BY originAttributes, originKey");
			try {
				while (st.executeStep()) {
					let m = /^(.*)\.:(https?)(?::(\d+))?$/.exec(st.row.k || "");
					if (!m || st.row.a || !st.row.n) { continue; }   // another scheme, a private window's or a container's
					let origin = m[2] + "://" + m[1].split("").reverse().join("") + (m[3] ? ":" + m[3] : "");
					out.push({origin: origin, usage: st.row.n, kind: "local"});
				}
			} finally { st.finalize(); }
		} finally { db.close(); }
	} catch (e) {}
	return out;
}
// The quota manager answers later: the list is drawn again when it has
function readStorage(then) {
	let done = function (list) { gStorage = list.concat(readLocal()); gStorageRead = true; then(); };
	try {
		let qms = Cc["@mozilla.org/dom/quota-manager-service;1"].getService(Ci.nsIQuotaManagerService);
		qms.getUsage({
			onUsageResult: function (req) {
				let list = [];
				if (req.resultCode === 0 && req.result) {
					for (let i = 0; i < req.result.length; i++) {
						let r = req.result[i];
						list.push({origin: r.origin, usage: r.usage, kind: "quota"});
					}
				}
				done(list);
			}
		});
	} catch (e) { done([]); }
}
// A site's localStorage cleared: the engine's own purge (all its origins, its subdomains'), then the page waits until
// the storage thread has written that (up to its 5 s batch) so a list drawn next does not show it again
function clearLocal(host, then) {
	try { Services.obs.notifyObservers(null, "browser:purge-domain-data", host); } catch (e) {}
	let t0 = Date.now();
	let poll = function () {
		let left = readLocal().filter(x => { let h = hostOfOrigin(x.origin); return h === host || h.endsWith("." + host); });
		if (!left.length || Date.now() - t0 > 8000) { then(); return; }
		setTimeout(poll, 400);
	};
	poll();
}

let gFilter = "";

function clearBox() {
	let box = document.getElementById("content");
	while (box.firstChild) { box.removeChild(box.firstChild); }
	return box;
}
function tabs(which) {
	let nav = document.getElementById("panes");
	while (nav.firstChild) { nav.removeChild(nav.firstChild); }
	[["sites", "Sites"], ["forms", "Form data"]].forEach(function (t) {
		let b = button(t[1], "pane-tab" + (t[0] === which ? " on" : ""), function () {
			// a load of its own, not a #forms on the same document: a card's page that changes without a load keeps the
			// last page's scroll and reports no load done
			location.href = t[0] === "forms" ? "about:data?pane=forms" : "about:data";
		});
		b.id = "tab-" + t[0];
		b.setAttribute("data-pane", t[0]);
		nav.appendChild(b);
	});
}

function renderSites() {
	tabs("sites");
	collect();
	let box = clearBox();
	let sites = Array.from(gSites.values()).sort((a, b) => a.domain.localeCompare(b.domain));
	document.getElementById("page-sub").textContent = plural(sites.length, "site keeps data", "sites keep data");
	let sg = el("div", "group");
	let srow = el("div", "row");
	let search = el("input", "txt search"); search.setAttribute("type", "search"); search.id = "search";
	search.setAttribute("placeholder", "Search sites");
	search.value = gFilter;
	srow.appendChild(search);
	sg.appendChild(srow);
	box.appendChild(sg);
	let lg = el("div", "group");
	lg.id = "sites";
	box.appendChild(lg);
	let draw = function () {
		while (lg.firstChild) { lg.removeChild(lg.firstChild); }
		lg.appendChild(el("div", "group-title", "Sites"));
		let f = gFilter.trim().toLowerCase();
		let shown = sites.filter(s => !f || s.domain.indexOf(f) >= 0);
		if (!sites.length) { lg.appendChild(el("div", "row row-help empty", "No site keeps any data in this browser.")); return; }
		if (!shown.length) { lg.appendChild(el("div", "row row-help empty", "No site matches “" + gFilter + "”.")); return; }
		shown.forEach(function (s) {
			let r = el("div", "row go-row site-row");
			r.setAttribute("data-site", s.domain);
			let l = el("label", "row-label", s.domain);
			let parts = [];
			if (s.cookies.length) { parts.push(plural(s.cookies.length, "cookie", "cookies")); }
			if (s.perms.length) { parts.push(plural(s.perms.length, "permission", "permissions")); }
			if (s.logins.length) { parts.push(plural(s.logins.length, "login", "logins")); }
			let used = s.storage.reduce((n, x) => n + x.usage, 0);
			if (used) { parts.push(size(used) + " stored"); }
			l.appendChild(el("div", "row-help site-counts", parts.join(" · ")));
			r.appendChild(l);
			r.appendChild(button("Open", "btn-nav site-open", function () { location.href = "about:data?site=" + encodeURIComponent(s.domain); }));
			lg.appendChild(r);
		});
	};
	search.addEventListener("input", function () { gFilter = search.value; draw(); }, false);
	draw();
	if (!gStorageRead) { readStorage(renderSites); }
}

function renderSite(domain) {
	tabs("sites");
	collect();
	let box = clearBox();
	let s = gSites.get(domain) || {domain: domain, cookies: [], perms: [], logins: [], storage: [], origins: new Set()};
	document.getElementById("page-title").textContent = domain;
	document.getElementById("page-sub").textContent = "What this browser keeps for the site";
	let back = el("div", "group");
	let br = el("div", "row");
	br.appendChild(button("‹ All sites", "btn-nav site-back", function () { location.href = "about:data"; }));
	back.appendChild(br);
	box.appendChild(back);

	// Cookies
	let cg = el("div", "group");
	cg.id = "cookies";
	cg.appendChild(el("div", "group-title", "Cookies (" + s.cookies.length + ")"));
	if (!s.cookies.length) { cg.appendChild(el("div", "row row-help", "None.")); }
	s.cookies.sort((a, b) => a.host.localeCompare(b.host) || a.name.localeCompare(b.name)).forEach(function (c) {
		let r = el("div", "row cookie-row");
		r.setAttribute("data-name", c.name);
		let l = el("label", "row-label", c.name);
		l.appendChild(el("div", "row-help", c.host + c.path + " · " + (c.isSession ? "until the browser closes" :
			"until " + new Date(c.expiry * 1000).toISOString().slice(0, 10)) + (c.isSecure ? " · secure only" : "")));
		let v = el("div", "row-help cookie-value", "");
		l.appendChild(v);
		r.appendChild(l);
		let shown = false;
		let sb = button("Value", "cookie-show", function () { shown = !shown; v.textContent = shown ? (c.value || "(empty)") : ""; sb.textContent = shown ? "Hide" : "Value"; });
		r.appendChild(sb);
		r.appendChild(button("Remove", "cookie-remove", function () {
			try { Services.cookies.remove(c.host, c.name, c.path, false, c.originAttributes); say("The cookie " + c.name + " removed"); } catch (e) { say("Not removed: " + e); }
			renderSite(domain);
		}));
		cg.appendChild(r);
	});
	if (s.cookies.length) {
		let r = el("div", "row");
		r.appendChild(button("Remove all cookies", "cookies-remove-all", function () {
			s.cookies.forEach(function (c) { try { Services.cookies.remove(c.host, c.name, c.path, false, c.originAttributes); } catch (e) {} });
			say("Every cookie of " + domain + " removed");
			renderSite(domain);
		}));
		cg.appendChild(r);
	}
	box.appendChild(cg);

	// Permissions, per origin: those the site has, and its https origin for the ones it has not
	let pg = el("div", "group");
	pg.id = "permissions";
	pg.appendChild(el("div", "group-title", "Permissions"));
	let origins = Array.from(s.origins).filter(o => /^https?:\/\//.test(o));
	if (!origins.length) { origins = ["https://" + domain]; }
	origins.sort().forEach(function (origin) {
		let principal = null;
		try { principal = principalOf(origin); } catch (e) { return; }
		pg.appendChild(el("div", "row row-help perm-origin", origin));
		PERMS.forEach(function (t) {
			let cur = Services.perms.UNKNOWN_ACTION;
			try { cur = Services.perms.testExactPermissionFromPrincipal(principal, t[0]); } catch (e) {}
			let r = el("div", "row perm-row");
			r.setAttribute("data-perm", t[0]);
			r.setAttribute("data-origin", origin);
			r.setAttribute("data-value", String(cur));   // what the engine holds, as the page was drawn
			r.appendChild(el("label", "row-label", t[1]));
			let sel = el("select", "sel");
			let opts = [[0, "Default"], [1, "Allow"], [2, "Block"]];
			if (t[2]) { opts.push([8, "Allow for the session"]); }
			opts.forEach(function (o) { let op = el("option", "", o[1]); op.value = String(o[0]); sel.appendChild(op); });
			sel.value = String(cur);
			sel.addEventListener("change", function () {
				let v = parseInt(sel.value, 10);
				try {
					if (v === 0) { Services.perms.removeFromPrincipal(principal, t[0]); }
					else { Services.perms.addFromPrincipal(principal, t[0], v); }
					say(t[1] + " for " + origin + ": " + sel.options[sel.selectedIndex].text);
				} catch (e) { say("Not changed: " + e); }
			}, false);
			r.appendChild(sel);
			pg.appendChild(r);
		});
	});
	box.appendChild(pg);

	// Logins
	let lg = el("div", "group");
	lg.id = "site-logins";
	lg.appendChild(el("div", "group-title", "Saved logins"));
	let lr = el("div", "row go-row");
	lr.appendChild(el("label", "row-label", locked() ? "Locked by the master password" : plural(s.logins.length, "saved login", "saved logins")));
	lr.appendChild(button("Saved logins", "btn-nav site-logins-open", function () { location.href = "about:logins?filter=" + encodeURIComponent(domain); }));
	lg.appendChild(lr);
	box.appendChild(lg);

	// Stored data
	let dg = el("div", "group");
	dg.id = "storage";
	dg.appendChild(el("div", "group-title", "Stored data"));
	if (!gStorageRead) { dg.appendChild(el("div", "row row-help", "Reading…")); readStorage(function () { renderSite(domain); }); }
	else if (!s.storage.length) { dg.appendChild(el("div", "row row-help", "None.")); }
	s.storage.forEach(function (x) {
		let r = el("div", "row storage-row");
		r.setAttribute("data-origin", x.origin);
		r.setAttribute("data-kind", x.kind);
		let l = el("label", "row-label", x.origin.replace(/\^.*$/, ""));
		l.appendChild(el("div", "row-help", size(x.usage) + (x.kind === "local" ? " (local storage)" : " (databases, caches)")));
		r.appendChild(l);
		r.appendChild(button("Clear", "storage-clear", function () {
			let again = function () { gStorageRead = false; renderSite(domain); };
			if (x.kind === "local") {
				let h = hostOfOrigin(x.origin);
				say("Clearing the local storage of " + h + "…");
				clearLocal(h, function () { say("The local storage of " + h + " cleared"); again(); });
				return;
			}
			// the list is read again once the quota manager says the clearing is done (it runs on its own thread)
			try {
				let qms = Cc["@mozilla.org/dom/quota-manager-service;1"].getService(Ci.nsIQuotaManagerService);
				let req = qms.clearStoragesForPrincipal(principalOf(x.origin.replace(/\^.*$/, "")));
				say("Clearing the stored data of " + x.origin + "…");
				req.callback = function () {
					say(req.resultCode === Cr.NS_OK ? "The stored data of " + x.origin + " cleared" : "Not cleared: " + req.resultCode);
					again();
				};
			} catch (e) { say("Not cleared: " + e); again(); }
		}));
		dg.appendChild(r);
	});
	box.appendChild(dg);

	// Forget this site
	let fg = el("div", "group");
	let fr = el("div", "row clear-row");
	fr.id = "forget-row";
	let fl = el("label", "row-label", "Forget this site");
	let fh = el("div", "row-help", "Its cookies, permissions, saved logins, stored data and site settings.");
	fl.appendChild(fh);
	fr.appendChild(fl);
	let fb = button("Forget", "site-forget", function () {
		if (!fr.classList.contains("asking")) {
			fr.classList.add("asking"); fh.textContent = "Forget everything this browser keeps for " + domain + "?";
			fb.textContent = "Forget it"; fb.classList.add("btn-go"); return;
		}
		say("Forgetting " + domain + "…");
		forget(domain, s, function () { say(domain + " forgotten"); gStorageRead = false; location.href = "about:data"; });
	});
	fb.id = "forget";
	fr.appendChild(fb);
	fg.appendChild(fr);
	box.appendChild(fg);
}

function forget(domain, s, then) {
	s.cookies.forEach(function (c) { try { Services.cookies.remove(c.host, c.name, c.path, false, c.originAttributes); } catch (e) {} });
	s.perms.forEach(function (p) { try { Services.perms.removePermission(p); } catch (e) {} });
	if (!locked()) { s.logins.forEach(function (l) { try { Services.logins.removeLogin(l); } catch (e) {} }); }
	try {
		Services.logins.getAllDisabledHosts({}).forEach(function (h) {
			if (base(hostOfOrigin(h)) === domain) { Services.logins.setLoginSavingEnabled(h, true); }
		});
	} catch (e) {}
	try {
		let qms = Cc["@mozilla.org/dom/quota-manager-service;1"].getService(Ci.nsIQuotaManagerService);
		s.storage.forEach(function (x) { if (x.kind !== "local") { try { qms.clearStoragesForPrincipal(principalOf(x.origin.replace(/\^.*$/, ""))); } catch (e) {} } });
	} catch (e) {}
	// the local storage last: clearLocal's purge is the engine's "forget this domain" notice, which its other keepers
	// (service workers among them) take too
	let rest = function () { clearLocal(domain, then); };
	try {
		let cps = Cc["@mozilla.org/content-pref/service;1"].getService(Ci.nsIContentPrefService2);
		cps.removeBySubdomain(domain, null, { handleCompletion: rest, handleError: function () {} });
		return;
	} catch (e) {}
	rest();
}

function renderForms() {
	tabs("forms");
	let box = clearBox();
	document.getElementById("page-title").textContent = "Site data";
	document.getElementById("page-sub").textContent = "What was typed into pages' fields";
	if (!FormHistory) { box.appendChild(el("div", "row row-help", "The form history is not available in this build.")); return; }
	let entries = [];
	let sg = el("div", "group");
	let srow = el("div", "row");
	let search = el("input", "txt search"); search.setAttribute("type", "search"); search.id = "search";
	search.setAttribute("placeholder", "Search fields and values");
	srow.appendChild(search);
	sg.appendChild(srow);
	box.appendChild(sg);
	let lg = el("div", "group");
	lg.id = "forms";
	box.appendChild(lg);
	let draw = function () {
		while (lg.firstChild) { lg.removeChild(lg.firstChild); }
		lg.appendChild(el("div", "group-title", "Form data (" + entries.length + ")"));
		let f = search.value.trim().toLowerCase();
		let shown = entries.filter(e => !f || e.fieldname.toLowerCase().indexOf(f) >= 0 || e.value.toLowerCase().indexOf(f) >= 0);
		if (!entries.length) { lg.appendChild(el("div", "row row-help empty", "Nothing typed into a page's fields is kept.")); return; }
		if (!shown.length) { lg.appendChild(el("div", "row row-help empty", "No entry matches.")); }
		shown.forEach(function (e) {
			let r = el("div", "row form-row");
			r.setAttribute("data-field", e.fieldname);
			r.setAttribute("data-value", e.value);
			let l = el("label", "row-label", e.value);
			l.appendChild(el("div", "row-help", e.fieldname + " · used " + plural(e.timesUsed || 1, "time", "times")));
			r.appendChild(l);
			r.appendChild(button("Remove", "form-remove", function () {
				FormHistory.update({op: "remove", fieldname: e.fieldname, value: e.value}, {handleCompletion: function () { say("Removed"); renderForms(); }, handleError: function (x) { say("Not removed: " + x.message); }});
			}));
			lg.appendChild(r);
		});
		if (entries.length) {
			let ar = el("div", "row clear-row");
			ar.id = "forms-remove-all-row";
			let al = el("label", "row-label", "Remove all form data");
			let ah = el("div", "row-help", "");
			al.appendChild(ah);
			ar.appendChild(al);
			let ab = button("Remove all", "forms-remove-all", function () {
				if (!ar.classList.contains("asking")) { ar.classList.add("asking"); ah.textContent = "Remove all " + entries.length + " entries?"; ab.textContent = "Remove them"; ab.classList.add("btn-go"); return; }
				FormHistory.update({op: "remove"}, {handleCompletion: function () { say("Every entry removed"); renderForms(); }, handleError: function (x) { say("Not removed: " + x.message); }});
			});
			ar.appendChild(ab);
			lg.appendChild(ar);
		}
	};
	search.addEventListener("input", draw, false);
	FormHistory.search(["fieldname", "value", "timesUsed", "lastUsed"], {}, {
		handleResult: function (r) { entries.push(r); },
		handleError: function (x) { say("The form history cannot be read: " + x.message); },
		handleCompletion: function () { entries.sort((a, b) => (b.lastUsed || 0) - (a.lastUsed || 0)); draw(); }
	});
}

function route() {
	// an about: address has no search or hash parts to read through location: its whole text is read
	let href = String(location.href);
	let m = /[?&]site=([^&#]*)/.exec(href);
	if (m) { renderSite(decodeURIComponent(m[1]).toLowerCase()); return; }
	if (/[?&]pane=forms\b/.test(href) || /#forms$/.test(href)) { renderForms(); return; }
	renderSites();
}

window.addEventListener("DOMContentLoaded", route, false);
window.addEventListener("hashchange", route, false);
