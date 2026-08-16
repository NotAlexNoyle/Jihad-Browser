---
created: "2026-06-30"
last_edited: "2026-08-16"
---

# Cavekit Overview

## Project
Jihad Browser — port the UXP/Goanna web engine into the isis-browser webOS 3
(HP TouchPad) shell, replacing QtWebKit while keeping the
BrowserAdapter↔BrowserServer YAP IPC command/message contract byte-identical.
Phase 1 brings the engine up on desktop x86_64; Phase 2 cross-compiles for the
device. Jihad ships **self-contained, coexisting with the stock browser** — its
own MIME/adapter/daemon/upstart job, nothing system-level replaced (Atlas model;
see `jihad-self-contained-arch.md`). The adapter carries a two-line rebrand (MIME
string + YAP server name) for coexistence; the YAP command/message interface it
speaks is unchanged.

**Three independent front-ends (user decision 2026-07-31).** Enyo 1.0 (`app/`), Enyo 2 + Mochi
(`app-mochi/`), and Mojo (`app-mojo/`) each ship as a complete, standalone browser with its own
MIME / adapter shim + impl / YAP name / socket / upstart job / daemon process — installing or
removing one cannot affect another (cavekit-device-build.md **R7**). Each package runs its engine
in place from its own cryptofs `deviceroot` and writes nothing to the user's `/media/internal`
storage (**R8**).

Grounding: `context/refs/refs-overview.md`, `docs/IPC-CONTRACT.md`,
`render/goanna/PORT-MAP.md`.

## Domain Index
Status legend: ✅ complete · 🟢 mostly (device/edge items remain) · 🟡 partial · ⬜ not started.

> **Flash reality check (2026-08-10, end of day).** Adobe Flash renders animated content on the
> TouchPad, takes mouse and keyboard input, and survives crashes, spotlight events and
> freeze/thaw. Against the user requirement *"smooth fps, keyboard and audio, so Flash games are
> a first-class experience"* (cavekit-addons-extensions.md **R7**, three criteria added for it):
>
> - **fps — the RATE is done; the PACING is NOT (corrected 2026-08-10).** Everything below is
>   still true and none of it is withdrawn, but it measures the wrong thing for the word the user
>   used. The average was 35-42 fps for a 30 fps source at the exact time the animation looked
>   worst; a gap HISTOGRAM shows why — frames were landing at random instants from under 16 ms to
>   55 ms. Patch `0028` (1:1 delivery) and `0029` (a real PULL, so the daemon asks for each frame
>   instead of sampling) are both live on all three variants and the histogram is still not
>   concentrated. The open question is counted, not guessed: **3988 plugin draws against 3072 host
>   requests** — and tagging every draw call site then showed the gap was NOT a third party but
>   our own host-drive lease flapping (1856 host requests + 1846 child-timer invalidates = 3702,
>   exactly), because the daemon's re-ask timeout was longer than the child's lease. See
>   cavekit-addons-extensions.md R7, whose
>   frame-rate criterion was REOPENED from `[x]` to `[~]` for exactly this reason. Two ceilings
>   were removed and they were real: a hard `kJihadRepaintEveryTicks = 4` that asked the
>   plugin to repaint at ~15 Hz for 30 fps content, and the two-buffer handoff that made every
>   frame cost an adapter round trip. A THIRD shared buffer (`asyncCmdSetExtraBuffer`, YAP
>   0x1600 — the one additive change to an otherwise frozen contract, cavekit-ipc-contract.md R1)
>   took `deferred` from 77 to **0** with `wanted == done`; composite **27.1-32.2 fps**, frame gap
>   avg 31-36 / **max 127 ms → 52-57 ms**. Verified against the build the `.ipk` actually ships.
>   **Update 2026-08-16 — a stale-frame source in the SAME paint path was found + fixed.** The
>   damage-only repaint rotates three buffers but applied only the damage since the last GLOBAL
>   paint to a buffer last painted several frames earlier, so each buffer kept a GHOST of a moving
>   element at its own stale position — surfaced as a jittery, skip-back media scrubber (fixed +
>   device-confirmed, cavekit-gre-widgets.md R1) and, because the same path composites plugin
>   frames, is expected to steady Flash too (per-buffer damage accumulation, `BrowserPageGoanna`
>   `BufFrame.dmg*`, commit `36ddb80a`). Flash was not re-measured, so the histogram numbers above
>   stand as the last Flash-specific figures; the ceiling remains the ~25–30fps software paint.
> - **keyboard — DONE at the content level.** A SWF's own AVM1 `onClipEvent(keyDown)` runs on a
>   keypress (green 11768 → red 11750). The adapter's key arbitration is fixed and deployed (one
>   fix ported from Atlas). Still unverified: that the CARD does not also consume the key —
>   client-side, needs a human or a chrome-side probe (cavekit-input-bridging.md R7).
> - **audio — DONE, at parity with stock (corrected 2026-08-10).** The paragraph that stood here
>   was wrong in every part and is deleted rather than softened: `0x70ee0`/`0x73020` are the
>   hardware **H.264 video** decoder (OMX), not audio; the `+24 == 7` gate was stated INVERTED
>   (it does equal 7); and nopping both candidate gates on-device changed nothing. **The defect
>   was in the TEST ASSET.** `make-audio-swf.py` emitted `DefineShape` and `DefineSound` with the
>   same character ID, and one character dictionary is shared by both — so the sound was silently
>   dropped by any player, which is why all three assets "behaved identically". With the asset
>   fixed and STOCK unpatched `libflashplayer.so`, the full ALSA bring-up appears and the tone is
>   audible from the speaker. The residual per-loop click is Flash restarting its MP3 decoder and
>   the STOCK browser does it identically. **Do not plan audio work off this file or off a
>   looping asset** — only `jihad-audio-long.swf` (30 s, `--loops 1`) can answer a quality
>   question.
>
> Two requirements came out of the same work and are met: cavekit-offscreen-rendering.md **R8**
> (the damage-only repaint may never publish a frame it cannot prove — four real defects, all
> fixed) and cavekit-device-build.md **R9** (the bundled-glibc-2.23 vs device-glibc-2.8 ABI
> boundary, which took Flash out entirely after one reboot and looked like a code regression on
> untouched binaries — it is a per-boot race). **R9 is the one to read first when something on
> this device breaks "for no reason."**

