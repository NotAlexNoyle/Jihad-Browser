# The scoreboard with the Tablet UA (2026-11-19)

The owner's decision of 2026-11-19 put "Tablet" in the UA everywhere (`...; TouchPad; Tablet)`, Firefox's tablet
token). This round asks whether the scoreboard's sites answer it differently. It is one run of Jihad alone
(`RUNS=1 BROWSERS="jihad" ROUND_DIR=docs/rounds/tablet-ua-2026-11-19 bash tools/device/scoreboard.sh`), on engine 0125
and the daemon of `60f6fc46`, against `docs/rounds/scoreboard-2026-11-12`: three runs on engine 0113, before the
Tablet token, 0123's CPU count and the memory guard's thread.

**The pages are the same pages.** Each site's picture at rest (`rest-jihad-*-r1.png`) has the layout it had on
2026-11-12, read by eye: Wikipedia's desktop article, Hacker News, BBC News, the GitHub repository, MDN's `display`
page, and Orisinal's Bells. No site served a tablet layout for the token.

| site | first paint ms (11-12 mean of 3 → now) | settled ms | pan fps | memory growth MB |
|---|---|---|---|---|
| Wikipedia, HP TouchPad | 3489 → 4481 | 27563 → 33282 | 21.2 → 21.4 | 223 → 298 |
| Hacker News | 3697 → 2943 | 7780 → 8543 | 35.3 → 35.2 | 104 → 105 |
| BBC News | 3192 → 2816 | 19595 → 36573 | 24.5 → 22.9 | 152 → 143 |
| GitHub (the page does not pan: `pan_moved` 0.01 in both) | 2939 → 3750 | 13159 → 16267 | (no pan) | 156 → 79 |
| MDN, `display` | 3209 → 3339 | 25013 → 31806 | 26.3 → 23.3 | 195 → 170 |
| Orisinal, Bells | 3532 → 3666 | 17522 → 14065 | 30.4 → 28.7 | 177 → 146 |

One run against a mean of three, with engine changes between them, so the numbers can't single out the Tablet
token. The baseline's own runs spread as widely: Wikipedia settled at 21 to 35 s, BBC at 9 to 31 s. Pan rates are
within a frame or two of the baseline. Memory growth is lower on four sites and higher on Wikipedia (298 MB against
207-247 MB in the baseline's runs). That one figure is worth three runs before anything is read into it (T-652,
deferred: less memory than stock on every site).

Before this round, `scoreboard.sh` had voided a run: its device directory `/media/internal/jihad-stream` had gone,
every row's redirect failed, and the host saw only "no done marker". The script now makes the directory each time.
