---
created: "2026-08-01"
last_edited: "2026-08-01"
---

# Cavekit: Add-ons & Extensions

## Scope
`about:addons` and XPI extension support, working. User requirement, 2026-08-01: *"i want
about:addons and xpi extension support to work."*

This is a real capability, not a checkbox: a user can open the add-ons manager, install an `.xpi`,
and have it take effect on browsing — and it survives a restart. Goanna/UXP is a Firefox-52-era
fork, so it carries the **classic (bootstrapped/overlay) XPI** stack, not WebExtensions; that is
the extension model in play here.

The engine side already exists and is already shipped in the device bundle. What was missing is the
**application identity** every part of that stack keys off.

## Why this was broken (diagnosed on-device 2026-08-01)

The daemon embeds the engine through `XRE_InitEmbedding2`, which has no `application.ini` — so the
runtime had **no `nsIXULAppInfo` at all**. `AddonManager.jsm:770-781` does:

```js
try { appChanged = Services.appinfo.version != oldAppVersion; } catch (e) { }
…
if (appChanged !== false) {
  Services.prefs.setCharPref(PREF_EM_LAST_APP_VERSION,      Services.appinfo.version);
  Services.prefs.setCharPref(PREF_EM_LAST_PLATFORM_VERSION, Services.appinfo.platformVersion);
```

The `try/catch` swallows the missing-appinfo throw, leaves `appChanged` undefined, so
`appChanged !== false` is true, and it walks into `setCharPref(…, undefined)` →
`NS_ERROR_ILLEGAL_VALUE`. On desktop that is survivable noise; **on the device the daemon
SIGSEGVs** in the same window — `XRE_NotifyProfile()` → `DoStartup()` — reproducibly, 117 upstart
respawns, exit 139. Proven by a kill switch: with `XRE_NotifyProfile` skipped the daemon reaches
`engine up; serving YAP` and binds its socket; with it enabled it dies every time.

So R1 below is simultaneously the P0 crash fix and the prerequisite for the whole feature. Skipping
`XRE_NotifyProfile` is NOT an acceptable workaround — it is what enables the profile, and therefore
cookie persistence (cavekit-browser-services R2).

## What is already present (verified in the shipped bundle, 2026-08-01)

Nothing needs to be added to the engine build for the UI to exist:

| Piece | Where |
|---|---|
| `about:addons` → the UI | `nsAboutRedirector.cpp:39` maps `"addons"` → `chrome://mozapps/content/extensions/extensions.xul` |
| The add-ons manager UI | `chrome/toolkit/content/mozapps/extensions/extensions.xul` (+ `.xml` bindings) |
| The XPI install UI | `chrome/toolkit/content/mozapps/xpinstall/` |
| Locale + skin | `chrome/en-US/locale/en-US/mozapps/`, `chrome/toolkit/skin/classic/mozapps/` |
| The manager itself | `components/addonManager.js`, `components/extensions.manifest`, `modules/AddonManager.jsm`, `modules/addons/` |

## Requirements

