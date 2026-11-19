# T-666, Eaglercraft loads in each shell (2026-11-18, in progress)

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

## Next

1.8.8 again on engine 0120 (no alert expected); then the slow-script dialog and the "press any key" screen; the title
screen's reference picture; 1.12.2 and 1.5.2.
