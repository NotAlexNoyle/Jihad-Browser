---
created: "2026-08-01"
last_edited: "2026-08-01"
---

# Implementation: application identity (`nsIXULAppInfo`) — the P0 crash fix

Implements `cavekit-addons-extensions.md` **R1**. Read that kit first; this file records HOW, the
dead ends, and what is still device-gated.

## What the daemon was missing

`XRE_InitEmbedding2` is the embedding entry point: there is no `application.ini`, so UXP's
`gAppData` is never set. UXP still registers its stock `nsXULAppInfo` (static module `Apprunner`,
`toolkit/xre/nsAppRunner.cpp`), so `@mozilla.org/xre/app-info;1` *resolves* — but its interface map
is conditional:

```cpp
NS_INTERFACE_MAP_ENTRY_CONDITIONAL(nsIXULAppInfo, gAppData || XRE_IsContentProcess())
```

so `QI(nsIXULAppInfo)` fails and the service exposes `nsIXULRuntime` **only**. `Services.jsm`
swallows exactly that (`getService(nsIXULRuntime)`, then a QI whose `NS_NOINTERFACE` is ignored), so
`Services.appinfo` exists with every `nsIXULAppInfo` attribute `undefined`.

`AddonManager.jsm:769-779` then does:

```js
try { oldAppVersion = Services.prefs.getCharPref(PREF_EM_LAST_APP_VERSION);   // throws: fresh profile, no default arg
      appChanged = Services.appinfo.version != oldAppVersion; } catch (e) { }
if (appChanged !== false)                                                     // undefined !== false
  Services.prefs.setCharPref(PREF_EM_LAST_APP_VERSION, Services.appinfo.version);   // undefined -> null char*
```

`NS_ERROR_ILLEGAL_VALUE` is `NS_ERROR_INVALID_ARG`; it comes from `SetCharPrefInternal`'s
`NS_ENSURE_ARG(aValue)` — i.e. it is precisely the "value is null" signature, confirming
`Services.appinfo.version` was undefined. Note the first `getCharPref` has **no default argument**,
so on a fresh profile the `try` always throws before appinfo is even reached — meaning this path is
taken on *every* first run regardless.

On desktop that is survivable noise. On the TouchPad the daemon **SIGSEGV**s in the same window
(`XRE_NotifyProfile()` → `DoStartup()`), 117 upstart respawns, exit 139.

## The part that was not obvious: contract remapping does not reach JS

The first implementation registered `JihadAppInfo` under a fresh CID and re-pointed
`@mozilla.org/xre/app-info;1` at it. `do_GetService` returned our object at every probe point — and
**the add-on manager still read `undefined`**. Instrumenting `GetVersion()` proved JS never called
our object at all.

Two independent caches defeat contract remapping:

1. **`nsJSCID` pins the CID.** `Cc["@mozilla.org/xre/app-info;1"]` builds an `nsJSCID`, and
   `nsJSCID::NewID` (`js/xpconnect/src/XPCJSID.cpp`) resolves the contract string to a CID
   *eagerly* (`ContractIDToCID` → `InitWithName(*cid, str)`). From then on `getService()` uses the
   pinned CID and never consults the contract table.
2. **`Services.appinfo` is a `defineLazyGetter`** — the first access anywhere caches the object for
   the life of the process. (`AddonManager.jsm:12-15` carries a comment about exactly this hazard.)

So the fix has two halves:

- **Own the stock CID**, `{95d89e3e-a169-41a3-8e56-719978e15b12}`, not a new one. The component
  manager rejects a duplicate CID with `NS_ERROR_FACTORY_EXISTS`, so the stock factory is fetched
  with `GetClassObject` and `UnregisterFactory`'d first. Safe this early: the stock service is a
  `gAppData`-less stub that cannot answer `nsIXULAppInfo`, and any cached service instance goes with
  its factory entry.
