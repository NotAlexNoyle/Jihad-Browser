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
- [ ] Submitting a form with a password field and a username field asks, in the card, whether to remember the login for
  that site; OK saves it (username, password, the site's origin, the form's action origin) and Cancel saves nothing
- [ ] A login saved for a site fills that site's login form when the page loads again: the username and the password
  fields hold the saved values, and the page's own input events fire as if typed
- [ ] Submitting a known username with a new password asks whether to update the saved password; OK changes it
- [ ] With Settings' "Remember logins for sites" off nothing is asked and nothing saved; with "Fill in logins
  automatically" off a saved login is not filled
- [ ] A site on the never-save list (R2) is not asked about
- [ ] A form in a frame of another origin than the page is filled only for the frame's own origin
- [ ] Logins persist across a daemon restart and live in the variant's own profile (`logins.json`), encrypted by the
  engine's own login storage
**Dependencies:** cavekit-browser-services.md, cavekit-preferences-ui.md

### R2: about:logins
**Description:** Every saved login, findable, readable, changeable and removable with a finger.
**Acceptance Criteria:**
- [ ] `about:logins` lists every saved login as a row: the site, the username, the password hidden, the date last used
  or changed; sorted by site; an empty list says how logins get saved
- [ ] A search field narrows the list to the logins whose site or username holds the text
- [ ] A row's Show reveals its password and Hide hides it again; with a master password set and not entered, the page
  asks for it before it lists anything (a locked store gives no login, its username included: revised 2026-11-18 from
  "Show asks for it first"), a wrong one shows nothing and the right one lists the logins
- [ ] Copy username and Copy password put the value on the system clipboard (cavekit-input-bridging's Paste reads it)
- [ ] Edit changes a login's username and password in place and saves them; the change is what R1 then fills
- [ ] Remove deletes one login after a confirm in the page; Remove all deletes every login after a confirm
- [ ] The never-save list is listed below, each with Remove, and a site can be added to it there (the card's dialog has
  only OK and Cancel, so "never for this site" is said in this page)
- [ ] A site's name opens the site in the card
**Dependencies:** R1

### R3: about:data
**Description:** What the browser keeps about each site, one site at a time, and the form history.
**Acceptance Criteria:**
- [ ] `about:data` lists every site the browser keeps something for (cookies, permissions, saved logins, stored data),
  grouped by the site's base domain, each with counts of what is kept; a search field narrows it
- [ ] A site opened from the list shows its cookies (name, host, path, expiry, value on demand), each removable, and
  Remove all cookies for the site
- [ ] The site's permissions are shown and changeable among Allow, Block and the default: cookies, images, pop-ups,
  plugins (Flash), add-on installs, location, notifications, as the engine keeps them
- [ ] The site's saved logins are counted with a link to `about:logins` narrowed to the site
- [ ] The site's stored data (localStorage, IndexedDB, the cache it holds, as the quota manager reports) is shown with
  its size and can be cleared
- [ ] **Forget this site** clears every kind of data the engine keeps for it, after a confirm, and the site leaves the
  list
- [ ] The form history is listed (field name and value) with a search, each entry removable, and Remove all
**Dependencies:** R1

### R4: In all three interfaces, safe, and reached the ways a person looks
**Description:** The two pages are the browser's own, in every shell, and out of a web page's reach.
**Acceptance Criteria:**
- [ ] Both pages open from the address bar in Enyo, Mochi and Mojo, drawn in that shell's look (the browser's own pages'
  colours, the Persona's where the other pages take it), every control at least 44 px tall
- [ ] Settings has "Saved logins…" (to `about:logins`) in its Passwords group and "Site data…" (to `about:data`) in its
  Cookies group, in the three shells
- [ ] A web page cannot open, frame or link into either page (no `URI_SAFE_FOR_UNTRUSTED_CONTENT`), and either page
  read by a web page's script gives nothing
- [ ] Each criterion of R1-R3 is a finger script of the suite run in the three shells (`docs/FINGER-TESTS.md`), from test
  pages served on the host (a login form, a page that sets cookies and stores data, a page that asks a permission)
**Dependencies:** R1, R2, R3, cavekit-chrome-look.md, cavekit-finger-testing.md
