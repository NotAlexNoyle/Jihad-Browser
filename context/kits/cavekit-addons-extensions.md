---
created: "2026-08-01"
last_edited: "2026-10-06"
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

The daemon embeds the engine through `XRE_InitEmbedding2`, which has no `application.ini`, so the runtime
had **no `nsIXULAppInfo`**: `AddonManager.jsm` (`:770-781`) then wrote `Services.appinfo.version` as
`undefined` into a pref (`NS_ERROR_ILLEGAL_VALUE`). The device SIGSEGV in the same window
(`XRE_NotifyProfile()` → `DoStartup()`, 117 upstart respawns, exit 139) turned out to be a second defect,
`$HOME` unset (R1, last criterion). Skipping `XRE_NotifyProfile` is NOT an acceptable workaround: it
enables the profile and with it cookie persistence (cavekit-browser-services R2).

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
- [x] An `nsIXULAppInfo` implementation is registered BEFORE `XRE_NotifyProfile()`, exposing at minimum `vendor`, `name`, `ID`, `version`, `appBuildID`, `platformVersion`, `platformBuildID`. *(2026-08-01: `JihadAppInfo` in `render/goanna/EngineHost.cpp` implements `nsIXULAppInfo` + `nsIPlatformInfo` + `nsIXULRuntime`; it registers far earlier than that, criterion 6.)*
- [x] The `nsIXULRuntime` members needed by the add-on stack (`OS`, `XPCOMABI`, `widgetToolkit`, `processType`, `inSafeMode`) are answered rather than throwing. *(2026-08-01: the full `NS_DECL_NSIXULRUNTIME` surface is implemented. `XPCOMABI` reports the engine build's real `TARGET_XPCOM_ABI` — `arm-eabi-gcc3` on device — because it gates XPIs carrying binary components; `widgetToolkit` reports `headless`, matching `--enable-default-toolkit=cairo-headless`.)*
- [x] The application **ID** is stable and documented — extensions declare `targetApplication` against it, so changing it later invalidates every installed extension's compatibility. *(2026-08-01: `JIHAD_APP_ID = {4534aac8-d8c8-4765-95ee-7f61fd0b762d}`, minted once and marked FROZEN in `JihadUserAgent.h` with the compatibility consequence spelled out. **USER SIGN-OFF GIVEN 2026-08-01 ("keep the app id") — this ID and `version = 1.0` are now a compatibility contract. Do not change either without treating it as a breaking change that invalidates every installed extension.**)*
- [x] Identity values come from ONE source shared with the User-Agent string (`render/goanna/JihadUserAgent.h`), not a second hard-coded copy. *(2026-08-01: the UA string is now COMPOSED from `JIHAD_APP_NAME`/`JIHAD_APP_VERSION`, so the product token and `appinfo.version` cannot disagree. The composed value is byte-identical to the previously verified UA.)*
- [x] No Pale Moon / Basilisk / Moonchild identity is reintroduced (cavekit-licensing-branding.md R3 strips those; this is Jihad's own identity). *(2026-08-01: vendor "Jihad Browser project", name "JihadBrowser"; `isOfficialBranding`/`isOfficial` report false. Verified by reading the identity strings back out of the ARM binary.)*
- [x] **Registration must beat the JS lazy getter, not merely `XRE_NotifyProfile`.** *(2026-08-01: re-pointing the contract id is not enough. `nsJSCID::NewID` (js/xpconnect/src/XPCJSID.cpp) resolves a contract string to a CID eagerly and pins it, and `Services.jsm` caches `Services.appinfo` on first access. So the daemon (a) takes over the stock `APPINFO_CID` `{95d89e3e-…}`, unregistering the stock factory first (the component manager rejects a duplicate CID), and (b) registers from the first hook inside `NS_InitXPCOM2`, the directory-service provider's `GetFile`, retried on each call until the component manager exists (`NS_ERROR_NOT_INITIALIZED`); the call in `Init()` is a backstop.)*
- [x] **The device SIGSEGV in `XRE_NotifyProfile()` → `DoStartup()` is gone**, with `XRE_NotifyProfile` ENABLED (the `JIHAD_NO_PROFILE_NOTIFY` diagnostic switch off), and `AddonManager.jsm` no longer logs `NS_ERROR_ILLEGAL_VALUE`. *(2026-08-01, device-verified. Two separate defects: the missing `nsIXULAppInfo` caused the `NS_ERROR_ILLEGAL_VALUE` (1 per run → 0); the crash was `$HOME` unset. UXP does `nsDependentCString(PR_GetEnv("HOME"))` with no null check (`SpecialSystemDirectory.cpp:189`, `nsAppFileLocationProvider.cpp:318`), and an upstart job inherits init's environment, which has no `HOME` (the desktop harness always passed one). Fixed in `Main.cpp` (`setenv("HOME", <state dir>, 0)`, never overwriting a set value) plus the upstart job's `exec env` line. Device: `HOME was unset — set to /var/palm/jihad/mochi`, `XRE_NotifyProfile returned`, `engine up; serving YAP 'jihad-browser-mochi'`, socket bound in 2 s.)*

**Two measurement traps** (both gave false passes; prose, not a product property): the shell prints
`Segmentation fault` only for foreground jobs, so poll `/proc/<pid>` (`kill -0` succeeds on a zombie); and a
unix socket file outlives the process that bound it, so `rm -f /tmp/yapserver.<name>` before a run and poll
for it to appear.

**Dependencies:** cavekit-engine-embedding.md (R2)

### R2: `about:addons` opens and is usable
**Description:** The user can reach the add-ons manager from the browser and operate it.
**Acceptance Criteria:**
- [x] Navigating to `about:addons` renders the add-ons manager rather than an error or a blank card.
      **Met on device 2026-08-02** — `title=[Add-ons Manager]`, and the fb1 capture shows the
      Extensions/Themes/Plugins category list and the search field. It had been failing on a
      missing `chrome://branding/` package (a DTD load failure = hard XML parse error), fixed by
      shipping our own branding package; see context/impl/impl-addons-branding.md.
- [x] Installed extensions, themes and plugins are listed with name, version and state. *(2026-08-04: the Extensions list renders both installed add-ons with name, version, the enabled/disabled dot and a "(disabled)" label, plus working Disable/Enable and Remove buttons; Themes and Plugins categories are present in the sidebar.)*
- [x] Enable / disable / remove work from the UI and the change is reflected after a restart. *(2026-08-04: all three verbs by synthesized clicks on the row's XBL `enable-btn`/`disable-btn`/`remove-btn`, the daemon stopped by SIGTERM (what upstart sends) between phases: disabled → its `startup()` does not run after the restart; re-enabled → it does; removed → the XPI is gone from the profile (removal commits on the next start). Reaching XBL anonymous content needed the inject channel's `sel:<css>` and `anon:<css>|<anonid>` forms: `jsurl` does not run in a chrome document at all (`LoadURIWithOptions` returns NS_OK and nothing runs).)*
- [x] **The daemon shuts down cleanly on SIGTERM, and engine state is flushed.** *(2026-08-04: without a SIGTERM handler `stop <job>` killed the daemon outright, `XRE_TermEmbedding` never ran and the deferred savers (`XPIDatabase`, prefs) never flushed, so a disabled add-on came back enabled. `Main.cpp` now catches SIGTERM/SIGINT/SIGHUP, sets a flag, and the 16 ms tick quits the GLib loop (`g_main_loop_quit` is not async-signal-safe). Also fixed: `~YapServer` deleted the never-initialised `deadlockDetector`, a SIGSEGV on every clean exit.)*
- [x] The page is **operable through the adapter's synthesized input path**, not merely rendered. *(2026-08-04: its buttons activate and its list responds. The XUL input work is cavekit-input-bridging.md R6 (the owner, 2026-08-01: "fix xul input so it works everywhere like about:config etc"), whose XUL criteria closed on the same runs.)*
- [x] The page's **tools menu opens, is readable, dismissible and operable**. It is a XUL `<menupopup>` — a separate display root the offscreen capture never contains — now composited over the frame and given its own input routing by **cavekit-offscreen-rendering.md R7**. Desktop-verified through a real state change (toggling "Update Add-ons Automatically" flips the menu's own checkmark and last item); device-verified as far as opening and compositing.
**Dependencies:** R1, cavekit-input-bridging.md, cavekit-offscreen-rendering.md (R7)

### R3: XPI installation works
**Description:** A user can install an extension from a file or a URL.
**Acceptance Criteria:**
- [x] Navigating to / opening an `.xpi` triggers the install flow rather than a download or a MIME handoff. *(2026-10-01, CE 3.1.0: navigating to an `.xpi` (typed, or a plain link tapped) did nothing until `amContentHandler.js` was fixed on `jihad-engine-mods` (the handler cancelled the load and failed to hand the install on); now `url http://…/jihad-effect.xpi` and a tapped link each raise `dialog confirm -> card` within a second. 2026-08-03: `InstallTrigger.install()` runs end to end on desktop and device after patch 0013 (`amInstallTrigger` required a chrome `<browser>` or a content message manager, and `installAddonsFromWebpage` dereferenced its optional `aBrowser`).)*
- [x] The install prompt identifies the extension and can be accepted or declined; declining installs nothing. *(Both answers verified 2026-08-03. Declining tells the page `USER_CANCELLED` and installs nothing; accepting completes the install — the daemon's new `DialogSink` (cavekit-browser-services.md R3) carries the question to the card and the answer back.)*
- [x] After accepting, the extension appears in `about:addons` and is active (subject to a restart if it is not bootstrapped). *(2026-08-03, desktop and device: the page's callback reports status 0, the XPI is unpacked into the profile's `extensions/`, about:addons lists "Jihad Test Add-on 1.0"; on the TouchPad `profile/extensions/jihad-test-addon@riverstonerelay.net.xpi`.)*
- [x] An extension whose `targetApplication` does not match this app's ID/version is rejected with a clear reason rather than installed and silently inert. *(2026-10-01, CE 3.1.0: `jihad-t103-mismatch.xpi` offered through `InstallTrigger.install()` on the Enyo card: the page got `status=-210`, the card showed "Jihad T103 Mismatch could not be installed because it is not compatible with Jihad Browser 1.0." as a toast (`../impl/evidence-2026-10-01/xpi-mismatch-toast_prompt_installed-toast_addons-list.jpg`), no install prompt, `profile/extensions/` empty.)*
  *(T-103, 2026-08-10 and 08-15: the page-facing status stays `-210` (`USER_CANCELLED`) by decision. This UXP
  has no `ERROR_INCOMPATIBLE`, and `amWebInstallListener.js` cancels the install before it notifies, so the
  page cannot tell "incompatible" from "declined", as in Firefox. The clear reason is the user-facing alert:
  `render/goanna/components/jihadInstallPrompt.js` observes `addon-install-failed` for `appDisabled` installs and raises a
  card alert through `JihadPrompter::Alert`; `DialogService.cpp` instantiates it at daemon start; the
  classification has to be in JS. Desktop harness `render/goanna/test/xpi_mismatch_test.cpp` +
  `build/desktop/build-xpi-mismatch-test.sh`: 6 checks pass, and the matching add-on declined gives 0 alerts.
  A missing `extensions.update.url` used to kill every install at its update check; both URL prefs ship in
  `packaging/prefs/jihad-addon-prefs.js`. Record: `../impl/impl-addons-appinfo.md`.)*
**Dependencies:** R1, R2, cavekit-browser-services.md (R4)

### R4: An installed extension actually affects browsing
**Description:** Extensions are functional, not just listed.
**Acceptance Criteria:**
- [x] A test extension that observably alters page behavior (e.g. blocks a request, injects a style, or rewrites content) demonstrably does so on a real page. *(2026-08-04: a bootstrapped extension registering a USER stylesheet, installed through the web-install flow. Judged from the PIXELS, not a log line — it paints page backgrounds a colour nothing else here uses, and the rendered frame carries 693,224 of them.)*
- [x] Disabling it stops the effect; re-enabling restores it. *(Both driven through the REAL `about:addons` UI — clicking its Disable/Enable buttons, which also exercises the XUL input path. The extension's `shutdown()`/`startup()` run on each click and the page rendering follows exactly: 693,224 → 0 → 693,224 styled pixels. NB the list re-sorts enabled-first on every toggle, so a test must locate the button in the CURRENT frame rather than reuse a coordinate.)*
**Dependencies:** R3

### R5: Extensions are per-variant and survive restart
**Description:** Extension state persists, and — per cavekit-device-build.md R7 — belongs to exactly one variant.
**Acceptance Criteria:**
- [x] Installed extensions survive a daemon restart and a device reboot. *(2026-08-04: after a real cold boot (`/sbin/reboot`, uptime 1033 s) both XPIs are in the profile, `extensions.json` keeps them enabled, the enabled add-on's `startup()` ran, `cookies.sqlite` is intact, and all three variants' daemons auto-started. 2026-10-02, CE 3.1.0: the extension in Enyo and Mochi survives a reboot with its effect: `../impl/evidence-2026-10-02/after-reboot_extension-still-active_enyo_mochi.jpg`.)*
- [x] Each variant's extensions live in ITS OWN profile (`$APP/profile/extensions`), so installing an extension in one variant does not appear in, or affect, another. *(DEVICE-VERIFIED 2026-08-04: the add-on installed in the Enyo variant is in Enyo's profile alone — Mochi and Mojo both report zero extensions.)*
- [x] Removing a variant removes its extensions with it and leaves the other variants' untouched. *(2026-08-05: an extension seeded into Mojo's profile, then Mojo removed through `org.webosinternals.ipkgservice`: `prerm` ran and every Mojo artifact went (app dir, profile with its extension, cache, upstart job, adapter shim and impl, state dir, Luna role file, D-Bus service file, script dir, ipkg records), while Enyo's two extensions and Mochi's one stayed and both daemons kept running.)*
**Dependencies:** R1, cavekit-device-build.md (R7, R8)

### R6: Extension storage respects the install-footprint contract
**Description:** Add-on data lands where the rest of the engine's state does.
**Acceptance Criteria:**
- [x] Extensions and their data live under the variant's profile on cryptofs — never on `/media/internal` (the user's volume) and never on the 62 MB `/var` partition. *(2026-08-04, on device: the XPI installed by the run below landed at `$APP/profile/extensions/jihad-effect@riverstonerelay.net.xpi` on cryptofs, alongside the one already there.)*
- [x] Extension install/removal writes nothing outside the variant's own profile. *(2026-08-04: a real on-device install between two `device-citizen-audit.sh` snapshots: `/media/internal` byte-identical, `/usr/lib/jihad` unchanged, nothing new under any profile or cache; the only new paths were the daemons' own `jihad-menu.css` in their state dirs. The snapshot lists profile directories only, so the `.xpi` itself is shown by `ls`.)*
**Dependencies:** cavekit-device-build.md (R8)

