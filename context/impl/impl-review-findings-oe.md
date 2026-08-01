---
created: "2026-07-29"
last_edited: "2026-07-31"
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
| 2 | critical | **Unauthenticated root-level supply chain** — Ubuntu rootfs over plain HTTP, no checksum, extracted+run as root; GCC PPA `[trusted=yes]` disables apt auth. Chroot is not a security boundary. | ✅ fixed (rootfs → HTTPS + SHA256 pin; PPA key pinned by full fingerprint — see below) |
| 3 | critical | **`OE_ROOTFS` cleanup can `rm -rf` arbitrary host trees / traverse a live bind** — target unvalidated, `/` check only `return`s, unmount failures ignored then delete, substring mount-match. | ✅ fixed (validate target + component-boundary match + abort if mounts remain) |
| 4 | high | **Removing either UI `.ipk` destroys the other's runtime** — both install the same `/media/internal/jihad/hl` + system shim + `/etc/event.d/jihad`; either `prerm` deletes all three. | ✅ fixed — `pkg_prerm` refcounts sibling `net.riverstonerelay.jihad-browser*` app dirs and only tears down the shared runtime when it is the LAST variant |
| 5 | high | **Shim impl-path hard-coded to the Enyo appid** — `BrowserAdapterShim.cpp` `kImplTrusted = .../net.riverstonerelay.jihad-browser/BrowserAdapterImpl.so`; Mochi/Mojo can't load their own impl. | ✅ fixed + verified — shim now loads a SHARED root-owned rootfs path `/usr/lib/jihad/BrowserAdapterImpl.so` first (variant-agnostic; passes the trust check); each package's postinst deploys the impl there. Confirmed in the shim binary + both `.ipk`s. |
| 6 | high | **`postinst` reports success after partial/failed deploy** — remount/cp/chmod/start failures ignored, ends `true`. | ✅ fixed (critical ops fatal + verify before success) |
| 7 | high | **Not reproducible from a clean checkout** — consumes git-ignored prebuilt inputs (crosstool-NG toolchain, Jessie sysroot, Palm PDK, adapter-deps); `jihad-cross-toolchain-native` only CHECKS the toolchain. Contradicts "whole stack from source". | 📝 kit + docs corrected (claim downgraded) + ⬜ (pin/checksum construction) |
| 8 | high | **Bitbake task signatures omit `${JIHAD_REPO}` inputs** (patches, mozconfig, bundler, event.d, PDK, sysroot) → stale sstate when they change. | ✅ fixed via per-task `file-checksums` (see below) — **parse/runtime NOT verified** |
| 9 | medium | **Patch + runtime-closure failures hidden** — `patch … \|\| true`; unresolved `DT_NEEDED` printed not fatal; NSS/GRE optional copies. | ✅ partial — `make-device-bundle.sh` now aborts if the bundle is missing any REQUIRED file (daemon + libxul + glibc core + NSS TLS modules + goanna.js); verified (it caught a missing file). Patch-reject `.rej` fatal-check deferred (it would force a ~1h engine rebuild) → folds in at the next engine pin bump. |
| 10 | medium | **Existing `build-webos` checkout not pinned/validated** — pin only applied on fresh clone. | ✅ fixed (verify HEAD==pin, warn/fetch on mismatch) |
| 11 | medium | **"Any Linux" + direct-root claims false** — AMD64 rootfs (no ARM64), `chroot --userspec`/`mount --rbind`/`umount -R` not BusyBox-universal, `HOST_UID=0` when run as root. | 📝 docs qualified to x86_64 + ✅ (require OE_HOST_UID when euid 0) |
| 12 | medium | **`.ipk`s omit LICENSE/NOTICE/attribution** despite bundling MPL Goanna + Apache parts (direct `build-mochi-ipk.sh` shipped them). | ✅ fixed (jihad-app.inc installs LICENSE+NOTICE) |
| 13 | high | **Cavekit R3 status contradicted by the code** — R3 marked done; overview line 29 (Mochi 0/5) vs line 37 (device build complete) inconsistent; Mojo "engine works" unproven. | ✅ fixed (R3 reverted to build-produced; overview + impl-overview corrected) |

## 2026-07-31 — #8 and #2-residual fixed (NOT build-verified)

> **Verification caveat, stated up front.** The chroot could not be entered this session (`sudo`
> needs an interactive password), so **the OE build was not run**: neither `bitbake -p` (parse) nor
> a task-hash/sstate run exercised these changes. What *was* verified is listed per item below.
> **Verify both at the next `oe-env.sh` run.**

