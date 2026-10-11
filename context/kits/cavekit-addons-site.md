---
created: "2026-10-11"
last_edited: "2026-10-11"
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
- [ ] The repository keeps Phoebus's history and licences (MPL-2.0 for the code; Smarty LGPL-3.0,
  SafeMySQL Apache-2.0, librdf LGPL-2.1, the Silk icons CC BY 2.5 credited) and says in its
  README what it is, where it came from and what was changed.
- [ ] The deployed tree carries no Pale Moon, Basilisk, Epyrus or Interlink skin, wordmark, font
  (Museo), discover-pane picture, shared category icon of unknown origin, or favicon; no
  application entry but Jihad's; no Moonchild mail address, link, ban list or named-user query.
- [ ] Every page says "Jihad Browser Add-ons", never "Pale Moon" or "Phoebus 2.4.2" as its brand
  (a "powered by Phoebus" credit is kept).
**Dependencies:** none

### R2: It runs on PHP 8.2 and 8.4
**Description:** Devuan 5 ships PHP 8.2 and Devuan 6 PHP 8.4; the site works on both.
**Acceptance Criteria:**
- [ ] Every page type (front page, each category, an add-on page, a licence page, search, the
  Personas page, the feed, the update service, the discover pane, the panel) answers 200 with
  no PHP notice, warning or deprecation in the response or the log, on PHP 8.2 and on 8.4.
- [ ] Smarty is a maintained release with the 2026 security fixes (4.5.8 or later).
- [ ] A notice or deprecation is logged, not turned into an error page; a real error shows a plain
  page with no file names, SQL or credentials.
**Dependencies:** R1

### R3: Safe to put on the internet
**Description:** Every hole in the analysis (P0–P2) is closed before the first deploy.
**Acceptance Criteria:**
- [ ] The database credentials and all configuration live outside the web root; nginx serves no
  dot-file and nothing under the configuration directory. *(Request for each → 404.)*
- [ ] There is no registration, verification or password-mail path in the code; there is no
  `special` component; `component=panel` is honoured only for a `/panel/` request path.
- [ ] Every panel form that changes something carries a per-user token and checks the request's
  origin; a forged cross-site POST is refused. *(Test: a POST without the token → refused.)*
- [ ] Templates escape every value by default; the few pre-rendered HTML fields are marked
  explicitly. A name with `'`, `"`, `<` or `&` shows as text everywhere, the Personas page's
  `data-browsertheme` included.
- [ ] The second-order SQL injection (`findUserAddon`) is fixed; `panel=1` downloads need the panel's
  login; `debugOff` is gone; inactive and unreviewed packages are not public by direct URL.
- [ ] The panel is behind a rate limit and, optionally, an address allowlist; responses carry HSTS,
  `X-Frame-Options`, `X-Content-Type-Options` and a Content-Security-Policy; forwarded-for headers
  are not trusted.
- [ ] Codex (or the inspector) reviews the site's code before any deploy, and no P0/P1 is open.
**Dependencies:** R2

### R4: The Jihad application
**Description:** The site serves one application: Jihad Browser.
**Acceptance Criteria:**
- [ ] `addons.riverstonerelay.org` is Jihad's site, with Jihad's add-on id
  `{4534aac8-d8c8-4765-95ee-7f61fd0b762d}` (`render/goanna/JihadUserAgent.h`, frozen), its user-agent
  token `JihadBrowser/`, and a minimum version below 1.0 so Jihad 1.0 is not "old".
- [ ] Every feature is on (extensions, themes, Personas, search plugins, language packs); the
  categories that have nothing yet say so plainly instead of erroring.
- [ ] Every URL the site writes (pages, pictures, downloads, updates, the discover pane) is https.
**Dependencies:** R2

### R5: Personas on the site
**Description:** The Personas page lists what the project may share, with credits, and each
Persona installs into Jihad, Pale Moon and Basilisk with one tap.
**Acceptance Criteria:**
- [ ] Personas have a version, a licence, a licence URL, a source URL and a credit line; ids are
  `[a-z0-9-]` only.
- [ ] An importer adds or updates Personas from Jihad's own `render/goanna/personas/personas.json` (the
  fifteen bundled: the project's art and the vetted CC set), copying the pictures with their sha256
  checked; running it twice changes nothing; a changed picture bumps the version.
- [ ] `/personas/` lists them with picture, name, author, version and licence; each button installs
  the Persona (an `InstallBrowserTheme` event on an element whose `data-browsertheme` is valid JSON).
  *(Device: a tap-equivalent in Jihad installs one through the usual confirm.)*
- [ ] The update service answers `?component=aus&persona=<id>` with the Persona including its
  version (https only), `{}` for an unknown or inactive one.
- [ ] Previews are made when a Persona is imported, never while a page is being served.
**Dependencies:** R3, R4

### R6: The feed Jihad reads
**Description:** A small JSON feed for about:personas: paged, searchable, Personas only.
**Acceptance Criteria:**
- [ ] `/?component=personas&page=N&per_page=M&q=TEXT` returns
  `{page, pages, total, personas: [...]}`; each Persona has string-valued `id, name, author,
  version, headerURL, previewURL, accentcolor, textcolor, updateURL, license, licenseURL` (and
  `footerURL`, `description`, `colors` when it has them), all URLs https.
