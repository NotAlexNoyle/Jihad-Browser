---
created: "2026-08-01"
last_edited: "2026-08-01"
---

# ipkg deletes a DOT-child package's metadata when you remove its parent

**Severity: P1 against cavekit-device-build.md R7** ("removing any one variant leaves every other
installed variant fully functional"). Found 2026-08-01 while diagnosing why the Mojo variant's
`prerm` never ran during the acceptance matrix.

## The defect

webOS's `/usr/bin/ipkg` stores per-package metadata as `info/<pkgid>.{control,list,prerm,postinst}`.
Removing package `X` cleans up with a glob on `X.*` — which **also matches `X.child.control`**.
So removing a package deletes the metadata of every package whose id begins with `<parent>.`.

Our ids were exactly that shape:

```
net.riverstonerelay.jihad-browser          (Enyo)
net.riverstonerelay.jihad-browser.mochi    <- dot-child of Enyo
net.riverstonerelay.jihad-browser.mojo     <- dot-child of Enyo
```

**Uninstalling Enyo silently destroys Mochi's and Mojo's `.control`/`.list`/`.prerm`.** Those
packages are then un-uninstallable: no `prerm` to run, no file list to remove, so their rootfs
footprint (adapter shim, adapter impl, upstart job) becomes permanent residue — precisely the
outcome R7 and R8 exist to prevent.

This is the app-id **prefix-confusion** hazard the 2026-08-01 adversarial review probed for in *our*
scripts and correctly found safe. It lives in the third-party tool instead.

## Proof — isolated, not inferred

Built two pairs of minimal `.ipk`s (a marker file plus a `prerm` that echoes) and installed them
with the same `ipkg -o /media/cryptofs/apps install` the real packages use:

| pair | after removing the PARENT |
|---|---|
| `net.riverstonerelay.testpfx` + `net.riverstonerelay.testpfx.child` (**dot**) | child's `.control`, `.list`, `.prerm` — **all gone**; child's app dir still installed |
| `net.riverstonerelay.hyp` + `net.riverstonerelay.hyp-child` (**hyphen**) | child's `.control`, `.list`, `.prerm` — **all intact** |

So the separator decides it: `.` makes a child, `-` does not.

The same run produced ipkg's own confirmation of the deferral behaviour recorded as review F-3:

```
Removing package net.riverstonerelay.testpfx from root...
(offline root mode: not running net.riverstonerelay.testpfx.prerm)
```

## The fix

Rename the two suffixed variants to use a **hyphen**, keeping Enyo's id untouched so the one
deployment known to work is unaffected:

| variant | before | after |
|---|---|---|
| Enyo | `net.riverstonerelay.jihad-browser` | unchanged |
| Mochi | `net.riverstonerelay.jihad-browser.mochi` | `net.riverstonerelay.jihad-browser-mochi` |
| Mojo | `net.riverstonerelay.jihad-browser.mojo` | `net.riverstonerelay.jihad-browser-mojo` |

Proven safe by the hyphen pair above. The change is mechanical but wide: the app id appears in the
identity table and all its consumers (`plan-variant-identity.md`, `gen-variant-scripts.sh`,
`jihad-variants.inc`, `build-variant-ipk.sh`, `device-independence-test.sh`), in each
`appinfo.json`, in the Mochi db8 kind namespace and owner fields, in the OE recipe filenames, and
in the docs.

## APPLIED 2026-08-01 — in the working tree, NOT yet rebuilt or re-deployed

Every consumer of the app id was renamed. Beyond the list above, the sweep also caught five
places that list did not name:

| also renamed | why it mattered |
|---|---|
| `render/goanna/JihadRuntimePaths.h` | `RuntimeAppIdForVariant()` — the daemon derives `$APP/cache` (Gecko's `ProfLD`) from it. A stale id would make the engine write its cache to a path that does not exist and that no `prerm` removes. |
| `build/webos-oe/build-all-device.sh` | the artifact-manifest globs (`…jihad-browser{,.mochi,.mojo}_*.ipk`) would have reported the Mochi/Mojo `.ipk`s MISSING and exited 1. |
| `build/webos-oe/build-mochi-ipk.sh` | its own `APP_ID` — the legacy single-variant Mochi packager. |
| `build/webos-oe/device-citizen-audit.sh` | the per-app `profile/`+`cache/` snapshot rows — they would have watched directories that no longer exist, so R8 residue in the real ones could not show up in a diff. |
| `build/webos-oe/device-purge-legacy.sh` | the step-0 "refuse to run against a configured install" guard. |
| `packaging/event.d/jihad-{mochi,mojo}`, `packaging/{mochi,mojo}/{postinst,prerm}` | generated — regenerated from the table. |

Verified statically: `gen-variant-scripts.sh --check` passes; the recipes re-parse under bitbake
1.18.0's own line grammar (12 files, 0 unparsed lines); the OE `pkg_postinst`/`pkg_prerm` bodies
expanded per variant are byte-identical to `packaging/<V>/{postinst,prerm}` and `sh -n` clean;
every `appinfo.json` and db8 kind/permission file parses; the 3-kinds/3-permissions + self-owner
assertion still holds with no cross-variant grant.

**Not done here, on purpose:** no `.ipk` was rebuilt and the device was not touched. The
acceptance matrix (R7 install/remove/coexist and R8 residue) must be re-run under the new ids —
the 2026-07-19 coexistence evidence in `../kits/cavekit-mochi-ui.md` R1 predates the rename.

One consequence to expect on a device that already carries a **pre-rename** install: the old
dotted packages are a different package id, so a new install will not upgrade them. Remove the old
ones first (or, if their metadata was already destroyed by this very bug, clear their residue by
hand: their shim, `/usr/lib/jihad/<V>/`, `/etc/event.d/jihad-<V>`, `/var/palm/jihad/<V>/` and the
old app directory).

## Why the matrix did not catch it as a failure

It did — the symptom was there and was initially misread. `=== remove mojo ===` printed no `prerm`
output at all, and Mojo's shim and upstart job survived. The first reading was "the harness's prerm
invocation is wrong"; the second was "ipkg deletes control scripts before running them". Only the
isolated two-package experiment separated *ipkg defers prerm* (true, F-3) from *ipkg ate the
metadata because of the dot* (also true, and the actual cause here).

Lesson worth keeping: when a removal silently does nothing, check whether the metadata still exists
**before** blaming the invocation.

## Two further device findings from the cleanup (2026-08-01)

**`killall jihad-browserserver` could never have worked.** The daemon is exec'd through the bundled
loader — `./ld-2.23.so --library-path <hl> ./jihad-browserserver <hl>` — so its `comm` (and what
`pidof` matches) is **`ld-2.23.so`**, not `jihad-browserserver`. `pidof jihad-browserserver` returns
nothing and `killall jihad-browserserver` is a no-op. The pre-2026-07-31 packaging used exactly that
call, so its "stop the daemon" fallback was doubly broken: cross-variant in intent *and* ineffective
in practice. The current `prerm` matches the variant's own bundle directory as a whole argument in
`/proc/*/cmdline`, which is correct for this exec shape — this is a second, independent reason that
rewrite was necessary.

**Uninstalling with the daemon still running leaves `.fuse_hidden*` residue.** cryptofs is FUSE, so
deleting a file another process still holds open renames it to `.fuse_hidden<hex>` until the last
handle closes. Removing a variant's app directory while its daemon was live left 30 such entries and
made `rm -rf` fail with "Directory not empty" — residue that an R8 audit would legitimately flag.
The shipped `prerm` already does the right thing (stop the daemon, *then* let the package manager
remove files), and this is the concrete reason that ordering matters rather than being tidy.
