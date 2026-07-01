| Severity(P0-P3) | File | Line | Description |
|---|---|---:|---|
| P0 | [YapPacket.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/browserserver/Yap/YapPacket.cpp:344) | 344-367 | String deserialization stores length in signed `int16_t`; a packet length like `0xffff` becomes negative, passes the bounds check, then reaches `malloc(strLen + 1)` / `memcpy(..., strLen)` as a huge `size_t`. Local malformed YAP client can crash or corrupt the daemon. |
| P0 | [GoannaRenderPage.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/goanna/GoannaRenderPage.cpp:148) | 148 | `PumpFor()` recursively runs GTK/GLib work while `BrowserPageGoanna::pump()` is on-stack. A disconnect or command dispatched there can hit `clientDisconnected()` and `delete mPage` at `JihadBrowserServer.cpp:21`; control then returns to freed `BrowserPageGoanna` and calls `emitLoadAndLocation()` at `BrowserPageGoanna.cpp:85`. |
| P0 | [JihadBrowserServer.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/browserserver/JihadBrowserServer.cpp:15) | 15-21 | Single global `mProxy`/`mPage` is not client-safe. A second client steals `mProxy`; a stale client disconnect still unconditionally deletes the current page. This can cross-send page messages and destroy another client’s live page. |
| P1 | [BrowserPageGoanna.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/goanna/BrowserPageGoanna.cpp:99) | 99-102 | Double-buffer protocol is broken: after `msgPainted(mActiveKey)`, the next tick writes the other key without waiting for `ReturnBuffer`. `asyncCmdReturnBuffer()` is a stub at `JihadBrowserServer.cpp:358`, so the daemon can paint into buffers the adapter is still displaying. |
| P1 | [BrowserPageGoanna.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/goanna/BrowserPageGoanna.cpp:24) | 24-32 | Destructor removes SysV shm segments with `IPC_RMID` even though comments say the adapter owns them. In the real daemon this can invalidate adapter-owned buffers after disconnect/reconnect. |
| P1 | [YapProxy.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/browserserver/Yap/YapProxy.cpp:259) | 259-263 | `msg*` sends are blocking socket writes on the GLib thread. Since `tick()` emits `msgPainted` at 60 Hz, a slow or wedged adapter can stall Goanna pumping, command handling, and disconnect processing. |
| P1 | [GoannaRenderPage.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/goanna/GoannaRenderPage.cpp:94) | 94-102 | Teardown does not stop loads, remove web-progress listener, clear container window, or drain pending XPCOM events before releasing `PageChrome`/browser. Pending progress or paint work can observe half-destroyed state. |
| P1 | [GoannaRenderPage.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/goanna/GoannaRenderPage.cpp:112) | 112-129 | `PageChrome` is transferred to raw ownership only at line 129. Failures after `SetContainerWindow()` but before `forget().take()` can leave browser/chrome refs outside `GoannaRenderPage`, violating the “all browsers gone before `XRE_TermEmbedding`” contract. |
| P1 | [JihadBrowserServer.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/browserserver/JihadBrowserServer.cpp:36) | 36-48 | `identifier` is ignored and there is only one `BrowserPageGoanna`. The generated protocol’s per-page connect surface cannot work for multiple cards/pages; all messages are routed through the last proxy. |
| P2 | [BrowserPageGoanna.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/goanna/BrowserPageGoanna.cpp:49) | 49-50 | Server creates shm segments with `IPC_CREAT` instead of strictly attaching to adapter-provided segments. This masks protocol/setup errors and can create daemon-owned buffers with keys the adapter did not actually allocate. |
| P2 | [BrowserPageGoanna.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/goanna/BrowserPageGoanna.cpp:39) | 39 | No dimension/size validation. `width * height * 4` can overflow, `sharedBufferSize` can be too small or inconsistent, and invalid values flow into `shmget()`/`ReadPixels()`. |
| P2 | [JihadBrowserServer.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/browserserver/JihadBrowserServer.cpp:31) | 31-32 | `tick()` paints every 16 ms regardless of load state, invalidation, nonblank pixels, returned buffers, or adapter backpressure. This produces blank/stale frames and burns the same loop needed for IPC and Goanna events. |
| P2 | [BrowserPageGoanna.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/goanna/BrowserPageGoanna.cpp:55) | 55-57 | `setWindowSize()` is a no-op. After adapter resize, browser dimensions, GDK drawable size, and shm buffer size can diverge, causing failed paints or wrong-size frames. |
| P2 | [BrowserPageGoanna.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/goanna/BrowserPageGoanna.cpp:62) | 62-63 | `openUrl()` emits `LoadStarted` before checking `LoadUrl()` result and never emits `FailedLoad`/`LoadStopped` on failure. Bad URLs or null/malformed YAP strings can leave the adapter in a permanent loading state. |
| P2 | [GoannaRenderPage.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/goanna/GoannaRenderPage.cpp:80) | 80-81 | Load completion is any `STATE_STOP` with `STATE_IS_WINDOW` or `STATE_IS_NETWORK`; it does not verify top-level document/window. Subresource or intermediate stops can mark the page done too early. |
| P3 | [BrowserPageGoanna.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/goanna/BrowserPageGoanna.cpp:78) | 78 | Passes `mPage->CurrentUri().c_str()` from a temporary string into the sink. Current sink copies synchronously, but the interface makes this a dangling pointer hazard for any future async sink. |
| P3 | [adapter_client.cpp](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/browserserver/test/adapter_client.cpp:53) | 53-56 | Smoke client counts `msgPainted` only; it never attaches shm, checks pixels, verifies nonblank content, or sends `ReturnBuffer`. The “round-trip pass” can pass while the real double-buffer protocol is broken. |

