# cavekit-content-blocking.md history, moved out of the kit by the 2026-11-07 sweep; verbatim.

## R1, criterion 2 (removed 2026-11-07)

*(2026-10-24, T-520, the Enyo card with `ad.doubleclick.net` mapped to the host PC in the device's `/etc/hosts`: `http://ad.doubleclick.net:8077/jihad-colors.html` opens (title `jihad-colors`, `load done uri=http://ad.doubleclick.net:8077/jihad-colors.html`), while an image from the same host on another page is refused (`blocked image by ad.doubleclick.net`). `docs/rounds/t520-2026-10-24/t520-run3-host.log`, `t520-daemon.log`.)*

## R1, criterion 3 (removed 2026-11-07)

*(2026-10-24, T-520: EasyPrivacy's `||123count.com^$third-party`, the host mapped to the PC. `jihad-tp-first.html` served FROM `123count.com:8077` loads both its images from that host (title `jihad-tp-first 2/2 loaded`, no `blocked` line); `jihad-tp-third.html` served from the PC's address asks for the same image and is refused (`jihad-tp-third 1/2 loaded`, `blocked image by 123count.com$third-party`, the page's own image the control).)*

## R1, criterion 4 (removed 2026-11-07)

*(2026-10-24, T-520: the bundled lists carry no bare exception that negates one of their own host rules (their four are for generic rules the compiler does not read), so the test uses an EasyList copy in the profile with `@@||ad.doubleclick.net^` appended and a newer `! Version:`; the daemon compiles it (`exceptions 6+1083` against the published `5+1079`) and `jihad-exception.html`'s image from `ad.doubleclick.net:8077` loads (`jihad-exception 2/2 loaded`, no `blocked` line); with the published copy back the same page reads `1/2 loaded` and `blocked image by ad.doubleclick.net`.)*

## R1, criterion 5 (removed 2026-11-07)

*(2026-10-24, T-520: `jihad.blocking.enabled` set false through the inject channel's `setpref` (the component logs `blocking off`), the third-party page loaded again reads `jihad-tp-third 2/2 loaded` with 0 `blocked` lines; set true again (`blocking on`), the next load is `1/2 loaded` with the `blocked image by 123count.com$third-party` line, the daemon never restarted.)*

## R2, criterion 3 (removed 2026-11-07)

*(v3: 5.9 s in slices, 1.5 s after start; v2's 21.6 s was a quadratic exception pass, fixed; the cache (`compiled.json`, 3 MB) is loaded on restart.)*

## R3, criterion 2 (removed 2026-11-07)

*(2026-10-24, T-522, after the fix below: the MDN page's copy, hiding off 2712, 2578, 2846 ms, on 2868, 3104, 2876 — on is 8.7 % over off on the mean, within the 15 %; a daemon's first load 3.7 s where it took 8-12; `docs/rounds/t520-2026-10-24/t522-run2-host.log`. The measurement that found the cost, T-520, MEASURED AND NOT MET before the fix: the MDN CSS page's HTML (327 KB) served from the LAN under a host name (EasyList's `@@://192.168.$generichide` exempts LAN addresses, and MDN's own CSP refuses the `javascript:` timing probe), `performance.timing` from navigation to the load event, hiding off then on alternating: off 2622, 3046, 2996 ms; on 3728, 7416, 7098 ms — the sheet costs about 4 s on two runs in three, 1.1 s on the third. The real cached page by the host's coarse timer: off 11 and 11 s, on 15 and 15. Four variants of the cached sheet named the cause: the 268 `@-moz-document regexp` groups, one per site-excepted generic selector and each carrying the 160 generic-hide hosts' 2.4 KB regexp, evaluated per document — the same 13,079 keyed selectors under `url-prefix` scopes or with no wrapper cost nothing (`t520-sheet-variants-host.log`). T-522: those selectors travel with the per-document sheet instead (`excSel`, compiler 11; a `$generichide` host gets none, an excepting host not its own), and the global sheet keeps its one regexp group: `jihad-hiding.html` still hidden under a host name, `jihad-excsel.html`'s site-excepted `#ad-carousel` hidden on `jihad.test`; on a `$generichide` host (jetzt.de mapped to the PC) neither page is touched (`t522-generichide-host.log`). Numbers in `docs/MODERN-WEB.md`.)*

## R3, criterion 3 (removed 2026-11-07)

*(2026-10-24, T-520: `jihad-hiding.html` (a `#ad-rotator` box under EasyList's generic `###ad-rotator`) served under a host name (`jihad.test`: EasyList's `@@://192.168.$generichide` exempts LAN addresses from the generic sheet) reports `ad-rotator=none control=block`; `jihad.blocking.cosmetic` set false live, the next load reports `ad-rotator=block`; set true again, `none` again. The component re-registers the user sheet on the pref change (`_applySheet`).)*

## R4, criterion 1 (removed 2026-11-07)

*(Mochi: `cb-settings` 12:04, all four, the counter live; Enyo and Mojo after the package push. 2026-10-24, T-520: Enyo `docs/rounds/t520-2026-10-24/t520-enyo-privacy2.jpg` (the line "EasyList 202610241630 · EasyPrivacy 202610241606, updated 2026-10-24; 10 requests blocked since start"), Mojo `t520-mojo-privacy.jpg` through its test hook (its own daemon's lists, 202610241117).)*

## R4, criterion 2 (removed 2026-11-07)

*(2026-10-24, T-520: both prefs flipped live through `setpref`, the same path the page's switches take (`Services.prefs.setBoolPref`, the component's observer): blocking off and on (R3's last criterion), hiding off and on (R4's second); the lines `blocking off`/`blocking on`/`hiding on (13079 generic selectors)` in `t520-daemon.log`.)*

## R4, criterion 3 (removed 2026-11-07)

*(2026-10-24, T-520, read with the inject channel's `gettext .row-text` on `about:preferences#pane=privacy`: "EasyList 202610241630 · EasyPrivacy 202610241606, updated 2026-10-24; 10 requests blocked since start", then after `jihad-blocking.html` (6 refusals) "... 16 requests blocked since start"; the versions are the compiled set's (`compiled: EasyList 202610241630, EasyPrivacy 202610241606`). `t520-enyo-privacy3.jpg`.)*

## R4, criterion 4 (removed 2026-11-07)

*(2026-10-24, T-520, the Enyo card, the button clicked through the inject channel (`rect sel:button.btn`, `click`): the first run took the lists from 202610241105 to 202610241606 (`update: EasyList 202610241606, EasyPrivacy 202610241606 - hosts 95042 ...`) and the line read "EasyList 202610241606 · EasyPrivacy 202610241606, updated 2026-10-24" (`t520-enyo-privacy2.jpg` of run 1, `t520-run1-host.log`); the second run, 25 minutes later, to EasyList 202610241630 with EasyPrivacy unchanged, the line following (`t520-run2-host.log`).)*

## R5, criterion 1 (removed 2026-11-07)

The run found EasyPrivacy refused as "truncated" on every update (its 57,139 lines held against both lists' rules together): each download is now held against its own list's copy.)*

## R5, criterion 3 (removed 2026-11-07)

The runs found the timer's one early look skipped when a page had started within 20 s, the next look six hours away: a due update now looks again 60-90 s later.)*

## R6, criterion 1 (removed 2026-11-07)

*(Measured 2026-10-15, Mochi idle after a cache load: v3 69.8 MB with, 33.4 MB without = +36 MB; v5.1 74.4 MB with, 34.2 MB without = **+40 MB**, and the `heap-minimize` after install changed nothing (74.4 before and after). The stylesheet costs nothing (hiding on/off: 70.3/70.1). The memory reporter after install (v5.1, 15:00): resident 72.2 MB, **JS GC heap 12.0 MB**, heap-allocated unavailable (that reporter is jemalloc's; this build has none): the JS heap holds the sets, the other ~26 MB is the bundled glibc's heap after the 3 MB cache parse, which it never returns on its own. Two changes: v6 keeps each host set as ONE string plus a typed-array hash table (one GC thing instead of 100,000), and the daemon `malloc_trim`s on the "jihad-blocker-installed" notification. **Re-measured (v6, 15:30, cache load, idle): 61.2 MB with the lists, 33.7 MB without = +27.5 MB; JS GC heap 7.0 MB; the trim returned pages.** Over the 12 MB budget still; what is left is the bundled glibc's retention after the 3 MB cache JSON is parsed — **v7** keeps the cache as plain files (`hosts.txt`, `tp.txt`, `exc.txt`, `cosmetic.css`, `meta.json`) read straight into the strings the sets keep, no JSON of the blobs and no parse: resident after install 56.0 MB (v6 59.7, v5.1 72.2), JS heap 7.0 MB. **v8** holds the lists only while a card is connected, so the idle cost of an unused variant is nil; the no-card idle figure is in the device log of 2026-10-15 16:30.)*

## R6, criterion 2 (removed 2026-11-07)

*(2026-10-24, T-520, the Enyo card, `performance.timing` read through the inject channel, the `blocked` log off, the twenty hosts mapped to the host PC so the blocking-off loads fail at once (the PC refuses port 443) instead of waiting on DNS, on and off alternating: blocking on 8592, 4852, 4498, 8078 ms (mean 6505); off 5746, 6016, 6304, 6108 (mean 6044): on is 7.6 % over off on the mean, within the 10 %; two of the four on-runs took a third longer than the others while the off-runs held steady, so the margin is the page's own variance under the daemon's tick, not a cost that grows with the refusals. The 4.5-6 s floor is the engine's per-image work on 200 images on this CPU, the same on both paths. `docs/rounds/t520-2026-10-24/t520-confirm-host.log`.)*
