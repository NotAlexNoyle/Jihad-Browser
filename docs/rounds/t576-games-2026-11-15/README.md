# T-576: the 37 games in the three shells (2026-11-15, engine 0113)

`game-round.py enyo|mochi|mojo` over the whole list (`enyo.log`, `mochi.log`, `mojo.log`, the `.json` beside each;
`report.md` is `game-report.py`'s table). The player lived through every game but Duck Life 4 (its limit) and the
daemons logged no fault.

- **Input reaches the game**: 15 games react in 12 of 12 windows in all three shells; nine more in 10 or 11 somewhere
  (a window where the game showed a menu or a death). Lower: Alien Hominid 9/8/11, GemCraft 9/10/7, Happy Wheels
  5/9/7, Motherload 9/8/12 (Enyo/Mochi/Mojo): their scripts meet menus and level ends, to be read with step shots.
- **Limits, as before**: Bejeweled 2, Duck Life 4, Plants vs Zombies, QB Typing, Run 3, Samorost's intro, Learn to
  Fly 3.
- **The report's verdict is fail for every game that plays**, on one line: the content rate under the authored bar
  (Helicopter 19.6/s of 27, Bubble Tanks 2 23.6 of 49.5). Drawing speed, parked with the optimisation rows.

## Found: Enyo lost the tap after the one that engaged a movie (fixed)

Fancy Pants read 0 of 12 in Enyo, twice (`fp/`), 10 of 12 in Mochi and Mojo: its runner never moved. It is the one
keys game whose first touch is the double tap that asks for the keyboard. In Enyo that double tap arrived as one tap:
the adapter logged a single pen down, no keyboard came, and the keys went nowhere. Any second tap within two seconds
of the engaging one was lost; later taps arrived.

Cause: `flashGestureLockChange` runs on enyo.WebView's inner BasicWebView and calls that object's own
`showFlashLockedMessage`, the framework's modal Popup; `JihadWebView.js` replaced the method on the wrapper only
(2026-11-08), so the Popup still came and its scrim took the next tap. Fix: the method is bound onto the inner view,
as the download callbacks are. After it: two pen downs, the smart-zoom request, the keyboard up, the runner jumps.

## Open: a script's tap on a sponsor's link

Wonderputt's start taps open Kongregate's page in every shell. Enyo and Mochi open a new card and the game plays on
under it (read as 1 and 0 of 12: the round reads the front card); Mojo pushes a scene on the same card, as it does
for any new window, and the round ended after one window. Learn to Fly 3 in Mojo, the same (a limit already). The
script's points, not the browser.
