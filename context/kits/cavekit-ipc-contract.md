---
created: "2026-06-30"
last_edited: "2026-07-07"
---

# Cavekit: IPC Contract Preservation

## Scope
The engine-agnostic render daemon and its contract with the BrowserAdapter: the
YAP command/message interface, the shared-memory framebuffer protocol, the
daemon lifecycle/page manager, and the LunaService surface. The **command/message
interface is frozen**: the port hinges on the YAP commands and messages staying
byte-identical so the UI needs no change. The self-contained build does rename the
adapter's NPAPI MIME and YAP *server socket name* for coexistence with the stock
browser (see R5, `jihad-self-contained-arch.md`) — a transport-addressing rebrand
that leaves every command/message signature untouched. Reference:
`docs/IPC-CONTRACT.md`, `context/refs/refs-overview.md`.

## Requirements

### R1: YAP command/message interface is byte-identical to upstream
**Description:** The set and signatures of YAP commands (async + sync) and messages match the isis-project BrowserServer exactly.
**Acceptance Criteria:**
- [x] The generated YAP interface (commands and messages, including argument types and order) is unchanged from upstream `BrowserServerBase.h`.
- [x] No command or message is added, removed, renamed, or has its signature altered.
- [x] If regenerated, it is regenerated from the upstream `.yap` definition, not hand-edited.
**Dependencies:** none

### R2: Shared-memory framebuffer semantics preserved
**Description:** Frames are delivered through the same double-buffered shared-memory protocol.
**Acceptance Criteria:**
- [x] `connect` and `thaw` accept two shared-buffer keys and a size and attach both segments.
- [x] The renderer fills the inactive buffer and emits exactly one paint-ready notification naming that buffer.
- [x] The renderer does not reuse a buffer until it has been returned by the client.
- [x] Pixel format, stride, and size match the upstream offscreen-buffer contract (32-bit, page dimensions).
**Dependencies:** cavekit-offscreen-rendering.md (R2, R3)

### R3: Daemon lifecycle and page manager preserved
**Description:** The daemon process behaves like the upstream render daemon.
**Acceptance Criteria:**
- [x] Daemon starts, accepts a `connect`, and creates one page per `identifier`.
- [x] `freeze`/`thaw` detach/reattach buffers; `purgePage` and low-memory purge behave as upstream.
- [x] The daemon exits after the last client disconnects (when built with that option).
- [x] Multiple pages (cards) are managed independently.
**Dependencies:** none (engine-agnostic daemon scaffolding kept from upstream; the engine instance it manages is specified in cavekit-engine-embedding.md R2)

### R4: LunaService surface preserved
**Description:** The service methods the UI calls directly remain available on the device build.
**Acceptance Criteria:**
- [ ] `palm://com.palm.browserServer/clearCache` and `.../clearCookies` are registered and perform their actions on the device build. *(**2026-08-04 — measured, and the result is worse than "not done": these calls SUCCEED on device and act on the WRONG BROWSER.** Our daemon registers no Luna service at all (no `LSRegister` anywhere in `render/`), and `com.palm.browserServer` is the STOCK browser's service, which we deliberately coexist with. So `luna-send palm://com.palm.browserServer/clearCookies` returns `{"returnValue":true}` — from the stock daemon — while Jihad's own store is untouched: a cookie count taken straight afterwards still shows all three of Jihad's cookies. The Enyo card's Preferences "clear cookies/cache" therefore clears the STOCK browser's data and silently does nothing for this one. That became user-visible the moment cookie persistence started working (browser-services R2, closed the same day).*
  *The clearing itself is NOT missing: `asyncCmdClearCache`/`asyncCmdClearCookies` exist over YAP and are implemented in the daemon (`jihad::ClearCache/ClearCookies`). What is missing is a route from the card to them — the isis adapter exposes no scriptable `clearCache`/`clearCookies`, which is why the app uses the Luna URI. Two ways out, and it is a CONTRACT decision rather than a bug fix: register a per-variant service (`palm://net.riverstonerelay.jihad-browser/...`) and point each app at its own — a documented divergence like the MIME/YAP rebrand — or expose the two as scriptable adapter methods, which widens the frozen app-facing call set.*

  ***DECISION, 2026-08-04: register a PER-VARIANT service. This is no longer an open choice.*** *It
  is not a coin flip once the project's own precedent is applied: the second option widens the
  `callBrowserAdapter` set, which cavekit-ui-shell.md R2 holds byte-identical to upstream and marks
  MET — so taking it would knowingly break a met criterion in another kit. The first option is the
  same move already made, deliberately, four times over for coexistence: our own NPAPI MIME, our own
  YAP service name, our own upstart job, our own state directory. A Luna service name is the fifth
  instance of one pattern, not a new kind of divergence, and it leaves the app-facing contract
  untouched. **What implementing it takes:** the daemon registers `palm://net.riverstonerelay.jihad-browser{,-mochi,-mojo}/`
  with `clearCache` + `clearCookies` routed to the existing `jihad::ClearCache/ClearCookies`, name
  derived from `JIHAD_BS_NAME` like every other per-variant identity; each app's Preferences points
  at its own URI. Note there is no `lunaservice.h` in the sysroot — the device has
  `/usr/lib/liblunaservice.so` but no headers here — so the practical route is `dlopen` + the handful
  of `LS*` entry points, which also keeps the daemon loadable if the library is ever absent.
  Add `plan-variant-identity.md`'s table as the single source for the new name.)*
- [x] On the desktop build the service layer can be compiled out without affecting the YAP path.
**Dependencies:** cavekit-browser-services.md (R2)

### R5: BrowserAdapter drives the daemon; no change for the engine swap
**Description:** The NPAPI BrowserAdapter, rebuilt, drives the Goanna daemon through a full load+paint cycle. The engine swap requires no adapter change; the only adapter edit is a self-contained **coexistence rebrand** that leaves the YAP command/message interface untouched.
**Acceptance Criteria:**
- [x] The rebuilt BrowserAdapter connects to the daemon and completes a load+paint cycle (on-device: `http://example.com` → `loaderr failed=0`, `painted bytes=723456`).
- [x] No adapter source change is required to accommodate the **Goanna backend** — the YAP command/message interface (R1) is byte-identical.
- [x] The self-contained build adds only a **two-line rebrand** — MIME `application/x-jihad-browser` (`AdapterGetMIMEDescription`) and YAP server name `BrowserClientBase("jihad-browser", …)` — so `BrowserAdapterJihad.so` coexists with the stock adapter without collision. This does not add/remove/rename/re-type any YAP command or message.
**Dependencies:** cavekit-offscreen-rendering.md, cavekit-input-bridging.md, jihad-self-contained-arch.md

## Out of Scope
- How frames are produced or how engine events originate (rendering/engine domains).
- The transport library internals (YAP/libYap is reused as-is).

## Cross-References
- See also: cavekit-offscreen-rendering.md, cavekit-engine-embedding.md, cavekit-browser-services.md, cavekit-ui-shell.md

## Changelog
- 2026-06-30: Initial draft.
- 2026-07-04: Reconciled — R1 byte-identical YAP, R2 shmem double-buffer, R3 daemon lifecycle/freeze-thaw all verified (ROUND-TRIP + FREEZE-THAW PASS). R4 device LunaService registration and R5 real NPAPI BrowserAdapter rebuild remain device-integration work (desktop compiles the service layer out).
