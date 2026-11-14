# T-576 drag games on engine 0112, Mochi (2026-11-14)

`game-round.py mochi` over the six games the 11-13 round read as far under stock (`mochi.log`); `mochi-coalesce.log` is
the same with queued moves coalesced in the plugin child (dropped: it never found a move behind another, no gain).

| game | 11-13 (fac2b0f814) | 11-14 (0112) | stock 11-13 | same screen as stock? |
|---|---|---|---|---|
| Raft Wars | 0.6/s, 3 of 12 windows changed | 3.8/s, 12 of 12 | 24.2 | **no**: stock ends on the level-1 intro |
| Ragdoll Cannon 2 | 0.6, 0 changed | 2.4, 12 | 21.0 | **no**: stock ends on the main menu |
| Paper Airplane | 4.6 | 4.6 | 18.2 | **no**: stock ends on the title screen |
| Effing Meteors | 4.4 | 4.4 / 5.0 | 11.0 | yes |
| Indestructo-copter | 5.0 | 7.6 / 6.4 | 14.4 | yes |
| Motherload | 15.0 | 14.0 | 29.4 | (keys) |

`stock-vs-mochi-play.jpg`: each game's last picture, stock left, Jihad right. The round's start taps and drags put Jihad
into play in Raft Wars, Ragdoll Cannon 2 and Paper Airplane, and stock not: the stock browser has no gesture lock, so
its drags pan the page, and its rate is a menu's animation. Those three verdicts against the stock bar are void;
there is no stock reading of their play.

What 0112 fixed: the daemon no longer waits for each move's Flash frame, so a drag no longer stops the card's frames
(Raft Wars 0.6 → 3.8/s, every window changing). What is left is the player's own speed in play (Raft Wars ~4 frames a
second while dragged, 9-14 untouched) and, where the screens match, half the stock rate: rendering speed, parked with
the optimisation rows until the browser is done.

By hand (Mochi, the round's points): the first drag reached Flash as 20 moves and panned the game's camera; the drag
back came as a press of 88 ms with no moves from the card. The test page does not show it (`flash-input-kbdrag`, two
drags after the keyboard, passes in all three shells): Raft Wars only, not yet explained.

## Super Mario Flash: the round's keys, not Jihad (closed)

The 11-13 round read 0 of 12 windows reacting in Jihad against 12 of 12 in stock. Both play pictures show Mario on the
same map square: stock's "reaction" was not Mario moving (its arrows reach no Flash movie at all, T-575's notes; and
with no gesture lock its nub drags reach the page). The keys reach the movie in Jihad (`palm key … raw=146 handled=1`;
the AVM1 test movie reads keydown 39 and 38 from the nub). The map's start square has a path only downwards, and the
round never pressed down: by the kernel keyboard (new `hold CODE MS` in uinput_kbd), a held right moved nothing, a held
or a 30 ms down moved Mario, up brought him back. With `down, left, space, right, right, up` (flash-games.json):
6 of 6 windows react, 13.0 frames a second, Mario plays level 1 (`mochi-super-mario-flash-windows.jpg`, `mochi-mario.log`).
