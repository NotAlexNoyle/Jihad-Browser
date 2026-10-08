# UXP complete themes in Jihad Browser: how much applies

Research only. Nothing in the repo or on the device was changed. Paths are relative to
`Jihad-Browser/`, and `uxp/` means `third_party/uxp/`. The device bundle read here is
`build/webos-oe/device-bundle/` (`jihad-bundle.id` is dated 2026-10-08).

## Short answer

A complete theme can only re-skin `chrome://global/skin` and `chrome://mozapps/skin`. In Jihad those
skins reach a few engine pages (about:addons, about:config, about:support and similar) and a few
widgets inside web pages (video controls, the plug-in overlay, scrollbars on scrollable boxes). They
can never reach the three card shells, where the browser's toolbar, buttons and dialogs live.

Today every real complete theme is refused at install, because of where it says it runs. Making
them work would take five fixes in UXP and the daemon. It would also switch off the user's Persona,
which is the only theme that changes the toolbar. **Recommendation: (a). Say plainly that complete
themes are unsupported, and refuse them with a message that points to Personas.**

---

## 1. How UXP installs and applies a complete theme

**What makes an add-on a complete theme.** If `install.rdf` gives an `internalName` and no `type`,
the add-on is a theme (`uxp/toolkit/mozapps/extensions/internal/XPIProvider.jsm:821-823`). A theme
without an `internalName` is rejected (`:874-877`). The theme's `chrome.manifest` is then registered
as a *skin location*, which may use only `skin`, `style`, `override` (for `chrome://…/skin/` URLs
only) and `manifest`. Any `content`, `locale`, `resource` or `component` line is refused
(`uxp/xpcom/components/ManifestParser.cpp:94-125` and `:624-628`). So a theme cannot replace
`resource://gre-resources/forms.css`, and it cannot replace Jihad's own
`chrome://jihad-prefs/content/` page.

**Which skin is in use.** Two preferences control it:

- `general.skins.selectedSkin` decides the skin. It defaults to `classic/1.0`
  (`XPIProvider.jsm:75`, `:1786`, `:2029-2033`). The chrome registry reads it at start-up and
  watches it for changes (`uxp/chrome/nsChromeRegistryChrome.cpp:115`, `:137-145`, `:319-328`).
- When a package has no skin under the selected name, the registry uses the first skin registered
  for that package (`ANY`), which here is `classic/1.0` (`nsChromeRegistryChrome.cpp:514`,
  `:535-565`). The theme's skin replaces a whole package at once, not file by file.
- `extensions.dss.enabled`, `extensions.dss.switchPending` and `extensions.lastSelectedSkin`
  (`XPIProvider.jsm:72-74`) handle the switch.
- The theme's manifests are registered only at start-up. `nsXREDirProvider::DoStartup` calls
  `LoadExtensionBundleDirectories`, which reads `[ThemeDirs]` from `extensions.ini`
  (`uxp/toolkit/xre/nsXREDirProvider.cpp:705-733` and `:930`). The daemon does reach `DoStartup`,
  through `XRE_NotifyProfile` (`render/goanna/EngineHost.cpp:1056-1075`).
- `writeAddonsList` lists only the selected theme, or every theme when DSS is on
  (`uxp/toolkit/mozapps/extensions/internal/XPIProviderUtils.js:1362-1387`).

**When a restart is needed.**

- **Installing** a theme always needs a restart, even with DSS on: `return aAddon.type == "theme"
  || !aAddon.bootstrap` (`XPIProvider.jsm:4146-4150`).
- **Switching** themes needs a restart unless `extensions.dss.enabled` is set (`:4045-4055`,
  `:4082-4100`).
- Jihad sets neither preference. The bundle's `goanna.js` and `defaults/pref/xulrunner.js` contain
  no `general.skins` or `dss` line.
- Even with DSS on, `RefreshSkins()` only redraws windows it finds through `nsIWindowMediator`
  (`uxp/chrome/nsChromeRegistry.cpp:332-367`). Jihad's pages are bare `nsIWebBrowser` instances
  (`EngineHost.cpp:1214-1223`) that are never registered there. A live switch would therefore
  flush the caches but redraw nothing until the page is reloaded.
