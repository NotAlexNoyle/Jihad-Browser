# render/adapter — NPAPI BrowserAdapter (Goanna + self-contained)

The sysmgr-card NPAPI plugin: the isis UI's WebView loads it, and it speaks YAP to
the render daemon and blits the daemon's shared-memory framebuffer onto the card.
Built as `BrowserAdapterJihad.so` by `build/webos-oe/build-adapter-{pdk,arm}.sh`.

## Origin & license (Apache-2.0)

Imported from the isis-project sources (Apache-2.0, © 2012 Hewlett-Packard
Development Company, L.P. / © 2012-2014 LG Electronics, Inc.), with each file's
`@@@LICENSE` header preserved:

- `BrowserAdapter.*`, `BrowserAdapterManager.*`, `BrowserClientBase.*`,
  `Rectangle.*`, `UrlInfo.*`, `InteractiveInfo.*`, `ElementInfo.*`, `ImageInfo.*`,
  `JsonNPObject.*`, `NPObjectEvent.*`, `KineticScroller.*`, `BrowserOffscreen.*`,
  `Browser{CenteredZoom,MetaViewport,ScrollableLayer}.h`, `Debug.h`,
  `BrowserAdapter.exports` — from
  [isis-project/BrowserAdapter](https://github.com/isis-project/BrowserAdapter)
  @ `245f9c0`.
- `AdapterBase.*` — from
  [isis-project/AdapterBase](https://github.com/isis-project/AdapterBase).

Not imported: the upstream `data/` bitmaps (unused by the raw-blit paint path),
`debian/` packaging, and isis Makefiles (the build uses `build/webos-oe/*.sh`).
The YAP transport (`Yap/`, `IpcBuffer`) lives in `../browserserver` and is shared.

## Jihad modifications (all in `BrowserAdapter.cpp`)

Per Apache-2.0 §4(b), the modified file carries an in-file change notice. Summary:

1. **Self-contained coexistence** — MIME `application/x-jihad-browser`
   (`AdapterGetMIMEDescription`) + YAP server name `BrowserClientBase("jihad-browser")`,
   so this plugin runs alongside the stock `BrowserAdapter.so` without collision.
   Paired with `app/source/JihadEngineOverride.js` and `packaging/`.
2. **`dstBuffer` raw-blit `handlePaint`** — replaces the QPainter/PGContext path
   (which segfaults the device's LunaCE WebKit1) with a raw ARGB blit into the
   WebKit paint buffer; plus a landscape rotation guard, pinch-zoom scaled blit,
   and scroll indicator.
3. **Single-tap → click** — `npPalmGestureSingleTapEvent` dispatches
   `asyncCmdClickAt` in content coords so taps activate links/buttons/fields.

Items 2–3 are **ported from the Atlas Browser project** (Herman van Hazendonk /
Herrie82, Apache-2.0; commits `7b91ab8`, `aefab6d`, `dbc897c`, `27b54a5`,
`77ab724`, `01da087`), attributed in-code and in the repo `NOTICE`.
