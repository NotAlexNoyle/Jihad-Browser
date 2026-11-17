# T-577, the Flash apps, and T-657, a movie's short sounds (2026-11-16, engine 0113, the three shells)

`finger-report.md`: 57 runs. The ten apps' tasks, `flash-text`, the sound scripts and three site scripts pass in the
three shells; `flash-clipboard-out` stands at T-658 (below).

## The apps (cavekit-flash.md R7)

| app | the task, and what judges it |
|---|---|
| calculator | `7 + 8 =`: the display's picture equals the one after keying `15`, and differs from Clear's |
| colouring book | a colour, a tap on the sky: the sky's picture changes, the far hills' does not (the movie's brush hangs below the tap) |
| chart (YUI 2) | the page's rows by ExternalInterface; a tapped column comes back as its row: `click 2 Mar series 0`, `click 4 May series 0` |
| postcard viewer | its gallery (XML and six pictures made by the corpus's server); a card forward over the plain band, and back |
| clock | its picture moves on by itself, twice |
| piano | a key sounds at the sink, silence, another key sounds |
| phonemic chart | the first tapped symbol sounds, and two more; silence before and after |
| drum machine | its players' place empty, then drawn after the taps; it plays at the sink across them |
| MP3 player | silent, play: the tune at the sink (a quiet file: a peak of 600 is the bar), pause: silent |
| form (website-viewer) | its ActionScript runs away without its own site: the player's "Kill this Action Script?" dialog, answered, the page left standing |

New in the suite: `stage-tap`, `stage-drag`, a stage rectangle as a region (`m@550,550:165,99,397,155`),
`sound-watch MS` with `expect sound-watched heard|silent [MIN_PEAK]` (the recorder across the gesture: a drum hit is
over before a recorder started after the tap connects), `kernel-chord MOD CODE`, `chrome field-paste`.

## What the apps found

- **T-657, short sounds were never heard.** The plugin's streams were raised from the audio policy's 0 % by a script
  once a second, and the speaker amp came on only after a scan saw a sound open: the chart's 0.4 s symbols read 0
  at the sink, whichever was tapped first, and then by chance. `render/goanna/JihadFlashAudio.h`: a thread follows
  the sound server's stream events and raises a plugin's stream as it opens (at most four times a stream); the amp
  is held from a plugin's start by one script and the level's script runs only while a sound plays. The first
  symbol now peaks some 7500 in each shell. Reviewed by ck:inspector: no periodic script while silent, the raises
  bounded, libpulse opened without RTLD_GLOBAL, the stale count on a failed scan.
- **The user agent had no Gecko/ token**, and YUI 2 looks for Flash only under Gecko/, AppleWebKit/ or Opera: it
  reported no Flash and embedded nothing. The owner, asked: "add the gecko version from when pale moon last forked
  gecko": `Gecko/52.9` (JihadUserAgent.h). The chart's page gives YUI no help and draws: the token's test.
  `flash-tags`, `everyday-browsing` and `ddg-results` pass with it.
- **T-658, the clipboard out of a movie is not the card's**: `System.setClipboard` runs and Paste gives the older
  text (Enyo, Mochi; Mojo's Paste tap missed its menu). FileReference.browse() and .download() answer false and
  start nothing: the device's player has no file dialog.
- The corpus: website-viewer needs humbird0.com; the postcard viewer needed its gallery; the sample MP3 is quiet.
