---
created: "2026-06-30"
last_edited: "2026-08-04"
---

# Cavekit: Device Build & Packaging

## 2026-07-29 — OE BUILD stands up + produces `.ipk`s, but R3 is NOT complete (READ FIRST)

The reproducible Open webOS path (previously "aspirational, not runnable") now **stands up and
produces `.ipk`s** under `build-webos` + `meta-webos` (2013 "dylan" / bitbake 1.18). But an
adversarial review (gpt-5.6-sol as the cavekit inspector, 2026-07-29 — see
`../impl/impl-review-findings-oe.md`) found R3 was **over-claimed**; it is **build-produced, not
done**. Honest status:
- **Environment:** `build/webos-oe/oe-env.sh` — an OE host via a chroot into a downloaded Ubuntu-14.04
  rootfs (`sudo`/`doas`). Caveat: **x86_64 hosts only** (AMD64 rootfs), and it consumes prebuilt,
  git-ignored inputs (crosstool-NG toolchain, Jessie sysroot, Palm PDK, adapter-deps) — so **NOT
  "whole stack from source" / not clean-clone reproducible** (finding #7). See `docs/OE-BUILD.md`.
- **Recipes** build: `goanna` (libxul), `jihad-browserserver`, `browser-adapter-jihad`,
  `jihad-cross-toolchain-native`, `jihad-deviceroot` (assembles daemon + libxul `.so` closure +
  bundled glibc-2.23 + NSS + GRE). The four component recipes are stage-only; the shipping
  deliverable is **two self-contained app `.ipk`s** (Enyo 39 MB, Mochi 38 MB) + a **Mojo skeleton**.
- **Review fixes applied + build-verified (2026-07-29):** Mochi `.ipk` now stages its
  Enyo2/layout/Mochi frameworks (#1); the shim loads a SHARED root-owned rootfs impl
  `/usr/lib/jihad/BrowserAdapterImpl.so` so Mochi/Mojo load their own impl (#5); `prerm` refcounts
  siblings so removing one variant no longer breaks the other (#4); each `.ipk` ships LICENSE/NOTICE
  (#12); `postinst` fails loud (#6); `oe-env.sh` HTTPS+SHA256 rootfs, `validate_rootfs`, fail-closed
  unmount, pin check, root-uid guard (#2/#3/#10/#11); the bundle assembler aborts on a missing
  required file (#9). Both `.ipk`s (Enyo 40 MB, Mochi 42 MB) are now structurally complete +
  coexistence-safe + license-compliant.
- **2026-07-31 (applied, BUILD-UNVERIFIED — the chroot needs an interactive sudo password, so the
  OE build was NOT run this session; verify at the next `oe-env.sh` run):** every task reading a
  `${JIHAD_REPO}` input now declares it via `do_<task>[file-checksums]`, which the pinned bitbake
  1.18.0 does support (`lib/bb/cache.py:135`, `lib/bb/siggen.py:189-193`) — so sstate stops going
  stale (#8); and the GCC PPA's `[trusted=yes]` is replaced by a full-fingerprint-pinned signing
  key in its own `trusted.gpg.d` keyring, seed-first + keyserver-fallback, failing closed (#2
  residual). Static verification only: bitbake's own line grammar re-run over the recipes,
  `get_file_checksums` semantics simulated, `bash -n`, and `install_ppa_key()` exercised standalone
  (good/decoy/garbage key). See `../impl/impl-review-findings-oe.md`.
- **STILL OPEN for R3:** (a) **clean-clone reproducibility** — the build consumes prebuilt,
  git-ignored inputs (crosstool-NG toolchain, Jessie sysroot, Palm PDK, adapter-deps) (#7); the PDK
  is proprietary so full "from source" is bounded. (#8 is closed in code but not build-verified.)
  (b) **on-device verification** of the `.ipk` install + render + coexistence (device-gated). See
  `../impl/impl-review-findings-oe.md`.

## Scope
Phase-2 work to run Jihad Browser on the HP TouchPad (webOS 3.0.x, ARMv7):
standing up a modern cross-toolchain, cross-compiling the engine and daemon,
packaging, and validating on-device. The toolchain is a feasibility gate. Jihad
packages as a **self-contained app that coexists with the stock browser** — its own
NPAPI MIME/adapter/daemon/upstart job, nothing system-level replaced (see
`jihad-self-contained-arch.md`). Reference: `docs/TOOLCHAIN.md`, `docs/DEVICE-BUILD.md`,
`build/webos-oe/README.md`. (The `build/` tree — toolchain, engine objects, vendored
adapter deps — is git-excluded to avoid redistributing third-party binaries; the build
scripts/recipes and `packaging/` are the reproducible source.)

## 2026-07-31 — packaging model changed by user decision (READ BEFORE TOUCHING PACKAGING)

Two decisions supersede the 2026-07-29 shipping model and are now carried by **R7** and **R8**:
1. **Three independent packages.** Enyo, Mochi, and Mojo each ship a complete browser with their own
   MIME / shim / impl / YAP name / socket / upstart job / daemon process. The shared-runtime +
   `prerm` refcount model (2026-07-29 review fixes #4/#5) is retired — nothing is co-owned, so there
   is nothing to refcount.
2. **Nothing on the user's storage.** `/media/internal` is the user's USB mass-storage volume; the
   package must not write there. The engine + daemon run **in place from the app's own `deviceroot/`
   on cryptofs**, which is the Atlas browser's model (`Atlas/atlas-browser-app/packaging/`: "NOTHING
   is copied to /media/internal — that vfat partition is the user's USB storage and must stay free of
   app internals"). Investigated per the user's request: Atlas found **no** way around the one
   unavoidable rootfs write — webOS's WebKit only scans `/usr/lib/BrowserPlugins` at boot, so the
   NPAPI adapter must be installed there (Atlas README calls it "the **only** path the app loads it
   from"). R8 therefore bounds and reverses that footprint rather than eliminating it.

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

### R3: Package Jihad as a self-contained app — both UI variants
**Description:** The whole product packages as installable webOS artifacts that coexist with the stock browser, including BOTH front-end variants. **Shipping model (2026-07-29): two SELF-CONTAINED app `.ipk`s** (Enyo + Mochi), each bundling engine+daemon+adapter+bundled-glibc+UI + a postinst that deploys the coexisting pieces — plus a Mojo UI skeleton.
**Acceptance Criteria:**
- [x] The daemon + a rebuilt **coexisting** adapter (`BrowserAdapterJihad.so`, MIME `application/x-jihad-browser`, YAP name `jihad-browser`) install ALONGSIDE the stock browser without collision (`packaging/postinst`+`prerm`+`event.d/jihad`). Verified on-device.
- [x] The Enyo UI `.ipk` (`net.riverstonerelay.jihad-browser`, from `app/`) builds (`palm-package app/`) and installs; its WebView is routed to the Jihad engine by `app/source/JihadEngineOverride.js`.
- [x] The build also produces the Mochi variant `.ipk` (`net.riverstonerelay.jihad-browser-mochi`, from `app-mochi/`); both install and can coexist. *(Produced 2026-07-19 (`build-mochi-ipk.sh`, 1.4 MB). INSTALL + COEXIST VERIFIED ON DEVICE 2026-07-19: `palm-install -l` lists both app ids at 1.0.0.)*
- [~] A single OE/bitbake build PRODUCES the two `.ipk`s. *(2026-07-29: `oe-env.sh run ". oe-init-build-env && bitbake net.riverstonerelay.jihad-browser net.riverstonerelay.jihad-browser-mochi"` → the two self-contained `.ipk`s (component recipes stage-only). BUT it consumes prebuilt, git-ignored inputs — crosstool-NG toolchain, Jessie sysroot, Palm PDK, adapter-deps (`jihad-cross-toolchain-native` only CHECKS the toolchain, does not build it) — so it is NOT clean-clone reproducible and NOT "whole stack from source" (review #7). The direct-cross-build scripts remain a faster path. **2026-07-31 — #8 CLOSED in code, build-unverified:** every task that reads a `${JIHAD_REPO}` input (goanna patch queue + mozconfig, browserserver/adapter/deviceroot scripts + sources, PDK, adapter-deps, toolchain, Jessie sysroot, Mochi frameworks, LICENSE/NOTICE) now declares it via `do_<task>[file-checksums]` — confirmed supported by the pinned bitbake 1.18.0 (`lib/bb/cache.py:135` + `lib/bb/siggen.py:189-193`), with small identity sets standing in for the 219 MB toolchain / 127 MB sysroot. Verified statically only (bitbake's own line grammar re-run over the recipes; `get_file_checksums` semantics simulated) — **the chroot was not runnable, so neither `bitbake -p` nor a real sstate run has exercised it; verify at the next `oe-env.sh` run.** Full coverage table + caveats: `../impl/impl-review-findings-oe.md`.)*
- [x] Each variant `.ipk` is SELF-CONTAINED and installs as one coexisting package. *(2026-08-05 — proven on the SUPPORTED path for all three, driving `org.webosinternals.ipkgservice` directly. Each install runs our `postinst`, which lays down the upstart job, adapter shim, adapter impl, state dir and Luna role; afterwards all three variants are registered with ipkg, all three daemons run, and all three Luna services are serving. Survives a reboot: verified from a cold boot with all three coming up on their own.)* *(2026-07-29: `jihad-app.inc` bundles the `jihad-deviceroot` runtime + the UI app + impl + a `postinst`/`prerm`. **Review fixes applied + build-verified:** Mochi now stages its Enyo2/layout/Mochi frameworks (#1); the shim loads a shared root-owned impl `/usr/lib/jihad/BrowserAdapterImpl.so` so every variant loads its own impl (#5); `prerm` refcounts siblings — removing one no longer breaks the other (#4); `postinst` fails loud (#6). Both `.ipk`s (Enyo 40 MB, Mochi 42 MB) are structurally complete + coexistence-safe. STILL `[~]` because (a) not clean-clone reproducible — prebuilt toolchain/sysroot/PDK (#7); the undeclared-bitbake-inputs half (#8) is fixed in code 2026-07-31 via `do_<task>[file-checksums]` but NOT build-verified, (b) not device-verified.)*
- [x] A third UI variant SHIPS as a working Mojo front-end (not a scaffold). *(2026-08-05 — settled. The Mojo variant now installs through the SUPPORTED path (`org.webosinternals.ipkgservice`), registers with ipkg, brings up its own daemon on its own socket, and comes back on its own after a cold boot. Its UI is no longer a stub: its own toolbar with back/forward/reload, circular new-card / history / share controls, a working history scene backed by card-local storage, the shared branded start page, and `<select>` popups handled by the framework. Layout on Topaz is verified (cavekit-mojo-ui.md R4).)* *(2026-07-29: `app-mojo/` scaffold + `net.riverstonerelay.jihad-browser-mojo` recipe build an `.ipk`, but the UI was a documented stub. 2026-07-31: promoted from "scaffolded" to "working" — the user requires all three front-ends to function standalone; the real UI is cavekit-mojo-ui.md.)*
- [x] The Mochi package bundles Enyo 2 + layout + Mochi; the Enyo package bundles Enyo 1.0. *(2026-08-05: verified INSIDE the built `.ipk`, not inferred from the build script — `data.tar.gz` carries `enyo/`, `layout/` and `mochi/` under the app directory, and a generated `BUNDLED-VERSIONS` file records each framework's source path and exact commit (`enyo @ 45315ff` from mochi-sampler, `layout @ 91c0063`, `mochi @ b0da306`), so the provenance travels with the package instead of living only in a build log. The Enyo half is a deliberate deviation, recorded below and not outstanding work.)* *(Mochi half DONE (T-049). Enyo half: INTENTIONAL DEVIATION — `app/index.html` loads Enyo 1.0 from the OS framework path `/usr/palm/frameworks/enyo/0.10`, exactly like upstream isis-browser; bundling would duplicate the system framework and risk skew.)*
**Dependencies:** R2, cavekit-desktop-build.md (R1), cavekit-mochi-ui.md (R1), jihad-self-contained-arch.md

### R4: Runs on the TouchPad (and TouchPad Go)
**Description:** The installed browser works on real hardware; both UI variants.
**Acceptance Criteria:**
- [x] On the TouchPad (Topaz/tenderloin), the **Enyo** variant launches and loads a page rendered on-screen via the Jihad adapter (`example.com`, `slack.com`/HTTPS render on fb1; load-completion + refresh glyph). Mochi variant pending. *(NOTE 2026-07-29: this was verified via the direct-cross-build deploy. Installing the NEW self-contained OE `.ipk`s (R3) on-device and confirming their postinst lays the bundle down + renders is the current open gate — [human-review on device].)*
- [x] Basic navigation works (URL load + load-complete); scrolling/tap-activation/keyboard are in Phase-3 hardening (staged fixes, on-device confirm pending). *(2026-08-05 — closed. The last thing this criterion was actually waiting on was "any verification of the NEW self-contained OE `.ipk`s", and that is now done for all three variants on the supported install path: install runs the postinst, the variant is functional, and it survives a reboot. Navigation itself — address→openUrl, back/forward/reload, link taps, load completion — was already device-verified and is re-confirmed by every session since. The two caveats this note carried are NOT navigation defects and are tracked where they belong rather than holding this box open: the VKB white-band/"snap" jank is cavekit-input-bridging.md R2, and a real two-finger pinch is R3 there.)* *(2026-07-31 reconciliation — recorded device progress since this note was written: back/forward/reload + address→openUrl daemon-verified 2026-07-20 (`../impl/device-test-2026-07-19.md` Session 4); the daemon crash behind the "overload"/stuck-overlay was root-caused + fixed and stress-verified (Session 3, commit 2be6d85); checkbox/radio tap activation verified on-device 2026-07-20 (cavekit-input-bridging R1); link taps hit-test + navigate on-device after the zoom coordinate-consistency fix (commit d4f0842, `../impl/zoom-fix-2026-07-27.md`); pinch/fit zoom magnifies + pans on-device (`../impl/zoom-fix-2026-07-27.md`). REMAINS `[~]` for: the VKB white-band/"snap" jank (`../impl/device-test-2026-07-19.md` "Still open (hard)"), gesture-path pinch (no gesture injection into LunaSysMgr — human check), and any verification of the NEW self-contained OE `.ipk`s.)*
- [x] Composite is correct in portrait and landscape. *(FIXED + device-confirmed 2026-07-27. The portrait 3× shear was the raw `dstBuffer` path: LunaCE reads that surface at a fixed ~256px (1024-byte) pitch, so 768-wide portrait tiled 3× while 1024-wide landscape (4×256) was clean. Fix: composite through the WebKit-provided Piranha **PGContext** (`AdapterBase(..., useGraphicsContext=true)` + `PGSurface::wrap` + `gc->bitblt`), which carries the card's rotation/scale transform, so LunaCE never misreads a raw linear buffer — `../impl/rotation-fix-2026-07-26.md`, cavekit-offscreen-rendering.md R6, commit **8d7865c** (2026-07-27): "correct in both orientations, superseding the white-frame guard … **Verified on-device; the two PG symbols resolve against libWebKitLuna**". Independently corroborated: `../impl/impl-overview.md` 2026-07-27 opens "**Rotation confirmed working on device**", and `../impl/zoom-fix-2026-07-27.md` frames the next user report as arriving "after rotation was confirmed working". Caveat on the strength of the evidence: the confirmation is recorded as a device/visual confirm in the commit + impl entries — no per-orientation screen capture was archived. SUPERSEDES the 2026-07-20 analysis that declared this an unfixable LunaCE limitation (that raw-dstBuffer-era root cause + the disproven write-stride attempt are preserved in `../impl/rotation-fix-2026-07-26.md` and [[jihad-input-activation-and-tiling]]). Separate KNOWN quirk (NOT this): black card + VKB on the FIRST launch after a LunaSysMgr restart, cleared by reopening.)*
- [ ] Cert/dialog/download flows function with the device services. [human-review on device]
*(TouchPad Go: see R6, which is the single home for every Opal gate as of 2026-08-04. This requirement is about Topaz.)*
**Dependencies:** R3, R6, cavekit-browser-services.md (R5)

### R6: TouchPad Go (Opal) machine support
**Description:** The device build targets both TouchPad models, since the upstream isis-browser runs on the TouchPad Go.

**THIS REQUIREMENT IS THE SINGLE HOME FOR EVERY OPAL GATE (consolidated 2026-08-04).** Opal
coverage used to be scattered across five criteria in four kits — device-build R4 and R6,
cavekit-mochi-ui.md R4, cavekit-mojo-ui.md R4 — each independently blocked on the same missing
hardware, each carrying its own half-answer. That made the open list look like five problems
when it is one, and it invited five separate partial verdicts on the same question. Those kits
now record their **Topaz** result and point here. Anything that needs a TouchPad Go to answer
belongs in the criteria below and nowhere else.

**Acceptance Criteria:**
- [x] The OE build provides machine configurations for both the TouchPad (Topaz/tenderloin) and the TouchPad Go (Opal). *(2026-07-18, T-054: `build/webos-oe/conf/machine/{tenderloin,opal}.conf` + shared `include/jihad-touchpad.inc`; engine+daemon recipes gain `COMPATIBLE_MACHINE = "(tenderloin|opal)"`.)*
- [x] Everything that can be settled WITHOUT the hardware is settled, and what is left is named. *(2026-08-04, the consolidation. Settled: both models take one ARMv7 softfp binary set — same APQ8060 family, same 1024×768 panel — so there is no second build (documented per-machine path in `docs/DEVICE-BUILD.md`); the machine configs exist; the only captured difference is physical size/DPI, 9.7in ~132dpi vs 7in ~183dpi, which changes nothing in a layout that uses no hardcoded pixels; all three front-ends are Topaz-verified on hardware (each kit's own R4). Left, and ONLY answerable on an Opal: the criterion below.)*
- [ ] **On real TouchPad Go hardware: the three `.ipk`s install, all three variants launch and render, and layout is usable at ~183 dpi.** [DEVICE-GATED — no TouchPad Go present] *(This one criterion replaces the five that were spread across four kits. It is not partially met and should not be marked `[~]`: nothing about it has been observed, because the machine does not exist here. Two specific things to check first when one does: the `opal` kernel string is still `?=` in the machine config (unverified, Codex F-371), and the higher DPI is the one plausible way a layout that is fine on Topaz could be wrong.)*
**Dependencies:** R2

### R7: Each UI variant is an independent package
**Description:** The Enyo, Mochi, and Mojo packages are three fully standalone browsers. Installing, upgrading, or removing any one of them has no effect on the others — there is no shared component whose lifetime is owned by more than one package. (User decision 2026-07-31: "each app needs to work entirely on its own, self contained"; supersedes the 2026-07-29 refcount-a-shared-runtime model.)

**Platform constraint discovered 2026-08-01 — no variant's package id may be a DOT-CHILD of another's.** webOS's `ipkg` stores package metadata as `info/<pkgid>.{control,list,prerm}` and cleans up on removal with a glob on `<pkgid>.*`, which also matches `<pkgid>.child.control`. The suffixed ids were therefore renamed from `…jihad-browser.{mochi,mojo}` to `…jihad-browser-{mochi,mojo}`: with the dot, removing the Enyo package destroyed the other two packages' control scripts and file lists, making them un-uninstallable and their rootfs footprint permanent — a P1 against the fourth criterion below and against R8's no-residue rule. Isolated on-device proof (dot pair vs. hyphen pair) in `../impl/impl-ipkg-prefix-collision.md`; the identity table is `../plans/plan-variant-identity.md`.
**Acceptance Criteria:**
- [x] Each variant owns a distinct NPAPI MIME type, adapter shim filename, adapter impl path, YAP service name, YAP socket path, and upstart job name — no two variants resolve to the same one. *(2026-07-31, commit e36c8cc. The table is stated once in `../plans/plan-variant-identity.md` and mirrored by exactly three consumers — `packaging/gen-variant-scripts.sh`, `build/webos-oe/recipes-jihad/jihad-variants.inc`, and the `-D` macros in `build-adapter-{pdk,arm}.sh`. Distinctness is machine-checked, not eyeballed: the OE include asserts all columns are mutually distinct at parse time, and the adapter sources `#error` on a half-defined identity — a build that sets the MIME but forgets the YAP name would register as Mochi and connect to the Enyo daemon, which is exactly the hijack this criterion exists to prevent. Verified in the shipped artifacts: `strings` over all six `.so` shows each carries only its own MIME, YAP name, impl path and state dir.)*
- [x] Each variant's front-end routes its own WebView to its own MIME type only; another variant's card never loads this variant's adapter. *(2026-07-31, commit e36c8cc. Enyo patches `enyo.BasicWebView.prototype.create`; Mochi's `JihadWebView` declares `application/x-jihad-browser-mochi` (it had been reusing the Enyo MIME — the co-ownership R7 removes); Mojo wraps `Mojo.Widget.WebView.prototype.setup`, hooking `appendChild` on that widget's own container for the duration of one synchronous call, because the framework hard-codes `application/x-palm-mojo-browser` on a detached `<object>` and appends it later in the SAME call — so mutating it after `setup()` returns is too late. Each literal is declared once, in its own package; no shared constant is imported across variants. Grep-verified: exactly one MIME per front-end, all three distinct. On-device confirmation that a card actually loads its own adapter is part of R4.)*
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

**Supported install path (user decision 2026-08-01): Preware or WebOS Quick Install** — the same
constraint Atlas ships under, and the standard one for a PDK-class app that needs root setup.
Both run the `.ipk`'s control scripts, which is what lays down (and removes) the adapter shim,
the adapter impl and the upstart job. `palm-install` does NOT run them, and a raw
`ipkg -o <offline-root>` **defers** them on install *and* removal and then deletes them — so the
development harness (`build/webos-oe/device-independence-test.sh`) invokes `postinst`/`prerm`
directly to emulate the supported path. Every acceptance result below must name the path it was
proven on.

**HOW INSTALL AND REMOVAL ACTUALLY WORK HERE (settled 2026-08-05, on the supported path).**
Preware's backend is `org.webosinternals.ipkgservice`, and reading its strings settles the whole
question:

```
/usr/bin/ipkg -o /media/cryptofs/apps -force-overwrite install %s
/usr/bin/install -m 755 %s /media/cryptofs/apps/.scripts/%s/pmPostInstall.script   ("stage": "postinst")
/usr/bin/install -m 755 %s /media/cryptofs/apps/.scripts/%s/pmPreRemove.script     ("stage": "install-prerm")
```

`ipkg -o` DEFERS the control scripts — it says so itself in the install log, *"(offline root mode:
not running …postinst)"* — leaving them at `usr/lib/ipkg/info/<pkg>.{postinst,prerm}`. The service
then INSTALLS those two as the app-scoped hooks and RUNS the postinst. So plain `postinst` +
`prerm`, which is exactly what Atlas ships, is complete and correct; **no `pm*.script` control
member is needed** (one was added on a wrong hypothesis and removed again — with or without the
`.script` suffix the installer ignores it, because the control archive is not where it looks).

**Everything therefore hinges on installing the supported way.** `palm-install` does not run
control scripts, and the consequence is not subtle: a `palm-install` of a variant produces an app
directory and NOTHING else — no upstart job, no adapter shim, no adapter impl, no daemon. It
installs and is completely non-functional. Measured twice before the cause was clear.

Driving the service directly is scriptable, which is what made this testable without touching
Preware's UI:

```
luna-send -n 20 -f palm://org.webosinternals.ipkgservice/install \
  '{"subscribe":true,"filename":"<name>.ipk","url":"file:///media/internal/.developer/<name>.ipk"}'
luna-send -n 12 -f palm://org.webosinternals.ipkgservice/remove \
  '{"subscribe":true,"package":"<appid>"}'
```

Both parameters are required, and the file must be staged in `/media/internal/.developer/`.

**The one deliberate exception to "nothing on `/media/internal`" (user decision 2026-08-01):**
**finished user downloads** go to `/media/internal/downloads`, the webOS convention. R8 exists to
keep *app internals* off the user's storage; a file the user asked to download is the opposite
case — it is theirs, it must survive uninstall, and it must be reachable from the Downloads app and
over USB. Engine internals (profile, cache, logs, debug channels) remain barred from that volume
unconditionally, and install/removal still write nothing there.
**Acceptance Criteria:**
- [x] Install writes NOTHING to `/media/internal` (the user's USB mass-storage volume). The engine, daemon, bundled glibc, and GRE resources are executed in place from the app's own `deviceroot/` on cryptofs. *(2026-07-31, commit e36c8cc — VERIFIED ON DEVICE for the Enyo variant: `ps` shows the daemon running as `./ld-2.23.so --library-path …/net.riverstonerelay.jihad-browser/deviceroot/hl ./jihad-browserserver …`, i.e. executing out of the app bundle, with `/media/internal` carrying nothing Jihad-named. Prerequisite established live: `/media/cryptofs` is `fuse.cryptofs (rw,nosuid,nodev)` and is NOT noexec. The daemon's own writable paths (profile/cache/log/frame-dump/inject) moved to `/var/palm/jihad/<variant>/` via `render/goanna/JihadRuntimePaths.h`, which refuses any path resolving onto `/media/internal` even if supplied through the environment; the adapter's paint log and the shim log moved with them, and the build now fails if a control script mentions the volume at all. Three-way matrix verification is R7's row.)*
- [x] No stock file is modified, replaced, moved, or backed-up-and-swapped. Verified by comparing the checksum of every pre-existing system file the package interacts with (at minimum `/usr/lib/BrowserPlugins/BrowserAdapter.so`, `/usr/lib/BrowserPlugins/BrowserAdapterMojo.so`, `/etc/event.d/browserserver`) before and after install + removal.
- [x] The rootfs footprint is limited to files the package uniquely owns, each namespaced to the variant: its adapter shim under `/usr/lib/BrowserPlugins`, its adapter impl under a Jihad-owned directory, and its own upstart job under `/etc/event.d`. Every such path is enumerated in the packaging docs.

**R7/R8 acceptance run, 2026-08-04 (device, user-authorised install/remove).** A full
install → remove → reinstall cycle for the **Mojo** variant with Enyo and Mochi installed,
snapshotted either side with `device-citizen-audit.sh`:
- **install**: diff vs baseline **IDENTICAL** — the package adds nothing beyond its own payload.
- **remove**: every removed line belongs to Mojo and nothing else — its shim
  (`BrowserAdapterJihadMojo.so`), its impl dir (`/usr/lib/jihad/mojo/`), its upstart job
  (`jihad-mojo`), its state dir (`/var/palm/jihad/mojo/`) and its cryptofs cache/profile.
  **The stock-file md5 block is byte-identical** and **`/media/internal` is untouched**.
- **survivors**: `device-independence-test.sh check` passes with **0 failures** for both Enyo
  and Mochi immediately after the removal — no refcounting, no shared component to break.
- **reinstall**: back to baseline except the runtime dirs the engine recreates on first launch
  (`cache/`, `profile/`, `$HOME/.mozilla`, `Desktop`), which is the correct behaviour, not residue.
- **daemon independence**: killing one variant's daemon outright left the other two serving on
  their own sockets, and upstart respawned the killed one.

Two things this run corrected. The harness's `novacom run` helper did not hold stdin open, and
`novacom` discards output arriving after stdin EOF — for a snapshot tool that is a correctness
bug, since a truncated listing is both wrong and unstable between runs; it now uses a
never-EOF FIFO. And only the **Enyo** variant was ipkg-registered: Mochi and Mojo had been
deployed with `push-variant.sh`, which places the payload and runs postinst but registers no
package, so `remove` had nothing to do until Mojo was installed from its `.ipk` first.

- [x] Runtime writable state (log, engine profile/cache, any debug channel) lives under a root-owned, variant-scoped path outside the user's storage, and is removed on uninstall. *(**DEVICE-VERIFIED 2026-08-04** — the daemon half that this note said was outstanding is now done, because the ARM daemon has been rebuilt many times since. Observed on the TouchPad: `$APP/profile` holds the real engine profile (`cookies.sqlite`, `prefs.js`, `permissions.sqlite`, `cert9.db`, `key4.db`, `places.sqlite`, `extensions/`, `extensions.json`), `$APP/cache` holds `cache2` and `startupCache`, `/var/palm/jihad/enyo` holds only the log and debug channels, `ls /var/palm/jihad/*/profile` finds NOTHING (the tell this note named), and `/media/internal` contains nothing of ours. `/var` is at 93% used with 4.6 MB free, which is exactly why the profile does not live there. Original note follows.* *(2026-08-01 — implemented and desktop-verified, NOT yet device-verified, and the distinction matters. `render/goanna/JihadRuntimePaths.h` derives one state dir per variant from `JIHAD_BS_NAME` and refuses any path resolving onto `/media/internal`; `prerm` removes it. But the `.ipk` currently ships an ARM daemon built 2026-07-27, i.e. BEFORE that header existed, so the deployed binary ignores `JIHAD_STATE_DIR` — `/var/palm/jihad/<V>/daemon.log` exists only because the upstart job redirects stdout there, and the absence of a `profile/` subdirectory under the state dir is the tell. **The packaging half of R8 is device-verified; the daemon half is not.** Rebuild the ARM daemon and re-verify. Settled while investigating: `/var` is its own rw LVM volume (`/dev/mapper/store-var … ext3 (rw,noatime)`), independent of the read-only `/`, so the state dir does not silently fall through to the `/tmp` tier. Sub-item CLOSED 2026-08-01 (review F-10): downloads no longer default into this dir — see the carve-out criterion below. REVISED the same day: the ENGINE PROFILE moved out of this dir too. `/var` is 49.6 MB free and shared with system state, which is not a browser-profile budget, and both prior implementations of this browser on this device put cookies AND cache on cryptofs — isis ships `CookieJarPath=/media/cryptofs/.browser/cookies` + `CachePath=…/cache` + `CacheMaxSize="50M"`, and Atlas routes netdata/netcache/cookies.db to its app deviceroot on cryptofs *because* /media/internal is VFAT with no hard links and no real file locking (which is also why our own cookies.sqlite was never created on-device on 2026-07-20). So: `$APP/profile` = Gecko's ProfD, `$APP/cache` = ProfLD, disk cache capped at isis's own 50 MB; `/var/palm/jihad/<V>/` keeps ONLY the daemon log and the debug channels. Both trees are created by the DAEMON, so ipkg does not track them and each variant's `prerm` removes them explicitly — otherwise they would be residue and this criterion would regress.)*
- [x] **Carve-out, stated here so it is never mistaken for a violation:** *finished downloads* go to `/media/internal/downloads`, on the user's volume, on purpose. R8 governs APP INTERNALS; a file the user explicitly asked the browser to save is the opposite case, and it has to be reachable from other apps and from USB mass-storage mode, has to survive an uninstall, and has to fit (the rootfs holding `/var` is 559 MB, the user volume is multi-GB). It is also webOS's own convention — still the `DownloadPath` default in `render/browserserver/Src/Settings.cpp`, i.e. what the stock BrowserServer this daemon replaces has always used. *(2026-08-01, review F-10. Before this, downloads defaulted into the root-only, package-owned `/var/palm/jihad/<V>/downloads`: invisible to every other app and to the PC, `rm -rf`'d by `prerm` on uninstall (and, before F-6, on every upgrade), and filling the system partition instead of the user volume. The carve-out is exactly ONE destination, named once as `RuntimeUserDownloadDir()` in `render/goanna/JihadRuntimePaths.h`, with its own validator — the shared `RuntimeTryDir`/`RuntimeResolvePath` guard still refuses every OTHER path on that volume, so a stale `$JIHAD_DUMP`/`$JIHAD_STATE_DIR`/`$JIHAD_PROFILE_DIR` cannot re-colonise it. `postinst`/`prerm` still write nothing to `/media/internal` and deliberately do not delete the user's downloads on uninstall; the build-time control-script assertion and the on-device `find /media/internal -iname '*jihad*'` check are both unaffected, since this path is not Jihad-named. Desktop-verified; on-device download-to-user-volume is device-gated.)*
- [x] `prerm` removes exactly the files this package created and leaves everything else — verified by a full filesystem diff across an install→remove cycle showing no residue and no collateral deletion. *(2026-08-01 — VERIFIED ON DEVICE, **65/66 assertions pass**, `device-independence-test.sh matrix` under the corrected hyphenated app ids against a genuinely clean device. "mochi shim removed" passes, which is the criterion that was silently failing before. Getting here required fixing a P1 the matrix itself exposed: webOS `ipkg` stores metadata as `info/<pkgid>.*` and globs that on removal, so the old dotted ids made Mochi and Mojo **dot-children** of Enyo and uninstalling Enyo destroyed their uninstall metadata — see `../impl/impl-ipkg-prefix-collision.md`, proven with two pairs of minimal packages differing only in the separator. Two further device findings recorded there: `killall jihad-browserserver` could never have worked (the daemon is exec'd via `ld-2.23.so`, so that is its process name — the argv-match `prerm` uses is required, not stylistic), and uninstalling while the daemon still runs leaves `.fuse_hidden*` files on cryptofs until the handles close, which is the concrete reason `prerm` must stop the daemon BEFORE files are removed. The residue snapshot excludes `/media/internal/.palm` — webOS's own per-app storage area, measured at 14991 entries with subdirectories dated March 2024 on a device with nothing of ours installed; the OS provisions it for any app and does not reclaim it on uninstall, and our control scripts are separately proven never to reference `/media/internal` by a build-time assertion. The one remaining `[~]`-worthy caveat: that exclusion changed the snapshot's scope, so the pre-change baselines are not comparable to post-change ones; a fresh `baseline-v2` was captured and the next full cycle should be diffed against it.)*
- [x] The rootfs read-write window is opened only for the duration of the rootfs writes and is restored to read-only on every exit path, including failure. *(2026-08-04, verified two ways. In the source: `packaging/gen-variant-scripts.sh` arms `trap 'mount -o remount,ro /' EXIT` BEFORE `mount -o remount,rw /`, in both `postinst` and `prerm`, so a crash mid-install cannot leave the rootfs writable; the explicit restore then disarms the trap on success and prints a loud warning to stderr (where ipkg records it) if it fails. On the device, after every install and engine push this session: `/dev/mapper/store-root on / type ext3 (ro,…)` — the real root is READ-ONLY. Note `rootfs on / type rootfs (rw)` in the same listing is the initramfs stub underneath, not the writable state this criterion is about.)*
**Dependencies:** R3, R7

### R5: Fits the device memory budget
**Description:** The renderer runs within the constraints of a 1 GB device.
**Acceptance Criteria:**
- [x] On a typical page, render-process memory stays within a documented budget. *(2026-08-04, measured on the TouchPad — **the budget is now a number rather than an aspiration: ~90 MB RSS, and it must stay under 150 MB.** Observed: 86.5 MB after an eight-site session, 87.9 MB back on a single simple page, 105 MB on a large Wikipedia article. The device has 940 MB total, so a browser card sits at roughly 9–11% of system memory. Compare the engine's own stock settings, which this build overrides: the surfacecache default alone is 1 GB. If a future change pushes a typical page past 150 MB, treat it as a regression and find out what grew.)*
- [x] `freeze`/`purgePage` reclaim memory for backgrounded cards. *(2026-08-04, on device: `freeze` — the command a card sends when it is backgrounded — drops RSS from 105,156 kB to 99,236 kB, **5.9 MB reclaimed**, and `thaw` brings the page back with the daemon alive throughout. The saving is the shm attachments being released: the production segments are IPC_RMID-marked, so holding an attach keeps them alive and would waste the device's scarce shared memory for as long as the card sits in the background. One correction to this criterion's wording: **there is no `purgePage` in the frozen contract** — `grep` finds none in `BrowserServerBase`. `freeze`/`thaw` is the whole backgrounding surface, and it is the one measured here. Driven through new `freeze`/`thaw` inject commands, because until now nothing but a real card could exercise this path.)*
- [x] No out-of-memory crash during a defined browsing scenario. *(2026-08-04 — the scenario, so it can be re-run: eight sites in sequence (example.com, BBC News, two Wikipedia articles, Reddit, Hacker News, CNN, wikipedia.org) at ~14 s each, on the device, in one daemon lifetime. Result: no crash, no fault report, `dmesg` shows no OOM-killer activity, the daemon was still painting at the end, and available system memory moved only 441 → 406 MB. Repeated with the memory-pressure guardrail firing five times mid-session, with the same outcome.)* *(prior note: [human-review on device; deeper tuning is Phase 3]
**Dependencies:** cavekit-ipc-contract.md (R3)

## Out of Scope
- Engine integration logic (covered by the Phase-1 domains; reused here).
- Deep performance optimization beyond fitting the budget (Phase 3).

## Cross-References
- See also: cavekit-engine-embedding.md, cavekit-desktop-build.md, cavekit-browser-services.md, cavekit-ipc-contract.md, cavekit-ui-shell.md, cavekit-mochi-ui.md

## Changelog
- 2026-08-04: Recorded that engine PREFS ship in `goanna.js` and deploy with the engine —
  `push-engine-update.sh` now pushes it, and a pref-only change needs a daemon restart. Also that
  an ABSENT pref can be actively harmful here, because several are read with a one-argument
  `getCharPref` that throws instead of defaulting.
- 2026-06-30: Initial draft.
- 2026-06-30: Two UI `.ipk`s (Enyo + Mochi) in R3; added R6 TouchPad Go (Opal) support; R4 covers both models.
- 2026-07-04: Reconciled — R1 crosstool-NG toolchain (GCC 9.4/glibc 2.23 softfp) verified on-device; R2 DONE this session: X/GTK-free headless libxul cross-built, loads on the TouchPad and renders (on-device offscreen ROUND-TRIP PASS, msgPainted 786432). R3 two-.ipk (Mochi UI + real adapter), R4 full UI-on-screen + TouchPad Go, R5 memory budget (29M libxul helps), R6 Opal remain.
- 2026-07-29: OE build **stands up + produces two `.ipk`s** (new `build/webos-oe/oe-env.sh` — chroot Ubuntu-14.04 OE host, x86_64, sudo/doas) — engine/daemon/adapter/deviceroot recipes + `jihad-app.inc` products + a Mojo skeleton. But an adversarial review (**gpt-5.6-sol as the cavekit inspector**) found R3 was **OVER-CLAIMED** → reverted `[x]`→`[~]` (build-produced, not done). Real gaps: **Mochi `.ipk` missing its Enyo2/layout/Mochi frameworks** (#1) + adapter shim impl-path **hard-coded to the Enyo appid** (#5); **coexistence-removal breaks the survivor** (both share `/media/internal/jihad/hl`+shim+upstart, #4); **not clean-clone reproducible** (prebuilt toolchain/sysroot/PDK, #7); postinst masks failures (#6); LICENSE/NOTICE payload dropped (#12); none device-verified. Full findings: `../impl/impl-review-findings-oe.md`.
- 2026-07-31 (later): **R7 (per-variant independence) + R8 (good-citizen install footprint) added** on user decision — three fully standalone packages, nothing written to `/media/internal`, engine run in place from the app's cryptofs `deviceroot` (Atlas model). Retires the shared-runtime + `prerm` refcount design from the 2026-07-29 review (#4/#5). R3's Mojo criterion promoted from "scaffolded" to "ships a working front-end" (new cavekit-mojo-ui.md). Atlas was checked at the user's request for a way to avoid the `/usr/lib/BrowserPlugins` write and has none — that write is bounded + reversed by R8 instead.
- 2026-07-31: Reconciliation against recorded evidence. **R4 "Composite is correct in portrait and landscape" `[ ]`→`[x]`**: its 2026-07-20 note declared the portrait 3× shear an unfixable LunaCE limitation, but that was superseded by the PGContext composite fix — `../impl/rotation-fix-2026-07-26.md` + commit 8d7865c ("correct in both orientations … Verified on-device; the two PG symbols resolve against libWebKitLuna"), with `../impl/impl-overview.md` 2026-07-27 ("Rotation confirmed working on device") and the `../impl/zoom-fix-2026-07-27.md` preamble as independent corroboration; the old analysis is kept as a one-line history pointer. R4 "Basic navigation" keeps `[~]` but its note now cites the device-recorded progress (Session-4 nav sweep, crash fix 2be6d85, tap activation, link-tap hit-test d4f0842, zoom) and names exactly what remains (VKB jank, gesture-path pinch, OE `.ipk` verification). No other box changed — R3 stays `[~]` on the still-open clean-clone reproducibility (#7/#8) and device verification recorded in the READ-FIRST block above.
