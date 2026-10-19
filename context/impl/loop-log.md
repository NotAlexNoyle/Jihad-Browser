---
created: "2026-07-19"
last_edited: "2026-07-19"
---
# Cavekit Loop Log

Build site: context/plans/build-site.md

### Iteration 1 (Wave 1) — 2026-07-18/19
- T-049: Mochi app shell + bundling — DONE. Files: app-mochi/appinfo.json, app-mochi/README.md, build/webos-oe/build-mochi-ipk.sh. Build P (ipk 1.4 MB, 394 entries, re-verified on merged main), Acceptance 4/5 ([~] dual-install device-gated). Commit c6d0e9a.
- T-054: TouchPad Go machine config — DONE. Files: build/webos-oe/conf/machine/{tenderloin,opal}.conf + include/jihad-touchpad.inc, recipes-jihad/*.bb, docs/DEVICE-BUILD.md. Parse-sane P, Acceptance 2/3 + 1 [~] (Opal install device-gated). Commit 41d49f0. Finding: both models one ARMv7 softfp binary set; only DPI differs.
- Infra: ck:task-builder agent def broken (tools: [All, tools] → zero tools) — used general-purpose agents. Worktrees branch from stale origin/main (50 behind) — agents fast-forward to local main first.
- Device: offline all wave (novacom -l empty) — T1–T5 retest still pending.
- Next: Wave 2 = T-050 + T-051 + T-052 (single packet, shared app-mochi/source surface), then T-053.

### Tier gate (after Wave 1) — 2026-07-19
- Codex cycle 1: 9 unique findings, 2 P1 (layer.conf missing; UI recipes depended on stock adapter). Fixed 8081387: layer.conf, browser-adapter-jihad recipe, PACKAGE_ARCH=all, ipk ships LICENSE/NOTICE + BUNDLED-VERSIONS provenance, pipefail globs, kit/doc honesty.
- Codex cycle 2: 6 P1, one class — OE skeletons non-executable (stub do_compile, external SRC_URI, LIC checksums, 2014-era class/override syntax, missing upstart job, Impl.so-in-app-bundle). Fixed 044f295 by honest re-scope: Full-OE path documented NON-RUNNABLE with gap list; daemon recipe installs event.d/jihad; underscore overrides; host-path leak out of BUNDLED-VERSIONS; impl statuses annotated; .claude/ untracked+ignored. F-390 NOTICE attribution → T-050. F-391 enforcement deferred, documented.
- 2-cycle cap reached → ADVANCE.

### Iteration 2 (Wave 2) — 2026-07-19 — IN FLIGHT
- Packet: T-050 (NOTICE/headers) + T-051 (WebView kind, MIME application/x-jihad-browser, frozen method set) + T-052 (Mochi shell layout). Single agent — shared app-mochi/source surface.

### Iteration 3 (2026-08-15 session, Wave 1) — 2026-08-15 — IN FLIGHT
- Site: context/plans/build-site.md (2026-08-10, 56 tasks). RECONCILED first: kit annotations dated 2026-08-10 showed rows T-104/T-106/T-109/T-110/T-112/T-113/T-114/T-121/T-127/T-128/T-140 done or answered and T-111/T-115/T-120/T-130/T-133/T-139 partial — prior session updated kits, not site. Rows now carry reconciled status.
- Device: ABSENT. No TouchPad on USB, novacomd needs root (sudo needs password). All device-gated tasks parked: T-108, T-116, T-119, T-124, T-141, T-142, T-145 + Tier 5.
- Mode deviations, both deliberate: NO worktrees (tree carries 137 uncommitted prior-session files; worktree from HEAD loses them all), NO commits (would sweep prior uncommitted work under wrong messages; user has not asked for commits). Agents run in main tree with disjoint file ownership.
- Infra: ck:task-builder def still broken (tools: [All, tools] → zero tools; same as Iteration 1) — general-purpose agents, model opus per quality preset.
- Wave 1 packets (5, parallel): P1 T-122 opal-kernel pin (evidence-gated) + T-115 sysroot/adapter-deps manifests; P2 T-129 vendor+patch YapCodeGen (last ipc-contract R1 box); P3 cubeb ALSA backend into ARM libxul (gre-widgets R1/R7 audio half; owns out-arm) + T-111 build-goanna.sh append hardening; P4 T-107 keyarb address-bar observable + latch-setting-input confirmation; P5 T-135 cert-store integration (three live defects + JihadCertStore dlopen path; desktop compile only).
- Wave 2 queue: T-120 remainder (28-byte npapi.h re-stage + enable + double-activation; deferred — file conflict with P5 on render/goanna/), T-139 run-verification, T-103 remainder, ARM daemon rebuild after P3/P5 land.

### Iteration 3 Wave 1 RESULTS — 2026-08-15
- P1 T-122: DONE — old value WRONG, board is `shortloin` not `opal`; pinned `2.6.35-palm-shortloin` from HP's own Opal source drop w/ tenderloin control case; MACHINEOVERRIDES fixed. T-115: declaration COMPLETE — 187-deb sysroot manifest (sha256, archive-verified, --fetch proven) + adapter-deps manifest 19/19 (qt4-extract = STOCK jessie debs not device pulls; 7 device ELF not 10). R3/R6 annotated. Meta-lesson: "device-gated" label had hidden a source-derivable answer since 2011.
- P2 T-129: DONE — generator vendored+patched (space-hang fixed, `async optional` flag), regeneration reproduces shipped wire w/ ZERO hand edits, check-yap-contract ZERO defects, patches proven inert on upstream input. **ipc-contract R1 fully [x] — first whole requirement closed this session.** Client pair is older generator vintage (wire-identical) — separate decision, not taken.
- P3 AUDIO: DONE (built, unheard) — `--enable-alsa`, Jessie armel alsa-lib staged, cubeb_alsa.o + DT_NEEDED libasound.so.2 + 31 snd_* imports IN the ARM libxul. Runtime = bundle Jessie lib (hard DT_NEEDED; device lib = PmLogLib-shape risk). Device fallbacks recorded (PCM `default`→pulse-plugin trap; ALSA_CONFIG_PATH override; full-bundle deploy REQUIRED — push-engine-update ships no .so). T-111 remainder: DONE — build-goanna.sh appends fail-loud + value-checked (cmp), idempotence + staleness paths TESTED against scratch dist.
- P5 T-135: DONE (write path) — JihadCertStore dlopen wrapper; three defects fixed AND desktop-proven live (PEM fingerprint = server cert; answer=1 permanent w/ cert_override.txt vs answer=2 session verified; degrade = one line). Find: the emitted nsresult was a transport-class LIE — classification now from nsISSLStatus flags. Read direction COSTED (blocked on enum values dlopen can't give). 13 harness scripts gained the link line. ARM rebuild deferred (objdir owned by P3).
- P4 T-107: DONE — see Wave 1 in-flight note; latch finding rescoped T-124/T-144 (double-tap-only set; no latch log compiles — Debug.h DEBUG off).
- Parent inline: fixed build-goanna-arm.sh pipefail bug (grep|while killed EVERY successful run before its own staleness guards; 27/29 patches have no JS) — reproduced, fixed w/ `|| true`, verified.
- Site rows updated for all of the above. No commits (session policy). Tier 0 not closeable: T-108/T-116/T-119 device-blocked; frontier continues into unblocked deeper tiers.

### Iteration 3 Wave 2 RESULTS — 2026-08-15
- W2-A T-120: DONE (code+built, device-verify pending) — 28-byte header re-staged SURGICALLY (debian copy also moves npPalmApplicationIdentifier — whole-file swap would have silently changed a verified call); sizeof guard proven both directions; fence out behind JIHAD_TOUCH_EVENTS=1 default OFF. Two real bugs: TouchEnd indexed past shorter array; npPalmEnableTouchEvents never set (un-fence alone = silent no-op). Suppressor: consumed-touchstart drops mouse for the sequence + 400ms/48px tail + 5s cap; failure modes recorded. Rebuilds ALL PASS: ARM daemon (contract check 73/58 incl 0x1600), both adapters x3 variants, .ipks 1.0.4 with libasound.so.2 VERIFIED inside all three. TRAP: stale out-ipk ≤1.0.3 lack libasound — daemon won't start from them.
- W2-B: three verifications RUN with deliberate-failure proofs. T-139 PASS → gre-widgets R5 first criterion [x] (38 checks, GTK AND offscreen PuppetWidget identical). T-103 desktop half PASS (mismatch alert fires + names addon; matching-declined control zero alerts) — stays [~] on device screenshot. T-111 PASS → R7 second criterion [x] (eleven rows from shared file, zero unavailable). New dead-ends recorded: javascript: dead in chrome HTML docs too (ScrollTo no-op on chrome pages); stale desktop profile fakes prefs regressions.
- Parent: preferences-ui flush box reconciled [x] (T-132 evidence was site-only); DESKTOP-POC.md test table gained the two new runners.
- Codex tier gate NOT run: tier_gate_mode=severity + codex available, but no tier completed (Tier 0 holds device-blocked T-108/T-116/T-119) and no clean git base exists for a session-only diff (tree carried 137 uncommitted files before session start) — a review would drown in prior work. Revisit when the user decides on committing.

### Iteration 3 Wave 3 RESULTS — 2026-08-15
- W3-A T-146: DONE (daemon measured, card toast device-gated) — PostNotification + jihad-notify observer + `notifications` Luna subscription (public bus, degrade-safe); toasts in all three shells (Mochi needed new lunaSubscribe — existing bridge self-destructs on first reply). T-148: DONE — audit found the T-103 alert was the ONLY informational dialog; cookies/cache-cleared + addon-installed notices ADDED (emitted nothing before); popup-blocked deliberately not built (nothing listens to DOMPopupBlocked; rate-limit judgement needs device). Bonus: msgDialogUserPassword never sent (dead arm). Desktop measured w/ negative controls; ARM rebuild + contract guard GREEN. gre-widgets R5 last criterion [ ]→[~].

### Iteration 3 DEVICE addendum — 2026-08-15 — TouchPad reappeared, redeployed enyo, verified 4 things
- USB monitor caught the device (VID 0830); novacomd was already up. Installed ipk 1.0.4, swapped adapter to touch-events PDK build, pushed current cert-store daemon. Removed two stray jihad.bak.* jobs contending for the enyo socket.
- CERT STORE (T-135): CLOSED end-to-end on hardware — /var/ssl write access, DER→PEM, ordinal-18 classification, CertInitCertMgr rc=0 + CertAddTrustedCert serial 6, override+reload, page loads. browser-services R5 device box → [x]. libPmCertificateMgr present on 3.0.5; read-direction syms resolve (un-blocks the costed enumeration half).
- TOAST (T-146): CLOSED — clearCookies public-bus push → "Cookies cleared." toast visible in device screenshot. gre-widgets R5 last → [x].
- FOCUS: device transitions 1→0→1 correct; desktop focus_test failure is harness-side.
- AUDIO: backend reaches pulse (err=0, sink-inputs live) but output webOS-POLICY-gated (sinks SUSPENDED, 0%/-inf dB; needs com.palm.audio scenario grant). Bundle-Jessie-libasound decision REVERSED (device lib → err=0, Jessie → err=3). New memory + dead-ends recorded.
- NOT closed: file:// XPI doesn't trigger install on device (T-103 device screenshot still open); container test sweep 19/23 (failures: 3 harness-signature-only + xul pixel dead-end, none session regressions).
- TYPING (T-141, gre-widgets R3): device-tested via inject → core works (text insertion Hello123, Backspace, textarea Enter-newline), retiring the highest-risk "typing broken" concern → [ ]→[~]. Open: accelerate-run (inject can't send rapid keys), caret-by-arrow (0xE0A2 didn't move — the known keycode-map reversal), textarea first-word-after-focus drop. Full VKB close still needs a human on the on-screen keyboard.
- preferences-ui R6 chrome-page typing box UNCHANGED: my test corroborates its "works on content" half; the about:preferences (chrome) typing failure it names is the open item (needs the InsertText instrument run).

### Iteration 3 AUDIO-POLICY dig — 2026-08-15 — the one non-human-gated engineering item, fully scoped
- Took on the audio-scenario LunaService task (the hook correctly flagged it as scoped-not-gated). Reverse-engineered the whole webOS audio policy gate on device.
- Stock mechanism found: `element.palm.audioClass="media"` (Palm WebKit extension, /usr/palm/frameworks/media/media.js:288) → libWebKitLuna routes to pmedia sink + drives audiod media scenario. Goanna lacks it → our streams are anonymous pulse clients that module-palm-policy never routes.
- API probed: palm://com.palm.audio/media/{enableScenario,setCurrentScenario,...}, scenarios media_front/back_speaker; audiod owns msm_playback_route; policy over /tmp/palmaudio.
- Measured: enableScenario ALONE insufficient (returns true, sinks stay SUSPENDED); paplay HANGS to EVERY sink incl raw hardware pcm_output (100%/unmuted) → no shell reference playback exists, codec route powered only during a policy-recognised media stream. Flash audible because it has its own pflash sink + plugin policy path.
- Port path SPECIFIED (impl-audio-backend.md §2026-08-15): (1) ALSA default→pmedia via ALSA_CONFIG_PATH, (2) ref-counted audiod scenario hold with $activity over AudioStream lifetime (daemon has outbound Luna client already), (3) validate sink RESUMED. Deliberately NOT implemented: needs rebuild+deploy cycle AND has no on-device A/B reference, so blind implementation = guessing. One focused next-session task, now fully bounded (was "cause unknown").
- Also device-verified this stretch: typing works (Hello123/Backspace/textarea-Enter); caret-by-arrow open (keycode-map reversal T-127); the "bundle Jessie libasound" decision REVERSED (device lib err=0 vs Jessie err=3).

### Iteration 3 AUDIO RE COMPLETE — 2026-08-15 — mechanism fully decoded, fix determined
- User: "continue with all non hardware gated kits". Drove the audio task (the one non-hw-gated item) to a definitive conclusion via device experiments + host RE of module-palm-policy.so + audiod.
- MECHANISM: virtual sinks (pmedia/…) are NULL sinks; audiod bridges one→hardware + powers WM8994 codec ONLY for a stream CREATED on that virtual sink (route_sink_input_new_hook_callback → notifies audiod). Policy string: "THE DEFAULT DEVICE WAS USED TO CREATE THIS STREAM - PLEASE CATEGORIZE USING A VIRTUAL STREAM". Stock uses element.palm.audioClass="media" (WebKit→libpulse on pmedia).
- ENGINE PIPELINE WORKS: <audio> decodes + clock runs 0→3.00, ended, err=0 (via ALSA_CONFIG_PATH alias). Only audibility missing.
- ALSA BACKEND = DEAD END, proven: our stream AND stock `aplay -D media` BOTH land on Sink 0 (pcm_output) at Volume 0% — the ALSA pulse plugin can't reach the pmedia sink; move-sink-input + force-volume also fail (null sink). Do NOT deploy the ALSA libxul expecting sound.
- FIX DETERMINED (bounded build task): cubeb PULSE backend (flip mozconfig off --disable-pulseaudio) + pa_stream_connect_playback(sink="pmedia") + hold audiod media scenario from the daemon Luna client. libpulse.so.0 on device; needs libpulse headers in sysroot. Gate on sink-input landing on Sink 5=pmedia w/ non-zero volume; final = human ear (no on-device capture works). Full spec impl-audio-backend.md §FULLY RESOLVED 2026-08-15; memory updated.
- Pulled module-palm-policy.so + audiod to scratchpad for host RE. Device left clean (shipped flags, JIHAD_INJECT=1, no ALSA/MOZ_LOG, 3 jobs, daemon up, audio reset).

### Iteration 3 AUDIO FIX BUILT + DEPLOYED + VERIFIED — 2026-08-15 — pulse backend unmutes engine audio
- User: "continue with all non hardware gated kits until all are complete." Implemented the determined audio fix end to end.
- IMPLEMENTED: mozconfig --enable-pulseaudio; staged pulse-0.9.22 headers + libpulse.pc + link stub in arm-sysroot (device-ABI, NOT Jessie 5.0; verified all 62 cubeb symbols present in device libpulse.so.0 via host nm); patch 0030-cubeb-pulse-webos-virtual-sink (JIHAD_PULSE_SINK targets pmedia when default). cubeb dlopens libpulse (no link dep).
- BUILT: ARM libxul clean (cubeb_pulse.o present, JIHAD_PULSE_SINK + libpulse.so.0 strings + pulse_init in binary). configure: "checking for libpulse... yes".
- DEPLOYED: push-engine-update.sh enyo (new libxul md5 c93f93fe...).
- VERIFIED ON DEVICE: engine stream now PULSE-NATIVE (application.name="Jihad Browser", was "ALSA plug-in"), Volume 100%/uncorked (ALSA path was forced 0%), and /proc/asound DAC hw_ptr advancing ~44100/s = samples reaching the WM8994 codec. Plays to completion err=0. The pulse-native app IDENTITY is what unmutes it (measured: still 100% with scenario DISABLED).
- RESIDUAL (human ear only): speaker physical emission — both automated oracles broken (parec monitor=0 bytes; hw_ptr runs during pause) — + possible daemon media-scenario hold for the speaker amp (unmute doesn't need it). pmedia routing fell back to pcm_output, unnecessary for unmute.
- Device clean: JIHAD_PULSE_SINK removed from job (no-op), pulse-backend libxul deployed, JIHAD_INJECT=1 intact, audio reset, 3 jobs, daemon up. Patch 0030 + sysroot pulse inputs in tree.
- This moves engine audio from "no output, cause unknown" all the way to "unmuted, reaching the DAC, verified" — essentially closing the engine-audio half of gre-widgets R1/R7 pending a human listen.

### Iteration 3 CLOSE — 2026-08-15 — REMAINING WORK EXTERNALLY GATED OR SCOPED-FOR-NEXT-SESSION
- Host-doable frontier EXHAUSTED. Open criteria all gated on: device+novacom (audio listen, cert run, touch verify, toast run, T-103 screenshot, T-108, T-116, T-119/T-133, T-131, T-141, T-142→T-147, keyarb procedure), human-at-device (T-150/T-151/T-152, frame-pacing eyeball decision), absent hardware (T-149 BT keyboard, T-153 TouchPad Go), human decisions (T-155 sign-off, T-156 Mojo exemption, commit policy, T-154 sudo bitbake).
- Session totals: 3 waves, 8 agent packets, ~15 tasks closed/advanced; 2 whole criteria boxes closed + 1 requirement (ipc R1) fully closed; 4 real code bugs found+fixed beyond task scope (TouchEnd OOB, npPalmEnableTouchEvents never set, build-goanna-arm.sh exit-1-on-success, T-107's SWF-fires-both-ways observable defect); 4 new dead-ends recorded.
- PICKUP.md carries the 2026-08-15 delta section (new traps: ipks ≤1.0.3 lack libasound; double-tap latch; javascript: dead in chrome HTML docs; stale desktop profile).
- Codex tier gate: not run (no completed tier; no clean session-only git base — see Wave 2 note).

### Iteration 3 T-131 FIX — 2026-08-15 — chrome-page typing diagnosed AND fixed on device
- Diagnosed via JIHAD_LOG_INSERT instrument: mFocusedEditable NULL in chrome docs (branch=NO-TARGET editable=0). Root cause: focus listener on top content doc doesn't see chrome field focus events.
- FIXED: InsertText recovers null mFocusedEditable from document.activeElement (per-doc, dodges focus-manager window-scoping; gated on edIsTextInput so content/unfocused pages untouched). GoannaRenderPage.cpp.
- Rebuilt daemon (contract OK), deployed (md5 24251922→35fdba44), device-verified: typing into about:preferences home field → "recovered editable from document.activeElement" + branch=value/sel before=29 want=32 readback=32 (value grew by inserted length). Chrome-page keyboard entry WORKS.
- Residual: real-finger tap to focus (human, =T-150). preferences-ui R6 keyboard-entry advanced. Deployed daemon now carries the T-131 fix (rides push-engine-update). Device clean, test hooks reverted.

## Build site 2026-10-01 (webOS CE 3.1.0) — context/plans/build-site.md

### Iteration 1 — 2026-10-01
- T-202 engine CopyData fix — DONE. Files: third_party/uxp gfx/layers/basic/BasicImages.cpp (`1fff8023`). Build P.
- T-216 libxul build — DONE. md5 65e3cc84.
- T-201 housekeeping + baselines — DONE. MOZ_LOG gone; idle 2.4%/117 wakeups/s/48 B/s; connect→paint ~1.0 s.
- T-209 test assets — DONE. make-media-assets.sh, jihad-codecs.html, jihad-input-echo.html, stage-media-pages.sh.
- T-219 engine push ×3 — DONE. All three 65e3cc84, daemon unchanged.
- T-220 `<video>` to end — DONE. 12/12, ended=true err=0 vq=149/0/0. Kit box `[x]`.
- Next: T-221 codec table.

### Iteration 2 — 2026-10-01
- T-221 codec table — DONE. 8 formats play to end, 3 refused (no decoder). FLAC fixed: ffvpx libs now bundled + pushed ×3. Kit codec box [x].
- T-222 play/pause — DONE (agent part). Both directions via `click` card px. Kit stays [~] for finger.
- T-223 prefs media block — DONE. Kit R7 [x].
- Display was off/locked → woke via display/control. Screenshots archived.
- Next: T-224 R10 media clock measurement; daemon batch (T-203..T-206).

### Iteration — 2026-10-01 (third session, prefs/input lane)
- T-250/251/252/253/254/255/214/278: DONE on device, 3 variants. Fixes: change event, number-field target, native scroll, JS default branch + row, location.replace, window.open → new card, meta-viewport only when declared, adapter late-identifier reconnect + 0x0 window. T-212 built (needs human double tap). T-257 N/A.
- Daemon now `4cdc0846…`, adapter impls enyo `20b9dead…` mochi `42d58021…` mojo `a187efbd…` (live-swapped). `.ipk`s still stale → T-263.

### Iteration — 2026-10-01 (third session, sweeps + packages)
- T-256/258/260/262/264/265/266/267/268/269/270/271/272/274/275/277: DONE on device (CE 3.1.0). Fixes found by the sweeps: DOM key codes for page key events; field under the keyboard lifted; popup tap double zoom + repaint on popup open; Mojo launch `target`; selection colours; HTTP auth (`nsIAuthPrompt2`); Up/Down swap + Home/End; damage scaled at a fit zoom.
- T-263/T-276: three `.ipk`s built + installed via ipkgservice, twice (batch 2, then final). Payload md5s read back.
- T-240: HUD appears on a volume KEY press during page audio (kernel keyboard); streams sit on `pcm_output`. Box `[~]` (rocker = human).
- T-261 N/A (sinceResize 94–129 ms). T-236 stays blocked behind T-234.
- Experiment: `JIHAD_GRE_KEYS` (off by default) — engine editor takes field keys; results in gre-widgets R3.
- Prefs: all 56 table rows of `about:preferences` walked through their own control on the device — 56 change the engine pref, 55 restored by the control, 1 by the test's `prefs.js` copy (preferences-ui R2). Real key path (kernel keyboard) types into a settings field.
- Blocked on owner: T-231–T-234, T-273 (removal refused by permission gate), T-279 (push), T-280–T-291 (finger / eye / decisions).

### Iteration — 2026-10-02 (owner go-ahead)
- Decisions: card rows out (T-290→T-294), Flash from system folder + real switch (T-295/296/309), all Mochi omissions built (T-297…T-308). Print blocked (no printer).
- Removal audit + engine push: approved, refused by permission gate twice → owner runs (`/tmp/rm1.sh`, `git push fork`).
- Found + fixed: dialog-sink teardown killed add-on prompt after first card close; Mochi bookmarks list never returned rows; hidden panels never laid out.
- New tools: `card-selftest.sh` (+ `mochi-selftest.js`, `enyo-selftest.js`), `prefs-row-walk.sh`.
- Batch 3 built + installed ×3 (daemon `78fb52e8…`).

### Iteration — 2026-10-05 (owner direction: printing, wallpaper/share, root rw, Atlas report)
- Print: stock stack SEES Epson, drops non-HP (`not HP printer and doing printer filtering`). Built own path: `jihadPrint.js` (mDNS from :5353, IPP/IPPS, JPEG sheets) + `JihadPrintDialog` Enyo + Mochi. Device: real printers listed; sheets verified on `ipp-sink.py`. Mochi Print box `[x]`. T-321/324/325.
- ACCIDENT: dry-run test printed 1 dark sheet on owner's Epson. Cause = stale `startupCache` served old component. Fix: daemon drops cache when bundle fingerprint changes (T-326, new R4 criterion `[x]`). Rule: sink first.
- Mochi fresh install from nothing (owner removed it): clean; launcher needs installer `rescan`; empty `packages/<id>` dir = removal residue (R8 note). T-319.
- Root rw: installer service remounts `/` rw and leaves it; ours leave as found. T-322. Atlas report: Atlas's own, at shutdown; side by side OK; Atlas box `[x]`. T-323.
- Share Image / Set as Wallpaper run via sheet handlers; wallpaper restored. T-320.
- Icons: Home + Bookmarks/History redrawn (3 rounds). `luna-restart.sh` closes USB alert (`-m com.palm.lunasend`). `dev-sh.sh`. T-327/328.
- All three variants on bundle `a4a2f8e2…`, daemon `889263013e40…`. T-329.
- Open, owner/human only: T-231–T-234, T-236, T-273 (Enyo/Mojo removal), T-280–T-289, T-317.

### Iteration — 2026-10-05 (evening) → 2026-10-06 (owner: long-press, keep pages painted, DDG links + Palefill, padlock, Codex gate)
- DDG "links don't work" = Flash destination pages killing the daemon in a reopen loop → engine windowless fix + child env + crash guard. T-330.
- Settings merged into one page; Help gone. T-331.
- Long-press menu, all shells, frozen contract (component + notification channel + clip files). Device Enyo/Mochi scripted. T-332; two review rounds of fixes T-333.
- Composed frames: 4096 rows, copy+render, sweeps, grow, wide when zoomed, adapter guard by viewport width. Dumped frames row-perfect after jumps; heavy page 110-180 ms. Pans need a finger. Correction: earlier "scroll test" was a stub call. T-334.
- Padlock → certificate (daemon `pageCertificate`, own-card-only). T-335. Found adapter `pageFocused` stub → fixed, verified. T-336. TLS-failure misclassification fixed. T-338.
- Palefill bundled unmodified (MPL-2.0 notice, hash-pinned) + engine AppCompat rule fix; active in all three. T-337.
- Codex gate: Cavekit 3.1 script incompatible with Codex 0.160 (silent pass) → Codex run directly; inspector when credits ran out. Findings + dispositions: `impl-review-findings-6.md`. T-339.
- Frame dumps filled the 40 MB `/tmp` once (cleared; guard added). Root fs needs a LunaSysMgr restart before it remounts ro after an adapter upgrade.
- 2026-10-06: three packages rebuilt at `707795de` and installed (T-340, in progress at the time of writing); kits/registry/docs updated.
- 2026-10-06 (later): Codex on Palefill (1 P1 accepted/documented, 3 P2 fixed, 1 P3 fixed); credits out again → inspector on the last fix round (3 P2 fixed: `<body>` passed the pressed-field rule, Cut claimed success over non-editable targets, read-only fields had lost Copy; P3s fixed/documented). Device: read-only Select All → Copy lands; no Paste/Cut. Daemon `b7be3e3c…` pushed to all three; packages rebuilt + installed last (T-340).

- 2026-10-06 (third): Mochi padlock after a failed load verified (hook `a` step); pinch through the adapter's gesture entry points works and showed the wide region being dropped at the content's right edge (one-px rounding; fixed, `region columns:` line added, change-gated); Mojo test hook written (`mojo-selftest.js`, `card-selftest.sh mojo`) and showed Copy/Cut/pageFocused never reached the engine on Mojo (widget's `copy(undefined)`, no `cut`/`pageFocused` wrappers; fixed in the shell, method set unchanged). Codex out of credits → inspector round 5.
- 2026-10-06 (go-ahead): SDK 0.4 (owner ran sudo); Enyo + Mojo removal/fresh-install audited (T-231–T-233, T-273 DONE); reboot T-234 DONE (T-236 not needed); real print to the Epson (job 32) after the sink; Codex round 6 → 4 fixes installed as upgrade; tabs groundwork (kit, design note, Tier 71 pending); pushes refused by the permission gate → owner.
- 2026-10-06 (night): Codex r7 12 findings fixed + device-verified (referrer 6 cases, title flood, preview gating, static-theme limits). Menus = Atlas on Mochi (title-bar app menu via sysmgr relaunch) and Mojo (bottom circles, owner; reload in the url field). Flash: strip-gen regression fixed (16 → 34 fps paints); host drivers off by default, content rate unchanged (grid SWF 22.5 vs 22.1). Mojo pinch fixed in the adapter (hold cancelled; widget owns zoom). DDG Lite default. Codex out of credits → inspector round 8. T-355–T-364.

### Iteration 1 — 2026-10-18 (Cavekit make loop; Tier 87 added, T-503 done before the loop)
- T-385: positional engine-call wrappers — DONE. Wrappers existed (Tier 81); guard `build/engine/check-engine-calls.sh` added, found+fixed `test/embed_load.cpp` raw LoadURI (container compile P). Next: T-386.
- T-386: engine JS surface shim — DONE. `render/goanna/modules/JihadEngine.jsm`, components through it, checker reads it (+ check 4b, mozconfig glob fix 1 h -> 36 s). Device Enyo about:addons + Personas view P. Commit e3e24178 (with T-387 parts 1-2).
- T-387: install script, derived push list, desktop tests — DONE. Desktop engine RB_20261006 (patch 0043), personas 7/7, media 18/18, prefsui 38/38, negatives fail. Next: T-504 (zoom hold in progress: first frame at new zoom 50-600 ms, zoom out waits on page JS).

### Iteration 2 — 2026-10-18
- **Task:** T-504 — Zoom keeps up
- **Tier:** 87
- **Status:** PARTIAL
- **Files:** render/goanna/BrowserPageGoanna.{cpp,h}, render/goanna/GoannaRenderPage.cpp, engine patches 0044-0045 (gfx/gl/GLTextureImage.{h,cpp}, gfx/layers/opengl/CompositorOGL.cpp, gfx/layers/ipc/CompositorBridgeParent.cpp), render/goanna/test/enyo-selftest.js, tools/device/gpu-bench.c, tools/device/t502/zoom-run.sh + README
- **Validation:** Build P (ARM libxul + daemon), device rounds 162-186; Acceptance 4/5 (the first frame ≤ 250 ms holds on settled pages, not while the page's script holds the main thread)
- **Next:** T-505 — real-page pans and the main-thread gaps (frames made off the main thread, which also closes T-504's open criterion)

### Iteration 3 — 2026-10-18
- **Task:** T-505 — Real-page pans to the chart
- **Tier:** 87
- **Status:** PARTIAL
- **Files:** render/goanna/BrowserPageGoanna.{cpp,h}, render/goanna/GoannaRenderPage.{cpp,h}, render/goanna/EngineCompat.h, engine patch 0046 (layout/base/nsLayoutUtils.{h,cpp}, layout/generic/nsGfxScrollFrame.cpp), tools/device/t502/pan-run.sh
- **Validation:** Build P; verify 0 structurally wrong rows on four pages with overscan; the per-page table; Acceptance: the table and the trace yes, the overscan criterion yes, Wikipedia ≥ 24 fps no (16.5-24)
- **Next:** T-506 (animation, new content against the chart); T-515 added for the frames off the main thread

### Iteration 4 — 2026-10-18
- **Task:** T-506 — Animation, new content and self-scroll against the chart
- **Tier:** 87
- **Status:** DONE
- **Files:** render/browserserver/JihadBrowserServer.cpp, render/goanna/BrowserPageGoanna.cpp
- **Validation:** Build P; device rounds 202-203; Acceptance 3/3
- **Next:** T-507 — every other card event on the GPU path

### Iteration 5 — 2026-10-18
- **Task:** T-507 — every other card event on the GPU path
- **Tier:** 87
- **Status:** DONE
- **Files:** render/goanna/BrowserPageGoanna.{cpp,h}, render/browserserver/JihadBrowserServer.cpp, render/goanna/test/enyo-selftest.js, render/goanna/test/jihad-card-events.html
- **Validation:** Build P; device rounds 204-212 (screenshots looked at); Acceptance 3/3
- **Next:** T-508 — video and Flash on the GPU path

### Iteration 6 — 2026-10-19
- **Task:** T-508 — video and Flash on the GPU path
- **Tier:** 87
- **Status:** DONE
- **Files:** build/webos-oe/push-engine-update.sh, render/goanna/test/jihad-video-styled.html, jihad-anim-opacity.html, jihad-canplay.html; build site T-499..T-501 placeholders filled
- **Validation:** device rounds 213-228 (titles, screenshots, brightness of the fade); Acceptance 3/3
- **Next:** T-509 — safety and fit

### Iteration 7 — 2026-10-19
- **Task:** T-509 — safety and fit
- **Tier:** 87
- **Status:** DONE
- **Files:** render/goanna/BrowserPageGoanna.{cpp,h}, render/goanna/GoannaRenderPage.{cpp,h}, engine patch 0047, tools/device/t502/three-daemons*.sh
- **Validation:** Build P; device rounds 229-232; Acceptance 4/4 (R1's two, R10's two)
- **Next:** T-510 — the GPU path as the default (blocked by T-504's open criterion; T-515 first)
