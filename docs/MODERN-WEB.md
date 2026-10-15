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

The per-site rows (cause and disposition each), the fixes (T-492) and the second run are the next
session's; the analysis table is the ledger's input.