### R1: The embedded runtime has a real application identity
**Description:** The daemon registers `nsIXULAppInfo` (and `nsIXULRuntime`) before the profile is notified, so every consumer that keys off app identity — the add-on manager first among them — has real values.
**Acceptance Criteria:**
- [ ] An `nsIXULAppInfo` implementation is registered BEFORE `XRE_NotifyProfile()`, exposing at minimum `vendor`, `name`, `ID`, `version`, `appBuildID`, `platformVersion`, `platformBuildID`.
- [ ] The `nsIXULRuntime` members needed by the add-on stack (`OS`, `XPCOMABI`, `widgetToolkit`, `processType`, `inSafeMode`) are answered rather than throwing.
- [ ] The application **ID** is stable and documented — extensions declare `targetApplication` against it, so changing it later invalidates every installed extension's compatibility.
- [ ] Identity values come from ONE source shared with the User-Agent string (`render/goanna/JihadUserAgent.h`), not a second hard-coded copy.
- [ ] No Pale Moon / Basilisk / Moonchild identity is reintroduced (cavekit-licensing-branding.md R3 strips those; this is Jihad's own identity).
- [ ] **The device SIGSEGV in `XRE_NotifyProfile()` → `DoStartup()` is gone**, with `XRE_NotifyProfile` ENABLED (the `JIHAD_NO_PROFILE_NOTIFY` diagnostic switch off), and `AddonManager.jsm` no longer logs `NS_ERROR_ILLEGAL_VALUE`.
**Dependencies:** cavekit-engine-embedding.md (R2)

### R2: `about:addons` opens and is usable
**Description:** The user can reach the add-ons manager from the browser and operate it.
**Acceptance Criteria:**
- [ ] Navigating to `about:addons` renders the add-ons manager rather than an error or a blank card.
- [ ] Installed extensions, themes and plugins are listed with name, version and state.
- [ ] Enable / disable / remove work from the UI and the change is reflected after a restart.
- [ ] The page is **operable through the adapter's synthesized input path**, not merely rendered. `about:addons` is a XUL document, and this project has a recorded XUL input hazard (XUL taps crashed, so `mouseSend` was deliberately skipped — see [[jihad-input-activation-and-tiling]]). If XUL input needs work, that is part of this requirement, not an excuse.
**Dependencies:** R1, cavekit-input-bridging.md, cavekit-offscreen-rendering.md

### R3: XPI installation works
**Description:** A user can install an extension from a file or a URL.
**Acceptance Criteria:**
- [ ] Navigating to / opening an `.xpi` triggers the install flow rather than a download or a MIME handoff.
- [ ] The install prompt identifies the extension and can be accepted or declined; declining installs nothing.
- [ ] After accepting, the extension appears in `about:addons` and is active (subject to a restart if it is not bootstrapped).
- [ ] An extension whose `targetApplication` does not match this app's ID/version is rejected with a clear reason rather than installed and silently inert.
**Dependencies:** R1, R2, cavekit-browser-services.md (R4)

### R4: An installed extension actually affects browsing
**Description:** Extensions are functional, not just listed.
**Acceptance Criteria:**
- [ ] A test extension that observably alters page behavior (e.g. blocks a request, injects a style, or rewrites content) demonstrably does so on a real page.
- [ ] Disabling it stops the effect; re-enabling restores it.
**Dependencies:** R3

### R5: Extensions are per-variant and survive restart
**Description:** Extension state persists, and — per cavekit-device-build.md R7 — belongs to exactly one variant.
**Acceptance Criteria:**
- [ ] Installed extensions survive a daemon restart and a device reboot.
- [ ] Each variant's extensions live in ITS OWN profile (`$APP/profile/extensions`), so installing an extension in one variant does not appear in, or affect, another.
- [ ] Removing a variant removes its extensions with it and leaves the other variants' untouched.
**Dependencies:** R1, cavekit-device-build.md (R7, R8)

### R6: Extension storage respects the install-footprint contract
**Description:** Add-on data lands where the rest of the engine's state does.
**Acceptance Criteria:**
- [ ] Extensions and their data live under the variant's profile on cryptofs — never on `/media/internal` (the user's volume) and never on the 62 MB `/var` partition.
- [ ] Extension install/removal writes nothing outside the variant's own profile.
**Dependencies:** cavekit-device-build.md (R8)

## Out of Scope
- **WebExtensions.** UXP is a Firefox-52-era fork carrying the classic XPI stack; the WebExtensions
  API surface is not the target and is not implied by any criterion above.
- Signing/AMO integration, automatic update checks, and the blocklist ping — deliberately not
  required. A device this old has no useful relationship with AMO, and the update timer is a
  background cost. If unsigned local installs need `xpinstall.signatures.required=false`, that is an
  implementation detail of R3, recorded there.
- Any change to the YAP contract. Add-ons are engine-side; the adapter contract stays frozen.

## Cross-References
- See also: cavekit-engine-embedding.md (R1 registers into the embedded runtime),
  cavekit-browser-services.md (R2 profile/persistence, R4 the download/MIME path an `.xpi` arrives
  through), cavekit-device-build.md (R7 per-variant independence, R8 storage footprint),
  cavekit-input-bridging.md (the XUL input hazard in R2),
  cavekit-licensing-branding.md (R3 — identity must not reintroduce stripped branding).

## Changelog
- 2026-08-01: Initial draft. Created when the user required `about:addons` + XPI support to work,
  immediately after the on-device diagnosis showed the daemon's SIGSEGV was the missing
  `nsIXULAppInfo` that the add-on manager depends on — making the crash fix and the feature the same
  piece of work. Verified while drafting that the entire add-ons UI, the XPI install UI and the
  `about:addons` redirector already ship in the device bundle.
