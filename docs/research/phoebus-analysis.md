# Phoebus 2.4.2 — engineering analysis for addons.riverstonerelay.org

Subject: the fork at `../Riverstone-Addons` (beside this repository), branch TRUNK, commit `ed59173`
(2025-06-27). Submodules: Smarty `v3.1.33` (`dd55b23`), SafeMySQL `v1.1.0-2-ge64dacc`, librdf `76b2b9f`
("Update for PHP 7.4 compatibility").

Method:
- Every non-library file was read in full: `index.php`, `fundamentals.php`, `base/`, `modules/`, `components/`,
  `databases/`, all `.xhtml` templates, and the two vendored libraries Phoebus calls directly (SafeMySQL, librdf).
- Smarty was checked by running it.
- A throwaway harness (`harness/run.php`, `harness/boot.php`, not kept) ran the real code under **PHP 8.4.22
  CLI**, with Phoebus's own error handling replaced by a recorder. The local PHP has no mysqli, gd or zip, so
  only the non-database paths ran.
- Production behaviour was sampled read-only from addons.palemoon.org: `/personas/`, `?component=aus&persona=`,
  `?component=integration`, slashless redirects and image sizes. No security endpoints were probed.
- Smarty advisories, CHANGELOG and release data were fetched from GitHub on 2026-10-11.

Tags:
- **[tested-8.4]** reproduced on PHP 8.4.22.
- **[live]** observed on addons.palemoon.org.
- **[inferred]** deduced, not proven.

Untagged claims come from reading the code. All paths are relative to `Riverstone-Addons/` unless prefixed
with `Jihad-Browser/`.

---

## 0. Executive summary

1. **Phoebus as shipped cannot serve a single page on PHP 8.2 or 8.4.**
   - `index.php:9` sets `error_reporting(E_ALL)`. `fundamentals.php:242-256` installs `gfErrorHandler`, which
     turns every notice, warning and deprecation into a fatal "PHP Error" page (`gfError` → `exit`,
     `fundamentals.php:232`).
   - Every Smarty render, every SQL query (SafeMySQL `preg_split(..., null)`) and every `install.rdf` parse
     (librdf `xml_set_object`) raises a deprecation on 8.2/8.4 **[tested-8.4]**.
   - On 8.4, `E_STRICT` (`fundamentals.php:39`) prints a deprecation *before* the handler exists. That
     corrupts every XML/JSON response and causes "headers already sent" **[tested-8.4]**.
2. **Smarty 3.1.33 must go.** Upgrade to **Smarty 4.5.8**. It is a drop-in for Phoebus's API, supports PHP 8.4
   since 4.5.5, and carries the 2026 security fixes. Smarty 5.8.5 is the longer-term option but needs code
   changes (§8).
3. **There is no URL rewriting in PHP.** The web server must map `/extensions/` etc. to
   `index.php?component=site&path=<uri>`.
   - `/panel/…` and `/special/…` are picked by `REQUEST_URI` prefix only (`index.php:483-494`).
   - Downloads use nginx `X-Accel-Redirect` (`base/addonDownload.php:52`). Production runs nginx and
     302-redirects slashless paths **[live]**.
4. **The app is chosen by `SERVER_NAME`** against `TARGET_APPLICATION_SITE[*].domain` (`index.php:73-96,
   411-427`). Its add-on ID comes from `TARGET_APPLICATION_ID` (`index.php:110-118`).
   - Pale Moon: `{8de7fcbb-c55c-4fbe-bfc5-fc555c87dbc4}`.
   - Basilisk: `{ec8030f7-c20a-464f-9b0e-13a3a9e97384}`, which is Firefox's ID.
   - Jihad must use `{4534aac8-d8c8-4765-95ee-7f61fd0b762d}` (`Jihad-Browser/render/goanna/JihadUserAgent.h:52`).
5. **The app key `jihad` is also a database column, a skin directory, a front-page file name and a
   user-agent token.**
   - Jihad's UA token is `JihadBrowser/1.0` (`JihadUserAgent.h:67-70`).
   - Phoebus looks for `jihad/` in the UA (`index.php:276`), so its version check never matches unless it is
     patched.
6. **The database credentials live inside the web root**, at `datastore/.phoebus/sql` (`modules/classDatabase.php:16`).
   The datastore must be publicly served for images, so without a deny rule nginx serves this file as plain
   text. **P0.**
7. **Registration is open by default.**
   - It stores unvalidated `displayName`/`email`, which the admin "Users" list renders raw.
   - Result: anonymous stored XSS → takeover of the admin, whose Basic-auth credentials are replayed by the
     browser. **P0 unless registration is removed in code.**
   - An nginx block on `/panel/registration/` is not enough. `/?component=panel&path=/panel/registration/`
     reaches the same code (`index.php:499-500`).
8. **The panel has no CSRF protection anywhere.** Its auth is HTTP Basic (no sessions or cookies), so every
   panel POST can be forged cross-site (delete add-on, change password or level). **P1.**
9. **`/special/phpinfo/`, `/special/validator/` and `/special/test/` need no authentication**
   (`base/specialComponent.php:38-100`). phpinfo was reproduced. They are also reachable via
   `?component=special`. Remove `special` from `COMPONENTS`. **P1.**
10. **Personas have no creation path at all.** No panel UI and no INSERT exist. Upstream rows were inserted by
    hand **[live]**.
    - The real `persona` columns are `id` (string), name, author, description, textcolor, accentcolor,
      hasFooter, imageFormat, license, active, reviewed.
    - There is **no `version`**, so `LightweightThemeManager` never applies an update
      (`LightweightThemeManager.jsm:203-206, 676-677`).
11. **The persona listing template is broken for real-world data.**
    - Raw JSON goes into a single-quoted attribute with no apostrophe escaping (`persona-category.xhtml:17`).
      `Owner's Moon` breaks it **[tested-8.4]**.
    - URLs are built as `http://` (`classPersona.php:148`). The template strips exactly 5 characters with
      `|substr:5` (`:18-19`), which breaks the moment the code switches to https.
    - Descriptions are byte-truncated (`classPersona.php:144`), which can split UTF-8 so that `json_encode`
      fails.
12. **Smarty auto-escaping is off** (`escape_html=false`, `Smarty.class.php:562`, never enabled in
    `classGenerateContent.php:59-80`). Every `{$var}` is raw HTML.
13. **SQL is parametrised (SafeMySQL) everywhere except one place**: `classAccount.php:387` interpolates a slug
    into `LIKE` (second-order SQLi via a crafted langpack id).
    - `?component=download&panel=1` serves inactive or unreviewed XPIs to anyone
      (`base/addonDownload.php:104-106`).
    - `?debugOff=1` on the live host switches to the dev database (`classDatabase.php:24`).
14. **Moonchild-specific code must be stripped or generalised:**
    - Pale Moon-only language packs (`classWriteManifest.php:50-55, 110-115, 186-188`).
    - Mail from `addons@palemoon.org` (`classAccount.php:487-505`).
    - A `/dictionaries/` redirect to archive.palemoon.org (`addonSite.php:176`).
    - A "JustOff" exclusion query that also crashes on PHP 8 (`classReadManifest.php:229-244`).
    - Personal ban lists (`classAccount.php:41-43`, `classWriteManifest.php:743`).
    - Profane error strings.
15. **Licence split for the new skin.**
    - MPL and reusable: `base/skin/default/`, `components/*/skin/default/`, all PHP.
    - Reusable with credit: Silk icons, CC BY 2.5 (`components/panel/skin/shared/`).
    - **Do not reuse**: `base/skin/{palemoon,basilisk,epyrus}` (Museo fonts, wordmarks),
      `components/discover/skin/*.png`, `base/skin/shared/*.png` (provenance unknown), root `favicon.ico`.
    - A new skin `base/skin/jihad/` needs 9 files plus `base/content/jihad-frontpage.xhtml` (§9).

---

## 1. Request routing

### 1.1 Boot sequence (`index.php`)

| Step | Lines | What happens |
|---|---|---|
| Error mode | 9-11 | `error_reporting(E_ALL)`, `display_errors=on`, `html_errors=off`. |
| Root | 17 | `ROOT_PATH = $_SERVER['DOCUMENT_ROOT']` (no trailing slash). All absolute paths derive from it, so nginx must pass the Phoebus checkout as `DOCUMENT_ROOT`. |
| Debug const | 20 | `DEBUG_MODE = $_GET['debug']`. Only a fallback inside `gfHeader` (`fundamentals.php:342-349`); effectively unused. |
| Path constants | 23-32 | `DATASTORE_RELPATH='/datastore/'`, `OBJ_RELPATH='/.obj/'`, `SPECIAL_SKIN_PATH=ROOT/base/skin/default/`. |
| Components | 35-43 | `aus`, `discover`, `download`, `integration`, `panel`, `site`, `special` → PHP file. |
| Modules / DBs / libs | 46-69 | Name → file maps used by `gfImportModules` (`fundamentals.php:484-511`). |
| Apps | 73-96 | `TARGET_APPLICATION_SITE` (see 1.4). |
| App IDs | 110-118 | `TARGET_APPLICATION_ID` (see 1.4). |
| Categories | 120-139 | `EXTENSION_CATEGORY_SLUGS`, `OTHER_CATEGORY_SLUGS`. |
| Fundamentals | 142, 145 | `require_once('./fundamentals.php')` (relative to CWD), then the version comparator. |
| Runtime state | 338-359 | `$gaRuntime`: `qComponent=?component`, `qPath=?path`, `qApplication=?appOverride`, `qDebugOff=?debugOff`, `qSearchTerms=?terms`. `remoteAddr` = `X-Forwarded-For` ?? `REMOTE_ADDR` (351-352). `currentScheme` = `$_SERVER['SCHEME']` ?? (`HTTPS` ? https : http) (343). |
| Offline mode | 364-405 | If `ROOT/.offline` exists and there is no `overrideOffline` cookie (any value), it serves empty AUS/API answers and an error page otherwise. |
| App selection | 411-427 | `switch ($_SERVER['SERVER_NAME'])` against each app's `domain.live` / `domain.dev`. A dev match sets `debugMode`. |
| Debug extras | 432-454 | Only on a dev domain: `?debugOff` and `?appOverride=<app>`. |
| Sanity | 459-472 | No domain → "Invalid domain". App disabled → error. |
| Client check | 477-478, 239-331 | `gfValidClientVersion()`: details in 1.4(c). |
| Component pick | 483-494 | No `component` and no `path` → `site` + `/`. **Else**, if `REQUEST_URI` starts with `/panel/` → `panel`; `/special/` → `special`. These prefix rules only run when `component` or `path` is already set. |
| Dispatch | 499-507 | `require_once(COMPONENTS[qComponent])`. Unknown → 404. |

All `$_GET` values pass through `gfSuperVar('get')`. That strips every character outside
`[-a-zA-Z0-9_/{}@.%\s,]` (`fundamentals.php:102, 307-311`). The regex has no `/u` flag, so **all non-ASCII is
removed**: a search for "Fünf" becomes "Fnf". `$_POST` values are not filtered at all (`fundamentals.php:300`).

### 1.2 Site URL map (`base/addonSite.php`)

URI constants are at lines 9-19; dispatch is at 69-296. `gfStripPath()` (38-40) removes every `/` to get a slug.

| URL | Lines | Feature gate | Data → template (`PAGE_TYPE`) |
|---|---|---|---|
| `/` | 70-81 | — | `base/content/<app>-frontpage.xhtml` (content file, not skin). `interlink` redirects to `/extensions/` (72-74). |
| `/search/?terms=` | 82-98 | none | `getAddons('site-search')` → `addon-category.xhtml` (`search`). `*` is removed (85). |
| `/extensions/` | 99-126 | `extensions-cat` (optional) | With `extensions-cat` and no `?all` → `extension-category.xhtml` (`cat-extension-category`). Otherwise all extensions → `addon-category.xhtml` (`cat-all-extensions`). **The `extensions` feature flag is never checked.** |
| `/extensions/<cat>/` | 203-227 | `extensions-cat` | `cat-extensions`. An unknown slug redirects to `/addon/<slug>` (211-213). An empty category → 404 (219-221). |
| `/themes/` | 127-142 | `themes` | `cat-themes`. |
| `/personas/` | 143-158 | `personas` | `classPersona::getPersonas('site-all-personas')` → `persona-category.xhtml` (`cat-personas`). |
| `/language-packs/` | 159-174 | `language-packs` | `langpack-category.xhtml`. |
| `/dictionaries/` | 175-177 | none | 302 to `https://archive.palemoon.org/palemoon/dictionaries/`. |
| `/search-plugins/` | 178-193 | `search-plugins` | `getSearchPlugins()` → `searchplugin-category.xhtml`. |
| `/addon/`, `/releases/`, `/license/` (bare) | 194-199 | — | 302 to `/`. |
| `/addon/<slug>/` | 229-243 | — | `getAddon('by-slug')` → `addon-page.xhtml` (`addon-page`). |
| `/releases/<slug>/` | 245-259 | — | `addon-page.xhtml` (`addon-releases`). |
| `/license/<slug>/` | 261-292 | — | Redirect to `licenseURL` if set (274-276). `pd`/`copyright`/`custom` → `addon-license.xhtml`. Otherwise 302 to `https://opensource.org/licenses/<license>` (291). |
| anything else | 295 | — | 404. A slashless path like `/personas` is a 404 in PHP. Production nginx 302s it to `/personas/` **[live]**. |

