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
registered in the `content-policy` category) implements `nsIContentPolicy.shouldLoad`. For a load
of type script, image, stylesheet, subdocument, XMLHttpRequest, fetch, media, font, object, ping,
beacon or websocket whose host — or any parent domain of it — is in the block set, it returns
`REJECT_REQUEST`. Never blocked: the top-level document (a listed site still opens when the user
asks for it), and a load whose host equals the document's own host. Rules marked `$third-party`
block only when the request's base domain (the eTLD service) differs from the document's. Rules
with type options and/or `third-party` (`||google-analytics.com^$script,third-party,xmlhttprequest`)
block those types under that party rule; `||host/path` rules block loads whose path starts with the
prefix (MDN's own advert server, `||developer.mozilla.org/pong/`). An exception rule `@@||host^`
removes that host and its subdomains from blocking. *(The typed and path shapes were added after
the first device test, 2026-10-15: without them google-analytics loaded and MDN's advert stayed.)*
**Acceptance Criteria:**
- [x] On the device, a page that references `https://www.google-analytics.com/analytics.js` and `https://doubleclick.net/…` (`render/goanna/test/jihad-blocking.html`) loads with those requests refused: the daemon log shows one `[jihad-bs] js-console (no source):0 blocker: blocked <type> <host> on <page host>` line per refusal (the component logs through Services.console, as the other components do) and the page's own script reports `onerror` for them; the page's first-party image and script load.
- [ ] The top-level navigation to a listed host (e.g. `https://doubleclick.net/`) is NOT blocked (the page opens, or fails for its own reasons).
- [ ] A `$third-party` rule's host loads when first-party and is refused when third-party (two test pages).
- [ ] An `@@||host^` exception in the list makes that host load.
- [ ] With the master switch off (R4), the same page loads everything and no `blocked` line is logged — without a daemon restart.
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
- [x] Compiling on the device finishes off the start-up path (the first page can load before it is done; blocking starts when it is) and takes under 10 s; a daemon restart loads the cache instead of recompiling. *(v3: 5.9 s in slices, 1.5 s after start; v2's 21.6 s was a quadratic exception pass, fixed; the cache (`compiled.json`, 3 MB) is loaded on restart.)*
- [x] NOTICE, CREDITS.md and about:jihad name EasyList and EasyPrivacy, their authors, the GPLv3+ choice of the dual licence, the snapshot versions and where the text ships. *(And the Settings About pane.)*
**Dependencies:** none

### R3: The holes adverts leave are hidden
**Description:** EasyList's generic element-hiding rules — the simple ones, a single class or id
(`##.ad-banner`, `###adbox`; ~13,000) — become one user stylesheet (`nsIStyleSheetService`,
`USER_SHEET`) of `selector { display: none !important }`, limited to http(s) documents, registered
when hiding is on and unregistered when it is off. Site-specific rules (`a.com,b.com##selector`,
~8,000) go into the same sheet as `@-moz-document domain(a.com),domain(b.com)` blocks, one CSS rule
per selector so a selector the engine cannot parse drops only itself (MDN's banner,
`developer.mozilla.org##.page-layout__banner`). Generic exceptions (`#@#selector`) and site
exceptions (`a.com#@#selector`) remove a selector. Complex generic selectors and `~domain`
negations are counted as skipped.
**Acceptance Criteria:**
- [x] `developer.mozilla.org/en-US/docs/Web/CSS` on the device shows no advert and no empty advert frame in the sidebar (screenshot, before and after). *(Before: `enyo-wiki` 10:25 (banner + sidebar advert); after v3: `cb3-mdn` 12:34, neither — the banner by the site rule, the sidebar by the `/pong/` path rule.)*
- [ ] The sheet's cost is measured: the daemon's `load done` time for a reference page (the MDN page, cached) with hiding on is within 15% of the time with it off, three runs each, numbers in the ledger.
- [ ] Turning hiding off takes effect on the next page load without a daemon restart.
**Dependencies:** R2

### R4: Settings
**Description:** Settings → Privacy gains a "Content blocking" group: **Block ads and trackers**
(`jihad.blocking.enabled`, default on), **Hide the frames adverts leave** (`jihad.blocking.cosmetic`,
default on), a read-only line "EasyList <version> · EasyPrivacy <version>, updated <date>; <N>
requests blocked since start", and **Update the lists now** (the page's first button row; the
row type is added to `packaging/prefsui/content/preferences.js`). Each pref takes effect live,
effect-verified (cavekit-preferences-ui.md's rule), in all three variants.
**Acceptance Criteria:**
- [~] The four rows render in Enyo, Mochi and Mojo (screenshots) in the Privacy pane. *(Mochi: `cb-settings` 12:04, all four, the counter live; Enyo and Mojo after the package push.)*
- [ ] Toggling each switch changes behaviour on the next load with no restart (R1's and R3's last criteria, driven from the page).
- [ ] The read-only line shows the versions the compiled set came from and a counter that rises as requests are refused.
- [ ] "Update the lists now" runs R5's update and the line changes to the new versions and date.
**Dependencies:** R1, R3, R5

### R5: The lists stay current
**Description:** Both lists say `! Expires: 4 days`. The component fetches
`https://easylist.to/easylist/easylist.txt` and `easyprivacy.txt` no more often than every 4 days
(a timer off the start-up path, and never while a page is loading), keeps the download only if it
parses and its `! Version:` is newer, compiles it (R2) and swaps the sets in place; on any failure
it keeps what it has. The bundled snapshot is the fallback forever. Updates are logged.
**Acceptance Criteria:**
- [ ] A forced update on the device (the button, or `jihad.blocking.lastUpdate` cleared) downloads both files over HTTPS, logs the versions and the counts, and the next page is blocked by the new set (a rule present only in the new list is honoured).
- [ ] With the network off, the update fails cleanly: a log line, no change, no dialog.
- [ ] The download is not attempted more often than every 4 days (the timestamp pref, checked across a daemon restart).
**Dependencies:** R2

### R6: It fits the device
**Description:** Three daemons may run at once on a 1 GB TouchPad. The block set is kept compact
and its cost measured.
**Acceptance Criteria:**
- [ ] The daemon's `VmRSS` (`/proc/<pid>/status`) with the sets loaded, minus without, is recorded for one variant and is under 12 MB; if it is not, the representation changes (a sorted joined string with binary search, or a typed-array hash) until it is. *(Measured 2026-10-15, Mochi idle after a cache load: 69.8 MB with, 33.4 MB without = **+36 MB**, over budget; the breakdown (sets vs the parsed stylesheet) is being measured, then the heavy part changes.)*
- [ ] `shouldLoad` adds no visible latency: a page with 200 third-party image requests (`render/goanna/test/jihad-blocking-many.html`) has a `load done` time within 10% of the time with blocking off.
- [ ] The daemon's memory-pressure guardrail (cavekit-browser-services.md R2) is not triggered by the sets on a fresh start.
**Dependencies:** R1, R2

### R7: Licensing, packaging and the README
**Description:** The list files, their README, NOTICE, CREDITS.md, about:jihad and the licensing
audit (impl-review-findings-6.md) carry the lists; the README's "What only Jihad does" gains the
feature with EasyList and EasyPrivacy credited; the hero screenshot of MDN is retaken without
adverts.
**Acceptance Criteria:**
- [ ] `tools/host/check-pins.sh` or the bundle's own checks fail if a list file's sha256 differs from its notice.
- [ ] The README names the feature, the two lists and their licence in one sentence, and `docs/screenshots/` carries the MDN page with the adverts gone.
- [ ] The inspector's review of the tier finds no licence gap (every bundled file named, its licence text shipped, the GPLv3+ choice stated).
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
