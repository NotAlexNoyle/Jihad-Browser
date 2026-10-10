# Old Firefox Persona archives: history, what survives, licences, and theme-format timeline

Research date 2026-10-10. Legend: **[V]** = verified (I read the source, ran the query or inspected the file myself);
**[I]** = inferred (reasoned from verified facts, or taken from a search-result summary I could not open).
Live probes (HTTP status, API counts) are as of 2026-10-10.

---

## 0. Bottom line for Jihad

1. **AMO is the archive.** About 441,000 of AMO's 515,225 Firefox static themes have a `created` date before
   2018-09-20, when AMO started taking static-theme submissions. These are the migrated Personas/LWTs. The 2019 migration
   kept each LWT's `created` date. [V counts / I attribution] Each one still ships its **original header
   image byte-for-byte** in its XPI, with the original colours. The **footer was dropped** in the migration. [V]
2. **Licences.** Every AMO version carries a licence, and the search API returns it. By weighted sampling
   (§4.4), about **48% of all static themes are CC BY-SA 3.0** and about **32% All Rights Reserved**. Almost all
   of the BY-SA ones are getpersonas-era themes (2009 to March 2013). There is **no CC0** option for themes. [V sample / I extrapolation]
   **Caveat:** the BY-SA labels on getpersonas-era themes may not match what designers agreed to. Getpersonas
   offered "CC BY-NC-SA or restricted", and at least one ARR theme became BY-SA during the 2013 import (§4.1–4.3). [V sources]
3. **The archives most people point to don't hold Personas.** The Classic Add-ons Archive (ca-archive) is
   extensions only. The Internet Archive "Firefox Legacy Collector Dump" is extensions only. I found no
   persona dump on archive.org or GitHub. [V]
   The **Wayback Machine** does hold many old LWT header and footer image URLs on `addons.cdn.mozilla.net`, but only
   partially, and keyed by *old* LWT ids. [V] **addons.thunderbird.net** still serves some orphaned LWT image files. [V]
4. **Old endpoints are dead.** `getpersonas.com` redirects (301) to `addons.mozilla.org/themes/`. The `addons.cdn.mozilla.net` and
   `getpersonas-cdn.mozilla.net` hosts have no DNS. The versioncheck update-check endpoint returns 403. Old LWT ids give 404 in the API. [V]
5. **Recommended approach [I].** The index stores AMO metadata: slug, guid, name, author, licence and preview URL.
   It links to the AMO XPI, and the browser converts the XPI on the device. This uses the static-theme converter
   Jihad already has. Jihad would host extracted header images only for themes whose licence allows
   redistribution: BY, BY-SA, and BY-NC* for a non-commercial gallery. ND variants are a judgement call.
   Exclude Firefox, Mozilla and third-party brand art regardless of the licence label.

---

## 1. History

