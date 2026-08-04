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
