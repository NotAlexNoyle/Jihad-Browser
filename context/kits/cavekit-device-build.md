---
created: "2026-06-30"
last_edited: "2026-11-07"
---

# Cavekit: Device Build & Packaging

## 2026-10-01 — THE TARGET PLATFORM IS NOW webOS CE 3.1.0 (READ FIRST)

The TouchPad this project is verified on was reflashed to **webOS CE 3.1.0** (build 86, BUILDTIME
20260830; read from `/etc/palm-build-info`). Everything in this kit dated before 2026-09-27 that
says "device-verified" was verified on HP webOS **3.0.5** and has NOT been re-run unless its own
note says so. What was measured on the new system, and what follows from it:

- **Unchanged, measured:** kernel `2.6.35-palm-tenderloin`, **glibc 2.8**, no keyboard
  (`gpio-keys`, `pmic8058_pwrkey`, `headset` only), Flash present (`com.palm.app.flashplugin-001`,
  same `libflashplayer.so`), the `com.palm.flashgraphics` role name in the stock role files. So
  every statement in **R9** (the 2.23-vs-2.8 ABI boundary) still describes this device, and the
  run-in-place packaging model (R3/R8) is unaffected by construction.
- **New:** a side-by-side OpenSSL 1.1.1w stack (`ssl11`, `ssl11dl`, `ssl11mail`; `curl 7.88.1 …
  OpenSSL/1.1.1w`) next to the stock 0.9.8, which is still `/usr/lib/libssl.so.0.9.8`. Jihad links
  neither — page TLS is the engine's own NSS (cavekit-browser-services.md R5).
- **New:** CE preinstalls the **Atlas** browser (`org.webosports.app.atlas`,
  `BrowserAdapterAtlas.so`, `/tmp/yapserver.atlas`, its own daemon). Four browsers now coexist
  (stock, Atlas, and up to three Jihad variants); R7's independence proofs predate it.
- **Defects found in the CE stock-take (2026-09-27)** became criteria: first-start Luna registration
  (R4), the log growing and the idle daemon ticking (R8), and the blank-page repaint storm
  (cavekit-offscreen-rendering.md R8), all met since; `<video>` failing after 10–60 frames
  (cavekit-gre-widgets.md R1).

**The rule from here on:** a box closes on CE 3.1.0 evidence. A box that is `[x]` on 3.0.5
evidence stays `[x]` only as "met on 3.0.5" and is listed in R4's re-verification sweep; when the
sweep re-runs it, it gains a dated "CE 3.1.0 re-verified" note or is downgraded in its own kit.

## Scope
Phase-2 work to run Jihad Browser on the HP TouchPad (webOS 3.0.5 and webOS CE 3.1.0, ARMv7):
standing up a modern cross-toolchain, cross-compiling the engine and daemon,
packaging, and validating on-device. The toolchain is a feasibility gate. Jihad
packages as a **self-contained app that coexists with the stock browser** — its own
NPAPI MIME/adapter/daemon/upstart job, nothing system-level replaced (auto-memory
`jihad-self-contained-arch.md`). Reference: `docs/TOOLCHAIN.md`, `docs/DEVICE-BUILD.md`,
`docs/OE-BUILD.md`, `build/webos-oe/README.md`. (The `build/` tree — toolchain, engine objects,
vendored adapter deps — is git-excluded to avoid redistributing third-party binaries; the build
scripts/recipes and `packaging/` are the reproducible source.)

## Packaging model (user decision 2026-07-31; READ BEFORE TOUCHING PACKAGING)

Carried by **R7** and **R8**. Three independent packages (Enyo, Mochi, Mojo), each with its own
MIME, shim, impl, YAP name, socket, upstart job and daemon; nothing is co-owned, so nothing is
refcounted. Nothing on `/media/internal`, the user's USB mass-storage volume: the engine and
daemon run in place from the app's own `deviceroot/` on cryptofs, the Atlas browser's model. The
one unavoidable rootfs write is the NPAPI adapter in `/usr/lib/BrowserPlugins`, the only
directory webOS's WebKit scans for plugins (at boot); R8 bounds and reverses it.

## Requirements

### R1: Modern ARMv7 cross-toolchain (feasibility gate)
**Description:** A toolchain capable of building the engine for the device exists, since the stock device compiler cannot.
**Acceptance Criteria:**
- [x] A documented, reproducible cross-toolchain (modern C++14-capable compiler) targets the device's glibc/kernel ABI.
- [x] A trivial C++14 program built with it runs on a webOS 3 device/emulator.
- [x] The toolchain links against (or matches) the device sysroot so binaries do not require a newer glibc than the device has.
**Dependencies:** none

### R2: Engine cross-compiles for webOS 3 ARMv7
**Description:** Goanna builds for the device target.
**Acceptance Criteria:**
- [x] The engine builds with the cross-toolchain against the device sysroot.
- [x] The resulting libraries load on the device without missing-symbol/ABI errors.
- [x] ARMv7 FP/SIMD flags match the device CPU.
**Dependencies:** R1, cavekit-engine-embedding.md (R1)