> **Device reality check (2026-08-03, second session).** **All three variants (Enyo, Mochi, Mojo)
> are live on hardware** — each card paints through its own daemon on its own socket, cold boot
> auto-starts all three, and `device-independence-test.sh check` passes 24/24. Feature work is no
> longer Enyo-only: `<select>` popups and the start page are now verified on **all three**, and the
> Mojo shell gained its chrome actions (new card / history / share).
>
> **The card JS dev loop is RESTORED** — `build/webos-oe/push-card-js.sh` is the tool, and it
> proves each reload by a per-push stamp reaching the device log. Two independent causes had
> broken it: **`novacom run` discards output that arrives after the host's stdin hits EOF** (hold
> it open — `sleep 4 | novacom run …`; this masqueraded as luna-send "blackouts", dead
> `applicationManager/running` queries, and "`enyo.log` stopped reaching `palm-log`"), and the
> **WebAppMgr in-process JS cache really does serve a stale build** after a close+relaunch (a
> LunaSysMgr restart per cycle busts it, and is the script's default).
>
> Cross-cutting defects RESOLVED: scroll pan headroom (user signed off), long-press `contextmenu`,
> below-the-fold input landing, and the **`<select>` popup on all three variants** — the "empty
> popup" was a JSON field-name mismatch, not a rendering problem (the stock framework consumes the
> event itself and expects the isis `items[].text/isEnabled` shape; Mojo needed no app code at
> all). The **`<menupopup>` track then closed too**: engine popups are composited over the frame,
> taps/moves route into them, the row under the finger highlights, drag-and-release selects, and a
> tap outside rolls up — the `about:addons` tools menu opens, reads correctly and its items act.
> **XPI web install now works end to end, on device** (the first extension this browser has
> installed), which required fixing two engine assumptions that a browser chrome exists above the
> content, and then implementing the **DialogSink the daemon never had** — a claim the kits carried
> on the strength of a test that supplied its own sink. Still open on these tracks: `<optgroup>`
> header rows in `<select>` lists, and the SSL accept-and-reload branch. See
> `impl-select-popup-2026-08-03.md`, `impl-menupopup-2026-08-02.md`,
> `impl-scroll-overscan-2026-08-02.md`, `impl-addons-icons-open.md`.
>
> **Platform constraint found the hard way:** the webOS 3 **card** WebKit (~534.x) silently
> ignores unprefixed `box-sizing` and modern flexbox, so all card-chrome CSS must carry
> `-webkit-` prefixes (it made the Mojo toolbar 784 px wide on a 768 px screen). Pages rendered by
> our own Goanna engine are modern and unaffected.
Verified on desktop x86_64 AND cross-built + rendering on the HP TouchPad unless noted.

