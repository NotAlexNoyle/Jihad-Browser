---
created: "2026-11-18"
last_edited: "2026-11-18"
complexity: medium
---

# Cavekit: Saved Logins and Site Data (about:logins, about:data)

## Scope
The owner, 2026-11-18: **"new kit: ensure about:data works (all variants)"** and **"new kit: ensure about:logins works
(all variants)"**.

Neither page exists in this engine. UXP's toolkit registers no `about:data` and no `about:logins`; Pale Moon keeps
saved passwords in a XUL dialog (`passwordManager.xul`) and site permissions in another, both made for a chrome window
this embedding does not have. `about:data` is the name SeaMonkey's Data Manager made familiar: one page for what the
browser keeps about each site (its cookies, its permissions, its saved logins, its stored data) and the form history.
`about:logins` is the name Firefox gives its saved-logins page. Both are built here, in the same way as
`about:preferences` (`render/goanna/components/jihadAboutPreferences.js`, `packaging/prefsui/`): a chrome document with
the system principal, shared by the three interfaces, in the look of the browser's own pages (cavekit-chrome-look.md).

Measured on reading the source (2026-11-18), before any page: **the password manager is not connected in this
embedding.** Its content half (`LoginManagerContent.jsm`) finds its parent through the docshell's frame message
manager, which an `nsIWebBrowser` embedding does not give, and nothing calls `LoginManagerParent.init()`. Settings'
"Remember logins for sites" therefore changes nothing: no login is ever saved and none is ever filled. A page that lists
saved logins is only worth having once logins are saved, so that comes first (R1).

**Out of scope:** a master password's set-up UI (Pale Moon's is a separate dialog; a master password already set is
honoured, R2); syncing; importing from other browsers; the per-site permission prompts themselves (what asks for a
permission is each feature's own kit; this kit shows and changes what was decided).

Reference: UXP `toolkit/components/passwordmgr/` (`nsILoginManager`, `LoginHelper.jsm`, `storage-json.js`),
`netwerk/cookie/` (`nsICookieManager2`), `extensions/cookie/` (`nsIPermissionManager`), `toolkit/components/satchel/`
(`FormHistory.jsm`), `toolkit/forgetaboutsite/ForgetAboutSite.jsm`, `dom/quota/` (`nsIQuotaManagerService`),
`render/goanna/DialogService.cpp` (how a prompt reaches the card).

## Requirements

### R1: Logins are saved and filled
**Description:** A login form submitted on a site offers to remember the login; a remembered login fills the form the
next time.
**Acceptance Criteria:**
- [x] Submitting a form with a password field and a username field asks, in the card, whether to remember the login for
  that site; OK saves it (username, password, the site's origin, the form's action origin) and Cancel saves nothing *(2026-11-18, T-661: `site-logins-save` (OK: about:logins lists it, the next load is filled) and `site-logins-refuse-update` (Cancel: nothing saved), Enyo, Mochi and Mojo, docs/rounds/finger-2026-11-17. The login is an nsILoginInfo with the page's origin and the form's action origin (`render/goanna/components/jihadLogins.js`); the question is asked at earlyformsubmit, before the navigation, as a page's confirm() is)*
- [x] A login saved for a site fills that site's login form when the page loads again: the username and the password
  fields hold the saved values, and the page's own input events fire as if typed *(2026-11-18: `site-logins-save`, the three shells: the page's own readout `user=jihad-tester pass=fill-me-3 events=4`)*
- [x] Submitting a known username with a new password asks whether to update the saved password; OK changes it *(2026-11-18: `site-logins-refuse-update`, the three shells)*
- [x] With Settings' "Remember logins for sites" off nothing is asked and nothing saved; with "Fill in logins
  automatically" off a saved login is not filled *(2026-11-18: `site-logins-switches`, the three shells (the prefs Settings' two rows write))*
- [x] A site on the never-save list (R2) is not asked about *(2026-11-18: `site-logins-switches`, the three shells: added in about:logins, no dialog in 5 s; removed, asked again)*
- [x] A form in a frame of another origin than the page is filled only for the frame's own origin *(2026-11-18: `site-logins-frame`, the three shells: a page on :8077 framing :8079 (fixture `second-origin`); the frame filled with :8079's login, never :8077's)*
- [x] Logins persist across a daemon restart and live in the variant's own profile (`logins.json`), encrypted by the
  engine's own login storage *(2026-11-18: `site-logins-persist`, the three shells: filled again after `daemon-restart`; `logins.json` in the shell's own profile holds `encryptedPassword` and neither the username nor the password in clear)*
