---
created: "2026-10-15"
last_edited: "2026-10-15"
---

# Cavekit: Content Blocking (ads and trackers)

## Scope
An ad and tracker blocker built into the engine side of Jihad Browser, so all three interfaces
(Mojo, Enyo, Mochi) have it the way they share Settings and Personas: requests to known advertising
and tracking hosts are refused before they leave the device, and the empty frames adverts leave
behind are hidden. It uses the two canonical community lists, EasyList and EasyPrivacy, under
their own licence, bundled and refreshed. Nothing the stock browser or Atlas Browser has.

**User requirement, 2026-10-15:** *"build in an ad and tracker blocker if possible (following
license terms)"*.

### Why in the engine, and why not an add-on
- **uBlock Origin Legacy** (GPLv3) is the usual answer on Pale Moon, but its last release is
  1.16.4.33 (February 2024), its maintainer has been silent since 2021, Pale Moon's forum (March
  2025) reproduced unpatched findings in it, and its memory use is sized for desktops, not a 1 GB
  tablet with three browser daemons. Not bundled.
- **Adblock Latitude / ηMatrix** are Pale Moon's own; their fit with this engine's add-on compatibility
  and their maintenance were not established. Not bundled.
- A **small component of our own** does the two things that matter on this device, host blocking
  and generic element hiding, with the engine's own `nsIContentPolicy` and `nsIStyleSheetService`,
  costs a few megabytes, and is ours to keep working. That is this kit.

### The lists and their licences (decided)
| List | Use | Licence | Decision |
|---|---|---|---|
| **EasyList** (easylist.to) | adverts: ~48,700 `\|\|domain^` host rules, ~13,600 generic element-hiding rules | GPLv3+ **or** CC BY-SA 3.0+ (dual, the project's own licence page) | **bundled, taken under GPLv3+** (`licenses/GPL-3.0.txt` already ships); attribution "The EasyList authors" |
| **EasyPrivacy** (easylist.to) | trackers: ~43,200 host rules, ~3,800 third-party-only | same dual licence | **bundled, GPLv3+** |
| AdGuard filters | — | GPL-3.0 | possible later; not needed for v1 |
| HaGeZi DNS blocklists | — | GPL-3.0 | possible later (domain lists, no cosmetic rules) |
| Disconnect tracking protection | — | CC BY-NC-SA 4.0 | **rejected**: NonCommercial (the project does not ship NC material) |
| Peter Lowe's list | — | no licence stated ("feel free to combine…") | **rejected**: no licence |

The list text is data the engine reads, not code linked into it: the GPL covers the list files
and anything derived from them (the compiled form in the profile), not the browser. They ship
like the hyphenation patterns and the fonts: with their licence text, named in NOTICE, CREDITS.md
and about:jihad, and with the upstream version line of each snapshot kept.

## Requirements

### R1: Hosts on the lists are not loaded
**Description:** A component in the GRE (`render/goanna/components/jihadContentBlocker.js`,
registered in the `content-policy` category) implements `nsIContentPolicy.shouldLoad`, and an
`http-on-modify-request` observer repeats the decision for a REDIRECTED channel (which the policy
never sees again). A load of any type but the document itself and a download the user asked for
is refused (`REJECT_REQUEST`) when it matches:
- a **host rule** (`||host^`): the host or any parent domain; never for the page's own host;
- a **third-party host rule** (`||host^$third-party`): as above, only when the request's base
  domain (the eTLD service) differs from the page's;
- a **typed rule** (`||host^$script,third-party,xmlhttprequest`, ~200): those types under that party
  rule, each rule for a host kept separately;
- a **path rule** (`||host/path`, ~5,600; with or without `$third-party`): the request's path (case
  folded) starts with the prefix — on the page's own host too (MDN's own `/pong/` advert server);
