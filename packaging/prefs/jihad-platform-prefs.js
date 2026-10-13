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
// Selection colours. There is no native theme in this build (headless widget), so the engine
// takes them from its stand-in palette — which has no entry for the TEXT-selection colours and
// answers white for every one it does not list: selected text was white on white. Measured on the
// device 2026-10-01: a find-in-page hit drew as a solid white box with the word gone, in all three
// shells. These are the stand-in palette's own `highlight` / `highlighttext` pair for a normal
// selection, the stock green for the "attention" selection find-in-page uses, and the stock
// magenta for highlight-all.
pref("ui.textSelectBackground", "#3399ff");
pref("ui.textSelectForeground", "#ffffff");
pref("ui.textSelectBackgroundDisabled", "#b0b0b0");
pref("ui.textSelectBackgroundAttention", "#38d878");
pref("ui.textHighlightBackground", "#ef0fff");
pref("ui.textHighlightForeground", "#ffffff");

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

// --- HTML5 media: ON, because it WORKS — measured on the device, per format ------------------
// (cavekit-gre-widgets.md R1 "codec reality" + R7. Device-measured 2026-10-01 on webOS CE 3.1.0
// with render/goanna/test/jihad-codecs.html; this block replaces the 2026-08-10/08-15 text, which
// said "VP8 plays" before it did and "no engine audio" after there was.)
//
//   plays to the end   VP8/WebM, VP9/WebM, Theora/Ogg, VP8+Vorbis/WebM (picture and sound),
//                      Vorbis/Ogg, Opus/Ogg, WAV, FLAC
//   refused cleanly    H.264/MP4, MP3, AAC — canPlayType "" and MEDIA_ERR_SRC_NOT_SUPPORTED at
//                      load, because ffvpx is built with CONFIG_H264_DECODER 0, CONFIG_MP3_DECODER 0
//                      and CONFIG_AAC_DECODER 0 and the device has no system libavcodec to fall
//                      back to. A missing DECODER, not a defect, and the refusal is the behaviour
//                      we want: nothing is advertised that cannot play.
//
// So nothing media-related is turned off here, and this block is the "working, with the reason
// recorded" half of R7. Three things made that true, none of them a pref:
//   1. <video> past the first second — an engine fix (BasicPlanarYCbCrImage::CopyData reported
//      failure for a successful copy under delayed conversion; every clip died with
//      MEDIA_ERR_DECODE after 10-60 frames until 2026-10-01).
//   2. Audio output — the cubeb PULSE backend (mozconfig --enable-pulseaudio), with the daemon
//      routing its stream to the speaker at the system media volume (context/impl/
//      impl-audio-backend.md). The ALSA backend that preceded it was policy-muted on this platform.
//   3. FLAC — ffvpx (libmozavcodec/libmozavutil) is dlopen'd at runtime and was built but never
//      SHIPPED until 2026-10-01, so FLAC was advertised ("maybe") and failed every load. It is
//      the one format with no other decoder. make-device-bundle.sh now bundles both files.
//
// media.webm.enabled is already the default; it is pinned so a future trimming pass cannot take
// the main video path silently. media.mp4 is deliberately left alone: H.264 is refused by the
// decoder chain regardless, and that refusal is correct.
pref("media.webm.enabled", true);

// WHY AN UNDECODABLE CLIP NEEDS NO WORK FROM US: the GRE degrades visibly and the assets ship.
// videocontrols' error branch shows its status overlay with the label picked off
// video.error.code ("Video format or MIME type is not supported.") — seen on the device for an
// H.264 <video> and, since the 2026-08-16 videocontrols change that stopped suppressing the
// overlay for audio-only content on a real error, for an MP3 <audio> too.
//
// THE CODEC PREFS ARE CONTAINER-SCOPED, NOT TRACK-SCOPED — recorded because it is why a pref was
// never the answer to an audio problem: media.ogg.enabled carries Theora as well as Vorbis and
// media.webm.enabled carries VP8, so audio cannot be turned off without taking video with it.

// FLASH ON/OFF (cavekit-addons-extensions.md R7, owner decision 2026-10-02). One stored value:
// 2 = on, 0 = off. It is given a DEFAULT here for two reasons: the settings page only offers a
// row for a pref that exists (a row for an unknown pref is shown inert), and the card switch
// reads the same value through the variant's Luna service. The plugin itself is not shipped —
// the daemon scans the device's own plugin folder — so with no Flash installed this is a
// switch for something absent, and the card says so rather than hiding it.
pref("plugin.state.flash", 2);

// PROXIES AND webOS's CERTIFICATES (cavekit-proxies-certs.md, the owner, 2026-10-13).
// No proxy by default. The engine's own default, 5 ("use the system proxy"), reads GNOME's
// settings, which this headless build does not have, so it went direct while saying otherwise;
// webOS's own proxy setting is a choice on the settings page (jihad.proxy.mode "webos"), which
// components/jihadProxy.js follows.
pref("network.proxy.type", 0);
pref("jihad.proxy.mode", "none");          // none | webos | relay | webone | manual | pac
pref("jihad.proxy.webone.host", "");
pref("jihad.proxy.webone.port", 8080);
// The Riverstone Relay's CA: trusted only when chosen, and only while the Relay is in use (R2).
pref("jihad.proxy.relay.trust_ca", false);
// Trust the CA certificates webOS has that HP's image did not (a community update, Certificate
// Manager), other than ones Mozilla removed (R4). components/jihadCerts.js.
pref("jihad.security.webos_certs", true);
