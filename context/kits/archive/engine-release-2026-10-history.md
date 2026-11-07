# cavekit-engine-release-2026-10.md history, moved out of the kit by the 2026-11-07 sweep; verbatim.

## Scope, upstream as read on 2026-10-26 (removed 2026-11-07)

What upstream has, read on 2026-10-26 (`git ls-remote` of https://repo.palemoon.org/MoonchildProductions/UXP and its
compare view):
- The newest **release** tag (`RB_*`, a UXP shipped with a Pale Moon release) is **`RB_20261006`** (`5f290d1`, Pale Moon
  35.0.2) — the engine's base today. Nothing newer is tagged `RB_`.
- The `release` branch is at **`RC_20261026`** (`6b18be9`, "[js] Avoid OOB array copy", 2026-10-26), 12 commits past it
  (with `RC_20261024`, `037cc5f`, between): a JS self-hosting fix, the emSign roots removed and certdata to September
  2026, SQLite 3.54.1, a lock in the component manager's memory reports, a selection fix in layout/generic, a strong ref
  in the mutation observer, an feTile overflow in gfx/2d, `jar:` no longer loadable by anyone, and three Windows-only
  file-extension commits. Goanna stays 6.9.0, NSS unchanged, ICU `icudt78l.dat` unchanged.
- None of the 12 touches a file our 69 patches touch, except the certdata: patch 0060 already carries `037cc5f`'s roots,
  so it becomes empty on the new base.

## R1, criteria 2 and 3 (evidence moved after the criterion text; the Dependencies line moved below criterion 3) (removed 2026-11-07)

- [x] The chosen tag's commit and date are pinned in `build/engine/engine-version.env` (`UXP_BASE`/`UXP_TAG`), and a tag *(2026-11-03: `RB_20261027` = `6b18be917d462369593bfe611230a7003fd7d945`, 2026-10-26, in `engine-version.env`; the branch `jihad/uxp-RB_20261027`, the queue 72 patches, `patch-queue.sh --check` ok)*
  name of the `RC_` form is accepted by `update-engine.sh` (its default lookup matches `RB_` only) and by
  `check-engine-pins.sh`
