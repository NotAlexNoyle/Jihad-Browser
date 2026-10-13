# PICKUP — Jihad Browser, current state (2026-08-10 body, deltas 2026-08-15 … 2026-10-12 below)

**Read this file first. It is the only current handoff.** Everything else in `docs/` is
reference material for a specific subsystem, not a status document.

---

## 2026-10-12 (night, later) delta — Tier 81: the engine on UXP RB_20261006 (READ FIRST)

**The owner:** "Proceed with the UXP update, with a focus on making future updates easier, and
optimizing the code where possible, while supporting modern web standards. target the latest
release".

**Done** (docs/ENGINE.md has the whole story)
- **Engine:** on UXP release `RB_20261006` (was master `b2594a4`, 2026-06-30). Our 39 commits came
  across with no conflict, one commit per patch now (the squash of 0001–0031 was split). The six
  files both sides changed were read; only `--disable-npapi-gtk2` had to go from the mozconfigs.
  `check-engine-api.py`: 0 FAIL, and the baseline was re-written with no signature changes. The
  branch is `jihad/uxp-RB_20261006` at `706d4782ba`, not pushed.
- **Future updates are two commands:**
  - `build/engine/update-engine.sh [TAG]`: puts the queue on the release with a three-way merge,
    lists the overlapping files and runs the API check.
  - `--finish`: writes the queue again, checks out the new branch and rewrites every pin (the user
    agent, start pages and OE recipe; the bundler reads the ICU name from `engine-version.env`).
  - `UXP_TAG` is now in the pins.
  - `render/goanna/EngineCalls.h` puts the 15 fragile positional engine calls behind named
    wrappers.
- **Optimised:** a release build (dead-code removal, identical-code folding) in Thumb-2.
  - libxul, stripped: 46.2 → 30.2 MB (−35%); its code −43%.
  - Payload: 80 → 73 MB per variant.
  - Idle daemon: 33.8 MB resident, against 36 MB on the old engine.
- **Modern web, measured with `render/goanna/test/modern-web.html`:** `adoptedStyleSheets`,
  `replaceSync`, `URL.parse()` and `revert-layer` now work. `:has()` works with its new Settings
  switch, and is off by default as in upstream releases, because it is slow on some pages. TLS 1.3
  confirmed on howsmyssl.com.
- **Fixed on the way:** `postinst`/`prerm` took the initramfs `rootfs` line for a read-write `/`,
  never remounted it, and failed on a read-only root. The generator now reads the real root
  mount. The daemon includes `nsIDocument.h`, which the new `StyleSheet.h` needs.

**Verified on the device** (a full `push-variant.sh` to each)
- All three: "11 of 11 optional jihad_* exports present".
- Enyo: the feature page before and after; `:has()` switched on, then back off; TLS 1.3; a Persona
  worn and taken off; Palefill enabled; about:config; Flash at 23–25 fps.
- Mochi: the feature page. Mojo: Wikipedia.

**Device state as left:**
- All three on the new engine (bundle `71cef1fd…`; libxul from `jihad/uxp-RB_20261006`), daemons
  running, `/` read-only, no test hooks.
- Default look in all three; `:has()` off.
- `/media/internal/jihad-modern-web.html` is left in place as a test asset.

**Rollback:** the old engine build is `build/webos-oe/out-arm/obj-jihad-goanna-arm.RB-b2594a4` (with
`jihad-browserserver-arm.RB-b2594a4`). Put it back as `obj-jihad-goanna-arm`, rebundle,
`build-variant-ipk.sh`, then `push-variant.sh` each variant.

**Open:**
- Codex round 22 at 22:31 (session cron; the queue is kept out of its diff).
- The finger round (`docs/FINGER-TESTS.md`, engine section added).
- The desktop test build is not rebuilt on the new engine yet (docs/ENGINE.md §7).

## 2026-10-12 (late night) delta — Tier 80: fonts in Settings; no Persona by default; card pictures in the background

**The owner's rules this evening (both in auto-memory):**
- All finger tests wait for one round at the end, collected in `docs/FINGER-TESTS.md`.
- No Persona is worn by default in any variant. Tests may wear one, but put every variant back to
  the Default look afterwards.

**Done:**
- **Fonts (T-438, `11f09e07`, preferences-ui R9).** Settings → Content → Fonts & Colors has fonts per
  script, as Pale Moon's and Basilisk's Fonts & Colors and Advanced fonts dialog offer them: the
  engine's own list of installed fonts, a sample line under each, and Restore default fonts. The
  page link `about:settings#pane=content&section=fonts` opens the section.
  - Licensing: no font is shipped. The device's fonts are used in place: HP's Prelude family, the
    Microsoft core fonts, and the CJK fonts webOS carries. The code is our own MPL-2.0.
  - Device, Enyo: Georgia was picked for Serif and the serif text was drawn in it; Restore default
    fonts brought back Times New Roman. The section draws correctly in all three variants.
- **Default look (T-440, `97f5e871`).** The test Personas were taken off in all three variants.
  Fresh profiles already wore none: nothing sets `isThemeSelected`.
- **Card pictures (T-441, `97f5e871`).**
  - The cause: making pictures inside the gallery's own request held it 200–600 ms a card, the
    budget ran out by the eighth, and decodes failed under image-cache pressure. Carbon Light kept
    its full header, the one the owner saw flashing.
  - The fix: a background queue makes them one at a time, takes the size from the decode, writes
    natively and discards each decode.
  - Device, Mojo: all 16 made in one opening; every card drawn.
  - Mojo ignores `palm-launch`'s target parameter, so open its pages through the test hook.
- **Server.** It replied (`41-savagery-shamrock`, kept in `scratchpad/server-reply/`): package
  `5c1d54c` is live, migrations 001 and 002 are in, 24 Personas were imported (39 listed), the
  smoke test passes, and the credits are served. Its notes:
  - the Persona list's column drift: fixed (Riverstone-Addons `f148f1e`, which also records the
    deploy in DEPLOY.md);
  - Free Palestine's `license` stays a sentence, because the site's card shows that field as its
    credit;
  - Free Palestine's id matches in both repositories.
  - Nothing more for the server until Codex round 22.

**Device state as left:**
- All three variants have the same `deviceroot/hl` contents (the chrome scripts are those of
  `11f09e07`; `push-engine-update.sh` prints the id); daemon `644eba8a…`; libxul unchanged.
- No test hooks.
- Default look in all three (no Persona worn).
- Enyo's Western font settings are at their defaults (Restore default fonts was run after the
  Georgia test).

**Open:** Codex round 22 at 22:31 (session cron), then the finger round when the owner calls it.

## 2026-10-12 (night) delta — Tier 79: the owner's evening list; review rounds 20–21

