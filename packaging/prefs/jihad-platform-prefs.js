// Jihad Browser — platform prefs (what this DEVICE is). Copyright 2026 NotAlexNoyle. MPL-2.0.
//
// THE single source of these values: appended to goanna.js by BOTH the device bundler
// (build/webos-oe/make-device-bundle.sh) and the desktop build (build/desktop/build-goanna.sh),
// exactly as packaging/prefs/jihad-addon-prefs.js is. The sharing is the point — prefs that
// only existed on one side is how the desktop loop ended up unable to exercise features that
// worked on device, and vice versa.
// --- JIHAD platform prefs ---

// W3C TOUCH EVENTS: ON, not autodetected.
//
// The engine default is 2 = autodetect (modules/libpref/init/all.js), and its own comment says
// autodetection "is currently only supported on Windows and GTK3". This build is
// cairo-headless: there is no GTK, so autodetect finds no touch device and settles on OFF.
//
// The result is a browser that reports no touch support on a machine that has no other kind of
// input. Measured 2026-08-04: a page with touchstart/touchmove/touchend listeners received
// NOTHING from a fully wired multi-point touch dispatch — the events were suppressed before
// they reached the document. Anything that feature-detects `'ontouchstart' in window`, and
// every touch-driven site, silently takes its no-touch path.
//
// 1 = enabled unconditionally. That is the honest answer for a TouchPad, and it is also right
// for the desktop harness, whose whole job is to stand in for the device.
pref("dom.w3c_touch_events.enabled", 1);

// --- about:preferences rows that the GRE honors but ships no default for -----------------
// Both are read directly by platform code with a fallback, so they WORK with no browser app
// above the engine — they simply have no default pref entry, which made about:preferences
// show them as "not available in this build". Shipping the upstream defaults makes the rows
// live without changing behaviour.
pref("privacy.donottrackheader.enabled", false);   // necko sends DNT when true
pref("places.history.enabled", true);              // Places records visits when true

// --- date/time inputs: OFF, deliberately (cavekit-gre-widgets.md R2) ---------------------
// `dom.forms.datetime` defaults TRUE in this GRE, which makes <input type="date"> render as a
// date field whose picker is a XUL popup — a separate display root, the same class as the
// <select> dropdown and the about:addons tools menu. Measured on device 2026-08-05: tapping a
// date field opened NO popup (popups=0 before and after), so the field looked editable and was
// not. R2 names that outcome the worst of the three available, so the feature is turned off
// rather than left half-working: with this false the input degrades to a plain text field,
// which the VKB and the engine's own editing keys already handle.
// To revisit: route the picker card-side the way <select> is (cavekit-ui-shell.md R5), or make
// the popup composite (cavekit-offscreen-rendering.md R7); then flip this back and re-measure.
pref("dom.forms.datetime", false);
pref("dom.forms.datetime.timepicker", false);   // already the default; pinned so it cannot drift
pref("dom.forms.datetime.others", false);       // month/week — same story, same reason