| Domain | Cavekit File | Requirements | Status | Description |
|--------|--------------|--------------|--------|-------------|
| UI Shell (Enyo) | cavekit-ui-shell.md | 6 | 🟢 R1–R3 ✓; R4 3/4 ACs device-verified 2026-07-20 (launch, address→openUrl, back/forward/reload); findInPage WORKS as of patch 0015 (the long-recorded 'selection controller' cause was a misdiagnosis). `<select>` popup ✓; start page follows VKB/orientation and carries the shared logo/title/hint block; app renamed "Jihad Enyo" | Forked/rebranded Enyo-1.0 app (`app/`) using the unchanged adapter contract |
| Mochi UI Variant | cavekit-mochi-ui.md | 7 | 🟢 R1/R3/R5 ✓; card LIVE on device (loads + paints through its own daemon); **`<select>` popup device-verified 2026-08-03** (own overlay list, pick applied); start page matches the others; R2/R4 remaining on-device feature checks pending; renamed "Jihad Mochi" | Second front-end on Enyo-2/Mochi (`app-mochi/`), same contract, separate .ipk |
| Mojo UI Variant | cavekit-mojo-ui.md | 7 | 🟢 R1–R3 ✓ device-verified, R5 ✓, R4 2/3 (Opal hardware-gated); **R6 (new) chrome actions ✓** — new card / history / share, title row dropped, toolbar overflow fixed (`-webkit-box-sizing`); `<select>` popup works with **no app code** (system framework handles it); renamed "Jihad Mojo" | Third front-end on the system Mojo framework (`app-mojo/`), same contract, own independent .ipk |
| IPC Contract Preservation | cavekit-ipc-contract.md | 5 | 🟢 R1–R3 ✓; R5 ✓ on-device (adapter drives daemon; +2-line coexistence rebrand); R4 device LunaService | Frozen YAP command/message interface, shmem framebuffer, daemon, LunaService |
| Engine Embedding & Build | cavekit-engine-embedding.md | 4 | ✅ 4/4 (+ ARM cross-build). **R2's "shuts down cleanly" was met in error until 2026-08-04** — `main()` never returned, so `XRE_TermEmbedding` never ran and the engine's deferred savers never flushed; SIGTERM is handled now, pages are destroyed before the runtime, and the never-run `~YapServer` teardown it exposed was itself broken | Out-of-tree Goanna build, embedding runtime, event-loop integration |
| Offscreen Rendering | cavekit-offscreen-rendering.md | 8 | 🟢 R1–R6 ✅ desktop + on-device (R6 rotation composite + R5 zoom device-confirmed 2026-07-27); scroll pan headroom FIXED (overscan region paint, ≤2048-row SGX cap; user signed off, Opus-reviewed). **R7 (new) engine popups ✅**: `<select>` solved card-side on all three variants; `<menupopup>` composited over the frame AND interactive (taps routed in, rollover highlight, drag-select, tap-outside rollup) — the old "popup is 0x0 and never shown" diagnosis was measured on a `<select>` (the combobox path) and was wrong. Follow-up owed: F7 header frame-seq (needs an adapter rebuild) | Headless render → shared buffer → paint protocol + geometry events + orientation-correct composite + engine-popup delivery |
| Input Bridging | cavekit-input-bridging.md | 8 | 🟢 R1 ✓ + **long-press `contextmenu` works on device** (the daemon `asyncCmdHitTest` gate was a stub; real hit-test round-trip added, user-confirmed), R4 ✓, R5 ✓ + **doc→viewport coord mapping fixed** (below-the-fold taps landed a screenful low); R2 VKB jank / R3 gestures on-device; **R6 XUL input partial** — text reaches XUL fields through the engine editor, but real DOM key events need a ~2-line PuppetWidget patch + a full libxul rebuild | webOS pointer/key/touch/gesture → DOM events |
| Navigation, Loading & Events | cavekit-navigation-events.md | 7 | 🟢 R1–R7 met (R6's link-clicked AC still [~] — on-device link-tap navigation confirmed 2026-07-27, the message-emission re-test is open) | Nav commands + load/location/title/history message stream |
| Add-ons & Extensions | cavekit-addons-extensions.md | 8 | 🟡 R1 ✓, R2 ✓ (`about:addons` renders on device; branding pkg + the Jihad logo on about pages), R8 ✅, R5 ✓, **R2 now complete** (enable/disable/remove driven by clicks on the real about:addons row controls, each surviving a SIGTERM restart) and **R6 device-verified** (an on-device install between two footprint snapshots: `/media/internal` byte-identical, nothing new outside the variant's own tree); **R3 XPI install WORKS** — device-verified: a page's `InstallTrigger.install()` raises the confirm on the card, accept installs, and `about:addons` lists the add-on. Needed patch 0013 (`amInstallTrigger` and `AddonManager` both assume a chrome `<browser>` above the content) plus the new daemon DialogSink; **R7 reframed — windowless NPAPI does not exist in a cairo-headless build and must be PORTED, not enabled**. **FLASH now RENDERS, TAKES INPUT and PLAYS SOUND on the TouchPad (2026-08-10), at ~30 fps composite** via patch `0027` (a CPU-governor boost held for the life of a plugin instance — see cavekit-device-build.md R10). Two Flash items remain OPEN and are written up on their criteria: **chrome-side keyboard arbitration is UNTESTABLE on this hardware** (no keyboard exists; a synthetic uinput one registers and is opened by `hidd` and LunaSysMgr but never dispatches), and **frame PACING** — the composite RATE is met via `0027`, but the gap histogram is still unconcentrated and the criterion was reopened to `[~]`; `0028` and `0029` are live and did not close it, and the counted open question is that only 3072 of 3988 plugin draws come from the host pull. **MP3 audio is CLOSED, at parity with stock (corrected 2026-08-10)** — the earlier 'occasional static' line here was written before the test assets were fixed, and the residual per-loop click is Flash restarting its decoder, which the stock browser does identically. Both 'silent audio' and 'slow frame rate' turned out NOT to be port defects — the first was a malformed test SWF, the second the CPU governor. The tools menu now opens and is operable (cavekit-offscreen-rendering.md R7) | `about:addons` + classic XPI + NPAPI plugin support |
| Browser Services | cavekit-browser-services.md | 5 | 🟢 R1–R3 ✓ (**R3 dialogs really work now** — the daemon had NO DialogSink, so every engine dialog silently took its default; `BrowserPageGoanna` is now that sink, over the frozen FIFO contract, **and one 60 s deadline sized for a person** — the original 5 s "has the card picked up" deadline could never be satisfied, because the card opens the reply pipe only when the user taps, so every dialog answered later than five seconds was silently defaulted); R4 partial; R5 SSL confirm wired with a reply path, accept-and-reload not yet end-to-end verified | Settings, cookies/cache, JS dialogs, downloads, TLS |
| Desktop Build & PoC Harness | cavekit-desktop-build.md | 4 | ✅ R1–R3; R4 [human-review] | Phase-1 x86_64 build + YAP test client + end-to-end gate |
| Device Build & Packaging | cavekit-device-build.md | 10 | 🟡 R1/R2 ✓; R3 build-produced (`.ipk`s + review items fixed); **R7 (per-variant independence) now DEVICE-VERIFIED 2026-08-03** — all three variants live, `device-independence-test.sh check` 24/24, cold-boot auto-start, own sockets, `/media/internal` clean; R8 ✓. Deploy routes: `push-variant.sh` (full payload) / `push-engine-update.sh` (fast libxul+daemon swap) / **`push-card-js.sh` (card JS/CSS/assets, stamp-proven)** — all novacom, all md5-verified. The supported Preware/WOQI `.ipk` install (R3/R4 "device-verified") is still user-gated; clean-clone reproducibility (#7/#8) + R5/R6 (memory budget, Opal) open. **R10 (new 2026-08-10): CPU frequency scaling.** `ondemandtcl` is touch-biased (`up_threshold=95`), so passive playback runs at 192 MHz of 1188 — it was the real ceiling on Flash's frame rate. Safe lever is the governor's TUNABLES; writing `scaling_governor` DEADLOCKS cpufreq into unkillable D state and needs `sync; reboot -f` | Phase-2 ARM cross-toolchain; self-contained packaging via bitbake; TouchPad + TouchPad Go |
| GRE Widget Bindings | cavekit-gre-widgets.md | 7 | 🟡 8/22 met, 2 partial. **R2 closed by decision**: date/time input is OFF at the pref with the reason recorded (device-verified degrading to `type=text`) — note the side effect that `valueAsDate` leaves the DOM. **R6's recorded diagnosis was WRONG and is corrected**: `FindNext` did not SIGSEGV on a missing selection controller, it hit `MOZ_CRASH` in `SubjectPrincipal()` (no JSContext on an embedder call); fixed in patch 0015 — though `find_test.cpp` still only PRINTS its result and asserts nothing, so it is not yet a gate. R1 (media/codecs) is the big open block | Widgets the engine already gives us, used or explicitly declined |
| In-Browser Preferences | cavekit-preferences-ui.md | 7 | 🟡 11/26 met, 4 partial. **`about:preferences` + `about:settings` render on device**, built as an `nsIAboutModule` JS component + `chrome://jihad-prefs/` package — no C++ change, no daemon rebuild. In-content HTML (Basilisk's shape, Pale Moon's panes): `<prefwindow>` is a chrome-WINDOW binding and there is no window here. R3 3/4 device-proven from the PROFILE. **Adversarial review 2026-08-06 pulled three marks back**: "no decorative toggles" (the UA row was reverted by EngineHost every start; `intl.accept_languages` is a LOCALIZED pref that displayed a chrome:// url; and the card re-applies popups/cookies/min-font-size from db8 at every launch, overwriting the page), "opens in all three variants" (only two were loaded), and it disproved the recorded reason the Luna route was abandoned — **the public role file is already installed by `gen-variant-scripts.sh`**. Does not exist on the desktop build at all. **R5 is the blocking item** | An engine-side settings page the three shells can share |
| Licensing & Branding | cavekit-licensing-branding.md | 5 | ✅ 5/5 | Apache+MPL headers, NOTICE, trademark stripping (cross-cut) |

Totals: **15 domains, 94 requirements, 348 acceptance criteria — 313 met, 11 partial, 24 open**
*(2026-08-06 session 2, all device-verified: preferences-ui R5's two criteria (the settings page's
edits reach the shells), R1 AC3 (the page opens in all three variants), R3 AC4 (per-variant profile
isolation, cross-proven both ways), R4 AC2/AC3 (normal navigation path, frozen adapter set
untouched; the two-surfaces relationship decided and written down) and R6 AC2 (the page's `<select>`
opens in all three). R6 AC1 closed on the user's own confirmation that scrolling works.
One criterion went the OTHER way: cavekit-addons-extensions.md R7's plugin-crash bound was
DOWNGRADED to partial, because the shipped configuration changed from in-process to
out-of-process plugins and the failure mode it documented is no longer the one that ships.)*
(COUNTED programmatically 2026-08-06, not estimated, after an adversarial review pass. The count
went DOWN from the 2026-08-05 figure — three criteria un-marked because their evidence did not
support "met", and two new criteria added for defects the review found. A count that only ever
rises is not measuring anything. The open list is still dominated by the two domains that arrived
2026-08-05: GRE widget bindings and in-browser preferences.)

(How to re-count, and please do before quoting these numbers: count `^### R` for requirements and
`^- \[x\]` / `^- \[~\]` / `^- \[ \]` for criteria across `context/kits/cavekit-*.md`, excluding this
file. These numbers have drifted from the files they summarise more than once — a line here once
said 276/215/21/40 against files that said otherwise.)
Closed this session on top of the earlier scroll / long-press / coord-mapping / R7-independence
work: add-ons R2 (enable, disable and remove driven from the real `about:addons` controls, each
surviving a restart) and R6 (extension storage, device-verified against a footprint snapshot);
and three daemon-lifecycle defects that had been invisible because the daemon never exited —
no SIGTERM handling (so nothing flushed), `XRE_TermEmbedding` running while pages were live, and
a `~YapServer` teardown that had never once executed and was broken. One user-reported defect
closed with them: dialogs were being answered for the user after five seconds.
The bulk of what remains sits in two kits — add-ons (8 open, 4 partial: the NPAPI port, the
Pale Moon/Basilisk cross-validation) and device-build (6 open, 8 partial: clean-clone
reproducibility, Opal, memory budget) — plus the XUL-zoom gesture. Every open item is
hardware-gated, a named debug lead, or an engine port.
**Current priorities — see `../impl/impl-NEXT-AGENT-START-HERE.md`
for the detailed queue:**
(1) **XUL zoom on `about:addons`** — see below; the one open user-reported defect. Note the
OTHER user-reported defect from that session, "clicking the button to install the add-on doesn't
do anything", is CLOSED and was not an add-on bug at all: the dialog deadline was denying the
answer before the user could give it (cavekit-browser-services.md R3);
    (user-reported as unreliable. The fit-zoom itself is correct — a fixed-width 980 px XUL
document in a 768 px window — and the popup work does not perturb contentSize; an injected zoom
is overwritten by the card re-asserting its own, so the actual pinch path needs a real gesture.
A trace of every zoom the card requests is deployed.)
(3) **chrome icon repaint latency** (addons R2 polish) — repaint-delivery latency, has a debug
lead. Re-checked 2026-08-04 and NOT reproduced on a fresh load (icons present at 3/8/16 s);
(4) **SSL accept-and-reload** (browser-services R5) — wired, not yet verified end to end; a cert
failure also raises an engine ALERT before the confirm, which wants its own look;
(5) `<select>` `<optgroup>` header rows (needs a daemon reply-index remap); F7 scroll header
frame-seq (needs an adapter rebuild); VKB jank (input R2) + gestures (input R3);
 device LunaService (IPC R4);
(6) clean-clone OE reproducibility (device-build #7/#8); TouchPad Go + memory budget (device-build R5/R6).
Full OE-review findings: `../impl/impl-review-findings-oe.md`.

### Working on the card UIs
`build/webos-oe/push-card-js.sh <enyo|mochi|mojo> <files…>` is the card-JS loop: it stamps the
push, md5-verifies it on both sides, closes the card by its real `processid`, restarts LunaSysMgr
(the WebAppMgr JS cache is real), relaunches, and **fails unless the new stamp appears in the
device log**. A screenshot is not proof a card is alive (fb1 holds the last painted frame) and an
on-disk md5 is not proof a card reloaded — the stamp is. Hold stdin open on every `novacom run`
whose output matters (`sleep 4 | novacom run …`).

### Driving the browser without a human
Most acceptance criteria here are reachable without anyone touching the screen, but only if you
know the four traps below. All of this is gated on `$JIHAD_INJECT`, which stays OFF unless set.

**`jsurl` does NOT execute in a chrome document.** It loads a `javascript:` URL with the system
principal, which works in content — and in `about:addons` it does nothing at all:
`LoadURIWithOptions` returns NS_OK, so the inject line prints **`ok=1`**, and no code runs, no
error is raised, nothing reaches the console. Measured 2026-08-04 three ways (setting
`document.title`, `Components.utils.reportError`, a `gViewController` call). Treat `ok=1` from
`jsurl` as "the load was issued", never as "the script ran".

**So click the real control.** `rect`/`clickid` resolve an element and click its own centre in
viewport space, which is also zoom- and scroll-independent. Three forms:
`<id>` (getElementById) · `sel:<css>` (querySelector — an about:addons row is
`richlistitem[value="<addon id>"]`) · `anon:<css>|<anonid>` (XBL **anonymous** content, which is
where a row's `enable-btn`/`disable-btn`/`remove-btn` live — no id reaches them). Clicking the
real button is also the more honest test: it exercises the same command path a finger does.

**`clickid` is not a tap.** `clickid`/`dblclickid`/`clickoff` resolve an element and call the
engine's `ClickAt` DIRECTLY, skipping the input QUEUE — so anything `pump()` does after a click
does not happen: no link-clicked message, no click-nav re-drive, no `<select>` popup emission.
That is fine for reaching a chrome control, and wrong for anything about content navigation
semantics, where `click <x> <y> <n>` (card coordinates, the path a finger takes) is the one to
use. Measured 2026-08-04: the same anchor tap emitted nothing and ended `NS_BINDING_ABORTED`
through `clickid`, and emitted `linkClicked` and navigated through `click`.

**A dialog must be answered the way a PERSON would.** The daemon blocks on a reply FIFO with a
60 s deadline; a harness that answers in 300 ms proves almost nothing about the human path, and
one that never answers takes the default. `JIHAD_DIALOG_MS` shortens the deadline for tests that
WANT the default fast. When a test's whole point is the human path, **answer late on purpose** —
that is what caught the 5 s deadline that had been silently denying every real dialog
(cavekit-browser-services.md R3).

**On device, the card creates no engine page until it navigates.** The supervised upstart job
deliberately does not set `JIHAD_INJECT`, so driving the device means stopping the job and
running an ad-hoc daemon with the same environment plus `JIHAD_INJECT=1` — then launching the
card **at a URL**: `palm-launch -p '{"target":"…"}' <appid>`. Launch it bare and every inject
command answers `inject: no page`, because the start page is card-side HTML and no
`BrowserPageGoanna` exists yet. Kill the ad-hoc daemon with **SIGTERM** (not `-9`) when you are
done, or the add-on database and prefs never flush, and restart the job. Its process name is
`ld-2.23.so`, so `killall jihad-browserserver` matches nothing.

### Milestone (2026-07-27): rotation and zoom work on the device
The portrait↔landscape render-break and the "things get cut off" zoom are both fixed and
confirmed on hardware (commit 8d7865c; `../impl/rotation-fix-2026-07-26.md`,
`../impl/zoom-fix-2026-07-27.md`, `../impl/impl-overview.md` 2026-07-27 "Rotation confirmed
working on device"). Rotation: the adapter now composites through the WebKit Piranha
**PGContext** (transform-aware) instead of a raw `dstBuffer` blit that LunaCE read at a fixed
~256px pitch — that pitch, not any daemon bug, was the portrait 3× shear. Zoom: `RenderDocument`'s
internal scale cancelled the engine zoom, so the daemon now pre-scales the gfxContext and renders
an absolute document rect (magnify + full-page pan in both axes, layout viewport untouched so
rotation stays safe). The same change made the daemon report the EFFECTIVE render scale, which
closed the long-open small-link tap miss (input-bridging R5, commit d4f0842). Closes offscreen
R6, closes device-build R4's composite AC, and supersedes the 2026-07-20 "LunaCE limitation, NOT
fixable" analysis.

### Milestone (2026-07-07): self-contained app renders real pages on the TouchPad
Re-architected from *replacing* the system browser to a **self-contained app that
coexists** with it (Atlas model: own MIME `application/x-jihad-browser` →
`BrowserAdapterJihad.so` → `/tmp/yapserver.jihad-browser` → upstart job `jihad`; the
app's WebView routed there by `app/source/JihadEngineOverride.js`). On-device the card
loads the Jihad adapter → the Jihad daemon, `example.com` + `slack.com`/HTTPS load and
render (fb1), load-completion fires (address-bar refresh glyph), and the stock browser
keeps working for all other apps. This meets IPC-Contract R5 (rebuilt adapter drives the
daemon through a full load+paint cycle) and advances Device-Build R4. Deploy gotchas
(reboot registers the plugin; `.ipk` reinstall busts the WebKit cache; content is on fb1)
in auto-memory `jihad-self-contained-arch.md` + `jihad-device-gotchas.md`.

### Milestone (2026-07-04): headless engine, no X/GTK, renders on the TouchPad
`MOZ_WIDGET_TOOLKIT=headless` — libxul links **zero** gtk/gdk/pango/cairo/X libs
(freetype+fontconfig only); ARM cross-build (29 M libxul) + GTK-free daemon → lean
device bundle (**28 .so vs 68**) → **on-device offscreen ROUND-TRIP PASS** (msgPainted
786432). This closes Engine-Embedding R2, Offscreen Rendering on-device, and
Device-Build R2, and advances Device-Build R4/R5. Detail in
`context/impl/impl-overview.md` + auto-memory `jihad-headless-toolkit.md`.

## Cross-Reference Map
| Domain A | Interacts With | Interaction Type |
|----------|----------------|------------------|
| UI Shell (Enyo) | IPC Contract | uses contract (client) |
| UI Shell (Enyo) | Navigation/Events | drives navigation, observes events |
| Mochi UI Variant | IPC Contract / UI Shell | same contract; parity with Enyo UI |
| Mochi UI Variant | Device Build | second .ipk packaged + on both TouchPad models |
| Mojo UI Variant | IPC Contract / UI Shell | same contract; behavioral baseline is the Enyo shell |
| Mojo UI Variant | Device Build | third independent .ipk (own MIME/adapter/socket/upstart/daemon) |
| IPC Contract | Offscreen Rendering | framebuffer + paint protocol |
| IPC Contract | Engine Embedding | page lifecycle / page manager |
| IPC Contract | Browser Services | LunaService surface |
| Engine Embedding | Offscreen Rendering | provides instance + event loop for painting |
| Engine Embedding | Navigation/Events | provides event loop for load progress |
| Offscreen Rendering | Input Bridging | shared transform (zoom/scroll ↔ coordinate mapping) |
| Offscreen Rendering | Navigation/Events | geometry/viewport events |
| Navigation/Events | Browser Services | redirects, MIME handoff, downloads |
| Browser Services | Device Build | device cert store / Luna integration |
| Licensing & Branding | UI / IPC / Engine / Services | cross-cutting compliance |
| Desktop Build | Engine/Render/Nav/Input | integrates them into the Phase-1 PoC |
| Device Build | Engine Embedding / Desktop Build | reuses integration, cross-compiles + packages |

## Dependency Graph
Implementation order (earlier enables later):

```
Tier 0 (foundations, parallel):
  IPC Contract Preservation
  Licensing & Branding (cross-cutting; applied throughout)

Tier 1:
  Engine Embedding & Build     (needs: build host per docs/TOOLCHAIN.md)

Tier 2 (the integration core, co-developed on Engine Embedding + IPC):
  Offscreen Rendering          (needs: IPC R2, Engine Embedding R2/R3)
  Navigation, Loading & Events (needs: IPC R1, Engine Embedding R3)
  Input Bridging               (consumes Offscreen transform)
  Browser Services             (needs: IPC R4, Engine Embedding R3, Navigation R6)

Tier 3:
  Desktop Build & PoC Harness  (Phase-1 acceptance: needs IPC + Engine + Tier 2)

Tier 4 (Phase 2):
  Device Build & Packaging     (needs: Tier 2/3 + cross-toolchain gate;
                                produces THREE independent UI .ipks, each running
                                on both TouchPad models)

Parallel UI track (depends only on the contract; packaged by Device Build):
  UI Shell (Enyo)   — forked/rebranded; live on device
  Mochi UI Variant  — Enyo-2/Mochi rewrite to parity; live on device (uses
                      Navigation/Services contracts as its behavioral reference)
  Mojo UI Variant   — built on the device's own Mojo framework; live on device
```

Notes:
- The graph is acyclic. Tier-2 domains are tightly coupled (the paint loop needs
  both the engine event loop and the buffer protocol) so they are co-developed,
  but each depends only on Tier-0/Tier-1 — bidirectional links between them are
  conceptual (see each kit's Cross-References), not build-order dependencies.
- The cross-toolchain (Device Build R1) has no engine dependency and can be
  stood up in parallel with Phase-1 integration to de-risk Phase 2.
- Both UI variants share the BrowserAdapter contract and can be developed in
  parallel with the engine work; only their on-device verification needs the
  working daemon. The Mochi rewrite was the larger of the two; as of 2026-07-20
  its shell, parity views, dialogs, and `app-mochi/PARITY.md` are built and it
  dual-installs on device (2026-07-19) — what is left is on-device functional
  verification, not construction.

## Changelog
- 2026-08-10 (Flash as a first-class target): the requirement changed from "NPAPI plugins load and
  run" to "Flash games are playable", so the kits changed with it rather than being marked done.
  - **cavekit-addons-extensions.md R7** gained three UNCHECKED criteria — sustained composite frame
    rate, audio output, keyboard-to-a-focused-plugin — plus a port-status block. The existing
    criteria all stand; the point is that they never described playability.
  - **cavekit-offscreen-rendering.md R8 (new)**: the damage-only repaint may never publish a frame
    it cannot prove. Four defects found and fixed, including `mDamageForceFull` which was declared,
    read once, and never assigned true anywhere — so the protection its comment described did not
    exist — and a buffer record committed BEFORE the blank-frame suppression, which let a rejected
    white frame become the base for the next partial paint.
  - **cavekit-device-build.md R9 (new)**: the bundled glibc 2.23 vs device glibc 2.8 ABI boundary.
    A `sem_t` layout change in glibc 2.21 turned a per-boot race over `/dev/shm/sem.PmLogLib` into a
    total Flash outage that presented as a code regression on untouched binaries. Contained by
    interposing one name. The generalisable lesson — and the reason this is a requirement rather
    than a bug note — is that a device library loaded into our runtime must be PROVEN per library
    with a standalone probe run under both loaders, never assumed.
  - **cavekit-input-bridging.md R7 (new)**: keyboard arbitration between a focused plugin and the
    card chrome. A Flash game focuses no HTML editor, so the editor-focus test could not describe
    it and every keypress also drove the chrome.
  - **cavekit-ipc-contract.md R1 amended, not quietly left ticked**: ONE command was ADDED,
    `asyncCmdSetExtraBuffer` (0x1600), for a third shared framebuffer. Every existing id,
    argument list and byte order is untouched, and the addition is optional in both directions
    (an adapter that never sends it, or a daemon that ignores it, both fall back to two buffers).
    Rationale + measurements in `docs/IPC-CONTRACT.md`. NOTE `BrowserServerBase` lives here
    without its generator, so the 0x1600 arm is hand-written and a regeneration must re-add it.
  - Requirement counts: input-bridging 7→8, offscreen-rendering 7→8, device-build 8→9.
  - Outcome by end of day: **fps DONE** (deferred 77→0, composite 27.1-32.2 fps, frame-gap max
    127→52-57 ms, verified against the build the `.ipk` ships), **keyboard DONE at the content
    level** (a SWF's own AVM1 runs on a keypress), **audio still not working** but reduced from
    an open-ended hunt to a single instruction (`cmp r0,#7` at 0x70f14).
  - Docs consolidated: `HANDOFF-2026-08-06.md`, `DEVICE-HANDOFF.md` and `NEXT-AGENT-PROMPT.md`
    deleted, their durable content folded into a single `docs/PICKUP.md`, which is now the only
    handoff document. The root README's "binary plugins (Flash) are not yet possible" claim was
    corrected — it had been false since 2026-08-09.
- 2026-08-06 (adversarial review + fixes): an inspector pass over this session's claims found a
  **shipped regression and two false statements**, and the counts moved DOWN to 304/13/31 as a
  result — which is the point of counting.
  - **Regression:** repointing the Enyo app menu's only "Preferences" entry at `about:preferences`
    left the card's own panel unreachable, removing the default-search selector, the
    JavaScript/Flash toggles and all four clear-data actions from the product; a block delete had
    also taken five preference handlers with it. Both restored; the menu now carries two entries
    until R5 can absorb what only the card can do.
  - **Security:** `adoptFromUrl` accepted ANY `about:*` url carrying `#chrome=`. `about:blank` is
    content-loadable, so a visited page could have rewritten the home button — and a `javascript:`
    home target runs in page context. Now gated on the path, with an `https?:`/`about:` allowlist
    on stored values in both the card and the page.
  - **Two claims disproved:** the Luna route was NOT blocked by policy (this repo already installs
    a public LS2 role file); and `CurrentUri()` is not the thing stripping the url fragment (this
    project's own Mojo start page round-trips fragments fine). Both corrected in place rather than
    quietly dropped.
  - Three criteria un-marked because the evidence did not support "met".
- 2026-08-05 (closing sweep): 299 -> **308 met, 7 partial, 31 open**. Prioritised by cost, and the
  cheapest wins turned out to be corrections, not code:
  - **cavekit-gre-widgets.md R6 recorded a false blocker.** It said find-in-page SIGSEGVs because
    the offscreen browser never sets up a frame-selection controller. It does not: `FindNext`
    reaches `nsContentUtils::SubjectPrincipal()`, which is `MOZ_CRASH` when there is no JSContext
    — and an embedder calls it from C++ with no script on the stack. `MOZ_CRASH` presents as
    SIGSEGV at address 0, which is what was misread for two weeks. Patch 0015 fixes it and
    carries the explanation inline. A kit that records a wrong cause is worse than one that
    records nothing, so this is the highest-value edit in the sweep.
  - **R2 (date/time) closed by turning the feature off**, which is one of the two outcomes the
    criterion itself allowed. Verified by a probe page that prints its own result: `type=date`
    before, `type=text` after, value intact.
  - **A deploy trap worth remembering**, recorded in `../impl/impl-gre-widget-inventory.md`: the
    first attempt to apply that pref silently did nothing, because the idempotence guard grepped
    for the pref NAME and upstream already set it in the same file. Guard on your own marker.
  - **preferences-ui R3 3/4 device-proven** from the profile rather than the UI. The fourth
    measured a real defect and left the criterion open — the honest outcome, and one that reading
    the pref back would have hidden.
- 2026-08-05: **`about:preferences` / `about:settings` now render on device**, and the shells
  gained the two settings they own rather than the engine. Three new requirements
  (cavekit-ui-shell.md R6, cavekit-mochi-ui.md R7, cavekit-mojo-ui.md R7) for the home button's
  target and the start page's shortcut list; cavekit-mojo-ui.md R6 extended for the command row.
  Totals re-counted programmatically (see the Totals line for the current figure).

  Four things here are worth carrying forward more than the counts are:
  1. **The preferences page needed no C++ change.** An `nsIAboutModule` registered from a JS
     component reaches `Services.prefs` with the system principal, so the engine's own about:
     table in `BrowserPageGoanna.cpp` stays as the two-static-pages thing it was written to be.
  2. **A kit criterion was wrong, not just unmet.** cavekit-preferences-ui.md R1 required the
     page be built on `<prefwindow>`; that both prescribes a HOW (against the kit conventions)
     and prescribes the wrong thing, because `<prefwindow>` is a chrome-WINDOW binding and this
     embedding has no window. Restated as an outcome.
  3. **Every user-reported symptom this session was invisible in the source.** The Mojo command
     bar rendering empty was a `.palm-menu-fade` block pushing statically-positioned items out
     of a clipped box; the url bar "looking incorrect" was Mojo's TextField being two nodes that
     swap on blur. Both were found by screenshotting the device and dumping live geometry —
     `luna-send palm://com.palm.systemmanager/takeScreenShot` — not by reading CSS. The same
     method caught 13 preference rows bound to prefs a bare GRE does not ship.
  4. **`Mojo.stringifyJSON` does not exist in this framework build**, and `jihad-history.js`
     used it — so Mojo history had never persisted. Found only because an unrelated new call
     threw in the main scene's `setup()`.
- 2026-08-04 (second pass): A sweep against the open list closed **19** criteria and corrected two
  whose premise was wrong. The engine gained three patches, each fixing something that had been
  silently doing nothing: `PuppetWidget::GetCurrentWidgetListener` never fell back to
  `mWidgetListener`, so every synthesized KEY event was dropped (mouse hid it — there is a
  `SendMouseEventToWindow`, but no key equivalent); no `keypress` was ever synthesized, which is
  what XUL `<key>` elements match on; and `nsIBadCertListener2` reached nobody, so accepting a bad
  certificate had no cert to override. That last one came from the user's suggestion to look at how
  **Atlas** does it — WebKit hands Atlas the certificate as a signal argument, and the lesson
  (take it from the notification that carries it, don't recover it later) transferred exactly.
  Also: W3C touch events were OFF on a touchscreen-only device (autodetect only works on
  Windows/GTK3) and multi-touch parsed only its first point; Flash loads on the TouchPad and
  `about:plugins` lists it; the memory guardrail was observed firing for the first time, which
  required admitting it had been unexercisable; and the memory budget is now a number (~90 MB RSS,
  ceiling 150 MB). What remains is genuinely gated: TouchPad Go hardware, a human pinch, the
  windowless NPAPI port, a Luna service design decision, and the OE/bitbake track.
- 2026-08-04: Add-ons **R2** and **R6** closed, and with them three daemon-lifecycle defects that
  had been hidden by the daemon never exiting: no SIGTERM handling (so the add-on database and
  prefs never flushed — a UI disable came back enabled after a restart), `XRE_TermEmbedding`
  running while pages were still live, and a `~YapServer` teardown that had never once run and
  was broken. cavekit-engine-embedding.md R2's "shuts down cleanly" was corrected from met to
  met-with-evidence and gained two criteria. The user-reported "install button does nothing" was
  traced to the DIALOG deadline, not the add-on stack: the card opens the reply pipe only when
  the user taps, so the 5 s pickup deadline silently denied every dialog a person answered in
  time — every harness passed throughout (cavekit-browser-services.md R3). New section
  **"Driving the browser without a human"** records the four traps that cost time here, chiefly
  that `jsurl` does not execute in a chrome document while still reporting `ok=1`. Totals are now
  COUNTED rather than estimated; the previous line had drifted.
- 2026-08-03 (second session): The card JS dev loop is restored (`push-card-js.sh`, stamp-proven
  reloads) — the two causes were `novacom run`'s stdin-EOF output race and the real WebAppMgr JS
  cache. With it, the `<select>` popup closed on **all three** variants: the "empty popup" was a
  JSON field-name mismatch (the stock framework consumes `onOpenSelect` itself and expects the
  isis `items[].text/isEnabled` + `selectedIdx` shape), so the daemon now emits that shape, the
  Enyo app's own popup code was deleted as unreachable, Mochi got an overlay list, and Mojo needed
  no app code at all. Opus review hardened the apply path (disabled/optgroup/out-of-range/no-op
  guards, fail-closed popup file, process-global ids). Mojo gained **R6** (new card / history /
  share), lost its redundant title row, and had its toolbar overflow fixed — root cause: the card
  WebKit ignores unprefixed `box-sizing`, now documented as a platform constraint. All three start
  pages share the logo/title/engine-line/hint block. Totals → ~54 met / ~9 open; priorities
  reordered around the now-unblocked queue.
- 2026-08-03: Session handoff for the next Fable run. Closed on device: scroll pan headroom
  (offscreen, overscan paint, Opus-reviewed), long-press/`contextmenu` (input, hit-test round-trip
  was a daemon stub), input coord mapping (input R5, doc→viewport), and **device-build R7** — all
  three variants live (`push-variant.sh`/`push-engine-update.sh`, `device-independence-test.sh
  check` 24/24). Apps renamed Enyo/Mochi/Mojo; Jihad logo on the about pages; Enyo start page
  follows VKB/orientation. Built but card-blocked: the `<select>` popup (daemon done + verified,
  card list empty behind a broken card JS dev-loop) and the XPI install prompt (authored, unwired).
  Totals → ~48 met / ~14 open; priorities reordered around the card-dev-loop prerequisite. New impl
  docs: `impl-scroll-overscan-2026-08-02.md`, `impl-menupopup-2026-08-02.md`,
  `impl-select-popup-2026-08-03.md`; the START-HERE handoff was rewritten.
- 2026-07-31: Reconciliation against recorded evidence. The Mochi row read "⬜ 0/5 — skeleton
  only" while cavekit-mochi-ui.md records R1 (dual-install VERIFIED ON DEVICE 2026-07-19), R3 and
  R5 as met → now 🟡 R1/R3/R5 ✓, R2/R4 [~]. The UI-Shell row now reflects R4's three
  device-verified ACs (device-test-2026-07-19 Session 4). The Offscreen row goes 5→6 requirements
  (R6, added 2026-07-26) and 6/6 (rotation composite + zoom rework device-confirmed 2026-07-27).
  The Device-Build row no longer describes the Mochi `.ipk` as broken — review items #1/#4/#5/#6/
  #9/#12 are fixed + build-verified (commits 9413d16, b1c0112); the two real gaps (clean-clone
  reproducibility, on-device install) are named instead. Totals corrected to 55 requirements
  (~43 met, ~12 open) and "Current priorities" reordered to what is actually open — the list still
  led with those fixed review items. Added the 2026-07-27 rotation/zoom milestone.
