| Severity | File | Line | Description |
|---|---:|---:|---|
| P1 | `render/goanna/test/embed_load.cpp` | 318 | `host.Shutdown()` runs while XPCOM objects still hold refs: `weak`, `thread`, `cur`, and `chrome` survive until function exit. `XRE_TermEmbedding()` must happen only after all Gecko objects/listeners/windows are released. |
| P1 | `render/goanna/EngineHost.cpp` | 68 | `EngineHost::Shutdown()` has no active-browser tracking. Any caller can terminate the process-wide embedding runtime while `nsIWebBrowser` instances still exist. This is unsafe for a persistent multi-page daemon. |
| P1 | `render/goanna/test/embed_load.cpp` | 167 | Uses one fixed SysV key, `0x4a494841`, with `IPC_CREAT` and no `IPC_EXCL`. Multiple pages/processes collide, stale segments are reused, and resizing can fail or attach the wrong segment. |
| P1 | `render/goanna/test/embed_load.cpp` | 168 | The shm segment is never marked `IPC_RMID`. This leaks kernel shm objects after crashes/exits and diverges from the existing `IpcBuffer::create()` ownership pattern. |
| P1 | `render/goanna/test/embed_load.cpp` | 185 | The comment says the backend would emit `msgPainted(key)`, but the BrowserServer side ultimately `shmat()`s the integer it receives. That must be the shm id returned by `shmget`, not the `key_t` used to create it. |
| P1 | `render/goanna/test/embed_load.cpp` | 274 | The event loop is a polling loop over `NS_ProcessNextEvent(..., false)`, GTK pending events, and `g_usleep`. A daemon needs one integrated GLib/XPCOM main-loop source; this loop will add latency, block all pages during waits, and miss proper idle/timer behavior. |
| P1 | `build/desktop/patches/0004-xre-initembedding-gfx-init.patch` | 22 | Forcing `gfxPlatform::GetPlatform()` inside `XRE_InitEmbedding2` makes gfx prefs/display/compositor state process-global and early. In production it can initialize before the daemon has a writable profile, display/widget strategy, or final prefs. |
| P1 | `build/desktop/patches/0003-gfxplatform-jihad-disable-omtc-env.patch` | 13 | The OMTC override is a cached global env hack, not a sound render architecture. If `UsesOffMainThreadCompositing()` is called before the env/default prefs are set, the wrong value sticks for the process; it also cannot vary per page. |
| P2 | `render/goanna/test/embed_load.cpp` | 180 | `BrowserOffscreenInfo` is filled on the stack and discarded. If metadata is part of the adapter-visible contract, it is not delivered; if not, the code/comment falsely implies it is. |
| P2 | `render/goanna/test/embed_load.cpp` | 165 | The framebuffer size is derived from the captured GTK window, not from adapter-provided `sharedBufferSize` and page dimensions. The daemon path must validate dimensions before writing or it risks buffer mismatch/overflow. |
| P2 | `render/goanna/test/embed_load.cpp` | 177 | RGB-to-ARGB conversion handles source rowstride, but assumes at least 3 channels and forces alpha to `0xff`. GDK drawable capture under 24-bit Xvfb has no real alpha, so transparent content and premultiplied-alpha expectations are lost. |
| P2 | `render/goanna/test/embed_load.cpp` | 296 | Paint readiness is guessed with a fixed 1.5s sleep loop after load. A real server needs invalidation/composite completion driven frame delivery, otherwise it can send stale or blank frames under slow pages. |
| P2 | `render/goanna/test/embed_load.cpp` | 105 | Load completion treats any `STATE_STOP` with window/network flags as final and does not verify top-level progress. Subresources or redirects can mark the page done too early. |
| P2 | `render/goanna/test/embed_load.cpp` | 57 | `mDone` is initialized once and never reset before navigation. Reusing the chrome/browser for multiple loads would make later loads appear complete immediately. |
| P2 | `render/goanna/test/embed_load.cpp` | 232 | The GTK top-level window is never destroyed with `gtk_widget_destroy()`. `baseWin->Destroy()` tears down the embedded browser, not necessarily the host `GtkWidget`; daemon pages would leak native windows/resources. |
| P2 | `render/goanna/test/embed_load.cpp` | 258 | Critical return values are ignored: `AddWebBrowserListener`, `SetContainerWindow`, `Create`, `SetVisibility`, and `LoadURI`. Failures degrade into timeouts or invalid captures instead of deterministic teardown. |
| P2 | `render/goanna/EngineHost.cpp` | 46 | `greDir` is reused as the app dir and no directory-service/profile provider is supplied. Persistent browsing needs writable, controlled profile/cache/prefs locations before prefs and gfx are initialized. |
| P3 | `build/desktop/build-embed-load.sh` | 41 | The build script mutates `dist/bin/goanna.js` in place. This makes test behavior depend on previous runs and is not a clean deployment mechanism for daemon prefs. |
| P3 | `build/desktop/patches/0002-baseassembler-x64-format-overflow-pragma.patch` | 13 | The diagnostic suppression is file-wide and has no push/pop. It is probably build-only, but it masks future real `-Wformat-overflow` issues in the rest of the header. |
| P3 | `render/goanna/test/embed_load.cpp` | 306 | The disabled-render message says `JIHAD_TRY_RENDER`, but the code checks `JIHAD_NO_RENDER`. This will mislead debugging. |

