---
created: "2026-06-30"
last_edited: "2026-11-07"
---

# Cavekit: IPC Contract Preservation

## Scope
The engine-agnostic render daemon and its contract with the BrowserAdapter: the
YAP command/message interface, the shared-memory framebuffer protocol, the
daemon lifecycle/page manager, and the LunaService surface. The **command/message
interface is frozen** so the UI needs no change; the coexistence rebrand (R5) renames
only the adapter's NPAPI MIME and the YAP server socket name (auto-memory
`jihad-self-contained-arch.md`). Reference: `docs/IPC-CONTRACT.md`,
`context/refs/refs-overview.md`.

## Requirements

### R1: YAP command/message interface is byte-identical to upstream
**Description:** The set and signatures of YAP commands (async + sync) and messages match the isis-project BrowserServer exactly.
**Acceptance Criteria:**
- [x] The generated YAP interface (commands and messages, including argument types and order) is unchanged from upstream `BrowserServerBase.h`.
- [x] **No existing command or message is removed, renamed, or has its signature altered, and nothing is added except a NEW id that is optional in both directions and documented.** *(Wording amended 2026-08-10: it read "No command or message is added, removed, renamed, or has its signature altered". The owner authorised one addition that day to fix the Flash frame rate; any further addition needs the same three properties and its own authorisation. The addition: `asyncCmdSetExtraBuffer` (0x1600, two int32s) announces a third shared-memory buffer; `connect`/`thaw` are unchanged. Optional both ways: an adapter that never sends it leaves the daemon on two buffers (`mKey3` stays 0); a daemon without it hits `BrowserServerBase`'s `default:` arm (`Unknown async cmd`), because the server declaration is a non-pure virtual with an empty body. Measured: `deferred` 77 → 0, `wanted` == `done`, composite 27.1–32.2 fps, frame-gap max 127 ms → 52–57 ms. Wire detail: docs/IPC-CONTRACT.md.)*
- [x] If regenerated, it is regenerated from the in-tree `.defs` definition with the in-tree generator, not hand-edited. *(Wording amended 2026-08-15: it named "the upstream `.yap` definition", which must never be used — the upstream `.defs` is stale against its own generated output (KeyDown/KeyUp 0x1008/0x1009 lack the third `int32 chr` upstream hand-added), so regenerating from it breaks every keystroke.)* *(Closed 2026-08-15. Inputs: the corrected `render/browserserver/CodeGen/BrowserYapCommandMessages.defs` (0x1600 marked `async optional`) and the vendored, patched generator `render/browserserver/CodeGen/YapCodeGen.cpp` (README beside it): `ignoreLine()` advances its pointer, so a `.defs` line starting with a space no longer hangs it (upstream still hangs); the type field takes flags; an `optional` async command is emitted as a non-pure virtual with an empty body, so 0x1600's default body is generated. Host results (Qt 5.15.11 / g++ 14.2.1): generated `BrowserServerBase.cpp` differs from the shipped file by four cosmetic diffs only; the generated `.h` reproduces the 0x1600 declaration and body byte for byte; both generators give byte-identical output from the upstream `.defs`. `build/webos-oe/check-yap-contract.sh` (run by `build-daemon-arm.sh`; frozen manifest of 73 commands and 58 messages, the 0x1600 arm and the `chr` argument asserted) reports zero defects on the generated set and passes on the shipped tree. Not byte-reproducible and harmless: the copyright year, one comment, and 0x1600's declaration position in the server `.h` (the shipped `.h` and `.cpp` disagree on it). The generated files were not copied over `render/browserserver/Src/`. `JihadBrowserServer.h` marks `asyncCmdSetExtraBuffer` and `asyncCmdKeyDown` `override`, so losing a declaration fails to compile; the `case 0x1600:` arm is what the check script guards. Full record: `context/impl/impl-yap-codegen.md`.)*
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
- [x] `clearCache` and `clearCookies` are registered as a LUNA SERVICE and reach THIS daemon on the device build. *(2026-08-05, device: registered under a per-variant name, not `com.palm.browserServer`, which is the stock daemon. Decided 2026-08-04 over exposing scriptable adapter methods, which would widen the `callBrowserAdapter` set cavekit-ui-shell.md R2 holds frozen; the per-variant name follows the MIME, YAP name, upstart job and state directory, and the app-facing contract is untouched. Evidence: the daemon logs `luna: serving palm://net.riverstonerelay.jihadBrowser/{clearCache,clearCookies}`; `luna-send` returns `{"returnValue":true}` and OUR daemon logs `luna: clearCookies` / `luna: clearCache`; on a store seeded with three cookies through `nsICookieService::SetCookieString`, `cookie count=3` → 0. The handlers call `jihad::ClearCookies`/`ClearCache`, as the YAP commands do. Traps: an LS2 service name may not contain a hyphen (`_UriParse: Not a valid service name`); the hub's role files are keyed on `exeName`, which is the bundled loader `ld-2.23.so`; `LSMethod` on the device (luna-service2) has three fields, and a two-field struct registers nothing (`Unknown method "clearCookies" for category "/"`). The inject channel needs a card launched at a URL (`params.target`), or every command logs `inject: no page`. First-start registration on CE 3.1.0: cavekit-device-build.md R4.)*
- [x] On the desktop build the service layer can be compiled out without affecting the YAP path.
**Dependencies:** cavekit-browser-services.md (R2)

### R5: BrowserAdapter drives the daemon; no change for the engine swap
**Description:** The NPAPI BrowserAdapter, rebuilt, drives the Goanna daemon through a full load+paint cycle. The engine swap requires no adapter change; the only adapter edit is a self-contained **coexistence rebrand** that leaves the YAP command/message interface untouched.
**Acceptance Criteria:**
- [x] The rebuilt BrowserAdapter connects to the daemon and completes a load+paint cycle (on-device: `http://example.com` → `loaderr failed=0`, `painted bytes=723456`).
- [x] No adapter source change is required to accommodate the **Goanna backend** — the YAP command/message interface (R1) is byte-identical.
- [x] The self-contained build adds only a **two-line rebrand** — MIME `application/x-jihad-browser` (`AdapterGetMIMEDescription`) and YAP server name `BrowserClientBase("jihad-browser", …)` — so `BrowserAdapterJihad.so` coexists with the stock adapter without collision. This does not add/remove/rename/re-type any YAP command or message.
**Dependencies:** cavekit-offscreen-rendering.md, cavekit-input-bridging.md, auto-memory `jihad-self-contained-arch.md`

## Out of Scope
- How frames are produced or how engine events originate (rendering/engine domains).
- The transport library internals (YAP/libYap is reused as-is).

## Cross-References
- See also: cavekit-offscreen-rendering.md, cavekit-engine-embedding.md, cavekit-browser-services.md, cavekit-ui-shell.md

## Changelog
- 2026-11-07: Sweep: evidence notes compacted to date, task, proof and the deciding numbers; superseded notes and session narrative moved verbatim to `archive/ipc-contract-history.md`. No box changed state.
- 2026-10-01: **Target platform is now webOS CE 3.1.0.** No box changed state. R4's Luna-service
  box is 3.0.5 evidence, and a fresh install on CE logged the service FAILING to register on first
  start (-1027) — tracked as its own criterion in cavekit-device-build.md R4, and R4/R5 here are
  listed in that requirement's re-verification sweep. R1's first criterion says the interface is
  "unchanged from upstream"; read it with R1's second criterion, which records the one authorised
  addition (0x1600).
- 2026-06-30: Initial draft.
- 2026-08-15: R1's last criterion closed. The YAP generator is vendored into the tree and patched, so regeneration from the in-tree `.defs` reproduces the shipped interface — including the 0x1600 empty default body — with no hand edit. `context/impl/impl-yap-codegen.md`.
- 2026-07-04: Reconciled — R1 byte-identical YAP, R2 shmem double-buffer, R3 daemon lifecycle/freeze-thaw all verified (ROUND-TRIP + FREEZE-THAW PASS). R4 device LunaService registration and R5 real NPAPI BrowserAdapter rebuild remain device-integration work (desktop compiles the service layer out).
