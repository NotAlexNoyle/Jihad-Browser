---
created: "2026-10-15"
last_edited: "2026-10-15"
---

# Cavekit: The Modern Web on Jihad

## Scope
What happens when Jihad Browser opens today's web on the device, written down site by site, and
the fixes that record drives. The engine is UXP (Goanna 6.9, a Firefox-52 lineage with Pale Moon's
additions); Palefill fills some of what it lacks (cavekit-addons-extensions.md R7). This kit is the
ledger of where that is enough, where it is not, and why — so a failing site is a known, named
cause with a disposition, not a surprise in a screenshot.

**User requirement, 2026-10-15:** *"showcase the modern web"*, then, on three sites failing in one
screenshot round: *"dont just breeze past that. document and fix issues loading the modern web."*

### What this session found (the seed of the ledger)
On 2026-10-15, taking the README's screenshots on the TouchPad:

| Site | Variant | Result | Cause (as known) |
|---|---|---|---|
| `duckduckgo.com/?q=webOS` (results) | Mochi | the "A script on this page may be busy" dialog over a half-built page | the results bundle (`wpm.*.js`) runs longer than `dom.max_script_run_time` (UXP default 15 s) on the 1.2 GHz ARM; the dialog is the engine's slow-script guard, not a crash |
| `github.com/NotAlexNoyle/Jihad-Browser` | Mojo | the page frame renders (header, file list, About, Releases headings) but the commit messages and times, the README body, Releases/Packages/Contributors stay grey loading bars after 75 s | GitHub fills those with JavaScript after load (`include-fragment`/deferred partials); the engine runs the page but not those; no error reaches the device log because the daemon's console bridge drops content-page errors by design (EngineHost.cpp F-8) unless `JIHAD_JS_CONSOLE=all` |
| `www.w3.org` | Enyo | Cloudflare's "Performing security verification" page, "Why is this verification taking longer?", never passes | Cloudflare's challenge; Pale Moon users report it is user-agent sniffing (a per-host UA override for `challenges.cloudflare.com` passes it in Basilisk); Jihad's UA token is unknown to it |
| `developer.mozilla.org/en-US/docs/Web/CSS` | Enyo | renders well, with an MDN promo banner and a Twilio advert in the sidebar | works; the adverts are the ad-blocking kit's business (cavekit-content-blocking.md) |
| `en.m.wikipedia.org/wiki/WebOS` | Mojo | renders the DESKTOP article (`en.wikipedia.org`), small type on a 768-px card | the UA override table sends a Firefox/78 desktop UA for `wikipedia.org`, so Wikipedia serves desktop; readable but not the mobile layout the address asked for |
| `en.wikipedia.org/wiki/Goanna_(software)` | Enyo | Wikipedia's "no article" page | there is no such article (a trap for screenshots, not a browser fault) |
| `www.webosarchive.org`, `pivotce.com`, `www.howsmyssl.com`, `archive.org/download/…/flash_bloxorz.swf` | all | render / play | — |

A second survey of six sites in Mojo (codeberg.org, archive.org, mozilla.org, mastodon.social,
theverge.com, bbc.com/news) is recorded in `docs/MODERN-WEB.md` as it comes in.

## Requirements