- I found no restart path in the daemon either: a search for `nsIAppStartup` and `eRestart` across
  `render/goanna/` and its components found nothing. Today the user's only restart is
  stop and start of the upstart job (or a reboot).

**Will a theme pass the compatibility check?**

Jihad's app identity is set in code; there is no `application.ini`:

- ID `{4534aac8-d8c8-4765-95ee-7f61fd0b762d}` and version `1.0` (`render/goanna/JihadUserAgent.h:52-55`).
- They are served by `JihadAppInfo` (`EngineHost.cpp:278-348`).
- The platform version is `6.9.0` (`uxp/config/milestone.txt`).
- The AppCompat GUID is Firefox's, `{ec8030f7-…}`, at version `56.9`
  (`packaging/prefs/jihad-addon-prefs.js:76-77`; build define in `uxp/xulrunner/confvars.sh:19`).

How each kind of theme fares:

| Theme written for | Its `targetApplication` | Result in Jihad |
|---|---|---|
| Pale Moon | `{8de7fcbb-…}` (`uxp/services/sync/modules/constants.js:177`) | Nothing matches, so the theme is appDisabled |
| Basilisk or Firefox (Basilisk keeps Firefox's ID) | `{ec8030f7-…}` | The AppCompat match is limited to `type == "extension"` (`XPIProvider.jsm:6430-6450`). `isCompatibleWith` returns false for any other type on that GUID (`:6367-6372`). appDisabled. |
| Generic toolkit | `toolkit@mozilla.org` | Themes are always checked strictly, so `6.9.0` must lie between min and max (`:6381-6418`, `:206-209`). Rare. |
| Jihad itself | Jihad's ID | No such theme exists |

An appDisabled XPI fails a web install (`uxp/toolkit/mozapps/extensions/amWebInstallListener.js:110-114`).
Jihad already turns that into a card notice: "… could not be installed because it is not
compatible with Jihad Browser 1.0." (`render/goanna/components/jihadInstallPrompt.js:129-147`).
**So in effect, complete themes are already refused today**, but with a message about versions
rather than one that says the add-on type is unsupported.

## 2. What a complete theme could change in Jihad

**Engine pages** (`uxp/docshell/base/nsAboutRedirector.cpp`):

- `about:addons`: `extensions.xul` loads `chrome://global/skin/` and
  `chrome://mozapps/skin/extensions/{extensions,about}.css` (lines 6-9 in the bundle). This is the
  biggest target, and the cards link to it. Jihad already renames its Themes category to
  "Personas" and injects its own pane CSS (`render/goanna/components/jihadPersonas.js:831-835`,
  `:407`, `:900`).
- `about:config`: `config.xul` loads `chrome://global/skin/in-content/info-pages.css` and
  `config.css`. The cards link to it.
- Typed-only pages: `about:about`, `about:support`, `about:memory`, `about:plugins`,
  `about:license`, `about:networking`, `about:performance`, `about:profiles`, `about:buildconfig`
  and `about:newaddon`. Each uses global or mozapps skin CSS.

**Not reachable by a theme:**

- `about:preferences` and `about:settings` are Jihad's own HTML page, which loads only
  `chrome://jihad-prefs/content/preferences.css` (`packaging/prefsui/content/preferences.html:18`).
  That is a `content` package, so a skin manifest cannot touch it.
- Error and certificate pages are not engine pages. `mUseErrorPages` defaults to false and is never
  turned on (`uxp/docshell/base/nsDocShell.cpp:785`, `:5719`). Failed loads and bad certificates
  are reported to the card's own isis UI (`render/goanna/BrowserPageGoanna.cpp:1905-1930`;
  `render/goanna/GoannaRenderPage.cpp:285-300`, `:455-470`). The toolkit has no `about:certerror`.
- `alert`, `confirm`, `prompt`, the install prompt, downloads and printing all use the card's own
  dialogs, through DialogService and Luna.

**Widgets inside web pages:**

- **Affected:**
  - `<video controls>`: `videocontrols.xml:203` and `:1983` load
    `chrome://global/skin/media/videocontrols.css`, bound from `res/html.css:737-741`.
  - The Flash/plug-in problem overlay: `pluginProblem.xml:21` loads
    `chrome://mozapps/skin/plugins/pluginProblem.css`.
  - Scrollbars on scrollable elements: `chrome://global/skin/scrollbars.css` is added to every
    document (`uxp/layout/style/nsLayoutStylesheetCache.cpp:60-68`;
    `uxp/layout/base/nsDocumentViewer.cpp:2336-2340`).
  - The textarea resizer and textbox bindings.
