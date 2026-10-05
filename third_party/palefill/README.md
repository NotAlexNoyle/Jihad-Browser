# Palefill (bundled add-on)

`palefill-1.30.xpi` is the **unmodified release file** of the Palefill Web Technologies
Polyfill add-on, as published by its author:

| | |
|---|---|
| Add-on | Palefill Web Technologies Polyfill (`palefill@addons.martoks-place.de`) |
| Version | 1.30 (tag `v1.30`, 2025-03-06) |
| Author | Martok & contributors; portions based on GitHub Web Components Polyfill by JustOff and SeaHOH |
| Licence | Mozilla Public License 2.0 — every source file in the XPI carries the MPL header; the licence text ships with every Jihad Browser package as `licenses/MPL-2.0.txt` |
| Source | https://github.com/martok/palefill (the XPI *is* the source form: an XPI is a zip of the add-on's JavaScript) |
| Downloaded from | https://github.com/martok/palefill/releases/download/v1.30/palefill-1.30.xpi |
| SHA-256 | `5ba4b1bd90f7f37b47115595a9a4cd52e10bfc64eed650241c1754c3a0782993` |

Nothing in it has been changed. `build/webos-oe/make-device-bundle.sh` refuses to bundle a
file whose SHA-256 is not the one above, so a different file cannot ship under this notice
by accident. To move to a newer release: replace the file, update the version and hash here,
in `make-device-bundle.sh` and in `render/goanna/components/jihadBundledAddons.js`, and the
line in `NOTICE`.

## How it is used

All three Jihad Browser variants carry the file in their engine directory
(`bundled-addons/`). On a profile's first start the browser installs it through the engine's
own add-on manager, exactly as if the user had opened the file (see
`render/goanna/components/jihadBundledAddons.js`). From then on it is the user's add-on like
any other: it is listed in the add-ons manager, can be disabled or removed there, and is not
put back once removed. It installs under the add-on's own "generic UXP" compatibility
declaration (`toolkit@mozilla.org` 4.8.2 – 10.*; this engine is platform 6.9.0).

Palefill describes its support for UXP applications other than Pale Moon, Basilisk and
SeaMonkey as limited ("runs on any UXP toolkit browser, with limited support"). Problems
with a site in Jihad Browser are Jihad Browser's to look at first; they are not to be
reported to Palefill's author as Pale Moon issues.