**Dependencies:** cavekit-browser-services.md, cavekit-preferences-ui.md

### R2: about:logins
**Description:** Every saved login, findable, readable, changeable and removable with a finger.
**Acceptance Criteria:**
- [x] `about:logins` lists every saved login as a row: the site, the username, the password hidden, the date last used
  or changed; sorted by site; an empty list says how logins get saved *(2026-11-18, T-662: `site-logins-page` (rows with the site, the username, the password as dots, "Last used"; the empty list's text) and `site-logins-frame` (rows of :8077 then :8079), the three shells; the order is the page's sort by site then username)*
- [x] A search field narrows the list to the logins whose site or username holds the text *(2026-11-18: `site-logins-page`, the three shells: "jihad-beta" typed in the search on the system keyboard leaves that login alone, and the address's `?filter=` does the same)*
- [x] A row's Show reveals its password and Hide hides it again; with a master password set and not entered, the page
  asks for it before it lists anything (a locked store gives no login, its username included: revised 2026-11-18 from
  "Show asks for it first"), a wrong one shows nothing and the right one lists the logins *(2026-11-18: `site-logins-page` (Show, Hide) and `site-logins-master` (the master password set by a trace-mode test hook; the page asks first, a wrong one lists nothing, the right one lists and Show reveals), the three shells)*
- [x] Copy username and Copy password put the value on the system clipboard (cavekit-input-bridging's Paste reads it) *(2026-11-18: `site-logins-page`, the three shells: `/var/tmp/webkit-clipboard`, the card WebKit's shared clipboard that every card's Paste reads, holds the password, then the username. Found on the way: the engine's clipboard helper kept the text in the daemon only; it now writes that file (`render/goanna/EngineHost.cpp`))*
- [x] Edit changes a login's username and password in place and saves them; the change is what R1 then fills *(2026-11-18: `site-logins-page`, the three shells: the username and the password edited, Show reads the new password, and the login page is then filled `user=jihad-beta-two pass=edited-pass-7`)*
- [x] Remove deletes one login after a confirm in the page; Remove all deletes every login after a confirm *(2026-11-18: `site-logins-page`, the three shells: each asks in the page ("Remove this login?", "Remove all 1 logins? This cannot be undone.") before it acts)*
- [x] The never-save list is listed below, each with Remove, and a site can be added to it there (the card's dialog has
  only OK and Cancel, so "never for this site" is said in this page) *(2026-11-18: `site-logins-switches`, the three shells)*
- [x] A site's name opens the site in the card *(2026-11-18: `site-logins-page`, the three shells: the row's site tapped, the card on http://192.168.1.245:8077)*
**Dependencies:** R1

### R3: about:data
**Description:** What the browser keeps about each site, one site at a time, and the form history.
**Acceptance Criteria:**
- [x] `about:data` lists every site the browser keeps something for (cookies, permissions, saved logins, stored data),
  grouped by the site's base domain, each with counts of what is kept; a search field narrows it *(2026-11-18, T-663: `site-data-page`, Enyo, Mochi and Mojo, docs/rounds/finger-2026-11-17: the host's two origins (:8077, and :8079 by the fixture `second-origin`) are one row, `192.168.1.245`, with its counts ("1 login", stored data); the search narrows the list and says "No site matches". Base domains are the engine's eTLD service's (`data.js` `base()`); a host that is an address is its own)*
- [x] A site opened from the list shows its cookies (name, host, path, expiry, value on demand), each removable, and
  Remove all cookies for the site *(2026-11-18: `site-data-page`, Enyo, Mochi and Mojo, docs/rounds/finger-2026-11-17: the cookies listed (each row the cookie's name, host, path and expiry: `data.js`), a value shown on Show; one removed, then the rest with Remove all cookies)*
- [x] The site's permissions are shown and changeable among Allow, Block and the default: cookies, images, pop-ups,
  plugins (Flash), add-on installs, location, notifications, as the engine keeps them *(2026-11-18: `site-data-page`, Enyo, Mochi and Mojo, docs/rounds/finger-2026-11-17: a row per origin for each of the seven kinds (`data.js` PERMS); Pop-up windows set to Block, the page read again from the engine shows Block. The values are written to and read from nsIPermissionManager)*
- [x] The site's saved logins are counted with a link to `about:logins` narrowed to the site *(2026-11-18: `site-data-page`, Enyo, Mochi and Mojo, docs/rounds/finger-2026-11-17: "1 saved login", and its link opens `about:logins?filter=192.168.1.245` listing it)*
- [x] The site's stored data (localStorage, IndexedDB, the cache it holds, as the quota manager reports) is shown with
  its size and can be cleared *(2026-11-18: `site-data-page`, Enyo, Mochi and Mojo, docs/rounds/finger-2026-11-17: the test page keeps 4 KB of localStorage and 8 KB of IndexedDB on each origin; the site's page lists both origins' local storage (read from webappsstore.sqlite: UXP keeps it outside the quota manager) and databases (the quota manager's usage, 48 KB); :8077's local storage and databases each cleared, :8079's left. Found on the way: the databases' Clear read the list again before the quota manager had cleared; it now waits for the request's callback)*
- [x] **Forget this site** clears every kind of data the engine keeps for it, after a confirm, and the site leaves the
  list *(2026-11-18: `site-data-page`, Enyo, Mochi and Mojo, docs/rounds/finger-2026-11-17: after its confirm ("Forget everything…"), the site with both origins leaves the list. The form history is not the site's to forget: the engine keeps it by field name and value, with no site (Firefox's Forget About Site leaves it too); it is cleared in its own pane (the criterion below). A locked key store keeps the site's logins, and Forget says so (Codex review 2026-11-19))*
- [x] The form history is listed (field name and value) with a search, each entry removable, and Remove all *(2026-11-18: `site-data-page`, Enyo, Mochi and Mojo, docs/rounds/finger-2026-11-17: the username and the note typed into the test form listed with their field names; the search narrows them and says "No entry matches"; the note removed, then Remove all after its confirm)*
**Dependencies:** R1

### R4: In all three interfaces, safe, and reached the ways a person looks
**Description:** The two pages are the browser's own, in every shell, and out of a web page's reach.
**Acceptance Criteria:**
- [x] Both pages open from the address bar in Enyo, Mochi and Mojo, drawn in that shell's look (the browser's own pages'
  colours, the Persona's where the other pages take it), every control at least 44 px tall *(2026-11-18, T-664: `site-data-address`, the three shells: `about:logins` and `about:data` typed on the system keyboard (its ':' from the symbols layer) into the address bar and Enter; the kept pictures (`kept/*-site-data-address-*`) show each shell's chrome around the browser's own pages' dark look; `site-data-reach` reads the search fields, the never-save list's Add button and the pane tabs at 44 to 46 px; the rows' buttons, fields and selects take `sitedata.css`'s 44 px minimum)*
- [x] Settings has "Saved logins…" (to `about:logins`) in its Passwords group and "Site data…" (to `about:data`) in its
  Cookies group, in the three shells *(2026-11-18: `site-data-reach`, the three shells: each row tapped opens its page)*
- [x] A web page cannot open, frame or link into either page (no `URI_SAFE_FOR_UNTRUSTED_CONTENT`), and either page
  read by a web page's script gives nothing *(2026-11-18: `site-data-reach`, the three shells: a page on :8077 frames about:logins and calls window.open("about:data"); the frame stays an empty document (title ""), no window opens, and nothing of "Saved logins" reaches the page; the about: module does not set URI_SAFE_FOR_UNTRUSTED_CONTENT (`render/goanna/components/jihadAboutPreferences.js`))*
- [x] Each criterion of R1-R3 is a finger script of the suite run in the three shells (`docs/FINGER-TESTS.md`), from test
  pages served on the host (a login form, a page that sets cookies and stores data, a page that asks a permission) *(2026-11-18: `site-logins-{save,refuse-update,switches,page,frame,persist,master}`, `site-data-{page,reach,address}`, every one passing in the three shells in docs/rounds/finger-2026-11-17, from `jihad-login.html` (a login form that sets two cookies and keeps local storage and IndexedDB data), `jihad-login-frame.html` and the second origin on :8079. No page here asks for a permission that is then kept: this embedding has no permission prompt (the prompts are each feature's own kit, this kit's Out of scope); the permission is set in about:data and read back from the engine)*
**Dependencies:** R1, R2, R3, cavekit-chrome-look.md, cavekit-finger-testing.md
