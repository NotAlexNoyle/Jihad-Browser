Jihad Browser — Enyo 1.0 UI shell
=================================

This is the **Jihad Browser** fork of the isis-browser Enyo 1.0 app (Apache-2.0),
rebranded and rendering through the **UXP/Goanna** engine instead of QtWebKit.

It uses the enyo WebView control to display web content. Jihad is **self-contained
and coexists with the stock browser**: `source/JihadEngineOverride.js` (first entry
in `depends.js`) swaps THIS app's WebView plugin type to `application/x-jihad-browser`,
so the card loads `BrowserAdapterJihad.so` → the Jihad Goanna daemon
(`/tmp/yapserver.jihad-browser`). Every other app's stock `application/x-palm-browser`
WebView is untouched. See `../packaging/README.md` and `../docs/DEVICE-BUILD.md`.

**Iterating on this UI:** `../build/webos-oe/push-card-js.sh enyo source/Browser.js …`
pushes card files to a connected device and only reports success once a per-push stamp
reaches the device log — the WebAppMgr JS cache will otherwise keep running the previous
build with the new file already on disk. See `../build/webos-oe/README.md`.

**`<select>` dropdowns need no code here.** The stock `enyo.WebView` wrapper handles the
whole card side itself (`showSelect` → `createSelectPopup` → `PopupList`, answering with
`selectPopupMenuItem`) and never re-publishes the event, so an `onOpenSelect` handler in
`source/Browser.js` is unreachable — one was written, proved dead and removed. Do not
re-add one, and do not patch `BasicWebView.showPopupMenu`: an earlier patch shadowed the
framework method and killed the popup outright. What the feature actually needs is the
daemon emitting the isis option-list shape, which it does.

The original isis README follows.

isis-browser 
============

isis-browser is the enyo browser app for webOS. 

It uses the enyo WebView control to display web content which in turn
uses BrowserAdapter and BrowserServer to talk to webOS webkit.

# Copyright and License Information

All content, including all source code files and documentation files in this repository are: 
 Copyright (c) 2012 Hewlett-Packard Development Company, L.P.

All content, including all source code files and documentation files in this repository are:
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this content except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

# end Copyright and License Information