- **Not affected:**
  - The page's own scrolling. The page is laid out at full height and the adapter draws its own
    fading scrollbar (`render/adapter/BrowserAdapter.cpp:6564`).
  - Form controls. `forms.css` is `resource://gre-resources/`, which is not a skin.
  - `<select>`, which opens the card's own popup list (`BrowserPageGoanna.cpp:1466-1490`).
  - The native theme. There is none: `chrome-native-theme` is ABSENT, and Jihad draws its own menu
    hover sheet (`EngineHost.cpp:1132-1140`). Many theme rules depend on `-moz-appearance`, and with
    no native theme those rules have nothing to change.

**The card shells: never affected.**

- The toolbar, URL bar, buttons, menus, tabs and dialogs are Enyo, Enyo 2 and Mojo HTML. They run in
  LunaSysMgr's WebKit, in a different process from the engine.
- The engine's chrome registry exists only inside `jihad-browserserver`. `chrome://` URLs do not
  resolve in a card.
- The card receives only the engine's rendered frames, through the NPAPI adapter.
- A Persona reaches the cards only because `jihadPersonas.js` sends a small JSON of colours and an
  image path over Luna (`jihadPersonas.js:5-31`, `:81-99`). `app/source/ActionBar.js:81-110` then
  paints it.
- There is no `chrome://browser` package at all (no `browser.xul`). A complete theme's main purpose,
  skinning the browser window, has nothing to act on.

## 3. Does the build ship skins a theme could replace?

- The engine is not packed into `omni.ja` (`build/webos-oe/make-device-bundle.sh:125-127`). Loose
  `chrome/` and its manifests are copied instead (`:129-169`).
- The bundle registers exactly two skin packages:
  ```
  skin global classic/1.0 toolkit/skin/classic/global/
  skin mozapps classic/1.0 toolkit/skin/classic/mozapps/
  ```
  (`device-bundle/chrome/toolkit.manifest:24-25`; there are 305 files under
  `chrome/toolkit/skin/classic/`).
- The `pippki`, `devtools`, branding, prefsui and Jihad component manifests register no skins.
- **`classic/1.0` is present.** It is UXP's *Windows* toolkit theme, because cairo-headless is
  neither cocoa nor gtk (`uxp/toolkit/themes/moz.build:24-32`). That is why the bundle carries the
  `twisty-preWin10`/`win7` overrides at `toolkit.manifest:10-22`. Themes written for Pale Moon's
  Linux skin would therefore not quite match this skin's layout.
- There is **no default-theme add-on**: no `{972ce4c6-…}` and no `extensions/` directory in the
  bundle. Only `bundled-addons/palefill…xpi` and `personas/` ship.

## 4. Hazards that would surface if a complete theme were let through

1. **Disable does nothing.**
   - Setting `userDisabled = true` on a theme calls `XPIProvider.enableDefaultTheme()`
     (`XPIProvider.jsm:6961-6964`).
   - That function needs an add-on whose `internalName` is `classic/1.0`. None exists, so it only
     logs "Unable to activate the default theme" (`:3953-3976`).
   - Uninstalling goes through the same path (`:4672-4673` → `addonChanged(null)` → `:3851-3854`).
     After the restart the registry falls back to `classic/1.0` through `ANY`.
2. **It takes the user's Persona away.**
   - Turning on a complete theme disables the current lightweight theme
     (`uxp/toolkit/mozapps/extensions/LightweightThemeManager.jsm:309-322`; `XPIProvider.jsm:941-943`).
   - The cards then lose their toolbar theme, in exchange for a restyled about:addons.
3. **It can crash the daemon in a loop.**
   - `scrollbars.css` and the other user-agent sheets are loaded with `eCrash`. If the file is
     missing, `NS_RUNTIMEABORT` follows (`nsLayoutStylesheetCache.cpp:64-65`, `:446-460`).
   - So a theme whose global skin lacks `scrollbars.css` would kill the daemon on the first page
     load, after every respawn.
   - Upstart gives up after `respawn limit 10 5` (`packaging/event.d/jihad:145-146`).
   - The engine reports no safe mode (`EngineHost.cpp:364`, `GetInSafeMode` is false), so there is
     no way out except editing `prefs.js`.
