# Evidence, 2026-10-06

| File | What it shows |
|---|---|
| `enyo_pinch-2.5-wide-region-1388-columns.jpg` | Enyo card after a scripted pinch to zoom 1.2434 through the adapter's gesture entry points; the daemon painted a 1388x2266 region (`region columns:` line) and the composite is right. |
| `mojo_cut-emptied-the-field.jpg` | Mojo card after Select All → Cut in a field from the test hook: field empty, next hold offers Paste / Select All. The engine answered "Cut."; the clipboard held "filled field". |
| `mojo_find-on-page-selected-charlie.jpg` | Mojo find bar with "charlie", the word selected on the page — Find on Page reaching the engine from Mojo for the first time (the widget never wrapped `findInPage`). |
| `enyo_fresh-install-first-card.jpg`, `mojo_fresh-install-first-card.jpg` | First card of each variant after removal through the installer and install from nothing. |
| `mochi_print-small-page-as-the-sink-received-it.jpg` | The one-page test sheet as `ipp-sink.py` received it (1536x1986 JPEG, 0.7 % ink) before the same page went to the Epson ET-2800 (job-id 32). |

Audit snapshots: `build/webos-oe/out-audit/{pre,post}-{enyo,mojo}-{remove,fresh}-1006.txt` (git-ignored; on the host that ran them).
