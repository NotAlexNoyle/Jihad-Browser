# build/webos-oe — Phase 2 webOS 3 ARMv7 device build

Cross-compile the Phase-1 result for the HP TouchPad (webOS 3.0.x, ARMv7) and
package it as `.ipk` via OpenEmbedded, reusing the `meta-webos` layer
(`../../../meta-webos`).

## Approach

- Mirror the existing isis recipes (`meta-webos/recipes-webos/browserserver`,
  `browser-adapter`, `com.palm.app.browser`) as Jihad variants:
  - `jihad-browserserver.bb` — builds `render/browserserver` + `render/goanna`,
    `DEPENDS` on the Goanna engine recipe (new) instead of `webkit-webos` +
    `qt4-webos`.
  - `goanna.bb` — builds UXP for the webOS ARMv7 target with the embedding
    config; this is the heavy recipe and depends on the new toolchain (below).
  - `net.riverstonerelay.jihad-browser.bb` — packages the `app/` Enyo shell.
- Keep `BrowserAdapter` essentially as-is (it is engine-agnostic — it only
  speaks YAP and blits shmem); rebuild it against the Jihad daemon.

## Toolchain gate — ✅ cleared

The stock webOS 3 / CodeSourcery **gcc 4.4** cannot build UXP (needs C++14, a
modern libstdc++, newer binutils). A modern cross-toolchain was stood up with
crosstool-NG — **GCC 9.4 + glibc 2.23 (softfp, min-kernel 2.6.32), armv7-a NEON**
— matching the TouchPad's glibc/kernel ABI (`toolchain/`, see
`../../docs/TOOLCHAIN.md`). It builds libxul + the daemon, and a C++14 binary runs
on the device. The direct cross-build scripts (`build-goanna-arm.sh`,
`build-daemon-arm.sh`, `make-device-bundle.sh`, `build-adapter-pdk.sh`,
`build-mochi-ipk.sh`) are the working pipeline; `build-all-device.sh` runs them as
one entry. The `recipes-jihad/*.bb` + `conf/machine/{tenderloin,opal}.conf` are the
OE-layer shape (documentation of record — the direct scripts are what actually
build; see `../../docs/DEVICE-BUILD.md`).

## Runtime constraints (TouchPad)

- ~1 GB RAM, ARMv7 (single render process per card). Goanna is heavier than the
  old QtWebKit; memory tuning, `jemalloc`/`ptmalloc3` choice, and aggressive
  `freeze`/`purgePage` handling matter. Tracked in Phase 3.

## Deploying to a connected device

Three scripts cover the whole loop, coarsest first. Each pushes over `novacom` and
**md5-verifies both sides** — a push has died mid-transfer before and left a zero-byte
daemon behind while the exit status said success.

| Script | Pushes | When |
|--------|--------|------|
| `push-variant.sh <variant>` | the full payload, then runs that variant's real `postinst` as root | after a packaging change, or to reproduce what an `.ipk` install does |
| `push-engine-update.sh [variants…]` | `libxul.so` + the daemon + **`goanna.js`** (stripped, atomic `mv`), then restarts the upstart job | after an engine, daemon **or pref** rebuild |
| `push-card-js.sh <variant> <files…>` | card JS/CSS/assets | after a UI change — the fast loop |

`push-card-js.sh` is the one to reach for while working on a shell, and it exists
because two things on this device make a naive push untrustworthy:

- **The WebAppMgr JS cache really does serve a stale build** after a close-and-relaunch,
  with the new bytes already on disk. So the script restarts LunaSysMgr each cycle and
  then **requires a per-push stamp to appear in `/var/log/messages`** before reporting
  success. An on-disk md5 proves the file arrived; only the stamp proves the card
  reloaded it. (`/dev/fb1` holds the last painted frame, so a screenshot proves even
  less — confirm liveness from the daemon log.)
- **`novacom run` discards output that arrives after the host's stdin hits EOF.** Slow
  commands — anything crossing the Luna bus — come back EMPTY with exit 0, which reads
  exactly like "no result". Every device call in the script holds stdin open
  (`sleep 4 | novacom run …`) and retries rather than trusting an empty reply.

`push-engine-update.sh` ships `goanna.js` from `device-bundle/`, not from the dist: the dist copy
is stock upstream, and the low-RAM / add-on / OMTC pref blocks are appended by
`make-device-bundle.sh`. **Run `make-device-bundle.sh` after changing a pref**, or the push
carries the old file. Prefs are read once at engine startup, so a pref-only change still needs
the daemon restart this script performs — a card reload is not enough.

## Driving the device without touching the screen

The supervised upstart job deliberately does **not** set `JIHAD_INJECT`; the self-drive channel
stays off in normal operation. To drive a variant, stop its job and run an ad-hoc daemon with the
same environment plus `JIHAD_INJECT=1` (copy the `exec env` line out of `/etc/event.d/<job>` —
`LD_LIBRARY_PATH`, `ICU_DATA`, `HOME` and `JIHAD_BS_NAME` are all load-bearing), then write one
command per line to `$JIHAD_STATE_DIR/inject.cmd`.

Two things will waste your afternoon otherwise:

- **Launch the card AT a URL**: `palm-launch -p '{"target":"…"}' <appid>`. The start page is
  card-side HTML, and no `BrowserPageGoanna` exists until the card navigates — launch it bare and
  every inject command answers `inject: no page`.
- **Stop the ad-hoc daemon with SIGTERM, never `-9`.** The add-on database and prefs are deferred
  savers that only write during a clean shutdown. Also note the process name is **`ld-2.23.so`**
  (it runs via the bundled loader), so `killall jihad-browserserver` matches nothing and instances
  pile up, all polling the same inject file.

A dialog raised while you are driving needs answering, or it takes its default after 60 s: write
`\x00\x00\x00\x02` then `1\x00` to the `dialog-*.fifo` the daemon logs. `JIHAD_DIALOG_MS`
shortens the deadline for tests that want the default quickly. If what you are testing is the
HUMAN path, answer LATE on purpose — a 300 ms scripted answer hid a bug that denied every real
dialog for months (see `context/kits/cavekit-browser-services.md` R3).

**Neither push script carries `appinfo.json`.** `push-engine-update.sh` ships the engine, daemon
and prefs; `push-card-js.sh` ships the files you name. App METADATA — the launcher title, the
icon, the version — only travels with a full `push-variant.sh` or a real `.ipk` install. That is
how the Mojo variant sat on the device titled "Jihad Browser" for two days after the repo had
renamed it to "Jihad Mojo" (2026-08-05): the repo was right, the device was stale, and nothing
in the fast loop would ever have corrected it. `push-card-js.sh <variant> appinfo.json` does work
— the LunaSysMgr restart it already performs is what makes the launcher re-read the file — but
you have to know to ask for it. When you rename or re-version an app, push `appinfo.json`
explicitly and check the launcher, not the repo.

It also closes the running card by its **real** `processId` from
`applicationManager/running` (note the reply field is lowercase `processid` while the
close request wants camelCase `processId` — mixing them up closes nothing and the
window manager quietly restores the stale card).
