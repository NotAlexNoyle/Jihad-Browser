---
created: "2026-10-11"
last_edited: "2026-10-12"
complexity: complex
---

# Cavekit: Jihad's Add-on Site (addons.riverstonerelay.org)

## Scope
Pale Moon and Basilisk run their own add-on sites on **Phoebus**, Moonchild Productions' PHP site
software (MPL-2.0): Personas at `/personas/` as `data-browsertheme` buttons, extensions and themes,
search plugins, language packs, an update service (`?component=aus`) and the discover and search
APIs the add-ons manager calls. Jihad has had only a page inside the browser (`about:personas`,
cavekit-personas-ui.md R8) waiting for an index someone hosts.

The owner, 2026-10-11, choosing among the options in `docs/research/persona-archives.md`:
- hosting: **a Phoebus add-on site** at `addons.riverstonerelay.org` (the owner's domain);
- server: **one the owner already runs: Devuan, nginx, OpenRC** (PHP-FPM and MariaDB to add);
- scope: **the full Phoebus feature set**, with **Personas populated first, the rest later**;
- who adds content: **the owner only** (no public accounts);
- curation: **Jihad's own art plus hand-checked CC BY / BY-SA**, credited (as the bundle);
- in the browser: **about:personas reads a paged, searchable feed from the site, and the site's
  own buttons work too**; updates are checked **on demand only**;
- look: **a Palm-era webOS skin of our own**;
- first step: **write the kit and build it locally**; nothing is published, no DNS is touched, and
  nothing is pushed without the owner's go-ahead.

The code base is analysed in `docs/research/phoebus-analysis.md` (2026-10-11): as shipped it
renders no page on PHP 8.2/8.4, its database password sits inside the web root, registration is
open and feeds stored XSS into the admin pages, the panel has no CSRF protection, `/special/`
serves phpinfo to anyone, Personas have no way in except hand-written SQL and no `version`, and
the Personas page breaks on an apostrophe. This kit makes it a site fit to put on the internet.

The site's code lives in its own repository, `../Riverstone-Addons` (a fork of
`repo.palemoon.org/MoonchildProductions/phoebus`, branch TRUNK at `ed59173`, history kept). The
browser's half lives here.

## Requirements — the site

### R1: A fork we may run and share
**Description:** The site is Phoebus's code under its own licence, with nothing of Moonchild's that
is not ours to use.
**Acceptance Criteria:**
- [x] The repository keeps Phoebus's history and licences (MPL-2.0 for the code; Smarty LGPL-3.0,
  SafeMySQL Apache-2.0, librdf LGPL-2.1, the Silk icons CC BY 2.5 credited) and says in its
  README what it is, where it came from and what was changed. *(Riverstone-Addons: upstream history kept, `libraries/VERSIONS`, `NOTICE`.)*
- [x] The deployed tree carries no Pale Moon, Basilisk, Epyrus or Interlink skin, wordmark, font
  (Museo), discover-pane picture, shared category icon of unknown origin, or favicon; no
  application entry but Jihad's; no Moonchild mail address, link, ban list or named-user query. *(What is left: the "powered by Phoebus" credit link, a code comment's source URL, `palemoon.org` in the ids only the owner may submit, and Phoebus's dead Interlink branch.)*