### #8 — `${JIHAD_REPO}` inputs now declared in task signatures

**Mechanism: the per-task `do_<task>[file-checksums]` varflag — confirmed SUPPORTED in the pinned
bitbake 1.18.0** (`build-webos` rev `37540e5` → `bitbake` rev `0f7b6a0`, `lib/bb/__init__.py`
`__version__ = "1.18.0"`). Source evidence:

- `lib/bb/cache.py:135` — `self.file_checksums = self.flaglist('file-checksums', self.tasks, metadata, True)`
  (collected per task at parse, expanded).
- `lib/bb/siggen.py:189-193` — `if task in dataCache.file_checksums[fn]: checksums =
  bb.fetch2.get_file_checksums(...)` … `data = data + cs` → each listed file's md5 is folded into
  the **task hash** (and therefore the sstate hash).
- `lib/bb/fetch2/__init__.py:917` `get_file_checksums()` — `*` globs expanded, directories
  `os.walk`ed, missing paths caught (`except OSError` around `cached_mtime`) → warn + skip.
- `lib/bb/checksum.py` `FileChecksumCache` — md5s cached by mtime, so re-hashing is rare.

No fallback (parse-time `${@...}` hashing) was needed.

**Coverage** — every recipe that reads `${JIHAD_REPO}`:

| recipe | task | declared inputs |
|---|---|---|
| `goanna` | `do_apply_jihad_patches` | `build/desktop/patches/*.patch` |
| `goanna` | `do_configure` | `mozconfig.goanna-arm` + `JIHAD_TC_SIG` + `JIHAD_SYS_SIG` |
| `goanna` | `do_compile` | `JIHAD_TC_SIG` + `JIHAD_SYS_SIG` |
| `goanna` | `do_install` | `JIHAD_TC_SIG` (its `strip` shrinks libxul) |
| `jihad-browserserver` | `do_compile` | `JIHAD_TC_SIG` + `JIHAD_SYS_SIG` |
| `browser-adapter-jihad` | `do_compile` | `build-adapter-pdk.sh`, `render/adapter`, `render/browserserver/{Yap,Src}`, `JIHAD_PDK_SIG`, `JIHAD_DEPS_SIG`, `JIHAD_SYS_SIG` |
| `jihad-deviceroot` | `do_compile` | `make-device-bundle.sh`, `packaging/event.d/jihad`, `JIHAD_TC_SIG`, `JIHAD_SYS_SIG` |
| `jihad-cross-toolchain-native` | `do_install` | `JIHAD_TC_SIG` |
| `jihad-ui/jihad-app.inc` (all 3 variants) | `do_install` | `LICENSE`, `NOTICE`, `licenses/*.txt` |
| `…jihad-browser-mochi` | `do_install` (`+=`) | `third_party/{mochi-sampler/enyo,enyo-layout,mochi}` |

Already covered without new declarations, and left alone: `SRC_URI` entries (`app/`, `app-mochi/`,
`app-mojo/`, `render/`, `packaging/`) via oe-core's `do_fetch[file-checksums]`
(`base.bbclass:98`); the UXP tree via its pinned `SRCREV`; cross-recipe staged artifacts via the
`DEPENDS` task-hash chain. Deliberately **not** declared: `browser-adapter-jihad`'s `do_install`
inputs — they are `do_compile`'s *outputs*, and task hashes are computed before the build runs.

**Large prebuilt inputs — what was chosen instead of a whole-tree hash, and why:**

- `JIHAD_TC_SIG` = the gcc driver set + crosstool-NG's saved `build.log*`, standing in for the
  **219 MB** toolchain. Both are rewritten by any re-assembly of the toolchain, so the pair
  identifies the build without walking the tree at every runqueue prep.
- `JIHAD_SYS_SIG` = the Jessie sysroot's pkg-config manifest (89 `.pc` files, ~330 KB), standing
  in for the **127 MB** sysroot. This is *precisely what the builds consume* — `PKG_CONFIG_LIBDIR`
  points at exactly those two dirs — and every `.pc` carries its library's `Version`, so the set
  changes when the sysroot is re-assembled from different `.deb`s.
