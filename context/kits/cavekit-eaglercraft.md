---
created: "2026-11-18"
last_edited: "2026-11-18"
complexity: high
---

# Cavekit: Eaglercraft

## Scope
The owner, 2026-11-18: **"new kit: eaglercraft support"**.

Eaglercraft is Minecraft Java Edition's client compiled to JavaScript (TeaVM) so it runs in a web page: Eaglercraft 1.5.2
and EaglercraftX 1.8.8, each as a hosted page or as one offline HTML file. It draws with WebGL, plays sound through Web
Audio, keeps worlds and settings in IndexedDB, runs single-player's integrated server in a Web Worker, reaches servers
over WebSockets, and wants the keyboard, pointer lock and full screen. 1.8.8's newer releases add touch controls.
Every one of those is a part of this engine and this device that a page this heavy has not yet leaned on together: WebGL 1
on the Adreno 220 (cavekit-gpu-rendering.md R8; WebGL 2 is that kit's non-goal), a gigabyte of memory shared with the
system (cavekit-device-headroom.md), two cores, and a card with no hardware keyboard or mouse.

**Support means:** a published Eaglercraft client loads in each of the three interfaces, a person can play it with the
device's own input (its touch screen, the system keyboard, and a Bluetooth keyboard where one is paired), a world is
created, played, saved and found again, a server is joined, and the device stays reachable throughout. Where a release
cannot run on this device's engine (a WebAssembly-GC build, or a WebGL 2 requirement this engine does not meet), that is
measured, recorded as a limit with its evidence, and the release that can run is the one supported.

**Not redistributed:** the clients carry Mojang's assets. As with the Flash corpus (cavekit-flash.md R1), the files are
fetched from their public sources into a host cache by a script that records each one's source and checksum; the
repository holds the script and the manifest, never the files.

**Out of scope:** hosting a public Eaglercraft server; changing a client's own code; Minecraft Bedrock or Java proper.

## Requirements

### R1: The clients, declared and fetched
**Description:** What "Eaglercraft" means here is a written list, reproducible on the host.
**Acceptance Criteria:**
- [x] A manifest names at least Eaglercraft 1.5.2 and EaglercraftX 1.8.8 (the latest release of each that publishes a
  JavaScript build), each as its offline HTML and, where published, its hosted form, with the source URL, the release,
  the size and the SHA-256; a script fetches them into the host cache and checks them *(2026-11-18, T-665:
  tools/host/eaglercraft/clients.json, eaglercraft-fetch.py; 1.5.2 sp2.01, 1.8.8 JS and WASM-GC, 1.12.2; the hosted
  form is eaglercraft.com/play?version=, a launcher with no fixed artifact, its page's hash as read;
  docs/rounds/t665-2026-11-18)*
- [x] For each, what it needs is read from its own code and recorded before any device round: the WebGL version and the
  extensions it asks for, Web Audio, IndexedDB, Workers, WebSockets, pointer lock, full screen, touch input, and how much
  memory its own settings ask for *(2026-11-18, T-665: needs.py; 1.5.2 needs WebGL 2, 1.8.8/1.12.2 fall back to WebGL 1,
  1.8.8 has touch, all use BigInt, no client's settings name a memory figure; docs/rounds/t665-2026-11-18)*
- [x] A WebAssembly or WebAssembly-GC build is listed with the reason it is not the one tested, or tested if the engine
  runs it *(2026-11-18, T-665: EaglercraftX 1.8.8 WASM-GC listed; UXP's WebAssembly is the MVP, without GC)*
**Dependencies:** none

### R2: Loads, in each shell
**Description:** The client reaches its title screen and its menus answer.
**Acceptance Criteria:**
- [ ] Each declared client, served from the host, reaches its title screen in Enyo, Mochi and Mojo within a bound
  written here before the round (by a reference picture of the title screen, R7's check)
- [ ] Its menus (Options, Video settings, Language, Singleplayer, Multiplayer) open and close by touch
- [ ] What fails to load is recorded with the engine's own error (the console, a missing API, the WebGL context's
  refusal) and either fixed in the engine or recorded as a limit with that evidence
**Dependencies:** R1, cavekit-gpu-rendering.md R8

### R3: Played with the device's own input
**Description:** A person without a mouse can play.
**Acceptance Criteria:**
- [ ] The client's own touch controls, where the release has them, move, look, jump, place and break blocks by touch
- [ ] Where a release has no touch controls, the keys it needs reach it from the system keyboard (movement, jump, the
  inventory, Escape, chat and its typing) and the look follows a drag on the game; or the release is recorded as not
  playable by touch, with what a Bluetooth keyboard adds
- [ ] The game holds the input while it has it: a drag on it does not pan or zoom the page, a tap does not leave it
  (the gesture lock of cavekit-input-bridging.md and the plugin lane's rule, applied to a canvas that asks for pointer
  lock or full screen)
- [ ] Pointer lock and full screen requests are honoured as far as the card allows, and Escape gives them back
**Dependencies:** R2, cavekit-input-bridging.md

### R4: A world kept
**Description:** Single-player works end to end.
**Acceptance Criteria:**
- [ ] A new single-player world is created and entered; blocks placed and broken are there after leaving and entering
  the world again, after closing the card, and after a daemon restart (IndexedDB in the variant's own profile)
- [ ] The integrated server's worker runs; a world of the client's default size loads without exhausting the device's
  memory, with the daemon's and the device's memory recorded at the title screen, in the world, and after 15 minutes
- [ ] A world exported by the client's own export, and imported again, is the same world
**Dependencies:** R2, cavekit-device-headroom.md

### R5: A server joined
**Description:** Multiplayer over the client's WebSocket protocol.
**Acceptance Criteria:**
- [ ] A test server on the host (a Minecraft server with the Eaglercraft protocol plugin for the client's version) is
  joined by each client from each shell over `ws://`, and over `wss://` with the host's own certificate trusted
  (cavekit-certificate-trust.md)
- [ ] Two cards (or two shells) on the server see each other move and chat
- [ ] A dropped connection returns the client to its menu with its message, and joining again works
**Dependencies:** R2

### R6: Fast enough, and the device kept
**Description:** Numbers, against bars written here before the round.
**Acceptance Criteria:**
- [ ] The frame rate the client itself reports (its F3 counter) and the content rate the frame stream reads are
  recorded in the title screen and in a world at the client's lowest settings, for each client in each shell; the bar
  for "playable" is written here before the round
- [ ] What costs the most on the device is measured (the JS heap and GC, WebGL draw calls, the frame path) and the
  levers within this engine's reach are tried, each against the baseline, kept only on evidence
- [ ] Throughout a 15-minute session the device stays reachable (USB, the card, other cards), the memory guard of
  cavekit-device-headroom.md is not tripped by the browser, and sound plays
**Dependencies:** R2, R4

### R7: In the suite
**Description:** All of the above by script, in the three shells.
**Acceptance Criteria:**
- [ ] Each criterion of R2-R6 is a finger script of the suite (`docs/FINGER-TESTS.md`) or a round of a tool beside it,
  with reference pictures for the client's screens, run in Enyo, Mochi and Mojo
- [ ] The rounds' results and every limit are in `docs/rounds/`, and the README says what runs
**Dependencies:** R2-R6, cavekit-finger-testing.md
