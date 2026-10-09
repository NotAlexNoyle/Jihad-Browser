# Persona repositories and Persona licences: research for Jihad Browser

Researched 2026-10-08. Read-only: no repo files were changed and no device was touched.
I sparse-cloned upstream sources over git into `scratchpad/src/`: Pale Moon `3b31d1c`
(2026-09-14), Basilisk `264872f` (2026-09-22), UXP `f6442fb` (2026-10-03), Phoebus `ed59173`
(2025-06-27) and Epyrus `03d3223`. AMO API dumps, the downloaded XPIs and contact sheets are in
`scratchpad/amo/`.

Local repo state: the bundled set in `render/goanna/personas/` was renamed while I worked. It is now
edition 4: bubble-lagoon, cloud-nine, meadow-morning, lime-fizz and aurora-silk, written by
`make-personas.py` with a `formerNames` field. The design below extends that `personas.json` shape.

---

## Part A: how other UXP applications do "Get Personas"

### A1. Pale Moon

**Preferences** (`palemoon/app/profile/palemoon.js`, https://repo.palemoon.org/MoonchildProductions/Pale-Moon/src/branch/master/palemoon/app/profile/palemoon.js):

```
pref("extensions.getAddons.showPane", true);                       // l.38: show the "Get Add-ons" pane
#define AM_DOMAIN addons.palemoon.org
pref("extensions.getAddons.cache.enabled", false);
pref("extensions.getAddons.maxResults", 10);
pref("extensions.getAddons.get.url", "https://@AM_DOMAIN@/?component=integration&type=internal&request=get&addonguid=%IDS%&os=%OS%&version=%VERSION%");
pref("extensions.getAddons.getWithPerformance.url", <same>);
pref("extensions.getAddons.search.browseURL", "https://@AM_DOMAIN@/search/?terms=%TERMS%");
pref("extensions.getAddons.search.url", "https://@AM_DOMAIN@/?component=integration&type=internal&request=search&q=%TERMS%&locale=%LOCALE%&os=%OS%&version=%VERSION%");
pref("extensions.webservice.discoverURL", "http://@AM_DOMAIN@/?component=discover");
pref("extensions.getAddons.recommended.url", "https://@AM_DOMAIN@/?component=integration&type=internal&request=recommended&locale=%LOCALE%&os=%OS%");
pref("extensions.getAddons.browseAddons", "http://@AM_DOMAIN@/");
pref("extensions.getAddons.recommended.browseURL", "https://@AM_DOMAIN@/?component=integration&type=external&request=recommended");
pref("extensions.update.url", "https://@AM_DOMAIN@/?component=aus&@AM_AUS_ARGS@");   // l.215
pref("xpinstall.whitelist.required", false); ... directRequest/fileRequest true
pref("lightweightThemes.update.enabled", true);                       // l.238
pref("lightweightThemes.animation.enabled", false);
```

- The branding sets `pref("extensions.getMoreThemesURL", "https://addons.palemoon.org/themes/")` in
  `palemoon/branding/shared/pref/preferences.inc:27` and `branding/unofficial/pref/palemoon-branding.js:8`.
  **Nothing in Pale Moon or UXP reads it.** I grepped the whole `palemoon/` tree and UXP's
  `toolkit/mozapps/extensions`. It is a leftover from the Firefox 3.x add-on manager that some old
  extensions still read.
- **Pale Moon has no "Get Personas" link.** Users reach Personas through the discover pane. The
  "Get Add-ons" category loads `http://addons.palemoon.org/?component=discover`, which Phoebus's
  `components/discover/discoverPane.php` renders as category buttons, one of them
  "Personas: Lightweight Themes" pointing to `https://addons.palemoon.org/personas/`. They can also
  browse the site directly.
- **Install trust.** `palemoon/app/permissions` grants the `install` permission to
  `http(s)://www.palemoon.org` and `http(s)://addons.palemoon.org`.
  `LightWeightThemeWebInstaller` (`palemoon/base/content/browser-addons.js:285-480`) installs
  **without asking** from a site that holds `install`. Elsewhere it shows a notification bar
  ("lwthemeInstallRequest"), then a post-install bar with Undo and Manage. Preview needs the
  `install` permission. Requests from background tabs are ignored. Jihad's `previewAllowed()`
  copies this rule, except that Jihad always asks before installing.

**addons.palemoon.org/personas/** (fetched 2026-10-08; footer: "powered by Project Phoebus 2.4.2"):

- The page lists only **three Personas, all by Moonchild**: "Basilisk Gradient", "Pale Moon" and
  "Pale Moon Classic". Each carries `"license":"copyright"` and is Pale Moon or Basilisk branding,
  so **there is nothing there Jihad can bundle**.
- The format is exactly what Jihad already supports:
  - Each row is `<a … onclick="setTheme(this); return false;" data-browsertheme='{…}'>`.
  - `setTheme` is `document.createEvent("Events"); initEvent("InstallBrowserTheme", true, false); node.dispatchEvent(e)`.
  - The JSON is the Phoebus row plus URLs: `id, name, author, description, textcolor, accentcolor,
    hasFooter, imageFormat, license, active, reviewed, finalDescription, headerURL, footerURL,
    iconURL, previewURL, updateURL`.
  - Images are under `http://addons.palemoon.org/datastore/personas/<id>/{header,footer,icon,preview}.png`.
    The header is 3000×200. Note the `.png` name even when `imageFormat` is jpg.
  - `updateURL` is `https://addons.palemoon.org/?component=aus&persona=<id>`.

**Licensing on the site:**

- The site footer reads: "Site design and Branding © 2026 Moonchild Productions - All rights
  reserved. Any other content … copyright or trademark to their respective owners".
- The user-content policy (https://www.palemoon.org/policies/usercontent.shtml) sets **no licence
  terms** for hosted add-ons. It says only that software is "AS-IS" and that the uploader is the
  contact.
- So a hosted Persona carries whatever its `license` field says. Today all three say "copyright".

**The site software: Phoebus.** Repo: https://repo.palemoon.org/MoonchildProductions/phoebus (branch `TRUNK`).

- **Licence:**
  - "All operational code that is not contained within `libraries/` is licensed under the
    *Mozilla Public License Version 2.0*".
  - Skins under `components/*/skin/` are "copyright and/or trademark of their respective owners
    (Moonchild Productions)". `skin/default/` is exempt and is MPL.
  - Vendored libraries: Smarty (LGPL-3.0), SafeMySQL (Apache-2.0), rdf_parser (LGPL-2.1).
- **Authors** (`git shortlog`): Matt A. Tobin, i.e. Binary Outcast / "New Tobin Paradigm"
  (838 commits since 2015-04-05), Lootyhoof (200), Moonchild (75), FranklinDM, Ryan C and others.
  The original "Binary Outcast" add-ons site software is this code base. Moonchild now maintains it.
- **How Personas are stored** (`modules/classPersona.php`):
  - Personas live in a MySQL table `persona`. Columns include id, slug, name, author, description,
    textcolor, accentcolor, hasFooter, imageFormat, license, active, reviewed and type.
  - `funcProcessManifest()` adds `headerURL`, `footerURL` (if hasFooter), `iconURL` (or a default)
    and `previewURL`. If no preview exists it makes a 240×60 crop of the header with GD. It also
    adds `updateURL`.
  - Only rows that are both active and reviewed are listed.
- **How they are served** (`base/addonSite.php:143-157`): `/personas/` builds the category page from
  `persona-category.xhtml`, which holds the `setTheme` script shown above.
- **Persona updates** (`components/api/addonUpdateService.php:61-74`):
  `?component=aus&persona=<id>` prints the manifest as JSON, or `{}`. That is exactly the
  `LightweightThemeManager.updateCurrentTheme()` contract: same id, a different `version` means
  replace.
- **Which applications use it** (`index.php`, `TARGET_APPLICATION_SITE`):
  - palemoon: addons.palemoon.org. Features: https, extensions, extensions-cat, themes, **personas**,
    language-packs, search-plugins.
  - basilisk: addons.basilisk-browser.org. Features: https, extensions, themes, **personas**,
    search-plugins.
  - epyrus: addons.epyrus.org. Features: https, extensions, themes, disable-xpinstall. **No Personas.**
  - App ids for ambassador and borealis are also defined.

### A2. Basilisk and other UXP applications

**Basilisk** (`basilisk/app/profile/basilisk.js`, https://repo.palemoon.org/Basilisk-Dev/Basilisk/src/branch/master/basilisk/app/profile/basilisk.js):

- It sets the same `extensions.getAddons.*`, `discoverURL` and `extensions.update.url` set, with
  `AM_DOMAIN addons.basilisk-browser.org`.
- It also sets:
  - `pref("lightweightThemes.update.enabled", true);`
  - `pref("lightweightThemes.getMoreURL", "http://@AM_DOMAIN@/personas/");`
  - `pref("lightweightThemes.recommendedThemes", "[…5 entries…]")`
- `lightweightThemes.getMoreURL` is read by `components/customizableui/CustomizeMode.jsm:1287-1290`
  (`getMoreThemes()`: `Services.urlFormatter.formatURLPref("lightweightThemes.getMoreURL")` then
  `openUILinkIn(…, "tab")`). That is the "Get More Themes" item in Customize, then Themes.
- `recommendedThemes` is Firefox's five default lightweight themes. They ship **inside Basilisk**
  under `basilisk/base/content/defaultthemes/{1..5}.{header,footer,icon,preview}.*`, with
  `headerURL: "resource:///chrome/browser/content/browser/defaultthemes/N.header.*"`. That is a
  precedent for `resource://` header URLs in Persona JSON.
- Their credits are in `basilisk/locales/en-US/chrome/browser/lightweightThemes.properties`:
  - "A Web Browser Renaissance is (C) Sean.Martell. Available under CC-BY-SA. No warranty."
  - "Space Fantasy is (C) fx5800p. Available under CC-BY-SA. No warranty."
  - "Linen Light is (C) DVemer. Available under CC-BY-SA. No warranty."
  - "Pastel Gradient is (C) darrinhenein. Available under CC-BY. No warranty."
  - "Carbon Light is (C) Jaxivo. Available under CC-BY-SA. No warranty."
- `CustomizeMode.onLWThemesMenuShowing` lists the five most recent used themes, then the
  recommended ones. Picking a recommended one moves it out of the pref.
- `basilisk/app/permissions` grants `install` to `https://addons.basilisk-browser.org`,
  `https://addons.mozilla.org` and `https://testpilot.firefox.com`.
- addons.basilisk-browser.org/personas/ runs Phoebus 2.4.2 and serves the same three Moonchild
  Personas (`"license":"copyright"`).
- I compared the Basilisk header files byte for byte with the AMO XPIs. They are identical: Space
  Fantasy `312c5974…`, Linen Light `ecb8b962…`, Pastel Gradient `a1e98051…`, Carbon Light
  `e858834e…`, Renaissance `8223abe4…`.

**Epyrus** (UXP mail client; https://repo.palemoon.org/athenian200/epyrus, `mail/app/profile/all-thunderbird.js:147-193`):

- It uses the Phoebus model with `AM_DOMAIN addons.epyrus.org` and the same `getAddons.*`,
  `discoverURL` and AUS prefs.
- It sets `lightweightThemes.update.enabled` to true.
- Its site has no Personas feature (see the Phoebus `features` list above).

**Interlink, Borealis and Ambassador (Binary Outcast):**

- `interlink-addons.binaryoutcast.com` now redirects (302) to `https://addons.thereisonlyxul.org/`.
  That site and code.binaryoutcast.com are behind an Anubis proof-of-work wall, so I did not read
  them. Their sources are not on repo.palemoon.org any more.
- Forum and search snippets say Interlink is discontinued and Borealis never shipped binaries:
  https://forum.palemoon.org/viewtopic.php?t=28021 and https://forum.palemoon.org/viewtopic.php?t=31028.
  Interlink's add-on pages were Phoebus-style
  (https://interlink-addons.binaryoutcast.com/addon/menu-on-top/).
- Ambassador (the IRC client) has only an app id in Phoebus.
- **None of these offers a curated Persona gallery.** UXP practice is one of two things:
  - a Phoebus `/personas/` category page using the standard `data-browsertheme` +
    `InstallBrowserTheme` protocol (Pale Moon, Basilisk), or
  - a small set shipped in the app (Basilisk's five Firefox defaults).

### A3. The engine (third_party/uxp): prefs and code paths

| Pref / path | Read by | Behaviour | Jihad today |
|---|---|---|---|
| `extensions.webservice.discoverURL`, `extensions.getAddons.showPane`, `xpinstall.enabled` | `toolkit/mozapps/extensions/content/extensions.js:28-29,201-215` `isDiscoverEnabled()`; `gDiscoverView` 1941-2140 | Shows the "Get Add-ons" category. The URL goes through `urlFormatter` (`%COMPATIBILITY_MODE%` …) into `<browser id="discover-browser" type="content">`. Navigation is **locked to that host**, and to https if the home URL is https; anything else is cancelled (2123-2145). An `about:` URL has no host, so it cannot be used. | Not set, so the pane is hidden. |
| `extensions.getAddons.cache.enabled`, `extensions.%ID%.getAddons.cache.enabled` | `extensions.js:1991-2010`, `AddonRepository.jsm` | When on, a list of installed add-ons is added to the discover URL fragment and the metadata cache refreshes. | `false` (`packaging/prefs/jihad-addon-prefs.js:64`). |
| `extensions.getAddons.search.url`, `.maxResults`, `.search.browseURL`, `.get.url`, `.getWithPerformance.url`, `.recommended.url`, `.recommended.browseURL`, `.browseAddons` | `AddonRepository.jsm:32-44, 700-740` | Remote search results in about:addons search, "see all results", metadata, recommended. All expect AMO-API-compatible responses (Phoebus `component=integration`). | Not set: about:addons search is local only. |
| `extensions.update.enabled`, `extensions.update.url` | `AddonManager.jsm:1259-1280` `backgroundUpdateCheck()` | Also the **only** caller of `LightweightThemeManager.updateCurrentTheme()`. | `extensions.update.enabled=false`, so no Persona updates either. |
| `lightweightThemes.update.enabled` + a theme's `updateURL` | `LightweightThemeManager.jsm:174-214` | Fetches `updateURL` with no cache. `updateURL` must be **https**: the sanitiser at l.645 drops anything else. Replaces the **current** theme only, when the id matches and `version` differs. | Pref unset, so the function returns at once. |
| `lightweightThemes.maxUsedThemes` | `LightweightThemeManager.jsm:50-58, 684` | The used-themes list is capped (default `DEFAULT_MAX_USED_THEMES_COUNT = 30`). | Default 30. A gallery must not bulk-add. |
| `lightweightThemes.getMoreURL` | Basilisk `CustomizeMode.jsm` only (app code) | Not read anywhere in UXP. | `GET_THEMES_URL` is hard-coded in `jihadPersonas.js:395`. |
| `extensions.getMoreThemesURL` | Nothing (legacy) | n/a | n/a |
| `install` permission | Apps' `LightWeightThemeWebInstaller._isAllowed`; Jihad's `previewAllowed()` (`jihadPersonas.js:332`) | Which sites may preview (and in Pale Moon, install without asking). | Jihad always confirms installs. Preview needs https + `install`. |

Other engine facts that matter for a gallery:

- **Accepted image schemes.** `_sanitizeTheme` (`LightweightThemeManager.jsm:620-664`) accepts
  `headerURL`/`footerURL` only as `http`, `https` or `resource` (plus `file` when local). Mandatory
  fields are `id, name, headerURL`. Optional fields are `footerURL, textcolor, accentcolor, iconURL,
  previewURL, author, description, homepageURL, updateURL, version`; **unknown fields such as
  `license` are dropped**.
- **No local copies of resource: images.** `_persistImage` (`:752`) returns early for `file:` and
  `resource:` URLs. Only http(s) images are copied into the profile as `lightweighttheme-header`.
  The cards cannot load `resource:` either: `jihadPersonas.js:994-1006` translates only
  `resource://jihad-themes/…` into a file path.
- **Who may load resource: URLs.** The `resource:` protocol is `URI_IS_UI_RESOURCE`
  (`netwerk/protocol/res/nsResProtocolHandler.h:28`). Chrome or system pages can load it; web
  content cannot. UXP has no content-accessible flag for `resource:` substitutions.
- **Where Jihad catches the events.** `GoannaRenderPage.cpp:1307-1325, 3756` listens with a capture
  listener on the top content document. It records the origin as the element's `documentURI`.
  `jihadPersonas.handle()` then drops requests whose origin has no host (`:345`), so **`about:`
  pages are refused today**. A page at `resource://jihad-personas/…` would pass, but the confirm
  would name the host "jihad-personas".

**How a browser points its UI at its own repository:**

1. Set the discover pane URL (`extensions.webservice.discoverURL` + `getAddons.showPane`).
2. Set the AMO-API URLs (`getAddons.*`) for search.
3. Set the AUS URL (`extensions.update.url`) and `lightweightThemes.update.enabled` for updates.
4. Grant the `install` permission to its own site (`app/permissions`).
5. Have the application's own UI read a "get more" pref (Basilisk: `lightweightThemes.getMoreURL`).

Jihad has none of the server side, so only step 5, plus its own gallery, is relevant.

### A4. Recommended design: a Jihad Persona gallery

The goal is to work like Pale Moon's `/personas/` page and Basilisk's bundled defaults, need no
server, and keep a single install path.

**1. A pref for "Get Personas".** Replace `GET_THEMES_URL` with
`Services.urlFormatter.formatURLPref("lightweightThemes.getMoreURL")`. Ship
`pref("lightweightThemes.getMoreURL", "about:personas");` in `packaging/prefs/`. This is Basilisk's
pref name and nothing in UXP reads it, so there are no side effects. The gallery's footer keeps a
"More Personas on addons.mozilla.org" link, together with the existing "Download file" tip.

**2. `about:personas`.** Build it the same way as about:preferences. A small `nsIAboutModule`
(contract `@mozilla.org/network/protocol/about;1?what=personas`), in its own component or in
jihadPersonas.js, resolves to `chrome://jihad-personas/content/gallery.html`. It gets
`ALLOW_SCRIPT` only, not `URI_SAFE_FOR_UNTRUSTED_CONTENT`, like
`jihadAboutPreferences.js:55-59`. The page:

- reads `personas/gallery.json` from the bundle (and the cached remote index, below);
- renders each entry with **textContent only**, never innerHTML, because remote strings may be
  shown;
- shows a small preview, the name, "by author", a licence badge with a link, and a **Wear** button;
- builds each Wear button exactly like Phoebus does, so the same `gallery.html` also works unchanged
  on a web host and in Pale Moon or Basilisk (for http(s) images):
  `<button data-browsertheme='{"id","name","author","headerURL","accentcolor","textcolor","version","description","homepageURL"}'>`,
  with an onclick that dispatches `InstallBrowserTheme` (`createEvent("Events")`,
  `initEvent("InstallBrowserTheme", true, false)`).

Put the attribution in `description`, in Firefox's own wording: "Space Fantasy is (C) fx5800p.
Available under CC BY-SA 3.0." Put the source page in `homepageURL`. Both fields survive
`parseTheme` and show in the add-on details.

**3. Installs go through the existing confirm.** The click reaches the existing
`GoannaRenderPage` → pump → `jihadPersonas.handle()` → `confirmTheme(host, name)` → `LWTM.currentTheme`
path. Three small changes are needed in `handle()`:

- **(a) Name the gallery.** If `req.origin` is `about:personas`, use the label
  "Jihad's Persona gallery" as the host instead of refusing the request for having no host.
  Optionally let `previewAllowed()` accept that one origin.
- **(b) Bundled pictures.** For `headerURL` under `resource://jihad-personas/` (a substitution for
  `<bundle>/personas/`, registered like `jihad-themes`):
  - copy the file into `<profile>/jihad-themes/<sha256[0:24]>-header.<ext>`;
  - set `id = "static-theme:<hash>"` and `headerURL = resource://jihad-themes/…`.

  That is exactly what `bundledPersonas()` and `offerBundledOnce()` already do. Cards, the pane,
  Uninstall cleanup (`forgetThemeFiles`) and Undo then work unchanged. A gallery Persona and the
  same picture bundled are then one Persona.
- **(c) Remote pictures.** For `https` headers from the remote index, the component should
  download the image itself into a `pending-*` file. It checks the `sha256` from the index and
  `MAX_IMAGE`, runs `shrinkWallpaper`, and only after the user says yes renames the file and wears
  it as `static-theme:<hash>`. This is the same flow as `handleStaticTheme`. The Persona keeps
  working offline and the image is integrity-checked.

  The fallback needs no code: leave the https `headerURL` to LWTM, which copies only the *current*
  theme's image locally and re-downloads it when an older one is worn again.

Linking to AMO XPIs from about:personas, which would reuse the static-theme conversion, is **not
verified**. An XPI install started from a system-principal `about:` page is untested, and
`handleStaticTheme` also labels the confirm with `hostOf(origin)`, which is empty there.

**4. Index format** (`render/goanna/personas/gallery.json`; the remote copy uses the same schema).
It extends the current `personas.json` fields (`slug, name, file, accentcolor, textcolor, author,
version, formerNames`):

```json
{
  "schema": 1,
  "collection": "Jihad Persona Gallery",
  "edition": 1,
  "updated": "2026-10-08",
  "personas": [
    {
      "slug": "space-fantasy",
      "name": "Space Fantasy",
      "author": "fx5800p",
      "authorURL": null,
      "version": "1.0",
      "formerNames": [],
      "file": "space-fantasy.jpg",
      "sha256": "312c597405d299b5…(64 hex)",
      "bytes": 173983, "width": 3000, "height": 200,
      "preview": "space-fantasy.preview.jpg",
      "accentcolor": "#d9d9d9",
      "textcolor": "#ffffff",
      "license": "CC-BY-SA-3.0",
      "licenseURL": "https://creativecommons.org/licenses/by-sa/3.0/",
      "attribution": "Space Fantasy is (C) fx5800p. Available under CC BY-SA 3.0. No warranty.",
      "source": "https://addons.mozilla.org/firefox/addon/space-fantasy/",
      "modified": null,
      "tags": ["dark", "space"]
    }
  ]
}
```

Rules for the fields:

- `slug` is the stable key across editions.
- `file` and `preview` are relative to the index's location: the bundle dir, or the index URL's
  directory for a remote index.
- `license` must be on an allow-list: CC0-1.0, CC-BY-3.0, CC-BY-4.0, CC-BY-SA-3.0, CC-BY-SA-4.0,
  MIT, MPL-2.0, Apache-2.0. No NC or ND licences, and no "All Rights Reserved".
- `modified` must say what was done when the picture is not the original, for example
  "scaled to 1600 px". CC BY 4.0 requires indicating changes, and 3.0 requires labelling adaptations.
- **Previews are required for a device:** about 1024×68, under 40 KB. A 3000×200 header decodes to
  2.4 MB, so 15 of them (about 36 MB) would exceed the 32 MB `image.mem.surfacecache.max_size_kb`
  in `make-device-bundle.sh:411`.
- Recommended header size: 3000×200 (or 2500×200), under 500 KB, jpg or png.

**5. Remote index (optional, later).**

- Prefs:
  - `pref("jihad.personas.indexURL", "")`: off by default. Later something like
    `https://raw.githubusercontent.com/<owner>/Jihad-Browser/main/render/goanna/personas/gallery.json`
    or a GitHub Pages URL `https://<owner>.github.io/jihad-personas/gallery.json`. Both send
    `Access-Control-Allow-Origin: *`, but fetching from the privileged component avoids relying on
    CORS.
  - `jihad.personas.indexIntervalDays` (7) and `jihad.personas.indexLastCheck`.
- **When to fetch:** only when about:personas opens and the interval has passed, or from a
  "Check for new Personas" button. There should be no background timer: background cards do not
  run timers, and the daemon should not wake the network.
- **Validation:**
  - https only; at most 256 KB; `schema == 1`;
  - `edition` greater than both the cached and the bundled edition;
  - every entry validated: slug `^[a-z0-9-]{1,64}$`, name at most 80 characters, colours through
    `cssColor()`, licence in the allow-list, `file`/`preview` resolved to https on the index's own
    host, `sha256` exactly 64 hex characters.
- **Cache** in `<profile>/jihad-themes/gallery-index.json`.
- **Updates:** a higher `edition` shows new entries with a "New" badge, and nothing is ever
  auto-installed. An entry whose `version` changed for an installed slug offers "Update". Wearing
  it replaces the old picture, using the replace-by-`formerNames` logic in `offerBundledOnce()`
  but keyed by slug. Entries dropped from the index leave the gallery but stay installed.
- **Not recommended: LWTM `updateURL`.** It is off in Jihad, updates only the current theme, and
  needs `extensions.update.enabled`. If wanted later, it could mirror Phoebus with
  `https://<pages>/personas/<slug>.json`.

**6. Credits.** Add a `render/goanna/personas/CREDITS.md` (and a NOTICE entry) listing each
third-party Persona with title, author, licence plus link, source and modifications. In the
Personas pane, show the licence next to the author and version that are already there. Also fix
"Author unknown" for converted static themes: AMO manifests carry no author, so take it from the
gallery index.

---

## Part B: Personas Jihad could bundle

### B5. AMO survey

Query used: `GET https://addons.mozilla.org/api/v5/addons/search/?type=statictheme&sort=users&page_size=50&app=firefox&page=1..20`.
That gives the top 1,000 of 515,147 static themes; the licence is in `current_version.license.slug`.

| Licence | Count in top 1,000 | Bundle? |
|---|---|---|
| cc-all-rights-reserved | 225 | **No** |
| CC-BY-SA-3.0 | 189 | Yes, with attribution and share-alike |
| CC-BY-3.0 | 174 | Yes, with attribution |
| CC-BY-NC-SA-3.0 | 145 | No (NonCommercial) |
| CC-BY-NC-ND-3.0 | 100 | No |
| CC-BY-NC-3.0 | 63 | No |
| CC-BY-4.0 | 53 | Yes |
| CC-BY-NC-ND-4.0 | 15 | No |
| CC-BY-ND-3.0 | 10 | Avoid (no derivatives; the wallpaper shrink is borderline) |
| CC-BY-NC-4.0 / NC-SA-4.0 | 9 / 9 | No |
| CC-BY-SA-4.0 | 8 | Yes |

Themes on AMO only ever carry a Creative Commons licence or "All Rights Reserved". NC licences are
incompatible with bundling in an Apache-2.0/MPL-2.0 browser that anyone, including commercial
redistributors, may pass on.

Many BY and BY-SA themes are colour-only, or are SVG Mozilla colorways (for example the top results
by "Firefox", CC-BY-3.0, about 9 KB XPIs). `staticThemeFromXpi` needs a png, jpg or gif
`theme_frame` or `additional_backgrounds[0]`, so those cannot be used.

**The AMO licence is self-declared.** The Blue Nebula case below shows it can be wrong. Bundle only
where the provenance is plausible.

I downloaded each XPI below and read its manifest and header (`scratchpad/amo/inspect_xpi.py`).
Every one converts with Jihad's converter.

| # | Name (AMO) | Slug | Author | Licence (exact, AMO) | Attribution | XPI | Header | Look | Provenance |
|---|---|---|---|---|---|---|---|---|---|
| 1 | Space Fantasy | `space-fantasy` (repost by yoasif; also `space-fantasy-redux` by MFGookey) | **fx5800p** (original) | Creative Commons Attribution-ShareAlike 3.0 | "Space Fantasy is (C) fx5800p. Available under CC BY-SA 3.0." + link; SA | https://addons.mozilla.org/firefox/downloads/file/3368475/space_fantasy-1.0.xpi | jpg 3000×200, 170 KB | deep-blue starfield with a mirrored blue fractal nebula (Firefox colours text #ffffff, accent #d9d9d9) | Firefox default "recommended-2", shipped in Basilisk; byte-identical. **Strongest.** |
| 2 | Linen Light | `linen-light` | DVemer | CC Attribution-ShareAlike 3.0 | credit + link; SA | https://addons.mozilla.org/firefox/downloads/file/2716448/linen_light-2.0.xpi | png 3000×200, 287 KB | warm light-grey linen weave, neutral (#ada8a8 / #000) | Firefox default "recommended-3", in Basilisk |
| 3 | Pastel Gradient | `pastel-gradient` | darrinhenein (Darrin Henein) | Creative Commons Attribution 3.0 | credit + link | https://addons.mozilla.org/firefox/downloads/file/2839567/pastel_gradient-2.0.xpi | png 3000×200, 751 KB (re-encode as jpg) | lavender to mint to lime diagonal pin-stripe gradient | Firefox default "recommended-4", in Basilisk |
| 4 | Carbon Light | `carbon-light` | Jaxivo | CC Attribution-ShareAlike 3.0 | credit + link; SA | https://addons.mozilla.org/firefox/downloads/file/2413818/carbon_light-2.0.xpi | png 3000×200, 10 KB | light-grey carbon/twill texture (#2e2e2e / #3b3b3b) | Firefox default "recommended-5", in Basilisk |
| 5 | Summerwood | `summerwood` | digitalblasphemy | CC Attribution-ShareAlike 3.0 | credit "Summerwood by Digital Blasphemy" + link; SA | https://addons.mozilla.org/firefox/downloads/file/2410748/summerwood-2.0.xpi | jpg 3000×200, 209 KB | misty blue rendered forest with fireflies, fading to black | The artist's own account ("Persona of my 'Summerwood' wallpaper") |
| 6 | Autumn Forest | `autumn-forest` | olylady | CC Attribution-ShareAlike 3.0 | credit + link; SA | https://addons.mozilla.org/firefox/downloads/file/2432212/autumn_forest-2.0.xpi | jpg 3000×200, 213 KB | photo of golden autumn woodland (#3d3828 / #fff) | "Nature trail near my home." (own photo) |
| 7 | Van Goghs Sunflowers | `van-goghs-sunflowers` | clevy.9 | CC Attribution-ShareAlike 3.0 | credit + link; SA | https://addons.mozilla.org/firefox/downloads/file/2615566/van_goghs_sunflowers-2.0.xpi | png 3000×200, 132 KB | sage-grey band with Van Gogh's Sunflowers (1888) at the right | Public-domain painting. The photo is PD in the US; Germany grants museum photos a 50-year related right (minor risk). |
| 8 | kanagawa wave | `kanagawa-wave` | d_hantusch | CC Attribution-ShareAlike 3.0 | credit + link; SA | https://addons.mozilla.org/firefox/downloads/file/2486672/kanagawa_wave-2.0.xpi | jpg 3000×200, 287 KB | Hokusai-style great wave on cream parchment, light | PD motif (Hokusai, about 1831). Alternative: `japan-style-kanagawa-gr-232767` by firefuze, CC BY-SA 3.0, 2,597 users, https://addons.mozilla.org/firefox/downloads/file/2589656/japan_style_kanagawa_gr_232767-2.0.xpi (has a text cartouche). |
| 9 | Mont Blanc vu du Bettes St Gervais | `mont-blanc-vu-du-bettes-st-ger` | Gardfield75018 | CC Attribution-ShareAlike 3.0 | credit + link; SA | https://addons.mozilla.org/firefox/downloads/file/2502203/mont_blanc_vu_du_bettes_st_ger-2.0.xpi | jpg 3000×200, 52 KB | pastel pink-lilac sunrise sky over snowy peaks, light | "Levé de soleil derrière le Mont Blanc" (looks like own photo) |
| 10 | Flowers & Dragonflies (manifest name **"Field of flowers"**) | `flowers-dragonflies` | vc | Creative Commons Attribution 4.0 | credit + link + note changes | https://addons.mozilla.org/firefox/downloads/file/4484613/flowers_dragonflies-1.0.xpi | png 3000×200, 363 KB | sky blue with meadow flowers and dragonflies (rgb(69,153,204) / white) | Not stated; 7,366 users; 2025 upload (could be generated art) |
| 11 | Blue Paisley Moon | `blue-paisley-moon` | vc | Creative Commons Attribution 4.0 | credit + link | https://addons.mozilla.org/firefox/downloads/file/4496920/blue_paisley_moon-5.0.xpi | png 3000×200, 583 KB | navy speckled band with three paisley medallions (sun, moon, flower) | Not stated |
| 12 | Dream of Waves | `dream-of-waves` | jrhd437 | CC Attribution-ShareAlike 3.0 | credit + link; SA | https://addons.mozilla.org/firefox/downloads/file/2436995/dream_of_waves-2.0.xpi | png 3000×200, 74 KB | soft aqua, lilac and cream gradient, light (dark icons) | A simple gradient (trivially original); 4,234 users |
| 13 | Abstract: frosted red | `abstract-frosted-red` | rac | Creative Commons Attribution-ShareAlike 4.0 | credit + link + changes; SA | https://addons.mozilla.org/firefox/downloads/file/4577697/abstract_frosted_red-2.0.xpi | jpg 3379×300, 317 KB | dark abstract with frosted coral-red glow and spectral streaks | "Made with multiple effect from paint.net" (own work) |
| 14 | Sunset Glimmer | `sunset-glimmer` | spica | Creative Commons Attribution 4.0 | credit + link + changes | https://addons.mozilla.org/firefox/downloads/file/4376517/sunset_glimmer-1.0.xpi | jpg **1920×588**, 240 KB (Jihad scales it to 1600 wide; note the change) | violet sky with a pink cloud and soft sparkles | "Theme design" (own) |
| 15 | Two little birds | `two-litle-birds` | math3us.c | CC Attribution-ShareAlike 3.0 | credit + link; SA | https://addons.mozilla.org/firefox/downloads/file/2527685/two_litle_birds-2.0.xpi | jpg 2500×200, 147 KB | light illustration with two small birds | Not stated; 7,510 users (already in the README) |

**Considered and rejected:**

- *A Web Browser Renaissance* (Sean Martell, CC BY-SA 3.0, Firefox default 1): its right end is a
  Da Vinci-style sketch of the **fox-and-globe Firefox logo**, a Mozilla trademark. Jihad removes
  Mozilla and MCP marks.
- *Dark Fox* and *Face Fox Dark*: Firefox logo derivatives.
- *The Long Quiet*: from the game "Slay the Princess".
- *clouds dark*: anime-style painted background, provenance unknown.
- *Light Sea*: a stock-looking photo.
- *Teal Galaxy*: a 1920×1080 wallpaper of unknown provenance.
- *Pixel Art City*: pixel art of unknown provenance.
- *seaglass*: nearly flat, based on a Slack theme.
- All sports, anime, game, film and brand themes: Kirby, Persona 5, Deltarune, TARDIS, Totoro,
  Star Wars, Zelda, Snoopy, Hello Kitty, Ferrari, BMW, FC Barcelona, Metallica, JDownloader,
  Google Chrome, Apple, Kurzgesagt, and so on.
- All colour-only themes (Catppuccin, Nord, Gruvbox, Mozilla colorways): no picture to convert.

**Obligations when bundling.**

- **CC BY 3.0 / 4.0:** credit the creator (name or pseudonym as given) and the title (3.0), give a
  link to the licence and the source URI where practicable, and indicate modifications (4.0) or
  label adaptations (3.0). Format shifting and resizing are allowed as technically necessary
  modifications.
- **CC BY-SA:** the same, plus an *adapted* picture must stay under CC BY-SA. Shipping the file
  unmodified next to Apache/MPL code is a "Collection" under 3.0 / an aggregate. It places no
  licence demand on the browser.
- Bundling is distribution, so `CREDITS.md`/NOTICE must carry the credits even if the UI also shows
  them.

### B6. Licences of what the repo has or uses now

| Asset | Licence | In the repo | In README screenshots | Bundled |
|---|---|---|---|---|
| `test/personas/water_like-2.0.xpi` + `water-like-header.jpg`: **Water-Like** by stick177 (AMO 2009; sha256 `c6b8b50d…a04` = AMO's file today) | **All Rights Reserved** (AMO `cc-all-rights-reserved`) | **Not permitted.** The AMO agreement licenses Mozilla only ("publish, distribute, and promote Listed Add-ons as part of AMO", https://extensionworkshop.com/documentation/publish/firefox-add-on-distribution-agreement/). Fix: remove both files; have `echo-server.py` fetch the XPI from AMO at test time and pin the sha256; or ask stick177; or switch the test Persona to a CC one with a similar cyan look (Dream of Waves, or Blue Sky Ultra by Oberi, CC BY 3.0). Git history keeps the copies. | Not shown | No |
| `test/personas/persona-header.png` | **Project-made**: a 3000×200 navy-to-blue gradient with a "JIHAD PERSONA TEST" box, added in `5ccc50f4` | OK (repo licence) | OK | OK |
| **Running Foxes by MaDonna** (`running-foxes-by-madonna`, v8.0) | **All Rights Reserved** ("Theme designed by MaDonna ©") | No | **Not licensed.** Shown in `personas-strip.png` and `personas-install.webp`. Replace, or get written permission. | No |
| **Two little birds** (math3us.c) | CC BY-SA 3.0 | OK with credit | OK with credit and licence link (strip) | OK with credit; SA |
| **Field of flowers** = AMO *Flowers & Dragonflies* by vc (`flowers_dragonflies-1.0.xpi`, manifest name "Field of flowers") | CC BY 4.0 | OK with credit | OK. The README says only "from addons.mozilla.org"; it must also name **vc** and CC BY 4.0. Shown in the strip, `cardview.jpg` (Mojo) and the install shot. | OK with credit |
| **Aurora Borealis Delights** (Scarletta) | **All Rights Reserved**; also "Based on …/rainbow-blur-evolution/" | No | **Not licensed.** In `cardview.jpg` (Mochi) and `personas-install.webp`. Replace. | No |
| **Blue Nebula Space Theme** (eartharoid; manifest "Blue Nebula") | AMO says CC BY 3.0, **but** its description credits the artwork to Josef Bartoň (JoeyJazz, "Darklights"), whose DeviantArt page says **"For your personal desktop use ONLY!"** (https://www.deviantart.com/joeyjazz/art/Darklights-446622846). The uploader could not grant CC BY. | No | **Avoid.** Its row is in `personas-install.webp`. | No |
| **Dark Fox** (randomaster) | CC BY-SA 3.0, but "My dark version of the **Firefox logo**" | Copyright OK with credit; trademark problem | Avoid. Its row is in `personas-install.webp`. | No (trademark) |
| **Purple Night Theme** (breuxi / GitHub Br3uxi/Purple-Night-Theme, whose `LICENSE.md` is CC BY-NC-SA 3.0), based on Chu's "Purple Twinkle" (also CC BY-NC-SA 3.0) | **CC BY-NC-SA 3.0** | No: NonCommercial does not fit the browser's licences | A non-commercial README use would be allowed with credit and SA. It appears only in kit and device-test notes, not in the screenshots. | No |

**Recommended next steps for the screenshots.**

- Retake `cardview.jpg`, `personas-strip.png` and `personas-install.webp` using the bundled set plus
  CC Personas (for example Space Fantasy, Summerwood, Field of flowers, Two little birds).
- Drop Running Foxes, Aurora, Blue Nebula and Dark Fox from them.
- Add a credit line under the screenshots, for example: "Field of flowers by vc (CC BY 4.0); Two
  little birds by math3us.c (CC BY-SA 3.0), from addons.mozilla.org".
- Note the "Author unknown" rows in the pane today: static-theme manifests carry no author, so
  attribution has to come from Jihad's own index.

## Sources

- Pale Moon prefs: https://repo.palemoon.org/MoonchildProductions/Pale-Moon/src/branch/master/palemoon/app/profile/palemoon.js
- Pale Moon branding pref: https://repo.palemoon.org/MoonchildProductions/Pale-Moon/src/branch/master/palemoon/branding/shared/pref/preferences.inc
- Pale Moon permissions and theme web installer: https://repo.palemoon.org/MoonchildProductions/Pale-Moon/src/branch/master/palemoon/app/permissions and https://repo.palemoon.org/MoonchildProductions/Pale-Moon/src/branch/master/palemoon/base/content/browser-addons.js
- Persona pages: https://addons.palemoon.org/personas/ and https://addons.basilisk-browser.org/personas/
- Phoebus: https://repo.palemoon.org/MoonchildProductions/phoebus (README, LICENSE, `modules/classPersona.php`, `components/api/addonUpdateService.php`, `base/addonSite.php`, `components/discover/discoverPane.php`, `index.php`)
- Basilisk: https://repo.palemoon.org/Basilisk-Dev/Basilisk/src/branch/master/basilisk/app/profile/basilisk.js, `basilisk/components/customizableui/CustomizeMode.jsm`, `basilisk/locales/en-US/chrome/browser/lightweightThemes.properties`, `basilisk/base/content/defaultthemes/`, `basilisk/app/permissions`, `LICENSE.md`
- Epyrus: https://repo.palemoon.org/athenian200/epyrus (`mail/app/profile/all-thunderbird.js`)
- Pale Moon user content policy: https://www.palemoon.org/policies/usercontent.shtml
- AMO API: https://addons.mozilla.org/api/v5/addons/search/?type=statictheme&sort=users&page_size=50&app=firefox and https://addons.mozilla.org/api/v5/addons/addon/<slug>/
- AMO distribution agreement: https://extensionworkshop.com/documentation/publish/firefox-add-on-distribution-agreement/
- Blue Nebula artwork terms: https://www.deviantart.com/joeyjazz/art/Darklights-446622846
- Purple Night repo: https://github.com/Br3uxi/Purple-Night-Theme
- Interlink and Borealis status: https://forum.palemoon.org/viewtopic.php?t=28021, https://forum.palemoon.org/viewtopic.php?t=31028, and https://interlink-addons.binaryoutcast.com/ (now a 302 to https://addons.thereisonlyxul.org/)
- Local: `third_party/uxp/toolkit/mozapps/extensions/{content/extensions.js, LightweightThemeManager.jsm, AddonManager.jsm, internal/AddonRepository.jsm}`, `render/goanna/components/jihadPersonas.js`, `render/goanna/GoannaRenderPage.cpp`, `packaging/prefs/jihad-addon-prefs.js`, `build/webos-oe/make-device-bundle.sh`
