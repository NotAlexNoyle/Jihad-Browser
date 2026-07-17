# Jihad Browser — agent & reviewer context (incl. Codex)

This file primes any automated agent (including the Codex adversarial reviewer run via
cavekit) with the webOS-3 platform knowledge needed to review this codebase correctly.
**The authoritative, curated platform knowledge is the `webos-mcp` server**
(https://github.com/webOSArchive/webos-mcp) — resource `webos://knowledge/all`. The facts
below are the subset that matters for the browser port; when in doubt defer to webos-mcp.

## What this project is
Jihad Browser is a fork of **isis-browser** (webOS 3 / HP TouchPad) that swaps the QtWebKit
render engine for **UXP/Goanna** (an ESR52-derived, MPL-2.0 Gecko fork), while keeping the
isis UI and the **BrowserAdapter↔BrowserServer YAP IPC contract byte-identical**. Target:
HP TouchPad (topaz/tenderloin) + TouchPad Go (opal), webOS 3.0.5, ARMv7l. The goal is to
**seriously compete with Atlas Browser** (which uses a modern WPE/WebKit backend); no janky
shortcuts. **Must run on as little as 512 MB RAM** — memory is the #1 constraint. Heavy modern
pages were seen rendering fully then degrading to near-blank as surface/layer memory is evicted,
so engine perf/memory prefs are tuned low (bounded caches, no bfcache viewers, capped frame rate —
see `render/goanna/EngineHost.cpp`). Modern-site compat also relies on per-domain UA overrides
(a daemon `http-on-modify-request` observer; the pref-based path isn't init'd in this embedding).

## Architecture that a reviewer must know
- **BrowserAdapter** (in-process NPAPI plugin inside LunaSysMgr) ⇄ **BrowserServer** daemon
  (`jihad-browserserver`) over **YAP** (a length-prefixed socket RPC). The adapter sends
  commands (openUrl, clickAt, keyDown, returnBuffer…); the server sends messages (msgPainted,
  load lifecycle…). This wire contract must not change.
- Rendering is **offscreen/headless**: no X server, no real widget. The daemon drives a
  **PuppetWidget** (`JIHAD_OFFSCREEN=1`) and paints into **SysV shared-memory** buffers the
  adapter allocates (double-buffered; the adapter holds one buffer and returns the previous on
  each new msgPainted — honor `returnBuffer` before reusing a buffer or you corrupt the frame
  the adapter is still blitting).
- The engine editor **does** work headless after UXP patch 0010 (mTabChild null-guard):
  `Focus`, `SetValue`, `GetSelectionStart`/`SetSelectionRange` are crash-free. Synthetic key
  and mouse events, however, do **not** drive the editor headless (no text insert, no click
  hit-test) — so text editing is done by direct DOM value/selection manipulation.

## webOS gotchas that affect reviews here
- **On-screen keyboard (VKB) key codes** (measured on-device): printable chars arrive in the
  NPAPI key event's `key`/`rawkeyCode` field with `chr==0`. Control keys use ASCII where it
  exists (Backspace=8, Tab=9, Enter=13, Esc=27, DEL=127). Arrows use webOS's private-use block
  **0xE0A0=Down, 0xE0A1=Up, 0xE0A2=Left, 0xE0A3=Right**. These PUA codes must never be inserted
  as text (they render as invisible glyphs).
- **`javascript:` URLs reflow the page white**: loading a `javascript:` URL through the
  docShell flashes the isis loading overlay ("the page whites out"). Do not use it for
  per-interaction operations (per-keystroke, per-tap).
- **Never log keystrokes / field values** (F-163): the daemon's stderr is redirected to a
  persistent, user-readable file on device. Numeric key codes of *typed characters* are
  effectively the text — treat them as secret.
- **App/UI layer is ES5 only** (2009-era WebKit): no `let`/`const`/arrow-fns/Promises in
  `app/` (Enyo 1). The daemon/adapter are C++ (adapter is PDK gcc4.3.3 / C++03 — no `nullptr`,
  no C++11; the daemon is gcc9 / C++17).
- **BusyBox on device** lacks many GNU options (`head -c`, `tr '[:space:]'`, `cp` can EBADF on
  some mounts — use `cat >`). App content renders to `/dev/fb1`, not `fb0`.

## Licensing (must be upheld with credit)
UXP/Goanna family = MPL-2.0 (credit Moonchild); Atlas references = Apache-2.0 (credit
Herrie82); isis/webOS adapter = Apache-2.0 (HP). Do not vendor engine source; build out-of-tree.