### R7: NPAPI plugins load and run
**Finishing Flash has its own kit since 2026-11-05: `cavekit-flash.md` (real games, apps and videos by autonomous finger tests). It takes over this requirement's two `[~]` criteria below; this section stays as the record of how Flash came to run.**

**Description:** Binary NPAPI plugins work, not just XPI extensions. User requirement, 2026-08-01: *"ensure support for plugins not just extensions. for instance adobe flash and microsoft silverlight plugins which work on other uxp browsers."*

**How Flash runs here** (current. How it came to run, with every measurement, is in
`archive/addons-extensions-history.md`; the traps are in `docs/PICKUP.md`.)
- UXP has the full NPAPI subsystem. The ARM build keeps `--disable-npapi-gtk2` (GTK2 windowed plugins cannot
  be built on `cairo-headless`), so plugins run windowless, and out of process: in-process plugins cannot
  render in UXP (`nsPluginFrame::PaintPlugin` is an empty stub, `PluginPRLibrary::GetImageContainer` is not
  implemented). `dom.ipc.plugins.enabled` and `dom.ipc.plugins.asyncdrawing.enabled` are true, and
  `gfxPlatformHeadless` supports the async bitmap drawing model.
- `plugin-container` ships as the real ELF and is started through the bundled `ld-2.23.so`
  (`MOZ_CHILD_PROCESS_LOADER`, patch 0018): a `/bin/sh` wrapper segfaults on the inherited `LD_LIBRARY_PATH`.