### R3: Package Jihad as a self-contained app — all three UI variants
**Description:** The whole product packages as installable webOS artifacts that coexist with the stock browser, including ALL THREE front-end variants. **Shipping model: three SELF-CONTAINED app `.ipk`s** (Enyo + Mochi + Mojo), each bundling engine+daemon+adapter+bundled-glibc+UI + a postinst that deploys the coexisting pieces.
**Acceptance Criteria:**
- [x] The daemon + a rebuilt **coexisting** adapter (`BrowserAdapterJihad.so`, MIME `application/x-jihad-browser`, YAP name `jihad-browser`) install ALONGSIDE the stock browser without collision (`packaging/postinst`+`prerm`+`event.d/jihad`). Verified on-device.
- [x] The Enyo UI `.ipk` (`net.riverstonerelay.jihad-browser`, from `app/`) builds (`palm-package app/`) and installs; its WebView is routed to the Jihad engine by `app/source/JihadEngineOverride.js`.
- [x] The build also produces the Mochi variant `.ipk` (`net.riverstonerelay.jihad-browser-mochi`, from `app-mochi/`); both install and can coexist. *(Produced 2026-07-19 (`build-mochi-ipk.sh`, 1.4 MB). INSTALL + COEXIST VERIFIED ON DEVICE 2026-07-19: `palm-install -l` lists both app ids at 1.0.0.)*
- [x] A single OE/bitbake build PRODUCES the three `.ipk`s, from a clean clone PLUS ONLY the four prebuilt inputs declared in the carve-out block below. *(2026-08-16 (T-154): `bitbake net.riverstonerelay.jihad-browser{,-mochi,-mojo}` → `639 tasks ... all succeeded`, zero errors; the three `.ipk`s in `build-webos/BUILD/deploy/ipk/all/` (enyo 46.6 MB, mochi 48.4 MB, mojo 45.8 MB), content-checked: `deviceroot/hl/{jihad-browserserver, libxul.so, plugin-container, ld-2.23.so, goanna.js}` plus the loose chrome (1104 files). The run exercised the per-variant `do_install` loops of `browser-adapter-jihad` and `jihad-deviceroot` and the `do_<task>[file-checksums]` declarations (review #8, `../impl/impl-review-findings-oe.md`). Recipe gaps fixed on the way (`../plans/build-site.md` T-154, `../impl/dead-ends.md`): the engine delta captured as a commit pinned by `SRCREV` instead of a drifting patch queue; three daemon sources missing at link; `plugin-container` and the loose GRE dirs not staged; bitbake 1.18 does not re-fetch on a `SRCREV` bump, so a chrome-only change needs `cleansstate` and a content check. Clean-clone residual: the criterion below.)*
- [x] Each variant `.ipk` is SELF-CONTAINED and installs as one coexisting package. *(2026-08-05: proven on the supported path for all three, driving `org.webosinternals.ipkgservice` directly. Each install runs our `postinst`, which lays down the upstart job, adapter shim, adapter impl, state dir and Luna role; afterwards all three are registered with ipkg, all three daemons run, all three Luna services serve, and all three come back on their own after a cold boot. CE 3.1.0: R4's fresh-install criterion.)*
- [x] A third UI variant SHIPS as a working Mojo front-end (not a scaffold). *(2026-08-05: installs through `org.webosinternals.ipkgservice`, registers with ipkg, brings up its own daemon on its own socket and returns after a cold boot. The UI is cavekit-mojo-ui.md (toolbar, new-card / history / share controls, history scene, branded start page, framework `<select>` popups); Topaz layout verified there (R4).)*
- [x] The Mochi package bundles Enyo 2 + layout + Mochi; the Enyo package bundles Enyo 1.0. *(2026-08-05, checked inside the built `.ipk`: `data.tar.gz` carries `enyo/`, `layout/` and `mochi/` under the app directory, and a generated `BUNDLED-VERSIONS` records each framework's source path and commit. The Enyo half is a deliberate deviation: `app/index.html` loads Enyo 1.0 from the OS framework path `/usr/palm/frameworks/enyo/0.10`, as upstream isis-browser does, rather than duplicate the system framework.)*
- [x] The prebuilt inputs the bitbake build does NOT produce are enumerated with provenance, size and redistribution status, and the reproducibility criterion is scoped against them rather than against "everything from source". *(2026-08-10, T-115 — the carve-out block below. Every figure in it was measured on disk this session, not copied from a doc.)*
- [~] Every declared input is obtainable from a clean clone: an in-repo recipe, or a documented procedure for the ones that cannot be vendored. *(Declared and checksummed 2026-08-15 (T-115): `build/webos-oe/gen-sysroot-manifest.sh` → `arm-sysroot-debs.manifest` (187 `.deb`s with version, sha256, source package and `archive.debian.org` pool URL; `--check`, `--fetch`, `--check-urls`; the pool path is keyed by SOURCE package and drops the epoch, so neither is derivable from the on-disk filename); `gen-adapter-deps-manifest.sh` → `adapter-deps.manifest` (the Qt4 header drop is stock Debian jessie `qt4-x11`, pinned by sha256, `--check-qt4-debs`; the 7 device `.so`s — `libQt{Core,Gui,Network}.so.4.8.0`, `libpbnjson_{c,cpp}.so`, `libyajl.so.1`, `libpng12.so.0` — carry HP's own OE build paths and are fetched by a `novacom get` procedure, run on CE 3.1.0 on 2026-10-01: paths and sha256 match). Both manifests regenerate byte-identical. Version skew to leave alone: the pinned headers are pbnjson `submissions/10` / yajl 1.0.12 / Qt 4.8.6, the linked libraries pbnjson submission-38 / yajl 1.0.7 / Qt 4.8.0 (`build-adapter-pdk.sh` chose `submissions/10` deliberately); aligning them needs a hardware retest. The toolchain recipe is tracked in `build/webos-oe/toolchain/`, and since 2026-10-14 every input is fetched or built by a script in the repository (carve-out below, R11). The last open residual was the engine: the `goanna` recipe pins it by `SRCREV` in the UXP submodule, and a fresh clone resolves it only once that commit is pushed to the fork named in `.gitmodules`; on 2026-10-01 (T-279) the branch was unpushed. R11 records all eleven pins fetchable from their public URLs on 2026-10-14 (`tools/host/check-pins.sh`); this box has not been re-checked since.)*

**PREBUILT INPUTS — a bounded carve-out (declared 2026-08-10, T-115).** "Clean-clone
reproducible" means: **a clean clone plus exactly these four declared inputs reproduces the three
`.ipk`s** — nothing else may be required, no input may be undeclared, and each must be either
rebuildable from an in-repo recipe or obtainable by a documented procedure. Since 2026-10-14
(R11, T-472–T-474) every input is produced by a script in the repository, and
`build-all-device.sh` runs each one that is missing: the toolchain by `toolchain/build.sh` from
the crosstool-NG submodule; the sysroot by `gen-sysroot-manifest.sh --fetch` + `--assemble`,
which applies `arm-sysroot.recipe` (glibc 2.19 removed, `/lib` copied into `/usr/lib`, six
symlinks, and PulseAudio 0.9.22's headers) and refuses a tree whose digest differs; the PDK by
`fetch-pdk.sh` from the Internet Archive, sha256-checked (a supplied copy is checked the same
way); the Qt4 headers and the device libraries by `gen-adapter-deps-manifest.sh --fetch-qt4` /
`--fetch-device-libs`. The libpbnjson, yajl and npapi-headers checkouts are submodules, and the
headers Jihad made are files in git (`adapter-include/`). Device-gated, still: eight libraries
(the seven in `adapter-deps.manifest` plus `libpulse.so.0`), fetched once over novacom and
checked by sha256.

| Input | Path (git-ignored) | Size (2026-08-10) | What it is | Rebuildable? |
|---|---|---|---|---|
| crosstool-NG toolchain | `build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi` | 219 MB (+279 MB tarball cache) | `arm-webos-linux-gnueabi-gcc (crosstool-NG 1.25.0) 9.4.0`, glibc 2.23, armv7-a NEON softfp. `jihad-cross-toolchain-native` only CHECKS it | **YES, from source** (`build/webos-oe/toolchain/`); all inputs are FOSS |
| Debian Jessie armel sysroot | `build/webos-oe/arm-sysroot/root`, assembled from `arm-sysroot/debs` | 127 MB, from 187 `.deb`s | GTK2/X11/cairo/pango/fontconfig/glib that the engine links against; the 89 `.pc` files are exactly what `PKG_CONFIG_LIBDIR` points at | **YES** — stock Jessie packages on `archive.debian.org`, recorded with sha256 + pool URL in `arm-sysroot-debs.manifest` |
| Palm PDK | `build/webos-oe/pdk/opt/PalmPDK` | 442 MB | CodeSourcery `arm-none-linux-gnueabi-gcc (GCC) 4.3.3` + its device sysroot + Palm's PDL headers and device libs (`libc-2.5.so`, SDL, GLES) | **NO — PERMANENT.** Proprietary HP/Palm SDK, not redistributable; HP's `palm-sdk_3.0.5-svn528736-pho676_i386.deb` is preserved on the Internet Archive (item `webOSLegacySDK`) and fetched + sha256-checked, not vendored |
| adapter-deps | `build/webos-oe/adapter-deps/{staging,qt4-extract,libpbnjson,yajl,npapi-headers}` | 40 MB | 7 device ARM `.so`s linked against (Qt 4.8.0, pbnjson, yajl, png12) + 10 dev symlinks made here + the 25 MB Qt4 header drop + FOSS checkouts (`libpbnjson` tag `submissions/10` = `c4b0611`, `yajl` `1.0.12` = `17b1790`, `npapi-headers` `0.4` = `9d35121`) | **MIXED.** The checkouts are public git; the Qt4 drop is stock Debian jessie `qt4-x11 4:4.8.6+…+deb8u1`, byte-identical to `archive.debian.org`; 4 of the 5 NPAPI headers come from `npapi-headers`, only `nppalmdefs.h` is Palm-only; the 7 `.so`s come off a TouchPad and are not redistributable. Declared in `adapter-deps.manifest` |

**Why the PDK cannot be designed away.** The adapter is `dlopen`ed into LunaSysMgr's gcc-4 WebKit
process, and a gcc-9-built plugin collides on static libstdc++ and crashes the host — that is the
reason `build-adapter-pdk.sh` exists alongside `build-adapter-arm.sh` (its header records the
crash). Matching that ABI needs a gcc-4.3-era ARM toolchain, and the PDK is the one on hand known
to match. NOT ruled out, and nobody has tried it: a crosstool-NG gcc 4.3 would replace the PDK's
*compiler*, but not its device sysroot or its PDL headers, so it does not remove the carve-out.

**Dependencies:** R2, cavekit-desktop-build.md (R1), cavekit-mochi-ui.md (R1), auto-memory `jihad-self-contained-arch.md`

### R4: Runs on the TouchPad (and TouchPad Go)
**Description:** The installed browser works on real hardware — all three UI variants, on the current target platform, **webOS CE 3.1.0** (was HP webOS 3.0.5 until 2026-09; see the 2026-10-01 header).
**Acceptance Criteria:**
- [x] On the TouchPad (Topaz/tenderloin), the **Enyo** variant launches and loads a page rendered on-screen via the Jihad adapter (`example.com`, `slack.com`/HTTPS render on fb1; load-completion + refresh glyph). ~~Mochi variant pending.~~ *(CE 3.1.0, 2026-09-27: after deploying all three variants the card rendered and pages loaded over HTTP and HTTPS (`https://example.com`, `https://www.howsmyssl.com/a/check`). Mochi and Mojo launch-and-render are each variant kit's R4; the per-variant CE install is the fresh-install criterion below.)*
- [x] Basic navigation works (URL load + load-complete); scrolling/tap-activation/keyboard are in Phase-3 hardening (staged fixes, on-device confirm pending). *(Closed 2026-08-05, when the self-contained `.ipk`s of all three variants were installed on the supported path and worked across a reboot. Navigation — address → openUrl, back/forward/reload, link taps, load completion — device-verified since 2026-07-20 (`../impl/device-test-2026-07-19.md`; daemon crash fix 2be6d85; link taps after the zoom fix d4f0842). Not navigation defects, tracked elsewhere: the VKB white-band jank (cavekit-input-bridging.md R2) and a real two-finger pinch (R3 there).)*
- [x] Composite is correct in portrait and landscape. *(Device-confirmed 2026-07-27, commit 8d7865c. The portrait 3× shear came from the raw `dstBuffer` path, which LunaCE reads at a fixed ~256 px (1024-byte) pitch; the adapter now composites through the WebKit-provided Piranha **PGContext** (`AdapterBase(..., useGraphicsContext=true)` + `PGSurface::wrap` + `gc->bitblt`), which carries the card's rotation/scale. `../impl/rotation-fix-2026-07-26.md`, cavekit-offscreen-rendering.md R6. A visual confirm; no per-orientation capture archived. Separate known quirk: a black card + VKB on the first launch after a LunaSysMgr restart, cleared by reopening.)*
- [~] Cert/dialog/download flows function with the device services. [human-review on device] *(2026-10-01, all three re-run on webOS CE 3.1.0 (daemon `24a902eb…`, libxul `65e3cc84…`): the certificate flow end to end with the platform store (cavekit-browser-services.md R5); the SSL-confirm dialog rendered by the Enyo card; a 65,536-byte `application/octet-stream` download → `download handoff` → `[jihad-dl] finished … -> /media/internal/downloads/jihad-testfile-2.bin`, md5 `3fc476cb394bb1d941a9288ed3e2d868` identical to the source. 3.0.5 record (2026-08-15): `../impl/impl-device-2026-08-15.md`. Open: a person looking at the file in the Downloads app. Test residue to delete: `/media/internal/downloads/jihad-testfile{,-1,-2}.bin`.)*
- [x] **An UPGRADE install over a live system does not take LunaSysMgr down.** *(Added and met 2026-10-01. Found: after an upgrade install through `org.webosinternals.ipkgservice` the next card launch crashed LunaSysMgr (SIGSEGV in `ld-2.8.so` `check_match`/`do_lookup_x` under `WebCore::PluginPackage::load`), taking every card down: postinst `cp`'d the shim and impl over the inode LunaSysMgr still had mapped. Fix: postinst writes each file beside its destination under a dot-name, sets owner and mode, and `mv`s it over; prerm also removes the temp names. Verified on three upgrades with the variant's adapter loaded beforehand (Enyo, Mochi, Mojo): LunaSysMgr PIDs unchanged, no crash report, `/proc/<pid>/maps` shows the old shim as `(deleted)`, each card connected to its upgraded daemon and painted. Known, not fixed: for about a minute after an `ipkgservice` install a `launch` returns success and opens no card.)*
- [x] **Fresh install on webOS CE 3.1.0, on the supported install path, for all three variants.** Starting from a device with NO trace of the variant (no app dir, no upstart job, no adapter shim/impl, no role or service files, no state dir), installing its `.ipk` the supported way (R8: Preware / WebOS Quick Install, i.e. `org.webosinternals.ipkgservice`, which runs the control scripts) and then doing exactly what the postinst asks for — and nothing else, in particular no hand-run `ls-control`, no hand-started job, no hand-pushed file — leaves: the daemon running out of the app's own `deviceroot`, its own YAP socket present, and the card launching and rendering a page. Shown per variant. *(Done for all three: Mochi 2026-10-05, Enyo and Mojo 2026-10-06, each removed through `org.webosinternals.ipkgservice/remove` (stages `begin → prerm → remove → delete → completed`) and installed through `/install` + `/rescan`, nothing by hand. After removal: no app dir, job, shim, impl, role/service file, state dir or socket; the hub answered `Service does not exist`; the other daemons kept their PIDs; the `device-citizen-audit.sh` diff showed only the variant's entries plus the empty package-registry directory (R8). After install: postinst rescanned the hub (`role … is live`) and registered Enyo's db8 kinds; the daemon runs from `deviceroot/hl/ld-2.23.so` with `luna: serving` and zero `FAILED` on first start; YAP socket present; a card connected and painted (Wikipedia, in Enyo and Mojo); Palefill landed in the new profile (Mojo's after ~70 s of an open card; the installer retries at every start). The launcher knows a new app only after `ipkgservice/rescan`, which Preware calls after every install. Reboot (2026-10-02, upgrade-installed packages, shipped job files): all three jobs running 95 s after boot with nothing done by hand, 0 `FAILED`, root filesystem read-only at rest. Evidence: `build/webos-oe/out-audit/{pre,post}-{enyo,mojo}-{remove,fresh}-1006.txt`, `../impl/evidence-2026-10-05/mochi-fresh-install-audit.diff`, `../impl/evidence-2026-10-05/mochi_fresh-install-first-card.jpg`. User data kept across the cycle: Enyo's db8 history exported and restored (69 rows); Mojo's from `~/eclipse-workspace/Jihad/device-backups/jihad-backup-20261006.tgz`.)*
- [x] **A page that kills the daemon does not kill it in a loop.** Every top-level load is noted in the state directory with how many daemons in a row died on that address; a clean exit removes the note; an address that took two daemons down within ten minutes is not reopened a third time on the card's request — a short page says so and offers Try again. *(Device, 2026-10-05: daemon killed twice mid-load, third open refused with the page.)*
- [x] **A TLS failure that comes with no certificate is a failed connection, not a certificate question.** The security module also reports handshake failures (measured: `SSL_ERROR_RX_MALFORMED_SERVER_HELLO`, once, on a connection that succeeded next try); those used to raise "The security certificate … is invalid — Trust Always?" with nothing to trust. *(Code, 2026-10-05; the one-off did not recur in three fresh starts.)*
- [x] **A card's "I have the focus" reaches the daemon.** The adapter's `pageFocused` was an empty stub, so every Luna request that cannot say which card it comes from (thumbnail, print, certificate) was answered for the last card to connect or be given plugin focus. It now sends the adapter's existing message; the daemon clears the focus when a card gives it up. *(Device, 2026-10-05.)*
- [x] **All three variants on one build from the packages at `707795de`** (daemon `cce94243…`, libxul `ddf8cc70…`, adapters `88303cfc / 51d10886 / 1b0ac09d`, Palefill in each). Trap recorded: after an upgrade that replaces the adapter, `/` cannot be remounted read-only until LunaSysMgr restarts — it still maps the old file. *(Device, 2026-10-06.)*
- [x] **The variant's Luna service is registered after a fresh install without anyone restarting the daemon by hand.** *(Met 2026-10-01 on webOS CE 3.1.0. Reproduced first (Mochi): with the variant's role and service files moved aside and the hub rescanned, the daemon's first start logged `LSRegisterPalmService('net.riverstonerelay.jihadBrowserMochi') FAILED (code=-1027)` — the old postinst started the daemon before the hub knew the role, and the daemon asked once. Fixed at both ends: the daemon retries registration (5 s backing off to 60 s, about eleven minutes) — the same PID logged `luna: registered on retry 3` within 11 s of the role returning; and postinst runs `ls-control scan-services` after writing the role files and before starting the job. Installed through `org.webosinternals.ipkgservice` with the role hidden beforehand, in all three variants: postinst logged `ls-hubd rescanned: role … is live`, the first start logged `luna: serving …` with zero `FAILED` lines, and a public-bus call was answered by that daemon. While unregistered, `clearCache`/`clearCookies`, `getChromePrefs` and the toast channel (cavekit-gre-widgets.md R5) are silently dead. The from-nothing install: the fresh-install criterion above.)*
- [x] **An upgrade runs the scripts it installed, not the ones it replaced.** After a variant's `.ipk` is installed over an earlier one, every chrome script the daemon runs is the newly installed version: no compiled copy of a replaced script survives the upgrade, without the user's cache or profile being discarded for it. *(Added and met 2026-10-05, commit c4e9bb9e. The engine keeps compiled chrome scripts (JS components, modules, XUL) in `<ProfLD>/startupCache` (here `$APP/cache`, which an upgrade must not touch), keyed by path alone, and an embedder has no version check — a changed `components/jihadPrint.js` ran its previous cached version. Before the engine starts, the daemon fingerprints the bundle — `jihad-bundle.id` (written by `make-device-bundle.sh` from every bundled file's content) plus size and mtime of the files directly in the GRE directory and of `components/jihad*`; 54 files, 23–110 ms — and removes the startup cache when the fingerprint changes. Device: unchanged → `bundle unchanged … kept`; a hand-replaced script → `bundle CHANGED … 1 cached file(s) dropped` and the new copy ran; a real upgrade through `org.webosinternals.ipkgservice` (Enyo, Mojo) → `startup cache: bundle CHANGED (id a4a2f8e2…, 54 files, …) — 1 cached file(s) dropped`.)*
- [x] **Coexistence with Atlas, the fourth browser.** All of: (1) no Jihad identity column — NPAPI MIME type, shim filename, YAP name, socket path, upstart job, Luna service name — equals Atlas's, read off the device rather than assumed; (2) the files Atlas owns are byte-identical (md5) before and after an install + removal of a Jihad variant, i.e. Atlas's files join the "stock file" set of R8's second criterion; (3) with Atlas's daemon and a Jihad daemon both running, each browser's card loads and renders a page, and killing either daemon leaves the other serving; (4) a Jihad card never ends up on Atlas's adapter or socket, nor an Atlas card on ours (each daemon's log shows only its own card's connect). *(Met 2026-10-05, webOS CE 3.1.0. (1) Read off the device: Atlas is MIME `application/x-atlas-browser`, shim `BrowserAdapterAtlas.so`, socket `/tmp/yapserver.atlas`, jobs `atlas` + `atlas-sensord`, Luna name `org.webosports.browserserver`, app id `org.webosports.app.atlas` — no column equals a Jihad variant's. (2) Atlas's seven files (shim, both jobs, role file, appinfo, daemon binary) and the six stock-browser files have the same checksum block (`e9d24468…`) before and after three installs and after the Mochi remove → reinstall (`device-citizen-audit.sh`). (3) With a Jihad Enyo card connected an Atlas card rendered a page; `stop jihad` left Atlas's daemon on its PID serving, and `stop atlas` left the Jihad daemon on its PID serving (2026-10-01; side by side again on the 2026-10-05 build, `../impl/evidence-2026-10-05/atlas-card-renders-beside-a-jihad-card.jpg`). (4) Launching an Atlas card added no `client connected` to the Jihad daemon. The `RDXD4_BrowserServer-atlas…SIGBUS` report of 2026-10-02 19:56 UTC is Atlas's own allocator thread at the reboot, nothing of Jihad mapped (`../impl/evidence-2026-10-05/atlas-fault-report-2026-10-02.txt`).)*
**THE webOS CE 3.1.0 RE-VERIFICATION SWEEP (added 2026-10-01).** Every box below this line names
boxes, here or in another kit, whose only DEVICE evidence was taken on webOS 3.0.5. The lists were
built by reading every criterion in every kit on 2026-10-01 and classifying its evidence; boxes
whose evidence is host-side (desktop harness, build output, source reading, qemu) are not platform
evidence and are not listed. **How a sweep box closes:** each listed box is re-run on the CE 3.1.0
device and then carries, IN ITS OWN KIT, either a dated "CE 3.1.0 re-verified" note with what was
observed, or a downgrade with the reason. A listed box whose re-run needs a human at the device is
moved to the last sweep box (human-only residue) with the reason; it does not hold its kit's sweep
box open, and it is never satisfied by a daemon-level inject. Rules for every re-run: one device
task at a time; all three variants on the same engine and daemon build; assert the daemon log GREW
and the card is CONNECTED before believing a reading; record the engine build (libxul md5) beside
the result.
- [~] **Sweep — this kit.** R1: a C++14 program built with the toolchain runs on the device. R2: the engine libraries load without missing-symbol/ABI errors; FP/SIMD flags match the CPU. R3: the coexisting adapter leaves the stock adapters byte-identical; the Enyo, Mochi and Mojo `.ipk`s install and launch; each `.ipk` is self-contained (covered together with the fresh-install criterion above — do them in one cycle). R4: launch-and-render; basic navigation (URL load, back/forward/reload, load-complete); cert / dialog / download flows with the device services. R5: memory budget on a typical page; `freeze` reclaims memory; no OOM across the defined browsing scenario. R7: installing B leaves A's files untouched; removing one leaves the others functional; per-variant daemon independence (kill one, others serve, upstart respawns). R8: nothing written to `/media/internal`; no stock file modified (stock set extended with Atlas's files, and a FRESH CE baseline captured — the stored `baseline-v2` is a 3.0.5 snapshot); rootfs footprint limited to the enumerated paths; runtime state location and its removal on uninstall; the download carve-out; `prerm` removes exactly what was created (the 65/66 run left one failing assertion unnamed — name it this time); rootfs back to read-only on every exit path. R9: the `sem_t` containment holds across reboots; the per-library load proofs (`plugin_mime_probe`, `alsa_probe` under both loaders). R10: governor tunables read back at stock at rest; the plugin boost is taken and released (including on daemon kill); `scaling_governor` is READ only, never written. *(2026-10-01 (third session), CE 3.1.0, everything not needing a removal or a reboot: R1 `hello14` (crosstool `-std=c++14`) prints `c++201402 sum=42` under the bundled loader. R2 `readelf -A` of libxul and the daemon (`v7`, `VFPv3`, `NEONv1`) matches `/proc/cpuinfo` `vfpv3 neon`; no loader error in any daemon. R3 all three `.ipk`s installed through `org.webosinternals.ipkgservice` over the running install; payload md5s equal the packages' (libxul `65e3cc84…`, daemon `50615b82…`); LunaSysMgr PIDs unchanged, no new fault report; the seven device libraries match `adapter-deps.manifest`. R4 URL load, back, forward, reload with the right `back=`/`fwd=` flags in each variant; cert/dialog/download re-run the same day. R5 64 MB RSS on a simple page; the eight-site scenario ended at 126 MB (peak 131, under the 150 MB cap) on one PID, 0 OOM lines, available memory 435 → 385 MB; `freeze` returned 17 MB. 2026-10-02 reboot (owner-approved, job files byte-identical to `packaging/event.d/*`): 95 s after boot all three daemons up with nothing done by hand, `luna: serving` with 0 `FAILED`, no PmLogLib or semaphore line in `dmesg`, root read-only at rest. Open as of 2026-10-02: the removal audit and the fresh-from-nothing install (R3/R7/R8); both were run on 2026-10-05/06 (R4's fresh-install criterion) and this box was not re-read afterwards.)*
- [x] **Sweep — cavekit-offscreen-rendering.md.** R1: windowless render, surface size tracks the window. R2: non-blank frame in the shared buffer, pixel format/stride, colours and orientation of the frame (no channel swap, no flip). R3: one paint notification per frame, no buffer reuse before return, a content change repaints without input. R4: contents-size, scrolled-to and meta-viewport messages. R5: window/virtual-window size, scroll, zoom-and-scroll with input mapping at the new scale. R6: `PGContext::bitblt` / `PGSurface::wrap` resolve against the CE `libWebKitLuna.so`. R7: `<select>` handed to the card in each variant; the `about:addons` tools menu opens, composites, dismisses and its items ACTIVATE (activation was desktop-only even on 3.0.5); an open popup is interactive (this box records no venue or date at all — treat the CE run as its first verification). R8: an engine-initiated scroll forces one full frame (no post-fix device verification is recorded). *(Done 2026-10-01 (third session), CE 3.1.0, Enyo unless said: R1/R5 960x1400 buffers, three-buffer rotation; `size 600 500` → geometry and frame follow, and back. R2 `jihad-colors.html`: red (255,0,0) top-left, green top-right, blue below-left, yellow below-right. R3 an 8 s ticker painted 8 frames with no input. R4 contents-size on every geometry change; `scrolledTo -> card: 0,908 (content-initiated)` on PageDown; `metaViewport -> card: initial=1.0000 … 768x942` only for a page that declares one. R5 taps at the 0.78 fit zoom and after a scroll land on target. R6 the CE `libWebKitLuna.so` differs from 3.0.5's in two `.rodata` strings only (`video/x-ms-wmv` → `video/webm`, `HP webOS ` → `webOS CE `); `.text` is byte-identical. R7 `<select>` → `popupMenuShow` and a card list in each variant, the pick applied through `popupMenuSelect`; the `about:addons` tools menu opens, composites, closes on Escape and its items activate (after two fixes). R8 an engine-initiated scroll forces full frames; damage scaled into buffer pixels at a fit zoom (new criterion there).)*
- [x] **Sweep — cavekit-gre-widgets.md.** R1: the videocontrols binding attaches (first criterion); visible degradation for an undecodable `<video>` (H.264) and an undecodable `<audio>` (MP3) — error overlay, not a live-looking bar. R2: with the date/time pref off, a date field is `type=text` on the device. R4: `richlistbox` and `tree` operate in `about:addons` / `about:config` (select, toggle, value persists). R1's other boxes and R5's toast box are open in their own right and are not part of this sweep. *(Done 2026-10-01, CE 3.1.0: R1 the binding attaches; an H.264 `<video>` and an MP3 `<audio>` show "Video format or MIME type is not supported." (`../impl/evidence-2026-10-01/`). R2 `dom.forms.datetime=false` → `<input type=date>` reports `type=text`. R4 `about:addons` richlist rows select and their buttons act and persist; `about:config` tree — filter typed, row tapped, Enter toggled `browser.zoom.full`, the `user_pref` in `prefs.js` after `stop` and still set after a restart, toggled back by a double tap.)*
- [~] **Sweep — cavekit-addons-extensions.md.** R1: the daemon starts with no SIGSEGV in profile startup (`engine up; serving YAP`). R2: `about:addons` renders; lists installed items; enable / disable / remove work and survive a restart; clean SIGTERM flushes `prefs.js` and `extensions.json`; the page is operable through the synthesized input path; the tools menu. R3: opening an `.xpi` triggers the install flow; the prompt identifies the extension; after accepting it is listed and active (the accept is a tap on a card dialog — where no inject path answers it, that step is human-only). R4: a test extension observably alters a page; disabling stops the effect, re-enabling restores it. R5: extensions survive a daemon restart and a reboot; each variant's extensions live in its own profile; removing a variant removes its extensions only. R6: extension data lives under the variant's profile on cryptofs; install AND removal write nothing outside it (only an install was audited on 3.0.5 — audit the removal too). R7: the NPAPI plugin path and `about:plugins`; a windowless control plugin composites at the right position/size/zoom; plugin mouse and key input; the plugin-related YAP commands; a plugin crash does not take the daemon down; Flash — FIRST re-check that `libflashplayer.so`'s dependency closure still resolves on CE (it needed the device's `libssl`/`libcrypto` 0.9.8, `libcurl`, `libWebKitLuna`, `libPiranha`), then that it instantiates and renders `jihad-anim.swf`; the control plugin's documented behaviour. *(2026-10-01, CE 3.1.0, everything except what needs a reboot or a removal: R1 every daemon start logged `engine up; serving YAP`, no fault report. R2–R4 `about:addons` lists, disables, enables and removes a real extension through its own row buttons, each state surviving a restart; SIGTERM flushes `prefs.js`/`extensions.json`; the tools menu works (after two fixes). R3 an `.xpi` by `InstallTrigger`, by a typed address and by a tapped link each raise the install prompt (the last two after a fix); a mismatch is refused with a toast; the accept went over the reply pipe (a finger on the card dialog is in the human-only box). R5/R6 extensions live only in that variant's profile and survive a daemon restart; nothing on `/media/internal`. R7 `libflashplayer.so`'s 67-library closure resolves on CE and `jihad-anim.swf` renders at 33 fps; the control plugin composites at the right place at zoom 1 and at the fit zoom, takes a tap and a key; a killed `plugin-container` leaves the daemon up; `about:plugins` reads back. 2026-10-02: after the reboot the test extension is still in the Enyo and Mochi profiles and still acts (`../impl/evidence-2026-10-02/after-reboot_extension-still-active_enyo_mochi.jpg`); an `InstallTrigger` cancelled with no prompt after the first card closed was fixed (cavekit-browser-services.md R3). Open: "removing a variant removes its extensions only" and the removal half of the R6 audit.)*
- [x] **Sweep — cavekit-preferences-ui.md.** R1: `about:preferences` loads and renders in each variant, `about:settings` is its alias (the bare URL without the card's `#chrome=` fragment was seen showing only header/footer — settle that while there). R2: the named panes are present. R3: a change persists to the variant's own `prefs.js`, the clean-shutdown path flushes it, and it does not leak to another variant's profile. R5: a change in the page reaches the card's state (re-run by COORDINATE click — `clickid` has been seen returning `ok=1` without firing an HTML button's listener). R6: a dropdown in the page opens through the card-side popup path in each variant; no horizontal overflow, in BOTH orientations this time (only 768-wide was measured). *(Done 2026-10-01 (third session), CE 3.1.0, all three variants: R1 `about:preferences` and `about:settings` each load the same six panes; the bare URL renders fully (the 2026-08-15 header/footer-only sighting did not reproduce). R2 Browser / General / Content / Privacy / Security / Advanced, each opened by a coordinate tap. R3 a checkbox and a typed number land in that variant's `prefs.js` only (`1/0/0`, `0/1/0`, `0/0/1`) and survive a card relaunch and `stop`/`start`. R5 an edit on the page reaches the card's start page. R6 a `<select>` raises `popupMenuShow` in each variant; no horizontal overflow at 768 or 1024 wide. Five defects fixed: cavekit-preferences-ui.md's 2026-10-01 changelog entry. Physical rotation and a finger on the controls: the human-only box.)*
- [x] **Sweep — cavekit-input-bridging.md.** R1: `clickAt` hits the element at the content coordinate; click count preserved; checkbox/radio toggle with exactly one `change`; taps on XUL chrome do not crash the daemon; XUL buttons activate. R2: `keyDown`/`keyUp` produce DOM key events; typing inserts the expected characters (ASCII, symbols, non-ASCII); `insertStringAtCursor`; a visible caret (screenshot). R2a: Backspace/Delete by code point; typing over a selection; Enter (newline in a `<textarea>`, submit for an `<input>`) and Tab; number/email inputs accept text. R3: `touchEvent` delivers DOM touch events with the right count and coordinates — re-read the touch-event struct size from the CE `libWebKitLuna.so` FIRST. R5: a click at zoom ≠ 1 lands on the intended link. R6: `about:config` end to end (warning button, filter text, toggle, value in `prefs.js` after stop), `about:addons` buttons, XUL default actions, key input to XUL (Escape closes the tools menu), and no regression to HTML input after the XUL fix. The parts of these that need the on-screen keyboard or a real drag are human-only (last sweep box). *(Done 2026-10-01 (third session), CE 3.1.0, `render/goanna/test/jihad-input-sweep.html`, Enyo (the engine binary is the same in all three). R1 taps hit; `numClicks=2` arrives as `detail=2` with one `dblclick`; one `change` per checkbox/radio tap; XUL chrome taps crash nothing and XUL buttons activate. R2 DOM key codes (after a fix — they were raw); ASCII, the symbol row and `éß漢😀` typed exactly; a visible caret. R2a Backspace and Delete remove one code point; typing over a selection replaces it; Enter is a newline in a `<textarea>` and a submit in a form; Tab moves on; number and email fields hold what was typed. R3 `.text` of the CE `libWebKitLuna.so` equals 3.0.5's, so the 28-byte touch struct stands; one and two fingers give exact coordinates. R5 a tap at the 0.78 fit zoom lands on the link. R6 `about:config` end to end, the add-on row buttons, Escape closes the tools menu, HTML input unharmed.)*
- [x] **Sweep — the card front-ends: cavekit-ui-shell.md, cavekit-mochi-ui.md, cavekit-mojo-ui.md.** For EACH of the three shells, what can be established without a finger: the app launches to its start page and the icon/splash load (screenshot); a launch with a target URL issues `openUrl` and the page paints on this variant's own socket/adapter and nobody else's; the start page renders the STORED shortcut list; the home control is present immediately right of forward (screenshot); a `<select>` on a page produces the card-side list with the real options, anchored under the control (screenshot); a page `alert` reaches the card as a dialog; find-in-page reaches the engine (`find` → 1 / 0 on pages with and without the token); the Mochi variant's db8 kinds (bookmarks, history, downloads) are registered after a fresh install; the Mojo variant runs against the system Mojo framework CE ships (`/usr/palm/frameworks/mojo` present, card launches with no framework error), its command row holds the documented buttons with the documented geometry (the geometry probe), load state / title / committed URL / failed-load error are shown (screenshots); usable layout on the 1024×768 screen for each. Everything that is a TAP on card chrome — back/forward/reload/stop, typing in the address field, the menu entry that opens preferences, editing shortcuts in the card's panel, new card / share / history in Mojo, picking or dismissing a popup item, answering a dialog on the card — is human-only (last sweep box). *(Done 2026-10-01 (third session), CE 3.1.0, all three shells (`render/goanna/test/jihad-shell.html`; `../impl/evidence-2026-10-01/shell-sweep_page_select_alert_enyo_mochi_mojo.jpg`). Each launches to its start page with its icon, the home control right of forward and the stored shortcut list; a launch with a target issues `openUrl` on its own socket and no other daemon sees a connect (Mojo after a fix: it ignored `target`); a `<select>` raises the card list with the page's options next to the control; an `alert` is a card dialog that holds the page until answered; `find` answers 1 / 0 (and the hit is now visible). Mochi's db8 kinds are bookmarks, history and preferences (there is no downloads kind) and answer `returnValue:true`. Mojo runs on the system framework (`/usr/palm/frameworks/mojo/builtins/palmInitFramework506.js`) with 0 card errors; its command row shows back, forward, home, reload, new card, history, share, find (read off the screenshots; the geometry probe was not run); load state, title, URL and the error panel appear.)* *(suite: `chrome-buttons`, `chrome-share`, `chrome-history`, `chrome-padlock`, `card-select`, `card-dialog`; `docs/FINGER-TESTS.md`)*
- [x] **Sweep — the daemon-side kits: cavekit-browser-services.md, cavekit-ipc-contract.md, cavekit-engine-embedding.md, cavekit-navigation-events.md.** browser-services R2: cookies AND the HTTP disk cache persist across a daemon restart (only cookies were ever shown; show the cache too); the low-memory guardrail fires (threshold raised for the test); `clearCache`/`clearCookies` reachable over the variant's Luna service on the public bus. R3: `alert` / `confirm` / `prompt` / HTTP-auth each emit their dialog message and the page stays blocked until the reply arrives over the pipe (only `alert` and `confirm` were ever individually shown; do all four). ipc R4: the Luna service is registered and `clearCookies` reaches the engine (cookie count drops) — on a daemon that was started by the install, not restarted by hand (the R4 criterion above). ipc R5: the rebuilt adapter connects and completes a load + paint cycle. engine-embedding R2: clean shutdown flushes a pref written seconds earlier; stopping the job with a card connected exits without a fault; the bounded teardown holds at shutdown (the job stops on runlevel 0/6 and no fatal-signal report is written) — verify from the job file and from a `stop`, and from a reboot ONLY if one is happening anyway with the unmodified shipped job (this path bricked the device once; never test it with a modified boot-critical file). navigation R4: an ordinary HTTP 3xx is followed in-browser, with the final location reported and no hand-off to another browser. navigation R6: a tap on a link emits link-clicked with the absolute URL (use a coordinate click through the input queue, not `clickid`). *(Done 2026-10-01 (third session), CE 3.1.0. browser-services R2: a cookie and the HTTP disk cache survive `stop`/`start` (`200` then `304` at the server); the guardrail fires at a raised threshold; public-bus `clearCookies`/`clearCache` work. R3: `alert`, `confirm`, `prompt` and HTTP auth each hold the page until the reply (HTTP auth implemented that day). ipc R4: on a daemon started by the install, each variant logs `luna: serving …` and `luna: clearCookies`; the Enyo jar went from 20 cookies to 0. ipc R5: the packaged PDK adapter connects, loads and paints in each variant. engine-embedding R2: a pref toggled seconds before `stop` was in `prefs.js`; dozens of stops with a card connected, no fault report; the job file carries `stop on runlevel [06]`. navigation R4/R6: a `302` is followed to its final location; a coordinate tap emits link-clicked with the absolute URL.)*
- [ ] **Sweep — human-only residue (NOT satisfiable by an agent; listed so it is not mistaken for done).** Collected from the boxes above; each needs a person at the device, and a daemon-level inject does not count: physical rotation, portrait ↔ landscape composite (R4 here, cavekit-offscreen-rendering.md R6); Flash sound heard (cavekit-addons-extensions.md R7) and engine `<audio>` heard on the speaker; every tap on card chrome in each of the three shells (navigation buttons, address-field typing, menu entries, the card-side preferences panels, Mojo's new-card / share / history); picking and dismissing an item in a card-side popup list, and answering a card dialog by tap (including the XPI install confirm and the SSL confirm); text entry through the on-screen keyboard, caret visibility while typing, held-Backspace acceleration (cavekit-input-bridging.md R2/R2a); a real finger drag-scroll and a real two-finger pinch (R4/R3 there); a finger on the preferences page controls and on the media controls (cavekit-preferences-ui.md R6, cavekit-gre-widgets.md R1); looking at a finished download in the Downloads app. Keyboard arbitration on the card side stays hardware-blocked (no keyboard exists), and TouchPad Go stays owner-accepted without hardware (R6).
*(TouchPad Go: see R6, which is the single home for every Opal gate as of 2026-08-04. This requirement is about Topaz.)*
**Dependencies:** R3, R6, cavekit-browser-services.md (R5)

### R6: TouchPad Go (Opal) machine support
**Description:** The device build targets both TouchPad models, since the upstream isis-browser runs on the TouchPad Go.

**Every Opal gate lives in this requirement (consolidated 2026-08-04).** cavekit-mochi-ui.md R4
and cavekit-mojo-ui.md R4 record their Topaz result and point here.

**Acceptance Criteria:**
- [x] The OE build provides machine configurations for both the TouchPad (Topaz/tenderloin) and the TouchPad Go (Opal). *(2026-07-18, T-054: `build/webos-oe/conf/machine/{tenderloin,opal}.conf` + shared `include/jihad-touchpad.inc`; engine+daemon recipes gain `COMPATIBLE_MACHINE = "(tenderloin|opal)"`.)*
- [x] Everything that can be settled WITHOUT the hardware is settled, and what is left is named. *(2026-08-15 (T-122): the Opal kernel string is pinned with `=` to `2.6.35-palm-shortloin` (it was a guessed `2.6.35-palm-opal`): the TouchPad Go's board is `shortloin`, `opal` only the product codename. Derived from HP's Opal drop (webos-internals/webos-linux-kernel branch `opal`, head `683c4b8`: `EXTRAVERSION = -palm`, `CONFIG_LOCALVERSION="-shortloin"` in `shortloin_defconfig`), checked against the same tree's `tenderloin_defconfig` reproducing our device's `2.6.35-palm-tenderloin`, and matching webOS Internals' `support/kernel.mk`. Residual: the drop is 3.0.3-era and no 3.0.5 Opal source was published (LOCALVERSION is board-derived, so the string should not change). 2026-08-04: both models take one ARMv7 softfp binary set (same APQ8060 family, same 1024×768 panel; `docs/DEVICE-BUILD.md`); the only difference is size/DPI, 9.7in ~132 dpi vs 7in ~183 dpi.)*
- [x] **On real TouchPad Go hardware: the three `.ipk`s install, all three variants launch and render, and layout is usable at ~183 dpi.** ~~[DEVICE-GATED — no TouchPad Go present]~~ *(Closed 2026-08-15 by product-owner direction: "touchpad go testing is not necessary, there is no reason it wouldn't work, it's the same screen resolution as the touchpad and all other touchpad apps run on it." Same binary set and panel as Topaz, no hardcoded pixels, all three variants Topaz-verified. If a unit appears: `novacom run file://bin/uname -- -r`.)*
**Dependencies:** R2

### R7: Each UI variant is an independent package
**Description:** The Enyo, Mochi, and Mojo packages are three fully standalone browsers. Installing, upgrading, or removing any one of them has no effect on the others — there is no shared component whose lifetime is owned by more than one package. (User decision 2026-07-31: "each app needs to work entirely on its own, self contained"; supersedes the 2026-07-29 refcount-a-shared-runtime model.)

**Platform constraint discovered 2026-08-01 — no variant's package id may be a DOT-CHILD of another's.** webOS's `ipkg` stores package metadata as `info/<pkgid>.{control,list,prerm}` and cleans up on removal with a glob on `<pkgid>.*`, which also matches `<pkgid>.child.control`. The suffixed ids were therefore renamed from `…jihad-browser.{mochi,mojo}` to `…jihad-browser-{mochi,mojo}`: with the dot, removing the Enyo package destroyed the other two packages' control scripts and file lists, making them un-uninstallable and their rootfs footprint permanent — a P1 against the fourth criterion below and against R8's no-residue rule. Isolated on-device proof (dot pair vs. hyphen pair) in `../impl/impl-ipkg-prefix-collision.md`; the identity table is `../plans/plan-variant-identity.md`.
**Acceptance Criteria:**
- [x] Each variant owns a distinct NPAPI MIME type, adapter shim filename, adapter impl path, YAP service name, YAP socket path, and upstart job name — no two variants resolve to the same one. *(2026-07-31, commit e36c8cc. The table is stated once in `../plans/plan-variant-identity.md` and mirrored by `packaging/gen-variant-scripts.sh`, `build/webos-oe/recipes-jihad/jihad-variants.inc` and the `-D` macros in `build-adapter-{pdk,arm}.sh`. The OE include asserts every column distinct at parse time, and the adapter sources `#error` on a half-defined identity (one would register as Mochi and connect to the Enyo daemon). `strings` over all six `.so`s: each carries only its own MIME, YAP name, impl path and state dir.)*
- [x] Each variant's front-end routes its own WebView to its own MIME type only; another variant's card never loads this variant's adapter. *(2026-07-31, commit e36c8cc. Enyo patches `enyo.BasicWebView.prototype.create`; Mochi's `JihadWebView` declares `application/x-jihad-browser-mochi`; Mojo wraps `Mojo.Widget.WebView.prototype.setup` and hooks `appendChild` on that widget's container for the one synchronous call, because the framework appends its hard-coded `application/x-palm-mojo-browser` `<object>` later in the same call. Each literal is declared once, in its own package; grep shows one MIME per front-end, all distinct. On-device: R4.)*
- [x] Installing variant B while variant A is installed does not overwrite, downgrade, or invalidate any file A owns. *(2026-08-04, full install/remove/reinstall cycle for Mojo with Enyo + Mochi installed, snapshots either side: the install diff was IDENTICAL — the package adds nothing beyond its own payload and touches nothing A owns.)*
- [x] Removing any one variant leaves every other installed variant fully functional, with no refcount logic required to make that true.
- [x] With two or more variants installed, each card is served by its own daemon process on its own socket; a crash or restart of one daemon does not disturb another variant's card.
**Dependencies:** R3, cavekit-ipc-contract.md (R1), cavekit-ui-shell.md, cavekit-mochi-ui.md, cavekit-mojo-ui.md

**Engine PREFS are part of the engine, and are deployed with it (2026-08-04).** Everything that
tunes this embedding — the low-RAM block, the add-on prefs, OMTC-off — is appended to `goanna.js`
by `make-device-bundle.sh`, from `packaging/prefs/jihad-addon-prefs.js` and the bundler's own
blocks. Two consequences that have both bitten: the prefs are read ONCE at engine startup, so a
pref-only change is an engine change and needs a daemon restart, not just a card reload; and
`push-engine-update.sh` must ship `goanna.js` alongside `libxul.so` and the daemon, which it did
not until 2026-08-04 — a pref fixed on desktop left the device silently running the old file.
An absent pref is not always inert: several are read with a ONE-ARGUMENT `getCharPref`, which
THROWS rather than returning a default, so *omitting* one can disable a whole subsystem (it
killed add-on installs, and before that the add-ons manager itself). See
`packaging/prefs/jihad-addon-prefs.js`, which explains each value at the point it is set.

### R8: Good webOS citizen — install footprint contract
**Description:** The package behaves the way webOS expects a third-party app to behave: it keeps its own runtime inside its own app bundle, does not colonize the user's storage, touches the smallest possible amount of the system, and reverses itself exactly. (User decisions 2026-07-31: "without modifying system files", "follow atlas in not copying anything to /media/internal". Reference behavior: the Atlas browser, which runs its engine in place from the app's cryptofs `deviceroot` and copies nothing to `/media/internal`.)

**Supported install path (user decision 2026-08-01): Preware or WebOS Quick Install** — the
constraint Atlas ships under, and the standard one for a PDK-class app that needs root setup.
Both go through Preware's backend, `org.webosinternals.ipkgservice`, whose strings show what it
does (settled 2026-08-05):

```
/usr/bin/ipkg -o /media/cryptofs/apps -force-overwrite install %s
/usr/bin/install -m 755 %s /media/cryptofs/apps/.scripts/%s/pmPostInstall.script   ("stage": "postinst")
/usr/bin/install -m 755 %s /media/cryptofs/apps/.scripts/%s/pmPreRemove.script     ("stage": "install-prerm")
```

`ipkg -o` defers the control scripts (*"(offline root mode: not running …postinst)"*), leaving
them at `usr/lib/ipkg/info/<pkg>.{postinst,prerm}`; the service installs those two as the
app-scoped hooks and runs the postinst. So plain `postinst` + `prerm`, as Atlas ships, is
complete; a `pm*.script` control member is ignored. `palm-install` runs no control scripts: it
produces an app directory and nothing else — no upstart job, adapter shim, adapter impl or
daemon. The development harness (`build/webos-oe/device-independence-test.sh`) invokes
`postinst`/`prerm` directly. Every acceptance result below must name the path it was proven on.
Driving the service directly (both parameters required; the file staged in
`/media/internal/.developer/`):

```
luna-send -n 20 -f palm://org.webosinternals.ipkgservice/install \
  '{"subscribe":true,"filename":"<name>.ipk","url":"file:///media/internal/.developer/<name>.ipk"}'
luna-send -n 12 -f palm://org.webosinternals.ipkgservice/remove \
  '{"subscribe":true,"package":"<appid>"}'
```

The one exception to "nothing on `/media/internal`" (finished user downloads) is the carve-out
criterion below.
**Acceptance Criteria:**
- [x] Install writes NOTHING to `/media/internal` (the user's USB mass-storage volume). The engine, daemon, bundled glibc, and GRE resources are executed in place from the app's own `deviceroot/` on cryptofs. *(2026-07-31, commit e36c8cc, device (Enyo): the daemon runs as `./ld-2.23.so --library-path …/net.riverstonerelay.jihad-browser/deviceroot/hl ./jihad-browserserver …`, and `/media/internal` holds nothing Jihad-named. `/media/cryptofs` is `fuse.cryptofs (rw,nosuid,nodev)`, not noexec. `render/goanna/JihadRuntimePaths.h` refuses any writable path on `/media/internal`, even one supplied through the environment; the adapter's logs moved with the daemon's; the build fails if a control script mentions the volume.)*
- [x] No stock file is modified, replaced, moved, or backed-up-and-swapped. Verified by comparing the checksum of every pre-existing system file the package interacts with (at minimum `/usr/lib/BrowserPlugins/BrowserAdapter.so`, `/usr/lib/BrowserPlugins/BrowserAdapterMojo.so`, `/etc/event.d/browserserver`) before and after install + removal.
- [x] The rootfs footprint is limited to files the package uniquely owns, each namespaced to the variant: its adapter shim under `/usr/lib/BrowserPlugins`, its adapter impl under a Jihad-owned directory, and its own upstart job under `/etc/event.d`. Every such path is enumerated in the packaging docs.

**R7/R8 acceptance run, 2026-08-04 (device, user-authorised install/remove).** A full
install → remove → reinstall cycle for the **Mojo** variant with Enyo and Mochi installed,
snapshotted either side with `device-citizen-audit.sh`: the install added nothing beyond its own
payload; the removal took only Mojo's shim (`BrowserAdapterJihadMojo.so`), impl dir
(`/usr/lib/jihad/mojo/`), job (`jihad-mojo`), state dir (`/var/palm/jihad/mojo/`) and cryptofs
cache/profile, with the stock-file md5 block byte-identical and `/media/internal` untouched;
`device-independence-test.sh check` passed with 0 failures for Enyo and Mochi right after; the
reinstall returned to baseline except the runtime dirs the engine recreates (`cache/`,
`profile/`, `$HOME/.mozilla`, `Desktop`); killing one variant's daemon left the other two
serving and upstart respawned it. Harness notes: `novacom` drops output that arrives after
stdin EOF, so the snapshot helper holds stdin open with a never-EOF FIFO; `push-variant.sh` runs
postinst but registers no ipkg package, so only an `.ipk`-installed variant can be removed. CE
3.1.0 re-run: R4's fresh-install criterion (2026-10-05/06).

- [x] Runtime writable state (log, engine profile/cache, any debug channel) lives under a root-owned, variant-scoped path outside the user's storage, and is removed on uninstall. *(Device-verified 2026-08-04: `$APP/profile` holds the engine profile (`cookies.sqlite`, `prefs.js`, `permissions.sqlite`, `cert9.db`, `key4.db`, `places.sqlite`, `extensions/`, `extensions.json`), `$APP/cache` holds `cache2` and `startupCache`, `/var/palm/jihad/<variant>` holds only the log and debug channels, and `/media/internal` holds nothing of ours. Paths come from `render/goanna/JihadRuntimePaths.h` (one state dir per variant from `JIHAD_BS_NAME`). The profile is on cryptofs because `/var` is small and shared with system state (4.6 MB free that day), and `/media/internal` is VFAT with no hard links or real file locking — isis and Atlas keep cookies and cache on cryptofs too; the disk cache is capped at isis's 50 MB. The daemon creates both trees, so ipkg does not track them and each variant's `prerm` removes them explicitly.)*
- [x] **Carve-out, stated here so it is never mistaken for a violation:** *finished downloads* go to `/media/internal/downloads`, on the user's volume, on purpose. R8 governs APP INTERNALS; a file the user explicitly asked the browser to save is the opposite case, and it has to be reachable from other apps and from USB mass-storage mode, has to survive an uninstall, and has to fit (the rootfs holding `/var` is 559 MB, the user volume is multi-GB). It is also webOS's own convention — still the `DownloadPath` default in `render/browserserver/Src/Settings.cpp`, i.e. what the stock BrowserServer this daemon replaces has always used. *(2026-08-01, review F-10: before, downloads defaulted into the root-only `/var/palm/jihad/<V>/downloads`, invisible to other apps and deleted by `prerm`. Now exactly one destination, `RuntimeUserDownloadDir()` in `render/goanna/JihadRuntimePaths.h`, with its own validator; `RuntimeTryDir`/`RuntimeResolvePath` still refuse every other path on that volume, so a stale `$JIHAD_DUMP`/`$JIHAD_STATE_DIR`/`$JIHAD_PROFILE_DIR` cannot re-colonise it. Control scripts write nothing there and do not delete downloads on uninstall. Device: R4's cert/dialog/download criterion.)*
- [x] `prerm` removes exactly the files this package created and leaves everything else — verified by a full filesystem diff across an install→remove cycle showing no residue and no collateral deletion. *(2026-08-01, device: `device-independence-test.sh matrix` passed 65/66 assertions on a clean device with the hyphenated app ids, including "mochi shim removed" (with the old dotted ids Mochi and Mojo were dot-children of Enyo in ipkg's `info/<pkgid>.*` glob — `../impl/impl-ipkg-prefix-collision.md`). Findings: the daemon runs as `ld-2.23.so`, so `prerm` stops it by argv match (`killall jihad-browserserver` cannot work), and must stop it before removing files or `.fuse_hidden*` files stay on cryptofs. The snapshot excludes `/media/internal/.palm` (webOS's per-app storage, never reclaimed by the OS); a fresh `baseline-v2` was captured after that scope change. 2026-10-05: removal leaves `/media/cryptofs/apps/usr/palm/packages/<id>/` as an EMPTY directory — ipkg removes the two files it tracks (`icon.png`, `packageinfo.json`) but not the directory, and `prerm` runs while they still exist, so it cannot `rmdir` it. Not fixed (deleting ipkg-tracked files from `prerm` needs a trial on a removal the owner runs); `device-citizen-audit.sh` now lists that tree.)*
- [x] The rootfs read-write window is opened only for the duration of the rootfs writes and is restored to read-only on every exit path, including failure. *(2026-08-04: `packaging/gen-variant-scripts.sh` arms `trap 'mount -o remount,ro /' EXIT` before `mount -o remount,rw /` in both `postinst` and `prerm`; the explicit restore disarms it and warns loudly if it fails. Device after every install: `/dev/mapper/store-root on / type ext3 (ro,…)` (`rootfs on / type rootfs (rw)` is the initramfs stub). 2026-10-05, CE, measured from a read-only start: `org.webosinternals.ipkgservice` remounts `/` read-write before it runs any package's script and leaves it so until the next reboot. The scripts open a window only when they find `/` read-only and otherwise leave the mount as found (`/ was already read-write (installer-driven)`). To restore: `mount -o remount,ro /` or a reboot. After an upgrade that replaces the adapter, `/` cannot go read-only until LunaSysMgr restarts (R4).)*
- [x] **The daemon's log is bounded WHILE THE DAEMON RUNS, not only when it starts.** *(Met 2026-10-01, webOS CE 3.1.0, daemon `3e29f045…`, all three variants. (a) With no card, a 603 s window appended 0 bytes to each log; the statistics line prints only for a window with at least 20 ms of work. (b) The daemon checks its log every 30 s and past the cap copies it to `daemon.log.old` and truncates in place (copy-then-truncate, because `plugin-container` inherits the descriptor); at a test cap of 64 KB (`JIHAD_LOG_MAX=65536`, removed afterwards) PID 15840 rotated twice in one lifetime and kept logging. (c) Worst case `daemon.log` ≤ 2 MB (+ ~70 KB overshoot) plus one ≤ 2 MB generation: ~4 MB per variant, ~12 MB for three of a 62 MB `/var`, written in the job comment (`packaging/gen-variant-scripts.sh`) and `packaging/README.md`. Before: the cap was checked only at job start, an idle daemon wrote ~7 MB/day, and a full `/var` freezes the log silently.)*
- [x] **An idle daemon is idle.** *(Met 2026-10-01, webOS CE 3.1.0, all three variants on daemon `3e29f045…` / libxul `65e3cc84…`. The tick runs at the frame period while a card has a page and at 1 s with none (`render/browserserver/Main.cpp`); a connecting card re-arms the fast tick. No card, 62.7 s, from `/proc/<pid>/stat` and `/status`: 0.05–0.06% of a core (was ~2.4%), 1.0 main-thread wakeups/s (was 117), 0 log bytes (was 2970). The inject channel still answers with no card; launch → first non-blank paint 3.68 / 3.19 s on a warm daemon (3.08–4.35 s before); a disconnect returns the daemon to idle on the same PID (`tick: no card — idling at 1000 ms`). With no card, `stop <job>` is honoured within the 1 s tick.)*
**Dependencies:** R3, R7

### R9: Bundled glibc 2.23 vs the device's glibc 2.8 — the shared-object ABI boundary

**Description:** This port runs everything it cross-builds on a **bundled glibc 2.23** under a
bundled loader, while webOS 3's own userland is **glibc 2.8** (verified on device:
`GNU C Library (Sourcery G++ 4.3-234) stable release version 2.8`). That split is normally invisible
— until one of our processes loads a DEVICE library, or shares a kernel/IPC object with a device
process. Then the two libcs must agree on a memory layout they were never built to share, and where
they disagree the failure is silent, intermittent, and looks nothing like its cause.

This is not hypothetical and it is not one bug. It has already produced a total, reboot-triggered
Flash outage that read as a code regression on binaries nobody had touched.

**Acceptance Criteria:**
- [x] The `sem_t` layout split is understood and contained. *(2026-08-10. glibc 2.21 changed
      `sem_t`'s value word from a raw token count to `(tokens << 1) | has-waiters`. The device's
      Flash pulls in `libPmLogLib.so`, whose ELF constructor `sem_wait`s the POSIX named semaphore
      `PmLogLib`; `/dev/shm/sem.PmLogLib` is created once per boot by whichever process opens it
      first, IN THAT PROCESS'S LAYOUT, and outlives every later user. A system process winning that
      race stores raw `1`, our 2.23 reads it as "0 tokens, waiters pending", and `dlopen` never
      returns — the daemon wedges before it has logged a line. **It is a boot race, so it presents
      as intermittent and reboot-triggered.** Verified byte-exact from `/dev/shm`. Contained by
      interposing that ONE name into a process-private semaphore
      (`render/goanna/JihadPmLogSem.c`, patch `0025`), in the EXECUTABLE — a dlopened library
      resolves its PLT against the global scope, which the program image heads. Deliberately NOT by
      repairing the shared semaphore: that would hand every other process on the device an extra
      token on a lock guarding a device-wide table.)*
- [x] Everything else is forwarded rather than blanket-interposed. *(`sem.browserserver.*` are
      created and posted by the ADAPTER — a glibc-2.8 process inside LunaSysMgr — and nothing in
      this port ever calls `sem_wait`, so they are a post-only counter with no consumer and no live
      defect. The trap is ARMED, not sprung: adding a daemon-side `sem_wait` to "restore flow
      control" springs it, and the `nwaiters` field sits at a DIFFERENT OFFSET in the two layouts as
      well, so that wait would block permanently. If a blocking daemon↔adapter handshake is ever
      needed, use a primitive with no userspace layout — a SysV semaphore (`semop`) is a kernel
      object and needs no new YAP field.)*
- [x] Loading a device library from our runtime is proven per library, not assumed. *(The reusable
      technique is a standalone probe that reproduces the exact call sequence, run BOTH under
      `./ld-2.23.so --library-path $HL` and under the device's own loader — the difference between
      the two IS the diagnosis. `render/goanna/test/plugin_mime_probe.c` did this for the Flash
      dlopen; `render/goanna/test/alsa_probe.c` did it for audio and cleared our runtime of blame
      by playing an audible tone from both. `strace` EXISTS on the device (`/usr/bin/strace`) and
      settled the semaphore case in one run.)*
- [x] The remaining cross-ABI surfaces are enumerated rather than discovered one outage at a time —
      at minimum: `pthread_mutex_t`/`pthread_cond_t` in shared memory (`ProcessMutex` is compiled
      into the adapter as `PTHREAD_PROCESS_SHARED` and instantiated by nothing today, so it is the
      next one that would need this analysis), any struct passed through SysV shm, and any device
      library we `dlopen` that itself uses named semaphores or process-shared primitives.
      *(Closed 2026-10-01 on the device (webOS CE 3.1.0, glibc 2.8) with
      `build/webos-oe/r9-device-probes.sh`. (1) A `PTHREAD_PROCESS_SHARED` mutex under the
      device's own `libpthread-2.8.so` initialises with `__kind = 128` and locks one whose `__kind`
      is 128: glibc 2.8 and the bundled 2.23 agree, so a shared mutex created on either side is
      usable on the other (the host mismatch came from the PDK sysroot's glibc 2.5, a proxy).
      (2) `libflashplayer.so`'s 67-library closure (`out-arm/r9-closure`, SHA256SUMS beside it;
      every `DT_NEEDED` resolves on CE; the library is byte-identical to 3.0.5's) has these
      cross-ABI entry points: named semaphores — `libPmLogLib.so` only, contained by
      `JihadPmLogSem.c`; process-shared mutexes — `libLunaSysMgrIpc.so`, `libWebOsProxy.so`,
      `liblunaservice.so`, compatible per (1); a semaphore in shared memory — `libpowerd.so.0`
      (`shm_open` + `mmap` + `sem_init` + `sem_post`/`sem_wait`), a direct dependency of Flash:
      armed, not sprung, the first suspect for a Flash hang after a power-management call; plain
      shared memory — `libWebKitLuna.so`, `liblunaservice.so`, `librt`. Unnamed in-process
      semaphores (`libnapp`, `libremoteplugin`, `libv8`) do not cross the boundary.)*
      *(Host enumeration 2026-08-10 (T-104). Method: this class of bug is host-testable —
      static-link one probe with the PDK gcc 4.3.3 (`build/webos-oe/pdk/opt/PalmPDK/arm-gcc`,
      glibc 2.5 sysroot) and with our gcc 9.4 (glibc 2.23), run both under `qemu-arm`; the
      difference is the ABI. Findings: `sem_t` — 2.5 writes the value word `01 00 00 00`, 2.23
      `02 00 00 00`; a 2.23 process reading the old bytes sees 0 tokens (the PmLogLib hang), the
      reverse over-counts 2x; `nwaiters` is at offset 8 vs 12. `pthread_cond_t` — byte-identical
      through 2.23 and rewritten in 2.25, so bumping the bundled glibc past 2.24 breaks it;
      `pthread_rwlock_t` already differs at 2.23 with the same `sizeof`. `BrowserOffscreenInfo`
      through SysV shm — 32 bytes with the same offsets under both toolchains (plain
      `int`/`double`, fixed by AAPCS); the adapter attaches with `SHM_CACHE_WRITETHROUGH` and the
      daemon with flag 0 (read from code, never measured). `sem.browserserver.*` — the trap does
      not fire on an empty semaphore, so a first daemon-side `sem_wait` would seem to work until
      the adapter posts. Device code enters through two `dlopen`s: the plugin scan
      (`nsPluginsDirUnix.cpp`) and `liblunaservice.so` at every start (`JihadLunaService.cpp`);
      `JihadPmLogSem.c` sits in the program image and covers both. `libflashplayer.so` uses SysV
      semaphores (`semget`/`semop`), which have no userspace layout. Our own process-shared
      primitives (libxul `CrossProcessMutex_posix.cpp`, libvpx, Skia) only talk to our own
      glibc-2.23 `plugin-container`. Non-glibc skew in the same process: the device's
      `liblunaservice.so` (built against glib ~2.16) binds our glib 2.42.1 through `RTLD_GLOBAL`;
      `LSMethod` is 12 bytes on luna-service2 and `LSError` is carried as an over-sized zeroed
      buffer. `ProcessMutex` (adapter only, instantiated by nothing) records the analysis at its
      `setpshared` call.)*

**Dependencies:** R1, R2
**Cross-reference:** cavekit-addons-extensions.md R7 — the plugin host is where this boundary is
crossed most, because a device NPAPI plugin drags the whole webOS WebKit stack into our process.

### R5: Fits the device memory budget
**Description:** The renderer runs within the constraints of a 1 GB device.
**Acceptance Criteria:**
- [x] On a typical page, render-process memory stays within a documented budget. *(2026-08-04, device: the budget is ~90 MB RSS, and a typical page above 150 MB is a regression to investigate. 86.5 MB after an eight-site session, 87.9 MB on a simple page, 105 MB on a large Wikipedia article, of 940 MB total. The engine's stock settings are overridden (the surfacecache default alone is 1 GB). CE 3.1.0: R4's sweep box (64 MB simple page, 126 MB after the scenario).)*
- [x] `freeze`/`purgePage` reclaim memory for backgrounded cards. *(2026-08-04, device: `freeze`, which a card sends when backgrounded, drops RSS 105,156 → 99,236 kB (5.9 MB: the shm attachments are released; the segments are IPC_RMID-marked, so a held attach keeps them alive), and `thaw` restores the page. There is no `purgePage` in the frozen contract (`BrowserServerBase` has none); `freeze`/`thaw` is the whole backgrounding surface, driven here by the `freeze`/`thaw` inject commands.)*
- [x] No out-of-memory crash during a defined browsing scenario. *(2026-08-04, the scenario: example.com, BBC News, two Wikipedia articles, Reddit, Hacker News, CNN, wikipedia.org, ~14 s each, one daemon lifetime. No crash, no fault report, no OOM-killer activity in `dmesg`, still painting at the end, available memory 441 → 406 MB; the same with the memory-pressure guardrail firing five times.)*
**Dependencies:** cavekit-ipc-contract.md (R3)

### R10: CPU frequency scaling is part of the device contract

Added 2026-08-10 after it turned out to be the ceiling on Flash's frame rate AND the likely cause
of its audio xruns. This is a device-platform fact, not a Flash one, so it lives here rather than
in cavekit-addons-extensions.md R7 — anything in this browser that is judged on sustained
throughput will hit it.

The governor is Palm's **`ondemandtcl`**, and it is TOUCH-BIASED: stock `up_threshold` is 95, so
it only raises the clock when a single core exceeds 95% load. Passive work — playing an animation,
decoding audio, anything the user is watching rather than touching — never reaches that and runs
at the 192 MHz floor against a 1188 MHz maximum. Measured during Flash playback: 192000-384000,
snapping to 1188000 the moment the page was navigated away, with the whole system only 69% busy.

**Acceptance Criteria:**
- [x] The mechanism is measured rather than assumed, and the safe lever is identified.
      *(2026-08-10. Governor TUNABLES live at `/sys/devices/system/cpu/cpufreq/ondemandtcl/`;
      writing `up_threshold` and `sampling_rate` is safe, returns instantly, and does not switch
      governors. 40/50000 pins 1188000. The `override/` directory next to it — `turbo_mode`,
      `vdd_freqs` up to 1836000 — is OVERCLOCKING and is out of scope.)*
- [x] **Writing `scaling_governor` is documented as forbidden.** *(2026-08-10, learned by doing
      it: switching away from `ondemandtcl` deadlocks the cpufreq policy lock in the kernel. Every
      subsequent reader of any cpufreq node blocks in unkillable **D state** — the browser daemon,
      `/usr/sbin/powerlog`, and the restore script itself, so the boost is never released. Only a
      reboot clears it, and a clean `reboot` will not run because init waits on the D-state tasks:
      it takes `sync; sync; reboot -f`. Nothing persists, so the device returns to `ondemandtcl`.)*
- [x] Any component that raises the clock owns it and releases it. *(2026-08-10 — libxul patch
      `0027` holds the boost for the life of an NPAPI plugin instance and restores the values it
      read; the variant's upstart job restores stock from `post-stop` so a crashed daemon cannot
      strand a system-wide tuning. Both halves device-verified.)*
- [x] Whether the boost should extend beyond plugins — video, heavy JS, page load — is UNDECIDED.
      Nothing else in the browser currently raises the clock, so any non-plugin workload still
      runs against `up_threshold=95`.
      *(Decided 2026-08-10 (T-118) and re-confirmed 2026-10-01 on webOS CE 3.1.0 (engine
      `65e3cc84…`): DO NOT EXTEND IT. Under stock tunables (`up_threshold=95`,
      `sampling_rate=200000`) the governor ramps by itself for JS (a fixed JS benchmark,
      `/tmp/jihad-bench.html`, ran at 1188 MHz; tuned 40/50000 gained ~11%, within noise) and for
      engine media (`scaling_cur_freq` at 1 Hz: VP8 640x360 @ 24 fps at 648–1188 MHz, 473 decoded /
      0 dropped; VP8 320x240 @ 15 fps 254 / 0; Theora 320x240 @ 15 fps 149 / 0). Flash is the
      exception because its load is spread over two processes and never takes one core past the
      per-core threshold. Patch `0027` stays plugin-only. Method: tunables written and restored
      with a readback in one script; `scaling_governor` only read. Not measured: clips larger than
      640x360.)*

**Dependencies:** cavekit-addons-extensions.md (R7, the first consumer)

### R11: One repository builds Jihad on any x86_64 Linux
**Description:** The owner, 2026-10-14: "it should be as self contained to its own repo as possible
while running on a wide variety of linux distros", and "use git submodules for build dependencies
for automation". Everything a build compiles, links or packages comes from this repository, a git
submodule it pins, or a fetch script in it with a checksum; nothing is read from a sibling folder,
a session scratchpad or an install under `/usr/local` or `/opt`. The host needs as little as
possible, and the parts that depend on the host's age or libraries run in the build container.
**Acceptance Criteria:**
- [x] No build, packaging or test script reads a path outside the repository: the fallbacks to
  `../toolchains/`, `../mochi-sampler/` and `../../webos-stacks/` are gone, and no script names the
  checkout folder (`…/Jihad-Browser/…`). The OE route's `build-webos` tree lives inside the
  repository (git-ignored output).
- [x] Every git-hosted build dependency is a submodule pinned by commit: the engine, Enyo 2, layout
  and Mochi (already), and also crosstool-NG, libpbnjson, yajl, npapi-headers, webOS Archive's
  `webos-sdk-redux` (palm-package, novacom, novacomd, the PDK headers) and openwebos `build-webos`.
  The heavy or optional ones do not check out by default. A fresh `git clone --recurse-submodules`
  gets the rest, and the files Jihad adds to a dependency live in the repository, not in the
  dependency's checkout. *(2026-10-14: `tools/host/check-pins.sh`
  fetches all eleven pins from their public URLs after the owner pushed the engine branch.)*
- [x] The inputs that are not in git are fetched by scripts in the repository and checked by sha256:
  the Jessie sysroot and the Qt4 headers from archive.debian.org (already), and the Palm SDK `.deb`
  (the PDK's gcc 4.3.3) from the Internet Archive. Only the eight device libraries need a TouchPad
  (novacom; seven in `adapter-deps.manifest`, `libpulse.so.0` in `arm-sysroot.recipe`).
- [~] The host needs only git, podman (or docker), curl, binutils (`ar` unpacks the Debian and Palm
  packages) and the common tools every distribution has (bash, coreutils, tar, xz); the README's
  list says exactly that. The PDK's 32-bit compiler and palm-package's Java run inside the build
  container, so a host without 32-bit libraries or Java builds the adapter and the three packages,
  and the result is the same as the host-run build (same adapter `.so` bytes; same package
  contents). The run is shown once on an SELinux-enforcing host and once with a uid other than
  1000; docker is verified or not claimed. *(2026-10-14, round 25: `[~]` — shown under podman on
  this PC only, uid 1000, no SELinux; the toolchain container now takes any uid and disables
  relabelling; the README calls docker rootful and unrecorded.)*
- [x] novacom and novacomd for talking to a TouchPad build from the `webos-sdk-redux` submodule
  with one script, and the README names the packages it needs on the main distribution families.
- [x] The README has a "Building" section: the host packages for apt, dnf, pacman, zypper, xbps and
  apk systems, then the steps from a clean clone to the three `.ipk`s and onto a TouchPad.
- [x] The device test helpers and the review runner that lived in a session scratchpad are in
  `tools/`, with no session paths in them.
- [x] Pins are public: every gitlink in `.gitmodules` fetches from its URL for someone without this
  machine's objects, checked by `tools/host/check-pins.sh` before a tier that moves a pin is called
  done. *(2026-10-14: eleven of eleven after the engine push.)*
- [x] Builds do not depend on where the checkout is: no shipped binary or text names the checkout
  path or the builder's login name, and two checkouts at different paths give the same adapter
  bytes and package contents (BUNDLED-VERSIONS' date aside). *(2026-10-14, round 25: the impl
  carried a Qt header's `__FILE__` through `Q_ASSERT`, the checkout path and the login name in it;
  `-DQT_NO_DEBUG` ends it. Shown: the clean clone at `Jihad/.jihad-cleanclone-test` and this
  checkout give byte-identical shims (3), impls (3, `1bafcc0c`/`4b8c44ba`/`a5c205d1`) and daemon
  (`06e75a68`), and the three packages hold the same files; libxul differs (its build paths), as
  expected.)*

**Dependencies:** R1, R3

## Out of Scope
- Engine integration logic (covered by the Phase-1 domains; reused here).
- Deep performance optimization beyond fitting the budget (Phase 3).

## Cross-References
- See also: cavekit-engine-embedding.md, cavekit-desktop-build.md, cavekit-browser-services.md, cavekit-ipc-contract.md, cavekit-ui-shell.md, cavekit-mochi-ui.md

## Changelog
- 2026-11-07: Sweep: evidence notes compacted to date, task, proof and the deciding numbers; superseded notes and session narrative moved verbatim to `archive/device-build-history.md`. No box changed state.
- 2026-10-14 (round 25, later): the new "path-independent builds" criterion `[x]`: the clean clone
  at another path gives byte-identical shims, impls and daemon, and the same package contents
  (libxul's build id apart). The round-25 impls are on the device, all three variants, example.com
  loaded and painted in each. Criterion 2 waits on the engine push, criterion 4 on another host.
- 2026-10-14 (round 25, the inspector on Tier 84; build site T-479): R11 criteria 2 and 4 back to
  `[~]` (the engine pin was not on the public fork; the host criterion was shown on one host
  only); criterion 3 says eight libraries; two criteria added, public pins and path-independent
  builds; a dozen build-script fixes (the list is in archive/device-build-history.md).
- 2026-10-14 (later): R11's seven criteria `[x]` (Tier 84, T-471…T-477; evidence per task in
  build-site.md and in PICKUP's 2026-10-14 (later) delta): every build output rebuilt byte-identical
  from the repository alone — the adapter (both compilers), the daemon `06e75a68`, the three
  packages (in the containers too), the sysroot by the recipe, the PDK from the Internet Archive,
  the toolchain from the crosstool-NG submodule. The clean-clone run (`Jihad/.jihad-cleanclone-test`)
  got every input without the host's copies; its engine build was still running when this was
  written. R3's prebuilt-inputs carve-out above is annotated accordingly. Review: T-479.
- 2026-10-14: R11 added (the owner): one repository builds Jihad on any x86_64 Linux, with build
  dependencies as git submodules.
- 2026-10-06 (later): R7/R8 re-verified on webOS CE 3.1.0 through the supported installer for Enyo and Mojo (Mochi on 2026-10-05): removal left only the empty package-registry directory, and the fresh install came up with the Luna service on first start; details in R4's fresh-install criterion.
- 2026-10-06: R4 gains the crash-loop guard, the TLS-failure classification, the adapter's `pageFocused` fix and the 2026-10-06 one-build install (with the remount trap).
- 2026-10-05: R4 — a criterion ADDED and met, "an upgrade runs the scripts it installed" (the engine's
  compiled-script cache outlived the scripts; the daemon now drops it when the bundle changes;
  verified by hand and on a real upgrade). Atlas coexistence `[~]`→`[x]` (removal half measured;
  the 2026-10-02 Atlas fault report is Atlas's own, at shutdown). Fresh install: Mochi done from
  nothing, Enyo and Mojo still open. R8 — why `/` is found read-write on CE (the installer service,
  measured from a read-only start); an empty package-registry directory survives removal (recorded,
  not fixed; the audit script now lists it).
- 2026-10-01 (later still): R9 enumeration `[~]`→`[x]` — device probes run: glibc 2.8 writes the same
  process-shared mutex `__kind` as 2.23 (the "second trap" was a 2.5-proxy artefact), and Flash's
  67-library closure resolves on CE with its cross-ABI entry points listed (new named suspect:
  `libpowerd`'s semaphore in shared memory).
- 2026-10-01 (later still): R4 — Luna registration `[ ]`→`[x]` (the -1027 confirmed on the device,
  fixed in postinst and in the daemon, verified on the supported install path); a new criterion
  "an upgrade install does not take LunaSysMgr down" added and met (postinst overwrote the mapped
  adapter in place; now replaces by rename); the fresh-install box stays `[~]` — installed through
  ipkgservice on all three, but over the existing install, because removing a variant needs the
  owner's go-ahead.
- 2026-10-01 (later): R8 "log bounded while the daemon runs" `[ ]`→`[x]` — 0 bytes in a 10-minute
  idle window on all three; two in-lifetime rotations at a lowered cap on one PID.
- 2026-10-01 (later): R8 "an idle daemon is idle" `[ ]`→`[x]` — idle tick implemented and measured on
  the device: 2.4% → 0.05% of a core, 117 → 1 wakeups a second, per variant.
- 2026-10-01 (later): R10 "extend the boost beyond plugins" re-closed `[x]` — measured on engine
  `<video>` under stock tunables: the clock ramps by itself and no frame is dropped; do not extend.
- 2026-10-01: **Target platform is now webOS CE 3.1.0** (header at the top). R4: description
  updated; three criteria added (fresh install on the supported path `[~]`, first-start Luna
  registration `[ ]`, coexistence with Atlas `[ ]`) plus the CE 3.1.0 re-verification sweep, one
  criterion per kit, enumerating every box whose only device evidence is 3.0.5. R8: two criteria
  added from defects measured on CE — the log must be bounded while the daemon runs, and an idle
  daemon must be idle. R10: the "extend the boost beyond plugins" decision `[x]`→`[~]` (it was
  measured on JS only; media playback, the passive class the requirement is about, could not be
  measured at the time).
- 2026-08-10 (T-115): R3's prebuilt inputs declared as a bounded carve-out instead of an
  unmeetable "clean-clone reproducible" gate (toolchain 219 MB, Jessie sysroot 127 MB, Palm PDK
  442 MB — the only permanent one — and adapter-deps 40 MB). R3's two-variant wording corrected to
  three. `.gitignore`'s blanket `/build/webos-oe/toolchain/` narrowed so the toolchain recipe is in
  a clean clone.
- 2026-08-04: Recorded that engine PREFS ship in `goanna.js` and deploy with the engine —
  `push-engine-update.sh` now pushes it, and a pref-only change needs a daemon restart. Also that
  an ABSENT pref can be actively harmful here, because several are read with a one-argument
  `getCharPref` that throws instead of defaulting.
- 2026-06-30: Initial draft.
- 2026-06-30: Two UI `.ipk`s (Enyo + Mochi) in R3; added R6 TouchPad Go (Opal) support; R4 covers both models.
- 2026-07-04: Reconciled — R1 crosstool-NG toolchain (GCC 9.4/glibc 2.23 softfp) verified on-device; R2 DONE this session: X/GTK-free headless libxul cross-built, loads on the TouchPad and renders (on-device offscreen ROUND-TRIP PASS, msgPainted 786432). R3 two-.ipk (Mochi UI + real adapter), R4 full UI-on-screen + TouchPad Go, R5 memory budget (29M libxul helps), R6 Opal remain.
- 2026-07-29: OE build stands up and produces two `.ipk`s (`build/webos-oe/oe-env.sh`, a chroot into an Ubuntu 14.04 rootfs, x86_64 hosts). An adversarial review found R3 over-claimed and it went `[x]`→`[~]`; findings in `../impl/impl-review-findings-oe.md`.
- 2026-07-31 (later): R7 (per-variant independence) and R8 (install footprint) added on user decision: three standalone packages, nothing on `/media/internal`, engine run in place from the app's cryptofs `deviceroot` (the Atlas model). The 2026-07-29 shared-runtime + `prerm` refcount design is retired. R3's Mojo criterion promoted to a working front-end (cavekit-mojo-ui.md).
- 2026-07-31: Reconciliation against recorded evidence: R4 "Composite is correct in portrait and landscape" `[ ]`→`[x]` (PGContext composite, commit 8d7865c, `../impl/rotation-fix-2026-07-26.md`); R4 "Basic navigation" kept `[~]` with its device progress cited; no other box changed.
