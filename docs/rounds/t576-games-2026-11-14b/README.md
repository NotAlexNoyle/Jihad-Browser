# T-576: the menu games, Raft Wars' drag, Duck Life 4 against stock (2026-11-14, engine 0113)

`game-round.py enyo|mochi|mojo --only hobo-2,wonderputt,bejeweled-2 --steps` (`*.log`, `*.json`, the pictures).

| game | Enyo | Mochi | Mojo | |
|---|---|---|---|---|
| Hobo 2 | 12 of 12 windows react, 4.2/s | 12 of 12, 4.6/s | 12 of 12, 3.8/s | plays: the fight, GAME OVER, PLAY AGAIN |
| Wonderputt | 3 of 12, 7.6/s | 3 of 12, 7.6/s | 1 of 12, 5.6/s | plays: holes 1 and 2 putted in every shell (`*-wonderputt-play.jpg`) |
| Bejeweled 2 | 0 of 12 | 0 of 12 | 0 of 12 | its menu takes no tap (80 ms, 400 ms and 1 s presses), as in the stock browser: a limit |

- **Hobo 2 was a script fault, and its "limit" void.** The title's PLAY! is at 0.17, 0.54 of the movie; the script tapped
  0.12, 0.40, the title's text, in the stock round too. With the tap on the button, then EASY, it plays in every shell.
- **Wonderputt** putts on a tap towards its point; the script dragged. Its course rebuilds for seconds between holes and
  takes no input then, and the control windows before each input window see the same animated course: the reacted count
  is low where the pictures show the ball holed and the next hole built. Player alive, 0 faults.
- **Raft Wars' "lost second drag" is not reproduced.** `game-round.py mochi --only raft-wars --seconds 20`: 16 of 16
  drags reached the daemon as presses of 500-850 ms with 15-17 moves each, 4 of 4 windows react. The by-hand reading of
  an 88 ms press with no moves was one drag by hand, not seen again.
- **Duck Life 4 is the player's limit.** In the stock browser (stock-bench.sh): START, the splash left alone, PLAY, the
  speed screen, SLOW: its BrowserServer runs at 86-93 % of a core for over 40 s, is then gone, a new one started and the
  page loaded afresh (`stock-duck-life-4-after-slow.jpg`: 20 s, 41 s and 60 s after SLOW the speed screen still, then the
  loader again). Jihad's plugin hang timeout ends the player at 45 s and keeps the card.
- **A fault of the round tool, fixed.** Bejeweled 2's first replay read 4 faults: `[jihad-npapi-child] FAULT` lines of
  the morning's Duck Life 4 trials. This busybox's `tail -c +N` skips some 8 KB whatever N is, so "the log since the
  game began" was nearly the whole log. game-round.py, engine-checks.sh and push-card-js.sh now count from the end
  (`tail -c $((size - mark))`, as fingerlib does). Earlier rounds' fault and event counts were of the whole log; their
  0 faults stand (a log with none), their event counts do not.
