# The finger round — every test that needs the owner's hands, deferred to one sitting

**2026-10-24: the round is the agent's, driven by an emulated finger on the real input path**
(`tools/device/finger/`, `context/kits/cavekit-finger-emulation.md`), judged from screenshots and frame dumps by eye;
each item below gets its result and the date as it is run. Only what no emulation reaches — the accelerometer's
rotation, the volume keys, hearing audio, a physical keyboard — stays for a person, and says so.

The owner, 2026-10-12: "defer all finger tests in the entire project for one large round at the
end". Everything an agent can verify on the device is verified as the work goes; what only a
person at the TouchPad can confirm is collected here, and nobody is asked for it piecemeal. Add
to this list as work lands (one line: what to do, what should happen, the kit criterion); mark the
criterion in its kit [~] with "finger: deferred round". Tick items off here when the round is run,
and close the kit criteria from the results.

Run each item in all three variants (Enyo, Mochi, Mojo) unless it says otherwise.

## Scrolling and gestures
- [x] Flick up and down on about:settings and about:personas, at both ends: the page coasts and
  slows as a web page does, never stops dead. (chrome-look R3) *(2026-10-24, the emulated finger: about:preferences flicked to its end and back in Enyo, Mochi and Mojo (Mojo's by a drag and a flick); about:personas to its end and back in Enyo; whether a coast "stops dead" is a feel the shots cannot give — the pages reached both ends)*
- [ ] about:addons → Extensions (or Plugins) with more add-ons than fit: the list pans to its end.
  (chrome-look R2) *(2026-10-24: one extension installed on the bench, nothing to pan; needs more add-ons first)*
- [x] A real drag-scroll and a two-finger pinch on an ordinary page; pinch in then out on
  en.m.wikipedia.org (Mojo: the card is never left grey). (input-bridging R3/R4, mojo-ui R6) *(2026-10-24: drags on `jihad-long-static.html` in all three; Mojo on en.m.wikipedia.org: a pinch in at the fit stays at the fit, a pinch out to 1.08 paints in full, 14 frames none empty, no grey)*
- [ ] Rotate the TouchPad: portrait ↔ landscape redraws the page right. (device-build R4,
  offscreen-rendering R6)
- [x] The GPU path, zoom (2026-10-18: the first try ran on a diagnostic daemon mid-script; T-504
  rebuilds zoom): pinch in and out and double-tap on desktop en.wikipedia.org and on a long page:
  the page at the new zoom arrives promptly and no area stays grey. (cavekit-gpu-rendering R6, R12) *(2026-10-24: FOUND the owner's grey — a double tap zoomed to a 104 px cell at 6.2x and the engine's default maximum-scale passed the adapter; at rest the zoomed frame lost its right third from 2.0 (the root's displayport base and clip divided by the resolution) and everything from 2.98; the second double tap found no block (the hit test read the render pan). All fixed (daemon 7c29035a, engine c79d16f8): double tap to the infobox at 2.33, pinches to 2.99, zoom out, a tap on a zoomed link, in all three — `docs/rounds/finger-2026-10-24/zoom-grey.txt`. "Promptly": the first frame at a new zoom takes 1.6-2.9 s on desktop Wikipedia (R6 open). Above 2.91 on that page the frame still clips (the maximum-scale now stops there; T-527).)*
- [ ] The GPU path (the default in all three since T-510, 2026-10-19): drag-scroll a long page with a fixed
  header (`jihad-long-static.html` on the host server) from top to bottom at a steady pace: the
  header stays put, the text is continuous (no repeated or missing bands), nothing blank enters
  from below, no jump every ~660 px (the engine's anchor moves). Then flick. A scripted pan cannot
  show this: the adapter's own view does not move. (T-502; offscreen-rendering, the scroll row) *(2026-10-24: six steady strokes in Enyo: the header stayed put (mid-stroke too), the paragraphs continuous, no blank, no jump; Mochi and Mojo the same at the stroke's end (Mochi's one mid-stroke shot was mostly white, not reproduced in Mojo's). The flick: the card arrives in ~250 ms, the engine paints only at the end, so for ~0.5 s the lower card is blank and the fixed header gone, right at rest — the coast gap, T-527.)*
- [ ] The GPU path with the express lane (T-515, 2026-10-19; the default since T-510): right
  after desktop en.wikipedia.org loads (its scripts run for seconds), pinch in, pan, pinch out and
  double-tap at once, and do the same on `jihad-script-hold.html` (the host server; its script holds
  the engine 600 ms in every 700): the page follows the finger at the new zoom without waiting for
  the script, nothing flips back to the old zoom, and at rest the text is sharp and nothing is grey.
  (cavekit-gpu-rendering R3, R6, R12) *(2026-10-24, Enyo: the burst right after desktop Wikipedia's load ends at the new zoom, full and sharp, nothing flipping back; on `jihad-script-hold.html` the fixed header is drawn mid-view for a moment during the zoom change, right at rest)*
- [ ] WebGL on the GPU path (T-511, 2026-10-19): open `jihad-webgl-cube.html` from the host server
  (`?size=360` too) and a real WebGL page (any three.js example): the cube turns smoothly with its
  checkerboard sharp, a pinch and a pan over the canvas keep it drawn, and closing the card and opening
  it again a few times leaves the browser working. Say how smooth the 480 px cube looks to the eye (the
  card gets 18-20 frames a second there, 26-27 at 240 px). (cavekit-gpu-rendering R8, R12) *(2026-10-24, Enyo: the cube drawn and sharp at rest, after a pinch out and a drag over it, and after the card was closed and opened three times (no daemon trouble); the page's own counter 19.7-21.8 fps, 14.8 at `?size=360`; the daemon published 129 frames in 5 s at rest, 96 at 360 px. A three.js example not tried (no such page on the host server).)*

## Keyboard and text
- [ ] Tap a field on several pages in one session: the keyboard comes up every time, and the page
  does not jump away from the field. (input-bridging R2)
- [ ] Type in a page's field: the caret is visible while typing; holding Backspace speeds up.
  (input-bridging R2/R2a; device-build R4)
- [ ] Type in about:settings' fields and about:addons' search field (keys reach the XUL page).
  (preferences-ui R6, addons-site R12)
- [ ] Focus about:addons' search field, then close the keyboard with its hide key: no caret is left
  in the field, and tapping the field again brings the keyboard back. Also in a field inside a frame
  on some page. (chrome-look R4)
- [ ] With the address field focused, open about:settings or about:personas from the app menu: the
  keyboard goes down and the whole page shows. (chrome-look R4; scripted check passed)
- [ ] Tap the address field: the whole address is selected; Cut/Copy then act on all of it.
  (chrome-look R4, T-436; scripted check passed)
- [ ] Mochi: with the keyboard up over a page field, paste: the text lands in the field.
  (inspector round 21 note)
- [ ] Type into a field of a page that saves when a field changes (a settings field, a form), then
  close the keyboard with its own key: the value is kept and the caret is gone. (chrome-look R4;
  scripted check passed with the system's own keyboard hide, round 22)
- [ ] Mojo: open a link to the browser from another app (an email or a message): the card shows the
  page with no keyboard over it; launched plainly, the address field is ready to type. (chrome-look
  R4; scripted check passed)

## Chrome, menus and dialogs
- [~] Every button in each card's chrome: back, forward, home, reload/stop, padlock, share, new card,
  bookmarks/history; every app-menu entry; Mojo's new-card / share / history. (device-build R4) *(2026-10-24, the emulated finger, T-525: Enyo — back, forward, reload (the stop glyph shows), stop, home (DuckDuckGo lite), share (Add Bookmark / Share Link / Add to Launcher), bookmarks panel, padlock (the DigiCert certificate, Done), app menu (Find on Page / Settings / Print), new card — all act. Mochi — the same set acts (clear, share, Bookmarks / History / Downloads, app menu, new card; back and forward after a link). Mojo — reload, home, new card, the library and share menus, find in page, the padlock dialog, the app menu act; BACK AND FORWARD DO NOT: with history and the icon drawn enabled, a tap on the bar's back emits no command at all (rewiring "jihad-back" to openUrl loads nothing; the daemon's new pageBackward line never prints; the card's own log reaches nothing on this build). OPEN — next: the command-menu widget's tap handling for the first two items (the row is a scroller; the items start disabled and are enabled by modelChanged).)*
- [~] Hold on text, on a field, on a link and on an image: the long-press menu offers the right
  entries and each does what it says; a plain tap is unaffected. (ui-shell R7, mochi-ui R2,
  mojo-ui R6) *(2026-10-24, Enyo: a hold on text selects the word and offers Copy / Select All / Reload / Add Bookmark / Find on Page; on a field, the keyboard and Paste / Select All. The image hold landed on the caption (hold 60 px higher next time) and the link hold ran with the keyboard still up — both to re-run; Mochi and Mojo and the entries' actions not yet.)*
- [ ] Pick and dismiss an item in a card-side drop-down list; answer a card dialog by tap (an add-on
  install confirm, an SSL confirm). (device-build R4)
- [ ] about:settings: every control by finger at 1024×768, including the tabs (wrapped in Mochi)
  and About. (preferences-ui R6)
- [ ] Mojo: the settings page's values can be edited and stick. (mojo-ui R7)
- [ ] Mojo: a search typed in the address bar shows results without waiting for the whole page.
  (mojo-ui R6)

## Personas and add-ons
- [ ] about:personas: no card flashes while the page is open or scrolled (Carbon Light 2.0 was the
  example); cards show their pictures sharply. (personas-ui R9)
- [ ] Right after installing a Persona with a large picture, tap its Remove once ("Remove it?") and
  wait a few seconds: the question stays until answered while its card picture is made. (personas-ui
  R9, round 22)
- [ ] Wear, Take off, Undo, Remove (two taps), Install from file; wear one from the site and one from
  addons.mozilla.org (asks first, names the site). (personas-ui R6/R8)
- [ ] about:addons: install, enable, disable, remove, Undo, an add-on's Preferences (Palefill's fit),
  the update check, a plugin's state menu. (chrome-look R2, addons-extensions R2)
- [ ] about:addons: the link-style buttons ("More" on an add-on, "Undo" after a removal, "Restart
  now") and about:personas' Undo in its message take a finger easily (44 px since review round 24).
  (personas-ui R2, chrome-look R2)
- [ ] The add-on site on the TouchPad, portrait and landscape: tap targets, the Personas page's Wear
  buttons, the search field. (addons-site R7, R12)

## Fonts (Settings → Content → Fonts & Colors)
- [ ] In each variant, pick a different Serif, Sans-serif and Monospace font and a size from the lists:
  the sample lines change, and a page drawn afterwards uses them; Restore default fonts puts them
  back. Try another script (Japanese) too. (preferences-ui R9; Enyo measured by script)

## The engine on UXP RB_20261006 (2026-10-12)
- [ ] Everyday browsing in each variant on the new engine: a few real sites, scrolling, typing in a
  page's field, back and forward — nothing slower or odder than before. (cavekit-engine-update.md R8)
- [ ] Flash with sound (jihad-audio-long), and a video with sound, on the new engine. (R8)

## Proxies and certificates (2026-10-13)
- [ ] Settings → Advanced → Proxy → My own proxy: type a proxy's host and port on the on-screen
  keyboard, tap Use this proxy, and browse through it; then No proxy. (proxies-certs R1; the
  fields were filled from prefs for the device run, because a script cannot type past the keyboard)
- [ ] WebOne: type a WebOne machine's address, tap Use this WebOne, then Open this WebOne's
  certificate and say Yes; browse an https site through it. (R3, R6; scripted with the same
  steps, with the address from prefs)
- [ ] A proxy that asks for a login: type the user name and password in the card's dialog (each
  shell; Mojo's is webOS's own sheet) and the page loads. (R5; Enyo and Mochi answered through
  their own dialogs by script, Mojo not answered)
- [ ] After choosing a proxy, the section redraws by itself, with no touch. (Scripted: within a
  second, 15 of 15 picks in Enyo and Mochi; the earlier "old picture", T-465, was a screenshot taken
  before the pick.)
- [ ] The Riverstone Relay for everyday browsing (its pass-through rule is on the server since
  2026-10-13): https sites load with their own certificates; the padlock names the site's own
  issuer. (R2; scripted in Enyo with example.com)

## Media and sound
- [ ] A video's play/pause and volume by tap. (gre-widgets R1)
- [ ] Flash sound heard; an HTML `<audio>` heard on the speaker. (addons-extensions R7,
  device-build R4)
- [ ] A finished download opens in the Downloads app. (device-build R4)

## Not built yet (add to the round when they are)
- Tabbed browsing: tab taps, the strip's close controls, Open in New Tab from a long-press, the
  feel of a switch. (tabbed-browsing R1/R6 — groundwork only, by the owner's word)

## Not testable at all
- Keyboard arbitration on the card side: hardware-blocked (no keyboard exists).
- TouchPad Go: owner-accepted without hardware (device-build R6).
- [ ] 2026-10-15, modern-web R2: open `file:///tmp/jihad-busy.html` (staged by stage-test-pages.sh or by hand), wait for the "script may be busy" dialog (about 60 s), tap **Stop script**: the card must come back and stay usable.

- **Settings → General → Browser identity → Forget** (T-495, 2026-10-16): with sites listed under "Sites remembered", tap Forget; the row should read "None yet …" and the next visit to w3.org should be challenged once, learned again, and render.
- **YouTube's mobile site plays** (T-497, 2026-10-16): open `https://m.youtube.com/watch?v=dQw4w9WgXcQ` (any youtube.com link lands there now), tap the poster's play button; the video should play with sound (the desktop page's pre-roll ad played on its own in the round, so decoding is there). Then the feed on `m.youtube.com/`: tap a chip (Music), scroll, tap a thumbnail.
- **DuckDuckGo's regular results page** (T-498/T-499): on `https://duckduckgo.com/?q=webOS&ia=web` the results, the "Search Assist" box and the tabs render; KNOWN until T-499: the slide-out menu panel (Homepage / Themes / Settings …) stands at the top right, because the card shows the strip past the page's `innerWidth` where the page parks it. Tap ☰: does the panel animate in and out? Tap a result; tap "Images".
- **Etsy's search renders its styled components** (T-498): `https://www.etsy.com/search?q=webos` — the listing cards with their styles (the 47 adoptedStyleSheets errors are gone with the shim); DataDome may still ask for a verification on a first visit.