**Top Risks**

The biggest blockers for daemonizing are lifecycle ordering, shm ownership, and event-loop integration. All Gecko/XPCOM objects must die before `XRE_TermEmbedding()`, the backend must attach and rotate the adapter-provided double buffers instead of allocating a fixed public segment, and paint delivery must be driven by the real main loop/invalidation path rather than sleeps.

The OMTC/gfx patches are acceptable as a smoke-test bridge, but they are too global and order-sensitive for a persistent multi-page render server. Treat them as proof-of-concept scaffolding until the backend has a deliberate in-tree widget/compositor/readback path.

---

## Disposition (applied 2026-06-30, post-review)

**Fixed in the test/backend now (verified still renders, exit 0):**
- P1 shm never IPC_RMID'd + fixed key → use `IPC_PRIVATE` + `shmctl(IPC_RMID)`; documented that the daemon uses adapter-provided `sharedBufferKey`s (not a self-allocated segment).
- P1 teardown before `XRE_TermEmbedding` → release ALL nsCOMPtr/RefPtr (`cur`, `weak`, `nav`, `baseWin`, `wb`, `chrome`, `thread`) before `host.Shutdown()`.
- P2 GTK window leak → `gtk_widget_destroy(win)` in teardown.
- P2 `mDone` not reset → reset before each `LoadURI`.
- P3 wrong env name in message → fixed to `JIHAD_NO_RENDER`.
- P1 EngineHost lifecycle → added a caution + documented the daemon invariant (Shutdown only after all pages destroyed; future revision asserts instance count).
- P2 alpha/BrowserOffscreenInfo comments clarified; P1 msgPainted uses shmKey (matches isis IpcBuffer key-based shmget on both sides).

**Correctly deferred to the daemon (design-level, tracked in kits/PORT-MAP, NOT test bugs):**
- P1 real GLib/XPCOM main-loop integration (vs the test's poll loop) — cavekit-engine-embedding R3 / T-019; the daemon keeps the BrowserServer GSource cadence.
- P1/P2 adapter-provided double buffers + `sharedBufferSize` validation + buffer rotation — cavekit-ipc-contract R2 / cavekit-offscreen-rendering R3 (T-024/T-032 in the daemon).
- P2 invalidation/DidPaint-driven frame delivery (vs fixed 1.5s wait) — cavekit-offscreen-rendering R3.
- P2 top-level-only load completion; P2 profile/dir-service provider — cavekit-navigation-events / cavekit-engine-embedding R2.
- P1 patches 0003/0004 are PoC scaffolding (Codex concurs "acceptable as a smoke-test bridge"); the device/daemon build needs a deliberate widget/compositor/readback path and a non-global prefs mechanism — noted in PORT-MAP.md + dead-ends.md.
- P3 goanna.js in-place mutation is test scaffolding; the OE package ships prefs properly (cavekit-device-build).
