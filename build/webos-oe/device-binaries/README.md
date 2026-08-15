# Device reference binaries (NOT built here, NOT shipped)

Pulled off the TouchPad with `novacom get`. They are here because they are the **only
authoritative ABI oracle** for the webOS NPAPI work (cavekit-addons-extensions.md R7), and
re-obtaining them means having the device attached.

| file | what it settles |
|---|---|
| `libWebKitLuna.so` | Palm's real plugin host, **unstripped** — `WebCore::PluginView::*` by name. Every npPalm struct offset, the event-type mapping, the spotlight construction and the `npPalmIsInteractive` gate were read out of here. |
| `libflashplayer.so` | Flash 10.3.185.65, the topaz build (stripped, PLT still names imports). Its `NPP_HandleEvent` jump table is what proves which event types Flash actually acts on. |
| `libPiranha.so` | The graphics stack Flash renders through (`PSoftPixmap`, `PSoftContext2D`). |

Read them, do not link against them. Useful entry points:

```bash
readelf --syms -W libWebKitLuna.so | c++filt | grep 'PluginView::'
arm-webos-linux-gnueabi-objdump -d --start-address=0x4e9400 --stop-address=0x4e9500 libWebKitLuna.so
```

Findings already extracted are recorded in `docs/PICKUP.md` and the R7 section of
`context/kits/cavekit-addons-extensions.md`; check there before re-disassembling.
