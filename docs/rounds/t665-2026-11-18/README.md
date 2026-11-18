# T-665, Eaglercraft's clients declared, fetched, and read (2026-11-18)

cavekit-eaglercraft.md R1. The clients are eaglercraft.com's own downloads (its `/downloads` page, read 2026-11-18):
`tools/host/eaglercraft/clients.json` names each with its source, release, size and SHA-256;
`eaglercraft-fetch.py` fetches them into `~/.cache/jihad-eaglercraft/`, checks them and unpacks them (each is one
offline HTML file); `needs.py` reads what each asks of the browser from its own code. Nothing of them is in the
repository: they carry Mojang's assets.

## What each asks for (`needs.py`)

| client | build | HTML | WebGL asked (in order) | extensions named | platform parts |
|---|---|---|---|---|---|
| eagler-1.5.2 (1.5.2 sp2.01) | javascript | 23.5 MB | webgl then webgl2 | EXT_texture_filter_anisotropic, WEBGL_debug_renderer_info | IndexedDB, Workers, WebSocket, Web Audio, pointer lock, BigInt |
| eaglerx-1.8.8 (1.8.8 (62da5f39)) | javascript | 34.9 MB | webgl2 then webgl then experimental-webgl | ANGLE_instanced_arrays, EXT_color_buffer_float, EXT_color_buffer_half_float, EXT_shader_texture_lod, EXT_texture_filter_anisotropic, OES_fbo_render_mipmap, OES_texture_float, OES_texture_float_linear, OES_texture_half_float, OES_texture_half_float_linear, OES_vertex_array_object, WEBGL_debug_renderer_info, WEBGL_lose_context | IndexedDB, Workers, WebSocket, Web Audio, pointer lock, full screen, touch, BigInt |
| eaglerx-1.8.8-wasm (1.8.8 (62da5f39)) | wasm-gc | 16.8 MB | none | none |  |
| eaglerx-1.12.2 (1.12.2 (62da5f39)) | javascript | 42.9 MB | webgl2 then webgl then experimental-webgl | ANGLE_instanced_arrays, EXT_color_buffer_float, EXT_color_buffer_half_float, EXT_shader_texture_lod, EXT_texture_filter_anisotropic, OES_fbo_render_mipmap, OES_texture_float, OES_texture_float_linear, OES_texture_half_float, OES_texture_half_float_linear, OES_vertex_array_object, WEBGL_debug_renderer_info | IndexedDB, Workers, WebSocket, Web Audio, pointer lock, full screen, BigInt |

## Read from the code, beyond the table

- **Eaglercraft 1.5.2 sp2.01 needs WebGL 2.** Its one `getContext("webgl")` is a 64x64 probe that reads the GPU's
  name for the crash report; the game's own canvas is made with `getContext("webgl2", ...)` and a null answer ends in
  its "WebGL 2.0 is not supported" screen. This device's GPU is OpenGL ES 2.0 (the Adreno 220), WebGL 2 needs ES 3.0, and
  WebGL 2 is cavekit-gpu-rendering.md's non-goal: **1.5.2 sp2.01 cannot run here; a limit, to be confirmed on the
  device (its screen kept).**
- **EaglercraftX 1.8.8 and 1.12.2 fall back to WebGL 1**: `webgl2`, then `webgl`, then `experimental-webgl`,
  with a "Forcing WebGL 1.0 context" path; their README says shaders (dynamic lights, PBR) need WebGL 2 and are off in
  WebGL 1 mode. In WebGL 1 mode they name `OES_vertex_array_object` and `ANGLE_instanced_arrays` (which of the named
  extensions are required and which only used when present is the device round's to find).
- **1.8.8 has touch controls** (`touchstart`); 1.12.2's build names none.
- **All three JavaScript builds use BigInt** (TeaVM's 64-bit longs): the engine must have it (UXP has had BigInt since
  2023; to be confirmed on the device).
- **The WebAssembly build (EaglercraftX 1.8.8 WASM-GC) is not tested**: WebAssembly GC is not in this engine (UXP's
  WebAssembly is the MVP); the JavaScript build of the same release is the one supported.
- **Size:** the offline files are 23.5 to 43 MB of HTML (the assets as base64 in the page): parsing one is itself a
  memory question on a 1 GB device (cavekit-device-headroom.md), measured in T-666.
- **Memory the clients' own settings ask for: none.** The JavaScript builds' launch options (`eaglercraftXOpts` for
  1.8.8 and 1.12.2: the container, the worlds database, the relays; `eaglercraftOpts` for 1.5.2: the container, the
  assets, the integrated server's worker, the worlds folder, the relays) carry no memory figure, and TeaVM's Java heap
  is the JavaScript engine's own (the `maxMemory` the game reports is a value of the build, not an allocation). What
  the device spends is measured in T-666/T-668 (the daemon's and the device's memory at the title screen, in a world,
  after 15 minutes).

## The hosted forms

Each client is also published hosted, at `https://eaglercraft.com/play?version=<client>` (`1.5.2`, `1.8.8`,
`1.8.8-wasm`, `1.12.2`; recorded in `clients.json`). That page is a launcher, not the client: a Next.js application
(ads, telemetry, cloud saves, a worker) that fetches the chosen build at run time from
`cdn.eaglercraft.net/objects/builds/<parent>/<child>/<version>` (mirror `cdn.eaglercraft.ru`), named by a descriptor it
reads then. Its own download table names the same offline zips `clients.json` pins. It is no fixed artifact, so its
size and SHA-256 in `clients.json` are the launcher page as read on 2026-11-18 (294 298 bytes). The device round tests
the offline builds and tries the launcher once, recording what it does (T-666).

So the clients to support are **EaglercraftX 1.8.8** (with touch) first, and **1.12.2**; 1.5.2 sp2.01 is expected to be a
limit.
