/* Copyright 2026 NotAlexNoyle. MPL-2.0 (matches the GRE this is registered into).
 *
 * about:logins (cavekit-site-data.md R2, T-662): every saved login, found, read, changed and removed with a finger.
 * The logins are the engine's own (nsILoginManager, logins.json in the variant's profile); components/jihadLogins.js
 * saves and fills them. A confirm is asked in the page itself (the row asks, a red button confirms), as Settings' Clear
 * rows do. With a master password set and not yet entered, the page asks for it first and lists nothing until it is
 * right: reading a login would otherwise raise a prompt the card cannot answer.
 * ?filter=<text> opens the list narrowed (about:data's link for one site).
 */
"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;
Cu.import("resource://gre/modules/Services.jsm");

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
function token() {
	try { return Cc["@mozilla.org/security/pk11tokendb;1"].getService(Ci.nsIPK11TokenDB).getInternalKeyToken(); } catch (e) { return null; }
}
function locked() { let t = token(); try { return !!t && t.needsLogin() && !t.isLoggedIn(); } catch (e) { return false; } }
function host(origin) { return String(origin || "").replace(/^[a-z]+:\/\//, ""); }
function when(ms) {
	if (!ms) { return ""; }
	let d = new Date(ms);
	return d.getFullYear() + "-" + ("0" + (d.getMonth() + 1)).slice(-2) + "-" + ("0" + d.getDate()).slice(-2);
}
function copy(v, what) {
	try {
		Cc["@mozilla.org/widget/clipboardhelper;1"].getService(Ci.nsIClipboardHelper).copyString(v);
		say(what + " copied");
	} catch (e) { say("Could not copy: " + e); }
}

let gFilter = "";

function render() {
	let box = document.getElementById("content");
	while (box.firstChild) { box.removeChild(box.firstChild); }
	if (locked()) { renderUnlock(box); return; }
	let all = [];
	try { all = Services.logins.getAllLogins({}); } catch (e) { say("The logins cannot be read: " + e); }
	all.sort((a, b) => host(a.hostname).localeCompare(host(b.hostname)) || String(a.username).localeCompare(String(b.username)));
	document.getElementById("page-sub").textContent = all.length === 1 ? "1 saved login" : all.length + " saved logins";

	let sg = el("div", "group");
	let srow = el("div", "row");
	let search = el("input", "txt search");
	search.setAttribute("type", "search");
	search.setAttribute("placeholder", "Search sites and usernames");
	search.id = "search";
	search.value = gFilter;
	search.addEventListener("input", function () { gFilter = search.value; list(all, lg); }, false);
	srow.appendChild(search);
	sg.appendChild(srow);
	box.appendChild(sg);

	let lg = el("div", "group");
	lg.id = "logins";
	box.appendChild(lg);
	list(all, lg);

	if (all.length) {
		let ag = el("div", "group");
		let arow = el("div", "row clear-row");
		arow.id = "remove-all-row";
		let al = el("label", "row-label", "Remove all saved logins");
		let ah = el("div", "row-help", "Every site's username and password.");
		al.appendChild(ah);
		arow.appendChild(al);
		let ab = button("Remove all", "", function () {
			if (!arow.classList.contains("asking")) {
				arow.classList.add("asking"); ah.textContent = "Remove all " + all.length + " logins? This cannot be undone.";
				ab.textContent = "Remove them"; ab.classList.add("btn-go"); return;
			}
			try { Services.logins.removeAllLogins(); say("Every login removed"); } catch (e) { say("Not removed: " + e); }
			render();
		});
		ab.id = "remove-all";
		arow.appendChild(ab);
		ag.appendChild(arow);
		box.appendChild(ag);
	}
	renderNever(box);
}

function list(all, lg) {
	while (lg.firstChild) { lg.removeChild(lg.firstChild); }
	lg.appendChild(el("div", "group-title", "Logins"));
	let f = gFilter.trim().toLowerCase();
	let shown = all.filter(l => !f || host(l.hostname).toLowerCase().indexOf(f) >= 0 || String(l.username).toLowerCase().indexOf(f) >= 0);
	if (!all.length) {
		lg.appendChild(el("div", "row row-help empty", "No logins are saved. When you sign in to a site, the browser asks whether to remember the login (Settings → Security → Passwords)."));
		return;
	}
	if (!shown.length) { lg.appendChild(el("div", "row row-help empty", "No saved login matches “" + gFilter + "”.")); return; }
	shown.forEach(l => lg.appendChild(loginRow(l)));
}

function loginRow(l) {
	let row = el("div", "row login-row");
	row.setAttribute("data-site", host(l.hostname));
	row.setAttribute("data-user", l.username || "");
	let meta = null;
	try { meta = l.QueryInterface(Ci.nsILoginMetaInfo); } catch (e) {}
	let info = el("div", "row-label");
	let site = el("a", "login-site", host(l.hostname));
	site.setAttribute("href", l.hostname);
	info.appendChild(site);
	let user = el("div", "login-user", l.username ? l.username : "(no username)");
	info.appendChild(user);
	let pw = el("div", "login-pass", "••••••••");
	info.appendChild(pw);
	if (meta) {
		let used = meta.timeLastUsed || meta.timePasswordChanged;
		info.appendChild(el("div", "row-help", (meta.timeLastUsed ? "Last used " : "Changed ") + when(used)));
	}
	let help = el("div", "row-help asking-text", "");
	info.appendChild(help);
	row.appendChild(info);

	let acts = el("div", "login-acts");
	let shownPw = false;
	let show = button("Show", "login-show", function () {
		shownPw = !shownPw;
		pw.textContent = shownPw ? l.password : "••••••••";
		show.textContent = shownPw ? "Hide" : "Show";
	});
	acts.appendChild(show);
	acts.appendChild(button("Copy username", "login-copy-user", function () { copy(l.username, "The username"); }));
	acts.appendChild(button("Copy password", "login-copy-pass", function () { copy(l.password, "The password"); }));
	acts.appendChild(button("Edit", "login-edit", function () { edit(l, row); }));
	let rm = button("Remove", "login-remove", function () {
		if (!row.classList.contains("asking")) {
			row.classList.add("asking"); help.textContent = "Remove this login?"; rm.textContent = "Remove it"; rm.classList.add("btn-go"); return;
		}
		try { Services.logins.removeLogin(l); say("The login for " + host(l.hostname) + " removed"); } catch (e) { say("Not removed: " + e); }
		render();
	});
	acts.appendChild(rm);
	row.appendChild(acts);
	return row;
}

function edit(l, row) {
	while (row.firstChild) { row.removeChild(row.firstChild); }
	row.classList.add("editing");
	let info = el("div", "row-label");
	info.appendChild(el("div", "login-site", host(l.hostname)));
	let u = el("input", "txt edit-user"); u.setAttribute("type", "text"); u.value = l.username || ""; u.setAttribute("placeholder", "Username");
	let p = el("input", "txt edit-pass"); p.setAttribute("type", "text"); p.value = l.password || ""; p.setAttribute("placeholder", "Password");
	// what is typed replaces the old value: a finger cannot easily select a field's text
	[u, p].forEach(function (f) { f.addEventListener("focus", function () { f.select(); }, false); });
	info.appendChild(u); info.appendChild(p);
	row.appendChild(info);
	let acts = el("div", "login-acts");
	acts.appendChild(button("Save", "btn-nav login-save", function () {
		if (!p.value) { say("A login needs a password"); return; }
		try {
			let n = Cc["@mozilla.org/login-manager/loginInfo;1"].createInstance(Ci.nsILoginInfo);
			n.init(l.hostname, l.formSubmitURL, l.httpRealm, u.value, p.value, l.usernameField, l.passwordField);
			Services.logins.modifyLogin(l, n);
			say("The login for " + host(l.hostname) + " saved");
		} catch (e) { say("Not saved: " + e); }
		render();
	}));
	acts.appendChild(button("Cancel", "login-cancel", function () { render(); }));
	row.appendChild(acts);
	u.focus();
}

function renderNever(box) {
	let g = el("div", "group");
	g.id = "never";
	g.appendChild(el("div", "group-title", "Never save logins for"));
	let hosts = [];
	try { hosts = Services.logins.getAllDisabledHosts({}); } catch (e) {}
	hosts.sort();
	if (!hosts.length) { g.appendChild(el("div", "row row-help", "No site. A site added here is never asked about.")); }
	hosts.forEach(function (h) {
		let r = el("div", "row never-row");
		r.setAttribute("data-site", host(h));
		r.appendChild(el("label", "row-label", host(h)));
		r.appendChild(button("Remove", "never-remove", function () {
			try { Services.logins.setLoginSavingEnabled(h, true); } catch (e) { say("Not removed: " + e); }
			render();
		}));
		g.appendChild(r);
	});
	let add = el("div", "row");
	let t = el("input", "txt never-add"); t.setAttribute("type", "url"); t.setAttribute("placeholder", "https://example.com");
	add.appendChild(t);
	add.appendChild(button("Add", "never-add-btn", function () {
		let v = t.value.trim();
		if (!/^[a-z]+:\/\//i.test(v)) { v = "https://" + v; }
		let origin = "";
		try { origin = Services.io.newURI(v, null, null).prePath; } catch (e) {}
		if (!origin || !/^https?:/.test(origin)) { say("Not a site: " + t.value); return; }
		try { Services.logins.setLoginSavingEnabled(origin, false); say(host(origin) + " is never asked about"); } catch (e) { say("Not added: " + e); }
		render();
	}));
	g.appendChild(add);
	box.appendChild(g);
}

function renderUnlock(box) {
	document.getElementById("page-sub").textContent = "Locked by the master password";
	let g = el("div", "group");
	g.id = "unlock";
	g.appendChild(el("div", "group-title", "Master password"));
	let r = el("div", "row");
	let t = el("input", "txt unlock-pass"); t.setAttribute("type", "password"); t.setAttribute("placeholder", "Master password");
	r.appendChild(t);
	r.appendChild(button("Unlock", "btn-nav unlock-btn", function () {
		let ok = false;
		try { ok = token().checkPassword(t.value); } catch (e) {}
		t.value = "";
		if (!ok) { say("That is not the master password"); return; }
		say("");
		render();
	}));
	g.appendChild(r);
	g.appendChild(el("div", "row row-help", "The saved logins are encrypted with the master password set for this browser. Enter it to see them."));
	box.appendChild(g);
}

window.addEventListener("DOMContentLoaded", function () {
	let m = /[?&]filter=([^&#]*)/.exec(String(location.href));   // an about: address: its whole text
	if (m) { try { gFilter = decodeURIComponent(m[1]); } catch (e) { gFilter = m[1]; } }
	render();
}, false);