### R1: The compatibility ledger
**Description:** `docs/MODERN-WEB.md` is the one place that says how a site behaves in Jihad on the
device: the address, the variant, the date, the engine build, what was seen (a screenshot name
under `docs/screenshots/ledger/` or the session's shots), the cause by category — *engine feature*
(JavaScript or CSS the engine lacks, with the feature named when the console says it), *user-agent
sniffing*, *bot challenge*, *slow script*, *adverts and trackers*, *layout on 768 px*, *not a
browser fault* — and the disposition: fixed (how, where), workaround (what the user can do),
open (why), or upstream (Palefill, UXP).
**Acceptance Criteria:**
- [x] The ledger exists with every site of the table above and of the six-site survey, each row complete (address, variant, date, build, result, cause, disposition). *(2026-10-15, `docs/MODERN-WEB.md`, 19 rows.)*
- [x] Every failure row names its cause category, and an *engine feature* row names the feature or the exact console error (from `JIHAD_JS_CONSOLE=all`), not a guess. *(GitHub: `Turbo is not defined`; Mastodon: `import.meta may only appear in a module`; DuckDuckGo's full page: not yet captured — its row says so.)*
- [ ] Every fix landed by R2–R4 is re-tested on the device and its row updated with the result and the screenshot.
- [ ] The README's "Known issues" line links to the ledger.
**Dependencies:** none

### R2: The slow-script guard fits the device
**Description:** UXP stops a content script after `dom.max_script_run_time` seconds (15) and asks the
user. On the TouchPad's CPU, pages that finish in 3 s on a desktop take 20–40 s, so working pages
are interrupted. The threshold becomes device-sized without removing the guard.
**Acceptance Criteria:**
- [x] `packaging/prefs/` sets `dom.max_script_run_time` to a value chosen from measurements on the device (the DuckDuckGo results page as the reference; 60 s expected), with the reason in a comment; the guard is NOT disabled (0). *(2026-10-15: `jihad-platform-prefs.js`, 60 s; the reference became The Verge, whose Next.js chunk brought the dialog at 15 s and renders in full at 60 — DuckDuckGo's full results page turned out to be an engine-feature case, not a timing one, see the ledger.)*
- [~] The DuckDuckGo results page for a query renders on the device without the dialog, in all three variants (screenshot in the ledger). *(The full page does not render for another reason; DuckDuckGo Lite, Jihad's default search, renders (`t481-lite`). The Verge renders at 60 s in Mojo (`t481-verge`); the other two variants carry the pref from the package, not yet re-shot.)*
- [x] A deliberately endless script (`render/goanna/test/jihad-busy.html`, `while(true){}`) still brings the dialog, and "Stop script" recovers the card. *(2026-10-15, Mochi: the dialog names the script (`t481-busy`); the recovery tap is a finger test, docs/FINGER-TESTS.md.)*
**Dependencies:** none

### R3: The browser's identity, adjusted to the page (Cloudflare's challenge and the like)
**Description:** The browser is `Firefox/52.9` by default (`JihadUserAgent.h`; the owner,
2026-10-16: "52.9 by default but adjust based on the needs of the page"). Sites whose bot check
rejects a 2017 token on this engine — Cloudflare's interstitial (w3.org, Britannica, npm, AP,
Medium), Google's `/sorry/`, Bing's captcha, DataDome (Etsy), YouTube's "update your browser" —
get a current Firefox ESR identity in two ways: the daemon's hand-curated table
(`render/goanna/EngineHost.cpp`, ported from Basilisk's, with the four rows round 2 proved), and
the learned list of `components/jihadUserAgent.js`: a top-level response marked `cf-mitigated:
challenge` puts the site's base domain on `jihad.ua.modernHosts`, the docShell is pinned and the
page loaded again once; from then on the site's requests and documents carry the identity,
header and `navigator.userAgent` together (the two apart was measured to fail). Settings → General
shows and forgets the learned sites; `jihad.ua.learn` turns the learning off.
**Acceptance Criteria:**
- [x] The daemon's `challenges.cloudflare.com` row is OUT of the UA table: a modern header on the challenge host against a frame whose `navigator.userAgent` is the top document's was an inconsistency of our own making (measured 2026-10-16: it never passed); the learned identity covers the challenge host as a learned page's subresource.
- [x] `https://www.w3.org/` opens past the verification on the device (screenshot in the ledger). *(2026-10-16 12:00, with a whole-page Firefox/128 identity: w3.org, Britannica and npm render; Medium does not.)*
- [x] The learned identity works end to end on the device: a first visit to a challenging site is turned away, the daemon logs "learned <site>", the page loads again by itself and renders; the site is on `jihad.ua.modernHosts` after a daemon restart and loads straight away; Settings → General shows it. *(2026-10-16 12:15–12:25, Mochi: w3.org and Medium learned and rendered; `w3.org,medium.com` persisted; w3.org straight after the restart; the General pane shows both. Forget is a finger test, docs/FINGER-TESTS.md.)*
- [x] A learned site's frames and subresources carry the identity too (the header on the challenge host's requests, `navigator.userAgent` inside the frame). *(2026-10-16 12:17, Enyo, read on the wire by the harness `render/goanna/test/jihad-identity-server.py`: on the learned host the top document, its frame, image, script and XHR all carried the current identity in the header, and the top document and the frame both reported the same `navigator.userAgent`; the baseline page before the learning carried the default identity everywhere, header and script agreeing.)*
- [x] A site that challenges again with the current identity is left alone (one retry per ten minutes, no loop); the log says so. *(2026-10-16 12:14, Enyo, the harness's `/challenge/` page answers every load with `cf-mitigated: challenge`: the server saw exactly two loads one second apart — the default identity, then the current one — and the second answer was shown as it is, `navigator.userAgent` current; a new card on the same page 46 s later made ONE load and was shown at once.)*
- [x] A second signature learns a site whose own gate redirects the page to its captcha page (Startpage's `/sp/captcha-block`), and the retry passes a gate that lets a current token through. *(2026-10-16 12:23, Enyo: the harness's `/sp-gate/` on a host not yet learned — the default token got the 303, the retry six seconds later carried the current one and passed; the real `startpage.com` learned the same way, its home rendered, and its search finished Anubis's proof of work in about four minutes and rendered the results.)*
- [x] A site can be given a particular identity by rule, header and `navigator.userAgent` together (`jihad.ua.sites`): youtube.com gets a mobile Firefox identity and YouTube serves its mobile site. *(2026-10-16, T-497: the desktop watch page's 24 s worst stall and 274 MB against the mobile site's 5 s and 162 MB; the home feed rendered in 18 s; the ledger has the tables.)*
- [x] No Pale Moon, Basilisk or Moonchild token is put in any UA string (cavekit-licensing-branding.md R3): the override uses a Firefox token like the rest of the table. *(The learned identity is `Firefox/128.0` on `X11; Linux armv7l`, `DEFAULT_MODERN` in jihadUserAgent.js.)*
**Dependencies:** none

### R4: GitHub's deferred parts
**Description:** GitHub's repository page renders its frame but not its deferred parts. The cause
is found from the console with `JIHAD_JS_CONSOLE=all` on the device (the job's environment, or the
daemon started by hand from a shell with the variable), then dispositioned: a Palefill limit
(reported upstream, cited), a user-agent choice (the table sends Firefox/78 for `github.com`; a
newer token may change what GitHub serves), or an engine feature (named, left to the engine).
**Acceptance Criteria:**
- [x] The ledger's GitHub row names the exact errors or the missing feature from the console output. *(`ReferenceError: Turbo is not defined`; ES-module loading, as Mastodon's `import.meta` error shows.)*
- [x] If a configuration change (UA token, a pref) fills the parts, it is applied and device-verified with a screenshot; otherwise the row is *upstream* or *open* with the named cause. *(upstream: UXP's module loading; no configuration applies.)*
- [x] `docs/MODERN-WEB.md` tells a developer how to get content-page console output on the device (`JIHAD_JS_CONSOLE=all`, and that it is off by default because page text would reach the persistent log). *(With the log-rotation caveat: window by line number after the newest "bridge on".)*
**Dependencies:** none

### R5: Wikipedia on a 768-px card
**Description:** The UA override makes Wikipedia serve desktop pages. Decide and record: keep the
desktop layout (it is readable, and the override exists for a reason) or let the mobile address
stay mobile. Either way the ledger row says why.
**Acceptance Criteria:**
- [x] The decision is in the ledger with its reason; if the mobile layout is chosen, `en.m.wikipedia.org` keeps its mobile layout on the device (screenshot). *(2026-10-15: desktop kept; the reason in the ledger.)*
**Dependencies:** none

### R6: A user's half hour
**Description:** Not a list of home pages but a session: about thirty visits as a user would make
them — a search and its results, an article, a product search, a subreddit, a video page, a
question page, a package page, a map, a weather forecast, a film page, the community's sites —
each given the time a user would give it, in one interface with the console bridge on all sources,
every visit logged (what loaded, how many frames painted, what the blocker refused, what the page
threw) and shot. Every issue found gets a ledger row, a cause and a disposition; the ones
configuration or the browser's own code can fix are fixed and re-run.
**User requirement, 2026-10-15:** *"set up a new round of tests where you browse the modern web as
a pretend user for about 30 minutes. document any issues you find and then tackle them."*
**Acceptance Criteria:**
- [x] The round's script and its site list are in `tools/device/` (not the session scratchpad), re-runnable; the round takes 25–35 minutes on the device. *(`browse-round.sh`, `browse-round-sites.txt`, `browse-round-analyse.py`; `tools/host/run-browse-round.sh` runs it end to end; the first run took 40 min with its restarts.)*
- [x] The ledger has a "Round 2" section with one row per visit, each with the log's numbers (load, paints, refusals, errors), the shot, the cause category and the disposition. *(2026-10-15: 30 rows; the numbers in `docs/rounds/round2-2026-10-15-analysis.md`, the shots in `docs/rounds/round2-2026-10-15-shots/`. 17 render, 7 stopped by bot detection, YouTube by UA sniffing, 2 engine-feature blanks, 2 the test's own faults.)*
- [~] Every issue the round found is either fixed and re-verified (its row says how and shows the re-run) or dispositioned as upstream/open with the named cause. *(Fixed and re-verified: Google, Bing, Etsy, YouTube (the UA rows); the GC stalls' engine share (the benchmark). Open with the cause named: Cloudflare's challenge (6 sites), Startpage's JS challenge, eBay, old Reddit's gate, the four blank apps (ES modules, upstream), the NS_ERROR_NOT_INITIALIZED thrower (the jQuery probe is written, not yet run), the daemon's memory after cards close (daemon 9, to measure).)*
- [x] The round is run again after the fixes and its results compared with the first run in the ledger. *(2c, 2026-10-15 16:33; the comparison and the reading in the ledger.)*
- [~] **Performance** (the owner, 2026-10-15: "also address any performance issues you encounter"): *(2026-10-16: the daemon's memory after cards is read — closed pages are released (one detached window, the last page, no ghosts); the retention is the allocator's (no jemalloc) and libxul's paging; the per-card purge and the collections bring 428 MB to 170 MB at the last close; an engine rebuild with jemalloc is proposed (T-493). Second run: the benchmark's allocation tests 5–6× faster at the shipped threshold with the v8 sets; the sites' stalls unchanged (their own scripts); the daemon's memory after cards close is the open item — 589 MB after YouTube, never released — daemon 9 purges on the last card's close, to measure. First run measured; the cause found — non-incremental full garbage collections at the engine's 30 MB allocation threshold, and the blocker's 100,000 strings making each one 3.5× longer; fixed by `javascript.options.mem.gc_allocation_threshold_mb` 160 + high-water mark 192 (measured on the device, the ledger) and by component v6's typed-array host sets; the second run measures the result.)* every visit records time to first paint, time to load-done, the paint pipeline's average and lowest frame rate during the dwell, the worst tick (ms of work in one 100 ms period), the daemon's RSS after the visit and the load average; a visit that takes more than 30 s to load-done, paints under 5 fps, shows the slow-script dialog, or leaves the daemon more than 40 MB heavier than the visit before is an issue with a row; memory across the whole session (first visit to last) is in the ledger; the fixable ones are fixed and re-measured. *(2026-10-16 03:40, T-493: the engine rebuilt with jemalloc, the allocator in the daemon; after three heavy pages and the last close 149 MB resident, was 170; `heap-allocated` reports now, 60 MB; the numbers in the ledger.)*
- [x] YouTube, the heaviest page of the round, is sped up by a measured lever (the owner, 2026-10-16: "speed up youtube"): the identity rule above; the before/after table is in the ledger (T-497).
- [x] The web platform members this engine lacks and modern pages assume (checked in the engine's source, then on the device) are supplied to each page before its scripts run, each only where the engine has none, each tested on its own page (`render/goanna/test/jihad-compat.html`): CSSStyleSheet.replace, popover, inert, Intl.ListFormat, reportError, checkVisibility, AbortSignal.any, URLSearchParams.size, Array.fromAsync, Blob.text, Response.json, Promise.try, the Set methods, the iterator helpers, ReadableStream iteration, crypto.randomUUID, scheduler.postTask, requestVideoFrameCallback, canvas.roundRect. *(2026-10-16, T-498: 16/16 on the device; Etsy's search styled; the ledger lists what stays engine work.)*
- [ ] A page laid out wider than the window gets a layout viewport as wide as its content, so what it parks just past `innerWidth` stays off screen while the card fit-zooms the whole width. *(T-499, open: DuckDuckGo's slide-out menu at x = 772 in a 768 window on a 1000-px page; the ledger has the measurement and the design.)*
**Dependencies:** R1

## Non-goals
- Making every modern site work: the engine is what it is; this kit records and fixes what
  configuration can fix, and names the rest.
- Replacing Palefill or shipping other compatibility add-ons.

## References
- Pale Moon forum, "CloudFlare discussion thread" (2025): the `general.useragent.override.challenges.cloudflare.com` finding.
- `render/goanna/EngineHost.cpp`: the per-host UA table and the console bridge (`JIHAD_JS_CONSOLE`).
- `third_party/uxp/modules/libpref/init/all.js`: `dom.max_script_run_time`.