**Dependencies:** cavekit-engine-update.md R2
- [x] The certificate and HTTPS changes upstream pushed in response to recent exploits get special attention (the owner, 2026-11-02): every commit in the range touching `security/` (NSS, PSM, certdata, pkix), `netwerk/` TLS and HSTS, the security prefs and the certificate UI is listed with its disposition in `docs/ENGINE-SOURCE.md`, carried through the rebase and checked against the queue (none reverted by a patch; the roots by `audit-roots.py`), and the certificate kit's device checks (the TLS floor, a revoked, expired and mismatched host, the Trust dialog) run on the new engine before the push *(2026-11-02: `RB_20261027` found by T-551's check — 12 commits over RB_20261006, 11 files: `[NSS] Remove emSign roots (eMudhra compromise)` 037cc5f614 and `[NSS] Update certdata to Sept 2026 status` 4a68bbbd53 are the certificate ones and are already on the device (the certificate-trust kit shipped certdata from 037cc5f614 on 2026-10-25, `ROOTS_SOURCE`); no PSM/TLS protocol change in the range; the memory-safety fixes the device lacks: `[js] Avoid OOB array copy` 6b18be917d, the `jar:` protocol's `URI_LOADABLE_BY_ANYONE` flag removed d42616bb12 (Jihad's own `jar:` pages to be checked, R4), feTile overflow bae7a6ec8f, the layout frame table 8fea2ad823, the animation observer's strong ref c4339b14dc, SQLite 3.54.1)* *(2026-11-03: the range's 12 commits each with a disposition in `docs/ENGINE-SOURCE.md` ("The move to RB_20261027"); the two NSS commits taken with the base (the device had them through the roots since 2026-10-25); no PSM/TLS/HSTS/UI change in the range; the certificate device checks on the new engine are T-554's)* *(device, 2026-11-03, `docs/rounds/engine-rb27-2026-11-03/cert-checks*.txt`: TLS 1.0 refused with the alert, TLS 1.2 / sha256 / Wikipedia clean, expired, wrong.host, self-signed and untrusted-root with the SSL confirm's right flags, OCSP revocation enforced (grc.com's confirm, ssl.com's and DigiCert's alerts without Trust), revoked.badssl.com's CRL-only case loaded as on 2026-10-25; the roots the base's, identical to the shipped ones (`audit-roots.py` OK): nothing changed with the base)*

## R2, criteria 1-3 (evidence moved after the criterion text) (removed 2026-11-07)

- [x] `update-engine.sh <tag>` applies the queue on the new base in a worktree; patch 0060 (the roots, now upstream) is *(2026-11-03: applied in `../uxp-update-RB_20261027`; 0060 fell away by itself (byte-identical: `git am` "already applied"), the owner's choice; one overlap listed, `certdata.txt`)*
  dropped or kept empty as the owner decides, and every other patch applies (or its conflict is resolved and recorded)
- [x] `update-engine.sh --finish` moves the engine branch (`jihad/uxp-<tag>`), the pins (the OE recipe's SRCREV, the *(2026-11-03: `jihad/uxp-RB_20261027` at `52ad9ab66e` checked out in `third_party/uxp`; the queue rewritten (72, renumbered from 0060); `.gitmodules`, the recipe's SRCREV, the UA and the three start pages say UXP/6b18be9; `patch-queue.sh --check` ok, `check-engine-pins.sh` all agree but the gitlink until the commit)*
  user agent's short hash, the three start pages, `.gitmodules`), and `patch-queue.sh --check` reproduces the new pin
  exactly; `check-engine-pins.sh` passes
- [x] `check-engine-api.py` reports no export lost; the daemon's start line says every optional export is present *(2026-11-03: 0 FAIL, 0 WARN on the worktree and at `--finish`; the daemon's start line is read at T-554)* *(device 2026-11-03: the start line "28 of 29 optional jihad_* exports present", the one missing (`jihad_layout_set_displayport`, the v1 its v2 replaced) missing before the update too; the baseline rewritten, unchanged)*

## R3, criterion 1 (evidence moved after the criterion text) (removed 2026-11-07)

- [x] `tools/host/audit-roots.py` finds the built-in roots equal to the new base's certdata (0 findings), and the *(2026-11-03: `docs/rounds/certs-2026-11-03/audit.txt`: 169 certificates, 118 trusted, identical set and trust with the fetched release certdata, none on the removed report (106 rows), the platform list carries all 106; RESULT OK)*
  roots' pin (`ROOTS_SOURCE`, `ROOTS_SHA256`) names the new base's commit

## R4, criteria 2 and 3 (evidence moved after the criterion text) (removed 2026-11-07)

- [x] The ARM engine, daemon and three packages are built and installed by full `push-variant.sh` runs; default-check *(2026-11-03: libxul at -j4 after earlyoom killed the -j8 build beside the desktop one; the daemon, the bundle, the three .ipks; push-variant.sh enyo, mochi, mojo (md5-verified); default-check ×3 faults 0, GPU off 0, express off 0, the screenshot looked at; the daemon's start line 28 of 29 optional exports as before the update (the v1 displayport export its v2 replaced); `docs/rounds/engine-rb27-2026-11-03/notes.txt`)*
  passes in all three variants
- [x] The areas the new commits touch are checked on the device: Jihad's own pages that load through `jar:` and *(2026-11-03: tools/device/engine-checks.sh — about:settings, about:personas and about:addons through jar:/resource: under the jar: flag change, rendered (ec-*.jpg looked at), 0 JS errors; a cookie set and 88 counted through SQLite 3.54.1, no sqlite error; render/goanna/test/jihad-svg-filter.html's feTile boxes and the 1x1-tile-over-4000x4000 case painted; a long press selects a word and raises the context menu (the frame-table fix); faults 0)*
  `resource:` (about:settings, about:addons, about:personas, the start page) open; a profile with cookies and history
  survives the SQLite update (cookies kept, history listed); selection in a page (a long press) and an SVG filter page
  render
