# The modern web on Jihad — the compatibility ledger

*Kit: `context/kits/cavekit-modern-web.md` (R1). One row per site tested on the device. A failure
names its cause by category — engine feature, user-agent sniffing, bot challenge, slow script,
adverts and trackers, layout on 768 px, not a browser fault — and its disposition: fixed (how,
where), workaround, open (why), upstream (Palefill, UXP).*

**Engine build for every row below:** Goanna 6.9, UXP `RB_20261006` (`jihad/uxp-RB_20261006`
`706d4782`), daemon `06e75a68`, Palefill 1.30; webOS 3.0.5 TouchPad. Screenshots: the session's
raw captures (`<shot>.png`, 1024×768 rotated to portrait) unless a `docs/screenshots/` name is
given.

## How to see a page's own JavaScript errors on the device

The daemon bridges the engine's console to `/var/palm/jihad/<variant>/daemon.log` as
`[jihad-bs] js-console <source>:<line>:<column> <message>` (the column since daemon 12, 2026-10-16: a one-line bundle is named by its column) — but by default only chrome://, resource://
and the browser's own about: pages, because a page's errors carry page-controlled text and the
device log is persistent (EngineHost.cpp, finding F-8). To see content errors for a diagnosis,
start the daemon with `JIHAD_JS_CONSOLE=all` in its environment (the upstart job's `env` line in
`/etc/event.d/jihad-<variant>`, `/` remounted read-write for the edit, back to read-only after;
or `stop jihad-<variant>` and run the job's command by hand from `dev-sh.sh` with the variable
set). Put it back afterwards. `JIHAD_JS_CONSOLE=0` silences the bridge. The daemon's log rotates at 2 MB, so never window it by a byte offset taken before
a restart: find the newest `js-console bridge on` line by number (`grep -n`) and read from there
(`tail -n +N`); the bridge's own line says `ALL sources` when the variable reached the daemon.

## Ledger (2026-10-15)