- [ ] `q` matches name, author and description, in any language (UTF-8 kept); `per_page` is capped
  (at most 50); an out-of-range page is an empty list, not an error.
- [ ] The response is cacheable for a few minutes and has the JSON content type; HTML-significant
  characters are escaped in the JSON.
**Dependencies:** R5

### R7: A Palm-era skin of our own
**Description:** The site looks like it belongs beside the README: the webOS 3 era, Jihad's own art.
**Acceptance Criteria:**
- [ ] A new skin (`base/skin/jihad/` and the front page) drawn for the project: Palm-era greys and
  rounded cards, the project's own logo and pictures, no third-party art without a licence that
  allows it (credited).
- [ ] Every page reads well on the TouchPad (1024×768 and 768×1024, a finger's tap targets) and on a
  desktop and a phone; the error pages use the same skin.
- [ ] Screenshots of the front page, the Personas page and an add-on page, taken in Jihad on the
  TouchPad. *(Device.)*
**Dependencies:** R4

### R8: Only the owner adds content
**Description:** There are no public accounts; the owner adds and changes add-ons and Personas.
**Acceptance Criteria:**
- [ ] One owner account, created by a setup command (the password never in the repository); no way
  to create another from the web.
- [ ] The owner can add, update, hide and delete an add-on and a Persona (the panel for add-ons, the
  importer or a panel page for Personas).
- [ ] Every change is logged with who, what and when.
**Dependencies:** R3

### R9: A local copy that matches the server
**Description:** Everything is built and tested on this PC first, the TouchPad reaching it over the
LAN, before anything touches the owner's server.
**Acceptance Criteria:**
- [ ] One command brings up the site locally (rootless containers: nginx, PHP-FPM 8.2 or 8.4,
  MariaDB), seeded with the schema, the owner account and the Personas; one command takes it down.
- [ ] It serves https with a local test certificate authority that is trusted only in a test copy of
  a device profile, which is put back afterwards. *(Device: the CA is gone from the profile at the
  end of every test.)*
- [ ] The same nginx server block, FPM pool settings and schema are used locally and in the deploy.
**Dependencies:** R2

### R10: Deploy to the owner's server, when the owner says so
**Description:** A deploy for Devuan with nginx, OpenRC, PHP-FPM and MariaDB that the owner runs.
**Acceptance Criteria:**
- [ ] A written checklist and scripts: packages, the database and its user, the configuration file
  outside the web root, the nginx server block, the FPM pool, OpenRC services, certbot, a nightly
  database and datastore backup, log rotation, and how to update the site.
- [ ] Every step that needs root is a command the owner runs; nothing is run on the server, and no
  DNS record is made, without the owner's go-ahead.
- [ ] After a deploy, a smoke test (each page type, the feed, the update service, a Persona install
  from a desktop UXP browser and from Jihad) passes. [the owner's server and DNS: not yet]
**Dependencies:** R3, R5, R6, R7, R8, R9

## Requirements — the browser's half

### R11: Jihad points at its site
**Description:** The browser knows its add-on site the way Pale Moon and Basilisk know theirs.
**Acceptance Criteria:**
- [ ] The prefs Pale Moon uses for its site (`extensions.getAddons.*`, `extensions.webservice.discoverURL`,
  `extensions.update.url`, `lightweightThemes.getMoreURL`) name `addons.riverstonerelay.org` (https),
  and `jihad.personas.galleryURL` names the feed.
- [ ] The site holds the "install" permission by default (so its Personas may preview and its
  add-ons install with the usual confirm), and no other site gains anything.
- [ ] Until the site is live, nothing breaks: about:personas says the gallery cannot be reached, and
  the add-ons manager's discover pane shows its own offline message.
**Dependencies:** R4, R6

### R12: about:personas browses the site
**Description:** The gallery page shows the site's Personas, a page at a time, with a search.
**Acceptance Criteria:**
- [ ] Below the Personas that come with Jihad, the site's Personas show as the same cards (picture,
  name, version, author, licence, Install / In use), a page at a time, with Next and Previous.
- [ ] A search field filters by name, author and description (the feed's `q`).
- [ ] "More on the web" opens the site's `/personas/` page.
- [ ] Offline, slow or broken answers show a plain message; nothing from the feed is put in as markup;
  pictures must be https. *(Device: against the local copy, R9.)*
**Dependencies:** R6, R11, cavekit-personas-ui.md R8

### R13: Updates when asked
**Description:** Personas and add-ons from the site are updated only when the user asks.
**Acceptance Criteria:**
- [ ] No background update check: `extensions.update.enabled` and `lightweightThemes.update.enabled`
  stay off, and nothing polls.
- [ ] about:personas has "Check for updates": every installed Persona whose update address is the
  site's is checked; those with a new version are listed with an Update button; Update replaces it
  (worn stays worn) through the usual confirm. *(Device, against the local copy.)*
- [ ] about:addons' own "Check for Updates" asks the site's update service for extensions.
  *(Device, against the local copy, with a test extension at two versions.)*
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
- 2026-10-11: Created from the owner's answers to the Persona-index options.
