---
created: "2026-08-02"
last_edited: "2026-08-02"
status: R8 DONE (comparison complete); drives R3/R7
---

# R8 — our add-on/plugin support vs Pale Moon and Basilisk

Reviewer: **fable**, against `../ref-forks/{Pale-Moon,Basilisk}` and the shared `third_party/uxp`.
Both forks' `platform/` submodules are empty checkouts, so all engine citations are from our tree —
which is the same UXP lineage both build from.

Every claim below that a fix was based on was **independently re-verified in the source before
acting** (`gStrictCompatibility = true` at `AddonManager.jsm:610`; `SCOPE_ALL` default at
`XPIProvider.jsm:1987-1988`; `xpinstall.signatures.required` already `false` at `all.js:4531`;
`MOZ_ENABLE_NPAPI=1` independent of the GTK2 flag at `old-configure.in:2142`).

## The finding that reframes R3

**F1 (P1) — with our frozen GUID and no AppCompat mechanism, no real-world XPI can EVER be
compatible.** The two forks solved this in opposite-looking ways that address the same problem:

- **Basilisk ships Firefox's own GUID** (`basilisk/confvars.sh:48`) and versions itself
  `52.9.YYYY.MM.DD`, so Firefox-targeted extensions match natively.
- **Pale Moon keeps its own GUID** *plus* the platform's dual-GUID feature `UXP_APPCOMPAT_GUID=1`
  (`palemoon/confvars.sh:71`, `palemoon/configure.in:19-28`), consumed by `XPIProvider.jsm:137-140`
  and `6357-6377`: a `targetApplication` naming the AppCompat GUID is treated as compatible, with
  the AppCompat *version* substituted. Pale Moon points it at Firefox's GUID + `"56.9"`
  (`palemoon.js:50-51`).

We build `--enable-application=xulrunner`, whose `confvars.sh` sets no `UXP_APPCOMPAT_GUID`, so our
`XPIProvider.jsm` is preprocessed **without the mechanism at all** — matching is exact-ID-or-
`toolkit@mozilla.org` only. Our frozen ID at version `"1.0"` is named by zero existing extensions,
so every real XPI arrives `appDisabled`, and the web-install path treats `appDisabled` as a failed
install (`amWebInstallListener.js:112-115`). **R3 is unreachable with any extension not authored
for Jihad specifically.** Pale Moon's route is the only one compatible with our frozen app ID
(user-signed-off), so that is the one to take.

**Ordering trap:** `XPIProvider.jsm:137-140` reads the AppCompat prefs with a one-argument
`getCharPref` at module top level. With the build define present and the prefs absent, the import
**throws and takes the whole add-ons manager down**. Prefs-first is harmless (simply unread), so
they were shipped first — deliberately making that failure mode unreachable.

## Findings and disposition

| # | Sev | Finding | Status |
|---|-----|---------|--------|
| F1 | P1 | Dual-GUID/AppCompat absent → no real XPI can be compatible | Prefs shipped; **build define still TODO** (`third_party/uxp/xulrunner/configure.in`) |
| F2 | P1 | `gStrictCompatibility` code-defaults **true**; both forks turn it off in app prefs we never inherit | FIXED — `extensions.strictCompatibility=false` |
| F3 | P1 | **Windowless NPAPI does not exist in a cairo-headless build.** `NPNVSupportsWindowless` answers false unless `XP_WIN\|\|XP_MACOSX\|\|(MOZ_X11&&MOZ_WIDGET_GTK)` (`nsNPAPIPlugin.cpp:1941-1948`), and the whole Unix windowless model is X11-defined (`nsPluginInstanceOwner.cpp:3005-3022` fills `ws_info->display = DefaultXDisplay()`). Neither fork faces this — both are GTK/X11 | OPEN — reframes R7 as **engine porting, not configuration** |
| F5 | P1(us) | `enabledScopes` defaults `SCOPE_ALL` → `XREUSysExt` → `EnsureDirectoryExists($HOME/.mozilla/extensions)`, i.e. **mkdir on the 62 MB /var partition every startup** (R6 violation) and a `$HOME`-shared dir across variants (R5 leak) | FIXED + **device-verified** (below) |
| F4 | P2 | Install prompt is a modal XUL chrome window (`amWebInstallListener.js:174-191`); headless cannot open it and the catch **cancels every install**. Platform hook `amIWebInstallPrompt` is checked first | OPEN — daemon service to register |
| F6 | P3 | Install failure surfaces only as observer notifications; with no observer, R3's "rejected with a clear reason" is invisible | OPEN — same service as F4 |
| F7 | P3 | Background update timer + blocklist point at infrastructure we do not operate | FIXED — update/blocklist/AMO-cache off |
| F8 | P3 | "Install Add-on From File" needs `nsIFilePicker`, absent headless | Documented as known-inert, not a bug to chase |
| F9 | P3 | `RunPluginOOP` defaults **true** on non-GTK builds and `plugin-container` is not bundled → a plugin load would stall spawning it | FIXED — `dom.ipc.plugins.enabled=false` |

## F5 confirmed on hardware, then fixed

Predicted from source, then **measured on device before the fix**:

```
/var/palm/jihad/enyo/.mozilla
/var/palm/jihad/enyo/.mozilla/extensions      <-- extension install location on /var
```

After shipping `extensions.enabledScopes=5` (PROFILE|APPLICATION), deleting the tree and
restarting the daemon, `.mozilla/extensions` **does not come back**. Two empty dirs (`.mozilla`,
`Desktop`, ~4 KB each) still appear from other `$HOME` lookups; both sit inside
`/var/palm/jihad/<variant>`, which `prerm` removes, so the R8 residue contract holds and no
extension data lands on `/var`.

