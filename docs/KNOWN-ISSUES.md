# Known issues

Everything above was measured on a TouchPad. Here's what doesn't work yet, or works with a catch.
Site-by-site details, with causes and what was tried, are in `docs/MODERN-WEB.md`.

- **Only the TouchPad** has been tested (on webOS 3.0.5, then webOS CE 3.1.0; every test is run by tools on the device, `docs/FINGER-TESTS.md`); the TouchPad Go runs the
  same binaries at the same resolution and is covered by that equivalence, not by a device test. Other webOS 3 devices should work and have
  not been tried.
- **The TLS fix covers Jihad only.** The stock browser, Mail and the system services still use the
  device's own OpenSSL; the [webOS Archive guide](https://docs.webosarchive.org/site/modern-tls/)
  covers those. Keep the clock set: certificate checks need the right date. howsmyssl.com's one
  "improvable" mark is post-quantum key agreement, which the engine does not have.
- **Frame rates are the device's.** Drawn on the GPU, a simple page pans at 30-36 frames a second
  and desktop Wikipedia at 23-24 (its two pinned side panels are drawn again for every frame); the
  first frame at a new zoom comes within a quarter of a second most of the time. Mochi's text can
  look soft at some zoom levels.
- **WebGL is version 1 on a 2011 GPU.** A 480-pixel spinning cube draws at 21-25 frames a second
  and reaches the card at about 20 (a smaller canvas reaches the card faster); four extensions desktop browsers offer are missing
  (`ANGLE_instanced_arrays`, `WEBGL_compressed_texture_s3tc`, `EXT_blend_minmax`,
  `EXT_shader_texture_lod`).
- **Modern single-page applications are slow.** A page built as one large script (GitLab, Reddit's
  login page, DuckDuckGo's full results page) takes tens of seconds of script on this CPU, and the
  page is frozen while it runs.
- **YouTube works, but it's heavy.** Its mobile watch page keeps one of the two cores busy for as
  long as it's open and grows past 200 MB, and video plays at 144p to 360p, decoded in software. The
  TouchPad has a hardware H.264 decoder that Jihad doesn't use yet. When a video ends, YouTube still
  moves on to the next one's page by itself (that is its own navigation, which nothing in the page's
  settings turns off from here); the video on it waits for a tap.
- **Two heavy cards don't fit.** A big Flash game beside a YouTube page pushes a 1 GB tablet, with
  the system's own memory already swapped out, into swapping so hard that the flash chip's queue
  stalls every process on it — the freeze and the dropped USB link of early November. The guard now
  cuts the browser back at the first sign (the game's player, then the page), and the tablet stays
  responsive; what you see is the game stopped and the page replaced by a notice with a link back.
- **Stopping a runaway page isn't free.** When the memory guard steps in, the tablet can hang for
  a few seconds while the system pulls its own memory back from swap.
- **Sites built as ES-module applications** load their frame but not every script-driven part
  (GitHub's commit list and README, Mastodon's timeline): the engine's module loading is behind
  them.
- **DuckDuckGo's full results page** shows its results, with its slide-out menu standing open at
  the top: the page is wider than the window and the card shows the strip past the edge where the
  page parks the menu. DuckDuckGo Lite, the default search, has no such page.
- **Some bot checks still refuse the engine.** Akamai's (eBay) and Reddit's login gate on
  old.reddit.com do, with any identity. Startpage's search works after its proof-of-work check,
  which takes about four minutes on this CPU the first time and then holds for a week.
- **What no shim covers:** CSS container queries, `backdrop-filter`, `content-visibility` and
  `Intl.Segmenter`. A page that needs one of those shows it.
- **`palm-install` skips the install scripts**, so a package installed with it does not work. Use
  Preware, WebOS Quick Install or `push-variant.sh`.
- **Flash Player is the device's own** and is not distributed with Jihad; a device without it has
  no Flash.
- **Flash Player 10.3 is from 2011.** Movies made for Flash Player 11 and later may show nothing or
  crawl (Run 3, QB Typing, Learn to Fly 3 at about a frame a second). The stock browser does the same
  with them.
- **A Flash movie in a card behind another one can go quiet** while the front card plays sound.
  Being looked at.
