# build/desktop — Phase 1 x86_64 Linux PoC

Goal: prove the Goanna backend satisfies the BrowserServer contract on a normal
desktop, before fighting the embedded toolchain. End state: launch
`BrowserServer` (Goanna backend), point the isis UI (`app/`) or a thin test
harness at it, and see a real page rendered into the shared framebuffer.

## Files here

- `Dockerfile` — pinned Goanna build container (Ubuntu 18.04: Python 2.7,
  autoconf2.13, GCC 7, yasm). See `../../docs/TOOLCHAIN.md`.
- `mozconfig.goanna` — embedding-oriented engine config (xulrunner target,
  GTK2 + basic layers, trimmed subsystems).
- `build-goanna.sh` — runs `mach configure`/`build` inside the container against
  a read-only UXP source mount, artifacts to `/out`.

## Build order

1. **Goanna engine** — build UXP (`third_party/uxp` (submodule)) out-of-tree inside the pinned
   container (a modern host cannot run the ESR-52 `mach`):
   ```
   podman build -t jihad-goanna-build build/desktop
   mkdir -p build/desktop/out
   podman run --rm --user 0 -e HOME=/out \
     -v <abs>/Jihad/UXP:/src/uxp \
     -v <abs>/Jihad/Jihad-Browser/build/desktop:/cfg:ro \
     -v <abs>/Jihad/Jihad-Browser/build/desktop/out:/out \
     jihad-goanna-build /cfg/build-goanna.sh all
   ```
   Notes: the UXP mount is **read-write** — ESR-52 `mach` regenerates
   `configure` (autoconf2.13) into the source tree (git-ignored; objdir stays
   external at `/out`, so the Jihad repo never vendors UXP). Rootless podman as
   `--user 0` maps to your host uid, so `/out` artifacts come back owned by you.
   Output: libxul + generated headers consumed by `render/goanna`. (T-010)
2. **render/browserserver** — engine-agnostic Apache-2.0 sources already imported
   under `render/browserserver/Src/` (see its `MANIFEST.md`); the YAP interface
   (`BrowserServerBase`) is frozen/unchanged. De-Qt the bucket-2 files and build.
3. **render/goanna** — build the backend (`BrowserPageGoanna`, `OffscreenWidget`,
   `GoannaFrameSink`, listeners, input bridge) against the Goanna headers.
4. **Link** the daemon with the Goanna backend providing the `BrowserPage`
   vtable.
5. **Run harness** — a minimal YAP client (or BrowserAdapter built for desktop)
   that connects, sends `openUrl`, and displays `msgPainted` buffers in a window.

## Notes

- On desktop there is no LunaService bus; stub `palm://com.palm.browserServer/*`
  or compile with `USE_LUNA_SERVICE=0`.
- pbnjson / glib / openssl are available as distro packages.
- This directory will hold the CMake (or qmake) wiring; the engine itself is
  configured via a `.mozconfig` committed under `build/` and built into an
  out-of-tree `obj-*` dir (git-ignored).