- `JIHAD_PDK_SIG` = the PDK's gcc 4.3.3 drivers (the SDK is proprietary + git-ignored).
- `JIHAD_DEPS_SIG` = the staged device `.so`s (the real link inputs) + npapi/pbnjson headers +
  `glib_compat.c`. The Qt4 header drop (15 MB / 2857 files) is deliberately not walked: it comes
  from the same device Qt 4.8.0 package as the staged `libQt*.so.4.8.0`, which *are* hashed.

**What was verified (static only):**

- **Grammar**: every line of all 8 recipe files + `layer.conf` re-parsed with bitbake 1.18's *own*
  `__config_regexp__` / `BBHandler.feeder` line grammar (regexes copied verbatim from the pinned
  checkout, including the `\`-continuation and comment-in-multiline rules) — 0 unparsed lines, and
  each `file-checksums` assignment resolves to the intended var/flag/op/token list.
- **`get_file_checksums` semantics** simulated over the *expanded* lists against the real tree:
  no glob matches a directory (which would raise an **uncaught python-2 `IOError`** in
  `md5_file()` — `get_file_checksums` only catches `OSError`; this is why the lists only ever glob
  file patterns and pass whole directories as bare paths), no absent bare paths (no `bb.warn`
  noise), and the per-task cost is bounded: 4–247 files / ≤36 MB for every task except the Mochi
  `do_install` (1597 files / 9.4 MB, the shipped framework payload).
- **NOT verified**: that bitbake actually parses the recipes, that the hashes move as intended,
  and that sstate invalidates. The first build after this lands **rebuilds** the affected recipes
  once (all their task hashes changed) — that is expected, not a regression.

### #2 residual — GCC PPA `[trusted=yes]` replaced with a pinned signing key

`oe-env.sh` no longer disables apt authentication. New `install_ppa_key()` (host-side, before the
chroot's `apt-get`) installs the ubuntu-toolchain-r key into its own keyring at
`$ROOTFS/etc/apt/trusted.gpg.d/ubuntu-toolchain-r.gpg`, and the sources.list line lost
`[trusted=yes]`. Design notes:

- **`trusted.gpg.d`, not `signed-by`** — the rootfs runs apt **1.0.1ubuntu2.20** (read from the
  provisioned rootfs's `var/lib/dpkg/status`); the sources.list `[signed-by=…]` option arrived in
  apt 1.1. A dedicated `trusted.gpg.d` keyring is the tightest scoping this apt supports, and
  `gnupg` is a hard `Depends` of apt so it is always present.
- **Full-fingerprint pin** `60C317803A41BA51845E371A1E9377A2BA9EF27F`: the key is imported into a
  throwaway `GNUPGHOME` and the 40-hex fingerprint must match, then only that fingerprint is
  exported (so key material carrying extra keys cannot smuggle them in).
- **Seed-first, fetch-fallback**: `build/webos-oe/keys/ubuntu-toolchain-r.asc` is used when
  present (offline-capable), else HTTPS `keyserver.ubuntu.com`. `PPA_KEY_SEED` / `PPA_KEYSERVER`
  override. Seeding is documented in `docs/OE-BUILD.md`.
- **Fails closed** everywhere: fetch failure, invalid key material, fingerprint mismatch, empty
  export, or a missing keyring inside the chroot all abort provisioning. No `[trusted=yes]` path
  remains.
- **Legacy repair**: a rootfs provisioned by the older script still carries the `[trusted=yes]`
  line; `provision()` now detects and rewrites it in place (idempotent) rather than leaving the
  hole until someone runs `clean`.

**What was verified:** the key was fetched from `keyserver.ubuntu.com`, its fingerprint confirmed
as `60C317803A41BA51845E371A1E9377A2BA9EF27F` (uid "Launchpad Toolchain builds"), and seeded into
the repo. `bash -n` passes. `install_ppa_key()` was extracted and **run standalone against a fake
rootfs**: the seeded path and the keyserver path both produce a valid binary keyring containing
exactly the pinned key; a decoy key and garbage input both abort with exit 1, leaving no keyring
and no temp dirs. **NOT verified**: the in-chroot half — that `apt-get update` accepts the PPA's
`Release.gpg` against this keyring and that `gcc-9`/`g++-9` install authenticated. That needs a
real `oe-env.sh provision`.

## Follow-up for R3 "done" (gates to pass, per the reviewer)
`build-produced` ✓ · `payload structurally validated` (Mochi frameworks #1) · `clean-clone
reproducible` (#7/#8) · `single-variant install/run` (#5) · `dual-version coexistence` (#4) ·
`uninstall-one-survivor` (#4) · `device verified`. Mark R3 complete only after these pass.
