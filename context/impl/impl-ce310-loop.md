---
created: "2026-10-01"
last_edited: "2026-10-01"
---
# Implementation Tracking: webOS CE 3.1.0 loop (build site of 2026-10-01)

Build site: context/plans/build-site.md

Device: HP TouchPad (topaz), webOS CE 3.1.0 build 86. One device — every device task ran alone.

| Task | Status | Notes |
|------|--------|-------|
| T-201 | DONE | Housekeeping + pre-change baselines, see "Baselines" below. Enyo's leftover `MOZ_LOG=…` removed from `/etc/event.d/jihad` (`sed`, one token; `JIHAD_INJECT=1` intact in all three jobs). `/var` 12% (55.9 MB free). |
| T-202 | DONE | Engine fix `1fff8023` on `third_party/uxp` `jihad-engine-mods` (local, unpushed — T-279): `BasicPlanarYCbCrImage::CopyData` returns the base result, `true` under delayed conversion. |
| T-209 | DONE | `render/goanna/test/make-media-assets.sh` (11 clips, ffprobe-checked), `jihad-codecs.html` (per-format table), `jihad-input-echo.html`, `jihad-media.html` `#state` made fixed-width; `build/webos-oe/stage-media-pages.sh` stages them md5-verified into `/tmp/jm`. |
| T-216 | DONE | libxul rebuilt (incremental, 1 min). Stripped md5 `65e3cc84891c59e9ede40cc7b3de0802`. |
| T-219 | DONE | Engine-only push to enyo/mochi/mojo; all three read back `65e3cc84…`; daemon unchanged `cf92f351…`; goanna.js unchanged `5874eafd…`. Enyo is off the traced build. |
| T-220 | DONE | `<video>` plays to the end: 12/12 (VP8 + Theora × file + http × 3). gre-widgets R1 criterion → `[x]`. |
| T-221 | DONE | Codec table on device (jihad-codecs.html): VP8/VP9/Theora/VP8+Vorbis/Vorbis/Opus/WAV play to the end; H.264/MP3/AAC refused (no decoder). FOUND: FLAC advertised but failed — ffvpx libs (`libmozavcodec.so`, `libmozavutil.so`) are dlopen'd, were never bundled. Fixed in make-device-bundle.sh + push-engine-update.sh; libs pushed to all three (md5 `a4eaef65…`, `d65eaf82…`); FLAC plays, others unchanged. `anon video`/`anon audio` show `videocontrols[1]` (binding attached). Error-overlay screenshots archived in `evidence-2026-10-01/`. gre-widgets R1 codec box → `[x]`. |
| T-222 | DONE (agent part) | Play/pause both directions via inject `click` (adapter clickAt entry): pause t=11.370 held 3 s; resume → 17.65. Kit box stays `[~]` for the finger (T-285). |
| T-223 | DONE | `packaging/prefs/jihad-platform-prefs.js` media block rewritten to the measured state (comment-only + unchanged pin). gre-widgets R7 first box → `[x]`. Reaches the device with the next goanna.js push (T-225). |
| T-224 | DONE | R10 on media: stock tunables untouched (read back 95/200000 before and after). 640x360@24 VP8: 648–1188 MHz, 473/0 frames; 320x240: 384–1188 MHz, 0 dropped. Decision: do not extend the boost. device-build R10 box → `[x]`. |
| T-226 | MOOT | Conditional on T-224 finding drops at the clock floor; it found neither. Nothing to build. |
| T-203 | DONE | Idle tick: `Main.cpp` arms the frame tick while a card has a page and a 1 s tick otherwise; `JihadBrowserServer::setWakeHook` re-arms on connect; inject polled on every idle tick; no stats window across an idle stretch. |
| T-204 | DONE (code) | No tick-stats line with no page or < 20 ms work per 2 s; `log_cap_cb` every 30 s, copy-then-truncate at 2 MB (`JIHAD_LOG_MAX` test knob), one kept generation; numbers in the job comment + `packaging/README.md`. Device verification: T-227. |
| T-205 | DONE (code) | Blank settle: 500 ms after load done a persisting blank frame is published once; refused blanks retried on a 100 ms grid. Device verification: T-228. |
| T-206 | DONE (code) | Confirmed from source: postinst wrote the role then started the daemon with no hub rescan, and the daemon registered exactly once. Fixed both ends: `ls-control scan-services` in postinst before `start`; registration retry with backoff (5→60 s, ~11 min) in `JihadLunaService.cpp`. Real reinstall: T-231..T-234. |
| T-211 | DONE | `g_message("jihad: flashGestureLock -> …")` at the single transition point in the adapter. |
| T-213 | DONE | PUA swallow extended to 0xE0E2–0xE0E7; `JIHAD_LOG_KEYS` logs non-printing key triples only (printable keys are never logged). |
| T-215 | DONE | One log line per frozen settings command; `render/goanna/test/jihad-settings.html` (click-opened popup, script marker, cookie, min-font). |
| T-217 | DONE | ARM daemon built, YAP contract OK (73 commands, 58 messages). Stripped md5 `3e29f045cf0f9bb392ef96f8eb8a7139`. |
| T-218 | DONE | PDK + crosstool adapters and three `.ipk`s (enyo 1.0.4, mochi/mojo 1.0.0); each payload verified: libxul `65e3cc84`, daemon `3e29f045`, ffvpx libs, `libasound.so.2`, goanna.js `01154654`, postinst carries the hub rescan. |
| T-225 | DONE | Daemon push ×3 (md5 read back). Idle: 0.05% CPU, ~1 wakeup/s, 0 log bytes per variant; inject answers `no page`; connect latency unchanged; re-idle on card close, same PID. device-build R8 idle box → `[x]`. |
| T-207 | DONE | `build/webos-oe/r9-device-probes.sh` (build / mutex / closure); PDK-linked dynamic `mtx_dump_pdk` built (NEEDED libpthread.so.0, libc.so.6). Device run: T-245. |
| T-208 | DONE | The three toolchain recipe files are tracked. `UXP-Jihad` `jihad-engine-mods` head = `bd49435b` = parent gitlink = OE SRCREV (checked with `git ls-remote` over https). Today's engine fix `1fff8023` is local only → push + gitlink/SRCREV bump is T-279 (needs the user's credential; no push this session). |
| T-210 | DONE | `app-mochi/PARITY.md` refreshed: own-service URIs, `selectPopupMenuItem`, home control, stored shortcuts, settings entry, toast, corrected dialog note. Human sign-off stays T-291. |
| T-227 | DONE | Log bounded: 0 bytes in a 603 s idle window ×3; two in-lifetime rotations on PID 15840 at a 64 KB test cap (1,400,842 then 66,193 bytes), `.old` exact, daemon kept logging. Job restored (no `JIHAD_LOG_MAX`), restarted. device-build R8 log box → `[x]`. |
| T-228 | DONE | Blank settle: about:blank 4 renders → settled at 558 ms → 0 paints/30 s; blank→plain first paint 0.78 s; slow load 12 suppressed blanks, 0 published; late-content page paints after settling. offscreen R8 box + R3 box → `[x]`. |
| T-229 | DONE (measured, NOT idle) | Card on a static page, display on: 12.3% of a core (main thread; tick `work avg 0–1 ms` × ~205 ticks/2 s). engine-embedding R3 stays `[~]`; fix is T-292 (quiet tick with a card connected), batch 2. |
| T-237 | DONE | TLS capture (run ahead of its lane slot — independent of the install cycle): howsmyssl `"tls_version":"TLS 1.3"` rating Improvable (no session tickets; no insecure suites; 21 suites); Cloudflare trace `tls=TLSv1.3 http=http/2`. browser-services R5 TLS box → `[x]`. Readback method: `jsurl` puts the extracted field in `document.title`, `title` prints it (gettext truncates the JSON). |
| T-230 | DONE | CE snapshot `out-audit/ce-3installed-pushvariant.txt` (audit script now also md5s Atlas's files). Atlas identity columns, read off the device: MIME `application/x-atlas-browser`, shim `BrowserAdapterAtlas.so`, socket `/tmp/yapserver.atlas`, jobs `atlas` + `atlas-sensord`, Luna `org.webosports.browserserver`, app `org.webosports.app.atlas` — none equals a Jihad column. |
| T-231 | BLOCKED (permission) | Removing a variant (prerm + payload `rm -rf`) was refused by the session's permission gate as irreversible deletion. Not attempted another way. Needs the owner's go-ahead. |
| T-232/T-233 | PARTIAL | Supported-path installs (ipkgservice) done for enyo, mochi, mojo — but as install-OVER. Luna registered on first start ×3 (Enyo from a hub with the role hidden). All three now ipkg-registered. Fresh-from-nothing and the reboot remain (blocked on T-231). |
| (found) | FIXED | Upgrade crash: postinst `cp` over the mapped adapter shim → LunaSysMgr SIGSEGV at the next card launch (seen once). postinst now replaces shim + impl by rename; verified on three upgrades. New kit criterion in device-build R4, `[x]`. |
| T-238 | DONE | Scrubber: stuck `0:00` bubble reproduced on `<audio>`, cause = engine damage rect excludes the bubble above the bar; daemon now grows the damage box by 32 CSS px (no plugin on page). Bubble tracks 0:04/0:10/0:15; click-seek works (audio 3.5→13.2 s, video 4.9→30.9 s). gre-widgets R1 controls-render + scrubber boxes → `[x]`. Engine-side overflow union tried, no effect, reverted (rebuild reproduced md5 `65e3cc84`). Daemon now `73cbc3a2dbc37b4446de955a92f2cd14` on all three. |
| T-239 | DONE (agent part) | Volume widget: mute + slider change `muted`/`volume`; pulse sink-input follows (100→54/96/78%, 0% muted); codec level untouched. T-241 (engine volume wiring) is MOOT. Kit box `[~]` for the finger (T-285). |
| T-292 | DONE | Quiet tick: frame period → 100 ms after ~0.2 s with no busy page and no card command; commands wake it from `pageFor()`; inject polled by the clock; tick-stats line only when the loop painted or a tick ran ≥40 ms. `JIHAD_QUIET_TICK_MS` (0 disables). Daemon `2786380985b928f05480419b6141aa6b` on all three. |
| T-293 | DONE except Flash | Static page 2.2% (was 12.3%), 11.5 wakeups/s; clicks ≤20 ms; video 149/0; 1 Hz timer page: max 216 / mean 65 ms late, 9.3% CPU. Flash histogram check rides the plugin sweep (T-247). engine R3 stays `[~]` until then. |
| T-242 | DONE | Toast in all three shells (public-bus `clearCookies` → `(pushed)` → screenshot). gre-widgets R5 toast box → `[x]`. Mojo toast overlapped by the command row → `bottom: 84px` in `jihad-toast.js`, pushed with `push-card-js.sh mojo --no-sysmgr`; the card still runs the cached script until LunaSysMgr restarts (not re-photographed). |
| T-244 | DONE (agent part) | Cert store on CE: lib unchanged (md5 `9414761f…`, links 0.9.8); self-signed endpoint → ssl-confirm (ordinal 18, PEM written) → Trust Once over the FIFO → `CertInitCertMgr rc=0`, serial trusted, reload → HTTPS page loads. Download md5 matches. Two daemon fixes: retry the FAILED url after trust; suppress the engine's connection-error alert when a cert error is pending. Daemon `24a902eb5f08a464288d1375f615f899` on all three. browser-services R5 CE box → `[x]`; device-build R4 flows stay `[~]` (Downloads-app look, T-280). Residue: three `jihad-testfile*.bin` in `/media/internal/downloads`, session serials 1–3 in the platform store (swept at daemon teardown/startup). |
| T-245 | DONE | R9 device probes: glibc 2.8 PROCESS_SHARED mutex `__kind=128`, lock(128)=0 — same as 2.23, trap was a 2.5-proxy artefact. Flash closure: 67 libs resolve on CE, libflashplayer md5 unchanged; entry points listed; `libpowerd.so.0` (shm + sem_init) named as an armed surface. device-build R9 box → `[x]`. |
| T-246 | DONE | Plugin sweep (Enyo, CE): `profile/plugins/` was EMPTY after the CE install — stock `libflashplayer.so` copied in by hand (test setup; nothing ships it). Loads, renders (screenshot), mouse events reach it, SIGSEGV of plugin-container leaves the daemon up, boost 40/50000 taken and 95/200000 restored. Not run: the control plugin, about:plugins text readout (`gettext body` returns the page's script), plugin key input. |
| T-247 | DONE (agent part) | Histograms A/B/C on CE (see addons kit R7 table). FOUND: shipped build ran the 8 ms tick for plugins (the 16 ms tick was a device-local job edit). FIXED: `jihadPluginTickPeriodMs()` = 16 while a plugin is on a page. A2 (no env) = 32.6 fps, 366/724 gaps in 32–39 ms, deferred 0. Daemon `46d3714d430162502b3630cf63d4854b` on all three. Device left on the SHIPPED job (no pacing env). B-vs-C is T-287 (human). |
| T-293 | DONE | Flash part closed by T-247. engine-embedding R3 → `[x]`. |
| T-235 | DONE (install half) | Atlas coexistence: identity columns distinct; stock + Atlas md5 identical after three ipkgservice installs; both browsers serve; stopping either daemon leaves the other on the same PID; no cross-connect. Removal half blocked with T-231. device-build R4 Atlas box → `[~]`. |
| T-248 | DONE | Add-ons sweep on CE (Enyo): about:addons renders and lists `Jihad Effect Test 1.0`; row buttons Disable/Enable/Remove work via `clickid anon:richlistitem[value="…"]\|…-btn` (quote the attribute value; select `category-extension` first); effect (magenta body) off/on/off accordingly and each state survives stop/start; removal stages then commits at restart; `extensions.json`/`prefs.js` flushed by SIGTERM; nothing in mochi/mojo profiles or `/media/internal`. |
| T-249 | DONE | XPI sweep on CE: InstallTrigger good → card prompt ("Allow … to install add-on: Jihad Effect Test") → accept over FIFO → `notify … installed. (pushed)`; mismatch → `-210`, toast photographed, nothing installed (addons R3 box → `[x]`). FOUND + FIXED: direct navigation / plain link to an `.xpi` did nothing — `amContentHandler.js` patched (engine commit, local), file pushed to all three `components/`; prompt now appears for both. The installed-toast was pushed but not photographed (the card's own prompt was still in front). |
| T-250 | DONE | Prefs sweep, three variants (CE): panes, alias, bare URL renders; page writes → that variant's `prefs.js` only; values survive a card relaunch (`setBlockPopups 1 / setAcceptCookies 1 / setMinFontSize 2` re-sent, page values kept); `<select>` → `popupMenuShow`; no overflow at 768 and 1024. FOUND + FIXED in the daemon: (1) no `change` after a daemon edit → typed settings never saved; (2) `<input type=number>` edits went to the anonymous inner field (`insert: … type=[text] … inDoc=0`); (3) `ScrollTo` was a `javascript:` URL → engine never scrolled on chrome pages / script off, taps below the fold hit `<null>` (Mochi's 655 px window showed it). Harness notes: click coords are DOCUMENT px × zoom; the card zooms to 1.0 when a field takes focus; Enyo cards share one process, so `close` by processId closes all of them. |
| T-251 | DONE | Page's own write of `javascript.enabled=false` + NAVIGATE → `JS:OFF` (so 2026-08-05 = "did not navigate"). Real defect: card relaunch re-sent `setEnableJavaScript 1` to the user branch → undone. |
| T-253 | DONE | Condition fired. Daemon: card value → default branch + `SyncScriptGate()` at every `BeginLoad`; JS row restored in `preferences.js`. Three variants: off → `JS:OFF`; survives card relaunch and daemon restart; page still works with script off; on → line removed. |
| T-252 | DONE | Frozen commands driven by the SHIPPING Enyo card via db8 merge (`luna-send -a <appid> palm://com.palm.db/merge`, the card watches the kind): handler lines + effects (`LOADPOPUP blocked/attempted`, `COOKIE kept/refused`, `JS ON/OFF`, min font `h=7/23`). No hook, nothing to revert. |
| T-254 | DONE | Mojo adopts page edits (start page re-opened with the edited shortcut after one Back). FOUND + FIXED: `location.hash =` pushed a history entry per edit; Back handed the card an older payload and it adopted it. Now `location.replace`. |
| T-255 | DONE | Echo page: `input` reaches plain / textarea / controlled; `change` (new) on blur / tap elsewhere / Enter; number field holds its value. Caret keys with the RIGHT codes (57504–57507) all work; daemon side is 0xE0A1=Up, 0xE0A0=Down. |
| T-214 | DONE | Both 2026-08-15 findings were the test: `57346` is 0xE002 not 0xE0A2 (inserted an invisible PUA glyph); the "dropped first word" tap landed elsewhere because the card had zoomed to 1.0 on focus. The inject `key` path calls `keyDown` and reaches the keycode map. |
| T-257 | N/A | Conditional did not fire (the `input` event is delivered). |
| T-278 | DONE | Four settings from the page, each shown on a real page and in `prefs.js`, three variants (daemon `e5701758…`): popups, min font 20 (`h=23`), cookies Never via `popupsel` (`COOKIE:refused`), JavaScript (`JS:OFF`); all restored. |
| T-212 | BUILT | Adapter: latch engage → card `editorFocused(true)`, clear → HTML editor state. On device (impl swapped), not exercised — needs a real double tap (T-289). |
| (new) | DONE | `window.open` → new card: `PageChrome` implements `nsIWindowProvider`; `msgCreatePage(id)`; the card connecting with `id` gets the URL. Adapter re-sends connect state when the identifier arrives after connect (Mochi). Three variants verified (tap with blocker on; load-time with blocker on = nothing, off = card). |
| (new) | DONE | Mojo: pages without a viewport tag drew as a 76x99 thumbnail (daemon sent `metaViewport is=0.1` for them; Mojo WebView applies it). Daemon now sends the message only for a declared viewport and only on change; adapter ignores a 0x0 window. |
| T-256 | DONE | Input sweep on CE (`jihad-input-sweep.html`): tap hit-test, `detail=2` + one `dblclick`, checkbox/radio one `change` per tap, ASCII + symbols + `éß漢😀`, selection replace, Backspace/Delete by code point, Enter (textarea newline / form submit), Tab, link tap at the 0.78 fit zoom. FOUND + FIXED: key events to pages carried raw webOS codes (keydown `57507`), no keypress charCode, Ctrl/Alt swapped → `jihadMapKey` in `GoannaRenderPage::KeyEvent`. |
| T-258 | DONE (agent part) | VKB session, three pages: keyboard raised each time (screenshots), top field does not move, dismiss restores. FOUND + FIXED: a field low on a tall page stayed UNDER the keyboard (engine did not scroll) → daemon scrolls it 16 px above the keyboard and tells the adapter. Finger half = T-282. |
| T-261 | N/A | Condition not met: `sinceResize` 94–129 ms, not single-digit. |
| T-260 | DONE | CE `libWebKitLuna.so` vs 3.0.5: `.text` byte-identical, two `.rodata` strings differ → touch struct unchanged. Touch page: one finger start/move/end, two fingers `n=2`. |
| T-262 | DONE | `about:config` end to end (warning, filter typed, row toggled by Enter, in `prefs.js` after stop, toggled back by double tap); add-on row buttons; Escape closes the tools menu. |
| T-264 | DONE | Offscreen R1–R6: size tracking, exact colours and orientation by pixel, buffer rotation over 8 frames, geometry/scroll/viewport messages. |
| T-265 | DONE | `<select>` → card list ×3 with real options; tools menu opens/composites/closes/activates. FOUND + FIXED: popup taps were zoom-divided twice (wrong row at the fit zoom); the menu was not painted until other damage arrived (repaint on popup open/close). |
| T-266/T-267/T-268 | DONE | Shell sweeps ×3 (`jihad-shell.html`): start page, stored shortcuts, launch target on own socket, select list, alert held, find 1/0, failed-load panel, 302 followed. FOUND + FIXED: Mojo ignored a launch `target`; find hit was white on white (selection colour prefs). |
| T-269 | DONE | Cookie + HTTP disk cache survive a restart (`200` then `304`); guardrail fires at a raised threshold; public `clearCookies`/`clearCache`; `confirm`/`prompt`/`alert`. FOUND + FIXED: HTTP auth raised no dialog at all → `nsIAuthPrompt2` implemented; verified ×3 (cancel → 401 page, credentials → `AUTH-OK`). |
| T-270 | DONE | On install-started daemons ×3: `luna: serving`, public `clearCookies` (Enyo jar 20 → 0), PDK adapter load + paint, pref flush on stop, no fault report. |
| T-271 | DONE | `hello14` runs (`c++201402`); `readelf -A` v7/VFPv3/NEON = CPU; nav back/forward/reload flags ×3. |
| T-272 | DONE | 64 MB simple page; eight sites → 126 MB (peak 131 < 150 cap), 0 OOM lines; `freeze` −17 MB. |
| T-274 | DONE | The 2026-08 finding does not hold on CE: LunaSysMgr hot-plugs the uinput keyboard as a Bluetooth keyboard and its keys reach the card. Raw codes measured (Up `0xE0A0` … End `0xE0B2`, Delete `0x7F`). FOUND + FIXED: daemon had Up/Down swapped; Home/End added. Unexplained: later runs lost Up (and sometimes Left/Right) to LunaSysMgr. `render/goanna/test/uinput_kbd.c`. |
| T-275 | DONE | Seven adapter-deps libraries pulled with the written procedure; sha256 = manifest; paths confirmed on CE. |
| T-263/T-276 | DONE (three times) | Batch-2 `.ipk`s built and installed through ipkgservice ×3, payload md5s read back; repeated twice more as the daemon changed; the device runs the last set (daemon `50615b82…`). LunaSysMgr pids unchanged across all installs. |
| T-277 | DONE | Re-runs after fixes: static page 1.7% CPU, clicks ≤ 20 ms, video 149/0, Flash 32.5 → 33.3 fps. |
| (found) | FIXED | Damage rect was used unscaled at a fit zoom (control plugin clipped at 0.78; Flash box inexact) → scaled into buffer px. New criterion in offscreen R8, `[x]`. |
| T-240 | DONE | Volume HUD: daemon streams sit on `pcm_output` (sink 0), not a Palm role sink; `PULSE_SINK=pmedia` does not move them. Not needed: `KEY_VOLUMEDOWN`/`UP` from the kernel keyboard during page audio changes audiod's media volume (70 → 59 → 70), the stream follows, and the HUD is on the screenshot. gre-widgets box `[ ]`→`[~]` (physical rocker = T-286). |
| (new) | DONE | `build/webos-oe/prefs-row-walk.sh`: all 56 table rows of `about:preferences` changed through their own control and read back from the engine; real key path (kernel keyboard) types into a settings field. preferences-ui R2 page side met; box stays `[~]` for T-290. |
| (new) | DONE (experiment) | `JIHAD_GRE_KEYS` (default off): engine's editor takes a focused field's keys. Typing, arrows, Home/End, Backspace, Delete work and nothing crashes; engine Enter in a textarea inserts nothing; mixed model double-fires `change`. Enter/Tab kept on the daemon path. gre-widgets R3 box `[ ]`→`[~]`. |
| (2026-10-02) | — | **Owner go-ahead and decisions.** Remove the three card rows; Flash from the device's own folder; build every Mochi parity omission. Removal audit and engine push approved but refused again by the session's permission gate → the owner runs them (staged: `/tmp/rm1.sh` on the device; `git -C third_party/uxp push fork jihad-engine-mods`). |
| (found) | FIXED | Closing any card unregistered the add-on install prompt and the JS notification door for the daemon's life (`SetDialogSink(nullptr)` in every page destructor dropped once-per-process observers): `InstallTrigger` → download → cancel, `-210`, no prompt. Now dropped only at shutdown; a closing page steps down only if it owns the dialogs. Verified after 3 connects / 2 disconnects. `7191edaa`. |
| T-290 / T-294 | DONE | Card rows removed (Enyo `Preferences.js`, Mochi `JihadPreferences.js`); stored copies no longer re-applied; "More Settings" opens the page. |
| T-295 | DONE | Daemon scans `/media/cryptofs/apps/usr/lib/BrowserServerPlugins` after the profile folder. Mochi (no profile copy) plays the test movie. |
| T-296 / T-309 | DONE | `plugin.state.flash` (default 2) + page row; Luna `getFlash` / `enableFlash` / `disableFlash` (setters: own card only — `luna-send` and another variant's id refused). Card switch real in Mochi and Enyo. |
| T-297 | DONE | Mochi bookmark edit (stored as typed). FOUND + FIXED: bookmarks list always empty (ordered by un-indexed `date`); hidden panels never laid out. |
| T-298 | DONE | History groups (14/14 bucket cases on host; 7 groups on device). |
| T-299 | DONE | Confirmation overlay before each clear (panel + history view). |
| T-300 | DONE | Clear History purges the engine's history too; controls disabled at once; engine `goBack` goes nowhere. |
| T-301 | DONE | Search engine choice (card store); address bar uses it. |
| T-302 | DONE (agent part) | Share sheet (Email, Text Message, Copy Address, Add to Launcher). Clipboard copy and `addLaunchPoint` verified; app launches by finger. |
| T-303 | DONE | Certificate details via `<appdir>/certinfo/<pem>.js` (certificate manager unreachable for a card on CE; XHR and `/var` refused). |
| T-304 | DONE | Engine downloads tracked from the adapter's download callbacks; retry re-drives the address. |
| T-305 / T-306 | DONE | Thumbnails: Luna `saveThumbnail` → 192x144 PNG of the next real paint into `<appdir>/thumbnails/`; adapter path untouched (it writes only under `/var`, `/tmp`). |
| T-307 | NOT NEEDED | Long-press image actions use the system download manager (writes into the downloads carve-out); the daemon's image stubs stay stubs. Limit: `data:` images cannot be saved. |
| T-308 | DONE (agent part) | Long-press menu via `eventFired` (hold on link/image claimed, tap and plain hold not). Open in New Card, Copy, Save Image verified; a real hold, Share Image and Set as Wallpaper are for a finger. |
| T-310 | BLOCKED (hardware) | Print: no printer reachable from the device. |
| T-311 / T-312 | DONE | Batch 3: daemon `78fb52e8…`, goanna.js `97f981b3…`, libxul `65e3cc84…`; three `.ipk`s installed through ipkgservice; LunaSysMgr pids unchanged. |
| (2026-10-05) | — | **Owner direction.** Printing in depth; Share Image and Set as Wallpaper on the device; why `/` is read-write; the Atlas fault report; the hands-on checklist deferred. The owner ran the engine push and the Mochi removal. During the session: redraw two Enyo toolbar icons; learn to close the USB alert after a Luna restart. Evidence: `evidence-2026-10-05/`. |
| T-318 | DONE | Gitlink and OE SRCREV at `9dedcd5fe5` (commit c726e8a7). |
| T-321 | DONE | The device HAS a print stack (`printmgrd`, `com.palm.printmgr`, HP's wprint) and it SEES the owner's Epson: `_resolve_reply: not HP printer and doing printer filtering, address 192.168.1.253, ID "EPSON ET-2800 Series"`. It discards every non-HP printer. The printer speaks IPPS and takes JPEG. |
| T-324 | DONE | `components/jihadPrint.js`: mDNS browse from port 5353 (the engine's own DNS-SD asks from an ephemeral port and the device firewall drops the direct answers), typed address, Get-Printer-Attributes probe, page drawn as JPEG sheets with `drawWindow`, IPP Print-Job per sheet, TLS first with the printer's own certificate accepted for the session. Luna `findPrinters` / `printPage`, own card only (refusals measured). `test/ipp-sink.py` keeps what it is sent. Device: 3 printers found in every search; 1- and 3-sheet pages arrive whole; ~0.3 s draw + ~0.8 s encode per sheet. |
| (accident) | — | A test meant as a dry run printed one full dark sheet on the owner's Epson (job 31, completed). Cause: T-326. Printing is tested against the sink first from now on. |
| T-326 | DONE (found) | The engine's compiled-script cache (`$APP/cache/startupCache`) is keyed by path and survived both a hand push and an upgrade, so replaced scripts kept running in their old version. The daemon now fingerprints the bundle (`jihad-bundle.id` + 54 stats, 23–110 ms) and drops the cache when it changed. Verified by hand (marked copy) and on real upgrades (`bundle CHANGED … dropped`). New criterion in device-build R4, `[x]`. `push-engine-update.sh` now also pushes this project's chrome scripts. |
| T-325 | DONE | `JihadPrintDialog` in both shells (Enyo: in place of the HP-only system dialog; Mochi: new menu entry). Device, through each shell's own handlers: the Epson ET-2800 and a Mac-shared ET-2720 listed; sink found by typed address; Enyo sent 1 sheet, Mochi 3; dialogs end on "Sent N page(s) to …". mochi-ui R2 Print `[ ]`→`[x]`; PARITY has no open row. Not covered: paper from the dialog seen by a person. |
| T-319 | DONE | Mochi installed from nothing through ipkgservice after the owner's removal: postinst rescanned the hub, daemon from its own deviceroot, socket, `luna: serving` with 0 FAILED, public-bus calls answered, 3 db8 kinds, fresh profile; audit diff shows only its own files and an unchanged stock/Atlas block. The launcher learned the app only after `ipkgservice/rescan` (Preware's own post-install call). Removal left one empty directory in the installer's package registry (recorded in R8; audit script now lists it). |
| T-322 | DONE | From a read-only `/`: ipkgservice remounts `/` read-write before running the package script and does not restore it; our scripts report it and leave it as found. Lasts until reboot. R8 note. |
| T-323 | DONE | The 2026-10-02 report is a SIGBUS in Atlas's own allocator thread at shutdown, nothing of ours mapped. Atlas 0.9.12 and a Jihad card rendered side by side today; no new report. Atlas criterion `[~]`→`[x]` (stock + Atlas checksums identical across removal and install). |
| T-320 | DONE | Long-press sheet → Set as Wallpaper changed the system wallpaper (`19.jpg` → `jihad-test.png`, "Wallpaper set."), then restored; → Share Image fetched the picture and opened Email (no account on the device: set-up screen). Driven through `eventFired` and the sheet's row handler — there is no touch device to inject into. Box stays `[~]` for the real hold. |
| T-327 | DONE | Home: a house at the stock glyphs' size and finish. Bookmarks/History: built like Share and New Card (2 px outlined clock as the body, ribbon as the accent), then with the circle left whole after the owner found the cut-out ring "incomplete". `packaging/branding/make-enyo-toolbar-icons.py` draws both. |
| T-328 | DONE | `luna-restart.sh`: `luna-send -m com.palm.lunasend -s …/storaged/MSMAvail {"mode-avail":false}` closes the system UI's USB alert (the hub forwards signals only from `com.palm.*` senders), sent 5 s after the system UI's own cable check; 32 s per restart instead of 75. `dev-sh.sh`: runs a device script and returns when it ends. |
| T-329 | DONE | All three on bundle `a4a2f8e2…`: libxul `65e3cc84…`, daemon `889263013e40…`, print component `44ce382c…`; each daemon from its own deviceroot, each answering on the public bus. |

## Baselines (T-201, daemon `cf92f351…`, before any daemon change)

- **No-card idle, 62 s window:** mochi 148 CPU ticks (2.39% of a core), 7244 voluntary context
  switches (117/s), log +2970 B (48 B/s). mojo 146 ticks (2.35%), 7246 switches, +2970 B.
  Tick statistics line: `tick period ms min=8 avg=8 … n≈239` every 2 s.
- **Card connected, static page (enyo), same window:** 291 ticks (4.7%), log +2970 B.
- **about:blank:** 663 full paints in 30 s (measured the same morning, docs/PICKUP.md 2026-10-01).
- **Connect → first non-blank `painted`, static page, 3 runs:** 1.04 / 1.12 / 0.90 s
  (launch → `client connected`: 2.04 / 2.19 / 3.45 s). ~0.1–0.2 s polling granularity.

## Device facts found on the way (CE 3.1.0)

- **`/` is mounted read-write** (`/dev/mapper/store-root on / type ext3 (rw,…)`). On 3.0.5 it
  was read-only at rest. Not yet known whether CE ships it rw or an earlier session left it so;
  matters for device-build R8's "rootfs back to read-only" box (postinst keeps the state it found).
- `org.webosinternals.ipkgservice` is NOT at `/media/cryptofs/apps/usr/palm/services/` nor
  `/usr/palm/services/`; `org.webosinternals.preware` IS installed as an app. Locate the service
  before the fresh-install cycle (T-230/T-231).
- Other CE apps present: `org.webosarchive.atlas-default-browser`, `org.webosarchive.open-in-atlas`.
- **The display was OFF and the device LOCKED** for the first hour of this session
  (`com.palm.display/control/status` → `"state":"off"`); `takeScreenShot` then returns the stale
  lock screen — three "different" screenshots with the same md5 were the tell. Woken with
  `display/control/setState {"state":"unlock"}` + `setProperty {"onWhenConnected":true}`.
  The engine runs page timers, decodes and paints with the display off (the `<video>` matrix was
  taken that way; a later run with the display on agrees), but ANY pixel evidence needs it on.
- Inject coordinate spaces, measured: `click x y n` and `move x y` take CARD pixels (CSS px ×
  page zoom) and go through the input queue; `clickoff dx dy n <id>` / `rect <id>` take a bare
  element id (or `sel:<css>`), CSS px, and bypass the queue — `clickoff` on the video play button
  returned `ok=1` and did not toggle it. `gettext` truncates long text (~330 chars).
- **A `launch` in the first ~minute after an `ipkgservice` install is silently dropped**
  (`returnValue:true`, no card). Wait a minute before concluding a card does not connect.
- The adapter shim loads its impl from a fresh `/tmp/jihad-<v>-adapter-XXXXXX` copy (unlinked) and
  hot-reloads a changed impl when no card of that variant is open; LunaSysMgr's maps are the way
  to see which build is loaded. The shim itself is only reloaded by a LunaSysMgr restart.
- `ipkgservice/install` returns to `luna-send -n N` after N messages while the install is still
  running; poll for the daemon before reading results.
- After a LunaSysMgr restart a system "Connected — USB Drive" dialog sits over everything and the
  card behind it is not recomposited: screenshots show a frozen, dimmed card. Dismissed by closing
  the two extra `com.palm.systemui` windows (`applicationManager/close` on their processids).
- A stale, disconnected card of the same app (left over from a daemon restart) is what
  `takeScreenShot` captures if it is in front — a dim scrim with a spinner. `fresh-card` pattern:
  close every card of the app, launch one, wait for `load done`, then test.
- `takeScreenShot` takes ~2 s; budget for it when timing against playback.
- Cards can be closed remotely: `applicationManager/running` → `processid`, then
  `applicationManager/close {"processId": …}`. Used for the connect-latency baseline.
