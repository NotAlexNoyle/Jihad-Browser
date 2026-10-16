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
`[jihad-bs] js-console <source>:<line> <message>` — but by default only chrome://, resource://
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
| `https://duckduckgo.com/?q=webOS&ia=web` | Mochi | at the engine's 15 s guard: "A script on this page may be busy" over a half-built page (shot `mochi-wiki`, 10:30). With the guard at 60 s (`dom.max_script_run_time`, T-481): no dialog, but the page is wrong — its slide-out menu drawn as a plain list on a white page, no results (shot `t481-ddg`, 10:47) | **engine feature** (to be named with the console): DuckDuckGo's full results page is a JavaScript application the engine cannot finish; the slow-script dialog was only the first symptom | **workaround, in place**: Jihad searches DuckDuckGo **Lite** (`lite.duckduckgo.com`) by default, which is plain HTML and renders its results (shot `t481-lite`, 11:56); the full page stays *open* |
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
| 9 | eBay search | eBay's **"SORRY, something went wrong"** page (shot); its bot script probes WebGL | **bot detection** (eBay) | open: the UA question |
| 10 | Etsy search | **"Verification Required"** (DataDome, shot); `captcha-delivery.com` throws NS_ERROR_NOT_INITIALIZED | **bot challenge** (DataDome) + the engine API | open: the UA question |
| 11 | old Reddit | redirected to **`old.reddit.com/login/?reason=lor2`** (a login gate Reddit now puts in front of old-reddit for unknown clients), whose form never rendered: the Snoo pattern background only (shot); 17.5 s stall | **bot/visitor gate** (Reddit) + the login page's app not rendering | open: the UA question, then the page |
| 12 | Reddit | never load-done; 32.5 s stall; **renders r/webos with its posts** (shot) despite "Deferred script … nonce" and es-module-shims | slow script; the module shim compiles its own loader (asm.js) | works, slowly |
| 13 | Bluesky | a **blank page with the Bluesky footer** (shot); 37.5 s stall | **engine feature**: a React app on ES modules | upstream |
| 14 | YouTube watch | **"Please update your browser"** page (shot) | **user-agent sniffing**: YouTube rejects the Firefox/78 token the UA table sends it | open: the UA question |
| 15 | Vimeo staff picks | the sidebar renders, the content area stays blank (shot); `NS_ERROR_NOT_INITIALIZED` ×5 from its Next.js chunk | **engine feature**: the same API | open → T-492 |
| 16 | Stack Overflow tag page | first paint 13 s, load-done 21 s; 23.8 s stall; **renders** the question list (shot) | slow script | works, slowly |
| 17 | GitLab explore | the frame renders ("Explore projects", filters) with an empty list (shot); 43.4 s stall; jQuery throws NS_ERROR_NOT_INITIALIZED | **engine feature**: the same API; the list is fetched by script | open → T-492 |
| 18 | npm package page | the card shows **Jihad's start page**: the address was never opened (shot) | a launch race (the card launched before the previous one had closed, as The Verge's first run) — not the site | retest in the second run |
| 19 | webos-ports.org | first paint 38 s, never load-done; a **125 s** stall; **renders** LuneOS's page (shot) | slow script — the worst of the round, on a WordPress theme | open → T-492 |
| 20 | webosnation.com | **redirects to techradar.com**, which renders (shot) — the site was folded into TechRadar | not a browser fault | works |
| 21 | Startpage results | **"Your connection has been suspended"** (shot): Startpage refuses the client as automated traffic | **bot detection** (Startpage) | open: the UA question |
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
(no GPU driver). So the `NS_ERROR_NOT_INITIALIZED` that AP News, Etsy's DataDome, Vimeo and
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
