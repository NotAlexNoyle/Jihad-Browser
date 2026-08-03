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
| `push-engine-update.sh [variants…]` | `libxul.so` + the daemon (stripped, atomic `mv`), then restarts the upstart job | after an engine or daemon rebuild |
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

It also closes the running card by its **real** `processId` from
`applicationManager/running` (note the reply field is lowercase `processid` while the
close request wants camelCase `processId` — mixing them up closes nothing and the
window manager quietly restores the stale card).