### 1.3 Query-string components

| Request | File | Parameters |
|---|---|---|
| `/?component=aus&…` | `components/api/addonUpdateService.php` | `persona`, `id`, `version`, `appID`, `appVersion`, `compatMode`, header `Moz-XPI-Update` or `?updateOverride` (49-55). §4.6 and §5.2. |
| `/?component=download&id=&version=&panel=` | `base/addonDownload.php` | 92-122. §5.1. |
| `/?component=integration&type=&request=&q=&addonguid=&version=` | `components/api/amIntegration.php` | 12-16. §5.5. |
| `/?component=discover` | `components/discover/discoverPane.php` | none. §5.4. |
| `/panel/…?task=&what=&slug=` | `components/panel/phoebusPanel.php` | 26-28. §6. |
| `/special/…` | `base/specialComponent.php` | `case`. §7. |

### 1.4 Defining the application "jihad"

**(a) Existing app IDs** (`index.php:110-118`; the comment at 109 says "Make sure these exist in the 'client' DB table"):

| Key | ID |
|---|---|
| toolkit | `toolkit@mozilla.org` |
| palemoon | `{8de7fcbb-c55c-4fbe-bfc5-fc555c87dbc4}` |
| basilisk | `{ec8030f7-c20a-464f-9b0e-13a3a9e97384}` (Firefox's ID) |
| ambassador | `{4523665a-317f-4a66-9376-3763d1ad1978}` |
| borealis | `{a3210b97-8e8a-4737-9aa0-aa0e607640b9}` |
| epyrus | `{29877c1d-27df-4421-9a79-382c31470151}` |
| thunderbird | `{3550f703-e582-4d05-9a08-453d09bdfdc6}` |

Phoebus expects an **em:id-style GUID** that matches the client's `nsIXULAppInfo.ID`, because:
- AUS compares it with `?appID=` (`addonUpdateService.php:112`);
- it is matched against `install.rdf` `<em:targetApplication><em:id>` (`classWriteManifest.php:956-967`);
- every XPI release entry lacking it is dropped from listings (`classReadManifest.php:384-387`).

For Jihad that is `{4534aac8-d8c8-4765-95ee-7f61fd0b762d}`, app version `1.0`
(`Jihad-Browser/render/goanna/JihadUserAgent.h:52, 55`).

**(b) Existing site entries** (`index.php:73-96`):

| Key | oldVersion | Domains (live / dev) | Features |
|---|---|---|---|
| palemoon | `24.0.0` | `addons.palemoon.org` / `addons-dev.palemoon.org` | https, extensions, extensions-cat, themes, personas, language-packs, search-plugins |
| basilisk | `20100101` | `addons.basilisk-browser.org` / null | https, extensions, themes, personas, search-plugins |
| epyrus | `1.9` | `addons.epyrus.org` / null | https, extensions, themes, disable-xpinstall |

Feature flags the code actually reads:
- `https`: panel allowed (`phoebusPanel.php:68`) and https download links (`classReadManifest.php:428`).
- `extensions-cat`: `addonSite.php:103, 205`; `discoverPane.php:54`.
- `themes`, `personas`, `language-packs`, `search-plugins`: the gates in 1.2.
- `disable-xpinstall`: binary downloads, and the client UA check is skipped (`index.php:250`,
  `addonDownload.php:95`).

**(c) The client check** (`index.php:239-331`):
- No UA → "Invalid client identification" error for **every** request (245-247).
- A UA containing `curl/`, `wget/` or `git/` → "Blocked for resource abuse" (263-267). Health checks and manual
  tests need `-A`.
- Version check: finds `<appkey>/` in the lower-cased UA (276) and compares the version after it with
  `oldVersion` (305, 325). `compare() <= 0` means "old".
- Jihad's UA ends `… Firefox/52.9 ECMAScript/2024 JihadBrowser/1.0` (`JihadUserAgent.h:67-70`). `jihad/` is not
  a substring of `jihadbrowser/1.0`, so `VALID_VERSION` would always be false.
- No shipped template uses `VALID_CLIENT`/`VALID_VERSION` (grep), so this only matters if the Jihad skin does.

**(d) Everything keyed by the app key, all of which "jihad" needs:**
1. `TARGET_APPLICATION_SITE['jihad']` (`index.php:73-96`). Suggested:
   ```php
   'jihad' => array(
     'enabled'    => true,
     'oldVersion' => '0.9',            // must sort BELOW JIHAD_APP_VERSION "1.0"; equal counts as "old"
     'uaToken'    => 'jihadbrowser',   // NEW key; patch index.php:276 and :292 to use it instead of the app key
     'name'       => 'Jihad Browser Add-ons',
     'domain'     => array('live' => 'addons.riverstonerelay.org', 'dev' => null),
     'features'   => array('https', 'extensions', 'extensions-cat', 'themes',
                           'personas', 'language-packs', 'search-plugins'),
   ),
   ```
   Remove the palemoon, basilisk and epyrus entries; their skins must not ship. Note that `'dev' => null` plus
   the loose `switch` at `index.php:411-421` means an **empty** `SERVER_NAME` matches `null`, turning on debug
   mode (`index.php:417-420`). nginx must always send `SERVER_NAME`; better, change it to a strict comparison.
2. `TARGET_APPLICATION_ID['jihad'] = '{4534aac8-d8c8-4765-95ee-7f61fd0b762d}'` (`index.php:110-118`). Keep
   `toolkit` only if a `client.toolkit` column exists: the validator inserts one column per key
   (`classWriteManifest.php:956-967, 131-135`).
3. Column `client.jihad` TINYINT. Every public query filters `WHERE ?n = 1` with the app key
   (`classReadManifest.php:77, 89, 155, 165, 175, 187, 199`).
4. Skin directory `base/skin/jihad/` (`classGenerateContent.php:41-45`).
5. Front page `base/content/jihad-frontpage.xhtml` (`addonSite.php:78-80`).
6. Writable Smarty dirs `.obj/smarty/site-jihad/{cache,compile,config,plugins,template}` (`classGenerateContent.php:72-80`).
7. Discover pane: `components/discover/skin/jihad.png` (`discoverPane.php:46`), plus a `case 'jihad'` for the
   display name and app type (`discoverPane.php:12-36`). The default would render "Jihad" / "application".
   Category buttons are hard-wired to `palemoon` (`discoverPane.php:54-55`).
8. External-add-on app checkboxes: the hard-coded list in `classWriteManifest.php:378-385` and
   `components/panel/skin/default/external-metadata.xhtml:92-98`.
9. Language packs: the Pale Moon assumptions in §5.3.
10. The new persona feed component: an entry in `COMPONENTS` (`index.php:35-43`), §4.7.

**(e) Client-side prefs for Jihad.** These are not Phoebus, but they are needed for it to be used.

Jihad's `packaging/prefs/jihad-addon-prefs.js:51, 61-62, 64` currently has `extensions.update.enabled=false`,
`data:` placeholder update URLs and `getAddons.cache.enabled=false`. "Updates on demand only" means keeping
`extensions.update.enabled=false` and setting:
```
extensions.update.url = https://addons.riverstonerelay.org/?component=aus&reqVersion=%REQ_VERSION%&id=%ITEM_ID%&version=%ITEM_VERSION%&maxAppVersion=%ITEM_MAXAPPVERSION%&status=%ITEM_STATUS%&appID=%APP_ID%&appVersion=%APP_VERSION%&appOS=%APP_OS%&appABI=%APP_ABI%&locale=%APP_LOCALE%&currentAppVersion=%CURRENT_APP_VERSION%&updateType=%UPDATE_TYPE%&compatMode=%COMPATIBILITY_MODE%
extensions.getAddons.search.url = https://addons.riverstonerelay.org/?component=integration&type=internal&request=search&q=%TERMS%&locale=%LOCALE%&os=%OS%&version=%VERSION%
extensions.getAddons.get.url    = https://addons.riverstonerelay.org/?component=integration&type=internal&request=get&addonguid=%IDS%&os=%OS%&version=%VERSION%
extensions.getAddons.search.browseURL = https://addons.riverstonerelay.org/search/?terms=%TERMS%
extensions.webservice.discoverURL     = https://addons.riverstonerelay.org/?component=discover   (+ extensions.getAddons.showPane=true)
```
These follow the Pale Moon pattern recorded in `Jihad-Browser/docs/research/persona-licences.md:22-33`.

The persona install event also needs the `install` permission for the host (`persona-licences.md:181, 205-209`).

Persona updates run only inside `AddonManager.backgroundUpdateCheck()` when `extensions.update.enabled` is true
(`third_party/uxp/toolkit/mozapps/extensions/AddonManager.jsm:1259-1272`). On-demand therefore means Jihad's UI
must call `LightweightThemeManager.updateCurrentTheme()` itself, with `lightweightThemes.update.enabled=true`,
or fetch the feed itself.

---

## 2. Configuration inventory

**Secrets.** The only secret in the system is the database credentials. Phoebus has no CSRF or session
secrets, no API keys and no SMTP credentials. `mail()` goes to the local MTA.

| # | Item | Where | Current value / behaviour | Action |
|---|---|---|---|---|
| 1 | DB credentials | `modules/classDatabase.php:16-28` | `@include_once(ROOT/datastore/.phoebus/sql)`: a PHP file that must set `$arrayCreds = ['username'=>…, 'password'=>…, 'liveDB'=>…, 'devDB'=>…]`. **Inside the web root.** | Move to `/etc/riverstone-addons/config.php` (0640 root:www-data). |
| 2 | DB host / charset | `classDatabase.php:28, 34` | `mysqli_connect('localhost', …)`: no port or socket option. `mysqli_set_charset(…,'utf8')` = utf8mb3. | Config: host/socket. Use `utf8mb4`. |
| 3 | Dev DB switch | `classDatabase.php:22-26` | Uses `devDB` if `debugMode` **or `?debugOff` is present on any host**. | Remove the `qDebugOff` clause; single DB. |
| 4 | Error display | `index.php:9-11` | `E_ALL`, `display_errors=on`. | Config flag. Production: log only (and `php_admin_flag[display_errors]=off` in the FPM pool, which `ini_set` cannot override). |
| 5 | Error handler | `fundamentals.php:242-256` | Every reported error → fatal page that echoes message, file and line. | Make it log and continue for `E_DEPRECATED`, `E_USER_DEPRECATED`, `E_NOTICE`; show a generic message in production. |
| 6 | Debug const | `index.php:20` | `?debug`. | Drop. |
| 7 | Software name/version | `index.php:23-24` | `Phoebus` `2.4.2`. Shown in footers, the Basic-auth realm (`fundamentals.php:712`) and `X-Mailer` (`classAccount.php:492`). | Keep (MPL attribution) or brand it. |
| 8 | Paths | `index.php:25-32` | `/datastore/`, `/.obj/`, `/base/skin/default/`. | Config, or symlink `ROOT/datastore` and `ROOT/.obj` to `/srv/riverstone-addons/{datastore,obj}`. Upstream does this: see the realpath comment at `classWriteManifest.php:439-442`. |
| 9 | Apps, domains, features | `index.php:73-96` | Moonchild domains. | Config (§1.4). |
| 10 | App IDs | `index.php:110-118` | — | Config (§1.4). |
| 11 | Category slugs | `index.php:120-139` | Fixed list. | Config if desired. |
| 12 | Offline switch | `index.php:364-365` | File `ROOT/.offline`. Bypass cookie `overrideOffline` (**any value**). | Move the flag outside the web root; require a secret cookie value. |
| 13 | Panel switch | `phoebusPanel.php:58-60` | File `ROOT/.disablePanel`. Bypass cookie `overrideDisablePanel` (any value). | Same. |
| 14 | Legacy panel redirect | `phoebusPanel.php:68-70` | `https://addons-legacy.palemoon.org/panel/`. | Remove. |
| 15 | Smarty dirs, debug | `classGenerateContent.php:59-80` | `caching=0`. `?smartyDebug` works on dev domains only. Dirs under `ROOT/.obj/smarty/<component>-<skin>/`. | Config path outside the root. |
| 16 | Mail | `classAccount.php:487-505` | `sendmail_from addons@palemoon.org`, `From`/`Reply-To` addons@palemoon.org, envelope `-f no-reply@palemoon.org`, link `https://addons(-dev).palemoon.org/panel/verification/`. | Remove (registration disabled) or make configurable. |
| 17 | Logout URL | `classAccount.php:420` | `https://logout:logout@<domain>/panel/logout/`. | OK. |
| 18 | Banned users | `classAccount.php:41-43, 95-101` | `srazzano`. | Delete. |
| 19 | Email blacklist | `databases/emailBlacklist.php` | 9,269 `fnmatch` patterns. | Unused once registration is gone. |
| 20 | XPI types / limits | `classWriteManifest.php:710, 716-721` | zip / x-xpinstall / java-archive; types 2 and 4 (8 for langpacks). | Config. |
| 21 | Restricted IDs | `classWriteManifest.php:731-744` | Moonchild GUID fragments, palemoon.org, basilisk-browser.org, mozilla.org, `srazzano.com`… | Replace with a Jihad list (e.g. `riverstonerelay.org`). |
| 22 | Image limits | `classWriteManifest.php:1118-1121` | 524,288 bytes, PNG only. | Config. |
| 23 | Bulk upload dir | `classWriteManifest.php:514` | `ROOT/.obj/bulk-upload/<type>-<time>/`, never cleaned. | Config plus cleanup. |
| 24 | Langpack rules | `classWriteManifest.php:50-55, 110-115, 186-188` | Pale Moon only; slug `pm-`; creator "Moonchild Productions". | Generalise. |
| 25 | Licences | `classReadManifest.php:20-36` | SPDX-ish list. | OK. |
| 26 | JustOff exclusion | `classReadManifest.php:229-244` | Queries user `justoff`. | Delete (it also crashes on PHP 8, §8). |
| 27 | Persona URLs | `classPersona.php:148-169` | `http://` + domain; default icon `datastore/addons/default/theme.png`; preview 240×60; truncation at 235/230 bytes. | Use https; config. |
| 28 | API XML URLs | `classGenerateContent.php:372-373` | `http://` + domain. | Use https. |
| 29 | Bad AUS IDs | `addonUpdateService.php:38-43` | Mozilla default theme, Modern, GData, Lightning. | Keep or trim. |
| 30 | Dictionaries | `addonSite.php:175-177` | archive.palemoon.org. | Remove or repoint. |
| 31 | OSI redirect | `addonSite.php:291` | opensource.org. | OK. |
| 32 | Discover | `discoverPane.php:12-36, 45, 57, 84, 98` | Per-app strings; JustOff CAA link (98); `http://` button (45). | Jihad case. |
| 33 | Search plugins | `databases/searchPlugins.php:2-36` | 33 hard-coded ids `search-100…132` → files in `datastore/searchplugins/` (not in the repo), incl. `pale-moon-add-ons-site.xml`. | Replace. |
| 34 | Log size | `classLog.php:45` vs `administration.php:62` | `LIMIT 250`, titled "Last 100". | Cosmetic. |
| 35 | Templates | `account-registration.xhtml:12,14`, `account-validation.xhtml:4`, `addon-metadata.xhtml:192`, `panel-frontpage.xhtml:4-6`, `developer-frontpage.xhtml:12-17` | palemoon.org addresses and links, Moonchild logos. | Edit. |
| 36 | Special skin logo | `base/skin/default/template-header.xhtml:128` | `/components/special/skin/default/logo.png`: **this path does not exist**. The file is `base/skin/default/logo.png`. | Fix. |

**What belongs in a local config outside the web root** (proposed `/etc/riverstone-addons/config.php`,
`require`d at the top of `index.php` before the `const` arrays):
- DB host or socket, user, password, database name and charset;
- the app entry (domain, name, oldVersion, uaToken, features) and the app ID map;
- the debug and display-errors flag;
- the maintenance override secret;
- absolute datastore and obj paths;
- upload limits;
- a `registration_enabled=false` flag, as defence in depth (the code paths should still be removed);
- mail identity, if ever needed.

Top-level `const` declarations in `index.php` execute in order and can reference `define()`d values; the
comment at `index.php:13-16` relies on this.

---

## 3. Reconstructed database schema

There is no schema in the repo. Below is every column the code reads or writes; types are **[inferred]** unless
marked. MariaDB, InnoDB, `utf8mb4`.

```sql
-- ── addon ───────────────────────────────────────────────────────────────────────────────────────────────
-- Read: classReadManifest.php:74-111 (single), 152-255 (lists); written: classWriteManifest.php:93-108, 133-136,
-- 209-213, 238-239, 256-269, 315-316, 328-355, 367-409, 465-466.
CREATE TABLE `addon` (
  `id`           VARCHAR(255) NOT NULL,        -- em:id ({GUID} or user@host); externals '<slug>@external' (cWM:342),
                                               -- legacy externals '…@ex-NNN' (cRM:443-454)
  `slug`         VARCHAR(64)  NOT NULL,        -- [a-z0-9-]{3,32} (cWM:38-43, 335-340); langpacks 'pm-<locale>' (cWM:54)
  `type`         ENUM('extension','theme','langpack','external') NOT NULL,   -- cWM:77-81, 332
  `active`       TINYINT(1)   NOT NULL DEFAULT 0,  -- cast to bool cRM:347-348
  `reviewed`     TINYINT(1)   NOT NULL DEFAULT 0,
  `releaseXPI`   VARCHAR(255) NULL,            -- '<slug>-<em:version>.xpi' (cWM:87, 202); key into xpinstall
  `xpinstall`    MEDIUMTEXT   NULL,            -- JSON, see below (cWM:88-91, 107, 203-207)
  `category`     VARCHAR(64)  NOT NULL DEFAULT 'unlisted',  -- extension slug | 'themes' | 'language-packs' | 'unlisted'
                                               -- (cWM:100, 111, 284; cRM:156-157, 167). NOT NULL+default matters: the
                                               -- external INSERT (cWM:328-355) sets no category, and NULL fails every
                                               -- `NOT category = …` filter.
  `url`          VARCHAR(512) NULL,            -- '/addon/<slug>/' or absolute external URL (cWM:101, 267, 374)
  `name`         VARCHAR(255) NULL,            -- em:name en-US (cWM:102, 216); nullable: externals start empty
  `creator`      VARCHAR(255) NULL,            -- em:creator
  `description`  TEXT         NULL,            -- em:description en-US
  `homepageURL`  VARCHAR(512) NULL,
  `license`      VARCHAR(32)  NULL,            -- lower-case code from LICENSES or 'copyright'/'custom' (cRM:537-592)
  `licenseText`  MEDIUMTEXT   NULL,            -- custom licence text (cWM:262, 301-307)
  `licenseURL`   VARCHAR(512) NULL,            -- READ ONLY in code (addonSite.php:274; cRM:553, 568); set by hand
  `repository`   VARCHAR(512) NULL,            -- cWM:263
  `supportURL`   VARCHAR(512) NULL,            -- cWM:264
  `supportEmail` VARCHAR(254) NULL,            -- cWM:265
  `tags`         TEXT         NULL,            -- search target (cWM:266, 375)
  `content`      MEDIUMTEXT   NULL,            -- "phoebusCode" page body (cWM:268; cRM:489-529)
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_slug` (`slug`),
  KEY `ix_type` (`type`),
  KEY `ix_category` (`category`),
  FULLTEXT KEY `ft_tags` (`tags`)              -- REQUIRED: MATCH(`tags`) AGAINST(...) (cRM:178, 190). Search only
                                               -- matches tags, never name or description.
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- xpinstall JSON (cWM:87-91):
-- { "<slug>-<ver>.xpi": { "version":"1.2", "hash":"<sha256 hex>", "epoch":1700000000,
--                         "targetApplication": { "{4534aac8-…}": {"minVersion":"1.0","maxVersion":"1.*"}, … } }, … }
-- cRM:392 adds "date" at read time; cRM:398 sorts newest-first by epoch.

-- ── client ──────────────────────────────────────────────────────────────────────────────────────────────
-- One TINYINT column per TARGET_APPLICATION_ID key (index.php:109-118): the validator INSERTs/UPDATEs every key
-- (cWM:131-135, 233-236, 956-967) and public queries use `WHERE <appkey> = 1`.
CREATE TABLE `client` (
  `addonID` VARCHAR(255) NOT NULL,             -- = addon.id; JOIN ON addon.id = client.addonID (cRM:76 etc.)
  `toolkit` TINYINT(1) NOT NULL DEFAULT 0,     -- keep only if 'toolkit' stays in TARGET_APPLICATION_ID
  `jihad`   TINYINT(1) NOT NULL DEFAULT 0,
  -- upstream also: palemoon, basilisk, ambassador, borealis, epyrus, thunderbird (and the externals form writes
  -- palemoon/basilisk/ambassador/borealis/epyrus explicitly, cWM:378-385, 408-409)
  PRIMARY KEY (`addonID`)
  -- NO foreign key: the code INSERTs client BEFORE addon (cWM:135-136, 354-355). The panel's `SELECT *` joins
  -- both tables, so client columns must never share a name with an addon column (cRM:106-111, 358-365).
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ── persona ─────────────────────────────────────────────────────────────────────────────────────────────
-- Upstream columns and their order are VERIFIED [live] from the SELECT * JSON on addons.palemoon.org
-- (?component=aus&persona=palemoon). Code: classPersona.php:24-29, 63-90, 130-169; persona-category.xhtml:17-22.
-- SELECT * is published wholesale (HTML attribute and AUS JSON), so EVERY column is public.
CREATE TABLE `persona` (
  `id`          VARCHAR(64)  NOT NULL,          -- STRING ids ('palemoon', 'palemoon-classic') [live]; also the directory
                                                -- name datastore/personas/<id>/ and the LWT id. Restrict to [a-z0-9-].
                                                -- LWT requires a JSON *string* (LightweightThemeManager.jsm:634).
  `name`        VARCHAR(255) NOT NULL,          -- LWT mandatory (LightweightThemeManager.jsm:30)
  `author`      VARCHAR(255) NULL,              -- used by persona-category.xhtml:21 (palemoon skin)
  `description` TEXT         NULL,
  `textcolor`   VARCHAR(7)   NULL,              -- '#092040' [live]
  `accentcolor` VARCHAR(7)   NULL,              -- '#8bb3e3' [live]
  `hasFooter`   TINYINT(1)   NOT NULL DEFAULT 0,-- returned as string "1" (never cast) [live]; gates footerURL (cP:153-155)
  `imageFormat` VARCHAR(4)   NULL,              -- 'png'|'jpg' [live]; NOT read by any code; files are still *.png
  `license`     VARCHAR(32)  NULL,              -- 'copyright' [live]; NOT read by code; LWT drops it
  `active`      TINYINT(1)   NOT NULL DEFAULT 0,-- cast to int (cP:130-131)
  `reviewed`    TINYINT(1)   NOT NULL DEFAULT 0,
  -- Referenced only by the dead 'panel-user-personas' query (cP:70-81, never called; `type IN ('extension','theme')`
  -- is a copy-paste from classReadManifest): `slug`, `type`. Not present upstream [live]. Do not add.
  -- ── Recommended Jihad additions [new] ──
  `version`     VARCHAR(32)  NOT NULL DEFAULT '1',  -- WITHOUT this, LWT updates never apply (LightweightThemeManager.jsm:203-206, 676-677)
  `licenseURL`  VARCHAR(512) NULL,              -- CC BY / BY-SA attribution (owner's licensed-art rule)
  `sourceURL`   VARCHAR(512) NULL,
  `attribution` VARCHAR(512) NULL,
  `created`     INT UNSIGNED NOT NULL DEFAULT 0,
  `updated`     INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `ix_listed` (`active`, `reviewed`, `name`),
  FULLTEXT KEY `ft_persona` (`name`, `author`, `description`)   -- for the new search feed
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ── user ────────────────────────────────────────────────────────────────────────────────────────────────
-- classAccount.php:31-39, 105-114, 158-164, 263-264, 280, 305, 330-345, 357-376, 387.
CREATE TABLE `user` (
  `username`    VARCHAR(32)  NOT NULL,          -- [a-z0-9_-]{3,32} (cA:52-66)
  `password`    VARCHAR(255) NOT NULL,          -- password_hash(PASSWORD_BCRYPT) (cA:74, 249); 60 chars today
  `active`      TINYINT(1)   NOT NULL DEFAULT 0,
  `level`       TINYINT UNSIGNED NOT NULL DEFAULT 1,  -- 1..5 (cA:8-14, 193-195)
  `displayName` VARCHAR(255) NULL,
  `email`       VARCHAR(254) NULL,              -- uniqueness enforced in code (cA:84-93, 204-211)
  `extraData`   TEXT NOT NULL,                  -- JSON {"regEpoch":<int>,"verification":"<40 hex>"|null}
                                                -- (cA:105-110). MUST be valid JSON with regEpoch, or cA:291-292/315
                                                -- emit warnings → fatal under Phoebus's handler.
  `addons`      TEXT NOT NULL,                  -- JSON array of owned slugs, '[]' (cA:111, 337-345); searched with
                                                -- LIKE '%"slug"%' (cA:387)
  PRIMARY KEY (`username`),
  UNIQUE KEY `uq_email` (`email`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ── log ─────────────────────────────────────────────────────────────────────────────────────────────────
-- classLog.php:21-29 (INSERT ?n SET ?u), 42-52 (ORDER BY eventID DESC LIMIT 250).
CREATE TABLE `log` (
  `eventID`  INT UNSIGNED NOT NULL AUTO_INCREMENT,   -- only sorted on, never written
  `epoch`    INT UNSIGNED NOT NULL,
  `username` VARCHAR(32)  NOT NULL,                  -- or 'anonymous'
  `ip`       VARCHAR(255) NOT NULL,                  -- X-Forwarded-For list possible (index.php:351)
  `action`   TEXT NOT NULL,
  PRIMARY KEY (`eventID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ── amo ─────────────────────────────────────────────────────────────────────────────────────────────────
-- classWriteManifest.php:1041-1052: "SELECT id, blocked FROM amo WHERE id = ?s AND blocked = 1". It is queried on
-- every new submission (cWM:67), so the table MUST exist (empty is fine); otherwise SafeMySQL throws.
CREATE TABLE `amo` (
  `id`      VARCHAR(255) NOT NULL,
  `blocked` TINYINT(1)   NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
```

**Owner bootstrap.** The code has no installer and no first-admin path:
```sql
-- hash: php -r 'echo password_hash("…", PASSWORD_BCRYPT), "\n";'
INSERT INTO `user` SET `username`='owner', `password`='$2y$10$…', `active`=1, `level`=5,
  `displayName`='…', `email`='…', `extraData`='{"regEpoch":1760140800,"verification":null}', `addons`='[]';
```
- The account `email` and `displayName` must be non-empty, or `authenticate()` redirects level <3 users to
  `/panel/account/` (`classAccount.php:460-464`). It does not do this for level 5.
- Abbreviations above: cWM = classWriteManifest.php, cRM = classReadManifest.php, cA = classAccount.php,
  cP = classPersona.php.

---

## 4. Personas end to end

### 4.1 Storage

- **Table** `persona` (§3). Personas are **global, not per application**: there is no `client` join
  (`classPersona.php:63-90`), so every app site lists every persona.
- **Datastore** `ROOT/datastore/personas/<id>/` (`classPersona.php:149`). The `.png` name is used even when the
  bytes are JPEG (`imageFormat:'jpg'` [live]). The GD preview step detects GIF/JPEG/PNG by content
  (`classPersona.php:182-194`).

| File | Size [live] | Required | Source |
|---|---|---|---|
| `header.png` | 3000×200 | yes (LWT mandatory `headerURL`) | `classPersona.php:151` |
| `footer.png` | 3000×100 | if `hasFooter` | `:153-155` |
| `icon.png` | 32×32 | no; fallback `datastore/addons/default/theme.png` (32×32 [live]) | `:157-160` |
| `preview.png` | 240×60 | auto-generated from the header if missing | `:162-165, 175-248` |

### 4.2 Creation

**There is none.**
- No panel task, form, upload handler or INSERT touches `persona` or `datastore/personas/` (grep of the whole
  tree). Upstream personas were created by hand (SQL plus file copy).
- Jihad needs either a scripted importer (SQL + rsync, the simplest for a fixed bundle of 15) or a new admin
  panel task. The task would follow `classWriteManifest::filesToDatastore()` (`:1117-1169`) as a model, adding
  dimension checks with `getimagesize`.

### 4.3 Listing: `/personas/`

1. `addonSite.php:143-158`: gate `personas`, then `getPersonas('site-all-personas')`
   (`classPersona.php:62-69`: `SELECT * FROM persona ORDER BY name`, **no LIMIT or paging**), then
   `addonSite('cat-personas', 'Personas', $list)`.
2. `funcProcessManifest()` (`classPersona.php:124-173`), per row:
   - drops the row unless active and reviewed (130-139);
   - `finalDescription` = description, or `substr(…,0,230).'&hellip;'` if ≥235 **bytes** (142-145). Byte
     truncation can split a UTF-8 sequence, after which `json_encode` returns `false` and the attribute is
     empty. The entity also reaches the browser as literal text;
   - URLs use a hard-coded `'http://' . currentDomain` (148), even though the site is https-only;
   - `headerURL`, `footerURL` (if `hasFooter`), and `iconURL` (or the default, checked with `file_exists`
     relative to CWD) (151-160);
   - `previewURL`, and if the file is missing it **writes** `preview.png` synchronously during the request
     (162-165);
   - `updateURL = https://<domain>/?component=aus&persona=<id>` (168-169).
3. Template contract (`base/skin/palemoon/persona-category.xhtml`, identical in basilisk except `author`):
   - lines 1-7: `setTheme(node)` dispatches a bubbling `InstallBrowserTheme` DOM event from the clicked node.
     The browser (Jihad: `GoannaRenderPage.cpp` capture listener → `jihadPersonas.handle()`, per
     `persona-licences.md:196-201`) reads the node's `data-browsertheme`.
   - line 17: `<a href="" … onclick="setTheme(this); return false;" data-browsertheme='{$key|json_encode:320 nofilter}'>`.
     320 = `JSON_UNESCAPED_SLASHES|JSON_UNESCAPED_UNICODE`. **No `JSON_HEX_APOS`/`JSON_HEX_TAG`/`JSON_HEX_AMP`**,
     inside a single-quoted attribute. A name like `Owner's <b>Moon</b>` produces
     `data-browsertheme='{"…":"Owner's <b>Moon</b>"…}'` **[tested-8.4]**: the attribute breaks and HTML is
     injected.
   - lines 18-19: `{$key.iconURL|substr:5}` / `{$key.previewURL|substr:5}` strip exactly `http:` to make
     protocol-relative URLs. Switching `classPersona.php:148` to https without changing these lines yields
     `s://…`.
   - lines 21-22: `{$key.name}`, `{$key.author}`, `{$key.description}` are raw (not `finalDescription`).
   - Smarty variables used: `PAGE_TITLE`, `PAGE_DATA` (a list of processed persona rows).
4. The production element **[live]** (the whole processed row is serialised):
   ```
   data-browsertheme='{"id":"palemoon","name":"Pale Moon","author":"Moonchild","description":"…","textcolor":"#092040",
   "accentcolor":"#8bb3e3","hasFooter":"1","imageFormat":"jpg","license":"copyright","active":1,"reviewed":1,
   "finalDescription":"…","headerURL":"http://addons.palemoon.org/datastore/personas/palemoon/header.png",
   "footerURL":"http://…/footer.png","iconURL":"http://…/icon.png","previewURL":"http://…/preview.png",
   "updateURL":"https://addons.palemoon.org/?component=aus&persona=palemoon"}'
   ```

### 4.4 What the browser accepts (UXP)

`_sanitizeTheme` (`Jihad-Browser/third_party/uxp/toolkit/mozapps/extensions/LightweightThemeManager.jsm:622-664`):
- Mandatory `id`, `name`, `headerURL` (`:30`); optional fields at `:31`.
- Each value must be a **string** (`:634`), so a numeric `id` makes the install fail.
- Image URLs must be http, https or resource. `updateURL` must be **https** (`:645`).
- Unknown keys (`license`, `active`, `hasFooter`, `imageFormat`, `finalDescription`) are dropped.

### 4.5 Previews (GD crop)

`classPersona.php:175-248`:
1. Load `header.png` by detected type (182-194).
2. Scale to cover 240×60 (196-225).
3. Crop **right-aligned** horizontally (`x0 = temp_width − 240`, 231), because LWT headers anchor top-right,
   and **vertically centred** (232).
4. `imagepng()` to `preview.png` (247).

Failure modes, each fatal under Phoebus's handler:
- missing header → `getimagesize` false → `DivisionByZeroError` at 196 **[tested-8.4]**;
- unsupported type → undefined `$source_gdim`;
- odd height difference → fractional `y0` passed to `imagecopy()` int param → PHP 8.1 "Implicit conversion
  from float … loses precision" **[tested-8.4 mechanism]**;
- unwritable directory → `imagepng` warning.

PHP-FPM therefore needs **write** access to `datastore/personas/*`. Pre-generate previews offline instead.

### 4.6 AUS persona endpoint `?component=aus&persona=<id>`

`addonUpdateService.php:49, 61-74`:
- Handled **before** the AUS sanity checks, so a plain GET works; no `Moz-XPI-Update` needed.
- `getPersonaByID()` (`classPersona.php:22-43`: `WHERE id = ?s`) → `funcProcessManifest($row, null, true)`.
  Inactive returns `{}`; **unreviewed-but-active is returned**. It can also trigger preview generation.
- Response: `Content-Type: application/json` (`fundamentals.php:53`), pretty-printed `JSON_ENCODE_FLAGS`
  (`fundamentals.php:97`), the same object as the listing **[live]**. `{}` if not found.
- Client use: `LightweightThemeManager.updateCurrentTheme()` (`LightweightThemeManager.jsm:174-214`):
  - only when `lightweightThemes.update.enabled` is set;
  - only for the **current** theme;
  - replaces it when the id matches and `version` differs (203-206).
  
  Upstream rows have no `version`, so `"" == ""` and nothing ever updates. Add the `version` column (§3) and
  bump it on every image change.

### 4.7 Other places personas appear

- The discover pane labels the category "Lightweight Themes" (`discoverPane.php:62-63`), Pale Moon only.
- `OTHER_CATEGORY_SLUGS['personas']` (`index.php:136`).
- Menu links in the site templates.
- The `persona.id` value becomes a filesystem path component (`classPersona.php:149, 158, 163-164`). It is
  only safe because it comes from the DB, so restrict ids to `[a-z0-9-]` at insert time.

### 4.8 The new JSON feed component (design constraints from the code)

- Register in `COMPONENTS` (`index.php:35-43`), e.g. `'personas' => ROOT_PATH . '/components/api/personaFeed.php'`,
  and call it as `/?component=personas&page=1&per_page=24&q=moon`. With `component` set and no `path`, routing
  goes straight to it (`index.php:483-500`).
- Use `gfImportModules('database')` and query directly with `?i` (LIMIT/OFFSET) and `?s` (FULLTEXT or escaped
  LIKE). Do **not** reuse `funcProcessManifest()` as-is: it builds http URLs, writes previews, and byte-truncates.
  Factor out URL building.
- `gfSuperVar('get')` strips non-ASCII (`fundamentals.php:102`), so non-English searches break. Read `q` with a
  `/u`-aware sanitiser.
- The global client gate still applies: the request must carry a UA, and not curl/wget/git
  (`index.php:245-267`). Jihad's chrome XHR sends the Jihad UA.
- Emit LWT-shaped objects with string values:
  `{id,name,author,description,headerURL,footerURL?,iconURL,previewURL,textcolor,accentcolor,updateURL,version}`,
  plus licence and attribution. Use `JSON_HEX_TAG|JSON_HEX_APOS|JSON_HEX_QUOT|JSON_HEX_AMP` and
  `JSON_INVALID_UTF8_SUBSTITUTE`. Add `Cache-Control: public, max-age=300`.
- Add CORS only if the page that fetches it is unprivileged content.

---

## 5. Other features

### 5.1 Extensions and themes (XPI handling)

**Submission** (`classWriteManifest::submitNewAddon`, `:27-143`):
1. slug check (38-44);
2. `validateAddon()` (678-1002);
3. AMO-blocked check unless level ≥3 (67-74);
4. type map 2 → extension, 4 → theme, 8 → langpack (76-85);
5. release file `<slug>-<em:version>.xpi` with sha256 and epoch (87-91);
6. `active = level>1`, `reviewed = level>2` (97-98);
7. the add-on is assigned to the user's `addons` JSON (117-118);
8. files go to the datastore (121-125);
9. INSERT `client`, then `addon` (134-136).

**Validation** (`:678-1002`):
- MIME via fileinfo ∈ zip/x-xpinstall/java-archive (710, 749-756; `fundamentals.php:326`).
- `install.rdf` read through the `zip://` wrapper (761-771, 1007-1020) and parsed by `classMozillaRDF`.
- Jetpack (`harness-options.json`/`package.json`) and WebExtension (`manifest.json`) are refused (780-794).
- Required: id, type ∈ {2,4} (or {8}), version, creator, name, description (non-langpack), targetApplication.
- Refused: embedded WebExtension, `updateURL`/`updateKey` (817-903).
- Licence normalised against `LICENSES` (907-917).
- ID syntax and restricted IDs, **only when level <3** (926-951).
- At least one non-toolkit supported app (956-973).
- No `maxVersion="*"` (978-994).
- **`em:version` is never validated.** It goes into file names (87, 1094) and into XML output unescaped
  (`classGenerateContent.php:290`, `update.rdf:6`).

**install.rdf parsing** (`modules/classMozillaRDF.php`):
- Rdf_parser (expat, `libraries/librdf/rdf_parser.php:82-92`) with a static statement handler (`:77-123`).
- Collected: single props `id, type, version, creator, homepageURL, updateURL, updateKey, bootstrap,
  hasEmbeddedWebExtension, multiprocessCompatible, skinnable, strictCompatibility, license, iconURL, icon64URL`
  (86-88); multi props `contributor, developer, translator, targetApplication` (91); localised `name`,
  `description` keyed by `xml:lang`, default `en-US` (94, 111-114).
- `targetApplication` is flattened to `{appId:{minVersion,maxVersion}}` (40-56). Missing values → `gfError` (44-48).

**Storage** (`filesToDatastore`, `:1057-1170`):
- `ROOT/datastore/addons/<slug>/`, created 0755 (1075-1081);
- XPI chmod 0644 (1097), then **`if ($aType = 'langpack')`**, an assignment that is always true (1102), so it
  always uses `rename()` and never `move_uploaded_file()`;
- icon/preview: PNG only, ≤512 KiB → `icon.png`, `preview.png` (1118-1167).

**Display** (`classReadManifest::processManifest`, `:339-481`):
- `description` is `htmlentities`'d (402-404); `content` → phoebusCode (489-529); licence handling (537-592);
- `baseURL = https://<domain>/?component=download&version=latest&id=` (427-431);
- `basePath = ./datastore/addons/<slug>/` (435);
- icon/preview fall back to `datastore/addons/default/{icon,preview}.png` (457-477).

**Download** (`base/addonDownload.php`):
- `?component=download&id=<em:id>&version=<ver|latest>[&panel=1]` (92-122).
- Lookup `by-id`, or **`panel-by-id` (returns inactive and unreviewed, with no auth)** when `panel` is present
  (104-109).
- Headers: `Content-Type: application/x-xpinstall` (`octet-stream` for binary or panel), `Content-Disposition:
  inline`, `Content-Length`, **`X-Accel-Redirect: /datastore/addons/<slug>/<file>.xpi`** (41-55). PHP sends no
  body: nginx must serve it.
- An unknown id falls back to the search-plugin map: `search-NNN` → `readfile(./datastore/searchplugins/<file>)`,
  `text/xml` (65-86, 115-121).

### 5.2 update.rdf / AUS for add-ons

Request (`addonUpdateService.php:12-30, 49-55`):
- Required: `id`, `version`, `appID`, `appVersion`, `compatMode`, plus the **`Moz-XPI-Update`** header (UXP sends
  it: `AddonUpdateChecker.jsm:632`, `XPIProvider.jsm:5385`) or `?updateOverride`. Otherwise an empty RDF in
  production (79-96).
- `id` ∈ `BAD_ADDON_IDS` → empty (101-107).
- `appID` must equal `TARGET_APPLICATION_ID[jihad]`, else empty (112-130).
- `getAddon('by-id')` (`classReadManifest.php:72-81`: joined `client`, `<app>=1`, type
  extension/theme/langpack; **unreviewed allowed**).

Response (`classGenerateContent::addonUpdateService`, `:257-308`, template `components/api/content/update.rdf:1-22`):
- `RDF:Description about="urn:mozilla:{type}:{id}"` with one `RDF:li`: version, targetApplication
  (id/min/max for the current app), `updateLink = baseURL+id`, `updateHash sha256:<hash>`.
- Langpacks use type `item` (283-285).
- Only the latest release is offered; client `version`/`appVersion` are ignored.
- Values are substituted raw (only `updateLink` is CDATA), so a `<` or `&` in a version breaks the XML.
- Empty answer: `<RDF:RDF … />` (10-11, 266-275).

If the release XPI lacks the Jihad GUID, line 280 reads an undefined key and the request dies as a fatal error
page instead of returning RDF.

### 5.3 Language packs

- Admin-only: level ≥3 can submit (`administration.php:73-94`, no extra check); level ≥4 to list or update
  (`:20-23, 136-141, 177-180`).
- Bulk zip upload (`administration.php:310-336`; `classWriteManifest.php:503-653`): extracted to `.obj`, never
  cleaned.
- **Hard-wired to Pale Moon**:
  - requires `supportedApplications['palemoon']` (`classWriteManifest.php:50-52, 186-188`);
  - slug `pm-<id minus @palemoon.org>` (54, 569);
  - creator "Moonchild Productions", description "Pale Moon Language Pack" (110-115);
  - category `language-packs`.
- Langpack IDs are not syntax-checked for admins, which feeds the second-order SQLi in §7.
- To support Jihad: parameterise the app key, slug prefix, creator and description.

### 5.4 Search plugins

- `databases/searchPlugins.php:2-36`: a PHP array `search-NNN` → XML file.
- Listing (`classReadManifest::getSearchPlugins`, `:291-315`): `simplexml_load_file(ROOT/datastore/searchplugins/<file>)`
  for each entry, every request. Uses `ShortName` and `Image` (a data URI); keyed by ShortName, so duplicates
  collapse.
- Install: `window.external.AddSearchProvider(SITE_DOMAIN + '/?component=download&id=search-NNN')`
  (`searchplugin-category.xhtml:1-6, 18`).
- The Pale Moon skin also advertises the site's own engine via
  `<link rel="search" … href="{$SITE_DOMAIN}/?component=download&id=search-119">` (`site-template.xhtml:8`).
- There is no panel UI. Search plugins are file plus array edits.

### 5.5 Discover pane

`components/discover/discoverPane.php` (no DB, no Smarty):
- `{%…}` substitution into `components/discover/content/template.xhtml` (`:108-131`).
- `template-announcement.xhtml` only for the app key `no-announcement` (108-110), which is dead code.
- Images: `/components/discover/skin/<app>.png`, `discover-logo.png`, and for Pale Moon
  `/base/skin/shared/<slug>.png` (46, 84; template:153).
- Basilisk gets a JustOff CAA button (96-103).
- UXP locks discover navigation to the discover host (`persona-licences.md:173`).

### 5.6 amIntegration API

`components/api/amIntegration.php`:
- Parameters: `type` (`internal`|`external`), `request`, `q`, `addonguid`, `version` (unused) (12-16). Missing
  `type`/`request` → error (23-26).
- **internal**:
  - `search`: `getAddons('api-search', q)` → FULLTEXT on `tags` only, types extension/theme/langpack, not
    `unlisted` (`classReadManifest.php:181-192`) → XML.
  - `get`: `addonguid` comma list → `api-get` (`:193-203`) → XML; empty → `<searchresults total_results="0" />`.
  - `recommended`: `<addons />`.
  - anything else: error.
- **external** redirects: `search` → `/search/?terms=q`, `themes` → `/themes/`, `searchplugins` →
  `/search-plugins/`, `devtools` → `/extensions/web-development/`, `recommended`/default → `/` (53-69). An
  unknown `type` returns an empty 200.
- Response (`classGenerateContent::amSearch`, `:315-396`; template `components/api/content/addon.xml`):
  - `<searchresults total_results="N">` + one `<addon>` per result: `name`, `type id` (1 extension, 2 theme,
    8 langpack, else 0), `guid`, `version`, `homepage`;
  - `learnmore` = **`http://`** + domain + url, wrong for externals because `url` is already absolute;
  - `authors/author/name`, `link=about:blank`, `summary` (htmlentities'd description in CDATA);
  - `icon size=64|32` (**`http://`**), `compatible_applications/application/{appID,min_version,max_version}`,
    `last_updated epoch`, `install` = download URL.
  - No paging and no `maxResults`. Personas are never returned.

---

## 6. The panel

**Entry** `components/panel/phoebusPanel.php`.
- URIs `/panel/`, `registration/`, `verification/`, `login/`, `logout/`, `developer/`, `account/`, `addons/`,
  `administration/` (9-17).
- Kill switch at 58-60.
- **HTTPS required**: the app must have the `https` feature (68-70) and `currentScheme` must be https (72-74).
  That needs `fastcgi_param HTTPS on` or `SCHEME https`.

**Authentication** (`classAccount::authenticate`, `:404-475`):
- **HTTP Basic** on every request (`PHP_AUTH_USER`/`PHP_AUTH_PW`, 409-415; prompt in `fundamentals.php:711-715`).
- `password_verify` (443).
- Unverified → redirect to `/panel/verification/` (449-451); inactive → re-prompt (454-457); level <3 without
  email or displayName → forced to `/panel/account/` (460-464).
- **No sessions, no cookies, no `session_start`, no lockout or rate limit** (grep). "Logout" is an XHR with
  `logout:logout@` credentials (419-434).
- So session fixation is not applicable, CSRF is fully applicable, and brute force is unthrottled.

**Password hashing**:
- `password_hash($pw, PASSWORD_BCRYPT)`, default cost 10 (74, 249); length 8-64 enforced on set (68-72, 246-248).
- No `password_needs_rehash`.
- The registration form's HTML `minlength="3"` (`account-registration.xhtml:23`) disagrees with the server's 8.

**Account levels** (`classAccount.php:8-14`) and where they matter:

| Level | Name | Effects |
|---|---|---|
| 1 | Add-on Developer | Submissions inactive and unreviewed (`classWriteManifest.php:97-98`). ID and AMO checks enforced (`:46, 69`). Developer pages only. |
| 2 | Advanced/Legacy | Submissions **active** but unreviewed. Public by URL (`by-slug`/`by-id` allow unreviewed, `classReadManifest.php:73, 83`) but not in lists. |
| 3 | Add-ons Team Member | `/panel/administration/` (`phoebusPanel.php:126-129`). Submissions auto-reviewed. ID checks skipped. Can submit langpacks. Can edit lower users. |
| 4 | Team Leader | List and update langpacks; delete add-ons (`administration.php:278-281`). |
| 5 | Administrator | Edit any user (`administration.php:238-243`; `classAccount.php:224-234`). |

The owner should be level 5. The rank checks in `updateUserManifest` (`classAccount.php:224-234`) prevent
self-promotion above one's own level.

**Registration**:
- `registerUser()` (`classAccount.php:49-128`): username regex, password length, email **non-empty only (no
  format check)**, displayName non-empty, uniqueness, `fnmatch` blacklist (512-524), ban list.
- INSERTs `active=0, level=1` **before** mailing (113-124), so the row exists even if mail fails.
- Verification code = 40 hex chars from `random_bytes` (`fundamentals.php:640-649`), compared with `!=`
  (`classAccount.php:150`; loose, not constant-time).

**How to disable registration for owner-only operation.** It must be done in code; nginx alone is insufficient
because `/?component=panel&path=/panel/registration/` reaches the same switch (`index.php:499-500` + `phoebusPanel.php:79`).
1. Delete the `URI_REG` and `URI_VERIFY` cases (`phoebusPanel.php:83-107`) so they 404, or put a config guard on them.
2. Remove the links in `components/panel/skin/default/site-template.xhtml:27-28` and the copy in
   `panel-frontpage.xhtml`.
3. Remove the level <3 email-change re-verification path (`classAccount.php:203-221`).
4. Create the owner by SQL (§3).
5. Restrict who can reach the panel in nginx (allowlist or mTLS on `location ^~ /panel/`). Also make `index.php`
   refuse `component=panel` unless `REQUEST_URI` starts with `/panel/`.

**CSRF**: **none.**
- No token is generated or checked anywhere.
- Forms post only `slug`/`username` hidden fields: `delete-addon.xhtml:5-10`, `account-metadata.xhtml:114-117`,
  `addon-metadata.xhtml:204-207`, `external-metadata.xhtml:104-108`, `addon-submit-update.xhtml:17-34`,
  `addon-bulk-upload.xhtml:37-47`.
- The browser replays Basic credentials on cross-site form posts. Add a per-user HMAC token (e.g.
  `hash_hmac('sha256', username|path, secret)`), check `Origin`/`Referer`, and send `X-Frame-Options`.

**Upload validation**:
- XPIs and images: §5.1. No size cap in code (relies on `upload_max_filesize`/`post_max_size`/nginx
  `client_max_body_size`). No dimension or decode check on images.
- `gfSuperVar('files')` hard-fails on any upload error and replaces the client MIME with fileinfo's
  (`fundamentals.php:315-327`).

**Logging**: `classLog::record` on panel successes and some failures (e.g. `developer.php:77, 107, 120, 153`;
`administration.php:87, 139, 157…`). The `ip` value is spoofable (§7).

---

## 7. Security review (prioritised)

Severity assumes the planned deployment (owner-only, internet-facing). "Owner-only" lowers anything that needs a
malicious uploader.

### P0: fix before first deploy
| # | Finding | Evidence | Fix |
|---|---|---|---|
| P0-1 | **DB credentials in the web root.** The datastore must be public (images, `classPersona.php:151`; `classReadManifest.php:460`), so `/datastore/.phoebus/sql` is downloadable as plain text unless denied. | `classDatabase.php:16` | Move creds out of the root (§2). Deny `~ /\.` in nginx (§10). |
| P0-2 | **Open registration → anonymous stored XSS → admin takeover.** `displayName`/`email` are unfiltered POSTs (`fundamentals.php:300`; no email validation, `classAccount.php:76-82`). They are stored before mailing (113-124) and rendered raw in the admin list (`administration-users-list.xhtml:36-38`) and titles (`administration.php:57, 260`). Smarty does not escape (`Smarty.class.php:562`). The script runs with the admin's Basic credentials, so it can POST a level change. | as cited | Remove registration in code (§6). Enable `escape_html`. |

### P1
| # | Finding | Evidence | Fix |
|---|---|---|---|
| P1-1 | **Unauthenticated `special` component**: `/special/phpinfo/` (`phpinfo(INFO_GENERAL|CONFIGURATION|ENVIRONMENT|VARIABLES)`) **[tested-8.4]**; `/special/validator/` takes anonymous uploads, opens a DB connection and parses zip/XML (DoS; textarea break-out in `gfGenContent`, `index.php:181-186`); `/special/test/` includes `base/tests/*.php`. Also reachable as `?component=special`. | `specialComponent.php:38-100`; `index.php:42, 492-494, 499-500` | Remove `special` from `COMPONENTS`. Deny `/special/` in nginx. |
| P1-2 | **No CSRF protection** on any panel POST (delete add-on, change password, change level, upload). | §6 | Tokens, Origin check, `X-Frame-Options`. |
| P1-3 | **Errors disclose internals.** `display_errors=on` (`index.php:10`); `gfErrorHandler` prints message, file and line (`fundamentals.php:242-253`); SafeMySQL throws `Exception("…Full query: [<SQL>]")`, uncaught (`safemysql.class.php:483, 614-624`); on PHP ≥8.1 mysqli throws `mysqli_sql_exception` (user and host in the message). | as cited | Production error mode; catch exceptions; log. |

### P2
| # | Finding | Evidence | Fix |
|---|---|---|---|
| P2-1 | **Second-order SQL injection** in `findUserAddon()`: `"… LIKE '%\"{$aSlug}\"%'"`. The slug comes from the DB. Langpack slugs derive from the uploaded `em:id` with no syntax check for level ≥3 (`classWriteManifest.php:46, 54, 926`). It runs for every add-on shown to level ≥3 (`classReadManifest.php:368-372`). A slug containing `?s` also breaks SafeMySQL's placeholder count. | `classAccount.php:387` | `$db->query('col', "SELECT username FROM user WHERE JSON_CONTAINS(addons, ?s)", json_encode($slug))`, or an escaped `LIKE ?s`. Validate langpack slugs. |
| P2-2 | **Unpublished XPIs downloadable by anyone**: `?component=download&id=<id>&panel=1` uses `panel-by-id` (inactive and unreviewed). There is no auth in the download component. | `addonDownload.php:94, 104-106`; `classReadManifest.php:94-102` | Require panel auth for `panel=1`. Make `*.xpi` `internal` in nginx. |
| P2-3 | **`?debugOff=1` switches the live site to `devDB`** (or errors if `devDB` is unset). | `classDatabase.php:24-26`; `index.php:357` | Remove the clause. |
| P2-4 | **No brute-force protection** on Basic auth. | `classAccount.php:404-447` | nginx `limit_req` on `/panel/`, allowlist or mTLS, fail2ban on 401s. |
| P2-5 | **No output escaping by default** (systemic; Smarty `escape_html=false`, never set in `classGenerateContent.php:59-80`). | — | `$smarty->setEscapeHtml(true)`. Mark pre-rendered HTML `nofilter`: `content` (`addon-page.xhtml:19`), `licenseText` (`addon-license.xhtml:12`), the bulk-upload messages containing `<strong>` (`addon-bulk-upload.xhtml:19, 28`), the persona JSON. Remove `htmlentities` on `description` (`classReadManifest.php:403`) to avoid double escaping. |
| P2-6 | **Panel access-control bypass of nginx path rules**: `/?component=panel&path=/panel/administration/` reaches the panel without matching `location /panel/`. | `index.php:483-500` | Force `component=panel` only for a `/panel/` `REQUEST_URI`, and refuse `component=panel|special` in the query string. |

### P3
| # | Finding | Evidence |
|---|---|---|
| P3-1 | Stored XSS from add-on metadata: `name`/`creator` raw (`addon-page.xhtml:3, 7`; `addon-category.xhtml:47`). `homepageURL`/`supportURL`/`repository` in `href` allow `javascript:` (`addon-page.xhtml:145-154`). phoebusCode `[url=javascript:…]` → `<a href="$1">` (`classReadManifest.php:515-516`). DB `licenseURL` → redirect (`addonSite.php:274-276`). Owner-only data. |
| P3-2 | Persona JSON attribute breakout and raw name/author/description (`persona-category.xhtml:17, 21-22`) **[tested-8.4]**. Owner-only data, but it breaks on any apostrophe. |
| P3-3 | `em:version` is unvalidated, used in file names (`classWriteManifest.php:87, 1094`) and unescaped in update.rdf (`classGenerateContent.php:290`) and `Content-Disposition` (`addonDownload.php:49`). The `$aType = 'langpack'` assignment bug (`classWriteManifest.php:1102`) skips `move_uploaded_file`. Bulk-upload zips extract into `.obj` with no size cap (zip bomb), admin only (`:514-533`). |
| P3-4 | Maintenance and panel-disable bypass cookies accept any value (`index.php:364`; `phoebusPanel.php:58`). |
| P3-5 | `remoteAddr` trusts `X-Forwarded-For` (`index.php:351-352`) → forged log IPs. |
| P3-6 | Array query parameters (`?path[]=x`, `?component[]=`) pass the filter as arrays (`fundamentals.php:309-311`) → `TypeError` 500 **[tested-8.4]**. |
| P3-7 | App chosen by `SERVER_NAME`. A loose `switch` matches an empty name to `dev => null` → debug mode (`index.php:411-421`). |
| P3-8 | No `X-Frame-Options`/CSP/HSTS from the app (panel clickjacking). |
| P3-9 | Verification code compared with `!=` (`classAccount.php:150`). Moot once registration is removed. |
| P3-10 | `mail()` recipient is an unvalidated POST (`classAccount.php:501`). Moot once registration is removed. |

### Checked and not found
- **SQL injection**: all other queries use SafeMySQL placeholders (`?s ?i ?n ?a ?u`). Identifiers come from code or config.
- **Reflected XSS via GET**: the GET filter removes `< > " '` (`fundamentals.php:102`). `PAGE_TITLE`, `SEARCH_TERMS` and `PAGE_PATH` are safe.
- **Path traversal**:
  - download: id → DB, and version must match a DB entry (`addonDownload.php:18-38`);
  - search plugin: key must exist (`:117`);
  - tests: `in_array` (`specialComponent.php:72`);
  - persona and add-on paths come from DB values.
- **Open redirect**: none attacker-controlled. External API redirects are local paths (`amIntegration.php:56-67`), and the panel redirect keeps the `/panel/` prefix (`developer.php:58`).
- **Template injection**: the template source is built only from files (`classGenerateContent.php:207-209`).
- **Outbound network calls / telemetry**: none (grep). The only external side effect is `mail()`.
- **Session fixation**: no sessions exist.

### Smarty CVEs affecting 3.1.33

Fetched from github.com/smarty-php/smarty/security/advisories on 2026-10-11. Practical exposure in Phoebus is
low: templates are trusted, there is no security policy, no `{fetch}`/`{math}`/`escape:'javascript'`/`{extends}`,
and `caching=0`. **But 3.x is unmaintained.**

| CVE | Issue | Fixed in |
|---|---|---|
| CVE-2021-26120 | Code injection via `{function name=}` | 3.1.39 |
| CVE-2021-26119 | Sandbox escape via `$smarty.template_object` | 3.1.39 (NVD; not in the GitHub list) |
| CVE-2021-29454 | `{math}` code injection | 3.1.42 / 4.0.2 |
| CVE-2021-21408 | Static class access sandbox bypass | 3.1.43 / 4.0.3 |
| CVE-2022-29221 | Code injection via `{block}`/`{include}` names | 3.1.45 / 4.1.1 |
| CVE-2023-28447 | XSS in `escape:'javascript'` | 3.1.48 / 4.3.1 |
| CVE-2024-35226 | Code injection via `{extends}` file name | **all 3.x**; 4.5.3 / 5.2.0 |
| CVE-2026-62993 | SSRF via redirect in `{fetch}` | <4.5.7 (incl. 3.x); 4.5.7 / 5.8.2 |
| CVE-2026-62992 | Symlink escape of trusted dirs | <4.5.7 (incl. 3.x); 4.5.7 / 5.8.2 |
| CVE-2026-82531 | Nocache-hash forgery → PHP in cache files | <4.5.8 (incl. 3.x); 4.5.8 / 5.8.5 |
| CVE-2026-62996 | `stream:` resource bypass | 5.x only (<5.8.4) |

---

## 8. PHP 8.2 / 8.4 compatibility

### 8.1 Why every deprecation matters here

`index.php:9` sets `error_reporting(E_ALL)` and `fundamentals.php:256` installs `gfErrorHandler` (242-253), which
calls `gfError(…, true)` → `gfGenContent` → `exit` for **any** reported level, including `E_DEPRECATED`. A
deprecation is therefore a fatal page. Patch the handler first:
```php
if ($eCode & (E_DEPRECATED | E_USER_DEPRECATED | E_NOTICE | E_USER_NOTICE)) { error_log($eMessage); return true; }
```

### 8.2 Findings

Tested items were run on PHP 8.4.22 through the harness (`run.php`, `boot.php`; not kept).

| Item | PHP | Location | Effect | Fix |
|---|---|---|---|---|
| `E_STRICT` constant | 8.4 deprecated | `fundamentals.php:39` | Emitted at compile time **before** the handler is installed, printed (display_errors on). Then every `header()` → "headers already sent" → fatal page **[tested-8.4]**. Under FPM with output buffering it prefixes every XML/JSON body. | Delete the entry. |
| `preg_split(…, null, …)` | 8.1 deprecated (null to int) | `libraries/safemysql/safemysql.class.php:493` | **Every SQL query** **[tested-8.4]**. | `-1` (patch, or newer upstream). |
| mysqli exceptions | 8.1 default `MYSQLI_REPORT_ERROR|STRICT` | `classDatabase.php:28-31` | Connect and query failures throw instead of returning false. | `mysqli_report(MYSQLI_REPORT_OFF)` before connect, or try/catch. |
| `mysqli_connect_errno($conn)` | 8.0 `ArgumentCountError` | `classDatabase.php:31` | Error path throws. | Drop the argument. |
| `trigger_error(…, E_USER_ERROR)` | 8.4 deprecated | `safemysql.class.php:621` | Only in `errmode=error`; Phoebus uses the default `exception`. | n/a |
| `xml_set_object` + string handlers | 8.4 deprecated | `libraries/librdf/rdf_parser.php:87-89` | Every `install.rdf` parse (3 deprecations) **[tested-8.4]**. | `xml_set_element_handler($p, [$this,'_start_element_handler'], [$this,'_end_element_handler']); xml_set_character_data_handler($p, [$this,'_character_data_handler']);` and drop `xml_set_object`. |
| JustOff block | 7.4+ warning; 8.1 deprecation | `classReadManifest.php:230-231` | `null['addons']` warning plus `json_decode(null)` → admin "Unreviewed" list fatal when the user does not exist **[tested-8.4]**. | Delete 228-244. |
| Float → int param | 8.1 deprecated | `classPersona.php:232-238` (`imagecopy` with `y0` = x.5) | Preview generation fatal for odd height differences **[tested-8.4 mechanism]**. | `(int)` casts. |
| `list()` from false, then divide | 8.0 `DivisionByZeroError` | `classPersona.php:182, 196` | Missing or invalid header → uncaught Error **[tested-8.4]**. | Check the `getimagesize` result. |
| `str_starts_with(null)` | 8.1 deprecated | `classReadManifest.php:568` (licence `custom` without URL); `classMozillaRDF.php:44-46` (missing targetApplication fields) | Fatal page **[tested-8.4]**. | `?? ''`. |
| `json_decode(null)` / missing `regEpoch` | 8.1 / 7.4+ | `classAccount.php:291-292, 314-316` | Hand-inserted users without valid `extraData`. | Data hygiene (§3). |
| Array GET input | 8.0 `TypeError` | `fundamentals.php:309-311` → string functions | 500 **[tested-8.4]**. | Reject non-strings in `gfSuperVar('get')`. |
| `each()`, `create_function`, `$s{0}`, `${var}` interpolation, `money_format`, `utf8_encode`, `FILTER_SANITIZE_STRING` | removed or deprecated | — | **None** in Phoebus, SafeMySQL or librdf (grep). | — |
| Dynamic properties (8.2) | — | Phoebus classes | **None**: all properties are declared (`classAccount.php:21-23`, `classWriteManifest.php:5-10`, `classDatabase.php:7-8`, `classGenerateContent.php:16`, `classReadManifest.php:9-10`, `classMozillaRDF.php:11`, `nsIVersionComparator.php:126-129`). | — |
| **Smarty 3.1.33**: `strftime()` | 8.1 deprecated | `smarty_internal_runtime_codeframe.php:48`, `smarty_internal_config_file_compiler.php:161` | Every template compile **[tested-8.4]**. | Upgrade. |
| **Smarty 3.1.33**: dynamic properties | 8.2 deprecated | `smarty_internal_template.php:719` (`$compiled`, `$compiler`), `smarty_internal_extension_handler.php:182` (`$_codeFrame`, `$getTemplateVars`, `$loadPlugin`, `$_writeFile`, `$_foreach`, …) | Every render **[tested-8.4]**. | Upgrade. |
| **Smarty 3.1.33**: implicit nullable params | 8.4 deprecated | `Smarty.class.php:1081` plus 12 more at require time and others at first render (`smarty_internal_data.php:193, 209, 223`, `smarty_internal_template.php:148`, `smarty_resource.php:177, 201`, …) | On `require` of Smarty **[tested-8.4]** (with opcache: first compile only, so flaky). | Upgrade. |

Without these fixes, on PHP 8.2 **every Smarty page** (site and panel), **every DB query**, **every add-on
submission** and **every persona preview** dies. On 8.4, additionally, every response is corrupted by the
`E_STRICT` notice.

### 8.3 Smarty upgrade recommendation

**Smarty 4.5.8** (bump the submodule to tag `v4.5.8`):
- Same `Smarty` class and `libs/Smarty.class.php` entry point.
- Phoebus's calls are all supported (`new Smarty`, `setCacheDir/setCompileDir/setConfigDir/addPluginsDir/setTemplateDir`,
  `assign`, `display('string:…')`, `classGenerateContent.php:53-80, 214-246`).
- Changelog: PHP 8.1 compat 4.1.0, 8.2 in 4.3.0, 8.3 in 4.4.0, **8.4 deprecations handled in 4.5.5**.
  Security fixes reach 4.5.7 and 4.5.8 (2026). `composer.json` requires `php ^7.1 || ^8.0`.
- Since 4.3.0, a PHP function used as a modifier raises `E_USER_DEPRECATED`, which is fatal under Phoebus's
  handler. 4.5.8 ships built-ins for `json_encode`, `substr`, `count`, `truncate`, `replace`, `lower`,
  `capitalize` (`libs/plugins/` listing).
- The only PHP-function modifier in the repo is `{'Y'|date}` (`base/skin/palemoon/site-template.xhtml:59`,
  `basilisk/site-template.xhtml:57`). Those templates are not reused; use `{$smarty.now|date_format:'%Y'}` or
  register `date`.

**Smarty 5.8.5** (long-term):
- Namespaced `\Smarty\Smarty`, meant to be loaded via the Composer autoloader.
- Requires `symfony/polyfill-mbstring`; `php ^7.2 || ^8.0`.
- The built-in modifier set includes `json_encode`, `substr`, `count` but **not** `date`.
- Needs a rewrite of the init in `classGenerateContent.php:53-80`.
- Reasonable when the skin is written from scratch anyway, but adds Composer to the deploy.

### 8.4 Required PHP extensions

Package names are Debian/Devuan `php8.x-*`.

| Extension | Package | Used at |
|---|---|---|
| mysqli (mysqlnd) | `php8.x-mysql` | `classDatabase.php:28-46`; SafeMySQL throughout |
| gd (PNG/JPEG/GIF) | `php8.x-gd` | `classPersona.php:182-247` |
| xml (expat) | `php8.x-xml` | `rdf_parser.php:83-89, 135` |
| simplexml | `php8.x-xml` | `classReadManifest.php:299` |
| zip (`zip://` wrapper, `ZipArchive`) | `php8.x-zip` | `fundamentals.php:596`; `classWriteManifest.php:515-533, 1008` |
| fileinfo (`mime_content_type`) | `php8.x-common` | `fundamentals.php:326`; `classWriteManifest.php:550` |
| mbstring | `php8.x-mbstring` | Smarty (required by 5.x, recommended for 4.x) |
| json, hash (sha256), random (`random_bytes`), pcre, ctype | core / `php8.x-common` | — |
| FPM | `php8.x-fpm` | — |

Not needed: session, curl, intl, openssl, pdo. `mail()` needs an MTA only if registration stays (it should not).

Devuan targets (inferred from their Debian bases; verify with `apt policy`):
- **Daedalus 5** (bookworm): PHP 8.2, MariaDB 10.11, nginx 1.22.
- **Excalibur 6** (trixie): PHP 8.4, MariaDB 11.8, nginx 1.26.

Both work once the §8.2 fixes and Smarty 4.5.8 are in. FULLTEXT on InnoDB is fine on both.

---

## 9. Skin contract

### 9.1 How a page is assembled

`classGenerateContent::addonSite()` (`classGenerateContent.php:92-250`):
1. Skin directory: `base/skin/<appkey>/` for the `site` component (`:39-45`); `<component>/skin/default/` for the
   panel. Content directory: `<component>/content/` (`:34`).
2. Reads the skin's `site-template.xhtml` and `site-stylesheet.css` (mandatory, `:103-114`).
3. Picks the content template by page type (`:118-198`). Unknown types are read from the **content** directory
   (`:194`); that is how `<app>-frontpage.xhtml` and the panel front pages load.
4. String-replaces `{%PAGE_CONTENT}` and `{%SITE_STYLESHEET}` (`:207-209`). The **CSS therefore goes through the
   Smarty compiler too.** With `auto_literal=true` (`Smarty.class.php:206`), a `{` followed by whitespace is
   literal, but `a{color:red}` is parsed as a Smarty tag and breaks compilation. Write CSS with a space after `{`
   or wrap it in `{literal}`. The CSS may use `{$BASE_PATH}`, `{if $APPLICATION_DEBUG}`
   (`palemoon/site-stylesheet.css:4, 110`).
5. Assigns globals (`:214-234`) and calls `display('string:'…)`.

A missing file gives "Content or template file could not be read or is missing" (`:200-202`).

### 9.2 Smarty variables available to every page

| Variable | Value | Assigned at |
|---|---|---|
| `APPLICATION_DEBUG` | debug mode | `:214` |
| `SITE_DOMAIN` | `https://addons.riverstonerelay.org` (scheme://domain) | `:215-217` |
| `PAGE_TITLE` | title passed by the caller | `:218` |
| `PAGE_PATH` | the filtered `?path` | `:219` |
| `BASE_PATH` | skin URL path, e.g. `/base/skin/jihad/` | `:220`, from `:46-47` |
| `PHOEBUS_VERSION` | `2.4.2` | `:221` |
| `SITE_NAME` | the app `name`; in debug, "Phoebus Development …" plus git branch and commit (`addonSite.php:52-64`) | `:222` |
| `SEARCH_TERMS` | `?terms` | `:223` |
| `APPLICATION_ID` | Jihad GUID | `:224` |
| `VALID_CLIENT`, `VALID_VERSION` | §1.4c | `:225-226` |
| `PAGE_TYPE` | page type | `:227` |
| `PAGE_DATA` | page data | `:228` |
| `EXTRA_DATA` | extra data | `:229` |
| `USER_LEVEL` | panel only | `:231-234` |

### 9.3 Per page type (site component, `base/skin/jihad/`)

| File (required) | PAGE_TYPE(s) | PAGE_DATA / EXTRA_DATA | Fields used by the reference templates |
|---|---|---|---|
| `site-template.xhtml` | all | — | Must contain `{%SITE_STYLESHEET}` and `{%PAGE_CONTENT}`. Uses `SITE_NAME`, `PAGE_TITLE`, `BASE_PATH`, `SITE_DOMAIN`, `PHOEBUS_VERSION`. Nav links `/extensions/ /themes/ /personas/ /language-packs/ /search-plugins/ /panel/`; search form `GET /search/?terms=`. |
| `site-stylesheet.css` | all | — | Inlined; Smarty-compiled (see 9.1). |
| `addon-page.xhtml` | `addon-page`, `addon-releases` | full add-on manifest (§5.1). The palemoon template closes the content div and opens a sidebar (`:107-108`), so the wrapper div IDs are a contract between the two files. | `id slug type name creator content license licenseName licenseDefault hasPreview preview icon homepageURL supportURL supportEmail repository releaseXPI xpinstall[*].{version,date,targetApplication[APPLICATION_ID].{minVersion,maxVersion}}`. Install link `/?component=download&id={id}&version={version}`. |
| `addon-license.xhtml` | `addon-license` | manifest; `licenseText` already `nl2br`'d (`addonSite.php:282-284`) | `icon name creator licenseText license`. |
| `addon-category.xhtml` | `cat-all-extensions`, `cat-extensions`, `cat-themes`, `search` | list of `{id slug type name description url reviewed active icon hasIcon preview hasPreview baseURL basePath}`; `EXTRA_DATA` = category map for `cat-*extensions`; `PAGE_DATA` null on "No search results" (`addonSite.php:89-91`) | `type` (`external` → target=_blank), `url`, `icon`, `preview` (themes), `name`, `description|truncate`; search form with `SEARCH_TERMS`. |
| `extension-category.xhtml` | `cat-extension-category` | `EXTENSION_CATEGORY_SLUGS` (slug → label; labels contain `&amp;`) | Links `/extensions/<slug>/`, `/extensions/?all=1`, icons `{$BASE_PATH}../shared/<slug>.png`. Needed only with `extensions-cat`. |
| `persona-category.xhtml` | `cat-personas` | list of processed persona rows (§4.3) | `setTheme()` + `InstallBrowserTheme` contract; `data-browsertheme` JSON. Fix the escaping and `substr:5` (§4.3). |
| `langpack-category.xhtml` | `cat-language-packs` | list | `url icon name`. Needed only with `language-packs`. |
| `searchplugin-category.xhtml` | `cat-search-plugins` | map name → `{type id slug name icon}` | `window.external.AddSearchProvider('{$SITE_DOMAIN}/?component=download&id=' + id)`. Needed only with `search-plugins`. |
| `base/content/jihad-frontpage.xhtml` (content dir) | `jihad-frontpage.xhtml` | null | free HTML + Smarty. |
| static assets | — | — | favicon, logo, fonts (own or OFL), via `{$BASE_PATH}`. |

### 9.4 Licence status of existing skin material

README: operational code MPL-2.0; `components/*/skin/` owned by their owners (Moonchild); `components/*/skin/default/`
MPL. Site skins actually live in `base/skin/` (the base component), so apply the same rule there.

| Material | Status | Reuse? |
|---|---|---|
| `base/skin/default/{template-header.xhtml, template-footer.xhtml, logo.png}`: the error/"special" page frame used by **every** `gfError` page (`index.php:154-210`) | MPL | Yes. Restyle; fix the logo path (`template-header.xhtml:128`). The logo is the "phoebus" wordmark. |
| `components/panel/skin/default/*` (13 templates, CSS, `logo.png` "phoebus panel", `header_tab.gif`) | MPL | Yes. |
| `components/panel/skin/shared/*.png` (Silk icons) | CC BY 2.5 © Mark James; credited at `components/panel/skin/default/site-template.xhtml:46` | Yes, keep the credit. |
| `components/panel/content/*.xhtml` | MPL (content) | Yes, but `panel-frontpage.xhtml:4-6` embeds Pale Moon, Basilisk and Ambassador logos: replace. |
| `components/discover/content/*.xhtml` | MPL (content); generic text | Rewrite (mentions Flash). |
| `components/discover/skin/*.png` (palemoon, basilisk, ambassador, discover-logo, caa-extension, modern-firefox, sad-firefox) | owner, Moonchild or third-party branding | **No.** Need `jihad.png` and a new `discover-logo.png`. |
| `base/skin/palemoon/*` (incl. **Museo300/500** `.eot`/`.woff`, `wordmark-*.png`, `sub-banner2.jpg`), `base/skin/basilisk/*`, `base/skin/epyrus/*` | Moonchild ("Site design and Branding © Moonchild", `palemoon/site-template.xhtml:59`) | **No.** Structure may guide; do not copy. |
| `base/skin/shared/*.png` (15 category icons, 64×64, not under `default/`) | Provenance unknown | Treat as not cleared; replace. |
| Root `favicon.ico` (5 icons) | Unknown | Replace. |
| `base/content/{palemoon,basilisk,epyrus}-frontpage.xhtml` | App text | Write new. |

**Minimum set for skin "jihad":** the 9 files under `base/skin/jihad/` in 9.3 (site-template, site-stylesheet,
addon-page, addon-license, addon-category, extension-category, persona-category, langpack-category,
searchplugin-category), plus:
- `base/content/jihad-frontpage.xhtml`;
- `components/discover/skin/jihad.png` and a new `discover-logo.png`;
- replacements for `base/skin/shared/*.png` (or change `extension-category.xhtml` and `discoverPane.php:84` to
  point elsewhere);
- a new root `favicon.ico`;
- edits to `components/panel/content/panel-frontpage.xhtml` and `base/skin/default/template-header.xhtml`.

Even if a feature is off, keep its template file so a mis-set flag fails soft.

---

## 10. nginx

### 10.1 What the code expects

- No `.htaccess` exists. `X-Accel-Redirect` (`addonDownload.php:52`) and production's `Server: NGinX` **[live]**
  mean upstream runs nginx with private rules.
- Pretty URLs go to `index.php` with `component=site&path=<uri>` plus the original args. `/panel/` and
  `/special/` are then re-routed by `REQUEST_URI` prefix (`index.php:487-494`).
- `/` with or without `?component=…` goes to `index.php` unchanged.
- Slashless directory paths are 302-redirected to the trailing-slash form; PHP would 404 them **[live]**.
- `/datastore/addons/**` and `/datastore/personas/**` are served statically (icons, previews, persona images).
  XPIs via `X-Accel-Redirect`.
- Static skin assets under `/base/skin/`, `/components/panel/skin/`, `/components/discover/skin/`.
- `DOCUMENT_ROOT` = the checkout (`index.php:17`). `SERVER_NAME` = the configured domain
  (`index.php:411-427`). `HTTPS=on` (`index.php:343`; panel requires it, `phoebusPanel.php:72`). CWD = the
  script dir (relative `./datastore`, `./fundamentals.php`).
- **Do not** put `$uri` into `QUERY_STRING`. An encoded `%26` in the path becomes `&component=special&path=…`
  after PHP parses it. Pass the path through a separate FastCGI param and patch `index.php:355` to
  `'qPath' => gfSuperVar('server','PHOEBUS_PATH') ?? gfSuperVar('get','path')`. That needs `'_SERVER'` handling,
  which `gfSuperVar` already has (`fundamentals.php:294`).

### 10.2 Suggested server block

For Devuan + OpenRC; adjust `php8.4` → `php8.2` on Daedalus.

```nginx
# /etc/nginx/conf.d/addons.riverstonerelay.org.conf
limit_req_zone $binary_remote_addr zone=phoebus_panel:10m rate=20r/m;

upstream phoebus_fpm { server unix:/run/php/php8.4-fpm-phoebus.sock; }

server {
    listen 80; listen [::]:80;
    server_name addons.riverstonerelay.org;
    location ^~ /.well-known/acme-challenge/ { root /var/www/acme; }
    location / { return 301 https://addons.riverstonerelay.org$request_uri; }
}

server {
    listen 443 ssl; listen [::]:443 ssl;
    http2 on;                                     # nginx >= 1.25.1 (Excalibur); on 1.22 use "listen 443 ssl http2;"
    server_name addons.riverstonerelay.org;

    ssl_certificate     /etc/letsencrypt/live/addons.riverstonerelay.org/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/addons.riverstonerelay.org/privkey.pem;

    root  /srv/riverstone-addons/www;             # Phoebus checkout = ROOT_PATH
    index index.php;
    charset utf-8;
    client_max_body_size 32m;                     # keep >= PHP upload_max_filesize/post_max_size
    server_tokens off;

    # NOTE: add_header in a location REPLACES these; repeat them where you add others.
    add_header Strict-Transport-Security "max-age=31536000" always;
    add_header X-Content-Type-Options    "nosniff" always;
    add_header Referrer-Policy           "same-origin" always;
    add_header X-Frame-Options           "SAMEORIGIN" always;

    # ── never served ────────────────────────────────────────────────────────────────────────────
    location ~ /\.            { return 404; }     # .git .obj .offline .disablePanel datastore/.phoebus
    location ^~ /libraries/   { return 404; }     # includes smarty/demo/*.php
    location ^~ /modules/     { return 404; }
    location ^~ /databases/   { return 404; }
    location ^~ /base/tests/  { return 404; }
    location ^~ /special/     { return 404; }     # and remove 'special' from COMPONENTS
    location ^~ /datastore/searchplugins/ { return 404; }   # served through ?component=download
    location ~ \.(php|xhtml|rdf|tpl|md|txt)$ { return 404; }   # only /index.php below runs PHP

    # ── static files Phoebus links to ───────────────────────────────────────────────────────────
    # ^~ stops regex matching, so the .xhtml/.php deny above must be repeated inside these locations
    location ^~ /base/skin/                { location ~ \.(php|xhtml)$ { return 404; } try_files $uri =404; expires 7d; }
    location ^~ /components/panel/skin/    { location ~ \.(php|xhtml)$ { return 404; } try_files $uri =404; expires 7d; }
    location ^~ /components/discover/skin/ { location ~ \.(php|xhtml)$ { return 404; } try_files $uri =404; expires 7d; }
    location ^~ /datastore/personas/       { try_files $uri =404; expires 1h; }
    location ^~ /datastore/addons/ {
        location ~ \.xpi$ {                       # only via X-Accel-Redirect from ?component=download
            internal;
            types { application/x-xpinstall xpi; }
        }
        try_files $uri =404; expires 1h;
    }
    location = /favicon.ico { try_files $uri =404; access_log off; }
    location = /robots.txt  { try_files $uri =404; access_log off; }

    # ── PHP: front page and ?component=… APIs ───────────────────────────────────────────────────
    location = / {
        if ($arg_component ~* "^(special|panel)$") { return 404; }  # panel only via /panel/
        include fastcgi_params;
        fastcgi_param SCRIPT_FILENAME $document_root/index.php;
        fastcgi_param SCRIPT_NAME     /index.php;
        fastcgi_param DOCUMENT_ROOT   $document_root;
        fastcgi_param SERVER_NAME     addons.riverstonerelay.org;   # app selection key; never $host
        fastcgi_param HTTPS           on;
        fastcgi_param SCHEME          https;
        fastcgi_param HTTP_X_FORWARDED_FOR "";     # no proxy in front: stop XFF log spoofing
        fastcgi_pass  phoebus_fpm;
    }
    location = /index.php { return 301 /$is_args$args; }

    # ── panel ───────────────────────────────────────────────────────────────────────────────────
    location ^~ /panel/ {
        # allow 203.0.113.10; deny all;           # owner IP(s), or use ssl_verify_client (mTLS)
        limit_req zone=phoebus_panel burst=30 nodelay;
        client_max_body_size 64m;
        rewrite ^(/panel/[^.]*[^/.])$ $1/ redirect;
        include fastcgi_params;
        fastcgi_param SCRIPT_FILENAME $document_root/index.php;
        fastcgi_param SCRIPT_NAME     /index.php;
        fastcgi_param DOCUMENT_ROOT   $document_root;
        fastcgi_param SERVER_NAME     addons.riverstonerelay.org;
        fastcgi_param HTTPS           on;
        fastcgi_param SCHEME          https;
        fastcgi_param HTTP_X_FORWARDED_FOR "";
        fastcgi_param PHOEBUS_PATH    $uri;        # see 10.1; index.php patched to read it
        fastcgi_param QUERY_STRING    component=panel&$args;
        fastcgi_pass  phoebus_fpm;
    }

    # ── pretty site URLs ────────────────────────────────────────────────────────────────────────
    location / {
        rewrite ^(/[^.]*[^/.])$ $1/ redirect;     # /personas -> /personas/ (matches production)
        include fastcgi_params;
        fastcgi_param SCRIPT_FILENAME $document_root/index.php;
        fastcgi_param SCRIPT_NAME     /index.php;
        fastcgi_param DOCUMENT_ROOT   $document_root;
        fastcgi_param SERVER_NAME     addons.riverstonerelay.org;
        fastcgi_param HTTPS           on;
        fastcgi_param SCHEME          https;
        fastcgi_param HTTP_X_FORWARDED_FOR "";
        fastcgi_param PHOEBUS_PATH    $uri;
        fastcgi_param QUERY_STRING    component=site&$args;
        fastcgi_pass  phoebus_fpm;
    }
}
```

**Unpatched fallback.** If `index.php` is not patched to read `PHOEBUS_PATH`, use
`fastcgi_param QUERY_STRING component=site&path=$uri&$args;` and accept the `%26` injection risk. It is only
safe once `special` is removed from `COMPONENTS` and `component=panel` is tied to a `/panel/` `REQUEST_URI` in PHP.

### 10.3 PHP-FPM pool and OpenRC

`/etc/php/8.4/fpm/pool.d/phoebus.conf`:
```ini
[phoebus]
user = www-data
group = www-data
listen = /run/php/php8.4-fpm-phoebus.sock
listen.owner = www-data
listen.group = www-data
pm = ondemand
pm.max_children = 8
php_admin_flag[display_errors] = off          ; cannot be overridden by index.php:10's ini_set
php_admin_value[error_log] = /var/log/php/phoebus.log
php_admin_value[upload_max_filesize] = 32M
php_admin_value[post_max_size] = 33M
php_admin_value[open_basedir] = /srv/riverstone-addons:/etc/riverstone-addons:/tmp
php_admin_value[mysqli.default_socket] = /run/mysqld/mysqld.sock
php_admin_value[date.timezone] = UTC
```

Writable by `www-data`:
- `/srv/riverstone-addons/www/.obj/` (Smarty compile dirs, bulk uploads), or a symlink target outside;
- `datastore/addons/`;
- `datastore/personas/*/` (preview generation; or pre-generate previews and keep it read-only).

The credentials file is mode 0640 root:www-data.

OpenRC:
```
rc-update add php8.4-fpm default
rc-update add nginx default
rc-update add mariadb default
rc-service php8.4-fpm restart
nginx -t && rc-service nginx reload
```

No cron jobs ship with Phoebus. Add your own for:
- DB plus datastore backups;
- `find .obj/bulk-upload -mtime +1 -delete`;
- certbot renew.

---

## 11. Other things an implementer should know

1. **Personas have no admin UI or importer** (§4.2). Budget for one.
2. **Personas lack `version`**, so client updates can never apply (§4.6). The site's `/personas/` list has no
   paging and generates previews synchronously on first view (`classPersona.php:62-69, 162-165`).
3. **Search only covers `tags`** (`classReadManifest.php:178, 190`). An add-on without tags is unfindable on
   `/search/` and in about:addons search. It also needs a FULLTEXT index, and InnoDB's minimum token size and
   stopwords apply.
4. **The GET filter is ASCII-only** (`fundamentals.php:102`): non-English search terms are mangled.
5. **`gfSuperVar('var')` turns the string `'none'` into null** (`fundamentals.php:284`). A field whose value is
   literally "none" disappears.
6. **Requests without a User-Agent, or with curl/wget/git UAs, get an error page** (`index.php:245-267`),
   including AUS and the new feed. Health checks need `-A`.
7. **Hard-coded Moonchild material**:
   - Pale Moon-only langpacks (§5.3);
   - palemoon.org mail and links (§2 #16, #35);
   - archive.palemoon.org dictionaries redirect (`addonSite.php:176`);
   - addons-legacy.palemoon.org (`phoebusPanel.php:69`);
   - JustOff exclusion (`classReadManifest.php:229-244`);
   - CAA button (`discoverPane.php:98`);
   - Moonchild IDs in the restricted list (`classWriteManifest.php:731-744`);
   - personal bans (`classAccount.php:41-43, 95-101`; `classWriteManifest.php:743`);
   - profane user-facing strings (`developer.php:108, 141`; `administration.php:145`; `classAccount.php:232, 277`).
8. **Dead or legacy references**: `interlink` (`addonSite.php:72`, `discoverPane.php:28`,
   `classWriteManifest.php:383`); `getDictionaries()` (`classReadManifest.php:320-328`); `template-announcement`;
   `panel-user-personas`; `$key = null` at the end of templates.
9. **Downloads need nginx**: the XPI body comes from `X-Accel-Redirect`. Under any other server, or a missing
   `location`, the client gets an empty 200 with a non-zero `Content-Length` (`addonDownload.php:50-52`).
10. **Inactive or unreviewed XPIs are public by direct URL** unless `*.xpi` is `internal` (§10), plus the
    `panel=1` hole (§7 P2-2). Active-but-unreviewed add-ons are reachable at `/addon/<slug>/` and via AUS
    (`classReadManifest.php:73, 83`).
11. **Generated URLs mix http and https**: `classPersona.php:148`, `classGenerateContent.php:372-373`,
    `discoverPane.php:45` emit `http://`. Fix all three, and the persona template's `|substr:5`, together.
12. **Panel UI assumptions**: it needs JavaScript (content is hidden until a script runs,
    `components/panel/skin/default/site-template.xhtml:32-41`), uses fixed 1200 px layouts
    (`base/skin/default/template-header.xhtml:20-22`; panel CSS), and its Basic-auth logout trick depends on
    XHR-with-credentials-in-URL behaviour.
13. **Mail is the only external side effect** (`classAccount.php:480-507`). It uses `mail()` with a hard-coded
    envelope sender; the last upstream commits (`614dfb9`) were about bounce fixes. Remove with registration.
    There are no outbound HTTP calls, no telemetry and no update pings.
14. **The `amo` and `log` tables must exist** even though Jihad will never fill `amo` (§3). Otherwise the first
    submission or panel action throws.
15. **Inserting `client` before `addon`** (`classWriteManifest.php:135-136, 354-355`) rules out a foreign key
    unless the order is swapped. `deleteAddon` removes both by hand (`:465-467`).
16. **Error pages for the whole site** (`gfGenContent`, `index.php:154-210`) use `base/skin/default/`, not the
    app skin. They show "<title> - Phoebus 2.4.2" and a 1200 px frame. Restyle them, or the site looks
    half-branded on any 404 in debug mode or any error.
17. **AUS ignores client compatibility** and always offers the latest release (§5.2). It can offer an update the
    client then rejects as incompatible.
18. **Version check on Jihad**: with `oldVersion='1.0'`, Jihad 1.0 counts as *old*, because `compare() <= 0`
    means old (`index.php:325`). Use something lower, and the `uaToken` patch (§1.4).
19. **Discover pane**: a non-Pale Moon app gets one "Add-ons Site" button over `http://` (`discoverPane.php:45,
    104-106`). Add a Jihad case if the pane is used. UXP locks discover navigation to that host.

---

## Appendix A: reproduction

The harness and the sampled images were session scratch and are not kept; what they did:


- `harness/run.php`: librdf parse, the SafeMySQL `preg_split`, the version comparator, Smarty require
  and render of every palemoon site template, all under a recording error handler (PHP 8.4.22).
- `harness/boot.php <host> <component> <path> [k=v…]`: boots the real `index.php` (only use with
  `discover` or `special`, which touch neither the DB nor Smarty's compile dir).
- `pm/`: production persona images (header 3000×200, footer 3000×100, icon 32×32, preview 240×60,
  default `theme.png` 32×32).

## Appendix B: patch checklist for "jihad" (in order)

1. `fundamentals.php:39` delete `E_STRICT`. Make `gfErrorHandler` log deprecations and notices (§8.1).
2. Smarty → v4.5.8. SafeMySQL `preg_split` limit `-1`. librdf callable handlers. `mysqli_report(OFF)` and drop
   the `mysqli_connect_errno` argument. Fix `classPersona` (int casts, `getimagesize` check). Delete the JustOff
   block.
3. Config file outside the root (§2). Credentials out of `datastore/.phoebus/`. Remove the `debugOff` DB switch.
   `utf8mb4`.
4. App entry plus ID plus `uaToken` (§1.4). Delete Moonchild app entries and skins from the deployment.
5. Remove `special` from `COMPONENTS`. Remove registration and verification. Tie `component=panel` to a `/panel/`
   `REQUEST_URI`. Read the path from `PHOEBUS_PATH`.
6. CSRF tokens. `setEscapeHtml(true)` plus `nofilter` audit. Fix `findUserAddon` SQL. Gate the `panel=1` download.
7. Persona: https URLs, a `version` column, a safe JSON attribute, an importer or panel task, the feed component.
8. Schema (§3), owner user, `amo`/`log` tables, nginx (§10), FPM pool, OpenRC services.
9. New skin `base/skin/jihad/` (§9), front page, discover image, error-frame restyle.
