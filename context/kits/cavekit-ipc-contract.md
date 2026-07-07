---
created: "2026-06-30"
last_edited: "2026-07-04"
---

# Cavekit: IPC Contract Preservation

## Scope
The engine-agnostic render daemon and its contract with the BrowserAdapter: the
YAP command/message interface, the shared-memory framebuffer protocol, the
daemon lifecycle/page manager, and the LunaService surface. This contract is
**frozen**: the entire port hinges on it staying identical so neither the UI nor
the BrowserAdapter must change. Reference: `docs/IPC-CONTRACT.md`,
`context/refs/refs-overview.md`.

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
- [ ] `palm://com.palm.browserServer/clearCache` and `.../clearCookies` are registered and perform their actions on the device build.
- [x] On the desktop build the service layer can be compiled out without affecting the YAP path.
**Dependencies:** cavekit-browser-services.md (R2)

### R5: BrowserAdapter requires no source change
**Description:** The existing NPAPI BrowserAdapter, rebuilt unmodified, drives the new daemon.
**Acceptance Criteria:**
- [ ] The upstream BrowserAdapter source (rebuilt) connects to the daemon and completes a load+paint cycle.
- [ ] No adapter source change is required to accommodate the Goanna backend.
**Dependencies:** cavekit-offscreen-rendering.md, cavekit-input-bridging.md

## Out of Scope
- How frames are produced or how engine events originate (rendering/engine domains).
- The transport library internals (YAP/libYap is reused as-is).

## Cross-References
- See also: cavekit-offscreen-rendering.md, cavekit-engine-embedding.md, cavekit-browser-services.md, cavekit-ui-shell.md

## Changelog
- 2026-06-30: Initial draft.
- 2026-07-04: Reconciled — R1 byte-identical YAP, R2 shmem double-buffer, R3 daemon lifecycle/freeze-thaw all verified (ROUND-TRIP + FREEZE-THAW PASS). R4 device LunaService registration and R5 real NPAPI BrowserAdapter rebuild remain device-integration work (desktop compiles the service layer out).
