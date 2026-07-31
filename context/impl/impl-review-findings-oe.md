---
created: "2026-07-29"
last_edited: "2026-07-29"
---

# OE build — adversarial review findings (gpt-5.6-sol as cavekit inspector)

Adversarial review of the OpenEmbedded build-from-source work (device-build R3): reviewer =
**codex `gpt-5.6-sol` @ high, run as the cavekit inspector** (2026-07-29). It inspected the recipes,
`oe-env.sh`, the packaging scripts, the produced 39/38 MB `.ipk`s, and the kits. **Verdict: R3's
"done" was an OVERCLAIM** — bitbake produces two large `.ipk`s and the engine/daemon path works, but
the Mochi package is structurally unrunnable and the coexistence install/removal design is unsafe.
Honest status = **build-produced, not complete** (the kit was corrected to match).

Status legend: ✅ fixed · 📝 kit corrected to reflect it · ⬜ open (tracked for R3 follow-up).

| # | Sev | Finding | Disposition |
|---|-----|---------|-------------|
| 1 | critical | **Mochi `.ipk` omits Enyo2 / layout / Mochi frameworks** — `app-mochi/index.html` loads `enyo/enyo.js`, `lib/layout/package.js`, `lib/mochi/package.js`; the recipe copies only `app-mochi/*` (the direct `build-mochi-ipk.sh` stages them from the `third_party/*` submodules). App fails before creating `JihadBrowser`. | ✅ fixed (mochi recipe stages the frameworks) + 📝 |
| 2 | critical | **Unauthenticated root-level supply chain** — Ubuntu rootfs over plain HTTP, no checksum, extracted+run as root; GCC PPA `[trusted=yes]` disables apt auth. Chroot is not a security boundary. | ✅ partial (rootfs → HTTPS + SHA256 pin) ⬜ PPA key |
| 3 | critical | **`OE_ROOTFS` cleanup can `rm -rf` arbitrary host trees / traverse a live bind** — target unvalidated, `/` check only `return`s, unmount failures ignored then delete, substring mount-match. | ✅ fixed (validate target + component-boundary match + abort if mounts remain) |
| 4 | high | **Removing either UI `.ipk` destroys the other's runtime** — both install the same `/media/internal/jihad/hl` + system shim + `/etc/event.d/jihad`; either `prerm` deletes all three. | ✅ fixed — `pkg_prerm` refcounts sibling `net.riverstonerelay.jihad-browser*` app dirs and only tears down the shared runtime when it is the LAST variant |
| 5 | high | **Shim impl-path hard-coded to the Enyo appid** — `BrowserAdapterShim.cpp` `kImplTrusted = .../net.riverstonerelay.jihad-browser/BrowserAdapterImpl.so`; Mochi/Mojo can't load their own impl. | ✅ fixed + verified — shim now loads a SHARED root-owned rootfs path `/usr/lib/jihad/BrowserAdapterImpl.so` first (variant-agnostic; passes the trust check); each package's postinst deploys the impl there. Confirmed in the shim binary + both `.ipk`s. |
| 6 | high | **`postinst` reports success after partial/failed deploy** — remount/cp/chmod/start failures ignored, ends `true`. | ✅ fixed (critical ops fatal + verify before success) |
| 7 | high | **Not reproducible from a clean checkout** — consumes git-ignored prebuilt inputs (crosstool-NG toolchain, Jessie sysroot, Palm PDK, adapter-deps); `jihad-cross-toolchain-native` only CHECKS the toolchain. Contradicts "whole stack from source". | 📝 kit + docs corrected (claim downgraded) + ⬜ (pin/checksum construction) |
| 8 | high | **Bitbake task signatures omit `${JIHAD_REPO}` inputs** (patches, mozconfig, bundler, event.d, PDK, sysroot) → stale sstate when they change. | ⬜ (declare via SRC_URI / file-checksums; build under WORKDIR) |
| 9 | medium | **Patch + runtime-closure failures hidden** — `patch … \|\| true`; unresolved `DT_NEEDED` printed not fatal; NSS/GRE optional copies. | ✅ partial — `make-device-bundle.sh` now aborts if the bundle is missing any REQUIRED file (daemon + libxul + glibc core + NSS TLS modules + goanna.js); verified (it caught a missing file). Patch-reject `.rej` fatal-check deferred (it would force a ~1h engine rebuild) → folds in at the next engine pin bump. |
| 10 | medium | **Existing `build-webos` checkout not pinned/validated** — pin only applied on fresh clone. | ✅ fixed (verify HEAD==pin, warn/fetch on mismatch) |
| 11 | medium | **"Any Linux" + direct-root claims false** — AMD64 rootfs (no ARM64), `chroot --userspec`/`mount --rbind`/`umount -R` not BusyBox-universal, `HOST_UID=0` when run as root. | 📝 docs qualified to x86_64 + ✅ (require OE_HOST_UID when euid 0) |
| 12 | medium | **`.ipk`s omit LICENSE/NOTICE/attribution** despite bundling MPL Goanna + Apache parts (direct `build-mochi-ipk.sh` shipped them). | ✅ fixed (jihad-app.inc installs LICENSE+NOTICE) |
| 13 | high | **Cavekit R3 status contradicted by the code** — R3 marked done; overview line 29 (Mochi 0/5) vs line 37 (device build complete) inconsistent; Mojo "engine works" unproven. | ✅ fixed (R3 reverted to build-produced; overview + impl-overview corrected) |

## Follow-up for R3 "done" (gates to pass, per the reviewer)
`build-produced` ✓ · `payload structurally validated` (Mochi frameworks #1) · `clean-clone
reproducible` (#7/#8) · `single-variant install/run` (#5) · `dual-version coexistence` (#4) ·
`uninstall-one-survivor` (#4) · `device verified`. Mark R3 complete only after these pass.