- [x] Every page says "Jihad Browser Add-ons", never "Pale Moon" or "Phoebus 2.4.2" as its brand
  (a "powered by Phoebus" credit is kept). *(The panel's own pages say "Riverstone Add-ons".)*
**Dependencies:** none

### R2: It runs on PHP 8.2 and 8.4
**Description:** Devuan 5 ships PHP 8.2 and Devuan 6 PHP 8.4; the site works on both.
**Acceptance Criteria:**
- [x] Every page type (front page, each category, an add-on page, a licence page, search, the
  Personas page, the feed, the update service, the discover pane, the panel) answers 200 with
  no PHP notice, warning or deprecation in the response or the log, on PHP 8.2 and on 8.4. *(Local copy, 2026-10-11, both versions: every page type, the services and the panel's pages; the PHP log empty.)*
- [x] Smarty is a maintained release with the 2026 security fixes (4.5.8 or later). *(4.5.8.)*
- [x] A notice or deprecation is logged, not turned into an error page; a real error shows a plain
  page with no file names, SQL or credentials. *(A 404 and an error page are the site's own, no paths or SQL — the public ones in the site's skin since 2026-10-11.)*
**Dependencies:** R1

### R3: Safe to put on the internet
**Description:** Every hole in the analysis (P0–P2) is closed before the first deploy.
**Acceptance Criteria:**
- [x] The database credentials and all configuration live outside the web root; nginx serves no
  dot-file and nothing under the configuration directory. *(Request for each → 404.)* *(Smoke test.)*
- [x] There is no registration, verification or password-mail path in the code; there is no
  `special` component; `component=panel` is honoured only for a `/panel/` request path. *(Since the inspector's 2026-10-11 review PHP takes the panel only from nginx's /panel/ location: /panel/../ and /panel/%2e%2e/ answer 404, in the smoke test.)*
- [x] Every panel form that changes something carries a per-user token and checks the request's
  origin; a forged cross-site POST is refused. *(Test: a POST without the token → refused.)* *(Local copy: a POST without the token → 403, a POST from another origin → 403, nothing changed.)*
- [x] Templates escape every value by default; the few pre-rendered HTML fields are marked
  explicitly. A name with `'`, `"`, `<` or `&` shows as text everywhere, the Personas page's
  `data-browsertheme` included. *(Local copy: a Persona named `Au"r<o>ra & O'Brien` showed as text on /personas/, came back intact from its `data-browsertheme` JSON, and was `\u`-escaped in the feed. Descriptions are no longer escaped twice.)*
- [x] The second-order SQL injection (`findUserAddon`) is fixed; `panel=1` downloads need the panel's
  login; `debugOff` is gone; inactive and unreviewed packages are not public by direct URL. *(Panel downloads moved to /panel/download/, behind its login. Local copy: the test add-on set unreviewed → its page and public download 404, its update check offers nothing, the panel still serves it.)*
- [x] The panel is behind a rate limit and, optionally, an address allowlist; responses carry HSTS,
  `X-Frame-Options`, `X-Content-Type-Options` and a Content-Security-Policy; forwarded-for headers
  are not trusted. *(Smoke test: the four headers, and X-Frame-Options on everything but the discover pane itself.)*
- [x] A request whose normalised path is outside `/panel/` never reaches the panel or its login
  check: `/panel/../?path=/panel/administration/` and `/panel/%2e%2e/?path=…` answer 404, and the
  panel's downloads are `/panel/download/`, inside its rate limit and allowlist. *(Smoke test; local
  copy: `?component=download&panel=1` no longer asks for a password.)* (Inspector, 2026-10-11.)
- [x] Panel responses carry `X-Frame-Options` and `frame-ancestors` whatever their query string says;
  only `/?component=discover` itself, with nothing else in its query, may be framed — a query that
  names `component` twice (nginx reads the first, PHP the last) is framed by the stricter rule.
  *(Smoke test: `/panel/?component=discover` and `/?component=discover&component=site&path=/personas/`
  keep the header, `/?component=discover` has none.)* (Inspector, 2026-10-11, rounds 14 and 15.)
- [x] Codex (or the inspector) reviews the site's code before any deploy, and no P0/P1 is open. *(Inspector, 2026-10-11, with Codex out of credits: one P1, the panel path, and the P2/P3s all fixed — `context/impl/impl-review-findings-6.md` round 14. Codex reviews it again when its credits return, before the deploy.)*
**Dependencies:** R2

### R4: The Jihad application
**Description:** The site serves one application: Jihad Browser.
**Acceptance Criteria:**
- [x] `addons.riverstonerelay.org` is Jihad's site, with Jihad's add-on id
  `{4534aac8-d8c8-4765-95ee-7f61fd0b762d}` (`render/goanna/JihadUserAgent.h`, frozen), its user-agent
  token `JihadBrowser/`, and a minimum version below 1.0 so Jihad 1.0 is not "old".
- [x] Every feature is on (extensions, themes, Personas, search plugins, language packs); the
  categories that have nothing yet say so plainly instead of erroring.
- [x] Every URL the site writes (pages, pictures, downloads, updates, the discover pane) is https.
**Dependencies:** R2

### R5: Personas on the site
**Description:** The Personas page lists what the project may share, with credits, and each
Persona installs into Jihad, Pale Moon and Basilisk with one tap.
**Acceptance Criteria:**
- [x] Personas have a version, a licence, a licence URL, a source URL and a credit line; ids are
  `[a-z0-9-]` only.
- [x] An importer adds or updates Personas from the site's own catalogue, `catalogue/personas.json`
  (since 2026-10-12; before, Jihad's `render/goanna/personas/`, which now holds only the two the
  browser bundles), copying the pictures with their sha256 checked; running it twice changes nothing;
  a changed picture or colour slots bump the version. *(Local copy: 39 entries; a second run changes
  nothing.)*
- [x] The catalogue carries three kinds (the server agent's work order, 2026-10-12): classic (a
  header picture), colour-only — the importer draws a 3000x200 header from the frame and toolbar
  colours, so Pale Moon and Basilisk can wear it too — and layered (up to 7 pictures merged into one
  header as a toolbar draws them, within cavekit-persona-formats.md R2's limits); any of them may
  carry the 15 colour slots, each one CSS colour (cavekit-persona-formats.md R3's rules). Every kind
  gets a preview; a colour theme's is a small toolbar swatch. *(Local copy; layered with a scratch
  index only — no licensed layered art in the catalogue yet.)*
- [x] 23 colour-only Personas from MIT-licensed palettes (the owner, 2026-10-12: allowed, credited):
  Catppuccin ×4, Nord, Dracula, Solarized ×2, Gruvbox ×2, Tokyo Night ×3, Rosé Pine ×3,
  Everforest ×2, Kanagawa ×3, One Dark, One Light; each credited with its licence and source in
  `catalogue/CREDITS.md`. *(Device, Enyo, local copy: Catppuccin Mocha installed from the gallery
  colours the toolbar rgb(17, 17, 27), its text rgb(205, 214, 244), the address field dark.)*
- [x] `/personas/` lists them with picture, name, author, version and licence; each button installs
  the Persona (an `InstallBrowserTheme` event on an element whose `data-browsertheme` is valid JSON).
  *(Device: a tap-equivalent in Jihad installs one through the usual confirm.)* *(Device, Enyo: Wear on the site's page, through the usual confirm.)* Pale Moon and Basilisk: R10's smoke test.
- [x] The update service answers `?component=aus&persona=<id>` with the Persona including its
  version (https only), `{}` for an unknown or inactive one.
- [x] Previews are made when a Persona is imported, never while a page is being served.
**Dependencies:** R3, R4

### R6: The feed Jihad reads
**Description:** A small JSON feed for about:personas: paged, searchable, Personas only.
**Acceptance Criteria:**
- [x] `/?component=personas&page=N&per_page=M&q=TEXT` returns
  `{page, pages, total, personas: [...]}`; each Persona has string-valued `id, name, author,
  version, headerURL, previewURL, accentcolor, textcolor, updateURL, license, licenseURL` (and
  `footerURL`, `description`, `colors` when it has them), all URLs https. *(Every field is present since 2026-10-11, "" when a Persona has none.)*
- [x] Since 2026-10-12 each also has `kind` ("classic", "colors" or "layered") and `colors` — "" or
  a JSON string of the slot object, so every field stays a string; the update service and the
  Personas page's `data-browsertheme` carry the same. *(Local copy: 40 entries, the same 17 string
  fields each; Jihad reads `colors` as that string — cavekit-persona-formats.md R4.)*
- [x] A colour Persona's browser id comes from its colours, not from the header the site drew for it:
  its catalogue entry's own `browserId` at first import, else `static-theme:` and 24 hex of the
  sha256 of its frame, text and slot colours (compact JSON, fixed order), so two palettes sharing
  a frame and toolbar are two Personas, and redrawing a header never changes an id. The version
  goes up only when what a browser draws changes (picture, colours, kind, a colour Persona's name).
  *(Local copy: the 23 palettes re-imported under these ids, all distinct; a second import changes
  nothing; the smoke test checks the feed's contract on every page — round 20.)*
- [x] `q` matches name, author and description, in any language (UTF-8 kept); `per_page` is capped
  (at most 50); an out-of-range page, however large its number, is an empty list, not an error. *(A page past the end, even 999999999999999999, is an empty list.)*
- [x] The response is cacheable for a few minutes and has the JSON content type; HTML-significant
  characters are escaped in the JSON.
**Dependencies:** R5

### R7: A Palm-era skin of our own
**Description:** The site looks like it belongs beside the README: the webOS 3 era, Jihad's own art.
**Acceptance Criteria:**
- [x] A new skin (`base/skin/jihad/` and the front page) drawn for the project: Palm-era greys and
  rounded cards, the project's own logo and pictures, no third-party art without a licence that
  allows it (credited).
- [~] Every page reads well on the TouchPad (1024×768 and 768×1024, a finger's tap targets) and on a
  desktop and a phone; the error pages use the same skin. *(Device: portrait, Enyo; the public error pages in the skin since 2026-10-11. Headless Firefox, 2026-10-11: 1024×768, a 1366-wide desktop and a 390-wide phone — front, Personas, add-on, search and 404 pages; on a phone the tabs wrap to two rows and the cards stack. Left: landscape on the TouchPad itself, which needs it turned by hand.)*
- [x] Screenshots of the front page, the Personas page and an add-on page, taken in Jihad on the
  TouchPad. *(Device.)* *(Enyo, 2026-10-11, against the local copy: the front page, /personas/, an add-on page and a 404.)*
**Dependencies:** R4

### R8: Only the owner adds content
**Description:** There are no public accounts; the owner adds and changes add-ons and Personas.
**Acceptance Criteria:**
- [x] One owner account, created by a setup command (the password never in the repository); no way
  to create another from the web.
- [x] The owner can add, update, hide and delete an add-on and a Persona (the panel for add-ons, the
  importer or a panel page for Personas). *(Local copy: the panel for add-ons; `persona-admin.php` hid a Persona (the feed dropped it), showed it again, and deleted a throwaway one, leaving the targets of symlinks planted in its folder alone.)*
- [x] Every change is logged with who, what and when.
**Dependencies:** R3

### R9: A local copy that matches the server
**Description:** Everything is built and tested on this PC first, the TouchPad reaching it over the
LAN, before anything touches the owner's server.
**Acceptance Criteria:**
- [x] One command brings up the site locally (rootless containers: nginx, PHP-FPM 8.2 or 8.4,
  MariaDB), seeded with the schema, the owner account and the Personas; one command takes it down.
- [x] It serves https with a local test certificate authority that is trusted only in a test copy of
  a device profile, which is put back afterwards: every file a test can change, the persisted
  Persona pictures (`lightweighttheme-header`/`-footer`) and whole copies of `jihad-themes/` and
  `extensions/`. A backup that is not complete, or a folder copy short of what was there, restores
  nothing; a restore runs without a network route. *(Device: the CA is gone from the profile at the
  end of every test.)* *(Device, Enyo. The first restore's name-matching clean-up deleted everything in `jihad-themes/` and `extensions/` twice (BusyBox grep has no -x) — recovered, see docs/PICKUP.md. Since round 15 a restore swaps the folders back from whole copies: on a scratch profile on the device a test's added, deleted and rewritten files all came back md5-identical, an incomplete backup and a short folder copy each restored nothing, and a restore interrupted halfway finished on a second run; on Enyo itself two real cycles left its pictures, Palefill and Persona pictures md5-identical.)* *(Device,
  2026-10-12, round 17's md5-judged script under BusyBox 1.17: a scratch app's setup, test changes and
  restore left it md5-identical, a tampered backup was refused untouched, a restore cut off between
  its moves finished; real cycles on Enyo and Mojo left both profiles md5-identical.)*
- [x] The same nginx server block, FPM pool settings and schema are used locally and in the deploy. *(The local PHP runs with the pool's open_basedir and disabled functions since 2026-10-11.)*
**Dependencies:** R2

### R10: Deploy to the owner's server, when the owner says so
**Description:** A deploy for Devuan with nginx, OpenRC, PHP-FPM and MariaDB that the owner runs.
**Acceptance Criteria:**
- [x] A written checklist and scripts: packages, the database and its user, the configuration file
  outside the web root, the nginx server block, the FPM pool, OpenRC services, certbot, a nightly
  database and datastore backup, log rotation, and how to update the site.
- [x] Every step that needs root is a command the owner runs; nothing is run on the server, and no
  DNS record is made, without the owner's go-ahead.
- [~] Re-running `install.sh` keeps what the owner changed: the panel allowlist lives in its own
  include, written only when absent, and an edited site block is kept with the new one written
  beside it (`.new`). *(Code; install.sh runs as root on the server and has not been run here.)*
  (Inspector, 2026-10-11.)
- [ ] After a deploy, a smoke test (each page type, the feed, the update service, a Persona install
  from a desktop UXP browser and from Jihad) passes. [the owner's server and DNS: not yet]
- [x] Every file reaches the server over magic-wormhole-rs, and no other wormhole is installed
  (the owner's rule, 2026-10-11). (Scratch: a site package sent to this PC, unpacked and put in place.)
- [x] An update replaces only the site's code: it refuses, never deletes, anything in a copy other
  than the code and the two links into the server's data; it never puts in place a copy that differs
  from its archive or is incomplete; and the copy before it comes back with one command until the
  next update. (Inspector, 2026-10-11. Scratch runs: a real datastore folder, a cut archive, a tar
  ending between members, a changed file and a stray file in the old copy were each refused with
  nothing changed; a rollback and its return worked; the links survived every case.) Sending the
  live copy again does not replace the way back, an edit made on the server in the copy before last
  stops the update rather than vanishing, and a run cut off part-way names the command that finishes
  it. (Inspector, 2026-10-12; scratch runs.) Only the site's `jihad` branch is sent.
- [x] An update says what to run next, from what the server has applied rather than from what
  changed in the files: install.sh when it has never finished (a run that stopped part-way counts),
  when what it sets up changed, or when a link is missing; otherwise the smoke test. A
  documentation-only change asks for nothing. Only the copy in place may run install.sh. (Scratch
  runs; the cleared stamp and the in-place rule in code: install.sh runs as root on the server.)
- [x] install.sh never leaves nginx with rules it rejects: when `nginx -t` fails, the files before
  are put back and it stops without restarting nginx or claiming to be done. It stops when cron is
  not running. (Inspector, 2026-10-12. The rollback run on its own with an nginx that rejects
  everything; cron in code.)
- [x] An update applies the database changes it brings, once each, BEFORE its code goes live, after a
  dump of the database; a database made from the current schema only records them. A change only
  adds and runs twice harmlessly, so a rollback runs the copy before on the changed database (the
  update names the changes the copy going back lacks), and a run cut off part-way finishes on the
  next. Each change is recorded in the session that makes it. The local copy applies them the same
  way. (Inspector, 2026-10-12. A throwaway MariaDB 10.11: recorded on a fresh database; dumped and
  applied once on an existing one; nothing and no new dump on a re-run; a failing change not
  recorded, applied when fixed; a bad file name refused before the database is touched; through
  swap-in.sh: applied before the swap, a failing change leaving the live copy alone, a rollback
  naming the change. The local copy restarted onto its existing database: 34/34.)
- [x] A failing certificate renewal reaches the owner by some way other than Let's Encrypt mail,
  which ended in June 2025, without the owner having to run anything: a daily check on the server
  (the served certificate has three weeks left; both nightly archives, the database's and the
  datastore's, were made in the last two days) puts what it
  finds on the panel's front page, which also says when the check itself stopped, and in the login
  message. The smoke test fails three weeks ahead too. (Inspector, 2026-10-12: the smoke test alone
  reached nobody. Scratch: the check's well, unreachable, near-expiry, old-backup and first-backup
  cases; the panel's front page on the local copy for each state, markup shown as text. Cron and the
  login message on the server: code.)
**Dependencies:** R3, R5, R6, R7, R8, R9

## Requirements — the browser's half

### R11: Jihad points at its site
**Description:** The browser knows its add-on site the way Pale Moon and Basilisk know theirs.
**Acceptance Criteria:**
- [x] The prefs Pale Moon uses for its site (`extensions.getAddons.*`, `extensions.webservice.discoverURL`,
  `extensions.update.url`) name `addons.riverstonerelay.org` (https), and `jihad.personas.galleryURL`
  names the feed. `lightweightThemes.getMoreURL` stays unset: the Personas pane's "Get Personas"
  opens about:personas, which browses the site (R12).
- [x] The site holds the "install" permission by default (so its Personas may preview and its
  add-ons install with the usual confirm), and no other site gains anything.
- [x] Until the site is live, nothing breaks: about:personas says the gallery cannot be reached, and
  the add-ons manager's discover pane shows its own offline message. *(Device: "The site could not be reached" in about:personas, all three.)*
**Dependencies:** R4, R6

### R12: about:personas browses the site
**Description:** The gallery page shows the site's Personas, a page at a time, with a search.
**Acceptance Criteria:**
- [x] Below the Personas that come with Jihad, the site's Personas show as the same cards (picture,
  name, version, author, licence, Install / In use), a page at a time, with Next and Previous; the
  pager waits for each reply, and a page past the end shows the last one. *(Device, Enyo, local copy: 16 Personas at 12 a page — "Page 1 of 2" with Previous off, Next → the last four, Space Fantasy marked In use, Next off.)*
  *(Superseded 2026-10-12 by the owner's "preview only 2 personas per category + full, fast remote
  search and links to websites" — cavekit-personas-ui.md R8: two previews, a full search in place
  with More results, and a link to the site; `about:personas#site-q=WORDS` links to a search.
  Device, Enyo, live site and local copy.)*
- [~] A search field filters by name, author and description (the feed's `q`). *(Code, and the feed's `q` on the local copy. Typing into the field needs a finger on the on-screen keyboard: the owner's test.)*
- [x] "More on the web" opens the site's `/personas/` page. *(Device, Enyo.)*
- [x] Offline, slow or broken answers show a plain message; nothing from the feed is put in as markup;
  pictures must be https. *(Device: against the local copy, R9.)*
**Dependencies:** R6, R11, cavekit-personas-ui.md R8

### R13: Updates when asked
**Description:** Personas and add-ons from the site are updated only when the user asks.
**Acceptance Criteria:**
- [x] No background update check: `extensions.update.enabled` and `lightweightThemes.update.enabled`
  stay off, and nothing polls. *(Both set false; the second named since 2026-10-11.)*
- [x] about:personas has "Check for updates": every installed Persona whose update address is the
  site's is checked; those with a higher version are listed with an Update button (never an older
  or an equal one, nor an answer about another Persona); Update replaces it
  (worn stays worn) through the usual confirm. When no answer comes it says the site could not be
  reached, not that everything is up to date. *(Device, against the local copy.)* *(Device, Enyo: the test Persona's picture replaced, md5 changed.)* *(Device, Enyo, 2026-10-12: installed
  at 1.0.3, the site at 1.0.2 → "Your Personas are up to date.", at 1.0.4 → an Update card for 1.0.4.)*
- [x] about:addons' own "Check for Updates" asks the site's update service for extensions.
  *(Device, against the local copy, with a test extension at two versions.)* *(Device, Enyo: 1.0 → 1.1.)*
**Dependencies:** R5, R11

## Out of Scope
- Public developer accounts, comments, ratings, statistics.
- Mirroring addons.mozilla.org or the Pale Moon site; any NonCommercial, NoDerivatives or All Rights
  Reserved art (owner, 2026-10-11: "Originals + vetted CC").
- Extensions, search plugins and language packs as content (the features are on; filling them is
  later, each add-on tested on the device first).

## Cross-References
- cavekit-personas-ui.md R8 (about:personas), R9 (licensed bundle).
- cavekit-persona-formats.md R4 (index Personas may carry colour slots).
- cavekit-addons-extensions.md (add-on installs and their confirm).
- `docs/research/persona-archives.md`, `persona-licences.md`, `phoebus-analysis.md`.

## Changelog
- 2026-10-12 (later): Codex's review of the deploy kit (round 18): both nightly archives watched,
  php-cli installed by name, a Persona import changes files and row together or not at all, the
  smoke test takes the panel allowlist's 403, the owner account is logged, the local copy imports
  the Personas at every start, and swap-in also refuses empty directories and special files. R13's
  update check device-verified (only a higher version offered).
- 2026-10-12: R10 after the inspector's review of the round-16 deploy fixes: a rejected nginx rule is
  rolled back, the install stamp is cleared while install.sh runs, database changes go in before
  the code (add-only, dumped first, named on a rollback), a daily check reaches the owner through
  the panel and the login message, and the update refuses three more ways of losing the way back.
- 2026-10-11 (later): The inspector's proposed criteria added: R3's normalised panel path and
  framing whatever the query, R6's large page numbers, R10's re-run keeping the owner's changes.
- 2026-10-11 (later): Built; checked on the local copy and the TouchPad; the inspector's review fixed
  (the panel path P1 among them). R11 narrowed: `lightweightThemes.getMoreURL` stays unset by design.
  Open: R7's landscape on the device (turning it), R12's typed search (a finger), R10's deploy (the owner's).
- 2026-10-11: Created from the owner's answers to the Persona-index options.