- **Register before any chrome JS runs.** The earliest hook an embedder owns is the directory
  service provider — `JihadDirProvider::GetFile`, called from inside `NS_InitXPCOM2`, before
  `XRE_InitEmbedding2` fires `app-startup`. It is **retried on every call**: the first few calls
  precede the component manager itself (`NS_GetComponentRegistrar` →
  `NS_ERROR_NOT_INITIALIZED`, measured), so a one-shot attempt always lost the race. The explicit
  call in `EngineHost::Init()` stays as a backstop and reports failure loudly.

Also mirrored in the second `RegisterFactory` call: aliasing a *second* contract to an
already-registered CID requires passing a **null factory** ("if a null factory is passed in, this
call just wants to reset the contract ID to point to an existing CID entry" —
`nsComponentManager.cpp`). Passing the factory again is another `NS_ERROR_FACTORY_EXISTS`, which is
what made the very first attempt return false silently.

## Identity values

All in `render/goanna/JihadUserAgent.h`, which is now the single source for the UA string *and* the
app identity — the UA is composed from the macros so the product token cannot drift from
`appinfo.version`. The composed UA is byte-identical to the previously device-verified string.

| | value | note |
|---|---|---|
| `ID` | `{4534aac8-d8c8-4765-95ee-7f61fd0b762d}` | **FROZEN.** XPIs declare `targetApplication` against it. Needs user sign-off. |
| `name` / `vendor` | `JihadBrowser` / `Jihad Browser project` | matches the UA product token and `appinfo.json` |
| `version` | `1.0` | the range add-ons match; a *different* namespace from the webOS package version |
| `appBuildID` | `20260801000000` | fixed, not `__DATE__` — a moving stamp forces an add-on rescan every build and breaks reproducibility. Bump deliberately to invalidate caches. |
| `platformVersion` | `MOZILLA_VERSION` (6.9.0) | from the engine's own `mozilla-config.h`; cannot drift |
| `XPCOMABI` | `TARGET_XPCOM_ABI` | real value (`arm-eabi-gcc3` on device) — gates XPIs with binary components |
| `widgetToolkit` | `headless` | matches `--enable-default-toolkit=cairo-headless` |

## Status

- **The appinfo defect is FIXED, confirmed ON DEVICE** (2026-08-01, md5
  `b78d29eeee77019ad74d1f6596efa1c9`): `addons.manager` error count 0 (was 1 every run), and
  `appinfo: registered early (before app-startup JS)` proves the directory-provider hook wins the
  race on hardware too. Desktop: ROUND-TRIP PASS, exit 0.
- **THE DEVICE CRASH ROOT CAUSE: `$HOME` was unset.** Found 2026-08-01 after the appinfo fix, using
  the crash reporter below. UXP dereferences `$HOME` unguarded —
  `nsDependentCString(PR_GetEnv("HOME"))` in `xpcom/io/SpecialSystemDirectory.cpp:189`
  (`GetUnixHomeDir`) and `xpcom/io/nsAppFileLocationProvider.cpp:318` — so an unset HOME is
  `strlen(NULL)` and an instant SIGSEGV at address 0. An **upstart job inherits init's environment,
  which on webOS 3 has no HOME**, while the container harness always passes `-e HOME=…` — which is
  the entire reason this failed 100% on device and never once on desktop.
  **It is NOT `gAppData`** and not an add-on defect: the add-on startup path was merely the first
  code to ask for a HOME-derived directory. Fixed in `Main.cpp` with `setenv("HOME", <state dir>, 0)`
  (covers every launch path, never overwrites a deliberate HOME) and again on the upstart job's
  `exec env` line. Pointing HOME at the variant's state dir keeps anything Gecko writes under `$HOME`
  inside the footprint that variant's `prerm` removes (R8).
  **Proven on the host, not just argued**: with `env -u HOME` the pre-fix daemon dies exit 139,
  `faultaddr=0x0`, at the identical last breadcrumb (`init: XRE_NotifyProfile`) as the device; the
  fixed daemon logs `HOME was unset — set to …` and reaches `engine up; serving YAP`.

