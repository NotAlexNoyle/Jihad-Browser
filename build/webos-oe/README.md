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
