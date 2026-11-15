# T-576: the 15 minute soaks (2026-11-14 and 15, Enyo, engine 0113)

`game-round.py enyo DIR --only … --seconds 450` (90 windows of 10 s; the player's resident memory read after each
window, `RSSW`). The soak line compares the median content rate of the first six windows with the last six, and the
resident memory after the sixth window with the last (cavekit-flash.md R6: both within 10 %).

| game | rate, minute 1 → end | player RSS, minute 1 → end (max) | faults | |
|---|---|---|---|---|
| Happy Wheels (heavy) | 9.8 → 10.0 | 59.5 → 62.4 MB (63.5) | 0 | **ok** (`b/`) |
| Helicopter | 18.8 → 19.4 | 24.7 → 23.9 MB (24.7) | 0 | **ok** (`b/`) |
| Bubble Tanks 2 (heavy) | 22.0 → 19.6 | 37.9 → 38.8 MB (41.3) | not read | memory ok; rate 11 % down (from minute 5 on, flat after) |
| SAS Zombie Assault | 11.2 → 7.4 | 50.3 → 52.4 MB (58.3) | 0 | memory ok; the rate is the game's cycle (a death and its menu every 60-70 s: windows of 0.4-1.4/s) |
| Desktop Tower Defense | 17.8 → 1.2 | 36.0 → 36.1 MB (36.8) | 0 | memory ok; the game was over after two minutes (its score dialog to the end): no rate to compare |
| GemCraft (heavy) | 8.8 → 4.6 | 69.2 → 64.3 MB (69.2) | 0 | memory ok; the level was lost and the map shown from minute 3: no rate to compare |
| QWOP | 12.0 → 10.4 | 34.2 → 55.5 MB | 0 | **memory grows** (below) |

Two pass whole (one heavy, one other). Four more hold their memory; their rates are not a soak's reading, because the
game on the screen at the end is not the game of minute 1 (a soak wants a script that keeps a game in one state for 15
minutes: scripts' work, open). The player lived through every soak and the daemon logged no fault.

## QWOP: the player's memory grows as the game is played (open)

- 15 minutes: 34 → 55 MB, three times (`b/`, `e/`); 30 minutes: 32.5 → 74.7 MB, a straight line of 1.4 MB a minute with
  no plateau, the rate 11 → 9 (`c/enyo.json`, `rss_w`).
- **Where**: anonymous memory only; the libraries and the two frame segments (`/dev/shm/org.chromium.*`, 1 MB each) do
  not move (`e/enyo-qwop-smaps.txt`, `--smaps`). Of the anonymous regions the engine allocator's aligned chunks stay
  (`2d400000`, `33100000`); all 21 MB is one run of unaligned blocks at `395c0000-3ac20000`, 1.7 MB at minute 1.
- **When**: started and left alone 10 minutes, flat (32.5 → 33.9 MB). Keys without the restart key, flat (32.4 → 35.1 in
  5 minutes: the runner falls once and lies there). Space alone, flat. Only whole rounds grow it: run, fall, restart.
- **Stock**: the same keys from the emulated kernel keyboard, 10 minutes each. Jihad 32.5 → 53.8 MB
  (`qwop-kernel-keys-jihad.out`); the stock browser flat at 40-42 MB (`qwop-kernel-keys-stock.out`), **but it played one
  round**: Q, W, O, P arrive, the runner falls, and neither Space nor R restarts it
  (`qwop-stock-stays-at-its-fall.jpg`, the same picture at minute 1 and at the end). So the stock browser never does
  what grows the memory here, and its flat line decides nothing.
- **Verdict: open, not a limit.** The growth sits in memory that is not the engine allocator's and comes with the
  game's rounds, which points at the player's own heap and the game's objects; the kit's rule wants the same failure in
  the stock browser or a trace into the player, and there is neither yet. It is not every restart's: SAS Zombie Assault
  died and restarted a dozen times in its soak and ended 4 % up. Next: the player's heap entry points under gdb while
  QWOP restarts.

## The tool

- `game-round.py`: the run's ceiling was `200 + 2 × seconds`; a soak takes 11.5 s a window and was cut at 1100 s twice
  for Bubble Tanks 2 and Helicopter (the `*-incomplete*.out` files: their numbers are whole but for the last three
  windows and the fault count). Now `300 + 3 × seconds`.
- `--smaps`: the player's resident memory by mapping, and its anonymous regions one by one, after the first minute and
  at the end. `rss_w` in the JSON: the memory after each window.