unless an **exception** matches first, at any level of the domain walk: `@@||host^`, and its
optioned forms with a path, types, a party and `$domain=` lists (~1,600). **An exception is never
silently dropped**: the first device test with only bare exceptions turned EasyList's breakage
fixes into blocks (inspector F-2). What a content policy cannot see is closed by prefs while the
blocker ships: `network.dns.disablePrefetch` and `network.http.speculative-parallel-limit=0`, so a
listed host is not contacted before a load is asked for.
**Acceptance Criteria:**
- [x] On the device, a page that references `https://www.google-analytics.com/analytics.js` and `https://ad.doubleclick.net/…` (`render/goanna/test/jihad-blocking.html`) loads with those requests refused: *(v5.1, 2026-10-15 14:40, served over http from the host: `ad.doubleclick.net` and `static.ads-twitter.com` refused by host rule, google-analytics' script by its typed rule and its image loaded, facebook's `/tr?` by its path rule and its favicon loaded, the page's own image loaded; the daemon logs `blocked <type> by <rule>`.)* the daemon log shows one `[jihad-bs] js-console (no source):0 blocker: blocked <type> <host> on <page host>` line per refusal (the component logs through Services.console, as the other components do) and the page's own script reports `onerror` for them; the page's first-party image and script load.
- [x] The top-level navigation to a listed host (e.g. `https://doubleclick.net/`) is NOT blocked (the page opens, or fails for its own reasons). *(2026-10-24, T-520, Enyo, `ad.doubleclick.net` mapped to the host PC in the device's `/etc/hosts`: `http://ad.doubleclick.net:8077/jihad-colors.html` opens, while an image from that host on another page is refused. `docs/rounds/t520-2026-10-24/t520-run3-host.log`, `t520-daemon.log`.)*
- [x] A `$third-party` rule's host loads when first-party and is refused when third-party (two test pages). *(2026-10-24, T-520: EasyPrivacy's `||123count.com^$third-party`, the host mapped to the PC: `jihad-tp-first.html` served from `123count.com:8077` loads both its images (`2/2 loaded`, no `blocked` line); `jihad-tp-third.html` served from the PC's address is refused that image (`1/2 loaded`, `blocked image by 123count.com$third-party`).)*
- [x] An `@@||host^` exception in the list makes that host load. *(2026-10-24, T-520: the bundled lists carry no bare exception that negates one of their own host rules, so the test uses a profile copy of EasyList with `@@||ad.doubleclick.net^` appended and a newer `! Version:` (compiled `exceptions 6+1083` against `5+1079`): `jihad-exception.html`'s image from `ad.doubleclick.net:8077` loads (`2/2 loaded`); with the published copy back, `1/2 loaded` and `blocked image by ad.doubleclick.net`.)*
- [x] With the master switch off (R4), the same page loads everything and no `blocked` line is logged — without a daemon restart. *(2026-10-24, T-520: `jihad.blocking.enabled` set false through the inject channel's `setpref` (`blocking off` logged): the third-party page reads `2/2 loaded` with 0 `blocked` lines; set true (`blocking on`): `1/2 loaded` and the `blocked` line; the daemon never restarted.)*
**Dependencies:** R2

### R2: The lists, bundled and compiled
**Description:** `third_party/easylist/easylist.txt` and `easyprivacy.txt` are pinned snapshots
(their `! Version:` and `! Last modified:` headers kept, sha256 in `make-device-bundle.sh` as
Palefill's is, a README in the Palefill README's form). The bundle carries them raw under
`blocklists/`. The component compiles them in JavaScript (one compiler, used for the bundled
snapshot and for updates): `||host^` → block set; `||host^$third-party` → third-party set; `@@||host^`
→ exception set; generic `##.class` / `###id` → the cosmetic sheet (R3); everything else is skipped
and counted. The compiled form is cached in the profile (`<profile>/jihad-blocklists/`) with the
versions it came from, and rebuilt when the raw text or the compiler version changes.
**Acceptance Criteria:**
- [x] The two snapshots are in the tree with README, their upstream version lines, and their sha256 checked by `make-device-bundle.sh` (a changed file refuses to bundle under the old notice). *(2026-10-15, `third_party/easylist/`, snapshots 202610151429.)*
- [x] The compiler reports counts (hosts, third-party hosts, exceptions, cosmetic selectors, skipped rules) to the daemon log, and they match an independent count of the same text (`tools/host/count-blocklist.py`) within the documented skip categories. *(Device: hosts 91,924 = 48,717 + 43,229 less 22 duplicates/exceptions; typed 199 (201 rules, two hosts twice); path 5,579 = 1,528 + 4,051; site selectors 8,074 = 8,051 + 23; generic 13,079 = 13,077 + 2.)*
- [x] Compiling on the device finishes off the start-up path (the first page can load before it is done; blocking starts when it is) and takes under 10 s; a daemon restart loads the cache instead of recompiling. *(v3: 5.9 s in slices, 1.5 s after start; a restart loads the profile cache, plain files since v7 (R6).)*
- [x] NOTICE, CREDITS.md and about:jihad name EasyList and EasyPrivacy, their authors, the GPLv3+ choice of the dual licence, the snapshot versions and where the text ships. *(And the Settings About pane.)*
**Dependencies:** none

### R3: The holes adverts leave are hidden
**Description:** EasyList's generic element-hiding rules — the simple ones, a single class or id
(`##.ad-banner`, `###adbox`; ~13,000) — become one user stylesheet (`nsIStyleSheetService`,
`USER_SHEET`) of `selector { display: none !important }`, limited to http(s) documents, registered
when hiding is on and unregistered when it is off. Site-specific rules (`a.com,b.com##selector`,
~8,000) are kept per domain and loaded into a document of that domain only
(`nsIDOMWindowUtils.loadSheetUsingURIString`, one CSS rule per selector so a selector the engine
cannot parse drops only itself; MDN's banner, `developer.mozilla.org##.page-layout__banner`):
no page pays for the 8,000 rules it cannot match. Generic exceptions (`#@#selector`) remove a
selector; a site exception against a generic selector (`53.com#@##ad-rotator`) scopes that
selector away from the site (T-522: such a selector travels with the per-document sheet to every host but
the excepting one, not as a `@-moz-document regexp` group of its own in the global sheet — 268 such groups
cost the MDN page 4 s a load); `$generichide`/`$elemhide`/`$document` exceptions take a site (or an
address prefix such as `://192.168.`) out of the generic sheet. Selectors with procedural
pseudo-classes, unbalanced brackets, `;`, `@` or `/*` are counted as bad; complex generic selectors
and `~domain` negations as skipped.
**Acceptance Criteria:**
- [x] `developer.mozilla.org/en-US/docs/Web/CSS` on the device shows no advert and no empty advert frame in the sidebar (screenshot, before and after). *(Before: `enyo-wiki` 10:25 (banner + sidebar advert); after v3: `cb3-mdn` 12:34, neither — the banner by the site rule, the sidebar by the `/pong/` path rule.)*
- [x] The sheet's cost is measured: the daemon's `load done` time for a reference page (the MDN page, cached) with hiding on is within 15% of the time with it off, three runs each, numbers in the ledger. *(2026-10-24, T-522: the MDN page's copy, hiding off 2712, 2578, 2846 ms, on 2868, 3104, 2876 — on is 8.7 % over off on the mean; a daemon's first load 3.7 s where it took 8-12; `docs/rounds/t520-2026-10-24/t522-run2-host.log`. T-520 had measured it NOT MET (on 3728, 7416, 7098 ms against off 2622, 3046, 2996): the cause was the 268 `@-moz-document regexp` groups (see the description), named by four variants of the cached sheet (`t520-sheet-variants-host.log`). After T-522 (`excSel`, compiler 11) `jihad-hiding.html` and `jihad-excsel.html` are still hidden under a host name, and a `$generichide` host is untouched (`t522-generichide-host.log`). Numbers in `docs/MODERN-WEB.md`.)*
- [x] Turning hiding off takes effect on the next page load without a daemon restart. *(2026-10-24, T-520: `jihad-hiding.html` (a `#ad-rotator` box under EasyList's generic `###ad-rotator`), served under the host name `jihad.test` because EasyList's `@@://192.168.$generichide` exempts LAN addresses, reports `ad-rotator=none`; `jihad.blocking.cosmetic` set false live, the next load `ad-rotator=block`; true again, `none`. The component re-registers the sheet on the pref change (`_applySheet`).)*
**Dependencies:** R2

### R4: Settings
**Description:** Settings → Privacy gains a "Content blocking" group: **Block ads and trackers**
(`jihad.blocking.enabled`, default on), **Hide the frames adverts leave** (`jihad.blocking.cosmetic`,
default on), a read-only line "EasyList <version> · EasyPrivacy <version>, updated <date>; <N>
requests blocked since start", and **Update the lists now** (the page's first button row; the
row type is added to `packaging/prefsui/content/preferences.js`). Each pref takes effect live,
effect-verified (cavekit-preferences-ui.md's rule), in all three variants.
**Acceptance Criteria:**
- [x] The four rows render in Enyo, Mochi and Mojo (screenshots) in the Privacy pane. *(Mochi: `cb-settings` 12:04, all four, the counter live. 2026-10-24, T-520: Enyo `docs/rounds/t520-2026-10-24/t520-enyo-privacy2.jpg`, Mojo `t520-mojo-privacy.jpg` through its test hook.)*
- [x] Toggling each switch changes behaviour on the next load with no restart (R1's and R3's last criteria, driven from the page). *(2026-10-24, T-520: both prefs flipped live through `setpref`, the path the page's switches take (`Services.prefs.setBoolPref`, the component's observer); `blocking off`/`blocking on`/`hiding on (13079 generic selectors)` in `t520-daemon.log`.)*
- [x] The read-only line shows the versions the compiled set came from and a counter that rises as requests are refused. *(2026-10-24, T-520, read with the inject channel's `gettext .row-text`: "EasyList 202610241630 · EasyPrivacy 202610241606, updated 2026-10-24; 10 requests blocked since start", then 16 after `jihad-blocking.html` (6 refusals); the versions are the compiled set's. `t520-enyo-privacy3.jpg`.)*
- [x] "Update the lists now" runs R5's update and the line changes to the new versions and date. *(2026-10-24, T-520, Enyo, the button clicked through the inject channel: run 1 took both lists from 202610241105 to 202610241606 and the line followed (`t520-run1-host.log`); run 2, 25 minutes later, took EasyList to 202610241630 (`t520-run2-host.log`).)*
**Dependencies:** R1, R3, R5

### R5: The lists stay current
**Description:** Both lists say `! Expires: 4 days`. The component fetches
`https://easylist.to/easylist/easylist.txt` and `easyprivacy.txt` no more often than every 4 days
(a timer off the start-up path with a random offset so three daemons do not fetch together, and
not within 20 s of a page starting), through an anonymous, uncached, background request (no
cookies, no cache, no prompts), keeps the download only if it parses, its `! Version:` is newer and
it is not truncated (at least half the rules it replaces), compiles it (R2) and swaps the sets in
place; on any failure it keeps what it has and waits a day before the next try; a clock set back
cannot postpone it. Whichever of the bundled snapshot and the download is newer is used, so a
package upgrade's snapshot is not shadowed by an older download. Updates are logged.
**Acceptance Criteria:**
- [x] A forced update on the device (the button, or `jihad.blocking.lastUpdate` cleared) downloads both files over HTTPS, logs the versions and the counts, and the next page is blocked by the new set (a rule present only in the new list is honoured). *(2026-10-19, the Enyo card, `lastUpdate` cleared: EasyList 202610151429 -> 202610191826 and EasyPrivacy 202610151429 -> 202610191828 over HTTPS, hosts 91,924 -> 93,360, logged with the counts; the next page's image from `beacon.cdnpure.com`, a rule only in the new EasyPrivacy, was blocked (`docs/rounds/t487-update.png`). The run found the truncation check holding EasyPrivacy against both lists' rules; each download is now held against its own list's copy.)*
- [x] With the network off, the update fails cleanly: a log line, no change, no dialog. *(2026-10-19: a dead proxy (127.0.0.1:9) and `lastUpdate` 1: `update: easylist: no download`, `easyprivacy: no download`, the versions and the compiled set unchanged, `lastAttempt` set, no dialog (`docs/rounds/t487-netoff.png`).)*
- [x] The download is not attempted more often than every 4 days (the timestamp pref, checked across a daemon restart), and a failed attempt not more often than daily (`jihad.blocking.lastAttempt`). *(2026-10-19, five minutes after a daemon restart each: with `lastUpdate` fresh, 5 card connects and 0 update lines; after the failed attempt above (`lastUpdate` 1, `lastAttempt` today), 4 connects and 0 update lines. A due update skipped because a page started within 20 s now looks again 60-90 s later, not six hours later (found by these runs).)*
**Dependencies:** R2

### R6: It fits the device
**Description:** Three daemons run from boot on a 1 GB TouchPad, one per installed variant, most
of them idle most of the time. The block set is kept compact, its cost measured, and **held only
while a card is connected**: the daemon tells the component when a card connects and when the
last one leaves; the lists load on the first card (the cache in about a second, a compile in six)
and are dropped ten minutes after the last card leaves, so an unused variant costs nothing.
*(2026-10-15: three daemons each holding the lists took ~80 MB and, with a 160 MB GC threshold,
the device refused new cards; see the ledger and PICKUP.)*
**Acceptance Criteria:**
- [~] The daemon's `VmRSS` (`/proc/<pid>/status`) with the sets loaded, minus without, is recorded for one variant and is under 12 MB; if it is not, the representation changes (a sorted joined string with binary search, or a typed-array hash) until it is. *(With a card: v7's 56.0 MB resident after install against 33.7 without = +22 MB, of which the JS heap is 7 MB; idle (no card) the lists are not held at all (+1.4 MB). The 12 MB with-card figure is not met; what is left is the allocator's retention of the cache files' reads (no jemalloc in this build). Accepted for this version with the idle cost nil; the kit's number stands as the goal.)* *(2026-10-15, the design that got there: each host set is one string plus a typed-array hash table (v6), the daemon `malloc_trim`s on "jihad-blocker-installed", the cache is plain files read straight into those strings (v7: `hosts.txt`, `tp.txt`, `exc.txt`, `cosmetic.css`, `meta.json`), and the lists are held only while a card is connected (v8). The stylesheet costs nothing (hiding on/off 70.3/70.1 MB).)*
- [x] `shouldLoad` adds no visible latency: a page with 200 third-party image requests (`render/goanna/test/jihad-blocking-many.html`) has a `load done` time within 10% of the time with blocking off. *(2026-10-24, T-520, Enyo, `performance.timing` through the inject channel, the twenty hosts mapped to the host PC so blocking-off loads fail at once: on 8592, 4852, 4498, 8078 ms (mean 6505), off 5746, 6016, 6304, 6108 (mean 6044): +7.6 %. The spread is the page's own variance, and the 4.5-6 s floor is the engine's per-image work on both paths. `docs/rounds/t520-2026-10-24/t520-confirm-host.log`.)*
- [x] **The sets do not slow the engine's garbage collector**: a full collection with the lists loaded is within 20% of one without (the GC log on `jihad-jsbench.html`; v5.1 measured 3.9 s against 1.1 s, which is why v6 keeps the hostnames out of the GC heap). *(v6, 15:32: 1,073 ms with the lists against 1,101 ms without.)*
- [x] The daemon's memory-pressure guardrail (cavekit-browser-services.md R2) is not triggered by the sets on a fresh start. *(2026-10-16: no "memory pressure" line on any start; idle 34.8 MB.)*
- [x] An idle daemon (no card for ten minutes) holds no lists: its RSS returns to within 3 MB of a daemon that never loaded them; the next card loads them again (the daemon log says so). *(2026-10-16: idle with no card 34.8 MB against 33.4 MB without the component; the lists load on the card ("cache: …"); the ten-minute drop logged in the 16:30 run.)*
**Dependencies:** R1, R2

### R7: Licensing, packaging and the README
**Description:** The list files, their README, NOTICE, CREDITS.md, about:jihad and the licensing
audit (impl-review-findings-6.md) carry the lists; the README's "What only Jihad does" gains the
feature with EasyList and EasyPrivacy credited; the hero screenshot of MDN is retaken without
adverts.
**Acceptance Criteria:**
- [x] `tools/host/check-pins.sh` or the bundle's own checks fail if a list file's sha256 differs from its notice. *(`build/webos-oe/make-device-bundle.sh`: each list's sha256 is compared with the pinned value, and the build stops unless `NOTICE` and `third_party/easylist/README.md` both name that hash (inspector F-20 made the notice the reference). `check-pins.sh` covers the submodule pins; the lists are plain files in the tree, so the bundle's check is the one that applies.)*
- [x] The README names the feature, the two lists and their licence in one sentence, and `docs/screenshots/` carries the MDN page with the adverts gone. *(README "Ads and trackers": loads from the hosts on EasyList and EasyPrivacy refused, the lists by The EasyList authors under the GNU GPL v3 or later; `docs/screenshots/cardview.jpg`, recomposed 2026-10-15 14:52 (T-484), shows MDN's CSS page in the Mochi card with the adverts gone.)*
- [x] The inspector's review of the tier finds no licence gap (every bundled file named, its licence text shipped, the GPLv3+ choice stated). *(`context/impl/impl-review-findings-6.md` round 26, the inspector on Tier 86: F-20 (about:jihad, CREDITS and the lists' README incomplete) fixed, and the packaging and the NOTICE entry found complete with the lists taken under GPLv3+; the licensing audit of 2026-10-14 extended to the lists.)*
**Dependencies:** R2, R3

## Non-goals (this version)
- Scriptlet injection, `$domain=`/`$redirect`/`$csp`/`$removeparam`/`$popup` options, wildcard and
  regex rules, `~domain` negations, complex generic cosmetic selectors: skipped and counted, listed
  for a later version.
- Per-site allowlisting from the card's menu (a later version; the data model allows it).
- Blocking in the stock browser or any other app: the component lives in Jihad's engine.

## References
- EasyList licence page: https://easylist.to/pages/licence.html (GPLv3+ / CC BY-SA 3.0+).
- `third_party/uxp/dom/base/nsIContentPolicy.idl`, `layout/base/nsIStyleSheetService.idl`,
  `netwerk/dns/nsIEffectiveTLDService.idl`.
- The Palefill bundling pattern: `third_party/palefill/README.md`, `build/webos-oe/make-device-bundle.sh`.