4. **No restart, and no live switch** (see section 1).

## 5. Options

### (a) Declare complete themes unsupported — RECOMMENDED

The refusal already happens. The remaining work:

- **A specific message.** In `jihadInstallPrompt.js`'s `addon-install-failed` observer, check
  `install.addon.type == "theme"` with an `internalName`. Say: "*Name* is a complete theme for
  Firefox or Pale Moon. Jihad uses Personas instead. Open Get Personas." It belongs next to the
  static-theme branch at `:119-128`.
- **Block the toolkit-targeted case.** The rare `toolkit@mozilla.org` theme would currently install
  and then hit hazards 1 to 3. Refuse it in `confirm()`, or in the same observer, so the refusal
  holds whatever the version range.
- **Documentation.** One line in `README.md` under Add-ons or Personas (around lines 48-56 and 74-77).
  Optionally one sentence in the Personas pane intro (`jihadPersonas.js:852-856`). Add an
  acceptance line to the kits (`cavekit-addons-extensions.md` R10, or `cavekit-personas-ui.md`).
- **Effort:** about half a day, plus one device check. Install a Pale Moon theme XPI and confirm
  the message.

### (b) Support complete themes for engine pages only

Needed:

- A UXP patch so themes are accepted under the AppCompat GUID and under Pale Moon's GUID, with a
  version mapping, and probably a looser strict-compatibility rule (`XPIProvider.jsm:6367-6383`,
  `:6430-6450`).
- A shipped default-theme add-on with `internalName classic/1.0` in the application scope (scope 4
  is enabled), or a patched `enableDefaultTheme`.
- A restart path the user can reach from the UI.
- `extensions.dss.enabled` set, plus a reload of the open engine pages.
- A crash guard for theme packages that lack the `eCrash` sheets.
- Separating the Persona from the complete skin, so the cards keep their Persona.

Effort: about one to two weeks, with device verification across three variants.

What the user gets: a restyled about:addons, about:config, video controls and plug-in overlay,
drawn for desktop widths and a mouse, on a Windows-flavoured base skin. **Not worth it.**

### (c) Turn a complete theme's colours into a Persona for the cards

- Technically possible but unreliable. A complete theme has no declared colours.
- The colours would have to be computed: register the theme's `browser` skin under a temporary
  name, load `chrome://browser/skin/browser.css` against a dummy XUL tree (`#navigator-toolbox`,
  `#nav-bar`, `toolbarbutton`), then read the computed `background-color`, `background-image` and
  `color`.
- That is fragile. Theme CSS depends on `browser.xul` structure, bindings and `-moz-appearance`,
  and often draws backgrounds with images or gradients.
- What a theme is mostly recognised by, its icon sprites (`Toolbar.png` and `-moz-image-region`
  per button), would not carry over to the cards' own button sets without per-theme mapping.
- Effort: about one to two weeks, with results that vary from theme to theme. Only consider it if
  users actually ask for a specific theme.

### (d) Other options

- **Dress the engine pages in the current Persona.** Apply its `accentcolor` and `textcolor` to
  about:addons and about:config through the agent or author sheet route that `jihadPersonas.js`
  already uses (`loadSheetUsingURIString`, `:900`). Effort: about one to two days. This gives a
  consistent look without any of the hazards above. It is the nearest thing to a complete theme
  worth building.
- **A Jihad-made dark skin for engine pages.** Ship a second skin or an `override` set for
  about:addons and about:config, switched from Settings. Users want this more than arbitrary
  complete themes, and Jihad would control the files, so hazard 3 cannot occur.

## Recommendation

**Do (a) now, and consider (d) later as the way to give engine pages a matching look.** Reasons:

- Complete themes cannot reach the cards, which is where Jihad's look lives.
- They would switch off the Persona, which can.
- The pages they could restyle are few and secondary.
- Supporting them safely needs about five separate engine and daemon changes, including a fix for
  a crash loop.
- The refusal already exists. Only the wording and the toolkit-targeted gap need fixing.
