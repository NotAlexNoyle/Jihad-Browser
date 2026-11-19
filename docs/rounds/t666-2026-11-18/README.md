# T-666, Eaglercraft loads in each shell (2026-11-18 and 19, in progress)

cavekit-eaglercraft.md R2. The clients are served from the host's cache on :8099 (`tools/host/eaglercraft/`). The
pictures the runner keeps of Eaglercraft's screens stay in `~/.cache/jihad-eaglercraft/rounds/t666-2026-11-18/` (they
carry the client's art, and its title screen Mojang's: `docs/rounds/.gitignore`); this file says what they show.

## First load of EaglercraftX 1.8.8 (engine 0119)

`eagler-load-188`, pictures at 30, 60, 120 and 180 s from the start of the load, in the three shells:

- 30 s: the page still loading (its 35 MB of HTML).
- By 60 s: the client's alert "WARNING: Detected "experimental" WebGL 1.0 support, certain graphics API features may be
  missing, and therefore EaglercraftX may malfunction and crash!" (Enyo, Mojo); behind it, by 120 s, the client's
  "Extracting Assets..." screen.
- 180 s: still extracting in Enyo (the alert up); in Mochi the engine's slow-script dialog over "Extracting Assets..."; in
  Mojo the slow-script dialog over the client's "press any key to enable sound... (required)". The script named is the
  page's own (`eaglercraft1-8.html:9633`), past `dom.max_script_run_time` (60 s, packaging/prefs).

## Why the client called WebGL "experimental": the engine's "webgl" name

The client makes its canvas's context with `getContext("webgl2", A)`, then `"webgl"`, then `"experimental-webgl"`
(A = antialias false, depth false, powerPreference "high-performance", desynchronized true, preserveDrawingBuffer false,
premultipliedAlpha false, alpha false), and warns when only the last answers. `render/goanna/test/jihad-webgl-attrs.html`
(`eagler-webgl-attrs`) tries each name with A, without each attribute in turn, and with none, and writes the engine's
own reason (the webglcontextcreationerror event):

| attempt | engine 0119 | engine 0120 |
|---|---|---|
| `webgl2` | null: "Error during EGL OpenGL init. Exhausted GL driver caps." | the same (WebGL 2 needs OpenGL ES 3; the Adreno 220 is ES 2.0: cavekit-gpu-rendering.md's non-goal) |
| `webgl`, with A, without any one attribute, or with none | null, no event | a context |
| `experimental-webgl` | a context | a context |

The cause: UXP maps `"webgl"` to WebGL 1 only when built with `MOZ_WEBGL_CONFORMANT`, a mozconfig variable this build
never set (`dom/canvas/CanvasUtils.cpp`), so every page asking for the standard name alone had no WebGL. Engine patch
0120 makes the two names aliases in every build, as WebGL 1.0 §2.1 says. `eagler-webgl-attrs` passes in the three shells
on engine 0120 (`finger-*-eagler-webgl-attrs.txt`, `kept/*-eagler-webgl-attrs-rows_.jpg`).

## Second and third loads (engine 0120)

- No alert any more. "Extracting Assets..." from about 60 s; the slow-script dialog by 120 s in the three shells (the
  extraction is one task of 60-120 s; the owner keeps `dom.max_script_run_time` at 60 s, 2026-11-19: the script taps
  Continue as a person would).
- With the guard off for one exploring run (Enyo), the extraction ended between 120 and 180 s and the client then
  waited at "press any key to enable sound... (required)"; a tap at 240 s did not count as a key (pictures at 255, 300,
  360 s unchanged). The client enters its tablet mode only when the UA holds "mobi" or "tablet"
  (`navigator.userAgent.toLowerCase()`), and its desktop mode wants a key a touch screen cannot press.
- **The owner, 2026-11-19: "Tablet" in the UA everywhere** (Firefox's tablet token; commit `30f3d77f`). Then (Enyo): the
  "Mobile Browser Detected" panel at 182 s from the start of the load, its "Launch EaglercraftX" tapped and gone; the
  canvas still at "press any key" 60 s later.

## The client's own console: a WeakMap that refused WebGL objects

A probe hooked `console.*` and `onerror` at the start of the load (Enyo), seconds from that hook:

```
4s   ClientMain: eaglercraftx is starting...
21s  Creating WebGL context
21s  [WebGL Error]: WebGL creation failed: Error during EGL OpenGL init. Exhausted GL driver caps.   (its webgl2 try)
21s  OpenGL Version: WebGL 1.0; extensions EXT_texture_filter_anisotropic, OES_texture_float, OES_texture_half_float,
     OES_texture_half_float_linear, OES_vertex_array_object
25s  Downloading: <data: 12269833 chars>
48s  Decompressing: <data: 12269833 chars>
134s Loaded 4872 resources from EPKs
136s Initializing sound engine... / Waiting for user to select option on mobile press any key screen
144s onerror: TypeError: cannot use the given object as a weak map key (eaglercraft1-8.html:1315)
```

The client keeps its JS objects' Java wrappers in a WeakMap (`HOD.set(obj, new WeakRef(wrapper))` for any object or
function); the engine's WeakMap asks the DOM to preserve a key's wrapper, and UXP's `TryPreserveWrapper` answered false
for every cycle-collected WebIDL class that is not nsISupports: the WebGL objects among them.
`render/goanna/test/jihad-weakmap-dom.html` (`weakmap-dom-keys`) on engine 0120 (Enyo): an element taken; WebGLBuffer,
WebGLTexture refused by WeakMap and WeakSet. Engine patch **0121** calls such a class's addProperty hook (which Codegen
gives it for exactly this: it preserves the wrapper and nothing else), as later Gecko does.