**The owner's list**
- Select All in the URL bar's Cut/Copy bubble.
- Momentum on Settings and about:personas.
- Personas flashing (Carbon Light 2.0).
- The keyboard down when Settings or about:personas opens.
- Extension settings cut off.
- A caret left in about:addons' search field after the keyboard closes.
- Typing in text boxes works (the owner's finger).

**Done (Jihad `272ce0cd`)**
- **Keyboard down on Jihad's pages:** opening about:settings, about:preferences, about:personas or
  about:addons from the address field or the app menu takes the focus off the address field.
  - Enyo blurs in `Browser.urlChanged`.
  - Mochi blurs in `keyboardAwayFor`, from `openUrl` and `urlChanged`.
  - Mojo blurs in `keyboardAwayFor`, from `openUrl` and on arriving by a link.
- **No caret left behind:** the daemon clears the page's focus when the window grows back by 120 px
  or more at the same width while a field is focused (`BlurFocusedEditable`). The engine's blur
  then reaches the shell as editor-unfocused.
- **Card pictures:** an installed Persona's card draws an 800 px copy of its picture
  (`<24 hex>-thumb.jpg` in `jihad-themes`), made at most 1.2 s' worth per opening and pruned.
- **Add-on preferences:** each preference is its own block, text fields are dark, and the
  desktop margins are trimmed.
- **Mochi flicks:** Mochi now defines `Mojo.handleGesture`, LunaSysMgr's callback for a card's
  gestures. Enyo 1 has it and Enyo 2 lacks it, so the earlier DOM `flick` listener could never fire.
  Flicks are passed in whole numbers.
- **Inspector round 20, fixed:**
  - about:addons measures only the view it shows;
  - a page's own scroll jump within 300 ms of a pan is now applied instead of lost;
  - CREDITS covers the README strip's CC pictures;
  - AMO card state, https-only redirects, and a full /tmp are handled;
  - formerly bundled Personas keep their licence lines;
  - the README is up to date;
  - dead code removed.
- **Site agent:** fixed the site's round-20 findings (`6845226`..`d66fbc5`). Colour Personas' ids
  now come from their colours (kits updated). There is a Cloudflare real-IP conf, and the smoke
  test checks the feed contract (37/37 locally).

**Verified on the device** (bundle `219f12cd…` plus the project-drawn licence line; daemon rebuilt
with the focus and echo changes; all three variants)
- **Focus, all three:** after focusing the address field, opening a Jihad page left the focus on
  BODY; example.org left it on INPUT. In Mochi and Mojo the window grew from 705 to 996 px, so the
  keyboard went down.
- **Card pictures, Enyo:** 2, then 9 copies were made over three openings. They are real
  800x53 / 800x80 pictures, the cards draw them, and each shows its licence.
- **Preferences, Mochi (Palefill):**
  - Before: the controls ran past the right edge and the page zoomed out to 0.78.
  - After: everything is inside the page, with no sideways scrollbar.
  - The manager still scrolls its detail box down to the preferences, so the add-on's name sits
    above, in a box the card cannot pan.

**Needs a finger (the owner)**
- Flicks coast in Mochi, Enyo and Mojo on Settings and about:personas.
- Closing the keyboard over about:addons' search field leaves no caret. A scripted tap focuses the
  field but raises no keyboard.
- The flashing is gone.

**Select All (T-436, `a90b7fe3`):** it can't be added to the URL bar's Cut/Copy bubble, because
the system draws that bubble (none of the shells, Enyo 1 or LunaSysMgr's sources do). The owner
chose select-on-tap for all three:
- Enyo: `selectAllOnFocus`, already there.
- Mochi: Enyo 2's `selectOnFocus`, new.
- Mojo: `focusSelectMode`, already there.

Verified on the device with hook steps `K` (focus), `S` (selection) and `RF` (refocus with a caret
at 3, against a bare input):
- Enyo and Mochi select 0–20 of 20 while the bare input keeps its caret.
- Mojo, blurred (`B`) then focused, selects 0–20 of 20.
- Don't use `RF` on Mojo: its widget swaps the input for a div and the address empties. It was
  removed from that hook.

**Reviews:** Codex ran out again at 17:3x after 389k tokens, before writing its findings; retry
**10:29 PM**. Its notes named the focus-clearing path and the AMO install flow as the riskiest. The
inspector is reviewing in its place (round 21): Jihad `6e7b31a0..a90b7fe3`, site `26a9fdf..d66fbc5`.

**Round 21 fixed (`5ccc75a5`; site `2923301`..`5c1d54c`):**
- **Daemon:** the keyboard-down blur moved into the guarded pump (it ran page script inside a
  socket callback). A field in a frame is cleared and reported by the daemon itself.
- **Card pictures:** PNG kept PNG, 1400 px, all pruned on each opening. Device, Enyo: the 800 px
  ones are gone, the 14 Persona pictures intact, the cards drawn.
- **about:addons:** hidden settings stay hidden; checkbox titles wrap. Device, Mochi: Palefill
  still fits.
- **Shells:** Mochi's blur no longer submits a half-typed address; Mojo no longer blurs twice.
- **Site:**
  - real IP in our own server block;
  - UXP's colour grammar;
  - migration 002 (unique `browserId`);
  - smoke test 43/43 with `RS_SMOKE_CONNECT`;
  - credits served at `/credits/*.txt`.
- Free Palestine's licence line reads "based on "Village of Battir" by Labour Palestine, CC BY
  2.0" in both repos.

**Sent to the server agent:** folder `jihad-reply-2026-10-12`, holding `REPLY.md` and
`riverstone-addons-5c1d54c.tar.gz` (sha256 `40a2dd4c147a899c…`). It went over wormhole-rs with
code **46-determine-drunken**, for the owner to relay. REPLY.md has the owner's overrides, what
was done per section, the feed contract, the deploy steps, and the memory-note changes.

**Device state as left:**
- All three variants on bundle `37c3757ca422b7bfddc35373b24b98fe`, daemon `644eba8a…`; libxul
  unchanged (`ddf8cc70…`); daemons running.
- Card scripts: Mochi `JihadBrowser.js`/`JihadWebView.js` and Mojo `main-assistant.js` as in
  `5ccc75a5`; Enyo `Browser.js` as in `272ce0cd`.
- No test hooks, no user.js, no test CA; `/tmp` 1% used.
- Enyo wears Space Fantasy, and its `jihad-themes` now holds 11 card pictures beside the 14
  Persona files. Mochi wears Aurora Silk, Mojo Lime Fizz.

**Open, in order**
1. **Codex round 22 at 10:29 PM.** A session cron (one-shot, 22:31) runs
   `scratchpad/codex/r22.sh` in four slices, newest first:
   - Jihad `6e7b31a0..HEAD`;
   - site deploy;
   - Jihad `4e480610..6e7b31a0`;
   - site PHP.

   A single run burned out at 389k tokens, so the range is split. If this session has ended, run
   them by hand with `run.sh`; it now takes `HEADREF` for an end commit.
2. **Finger tests: deferred, all of them, to one round at the end** (the owner, 2026-10-12). They
   are collected in `docs/FINGER-TESTS.md`. Add new ones there; don't ask the owner piecemeal.
3. **Server:** the server agent deploys per REPLY.md.
4. **Known gaps:**
   - the same palette from AMO and from the site is two Personas;
   - a layered Persona's id comes from the merged header;
   - in about:addons, a Preferences tap scrolls the detail box past the add-on's name, which the
     card cannot pan back to.

## 2026-10-12 (evening) delta — Tier 78: the server agent's work order, the gallery, each shell's look

**The inputs**
- The owner's asks: the gallery's load flicker; each shell's own buttons; a Free Palestine Persona,
  then "an artistic depiction of palestinian land", then "photo realistic".
- The server agent's work order, `/home/notalexnoyle/Documents/PROMPT-PC.md`: the gallery showing
  the site, bundling two, more catalogue formats, momentum, and about:addons in the webOS style.
- The owner's answers along the way:
  - bundle Free Palestine + Aurora Silk;
  - MIT colour palettes allowed, credited;
  - the gallery shows two previews per source, a full search of each and links to the websites,
    with the site's whole list staying on its own page;
  - addons.mozilla.org searched through its API, since its pages show this engine a robot check;
  - Personas managed in the gallery, because the card cannot scroll about:addons' lists;
  - about:addons dark, like Jihad's other pages: the light webOS look clashed.

**Done (Jihad `89571fb6`, `6128e29b`; Riverstone-Addons `b939443`..`26a9fdf`, built by a subagent to a fixed feed contract)**
- **about:personas:**
  - opens at once, and nothing moves or redraws while it loads;
  - Your Personas: wear, take off, remove (asks again), Undo in the toast, Default look, Install
    from file;
  - the bundled ones that aren't installed;
  - the site and AMO, two previews each, searched in place, each with a link to its website;
  - AMO Install fetches the theme from AMO's hosts only;
  - `#site-q=WORDS` links to a site search.
- **Bundle:** Free Palestine (from the CC BY 2.0 photo "Village of Battir" by Labour Palestine,
  credited) and Aurora Silk, edition 8. The rest are in the site's `catalogue/`, and a push now
  removes no-longer-bundled pictures from the device.
- **Look:** each shell's buttons on Settings, about:personas and about:addons (`shell.js`/`shell.css`,
  from `JIHAD_BS_NAME`). The settings tabs wrap. about:addons is dark with light text, the shell's
  buttons, toolbar buttons and dropdowns, and links drawn as links. Its Personas category is the worn
  one, Default look and a link to the gallery. The site's new discover pane is darkened inside it.
- **Scrolling:**
  - the daemon echoes no scroll while the card streams positions (an echo reset coasts on
    zoom-1 pages);
  - Mochi forwards flicks;
  - about:addons' root grows to the length of the list it shows, so the card can pan it.
- **Mojo:** the lost-connection panel floats under the bar instead of moving it.
- **Site:**
  - migration 001 (kind, colors);
  - the importer's three kinds (a colour theme gets a drawn header, layered ones are merged);
  - the feed's `kind`/`colors` strings;
  - 23 MIT palettes;
  - the discover pane in the Jihad skin;
  - the panel's text;
  - fail2ban's include;
  - DEPLOY.md's "The update of 2026-10-12".

**Verified on the device** (bundles up to `cddf3ea3…`, daemon `d200a97f…`, all three)
- **Gallery, Enyo:** the gallery against the live site and AMO; an AMO install through the confirm;
  Catppuccin Mocha from the local copy colouring the toolbar. Both profile tests were put back
  md5-identical.
- **Buttons and about:addons:** in all three.
- **Mojo:** the bar holds still during a lost connection.

**Needs a finger (the owner):** flicks coast on about:personas and Settings (Enyo, Mochi, Mojo); a
long about:addons list pans; typing a search.

**Device state as left:** all three on bundle `cddf3ea3941af6d8c055d7490c76d56e`, daemon
`d200a97f…` (libxul unchanged), daemons running; no test hooks, no user.js, no test CA; `/tmp` holds
nothing of ours; Enyo wears Space Fantasy (profile md5-identical to before the tests), Mochi Aurora
Silk, Mojo Lime Fizz; each profile has Free Palestine added (not worn). Local copy running with the
catalogue imported (40 Personas).

**Open, in order**
1. The inspector on both ranges (running), then Codex at 5:29 PM on the same ranges. Fix, record.
2. Pack the site (the catalogue travels inside it), write the reply to the server agent, send both
   with one wormhole code for the owner to relay.

## 2026-10-12 (later) delta — Settings: Personas, Extensions and Plugins, one button each

**Done** (`7cd6a158` and the round-19 fixes after it; `cavekit-preferences-ui.md` R8, build site
Tier 77, all criteria met). The owner asked for:
- a clear route to about:personas and extensions in Settings on all three variants;
- the extension not named as the row's subheading;
- plugins with the Flash Player's details;
- one button per row.

The Browser pane of about:settings now starts with a "Personas, Extensions and Plugins" group (blue
buttons). It replaces the old Appearance group:
- **Personas:** what is worn and how many are installed. **Browse Personas** opens about:personas,
  whose "Personas manager" link opens about:addons on its Personas list (take off, remove, Undo).
- **Extensions:** "Add features to web pages and to the browser. N installed" (and how many are
  off). **Manage Extensions** opens about:addons on its Extensions list.
- **Plugins:** "Adobe Flash Player 10.3 d185, on. From the HP Flash plugin installed on this
  device." **Manage Plugins** opens about:addons on its Plugins list.

**The inspector (round 19):** APPROVE, with 1 P2 and 4 P3, all fixed.
- Flash on "Ask to Activate" never plays here. The row now says so; the Content pane's Flash row shows
  it instead of a blank list; and the daemon's `getFlash` reports only state 2 as on, which needed a
  daemon rebuild.
- About reads the plugin host, so a Flash that is off no longer reads "not installed".
- The kits were updated for the new route.
- `render/goanna/test/prefs-addons-rows.test.js` covers the wordings (8 cases).
- The buttons are no longer the red of a confirming Clear.

**Verified on the device**
- All three variants: the group renders, on a page opened through each shell's own Settings entry,
  and each button lands on its page. The taps came from the hooks, not a finger; the pages were read
  back and photographed. The final bundle was re-checked on all three.
- Enyo, with `prefs.js` edited while the daemon was stopped (then put back md5-identical): state 1
  and state 0 each read the same in all four places. The manager link lands on the Personas list.

**Open**
1. Codex on `7cd6a158` and its fixes, when credits return (5:29 PM), together with the site's PHP
   part (afternoon delta, item 1).
2. The afternoon delta's other items are unchanged.

**Device state as left**
- All three variants on bundle `23953a34e209e90c358275c5e89c0dde`, daemon `4d8502ab…` (libxul
  unchanged); daemons running.
- No test hooks (the shipped manifests were checked by md5). No user.js, no test CA.
- Enyo's `prefs.js` is as before the test; Flash is on everywhere.
- Personas as below. `/tmp` holds nothing of ours. The test hooks gained Mojo's `t` (a tap).

## 2026-10-12 (afternoon) delta — round 17 verified on the device; Codex round 18 on the deploy kit fixed

**Verified on the device** (the TouchPad connected at 12:28; one task at a time)
- Bundle `eaa828a1877aa3b441213d51b6c2524e` on all three variants (the id read back from each).
  Enyo's `BrowserApp.js` and Mojo's `main-assistant.js` (round 17's UI fixes) were pushed with
  `push-card-js.sh`, and each card's boot stamp showed in the log.
- `test-site-profile.sh` under the device's BusyBox 1.17, on a scratch app
  (`JIHAD_TEST_APP=/tmp/fakeapp`, a copy of Enyo's profile):
  - setup, a test's changes, then restore → md5-identical;
  - a stray file planted in the backup → restore refused, profile untouched;
  - a restore cut off between its moves → finished.
  Then real cycles on Enyo and Mojo: both profiles md5-identical afterwards, with no CA, no user.js
  and no backup left.
- about:personas update check (local copy): the test Persona installed at 1.0.3. The site at 1.0.2
  → "Your Personas are up to date."; at 1.0.4 → an Update card for 1.0.4.
- Enyo, on a card started on the start page with a light-field theme: the reload and stop glyphs are
  drawn ink.
- Mojo share menu: a highlighted row's label takes `popup_highlight_text`. This was read from the
  stylesheet, with the row's `selected` class set by the hook, not under a finger.
- about:personas shows "Licence: MPL-2.0" on the five project-drawn Personas.
- Test tooling, now committed:
  - Enyo hook step `G` (the glyphs; it runs on the start page);
  - Mojo hook step `H` (a submenu's colours);
  - `card-selftest.sh … run <scenario> -` launches with no address.

**Codex round 18** (credits back at 12:28). The site range was split, because it never fits in one
window:
- **`deploy/`: done.** 2 P1, 5 P2, 1 P3, all fixed in site `9e5521b` and tested on the local copy
  and scratch directories (`impl-review-findings-6.md`, round 18):
  - php-cli installed by name;
  - both nightly archives watched;
  - a Persona import changes the files and the row together or not at all;
  - the smoke test accepts the allowlist's 403;
  - the owner account change is logged;
  - the local copy imports the Personas at every start;
  - unknown install.sh arguments are refused;
  - swap-in also refuses empty directories and special files.
- **The PHP part: ran out again.** Retry at **5:29 PM, 2026-10-12**.

**Open, in order**
1. Codex on the PHP part of the site. The prompt now tells it to stay off the web:
   `DIFFOPTS=--diff-filter=d EXTRA="This part is the PHP site (everything outside deploy/, which was
   reviewed separately)." scratchpad/codex/run.sh ../Riverstone-Addons upstream/TRUNK OUT LOG . ':!deploy'
   ':!libraries' ':!*.png' ':!*.jpg' ':!*.gif' ':!*.ico'`. Then fix, record, and stop the local
   copy (`deploy/local/down.sh`, no wipe).
2. The deploy guide: the copy sent with round 17 (`41-confidence-breakup`) timed out unreceived.
   DEPLOY.md is unchanged since `b9810ad`. Send it with wormhole-rs when the owner asks for a code.
3. The owner's items from the 2026-10-11 delta (deploy, typed search, landscape) are unchanged. The
   root-on-Devuan parts of the deploy kit (OpenRC, cron, nginx, the login message) are checked only
   by reading the code.

**Device state as left**
- All three variants on bundle `eaa828a1…` (daemon `5183c623…`, libxul unchanged `ddf8cc70…`),
  daemons running. Enyo's and Mojo's app sources are as committed, plus dev stamps from the pushes.
- No test hooks: the shipped manifests are back, checked by md5. No user.js, no test CA.
- Enyo wears Space Fantasy and Mojo Lime Fizz; both profiles md5-identical to before the tests.
  Mochi was untouched and wears Aurora Silk.
- `/tmp` holds nothing of ours (YAP sockets only). No Jihad card is open (the last LunaSysMgr restart
  closed them).
- Local copy `riverstone-local` running (PHP 8.4), 16 Personas; the test Persona is back at 1.0.3.

---

## 2026-10-12 delta — Codex round 17 on the Jihad range fixed; the site range still owed

**Done**
- The updated deploy guide reached the owner's desktop over wormhole-rs (the first send waited out
  its two hours unreceived; a fresh code worked).
- **Codex finished the Jihad range** `69c44675..83f0e817`: 1 P1, 5 P2, 4 P3, all fixed in
  `df590b3b` except one kept by design (`impl-review-findings-6.md`, round 17):
  - `test-site-profile.sh` judges every copy by md5, never by `cp`'s status (the device's BusyBox
    `cp` can report EBADF for a copy that landed). The backup records each folder's entry count
    (hidden and nested entries too) and every file's md5; restore refuses a backup that does not
    match, before touching anything. `extensions.ini` and `addons.json` are backed up too. A restore
    that does not finish leaves the daemon **stopped** and the backup kept: run restore again.
  - about:personas offers only a higher version as an update; an answer that is not about the
    Persona counts as not checked.
  - A colour-only Persona's drawn picture keeps translucent colours translucent (opaque ones are
    byte-identical to before, so their ids do not change); a multi-picture theme failing part-way
    removes the pictures it put in place.
  - Enyo: the ink reload/stop glyphs on a light field no longer need the browser view to exist.
    Mojo: highlighted menu labels take the theme's highlight text colour.
  - By design: updating or re-installing the worn Persona has no Undo (`cavekit-personas-ui.md` R3).
- Verified here: `test-site-profile.sh` in a local simulation (dash, a fake novacom and upstart, the
  engine's certutil): a full cycle left the profile identical; a `cp` that copies and then reports
  EBADF → setup and restore still succeed; a folder copy really cut short → daemon stopped, backup
  kept, the second restore finished identical; a changed nested file, an added hidden file, a
  changed backed-up file and an old-format backup are each refused with nothing touched. The
  gradient: opaque output identical to the old code's, the translucent one decodes as RGBA.

- **The inspector on the round-16 deploy fixes** (`cdaed4c`): REVISE, 1 P1 + 3 P2 + 9 P3, all fixed
  in site `b9810ad` (round 17 in the findings file). install.sh puts back nginx rules this server's
  nginx rejects and stops (the P1: `nginx -t && …` never stopped it), clears its stamp while it
  runs, runs only as the copy in place and needs cron. The new `migrate.sh` brings the database up
  to a copy BEFORE `swap-in.sh` puts it live (dump first; a migration only adds and runs twice
  harmlessly; a rollback names what it lacks). The new `watch.sh` (cron.daily) puts certificate and
  backup trouble on the panel's front page and in the login message. `swap-in.sh` refuses three
  more ways of losing the way back; `pack.sh` sends only `jihad`; the local copy re-issues its
  certificates and applies migrations. Tested on a throwaway MariaDB 10.11, scratch copies and the
  local copy (34/34); the root-on-Devuan parts are code only.

**Open, in order**
1. **Codex on the site range** `upstream/TRUNK..jihad` (now through `b9810ad`): it ran out again at
   ~150k tokens; retry **12:28 PM, 2026-10-12**. The owner's deploy guide changed with `b9810ad`:
   send it to the desktop again.
2. **When the TouchPad is connected** (one task at a time):
   - push bundle `eaa828a1877aa3b441213d51b6c2524e` (it also carries the plain `MPL-2.0` licence
     line) with `build/webos-oe/push-engine-update.sh --daemon-only enyo mochi mojo`;
   - reinstall the Enyo and Mojo app packages (the glyph and menu fixes are UI code);
   - run `test-site-profile.sh` on a scratch app directory first (`JIHAD_TEST_APP=/tmp/fakeapp`,
     under the device's BusyBox 1.17; the simulation used dash), then a real cycle on Enyo;
   - then check the about:personas update check against the local copy, the Enyo glyphs on a light
     field, the Mojo highlighted menu text, and an about:personas card's "Licence: MPL-2.0".
3. The owner's items from the 2026-10-11 delta (deploy, typed search, landscape) are unchanged.

**Device state as left:** unchanged since 2026-10-11 (below); the TouchPad was not connected at
00:32 or at 07:48 (`novacom -l` empty). Nothing was pushed.

---

## 2026-10-11 delta — Jihad's add-on site (built and tested locally, not deployed), three more Persona formats, review round 14 (READ FIRST)

**The owner's choices** (the Persona-index options, 2026-10-11): a Phoebus add-on site at
`addons.riverstonerelay.org`; colour-only themes, layered pictures and more colour slots; "Originals +
vetted CC"; build it locally first, with nothing published; their Devuan + nginx + OpenRC server;
the full Phoebus feature set, with Personas the only content for now; about:personas as a native
gallery plus the site; a Palm-era skin; submissions from the owner only; updates only when asked.
Kits: `cavekit-persona-formats.md`, `cavekit-addons-site.md`; build site Tiers 75–76 (T-390–T-414).

**Built**
- **Jihad** (`9e104699`, `7bf951a1`, `efa6844a`, `948e1c88`):
  - Colour-only and layered theme packages install as Personas. Extras live beside the Persona in
    `profile/jihad-themes/extras-<sha24>.json`.
  - A theme's other colours reach the address field, the menus, Mojo's command row and the start
    page.
  - Prefs point at the site: getAddons, discover, update, `galleryURL`/`siteURL`, and the install
    permission.
  - about:personas pages through and searches the site's Personas, and has "Check for updates".
  - No background update checks.
- **The site** is in `../Riverstone-Addons`, branch `jihad`. **Its only remote is `upstream`, which
  is Moonchild's Phoebus: never push there.** The owner creates the project's repository and adds
  it as `origin`. What it has:
  - the fork's hygiene, with licences and NOTICE;
  - PHP 8.2/8.4;
  - hardening;
  - Personas with an importer from `render/goanna/personas/personas.json`;
  - a JSON feed;
  - the Jihad skin;
  - a rootless local copy (`deploy/local/up.sh` / `down.sh`);
  - the deploy kit (`deploy/DEPLOY.md`, `deploy/server/*.sh`).

**Verified on the device** (against the local copy over the LAN, with its test CA in a test profile)
- Colour-only and four-layer Personas in all three shells. The colour-only bar is now stretched:
  Enyo measured #3e2164 at the top and #7c3aa8 at the bottom.
- The colour slots in all three shells. A classic Persona puts every slot back.
- Uninstall leaves nothing behind (Enyo).
- Re-installing the worn Persona with new colours repaints it (#cc2222).
- `hsl()` arrives as `rgb()`; a value carrying a shadow is dropped.
- An SVG-only theme: its picture is left out, and the Persona is offered and worn on its colours.
- The site's feed in about:personas. Its pager goes from page 1 of 2 to 2 of 2; "More on the web"
  opens `/personas/`.
- Install from the feed. Check for updates → Update (the picture's md5 changed).
- Wear on the site's page. The discover pane.
- about:addons Check for Updates, 1.0 → 1.1.
- Offline, about:personas says the site cannot be reached.
- Screenshots in Enyo: the front page, `/personas/`, an add-on page and a 404 in the skin.

**Verified on the local copy**
- Smoke test 31/31 on PHP 8.4 and on 8.2, with every page type and the panel's pages; the PHP log
  stayed empty.
- A POST without the token, or from another origin, gets 403.
- A hostile Persona name shows as text in the page, in the `data-browsertheme` JSON and in the feed.
- An unreviewed add-on is 404 publicly and still served by the panel.
- `persona-admin.php` hide, show and delete work; the delete leaves symlink targets alone.

**Reviews**
- Codex r13 (Jihad, morning): fixed.
- Inspector round 14, standing in for Codex, on both repos: one P1 (the panel path) and the
  P2/P3s, all fixed. See `context/impl/impl-review-findings-6.md`.
- Inspector round 15, on the round-14 fixes: APPROVE for both. The P2 (picture sizes could be
  under-read) and eighteen P3s are all fixed, and the dangerous ones were checked on the device.
- **Codex owes a review** of Jihad `69c44675..HEAD` and of the site's `upstream/TRUNK..jihad`.
  It ran out of credits again at 17:00, and again at 22:10 (after ~216k/243k tokens, before writing
  a finding); retry at **3:07 AM, 2026-10-12**. The site range now includes the wormhole-rs deploy
  (`8535d3b`..`cdaed4c`). The inspector reviewed it as round 16 (APPROVE, 2 P2 + 12 P3, all fixed in
  `cdaed4c`: copies checked against a manifest before going live, `www.prev` as the way back, an
  install stamp deciding when install.sh must run, `deploy/migrations/`, the certificate warned of
  by the smoke test; `impl-review-findings-6.md`).
  The wrapper is `codex exec -m gpt-5.6-sol -s read-only` with Cavekit's prompt and the diff (see
  the codex-gate memory); give the site diff `--diff-filter=d` and leave out `libraries/`.

**TRAP — it did real damage.** The TouchPad's BusyBox 1.17 `grep` has **no `-x`**.
`test-site-profile.sh restore` tested membership with `grep -qxF … || rm`. Every test errored, so
both of the day's Enyo restores deleted *everything* in `profile/jihad-themes/` and
`profile/extensions/`.
- Fixed, and then redesigned in round 15: restore no longer decides what to delete by matching
  names. Setup copies the whole of `jihad-themes/` and `extensions/` (and every profile file a test
  touches) into a backup that is written as `$BAK.tmp` and renamed only when complete. Restore
  refuses an incomplete backup or a short folder copy, and otherwise swaps the folders back.
- Tested on a scratch profile on the device (`JIHAD_TEST_APP=/tmp/fakeapp`): a test's added,
  deleted and rewritten files all came back md5-identical; the refusals changed nothing; an
  interrupted restore finished on a second run. Then two real cycles on Enyo were md5-identical.
- Enyo repaired:
  - its 13 pictures copied from Mochi under the same names (Mochi's and Mojo's copies are
    md5-identical);
  - Palefill re-offered by deleting `jihad.bundled-addons.palefill@addons.martoks-place.de.offered`,
    which reinstalled 1.30, active.
- The jihad-site-test add-on (a test file) went with it.
- **After any device-side clean-up, count what is left, not only what was meant to go.**

**Open, in order**
1. **Codex re-review** of both ranges (above). Fix the findings, then re-test on the local copy and
   on the device.
2. **[OWNER] Deploy**, following `../Riverstone-Addons/deploy/DEPLOY.md`, which is now numbered
   steps in the order they happen. **Every file goes over magic-wormhole-rs** (`wormhole-rs`; the
   owner's rule, 2026-10-11: no other wormhole may be installed, not the Python `magic-wormhole`
   that apt and xbps offer). `deploy/server/pack.sh` sends the last commit, and `swap-in.sh` puts it
   in place on the server; the Personas and the backups go the same way (a folder arrives as
   `<name>.tar`). `sync.sh` (rsync) is gone (site `8535d3b`, `fd41f7e`). In short:
   1. wormhole-rs is already on the PC (`~/.cargo/bin`) and on the server.
   2. Point DNS for `addons.riverstonerelay.org` at the server.
   3. Send the site and put it in place with `swap-in.sh`.
   4. On the server, as root, run `install.sh`, then `make-owner.php` (it asks for the password).
   5. Send the Personas and import them.
   6. Run `smoke-test.sh`.
   7. Install a Persona from a desktop Pale Moon or Basilisk, and from Jihad (R10).

   A repository of the site's own is optional (hosting doesn't need one); its only remote is still
   Moonchild's `upstream`, which is never pushed to.

   Until then, Jihad points at a site that does not exist. That is by design: about:personas says it
   cannot be reached, and the discover pane shows its offline message.
3. **[OWNER, finger]** Typed search in about:personas (the on-screen keyboard).
4. **[OWNER, hands]** The site in landscape on the TouchPad itself (R7). A desktop, a phone and
   1024×768 were checked in headless Firefox; they read well.
5. Undo, Default look and editions with the new formats have been checked in code only.

**Testing against the local site**
1. Run `../Riverstone-Addons/deploy/local/up.sh` (`PHP=8.2` for 8.2). The CA is at
   `deploy/local/state/tls/ca.crt` and the owner password at `state/owner-password`, both
   git-ignored.
2. Run `build/webos-oe/test-site-profile.sh setup <variant>`, test, then `restore <variant>`.
3. Run `status`, and count `jihad-themes/` and `extensions/`.

Click coordinates: Enyo's and Mochi's `clickAt` take view pixels (device y − 82); Mojo's are
card-relative (device y − 28). Only a finger pans, so jump with anchors (`about:personas#online-page`,
`#check-updates`), and use a `U<url>` step to re-jump after a tap. Phoebus refuses curl's user agent;
pass `-A JihadBrowser/1.0`.

**Device state as left**
- **Not yet on the device:** bundle `b3476e74b62eb6202250da58e0020bc6` (the five project-drawn
  Personas' licence reads plain `MPL-2.0`, without "(drawn for Jihad Browser)"; the owner's ask,
  2026-10-11). The TouchPad was not connected (`novacom -l` empty); push it with
  `build/webos-oe/push-engine-update.sh --daemon-only enyo mochi mojo` and check an about:personas card.
- All three variants on bundle `e6f75a783e82e5717c7b1b9e8d487008` (daemon-only pushes, round-15
  component), daemons running.
- No test hooks, no user.js, no test CA.
- Enyo wears Space Fantasy, with 13 pictures and Palefill 1.30 active. Mochi wears Aurora Silk and
  Mojo Lime Fizz.
- Test Personas removed from all three. Carbon Light's bundled details restored in Enyo.
- `/tmp` holds nothing of ours. The 18 stale YAP sockets of dead daemons were removed; the live
  ones were kept.

## 2026-10-08 (night) delta — complete themes, the Persona gallery, a licensed bundle, engine-update groundwork (READ FIRST)

**The owner's eight asks and what came of each:**
1. **Complete themes** — investigated (`docs/research/complete-themes.md`): they skin XUL chrome,
   and Jihad's look lives in the cards, which only Personas reach; wearing one would switch the
   Persona off, needs a restart path this embedding lacks, and a skin missing a required file can
   stop the engine at start-up. Decision: not supported. `jihadInstallPrompt.js` explains and
   points to Personas; a toolkit-targeted one that would install is taken straight back out.
   **Device (Enyo):** a Pale Moon complete theme → refused with the message; a toolkit one →
   installed then removed, nothing left in the profile.
2. **"Get Personas" repository** — `about:personas` (packaging/prefsui/content/personas.*), Jihad's
   own gallery (Pale Moon/Basilisk use their Phoebus sites; Basilisk's `lightweightThemes.getMoreURL`
   is honoured). Cards with picture, author, licence, Wear; an online https index
   (`jihad.personas.galleryURL`, empty by default) installs through the usual confirm — **the owner
   decides whether and where to publish one.** **Device:** Wear from the gallery works.
3. **Licences** — survey `docs/research/persona-licences.md`. Bundled: our five + ten CC BY / BY-SA
   Firefox-era Personas (pinned sha256, `import-third-party.py`; credits `CREDITS.md`, `NOTICE`,
   cards). Removed: Water-Like test assets (ARR; tests use Carbon Light), every README picture with
   an unlicensed Persona (retaken), and those Personas from the device profiles (Running Foxes,
   Aurora Borealis Delights, Blue Nebula, Dark Fox, Purple Night, Water-Like, test Personas).
4. **Names** — Bubble Lagoon, Cloud Nine, Meadow Morning, Aurora Silk, Lime Fizz (formerly Aero
   Aqua/Sky/Meadow/Ribbons/Orbs; renamed in place on every profile, uninstalled ones stay so).
5. **Clean-up** — see "Device state" below; scratch files deleted at the end of the session.
6. **README** — 333 → 201 lines (features that matter; no long-press notes; dev notes → pointers).
7. **Finger tests** — deferred by the owner; every `[human at the device]` box in the kits says so.
8. **Engine-update groundwork** — `docs/ENGINE.md` (read it before touching the engine);
   `build/engine/`: `engine-version.env` + `check-engine-pins.sh` (all pins agree),
   `patch-queue.sh --check` (the 39-patch queue reproduces `UXP_PIN` exactly — it had drifted:
   0031–0039 added, obsolete files removed), `check-engine-api.py` (0 FAIL on today's engine);
   `render/goanna/EngineCompat.h` declares every `jihad_*` export; the daemon logs
   "engine: 11 of 11 optional jihad_* exports present" at start-up. `docs/ENGINE-SOURCE.md` (the MPL
   record) lists all 39 changes. Kit `cavekit-engine-update.md`, Tier 74 (next steps T-385…T-387).

**Note:** the five renamed picture files landed in the engine-groundwork commit `30a4970b`
(staged renames rode along); the content is right, the commit boundary is not.

**Reviews:** the inspector's round 12 over `2cc01662..392c39e3`: ten P3s, all fixed (see
`impl-review-findings-6.md`). Found while verifying: removing a complete theme left its skin selected
and the Persona queued off for the next start — fixed (prompt clears both), Enyo's profile repaired.
**Codex owes a pass over `9e08c610..HEAD`** (out of credits until 10:07 PM 2026-10-08).

**Device state left:** all three variants on bundle 143 (daemon with `EngineCompat.h`, the
gallery, 15 bundled Personas, the complete-theme fix); Personas worn: Enyo Space Fantasy, Mochi
Aurora Silk, Mojo Lime Fizz; profiles hold only licensed Personas; test hooks restored to the shipped
manifests (Luna restarted); the device's `/tmp` test pages and scripts removed, and 76 stale
`/tmp/yapserver.jihad-browser*<pid>-<n>` sockets of dead processes removed (see below); host test
servers stopped; session scratch (~900 MB) and `~/.cache/jihad-personas` deleted —
`import-third-party.py` re-downloads and re-checks the pinned packages when run.

**Minor, open:** each card connection creates `/tmp/yapserver.<app><pid>-<n>`; `YapClient`'s
destructor unlinks it, but killing LunaSysMgr (every `luna-restart.sh`) skips that, so they pile up
until a reboot (0-byte files on the 40 MB tmpfs). A daemon-side sweep of dead-pid sockets at start-up
would fix it without touching the wire contract.

---

## 2026-10-08 delta — taps land where drawn, long-press menus back, the Personas manager, a Frutiger Aero set, the README (READ FIRST)

**Owner's reports this day and what came of each** (all device-verified unless marked):
- **"mojo tap targets seem inaccurate … I get duckduckgo when I click wikipedia"** — FIXED, three
  causes: the daemon divided adapter coordinates by the zoom a second time (since 2026-08-02; every
  shell, any zoom ≠ 1); Mojo's widget single-tap handler clicked a second time; the device's
  PGContext blit ignored the header strip, so the page was drawn one strip higher than taps were
  mapped. Mojo start page: one `clickAt` on the drawn Wikipedia link, opens it.
- **"using the url bar to search the web is slow"** — the first-paint hold is at most 1 s per
  document (it waited for load end, 12 s on Wikipedia). Owner's finger check pending.
- **Long-press link/image menus never appeared (found while taking screenshots)** — FIXED: the
  device's stock `/etc/palm/browser/HitTest.schema` does not compile in the pbnjson the adapter
  links (rejects even `{}`), and a rejected parse left the hit-test object empty. The adapter now
  validates against its own embedded schema. Enyo: link → Open In New Card / Share Link / Copy URL;
  image → Copy To Photos / Share Image / Set Wallpaper; Mochi's link sheet opens.
- **"make it user friendly to manage and install personas, using about:addons as a base"** — new kit
  `cavekit-personas-ui.md`, built (Tier 73, T-365…T-371). `components/jihadPersonas.js` decorates
  the engine's own about:addons (no engine change): category "Personas"; rows with the Persona's
  picture, author, version (1.0 default), In use first; Wear / Take off / Uninstall (44 px);
  Default look; an Undo bar for 30 s; Get Personas (AMO); Install from file (`.xpi` in
  `/media/internal/downloads` or `/media/internal`). Settings → Appearance is a summary with
  Manage Personas. Verified in Enyo, Mochi and Mojo. Owner direction folded in the same day: Try
  dropped for Wear + Undo, author and version on all rows, "Persona" everywhere (confirm, toasts,
  Settings, README), "Uninstall" not "Remove".
- **"add fruiter aero style themes"** — five original Frutiger Aero Personas
  (`render/goanna/personas/make-aero-personas.py` draws them; bundled under `personas/`), added once
  to each profile's list (not worn); an edition bump replaces the set's older pictures.
- **"blue nebula looks like it isnt working properly"** — wallpaper-sized pictures (AMO's
  3840×2160) exceed the 32 MB image cache and never drew in the engine: kept 1600 px wide at
  install, and the cards fit them to the bar (`headerFit`). Already-installed big ones need a
  reinstall.
- **"dark fox preview … not working"** — a row added while the pane is open is redecorated once its
  XBL binding attaches.
- **README** — rewritten as a webOS-era product page (card view, Personas strip, framed shots, the
  sourced HTTPS-on-webOS history). Art rebuilt by `docs/screenshots/readme-art.py` from raw device
  captures (Prelude fonts from the device, not in the repo). howsmyssl.com reports TLS 1.3.

**Found on the way, fixed:** the `resource://jihad-themes` mapping was never made after a daemon
restart (the constructor runs before the profile is known) — converted Personas lost their
pictures; the pane's `window.setTimeout` from the component never fires — the component uses its
own nsITimer; the daemon re-drove a page's load of an about: page by its resolved file address
(about:addons → `file:///…/extensions.xul`, refused as XUL from a file) — now by the original.

**Reviews:** Codex r9 (taps/first paint/hit-test, 4 findings, all dispositioned); the inspector r10
on the owed range `20310b3e..e7b9aa7d` (Codex ran out mid-run; P1 Mojo widget restoring zoom 1.0
on Back/reload, pinch drift, header after a link, stale meta viewport, setHeaderHeight — all fixed,
Back verified); the inspector r11 on the Personas work (8 findings, all fixed or already fixed) in
`context/impl/impl-review-findings-6.md`. **Codex owes a pass over `9e08c610..HEAD` when its credits
return (it said 10:07 PM on 2026-10-08).**

**Get Personas verified against the live AMO:** its Persona pages' **Download file** link installs
through the usual confirm (the big button only offers Firefox). **Still open:** the empty
state (needs every Persona uninstalled); double-tap speed and block choice; Mochi text soft at
~1.43; the owner's finger pass over all of the above; coordmap_test (fixed to the contract, not run
— no desktop engine build on this host).

**Device state left:** daemons, components, settings page and the Aero set (edition 3) deployed
to all three (bundle 137); adapters in `/usr/lib/jihad/<variant>/` with md5 enyo a952549dafab,
mochi 819269398a78, mojo 76f9d6979c55 (rootfs ro). Personas worn: Enyo Aero Ribbons (its Aero Sky
uninstalled for the edition test), Mochi Aero Ribbons, Mojo Aero Orbs. Test hooks restored to the
shipped manifests in all three (Luna restarted). The host's theme test server (8091) and http.server
(8092) are stopped. `/media/internal/downloads/dark_fox-2.0.xpi` is left there from the
Install-from-file test. Reviews: the inspector's round 11 on the Personas work is in
`impl-review-findings-6.md` (8 findings, all dispositioned).

---

## 2026-10-07 delta — owner's finger test: Mojo width loop fixed, menus polished, About page, README, Flash games (READ FIRST)

**The owner's finger-test report (2026-10-07) and what came of each item:**
- **Mojo "Wikipedia glitches, jumps, a stripe at the left"** — FIXED (daemon): the meta viewport was
  re-sent whenever its computed initial scale changed; Mojo sizes its node from that scale, so the
  page shrank each round (768 → 526 → … → 76 px). Re-sent now only when the page's declaration
  changes. Verified on en.m.wikipedia.org.
- **Long-press "not working on anything", all three** — NOT REPRODUCED. The owner's holds DID reach
  the engine (`hold at …`, `page-hold … (pushed)` in all three logs). A finger-like scripted
  press → hold → lift (new hook steps: Enyo/Mochi `J[ms]`, Mojo `Z0`) opens and keeps the menu in
  all three, whether it opens before or after the lift. The engine's answer takes ~1 s from
  finger-down. **Needs the owner:** does nothing appear at all, or does something flash and vanish,
  and how long was the finger held?
- **Mojo circles not themed** — FIXED: accent-colour fill, glyph set by text colour, bookmark glyph
  included.
- **Mochi dropdowns** — rounded corners; the app menu now flush with the left edge.
- **Mochi pinch makes text blurry** — OPEN, partly characterised: the daemon re-renders at the new
  zoom (frames at `contentZoom` = zoom, 1:1), unpinched text is equally sharp in Mochi and Enyo
  (edge metric 34.2 / 34.5), Mochi at 1.25 and Enyo at 1.28, 1.35 and 1.87 are crisp, Mochi at
  1.43 is soft. Not a stretched bitmap (the card's scaler is nearest-neighbour). Cause not found.
- **Home button default** — DuckDuckGo Lite in all three and on the settings page; a stored old
  default (`start.duckduckgo.com`) moves once, a home the user set is kept.
- **Enyo refresh icon** — 2 px higher (measured: field rows 38–66, glyph 47–59).
- **Flash test bookmark** — the page lives at `/media/internal/jihad-flash-test/` (survives reboots
  and reinstalls; a copy is in `render/goanna/test/flash-test-page/`), bookmarked in all three
  through each shell's own Add Bookmark. It now also links five real games on archive.org (direct
  SWFs: Helicopter, Bloxorz, Motherload, QWOP, Alien Hominid). **Bloxorz verified running** as a
  full-page plugin document. Enyo's address bar sent a typed `file:///` address to search — fixed.
- **Flash games / Bluetooth** — research in the session scratch (`flash-research.md`): Armor Games
  embeds the real SWF (should run natively), Y8 needs a UA tweak, Kongregate/CrazyGames/Newgrounds/
  archive.org item pages force Ruffle, archive.org DIRECT downloads work. webOS 3 pairs Bluetooth
  KEYBOARDS natively (keys reach the card and the adapter forwards them to the page) but has NO
  Bluetooth mouse support at all (pairing refused for non-keyboard HID) — a mouse is not feasible
  without OS changes; the touchscreen is Flash's mouse (double-tap for gesture lock). Keyboard +
  Flash still needs a real keyboard to verify (`key.html` on the test page).
- **About page** — Settings → About: interface, version, app id, engine and build, Flash version,
  user agent, licences and credits, read at run time. Verified (Mochi).
- **README** — rewritten around what only Jihad does on webOS, with screenshots in
  `docs/screenshots/`.

**Review:** Codex ran out of credits mid-review again (retry 12:41 PM) → inspector on the round-8
fixes (round 8b in `impl-review-findings-6.md`): one P1 — URLs still unbounded on other messages,
and an oversized YAP string reached the adapter as NULL → `strlen(NULL)` in the card host; fixed in
the daemon and in the shared YAP codec — plus five P3s, all fixed.

**Still open:** Mojo pinch (pinch in then out → grey; structural — see mojo-ui R6; proposed:
viewport-sized node in Mojo with the adapter owning pan/zoom, as in Enyo/Mochi); Mochi blur at
~1.43; long-press by finger (needs the owner's detail); slow pages can paint unstyled while their
stylesheets load (the offscreen renderer does not honour paint suppression); Mojo focuses the
address field (keyboard up) when launched with a URL; the Mochi profile carries test personas in
its used-themes list (Forget them in Settings → Appearance).

**Later the same day (owner: "long press works", "proceed with mojo pinch fix", "mojo themed
buttons are still broken", "the buttons and virtual keyboard should not be touching"):** Mojo's
WebView is now a screen-sized view with the adapter owning pan and zoom (mojo-ui R6 — verified pinch
in/out by script; **panning needs the owner's finger**); the themed circles keep the framework's
25 px borders so glyphs stay centred; the command row sits 23 px above the keyboard. Adapter: one
gesture path for every shell, pinch centre in window pixels. Long-press: confirmed working by the
owner.

**Second Mojo pass (2026-10-07/08, owner: scrolling dead, double tap dead, circles flash at
load, Wikipedia unstyled until touched, "I want mojo's toolbar to scroll off"):** all addressed —
see mojo-ui R6. Two of them were not Mojo's: the daemon's double-tap (smart zoom) request had
always been a stub (every shell), and the offscreen renderer ignored the engine's first-paint
gate (every shell; now held up to 5 s and fully repainted on release). Panning by finger in the
new Mojo model, and double tap in Enyo/Mochi, still want the owner's hands.

**Device state:** all three daemons on this round's build; adapter on the rootfs for all three
(md5 enyo `83e2b9f0…`, mochi `5851cff3…`, mojo `77d28024…`); rootfs ro; test hooks restored on all
three (shipped manifests); Flash test page + bookmarks
as above; theme Water-Like on all three.

## 2026-10-06 (night) delta — Codex r7 fixed; menus match on all three; Flash regression fixed and drivers off; Mojo pinch fixed (READ FIRST)

**Owner's asks this session:** "continue implementing everything you can using cavekit 3.1 with
codex adversarial review" plus: the upper-right menu the same on all three (Atlas as reference);
"optimize flash player for gaming performance"; DuckDuckGo Lite/HTML as search options, Lite the
default. Mid-session: "mojo should have no top toolbar icons next to url. bottom circular buttons
only"; "put the mojo refresh icon embedded in the right of the urlbar instead of the bottom as a
circular button"; "pinch to zoom is broken. the page turns grey and I get the copy/select
all/reload/find on page menu instead". The (evening) delta below is superseded: personas are done
and committed (`20310b3e`), and all three shells were device-verified with Water-Like.

**Verified on the device (TouchPad 3.0.5, one task at a time):**
- **Codex round 7 (12 findings) fixed** — table and device evidence in
  `context/impl/impl-review-findings-6.md` (round 7): referrer/policy carried on re-driven
  navigations (6 cases on the echo server's `/ref?v=`), title flood bounded and coalesced
  (`/titleflood`: 12 sends in ~6 s, ≤1024 bytes), theme preview only from https+"install"
  (http preview ignored), static-theme package limits/pending file/hash id, theme origin taken at
  the event, Enyo theme on the lazily created views (start page and browser bars both painted).
- **Menus:** Mochi's upper-right is now Share ▾ / New Card / Bookmarks ▾ and its title-bar app
  menu (Find on Page · Settings · Print) opens from a real sysmgr relaunch. Mojo has NO icons by
  the URL (owner) — Share ▸ and Bookmarks ▸ are submenus on the bottom circles (History circle
  became Bookmarks), reload/stop sits inside the address field, the app menu is Find · Settings ·
  Print; new Bookmarks / Downloads / Print scenes. Enyo was already the reference.
- **Flash, gaming:** a regression from `debd1934` made every frame a full paint (15-16 fps,
  ~25 ms paints); fixed (34 fps paints, 4-5 ms, all partial). The child's own timer is now off by
  default and the daemon only pulls after 150 ms of plugin silence: one producer, histogram at
  32-39 ms, a third less tick work — and the CONTENT rate (what a game's speed is), measured with
  a grid SWF and two timed screenshots, is the same as with the old drivers (22.5 vs 22.1 fps):
  they only re-drew frames. Knobs: `JIHAD_PLUGIN_SELF_DRIVE=1`, `JIHAD_PLUGIN_QUIET_MS=0` restore
  the old behaviour for an A/B. Shorter plugin ticks (10/12 ms) changed nothing.
- **Mojo pinch:** two adapter defects on the path only the Mojo widget uses (JS `gestureStart`):
  the first finger's hold timer was never cancelled (→ the long-press menu with Mojo's grey scrim),
  and the adapter zoomed/scrolled itself on top of the widget's own pinch (→ grey page). Fixed in
  `BrowserAdapter.cpp` (`widgetGestureStart`, hold dropped if a gesture began since). Verified with
  the hook's finger-like `Z` step (a held-still control does raise the menu; the pinch does not;
  engine zoom tracks the widget 1.01 → 1.20; page drawn in and back out). **A real finger pinch by
  the owner is still the final word.**
- DuckDuckGo Lite default (Mochi reads `duckduckgo-lite`), HTML listed, one-time migration.

**Not done / open:**
- **Codex is out of credits until Oct 7 1:53 AM** — round 8 (all of the above) was reviewed by the
  `ck:inspector` backup: 1 P1 (a re-drive without a referrer ran as the SYSTEM principal — could
  reach `file:` / top-level `data:`), 1 P2 (static-theme zip sizes trusted), 5 P3; all fixed and
  the P1/P2 verified on the device (round 8 in the findings file). **Owed: Codex on this
  session's commit (`20310b3e..HEAD`) once its credits are back** — the memory note
  `codex-gate-needs-direct-invocation.md` has the invocation.
- Flash's own rate on this CPU (~27 fps for a 30 fps 320x240 SWF, ~22 at 640x480) is the limit
  now; `sensord` takes ~15-20 % CPU on the test device at all times (not ours). Whether evenly
  paced 27 fps reads well in a real game is an eye test for the owner.
- One unexplained miss: a static theme's live notification did not paint Mochi's bar once (two
  later runs did). Tabs: groundwork only, as asked.
- Finger tests remain the owner's: pinch on Mojo, the menus by touch, the in-field reload.

**Device state as left:** all three daemons run the same hand-pushed build (this session's final
bundle; the packages under `build/webos-oe/out-ipk/` are built from the night commit, the same
tree, but NOT installed as packages — the device has the pushed files). Adapter on the rootfs for all three
(`/usr/lib/jihad/<V>/BrowserAdapterImpl.so`, md5 enyo `d9144bc4…`, mochi `831549b7…`, mojo
`8f1a90bd…`; Mojo-only widget pinch), rootfs **ro** (checked). Test hooks **restored** on all three
(shipped manifests, no hook files). No Jihad card open; no plugin-container. Host echo server
stopped.
`/tmp` holds `jihad-anim.*`, `jihad-grid.*`, `jihad-plain.html` (small). Mochi's current theme is
the static Water-Like (`static-theme:0dff6242…`), Enyo and Mojo Water-Like. The Enyo daemon is
on its job again (it was hand-started for the A/B runs).

## 2026-10-06 (evening) delta — personas IN PROGRESS, stopped at the usage limit (superseded by the night delta above)

**Owner's asks this session, in order:** fix the slow add-on manager (DONE, see commit `fb8d2cfc`);
"ensure all options in about:config that are relevant are working" (DONE: effect audit, same
commit); tabs decisions (DONE in the kit/design: `window.open` → new tab, strip invisible with
one tab, no cap — responsive scrolling strip, Atlas `TabLayer.js` as the UI reference, "buttons
and tabs should also be themed" added to R1); **personas** — "support firefox 52 personas, pale
moon personas, and basilisk personas", then "use the persona water-like by stick177 to test all
3 variants. buttons and tabs should also be themed" — IN PROGRESS, uncommitted parts committed
as they stand in the commit after this one.

**What is built and device-verified (Mochi only):**
- Engine/daemon: `GoannaRenderPage` catches the theme sites' `InstallBrowserTheme` /
  `PreviewBrowserTheme` / `ResetBrowserThemePreview` events on the content document; the pump
  hands them to `components/jihadPersonas.js` (new) as page command `theme`; the component asks
  the user through the existing `jihad-xpi-confirm` door (wording "Allow <host> to change the
  browser's look to the theme: <name>"), applies through the engine's `LightweightThemeManager`
  (which persists the header image into the profile), records the theme in pref
  `jihad.theme.current` and pushes category `theme` on the notification channel; Luna
  `getTheme` (new, both buses) answers a starting card. The notification door now lets `theme`
  JSON through whole (it was cut at 200 chars — the card parsed nothing; the first bug found).
- Mochi card: the toolbar is painted (accent + persisted header file, `rgb(92,30,47) url(file:///…/profile/lightweighttheme-header…)`) both at start (getTheme) and live on install. Test site:
  scratch `echo-server.py` `/persona?v=N` (and `?v=water` = Water-Like by stick177, from AMO's
  converted static theme: `theme_frame` jpg, frame `#00ccff`, text `#000000`); hook steps `T`
  (toolbar style), `Z` (clear card-side), `Y`/`a` (confirm answers).
- Settings page: an "Appearance" section (current theme + Remove; used themes with Use/Forget)
  in the Browser pane — CODE ONLY, not yet seen on the device.
- Static-theme XPIs (what AMO serves every former persona as): `jihadInstallPrompt.js` hands a
  -207 corrupt install's file to `jihadPersonas.js`, which reads a theme-only `manifest.json`,
  unpacks the image under `<profile>/jihad-themes/` (served as `resource://jihad-themes/…`),
  and applies it as a lightweight theme with the user's yes — CODE ONLY, untested. Test link on
  the site: "Install the AMO static-theme XPI" (`/water-like.xpi`, sha256 `c6b8b50d…`).
- Button tinting in all three shells (`-webkit-mask-image` of the glyph + textcolor; Enyo
  `.enyo-button-icon`, Mochi `img.jihad-nav-img` with a transparent src, Mojo the command row's
  sprites, re-applied after the row's model changes) — CODE ONLY, untested on the device
  (whether this WebKit honours `-webkit-mask-image` is the first thing to check: hook `T` reports
  the first icon's mask/background).
- Enyo and Mojo `applyTheme` + `getTheme` + `theme` routing — CODE ONLY, untested.

**Next steps, in order:** `card-selftest.sh {enyo,mochi,mojo} push`; per shell run
`page:w6;Z;T;107,146;w3;a;w3;T;w5;T;w1` on `http://192.168.1.245:8091/persona?v=water` (the
echo server must be running on the host: `python3 <scratch>/echo-server.py 8091`; the Water-Like
header jpg and xpi live in the scratch dir — copy them somewhere durable, e.g.
`render/goanna/test/personas/`, before the scratch dir is lost); screenshots of the themed bar
and buttons on each shell; the Mochi click coordinates differ per shell (Enyo/Mojo: read the
page's button position from a screenshot: image (80,168) at 576x768 → view x = 107, y =
168·1.333 − view top); then the AMO XPI link; then `about:preferences` Appearance section;
then `restore` all three hooks, rebuild bundle + packages (`build-variant-ipk.sh`), install
as upgrades, remount ro; kit cavekit-addons-extensions.md gains **R10 (lightweight themes)**
with these criteria (not yet written); PARITY rows; Codex/inspector review of the persona code
(`jihadPersonas.js`, `jihadInstallPrompt.js` hook, `GoannaRenderPage` theme events, the three
`applyTheme`s); PICKUP.

**Device state as left (NOT clean):** all three daemons run the hand-pushed bundle
`9feea335…` (daemon with idle drain, referrer, title, DNT, theme-door; components incl.
`jihadPersonas.js`), NOT the installed packages (those are at `3bafb9bb…`). **The Mochi test
hook is LOADED** (`card-selftest.sh mochi restore` before anything else); Enyo and Mojo shells
on the device are the OLD ones (no `applyTheme`). Mochi's profile holds test personas
`jihad-test-persona-1/2/3` as used themes and persona 3 as current — remove via the Appearance
section or `LightweightThemeManager.currentTheme = null`. Mochi's `prefs.js` is back to its
20 user prefs (the effect-audit batches were restored). The host echo server may still be
running (pid in `pgrep -f echo-server`). Root fs: rw after the hand pushes? — check
(`mount | grep store-root`), remount ro. No card open.

## 2026-10-06 (go-ahead) delta — everything that needed no hands: reinstall audit, reboot, a real sheet, Codex round 6, SDK 0.4, tabs groundwork

**The owner's word:** "You have my go ahead for all remaining tasks that you can achieve without me
physically touching the device. Also lay the groundwork for tabbed browsing but dont implement it
yet." Then: "Update to the latest community sdk" and "Pause completely when you need a sudo
command from me" (saved as a memory rule).

**Done, device-verified:**
- **SDK 0.4** (`~/eclipse-workspace/webos-sdk-redux` at tag `0.4-Release`; the owner ran
  `sudo ./install.sh`): `/opt/PalmSDK/Current → 0.4`, new `novacomd` (built 09:50) running, device
  seen. Among its fixes: novacom no longer drops a packet whose payload trails its header at
  close — the "output after stdin EOF is lost" trap may be softer now; `dev-sh.sh` stays the rule.
- **Enyo and Mojo removed through the installer and installed from nothing** (T-231–T-233, T-273;
  Mochi was 2026-10-05). Each removal left no app dir / job / shim / impl / role / service / state
  dir / socket, the hub forgot the name, the other daemons kept their PIDs, the citizen-audit diff
  shows only the variant's own entries (the empty package-registry dir stays, known). Each fresh
  postinst rescanned the hub, started the daemon from its deviceroot, registered Luna on first
  start, a card rendered Wikipedia, Palefill landed (Mojo's after ~70 s of an open card — the
  add-on manager is slow to come ready; the installer retries each start). Enyo's db8 history
  exported and put back (69 rows); Mojo's localStorage history restored. Backup of both profiles,
  state dirs and localStorage: `~/eclipse-workspace/Jihad/device-backups/jihad-backup-20261006.tgz`
  (+ `db8-enyo-*.json`). Snapshots in `build/webos-oe/out-audit/*-1006.txt` (git-ignored, host only).
- **Reboot (T-234):** `com.palm.power/shutdown/machineReboot`; back on novacom in ~40 s; 2.5 min
  later all three jobs running from the shipped upstart files, each daemon `luna: serving` with
  no `FAILED` and no retry, public-bus answered, sockets up, no D-state threads, `/` read-only, no
  crash reports, a card per variant rendering a live page. T-236's condition did not arise. The
  standalone `alsa_probe`/`plugin_mime_probe` were not re-run (diagnostic programs from closed
  investigations; the daemon itself plays media and scans plugins).
- **One real print** (T-310's last box): the small page printed to the IPP sink first
  (`evidence-2026-10-06/mochi_print-small-page-as-the-sink-received-it.jpg`, 0.7 % ink), then the
  same dialog to the Epson ET-2800 at 192.168.1.253 over IPPS: `sheet 1 accepted after 456 ms,
  job-id 32`. **The owner should find one sheet with three lines of text in the tray.**
- **Codex round 6** on the inspector-only range (its credits came back, then ran out again before
  the second range — retry 2:53 PM): nine findings, four already fixed, one accepted, four fixed
  now and installed as an upgrade: the padlock follows the COMMITTED address (typed https showed
  the old page's padlock, and its certificate, under the new address — Enyo `pending` through
  ActionBar/URLSearch/AddressInput, Mochi `_pendingUrl`; hook steps `U`/`L` drive it); Cut is bound
  to the field the copy came from and the copy must be under 15 s old; copied text is let go after
  15 s; `services_test`/`xpi_mismatch_test` sinks return `int`; `push-engine-update.sh` refuses a
  full push from a stale bundle before touching the device. Record: `impl-review-findings-6.md`.
- **Tabbed browsing — GROUNDWORK ONLY:** `context/kits/cavekit-tabbed-browsing.md` (R1–R6),
  `docs/TABS-GROUNDWORK.md` (plan B: pages in the daemon, one connection per card, `tabs/*` over
  the variant's own Luna service, YAP untouched; why not one adapter per tab: 37.7 MB of shared
  memory per connection), Tier 71 (T-347…T-354) all PENDING, overview row. Three questions for
  the owner at the end of the design note. Nothing built.

**Host test note:** `services_test` and `input_test` (desktop GTK path under Xvfb) read a black
window before any click — the desktop harness, not the device path; not chased.

**Not done, and why:** the two pushes (`git -C third_party/uxp push fork jihad-engine-mods`;
`git push origin jihad/about-preferences-and-settings-merge`) were refused by the session's
permission gate as outward publication — the owner runs them. The OE SRCREV is bumped to
`db070f0003` (the gitlink's commit; the recipe clones the in-repo submodule, so it needs no
push). T-341 / T-354 and T-280–T-289 / T-317 need a finger.

**Device state:** all three variants from the packages at this commit (bundle `3bafb9bb…`, daemon
`59c72b75…`, libxul `ddf8cc70…`; md5 `cf2fe975 / 5887986c / a5a78d55`), installed as upgrades
after the fresh installs, then REBOOTED and checked. Root read-only (the reboot did it). No hooks,
no staged packages, no backup copy on the device, `/tmp` holds nothing of mine. Palefill in all
three profiles. No card open. Enyo history 70 rows (69 restored + Wikipedia).

## 2026-10-06 (later) delta — Mojo's Copy/Cut/padlock never reached the engine; wide zoomed regions seen; three test hooks

**Why this session existed:** "Is there anything else you can do without me using the device
physically?" — the items that did not need a finger: the Mochi padlock after a failed load, a
pinch through the adapter's own gesture entry points, and a test hook for the Mojo shell. Each
turned up something.

**Found and fixed (device-verified, scripted):**
- **Mojo Copy, Cut and the padlock's `pageFocused` had never reached the engine.** The Mojo WebView
  widget (framework, not ours) implements `copy(callback)` as `this.adapter.copy(callback)`, so the
  shell's `callBrowserAdapter("copy")` handed the adapter one `undefined` argument, which
  `js_copy` refuses ("Bad arguments") — silently, because the shell's "method unavailable" line
  was at `Mojo.Log.warn`, which never reaches the device log (a trap already in the Mojo kit).
  And the widget wraps no `cut` and no `pageFocused` at all. Fix (`app-mojo/app/assistants/
  main-assistant.js`): `askCopy()` always passes a function; a method the widget does not wrap is
  called on the plugin element itself (`view.querySelector("object")` — `element.mojo` is the
  widget's exposed face and does not carry its `adapter`); the unavailable line is now `error`.
  The method-name set is unchanged (kit mojo-ui R3). Verified: word Copy → clipboard; field Paste;
  Select All → Copy/Cut appear; Cut → clipboard has the text, field empty, engine says "Cut.";
  padlock tap on Wikipedia → the Certificate alert; `pageFocused` logged by the daemon.
- **Wide zoomed regions were thrown away at the content's right edge.** The pinch hook (`g` step:
  adapter `gestureStart/Change/End`) does zoom (`setZoomAndScroll zoom=1.2434 scroll=658,686`),
  and the first wide frame came out one viewport wide: `mLastContentW * Zeff` gave 1387 while the
  band (clamped in CSS px, scaled the other way) ended at 1388, and the band-inside-region test
  dropped the wide region. `BrowserPageGoanna.cpp`: the content is at least as wide as the band.
  Verified: `region columns: content 1388 px wide at zoom 1.2434 ... -> [0, +1388), rows up to
  2266`, the composite on screen is right, pinch back to 0.8447 → 943x3335 with sweeps. The
  `region columns:` line logs only when the decision changes (kept; it is what made this visible).
- **Mochi padlock after a failed load** (`page:w6;l;a;w3;l;w3` over `https://no-such-host.invalid/`):
  showing before the engine's alert is answered, gone after, the tap then says "No certificate
  information is available for this page." Mochi hook gained `a` (answer the engine's alert).

**New: a Mojo test hook** — `render/goanna/test/mojo-selftest.js`, `card-selftest.sh mojo push|run|restore`
(the script now knows each shell's sources, manifest and hook path). Steps: `hX,Y` hold, `m`/`mN`
read/pick the open `popupSubmenu` (its model's items; a pick calls the model's `onChoose` with the
item's command, the popup's element removed first — this framework build's submenu has no
`mojo.close`), `l` padlock (`handleAddressTap` at x=12), `d`/`a` read and answer the open
`showAlertDialog`, `k`/`c=`/`p` clipboard, `W`/`P` list the widget's and the plugin's methods,
`?` state. The scenario comes from `PalmSystem.launchParams.jihadTest` (the stage hands the scene
only the url). Every `callBrowserAdapter` call is logged with the road it took (widget / plugin /
nowhere) — that is how the Copy defect showed.

**Review:** Codex still out of credits (retry 6:46 AM) → `ck:inspector` on the diff; findings and
what was done with them are in `context/impl/impl-review-findings-6.md` (round 5).

**Still open (unchanged in kind):** owner-only finger tests (T-341: pan/pinch feel, a hold on a link
per shell, the glyph itself), removal/reinstall (T-231…), hands-on checklist (T-280…), one real
print, pushing the engine branch. The sideways pan still has no script (it is the adapter's drag).
Agent-doable when Codex's credits return (6:46 AM local): a Codex pass over the inspector-only
rounds (3 and 5 in `impl-review-findings-6.md`; the scratch wrapper is described under "Codex
gate" in the delta below). Nothing else of the owner's list is reachable without a finger.

**Device state:** all three variants installed through the installer from the packages built at
this commit (bundle `622ea210…`, daemon `59c72b75…`, libxul unchanged `ddf8cc70…`, adapters
unchanged; packages `build/webos-oe/out-ipk/*.ipk`, md5 `c0273e34 / 3956cd95 / d7b7c1cd`) — the device is
the repository head. Each variant smoke-tested on Wikipedia from the package (title and frames
in its daemon log). Palefill XPI in each profile. Root filesystem read-only. Test hooks: none
loaded, manifests shipped. Staged packages and my `/tmp` files removed (`/tmp` 688 KB used). No
crash reports of mine (`/var/log/reports` holds only `librdx`). No card open.

**Review fixes applied after the inspector (same commit):** see `impl-review-findings-6.md`
round 5. The things to know: the Mojo hook answers an alert through the dialog's own
`close(value)` (a page `confirm()` → "answered yes" on the device) and taps a submenu node with
`Mojo.Event.tap`; Find on Page in Mojo had also never reached the engine (no `findInPage`
wrapper) and now does (`find 'charlie' -> 1`); a copy the engine answers `false` to gets its own
"could not be copied" toast after 1.5 s unless the engine's word came first.

## 2026-10-06 delta — the 2026-10-05 work finished, reviewed, packaged, device cleaned

**Commits (local, NOT pushed):** `657282c4` long-press menu · `debd1934` composed frames + padlock +
review fixes · `afb0db6e` Palefill · `707795de` / `bb16ea4a` docs · `31c5f445` review round 4
(Codex on Palefill, inspector on the last fixes). Engine branch `jihad-engine-mods` (local; owner pushes, then bump the OE SRCREV in
`build/webos-oe/recipes-jihad/goanna/goanna_1.0.bb`): `0048d50fa9` (windowless plugin),
`628ee50c99` (plugin child env), `db070f0003` (AppCompat vs toolkit target).

**What the owner asked for and what it is now** (kits: ui-shell R7/R8, offscreen R9, addons R9,
device-build R4; `context/impl/impl-review-findings-6.md` has every review finding):
- Long-press menu on any content, all three shells (Copy/Cut/Paste/Select All, page actions).
  Device, Enyo + Mochi, scripted holds. Mojo needs a finger.
- "Pages only load when observed": frames are composed (copy then render ≤ 768 new rows), region
  is the whole segment, sweeps, wide when zoomed. Row-exact after jumps. A real pan needs a finger.
- DDG "links don't work" = Flash pages killing the daemon in a loop → engine fix + crash guard.
- Padlock → certificate (+ connection, trust state). Enyo + Mochi device-verified; Mojo drawn.
- Palefill 1.30 bundled unmodified (MPL-2.0, hash-pinned, NOTICE); installed enabled in all three.
- Codex gate: Cavekit 3.1's script is silently incompatible with Codex CLI 0.160 — run
  `codex exec` directly with its prompt; `ck:inspector` when Codex's credits run out (they did,
  twice). Memory: `codex-gate-needs-direct-invocation.md`, `device-test-limits.md`.

**Device state:** all three variants installed through the installer from the packages built at
`31c5f445` (bundle `4887551a…`, daemon `b7be3e3c…`, libxul `ddf8cc70…`, adapters `88303cfc /
51d10886 / 1b0ac09d`, Palefill in each) — the device IS the repository head; nothing hand-pushed
on top. Packages: `build/webos-oe/out-ipk/net.riverstonerelay.jihad-browser{,-mochi,-mojo}_*.ipk`
(Enyo 1.0.4, the others 1.0.0). Root filesystem read-only. Test hooks restored (none loaded). `/tmp` holds nothing of
mine (39 MB free). `/var/palm/jihad/adapter-backup-20261005` removed; the 2026-08-10 backup is
still there. Crash reports: all of 2026-10-05's were mine (Flash loop, busybox, one
`media-pipeline.real` SIGSEGV at 15:18 during the first package install) and were deleted;
my attempt to copy them to the host first produced empty files, so they are gone unread.
Palefill in every profile. No card open.

**Still open:**
- Owner only: finger tests (pan/pinch smoothness; Mojo menu + padlock tap; Mochi padlock after a
  failed load; a hold on a link per shell) — T-341; Enyo/Mojo removal + install from nothing
  (T-231…); hands-on checklist (T-280…); one real print.
- Known gaps left on purpose: fields inside iframes get the page menu; Palefill buffers whole
  responses of the sites it rewrites (its own design); one `window.jihadClip` per card.
- Trap: after an upgrade that replaces the adapter, `/` will not remount read-only until
  LunaSysMgr restarts.

## 2026-10-05 delta — printing built; a stale-script defect found and fixed; Mochi installed from nothing (read first)

Per-task record: `context/impl/impl-ce310-loop.md` (rows from "2026-10-05"); evidence:
`context/impl/evidence-2026-10-05/`; plan: tiers 62–65 of `context/plans/build-site.md`.
**Nothing was pushed.** 10 local commits since `c726e8a7`.

**What the owner asked, and the answers:**
- *"I have a network printer, so why can't the TouchPad see it?"* It can. webOS's print daemon
  resolved the Epson over mDNS and logged `not HP printer and doing printer filtering` — the stock
  print stack discards every printer that is not HP. So the browser now prints by itself:
  `render/goanna/components/jihadPrint.js` finds IPP printers (mDNS, or a typed address), draws the
  page as JPEG sheets and sends them over IPP/IPPS; both the Enyo and the Mochi shell have a Print
  dialog (`JihadPrintDialog.js`) in place of the system one. On the device both dialogs list the
  Epson ET-2800 (192.168.1.253) and a Mac-shared ET-2720. Mochi's Print box is `[x]`; the parity
  list has no open row.
- *Share Image / Set as Wallpaper:* both run on the Mochi card through the long-press sheet's own
  handlers. The wallpaper changed and was put back (`19.jpg`); Share Image opened Email.
- *Why was `/` read-write?* The installer service (`org.webosinternals.ipkgservice`, Preware's
  backend) remounts `/` read-write before running a package's script and never restores it. Measured
  from a read-only start. Our scripts leave the state as found. A reboot, or
  `mount -o remount,ro /`, puts it back.
- *The Atlas fault report:* Atlas's own — a SIGBUS in its allocator thread at the 2026-10-02
  shutdown, nothing of ours in it. Atlas and a Jihad card rendered side by side again today.

**An accident, and the defect behind it.** A print test meant as a dry run put one full dark sheet
through the owner's Epson. The new print component was on the device and the daemon had been
restarted, but the engine ran the PREVIOUS version from its compiled-script cache
(`$APP/cache/startupCache`, keyed by path, never invalidated in an embedding — and untouched by an
`.ipk` upgrade, so every upgraded script was exposed to the same thing). Fixed in the daemon
(commit `c4e9bb9e`): it fingerprints the bundle at start and drops that cache when the bundle
changed. Look for `startup cache: bundle …` in the daemon log.

**Traps found this session:**
- **Test printing against `render/goanna/test/ipp-sink.py` first**, never the real printer:
  `python3 render/goanna/test/ipp-sink.py --port 8090 --out DIR`, then
  `card-selftest.sh <shell> run print:192.168.1.245:8090 <url> 66`. It keeps each sheet as a JPEG.
- After pushing ANY engine script by hand, confirm `startup cache: bundle CHANGED … dropped` before
  believing the new code is what runs.
- mDNS from an ephemeral port gets no answers on this device: the firewall admits only
  `udp dpt:5353` and established flows. Ask from port 5353.
- The Luna hub forwards a bus SIGNAL only from a sender named `com.palm.*`, and the device's
  `luna-send -s` needs `-m com.palm.lunasend` for that. Without it the signal shows in `ls-monitor`
  and reaches nobody.
- `ipkgservice/install` wants BOTH `filename` (the bare name) and
  `url` (`file:///media/internal/.developer/<name>`), and after a FIRST install the launcher does not
  know the app until `ipkgservice/rescan` (Preware calls it; a script must too).
- A card's `luna-send` on the device quits before its answer if its stdin is at end-of-file — which
  is what a carelessly held novacom session gives it. Use `build/webos-oe/dev-sh.sh`.
- `com.palm.display/control/setState off` is refused while on USB (`blockDisplay`), and there is no
  touch input device to inject a tap into.

**Tools added:** `build/webos-oe/luna-restart.sh` (restart LunaSysMgr and close the USB alert it
leaves over the top right of the screen; `--close` for the alert alone), `build/webos-oe/dev-sh.sh`
(run a local script on the device, all output, returns when it ends),
`render/goanna/test/ipp-sink.py`, `packaging/branding/make-enyo-toolbar-icons.py` (the Home and
Bookmarks/History toolbar icons, redrawn at the owner's request), print scenarios in both test
hooks, `holdpick` in the Mochi hook. `device-citizen-audit.sh` now lists the installer's package
directories.

**Still open, and why:**
- Enyo and Mojo removal and install from nothing (T-231…T-234, T-236, T-273): only the owner can run
  a removal on this device. Mochi's half is done. When that happens, look at
  `/media/cryptofs/apps/usr/palm/packages/<id>` afterwards: Mochi's removal left it as an empty
  directory (recorded in cavekit-device-build.md R8, not fixed — `prerm` cannot `rmdir` a directory
  ipkg still has files in). The test extension is no longer installed in any profile; T-273 needs it
  put back in two variants first.
- A sheet of paper printed from the dialog, seen by a person; a real long-press; and the rest of
  the hands-on checklist (T-280…T-289, T-317) — deferred by the owner.
- Not built yet: the daemon's "luna: serving" log line now names `findPrinters` / `printPage` in
  source, but the installed daemon predates that one-string change.

**Device state as left (2026-10-05):**
- All three variants installed from the same build through the installer: bundle id
  `a4a2f8e2b8bf…`, libxul `65e3cc84891c…`, daemon `889263013e40…`. Mochi was a fresh install, Enyo
  (1.0.4) and Mojo upgrades. Each daemon runs from its own deviceroot and answers on the public bus.
- Jobs: Enyo and Mojo as shipped; **Mochi's job carries `JIHAD_INJECT=1`** (added by the install
  helper). No test hook in any app directory. No extension in any profile.
- `/` is read-only (put back by hand after the installs). Wallpaper `19.jpg`, as found. No Email or
  Atlas card left open; one Enyo browser card is open.
- `/tmp` keeps the helpers (`fresh-card.sh`, `inst3.sh`, `rm1.sh`) and the test pages, including
  `jihad-print.html` and `jihad-print-tall.html`. `/tmp` is a tmpfs: all of it goes at a reboot.
- Old staged packages are still in `/media/internal/.developer/` from earlier sessions
  (`jihad-browser.ipk`, `jihad-mochi.ipk`, `jihad-mojo.ipk`, the 1.0.2 and 1.0.3 Enyo `.ipk`s) —
  about 230 MB of the user's volume that nothing uses. Left for the owner to delete.

---

## 2026-10-02 delta — owner decisions carried out; the Mochi parity omissions built (read first)

The owner answered the open decisions and gave a go-ahead for the removal audit, the engine push
and a reboot. Per-task record: `context/impl/impl-ce310-loop.md` (rows from "2026-10-02" on);
evidence: `context/impl/evidence-2026-10-02/`; plan: tiers 54–61 of `context/plans/build-site.md`.
**Nothing was pushed** (see "Waiting on the owner").

**Decisions taken, and done:**
- The card panels lose Block Popups / Accept Cookies / Enable JavaScript (Enyo, Mochi); the settings
  page owns them; "More Settings" opens it. The Enyo card no longer replays its stored copies.
- Flash comes from the device's own plugin folder (`/media/cryptofs/apps/usr/lib/BrowserServerPlugins`,
  the HP Flash plugin app) — scanned after the profile folder, skipped when the profile has its own
  copy. One stored on/off value (`plugin.state.flash`): a row on the settings page, a real switch in
  the Enyo and Mochi panels through the variant's Luna service (`getFlash` / `enableFlash` /
  `disableFlash`; the setters accept only the variant's own card).
- No Mochi parity omission was accepted. Built and shown on the device: bookmark editing, bookmark
  thumbnails, downloads a page starts (one fetch, with retry), long-press menu, share sheet and Add
  to Launcher, certificate details, search-engine choice, Flash switch, Clear History that clears the
  engine too, a confirmation before each clear, grouped history dates. `app-mochi/PARITY.md` is
  refreshed. **Print is not built**: no printer the device can reach, so nothing could be verified.

**Defects found on the way and fixed:**
- Closing ANY card unregistered the add-on install prompt and the JS notification door for the rest
  of the daemon's life (the page destructor cleared a process-wide sink, and clearing it dropped
  once-per-process observers). Installs were downloaded and then cancelled with no prompt.
- The Mochi bookmarks list could never show a row (ordered by an un-indexed db8 property), and a
  Mochi panel hidden at layout time opened blank (never measured).
- With a profile copy and the system folder both scanned, Flash was registered twice.
- The Enyo certificate view could never open on webOS CE (see traps) and threw on a certificate with
  no common name.

**Waiting on the owner (approved, but the session's permission gate refused both again):**
- Engine push: `git -C third_party/uxp push fork jihad-engine-mods`, then bump the gitlink and the OE
  SRCREV (T-279). Two local engine commits: `1fff802335`, `9dedcd5fe5`.
- Variant removal (T-231…T-234, T-273): the audit script is on the device —
  `/tmp/rm1.sh <variant> <appid> <job> <luna name> <shim>` (volatile: re-push it from the scratch
  copy or rewrite it if `/tmp` was cleared). The test extension is installed in the Enyo and Mochi
  profiles for the "its extensions go, the other's stay" check. Profile backups of all three variants
  were taken to the host before anything else.
- A printer the device can reach, for Print (T-310).
- The human checklist (T-317): hands-on steps, listed at the end of the session's report.

**Traps found this session:**
- `/tmp` on the device is a 40 MB tmpfs. Three profile tarballs filled it; `novacom put` then fails
  with "No space left on device". Stream large things straight to the host instead.
- A card may load a local file ONLY from its own application directory or `/media/internal`
  (LunaSysMgr `isAllowedFileAccess`), and its WebKit refuses a local file over XMLHttpRequest. A
  script or image tag pointing into the app directory works. `/var/…` and `/tmp/…` do not.
- `com.palm.certificatemanager` does not exist for a card on webOS CE ("Service does not exist").
- While an engine dialog is up the daemon answers nothing over Luna either.
- This Enyo 2 has `resize()`, not `resized()`; a panel shown after layout needs it.
- db8 refuses an ordered query on a property the kind has no index for (`-3965`), and an Enyo 2 card
  shows that as an empty list, not an error.
- `pkill -f <pattern>` kills the shell that runs it if the pattern is in its own command line.
- An upgrade `.ipk` install overwrites the package's files but does NOT remove an extra file: a
  pushed test hook's manifest edit is undone by an install, its `JihadSelfTest.js` is left lying in
  the app directory. Always finish a hook session with `card-selftest.sh <shell> restore`.

**Tools added:** `build/webos-oe/card-selftest.sh` (+ `render/goanna/test/{mochi,enyo}-selftest.js`)
drives a shell's own chrome on the device through its handlers; `build/webos-oe/prefs-row-walk.sh`
walks every settings-page row.

**Device state as left:**
- All three variants installed through `org.webosinternals.ipkgservice`: libxul
  `65e3cc84891c59e9ede40cc7b3de0802`, daemon `1a70344214e9e266c894782ed10286b9`, goanna.js `97f981b3bc03…`,
  PDK adapter impls unchanged (enyo `6a35d778…`, mochi `44ab2e23…`, mojo `25aa85a4…`). Shell sources
  on the device compare equal (md5) to the repo at the last commit. The Enyo package installed is
  `690d8c3c…`; Mochi and Mojo were installed from an earlier build of the same payload
  (`a9ca38d8…`, `59d727a0…`) — the copies now in `build/webos-oe/out-ipk/` are a later rebuild whose
  payload is identical but whose package bytes are not.
- **The device was rebooted once** (owner-approved) with the shipped, unmodified jobs: all three
  daemons came up, Luna registered at first start, extensions survived. Afterwards `JIHAD_INJECT=1`
  was re-added to the three jobs (remount rw → edit → remount ro) and the daemons restarted. The
  root filesystem is READ-ONLY at rest now (it was read-write before the reboot, found that way).
- Flash: no variant has a copy in its profile any more (the hand-copied one was removed from Enyo);
  all three get it from the system folder; the value is ON in all three.
- **The test extension `jihad-effect@riverstonerelay.net` is installed in the Enyo and Mochi profiles
  on purpose** (for the removal audit) — it turns every page's background magenta in those two
  variants. Remove it from `about:addons` once the audit is done.
- `/tmp` was wiped by the reboot. Restaged: the test pages (`stage-test-pages.sh`), `fresh-card.sh`,
  `inst3.sh`, `fin1.sh`, `ps.sh` and the removal audit script `rm1.sh`. NOT restaged: the media clips
  (`stage-media-pages.sh`), the Flash test movies except `jihad-anim`, `uinput_kbd`.
- Cards open: Enyo and Mochi on `file:///tmp/jihad-plain.html`. No test hook on the device: both
  manifests are the shipped ones and no `JihadSelfTest.js` is left (an upgrade install leaves an
  extra file behind — it was removed by hand).
- Mochi card data left from the tests: a few history rows and downloads-view rows (their files are
  deleted); the search engine is back on DuckDuckGo; no bookmarks.
- Four fault reports in `/var/log/reports/librdx/`; the new one is `BrowserServer-atlas` SIGBUS at
  the moment of the reboot — Atlas's daemon, not ours. LunaSysMgr was restarted about a dozen times
  by `card-selftest.sh push` / `restore` (by design: it caches card JS).
- `/var` 23%. Residue still to delete by hand: `/media/internal/downloads/jihad-testfile{,-1,-2}.bin`.
- Cavekit: registry 96 complete, 18 blocked (owner / person / printer), 1 pending behind them. The
  loop was torn down at the stop. `.cavekit/config` carries an uncommitted edit of the owner's.

---

## 2026-10-01 (third session) delta — every agent-doable sweep run on CE 3.1.0 (SUPERSEDED where the delta above disagrees)

`/ck:make` ran the rest of the build site inline. Registry: 72 complete, 18 blocked (owner / human),
1 pending behind a blocked task (T-236). Per-task record: `context/impl/impl-ce310-loop.md`; the kits
carry the evidence on each criterion; screenshots in `context/impl/evidence-2026-10-01/`.
**Nothing was pushed.** Engine commits `1fff802335` and `9dedcd5fe5` on `third_party/uxp`
`jihad-engine-mods` are LOCAL; the parent gitlink and the OE SRCREV still say `bd49435b` (T-279).

**Verified on the device (webOS CE 3.1.0), all three variants unless said:**
- Preferences: the page renders and writes only its own variant's `prefs.js`; typed values save on
  blur and on Enter; JavaScript row is back and survives a card relaunch and a daemon restart; the four
  frozen settings work from the page and from the shipping Enyo card; Mojo/Enyo/Mochi adopt page edits.
  Every one of the 56 table rows was then walked on the device through its own control
  (`build/webos-oe/prefs-row-walk.sh`): all 56 change the engine pref they name. A real key path
  (the kernel keyboard) types into a settings field.
- `window.open` from a tap opens a new card (new, `nsIWindowProvider` → `msgCreatePage`).
- Input sweep (taps, click count, toggles, key events, non-ASCII, selection, Enter/Tab, zoomed link),
  `about:config` end to end, the `about:addons` tools menu, touch events with one and two fingers.
- Offscreen sweep (size tracking, exact colours, buffer rotation, messages), `<select>` → card list.
- Shell sweeps: launch target on the variant's own socket, find, alert, failed load, 302, HTTP auth.
- Daemon sweeps: cookie + disk-cache persistence, low-memory guardrail, public `clearCookies` /
  `clearCache` on install-started daemons, dialogs hold the page.
- Device-build sweep: C++14 program, ELF attributes = CPU, memory (126 MB after eight sites, cap 150),
  the seven adapter-deps libraries match the manifest on CE.
- Key codes MEASURED from a real key path. `render/goanna/test/uinput_kbd.c` makes a kernel keyboard
  that LunaSysMgr on CE takes for a Bluetooth one; its keys reach the card and the daemon.
- Volume HUD: a volume KEY press during page audio changes the level and raises the HUD.
- Flash 33 fps; the in-tree control plugin loads, draws at the right place, takes a tap and a key.

**Defects found by the sweeps and fixed (all device-verified, all committed):**
no `change` event after typing; number fields edited the wrong element; chrome pages never scrolled
natively; the card re-enabled JavaScript on every connect; settings edits reverted on Back;
pages without a viewport tag drew as a thumbnail in Mojo; page key events carried raw webOS codes;
Up/Down were swapped, no Home/End; a field low on a tall page stayed under the keyboard; popup taps
were divided by the zoom twice and an opened menu was not painted; Mojo ignored a launch `target`;
find/selection was white on white; HTTP auth raised no dialog at all; damage rectangles were not
scaled at a fit zoom (clipped plugin, inexact Flash box).

**Still open, and why:**
- **Needs the owner's word:** removing a variant (T-231–T-234, T-273 — the permission gate refused
  it as irreversible deletion, so fresh-from-nothing install, the removal audit and "extensions of a
  removed variant" are untested); pushing the engine branch and bumping gitlink + SRCREV (T-279);
  how a user gets Flash; which rows the card's own settings panel should keep (T-290); Flash pacing
  B-vs-C (T-287); anything that needs a reboot (no reboot was made).
- **Needs a person at the device (T-280–T-291):** finger on card chrome, media controls, VKB typing
  and held Backspace, pinch, a real double tap on a plugin (T-212 is built and on the device), the
  physical volume rocker, the Mochi parity sign-off.
- **Known and recorded, not fixed:** arrow keys do not scroll a fit-zoomed page; the keyboard-cover
  check runs only at zoom 1; LunaSysMgr sometimes keeps Up/Left/Right from the kernel keyboard for
  itself; a failed load shows the engine's alert AND the shell's error panel; `window.open` gives
  the opener no window handle.
- **GRE key handover (gre-widgets R3):** `JIHAD_GRE_KEYS=1` in a job file turns it on for a session.
  It works for typing/arrows/Home/End/Backspace/Delete and does not crash; Enter/Tab stay on the
  daemon path. A clean handover needs two engine changes — see the kit.

**Device state as left:**
- All three variants installed through `org.webosinternals.ipkgservice` from the packages now in
  `build/webos-oe/out-ipk/` (enyo 1.0.4 `a0df34da…`, mochi 1.0.0 `dbe4f747…`, mojo 1.0.0 `074be9d3…`):
  libxul `65e3cc84891c59e9ede40cc7b3de0802`, daemon `50615b82ba73f06bb72cdbfc30a663fd`, goanna.js
  `82f9bbb3150b…`, PDK adapter impls enyo `6a35d778…` mochi `44ab2e23…` mojo `25aa85a4…`. Daemons were
  started by the installs; LunaSysMgr was not restarted by them (same pids before and after).
- Jobs are the shipped ones plus `JIHAD_INJECT=1` (re-added by hand after each install — an install
  writes the shipped job). No `JIHAD_GRE_KEYS`, `PULSE_SINK`, pacing or log env left behind.
- Enyo profile has `plugins/libflashplayer.so` (hand-copied, stock md5); the test plugin was removed.
  Mochi/Mojo have none. No test pref is left in any `prefs.js` (Enyo's was compared against a copy
  after the row walk).
- Cards left open: Enyo on `about:preferences`, Mochi and Mojo on `file:///tmp/jihad-shell.html` (none on `about:blank`).
- The uinput test keyboard is not running. `/tmp` holds test pages, scripts, `uinput_kbd` and
  screenshots (volatile). Residue to delete by hand: `/media/internal/downloads/jihad-testfile{,-1,-2}.bin`.
  `/media/internal/.developer/` holds our three `.ipk`s next to other people's.
- No reboot was made all session. `/` is read-write at rest (as found). `/var` at 21%.
- Three fault reports in `/var/log/reports/librdx/`, all from before this session's sweeps (the
  upgrade crash of the second session and two deliberate kills); none new.
- Cavekit: the loop is torn down (`teardown-loop`). The default `session_budget` of 500,000 stopped
  it at 1.1M tokens; the owner raised the live number in `.cavekit/token-ledger.json` to 10,000,000.
  `.cavekit/config.json` still says 500,000 and is what a NEW ledger starts from. `.cavekit/config` is
  a `key=value` file (it holds `codex_model`) and has an uncommitted edit of the owner's.

**Traps found this session:**
- Inject `click X Y` takes DOCUMENT px × zoom; `rect` answers in viewport CSS px. The card zooms to
  1.0 when a field takes focus, so coordinates taken before the first focus are wrong after it.
- `57346` is `0xE002`, not `0xE0A2` (57506). Arrow codes: Up 57504, Down 57505, Left 57506, Right 57507.
- Card JS is cached in LunaSysMgr until it restarts; `push-card-js.sh` handles it.
- `takeScreenShot` writes asynchronously: wait two seconds before reading the file.
- The `partial paint` log line is rate-limited to one per 2 s — it is not a frame counter.
- A banned port (e.g. 9) raises an engine alert that holds the daemon until the pipe is answered.
- An engine dialog holds the daemon: answer the FIFO named in the `dialog … -> card (pipe …)` line.
- `goanna.js` on the device comes from `make-device-bundle.sh`; a pref edit needs it re-run.
- A pref SET through the page while its value equals the default leaves no line in `prefs.js`.

---

## 2026-10-01 (second session) delta — kits updated, six defects fixed and device-verified; stopped early on the usage limit (SUPERSEDED where the delta above disagrees)

Kits were brought in line with CE 3.1.0 first (commit `f12c1111`), the build site regenerated
(`context/plans/build-site.md`, T-201…T-293), then `/ck:make` ran inline. Per-task record with every
number: `context/impl/impl-ce310-loop.md`. Screenshots: `context/impl/evidence-2026-10-01/`.
Nothing was pushed. The Cavekit loop was torn down at the stop (usage limit); the task registry (`.cavekit/task-status.json`, 37/91 complete) is intact and `/ck:resume` picks it up.

**Verified on the device (webOS CE 3.1.0), kit boxes closed:**
- `<video>` plays to the end — engine fix `1fff8023` on `third_party/uxp` `jihad-engine-mods`
  (LOCAL, unpushed; gitlink and OE SRCREV still `bd49435b`). 12/12 runs, VP8 + Theora, file + http.
- Codec table measured (VP8/VP9/Theora/Vorbis/Opus/WAV/FLAC play; H.264/MP3/AAC refused). FLAC needed
  the ffvpx libs, which were built but never bundled — now in `make-device-bundle.sh` and the push script.
- Idle daemon: 0.05% CPU, ~1 wakeup/s, 0 log bytes (was 2.4%, 117/s, 48 B/s). Log capped while running
  (copy-then-truncate, two in-lifetime rotations shown). `about:blank` settles (0 paints / 30 s).
- Card open on a static page: 2.2% CPU (was 12.3%) — quiet tick.
- Luna -1027 on fresh install: CONFIRMED, fixed in postinst (hub rescan) and daemon (retry), verified
  through `org.webosinternals.ipkgservice` on all three variants.
- Upgrade crash found and fixed: postinst `cp` over the mapped adapter shim killed LunaSysMgr at the
  next card launch; it now replaces by rename (three upgrades since, LunaSysMgr survived).
- TLS: `TLS 1.3` captured from howsmyssl and Cloudflare trace.
- Media controls: the stuck `0:00` scrubber bubble reproduced and fixed (daemon grows the damage box);
  click-seek works; mute/slider reach the pulse stream. Play/pause both directions.
- Cert store on CE works; two daemon fixes (retry the FAILED url after trust; drop the redundant
  engine error alert before the SSL confirm). Toast seen in all three shells.
- R9 probes: glibc 2.8 writes the same pshared-mutex `__kind` as 2.23 (the "second trap" was a
  2.5-proxy artefact); Flash's 67-lib closure resolves on CE.
- Flash on CE: works once `libflashplayer.so` is in the profile. The shipped build was pacing it on
  the 8 ms tick (worst histogram); the daemon now uses 16 ms while a plugin is on the page.

**Still open, and why:**
- **Fresh install from NOTHING + the postinst reboot (T-231…T-234, T-273):** removing a variant was
  refused by the session's permission gate (irreversible deletion). Needs the owner's go-ahead. The
  installs done today were install-OVER through ipkgservice.
- **Human-only (not attempted, not counted):** finger on the media controls / volume slider, volume
  HUD (needs a hardware key press), Flash B-vs-C eyeball, Downloads-app look, card-chrome taps, VKB.
- **Not reached (usage limit):** Atlas coexistence run (identity columns ARE checked: all distinct),
  volume-HUD write-up (T-240), add-ons/XPI sweep, preferences sweep, input sweep, offscreen sweep,
  shell sweeps, daemon sweeps, memory sweep, T-212 (plugin VKB), T-214 (arrow-key inject finding),
  rebuild of the three `.ipk`s.
- **Decisions for the owner:** how a user is meant to get Flash (nothing ships or installs
  `libflashplayer.so`; the CE profile had none); push `1fff8023` to the fork and bump gitlink + SRCREV.
- v3.1.0 `scripts/codex-review.sh` still needs `a833abf` cherry-picked; no Codex tier gate ran.

**Device state as left:**
- All three variants ipkg-registered, libxul `65e3cc84891c59e9ede40cc7b3de0802`, daemon
  `46d3714d430162502b3630cf63d4854b`, goanna.js `01154654…`, ffvpx libs present. **The `.ipk`s in
  `build/webos-oe/out-ipk/` are STALE** (daemon `3e29f045`, old Mojo toast): run `build-variant-ipk.sh`.
- Jobs are the shipped ones plus `JIHAD_INJECT=1` (all three). No pacing or log-cap env left behind.
  Enyo's `MOZ_LOG` is gone.
- Enyo profile has `plugins/libflashplayer.so` (hand-copied, stock md5). Mochi/Mojo have none.
- Mojo toast fix (`bottom: 84px`) is on the device but the card runs cached JS until LunaSysMgr restarts.
- LunaSysMgr crashed and restarted once today (the upgrade defect, 14:49 UTC; report in
  `/var/log/reports/librdx/`). The USB dialog it brought up was dismissed. Display set
  `onWhenConnected:true`.
- Residue to delete: `/media/internal/downloads/jihad-testfile{,-1,-2}.bin`; `/tmp/jm/*` and
  `/tmp/*.sh`/`*.png` are volatile. Other people's ipks in `/media/internal/.developer/` were left alone.
- `/` is mounted read-write at rest on this CE image (found that way). `/var` at 13%.
- Platform cert store holds session serials 1–3 from the tests (swept at daemon teardown/startup).

**Traps found today (all in `impl-ce310-loop.md`):** display off/locked or a system dialog makes
`takeScreenShot` return a stale frame; a second disconnected card is what gets photographed; inject
`click` takes CARD px, `clickoff`/`clickid` bypass the input queue; a `launch` in the first minute
after an ipkgservice install is dropped; `ipkgservice/install` returns before it is finished.

---

## 2026-10-01 session delta — device is now webOS CE 3.1.0, Cavekit is now v3.1.0 (SUPERSEDED by the delta above; kept as the stock-take)

This was a stock-take plus one finished diagnosis. **No kit was edited and nothing was fixed** — the
user's standing instruction is "update the kit criteria to reflect the current state BEFORE completing
tasks", and the session ended by moving the tooling to Cavekit 3, which needs a restart. Everything
below is input for that kit update. Nothing here is committed except what `git log` shows.

### Process: Cavekit v3.1.0 (was v2.0.0)

- `~/.cavekit` is on branch `cavekit-3.1.0` (tag `v3.1.0`, `9aa1905`). The two local v2 edits are saved
  as commits on `local/v2.0.0-edits-2026-10-01` (`a833abf` codex-review invocation, `b062631` findings).
  **v3.1.0's `scripts/codex-review.sh` still carries the stale `codex --approval-mode … -p` call and
  `o4-mini`**, so Codex review degrades until `a833abf` is cherry-picked onto it.
- **Start Claude Code from `Jihad-Browser/`, not from the parent `Jihad/`.** v3 resolves `context/` and
  `.cavekit/` against the CWD (`cavekit-tools.cjs`: `CAVEKIT_PROJECT_ROOT || process.cwd()`), and both
  live here. The parent's `CLAUDE.md` still loads from a subdirectory.
- First commands: `/ck:init` (idempotent — creates the v3 runtime state next to the existing v2
  `.cavekit/config`), then the kit update below, then `/ck:map`, then `/ck:make`. `/ck:status` and
  `/ck:resume` are new. The `bp:` alias is gone.
- **TRAP — v3's tool cache will serve stale device readings.** While a loop is active
  (`.cavekit/.loop.json` exists) a PreToolUse hook answers `Read`/`Grep`/`Glob`, and any Bash command
  that BEGINS with `ls|find|cat|head|tail|wc|which|file|git status|log|diff|show…`, from a 120 s cache.
  A device probe or a re-read of a growing log that starts with one of those words returns the previous
  output. Commands that begin with `novacom`, `cd`, or a variable assignment are never cached — keep
  device commands in that shape.

### The device: webOS CE 3.1.0 (build 86, BUILDTIME 20260830), freshly installed

Measured on the device, not assumed:

- Kernel `2.6.35-palm-tenderloin` and **glibc 2.8 are unchanged**, so every cross-ABI statement in
  cavekit-device-build.md R9 still describes this device.
- Stock OpenSSL 0.9.8 is still `/usr/lib/libssl.so.0.9.8`; the new stack is side-by-side OpenSSL 1.1.1w
  (`ssl11`, `ssl11dl`, `ssl11mail`; `curl 7.88.1 … OpenSSL/1.1.1w`). **Jihad uses neither** — page TLS is
  the engine's own NSS 3.90 (`security.tls.version.max=4`). HTTPS loads work (`https://example.com`,
  `https://www.howsmyssl.com/a/check`), but **the negotiated `tls_version` string was never captured**
  (the grep for it came back empty on 2026-09-27) — that one read is still owed before any kit says
  "TLS 1.3 verified".
- CE preinstalls the **Atlas** browser (`org.webosports.app.atlas`, `BrowserAdapterAtlas.so`,
  `/tmp/yapserver.atlas`, its own daemon). A fourth browser now coexists; R7 independence has not been
  re-checked against it.
- Flash is still present (`com.palm.app.flashplugin-001`, same `libflashplayer.so`), the
  `com.palm.flashgraphics` role name is in the stock browserServer role files, and there is still no
  keyboard (`gpio-keys`, `pmic8058_pwrkey`, `headset` only).
- **Every `[x]` that says "device-verified" was verified on webOS 3.0.5.** None has been re-run on CE
  3.1.0 except what is listed in this delta.

### What the interrupted 2026-09-27 session did (recovered from its transcript)

Rebuilt the daemon (YAP contract OK, 73 commands), built all three `.ipk`s (enyo 1.0.4, mochi/mojo
1.0.0) and deployed all three with `push-variant.sh` (the real postinst, as root). On CE 3.1.0 that
gave: three daemons running, three YAP sockets, card renders, pages load over HTTP and HTTPS.

- **`<audio>` works on a clean install.** Vorbis: `paused=false t=9.926 dur=15`, ALSA `state: RUNNING`,
  pulse sink-input RUNNING at 100%. The 2026-08-20 "currentTime never advances, stream stuck PREPARED"
  finding in `context/impl/impl-media-controls.md` was an artifact of the hand-pushed engine that
  session left behind — it does not reproduce. The scrubber/volume-widget/volume-HUD items in that file
  have NOT been re-tested.
- **Fresh-install LS2 ordering — unverified, looks real.** The first daemon start after postinst logged
  `LSRegisterPalmService('net.riverstonerelay.jihadBrowser') FAILED (code=-1027)`; it registered after
  `ls-control scan-services` plus a daemon restart. If postinst starts the daemon before the hub has
  rescanned the role files, the toast channel and `clearCookies`/`getChromePrefs` are dead until the
  next restart. Read `packaging/gen-variant-scripts.sh` before believing either way.

### NEW DEFECT 1 — `<video>` dies with MEDIA_ERR_DECODE after 10–60 frames. ROOT CAUSE FOUND, fix NOT applied

Symptom, every codec (VP8, Theora), `file://` and `http://`:
`err=3:0x8007000e: DoDecode`, `vq=9/0/0` … `57/0/0`, then playback stops. The frame count at failure
varies run to run, which is the tell.

Cause, from two rounds of temporary stderr traces plus the source:

1. `BasicImageLayer::Paint` (`gfx/layers/basic/BasicImageLayer.cpp:72`) swaps the `ImageContainer`'s
   factory to the BasicLayerManager's `BasicImageFactory` **for the duration of the paint** and restores
   it afterwards. `IsCompositingCheap()` is false for BasicLayers, and BasicLayers is what paints here
   because OMTC is off (`JIHAD_DISABLE_OMTC=1`).
2. The decoder thread calls `ImageContainer::CreatePlanarYCbCrImage()` concurrently. Inside that window
   it gets a `BasicPlanarYCbCrImage`.
3. `VideoData::SetVideoDataToImage` always does `SetDelayedConversion(true)` then `CopyData()`, and
   `BasicPlanarYCbCrImage::CopyData` (`gfx/layers/basic/BasicImages.cpp:88`) does
   `RecyclingPlanarYCbCrImage::CopyData(aData); if (mDelayedConversion) { return false; }` — it reports
   failure for a copy that succeeded.
4. `CreateAndCopyData` returns null → the decoder returns `NS_ERROR_OUT_OF_MEMORY` → MEDIA_ERR_DECODE.

On desktop the paint window is microseconds; on the TouchPad a software paint is 40–160 ms, so a decode
lands inside it within a second or two. Evidence: the trace fired `CreateAndCopyData:
SetVideoDataToImage/CopyData FAILED` with NONE of the inner allocation/lock/size traces — i.e. the base
copy succeeded and the `false` came from the delayed-conversion line.

Proposed fix (three lines, engine, **not built, not verified**): in `BasicPlanarYCbCrImage::CopyData`,
return the base class's result and return `true` under delayed conversion. A delayed image is fully
usable — `GetAsSourceSurface()` already converts on demand when `mDecodedBuffer` is null.

This also re-dates two kit boxes: "VP8 plays" (2026-08-10) and the play/pause `[~]` (2026-08-15,
`t=0.73`) were both read inside the first second, before the failure.

Test assets: device `/tmp/jm/` (volatile) holds `jihad-media.html` (state readout in `#state`, pick the
clip with `?v=clip10v.webm`), `clip10{,v,a,g,op}.webm`, `clip10.ogv`, `tone15.{ogg,wav}`. Regenerate:
`ffmpeg -f lavfi -i "testsrc=size=320x240:rate=15:duration=10" -c:v libvpx -b:v 200k -an clip10v.webm`,
`ffmpeg -f lavfi -i "sine=frequency=440:duration=15" -c:a libvorbis -q:a 3 tone15.ogg`. The page itself
was pulled off the device and is now `render/goanna/test/jihad-media.html` (untracked, md5-matched to
the device copy); the clips are regenerated, not stored.

### NEW DEFECT 2 — an idle daemon fills `/var`

A daemon with NO card connected writes one `tick period` line every 2 s and nothing else: **82 B/s,
~7 MB/day per variant** while the device is awake. mochi and mojo had reached **10.1 MB each** after
3.6 days idle, on a 62 MB `/var`. Three variants idle is ~21 MB/day. The job rotates the log only at
start. Same family as `jihad-var-full-kills-daemon-log`: a full `/var` freezes the log and a frozen log
reads as a live one. The same lines show the idle daemon ticking at 8 ms with no page (`n≈241` per 2 s,
~2.8% CPU each, indefinitely).

### NEW DEFECT 3 — `about:blank` is a repaint storm

On `about:blank` the daemon logs `painted shmid=… bytes=0 … ms=35` about 22 times a second forever
(663 full paints in 30 s, 2.4 KB/s of log, ~210 MB/day). The full path gets a zero-byte frame, will not
publish it, and tries again every tick. `about:blank` is an allowlisted home page (`safeUrl`), so a user
can select this state. A static page with real content is quiet (16 lines / 30 s).

### Seen, NOT verified

On the media test page the card and daemon traded `emitGeometry contentSize=…` / `setZoomAndScroll
zoom=…` continuously (1662 → 2102 → 2022 → 2012 → 1972 → 2042 px). The page's `#state` is
`white-space:pre` and its event line grows, so the content width really does change every 500 ms —
this is probably the page and not a fit-width feedback loop. Do not record it as a defect without a
page whose content width is fixed.

### Kit work owed before any task (this is the next step, not optional)

- cavekit-device-build.md R4: the target is now CE 3.1.0; add criteria for fresh install, LS2
  registration on first start, coexistence with Atlas, and a re-verification sweep of the 3.0.5-only
  `[x]` boxes. R8: a log-growth bound and an idle-tick criterion (defect 2).
- cavekit-gre-widgets.md R1: `<video>` is broken past the first second (defect 1) — the "VP8 plays"
  and play/pause boxes are overstated. R7 still says "the engine has NO audio output at all", which
  has been false since 2026-08-15.
- cavekit-offscreen-rendering.md R8: a genuinely blank page must settle (defect 3).
- cavekit-browser-services.md R5: record the engine-NSS TLS position and the uncaptured `tls_version`.
- `context/impl/impl-media-controls.md`: the 2026-08-20 diagnosis is superseded (see above).

### Device and tree state as left

- All three variants installed from the 2026-09-27 `.ipk` payloads. **enyo runs a TRACED libxul**
  (md5 `cb514e9e11f34a6db94ceb1d44222cce`, prints `[jihad-media-trace]` on a failed frame copy); mochi
  and mojo run the `.ipk` build (`78c3b51a60d9959cbe313696e8ae26a7`). A push of all three puts them
  back in step.
- `/etc/event.d/jihad{,-mochi,-mojo}` all carry the device-local `JIHAD_INJECT=1`. **enyo's also carries
  `MOZ_LOG=sync,MediaDecoder:5,…`** from 2026-09-27 — it produced zero lines and should be removed
  (`sed -i 's/ MOZ_LOG=[^ ]*//' /etc/event.d/jihad`, then `stop jihad; start jihad`).
- The enyo card is parked on `/tmp/jihad-plain.html` (static). **Do not park on `about:blank`** — defect 3.
- mochi and mojo logs were truncated at 05:31 device time; `/var` is at 12% (54.7 MB free). At the
  idle rate that is days, not weeks — check `df /var` first thing.
- `third_party/uxp` is clean at `bd49435b`; the trace edits were reverted. The out-arm objdir libxul
  (2026-10-01 08:25) is the traced build and is stale against the tree until the next engine build.

---

## 2026-08-17 session delta — speaker route + live volume SOLVED, Mochi safeUrl done (read first)

The non-human-gated engineering surface is essentially exhausted this session. Kit-wide only **2**
criteria remain open (`grep -rn '\- \[ \]' context/kits`), BOTH keyboard/hardware-gated (there is no
keyboard on the device — open work item 1). PICKUP **open work item 3 is STALE**: R8's last criterion
is `[x]`, decided 2026-08-10 (geometry guards + the sub-rect argument bound the partial path; no blank
check needed).

- **Engine media now plays on the built-in SPEAKER at the system volume, and volume buttons track it
  live** (`render/goanna/BrowserPageGoanna.cpp`, commit `ed6cced6`; pairs with UXP-Jihad cubeb
  `bd49435b`). audiod refuses to route/scale OUR stream (its media volume is a PULSE control, proven by
  an `amixer contents` diff showing zero ALSA change on a button press), so the daemon drives the
  WM8994 mixer directly: a poll thread on `jihad_pulse_active_streams()` runs state-dir shell scripts
  it (re)writes from embedded constants — `audio-on.sh` (level `numid=74/86` from `getVolume`, then the
  DAC1→speaker switches `191/186/237/235/87`), `audio-off.sh` (mute amp `87` at rest → no max-volume
  burst), and `audio-vol.sh` every ~400ms (re-mirror `getVolume` → live tracking). `pulse.props` keeps
  the stream on `pcm_output` (NOT `media.role`/pmedia, which the forced route can't reach). Full detail
  + the dead ends: `context/impl/impl-audio-backend.md` §2026-08-17. Ear-confirmed: speaker ✓, no burst
  ✓, ~volume ✓. **RESIDUALS (human-gated, not shipped-blocking):** the volume OSD/HUD does not show for
  direct mixer writes (audiod-bypass, cosmetic); HTML5 video-controls taps are the separate offscreen
  hit-test item. **SHIP GATE:** parent gitlink + OE SRCREV stay at `81e10aa3` until UXP-Jihad
  `bd49435b` is pushed — the .ipk engine keeps the counter inert (audio unchanged) until then; the
  device runs the full hand-deployed stack now.
- **Mochi `safeUrl` allowlist done** (`app-mochi/source/JihadChromePrefs.js`): `http(s)://` or
  `about:(preferences|settings|jihad|isis|blank)` pass, everything else → `""`, applied in
  cleanLinks/loadHome/saveHome.

---

## 2026-08-16 session delta — bitbake `.ipk`s ship + the media scrubber is fixed (read first)

The two biggest open items are CLOSED, and this session's work IS committed (branch
`jihad/about-preferences-and-settings-merge`; engine mods on `third_party/uxp` branch
`jihad-engine-mods`).

- **OE bitbake produces all three `.ipk`s (device-build R3 DONE).** `bitbake
  net.riverstonerelay.jihad-browser{,-mochi,-mojo}` → `639 tasks ... all succeeded`, zero errors;
  each `.ipk` extracted + verified COMPLETE by content (daemon, libxul, plugin-container, ld-2.23,
  goanna.js, 1104-file chrome). Five recipe gaps were fixed to get here — see `../context/plans/build-site.md`
  T-154 and `../context/impl/dead-ends.md`. **Trap that ate two "successful" builds:** a goanna
  SRCREV bump does NOT re-fetch in this bitbake-1.18 setup (do_unpack keeps the old checkout); a
  chrome-only change needs `bitbake -c cleansstate goanna jihad-deviceroot
  net.riverstonerelay.jihad-browser{,-mochi,-mojo}` (after pre-warming the DL_DIR git mirror) + a
  CONTENT check of the packaged file, never mtime/size.
- **Media scrubber jitter/flash/no-settle FIXED (gre-widgets R1 back to `[x]`), device-confirmed.**
  Instrumentation proved the media/DOM state is perfect — the defect was the daemon's offscreen
  paint: damage-only repaint into three rotating buffers left GHOST thumbs (each buffer got only the
  latest frame's damage though it was several frames stale). Fixed with per-buffer damage
  accumulation (`BrowserPageGoanna` `BufFrame.dmg*`). Plus videocontrols: `ended` snaps the thumb to
  `duration`, and a 40ms interval interpolates the thumb from the live currentTime clock. 40ms is
  the ceiling — faster (25ms) overruns the adapter round-trip and FREEZES (checked against Atlas
  BrowserServer's identical double-buffer + wait-for-return model). Residual un-smoothness is the
  ~25–30fps software-paint cap, i.e. hardware.
- **Audio plays** through the cubeb PULSE backend (`JIHAD_PULSE_SINK=pmedia`), device-confirmed
  audible — the ALSA-bridged path was policy-muted; the pulse-native path gets 100%.
- **Engine provenance changed:** the jihad engine delta is now a durable commit `0c10df5c` on
  `third_party/uxp` branch `jihad-engine-mods` (pristine UXP `b2594a4` + the delta), pinned by the
  goanna recipe SRCREV — not only the desktop patch queue. Reproducibility caveat: that commit is
  now pushed to the fork `https://github.com/NotAlexNoyle/UXP-Jihad` (`.gitmodules` points there,
  branch `jihad-engine-mods`, parent gitlink bumped to `81e10aa3`), so a fresh clone + submodule
  update resolves the SRCREV.
- **Dotted-mochi `.ipk` was a 07-29 orphan (removed).** Current recipes emit only the HYPHENATED
  ids (`…-mochi` / `…-mojo`).

Device was used this session (TouchPad on novacom); it runs the pulse-audio libxul + per-buffer-fix
daemon + scrubber-fixed videocontrols. The 2026-08-15 and 2026-08-10 sections below are retained as
history — where they say "nothing is committed" or "the device is absent", read this delta instead.

---

## 2026-08-15 DEVICE session — the TouchPad reappeared near the end (historical delta)

Full transcript: `context/impl/impl-device-2026-08-15.md`. The device was plugged in mid-session and
the enyo variant was redeployed (ipk 1.0.4 + current daemon/adapter). Verified ON HARDWARE:

- **Cert store (T-135) — CLOSED end to end.** `/var/ssl/jihad/enyo` created (daemon is in group
  `luna`), real DER→PEM written, classified to ordinal 18 from `nsISSLStatus` flags, `CertInitCertMgr
  rc=0`, `Adding Entry … serial 6`, `CertAddTrustedCert` rc=0, NSS override + reload, page loads.
  `libPmCertificateMgr.so` present on 3.0.5. browser-services R5 device box → `[x]`.
- **Toast channel (T-146) — CLOSED.** `clearCookies` on the public bus pushed a notify and the Enyo
  card rendered the "Cookies cleared." toast, visible in a device screenshot. gre-widgets R5 last → `[x]`.
- **Focus transitions correct on device** (editorFocused 1→0→1); the desktop focus_test failure is
  harness-side, not the daemon.
- **AUDIO: the backend works and reaches pulse, but output is webOS-POLICY-gated — and the gate is now
  FULLY reverse-engineered.** `<audio>` gives `err=0` and pulse shows our sink-inputs at
  `s16le 2ch 44100Hz`, but sinks stay SUSPENDED — webOS powers the WM8994 codec route only while a
  policy-recognised media stream plays. Stock mechanism: `element.palm.audioClass="media"` (Palm
  WebKit extension, `media.js:288`) → `pmedia` sink + audiod media scenario. Port's path (specified,
  three parts, none human-gated, in `impl-audio-backend.md` §2026-08-15): (1) route engine ALSA at the
  `pmedia` sink via `ALSA_CONFIG_PATH`, (2) hold a `com.palm.audio/media` scenario ref-counted over the
  playback lifetime (daemon already has an outbound Luna client), (3) validate. the mechanism is now
  FULLY DECODED and the fix is DETERMINED (a bounded build task, not research). Read
  `module-palm-policy.so`: virtual sinks (pmedia/…) are NULL sinks; audiod bridges one to hardware +
  powers the codec ONLY for a stream CREATED on that virtual sink; default-device streams are muted
  ("PLEASE CATEGORIZE USING A VIRTUAL STREAM"). Engine decode+clock WORKS (t 0→3.00, ended, err=0) but
  is policy-muted on pcm_output at 0%. **The ALSA backend was a DEAD END (policy-muted); the cubeb PULSE
  backend was BUILT + DEPLOYED + DEVICE-VERIFIED 2026-08-15 and it UNMUTES engine audio.** The engine
  stream is now pulse-native (`application.name="Jihad Browser"`), `Volume: 100%`/uncorked (not the
  ALSA path's forced 0%), and `/proc/asound/…/status` shows the WM8994 DAC consuming its samples
  (`hw_ptr` advancing ~44100/s). Implemented: mozconfig `--enable-pulseaudio`, pulse-0.9.22 headers
  staged in the sysroot (device-ABI-matching; re-fetch via `curl
  freedesktop.org/software/pulseaudio/releases/pulseaudio-0.9.22.tar.gz`, headers `src/pulse/*.h` +
  a `libpulse.pc`), patch `0030-cubeb-pulse-webos-virtual-sink.patch` (`JIHAD_PULSE_SINK`; cubeb
  dlopens libpulse, no link dep — all 62 symbols present in the device lib). The pulse-native app
  IDENTITY is what unmutes it. **Residual (the only thing left, needs a human EAR — both automated
  oracles broken here: `parec` monitor = 0 bytes, `hw_ptr` runs during pause): whether the speaker
  physically emits, plus a possible daemon-side hold of the `com.palm.audio/media` scenario to power
  the speaker amp (the unmute does not need it, measured).** Full spec: `impl-audio-backend.md`
  §"RESULT 2026-08-15". **Bundle-Jessie-libasound decision is WRONG:** Jessie 1.0.28 → `err=3`,
  device's own `/usr/lib/libasound.so.2` → `err=0`. Stop shipping the bundled one. MP3/AAC/H.264
  decoders remain a separate, orthogonal gap.

Device traps found: two stray `jihad.bak.*` upstart jobs were contending for the enyo socket
(removed); cryptofs forbids symlinks; a `file://` XPI navigation does NOT trigger the install flow
(so T-103's device screenshot is still open — needs an http-served or card-initiated XPI). Device
left: enyo on current binaries, job env back to shipped flags, test certs swept, adapter backup at
`/var/palm/jihad/adapter-backup-20260815/`.

## 2026-08-15 session delta — read before the 2026-08-10 body

A cavekit loop (`context/plans/build-site.md`, now reconciled row-by-row; per-wave record in
`context/impl/loop-log.md` Iteration 3) closed or advanced every host-doable open criterion.
No device all session, so nothing below is device-verified unless it says so.

**Closed outright:** ipc-contract R1 (generator vendored+patched, regeneration reproduces the
shipped wire, zero hand edits — `render/browserserver/CodeGen/`); gre-widgets R5 first criterion
(notificationbox attach/dismiss PASS on the offscreen PuppetWidget path — `prefsui_test`);
gre-widgets R7 prefs-single-source (eleven rows observed from the shared file on desktop);
preferences-ui flush box (was already device-proven 2026-08-10, box reconciled); opal machine
config (kernel string was WRONG — board is `shortloin`; pinned from HP's own source drop);
T-115 prebuilt-input manifests (187-deb sysroot + adapter-deps, generators in `build/webos-oe/`).

**Built, awaiting a device:**
- **Engine audio output** — cubeb ALSA backend is IN the ARM libxul (`--enable-alsa`, Jessie
  alsa-lib staged; `DT_NEEDED libasound.so.2`). First listen: `jihad-media.html` with an
  Ogg/Vorbis or WAV asset (MP3/AAC/H.264 decoders are STILL OFF — ffvpx config, separate gap).
  Fallbacks if silent, in order, in `context/impl/impl-audio-backend.md` (PCM `default` may route
  to a nonexistent pulse plugin; `ALSA_CONFIG_PATH` override; drop-bundled-lib).
- **Cert store (T-135)** — all three 2026-08-10 defects fixed and desktop-proven live (real PEM
  certFile, Palm-ordinal error mapping off `nsISSLStatus` — the raw nsresult was a transport-class
  lie —, three-way trust with session-serial sweep at teardown AND startup). Platform writes via
  dlopen'd `libPmCertificateMgr`; first device line: `ls /usr/lib/libPmCertificateMgr.so*`.
  Read-direction (store→NSS) is COSTED not built (`context/impl/impl-cert-store.md`).
- **Touch events (T-120)** — fence out behind **`JIHAD_TOUCH_EVENTS=1` (default OFF)**; 28-byte
  header re-staged (sizeof guard proven both ways); double-activation suppressor daemon-side;
  device procedure in `context/impl/impl-touch-events.md`. Two real bugs fixed en route (TouchEnd
  OOB index; `npPalmEnableTouchEvents` was never set — un-fencing alone was a silent no-op).
- **Non-blocking toast channel (T-146/T-148)** — `jihad::PostNotification` → `notifications` Luna
  subscription → plain-DOM toasts in all three shells. The only informational dialog (T-103
  incompatibility alert) moved onto it; cookies/cache-cleared and add-on-installed notices ADDED
  (they emitted nothing before). Three device unknowns; one Preferences→Clear-cookies run splits
  them (`context/impl/impl-toast-channel.md`).
- **XPI mismatch alert (T-103)** — observer fires on desktop with a discriminating control; device
  screenshot is the closer.

**NEW TRAPS, same class as the body's:**
- **`out-ipk/` ipks ≤1.0.3 have NO libasound.so.2 and their daemon will NOT START** against the
  new libxul. Install 1.0.4+ only; the enyo 1.0.4 / mochi / mojo ipks built 2026-08-15 carry it
  (verified in all three).
- **`build-goanna-arm.sh` exited 1 on every SUCCESSFUL build until 2026-08-15** (grep|while under
  pipefail; 27/29 patches have no JS). Fixed; historical automation that trusted its exit code
  never saw its own staleness guards run.
- **`mFlashGestureLock` is set ONLY by a DOUBLE-tap** (`handlePenDoubleClick`, which also
  smart-zooms) — the "tap inside a plugin rect" wording in older notes is wrong, and NO latch log
  compiles (`Debug.h` DEBUG off). T-124/T-144 rescoped; keyboard-arbitration procedure rebuilt
  around the address-bar screenshot (addons kit R7, 2026-08-15 annotation).
- **`javascript:` URLs are dead in ALL chrome documents** (HTML too, not just XUL) — so
  `GoannaRenderPage::ScrollTo` is a no-op on chrome pages. And a **stale desktop profile** fakes
  pref regressions (see `context/impl/dead-ends.md`, both dated 2026-08-15).
- The dotted Mochi app id in the workspace CLAUDE.md is fixed (hyphens are correct).

**What remains, exhaustively:** every open criterion is device-, hardware-, or human-gated. With
a device + novacom (needs root for novacomd): the verification batch above, T-108 (hidd routing),
T-116, T-119/T-133, T-131, T-141, T-142→T-147. Human at device: T-150/T-151/T-152 and the frame
pacing EYEBALL decision (device deliberately left in Flash-alone — body below). Human decisions:
T-155 sign-off, T-156 Mojo settings exemption, whether to commit this tree (137+ files were
already uncommitted before 2026-08-15; the session added more and committed nothing).

---

## What this is

Jihad Browser: a fork of isis-browser (webOS 3 / HP TouchPad) with **UXP/Goanna** replacing
QtWebKit, keeping the isis UI and the BrowserAdapter↔BrowserServer YAP IPC contract. Three
variants ship independently: Enyo (`app/`), Mochi (`app-mochi/`), Mojo (`app-mojo/`).

Requirements live in `context/kits/` — start at `cavekit-overview.md`. The Flash/NPAPI work is
`cavekit-addons-extensions.md` **R7**.

---

## Flash: where it actually stands

Adobe Flash renders animated content on the TouchPad through UXP/Goanna, takes mouse and
keyboard input, **plays sound**, survives crashes and freeze/thaw, and holds ~30 fps composite
with patch `0027`. Two things are UNFINISHED: chrome-side keyboard arbitration (untestable on
this hardware) and frame PACING — the composite RATE is met, and the pacing ROOT CAUSE is now
found and measured: **two producers.** Flash invalidates itself and we also drive it, and our
frames land between its own. Isolated three ways; Flash alone is slower but has **zero** gaps
under 16 ms. The decision left is a design one, not a debugging one — see open work item 2.
**Audio is CLOSED**, at parity with stock; an
earlier version of this sentence listed "MP3 static (fix landed, unheard)" and that contradicted
the dated audio section below, which cost a session start. Precisely:

| | state |
|---|---|
| renders animated content | ✅ device-verified |
| **frame rate** | ✅ **composite 30.6-32.5 fps** via patch `0027` (CPU boost); gap avg 28-29 / max 45-49 ms |
| **mouse input** | ✅ `palm event 0x1/0x2/0x10000` at correct coordinates |
| **keyboard input → plugin** | ✅ Flash's own AVM1 runs on a keypress (green 11768 → red 11750) |
| **keyboard arbitration (chrome)** | ⚠️ **UNTESTABLE on this hardware** — no keyboard exists; synthetic one is read by hidd but never dispatched |
| crash containment | ✅ daemon survives a plugin SIGSEGV; plugin respawns on reload |
| spotlight event | ✅ suppressed by default (delivering it kills Flash — matches stock) |
| **audio** | ✅ **CLOSED 2026-08-10 at parity with stock.** All three defects were in the TEST ASSETS. The residual per-loop click is Flash restarting its MP3 decoder, and the STOCK browser does it identically |
| **frame PACING** | ⚠️ **ROOT CAUSE FOUND 2026-08-10: TWO PRODUCERS.** Flash invalidates itself ~25/s AND we drive it ~26/s, and our frames land between its own — that interleaving is the bunching. Isolated and measured all three ways: Flash alone is **slower but perfectly unbunched (zero gaps under 16 ms)**, the pull is faster and bunched. The device is LEFT in Flash-alone for a human to eyeball |

### Audio: solved, and the bug was in the TEST ASSET

**Flash audio needed no port-side change at all.** Every "silent" measurement in this tree was
taken against SWFs that could not have made a sound in any player.

`make-audio-swf.py` emitted `DefineShape` **and** `DefineSound` with the **same character ID 1**.
A SWF has ONE character dictionary shared by shapes and sounds, so the sound redefined an ID that
was already taken; the player keeps the first definition and drops the sound, `StartSound 1` then
resolves to a shape, and nothing plays. Silently: no error, no audio device ever opened, no mixer
thread. All three assets (PCM, MP3 mono, MP3 stereo) shared the bug, which is exactly why they
"behaved identically" and made this look like a player-side gate.

Fixed by giving the sound its own ID (`SOUND_ID = 2`). With the STOCK, UNPATCHED
`libflashplayer.so` the corrected SWF immediately produces the whole ALSA bring-up that was
missing for the entire investigation:

```
open("/usr/share/alsa/alsa.conf")                       = 27
access("/etc/asound.conf", R_OK)                        = 0
open("/usr/lib/alsa-lib/libasound_module_pcm_pulse.so") = 27
open("/usr/lib/libsndfile.so.1")                        = 27
```
plugin-container goes **3 → 5 threads**, and the tone is audible from the device speaker.

All three assets have been regenerated and re-pushed. Regenerate before trusting any audio
measurement anyway — `/tmp` is volatile, and any SWF still sitting on a device from before
2026-08-10 is a broken one. The `--mp3` variants need a BARE frame stream with no ID3; make one
with `ffmpeg -f lavfi -i "aevalsrc=..." -codec:a libmp3lame -write_xing 0 -id3v2_version 0`.
PCM is confirmed clean by ear. **MP3 was heard too and IS clean** — the per-iteration click is
Flash restarting its decoder at the loop point, and the stock browser does it identically, so it
is not a port defect. Judge MP3 quality ONLY with `jihad-audio-long.swf` (30 s, `--loops 1`),
which has no loop point at all; every other MP3 asset here clicks once per iteration no matter how
healthy the port is. There is no open audio item.

### The "audio gate" in the previous handoff was the VIDEO DECODER — do not chase it

The function at **0x70ee0** and the loader at **0x73020** have nothing to do with audio. Measured,
not guessed: 0x73020's string table is `libmm-omxcore.so` / `libOmxCore.so` resolving `OMX_Init`,
`OMX_GetHandle`, `OMX_GetComponentsOfRole`; 0x70ee0 passes `"video_decoder.avc"` (rodata 0x760cec)
and `strncmp`s candidates against `"OMX.Nvidia"` (0x760d00) with 128-byte entries
(`OMX_MAX_STRINGNAME_SIZE`). It is hardware H.264 decoder setup. `+24` is a codec enum, and it
**does** equal 7 — the factory at 0x5bff0 returns NULL unless the caller asks for type 7 and the
constructor at 0x72a30 stores that same 7 (`72a94: str r4,[r8,#24]`). The old note had the gate
inverted as well as in the wrong subsystem.

**Where the real ALSA code lives**, if it is ever needed again (all GOT-relative, base 0x85897c):
- **0x2f7bd0** — the ALSA loader. `dlopen("libasound.so")`, falls back to `"libasound.so.2"`,
  resolves ~50 `snd_*`, then NULL-checks every one; any NULL and it returns 0.
- **0x2f71e0** — `OpenPCM`, vtable slot +8 at vptr 0x84e440, ctor 0x2f75f0. Opens the PCM, sets
  access/format/rate/channels, 500 ms buffer / 20 ms period, then `pthread_create`s the mixer.
  It asks for **`plughw:0,0`** (0x7b54c8) as its FALLBACK name — the old note's `hw:0,0` was that
  string read four bytes in.
- `0x2fbf08` is a separate availability **probe** that loads libasound unconditionally at startup.
  That is the `dlopen` every previous trace saw; it proves nothing about playback.

**Already eliminated — do NOT redo these:**
- Our runtime can play sound. `render/goanna/test/alsa_probe.c` writes a square wave and it is
  AUDIBLE, under both the device's own loader and `./ld-2.23.so --library-path $HL`.
- Not PIpc (zero `PIpcClient` ctors even with an audio SWF), not LS2 permissions (role grants
  `outbound:["*"]`), not Flash's OEM profile (`/media/cryptofs/apps/etc/adobe/{mms,oem}.cfg`,
  `cctrl = 3`), not click-to-play deferred start, and not the host — `libWebKitLuna` has no
  PluginView audio methods and no ALSA/pulse references at all.
- Not the two gates that looked like candidates. Both were nopped on-device and changed NOTHING:
  the sample-count check `2f7234: cmp r1,#31 / ble` in OpenPCM, and the `audioEnabled` check
  `19ba54: beq` on `sound+0x270`. That pair of null results is what finally pointed at the asset.
- One real bug WAS found and fixed here: Flash's `kind not registered:
  'com.palm.app.flashplayer.prefs:1' (-3970)` on every instantiation. The db8 kind is now
  registered (owner `com.palm.flashgraphics`) and the error is gone. It was unrelated.

---

### Frame rate: the cause is CPU FREQUENCY, not the pipeline and not Flash

**Measured 2026-08-10.** Sampling `/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq` once a
second while `jihad-anim.swf` played:

```
192000 192000 192000 192000 192000 192000 192000 540000 384000 384000 384000 384000 384000
idle, right after navigating AWAY: 1188000
```

Flash renders at **192-384 MHz out of a 1188 MHz maximum**. Frame rate tracks it exactly — daemon
paints 20.8-23.9 fps, composite 16.3-29.1 fps — and the SPREAD is what reads as "skippy". The
system was only 69% busy across 2 cores at the time, so this was never a capacity wall; the
pipeline is clean (`deferred=0`, `wanted == done`), and on one run at a higher clock the same
pipeline reached **39.2 fps**, well above the 30 the content is authored at.

The governor is Palm's **`ondemandtcl`** — touch-biased. It boosts while the user is touching the
screen and decays otherwise, so passive playback (watching an animation, listening to audio) runs
at the floor. This also explains the audio xruns: MP3 decode at 192 MHz underruns.

**DO NOT "just set the governor to performance" — it WEDGES THE DEVICE.** Writing
`scaling_governor` while `ondemandtcl` is live deadlocks the cpufreq policy lock in the kernel.
Every later reader of any cpufreq sysfs node blocks in **uninterruptible D state** — including the
Jihad daemon, `powerlog`, and the restore script itself, so the governor never gets put back and
the daemon stops painting. D state cannot be killed; only a reboot clears it. Nothing persists
(it is a runtime sysfs value), so a reboot returns to `ondemandtcl`, but the whole session's
measurements are lost. This cost a device reboot on 2026-08-10.

**THE SAFE LEVER, AND IT WORKS — measured 2026-08-10.** `ondemandtcl` exposes ordinary governor
tunables at `/sys/devices/system/cpu/cpufreq/ondemandtcl/`. Writing those does NOT switch
governors and does NOT take the path that deadlocks; the write returns instantly and a readback
succeeds. Stock values and the two that matter:

```
up_threshold  = 95      <-- only ramps when a core exceeds 95% load
sampling_rate = 200000  <-- and only looks 5 times a second
```

95% is why Flash never boosts: the daemon runs ~26% and plugin-container ~11% of two cores, so no
single core ever approaches the threshold. Setting `up_threshold=40` and `sampling_rate=50000`
pins the clock at 1188000 and produces, on `jihad-anim.swf` (30 fps authored), single instance:

| | stock | tuned |
|---|---|---|
| daemon paints | 24.0-28.7 fps | **31.4-33.0 fps** |
| card composite | 20.4-27.5 fps | **30.7-35.2 fps** |
| frame gap avg / max | 34-42 / 63-81 ms | **30-31 / 49-56 ms** |

Composite now meets the authored rate AND is matched to the draw rate, which is what the
criterion asks for. `deferred` stayed 0 throughout, both before and after — the pipeline was
never the constraint.

**What is NOT yet done: making this OURS rather than a manual poke.** These are SYSTEM-WIDE
tunables, so the daemon must apply them only while a plugin instance is alive and restore the
stock values on teardown (and on crash — an upstart `post-stop` is the honest place for the
restore, since a daemon that dies with `up_threshold=40` leaves the whole device tuned for
battery-burn). Read the stock values rather than hard-coding 95/200000. Do NOT touch
`/sys/devices/system/cpu/cpufreq/override/{turbo_mode,vdd_*}` — that is overclocking, and
`vdd_freqs` goes to 1836000.

---

## Frame pacing — what was wrong, what is fixed, what is left (2026-08-10)

The user's report was "the pink square is smoother in the STOCK browser". It is a real defect and
the average frame rate never showed it: the daemon was delivering 35-42 fps for a 30 fps SWF.

**The measurement that found it is a gap HISTOGRAM, not min/avg/max.** Summary statistics cannot
distinguish "evenly late" from "unevenly on time", and every earlier session's numbers were
summary statistics. Added to the 2 s counter block; the failure looked like this:

```
gap hist ms <16=19 16-23=20 24-31=10 32-39=24 40-55=6 56+=0     <-- FLAT = no pacing at all
```

Frames were landing at effectively random instants between 8 and 55 ms, so the compositor held
each one for anywhere between half a refresh and three. Three separate causes, all measured:

1. **`PumpFor` burned the tick.** It `g_usleep(1000)`s every iteration, and a 1 ms sleep really
   costs ~3 ms on this device, so `PumpFor(4)` measured **12 ms** of a 16 ms tick with the page
   IDLE. The tick was therefore WORK-bound at 26-28 ms rather than timer-bound at 16, and since
   a frame can only be published on a tick, every gap quantised to that. Fixed by `PumpReady()`,
   which drains what is ready and returns without sleeping — **work avg 12 ms → 1 ms idle**.
   `PumpFor` KEEPS its old behaviour because the tests use it as "let the engine run for N ms".
2. **Nothing paced the publish.** The damage gate was an 8 ms floor deliberately set BELOW the
   tick so it never gated. A fixed 33 ms deadline grid (not a floor — a floor drifts and beats,
   which is what the 2026-08 note in the source records) moved 30% → 69% of frames into the
   32-39 ms bucket.
3. **The daemon could not tell a NEW plugin frame from a repeat**, because its only signal is
   PuppetWidget's sticky dirty boolean, which the refresh driver and ordinary page damage also
   set. That ambiguity makes BOTH available policies wrong: publishing on every dirty tick sent
   ~78 card frames for ~60 plugin frames (duplicates, and `deferred` 22-44 because the adapter
   could not drain them), while the 33 ms grid sent ~57 for ~68 and DROPPED frames, so the
   animation stepped two positions at once. Fixed by exporting `jihad_plugin_frame_seq()` from
   `PluginInstanceParent::RecvShow` (weak-linked exactly like `jihad_offscreen_take_dirty`) and
   publishing one card frame per plugin frame: **done=91 against palm draw=92, `deferred=0`.**

**Why stock is structurally smoother, and it is not a tuning difference.** Stock has exactly ONE
clock: `PluginViewQt`/`PluginViewPalm` pulls the plugin synchronously inside the page paint, and
the producer blocks until the consumer drains, so publish times ARE drain times. We have two
free-running clocks — the plugin child's own repaint timer (a delayed task RE-POSTED after its
work, so its period drifts) and the daemon's tick — and we SAMPLE the plugin rather than pull it.
That is the remaining gap, and it is the next thing to attack.

**Two things were tried and are NOT settled — the runs that judged them were confounded:**
- **An 8 ms tick** (`JIHAD_TICK_MS=8`). Works mechanically: tick period 9-11 ms, 1:1 delivery,
  `deferred=0`. But the plugin then redrew ~45 times/s for 30 fps content, so content frames are
  shown 1 or 2 times in a beat — even DELIVERY of uneven CONTENT, the same artifact one layer up.
  Worth revisiting only together with a draw rate that matches the content.
- **`JIHAD_PLUGIN_REPAINT_DIV=3`** on top of it, to bring draws back to ~30. Draw rate did land
  at 28-34 fps, but the daemon then logged `flashRect remove id=N (gone)` and stopped painting
  while the child kept drawing.
  **That was NOT caused by the divisor, and the first write-up of it here was wrong.** The same
  stall persisted after reverting to `JIHAD_TICK_MS=16` and the default divisor, and a CLEAN
  restart (`stop jihad; pkill -f jihad-browserserver; pkill -f plugin-container; start jihad;
  killall LunaSysMgr`) cleared it completely on the identical build: `done=61 palm draw=63
  deferred=0`. What actually happened is a plugin CRASH LOOP — the log carries a
  plugin-container maps dump — reached by cycling `inject url` between pages repeatedly. Once the
  child is in that state the plugin rect goes away, widget invalidation stops with it, and every
  later number reads as a pacing regression that is not one.
  **NEW TRAP, same family as the two below: repeated inject-driven navigation can wedge the
  plugin, and the symptom is silence, not an error.** `flashRect remove id=N (gone)` with the
  child still logging `palm draw` is the signature. Do a clean restart before believing any
  frame measurement that follows one.

Everything here is env-tunable for A/B without a rebuild: `JIHAD_TICK_MS` (4-64, default 8 in
code / 16 on the device job), `JIHAD_PLUGIN_FRAME_MS` (0 disables the pacer and the seq gate with
it), `JIHAD_PLUGIN_REPAINT_DIV` (1-8, child side), and — added 2026-08-10 because the frame
question could not be A/B'd without them — the two DRIVER switches:

| var | side | effect |
|---|---|---|
| `JIHAD_PLUGIN_PULL=0` | daemon | disables the host request ONLY, leaving the publish policy alone. Use this, NOT `JIHAD_PLUGIN_FRAME_MS=0`, which also drops the publish gate to the 8 ms floor and so changes two things at once |
| `JIHAD_PLUGIN_SELF_DRIVE=0` | child | disables the child's own repaint timer outright |

**Setting only `JIHAD_PLUGIN_PULL=0` does NOT isolate the plugin** — the child's host-drive lease
expires and its timer takes over, so you measure patch `0026`, not Flash. Both switches together
leave the plugin's own `NPN_InvalidateRect` as the sole frame source. Each logs a line when it
engages (`plugin PULL disabled` / `self-drive timer DISABLED`); grep for it before believing a run.

**And when reading `show req:`, `hostinval` counts everything that reached
`JihadPalmRepaintTick` — the pull AND the child timer. It is not "the pull" on its own.**

### The PULL is built and measured (patch `0029`, 2026-08-10) — and it did NOT fix the pacing

`0029` does exactly what the item above asked for: an async parent→child `JihadRequestFrame`, the
child answering it with its existing `JihadPalmRepaintTick()`, its own timer standing down on a
150 ms LEASE (not a latch — the daemon can stop asking at any moment, and a latched plugin would
then never paint again), and the daemon asking once per whole-tick grid slot from its own tick.
It is live on all three variants. **The mechanism works and the outcome is not a win:**

|  | `0028` (sample) | `0029` (pull) |
|---|---|---|
| daemon done | 29.4-31.9 fps | 32.8-34.0 fps |
| frame gap avg | 31-34 ms | 29-30 ms |
| frame gap max | 47-62 ms | 45-55 ms |
| gap hist `<16` ms | 1-4 of ~60 | **5-8 of ~68** |
| `deferred` | 0 | 0 |

Average and max improve; the HISTOGRAM gets flatter, which is the metric that matters. Both runs
were clean restarts of `jihad-anim.swf` at `JIHAD_TICK_MS=16`, log-growth and card-connection
asserted. Why it does not work is open work item 2: **the pull is not exclusive** — 3988 draws
against 3072 requests — so a third driver is producing most of the frames.

Two things worth keeping from this even if `0029` is later reverted:

- **THE TICK IS ITSELF A JITTERY CLOCK, and that bounds every pacing scheme.** Against a 16 ms
  timer the measured period is `min=15 avg=18 max=41`, with `work avg=10-11` (pump 7, paint 3).
  Locking the plugin to that clock inherits its jitter, so "one clock" is necessary and not
  sufficient. **`JIHAD_TICK_MS=8` is WORSE, measured, and not for the reason recorded earlier:**
  the tick cannot actually run at 8 ms (work is 11-13 ms, so the observed period stays 14-16 ms),
  the draw rate rises to 36-40 fps for 30 fps content, and the histogram spreads across the 16-23
  and 24-31 buckets. Leave the device job at 16.
- **There is no clean runtime A/B for the pull alone.** `JIHAD_PLUGIN_FRAME_MS=0` does stop the
  requests (the child's lease expires and it resumes self-driving, i.e. `0026`/`0028` behaviour),
  but the same variable also drops the publish policy to the 8 ms floor, so the two effects cannot
  be separated without a rebuild. Do not read a `=0` run as "the pull, off".

## Open work, in priority order

1. **Keyboard arbitration, card side — blocked on the hardware, not on the code.** Keys reach and
   drive Flash; what is NOT verified is that the CARD does not also act on them. The gate is
   deployed (`handleKeyDown/Up` on `mPageFocused && (bEditorFocused || mFlashGestureLock)`) and
   its return value IS the arbitration, so the expression is not in doubt — what needs proving is
   that `mFlashGestureLock` is true during play.

   **A human at the device cannot test this either.** There is no keyboard: `/proc/bus/input/
   devices` has only `gpio-keys`, `pmic8058_pwrkey`, `headset`, and webOS's on-screen keyboard is
   drawn by LunaSysMgr in-process, not through `/dev/input`. The daemon's `key` inject command
   goes straight to the page and bypasses the adapter entirely, which is why it proved "Flash gets
   keys" and can never prove anything about the chrome.

   `render/goanna/test/uinput_kbd.c` synthesises a real one (PDK build → device glibc, no bundled
   loader; `build` it with the PDK gcc plus `--sysroot`). `/dev/input/uinput` exists and works,
   the device registers as `event3`, and **both `hidd` and LunaSysMgr open it** — yet no key ever
   reaches the daemon and `window.pageYOffset` never moves. So hidd reads it and does not dispatch
   it. Next: `/etc/hidd/HidPlugins.xml` routes generic devices through `HidInputDev`
   (`/usr/lib/libhidinputdev.so`, `/var/run/hidd/InputDevEventSocket`) — find out what it requires
   of a device before it forwards. Also fix the observable: arrows were used on the assumption the
   chrome scrolls, which a touch OS may not do at all; the adapter's own comment says the leak
   appeared as keystrokes landing in the ADDRESS BAR, so screenshot the URL field instead.
   `jihad-keyarb.html` (tall page + key SWF + `gettext #sy` scroll readout) is staged for this.
2. **Frame pacing: the pull is BUILT (patch `0029`) and it is NOT the fix. Find the third draw
   driver.** The pull described in the previous handoff now exists end to end and is device-live —
   see the section above for the numbers. It did not tighten the gap histogram, and the reason is
   a measurement nobody had taken before: **the plugin's draw rate is not set by whoever you think
   drives it.** Over 123 s the child logged **3988 draws against 3072 host requests**, and at an
   8 ms tick the gap widened to 3988-vs-~1900 territory (38 fps of draws for ~15 requests/s). So
   the majority of draws — and ALL of the excess — come from neither the child's own timer (it is
   provably stood down: exactly one `frame clock -> HOST` transition and no fall back for the
   whole run) nor from Flash self-invalidating (`gJihadNPNInvalidateRectCalls` stays 0, which is
   precisely why the host-driven path keeps running).
   **ANSWERED 2026-08-10, and the suspect this paragraph used to name was WRONG.** The counters
   were added (every `AsyncShowPluginFrame` call site, tagged, reported next to `palm draw` as
   `show req:`) and `RecvUpdateBackground` measured **exactly zero** — as did every other
   background path. The hypothesis that it was parent-driven and correlated with daemon paints is
   refuted; do not re-run it.
   **It was OUR OWN LEASE FLAPPING.** Every draw traces to `InvalidateRect`, and the accounting
   closes to the unit: **1856 host requests + 1846 child-timer invalidates = 3702 total**, with
   the child logging `HOST -> SELF -> HOST`. The daemon does not re-ask while a request is
   outstanding — it waits out `kJihadPluginReqTimeoutMs` (250 ms) — but the child's lease was
   150 ms, i.e. SHORTER, so one late frame dropped the lease and restarted the second clock. The
   invariant is now written into the source: **lease > request timeout + one grid period +
   margin** (lease 500 ms). Verified: the clock log now shows one `-> HOST` and no fallback.
   **AND THEN THE REAL ONE — PATCH `0026`'s FOUNDING PREMISE IS FALSE.** With the lease fixed,
   1344 host requests still did not account for 2662 invalidates. Splitting the counter at the
   funnel by whether `JihadPalmRepaintTick` is on the stack settles it:
   **`hostinval` ≈ 27/s against `plugininval` ≈ 22/s.** Flash invalidates itself constantly.
   The reason three sessions believed otherwise is that `gJihadNPNInvalidateRectCalls` — the
   counter `0026` reads to decide "the plugin never asks to be repainted" — is **structurally
   blind**: it lives in `JihadNPNInterpose.cpp` and only sees calls that bind the
   `NPN_InvalidateRect` SYMBOL, while an ordinary NPAPI plugin calls through the NPNetscapeFuncs
   table, which `PluginModuleChild` fills with its own `_invalidaterect` (`:986`) that forwards
   straight to `InstCast(aNPP)->InvalidateRect` (`:1302-1310`) and never touches the interpose.
   It can read 0 forever while the plugin invalidates 22 times a second.
   **So the pacing defect is TWO PRODUCERS BEATING**, ours and Flash's, funnelled into one
   coalescing path (~49 invalidates/s collapsing to ~32 draws/s). That is exactly the shape that
   produces an unconcentrated gap histogram at a healthy average.
   **NEXT EXPERIMENT, and it is cheap:** measure Flash with BOTH drivers off. `JIHAD_PLUGIN_FRAME_MS=0`
   alone is not enough — it stops the pull, the lease expires and the child's own timer takes
   over, which is just `0026` again. It needs one more child-side kill switch to disable the
   timer outright. If Flash alone holds the authored rate, the host clock should be deleted for
   plugins that self-invalidate rather than tuned.
   Measure with the gap histogram, NOT with average fps: the average was 35-42 fps throughout the
   whole period the animation looked worst, and `0029` IMPROVED the average and the max while
   making the histogram flatter.
3. **~~The partial-paint path publishes with no blank check~~ — STALE, resolved 2026-08-10.** R8's
   last criterion is `[x]`: the second pass decided NO blank check is needed. The geometry guards
   (segment-size check) catch the corruption class, and a partial paint only ever rewrites a sub-rect
   of a frame that already passed the full path's non-blank check, so it cannot publish a blank frame
   invisibly. Do not re-open. (Full reasoning: cavekit-offscreen-rendering.md R8, lines ~200-218.)
4. **`plugin-container` aborts on teardown.** Every navigation away from a Flash page ends in
   `mozalloc_abort` (pc 0xc35c in plugin-container's own text) — a deliberate Gecko abort, not a
   segfault. Harmless today (the child is going away) but it is noise in every log.
5. **The in-daemon plugin scan is a landmine.** `nsPluginFile::GetPluginInfo` dlopens EVERY
   plugin in the daemon ("Sadly we have to load the library for this to work"), pulling
   libWebKitLuna/libPiranha/libv8/libEGL into it, and `dlclose` does not unload them. A hostile
   plugin can wedge the daemon before any plugin logging runs.

---

## Traps — every one cost real time and none is discoverable from the source

**Device / measurement**
- **AVERAGE FRAME RATE CANNOT SEE THE DEFECT THE USER IS REPORTING.** Throughout the entire
  period the animation looked worst by eye, the daemon reported 35-42 fps for a 30 fps source —
  ABOVE the content rate. min/avg/max cannot separate "evenly late" from "unevenly on time"
  either. Read the `gap hist ms` bucket line: flat across the buckets means no pacing, weight in
  one bucket means smooth. Every earlier session's frame-rate conclusions were drawn from summary
  statistics and are worth re-reading with that in mind.
- **THE PLUGIN'S DRAW RATE IS NOT SET BY WHOEVER YOU THINK DRIVES IT — count, do not reason.**
  Three sessions have now attributed the draw rate to a specific clock (the child's timer, then
  the daemon's tick, then the host pull) and every attribution was made from reading the code, not
  from a count. When `0029` finally put a counter on the request side the numbers did not agree
  with ANY of them: 3988 draws against 3072 requests, with the child's own timer provably stood
  down and Flash provably not self-invalidating. Whenever you change what drives a frame, log both
  ends and divide. The ratio is the finding; the fps number cannot express it.
  **And the follow-up is the sharper lesson: "provably stood down" was itself an inference, and it
  was wrong.** The child's timer was gated on a lease, one `frame clock -> HOST` line was taken as
  proof the gate held, and nobody checked for a LATER transition. There were three. Tagging every
  call site closed the accounting to the unit — 1856 host + 1846 timer = 3702 — and the answer was
  a bug in the gate, not a third party. A transition log only proves what it logged; grep for
  ALL of them, and make counters that must SUM to something you already measure.
- **A WEDGED PLUGIN LOOKS EXACTLY LIKE A PACING REGRESSION.** Cycling `inject url` between pages
  repeatedly can leave the child in a crash loop; the daemon then logs `flashRect remove id=N
  (gone)`, painting stops entirely, and the child keeps logging `palm draw` the whole time so the
  plugin looks alive. This produced a confident, WRONG attribution to a tick-rate change on
  2026-08-10 — the identical build measured healthy after a clean restart. Before believing any
  frame number that follows repeated navigation: `stop jihad; pkill -f jihad-browserserver;
  pkill -f plugin-container; start jihad; killall LunaSysMgr`.
- **AN EMPTY LOG PASSES EVERY CHECK.** `grep -c 'inject: no page' $LOG` returns 0 on a log that
  is empty because the daemon wrote NOTHING, which reads as "card fine" and is the opposite of
  the truth. This produced two clean-looking but entirely void runs on 2026-08-10. Assert the log
  GREW (`wc -c`) before believing any grep over it — a zero-byte log means the run never
  happened, not that it went well. Same class as the "card connected" trap below, one level down.
- **NEVER write `/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`.** Switching away from
  Palm's `ondemandtcl` deadlocks the cpufreq policy lock in the kernel; every later reader of any
  cpufreq node blocks in unkillable **D state** — the daemon, `powerlog`, and your own restore
  script, so the governor is never put back and the daemon stops painting. Only a reboot clears
  it, and **a clean `reboot` will NOT run**, because init waits on those D-state tasks: it takes
  `sync; sync; reboot -f`. The governor's own TUNABLES (`cpufreq/ondemandtcl/*`) are safe to
  write and return instantly — that is what patch `0027` uses. Cost a device reboot 2026-08-10.
- **Regenerating `/etc/event.d/<job>` from `packaging/gen-variant-scripts.sh` and pushing it
  verbatim KILLS THE INJECT CHANNEL.** The shipping template does not set `JIHAD_INJECT=1`; the
  device's copy carries it as a local addition. Lose it and every inject silently does nothing,
  the daemon logs nothing, and — see the first trap — every subsequent check "passes". Re-add
  `JIHAD_INJECT=1` to the `exec env` line whenever you reinstall the job, and remember the running
  daemon keeps its OLD environment until you `stop`/`start` it.
- **ASSERT THE CARD IS CONNECTED before believing any measurement.** If it disconnected, the
  daemon logs `inject: no page` and every downstream number — container present? draw count?
  thread count? — reads as a dramatic failure that is really "no page". This invalidated three
  of my own measurements and twice produced a wrong "audio kills the plugin" conclusion. Inject
  a known page and grep for `inject: no page` first.
- `novacom run` drops output arriving after host stdin hits EOF — pipe `sleep N |` into every
  run whose reply matters.
- `novacom run file://bin/sh -- -c '…'` silently drops the `-c` string. Stage a script with
  `novacom put` and run that.
- There is **no `timeout` binary** on the device. Background the process and `kill` it.
- `strace` DOES exist (`/usr/bin/strace`) and settled two problems in one run each.
- A screenshot captures the FOREGROUND card. The double-launch dance creates a second card, so a
  screenshot can silently be of the wrong one.
- The first card launch after `killall LunaSysMgr` does not navigate — launch twice, or use the
  inject channel (`JIHAD_INJECT=1`, `<state>/inject.cmd`), which is better.
- `/tmp` is cleared by a reboot; `/etc` comes back read-only (`mount -o remount,rw /`).

- **NEVER regenerate the YAP interface from `../ref-BrowserServer/CodeGen` — it would break every
  keystroke.** Established 2026-08-10 by building `YapCodeGen.cpp` against Qt 5.15 and RUNNING it,
  not by reading. The upstream `.defs` is stale against its **own** upstream output: it declares
  `KeyDown`/`KeyUp` (0x1008/0x1009) as two `uint16_t`, while both upstream's generated
  `BrowserServerBase.cpp` and ours read a third `int32 chr` — upstream hand-edited its generated
  file and never updated the `.defs` (the tell is the six-space indent on those lines inside a
  tab-indented generated file). Regenerating from it silently rewrites two LIVE commands from 12
  wire bytes to 4. Note this is a bigger break than the missing 0x1600, and the obvious fix —
  "just add our command to the upstream `.defs`" — would have shipped it.
  **Use the corrected in-tree copy: `render/browserserver/CodeGen/BrowserYapCommandMessages.defs`.**
  `build/webos-oe/check-yap-contract.sh` now runs inside `build-daemon-arm.sh` and fails the build
  on any drift (verified three ways: passes here, fails with 10 named defects on a real upstream
  regeneration, fails with exactly one on a regeneration from the corrected `.defs` — that one
  being the hand edit no `.defs` can express, since the generator emits every async command as
  `= 0;` and 0x1600 needs an empty default body to stay optional).
  **TRAP INSIDE THE TRAP:** a `.defs` line beginning with a SPACE hangs `YapCodeGen` forever —
  `ignoreLine()` scans for a non-space without advancing its pointer.

**Cross-ABI (bundled glibc 2.23 vs device glibc 2.8)** — see cavekit-device-build.md **R9**
- glibc 2.21 changed `sem_t`'s value word to `(tokens << 1) | has-waiters`. A named semaphore
  created by a 2.8 process reads to us as "0 tokens, waiters pending" and blocks FOREVER. This
  took Flash out entirely after one reboot and looked exactly like a code regression on
  untouched binaries — **it is a per-boot race, not a bug in your change.** Contained for
  `/PmLogLib` by `render/goanna/JihadPmLogSem.c` (patch 0025).
- `sem.browserserver.*` are created by the ADAPTER and nothing ever waits on them. The trap is
  ARMED, not sprung: adding a daemon-side `sem_wait` springs it, and `nwaiters` sits at a
  different offset too (8 in the 2.8 family, 12 in 2.23 — measured). Use SysV `semop` (a kernel
  object, no userspace layout) if a blocking handshake is ever needed. It does NOT fire on an
  EMPTY semaphore (raw 0 and `0 << 1 | 0` are the same word), so a first `sem_wait` would look
  fine right up until the adapter posts.
- **THIS WHOLE CLASS IS HOST-TESTABLE — stop spending device sessions on it.** Both toolchains are
  in the tree and `qemu-arm` runs their output on the build host: static-link one probe with the
  PDK gcc (sysroot glibc 2.5, the pre-2.21 side) and one with the crosstool gcc (2.23), run both,
  and the DIFFERENCE is the ABI. The `sem_t` hang above reproduces this way in about a minute.
  The PDK sysroot is 2.5 and the device is 2.8, so it is a pre-2.21 PROXY, not the device.
- **`pthread_mutex_t` PROCESS_SHARED is a SECOND armed trap, and the layouts being IDENTICAL is
  what makes it dangerous.** 24 bytes and the same fields in both, so every size and marker check
  passes — but glibc 2.23 sets `__kind = 128` (PTHREAD_MUTEX_PSHARED_BIT) and takes the futex
  private/shared flag from that bit, while the pre-2.21 side writes `__kind = 0` and returns
  EINVAL on a mutex whose `__kind` is 128. So one direction hangs under CONTENTION ONLY, and the
  other turns every `lock()` into a silent no-op. `ProcessMutex` is adapter-only and instantiated
  by nothing today; see cavekit-device-build.md R9 and the comment in `ProcessMutex.cpp`.
- `pthread_cond_t` still matches (48 bytes, same fields) — but **2.23 is the LAST glibc where that
  is true**; the condvar was rewritten in 2.25, and 2.24/2.25 tarballs are already in the
  toolchain source dir. `pthread_rwlock_t` ALREADY differs at 2.23 (same size, `__flags` narrowed
  and a new `__shared` byte added), which is the standing proof that `sizeof` does not clear a
  struct.
- The framebuffer header IS clean: `BrowserOffscreenInfo` is 32 bytes with identical offsets under
  both toolchains (plain int/double, so AAPCS fixes it and libc has no say). Do not re-check it.
- Prove each device library with a standalone probe run under BOTH loaders — the difference IS
  the diagnosis. See `plugin_mime_probe.c` and `alsa_probe.c`.

**Reverse-engineering libflashplayer.so**
- The code is **ARM, not Thumb**.
- String addresses are PIC: `add rX, r5, <negative literal>` with a base near 0x8ad010. Searching
  for an absolute address AND for `movw`/`movt` immediates both find ZERO references. Read the
  literal pool and add the base.
- Take PLT stub addresses from `objdump -d --section=.plt`, which labels them. A hand-rolled
  decode gave a wrong stub and made a BL scan report no callers.
- A function's entry can be one instruction BEFORE the `push` (the PIC `ldr rX,[pc,…]` comes
  first). Being off by 4 made a caller scan report zero callers and nearly sent the audio hunt
  down a "it must be a vtable" dead end.

**Build / deploy**
- `push-engine-update.sh` pushes libxul + daemon + goanna.js. It does **NOT** push
  `plugin-container` or the adapter — **nor the loose JS components.**
  `components/jihadInstallPrompt.js` and `components/jihadAboutPreferences.js` live in each
  variant's `deviceroot/hl/components/` and are refreshed only by a bundle/`.ipk` push, or by
  pushing the one file per variant (which is what was done 2026-08-10 for the R3 install-failure
  observer). Editing one and running `push-engine-update.sh` ships nothing.
- **The variant app ids on device use a HYPHEN, not a dot: `net.riverstonerelay.jihad-browser-mochi`
  and `…-mojo`.** `CLAUDE.md` writes the Mochi id with a dot and is WRONG against the device.
  Pushing to the dotted path fails with `bad or error response from other side: 'file open failed'`,
  and the `ls` that follows reads as "this variant does not ship that file" — I briefly concluded
  mochi and mojo had no `components/` directory at all before checking the parent directory.
  Confirm a path against `ls /media/cryptofs/apps/usr/palm/applications/` before believing an absence.
- The `.ipk` ships the **PDK** adapter (`build-adapter-pdk.sh` → `adapter-deps/build-pdk/`), NOT
  the crosstool one (`build-adapter-arm.sh` → `adapter-deps/build/`). Rebuilding only the latter
  ships an old adapter in the package while the device runs the new one.
- The adapter runs INSIDE LunaSysMgr. Deploy it by swapping
  `/usr/lib/jihad/<variant>/BrowserAdapterImpl.so` + `/usr/lib/BrowserPlugins/BrowserAdapterJihad*.so`
  (needs `mount -o remount,rw /`), then restart LunaSysMgr. Back up first —
  `/var/palm/jihad/adapter-backup/` holds the pre-2026-08-10 pair.
- A hand-launched daemon survives `stop jihad` and serves a stale libxul. "Nothing changed" is a
  lie until you `pkill -f jihad-browserserver`.
- `cp` onto a mapped binary fails with "Bad file descriptor" on cryptofs — stop the daemon and
  `rm -f` first, or push to `.new` and `mv`.

---

## Build / deploy loop

```bash
# libxul (incremental ~45 s) and the daemon
podman run --rm --userns=keep-id \
  -v "$PWD/third_party/uxp:/src/uxp" -v "$PWD:/jihad" \
  -v "$PWD/build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi:/tc" \
  -v "$PWD/build/webos-oe/arm-sysroot/root:/sysroot" \
  -v "$PWD/build/desktop:/cfg" -v "$PWD/build/webos-oe:/armcfg" \
  -v "$PWD/build/webos-oe/out-arm:/out" \
  localhost/jihad-goanna-build bash /armcfg/build-goanna-arm.sh build   # or build-daemon-arm.sh

build/webos-oe/push-engine-update.sh enyo mochi mojo   # md5-verified

bash build/webos-oe/build-adapter-arm.sh    # adapter for a live swap
bash build/webos-oe/build-adapter-pdk.sh    # adapter the .ipk ships  <-- do both
bash build/webos-oe/build-variant-ipk.sh    # all three .ipks

bash build/webos-oe/build-test-plugin-arm.sh   # NPAPI control plugin
bash build/webos-oe/build-uinput-kbd.sh        # synthetic keyboard (PDK; needs --sysroot)
```

**Only `enyo` has libxul with patch `0027`.** `push-engine-update.sh enyo mochi mojo` pushes the
SAME libxul to all three, so just run it with all three variants next time you push and they are
back in step. Until then, a frame-rate measurement taken on mochi or mojo will show the OLD,
governor-limited numbers and look like a regression that is not one.

Clean device run:

```sh
stop jihad; sleep 2; pkill -f jihad-browserserver; pkill -f plugin-container; sleep 2
: > /var/palm/jihad/enyo/daemon.log
start jihad; sleep 3
killall LunaSysMgr; sleep 32
luna-send -n 1 palm://com.palm.applicationManager/launch \
  '{"id":"net.riverstonerelay.jihad-browser","params":{"target":"file:///tmp/jihad-plain.html"}}'
# then drive it with the inject channel:
echo 'url file:///tmp/jihad-anim.html' > /var/palm/jihad/enyo/inject.cmd
```

Read the `paint pipeline:` / `frame gap ms min/avg/max` counter pair for frame-rate work — the
gap SPREAD is what reads as stutter, not the average.

### Inject commands, and the two added 2026-08-10

`url`, `text`, `find`, `rect`, `gettext`, `clickid`, `dblclickid`, `clickoff`, `touch`, `cookie`,
`addon`, `spotlight`, `freeze`, `thaw`, `popups`, `jsurl`, and now:

| command | what it is for |
|---|---|
| `setpref <b\|i\|s> <name> <value>` | write a pref, with an immediate read-back on the same log line |
| `getpref <b\|i\|s> <name>` | read one back — after a restart, which is the half that proves persistence |
| `anon <selector>` | the element's NATIVE ANONYMOUS CONTENT: did an XBL binding actually attach. `?` = no such element, `0` = element exists and bound NOTHING (the real negative), otherwise a count plus the first few node names and `anonid`s |

**Both were added because their absence had already cost device runs, and both replace a probe
route that DOES NOT WORK — do not go back to it.** `jsurl` executes with the system PRINCIPAL but
in a CONTENT scope, so `Components` is not exposed: a `Components.classes[…preferences-service…]`
probe returns success and silently does nothing, which reads as a negative result about the thing
under test rather than a broken probe. Likewise `document.getAnonymousNodes` throws
`not a function` there regardless of privilege. Both questions have to be answered in the daemon,
which is in-process with libxul and already holds `nsIPrefBranch` and `nsIDOMDocumentXBL`.

---

## Test assets (`render/goanna/test/`)

| asset | what it answers |
|---|---|
| `make-anim-swf.py` → `jihad-anim.swf` | does content MOVE (x-centroid = frame number) |
| `make-audio-swf.py` → `jihad-audio*.swf` | audio; PCM by default, `--mp3`, `--stereo` |
| `make-key-swf.py` → `jihad-key.swf` | real AVM1: GREEN until a key is handled, then RED |
| `alsa_probe.c` | can THIS runtime play sound (run under both loaders) |
| `plugin_mime_probe.c` | reproduce the daemon's plugin scan standalone |
| `audio_shim.c` | LD_PRELOAD: trace dlopen/dlsym, substitute `snd_pcm_open` |
| `npapi_test_plugin.c` | control plugin — proves defects are OURS, not Flash's |
| `uinput_kbd.c` | synthesises a HARDWARE keyboard (this device has none) for chrome-side key tests |
| `abi_probe.c`, `sem_dump.c`, `mtx_dump.c` | **cross-ABI, and they need NO DEVICE.** Build each with both in-tree toolchains (PDK gcc 4.3.3 / glibc 2.5 and the crosstool 9.4 / glibc 2.23), run both under `qemu-arm`, and the diff IS the ABI. Reproduced the PmLogLib `sem_t` hang on the build host in about a minute. See the cross-ABI section at the end of this file for the 2.5-is-a-proxy caveat |

Pages/assets staged in `/tmp` on the device (volatile — re-push after any reboot). The HTML
wrappers are trivial and are NOT in the repo; regenerate them as a `<embed
type="application/x-shockwave-flash" src="<swf>" wmode="opaque" width="320" height="240">`:

| on-device page | asset | what it is for |
|---|---|---|
| `jihad-plain.html` | none | the SWF-FREE page. Navigate here first when you need a plugin-container to spawn FRESH on the next navigate, and to release the `0027` CPU boost |
| `jihad-anim.html` | `jihad-anim.swf` | frame-rate work |
| `jihad-audio.html` | `jihad-audio.swf` | PCM square — the only asset confirmed CLEAN by ear |
| `jihad-audio-mp3.html` / `-44k.html` | mono 22 k / stereo 44.1 k MP3 | MP3 decode path |
| `jihad-audio-5s.html` | `jihad-audio-5s.swf` | 5 s sine MP3. LOOPED, so it clicks once per iteration by design — that is Flash restarting its decoder, not a defect |
| `jihad-audio-long.html` | `jihad-audio-long.swf` | **30 s single-shot MP3 (`--loops 1`). The ONLY asset that can answer a question about audio QUALITY**, because it has no loop point at all. Every other MP3 asset here clicks per iteration no matter how healthy the port is |
| `jihad-keyarb.html` | `jihad-key.swf` + 3000 px scroll region + `#sy` readout | keyboard arbitration: read scroll back with `gettext #sy` |
| `jihad-media.html` | `jt.webm` (VP8) + `jt.mp3` | **ENGINE media, nothing to do with Flash.** Reports `canPlayType`/`readyState`/`currentTime`/`error` into `#r`; read it back with `gettext #r`. Measured 2026-08-10: **VP8 plays**, H.264 unsupported, **MP3 refused outright** (`error.code=4`) because the ARM build compiles cubeb with NO backend. Regenerate the two media files with `ffmpeg -f lavfi -i sine=…` and `-f lavfi -i testsrc=… -c:v libvpx` |

SWF authoring traps — **every one of these has already produced a false negative in this tree,
and a bad asset is indistinguishable from a broken port:**
- **Shapes and sounds share ONE character dictionary.** `DefineSound` may not reuse a
  `DefineShape` ID. The player keeps the first definition and drops the second, `StartSound`
  resolves to the wrong character, and the sound is silently never played. This cost a whole
  session chasing a nonexistent player-side audio gate. Fixed 2026-08-10 (`SOUND_ID = 2`).
- `0x00020000` is ClipEvent**KeyPress** (and carries a trailing KeyCode byte) — KeyDown is
  `0x00000040`; getting it wrong makes the SWF draw NOTHING rather than ignore keys.
- A sprite with no `Stop` on frame 1 loops, which looks exactly like a key response and is not.
- For MP3 event sounds the SoundData begins with a **SeekSamples SI16**, not the frame stream.

Parse an asset back out before believing a negative result from it. A 30-line tag walk that
prints each tag code and character ID is enough, and it is what caught the ID collision.

---

## Device state as left

- **2026-08-10, later session:** the enyo daemon and libxul now also carry the frame-pacing work
  (`PumpReady`, the 1:1 seq gate, patch `0028`, the tick/gap counters). `/etc/event.d/jihad`
  carries `JIHAD_TICK_MS=16` and `JIHAD_PLUGIN_FRAME_MS=33` as device-local additions ALONGSIDE
  `JIHAD_INJECT=1` — all three are lost if the job is regenerated from the template. The device
  was last left on a CLEAN restart running `/tmp/jihad-anim.html`, measured healthy: `done=61
  palm draw=63 deferred=0`, gap mode 32-39 ms.
- **2026-08-10, pull session: ALL THREE VARIANTS ARE IN STEP** and were re-pushed together at the
  end of the session, so every one carries the same libxul and daemon with patches `0027`, `0028`
  AND `0029`. The previous enyo-ahead-of-mochi/mojo skew is gone, so a frame measurement on any
  variant is now comparable. (Read the current md5s off the device rather than trusting a number
  written here — this file has carried a stale one before. `push-engine-update.sh` prints them.)
- **The libxul on the device carries the pacing INSTRUMENTATION as well as the fix**, and it is
  what makes the open question answerable: the child logs `show req: setwin=… hostinval=…
  plugininval=… retry=… bgupd=… bgtail=… bgdestroy=…` next to `palm draw`, on the same 2 s
  boundary. `hostinval` is our pull, `plugininval` is the plugin invalidating itself. Those two
  counters are the whole of open work item 2 — read them before changing anything about pacing.
  Adapter is unchanged — still the PDK build matching the `.ipk` (md5
  `0410a348042af58a7e711b495db69833`).
- **THE DEVICE IS DELIBERATELY LEFT IN THE "FLASH ALONE" CONFIGURATION — this is not an accident
  and it is not the shipping default.** `/etc/event.d/jihad` carries `JIHAD_PLUGIN_PULL=0` and
  `JIHAD_PLUGIN_SELF_DRIVE=0`, so BOTH host frame drivers are off and the plugin's own
  `NPN_InvalidateRect` is the only frame source (verified: `hostinval` is exactly 0).
  **It is left this way so a human can LOOK at it**, because the eye is the only instrument that
  has ever detected this defect: Flash alone measures SLOWER (24-27 vs 30-33 fps) but perfectly
  unbunched (**zero** gaps under 16 ms, against 1-5 with the pull on). If evenly-spaced 25 fps
  reads better than bunched 32 fps, the host frame clock should be deleted — see the R7 frame-rate
  criterion for the full three-way table and the recommendation.
  **Revert to the shipped default in one line** (then `stop jihad; start jihad`):
  ```sh
  mount -o remount,rw / && sed -i 's/ JIHAD_PLUGIN_PULL=0 JIHAD_PLUGIN_SELF_DRIVE=0//' /etc/event.d/jihad
  ```
- `/etc/event.d/jihad` is back at **`JIHAD_TICK_MS=16`**. It was moved to 8 for one A/B during this
  session and moved back; 8 measured WORSE (see the pull section). The edit was a `sed` on that one
  token, never a regeneration from the template, so `JIHAD_INJECT=1` survived — verified after each
  edit. If you A/B it again, do it the same way and re-check `JIHAD_INJECT=1` afterwards.
- `/etc/event.d/jihad` is the normal job (`JIHAD_INJECT=1`, no `LD_PRELOAD`, no kill switches) and
  now also carries the `post-stop` governor restore. **TRAP: the SHIPPING template in
  `packaging/gen-variant-scripts.sh` does NOT set `JIHAD_INJECT=1` — that is a device-local
  addition.** Regenerating the job and pushing it verbatim silently kills the inject channel, and
  every subsequent test then "passes" against an empty log. Re-add it when you reinstall the job.
- db8 kind `com.palm.app.flashplayer.prefs:1` registered (owner `com.palm.flashgraphics`) — this
  is a deliberate addition that fixes Flash's only reported error. Remove with `delKind` if
  unwanted.
- `/var/palm/jihad/adapter-backup/` holds the pre-2026-08-10 adapter pair.
- Flash's `oem.cfg` was temporarily modified and has been RESTORED to original.
- **The device was REBOOTED on 2026-08-10** (forced: `sync; sync; reboot -f`, because a clean
  `reboot` hangs on D-state tasks — see the cpufreq trap). It came back healthy on `ondemandtcl`
  and all three daemons auto-started. Governor tunables are at STOCK (`up_threshold=95`,
  `sampling_rate=200000`); the daemon takes them to 40/50000 only while a plugin is instantiated.
- Test pages/SWFs re-pushed to `/tmp` after that reboot and ALL CURRENT — `jihad-anim`,
  `jihad-audio`, `jihad-audio-mp3`, `jihad-audio-44k`, `jihad-audio-5s`, `jihad-audio-sine`,
  `jihad-key`, plus `jihad-plain.html` and `jihad-keyarb.html`. `/tmp` is volatile: re-push after
  any reboot (see the Test assets table for what each one is for).
- The synthetic keyboard is NOT running — it was destroyed (`echo quit > /tmp/uinput.cmd`) and the
  process is gone. `/tmp/uinput_kbd` may or may not still be there; rebuild with
  `build/webos-oe/build-uinput-kbd.sh`.
- `libflashplayer.so` in the enyo variant's `profile/plugins/` is **stock**, md5
  `14c20ba9fc2183a93648c70488e0ec8e`, byte-identical to
  `/media/cryptofs/apps/usr/lib/BrowserServerPlugins/libflashplayer.so`, which is the restore
  source. Two patched builds were run there during the audio hunt and both have been removed.

---

## Uncommitted work in the tree (2026-08-10)

Nothing is committed. What this session added, and why, so it is not mistaken for cruft:

| path | what it is |
|---|---|
| `build/desktop/patches/0027-npapi-plugin-cpu-boost.patch` | **the frame-rate fix.** Holds a CPU-governor boost for the life of a plugin instance. Applied to `third_party/uxp` already, and the enyo device libxul is built from it |
| `packaging/gen-variant-scripts.sh` | adds the `post-stop` governor restore to the upstart template, so a crashed daemon cannot strand the tuning |
| `render/goanna/test/uinput_kbd.c` + `build/webos-oe/build-uinput-kbd.sh` | the synthetic hardware keyboard, for the open keyboard-arbitration criterion |
| `render/goanna/test/make-audio-swf.py` | two real asset bugs fixed: duplicate character ID, and `SeekSamples` 0 → 1105 |
| `render/goanna/test/audio_shim.c` | header rewritten — its old "measured" conclusion was wrong |
| `docs/PICKUP.md`, `context/kits/cavekit-addons-extensions.md` | this handoff and R7 |
| `build/desktop/patches/0028-npapi-plugin-frame-seq.patch` | **the 1:1 frame fix.** Exports `jihad_plugin_frame_seq()` from `PluginInstanceParent::RecvShow` so the daemon can tell a NEW plugin frame from a repeat. Applied to `third_party/uxp` already; the enyo device libxul is built from it. Dry-run verified against a reconstructed pristine copy, since several other patches also touch that file |
| `render/goanna/GoannaRenderPage.{h,cpp}` | `PumpReady()` (drain-and-return, the tick's pump) beside `PumpFor()` (spend-the-budget, what the tests need); weak `jihad_plugin_frame_seq` binding + `PluginFrameSeq()` |
| `render/goanna/BrowserPageGoanna.{h,cpp}` | the publish policy: 1:1 on the frame seq with a stale-damage fallback, the deadline pacer as the no-seq fallback, and the frame-gap HISTOGRAM that made the defect visible at all |
| `render/browserserver/JihadBrowserServer.cpp` | tick cadence counters (period vs work, split pump/paint) on a monotonic clock; the tick pumps with `drainOnly` |
| `render/browserserver/Main.cpp` | tick period is `JIHAD_TICK_MS` (4-64), so cadence can be A/B'd without a rebuild |
| `render/goanna/test/dump-swf.py` | walks a SWF back out and FAILS it on the three asset bugs that have each impersonated a port defect here (duplicate character ID, SeekSamples, SyncNoMultiple retrigger) |
| `render/goanna/test/make-audio-assets.sh` | regenerates every audio asset from scratch and verifies each one. The MP3 streams behind the checked-in SWFs previously existed only in `/tmp`, so the repo could not reproduce its own test assets |
| `build/webos-oe/stage-test-pages.sh` | pushes the SWFs + generates the HTML wrappers into the device `/tmp`, md5-verified. Replaces hand-making them after every reboot |
| `README.md`, `docs/*.md` | **de-duplicated against the kits** (user instruction 2026-08-10). The kits own requirements and per-criterion STATUS; docs own procedure. 233 lines of restated status removed, and with them three claims that had already gone stale and wrong: "Flash audio does not work", DEVICE-BUILD's "no device sysroot, no OE tree … and no TouchPad", and TOOLCHAIN's three-way toolchain choice presented as still open. README is now user-facing and positions against isis and Atlas |
| `context/plans/build-site.md` + `plan-overview.md` | regenerated from the kits: **56 tasks, 6 tiers, covering the 41 still-open criteria at 100%**, with the 8 hardware/human-gated ones quarantined in a final tier. The 2026-06-30 site is kept as `build-site-2026-06-30.archived.md` |
| `build/desktop/patches/0029-npapi-plugin-pull-frame.patch` | **the PULL.** New async parent→child `JihadRequestFrame`; the child answers with `JihadPalmRepaintTick()` and stands its own timer down on a 150 ms lease, logging every transition; `PluginInstanceParent` exports `jihad_plugin_request_frame()` over a file-static live-instance list. Applied to `third_party/uxp` already and live on all three variants. **Its own header records that it does NOT fix the pacing and why** — read it before building on it. Verified by reconstructing the pre-0029 baseline (reverse-applying only this session's edits), dry-running the patch onto it, and confirming it reproduces the working tree byte-for-byte |
| `render/goanna/BrowserPageGoanna.{h,cpp}` (pull) | `requestNextPluginFrame()`: the whole-tick request grid, the one-outstanding backpressure gate with a 250 ms timeout, and `jihadTickPeriodMs()` — the single definition of the tick period, which `Main.cpp` now calls instead of reading the env itself. Deliberately NOT gated on `mEmittedFlashRects`: that map is what EMPTIES when the plugin wedges, so gating there turns a recoverable wedge into a permanent one |
| `render/browserserver/JihadBrowserServer.cpp` (pull) | calls `requestNextPluginFrame()` once per tick — NOT from `maybePaint()`, which `returnBuffer()` also reaches off-tick, which would put requests off the grid they exist to stay on |
| `build/desktop/patches/make-0029.sh` + `.baseline-pre-0029/` | **the fix for the patch-authoring trap this file has warned about three times.** Reconstructs the correct baseline — `git show HEAD:` plus every EARLIER patch that touches the same four files (`0008 0017 0020 0022 0024 0026 0028`) — and diffs the live tree against it, so a new patch cannot silently fold in its predecessors or come out empty. `--rebuild-baseline` regenerates the dir. Generalise it before authoring `0030`; the file list and the `EARLIER` list are the only per-patch parts |

`third_party/uxp/dom/plugins/ipc/PluginModuleParent.cpp` is modified in place (that is how patches
work here — the build applies them to the shared tree, idempotently, with a dry-run gate). If you
regenerate `0027`, diff against a pristine copy of that file, not against the working tree.

---

## If you are picking this up cold, do this first

1. Read the Flash status table above, then the two open items at the top of **Open work**.
2. `novacom -l` — confirm `topaz-linux`. Re-push `/tmp` assets if the device rebooted.
3. The cheapest real progress available is **counting the third draw driver** (open work item 2):
   two 2 s counters, one on `PluginInstanceParent::SendUpdateBackground` and one on each
   `AsyncShowPluginFrame` call site, then one clean-restart run. Every pacing conclusion in this
   document rests on not knowing that number.
   **Audio is CLOSED — do not start there.** An earlier revision of this list sent you to an
   "MP3 static" question that the audio section above had already resolved at parity with stock;
   the contradiction cost a session start. If two parts of this file disagree, the dated
   subsection wins over this list.
4. Keyboard arbitration is NOT blocked on the browser — it is blocked on getting a key out of
   `hidd` into the card. Start at `/usr/lib/libhidinputdev.so`, not at the adapter.
5. Whatever you measure: assert the daemon log GREW, and assert the card is connected, BEFORE
   believing any number. Both failure modes look exactly like success.

---

## The cross-ABI class is now HOST-TESTABLE — do not wait for the device (2026-08-10)

`render/goanna/test/{abi_probe,sem_dump,mtx_dump}.c`, promoted out of a scratch dir because this
is the single most reusable thing found this session. Both toolchains are already in the tree and
`qemu-arm` runs their output on the build host:

- `build/webos-oe/pdk/opt/PalmPDK/arm-gcc` — gcc 4.3.3, sysroot glibc **2.5**
- `build/webos-oe/toolchain/.../arm-webos-linux-gnueabi` — gcc 9.4, glibc **2.23**

Static-link the same probe with each, run both under `qemu-arm`, and **the diff IS the ABI.**
This reproduced the PmLogLib `sem_t` hang off-device in about a minute — a bug that previously
cost a device session to find.

**CAVEAT, stated because it matters: the PDK sysroot is glibc 2.5 and the device is 2.8.** Both
are pre-2.21, so it is the right side of every cliff that matters here, but 2.5 is a PROXY and a
result that hinges on 2.5-vs-2.8 rather than pre-vs-post-2.21 is not proven by this method.
