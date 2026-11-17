# T-576: the stock browser's minute of each game, and why games in play ran at a third of it (2026-11-16/17)

`stock.json`, `stock.log`: `game-round.py stock` over the 37 games (the stock browser under `stock-bench.sh`), the bar
R3 and R6 hold a game to. `enyo|mochi|mojo.json` and `report.md`: the three shells after the two fixes below
(engine 0114, the daemon of commit `8d47b966`).

## What the stock round says

- Where both browsers stand on the same screen the stock browser drew two to three times the frames in play:
  Achievement Unlocked 23.2 a second against 6.8 (20-22 in the later single runs), Hobo 2 14.8 against 4.0, Effing Meteors 12.2 against 3.8. At rest
  (title-round, 2026-11-06) the two were equal, so it is play, and sound, that costs.
- The stock reading is no bar where the stock browser never left a menu: its page has no gesture lock, so a drag
  pans its page and the keyboard's place is the page's (Ragdoll Cannon 2, Paper Airplane, Raft Wars' level card; the
  frames `stock-<game>-play.jpg`). Those games are held to the software path's minute, as R3 says.
- A changing rectangle is not a game: Run 3 reads 8.6 changes a second in the stock browser and its frame is one
  colour (`stock-run-3-play.jpg`, black), as it is here: blank in both, a limit as recorded on 2026-11-13.
  Fantastic Contraption and Samorost's intro read one colour at the end there too.

## Found and fixed

1. **The level scripts** (the daemon, `BrowserPageGoanna.cpp`). While a movie's sound stream was open the audio
   thread ran `audio-vol.sh` every 400 ms (a luna-send and three amixers; an amixer is 70 ms of CPU, it reads ALSA's
   whole configuration) and `flash-vol.sh` (`pactl list`) every second: 23-26 new processes a second against the
   stock browser's 7-9, the device's system time 60-65 % of a core, and the player left with 50 % of one. Now one
   subscriber to `com.palm.audio/media/status` sets the level at each change (`audio-track.sh`, started when a sound
   starts, ended when it stops or its daemon is gone), one amixer every two seconds holds the amp, and `flash-vol.sh`
   runs only while the pulse watch of T-657 does not stand. After: 10-11 new processes a second, system 38 %,
   a third of the device idle.
2. **A second tick before every draw** (engine 0114, `PluginInstanceChild.cpp`). The glib pump that runs before the
   draw event ("so the frame is current") ran a whole further tick of the movie when the draw had been asked for by
   the movie's own tick: 76-94 ms in it per draw.

Achievement Unlocked 6.8 → 12-16 a second (the player's own count, no frame reader running), Effing Meteors 3.8 →
10.2 (stock 12.2), Hobo 2 4.0 → 7.8 (stock 14.8).

## Open: the player's share of a core

After both, a tick of Achievement Unlocked is 65 ms of wall time, all of it inside the player's own frame (one glib
iteration; the child's readout "the loop"), with 41-47 ms of CPU in it; the stock browser's frame is 45 ms with 38 ms
of CPU. The player's main thread reads as runnable 94 % of the time (pcsample) and is given 63-70 % of a core
(84-87 % in the stock browser) while each core is a fifth idle. Ruled out, each by a run: its nice level (0 and -3:
the same 64 %), the frame reader (off: the same), the movie's quality (low, medium: the same), timers (usleep and
poll are exact to the millisecond on this kernel), what stands on its stack (stacksample: the player itself, 84 %).
Not ruled out: what the two cores' other work is in each browser at the scheduler's scale (4000 context switches a
second here, 3100 there; this kernel has no schedstats), and the player's own threads' locks (pthread_spin_lock and
pthread_cond_timedwait stand in the samples here and not in the stock browser's).

A caution on the two shares: the stock browser's thread is its BrowserServer's main thread, which carries WebKit's
page work beside the player's; here the thread is the plugin's own. The like-for-like numbers are per frame: 45 ms
of wall and 38 ms of CPU there, 65 ms and 41-47 ms here.

## The tools this left

`game-round.py --threads` keeps `<shell>-<game>-threads.txt`: the player's share of a core in every window (PCPU),
every thread's CPU with its system part, the device's time by kind and by core, new processes and context switches
(TT, `cputop.c`), the player's program counter running and waiting (PC, `pcsample.c`; `JIHAD_STACK=1` for
`stacksample.c`). Experiments by environment: `JIHAD_FB_OFF=1` (no frame reader: the child's own "palm draw" count is
the reading), `JIHAD_FB_NICE`, `JIHAD_PLAYER_NICE`, `JIHAD_NO_LOCK`, `JIHAD_GAME_QUERY='?quality=low'`.
`thread-top.sh` is gone: on a busy device its two readings took three seconds and most of a core each, and it
reported a thread at 178 % of a core.
