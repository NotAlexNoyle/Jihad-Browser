# The finger round — every test that needs the owner's hands, deferred to one sitting

The owner, 2026-10-12: "defer all finger tests in the entire project for one large round at the
end". Everything an agent can verify on the device is verified as the work goes; what only a
person at the TouchPad can confirm is collected here, and nobody is asked for it piecemeal. Add
to this list as work lands (one line: what to do, what should happen, the kit criterion); mark the
criterion in its kit [~] with "finger: deferred round". Tick items off here when the round is run,
and close the kit criteria from the results.

Run each item in all three variants (Enyo, Mochi, Mojo) unless it says otherwise.

## Scrolling and gestures
- [ ] Flick up and down on about:settings and about:personas, at both ends: the page coasts and
  slows as a web page does, never stops dead. (chrome-look R3)
- [ ] about:addons → Extensions (or Plugins) with more add-ons than fit: the list pans to its end.
  (chrome-look R2)
- [ ] A real drag-scroll and a two-finger pinch on an ordinary page; pinch in then out on
  en.m.wikipedia.org (Mojo: the card is never left grey). (input-bridging R3/R4, mojo-ui R6)
- [ ] Rotate the TouchPad: portrait ↔ landscape redraws the page right. (device-build R4,
  offscreen-rendering R6)

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
- [ ] Every button in each card's chrome: back, forward, home, reload/stop, padlock, share, new card,
  bookmarks/history; every app-menu entry; Mojo's new-card / share / history. (device-build R4)
- [ ] Hold on text, on a field, on a link and on an image: the long-press menu offers the right
  entries and each does what it says; a plain tap is unaffected. (ui-shell R7, mochi-ui R2,
  mojo-ui R6)
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
- [ ] After choosing a proxy, the section redraws by itself, with no touch. (Known open: after
  choosing Riverstone Relay the card kept its old picture until the next touch, Enyo and Mochi;
  build site T-465.)
- [ ] The Riverstone Relay for everyday browsing once its pass-through rule is on the server:
  https sites load with their own certificates. (R2)

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
