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
