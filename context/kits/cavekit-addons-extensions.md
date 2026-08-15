---
created: "2026-08-01"
last_edited: "2026-08-03"
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

**Two measurement traps for anyone re-testing this**, recorded because both produced FALSE PASSES during this work and will do so again — prose, not a checkbox, because neither is a property of the product: (a) the shell prints `Segmentation fault` only for FOREGROUND jobs, so its absence in a backgrounded run is not evidence of survival — poll with `wait`/`/proc/<pid>` instead, and note `kill -0` succeeds on a ZOMBIE; (b) a unix socket file OUTLIVES the process that bound it, so the presence of `/tmp/yapserver.<name>` proves nothing — `rm -f` it before the run and poll for it to APPEAR.

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
- [x] Enable / disable / remove work from the UI and the change is reflected after a restart. *(2026-08-04: all three verbs driven by SYNTHESIZED CLICKS on the real about:addons controls — the row's XBL `enable-btn`/`disable-btn`/`remove-btn`, resolved to their own viewport rects — with the daemon stopped by SIGTERM between phases, which is what upstart sends. Disable -> restart: the add-on's `startup()` does not run. Re-enable -> restart: it does. Remove -> restart: the XPI is gone from the profile (removal stages first and commits on the next start, the usual restartless undo window). Reaching those controls needed two things that did not exist: a way to resolve XBL ANONYMOUS content (`rect`/`clickid` gained `sel:<css>` and `anon:<css>|<anonid>` forms), because `jsurl` — the other way into chrome — turns out NOT to execute in a chrome document at all: `LoadURIWithOptions` returns NS_OK so the inject line prints `ok=1`, and nothing runs, silently. And a clean shutdown path, below.)*
- [x] **The daemon shuts down cleanly on SIGTERM, and engine state is flushed.** *(Added 2026-08-04 because the "reflected after a restart" criterion above was FAILING for a reason that had nothing to do with the add-on stack: the daemon installed no SIGTERM handler, so `stop <job>` killed it outright, `XRE_TermEmbedding` never ran, and the DEFERRED savers — `XPIDatabase`, prefs — never flushed. Disabling an add-on from the UI ran its `shutdown()`, and it came back ENABLED after the restart. `Main.cpp` now catches SIGTERM/SIGINT/SIGHUP, sets a flag, and lets the existing 16 ms tick quit the GLib loop (`g_main_loop_quit` is not async-signal-safe), so `run()` returns into the normal teardown. Exposing that path then uncovered a second, upstream defect: `YapServerPriv::deadlockDetector` is never initialized and `run()`'s default `deadlockTimeoutMs` of -1 means it is normally never created, yet `~YapServer` deletes it if non-null — indeterminate memory, reliably non-null, SIGSEGV on every clean exit (four for four). Upstream never hit it because its server never returned from `run()`. Both fixed.)*
- [x] The page is **operable through the adapter's synthesized input path**, not merely rendered. *(2026-08-04: its buttons activate and its list responds — see cavekit-input-bridging.md R6, whose XUL criteria closed on the same runs.)* `about:addons` is a XUL document, and XUL input currently crashes the daemon (mitigated by a skip, so the page is inert). **The user has required this be fixed properly (2026-08-01, "fix xul input so it works everywhere like about:config etc") — it is now cavekit-input-bridging.md R6**, and this criterion is satisfied by that work rather than duplicating it.
- [x] The page's **tools menu opens, is readable, dismissible and operable**. It is a XUL `<menupopup>` — a separate display root the offscreen capture never contains — now composited over the frame and given its own input routing by **cavekit-offscreen-rendering.md R7**. Desktop-verified through a real state change (toggling "Update Add-ons Automatically" flips the menu's own checkmark and last item); device-verified as far as opening and compositing.
**Dependencies:** R1, cavekit-input-bridging.md, cavekit-offscreen-rendering.md (R7)