This is a **deliberate divergence** from both forks, and is written down as R8 requires: they
accept all scopes because on a desktop OS those locations are a feature. Three independent
variants sharing one device `$HOME` is a webOS packaging requirement neither has.

## Confirmed FINE — do not re-litigate

- **Signature policy: nothing to do.** UXP carries *zero* signing enforcement — no
  `signedState`/`mustSign`/`MOZ_ADDON_SIGNING` in the XPI stack, and the platform default is
  already `xpinstall.signatures.required=false` (`all.js:4531`). Both forks diverge from Mozilla by
  not *building* enforcement; we inherit that by building the same platform. **Do not add signing
  config.**
- **App identity.** The forks get `nsIXULAppInfo` from `gAppData` parsed from `application.ini` by
  `XRE_main` — exactly what `XRE_InitEmbedding2` never populates. Our CID takeover in
  `EngineHost.cpp` is the correct equivalent and covers the complete field set the stack actually
  reads (ID, version, platformVersion, OS, XPCOMABI, inSafeMode, name/vendor);
  `annotateCrashReport` is `instanceof`-guarded, so `--disable-crashreporter` is safe. Nothing in
  the XPI stack reaches `gAppData` directly. One trap: `plugins.load_appdir_plugins` would call
  `XRE_GetBinaryPath(gArgv[0])` with `gArgv[0]==nullptr` in an embedding — **leave that pref off.**
- **Install location.** `ProfD/extensions` with our `ProfD = $APP/profile` is already exactly
  R5/R6's `$APP/profile/extensions` on cryptofs. No work beyond the scope clamp.
- **`about:addons` registration + branding.** The redirector is GRE-side and shared; neither fork
  adds anything app-side. Our `packaging/branding/` entity set is a **superset** of Pale Moon's
  official `brand.dtd`, and `brand.properties` carries the `brandShortName` the manager reads.
- **Our NPAPI configure flags are correct as-is** — `mozconfig.goanna-arm:20` is right; a
  gtk2-toolkit build may not disable npapi-gtk2, our headless build both may and must.
- **Plugin discovery for R7** is already answerable within contract: `$APP/profile/plugins` via
  ProfD, optionally plus `MOZ_PLUGIN_PATH` from the daemon.

## Remaining plan

### How to build the install prompt: follow Atlas, not the toolkit

The toolkit's install confirm is a modal XUL chrome window (`amWebInstallListener.js:174-191`),
which headless cannot open — and its failure path CANCELS every install (F4). Do NOT try to make a
XUL window appear.

**Atlas's model is the right one for this platform** (user direction, 2026-08-02): prompts are
CARD-SIDE Enyo dialogs driven by engine events, never engine-drawn chrome. In
`Atlas/atlas-browser-app`, `source/BrowserPrompt.js` is an Enyo kind built on
`VerticalAcceptCancelPopup` with accept/cancel buttons, declared as a component in
`source/BrowserApp.js` (e.g. `{name: "downloadError", kind: "BrowserPrompt", caption: …}`) and
opened when the browser layer reports something needing a decision.

**Each variant must use ITS OWN framework's dialog idiom** (user direction, 2026-08-02) — adapted
from Atlas's dialogs, not shared between variants and not drawn by the engine:

| variant | dialog style to adapt Atlas's `BrowserPrompt` into |
|---|---|
| **enyo** (`app/`) | Enyo 1.0 popups — `VerticalAcceptCancelPopup` / `enyo.Popup`, i.e. Atlas's shape almost directly; `app/source/BrowserPrompt.js` already exists to reuse |
| **mochi** (`app-mochi/`) | Mochi / Enyo 2 popup kinds — do NOT import the Enyo 1 kind; Mochi has its own popup + button styling |
| **mojo** (`app-mojo/`) | Mojo's own dialog API (`this.controller.showAlertDialog` / `showDialog` with a Mojo scene assistant), not an Enyo kind at all |

This keeps each app a native citizen of its framework and preserves R7 independence: the daemon
emits one framework-agnostic "a decision is needed" message over the YAP/DialogService surface, and
each card renders it in its own idiom and returns accept/decline. Nothing variant-specific belongs
in the daemon.

So: register `@mozilla.org/addons/web-install-prompt;1` (`amIWebInstallPrompt`) in the daemon,
have it emit a message over the existing YAP/DialogService surface, and let the card show its own
`BrowserPrompt` and send accept/decline back. Our `app/source/BrowserPrompt.js` already exists
(inherited from isis) and is the component to reuse — the same path should carry the
`addon-install-failed` / `addon-install-blocked` reasons for F6.

**R3:** (1) prefs — DONE; (2) `UXP_APPCOMPAT_GUID` define + libxul rebuild; (3) daemon
`amIWebInstallPrompt` (Atlas pattern above) + `addon-install-failed`/`-blocked` observers; (4) desktop flow test of
`amContentHandler` → message-manager → `AddonManager.jsm:1880` under a bare `nsWebBrowser`;
(5) the R3 acceptance run.

**R7:** configuration is essentially done. The blocker is F3: headless windowless rendering does
not exist and must be built (answer `NPNVSupportsWindowless`, define a non-X drawable contract,
paint into our cairo surface, map input per input-bridging R5). That **exceeds what either
reference browser ever built**, and the device's Flash additionally expects Palm's WebKitLuna host
— so treat R7 as engine porting with an honest chance the device Flash never runs.