**Top Risks**

The nominal YAP IDs for `Connect`, `OpenUrl`, and the load/paint messages match `BrowserServerBase`; the failures are in state ownership, deserialization hardening, and buffer protocol.

Highest risk is reentrancy: `tick()` enters nested GTK/GLib processing while page methods are active, and disconnect can delete the page underneath that stack.

Second is protocol ownership: one `mProxy`, one `mPage`, ignored `identifier`, ignored `ReturnBuffer`, and daemon-side `IPC_RMID` are incompatible with the real multi-page/double-buffer adapter contract.

Third is lifecycle: page teardown needs an explicit ordered shutdown path: stop navigation, remove listeners, clear container window, destroy browser/base window, destroy GTK widget, drain relevant events, then assert zero live browser refs before `XRE_TermEmbedding`.

Static review only; I did not run the daemon.

---

## Disposition (applied 2026-07-01; round-trip re-verified, exit 0)

**Fixed:**
- P0 YapPacket signed-length overflow → unsigned `uint16_t` length + bounded read (Yap/YapPacket.cpp).
- P0 reentrancy (page deleted under the tick stack) → tick() snapshots page pointers; reap() defers deletion to after the tick loop (mReap); no page freed mid-pump.
- P0 single global mProxy/mPage → per-proxy `std::map<YapProxy*, Page>` with a per-page `ProxySink` routing that page's msg* to its client; identifier-scoped connect replaces the per-proxy page.
- P1 shm ownership → the ADAPTER allocates the buffers; the daemon `init()` **attaches only** (no IPC_CREAT) and never IPC_RMIDs them; adapter RMIDs on exit.
- P1 GoannaRenderPage teardown → ordered: Stop(STOP_ALL), RemoveWebBrowserListener, SetContainerWindow(null), Destroy, release chrome, destroy GTK window.
- P2 dimension/size validation in init() (bounds + overflow + buffer-large-enough).
- P2 openUrl failure → emits LoadStopped instead of leaving the adapter stuck loading.
- P2 tick paints every frame → `maybePaint()` paints only when there is a new frame (once per load), not at 60 Hz.
- P3 adapter now attaches the shared buffer and verifies non-white pixels (751756) — proves the framebuffer content, not just msgPainted.

**Deferred to full daemon hardening (documented; not demo blockers):**
- P1 blocking msg* socket writes on the GLib thread (needs non-blocking/queued sends) — real backpressure handling is T-016.
- P1 returnBuffer/full double-buffer rotation (asyncCmdReturnBuffer still a stub) — maybePaint() paints once per load so we don't race, but strict double-buffering is T-016.
- P2 setWindowSize no-op (resize widget + rekey buffers) — T-016.
- P2 top-level-only load completion (verify top window/document) — refine in navigation-events.
- P3 CurrentUri().c_str() temporary — safe for the synchronous sinks used today; note for any future async sink.