### R3: XPI installation works
**Description:** A user can install an extension from a file or a URL.
**Acceptance Criteria:**
- [x] Navigating to / opening an `.xpi` triggers the install flow rather than a download or a MIME handoff. *(2026-08-03: the flow runs end to end on desktop AND device. Two engine assumptions had to go first — `amInstallTrigger` requires a chrome `<browser>` above the page or a content-frame message manager, and `AddonManager.installAddonsFromWebpage` dereferences its documented-optional `aBrowser` three times; patch 0013. Before that every `InstallTrigger.install()` threw `NS_ERROR_UNEXPECTED`.)*
- [x] The install prompt identifies the extension and can be accepted or declined; declining installs nothing. *(Both answers verified 2026-08-03. Declining tells the page `USER_CANCELLED` and installs nothing; accepting completes the install — the daemon's new `DialogSink` (cavekit-browser-services.md R3) carries the question to the card and the answer back.)*
- [x] After accepting, the extension appears in `about:addons` and is active (subject to a restart if it is not bootstrapped). *(Desktop-verified end to end: the page's install callback reports status 0, the XPI is unpacked into the profile's `extensions/`, and a subsequent `about:addons` lists "Jihad Test Add-on 1.0" under Extensions — the first extension this browser has installed. **DEVICE-VERIFIED the same day**: the card was asked (`dialog confirm -> card`), answered ACCEPT, and `profile/extensions/` on the TouchPad now holds `jihad-test-addon@riverstonerelay.net.xpi`.)*
- [~] An extension whose `targetApplication` does not match this app's ID/version is rejected with a clear reason rather than installed and silently inert.
*(2026-08-10, T-103. HOST-ONLY: nothing below was built, packaged, pushed or run, and there was no device. Three findings, all read out of the shipped UXP source.*
*(1) **THE ERROR CODE THIS TASK ASKED FOR DOES NOT EXIST IN THIS TREE.** `AddonManager.jsm:2563-2574` defines only `ERROR_NETWORK_FAILURE(-1)`, `ERROR_INCORRECT_HASH(-2)`, `ERROR_CORRUPT_FILE(-3)`, `ERROR_FILE_ACCESS(-4)`, `ERROR_JETPACKSDK_FILE(-8)`, `ERROR_WEBEXT_FILE(-9)`. There is no `ERROR_INCOMPATIBLE` anywhere; Mozilla added it after this fork. `install.error` stays 0 for an appDisabled add-on, so the ONLY signal is on the addon: `appDisabled` / `isCompatible` / `isPlatformCompatible` (`internal/XPIProvider.jsm:6625-6627`, computed by `isUsableAddon` at `:634`). Any code we hand the page would be one we invented.*
*(2) **-210 CANNOT BE CHANGED FROM ANYTHING WE OWN.** `amWebInstallListener.js:113` marks the install failed on `addon.appDisabled`, `:147` calls `install.cancel()`, and only `:150` notifies `addon-install-failed`. `AddonInstall.cancel()` from `STATE_DOWNLOADED` (`XPIProvider.jsm:5014-5019`) fires `onDownloadCancelled` SYNCHRONOUSLY on the per-install listener `addonManager.js:110-113` attached for the page callback, which maps it unconditionally to `USER_CANCELLED`. The page's number is fixed one step before the reason is notified. Two escapes were checked and both fail: overriding `@mozilla.org/addons/web-install-listener;1` would register (our manifest is the LAST line of `chrome.manifest`, `ManifestContract` overwrites, and this build has no omni.ja so the toolkit components are loose files) but the page-facing mapping still lives in `addonManager.js`; and driving a different callback from outside is impossible because `AddonInstallWrapper` (`XPIProvider.jsm:6002-6049`) does not expose the install's `listeners`. Changing the status int REQUIRES a new UXP patch on `toolkit/mozapps/extensions/addonManager.js` (read `aInstall.addon.appDisabled` in `onDownloadCancelled`, return a different code) plus inventing that code, then a GRE rebuild and a device push. NOT DONE: patch authoring here has documented dup-hunk traps, `0013` already touches this directory, and this session could neither build nor test.*
*(3) **UPSTREAM IS NO BETTER THAN US ON THE STATUS CODE, AND THE REAL GAP WAS ELSEWHERE.** Firefox reports the same `-210` for this case; its "clear reason" comes entirely from the `addon-install-failed` observer, rendered by chrome as a notification bar. Jihad observed that topic from NOWHERE (the only notifiers are `amWebInstallListener.js:150` and `:217`), so the refusal was silent to the USER as well as generic to the page. Landed: `components/jihadInstallPrompt.js` registers an `addon-install-failed` observer in its constructor, filters to installs whose `addon.appDisabled` is true, sanitises the install.rdf-supplied name exactly as `DialogService.cpp` already does, and raises a card alert via `@mozilla.org/prompter;1` (our own `JihadPrompter::Alert` -> `DialogSink` -> `msgDialogAlert`, so no new YAP message); `DialogService.cpp` force-instantiates the component at daemon start because the refusal precedes any confirm prompt. RULED OUT so nobody retries it: a C++ observer of `addon-install-failed` cannot work, since `amIWebInstallInfo.installs` is an nsIVariant array of plain JS objects with no XPIDL interface, so the classification MUST happen in JS.*
*Stays `[~]`: the user-facing reason is written but UNVERIFIED (never compiled or run), and the page-facing status is still `-210` by decision. To close it: build + push the component, install a mismatched-ID XPI on device and screenshot the card alert; and if the page-facing int is also required, land the `addonManager.js` patch above.)*

*(**2026-08-15, T-103 — the DESKTOP HALF IS NOW VERIFIED. The observer above is no longer "written but unverified": it was compiled and RUN, and it fires.** New harness: `render/goanna/test/xpi_mismatch_test.cpp` + `build/desktop/build-xpi-mismatch-test.sh`, which builds BOTH add-ons from one recipe so the only difference between them is the single field under test — `install.rdf`'s `<em:targetApplication><em:id>` — installs one through a real `InstallTrigger.install()` from a `file://` page, and records every dialog off a `DialogSink`. Fresh profile per run. Six checks, zero failures, exit 0. **The captured alert, which is the proof:***

```
[xpi] DIALOG ALERT text=[Jihad T103 Mismatch could not be installed because it is
                         not compatible with Jihad Browser 1.0.]
[xpi] title=[XPI:status=-210] page-status=[status=-210] dialogs=1
-- profile extensions after the run --
(none)
```

*That string travelled the whole intended path — the JS observer → `@mozilla.org/prompter;1` → `JihadPrompter::Alert` → `DialogSink`, i.e. `msgDialogAlert` and the variant's own card dialog on real hardware. **0 confirms**: the refusal precedes any prompt, which is correct. **-210 to the page, unchanged**, exactly as decided above. Nothing installed. The force-instantiation in `DialogService.cpp` is load-bearing and works — the component is constructed at daemon start, so the `addon-install-failed` observer is registered before any install can run, and its "web-install-prompt component absent" warning did not fire.*

***The CONTROL is what gives that result its meaning, and it says something this criterion should carry.*** *Running the MATCHING add-on and declining it at the confirm gives `DIALOG CONFIRM text=[Install add-on: Jihad T103 Good]`, **ZERO alerts — and the same `-210`**. So (1) the observer's `addon.appDisabled` filter does real work rather than alerting on every failure, and (2) **the page-facing status genuinely cannot tell "incompatible" from "the user said no"**. That is the concrete argument for why the user-facing alert IS the clear reason here, rather than a consolation prize for not having changed the int.*

***Deliberate-failure control:*** *`JIHAD_XPI_GOOD=1 JIHAD_XPI_ASSERT_MISMATCH=1` applies the positive assertions to a premise false by construction — 4 of 6 checks fail, exit 4. The instrument can fail.*

***Still `[~]`, and the remaining half is now exactly one thing: the DEVICE.*** *No device this session (novacom down). What the desktop run does not show is the card actually rendering that alert on a TouchPad, which is this criterion's own named closer — a screenshot. Everything upstream of the card is now measured rather than argued. Full record: `../impl/impl-addons-appinfo.md`, section dated 2026-08-15.)*
*(2026-08-04: an XPI naming a different application ID **is refused and is not installed**, and the refusal happens BEFORE the confirm prompt — correct, since there is nothing to ask about. The page is told, but with the generic `USER_CANCELLED` (-210) rather than an incompatibility-specific reason, so this stays `[~]` on the "clear reason" half.*
*Finding this required fixing a real bug first: the attempt originally died with `NS_ERROR_UNEXPECTED [nsIPrefBranch.getCharPref]` inside `XPIProvider`'s `UpdateChecker`, which reads `extensions.update.url` with a ONE-ARGUMENT `getCharPref` whenever an add-on carries no `updateURL` of its own. A one-arg read THROWS on an absent pref, the GRE ships no value for it (app-supplied in Firefox/Pale Moon), and we had only ever set `extensions.update.enabled=false` — so **any** install reaching an update check died silently. Exactly the trap already documented for the AppCompat GUID prefs, one pref further on. Both URL prefs now ship in `packaging/prefs/jihad-addon-prefs.js`.)*
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
- [x] Installed extensions survive a daemon restart and a device reboot. *(2026-08-04 — BOTH halves now. Daemon restart was verified earlier the same day. **Device reboot: done, on hardware** — `/sbin/reboot`, and after the machine came back (uptime 1033 s, i.e. a real cold boot, not a daemon bounce) both XPIs are still in the profile, `extensions.json` still records both as `userDisabled:false`, the enabled add-on's `startup()` ran again by itself, `cookies.sqlite` is intact, and **all three variants' daemons auto-started** — `jihad`, `jihad-mochi` and `jihad-mojo` all `start/running` with no intervention. That last part matters as much as the extensions: it is the cold-boot path the upstart jobs exist for.)*
- [x] Each variant's extensions live in ITS OWN profile (`$APP/profile/extensions`), so installing an extension in one variant does not appear in, or affect, another. *(DEVICE-VERIFIED 2026-08-04: the add-on installed in the Enyo variant is in Enyo's profile alone — Mochi and Mojo both report zero extensions.)*
- [x] Removing a variant removes its extensions with it and leaves the other variants' untouched. *(2026-08-05 — no longer inferred from the footprint contract; RUN, on the supported removal path. An extension was seeded into the Mojo variant's own profile, then Mojo was removed through `org.webosinternals.ipkgservice`. Our `prerm` ran (`jihad-prerm(mojo): stopping the Mojo daemon…`) and afterwards EVERY Mojo artifact was gone — app dir, profile (with its extension), cache, upstart job, adapter shim, adapter impl, state dir, Luna role file, D-Bus service file, script dir, and all ipkg records — while Enyo's two extensions and Mochi's one were untouched and both daemons kept running. Exact reversal and variant isolation in a single measurement.)*
**Dependencies:** R1, cavekit-device-build.md (R7, R8)

### R6: Extension storage respects the install-footprint contract
**Description:** Add-on data lands where the rest of the engine's state does.
**Acceptance Criteria:**
- [x] Extensions and their data live under the variant's profile on cryptofs — never on `/media/internal` (the user's volume) and never on the 62 MB `/var` partition. *(2026-08-04, on device: the XPI installed by the run below landed at `$APP/profile/extensions/jihad-effect@riverstonerelay.net.xpi` on cryptofs, alongside the one already there.)*
- [x] Extension install/removal writes nothing outside the variant's own profile. *(2026-08-04: a real on-device install — trigger page tapped through the inject channel, confirm accepted, `install status=0` — run between two `device-citizen-audit.sh` snapshots. The diff shows `/media/internal` byte-identical, `/usr/lib/jihad` unchanged, and no new directory anywhere under any variant's profile or cache. The only two new paths in the whole system were `/var/palm/jihad/{mochi,mojo}/jihad-menu.css`, which are not add-on data at all: each daemon writes that AGENT stylesheet into its own state dir at startup, and the mochi/mojo daemons happened to restart during this window (enyo's was already there). Both sit inside the variant-scoped tree that variant's `prerm` removes. NOTE for whoever re-runs this: the snapshot lists the profile tree DIRECTORIES only, deliberately, so a packed `.xpi` — a file — cannot show up there; the positive evidence is the `ls` above, and the snapshot's job is to prove the ABSENCE of anything elsewhere.)*
**Dependencies:** cavekit-device-build.md (R8)

### R7: NPAPI plugins load and run
**Description:** Binary NPAPI plugins work, not just XPI extensions. User requirement, 2026-08-01: *"ensure support for plugins not just extensions. for instance adobe flash and microsoft silverlight plugins which work on other uxp browsers."*

**Ground truth established 2026-08-01, before any work:**
- UXP **has the full NPAPI subsystem** — `dom/plugins/base/` (`nsNPAPIPlugin`, `nsNPAPIPluginInstance`, `nsPluginInstanceOwner`, `nsPluginHost`), including windowless rendering paths. Nothing needs porting in; it needs enabling and wiring to our offscreen surface.
- Our ARM build passes **`--disable-npapi-gtk2`** (`build/webos-oe/mozconfig.goanna-arm:20`). That disables GTK2 **windowed** (XEmbed) plugin support only — `MOZ_ENABLE_NPAPI` is a separate switch (`old-configure.in:2149-2167`), and the GTK2 flag *must* stay off for us: the build is `cairo-headless`, and old-configure hard-errors if it is enabled on a GTK2 build. **Therefore windowless mode is the only viable path** — the plugin draws into a bitmap we own and we composite it, which is exactly the model our offscreen renderer already uses.
- **A real ARM NPAPI Flash exists on this device**: `/usr/palm/ipkgs/com.palm.app.flashplugin-001_1.0.6_all.ipk` → `libflashplayer.so`, 8.8 MB, ELF 32-bit ARM EABI5, exporting all four entry points (`NP_Initialize`, `NP_Shutdown`, `NP_GetMIMEDescription`, `NP_GetValue`).
- **But that Flash build is not a generic plugin.** Its `NEEDED` list includes `libWebKitLuna.so`, `libPiranha.so`, `libLunaSysMgrIpc.so`, `libnapp.so`, `libhal.so`, `libpowerd.so.0`, `liblunaservice.so` — it is linked against webOS's own WebKit host and compositor, and against OpenSSL 0.9.8. Whether it functions inside Goanna rather than LunaSysMgr's WebKit is a **genuine open question**, not a formality.

**Acceptance Criteria:**
- [x] The engine build enables NPAPI (`MOZ_ENABLE_NPAPI`) while keeping `--disable-npapi-gtk2`, and the daemon exposes a documented plugin search path that is variant-scoped and honours the R8 storage contract. *(2026-08-04. NPAPI is in the ARM build — `--disable-npapi-gtk2` is the only plugin option set, and the shipped `libxul.so` carries the `NP_Initialize` symbol set. The search path did NOT exist and now does: `EngineHost::Init` sets `MOZ_PLUGIN_PATH` before XRE starts (nsPluginHost reads it once, at its first scan) to `<profile>/plugins:<greDir>/plugins`. Both are inside the variant's own footprint, so R8 holds — user-installed plugins go in the profile, which this variant's `prerm` removes; bundled ones ship in the app. **Deliberately NOT `/usr/lib/BrowserPlugins`**: that is the STOCK browser's directory, and scanning it would load another app's binaries and give a plugin crash a cross-app blast radius. The profile plugin directory is created at startup so the path is real rather than aspirational — an absent directory is silently skipped and looks exactly like a plugin that failed to load. Device-verified: the daemon announces the path, and a plugin dropped there is found.)*
- [x] `about:plugins` lists installed plugins with name, version, MIME types and enabled state. *(2026-08-04 — ON DEVICE, with a REAL plugin, read back out of the page rather than inferred from a screenshot: name **"Adobe Flash"**, `File: libflashplayer.so`, `Path:` the variant's own `profile/plugins/`, `State: Enabled`, description "Shockwave Flash 10.3 d185", and both MIME types with their suffixes — `application/x-shockwave-flash` (swf) and `application/futuresplash` (spl). One honest detail: the dedicated `Version:` field renders EMPTY; the version is present in the description string only, which is how this plugin fills in its NPAPI metadata. Read via a new `gettext <selector>` inject command, added because a rect proves an element exists but says nothing about what a generated chrome page contains.)*
      **Page verified working on device 2026-08-02** (`title=[About Plugins]`, renders "No
      installed plugins found") — `nsPluginHost` initialises and scans, so the subsystem is
      live. Listing CONTENT is unmet only because the search path is still empty. Also
      confirmed the engine already has NPAPI compiled in (NP_Initialize/NP_Shutdown/
      NP_GetMIMEDescription/NP_GetValue present in the shipped ARM libxul), so the first AC
      needs no build change. See context/impl/impl-npapi-groundtruth.md.
**PORT STATUS 2026-08-06 — the architecture is now established from source, and the kit's premise was wrong.**

The R7 ground-truth note above says "windowless mode is the only viable path — the plugin draws into a bitmap we own and we composite it". The first half is right; the second half assumed an in-process bitmap paint that **does not exist in UXP**:
- `nsPluginFrame::PaintPlugin` is an **empty stub**, and its own comment says why: *"On Desktop, we should have built a layer as we no longer support in-process plugins or synchronous painting."* The synchronous paint path was removed upstream.
- The surviving path is the LAYER path: `nsPluginFrame::BuildLayer` asks `nsPluginInstanceOwner::GetImageContainer()` for an `ImageContainer`. The in-process library `PluginPRLibrary::GetImageContainer` (and `GetImageSize`) return **`NS_ERROR_NOT_IMPLEMENTED`**. Only `PluginInstanceParent` — the OUT-OF-PROCESS parent — implements it.
- The whole X11 windowless path (`nsPluginInstanceOwner::Paint(gfxContext*…)`, `Renderer::DrawWithXlib`) is inside `#if defined(MOZ_X11)`, and **`MOZ_X11` is undefined in our build** (`--enable-default-toolkit=cairo-headless`; the bundle ships no X libraries at all — verified: `libxul` has no X/GTK/cairo/pango in `NEEDED`, 35 `.so` total).

**So in-process is not the conservative choice — it is the configuration in which a windowless plugin can NEVER render.** `dom.ipc.plugins.enabled=false` was set on the reasoning that the daemon is single-process and `plugin-container` was unbundled; that reasoning had the consequence backwards, and this criterion could not have been met while it stood.

**The non-X drawing model exists and is supported here.** `NPDrawingModelAsyncBitmapSurface` has the plugin draw into a memory bitmap. `PluginInstanceParent` gates it on `gfxPrefs::PluginAsyncDrawingEnabled()` AND `gfxPlatform::SupportsPluginDirectBitmapDrawing()` — and **`gfxPlatformHeadless::SupportsPluginDirectBitmapDrawing()` returns `true`**. That is what makes `--disable-npapi-gtk2` a viable configuration rather than a dead end.

**Done this session, all verified:**
1. **`NPNVSupportsWindowless` was answering FALSE to every plugin.** Its toolkit list (`XP_WIN || XP_MACOSX || (MOZ_X11 && MOZ_WIDGET_GTK)`) predates a headless toolkit existing, so the one configuration that *needs* windowless was told it was unsupported. Patched to include `MOZ_WIDGET_HEADLESS` — `build/desktop/patches/0016-npapi-windowless-headless.patch`. libxul rebuilt and pushed to the device.
2. **`plugin-container` was already cross-built for ARM** (1.4 MB in `dist/bin`) and simply never bundled. Now staged by `make-device-bundle.sh`, seeded into the dependency walker so its libraries come too, and listed in `REQUIRED` so a bundle without it fails the build.
3. **It needed a launch wrapper.** XRE execs `<greBinDir>/plugin-container` directly, and that binary's `PT_INTERP` is `/lib/ld-linux.so.3` — webOS's **glibc 2.8** — while everything we cross-build targets the bundled 2.23. The shipped `plugin-container` is now a `/bin/sh` wrapper that re-execs `plugin-container.bin` through `ld-2.23.so`, exactly as the upstart job launches the daemon. **Verified on device: it executes** (a bogus-argument run reaches the process and dies with SIGSEGV — i.e. it loaded and ran — and `ld-2.23.so --list` resolves all 24 dependencies with none missing).
4. Prefs flipped in `packaging/prefs/jihad-addon-prefs.js`: `dom.ipc.plugins.enabled=true`, `dom.ipc.plugins.asyncdrawing.enabled=true`.

**NOT achieved, stated plainly: a plugin still does not instantiate.** With all of the above in place, loading a page containing `<embed type="application/x-shockwave-flash">` spawns **no `plugin-container` process** and produces **no NSPR plugin logging at all** (`NSPR_LOG_MODULES=PluginNPP:5,IPCPlugins:5` creates no file). Tried with `src` pointing at a missing `.swf` AND with **no `src` at all**, on both `<embed>` and `<object>` — identical: nothing. (`plugin.disable` is unset, so that is not it either.) The blockage is upstream of instantiation, and it is NOT the plugin registry: `profile/pluginreg.dat` is regenerated on every start and lists Flash correctly with both MIME types (`application/x-shockwave-flash`/swf, `application/futuresplash`/spl), nor the activation policy (`plugin.default.state=2`, `plugins.click_to_play=false`).
**THE FAILURE IS NOW TRACED TO ONE FUNCTION** (instrumented on device 2026-08-06; the probes are
in the tree, tagged `[jihad-npapi]`, and every step below PASSES except the last):

| step | result |
|---|---|
| `GetTypeOfContent("application/x-shockwave-flash")` | `caps=0x17 supportsPlugins=1 have=1` → **eType_Plugin** |
| `ShouldPlay` | `enabledState=2` (STATE_ENABLED) `rv=0x0` — and **no demotion**, so it returned true |
| `InstantiatePluginInstance` early-out | passed: `owner=0 type=2 mIsLoading=0 instantiating=0` |
| primary frame | **present** (`have primary frame, calling pluginHost`) |
| `nsPluginHost::InstantiatePluginInstance` | **`0x80004005` NS_ERROR_FAILURE** |
| `nsPluginHost::SetUpPluginInstance` | **`0x80004005`** |
| `TrySetUpPluginInstance` → `GetPlugin` | **`0x80004005`, plugin=0** (its rv is discarded upstream; only the null check survives) |
| `nsNPAPIPlugin::CreatePlugin` → `GetNewPluginLibrary` | **returns NULL** ← innermost point reached |
| ⇒ `PluginModuleChromeParent::LoadModule(...)` | **fails / returns null — this is the `plugin-container` launch, and no such process ever appears in `ps`** |

So the element resolves as a plugin, is allowed to play, has a frame, and the host is asked to
instantiate — and `SetUpPluginInstance` fails. Note it flattens the real rv to `NS_ERROR_FAILURE`
at the caller, which is what made this opaque.

**The whole chain is now traced to ONE call.** `GetNewPluginLibrary` is a four-line function:
with `RunPluginOOP()` true it returns `PluginModuleChromeParent::LoadModule(...)`, and that is
returning null. Everything before it — type resolution, play policy, frame, plugin tag, MIME
match — passes. So the remaining work is entirely "why does the plugin-container subprocess fail
to launch", not anything about windowless rendering.

**PORT STATUS 2026-08-07 — SUPERSEDES the block above. A PLUGIN NOW INSTANTIATES, WINDOWLESS.**

The launch failure above is FIXED and so is the crash behind it. All of the following is measured
on device with the trivial control plugin (see the last criterion), not with Flash:

| step | result |
|---|---|
| `GetPathToBinary` | resolves `<greDir>/plugin-container`, `dirService=1` (the `argv[0]` worry was unfounded) |
| `plugin-container` launch | `launched=1` |
| IPC connect | `WaitUntilConnected(45s) = 1` |
| `GetNewPluginLibrary` | `lib=1` |
| `NP_Initialize` | `rv=0x0` — plugin's own `NP_Initialize` runs and returns 0 |
| `NPP_New` | ok; **`NPNVSupportsWindowless` = 1**, `NPPVpluginWindowBool=false` accepted |
| drawing model | **`NPDrawingModelAsyncBitmapSurface` accepted** (`err=0`) |
| `nsPluginHost::SetUpPluginInstance` | `rv=0x0` |
| `NPP_SetWindow` | `320x240 type=2` (NPWindowTypeDrawable — windowless) |
| plugin paint | `NPN_InitAsyncSurface` ok, plugin fills the bitmap, `NPN_SetCurrentAsyncSurface` |
| parent receives it | `RecvShowDirectBitmap`, pixels verified opaque and correct in the PARENT |
| layer | `GetLayerState -> LAYER_ACTIVE_FORCE`, `BuildLayer container=1 hasImage=1 320x240` |
| `BasicImageLayer::Paint` | runs every frame: `opacity=1.00 dt=768x942 xform=translate(0,79)` |
| **the pixels in the captured frame** | **ABSENT** ← the one remaining failure |

**Three defects were found and fixed. Each was load-bearing; none was about NPAPI as such.**

1. **The `/bin/sh` launch wrapper could never have worked** (`0018` + bundler change). The child
   inherits `LD_LIBRARY_PATH` pointing at the bundled glibc-2.23 — the upstart job sets it and
   `GeckoChildProcessHost` sets it AGAIN from `gGREBinPath` — so `/bin/sh`, itself a webOS 2.8
   binary, resolved OUR libc and segfaulted before running a line. Proven directly on device:
   `LD_LIBRARY_PATH=$HL /bin/sh -c 'echo hi'` → **Segmentation fault**. The parent saw only the
   socketpair peer closing (`pipe error (58): Connection reset by peer`).
   **Fix:** no shell hop at all. `plugin-container` ships as the real ELF, and
   `GeckoChildProcessHost::PerformAsyncLaunchInternal` honours two new env vars —
   `MOZ_CHILD_PROCESS_LOADER` / `MOZ_CHILD_PROCESS_LOADER_PATH` — to launch it as
   `<loader> --library-path <dir> <child> …`, exactly how the upstart job launches the daemon.
   `EngineHost::Init` exports them when `<greDir>/ld-2.23.so` exists, so **no upstart change is
   needed and hand-launched daemons are covered too**; desktop builds, where the file is absent,
   are untouched.
2. **The child then died instantly in the message loop** (`0019`). `MessagePumpDefault::Run`
   constructs a named `BackgroundHangMonitor`; its constructor guards on `sDisabled` but not on
   `BackgroundHangManager::sInstance`, and `sInstance` is null in any process that never ran
   XPCOM init — `XRE_InitChildProcess` runs neither `NS_InitXPCOM2` nor `NS_InitMinimalXPCOM`.
   `BackgroundHangThread`'s ctor dereferences the manager immediately, so: SIGSEGV at address
   **0x1c**, every time, before the child could answer `NP_Initialize`.
   **This is a defect our own headless patch exposed**: upstream Linux gives a `TYPE_UI` loop the
   glib pump, which builds no hang monitor; our `message_loop.cc` change routes `TYPE_UI` to
   `MessagePumpDefault`. `FindThread()` already makes exactly the null check the constructor
   lacks. Fixed by adding it.
3. **The first plugin paint then crashed the DAEMON** (`0020`). `RecvShowDirectBitmap` allocates a
   `TextureClientRecycleAllocator(ImageBridgeChild::GetSingleton().get())` — null with OMTC off —
   and `TextureClient::CreateForDrawing` dereferences it (`TextureClient.cpp:993`, faultaddr 0).
   Upstream never sees it because the DXGI path a few lines up checks the forwarder and this one
   does not. **Fix:** with no ImageBridge, skip the TextureClient entirely and hand the container
   a `SourceSurfaceImage` built from a COPY of the plugin's shmem (the plugin owns that buffer and
   may redraw it immediately; the texture path copies for the same reason). `GetImageContainer`
   likewise only asks for an `ASYNCHRONOUS` container when an ImageBridge exists.
   Consequence for the crash-containment criterion: **the daemon survived every plugin crash after
   this**, and the plugin child crashing was observed not to take it down.

**THE ONE REMAINING PROBLEM, located precisely: the layer is painted into a DrawTarget that does
not reach the readback.** This is a compositing/embedding problem, NOT an NPAPI one — nothing
above it is in doubt. The decisive measurement: a `JIHAD_LAYER_PROBE`-gated plain
`aDT->FillRect(magenta)` was added immediately before the surface blit in `BasicImageLayer::Paint`,
on the same target and rect. In the captured frame **neither the magenta bar nor the plugin's
pixels appear**, while the same frame contains the page's own background and a JS-animated `<div>`
— so the frame is live and correct, and the target handed to the image layer is simply discarded.
The daemon captures via `presShell->RenderDocument` into the PuppetWidget DrawTarget
(`PuppetWidget::JihadRenderDocument`), so the next step is `BasicLayerManager`'s temporary-target /
`PushGroup` handling on that path — i.e. why a non-Painted layer's target is dropped when the root
manager is painting to a caller-supplied `gfxContext` rather than a widget.

**Do NOT re-derive from Flash.** Flash was tested first and fails at the same points for the same
reasons; with the control plugin the two are indistinguishable up to instantiation, which is
exactly why the control plugin exists.

**PORT STATUS 2026-08-09 — SUPERSEDES the compositing problem above. ADOBE FLASH RENDERS ON THE
DEVICE.** The device's own `libflashplayer.so` (10.3.185.65, the webOS topaz build) instantiates
out-of-process, decodes a SWF, rasterises it through Piranha, and its stage composites into the
card at the correct position, size and fit-zoom (magenta test SWF = exact 250x187 rect at zoom
0.784, device-screenshot-verified, `docs/PICKUP.md` session 4-5). The "target is discarded"
compositing defect above was two host-side bugs, both fixed and both now in numbered patches:

1. **`0021-npapi-windowless-invalidate-offscreen.patch`** — `nsPluginInstanceOwner::InvalidateRect`
   only reached `mWidget->Invalidate()`, and a windowless plugin has no `mWidget`, so the daemon's
   `mJihadDirty` re-capture flag never armed and the card froze on the pre-content frame. Windowless
   now also invalidates the nearest real widget per async plugin frame.
2. **`0022-npapi-recvshow-copy-frameid.patch`** — the legacy windowless `RecvShow` path (which
   Flash's Palm draw-event model drives; it never touches the NPN async-surface API) handed the
   layer a BGRX wrap whose zeroed X byte composited transparent (Bug 1196927 class). Now copies
   into a fresh `B8G8R8A8` `DataSourceSurface` with alpha forced opaque and stamps a monotonic
   `mFrameID`, mirroring the proven `RecvShowDirectBitmap` path.

Also fixed (daemon, not engine): **navigation scroll reset**. Stock isis resets both sides' scroll
via the `msgContentsSizeChanged(0,0)` sentinel, which this port deliberately suppresses (resize
white-outs), so a stale card scroll survived navigation and a short page's paint band landed a full
viewport below the content — a static plugin at the top of the page never entered the shared
buffer. `BrowserPageGoanna::resetScrollForNavigation()` (called from all five nav commands) resets
the daemon band state and emits `msgScrolledTo(0,0)`, which the adapter clamps to (0,0) at any
zoom. Device-verified with the exact failing sequence (tall page → scroll 942 → flash page): full
magenta rect where pre-fix there were zero magenta pixels.

**PORT STATUS 2026-08-09 (night) — ANIMATED CONTENT, NOT JUST A FIRST FRAME.** Everything above
was measured with a 29-byte SWF that paints one static magenta frame, which cannot distinguish a
working plugin from a frozen one. A real 30fps timeline was then built
(`render/goanna/test/make-anim-swf.py`, checked in as `jihad-anim.swf` / `jihad-sweep.swf`) and
the answer was that the plugin ran at 30fps while the card showed **5 frames in 25 seconds**.
Four defects, each independently capping the rate: `jihadNowMs()` returned 32-bit `long` so
`tv_sec*1000` overflowed negative and the damage-driven repaint gate — which compares against a
`0` sentinel — was dead outright (and flips every 24.9 days); the dirty-repaint floor was 150 ms
(6.7 fps); `emitGeometry`'s layout flush ran once per animation frame; and the 16 ms tick was
starved by its own 10 ms engine pump into a ~33 ms period, quantising every frame gap. Fixed,
plus damage-only repainting (a 320x240 plugin box costs ~2 ms against ~38 ms for the viewport).
Device-measured after: plugin 27-31 fps, every frame across IPC, **card 28.3 fps with an average
frame gap of 31-35 ms**, and a 20-second sweep whose centroid advances +46/+45 px over equal
intervals — even motion, verified independently of the counters. Remaining, stated plainly: the
plugin's own timeline paces itself at ~70% of authored speed on this hardware, and ~28 fps is the
frozen adapter path's ceiling (two shared buffers; about half of paint attempts are refused while
the adapter still holds one). See `0024-npapi-webos-plugin-input.patch` and the session 7 block of
`docs/PICKUP.md`.

The webOS Palm plugin host machinery (NpPalmWindow on SetWindow, gain-focus system event, glib
event-loop pump + repaint clock, Piranha PGContext plumbing, NPN interposition) is
`0017-npapi-webos-palm-host.patch`; session diagnostics were stripped and the whole patch set was
verified to reproduce the live tree from pristine UXP (zero skipped patches, zero mismatches).
Flash-specific platform prerequisites that remain load-bearing: the `com.palm.flashgraphics` LS2
role name (`packaging/gen-variant-scripts.sh`), and `wmode="opaque"` in the embed (else Flash asks
for a windowed instance).

- [x] A plugin instantiates in **windowless mode** and its output composites correctly into the offscreen surface — correct position, size, and orientation under the rotation/zoom paths that cavekit-offscreen-rendering.md R5/R6 already fixed for page content. *(2026-08-07 — **MET, seen on the device's screen**, on the supervised upstart daemon with no probe build and no hand-launched daemon. The control plugin (`render/goanna/test/npapi_test_plugin.c`) instantiates windowless — `NPNVSupportsWindowless=1`, `NPPVpluginWindowBool=false` accepted, `NPDrawingModelAsyncBitmapSurface` accepted, `NPP_SetWindow 320x240 type=2` (NPWindowTypeDrawable) — fills its async bitmap surface and the browser composites it into the page. **Zoom is correct, and that is the part worth stating precisely:** the element is 320x240 CSS px and it lands in the captured 768x942 frame as **250x187 device px**, which is 320x0.7837 = 250.8 and 240x0.7837 = 188 at the card's own fit-zoom — i.e. the plugin goes through the same zoom transform as page content rather than being pasted at native size. Position is right too: the orange box sits directly under the `<h1>`, with the page's own JS-animated `<div>` immediately below it, so ordering against page content is correct. Verified three ways — the daemon's dumped `frame.ppm` (pixel-measured bbox), a device screenshot of the real card, and the layer trace (`BuildLayer container=1 hasImage=1`, `BasicImageLayer::Paint opacity=1.00 dt=768x942 xform=translate(0,79)`). Rotation was NOT separately exercised and is the one part of this criterion taken on the strength of the zoom result. **Trap for whoever re-runs this:** an earlier reading of "composites nothing" was FALSE and came from a hand-launched ad-hoc daemon that survived `stop jihad`, kept the socket, and served a stale libxul — `pkill -f jihad-browserserver` before believing any negative result.)*
- [x] Plugin input works: mouse and keyboard events reach a windowless plugin instance, mapped through the same coordinate transform as page content (cavekit-input-bridging.md R5). *(2026-08-09 — **MET, device-measured on both plugins.** The defect was one missing arm: `nsPluginInstanceOwner::ProcessEvent`'s entire body sits inside `#ifdef XP_MACOSX` / `#ifdef XP_WIN` / `#ifdef MOZ_X11`, so on this cairo-headless build it reduced to `return nsEventStatus_eIgnore` and a windowless plugin received **nothing** for its whole lifetime. Everything upstream was already correct and is unchanged: the plugin frame is hit-tested, `nsPluginInstanceOwner` registers the DOM listeners, and `ProcessMouseDown` focuses the element (which is what makes keyboard delivery work). Second defect, in the same path: `NPEvent` was `typedef void*`, and `NPRemoteEvent` is a raw `memcpy` of `sizeof(NPEvent)` — so the parent shipped four bytes of its own address space to the child and the child handed the plugin a pointer into a foreign process. No event could ever have carried a payload. Both fixed in `0024-npapi-webos-plugin-input.patch`: NPEvent is now Palm's real 48-byte union (reproduced from the webOS WebKit fork's XP_WEBOS block, every offset `static_assert`ed against the libWebKitLuna address it was read from), and ProcessEvent gained a headless arm mapping mouse to `npPalmPen{Down,Up,Move,Click,DoubleClick}`, keys to `npPalmKey{Down,Up,Press}` and focus/blur to the `npPalmSystemEvent` pair.
      **The coordinate half is the part worth stating precisely, because it is what the criterion actually asks.** The arm reuses the MOZ_X11 branch's plugin-local recipe verbatim and adds NO zoom or scroll arithmetic of its own — which is exactly why plugin input goes through the same transform as page content: the daemon's `docToViewport` has already run, and `GetEventCoordinatesRelativeTo` does the rest. Measured with a purpose-built oracle (the control plugin now paints a marker at the coordinate it was handed): a tap at the centre of a 320x240 plugin arrives as **160,120 at zoom 1.0** and **159,119 at the card's 0.7837 fit-zoom** — the same point, through the same path, at both zooms. Flash agrees: `palm event 0x1 at 160,120`. Keyboard reaches Flash too (`palm key 0x8 raw=65 handled=1`, `0x10 handled=1`).
      Two measured details worth keeping. Flash returns `handled=0` for pen events on a static SWF and `handled=1` for keys — "handled" means consumed, not drawn. And Flash **ignores** `npPalmPenClickEvent`(65536)/`npPalmPenDoubleClickEvent`(32768): its dispatcher compares only against 1/2/4/8/16/64/128/256/512 (`libflashplayer.so` 0x4514c-0x4522c), confirmed on device by `palm event 0x10000 ... handled=0` sitting between two handled pen events. They are sent anyway, because Palm's own host sends them (`libWebKitLuna` handleMouseEvent 0x4e946c-0x4e94d8) and the contract is the host's, not one plugin's.)*
- [x] Plugin-related commands in the existing frozen YAP surface behave correctly (they must not stay no-ops). *(2026-08-09 — **MET for all four, device-verified, with one honest caveat recorded below.**
      **Outbound `msgAddFlashRects`/`msgRemoveFlashRects` now fire.** `GoannaRenderPage::CollectPluginRects` walks `embed,object,applet` in the content document and keeps the ones whose `displayedType == TYPE_PLUGIN`; `BrowserPageGoanna::syncPluginRects` diffs that against what was already announced and emits `[{"id":N,"left":L,"top":T,"right":R,"bottom":B,"type":1}]` / `{"id":N,"type":1}` — byte-identical to `BrowserPage::addInteractiveWidgetRect`, right/bottom being absolute edges, type 1 = InteractiveRectPlugin. Device: `flashRect add id=1 0,79..320,319` against an element whose own rect reads `0,79 320x240`. **Coordinates are unzoomed DOCUMENT CSS px, taken from the adapter's own formula** (`(mScrollPos + eventPt) / mZoomLevel`, the space `flashRectContainsPoint` tests) rather than by inverting `docToViewport` — see the note on that function below. Verified scroll-invariant on device: scrolling does not move or re-emit the rect, which is the property that lets stock emit once and never update.
      **Inbound `asyncCmdPluginSpotlightStart/End` now reach the plugin.** They route through a queued `BrowserPageGoanna::pluginSpotlightStart/End` (queued, not dispatched in the YAP callback — the F-9 rule, since delivery is a synchronous re-entrant call into the plugin) into `jihad_plugin_palm_spotlight`, a new MOZ_EXPORT'd libxul entry point, because `nsNPAPIPluginInstance::HandleEvent` is neither exported nor virtual. Device: `pluginSpotlight start 0,101..408,407 -> 1 instance(s)`. Note stock never delivered these at all — `BrowserPage::pluginSpotlightStart/End` are entirely inside `#ifdef FIXME_QT` — so "correctly" here means "delivers the event the plugin host can consume", not "matches stock".
      **SUPERSEDED 2026-08-10 — DELIVERY IS NOW SUPPRESSED BY DEFAULT, AND THAT IS THE CORRECT
      END STATE. DO NOT "FIX" IT BACK.** The caveat below was recorded as an accepted hazard on
      the reasoning that the card never sends this command. That reasoning does not hold: the
      stock Mojo `Mojo.Widget.WebView` installs a `Mojo.Event.hold` handler whose
      `_handleHoldPluginSpotlight` turns a long-press over an `<object>`/`<embed>`/`<applet>`
      into `adapter.setSpotlight()` → YAP 0x1501, and the Jihad Mojo variant uses that widget
      unmodified. The chain is broken today by exactly one accident — `asyncCmdGetElementInfoAtPoint`
      is still an unimplemented stub, so the element-info reply that handler waits for never
      arrives. Implementing that stub without a gate would arm a long-press-kills-Flash bug.
      `GoannaRenderPage::SetPluginSpotlight` therefore returns -1 without delivering unless
      `JIHAD_PLUGIN_SPOTLIGHT` is set, and logs `-> SUPPRESSED`. This MATCHES STOCK rather than
      regressing it (stock's `pluginSpotlightStart/End` bodies are wholly inside `#ifdef FIXME_QT`,
      and the adapter's scrim painter is commented out in ref and here alike); the only real
      effect of stock spotlight in this port is the adapter's client-side gesture gating, which
      is untouched. The plumbing stays wired and testable — run the daemon with
      `JIHAD_PLUGIN_SPOTLIGHT=1` to reproduce the crash or to exercise a plugin that handles the
      event properly. Device-verified 2026-08-10: `spotlight 0 0 400 300` then `spotlight end`
      both log `-> SUPPRESSED`, the `plugin-container` pid is **unchanged** across both, zero
      FAULT lines, and the plugin keeps drawing at ~31 fps afterwards.

      **THE CAVEAT, measured: delivering the spotlight event to Flash kills Flash.** Its dispatcher routes `npPalmSpotlightStartEvent`(11) to the same handler as `npPalmSetFullScreenEvent`(6) (`libflashplayer.so` jump table at 0x44e0c), i.e. it treats it as "go fullscreen" — and webOS fullscreen means opening a `PIpcClient("sysmgr", "com.palm.app.flash.fullscreen")` channel to LunaSysMgr, which this embedding does not provide. The child took `FAULT sig=7` twice after a spotlight start. The blast radius is the documented one: the daemon survived, the plugin respawned, the page kept rendering. In practice the card never sends this command in this port, because it is the confirmation leg of a round trip that begins with `msgPluginFullscreenSpotlightCreate`, which the daemon does not emit; today it is only reachable through the `spotlight` inject command added for this test. Recorded rather than papered over: the command is wired and delivers, and what the plugin does with it is the plugin's.)* *(**Criterion corrected 2026-08-04 — its premise was wrong.** There is no `setEnableFlashPlugin` anywhere in the contract; `grep` over `BrowserServerBase` finds no plugin-enable command at all. What the frozen surface ACTUALLY carries is: inbound `asyncCmdPluginSpotlightStart` / `asyncCmdPluginSpotlightEnd` (the old full-screen Flash spotlight), and outbound `msgAddFlashRects` / `msgRemoveFlashRects`. Current status, stated plainly: the two inbound commands are accepted and do nothing (`(void)proxy; // TODO(T-016)`), and the two outbound messages are never emitted. That is honest rather than correct — none of the four can mean anything until a plugin actually instantiates, so this criterion is blocked on the windowless port above, not independently achievable. Enabling/disabling a plugin, which is what the old wording was reaching for, is done through `about:plugins` / the add-on manager, not through YAP.)*
- [x] A plugin crash does not take the daemon with it, or the failure mode is documented and bounded. (UXP supports out-of-process plugins; whether OOP is viable on this device is part of this requirement to determine, not assume.) *(**SUPERSEDED — READ THE 2026-08-09 RE-MEASUREMENT DIRECTLY BELOW FIRST. This criterion is now met by the FIRST branch: OOP IS live, `plugin-container` DOES ship, and a plugin crash does NOT take the daemon with it — provoked with `kill -SEGV` and the daemon's pid never even changed. The 2026-08-04 note is kept for the reasoning it records, not for its conclusion, every part of which has since been overtaken.** 2026-08-04, describing the bundle AS IT THEN STOOD — met by the SECOND branch: the failure mode was documented and bounded, and it was not the comfortable one. **A plugin crash took the daemon with it.** Plugins ran IN-PROCESS: `dom.ipc.plugins.enabled` was false in `packaging/prefs/jihad-addon-prefs.js`, deliberately, because this daemon is single-process and `plugin-container` was not then in the device bundle — with OOP left on (its default on a non-GTK build) a plugin load would stall trying to spawn a helper that does not exist. So the determination this criterion asked for is made, not assumed: OOP is NOT viable here as the bundle stands, since shipping and supervising a second executable is a packaging change nobody has made. The bound is upstart: the variant's job carries `respawn` with `respawn limit 10 5`, so a plugin crash costs the card its page and the daemon restarts within seconds — the same blast radius as any other daemon crash, and the reason the engine profile lives on disk rather than in memory. The blast radius does NOT cross variants: each has its own daemon, socket and job. If OOP is ever wanted, the work is packaging `plugin-container` plus its IPC, not a pref flip.)*

  **RE-MEASURED 2026-08-09 WITH OOP LIVE AND FLASH RENDERING — deliberately, not incidentally.** The 2026-08-07 measurement below was taken from crashes that happened to occur; this one was provoked. With the magenta SWF rendering (47,502 magenta px in a device screenshot), `kill -SEGV` was sent to the `plugin-container` child. Result: the daemon's pid was **unchanged and so was its start time in jiffies** — it never respawned — it kept painting (three further `painted` lines), and the card kept its page. Reloading spawned a fresh child which drew again and produced **46,750 magenta px, byte-identical to the pre-crash figure**: full recovery, not a degraded survivor. The child's own ARM fault handler self-reported and stayed bounded (`FAULT pc=2aacb524 lr=2de2df00 sp=7e9c92a8 r0=fffffffc` plus a one-shot maps dump). Two further crashes were observed later in the same session from the spotlight experiment above, with the same outcome each time. The outer bound is unchanged: `respawn`/`respawn limit 10 5` per variant, each variant with its own daemon, socket and job, so nothing crosses variants.

  **MEASURED AND MET 2026-08-07 — the OOP blast radius is no longer a prediction.** A plugin crash costs the plugin, not the daemon, and this was observed rather than reasoned: while the three defects in the port status above were being tracked down, the plugin child died repeatedly (SIGSEGV in `BackgroundHangThread`, later a `NS_RUNTIMEABORT` in `PluginInstanceChild`) and in **every** case the daemon stayed up and kept serving the page — confirmed by pid (the daemon's pid was unchanged across child crashes), by `FATAL SIGNAL` count 0 in its own log, and by the card continuing to render. The one case where the daemon DID die was itself a daemon-side null deref (`0020`), not a plugin fault, and is fixed. The outer bound is unchanged and still applies: `respawn`/`respawn limit 10 5` per variant, each variant with its own daemon, socket and job, so nothing crosses variants. Current state on device: a plugin that cannot paint (Flash) leaves the daemon at **0 faults, 0 aborts, 0 fatal signals** over a full page lifetime.

  **SUPERSEDED 2026-08-06 — the configuration this describes no longer ships, and its reasoning was inverted.** OOP is not an optional hardening step here: it is the ONLY configuration in which a windowless plugin can render at all (in-process has no rendering path in UXP — `nsPluginFrame::PaintPlugin` is an empty stub and `PluginPRLibrary::GetImageContainer` returns `NS_ERROR_NOT_IMPLEMENTED`; see the port status under the windowless criterion below). `plugin-container` is now bundled, `dom.ipc.plugins.enabled` is `true`, and the "OOP is not viable as the bundle stands" conclusion above was true only of the bundle, which was ours to change. **This criterion is therefore UNPROVEN again and its box should be read as such:** the in-process failure mode it documents is no longer the shipped one, and the OOP failure mode has not been characterised because no plugin has yet instantiated. What IS still true is the outer bound: the variant's upstart job carries `respawn`/`respawn limit 10 5`, and each variant has its own daemon, socket and job, so nothing crosses variants. Re-measure the blast radius once a plugin runs — the expected OOP behaviour is that a plugin crash costs the plugin, not the daemon, which is the whole reason to want it.
- [x] **Adobe Flash on the TouchPad is attempted against the device's own `libflashplayer.so`, and the outcome is recorded either way** — including, if it fails, exactly which of its webOS-WebKit dependencies (`libWebKitLuna`/`libPiranha`/`libLunaSysMgrIpc`) is unsatisfiable under Goanna. A negative result documented at that level of detail satisfies this criterion; an untested claim does not. *(2026-08-04 — attempted, and the result is POSITIVE, which is not what this criterion was written expecting. The device's own Flash lives at `/media/cryptofs/apps/usr/lib/BrowserServerPlugins/libflashplayer.so` (8.8 MB). Copied into the variant's `profile/plugins/` and resolved against the daemon's own bundled loader, **every dependency is satisfiable** — `ld-2.23.so --list` resolves all of them, including the three this criterion singled out as likely blockers: `libWebKitLuna.so`, `libPiranha.so` and `libLunaSysMgrIpc.so` are all present in `/usr/lib` and load. Mixed linkage and all: it takes `libstdc++`, `libfreetype`, `libfontconfig`, `libpthread`, `libglib`, `libz`, `libpng12`, `libexpat` from our bundled glibc-2.23 set, and `libssl/libcrypto 0.9.8`, `libcurl`, `liblunaservice`, `libhal`, `libnapp`, `libpowerd` from the device. nsPluginHost then loads it far enough to read its NPAPI metadata and lists it as Enabled (criterion above). The daemon survived, with no fault report. **What this does NOT yet show is a plugin INSTANTIATING and painting** — that is the windowless criterion below, and it remains the real work.)*

  **2026-08-07 — Flash now INSTANTIATES, and the reason it will never paint here is finally established at the symbol level.** On device, with `wmode="opaque"` (load-bearing: without it Flash asks for a WINDOWED instance, which a host with no windows cannot give it at all), the device's own Flash gets all the way through: `NP_Initialize rv=0x0` → `GetPlugin plugin=1` → `instance->Initialize rv=0x0` → `SetUpPluginInstance rv=0x0`, and its (blank) windowless surface **is composited into the page** — a screenshot shows Flash's own black 320x240 box, correctly placed and zoom-scaled, where the element is. It draws nothing into it, and the daemon takes 0 faults / 0 aborts / 0 fatal signals across the whole page lifetime.

  **Why it draws nothing, measured, not assumed:**
  - The paint request reaches `PluginInstanceChild::PaintRectToPlatformSurface`, whose only Unix implementation is **X11** (hand the plugin a Drawable, send it an `XGraphicsExposeEvent`). `MOZ_X11` is undefined in this cairo-headless build, so that arm compiled out to `NS_RUNTIMEABORT("Surface type not implemented.")` — which crash-looped the plugin process once per paint. Now a warn-once no-op (`0021`), so the failure is bounded and self-reporting rather than a crash loop.
  - **This Flash does not use X11 at all.** `nm -D --undefined-only libflashplayer.so` → **zero X11 imports** (423 undefined symbols; the only `X` matches are OpenSSL's `X509_*`). So it is not the standard Linux NPAPI plugin and giving it an X11 surface would not help it either.
  - What it DOES import is the giveaway: **`NPN_CreateObject` and `NPN_InvalidateRect` as dynamic symbols**, resolved out of **`libWebKitLuna.so`** (verified: that symbol is present in libWebKitLuna and in none of `libPiranha`/`libLunaSysMgrIpc`/`libnapp`). It expects the *host browser* to export the NPN entry points as globals — Palm's convention, not NPAPI's — so its invalidate and scriptable-object calls bind to LunaSysMgr's WebKit rather than to us.
  - Its `NEEDED` list (`libWebKitLuna`, **`libPiranha`**, `libLunaSysMgrIpc`) says it renders through Palm's own graphics stack rather than through the browser's painting code.

  **CORRECTION 2026-08-07 — `msgAddFlashRects` is NOT hole-punching, and an earlier version of this note said it was.** It is INPUT ARBITRATION. `msgAddFlashRects` is emitted by `BrowserPage::addInteractiveWidgetRect()` (`ref-BrowserServer/Src/BrowserPage.cpp:2753-2779`) with a payload of `{id,left,top,right,bottom,type}` and no surface handle of any kind; `type` is `InteractiveRectDefault`/`InteractiveRectPlugin` (`WebKit/Source/WebKit/qt/Api/qwebpage.h:81-83`), and it fires for ANY plugin that answers `npPalmIsInteractive` true (`PluginViewWebOs.cpp:113-123`). The receiving side proves the purpose: our own adapter files it into `mFlashRects` (`render/adapter/BrowserAdapter.cpp:4315`) and uses it ONLY through `flashRectContainsPoint()` / `mMouseInFlashRect` / `mFlashGestureLock` — i.e. deciding whether a drag pans the page or belongs to the plugin. **No surface id, no handle, no compositor call anywhere in that path.** So "reimplement the flash-rect protocol" is not a rendering route and never was; the rects are gesture arbitration and are worth implementing for INPUT once Flash paints, not before.

  **So the answer to "can Flash be linked against UXP instead of WebKit" is no, and not for the obvious reason.** It is a closed-source prebuilt `.so`, so nothing can be relinked; the *symbol* half is actually the fixable part (our own `NPN_CreateObject`/`NPN_InvalidateRect` could be interposed ahead of libWebKitLuna), but it buys nothing because there is still no drawing path. **Pale Moon and Basilisk really do run Flash — Adobe's GENERIC x86/x86-64 Linux build, on an X11 desktop, through the X11 windowless protocol.** Our UXP carries that same code; it is compiled out only because there is no X here. Two things separate us from them, and neither is fixable by relinking: Adobe never shipped a generic ARM Linux NPAPI Flash (every ARM build was device-specific — Android, webOS, PlayBook), and the TouchPad has no X. Note that bringing up X11 would NOT rescue this binary, since it imports no X11 symbols. **The only path that could ever make THIS Flash draw is reimplementing Palm's plugin-host protocol and emitting `msgAddFlashRects`/`msgRemoveFlashRects` so LunaSysMgr composites its Piranha surface** — the architecture the device was built for, whose browser-side half lives undocumented inside `libWebKitLuna`. That is a research project, not a fix, and it is tracked by the frozen-surface criterion below rather than here.)*
- [x] Platform reality is stated honestly in the docs rather than implied: **Silverlight has no ARM Linux build** (it shipped x86/x64 for Windows/macOS; the Linux implementation was Moonlight, x86-only and long dead), so it is **not achievable on the TouchPad** regardless of our plugin support. The requirement is that the NPAPI *architecture* is generic enough that any platform-appropriate NPAPI binary works — verifiable on the desktop x86_64 build, where such plugins exist. *(2026-08-07 — the Silverlight half stands as written and is unchanged. The generic-architecture half now has the control it was missing and was PARTLY met AT THIS DATE, which is why the box moved to `~` then — **it moved back to `[x]` on 2026-08-09; see the note immediately after this one, which is the current status**: **`render/goanna/test/npapi_test_plugin.c`** is a purpose-built NPAPI plugin — the four entry points, a windowless instance, an async-bitmap-surface paint of one flat colour, every callback announced on stderr as `[jihad-testplugin]` — built by `build/webos-oe/build-test-plugin-arm.sh` (ARM, 9.5 KB, `NEEDED` is libc alone) and `build/desktop/build-test-plugin.sh` (x86_64). Both scripts assert the four entry points are exported, because a plugin missing one is silently skipped by `nsPluginHost` and looks exactly like a plugin that was never found. **On device it loads, initialises, instantiates windowless and paints** — the full trace is in the port status above. It is what proved the three defects were OURS and not Flash's, which is the question this criterion exists to answer. NOTE: `-DXP_UNIX=1` is required to compile against UXP's `npfunctions.h`, which defines `NP_EXPORT` only inside that guard.)*

  **BOX MOVED TO `[x]` 2026-08-09.** The "last hop" that held it at `~` — the plugin's output reaching the screen — was fixed the same day (`0021`+`0022`, see the port status above), and the control plugin has since been extended into a full INPUT oracle: it decodes the Palm event union, logs the event type with its coordinates, flips its fill colour on a key-down and paints a black marker at the pen-down coordinate it was handed. That is what proved the coordinate transform rather than merely asserting it — a device screenshot shows the marker under the tapped point, the box cyan from the keystroke, and a focus ring on the `<embed>`. So the generic-architecture claim now rests on a plugin that loads, instantiates windowless, paints, composites, and **responds to input**, on both the ARM device and the desktop build. The Silverlight half is unchanged and still stands as written.
**PORT STATUS 2026-08-10 — THE BAR MOVED: "a plugin runs" is met, "Flash games are playable" is not.**

Every criterion above is about whether a plugin *works*. The user requirement as of 2026-08-10 is
stronger — *"ensure smooth fps, keyboard support, and audio support in flash ... to make playing
flash games a first class experience"* — and that is a different, mostly-unmet bar. The three
criteria below are new and deliberately start unchecked. What changed under them this session:

- **A device reboot broke Flash entirely, and it was never a code regression.** `dlopen` of the
  device's `libflashplayer.so` deadlocked because libPmLogLib's constructor waits on the POSIX
  named semaphore `PmLogLib`, and glibc 2.21 changed `sem_t`'s value word to
  `(tokens << 1) | has-waiters`. webOS is glibc **2.8** (old layout), our bundle is **2.23** (new),
  and `/dev/shm/sem.PmLogLib` is created once per boot by whichever process gets there first — so
  a system process winning that race makes our `sem_wait` read "0 tokens, waiters pending" and
  block forever. **It is a boot race; do not look for a code change when it reappears.** Fixed by
  interposing that ONE name (`render/goanna/JihadPmLogSem.c`, patch `0025`), in the executable
  because a dlopened plugin resolves its PLT against the global scope. The daemon's plugin scan
  dlopens every plugin IN-PROCESS (`nsPluginFile::GetPluginInfo`: *"Sadly we have to load the
  library for this to work"*), which is why the failure produced ZERO `[jihad-npapi]` lines — the
  first probe prints after the call that hung. See also cavekit-device-build.md R9.
- **Frame rate had a hard ceiling nobody had noticed**: `kJihadRepaintEveryTicks = 4` asked the
  plugin to repaint every 4th 16 ms tick — **~15 Hz by construction, for content authored at 30**.
  Now env-tunable (`JIHAD_PLUGIN_REPAINT_DIV`, default 2 ≈ 30 Hz; patch `0026`). Measured on device,
  single instance: divisor 4 → draw 21.6-24.0 fps / composite 28.4-29.4; divisor 2 → draw 26.9-31.4 /
  composite 27.0-27.4 (matched, best effective rate); divisor 1 → draw 28.5-29.8 / composite
  19.7-19.9 (worse — the two trade off, and the CARD COMPOSITE is what the user sees).
- **Audio WORKS** (2026-08-10, heard on the device speaker with the stock plugin). It is an ALSA
  question, not a Luna or PIpc one; the port was never the blocker and neither was our runtime —
  the test SWF was malformed. See the criterion below. ~~**Not yet CLEAN, though:** MP3 still has
  occasional static (xruns). Patch `0027` should fix it and has not been confirmed by ear.~~
  **CORRECTED 2026-08-10: audio is CLOSED at parity with stock.** The residual per-loop click is
  Flash restarting its MP3 decoder and the STOCK browser does it identically; only
  `jihad-audio-long.swf` (30 s, `--loops 1`) can answer a quality question, because every other
  MP3 asset here clicks once per iteration by design.
- **Frame RATE is met** via patch `0027`, which holds a CPU-governor boost for the life of a
  plugin instance — the ceiling was `ondemandtcl` running Flash at 192-384 MHz of 1188.
  **Frame PACING is NOT met** — see the reopened criterion below. Do not read "frame rate is met"
  as closing the user's "smooth" requirement; average fps is structurally unable to express it.
- **THE UNFINISHED FLASH ITEMS**, recorded in full on their criteria below:
  **(1) keyboard arbitration cannot be tested on this hardware** — there is no keyboard, and the
  synthetic one built for it (`render/goanna/test/uinput_kbd.c`) registers and is opened by both
  `hidd` and LunaSysMgr yet never dispatches a key to the card; the trail continues at
  `HidInputDev` in `/etc/hidd/HidPlugins.xml`. **(2) frame PACING** — the gap histogram is still
  unconcentrated; `0028` and `0029` are live and did not close it. ~~MP3 audio static~~ is closed,
  see above.
- Spotlight delivery is now suppressed by default (see the superseded note above) and several
  frame-path defects were fixed (cavekit-offscreen-rendering.md R8).

- [~] **Flash content holds a smooth, sustained frame rate on the card — the COMPOSITE rate, not the
      plugin's draw rate.**
      *(**2026-08-10, REOPENED from `[x]` — the RATE half is met, the SMOOTH half is not.** Patch
      `0027` genuinely closed "matched, and both at the content's authored rate", and nothing below
      is withdrawn. What it did not close is the word **smooth**: the user's verdict on that build
      was "the pink square is smoother in the STOCK browser", and the gap HISTOGRAM added
      afterwards shows why the fps numbers could not see it — frames were landing at effectively
      random instants across every bucket from under 16 ms to 55 ms while the average sat at a
      healthy 35-42 fps for a 30 fps source. Closing this box on average fps was the error;
      average fps is structurally incapable of expressing this criterion's own word.
      Two fixes have since gone in and the criterion is still not met. `0028` made delivery 1:1
      with the plugin and moved the histogram mode to the 32-39 ms bucket. `0029` then did what
      the open work item asked for — a real PULL, so the daemon asks the plugin for each frame
      instead of sampling it, with the child's own timer standing down on a lease — and it did
      NOT help: average and max improved slightly, the histogram got FLATTER.
      **The measurement that explains it: the pull was not exclusive, and the cause was OUR OWN.**
      Over 123 s the child logged 3988 draws against 3072 host requests. Tagging every
      `AsyncShowPluginFrame` call site then showed the background paths at **exactly zero** — the
      first guess, `RecvUpdateBackground`, is refuted — and the accounting closing to the unit:
      **1856 host requests + 1846 child-timer invalidates = 3702**, with the child logging
      `HOST -> SELF -> HOST`. The daemon waits 250 ms before re-asking for a late frame while the
      child's host-drive lease was only 150 ms, so a single hitch dropped the lease and restarted
      the second clock. Fixed by making the lease longer than the daemon's worst-case gap; the
      clock log now shows one `-> HOST` and no fallback.
      **THEN THE REAL FINDING, and it falsifies patch `0026`'s founding premise.** With the lease
      fixed, 1344 host requests still did not account for 2662 invalidates. Splitting the counter
      at the funnel by whether `JihadPalmRepaintTick` is on the stack settles it:
      **`hostinval` ≈ 27/s against `plugininval` ≈ 22/s — Flash invalidates itself constantly.**
      Three sessions believed otherwise because `gJihadNPNInvalidateRectCalls`, the counter `0026`
      reads to decide "the plugin never asks to be repainted", is **structurally blind**: it only
      sees calls that bind the `NPN_InvalidateRect` SYMBOL, while an ordinary NPAPI plugin calls
      through the NPNetscapeFuncs table into `PluginModuleChild::_invalidaterect` (`:986`,
      `:1302-1310`), which never touches the interpose. It can read 0 forever while the plugin
      invalidates 22 times a second.
      **So this criterion's remaining defect is TWO PRODUCERS BEATING** — ours and Flash's, both
      funnelled into one coalescing path (~49 invalidates/s collapsing to ~32 draws/s). The next
      experiment is to measure Flash with BOTH drivers off, which needs one more child-side kill
      switch: `JIHAD_PLUGIN_FRAME_MS=0` stops the pull but lets the lease expire and the child's
      timer take over, which is just `0026` again. If Flash alone holds the authored rate, the
      host clock should be DELETED for plugins that self-invalidate, not tuned.
      **A/B RUN 2026-08-10, and it settles one of the two questions: KEEP the pull.** A
      `JIHAD_PLUGIN_PULL=0` switch was added (daemon side, disables only the request and leaves the
      publish policy alone — `JIHAD_PLUGIN_FRAME_MS=0` cannot answer this because it also drops the
      publish gate to the 8 ms floor). As predicted it did NOT isolate Flash — the lease expired and
      the child's timer took over, so the run measures **patch `0026` versus patch `0029` on the
      same build**, which is worth having on its own:*

      | | `0026` child timer | `0029` host pull |
      |---|---|---|
      | daemon done | 24.3-29.4 fps | 30-33 fps |
      | gap hist `40-55` ms | 11-21 of ~52 | 5-13 of ~64 |
      | gap hist `56+` ms | 2-10 | 0-2 |

      *The pull is better on both rate and spread than the child timer, so `0029` is better than
      `0026` — which was a live question before this run. **Note when reading `show req:`:
      `hostinval` counts everything reaching `JihadPalmRepaintTick`, which is the pull AND the
      child's timer — it is not "the pull" on its own.**

      **FLASH ALONE, MEASURED 2026-08-10 with `JIHAD_PLUGIN_PULL=0` AND `JIHAD_PLUGIN_SELF_DRIVE=0`.
      Clean isolation — `hostinval` is exactly 0, `plugininval` 48-55 per 2 s — and this is the
      result the whole investigation was for:**

      | config | done | `<16` | `16-23` | `24-31` | `32-39` | `40-55` | `56+` |
      |---|---|---|---|---|---|---|---|
      | `0026` child timer | 24.3-29.4 fps | 0-1 | 0-6 | 8-13 | 6-28 | 11-21 | 2-10 |
      | `0029` pull, both drivers on | **30-33 fps** | 1-5 | 9-12 | 10-16 | 24-33 | 5-13 | 0-2 |
      | **Flash alone** | 23.8-27.4 fps | **0** | **0-1** | 3-9 | 17-38 | 12-19 | 1-8 |

      ***The configuration with the HIGHEST average fps has the WORST bunching, and the one with the
      LOWEST has NONE.*** *That is this kit's own thesis — average fps cannot express "smooth" —
      demonstrated on one build in one sitting. Flash on its own produces **evenly spaced but slow**
      frames: not a single gap under 16 ms across the whole run, because there is only one producer
      and nothing to interleave with. Adding our pull buys ~6 fps of average and pays for it by
      inserting frames BETWEEN Flash's own, which is precisely what populates the `<16` and `16-23`
      buckets.*

      ***So the two-producer design is the defect, not a tuning problem***, *and "make the pull
      smarter" is the wrong direction — a second producer cannot be phase-locked to a clock it does
      not own. The remaining question is no longer "how do we pace the pull" but **"why does Flash
      invalidate at ~25/s for 30 fps content"**, which is a single-producer question and probably a
      CPU one (patch `0027` already pins the clock; check whether it is engaged during these runs).*

      ***RECOMMENDATION, for a human to confirm by EYE, since that is the only instrument that has
      ever detected this defect:*** *run the device in Flash-alone and look at it. If evenly-spaced
      25 fps reads better than bunched 32 fps — which the histogram says it should — then the host
      frame clock should be DELETED for self-invalidating plugins, `0026` and `0029` both, and the
      rate question pursued separately. **The device has been LEFT in the Flash-alone configuration
      for exactly this comparison** — see the device-state section of docs/PICKUP.md for the one-line
      revert.)*
      **This criterion is met when the gap histogram concentrates**, not when fps looks right: a
      clear mode at the authored period with the `<16 ms` and `40+ ms` buckets near empty, on a
      clean-restart run of `jihad-anim.swf`. Record the histogram, not min/avg/max.)* *(2026-08-10, THIRD BUFFER LANDED — the binding constraint is gone.
      Two shared buffers meant the daemon could only paint into the one the adapter was not
      holding, so every frame cost a full adapter round trip: `deferred` climbed 11 → 38 → 77 as
      the plugin sped up while completed paints stayed pinned near 26/s. A third buffer
      (`asyncCmdSetExtraBuffer`, YAP 0x1600 — the one additive change to an otherwise frozen
      contract, see cavekit-ipc-contract.md R1) gives the daemon a free buffer while the adapter
      still owns the displayed one. Measured on the SHIPPED PDK adapter build, single instance:
      **`deferred` 0 in every window, `wanted` == `done` (57/57, 65/65, 55/55), composite
      27.1-32.2 fps, frame gap avg 31-36 / max 52-57 ms** — against max 127 ms before, and the
      gap MAXIMUM is what reads as stutter. Two earlier fixes fed into this: the host repaint
      divisor (~15 Hz → ~30 Hz) and a per-tick `GetScrollXY` that had cost ~17% of the daemon
      tick rate. Remaining gap to a locked 30: Flash's own draw rate (25-32 fps) on 2011 ARM,
      which is the plugin pacing itself, not a pipeline refusal.)* The number the user perceives is the card composite; a plugin drawing 30
      fps into a card compositing 20 is 20 fps of animation, and a card compositing 29 against a
      plugin drawing 24 wastes a fifth of its paints. They must be matched and both at the content's
      authored rate. *(2026-08-10, single instance on `jihad-anim.swf` (30 fps authored): draw
      26.9-31.4 fps, composite 27.0-27.4 fps, frame gap min 15-18 / avg 36 / max 65-68 ms. Better
      than the 15 Hz ceiling it started at and now matched, but NOT smooth 30, and the user reports
      it as "skippy". The remaining bottleneck is measured and daemon-side, not Flash: the daemon
      tick runs ~42/s rather than the 62/s a 16 ms tick allows, and ~25% of paint attempts are
      `deferred` because the adapter still holds the target buffer — there are only TWO shared
      buffers, both allocated by the adapter, so one adapter round trip gates every frame. A third
      buffer costs ~12.6 MB of SysV shm per page instance. Read the `paint pipeline:` /
      `frame gap ms min/avg/max` counter pair; the gap SPREAD is the thing that reads as stutter,
      not the average.)*
      *(**2026-08-10, CAUSE FOUND AND A FIX DEMONSTRATED — still `~` only because it is not yet
      productized.** The remaining gap was never the pipeline and never Flash: it is **CPU
      frequency scaling**. Sampling `scaling_cur_freq` once a second during `jihad-anim.swf`
      showed Flash running at **192-384 kHz-scaled steps out of a 1188000 maximum**, snapping
      back to 1188000 the instant the page was navigated away. The governor is Palm's
      **`ondemandtcl`**, whose stock `up_threshold=95` means it only ramps when a single core
      exceeds 95% load — and the daemon (~26%) plus plugin-container (~11%) spread over two cores
      never approach that, so passive playback sits at the floor. The system was only 69% busy;
      this was never a capacity wall. Setting `up_threshold=40` and `sampling_rate=50000` in
      `/sys/devices/system/cpu/cpufreq/ondemandtcl/` pins 1188000 and gives, single instance on
      the 30 fps authored asset: daemon paints 24.0-28.7 → **31.4-33.0 fps**, card composite
      20.4-27.5 → **30.7-35.2 fps**, frame gap avg 34-42 → **30-31 ms** and max 63-81 → **49-56
      ms**. `deferred` was 0 before and after. That meets "matched, and both at the content's
      authored rate"; what remains is ownership — these are SYSTEM-WIDE tunables, so the daemon
      must apply them only while a plugin instance is alive and restore the stock values (read,
      not hard-coded) on teardown AND from upstart `post-stop`, or a crashed daemon leaves the
      whole device tuned for battery burn. Scoping it to the upstart job instead would be
      permanent, since all three daemons run from boot; the correct hook is plugin-instance
      lifetime in `PluginModuleParent`, i.e. a UXP patch. **DONE — patch `0027` does exactly
      that, and the whole cycle is device-verified:** plain page 95/200000 (stock) → Flash
      playing 40/50000 with the clock pinned at 1188000, daemon paints **33.9-35.4 fps**, card
      composite **30.6-32.5 fps**, frame gap avg **28-29** / max **45-49 ms** → navigated away,
      restored to 95/200000 with `cpu boost release` logged. The boost is keyed to the INSTANCE,
      not a bare count, because a plugin that fails to start also runs through `NPP_Destroy` and
      a counter would release the boost out from under a plugin still playing; it is taken only
      on the single success path of `NPP_NewInternal`, since every failure return above it either
      deletes the instance itself or hands it to `NPP_Destroy`. It is ALSO released from
      `ActorDestroy`, because on this port navigating away from Flash ends in the child's own
      `mozalloc_abort` so the orderly `NPP_Destroy` often never runs — measured: the first build
      leaked the boost in exactly that way and left the device tuned. The upstart job restores
      stock values from `post-stop` as a last resort (verified on a real `stop jihad`), so a
      crashed daemon cannot strand the tuning. **HARD-WON, do not repeat:** the
      obvious approach — writing `scaling_governor` to `performance` — DEADLOCKS cpufreq in the
      kernel. Every subsequent reader of any cpufreq node blocks in unkillable D state, including
      the daemon, `powerlog` and the restore script itself, so the governor never gets put back
      and the daemon stops painting. Only a reboot clears it, and a clean `reboot` will not run
      because init waits on the D-state tasks — it takes `sync; reboot -f`. Governor TUNABLE
      writes are safe; governor SWITCHES are not. The device was left on stock values.)*
- [x] **Flash produces sound.** *(2026-08-10 — **DONE, HEARD BY A HUMAN on the TouchPad speaker**,
      with the STOCK, UNPATCHED `libflashplayer.so`. **The port never broke audio. The TEST ASSET
      was broken**, and every "silent" measurement below was taken against a SWF that could not
      have made a sound in any player. `make-audio-swf.py` emitted `DefineShape` and `DefineSound`
      with the SAME character ID 1; a SWF has one character dictionary shared by shapes and sounds,
      so the sound redefined an ID already taken, the player kept the shape, `StartSound 1`
      resolved to a shape, and the sound was dropped SILENTLY — no error, no device opened, no
      mixer thread. All three assets (PCM, MP3 mono, MP3 stereo) shared the bug, which is exactly
      why they "behaved identically" and made this look like a player-side gate. Fixed by giving
      the sound its own ID (`SOUND_ID = 2`); the corrected SWF immediately produces the full ALSA
      bring-up that had been missing throughout — `open("/usr/share/alsa/alsa.conf")`,
      `access("/etc/asound.conf")`, `open(".../libasound_module_pcm_pulse.so")`,
      `open("/usr/lib/libsndfile.so.1")` — and plugin-container goes from 3 to 5 threads.
      **Flash's real audio path, for the record:** ALSA by runtime `dlopen`, which is why nothing
      shows in `NEEDED` or the import table. Loader **0x2f7bd0** (`dlopen("libasound.so")`, falls
      back to `"libasound.so.2"`, ~50 `dlsym`s, NULL-checks every one); PCM open **0x2f71e0**
      (vtable slot +8 at vptr 0x84e440, ctor 0x2f75f0) which sets access/format/rate/channels,
      a 500 ms buffer / 20 ms period, then `pthread_create`s the mixer, and whose FALLBACK device
      name is **`plughw:0,0`** (vaddr 0x7b54c8). `0x2fbf08` is a separate availability probe that
      dlopens libasound unconditionally at startup — that is the dlopen every earlier trace saw,
      and it proves nothing about playback. Audio is NOT a Luna call and NOT PIpc.
      **The previous "single comparison" gate was the wrong subsystem entirely and is retracted.**
      `0x70ee0`/`0x73020` are the hardware H.264 decoder: 0x73020's strings are
      `libmm-omxcore.so`/`libOmxCore.so` resolving `OMX_Init`/`OMX_GetHandle`/
      `OMX_GetComponentsOfRole`, and 0x70ee0 passes `"video_decoder.avc"` (0x760cec) and
      `strncmp`s candidates against `"OMX.Nvidia"` (0x760d00) with 128-byte
      `OMX_MAX_STRINGNAME_SIZE` entries. `+24` is a codec enum and it DOES equal 7 — the factory
      at 0x5bff0 returns NULL unless asked for type 7 and the ctor at 0x72a30 stores that 7
      (`72a94: str r4,[r8,#24]`). The old reading had the gate inverted as well as misplaced.
      **Two candidate gates were nopped on-device and BOTH changed nothing**, which is what
      finally redirected suspicion to the asset: the sample-count check
      (`2f7234: cmp r1,#31 / ble`) in the PCM open, and the `audioEnabled` check
      (`19ba54: beq` on `sound+0x270`). Both patched builds have been removed; the plugin on the
      device is stock, md5 `14c20ba9fc2183a93648c70488e0ec8e`.
      **Still true and worth keeping:** our runtime was never the blocker —
      `render/goanna/test/alsa_probe.c` plays an audible square wave under both the device loader
      and `./ld-2.23.so --library-path $HL`; device config is `/etc/asound.conf` with
      `pcm.!default { type pulse }` and `audiod` holding `/dev/snd/*`, and Flash coexists with it.
      One unrelated bug was fixed on the way: `kind not registered:
      'com.palm.app.flashplayer.prefs:1' (-3970)` is now registered (owner
      `com.palm.flashgraphics`). **METHODOLOGY, twice load-bearing:** assert the card is connected
      — inject a known page and grep for `inject: no page` — before believing any
      container/draw/thread count; and **parse a generated test asset back out before believing a
      negative result from it**, because a malformed asset is indistinguishable from a broken port.
      Attach `strace -f` to the DAEMON, not the container: the container is spawned per navigation
      and attaching after it appears misses the whole audio bring-up, which reads as "no ALSA".
      RE traps that cost real time: the code is ARM, not Thumb; string addresses are PIC
      (`add rX, rBase, <negative literal>` with base = the GOT, 0x85897c), so absolute-address and
      `movw`/`movt` searches both find ZERO refs — read the literal pool and add the base; take
      PLT stub addresses from `objdump -d --section=.plt`, which labels them; and a function's
      entry can be one instruction BEFORE the `push`, which made an earlier caller scan report
      zero callers.)*
      *(**AUDIO QUALITY: CLOSED 2026-08-10 at parity with the stock browser. All three defects
      were in the TEST ASSETS; none was in the port.** The chain took three rounds because each
      fix uncovered the next, and every one of them impersonated a different port-level fault:
      **(a) FIXED — loop gaps.** `SeekSamples` was written as 0, so every iteration of a looped
      MP3 replayed the encoder delay plus frame padding. A 1 s loop chopped at ~1 Hz and the user
      described it as "morse code"; the period tracked the sound's length (confirmed by ear at 1 s
      and again at 5 s), which is exactly how a loop-boundary gap is told apart from an underrun.
      Now `SeekSamples = 1105` (LAME 576 + decoder 528+1). PCM never showed this because it has no
      encoder delay — a clean PCM tone beside a chopping MP3 one is NOT a decode fault.
      **(b) FIXED — stacked voices.** `StartSound` carried `SyncNoMultiple = 0` on a 60-frame /
      30 fps timeline with no `Stop`, so the timeline looped every 2 s and re-fired the tag, and
      the player started ANOTHER 200-loop voice each pass — ~15 of them by 30 s, summed. Phase-
      offset copies comb-filter and the sum clips, heard as crackle. Now `SyncNoMultiple = 1`.
      The PCM asset hid this almost perfectly: 2 s is exactly 882 whole periods of a 440 Hz tone
      at 11025 Hz, so its copies stacked IN PHASE and only got louder — which is why PCM "passed
      by ear" and MP3 did not, and why this read as a codec problem for two sessions.
      **(c) NOT OURS — the residual click is Flash restarting its MP3 decoder at every loop
      point.** Proven three independent ways on 2026-08-10: the click RATE tracks the sound's
      length (1 s asset ≈ 1/s, 5 s asset ≈ 1/5 s — the user's own A/B); the PCM asset never clicks,
      because it needs no decoder; and **the STOCK webOS browser playing the identical SWF clicks
      identically** (`/proc/<BrowserServer>/maps` confirmed `libflashplayer` + libasound/pulse
      mapped, with our card navigated away so only stock was sounding). Same Flash binary, same
      device, different host, same artifact — so it is Flash's MP3 loop behaviour on webOS 3, and
      R7 is at parity with the platform. The asset's own loop is arithmetically seamless: the
      decoded stream has ZERO discontinuities and the sample-to-sample step at the wrap (3980) is
      the normal step at a zero crossing for 440 Hz at 22050 (predicted 3903).
      **The clock was exonerated first**, and that mattered: the static was unchanged with patch
      `0027` live and the clock pinned at 1188000, which is what removed the frame-rate work from
      the suspect list and forced the search back onto the assets.
      **To listen to MP3 audio QUALITY, use `jihad-audio-long.swf`** — 30 s, `--loops 1`, so it has
      no loop point at all. Every other MP3 asset here is a LOOPING asset and will click once per
      iteration no matter how healthy the port is.)*
- [~] **Keyboard input reaches a focused plugin and does not simultaneously drive the card chrome.**
      A Flash game focuses no HTML editor, so the editor-focus test the adapter used cannot describe
      it. *(2026-08-10 — key events themselves already reach the plugin at the right codes; what was
      wrong is arbitration. `handleKeyDown`/`handleKeyUp` returned `bEditorFocused` alone, so every
      arrow/WASD press during a game ALSO drove the chrome. Now gated on `mPageFocused` — ported from
      Atlas BrowserAdapter `ce199bc`, Apache-2.0, credited in NOTICE — and extended with
      `mFlashGestureLock`, which is already the latch meaning "a tap landed inside a plugin rect and
      none has landed outside since", i.e. exactly "the plugin has focus". **Adapter-side, so it
      needs a full `.ipk` install to take effect and is NOT yet device-verified.** Verifying it also
      needs an interactive SWF. **That asset now exists and the content half is PROVEN:
      `render/goanna/test/make-key-swf.py` emits real AVM1 — a sprite resting on a GREEN frame
      with an `onClipEvent(keyDown)` handler of GotoFrame(2)+Stop, turning the stage RED. Device
      2026-08-10, single instance: before the key green=11768/red=0, after `key 65`
      green=18/red=11750, with `palm key 0x8 raw=65 handled=1` in the log. So the keystroke does
      not merely arrive — Flash's own bytecode runs on it.** Two traps cost real time and are
      recorded in the generator: `0x00020000` is ClipEventKEYPRESS, not KeyDown (0x00000040), and
      it carries a trailing KeyCode byte, so using it mis-parses the record and the SWF draws
      NOTHING rather than just ignoring keys; and a sprite with no Stop on frame 1 loops
      green/red forever, which looks exactly like a key response and is not one. Still open: that
      the CARD does not also consume the key — that is client-side, so it needs a human at the
      device or a chrome-side probe.)*
      *(**2026-08-10 — attempted properly and BLOCKED ON THE HARDWARE, which is itself the
      finding.** The chrome half can only be exercised by a key that LunaSysMgr dispatches,
      because `BrowserAdapter::handleKeyDown` runs inside LunaSysMgr and its RETURN VALUE is the
      whole arbitration. The daemon's `key` inject command delivers straight to the page and
      bypasses the adapter, so the existing "keys reach Flash" proof can never say anything about
      the chrome. **This hardware has no keyboard to press:** `/proc/bus/input/devices` lists only
      `gpio-keys`, `pmic8058_pwrkey` and `headset`, and webOS's on-screen keyboard is drawn by
      LunaSysMgr and delivered in-process rather than through `/dev/input`. So a human at the
      device cannot press a letter on a Flash page either — the real-world scenario this criterion
      describes needs a Bluetooth keyboard. Built `render/goanna/test/uinput_kbd.c` (PDK, device
      glibc, no bundled loader) to synthesise one: `/dev/input/uinput` EXISTS (char 10,223,
      built-in — `/proc/misc` lists it, no module), the device registers correctly as
      `N: Name="Jihad Test Keyboard" ... event3`, and **both `hidd` (`/usr/bin/hidd -f
      /etc/hidd/HidPlugins`) and LunaSysMgr open `/dev/input/event3`** — so it is being read.
      Despite that, neither `KEY_A` nor `KEY_DOWN` produced ANY key line in the daemon log and
      `window.pageYOffset` (read back with `gettext #sy` off a purpose-built tall page,
      `jihad-keyarb.html`) never moved. Keys are read by hidd and never dispatched to the card.
      `/etc/hidd/HidPlugins.xml` shows the generic route is the `HidInputDev` plugin
      (`/usr/lib/libhidinputdev.so`, `/var/run/hidd/InputDevEventSocket`); why it does not forward
      a synthetic keyboard is the next question, and it is a LunaSysMgr/hidd input-routing
      investigation rather than a browser one. NOTE the assumption that cost time and is probably
      wrong: arrow keys were used as the observable on the theory that the chrome scrolls with
      them, which a touch OS may simply not do — the adapter's own comment says the leak showed up
      as keystrokes going to the ADDRESS BAR, so the right observable is a screenshot of the URL
      field, not scroll position. The device was left clean: the virtual keyboard is destroyed and
      the process is gone.)*
      **2026-08-15 (T-107) — the OBSERVABLE is rebuilt around the ADDRESS BAR and the PROCEDURE is
      written out below, so that when a key CAN be delivered the answer is readable in one
      screenshot. Nothing here ran on the device: novacom was down this session. This is the test
      design and the source facts it rests on, not a result.**

      **Why `window.pageYOffset` is the wrong observable — two reasons, and the second is the
      damning one.** (a) Recorded 2026-08-10: a touch OS need not scroll on arrow keys at all, so
      "it did not move" carries no information — it reads identically to a pass, to a dead
      keyboard, and to a chrome that simply has no arrow-key binding. (b) Newly found in source
      and worse: the very input that engages the plugin ALSO scrolls the page. The latch-setting
      double-tap falls through to `prvSmartZoom` (`render/adapter/BrowserAdapter.cpp:1586`), so
      `pageYOffset` changes for a reason that has nothing to do with any keystroke. The observable
      therefore produced both false negatives and false positives. `#sy` is KEPT on the test page,
      but demoted from the answer to an instrument: it measures PANNING, which is how the latch is
      confirmed in step 6 below.

      **What actually sets `mFlashGestureLock`, exactly.** Two sites set it true and only two:
      `handlePenDoubleClick` (`BrowserAdapter.cpp:1583`) and `js_smartZoom` (`:3119`). Five sites
      clear it: `handlePenDown` when the tap is outside a plugin rect (`:1432`), a double-click
      outside one while locked (`:1576`), `doGestureStart` (`:1868`), `sendGestureStart` (`:1993`)
      and `js_smartZoom` outside a rect (`:3113`). Three consequences the earlier wording in this
      kit got wrong or left implicit:
      **(1) A SINGLE TAP INSIDE THE PLUGIN DOES NOT SET THE LATCH.** `handlePenDown` only ever
      clears it. It takes a DOUBLE-TAP inside the plugin rect. Where this criterion and
      cavekit-input-bridging.md R7 say the latch is "set when a tap lands inside a plugin rect",
      read *double*-tap; the behaviour described (drag goes to the plugin, keys are swallowed for
      it) is unchanged, but the gesture that arms it is not a single tap.
      **(2) Only a real LunaSysMgr-dispatched pen event can do it.** The daemon's inject channel
      (`render/browserserver/JihadBrowserServer.cpp`, the `click`/`clickid`/`clickoff`/`dblclickid`/
      `touch`/`key` commands) calls `BrowserPage` and `jihad::Debug*` directly inside the DAEMON
      process; it never enters LunaSysMgr, so it can neither set the latch nor reach
      `handleKeyDown` whose return value is the entire arbitration. An inject click is not a
      substitute for a finger here.
      **(3) There is no scriptable route today.** `js_smartZoom` is card-JS-callable as
      `adapter.smartZoom(x, y)` and would set the latch, but no in-tree card calls it (grep of
      `app/`, `app-mochi/`, `app-mojo/`: zero callers; the only `smartZoom` hits are
      `third_party/enyo-layout/imageview/source/PanZoomView.js`, unrelated). So step 5 needs a
      HUMAN DOUBLE-TAP. A three-line card-JS shim calling `adapter.smartZoom` would make it
      scriptable from the dev loop — that is a card change and a separate decision, noted here so
      the option is not rediscovered.

      **What the SWF colour does and does not prove.** `asyncCmdKeyDown` runs UNCONDITIONALLY,
      above the return (`:1613`), so the daemon and the plugin see the key whichever way the gate
      goes. `jihad-key.swf` going green→red therefore cannot distinguish "arbitration correct"
      from "arbitration leaking" — it never could. Its job in this procedure is the other split:
      it separates a real pass from NO KEY HAVING BEEN DELIVERED AT ALL, which is precisely the
      outcome the 2026-08-10 uinput attempt got and which the old observable rendered
      indistinguishable from success. Note also that the SWF is a ONE-SHOT per instance
      (`GotoFrame(2)` + `Stop`); only a fresh navigation resets it to green.

      **The test page** (`build/webos-oe/stage-test-pages.sh`, staged to
      `file:///tmp/jihad-keyarb.html`; `/tmp` is volatile, re-stage after every reboot) carries
      four readouts, three of them added for this procedure: `#id` — a magenta `JIHAD-KEYARB`
      banner, the wrong-card guard; `#sy` — `pageYOffset`, the pan instrument; `#kc` — count and
      last keyCode from a capturing window `keydown` listener, i.e. keys that reached the CONTENT,
      reset by a reload; `#fo` — `document.activeElement`, which measures the editor confound
      directly. All four are readable without a screenshot via the daemon's `gettext <selector>`.

      **PROCEDURE (2026-08-15).**
      Preconditions.
      P0. A key LunaSysMgr actually dispatches: a paired Bluetooth keyboard, or T-108's hidd fix.
      The daemon `key` inject is excluded by construction — see consequence (2) above.
      P1. The installed adapter must be a build containing the gate. Check it by PROVENANCE — md5
      of `/usr/lib/jihad/enyo/BrowserAdapterImpl.so` against the known build
      (`0410a348042af58a7e711b495db69833` for the current source) — never by
      `strings | grep FlashGestureLock`, which returns a confident and meaningless 0 (T-106).
      P2. Clean restart, and exactly ONE jihad card in existence before launching.
      P3. Test key: a LETTER (`A`/keyCode 65). NEVER Escape — `:1614` exempts `ESC_KEY`
      unconditionally, so Escape always falls through and always looks like a leak.
      Run.
      1. Launch the card at `file:///tmp/jihad-keyarb.html`, screenshot (`luna-send` →
      `takeScreenShot`). **WRONG-CARD GUARD:** `takeScreenShot` captures the FOREGROUND card, and
      the double-launch dance routinely makes that the wrong one. The shot must contain the magenta
      `JIHAD-KEYARB` banner AND a URL field reading `file:///tmp/jihad-keyarb.html`. If it does
      not, bring the right card forward and re-shoot; every later comparison is against a shot that
      passed this guard. (The URL field is in every shot: no in-tree card calls
      `js_setHeaderHeight` (`:3489`), so `m_headerHeight` is 0 and the address bar is a fixed Enyo
      toolbar that does not scroll away.)
      2. Confirm the starting state: SWF GREEN, `gettext #kc` → `0/-`, `gettext #fo` → `BODY`. If
      `#fo` names `box`, the page's own `<input>` holds focus, `bEditorFocused` satisfies the gate
      on its own, and the plugin half is NOT under test — tap the banner to blur it and re-check.
      3. PHASE A, the positive control — prove a leak would be VISIBLE. Tap the card's URL field
      once (it focuses and selects all). Press the test key once. Screenshot. Expected: the URL
      text is replaced by the single character and the URLSearch suggestion list opens, while the
      SWF also goes RED and `#kc` reads `1/65`. **If the URL field does not change here, the run is
      INVALID, not a pass** — read `#kc` to say which: still `0/-` means no key reached the device
      at all; `1/65` means the adapter swallowed a key it should not have (check the `mPageFocused`
      value in `handleFocus`'s `g_message`, `:1360`, via `palm-log` — it is the one part of this
      state machine that does log).
      4. Reload the page. This is mandatory between phases: the SWF's green→red is a one-shot and
      `#kc` resets with the document. Re-run the step-1 guard.
      5. Set the latch: **DOUBLE-TAP inside the SWF rect** (single tap will not do it — see
      consequence (1)). Expect the page to smart-zoom; that is `:1586` and is normal.
      6. Confirm the latch is set. Nothing logs it — `render/adapter/Debug.h:31` leaves `DEBUG`
      commented out, so every `TRACEF`, including `"Updating flashGestureLock status to"`, compiles
      to `(void)0`. Use the behavioural readback instead: drag a finger starting INSIDE the SWF
      rect; with the latch set the card must NOT pan, because `handlePenMove`'s `passToFlash`
      requires `mFlashGestureLock` (`:1507`). Calibrate `#sy` first if in doubt — the same drag
      started OUTSIDE the rect must pan and must move `gettext #sy`. Between here and step 8, do
      not pinch, flick, or tap outside the plugin: each of those clears the latch (`:1868`,
      `:1993`, `:1432`).
      7. Take the BASELINE screenshot NOW — after the double-tap, not before — so step 5's
      smart-zoom is outside the comparison entirely.
      8. Press the test key once. Screenshot.
      9. Compare the two shots' URL FIELD ONLY.
      Reading the result.
      - URL field identical to baseline, no suggestion list open, SWF GREEN→RED, `#kc` `1/65` →
      **ARBITRATION CORRECT.** This is the pass, and all four clauses are required.
      - URL field shows the character (or a caret, or the suggestion list opened), SWF RED →
      **LEAK.** `handleKeyDown` returned false with the latch set.
      - SWF still GREEN, `#kc` still `0/-`, URL unchanged → **NO KEY WAS DELIVERED.** Not a pass.
      This is the 2026-08-10 outcome, and distinguishing it from a pass is the whole reason the SWF
      and `#kc` stay in the picture.
      - SWF GREEN but the URL changed → the key reached the card only and the adapter never
      forwarded it; a different defect, in `handleFocus`/dispatch rather than in the gate.
      - `#fo` reading `box` at any point → INVALID: `bEditorFocused` carried the gate and the latch
      was never under test.

**Dependencies:** cavekit-offscreen-rendering.md (R2, R5, R6, R8), cavekit-input-bridging.md (R5, R7), cavekit-device-build.md (R8, R9)

### R8: The XPI implementation is validated against Pale Moon and Basilisk
**Description:** Extension support is compared against the two UXP browsers that ship it correctly, rather than reinvented. User requirement, 2026-08-01: *"when you do your adversarial review for xpi extension support make sure you compare it to how basilisk and pale moon do it correctly."*
**Acceptance Criteria:**
- [x] The adversarial review of R1–R6 explicitly diffs our approach against **Pale Moon** and **Basilisk** — both are checked out at `../ref-forks/{Pale-Moon,Basilisk}` and both ship the same UXP-era XPI stack we are targeting, so they are the authoritative reference for what "correct" means here.
- [x] Specifically compared, with findings recorded per item: how they populate application identity (`nsIXULAppInfo`, and whether they rely on `gAppData` from `application.ini` — the exact thing our embedding lacks); their extension install locations and `extensions.enabledScopes` defaults; their signature policy (`xpinstall.signatures.required`) and why; how `about:addons` is registered and reached; and their NPAPI plugin enablement and windowless handling for R7.
- [x] Every place we deliberately DIVERGE from both is justified in writing (embedding vs. full XRE app, headless vs. real widget, three independent variants vs. one browser) — divergence is expected, unexplained divergence is a defect.
- [x] Anything they do that we simply MISSED is filed as a finding, not silently absorbed.

**R8 DONE 2026-08-02** — 9 findings; see context/impl/impl-r8-palemoon-basilisk.md. Headline: R3 is
structurally impossible for any real extension until the Pale Moon `UXP_APPCOMPAT_GUID` mechanism is
built in (our frozen GUID is named by zero extensions), and R7 is much further out than the kit
assumed — windowless NPAPI does not exist in a cairo-headless build and must be ported, not merely
enabled. Signature policy needs no work at all: UXP builds no enforcement.
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
- 2026-08-03: R2 gains an explicit tools-menu criterion, pointing at
  cavekit-offscreen-rendering.md **R7** (engine popups reach the user) rather than leaving the
  `<menupopup>` work tracked only in impl notes. R3's card-side blocker is gone — the XPI confirm
  prompt was waiting on a proven card dialog-reply path, and the restored card dev loop plus the
  shipped `<select>` popup provide one — so wiring the authored `amIWebInstallPrompt` observer is
  now ordinary work rather than a dependency.
- 2026-08-01: Initial draft. Created when the user required `about:addons` + XPI support to work,
  immediately after the on-device diagnosis showed the daemon's SIGSEGV was the missing
  `nsIXULAppInfo` that the add-on manager depends on — making the crash fix and the feature the same
  piece of work. Verified while drafting that the entire add-ons UI, the XPI install UI and the
  `about:addons` redirector already ship in the device bundle.