// --- prefs that back an about:preferences ROW: SHARED, not device-only -------------------
// Moved here 2026-08-10 (T-111, cavekit-gre-widgets.md R7) out of the inline low-RAM heredoc in
// build/webos-oe/make-device-bundle.sh, which only the DEVICE build ever appended. Everything
// below is read by a row in packaging/prefsui/content/preferences.js, so a value that exists on
// one side only makes about:preferences a different page on each — the exact drift R7 forbids.
// The rest of that heredoc (surfacecache, JS GC water marks, media cache,
// max-persistent-connections-per-server, bfcache viewers, initialpaint delay, storage.synchronous)
// backs NO row and DELIBERATELY stays device-only: those are 512 MB / ARM numbers, and a desktop
// harness that GCs at a 32 MB heap and caps its surface cache at 32 MB is not standing in for the
// device, it is simulating a memory ceiling the host does not have.
//
// READ FROM SOURCE, NOT MEASURED — no device and no build was involved in this move, and two
// things in the kit's own wording were WRONG when checked against the tree:
//   * ELEVEN prefs back rows, not nine. Taken by intersecting the heredoc's pref names with the
//     `pref:` keys in preferences.js, so it is a complete list rather than a sample.
//   * Only THREE of the eleven ever rendered as "not available" on desktop — `prefExists()` is
//     `getPrefType() != PREF_INVALID` (preferences.js), and only these three have no default
//     anywhere in the GRE (grepped `pref("<name>"` across third_party/uxp):
//     general.smoothScroll (the GRE ships only its `.mouseWheel`/`.pixels`/… sub-prefs — the
//     master bool is a Firefox app pref), browser.sessionhistory.max_entries, and
//     browser.cache.memory.capacity, whose default is COMMENTED OUT at all.js:71.
//     The other EIGHT do have upstream defaults, so their rows were never disabled on desktop —
//     they silently showed the STOCK value instead. That is the quieter and more misleading half
//     of the same bug: a row that reads "not available" at least announces itself.
//
// These are the TouchPad's values on purpose, on both sides. The desktop build's job is to stand
// in for the device — the same argument dom.w3c_touch_events.enabled is set on above — and a
// harness running a 256 MB disk cache with an uncapped refresh driver cannot reproduce what the
// device does. The stock value each one overrides is named so the next reader can see the delta
// without opening all.js.
pref("general.smoothScroll", false);                 // no smooth-scroll compositing (no GRE default)
pref("browser.sessionhistory.max_entries", 20);      // bound history depth (no GRE default)
pref("browser.cache.memory.capacity", 16384);        // 16 MB in-RAM HTTP cache (no GRE default)
pref("layout.frame_rate", 30);                       // cap the refresh driver at 30 Hz (GRE: -1)
pref("image.animation_mode", "once");                // animate GIFs once, not forever (GRE: "normal")
pref("network.dns.disablePrefetch", true);           // no speculative DNS (GRE: false)
pref("network.prefetch-next", false);                // no speculative page prefetch (GRE: true)
pref("network.http.max-connections", 32);            // fewer sockets = less buffer RAM (GRE: 256)

// Disk cache. 50 MB is isis's own CacheMaxSize — the number the people who shipped this browser
// on this hardware chose — and smart-sizing stays OFF because cache2 sizes itself from FREE
// space, so a 10.2 GB partition would claim GBs. WHERE the cache lives is not decided at a pref:
// see render/goanna/JihadRuntimePaths.h and the long comment in make-device-bundle.sh for the
// cryptofs/VFAT reasoning, which is device reasoning and stayed there.
pref("browser.cache.disk.enable", true);
pref("browser.cache.disk.capacity", 51200);          // 50 MB (GRE: 256000)
pref("browser.cache.disk.smart_size.enabled", false); // never autosize from free space (GRE: true)

// HAZARD for whoever edits this file next, and it is asymmetric between the two builds.
// The device bundler wipes its whole $OUT and re-copies goanna.js every run (`rm -rf "$OUT"` at
// the top of make-device-bundle.sh), so its `grep -q 'JIHAD platform prefs'` guard can never be
// true and an edit here always lands. The DESKTOP build appends into
// out/obj-jihad-goanna/dist/bin/goanna.js in place under the same guard, so if a `mach build`
// does not rewrite that file, an edit to THIS file is silently not re-appended. Check for the
// value, not the marker, before concluding the desktop dist has what you just wrote.

// --- HTML5 media: <video> STAYS ON. The hole is AUDIO, and no pref can express it --------
// (cavekit-gre-widgets.md R1 last criterion + R7. Decided 2026-08-10 from source; NO DEVICE.)
//
// Device-measured 2026-08-10 and recorded on R1: WebM/VP8 video PLAYS (readyState=4, 160x120,
// currentTime advancing), H.264/MP4 is not offered, MP3 is refused outright (error.code=4).
// Video is VIABLE, so a blanket <video> disable would be WRONG. Nothing is turned off here —
// this block is the "working, with the reason recorded" half of R7.
//
// media.webm.enabled is already the default; it is pinned because it is the ONE media path
// that works, and a future trimming pass must not be able to take it silently. media.mp4 is
// deliberately left alone: H.264 is refused by the decoder chain regardless (ffvpx is built
// with CONFIG_H264_DECODER 0, config_components_audio_video.h:127) and that refusal IS the
// behaviour we want.
pref("media.webm.enabled", true);

