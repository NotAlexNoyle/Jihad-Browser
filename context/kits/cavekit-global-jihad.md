---
created: "2026-11-11"
last_edited: "2026-11-11"
complexity: thorough
---

# Cavekit: Global Jihad

## Scope
Global Jihad is a project distinct from Jihad Browser. **It begins only when Jihad Browser is finished;** nothing of it is
built inside Jihad Browser before then. The owner set its scope:
- 2026-10-18: system-wide Goanna for every app on the TouchPad.
- 2026-11-11: "my goal is to replace webkit and chromium globally": every app runtime in the webOS family that renders
  with WebKit or Chromium.
- 2026-11-11: "the aim should always be to replace the system engine rather than run beside it to save ram".
- 2026-11-11: "no lunacy for global jihad"; "drop lg tv support".
- 2026-11-11: "global jihad should actively block ce ota updates (via the community checker) until its uninstalled".

The TouchPad strategy (rebuilding the forked WebAppManager on Goanna, patching the frameworks in place at
`/usr/palm/frameworks`, the `PalmSystem` and `PalmServiceBridge` objects, NPAPI hosting, the IME path) and the
on-device facts behind it are in `docs/PICKUP.md`, section "After Jihad Browser: Global Jihad".

**Will support:**
- HP TouchPad and TouchPad Go on stock webOS 3.0.x (Palm's WebKit 534.6, `libWebKitLuna.so`, in the forked
  `LunaSysMgr -s` WebAppManager).
- HP TouchPad on webOS CE 3.1.0 and later CE releases (the same WebKit, byte-patched by CE; CE 3.2.0's OTAs rewrite
  `libWebKitLuna.so` and WebAppMgr files, so each is checked against Global Jihad).
- webOS 2.x phones: Pre 2, Pre 3, Veer, on 2.2.4 (Palm's WebKit, WebAppMgr, Mojo). Builds on the planned port of the
  browser and engine to legacy webOS. 512 MB RAM.
- webOS 1.x phones: Pre, Pre Plus, Pixi, Pixi Plus (Palm's older WebKit, Mojo). The same port; 256 MB RAM, and the Pixi
  is ARMv6 without NEON (an engine build of its own).
- LuneOS (webOS-ports): Qt WebEngine (Chromium) behind the webOS OSE WebAppManager (WAM), and its browser. A Goanna
  backend for WAM.
- webOS OSE: Chromium behind the same WAM. The same backend.

**Won't support:**
- Lunacy, on Android and iOS (the owner, 2026-11-11). Its source (webOSArchive/Lunacy at `9a7fca6`) runs every card in
  `android.webkit.WebView`, and on iOS in `WKWebView`.
- LG webOS TVs, rooted or not (the owner, 2026-11-11).
- Open webOS 1.x (HP, 2012, QtWebKit 2): no live users or devices.
- Android and iOS in general: the system WebView cannot be swapped on stock Android, and iOS requires WebKit outside the
  EU.

## Requirements

### R1: The engine replaced on every supported platform, never run beside it
**Acceptance Criteria:**
- [ ] On each platform under "Will support", apps render with Goanna, and the platform's own WebKit or Chromium is not
  loaded: no process maps it while apps run. It stays on disk at most as a fallback during a migration.
- [ ] The memory of a running app, measured on the device, is at most what the system engine needed for the same app
  (the reason for replacing rather than adding).
- [ ] The apps need no change of their own: frameworks and system objects are provided by the platform layer.
**Dependencies:** Jihad Browser finished; the legacy browser and engine port for the phone rows.

### R2: Platform order
**Acceptance Criteria:**
- [ ] The TouchPad on stock 3.0.x and CE first, both (stock is untestable on the bench until the stock TouchPad
  arrives; the owner is asked before stock behaviour is claimed).
- [ ] Then the phones (after the legacy port), then LuneOS and webOS OSE (one WAM backend); the order among these is
  the owner's call when the TouchPad is done.
**Dependencies:** R1

### R3: CE OTA updates blocked while Global Jihad is installed
CE's community update path, as built at CE's main branch `b1d75e2`: the OTA Ready app `org.webosarchive.otaready`, its
root daemon, and its reroute of the native "System Updates" app `com.palm.app.updates`. It asks the community server
(`GET /api/updates/check`) what the device needs, stages the packages, and installs them through
`make-update-uimage` and a reboot. A CE update replaces system files (CE 3.2.0 rewrites `libWebKitLuna.so` and
WebAppMgr files), which would undo or break the engine replacement.
**Acceptance Criteria:**
- [ ] While Global Jihad is installed on CE, the community checker offers no CE update and stages none. The block is
  active: it does not depend on the server, on timing, or on the user declining.
- [ ] The user who opens the updater sees that CE updates are held because Global Jihad is installed, and that removing
  Global Jihad releases them.
- [ ] Nothing CE's updater already staged can be installed while Global Jihad is installed. The device boots and
  works normally, with Global Jihad intact.
- [ ] Uninstalling Global Jihad lifts the block completely: the next check reaches the server and offers what it offers
  any CE device. No change to CE's own files is left behind.
- [ ] Re-checked against each CE release that changes the checker, since CE's OTA client is still being built
  (`Docs/OTA-STRATEGY.md`).
- [ ] (R1's companion) Global Jihad's daemons start at the first boot, unlike Jihad Browser's (the owner,
  2026-11-11: "global jihads daemons should start at first boot"): every app needs the engine from the start
**Dependencies:** R1

## Out of Scope
- Everything under "Won't support".
- Any work before Jihad Browser is finished.

## Cross-References
- `docs/PICKUP.md` "After Jihad Browser: Global Jihad": the TouchPad strategy and its facts.
- cavekit-webos-sdk-update.md: the device tools the work runs on.

## Changelog
- 2026-11-11: R3 (CE OTA updates blocked until Global Jihad is uninstalled); LG TVs dropped.
- 2026-11-11: created from the owner's scope decisions of 2026-10-18 and 2026-11-11 (WebKit and Chromium globally,
  replace never beside, no Lunacy).