- **The appinfo fix was a co-symptom, and saying so matters.** The daemon still dies ~2 s into `XRE_NotifyProfile` — no
  `XRE_NotifyProfile returned`, no `engine up`, no socket. So the missing identity was a real
  defect and a **co-symptom**, not the cause: with valid appinfo the add-on manager gets FURTHER
  into startup and hits a second, distinct failure. `$APP/profile/extensions` is never created, so
  it dies before XPIProvider establishes its install locations — i.e. early in
  `profile-after-change`, not deep in an extension scan. `nsBlocklistService.js` is the leading
  suspect: top-level `Cu.import` of `AddonManager.jsm`, registered `category profile-after-change`.

## Getting an exact line without a debugger on the device

`render/goanna/JihadCrashReport.h` makes the daemon report its own death (faulting address, PC/LR/SP,
and the load addresses from `/proc/self/maps`). Host-side, the ARM libxul in the build tree is
**1.1 GB with full `.debug_info`/`.debug_line`**, so a runtime PC resolves to function + file:line:

```sh
TC=build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin
L=build/webos-oe/out-arm/obj-jihad-goanna-arm/toolkit/library/libxul.so
# from the report's maps line for libxul's r-xp segment:  START-END r-xp OFFSET ...
#   file vaddr = PC - START + OFFSET
$TC/arm-webos-linux-gnueabi-addr2line -f -C -i -e $L 0x<vaddr>
```

Proven end-to-end before shipping: `nm` gives `XRE_NotifyProfile` at `0x01b97bfc`, and `addr2line`
resolves that to `/src/uxp/toolkit/xre/nsEmbedFunctions.cpp:195`.

**The handler MUST be re-armed after engine init** — SpiderMonkey installs its own SIGSEGV handler
(wasm/asm.js bounds-check trap) during `XRE_InitEmbedding2` and replaces ours. Measured: with only
the `main()` install, a real fault after engine init produced NO output whatsoever. Verified working
via `JIHAD_TEST_CRASH=1`, which faults deliberately once the engine is up: exit 139, correct fault
address, maps captured.

## Measurement traps that produced FALSE PASSES (both real, both cost time)

- **`Segmentation fault` is printed by the shell only for FOREGROUND jobs.** Its absence in a
  backgrounded run is not evidence of survival.
- **`kill -0` succeeds on a ZOMBIE.** Use `wait` (or read `/proc/<pid>/status`) for liveness. This
  is what made the first version of the crash-handler self-test report "STILL ALIVE — handler
  swallowed it" when the handler had in fact not run at all.
- **A unix socket file OUTLIVES the process that bound it.** `/tmp/yapserver.<name>` being present
  proves nothing; `rm -f` before the run and poll for it to APPEAR.

## Dead ends (do not retry)

- Registering under a **new** CID and re-pointing only the contract. Correct at the XPCOM level,
  invisible to JS. See above.
- Registering **after** `XRE_InitEmbedding2` returns (including immediately before
  `XRE_NotifyProfile`). Too late: `app-startup` JS has already run.
- A **one-shot** attempt from the directory-provider hook. The first `GetFile` predates the
  component manager.
- Leaving `JIHAD_NO_PROFILE_NOTIFY=1` as a "fix". It is a pure diagnostic: skipping
  `XRE_NotifyProfile` disarms the profile keys and costs cookie persistence
  (`cavekit-browser-services.md` R2), which is the whole point of the profile work.
- **Dropping the add-on manager from the device bundle** (considered 2026-08-01, then reversed by
  the user, who requires `about:addons` + XPI support). Recorded because the analysis is still
  useful: `nsBlocklistService.js` does a **top-level** `Cu.import("resource://gre/modules/AddonManager.jsm")`
  and is registered `category profile-after-change`, so the add-on modules are **not** cleanly
  separable at the file level — deleting them would break the blocklist service in the same startup
  window. Only the *registration* is separable.