| Date | Event | Source | |
|---|---|---|---|
| 2007-12-14 | Mozilla Labs announces **Personas for Firefox**, an extension adding "lightweight theming". It used "header" and "footer" images loaded "via URLs to remote resources", applied without restart. | https://blog.mozilla.org/labs/2007/12/personas-for-firefox/ | V |
| 2008-03 | Major update to the Personas extension | https://blog.mozilla.org/labs/2008/03/major-update-to-personas-for-firefox/ (title only) | I |
| 2008 | **getpersonas.com** started as a Labs project. "Personas became stable and was released to the public in December 2008." Backend: PHP and SQLite, with a cron job generating a static HTML/JSON/image tree every 2 h. Source code was at `svn.mozilla.org/projects/getpersonas.com`. | https://wiki.mozilla.org/Labs/Personas ; https://micropipes.com/2011/04/12/getpersonas-com-where-its-from-where-its-going/ | V |
| 2009-03 | Licensing step added to the getpersonas submit flow. Bug 482576 says "user will choose one of two choices". The draft copy offers CC BY-NC-SA 3.0 US with free hosting, or "retain full control", which carried a fee after a 30-day trial. | https://bugzilla.mozilla.org/show_bug.cgi?id=482576 (attachment 366814, "License-Page-Copy_v5_dr.doc", 2009-03-11) | V (draft text) |
| 2009-09-22 | Bug 511771 fixed: "Web content needs a way to install lightweight themes". This is the `InstallBrowserTheme` / `PreviewBrowserTheme` DOM-event mechanism. | https://bugzilla.mozilla.org/show_bug.cgi?id=511771 | V |
| 2010-01-21 | **Firefox 3.6** ships built-in support for "a new type of theme called Personas" (lightweight themes). The add-on is renamed **Personas Plus**. | http://website-archive.mozilla.org/www.mozilla.org/firefox_releasenotes/en-US/firefox/3.6/releasenotes/ ; https://blog.mozilla.org/addons/2010/03/03/the-future-of-personas/ | V |
| 2010-03-03 | "The Future of Personas": plan to migrate getpersonas.com to AMO (ratings, profiles, stats, 30+ languages). | https://blog.mozilla.org/addons/2010/03/03/the-future-of-personas/ | V |
| 2010–2011 | AMO listing pages mirror getpersonas through a "precarious cron job synchronizing the getpersonas.com and AMO databases". In 2011 getpersonas had **1.25 M registered users, 400,000 personas, and 500 k hits/day**. Full migration was planned for Q3 2011. | micropipes 2011 (above) | V |
| early 2012 | Personas renamed **"Themes"** (also called "Background Themes") to avoid confusion with Mozilla Persona ID. Old-style themes became "Complete Themes". | https://blog.mozilla.org/addons/personas-are-now-firefox-themes/ | V |
| 2013-03-28 → 2013-04-11 | getpersonas.com goes **read-only** on 3/28. Favourites and created themes moved to AMO. The move was declared complete on 2013-04-11, and traffic was redirected to `addons.mozilla.org/firefox/themes/`. About 1.5 M users were imported in one night. | https://blog.mozilla.org/addons/2013/02/28/getpersonas-com-migration-update/ ; https://blog.mozilla.org/addons/2013/03/27/getpersonas-com-is-becoming-read-only/ ; https://blog.mozilla.org/addons/2013/04/11/background-themes-have-moved-to-amo/ ; https://micropipes.com/2013/04/11/the-sunset-of-getpersonas-com/ | V |
| 2014 (Australis, Fx 29) | Footer stops displaying in Firefox: "ever since Australis landed on trunk, Nightly no longer displays the lwtheme footer anymore". | https://bugzilla.mozilla.org/show_bug.cgi?id=898988 (comment 2, 2014-01-04) | V |
| 2017-02-24 | Mozilla says Complete Themes end with Fx 57, since only 60 of 500 were compatible and 0.089% of users used one. LWTs (over 400,000) continue. A new manifest-based theme API is coming. | https://blog.mozilla.org/addons/2017/02/24/improving-themes-in-firefox/ | V |
| 2018-03-08 | Static themes (manifest + images) to be supported on AMO "early this year". | https://blog.mozilla.org/addons/2018/03/08/theme-api-update/ | V |
| 2018-03 → 2018-07 | addons-server work: migration script (mozilla/addons#5361, closed 2018-05-15), signing (#8386), last_updated copy (#8802), and the update service serving the static XPI for migrated LWTs (#5412). | https://github.com/mozilla/addons/issues/5361 , /5412 | V |
| 2018-06-09 | Bug 1430276 (Fx 62): "Allow updating a lightweight theme to a static WebExtensions theme". Installed LWTs could then auto-update into their migrated static theme. | https://bugzilla.mozilla.org/show_bug.cgi?id=1430276 | V |
| 2018-09-20 | "The future of themes is here!" AMO takes static-theme submissions. "Over 400 thousand" LWTs are to be transformed "later this year". | https://blog.mozilla.org/addons/2018/09/20/future-themes-here/ | V |
| 2018-10 | Test migrations on addons-dev: 995, then batches of 500. The dev index showed **349,970 LWT** before the first run. | https://github.com/mozilla/addons/issues/6079 | V (dev environment) |
| 2019-01-14/15 | Collection membership moved to the migrated themes (#6302 / addons-server #10381). | https://github.com/mozilla/addons/issues/6302 | V |
| **2019-02-01** | "**Migration is complete.**" (eviljeff). QA verified on production on 2019-02-06. | https://github.com/mozilla/addons/issues/6079 | V |
| 2019-03-06/07 | "add command to delete LWT". The command deletes `Addon.objects.filter(type=amo.ADDON_PERSONA)`, i.e. every remaining LWT, which by then were only the non-public ones. | https://github.com/mozilla/addons/issues/6460 ; code: `process_addons` `'delete_lwt'` task | V (command exists) / I (run on prod) |
| 2019-02-07 → 2019-04-18 | AMO **repacks static themes that use deprecated keys** (accentcolor/textcolor/headerURL). These keys were removed in Fx 69/70. Themes it could not repack got max-version 68. | https://github.com/mozilla/addons/issues/6385 , https://github.com/mozilla/addons/issues/6386 | V |
| 2019-03-28 → 2019-03-31 (Fx 68) | Bug 1525511 "Get rid of most extraneous LWT gunk" (Part 1: "Remove LWT install gunk", i.e. the web install). Bug 1525762 "Convert built-in LWTs to static themes". | https://bugzilla.mozilla.org/show_bug.cgi?id=1525511 ; https://bugzilla.mozilla.org/show_bug.cgi?id=1525762 ; https://wiki.mozilla.org/Firefox/Meeting/9-Apr-2019 | V |

### What the 2019 migration did, per item (from the code, addons-server tag `2019.03.14`) [V]
Files: `src/olympia/addons/tasks.py` (`add_static_theme_from_lwt`, `migrate_lwts_to_static_themes`) and
`src/olympia/addons/utils.py` (`build_static_theme_xpi_from_lwt`). Raw URLs:
https://raw.githubusercontent.com/mozilla/addons-server/2019.03.14/src/olympia/addons/tasks.py ,
https://raw.githubusercontent.com/mozilla/addons-server/2019.03.14/src/olympia/addons/utils.py ,
https://raw.githubusercontent.com/mozilla/addons-server/2019.03.14/src/olympia/addons/management/commands/process_addons.py

- **Selected:** only `type=PERSONA, status=PUBLIC`. All remaining LWTs were later deleted by `delete_lwt`.
- **XPI built:** `manifest_version 2`, `name`, `version "1.0"`, optional `description`. The theme block is
  `{images:{headerURL:<original header filename>}, colors:{accentcolor:"#"+accent or default, textcolor:"#"+(text or "000")}}`.
  The zip holds the manifest plus the original header file. **No footer.**
- **Copied:** licence (`PERSONA_LICENSES_IDS.get(persona.license, LICENSE_COPYRIGHT_AR)`, so a missing licence becomes "All
  Rights Reserved"), category, tags, ratings, collection membership, ADU, and created/modified/last_updated dates. The XPI is signed and made public.
- The new add-on gets a **new id and a new GUID**. It took the old **slug** after the LWT was deleted. A `MigratedLWT` row kept
  `getpersonas_id`, but that mapping is not exposed by the public API. **Old LWT ids give 404** in `/api/v5/addons/addon/<oldid>/`.
- No author: the theme is assigned to an anonymised default user (`MIGRATED_LWT_DEFAULT_OWNER_EMAIL`).

Example [V]: *Blade Runner – Cityscape*. The old LWT id was 482832 (from a 2019 forum post); the new AMO id is 1351313, slug
`blade-runner-cityscape`, created 2014-01-31.
`1.0.xpi` (2019-01-26) holds `{"images":{"headerURL":"header.png"},"colors":{"accentcolor":"#041042","textcolor":"#f5f5f5"}}`
and a `header.png` dated 2014-01-31 (513,337 bytes, 3000×200).
`2.0.xpi` (2019-05-13, the AMO repack) holds the **same header.png** with `theme_frame`/`frame`/`tab_background_text`.
Both versions are still downloadable. https://addons.mozilla.org/api/v5/addons/addon/blade-runner-cityscape/versions/

## 2. URLs and APIs that existed, with status on 2026-10-10

| Endpoint | Format | Status now |
|---|---|---|
| `http://www.getpersonas.com/static/index_1.json` (menu feed: `categories/featured/popular/recent/total`). Per-persona `.../static/<a>/<b>/<id>/index_1.json`. Favourites at `/en-US/gallery/All/Favorites?json=1`. Documented at https://wiki.mozilla.org/Personas/Add-On_Endpoints [V] | JSON. Each persona has `id, name, accentcolor, textcolor, header, footer, category, description, author, username, detailURL, headerURL, footerURL, previewURL, iconURL, updateURL, dataurl`. Images were on `getpersonas-cdn.mozilla.net/static/...`. **No licence field.** [V, Wayback capture 2010-04-04 with 193 personas] | Domain 301s to `addons.mozilla.org/themes/`. Wayback captured index_1.json on 2009-06-12 and 2010-04-04 [V]. `getpersonas-cdn.mozilla.net` has no DNS [V]. Wayback holds about 9,100 distinct URLs under it [V] |
| `https://www.getpersonas.com/en-US/update_check/<id>` | LWT JSON | dead [I] |
| AMO 2013–2018 gallery `https://addons.mozilla.org/<locale>/firefox/themes/<category>` with a `data-browsertheme='{…}'` attribute per tile, installed by firing `InstallBrowserTheme` | LWT JSON (`id,name,headerURL,footerURL,accentcolor,textcolor,iconURL,previewURL,author,updateURL,version,category,…`). Images on `https://addons.cdn.mozilla.net/user-media/addons/<lwtid>/…?<ts>` | Now the static-theme SPA. Wayback capture https://web.archive.org/web/20170913051251/https://addons.mozilla.org/en-Gb/firefox/themes/nature has 48 tiles with data-browsertheme [V] |
| `https://versioncheck.addons.mozilla.org/<locale>/themes/update-check/<id>` (`services/theme_update.py`). Returns LWT JSON, and served the static XPI for migrated themes from 2018 | JSON | **403** [V] |
| AMO legacy XML API `/api/<ver>/addon/<id>`, `/api/<ver>/search/...`, `/api/<ver>/list/...` (`src/olympia/legacy_api`, e.g. `/api/1.5/`). Personas were type 9 | XML | `/en-US/firefox/api/1.5/addon/…` now returns the frontend HTML [V] |
| `addons.cdn.mozilla.net/user-media/addons/<id>/{header,footer}.(png|jpg)` | images | **No DNS** [V]. Wayback CDX for this prefix has 1,455 index pages. The first 200,000 URL rows (ids starting "1"…"127437") include 60,713 header/footer-type image URLs for 24,284 distinct old ids [V] |
| `addons.thunderbird.net/user-media/addons/<id>/…` (ATN forked AMO before the migration) | images | **Still served** for old LWT images, e.g. `/171553/StylusPersonaOrange.png` → 200, 242 KB, and `/46852/NSH.jpg` → 200 [V]. The listing pages are gone (`/thunderbird/addon/171553/` → 404) [V]. ATN `/themes/` pages now 301 to `static-themes/`; the SeaMonkey one rendered "No results found" [V] |
| **AMO API v5 (current)** `https://addons.mozilla.org/api/v5/addons/search/?type=statictheme&app=firefox` | JSON | Live. Count **515,225** (android: 0) [V] |

## 3. AMO as the archive: counts [V, live API on 2026-10-10]

| Query (`type=statictheme&app=firefox`) | count |
|---|---|
| all | 515,225 |
| `created__lt=2013-04-01` (getpersonas era) | 325,536 |
| `created__lt=2018-01-01` | 433,162 |
| `created__lt=2018-09-20` (before static submissions opened) | 441,220 |
| `created__gte=2018-09-20` | 74,005 |
| `created__gte=2020-01-01` | 60,806 |
| `created__gte=2024-01-01` | 23,119 |

Inference [I]: about 433–441 k public static themes are migrated LWTs. Compare the "400 thousand" in 2017–2018.
Pending, rejected or disabled LWTs were not migrated and were deleted.

## 4. Licences

### 4.1 getpersonas.com (2008–2013)
- The 2009 FAQ says designers keep ownership: "Do I still retain ownership over Persona artwork …? Yes". Header and footer ≤ 300 KB.
  Content rules: original or authorised art, PG-rated. https://web.archive.org/web/20091001135920/http://www.getpersonas.com/en-US/faq [V]
- The licensing step was a **binary choice**. The draft copy (bug 482576, 2009) reads: CC **BY-NC-SA 3.0 US** with free hosting, versus
  restricted with a fee after a 30-day trial. [V draft] Bug 538618 (2010) says "Most personas … are in fact CC licensed …
  the submission form links to by-nc-sa 3.0". CC badges were added to persona pages in March 2010. [V]
  Bug 792112 comment 1 (2012) is a designer's account that Mozilla "was trying to force designers to choose CC by stating that if they
  chose full copyright, then they would be liable for paying hosting fees". [V as a claim]
  https://bugzilla.mozilla.org/show_bug.cgi?id=538618 , https://bugzilla.mozilla.org/show_bug.cgi?id=792112

### 4.2 AMO (2010–2018 persona model)
- `Persona.license` held one of the PERSONA_LICENSES: All Rights Reserved (1), CC BY 3.0 (2), BY-NC (3),
  BY-NC-ND (4), BY-NC-SA (5), BY-ND (6), **BY-SA 3.0 (7)**.
  https://raw.githubusercontent.com/mozilla/addons-server/2019.03.14/src/olympia/constants/licenses.py [V]
- Migration SQL 555 remapped old AMO licence-table ids to these constants: 7/1034 → ARR, 8/1035 → BY-NC-SA, 9 → BY,
  10 → BY-NC, 11 → BY-NC-ND, 12 → BY-ND, **13 → BY-SA**. Migration 592: "This should've been fixed in migration 555. I don't even know …".
  https://raw.githubusercontent.com/mozilla/addons-server/2018.05.31/src/olympia/migrations/555-theme-licenses.sql [V]
- **2013 import problems:** bug 862925 reports "used to be All Right Reserved and became Creative Commons Attribution-ShareAlike 3.0 after the
  migration" (WONTFIX). Bug 894064 (2013) reports the new AMO submit flow offering only CC-BY-NC-SA (WONTFIX).
  https://bugzilla.mozilla.org/show_bug.cgi?id=862925 , https://bugzilla.mozilla.org/show_bug.cgi?id=894064 [V]
- AMO submit form (2013–2018): a CC chooser with three yes/no questions. **New submissions defaulted to "All Rights Reserved"**
  (`persona_creation.js`: "If there's no license saved (i.e., this is a new submission), then set the 'All Rights Reserved' license as the default.").
  https://raw.githubusercontent.com/mozilla/addons-server/2018.05.31/static/js/impala/persona_creation.js [V]
- **Where the BY-SA dominance comes from.** It is **not** an AMO default. In the sample, 74% of getpersonas-era themes
  carry BY-SA 3.0 and almost all the rest ARR (§4.4). That pattern means getpersonas' "CC" option was imported as
  BY-SA 3.0 (old licence id 13), even though the getpersonas UI text pointed to BY-NC-SA. [I, consistent with the V sources above]
  I found no Mozilla document that states "BY-SA was the Personas default".

### 4.3 The migration (2019) and today
- The migration copied the persona licence to the static theme's version licence, and **a missing licence became All Rights Reserved** (code, §1). [V]
- AMO today lists licences per version (`current_version.license.slug`). The CC **3.0** licences are `on_form=False`, so new uploads
  can only choose ARR or CC 4.0. There is **no CC0** among theme licences; "Unlicense" exists for code.
  https://raw.githubusercontent.com/mozilla/addons-server/master/src/olympia/constants/licenses.py [V]

### 4.4 Licence mix, sampled from the live API [V numbers, I weighting]
Method: random 1-day `created` windows with `sort=created` and `page_size=50`, about 1 request per second. Rough cluster sample.
| Stratum (count) | n | Mix |
|---|---|---|
| getpersonas era, created 2009-04..2013-03 (325,536) | 1,431 | **BY-SA-3.0 73.7%**, ARR 26.2%, others 0.1% |
| AMO-submitted LWTs, 2013-04..2018-08 (~115,700) | 1,315 | ARR 50.2%, BY-NC-ND-3.0 24.1%, BY-NC-SA-3.0 9.4%, BY-3.0 7.3%, BY-SA-3.0 3.4%, BY-NC 2.9%, BY-ND 2.7% |
| native static themes, 2019-03..2026-09 (~74,000) | 923 | ARR 26.2%, BY-3.0 19.6%, BY-NC-ND-3.0 12.1%, BY-NC-SA-3.0 11.8%, BY-SA-3.0 8.5%, BY-NC-3.0 6.8%, 4.0 variants 12.9% |
| **Weighted whole-AMO estimate** | | **BY-SA-3.0 ≈ 250 k (48%)**, ARR ≈ 163 k (32%), BY-NC-ND-3.0 ≈ 37 k, BY-3.0 ≈ 23 k, BY-NC-SA-3.0 ≈ 20 k, BY-NC-3.0 ≈ 8 k, BY-ND-3.0 ≈ 4.5 k, BY-4.0 ≈ 2.5 k, CC0 ≈ 0 |

This differs from your top-1000 sample (225 ARR / 189 BY-SA / 174 BY-3.0 / 53 BY-4.0). That is expected [I]: popular themes skew
toward recent native static themes and away from the long tail of 2009–2012 personas.

## 5. Other archives

- **Classic Add-ons Archive (JustOff/ca-archive)** https://github.com/JustOff/ca-archive . Created 2017-11-09, last push
  2021-09-17, no GitHub licence metadata. The README says the catalog is MPL 2.0 and the design (from AMO) is CC-BY-SA-3.0. [V]
  I downloaded release 2.0.3 (44.8 MB XPI). Its data is `content/db/ca-archive-19030501.sqlite` with 19,450 add-ons and
  93,598 versions. Tables: addons, versions (url, licence id), licenses, authors, categories (extension categories only),
  tags, eulas, policies, icons. **No themes, no personas.** Downloads come from its own mirror,
  `https://ca-archive.us.to/storage/<id/1000>/<id>/<file>`, not from AMO's CDN. Links go to AMO and to Wayback (`web.archive.org/web/201810/...`). [V]
- **Internet Archive.** "Firefox Legacy Collector Dump" (legacycollector.org, 15 GB, 2019-10) has 76,333 file entries,
  all under `firefox-addons/<id>/…xpi`. **No personas.** https://archive.org/details/Firefox_Legacy_Collector_Dump [V]
  Advanced searches for "getpersonas" (0 hits), "firefox personas" and "lightweight themes firefox" found no persona dump. [V]
- **Wayback Machine** (above): partial getpersonas JSON feeds and many `addons.cdn.mozilla.net/user-media/addons/<lwtid>/` header and footer
  images. You find them with the CDX prefix query
  `http://web.archive.org/cdx/search/cdx?url=addons.cdn.mozilla.net/user-media/addons/&matchType=prefix`. [V]
- **GitHub.** `mozilla/personas` is the Personas client add-on (archived, mercurial mirror). `mozilla/personas-plus` is the add-on.
  Repository searches for persona dumps found nothing. [V]
- **Community.** A Pale Moon forum thread (2023-10-28, "Is there an archive of lightweight themes …?") concluded nothing is
  archived. It notes that ATN user-media links still work and that converted static themes "lack the footer image".
  https://forum.palemoon.org/viewtopic.php?t=30477 (read via Wayback 2025-08-04) [V]
  Another thread (2019-11-03) says Pale Moon supports old-style personas "with a few available", and that "Mozilla … completely remove[d] their original
  personas site". https://forum.palemoon.org/viewtopic.php?t=23216 [V]
  `addons.thereisonlyxul.org` is behind a bot wall, and Wayback has nothing persona-related for it or for addons.palemoon.org. [V]

## 6. Legal and practical constraints for third-party indexing or re-hosting

- **The per-item licence governs.** AMO's Distribution Agreement licenses content *to Mozilla* (to "publish, distribute,
  and promote Listed Add-ons") and only "encourages" CC licences for creative works. It gives third parties nothing beyond the
  author's chosen licence. https://extensionworkshop.com/documentation/publish/firefox-add-on-distribution-agreement/ (effective 2021-12-01) [V]
- **Mozilla terms** (Websites & Communications ToU, which name AMO) contain no scraping clause. The Acceptable Use Policy only bars
  disrupting services. AMO's API docs have no ToU and no documented rate limits. v5 is "stable" but not frozen;
  v4 is frozen. `page_size` is at most 50. The `page_count` "may be lower … beyond our max_result_window", and
  page 601 at 50/page returns `{"detail":"Invalid page."}`, so each query is capped at **30,000 results**.
  Partition by `created` windows, category, etc.
  https://www.mozilla.org/en-US/about/legal/terms/mozilla/ , https://www.mozilla.org/en-US/about/legal/acceptable-use/ ,
  https://mozilla.github.io/addons-server/topics/api/overview.html , https://mozilla.github.io/addons-server/topics/api/addons.html [V]
  About 110 requests at 1 per second drew no 429s [V]. The API sends `Access-Control-Allow-Origin: *` [V].
- **robots.txt** has `Disallow: /firefox/downloads/` for crawlers, so do not bulk-crawl XPIs with a bot. A user-initiated download on the device
  is ordinary use [I]. XPIs are served directly with `ACAO: *`, `cache-control: max-age=86400`. Preview images are served from
  `addons.mozilla.org/user-media/version-previews/…` with `immutable` cache. There is no explicit hotlinking permission or prohibition. [V headers]
- **Hotlinking does not actually work for headers.** A Persona needs a header-image URL. On AMO the header exists only *inside the XPI*;
  the user-media previews are 680×92 and 720×92 renders, not the header. So a pure index cannot point a Persona at an AMO image URL. It must either
  (a) let the device fetch the XPI and convert it, or (b) extract and host the image, which is copying and needs a permissive licence. [V facts / I conclusion]
- **ARR themes:** link only. Do not re-host. **BY / BY-SA:** re-host with attribution (author, title, licence link, AMO source URL, and a note on
  any change; BY-SA adaptations stay BY-SA). **BY-NC\***: OK for a free non-commercial gallery. **ND:** byte-identical re-hosting is fine; cropping or
  recolouring is an adaptation. [I, standard CC 3.0 reading; I did not fetch CC's FAQ]
- **Licence labels can be wrong.** See the getpersonas BY-SA caveat (§4.2). Uploader labels are also only as good as the uploader's rights:
  Blade Runner – Cityscape (a film still) is labelled CC-BY-NC-SA-3.0 [V]. Build a takedown process. A conservative filter for
  getpersonas-era BY-SA items is to treat them as BY-NC-SA [I].
- **Trademarks.** Mozilla's guidelines forbid modifying marks or implying affiliation. AMO's add-on policies require
  add-ons that use Mozilla marks to comply. https://www.mozilla.org/en-US/foundation/trademarks/policy/ ,
  https://extensionworkshop.com/documentation/publish/add-on-policies/ (last updated 2026-04-30) [V]
  Firefox-logo themes in a non-Mozilla browser's gallery, and third-party brand or celebrity themes, are the riskiest. Exclude them [I].

## 7. Technical notes for an index

- **LWT JSON (UXP).** `toolkit/mozapps/extensions/LightweightThemeManager.jsm` at Jihad's pin:
  `MANDATORY = ["id","name","headerURL"]` and `OPTIONAL = ["footerURL","textcolor","accentcolor","iconURL","previewURL","author","description","homepageURL","updateURL","version"]`.
  `*URL` values must be `http|https|resource` (`file` only for local), so **`data:` URLs are rejected**. `updateURL` must be **https**, and it is
  polled by `updateCurrentTheme()`. Header and footer are persisted to the profile as `lightweighttheme-header/footer`. The default
  `maxUsedThemes` is 30. [V local source]
- **LWT image conventions.** Header 3000×200, anchored top-right, behind the toolbars. Footer 3000×100, anchored bottom-left, behind the status
  and find bars. PNG or JPG, ≤300 KB each (getpersonas guide 2010 and FAQ 2009). https://web.archive.org/web/20101120093700/http://www.getpersonas.com:80/en-US/demo_create
  and `demo_create_2` [V]. AMO constants: `PERSONA_IMAGE_SIZES = {'header': [(680,100),(3000,200)], 'footer': [None,(3000,100)], 'icon': [None,(32,32)]}`,
  `IMG_TYPES = png, jpeg`, and add-on types 2 = complete theme, 9 = persona, 10 = static theme.
  https://raw.githubusercontent.com/mozilla/addons-server/2018.05.31/src/olympia/constants/base.py [V]
- **Static theme XPI.** `manifest.json` `theme.images.theme_frame` (was `headerURL`), `additional_backgrounds[]`, `theme.colors.*`,
  `theme.properties.*`, plus `META-INF` signatures. Header area ≤ 200 px high; width up to ~5120 px for hi-dpi. Formats JPEG, PNG, APNG, SVG
  (animated SVG from Fx 59); animated GIF not supported.
  https://extensionworkshop.com/documentation/themes/static-themes/ , https://developer.mozilla.org/en-US/docs/Mozilla/Add-ons/WebExtensions/manifest.json/theme [V]
- **API fields for the index** (search results already include them) [V]: `id, guid, slug, name, authors[].name, created,
  last_updated, average_daily_users, categories, tags, ratings, previews[]{image_url,image_size,thumbnail_url}, current_version{version,
  license{slug,name,url,is_custom}, file{url,size,hash,created}}`. Search filters include `created`, `updated`, `users`, `category`, `tag`, `author`,
  `color` (experimental, hex) and `sort`. **There is no licence filter.** Previews for migrated themes: a 680×92 PNG and a 720×92 SVG render.
- **Recovering the original LWT values:** `GET /api/v5/addons/addon/<slug>/versions/`. The oldest version (`1.0`, created during the migration in Jan 2019)
  holds `headerURL/accentcolor/textcolor` and the original header. Persona mapping: `accentcolor ← frame/accentcolor`,
  `textcolor ← tab_background_text/textcolor`, `headerURL ← theme_frame/headerURL` (host it, or use a device-side `resource://`/file).
  **footerURL: none on AMO.** You can try the Wayback for `addons.cdn.mozilla.net/user-media/addons/<oldLWTid>/…footer…`, but old ids are not
  derivable from the API. Matching would be by slug or name plus a Wayback copy of the old listing [I].
- **Static themes with `additional_backgrounds`, many colours, or `dark_theme`** do not map losslessly to a Persona (one header plus two colours) [I].

---

## 8. Timeline: theme eras, formats, and the UXP fork (added at the coordinator's request)

| When | Era / change | Format and image conventions | Source | |
|---|---|---|---|---|
| Phoenix/Firefox 0.x–1.0 (2003–04) | **Complete themes**: XUL chrome skins (CSS and images) for browser/global/mozapps | XPI: `install.rdf` with `em:internalName`, plus `chrome.manifest` lines `skin <pkg> <internalName> <path>`. Restart to switch | https://udn.realityripple.com/docs/Archive/Themes/Building_a_Theme | V format / I start date |
| Firefox 1.5 (2005) | `em:type` added (`4` = theme; "only required for add-on types other than Extensions and Themes") | install.rdf | https://udn.realityripple.com/docs/Archive/Add-ons/Install_Manifests | V |
| 2007-12-14 | **Personas** Labs extension | Remote header and footer images; menu | Labs blog | V |
| 2008–2009 | getpersonas.com gallery and JSON feeds | Header 3000×200 (top-right), footer 3000×100 (bottom-left), ≤300 KB, accentcolor/textcolor | getpersonas demo_create and FAQ | V |
| 2009-10-04 | Pale Moon 1st public release, built from the Firefox 3.5.2 code; later 3.6 and "Next Generation" 4+ branches | — | https://www.palemoon.org/history.shtml | V |
| 2010-01-21 | **Firefox 3.6: lightweight themes built in**, with web install through `InstallBrowserTheme` (bug 511771) | LWT JSON (`id,name,headerURL,footerURL,accentcolor,textcolor,…`) | Fx 3.6 release notes | V |
| 2012–2013-04-11 | Personas become AMO "(Background) Themes", and getpersonas is retired | AMO type 9. `data-browsertheme` on gallery pages | AMO blog | V |
| 2014-04-29 (Fx 29, Australis) | **Footer no longer displayed** in Firefox. The add-on bar is gone | Footer still accepted but invisible | bug 898988 c2 ; release date from https://en.wikipedia.org/wiki/Firefox_version_history | V |
| 2014-10-10 | Pale Moon 25: independent of Firefox; keeps "Firefox 24.* extension compatibility", with an ESR24-based platform | complete themes + personas | https://www.palemoon.org/releasenotes-archived.shtml ; history.shtml | V / I (ESR24) |
| 2016-01-26 | **Pale Moon 26: Goanna engine** (fork of Gecko; the "first real split-off point … 24.6") | — | release notes ; history | V |
| 2016-11-22 | Pale Moon 27 "Tycho": ESR38 platform base plus PM application code; Goanna 3 | — | history ; release notes | V |
| 2017-03-07 | Firefox 52 ESR, the base UXP later forks from | — | Wikipedia version history | V |
| 2017-08-08 (Fx 55) | **WebExtension `theme` manifest key and `browser.theme` API (dynamic themes: `update`)** | `images.headerURL`, `additional_backgrounds`; `colors.accentcolor/textcolor/tab_text`; `properties` | MDN BCD `webextensions/manifest/theme.json`, `api/theme.json` (https://github.com/mdn/browser-compat-data) | V |
| Fx 56–58 | `theme.reset` (56); `toolbar`, `toolbar_field`, `toolbar_text`, `toolbar_field_text` and `windowId` (57); `bookmark_text`, separators, `theme.getCurrent`/`onUpdated` (58) | | BCD | V |
| **2017-11-14 (Fx 57)** | **Complete themes (and all legacy add-ons) stop working**. LWTs are kept | — | AMO blog 2017-02-24 | V |
| 2017-11-17 | **Basilisk** first public build. It came from the "Möbius" platform: "gecko/53 as a base, and backporting from 54 and 55 heavily. This is why Basilisk carried a Mozilla version number of 55" | XUL extensions, complete themes, LWT, "experimental WebExtensions" | https://www.palemoon.org/history.shtml ; https://www.ghacks.net/2017/11/17/pale-moon-team-releases-first-version-of-basilisk-browser/ | V |
| **2018-02-01/02** | **UXP repository created**. "Initial commit" (Moonchild, 2018-02-01), then "Add m-esr52 at 52.6.0" (2018-02-02): "Take 2", built on the **Firefox ESR-52 level**. The day-one "Add Basilisk" import (app version 55.0.0) carried a 19-line `ext-theme.js` and a 24-line `schemas/theme.json`. Both were removed by "Revert 'Add Basilisk'" the same day | — | `git log` of Jihad's `third_party/uxp` (commits 9b6b26a0a4, 5f8de423f1, e72ef92b5b, 6d614170cb) ; history.shtml | V |
| 2018-02-09 → 2018-04-20 | WebExtensions made optional per application; Pale Moon (PMkit) "Remove embedded WebExtensions support" | | UXP git (30de401891, 14bfaf641a, 4c0556f928) | V |
| 2018-02-21 | UXP identifies as Goanna; milestone 4.1.0 | | UXP git 2e07719b30 | V |
| 2018-03-13 (Fx 59) | CSS colour strings and RGB arrays allowed; tab_background_text; animated SVG backgrounds | | BCD notes ; MDN theme page | V |
| 2018-05-09 (Fx 60) | `frame_inactive`, `tab_selected`, `tab_line`, `tab_loading`, `popup*`, `icons*`, `button_background_*`; `images` no longer mandatory | | BCD | V |
| 2018-06-05 | **Firefox Color** (Test Pilot): a colour-and-texture theme editor; themes shared by URL | colour-only plus built-in textures | https://techcrunch.com/2018/06/05/firefox-launches-side-by-side-browsing-and-a-theme-editor-as-test-pilot-experiments/ | V (search summary) |
| 2018-06-26 | Firefox **52.9.0 ESR**, the last ESR52 point release (next: 60.0esr). This is the "Firefox/52.9" compatibility token | — | https://www.mozilla.org/en-US/firefox/52.9.0/releasenotes/ | V |
| **2018-08-16** | **Pale Moon 28.0 on UXP, Goanna 4** | complete themes plus personas (header and footer) | Pale Moon release notes | V |
| 2018-09-05 (Fx 62) | Installed LWTs can update to their migrated static theme (bug 1430276) | | Bugzilla | V |
| 2018-09-20 | AMO accepts static themes; 400 k LWTs to be converted | | AMO blog | V |
| 2018-10-23 (Fx 63) | `theme_experiment` key; `ntp_background/ntp_text`; accentcolor/textcolor no longer mandatory | | BCD | V |
| 2018-12-11 (Fx 64) | `footerURL` code removed ("not displayed anywhere"), bug 1471305 | | Bugzilla | V |
| **2019-02-01** | **AMO LWT → static migration complete** (public LWTs only; the rest deleted, command added 2019-03-07) | static XPI with `headerURL`/`accentcolor`/`textcolor`, no footer | mozilla/addons#6079, #6460 | V |
| 2019-02-13 | **UXP removes WebExtension support** from the platform and from Basilisk | UXP never shipped the `theme` key | UXP git 43d44975b1, 6bb02d95f7, 1e0da1994d | V |
| 2019-04-18 | AMO repacks themes using deprecated keys | `theme_frame`/`frame`/`tab_background_text` | mozilla/addons#6385 | V |
| **2019-07-09 (Fx 68)** | **LWT web install and remaining LWT plumbing removed**; built-in LWTs converted; **`dark_theme`** key | | bugs 1525511, 1525762 ; BCD `dark_theme` | V |
| 2019-10-22 (Fx 70) | **`accentcolor`, `textcolor`, `headerURL` aliases removed** (use `frame`, `tab_background_text`, `theme_frame`) | | BCD v2.0.7 https://cdn.jsdelivr.net/npm/@mdn/browser-compat-data@2.0.7/webextensions/manifest/theme.json ; bug 1567684 | V |
| 2020-09-22 (Fx 81) | **Alpenglow** built-in theme | static theme with SVG `additional_backgrounds` ×3, `additional_backgrounds_alignment`, `additional_backgrounds_tiling` `["no-repeat","no-repeat","repeat-x"]` | Fx 81 release notes ; hg `FIREFOX_81_0_RELEASE/browser/themes/addons/alpenglow/manifest.json` | V |
| 2021-11-02 (Fx 94) | **Colorways**: 18 built-in themes (6 colourways × soft, balanced, bold) for a limited time | **colours only**, no images, plus `theme_experiment` (e.g. `abstract-soft-colorway@mozilla.org`) | https://blog.mozilla.org/en/products/firefox/introducing-new-colorways-for-firefox-94/ ; hg `FIREFOX_94_0_RELEASE/browser/themes/addons/monochromatic/abstract/soft/manifest.json` | V |
| 2022-03-17 / 2022-05-10 | Pale Moon 30 on the "Goanna Runtime Environment"; PM 31 on "UXP and Goanna (v5.1)" | | release notes | V |
| 2022-05-03 (Fx 100) | `properties.color_scheme` / `content_color_scheme` | | BCD | V |
| 2022-10-18 → 2023-01-16 | Fx 106 "Independent Voices" Colorways, available until 2023-01-16 | | press coverage via search (e.g. https://www.ghacks.net/2022/10/18/firefox-106-is-out-with-firefox-view-and-other-improvements/) | I (not opened) |
| 2022-12-24 → 2026-04-29 | UXP milestone 6.0.0 (2022-12-24), 6.1 (2023-03-12), 6.2 (2023-05-03; PM 32.2.0 "platform version to 6.2"), 6.3 (2023-07-25), 6.5 (2023-10-25), 6.6 (2024-01-01), 6.7 (2024-07-16), 6.8 (2025-07-08), 6.8.1 (2026-02-14), **6.9.0 (2026-04-29)** | | UXP git `config/milestone.txt` ; PM release notes | V |
| Fx 153 / Fx 156 | CSS gradients in images; `additional_backgrounds_size` (153); `backgrounds_area` (156) | | BCD / MDN | V |
| — | APNG animated themes | supported in static themes (no version found) | Extension Workshop | V (no version) |

### What UXP (and so Jihad) supports natively, and what came after the fork
- **Native in UXP:** complete themes (install.rdf/chrome.manifest skins) and **lightweight themes / Personas with header and footer**
  (`LightweightThemeConsumer.jsm` applies `footerURL` to the footer element) [V local source]. Pale Moon lists "Support for Personas
  ('lightweight' themes)" and "Support for Complete Themes" (https://www.palemoon.org/technical.shtml) [V]. Pale Moon documents the self-hosted install:
  `data-browsertheme` plus the `InstallBrowserTheme` event, with footerURL "as the background for the status and find bars"
  (https://developer.palemoon.org/addons/themes/personas/) [V]. The current UXP tree has no `application/` directory, so the
  `InstallBrowserTheme` page listener lives in each application, not in the platform [V].
- **Never supported in UXP (post-fork or WebExtension-only):** the WebExtension `theme` key (Fx 55+). It existed only in the 2018-02-02
  import of the 55-era Basilisk app and was reverted that day. ESR52's WebExtensions never had it, and UXP dropped WebExtensions entirely on 2019-02-13.
  Also unsupported: `additional_backgrounds` with alignment, tiling or size; the expanded colours list; `dark_theme`, `color_scheme` and `content_color_scheme`;
  `theme_experiment`; the `browser.theme` dynamic API; Firefox Color; Alpenglow; Colorways. [V git + BCD; I "never" for releases I did not inspect]
- **Jihad's UA.** `Goanna/6.9 UXP/b2594a4 Firefox/52.9 ECMAScript/2024` comes from Jihad's own
  `render/goanna/JihadUserAgent.h`. `b2594a4` is a UXP master merge dated 2026-06-30, which is on the 6.9.0 milestone (since 2026-04-29) [V].
  "Firefox/52.9" is Jihad's own site-compat token naming the ESR52 base (52.9.0esr, 2018-06-26). UXP's own default is
  `general.useragent.compatMode.version = "128.0"` (`modules/libpref/init/all.js`) [V]. Which Pale Moon release first shipped Goanna 6.9:
  probably 34.3.0 (2026-06-02) or later [I, not confirmed in the release notes].

## 9. Not found / open
- An exact count of LWTs migrated on production. The best proxies are the 441,220 pre-2018-09-20 static themes now, "400 k+" in 2018 blogs, and 349,970 on the dev copy.
- A first-party statement of the getpersonas licence default and of how it was mapped to AMO licence id 13 (BY-SA).
- Whether `delete_lwt` was run on production, and when. Strongly implied by the absence of LWTs today.
- The exact Firefox version that first supported APNG and the per-property versions of `additional_backgrounds_alignment/tiling`. BCD lists only `properties` (55) and `additional_backgrounds_size` (153).