// WHY <video> NEEDS NO "DEGRADE VISIBLY" WORK FROM US: the GRE already does it and the assets
// ship. videocontrols.xml's "error" branch (:612-632) sets statusIcon type="error", picks the
// label off video.error.code in updateErrorText (:694-726) — MEDIA_ERR_SRC_NOT_SUPPORTED
// renders "Video format or MIME type is not supported." — shows the status overlay, and fades
// the control bar OUT when no frame was ever shown. Present in build/webos-oe/device-bundle:
// videocontrols.xml, videocontrols.dtd (the error.* entities are there, so the anonymous
// content parses at all), skin/classic/global/media/videocontrols.css and error.png.
// SOURCE-READ ONLY: that the binding attaches and composites is R1's first two criteria,
// still open.
//
// THE HOLE, and a pref cannot close it. videocontrols suppresses the error overlay for
// audio-only content BY CONSTRUCTION: setupStatusFader forces show=false when isAudioOnly
// (:336-339), and "loadedmetadata" sets isAudioOnly=true for any <video> whose videoWidth or
// videoHeight is 0 (:524-531), after which dynamicControls (:451-461) force-shows the control
// bar. So an audio-only resource — <audio>, or a <video> pointed at one — renders exactly the
// "blank box with working-looking controls" R1 forbids.
//
// The second half of this paragraph used to read "and no engine audio can EVER play: cubeb_init's
// backend dispatch table is empty with both backends configured out (media/libcubeb/src/cubeb.c
// :114), so AudioStream::Init returns CUBEB_INITIALIZATION_ERR (dom/media/AudioStream.cpp
// :337-341)". THAT IS NO LONGER TRUE OF THE BUILD, as of 2026-08-15: the ARM build now compiles
// cubeb_alsa.c and links libxul against libasound, so the dispatch table has an entry and
// AudioStream::Init has something to open. It has NOT been heard on hardware yet — no device was
// connected that day — so the honest status is "the output path is built, not yet demonstrated".
// See context/impl/impl-audio-backend.md and the mozconfig note below.
//
// TURNING AUDIO OFF AT A PREF IS NOT AVAILABLE — the codec prefs are CONTAINER-scoped, not
// track-scoped. media.wave.enabled and media.flac.enabled could go (audio-only containers),
// but media.ogg.enabled also carries Theora video and media.webm.enabled carries the VP8 that
// works, and audio/ogg + audio/webm are offered with no output check whatsoever
// (OggDecoder.cpp:27-30 and WebMDecoder.cpp:29-32 read only the pref; AgnosticDecoderModule
// .cpp:26-46 advertises Vorbis/Opus/Wave/Theora/VPX regardless). Flipping the two that CAN go
// would leave audio/wav refused and audio/ogg accepted without closing anything, so nothing
// is flipped. RULED OUT, not overlooked.
//
// THE FIX WAS ONE BUILD FLAG, AND IT IS DONE — 2026-08-15. build/webos-oe/mozconfig.goanna-arm
// passed --disable-alsa, and its stated reason ("Headless render daemon: no audio output needed
// (offscreen render only)") was STALE: offscreen rendering has nothing to do with audio output.
// It now passes --enable-alsa, with alsa-lib staged into the cross sysroot from Debian-Jessie
// armel libasound2/libasound2-dev 1.0.28-1. media/libcubeb/src compiled cubeb_alsa.o for the
// first time and libxul.so imports snd_pcm_open. No pref changed, exactly as predicted.
//
// WHAT IS STILL OPEN, and it is why nothing here is marked done: the backend is BUILT, not HEARD.
// Nobody has played a sound through the engine on hardware. Two things could still fail on the
// device and neither is visible from the build — which libasound the daemon binds at runtime,
// and whether ALSA "default" opens under our bundled glibc. Both are analysed, with the fallback
// for each, in context/impl/impl-audio-backend.md.
//
// SEPARATELY, AND NOT FIXED BY THIS: the missing DECODERS. ffvpx is compiled with
// CONFIG_MP3_DECODER 0, CONFIG_AAC_DECODER 0 and CONFIG_H264_DECODER 0, so audio/mpeg and
// H.264 stay refused however well the output path works. The backend only helps the formats
// that already decode — audio/ogg, audio/webm, audio/wav, audio/flac (Vorbis/Opus/Wave/FLAC
// via AgnosticDecoderModule, plus ffvpx's FLAC). Do not read "audio works now" as "MP3 works".