| Address | Variant | Result | Cause | Disposition |
|---|---|---|---|---|
| `https://duckduckgo.com/?q=webOS&ia=web` | Mochi | at the engine's 15 s guard: "A script on this page may be busy" over a half-built page (shot `mochi-wiki`, 10:30). With the guard at 60 s (`dom.max_script_run_time`, T-481): no dialog, but the page is wrong — its slide-out menu drawn as a plain list on a white page, no results (shot `t481-ddg`, 10:47) | **engine feature** (to be named with the console): DuckDuckGo's full results page is a JavaScript application the engine cannot finish; the slow-script dialog was only the first symptom | **workaround, in place**: Jihad searches DuckDuckGo **Lite** (`lite.duckduckgo.com`) by default, which is plain HTML and renders its results (shot `t481-lite`, 11:56). **2026-10-16 (T-498, the owner: "fix the regular duckduckgo"): the full page renders its results now** — with the 60 s guard, daemon 12 and the console on, the page loads in 39–42 s (a 13 s main-thread stall, 196–263 MB) with ZERO script errors; what was wrong in the shot was the popover attribute, which the engine lacks (its menu panel and a promo stood open at the top). The web-compat component's popover shim closes that (the trace round's shot). Slow, not broken. |
| `https://github.com/NotAlexNoyle/Jihad-Browser` | Mojo, Mochi | the frame renders — header, file list with folder names, About with the description, Releases/Packages/Contributors/Languages headings, README/License tabs — but the commit messages and times, the README body and the sidebar counts stay grey loading bars after 75 s (shots `mojo-wiki` 10:30, `con-gh` 12:16) | **engine feature**, named from the console (`JIHAD_JS_CONSOLE=all`, 12:16): `ReferenceError: Turbo is not defined` at the page's line 68 — GitHub's Hotwire Turbo and its `include-fragment` elements are ES modules the engine never loads (the same module limitation Mastodon shows as `import.meta may only appear in a module`); the deferred parts are theirs to fill. Palefill 1.30 polyfills APIs, not module syntax | **upstream (UXP)**: ES-module loading (`import.meta`, modern module graphs); no configuration fixes it. A newer UA token would only make GitHub serve more of the same |
| `https://www.w3.org/` | Enyo, Mochi | Cloudflare: "www.w3.org — Performing security verification … Why is this verification taking longer?", never passes (shots `enyo-wiki` 10:31; `cb-w3` 12:05 with the UA rule; `con-w3` 12:14 with the console on) | **bot challenge**: Cloudflare's Turnstile. The console shows no script error — the challenge runs: CSP warnings for `trusted-types` directives the engine does not know, and inside the challenge frame "Content at https://challenges.cloudflare.com/…/turnstile/… may not load data from https://example.org/" (one of its probes). It then fails on its own verdict (environment fingerprint; the main request carries Jihad's UA with `Firefox/52.9`). The per-host rule for `challenges.cloudflare.com` (T-482, Firefox/128) is in the daemon and did not change the outcome | **open**: the next experiment is a current Firefox ESR token on the SITE's own requests (the challenge is decided on the first request), which means deciding whether the base UA's `Firefox/52.9` should move to a current ESR for every site — the owner's call, since it changes what every site serves |
| `https://developer.mozilla.org/en-US/docs/Web/CSS` | Enyo, Mochi | renders well: masthead, breadcrumb, article, sidebar (shot `enyo-wiki`, 10:25); an MDN promo banner at the top and a Twilio advert in the sidebar. **With the content blocker (v3, 12:34, `cb3-mdn`): neither** — the banner by EasyList's site rule, the sidebar advert by the `||developer.mozilla.org/pong/` path rule (the daemon logs `blocked fetch developer.mozilla.org`) | **adverts and trackers** | **fixed** (cavekit-content-blocking.md; the README's hero shows this page) |
| `https://en.m.wikipedia.org/wiki/WebOS` | Mojo | the desktop article at `en.wikipedia.org/wiki/WebOS` (shot `mojo-wiki`, 10:13): complete, small type | **layout on 768 px**: the UA table sends a desktop Firefox/78 token for `wikipedia.org`, so Wikipedia serves and redirects to the desktop site | **decided (T-484): kept.** The override exists so Wikipedia serves a page the engine renders well, the desktop article is complete and readable with a pinch, and a mobile layout would need either a per-site exception to the override (a second rule shape in the daemon) or Wikipedia's own "mobile view" link, which works. Revisit if a user asks |
| `https://en.wikipedia.org/wiki/Goanna_(software)` | Enyo | Wikipedia's "does not have an article with this exact name" page | **not a browser fault** (no such article) | closed |
| `https://en.m.wikipedia.org/wiki/Palm,_Inc.` | Mochi | the desktop article, complete | as the WebOS row | as above |
| `file:///tmp/jihad-busy.html` (`render/goanna/test/jihad-busy.html`, an endless script) | Mochi | at the 60 s guard the "script may be busy" dialog appears, naming the script (shot `t481-busy`, 11:59): the guard is kept | — | works (T-481's third criterion) |
| `https://www.webosarchive.org/` | Enyo | renders: the 3.1.0 Community Edition banner, Apps/Developer/Support (shot `enyo-wiki`, 10:17; in the README's card view) | — | works |
| `https://pivotce.com/` | Mochi | renders: masthead, hero image, "Guide: create an original barrel charging wire", tags (shot `mochi-wiki`, 10:32; in the README's card view) | — | works |
| `https://www.howsmyssl.com/` | Mojo | renders; Version: Good, TLS 1.3; rating "Improvable" for no post-quantum group (`docs/screenshots/tls.webp`) | — | works (PQ deferred by the owner; PICKUP 2026-10-15) |
| `https://archive.org/download/flash_bloxorz/flash_bloxorz.swf` | Enyo | Bloxorz plays full-page in the device's Flash Player (`docs/screenshots/flash.webp`) | — | works |
| `https://codeberg.org/` | Mojo | renders: the Forgejo home with logo, text, Register (shot `s-codeberg`, 10:36) | — | works |
| `https://archive.org/` | Mojo | renders: Wayback Machine search, the media icons, Archive News (shot `s-archive`, 10:37) | — | works |
| `https://www.mozilla.org/en-US/` | Mojo | renders: the hero and the sections (shot `s-mozilla`, 10:38) | — | works |
| `https://mastodon.social/explore` | Mojo, Mochi | the shell renders (search, Posts/Hashtags/News tabs, the sidebar) but the trending posts column stays empty (shots `s-mastodon` 10:39, `con-masto` 12:18) | **engine feature**, named from the console: `SyntaxError: import.meta may only appear in a module` in a `blob:` script Mastodon's app creates (a module worker/chunk); the app's data loading never runs. Also a CSP source the engine does not parse (`wasm-unsafe-eval`, harmless) | **upstream (UXP)**: ES-module loading in blob/worker scripts |
| `https://www.theverge.com/` | Mojo | first run: the start page (a launch race, the daemon had not opened the address yet — a wait too short, not a refusal). At the 15 s guard: the masthead, logo and Top stories / Latest / Following tabs render, then "A script on this page may be busy … `/_next/static/chunks/001weln5n2jwt.js`" (shot `s-verge`, 10:48). **At the 60 s guard: the whole front page renders, hero picture and headline, no dialog** (shot `t481-verge`, 11:57) | **slow script**: the site's Next.js chunk runs past 15 s on this CPU | **fixed, T-481**: `dom.max_script_run_time` 60 in `packaging/prefs/jihad-platform-prefs.js` |
| `https://www.bbc.com/news` | Mojo | renders: the BBC News front with pictures (shot `s-bbc`, 10:41) | — | works |

## Method

One card per shot: every Jihad card closed (`com.palm.applicationManager/close` per process id),
the variant launched with `{"target": url}`, 35–75 s of wait, `com.palm.systemmanager/takeScreenShot`;
the daemon log's `load done uri=` line and paint counts read for the same window. The scripts live in
the session scratchpad; `build/webos-oe/card-selftest.sh` does the same with a test hook when a card
must be driven.


## Round 2 — a pretend user's half hour (2026-10-15, 11:40–12:20 device time; T-491)

Thirty visits in Mochi, the console bridge on all sources, the content blocker at v3 (the global
sheet still carrying 8,000 site rules), `dom.max_script_run_time` 60. The script, the site list and
the analysis: `tools/device/browse-round*.{sh,txt,py}`; the raw log and the table:
`docs/rounds/round2-2026-10-15.log`, `docs/rounds/round2-2026-10-15-analysis.md`; the shots are
`/tmp/shot-r2-<name>.png` on the device until it reboots (not pulled: the session ended).

**What the half hour says, before the per-site rows are written:**
- **Performance is the issue, on every site.** First paint comes in 1–5 s, but the page then
  stalls the main thread: the worst tick per visit is 9–43 s (Guardian 9.2, NPR 15.2, AP 13.9,
  Al Jazeera 17.4, Britannica 40.3, OSM 21.3, Reddit 32.5, Bluesky 37.5, GitLab 43.4, html5test
  40.0) and webos-ports.org 125 s; the frame rate during the dwell averages 0.3–1.5 fps. Ten of
  thirty never reached load-done within 45–65 s (Guardian, AP, Reddit, Vimeo, GitLab, npm,
  webos-ports, webosnation, weather.com, Craigslist, Acid3). The stalls are JavaScript on the
  main thread (each site's own bundles and ad-tech); with the guard at 60 s the card freezes for
  that long instead of asking at 15 s — **the guard's value is a trade-off to decide with these
  numbers**, and whether the JIT is live on this ARM build is the first thing to measure
  (`render/goanna/test/jihad-jsbench.html`, in the verification script; host V8 reference: the
  50M loop 167 ms).
- **The blocker's share** is unknown until the second run with v5.1 (site rules per document,
  no 21,000-rule global sheet): the round ran v3.
- **Per-site faults seen in the console:** Al Jazeera loads an anti-adblock script
  (`cdn.jsdelivr.net/gh/ad-shield/e/index.js`, "location is null"); Britannica embeds Cloudflare's
  widget (`postMessage` to challenges.cloudflare.com fails); The Guardian's images refused by the
  engine's CORS ("may not load data from uploads.guim.co.uk"); AP: `decodeAudioData` unknown
  content type, `NS_ERROR_NOT_INITIALIZED`; OSM: WebGL refused by the blacklist (software
  rendering; raster tiles should still draw); NPR: SVG `dur` values the engine does not parse.
- **Refusals per visit** ranged 0–29 (AP 29, NPR 7, Reddit 5, Substack 5); seven visits had none.
- **Memory per visit was not captured** (the hand-started daemon's pid was not found by BusyBox
  `ps`; fixed in the script for the next run); the controlled A/B is in cavekit-content-blocking.md
  R6.


### Round 2, site by site (the analysis table has the numbers; this is the reading)

A caveat on the console lines: the log window of a visit begins when the previous card closes,
so the first lines of a visit can be the previous page's last (NPR's SVG warnings under AP,
Vimeo's under Stack Overflow, the "scripts are blocked internally" lines of a page being closed).
Every row below reads the lines against the address that produced them.

| # | Visit | What happened | Cause | Disposition |
|---|---|---|---|---|
| 1 | Guardian | painted in 5 s, never load-done; a 9.2 s stall; the front page **renders** with pictures (shot); "Content at theguardian.com may not load data from uploads.guim.co.uk/…loop" | **slow script** (the site's bundle); an engine URI-check refusal for the animated loop media (named later) | works, slowly → T-492 (performance) |
| 2 | NPR | load-done 25 s; a 15.2 s stall; SVG `dur=".8s"` not parsed | slow script; engine: SVG animation timing values without a leading zero | open; the SVG parser gap is upstream (cosmetic) |
| 3 | AP News | never load-done (its page keeps loading); 13.9 s stall; **renders** (shot); 29 refusals; `decodeAudioData` unknown content type; `NS_ERROR_NOT_INITIALIZED` from its bundle | slow script; **engine feature**: an API that throws NS_ERROR_NOT_INITIALIZED to page script (four sites) — `jihad-apis.html` names it | works, slowly → T-492 |
| 4 | Al Jazeera | load-done 36 s; 17.4 s stall; **renders** (shot: LIVE, headlines); the anti-adblock script `cdn.jsdelivr.net/gh/ad-shield/e/index.js` throws | slow script; an anti-adblock script from jsdelivr (not on the lists) | works, slowly |
| 5 | Britannica | **Cloudflare's "Performing security verification"** page (shot), never passes; a 40.3 s stall before it | **bot challenge** (Cloudflare) | open: the UA question (below) |
| 6 | OpenStreetMap | load-done 23 s; 21.3 s stall; **renders** the map tiles and its welcome dialog (shot); WebGL refused (blacklist) | slow script; WebGL expected without a GPU driver | works, slowly |
| 7 | wikiHow | load-done 9 s; the site renders its "page doesn't exist" page (shot) — my address was wrong | not a browser fault | works |
| 8 | Amazon search | load-done 37 s; 3.3 s stall; **renders** the results with pictures and prices (shot) | slow load; PerformanceObserver entry types missing (warnings) | works, slowly |
| 9 | eBay search | eBay's **"SORRY, something went wrong"** page (shot); its bot script probes WebGL | **bot detection** (eBay) | **not the identity** (T-496, 2026-10-16: curl from the host gets the same Akamai 403 page with the 52.9 and the 128 token alike, over HTTP/2 and 1.1, with a browser's full header set): Akamai's classification of the transport; open, no configuration applies |
| 10 | Etsy search | **"Verification Required"** (DataDome, shot); `captcha-delivery.com` throws NS_ERROR_NOT_INITIALIZED | **bot challenge** (DataDome) + the engine API | open: the UA question |
| 11 | old Reddit | redirected to **`old.reddit.com/login/?reason=lor2`** (a login gate Reddit now puts in front of old-reddit for unknown clients), whose form never rendered: the Snoo pattern background only (shot); 17.5 s stall | **bot/visitor gate** (Reddit) + the login page's app not rendering | **not the identity** (T-496: curl from the host is redirected the same way with either token — Reddit's gate for logged-out visitors from this network); open: the login page's app (an `NS_ERROR_NOT_INITIALIZED` thrower, see T-496) |
| 12 | Reddit | never load-done; 32.5 s stall; **renders r/webos with its posts** (shot) despite "Deferred script … nonce" and es-module-shims | slow script; the module shim compiles its own loader (asm.js) | works, slowly |
| 13 | Bluesky | a **blank page with the Bluesky footer** (shot); 37.5 s stall | **engine feature**: a React app on ES modules | upstream |
| 14 | YouTube watch | **"Please update your browser"** page (shot) | **user-agent sniffing**: YouTube rejects the Firefox/78 token the UA table sends it | the Firefox/128 row made the desktop page render (2026-10-15); **T-497 (2026-10-16, the owner: "speed up youtube"): youtube.com now gets a MOBILE Firefox identity and YouTube serves its mobile site** — see T-497 below for the numbers |
| 15 | Vimeo staff picks | the sidebar renders, the content area stays blank (shot); `NS_ERROR_NOT_INITIALIZED` ×5 from its Next.js chunk | **engine feature**: the same API | open → T-492 |
| 16 | Stack Overflow tag page | first paint 13 s, load-done 21 s; 23.8 s stall; **renders** the question list (shot) | slow script | works, slowly |
| 17 | GitLab explore | the frame renders ("Explore projects", filters) with an empty list (shot); 43.4 s stall; jQuery throws NS_ERROR_NOT_INITIALIZED | **engine feature**: the same API; the list is fetched by script | open → T-492 |
| 18 | npm package page | the card shows **Jihad's start page**: the address was never opened (shot) | a launch race (the card launched before the previous one had closed, as The Verge's first run) — not the site | retest in the second run |
| 19 | webos-ports.org | first paint 38 s, never load-done; a **125 s** stall; **renders** LuneOS's page (shot) | slow script — the worst of the round, on a WordPress theme | open → T-492 |
| 20 | webosnation.com | **redirects to techradar.com**, which renders (shot) — the site was folded into TechRadar | not a browser fault | works |
| 21 | Startpage results | **"Your connection has been suspended"** (shot): Startpage refuses the client as automated traffic | **bot detection** (Startpage) | **learned** (T-496, 2026-10-16: the 52.9 token is answered with a 303 to `/sp/captcha-block` on every page, a current token with the page — the second learn signature; on the device the home page is learned, loaded again and renders); the search then meets Anubis's proof of work, which is the same for every browser (see T-496 for how long it takes here) |
| 22 | Bing results | Bing's **"I'm not a robot"** page (shot) | **bot challenge** (the UA table sends Firefox/52.9 to bing.com) | open: the UA question |
| 23 | Google results | Google's **`/sorry/` CAPTCHA** page (shot); "reCAPTCHA Timeout" | **bot challenge** (Google; the table sends Firefox/71) | open: the UA question |
| 24 | weather.com | weather.com's own **"Please try again"** error page (shot) | the site's app failed in this engine (engine feature, unnamed) | open |
| 25 | Medium topic | **Cloudflare's "Performing security verification"** (shot) | **bot challenge** (Cloudflare) | open: the UA question |
| 26 | Substack discover | a **blank page** under a black header strip (shot); load-done 46 s | **engine feature** (its app did not render; unnamed) | open |
| 27 | Craigslist SF | **renders** the category list (shot); never load-done (a late resource) | fine | works |
| 28 | IMDb title | **renders** The Shawshank Redemption's page with poster and ratings (shot); AWS WAF's challenge script ran and passed | works; the WAF probe passed | works |
| 29 | html5test | **450 of 588 points** (shot; the README said 447 from an earlier build) | — | works |
| 30 | Acid3 | Jihad's own certificate dialog: "the security certificate acid3.acidtests.org isn't trustworthy" (shot) — the site's certificate | not a browser fault; the dialog works | closed |

**Across the round, with the shots read:** 17 of 30 render as a user would want (slowly);
**7 of 30 are stopped by bot detection** — Cloudflare (Medium, Britannica), DataDome (Etsy),
Bing's captcha, Google's `/sorry/`, Startpage's "connection suspended", eBay's error page — and
YouTube serves "update your browser": **the user-agent is the single biggest functional issue**
of the round. The base UA carries `Firefox/52.9` and the override table sends Firefox 52.9–78 to
the big sites; to today's bot filters that reads as a decade-old browser. The fix to try first
is a current Firefox ESR token (128) on those hosts in the table (an experiment on the seven),
then, with the result, the base UA itself — the owner's decision, since it changes what every
site serves. (b) Every page stalls the main thread 9–43 s while its scripts run. **The benchmark**
(`jihad-jsbench.html`, Mojo, 14:48; the host's V8 in brackets): loop 50M **976 ms** (167),
objects 2M **6,854 ms** (200), strings 200k **8,280 ms** (19), regex 100k **902 ms** (17), DOM
20k **10,156 ms**. The JIT is live (integer code at 6× the host); allocation-heavy code is
34–436× slower: a garbage-collection or allocator problem (the ARM build has
`--disable-jemalloc`; the daemon's low-memory guardrail is not it — its threshold is 48 MB
available and the device sits near 470 MB). **The GC log (`javascript.options.mem.log`, 14:57–15:02) apportions it:** the object test's time
is garbage collection — four NON-incremental full collections ("GCBytesTrigger": the allocation
threshold reached, `javascript.options.mem.gc_allocation_threshold_mb` 30) of 0.7–3.9 s each,
about three quarters of the test; and the first full collection costs **3.9 s on Mochi with the
blocker's 100,000 hostname strings in the heap against 1.1 s on Mojo without** — the blocker's own
performance cost, fixed by keeping the hostnames in one string with a typed-array hash table
(component v6) so a collection marks a handful of things. **The threshold measured on Mojo (15:05–15:20, two runs each):** at 30 MB (the engine's default)
5–7 full non-incremental collections a run, 2.7–3.1 s of stalls, objects 3,862/2,940 ms; at
96 MB one run good (objects 972 ms) and one terrible (9,678 ms: an incremental collection landed
inside the test); at **160 MB no full collection**, objects 1,584/1,012 ms, strings 852/522 ms,
regex 190/204 ms. **Shipped: 160 MB** with the high-water mark at 192 (`jihad-platform-prefs.js`),
the daemon's low-memory guardrail as the safety net; the second run's per-visit RSS watches it. The run-to-run
spread is wide (objects 3.7–9.5 s, strings 0.5–8.3 s): a collection landing inside a test
dominates it. The ARM build's `--disable-jemalloc`
was inherited from the desktop headless config with no stated reason (DEVICE-BUILD.md: "the
platform allocator"); the engine allocates through the bundled glibc 2.23 malloc. If the GC log
shows collection time small and the allocation tests still slow, an engine rebuild with
jemalloc on (hours, cavekit-engine-update.md) is the experiment that follows.
(c) One engine API throws `NS_ERROR_NOT_INITIALIZED` to page scripts on AP, Etsy, Vimeo and
GitLab — `jihad-apis.html` probes the usual suspects. (d) ES-module apps (Bluesky, Substack?,
weather.com?) are upstream until named. (e) Two visits were the test's own faults (wikiHow's
address, npm's launch race).

## Round 2, second run — aborted at its first visit (2026-10-15, 15:40)

Run on Mochi with blocker v6, the UA experiment daemon and the GC allocation threshold at 160 MB
(`docs/rounds/round2b-2026-10-15/`). The API probe page loaded (first paint 11 s, load-done 14 s,
three console lines) with the daemon at **214 MB resident**; the device then refused new cards
("Sorry, too many cards") and the owner saw it hang. Cause: the threshold lets the heap grow to
160 MB before a major collection, and three daemons share 1 GB with LunaSysMgr. Recovered with a
LunaSysMgr restart; the shipped threshold is 64 MB; the blocker's lists now load only while a
card is connected (v8). The run is repeated at that state.

## Round 2, second run (2c) — 2026-10-15 16:33–17:34, read 2026-10-16

Mochi, blocker **v8**, daemon 8 (the UA experiment, the allocator trim, the card notifications),
GC allocation threshold **64 MB**; the two probes first, then the thirty sites. Log, analysis,
shots: `docs/rounds/round2c-2026-10-15/`; side by side with the first run:
`tools/device/browse-round-compare.py docs/rounds/round2-2026-10-15.log docs/rounds/round2c-2026-10-15/round.log`.

**The user-agent experiment — four of the eight proved out.** With a Firefox/128 token for
those hosts: **Google** renders its results (the `/sorry/` CAPTCHA before), **Bing** its results
("I'm not a robot" before), **Etsy** its results (DataDome's "Verification Required" before),
**YouTube** its watch page ("Please update your browser" before). Unchanged: Cloudflare's
"Performing security verification" on Medium and Britannica — and this run also on AP News and
npm, which the first run opened: the challenge fingerprints the engine and keys on the day's
repeated visits, not the token; Startpage moved to "Verifying your request…" (a JS challenge);
eBay still its error page; old Reddit still the login gate. The four rows stay in the daemon's
table at 128 (`EngineHost.cpp`), the other five experimental rows are out. **The base UA still
carries `Firefox/52.9`; the four say what moving it to a current ESR would gain** — the owner's
decision.

**The API probe (`jihad-apis.html`, shot):** every probed API and language feature is present —
storage, matchMedia, PerformanceObserver (mark, measure, navigation, resource), the observers,
fetch and streams, the ES2017–2022 syntax, dynamic import(), custom elements, Shadow DOM, CSS
grid/gap/custom properties/:is()/aspect-ratio/sticky — except **CSS `:has()`** and **WebGL**
(no GPU driver). *(`:has()` turned out to be in the engine behind a pref, and is on since
2026-10-16 — T-498.)* So the `NS_ERROR_NOT_INITIALIZED` that AP News, Etsy's DataDome, Vimeo and
GitLab's jQuery throw is not among these; `render/goanna/test/jihad-jquery-probe.html` loads
jQuery 3.7.1 under `window.onerror` to get the stack and name the call.

**The benchmark at the shipped threshold (Mochi, v8 lists loaded):** loop 50M 550 ms, objects
2M **1,186 ms** (6,854 in the first run), strings 200k **1,304 ms** (8,280), regex 100k
**178 ms** (902), DOM 20k 5,242 ms (10,156) — the garbage-collection fixes did what the GC log
said they would.

**The sites' stalls did not move** (worst tick better by 30% on 7, worse on 8; Amazon 60 s and
Substack 55 s this time, webos-ports 125 → 11 s): a page's own scripts on this CPU, not the
engine's collector. 20 of 30 render (Craigslist, IMDb, Amazon, Guardian, NPR, Al Jazeera, OSM,
Stack Overflow, html5test 450, new Reddit, webos-ports, TechRadar, Google, Bing, Etsy, YouTube,
weather.com's page this time, wikiHow (a timeout dialog on this run: transient), Startpage's
challenge page counts as blocked), 6 stopped by challenges (Medium, Britannica, AP News, npm,
Startpage, eBay), 4 blank apps (Bluesky, GitLab's list, Vimeo's content, Substack, old Reddit's
login form).

**Memory, the run's own finding:** the RSS column works now and the daemon **climbs and never
comes back** — 149 MB at the probe, 209 after The Guardian, 433 after Reddit, 564 after Bluesky,
**589 after YouTube**, then 500–580 MB for every later visit although each card is closed before
the next opens. The engine keeps page memory and caches after a card closes and the bundled
glibc returns nothing on its own. Fixed in daemon 9: when the last card leaves, a
`memory-pressure heap-minimize` and a `malloc_trim` (the low-memory guardrail's reaction, run
once at the moment nothing is on screen). To be measured across three heavy pages.

**Memory after cards, measured with daemon 10 (2026-10-16 02:05–02:11, Mochi, v8 lists on
demand):** idle with no card **34.8 MB** (33.4 without the blocker at all: the on-demand lists
cost nothing idle). With a card, each closed before the next: YouTube 163 MB, Reddit 339 MB,
Bluesky 428 MB. After the last close the daemon's purge ("caches dropped, malloc_trim returned
pages") brings it to **226 MB**, and it stays there (225 MB a minute later) with nothing open:
~190 MB the purge does not reach — closed windows that no collection has freed yet, or the
allocator's fragmentation. Component v9 runs cycle collection, a shrinking GC and cycle
collection again three seconds after the last card leaves and logs the JS compartment count: a
user compartment with no card is a zombie window.

**The jQuery probe:** jQuery 3.7.1 loads from jsdelivr without any error (`loaded 3.7.1`), so
the `NS_ERROR_NOT_INITIALIZED` on GitLab, AP News, Etsy's DataDome and Vimeo is thrown by a
call those pages make later (an XHR, a storage or a crypto call under their settings), not by
jQuery's start. Unnamed; open. The next probe wraps `XMLHttpRequest.prototype.open/send`,
`localStorage`, `crypto.subtle` and `navigator.sendBeacon` with logging and loads GitLab's
explore page.

**Zombie windows, confirmed (v9, 2026-10-16 02:30):** three heavy pages, each card closed before
the next; after the last close and v9's CC → shrinking GC → CC: resident **169.8 MB**, JS GC heap
**41 MB** (7 MB idle), **compartments 53 system + 3 user with no card open** — and 53 + 6 while
the second card was open: every closed page leaves its window's compartments behind. The daemon's
page teardown does not release the window; the allocator was never the story. Fix: in the
daemon's close path, close the content window and destroy the web browser/docshell when a card
goes (then the collections free them). Measured next.

**The memory report, read (2026-10-16 03:00; `tools/host/memory-report-read.py` on the dump v9.1
writes after the last card):** resident 161 MB, JS GC heap 33 MB, **one detached window — the
last page (Bluesky, 16.5 MB), still being torn down; ghost windows 0**; YouTube's and Reddit's
windows are gone. So the daemon does release closed pages; the "zombie" reading was the last
page's teardown caught early. What stays: explicit memory of about 65 MB (compiled script data
14.5 MB, the blocker's host blob 3.6 MB, atoms 4.1 MB, the strings cache 3.8 MB, sqlite 1.4 MB,
ICU 1 MB) and, beyond it, the allocator's retention and libxul's paged-in code — a build without
jemalloc neither reports its heap (`heap-allocated` is unavailable) nor returns fragmented pages.
The device sat at 280–370 MB free after the closes at the 64 MB threshold and never refused a
card. **The one lever left for resident memory is an engine rebuild with jemalloc** (an hours-long
build, cavekit-engine-update.md): proposed as T-493, the owner's call. The per-card purge and v9's
collections stay (they bring a 428 MB daemon to 170 MB within seconds of the last close).

## T-494 — the challenges (2026-10-16; the owner: "Try to fix the challenges blocking sites from loading including cloudflare and RECAPTCHA"; the base UA stays `Firefox/52.9`)

**reCAPTCHA (Google's `/sorry/`):** past already — google.com's row in the daemon's UA table
(a site-compat entry like the thirty others) makes Google serve its results page (run 2c).

**Cloudflare's interstitial — what was established:**
- HTTP/2 is on (UXP default, nothing of ours turns it off); cookies accepted (`cookieBehavior` 0);
  referers sent.
- `jihad-challenge-probe.html` from a page: a no-cors fetch to example.org returns an opaque
  response, a beacon queues, an XHR fails on CORS as it should, the image and script "errors" are
  example.org's own 404s — the engine's load policy does not refuse the challenge's connectivity
  probe at page level. The earlier "may not load data from https://example.org/" came from inside
  the challenge frame; the null-principal case is probed next (a `data:` frame that reports).
- With the console on all sources, w3.org and Medium show NOTHING from the challenge frames in
  60 s beyond CSP warnings for `trusted-types` (an unknown directive, harmless): no script error,
  no verdict line; the frame runs and does not pass.
- Pale Moon's long thread on the same loop (forum t=32045, 2025–2026): Cloudflare's own position is
  that no browser-side release fixes it; a participant reproduced the loop in Firefox by disabling
  HTTP/2 and sending Pale Moon's user-agent — the check keys on the transport and the token;
  browsers "more than five years old" get "limited support" by Cloudflare's own page.
- **A signal of our own making, found in the daemon:** the per-host table rewrites the User-Agent
  header for `challenges.cloudflare.com` to Firefox/128, while `navigator.userAgent` inside that
  frame is the top document's (the docShell pin is inherited): header and script disagree inside
  the challenge — a textbook bot signal. v9.2 of the blocker component carries an experiment
  (`jihad.ua.consistentChallenge`): the header for that host set to the loading window's own
  `navigator.userAgent`. Measured next on w3.org, Britannica and Medium.

**Measured (2026-10-16 11:45–11:55, v9.2):** the null-principal frame's loads to example.org
behave exactly as a page's (no-cors fetch opaque, iframe loaded, script and image 404, XHR on
CORS, storage a SecurityError as it must be): the engine's load policy is cleared for the
challenge's probes. Making the challenge frame's header match its script identity changed
nothing: w3.org and Britannica stayed on the interstitial ("Why is this verification taking
longer?"); Medium's visit hit the launch race. Next, the decisive experiment on the cause: the
whole page served a Firefox/128 identity (header and docShell pin, frames inheriting), pref
`jihad.ua.experimentHosts` — if the challenge then passes, it keys on the 52.9 token (a 2017
browser's token on a 2026 TLS stack), and the base UA is the trade-off; if it still fails, the
token is not the lever.

**The decisive experiment (12:00–12:10): the whole page given a Firefox/128 identity, header and
script together — w3.org renders (the W3C home), Britannica renders (the Palm Inc. article), npm
renders (the express package, README and versions); Medium alone stays on the interstitial.**
The token is the lever: Cloudflare's check rejects a 2017 browser's token on this engine; given a
current one the challenge passes on three of four. The owner's rule, "52.9 by default but adjust
based on the needs of the page", becomes **T-495, the learned identity**
(`render/goanna/components/jihadUserAgent.js`): a top-level response marked
`cf-mitigated: challenge` puts the site's base domain on `jihad.ua.modernHosts` (persisted), the
docShell is pinned and the page loaded again once with the current identity; from then on the
site's requests and documents carry it. The daemon's table stays the hand-curated part. Settings →
General → Browser identity: the switch, the remembered sites, Forget. Medium: open (its challenge
level is higher, or the day's repeated visits from this address count against it).

## T-493 — the engine rebuilt with jemalloc (2026-10-16; the owner: "Do the engine rebuild with jemalloc")

**The build:** `mozconfig.goanna-arm` carried `--disable-jemalloc` from the first ARM config with
no reason recorded; now `--enable-jemalloc`. The rebuild took **22 minutes**, not the hours the
plan feared (incremental: `memory/` compiled, libxul relinked; `build/webos-oe/out-arm/
engine-jemalloc-build.log`). **The trap, found at the daemon's link:** on Linux UXP builds the
allocator INTO THE PROGRAM (`MOZ_GLUE_IN_PROGRAM=1` in the obj dir's `config/autoconf.mk`) — there
is no `libmozglue.so`, the dist's `libmozglue.a` and `libmemory.a` are 33-byte expandlibs stubs
(their `.a.desc` beside them lists the objects), and `libxul.so` leaves `malloc`, `free` and
`jemalloc_stats` UNDEFINED for the executable to export. The first link line written (`-lmozglue`
first, dynamic) was wrong; `build-daemon-arm.sh` now expands the two descriptors as xpcshell's
`backend.mk` does (`libmozglue.a` → arm, mozalloc, mozglue_misc, mfbt; `libmemory.a` → jemalloc,
mozmemory_wrap, jemalloc_config), links them with `-rdynamic`, and checks with nm that the daemon
exports `malloc … jemalloc_stats`. An unversioned `malloc` in the executable satisfies libxul's
`malloc@GLIBC_2.4` reference, and every library in the process binds to it — the one allocator
the daemon's frees of engine-allocated buffers need. The daemon's three `malloc_trim` calls became
`jihadPurgeHeap` (`jemalloc_free_dirty_pages` + the glibc trim for the loader's arena); its log
line says what was purged. A libxul from this config with the previous daemon would die at the
first `jemalloc_stats` (lazy binding: the memory report after the last card) — the two ship
together. Daemon 11 `4c318d49`. Measured next: idle RSS, the three heavy pages, the purge at the
last close, the dump with `heap-allocated`, the benchmark.

**T-495 measured on the device (2026-10-16 12:15–12:25, Mochi, daemon 10 with the component
installed by hand; `docs/rounds/t495-2026-10-16/`):** w3.org — turned away, the daemon logs
"the site's bot check turned the page away (cf-mitigated: challenge); learned w3.org, loading
again with a current Firefox identity", the page loads again by itself and **renders** (the W3C
home). Britannica and npm — no challenge this visit (their clearance cookies from the
experiment an hour earlier still stood), rendered. **Medium — challenged, learned, reloaded,
renders** (the Technology topic page: the learned retry passes where the whole-page experiment
had not). `prefs.js` holds `jihad.ua.modernHosts = "w3.org,medium.com"`; after a daemon
restart w3.org loads straight away with one load and no challenge. Settings → General → Browser
identity: the switch, "2 sites given a current Firefox identity: w3.org, medium.com", the Forget
button (a finger test, `docs/FINGER-TESTS.md`). Left for T-496 that day: the no-loop branch
(no site challenged twice) and the challenge host's own header on the wire.

**T-496 — the identity read on the wire; the second signature (2026-10-16 12:13–12:25, Enyo,
component `e3628b84`).** `render/goanna/test/jihad-identity-server.py` is a site on the host PC
(port 8078) that behaves like the bot checks: it logs every request with the User-Agent header it
carried, every page reports its `navigator.userAgent` back, `/challenge/` answers EVERY load with
`cf-mitigated: challenge`, `/sp-gate/` answers any token but a current Firefox's with Startpage's
303 to `/sp/captcha-block`, `/site/` has a same-site frame, an image, a script, an XHR and a
third-party frame (httpbin.org/user-agent, which prints the header it received). Measured, from
the server's log and the shots (`docs/rounds/t496-2026-10-16/`):
- **Baseline** (`/plain/`, nothing learned): the top document, its frame, the image, the script
  and the XHR all carried the default identity (`… Goanna/6.9 UXP/5f290d1 Firefox/52.9
  ECMAScript/2024 JihadBrowser/1.0`), and both documents reported it as `navigator.userAgent`.
- **The no-loop branch**: `/challenge/` was loaded twice, one second apart — the default identity,
  then the current one — and the second answer was shown as it is (the shot: "CHALLENGE PAGE …
  served to Firefox/128", `navigator.userAgent` 128); the daemon logged "learned 192.168.1.245,
  loading again" and then "challenged again with the current identity; the page is left to the
  site". A new card on the same page 46 s later made ONE load, shown at once (the one retry per
  ten minutes).
- **The frames and subresources of a learned site** (`/site/`): the top document, the same-site
  frame, the image, the script and the XHR all carried the current identity in the header; the
  top document and the frame both reported it as `navigator.userAgent`; and **the third-party
  frame — httpbin.org under the learned page, the shape of challenges.cloudflare.com under
  w3.org — printed the current identity as the header it received.** The claim R3 made on
  indirect evidence is now read on the wire.
- **Settings → General** showed "1 site given a current Firefox identity: 192.168.1.245".
- **The second signature, on the real site**: `https://www.startpage.com/` — the daemon logged
  "www.startpage.com: the site's bot check turned the page away (a redirect to /sp/captcha-block);
  learned startpage.com, loading again with a current Firefox identity" and **Startpage's home
  renders** (the shot: the search box, "The world's most private search engine"). The search
  (`/do/search?q=webos+touchpad`) is then met by **Anubis** (Techaro's proof-of-work check,
  `main.mjs` v1.26.4: a Worker hashing with `crypto.subtle`, plain bundled JavaScript — it loads
  and runs here): "Verifying your request…" with its progress bar half way after 90 s; left to run (Enyo,
  12:58–13:04, a shot every 30 s): **the proof of work finished between 210 and 240 s and the
  results page rendered** (the shot `sp-240`…`sp-360`: the results for "webos touchpad"). Anubis
  keeps its pass in a cookie (a week by its default), so the four minutes are paid once per
  profile per week; the TouchPad's two cores do what a laptop does in a second or two.
- **eBay and old Reddit are not the identity's**: curl from the host PC gets eBay's Akamai 403
  page with either token, over HTTP/2 and 1.1, with a browser's full header set; old Reddit
  redirects curl to `/login/?reason=lor2` with either token. Rows 9 and 11 say so.
- The harness address was taken off the learned list afterwards (the daemon stopped, the pref
  edited); `startpage.com` stays learned on Enyo as product state.

**T-497 — YouTube, sped up (2026-10-16 12:33–13:30, Mochi, daemon 12/13; the owner: "speed up
youtube").** What YouTube hands the engine, measured from the host with the engine's own download
behaviour (`curl --compressed`, the page plus every external script):

| page | on the wire | decompressed | external scripts | inline script |
|---|---|---|---|---|
| `www.youtube.com/watch?v=…` (desktop, Firefox/128 token) | 3.6 MB | **13.7 MB** (the Polymer bundle 10.9 MB, the player 2.6 MB) + a 1.3 MB page | 10 | 0.55 MB |
| `m.youtube.com/watch?v=…` (mobile Firefox token) | 1.07 MB | **3.0 MB** + a 0.73 MB page | 3 | 0.44 MB |

On the device (the round script's metrics: first paint, load-done, the paint pipeline's worst
tick = the longest main-thread stall, resident memory), the same watch page:

| visit | first paint | load-done | worst stall | RSS | the shot |
|---|---|---|---|---|---|
| A: as shipped (desktop 128, blocker on, cold cache) | 7 s | none in 65 s | **24.4 s** | 274 MB | grey skeletons; only the pre-roll ad plays |
| B: the same, blocker off (warm cache) | 7 s | 37 s | 11.1 s | 207 MB | — |
| A2: as shipped again (warm cache) | 11 s | (log rotated) | — | 216 MB | still grey skeletons after 84 s, the ad at 0:11 |
| C: **mobile identity → `m.youtube.com`** (blocker on) | 12 s | 34 s | **5.3 s** | **162 MB** | the player with its poster, "Open App" |
| C: `m.youtube.com/` (the home feed) | **1 s** | **18 s** | 10.6 s | 228 MB | the feed rendered: chips, thumbnails, titles |

A versus B is confounded by the cache (A downloaded the 13.7 MB); the blocker's share is at most
the difference and is not the lever. The lever is the page itself: a quarter of the script. So
youtube.com now gets a **mobile Firefox identity** (`jihad.ua.sites` in the platform prefs,
applied by `components/jihadUserAgent.js` to the header AND `navigator.userAgent`, frames and
subresources included; the daemon's desktop row for youtube.com is out of `EngineHost.cpp`).
Settings → General → "Sites with a set identity" says so. Playback: the desktop page's pre-roll
ad PLAYED (video decoding works on this build for that stream); the mobile page waits for a tap
on the poster, which a script cannot give — a finger test (`docs/FINGER-TESTS.md`).

**T-498 — the web compatibility shims (2026-10-16; the owner: "fix the regular duckduckgo").**
`render/goanna/components/jihadWebCompat.js`: at `document-element-inserted` a sandbox with the
page's principal defines on the page's own prototypes what the engine lacks and modern pages
assume — each shim only where the engine has no native member, each small enough to read:
`CSSStyleSheet.prototype.replace` (the engine has the constructor, `replaceSync` and
`adoptedStyleSheets` — Lit tests `replace` and Etsy's components then hand `adoptedStyleSheets`
undefined sheets: 47 TypeErrors per search page), `reportError`, `Intl.ListFormat` (English
forms), `Element.checkVisibility`, `AbortSignal.any`, `URLSearchParams.size`, `Array.fromAsync`,
the `inert` property, and the **popover** attribute with `showPopover`/`hidePopover`/
`togglePopover`, `popovertarget` buttons, light dismiss, `beforetoggle`/`toggle` events — with one
AGENT sheet for the two attributes' UA rules (`[popover]` hidden until shown; `[inert]` without
pointer events). Prefs `jihad.compat.enabled` (on) and `jihad.compat.trace` (off: a diagnosis mode
that wraps the engine calls which can throw an XPCOM result to a page and logs the name and the
page's own stack — the console bridge sees such a throw only as "NS_ERROR_NOT_INITIALIZED: " at
column 0, because the engine's Exception object has no column). What the engine HAS (checked in
the engine's source, `third_party/uxp`, not guessed): `<dialog>`, `replaceChildren`,
`structuredClone`, `crypto.randomUUID`, `toSorted`/`toReversed`/`findLast`, `Object.hasOwn`,
`Object.groupBy`, `Promise.withResolvers`, `FinalizationRegistry`, `AbortSignal.timeout`,
`URL.canParse`, `MediaSource`, `OffscreenCanvas`, `ResizeObserver`, CSS `:is`/`:where`,
`color-mix()`, `inset-inline`, `dvh`; and `:has()` behind `layout.css.has-pseudo.enabled`
(off upstream by default — **on in Jihad since 2026-10-16, all three variants, the owner's
call: "enable :has() by default in all 3 variants"**; the platform prefs carry it; a rule whose
selector uses `:has()` was dropped whole before, DuckDuckGo's stylesheets alone use it 33 times). What it lacks beyond the shims: container queries,
`backdrop-filter`, `content-visibility`, `text-wrap`, `accent-color`, `Intl.Segmenter`,
`navigator.locks`, `CompressionStream`, `requestVideoFrameCallback`, `ElementInternals`.
**Measured (three trace rounds on Mochi, 2026-10-16 13:30–14:30, daemon 13/14, console on,
`jihad.compat.trace` on):** the shims install on every page ("compat: <host>: CSSStyleSheet.replace,
reportError, Intl.ListFormat, …, popover") and their own test page
`render/goanna/test/jihad-compat.html` passes 16/16 (popover hidden until shown, shown, toggled,
its events; inert; replace; ListFormat; reportError; checkVisibility; AbortSignal.any; size;
fromAsync). **Etsy's search renders fully styled** — the 47 `adoptedStyleSheets` errors are gone,
7 unrelated lines remain (WebGL refused, a consent script's own TypeError). **Bluesky's two
"NetworkError when attempting to fetch resource"** are the blocker refusing `events.bsky.app`
(telemetry) and `sentry.io` — EasyPrivacy, as designed. The second shim batch (`Blob.text`,
`Response.json`, `Promise.try`, the Set methods, the iterator helpers, ReadableStream iteration,
`crypto.randomUUID`, `scheduler.postTask`, `requestVideoFrameCallback`, `canvas.roundRect`)
came from the API probe's full list (169 probes, the page posts its list to the harness server:
missing are also CSS container queries, `backdrop-filter`, `content-visibility`, the individual
`translate` property, `scroll-snap-type`, `overscroll-behavior`, `text-wrap`, `accent-color`,
`::marker`, `subgrid`, nesting, `image-set()`, `Intl.Segmenter`/`DisplayNames`,
`CompressionStream`, `OffscreenCanvas`, `ElementInternals`, `navigator.clipboard`/`locks`/
`share`/`userAgentData`, `ServiceWorker`, `Symbol.dispose`, the regex `v` flag, WebGL — and in
MSE **no H.264/AAC**, only WebM/VP9/Opus: `MSE mp4/avc1`, `MSE audio mp4a`, `video canPlayType
mp4 h264` all missing). The third round caught the component's own two faults — the embedded sources were not strict,
so a page's bare `matchMedia()` reached the wrapper with `this` set to the sandbox global, and the
iterator helpers had been installed on the sandbox's own iterator prototype — fixed (strict
sources; the page's `%IteratorPrototype%` reached through the page's `Array`), dry-run on the
host; the shims' page on the device after that: **28/28 ok** (14:55, Mochi, the shipped component `47c33ff6`), the API probe 131/169 with `:has()` on (119 before the shims). **The
`NS_ERROR_NOT_INITIALIZED` thrower** (AP, Etsy's DataDome, Vimeo, GitLab, old Reddit, Bluesky in
round 2) did not throw again in the two trace rounds with the wrappers in place — Vimeo's chunk
changes name with every deploy, the other pages' bundles too; the trace mode stays as the tool
for the next time it does (`jihad.compat.trace`, and the console line's column).

**T-499 — DuckDuckGo's menu panel: the daemon's fit-zoom shows a page's off-canvas parking
(found 2026-10-16 14:10, OPEN, designed).** The trace probe walked the open panel's ancestors:
`div.nav-menu--slideout` is `position: fixed` at x = 772 in a window whose `innerWidth` is 768 —
DuckDuckGo parks its slide-out menu four pixels past the right edge with `translateX(262.8px)`,
as every browser hides it. But the page lays out 1000 CSS px wide (its right rail), and this
daemon's design is "the engine's layout viewport is the window width; content wider than it is
fit-zoomed by the card" (`BrowserPageGoanna.cpp` emitGeometry, `GoannaRenderPage.cpp` SetZoom):
the card shows the whole 1000 px at zoom 0.768, so the strip past `innerWidth` — where pages
park things — is on screen. Not DuckDuckGo's doing, and not the popover's: any page that hides
a drawer just past its right edge shows it here when its layout is wider than the window. The
engine's `nsIDOMWindowUtils` has no CSS-viewport setter; the fix is an offscreen engine window
as wide as the page's overflow (once per load: 1000 wide for DuckDuckGo, `innerWidth` 1000, the
drawer parked at 1004, the card's fit-zoom unchanged) with the card's own size kept apart from
the engine's in the pan and floor arithmetic — a daemon change through the code whose comments
record the 2026-07/08 zoom-oscillation and rotation bugs, so a task of its own with device
trials, not a patch for this round. Until then the regular DuckDuckGo page shows its results
with the menu panel standing at the top (the shot `t-ddg`), and DuckDuckGo Lite stays the
default search.

**Measured with daemon 11 (2026-10-16 03:33–03:40, Mochi, the same three pages as daemon 10's
run; `memory-report-read.py` on the dump):**

| | daemon 10 (glibc) | daemon 11 (jemalloc) |
|---|---|---|
| idle, no card | 34.8 MB | 36.0 MB |
| YouTube open | 163 MB | 195 MB |
| Reddit open (YouTube's card closed) | 339 MB | 333 MB |
| Bluesky open | 428 MB | 422 MB |
| after the last close, purge + collections | 170 MB (226 before v9's collections) | **149 MB**, still 149 a minute later |
| `heap-allocated` | unavailable | **60.3 MB** (bin-unused 17.9, page-cache 10.5, bookkeeping 1.8) |
| ghost windows / detached | 0 / the last page | 0 / the last page (Bluesky, 16.9 MB) |

So the allocator was worth about 20 MB after the closes and nothing while a page is open (the
pages' own working set is the pages'); above all the heap is now REPORTED: of 149 MB resident,
60 MB is the malloc heap (28 MB of it jemalloc's own overhead), 25 MB the JS GC heap's committed
chunks (the component's `JSMainRuntimeGCHeap` figure, 32 MB, counts the decommitted ones too),
explicit memory 110 MB in all, and the rest libxul's mapped code and the engine's non-heap maps;
jemalloc has 314 MB of address space mapped, most of it not resident. The dump and the reader's
output: `docs/rounds/t493-2026-10-16/`. The purge order is right: the
component's collections fire `jihad-blocker-installed`, so the daemon's `jihadPurgeHeap` runs
after them (two "heap purged" lines per last card). Shipped besides: `memory.free_dirty_pages`
true, so every memory-pressure notification returns jemalloc's dirty pages as well. The device
sat at 415 MB free after the closes. The kit's 12 MB idle goal stays unmet (36 MB idle is the
engine's floor with the components loaded); R6's row is closed as measured.

**The benchmark under daemon 11 (03:50, `jihad-jsbench.html`, the JIT and the GC prefs unchanged):**
loop 50M 554 ms; objects 2M 1,464 ms (1,186 under daemon 10 — the GC heap, not malloc, carries
objects; run-to-run noise on a device with three daemons); strings 200k 1,150 ms (1,304); regex
100k 152 ms (178); DOM 20k 5,312 ms (no earlier figure). The allocator moved nothing the user
feels; its gain is the 20 MB after the closes and the heap that reports.
