# build/desktop — Phase 1 x86_64 Linux PoC

Goal: prove the Goanna backend satisfies the BrowserServer contract on a normal
desktop, before fighting the embedded toolchain. End state: launch
`BrowserServer` (Goanna backend), point the isis UI (`app/`) or a thin test
harness at it, and see a real page rendered into the shared framebuffer.

## Build order

1. **Goanna engine** — build UXP (`../../../UXP`) out-of-tree with an
   embedding-oriented config:
   - basic (non-GL) layers first, no full XUL front-end, system NSPR/NSS off as
     needed, `--enable-application=` set to a minimal embedding target.
   - Python 2.7 + autoconf-2.13 build host requirements apply.
   - Output: libxul + headers + IDL-generated headers consumed by `render/goanna`.
2. **render/browserserver** — import the engine-agnostic Apache-2.0 sources from
   `../../../ref-BrowserServer` (YAP, shmem, daemon, LunaService stub) and build
   them; the YAP interface (`BrowserServerBase`) is unchanged.
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