- Engine patches: 0016 (`NPNVSupportsWindowless` on headless), 0017 (the webOS Palm plugin host: NpPalmWindow,
  the gain-focus event, the glib pump and repaint clock, Piranha PGContext, NPN interposition), 0019 (the
  child's BackgroundHangMonitor), 0020 (compositing without an ImageBridge), 0021 (a windowless invalidate
  reaches a real widget), 0022 (`RecvShow` copies into an opaque frame with a frame id), 0024 (input), 0025
  (the PmLogLib semaphore), 0026-0029 (repaint rate, CPU boost, frame delivery).
- The device's Flash (`libflashplayer.so` 10.3.185.65, installed by HP's Flash plugin app) is not a generic
  NPAPI plugin: it imports no X11, binds `NPN_CreateObject`/`NPN_InvalidateRect` out of `libWebKitLuna.so`
  and draws through Piranha. `msgAddFlashRects` is input arbitration (the adapter's gesture lock), not hole
  punching.
- Load-bearing: the `com.palm.flashgraphics` LS2 role name (`packaging/gen-variant-scripts.sh`); the
  `PmLogLib` named semaphore interposed by `render/goanna/JihadPmLogSem.c` (webOS's glibc 2.8 and the bundled
  2.23 lay `sem_t` out differently and the first process to create it each boot wins: a boot race, not a
  code regression; cavekit-device-build.md R9); the navigation scroll reset
  (`BrowserPageGoanna::resetScrollForNavigation()`, `msgScrolledTo(0,0)`); an embed with the default
  `wmode=window` made windowless (engine `0048d50fa9`, Changelog 2026-10-06).
- Never write `scaling_governor`: a governor switch deadlocks cpufreq and only `sync; reboot -f` clears it.
  The CPU boost writes only `ondemandtcl` tunables.
- The control plugin `render/goanna/test/npapi_test_plugin.c` tells our defects from Flash's; Flash and it
  are indistinguishable up to instantiation, so do not re-derive a port defect from Flash.

**Acceptance Criteria:**
- [x] The engine build enables NPAPI (`MOZ_ENABLE_NPAPI`) while keeping `--disable-npapi-gtk2`, and the daemon exposes a documented plugin search path that is variant-scoped and honours the R8 storage contract. *(2026-08-04: the shipped `libxul.so` carries the `NP_*` symbols. `EngineHost::Init` sets `MOZ_PLUGIN_PATH` before XRE starts to `<profile>/plugins:<greDir>/plugins` (the device's own folder joined it 2026-10-02, this requirement's last-but-one criterion), both inside the variant's footprint, and creates the profile folder at start, since an absent folder is skipped silently. Deliberately NOT `/usr/lib/BrowserPlugins`, the stock browser's folder. Device: the daemon logs the path, and a plugin dropped there is found.)*
- [x] `about:plugins` lists installed plugins with name, version, MIME types and enabled state. *(2026-08-04, device, read back with the inject channel's `gettext`: "Adobe Flash", `libflashplayer.so` in the variant's `profile/plugins/`, Enabled, "Shockwave Flash 10.3 d185", `application/x-shockwave-flash` (swf) and `application/futuresplash` (spl). The `Version:` field is empty: this plugin puts its version in the description only.)*
- [x] A plugin instantiates in **windowless mode** and its output composites correctly into the offscreen surface — correct position, size, and orientation under the rotation/zoom paths that cavekit-offscreen-rendering.md R5/R6 already fixed for page content. *(2026-08-07, on the device's screen, supervised upstart daemon: the control plugin instantiates windowless (`NPNVSupportsWindowless=1`, `NPDrawingModelAsyncBitmapSurface`, `NPP_SetWindow 320x240 type=2`) and composites at the card's fit zoom: 320x240 CSS px land as 250x187 device px (×0.7837), directly under the `<h1>`, checked by the dumped `frame.ppm`, a screenshot and the layer trace. Rotation was not separately exercised. Trap: a stale hand-launched daemon once faked "composites nothing"; `pkill -f jihad-browserserver` before believing a negative.)*
- [x] Plugin input works: mouse and keyboard events reach a windowless plugin instance, mapped through the same coordinate transform as page content (cavekit-input-bridging.md R5). *(2026-08-09, device, both plugins, patch 0024. `nsPluginInstanceOwner::ProcessEvent` had no headless arm, so a windowless plugin received nothing, and `NPEvent` was a `void*` copied raw across IPC. Now `NPEvent` is Palm's 48-byte union (offsets `static_assert`ed against libWebKitLuna) and a headless arm maps mouse to `npPalmPen{Down,Up,Move,Click,DoubleClick}`, keys to `npPalmKey{Down,Up,Press}`, focus and blur to `npPalmSystemEvent`. The arm adds no zoom arithmetic of its own: a tap at the centre of a 320x240 plugin arrives as 160,120 at zoom 1.0 and 159,119 at 0.7837; Flash logs `palm event 0x1 at 160,120` and `palm key 0x8 raw=65 handled=1`. Flash ignores `npPalmPenClickEvent`/`npPalmPenDoubleClickEvent`; they are sent anyway, as Palm's own host sends them.)*
- [x] Plugin-related commands in the existing frozen YAP surface behave correctly (they must not stay no-ops). *(2026-08-09, device, all four. Outbound: `GoannaRenderPage::CollectPluginRects` and `BrowserPageGoanna::syncPluginRects` emit `msgAddFlashRects`/`msgRemoveFlashRects` byte-identical to `BrowserPage::addInteractiveWidgetRect` (`flashRect add id=1 0,79..320,319`), in unzoomed document CSS px by the adapter's own formula, scroll-invariant. Inbound: `asyncCmdPluginSpotlightStart/End` are queued (the F-9 rule) into `jihad_plugin_palm_spotlight`, but since 2026-08-10 delivery is SUPPRESSED unless `JIHAD_PLUGIN_SPOTLIGHT` is set. Do not "fix" it back: Flash treats the spotlight event as full screen and its process faults (sig 7), and the Mojo WebView's hold-over-a-plugin handler would send it once `asyncCmdGetElementInfoAtPoint` is implemented. Stock never delivered it either (`#ifdef FIXME_QT`). Device: `spotlight` start and end both log `-> SUPPRESSED`, the plugin-container pid unchanged, ~31 fps after. The contract carries no plugin-enable command; enabling is about:plugins and the add-on manager.)*
- [x] A plugin crash does not take the daemon with it, or the failure mode is documented and bounded. (UXP supports out-of-process plugins; whether OOP is viable on this device is part of this requirement to determine, not assume.) *(2026-08-09, provoked: `kill -SEGV` to plugin-container with Flash drawing; the daemon's pid and start time unchanged, it kept painting, and a reload drew again (46,750 magenta px, as before the crash). Plugin children crashed repeatedly during the 2026-08-07 work with the daemon up every time. Outer bound: upstart `respawn`/`respawn limit 10 5` per variant, and nothing crosses variants. In-process plugins, the configuration of 2026-08-04, cannot render in UXP at all.)*
- [x] **Adobe Flash on the TouchPad is attempted against the device's own `libflashplayer.so`, and the outcome is recorded either way** — including, if it fails, exactly which of its webOS-WebKit dependencies (`libWebKitLuna`/`libPiranha`/`libLunaSysMgrIpc`) is unsatisfiable under Goanna. A negative result documented at that level of detail satisfies this criterion; an untested claim does not. *(2026-08-04: every dependency of the device's `libflashplayer.so` (`/media/cryptofs/apps/usr/lib/BrowserServerPlugins/`, 8.8 MB) resolves under the bundled loader, `libWebKitLuna.so`, `libPiranha.so` and `libLunaSysMgrIpc.so` included; the bundled glibc-2.23 set plus the device's OpenSSL 0.9.8, libcurl, liblunaservice, libhal, libnapp and libpowerd. 2026-08-07: it instantiates (`NP_Initialize rv=0x0` → `SetUpPluginInstance rv=0x0`). 2026-08-09: it draws (patches 0021, 0022): the magenta test SWF as an exact 250x187 rect at zoom 0.784. How it differs from a generic plugin: "How Flash runs here" above.)*
- [x] Platform reality is stated honestly in the docs rather than implied: **Silverlight has no ARM Linux build** (it shipped x86/x64 for Windows/macOS; the Linux implementation was Moonlight, x86-only and long dead), so it is **not achievable on the TouchPad** regardless of our plugin support. The requirement is that the NPAPI *architecture* is generic enough that any platform-appropriate NPAPI binary works — verifiable on the desktop x86_64 build, where such plugins exist. *(2026-08-07 and 08-09: `render/goanna/test/npapi_test_plugin.c`, built for ARM by `build/webos-oe/build-test-plugin-arm.sh` (9.5 KB, libc only) and for x86_64 by `build/desktop/build-test-plugin.sh` (both assert the four entry points are exported), loads, instantiates windowless, paints, composites and responds to input on the device and the desktop: it paints a marker at the pen point it was handed and turns cyan on a key. `-DXP_UNIX=1` is needed for `NP_EXPORT`.)*
- [~] **Flash content holds a smooth, sustained frame rate on the card — the COMPOSITE rate, not the
      plugin's draw rate.** The number the user perceives is the card composite; a plugin drawing 30
      fps into a card compositing 20 is 20 fps of animation, and a card compositing 29 against a
      plugin drawing 24 wastes a fifth of its paints. They must be matched and both at the content's
      authored rate. **This criterion is met when the gap histogram concentrates**, not when fps looks
      right: a clear mode at the authored period with the `<16 ms` and `40+ ms` buckets near empty, on a
      clean-restart run of `jihad-anim.swf`. Record the histogram, not min/avg/max.
      *(Moved to cavekit-flash.md (R3, R6, R14) on 2026-11-05. State at the move:
      2026-08-10: the rate half met. Patch 0027 holds `up_threshold=40`, `sampling_rate=50000` under
      `ondemandtcl` for a plugin instance's life (Flash had run at 192-384 MHz of 1188), released on
      teardown, from `ActorDestroy` and from the upstart `post-stop`; a third shared buffer
      (`asyncCmdSetExtraBuffer`, YAP 0x1600, cavekit-ipc-contract.md R1) ended the `deferred` paints. The
      smooth half not met: two frame producers, our host clock and Flash's own invalidates (~22/s), spread
      the gaps, while Flash alone gives evenly spaced, slower frames.
      2026-10-01, CE 3.1.0: the shipped job had run an 8 ms tick (39.7 fps, gaps spread flat; the 16 ms
      tick had been a device-local edit). The daemon now ticks at 16 ms while a plugin is on the page:
      the shipped build with no overrides, 32.6 fps, gaps <16 59, 16-23 113, 24-31 107, 32-39 366,
      40-55 78, 56+ 1.
      2026-10-06 (owner: "optimize flash player for gaming performance"): a partial-paint regression
      (`debd1934`) fixed, 15-16 → 34 fps paints; the host frame drivers off by default (the child's timer
      off, `JIHAD_PLUGIN_SELF_DRIVE=0`; the daemon's pull only after 150 ms of plugin silence,
      `JIHAD_PLUGIN_QUIET_MS`): the histogram gathers at 32-39 ms with `<16` empty, and the content rate is
      Flash's own (22.5 fps Flash alone against 22.1 with both old drivers, a 640x480 grid SWF from
      `make-anim-swf.py --grid 32`). `jihad-anim.swf`: Enyo 26-27 fps, Mochi 25-26, Mojo ~20. Left:
      Flash's own rate on this CPU (~27 fps for a 30 fps 320x240 SWF, ~22 at 640x480), and whether it
      looks smooth, a person's judgement.)*
- [x] **Flash produces sound.** *(2026-08-10, heard on the TouchPad speaker with the stock,
      unpatched `libflashplayer.so` (md5 `14c20ba9fc2183a93648c70488e0ec8e`). The port never broke audio;
      the test SWFs were malformed (`make-audio-swf.py` gave a shape and the sound the same character ID;
      MP3 loops lacked `SeekSamples`; `SyncNoMultiple = 0` stacked voices). Flash plays through ALSA by a
      runtime `dlopen` of `libasound` (fallback device `plughw:0,0`), not through Luna or PIpc;
      `/etc/asound.conf` routes it to PulseAudio. Quality is at parity with the stock browser: the click at
      each MP3 loop point is Flash restarting its decoder, and the stock browser clicks identically; judge
      quality with `jihad-audio-long.swf` (30 s, no loop point). The db8 kind
      `com.palm.app.flashplayer.prefs:1` is registered (owner `com.palm.flashgraphics`). 2026-11-06: the
      bundle no longer carries libasound (Changelog). Methodology: assert the card is connected before
      believing a count; parse a generated asset back before believing a negative from it; `strace -f` the
      daemon, not the container. The reverse-engineering notes are in this kit's history archive.)*
- [x] **Keyboard input reaches a focused plugin and does not simultaneously drive the card chrome.** *(2026-10-01, CE: the control plugin runs on the device, `NPP_New ok windowless=1`, a tap at its own 100,75, `key 65` as `NPP_HandleEvent key … raw=65`, its block where the page puts it at zoom 1 and 0.78; the spotlight commands answer `SUPPRESSED`. 2026-11-08: the chrome half measured by the emulated finger and the kernel keyboard, `suite/keyboard-arbitration.test` and `suite/flash-input-kernel.test` (cavekit-flash.md R5, cavekit-input-bridging.md R7), passing on the three shells: the movie engaged by its first tap takes the key and the address bar's pixels do not change; released, the address field takes the next key.)*
      A Flash game focuses no HTML editor, so the editor-focus test the adapter used cannot describe
      it. *(Moved to cavekit-flash.md R5 on 2026-11-05; its kernel-keyboard criterion, `suite/flash-input-kernel.test`,
      is the closer. State at the move: keys reach the plugin at the right codes (2026-08-10:
      `make-key-swf.py`'s AVM1 movie turns green → red on `key 65`, `palm key 0x8 raw=65 handled=1`). The
      adapter's `handleKeyDown`/`handleKeyUp` are gated on `mPageFocused` (from Atlas BrowserAdapter
      `ce199bc`, Apache-2.0, credited in NOTICE) and on `mFlashGestureLock`. The first pen down inside a
      plugin rect sets the latch (`handlePenDown`, T-289, 2026-11-08; until then only a double tap,
      `handlePenDoubleClick` or `js_smartZoom`, did), and only a LunaSysMgr-dispatched
      event can set it or reach `handleKeyDown`; the daemon's `key` inject bypasses the adapter. The
      TouchPad has no keyboard: a uinput keyboard (`render/goanna/test/uinput_kbd.c`) was read by hidd and
      never dispatched (2026-08-10); CE's kernel keyboard (cavekit-input-bridging.md R2a) now exists. The
      observable is the address bar, not `pageYOffset`; `suite/keyboard-arbitration.test` measures it with the
      kernel keyboard, passing on the three shells since 2026-11-08.
      T-107's test page (`jihad-keyarb.html`, `build/webos-oe/stage-test-pages.sh`) and procedure are in
      this kit's history archive.)*

- [x] **Flash is found where the device keeps it.** *(Owner decision, 2026-10-02: nothing shipped gave a user Flash.)* The daemon also scans the device's own plugin folder — `/media/cryptofs/apps/usr/lib/BrowserServerPlugins/`, where the HP Flash plugin app installs `libflashplayer.so` — so Flash works whenever that app is installed, nothing is copied or redistributed, and removing that app removes Flash from this browser. A copy in the profile folder does not make it load twice. *(2026-10-02, CE 3.1.0: each daemon logs `plugin search path: <profile>/plugins:/media/cryptofs/apps/usr/lib/BrowserServerPlugins:<gre>/plugins`; Mochi, its profile folder empty, plays `jihad-anim.swf` (`../impl/evidence-2026-10-02/mochi-flash-from-system-folder_switch-on_switch-off.jpg`); `getFlash` answers `"available":true` on all three. The plugin host does not fold two paths into one, so the system folder is skipped when the profile has its own copy: one `about:plugins` entry (`../impl/evidence-2026-10-02/enyo_plugin-list_single-flash-entry.jpg`). Nothing is written to that folder.)*
- [x] **Flash can be switched off and on.** One stored value, shown on the settings page and by the card switch the Mochi parity work adds (cavekit-mochi-ui.md R2); off means a page with Flash content instantiates no plugin. *(2026-10-02, CE 3.1.0: one stored value, `plugin.state.flash`. The Mochi and Enyo card switches set `getFlash` `"enabled"` false and true, kept across a relaunch; the settings page's "Adobe Flash" row `2 → 0 → 2`; off, the Flash test page instantiates no plugin. The two setters accept only the variant's own card: others get `"errorText":"not permitted"`, logged `REFUSED: caller is not this variant's card`.)*
**Dependencies:** cavekit-offscreen-rendering.md (R2, R5, R6, R8), cavekit-input-bridging.md (R5, R7), cavekit-device-build.md (R8, R9)

### R8: The XPI implementation is validated against Pale Moon and Basilisk
**Description:** Extension support is compared against the two UXP browsers that ship it correctly, rather than reinvented. User requirement, 2026-08-01: *"when you do your adversarial review for xpi extension support make sure you compare it to how basilisk and pale moon do it correctly."*
**Acceptance Criteria:**
- [x] The adversarial review of R1–R6 explicitly diffs our approach against **Pale Moon** and **Basilisk** — both are checked out at `../ref-forks/{Pale-Moon,Basilisk}` and both ship the same UXP-era XPI stack we are targeting, so they are the authoritative reference for what "correct" means here.
- [x] Specifically compared, with findings recorded per item: how they populate application identity (`nsIXULAppInfo`, and whether they rely on `gAppData` from `application.ini` — the exact thing our embedding lacks); their extension install locations and `extensions.enabledScopes` defaults; their signature policy (`xpinstall.signatures.required`) and why; how `about:addons` is registered and reached; and their NPAPI plugin enablement and windowless handling for R7.
- [x] Every place we deliberately DIVERGE from both is justified in writing (embedding vs. full XRE app, headless vs. real widget, three independent variants vs. one browser) — divergence is expected, unexplained divergence is a defect.
- [x] Anything they do that we simply MISSED is filed as a finding, not silently absorbed.

**R8 DONE 2026-08-02**: 9 findings, context/impl/impl-r8-palemoon-basilisk.md. The two that shaped later
work, both since met: R3 needed Pale Moon's `UXP_APPCOMPAT_GUID` mechanism (our frozen GUID is named by no
extension), and R7 needed a windowless NPAPI port for cairo-headless. Signature policy needs no work: UXP
builds no enforcement.
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

### R9: Palefill ships with the browser, unmodified, under its own licence
**Description:** The Palefill Web Technologies Polyfill add-on (Martok & contributors, MPL-2.0) comes with all three variants, so sites that need polyfills get them without the user finding the add-on.
**Added 2026-10-05 (owner: "add palefill add-on to all 3 stock browsers, while respecting its software license").**
**Acceptance Criteria:**
- [x] The author's release file (v1.30) is vendored byte for byte (`third_party/palefill/`, README with origin, version, licence, SHA-256); the bundle script refuses any other hash; `NOTICE` names authors, licence and origin and says the file is unmodified; every package carries `licenses/MPL-2.0.txt`; `about:jihad` credits it.
- [x] It is offered to a profile ONCE, through the engine's own add-on manager (`components/jihadBundledAddons.js`, the same path as a user opening the file): listed in the add-ons manager, removable there, not put back once removed; a newer bundled version upgrades a copy the user kept.
- [x] It installs ENABLED. It did not: the engine preferred the add-on's Firefox-GUID entry (written for Basilisk 52.9.*) over its toolkit entry, and this engine's AppCompat version is 56.9. The engine rule now uses the GUID entry only if it fits or there is no toolkit entry (`third_party/uxp` `db070f0003`); Palefill is unmodified and keeps working through its own updates. *(Device, all three variants, 2026-10-05: installed on first start, active.)*
- [x] It does something: with its debug output on for one run, "applied fixes to https://developer.apple.com/: std-customElements". *(Device, 2026-10-05.)*
- [x] Known and accepted: on first install it tries to open its settings in a browser window this embedding does not have; the engine logs one exception and the add-on runs. Its "generic UXP" support is limited by its author's own account; site problems in Jihad Browser are not Palefill issues.
**Dependencies:** R1–R4 (the add-on manager), cavekit-licensing-branding.md

### R10: Lightweight themes (personas) from Firefox 52, Pale Moon and Basilisk sites and AMO
**Description:** A theme site's "Add" (and preview) works as it does in Firefox 52 / Pale Moon /
Basilisk, an AMO static theme (what AMO serves every former persona as) installs as the same
lightweight theme, and all three shells wear it — toolbar, buttons, and (when built) tabs.
**Added 2026-10-06 (owner: "support firefox 52 personas, pale moon personas, and basilisk
personas"; "use the persona water-like by stick177 to test all 3 variants. buttons and tabs should
also be themed").**
**Acceptance Criteria:**
- [x] A site's `InstallBrowserTheme` asks the user (the daemon's blocking confirm, naming the site
  and the theme), and a yes makes it the theme; a no changes nothing. *(Device, all three shells,
  Water-Like.)*
- [x] The theme is painted on every shell's toolbar, with the ink glyph set when the theme's text
  colour is dark (Firefox's light/dark icon sets): Mochi 6/8, Enyo 6/6, Mojo 8/8 glyphs. A card
  that starts later gets the theme from Luna `getTheme`; a running card follows the `theme`
  notification. Enyo paints both the browser and the start page bar, whichever exists (its views
  are lazy). *(Device.)*
- [x] An AMO static-theme XPI (theme-only `manifest.json`) installs as the lightweight theme after
  the same confirm; anything needing WebExtension machinery still fails as before. Package limits:
  manifest ≤ 64 KB, image ≤ 4 MB, png/jpeg/gif only; the image is unpacked to a pending file kept
  only on a yes; the theme is named by a hash of its image. *(Device, Mochi, Water-Like 2.0 XPI.)*
- [x] A PREVIEW is honoured only from an https page whose site holds the "install" permission
  (Firefox 52's rule) and is reset when the page that showed it is left; only the installed theme
  is remembered. *(Device: an http preview is logged as ignored.)*
- [x] Settings shows the theme worn and leads to the Personas (cavekit-preferences-ui.md R8; since
  2026-10-12 the gallery, from which the Personas manager has Remove, Use and Undo — cavekit-personas-ui.md).
  *(Device, Mochi; the Settings route device-checked in all three 2026-10-12.)*
- [ ] Tabs are themed — waits on tabbed browsing (cavekit-tabbed-browsing.md R1).
**Dependencies:** cavekit-preferences-ui.md (R8), cavekit-tabbed-browsing.md

## Changelog
- 2026-11-06: R7 — the player had no sound stream since the 2026-08-15 bundle: libxul's NEEDED pulled Jessie's libasound into the bundle, the player's dlopen found it first, and it could not load its pulse plugin. `make-device-bundle.sh` leaves it out, the loader finds the device's own, the suite's `flash-audio` reads the stream open and running; `device-check.sh` fails if it comes back. A tap on a movie no longer leaves a dotted focus ring round it.
- 2026-10-08: R10 — a static theme whose header art is only in `additional_backgrounds` (three of five popular AMO themes tried) now converts; device-verified installing five AMO themes in Enyo, Mochi and Mojo. The management UI is now its own kit, cavekit-personas-ui.md.
- 2026-10-06 (night): R7 — partial-paint regression fixed (Flash 16 → 34 fps paints) and the host
  frame drivers off by default, with the content-rate measurement that justifies it. R10 added
  (personas, AMO static themes) with the Codex round-7 hardening: preview gating and reset, theme
  origin taken at the event, bounded JSON and packages, persistence of the installed theme only.
- 2026-10-06 (later): R9 — the bundled add-on now lands within seconds of the daemon's start with NO card open (Mojo, reset profile: manager ready at 4.8 s, Palefill installed at 7.5 s). It used to wait for the first card (41 s measured): the daemon's idle tick drained nothing from the engine's main thread. Fixed in the daemon (`PumpReadyMainThread` on the idle tick; a 20 ms work period with a one-second hysteresis). The installer logs its timeline (`bundled add-ons: … ms after start`).
- 2026-10-06: Added R9 (Palefill bundled). R7: a plugin embedded with the default `wmode=window` is "windowed", and the engine then created a widget this build has none of and dereferenced it — every Flash page not written with `wmode=opaque` killed the daemon, and the card's reconnect-and-reopen turned that into a loop (~40 crash reports in minutes; what the owner saw as "I can't click any links on lite.duckduckgo.com", both tapped results being Flash pages). Engine fix `0048d50fa9` (such an instance is windowless); `628ee50c99` keeps the bundled library path from the shell Flash spawns (one crash report per Flash page); a daemon crash guard refuses the third automatic reopen of an address that took two daemons down (cavekit-device-build.md R4). R3: the AppCompat preference fix above also means a compatibility check is no longer decided by an entry that cannot fit.
- 2026-10-01 (later): R2–R6 re-run on CE 3.1.0 with a real extension (`jihad-effect.xpi`): install prompt,
  accept, listed in about:addons, effect on pages, disable / enable / remove through the row buttons with
  each state surviving a daemon restart, variant-scoped storage. R3 mismatch box `[~]`→`[x]` (toast on the
  card). Found and fixed: a plain link or navigation to an `.xpi` did nothing (only `InstallTrigger` worked).
- 2026-10-01: **Target platform is now webOS CE 3.1.0 — R7 re-run on it.** Flash loads, renders, takes
  input and survives a plugin crash on CE once `libflashplayer.so` is in the profile (the CE install
  had none). Frame-gap histograms re-measured in three configurations; the shipped build was found
  running the worst one (8 ms tick) because the 16 ms tick had only ever been a device-local job
  edit — fixed in the daemon (16 ms tick while a plugin is on the page).
- 2026-08-03: R2 gains the tools-menu criterion (cavekit-offscreen-rendering.md R7, engine popups reach
  the user). R3's card-side blocker, a proven card dialog-reply path, is gone.
- 2026-08-01: Initial draft, from the user's requirement and the on-device diagnosis of the daemon's start
  (R1). The add-ons UI, the XPI install UI and the `about:addons` redirector already ship in the device bundle.
