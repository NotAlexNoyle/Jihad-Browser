---
created: "2026-08-01"
last_edited: "2026-08-01"
---

# Cavekit: Add-ons, Extensions & Plugins

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
- [x] An `nsIXULAppInfo` implementation is registered BEFORE `XRE_NotifyProfile()`, exposing at minimum `vendor`, `name`, `ID`, `version`, `appBuildID`, `platformVersion`, `platformBuildID`. *(2026-08-01: `JihadAppInfo` in `render/goanna/EngineHost.cpp` implements `nsIXULAppInfo` + `nsIPlatformInfo` + `nsIXULRuntime`. It is registered far earlier than "before XRE_NotifyProfile" — see the criterion below for why that was not sufficient.)*
- [x] The `nsIXULRuntime` members needed by the add-on stack (`OS`, `XPCOMABI`, `widgetToolkit`, `processType`, `inSafeMode`) are answered rather than throwing. *(2026-08-01: the full `NS_DECL_NSIXULRUNTIME` surface is implemented. `XPCOMABI` reports the engine build's real `TARGET_XPCOM_ABI` — `arm-eabi-gcc3` on device — because it gates XPIs carrying binary components; `widgetToolkit` reports `headless`, matching `--enable-default-toolkit=cairo-headless`.)*
- [x] The application **ID** is stable and documented — extensions declare `targetApplication` against it, so changing it later invalidates every installed extension's compatibility. *(2026-08-01: `JIHAD_APP_ID = {4534aac8-d8c8-4765-95ee-7f61fd0b762d}`, minted once and marked FROZEN in `JihadUserAgent.h` with the compatibility consequence spelled out. **USER SIGN-OFF GIVEN 2026-08-01 ("keep the app id") — this ID and `version = 1.0` are now a compatibility contract. Do not change either without treating it as a breaking change that invalidates every installed extension.**)*
- [x] Identity values come from ONE source shared with the User-Agent string (`render/goanna/JihadUserAgent.h`), not a second hard-coded copy. *(2026-08-01: the UA string is now COMPOSED from `JIHAD_APP_NAME`/`JIHAD_APP_VERSION`, so the product token and `appinfo.version` cannot disagree. The composed value is byte-identical to the previously verified UA.)*
- [x] No Pale Moon / Basilisk / Moonchild identity is reintroduced (cavekit-licensing-branding.md R3 strips those; this is Jihad's own identity). *(2026-08-01: vendor "Jihad Browser project", name "JihadBrowser"; `isOfficialBranding`/`isOfficial` report false. Verified by reading the identity strings back out of the ARM binary.)*
- [x] **Registration must beat the JS lazy getter, not merely `XRE_NotifyProfile`.** *(2026-08-01, the hard-won part. Re-pointing the CONTRACT id is NOT enough: `Cc["@mozilla.org/xre/app-info;1"]` builds an `nsJSCID`, and `nsJSCID::NewID` (js/xpconnect/src/XPCJSID.cpp) resolves a contract string to a CID **eagerly** and pins it, so JS never consults the contract table again; and `Services.jsm` caches `Services.appinfo` on first access forever. Measured: with only the contract remapped, `do_GetService` returned our object at every probe point while `AddonManager` still read `undefined` and our `GetVersion()` was never called from JS at all. The fix is (a) take over the STOCK `APPINFO_CID` `{95d89e3e-…}` — unregistering the stock factory first, since the component manager rejects a duplicate CID — and (b) register from the FIRST hook that runs inside `NS_InitXPCOM2`, the directory-service provider's `GetFile`, retried on each call because the earliest calls precede the component manager itself (`NS_ERROR_NOT_INITIALIZED`). The explicit call in `Init()` remains a backstop.)*
- [x] **The device SIGSEGV in `XRE_NotifyProfile()` → `DoStartup()` is gone**, with `XRE_NotifyProfile` ENABLED (the `JIHAD_NO_PROFILE_NOTIFY` diagnostic switch off), and `AddonManager.jsm` no longer logs `NS_ERROR_ILLEGAL_VALUE`. *(2026-08-01 — VERIFIED ON DEVICE. Both halves fixed, and they were two SEPARATE defects: (a) the missing `nsIXULAppInfo` (this requirement's other criteria) removed the `NS_ERROR_ILLEGAL_VALUE` — error count 1-per-run → 0; (b) the crash itself was **`$HOME` unset**. UXP dereferences `PR_GetEnv("HOME")` with NO null check — `SpecialSystemDirectory.cpp:189` and `nsAppFileLocationProvider.cpp:318` both do `nsDependentCString(PR_GetEnv("HOME"))` → `strlen(NULL)` → SIGSEGV at address 0. Device-only because an **upstart job inherits init's environment, which has no `HOME`**, while the desktop container harness always passes `-e HOME=/out` — which is exactly why this reproduced 100% on hardware and never once on desktop. Confirmed by the on-device fatal report (`faultaddr=0x00000000`, `lr` → `nsDependentCString(char const*)` at `nsTDependentString.h:53`, `pc` outside every libxul segment i.e. inside libc `strlen`), then reproduced ON THE HOST with `env -u HOME` (pre-fix binary dies at the identical breadcrumb; fixed binary reaches `engine up`). Fixed in `Main.cpp` via `setenv("HOME", <state dir>, 0)` — never overwriting a deliberate value — plus the upstart job's `exec env` line so the requirement is visible where the environment is defined. Device result: `HOME was unset — set to /var/palm/jihad/mochi`, `XRE_NotifyProfile returned`, `engine up; serving YAP 'jihad-browser-mochi'`, socket bound in 2s, process stable. The `gAppData` hypothesis was investigated and CLEANLY EXCLUDED — both paths that could produce this early-return on null.)*
- [ ] Two measurement traps, recorded because both produced FALSE PASSES during this work and will do so again: (a) the shell prints `Segmentation fault` only for FOREGROUND jobs, so its absence in a backgrounded run is not evidence of survival — poll with `wait`/`/proc/<pid>` instead, and note `kill -0` succeeds on a ZOMBIE; (b) a unix socket file OUTLIVES the process that bound it, so the presence of `/tmp/yapserver.<name>` proves nothing — `rm -f` it before the run and poll for it to APPEAR.

**Dependencies:** cavekit-engine-embedding.md (R2)

### R2: `about:addons` opens and is usable
**Description:** The user can reach the add-ons manager from the browser and operate it.
**Acceptance Criteria:**
- [x] Navigating to `about:addons` renders the add-ons manager rather than an error or a blank card.
      **Met on device 2026-08-02** — `title=[Add-ons Manager]`, and the fb1 capture shows the
      Extensions/Themes/Plugins category list and the search field. It had been failing on a
      missing `chrome://branding/` package (a DTD load failure = hard XML parse error), fixed by
      shipping our own branding package; see context/impl/impl-addons-branding.md.
- [ ] Installed extensions, themes and plugins are listed with name, version and state.
- [ ] Enable / disable / remove work from the UI and the change is reflected after a restart.
- [ ] The page is **operable through the adapter's synthesized input path**, not merely rendered. `about:addons` is a XUL document, and XUL input currently crashes the daemon (mitigated by a skip, so the page is inert). **The user has required this be fixed properly (2026-08-01, "fix xul input so it works everywhere like about:config etc") — it is now cavekit-input-bridging.md R6**, and this criterion is satisfied by that work rather than duplicating it.
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

### R7: NPAPI plugins load and run
**Description:** Binary NPAPI plugins work, not just XPI extensions. User requirement, 2026-08-01: *"ensure support for plugins not just extensions. for instance adobe flash and microsoft silverlight plugins which work on other uxp browsers."*

**Ground truth established 2026-08-01, before any work:**
- UXP **has the full NPAPI subsystem** — `dom/plugins/base/` (`nsNPAPIPlugin`, `nsNPAPIPluginInstance`, `nsPluginInstanceOwner`, `nsPluginHost`), including windowless rendering paths. Nothing needs porting in; it needs enabling and wiring to our offscreen surface.
- Our ARM build passes **`--disable-npapi-gtk2`** (`build/webos-oe/mozconfig.goanna-arm:20`). That disables GTK2 **windowed** (XEmbed) plugin support only — `MOZ_ENABLE_NPAPI` is a separate switch (`old-configure.in:2149-2167`), and the GTK2 flag *must* stay off for us: the build is `cairo-headless`, and old-configure hard-errors if it is enabled on a GTK2 build. **Therefore windowless mode is the only viable path** — the plugin draws into a bitmap we own and we composite it, which is exactly the model our offscreen renderer already uses.
- **A real ARM NPAPI Flash exists on this device**: `/usr/palm/ipkgs/com.palm.app.flashplugin-001_1.0.6_all.ipk` → `libflashplayer.so`, 8.8 MB, ELF 32-bit ARM EABI5, exporting all four entry points (`NP_Initialize`, `NP_Shutdown`, `NP_GetMIMEDescription`, `NP_GetValue`).
- **But that Flash build is not a generic plugin.** Its `NEEDED` list includes `libWebKitLuna.so`, `libPiranha.so`, `libLunaSysMgrIpc.so`, `libnapp.so`, `libhal.so`, `libpowerd.so.0`, `liblunaservice.so` — it is linked against webOS's own WebKit host and compositor, and against OpenSSL 0.9.8. Whether it functions inside Goanna rather than LunaSysMgr's WebKit is a **genuine open question**, not a formality.

**Acceptance Criteria:**
- [ ] The engine build enables NPAPI (`MOZ_ENABLE_NPAPI`) while keeping `--disable-npapi-gtk2`, and the daemon exposes a documented plugin search path that is variant-scoped and honours the R8 storage contract.
- [~] `about:plugins` lists installed plugins with name, version, MIME types and enabled state.
      **Page verified working on device 2026-08-02** (`title=[About Plugins]`, renders "No
      installed plugins found") — `nsPluginHost` initialises and scans, so the subsystem is
      live. Listing CONTENT is unmet only because the search path is still empty. Also
      confirmed the engine already has NPAPI compiled in (NP_Initialize/NP_Shutdown/
      NP_GetMIMEDescription/NP_GetValue present in the shipped ARM libxul), so the first AC
      needs no build change. See context/impl/impl-npapi-groundtruth.md.
- [ ] A plugin instantiates in **windowless mode** and its output composites correctly into the offscreen surface — correct position, size, and orientation under the rotation/zoom paths that cavekit-offscreen-rendering.md R5/R6 already fixed for page content.
- [ ] Plugin input works: mouse and keyboard events reach a windowless plugin instance, mapped through the same coordinate transform as page content (cavekit-input-bridging.md R5).
- [ ] `setEnableFlashPlugin` / plugin-enable settings in the existing frozen YAP surface behave correctly (the contract already carries plugin-related commands — they must not become no-ops).
- [ ] A plugin crash does not take the daemon with it, or the failure mode is documented and bounded. (UXP supports out-of-process plugins; whether OOP is viable on this device is part of this requirement to determine, not assume.)
- [ ] **Adobe Flash on the TouchPad is attempted against the device's own `libflashplayer.so`, and the outcome is recorded either way** — including, if it fails, exactly which of its webOS-WebKit dependencies (`libWebKitLuna`/`libPiranha`/`libLunaSysMgrIpc`) is unsatisfiable under Goanna. A negative result documented at that level of detail satisfies this criterion; an untested claim does not.
- [ ] Platform reality is stated honestly in the docs rather than implied: **Silverlight has no ARM Linux build** (it shipped x86/x64 for Windows/macOS; the Linux implementation was Moonlight, x86-only and long dead), so it is **not achievable on the TouchPad** regardless of our plugin support. The requirement is that the NPAPI *architecture* is generic enough that any platform-appropriate NPAPI binary works — verifiable on the desktop x86_64 build, where such plugins exist.
**Dependencies:** cavekit-offscreen-rendering.md (R2, R5, R6), cavekit-input-bridging.md (R5), cavekit-device-build.md (R8)

### R8: The XPI implementation is validated against Pale Moon and Basilisk
**Description:** Extension support is compared against the two UXP browsers that ship it correctly, rather than reinvented. User requirement, 2026-08-01: *"when you do your adversarial review for xpi extension support make sure you compare it to how basilisk and pale moon do it correctly."*
**Acceptance Criteria:**
- [ ] The adversarial review of R1–R6 explicitly diffs our approach against **Pale Moon** and **Basilisk** — both are checked out at `../ref-forks/{Pale-Moon,Basilisk}` and both ship the same UXP-era XPI stack we are targeting, so they are the authoritative reference for what "correct" means here.
- [ ] Specifically compared, with findings recorded per item: how they populate application identity (`nsIXULAppInfo`, and whether they rely on `gAppData` from `application.ini` — the exact thing our embedding lacks); their extension install locations and `extensions.enabledScopes` defaults; their signature policy (`xpinstall.signatures.required`) and why; how `about:addons` is registered and reached; and their NPAPI plugin enablement and windowless handling for R7.
- [ ] Every place we deliberately DIVERGE from both is justified in writing (embedding vs. full XRE app, headless vs. real widget, three independent variants vs. one browser) — divergence is expected, unexplained divergence is a defect.
- [ ] Anything they do that we simply MISSED is filed as a finding, not silently absorbed.
**Dependencies:** R1, R7

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