## Past Launch (engine 0121): the shader preprocessor

With 0121 the Launch goes through (two slow-script dialogs answered in one run, `wait rect-ok`), and the client sets up
its renderer: no 16- or 32-bit float render targets (its shaders off, as expected on this GPU), no instancing; then:

```
183s error: Failed to compile GL_VERTEX_SHADER "/assets/eagler/glsl/local.vsh"!
183s error: [VERT] ERROR: 0:19: '(' : syntax error
183s error: ClientMain: eaglercraftx's runtime could not be initialized!
```

Line 19 is `EAGLER_VSH_LAYOUT_BEGIN()`, a function-like macro with no parameters and an empty body that the client's
prelude defines (the source the engine got, captured by a probe on `compileShader`, has the prelude and the defines
before it). `render/goanna/test/jihad-webgl-macro.html` (`webgl-macro`), engine 0121, the three shells: plain and
object-like macros compile; every function-like macro fails at its invocation, empty, with a body or with parameters.

The cause is upstream: UXP's Issue #2978 (2026-03, "Avoid empty replacements in macro expansion") put
`replacements->size() == 0 ||` before `collectMacroArgs` in ANGLE's `MacroExpander::expandMacro`. `replacements` is that
function's output, cleared on entry and filled only after that call, so the test is always true and no function-like
macro expands. Engine patch **0122** takes the test out and bounds what can actually run away, the arguments'
pre-expansion (one MacroExpander nested in another per level), at 256 levels, as later ANGLE does.

## Past the shaders (engines 0122 and 0123): slow, then a GPU hang

- **0122, 20 minutes past Launch, Continue tapped as it came (8, 8, 6 times per 5 minutes):** the Mojang screen by 10
  minutes; `[OptiFine]: ConnectedTextures` at 1101 s and `Created: 1024x512 textures-atlas` at 1186 s from the load's
  start; no title screen. The stack at each slow-script prompt (trace mode, `035f6d12`): Minecraft's block-state tables
  (TeaVM `HashMap.get` under the same loop), slow, not stuck.
- **Engine 0123** (the CPU count; `docs/rounds/cpucount-2026-11-19`): the same milestone at 904 s instead of 1101.
- **Then, on 0123 at 09:12:10 (device time), the GPU hung** while the client built its texture atlas: the kernel's
  `kgsl_postmortem_dump` (an `IB1` of 29020 dwords the Adreno 220 never finished), `kgsl_yamato_idle: spun too long
  waiting for RB to idle` every few seconds after it, the daemon's GL wait (`IOCTL_KGSL_DEVICE_WAITTIMESTAMP ... EINTR`)
  654 times, the daemon unable to stop, LunaSysMgr's screenshots empty, and a LunaSysMgr restart not enough: **the device
  was rebooted** (battery 100 %; nothing boot-related had changed). A web page that can wedge the GPU freezes every card
  and the system UI until a reboot: that is the defect to find first (the GL calls before the hang are traced next).

## The GPU hang again, and what the game's start costs (engine 0123)

- **A second untraced load hung the GPU the same way** (a `kgsl_postmortem_dump` about 23 minutes in, after the atlas);
  the device was rebooted again.
- **A WebGL trace** (every call, sent as beacons to a logger on the host before it ran): the atlas is a 1024x512
  `texImage2D(null)` and some 1400 16x16 `texSubImage2D`, each after four `texParameteri`. Both traced loads stopped
  sending at the same call (#7546, a `bindFramebuffer` after the last upload) and neither hung the GPU or reached the title
  in 45 minutes: the tracer itself is suspect there. `jihad-webgl-uploads.html` (6000 such uploads in one task, no
  flush) ran in 2.4 s and left the GPU idle 10 ms later: that pattern alone is not the trigger.
- **What the start costs.** The client makes every Java `long` a BigInt when the browser has BigInt.
  `jihad-bigint-bench.html` on the device (Enyo, 300 000 steps): numbers 38 ms, BigInt (TeaVM's `asIntN(64, ...)`)
  **2312 ms**, TeaVM's `{lo,hi}` object fallback 56 ms; on the host's V8 numbers 16 ms, BigInt 47 ms. UXP's BigInt runs in
  C++ outside the JITs, about 60 times its number arithmetic; the slow-script stacks during the start are TeaVM's
  `HashMap` and typed-array helpers.

**Where 1.8.8 stands on the TouchPad:** Launch at 182-196 s; the Mojang screen by about 10 minutes; its texture atlas at
15-20 minutes; no title screen within 45 minutes; and, untraced, a GPU hang that only a reboot clears. The 180 s bound is
far out of reach without making BigInt (or the code that uses it) much faster.

## Next

The owner's call (asked 2026-11-19): BigInt in the engine now, or Eaglercraft's speed parked; and the GPU hang (a WebGL trace beaconed to the host); then 1.8.8 to its title screen; the title screen's reference
picture; the menus by touch; 1.12.2 and 1.5.2.
