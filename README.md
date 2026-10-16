<p align="center">
  <img src="docs/jihad-browser-logo.png" alt="Jihad Browser logo" width="168">
</p>

<h1 align="center">Jihad Browser</h1>

<p align="center"><b>UXP inside, webOS alive, inshallah!</b></p>

<p align="center">
  <img src="docs/screenshots/cardview.jpg" alt="Three cards side by side, as in webOS card view, on a Frutiger Aero backdrop: Jihad Mojo on the Internet Archive's home page, Jihad Enyo on MDN's CSS reference with its adverts blocked, and Jihad Mochi on BBC News" width="900">
</p>

---

## A UXP browser for webOS

Jihad Browser puts **Goanna**, the engine of the **UXP** platform that Pale Moon and Basilisk are
built on, under the webOS browser you already know: the stock browser's interfaces with their 2011
WebKit taken out. That makes it a member of the family of UXP browsers and projects. It runs their
engine, their XPI add-ons and their Personas, lays its settings out in their panes, and has an add-on
site of its own built on Phoebus, the software the family's sites run.

It installs **next to** the stock browser and changes nothing else on the device. It is written for
webOS 3 as a whole; the TouchPad and TouchPad Go are the devices it is built and tested on, and the
only ones supported so far. A stubborn, wonderful community kept webOS alive: webOS Internals,
Preware, PivotCE, webOS Ports, the webOS Archive, Herrie82's TLS work and webOS CE 3.1.0. Jihad is
our part of that.

<table>
<tr><td><b>Version</b></td><td>1.0 (Jihad Mojo 1.0.0, Jihad Enyo 1.0.4, Jihad Mochi 1.0.0)</td></tr>
<tr><td><b>Runs on</b></td><td>webOS 3.0.5 and webOS CE 3.1.0; the TouchPad and TouchPad Go for now</td></tr>
<tr><td><b>Engine</b></td><td>Goanna 6.9 (UXP), with its own NSS for TLS 1.2 and 1.3</td></tr>
<tr><td><b>Flash</b></td><td>The Flash Player 10.3 already on the device</td></tr>
<tr><td><b>Licence</b></td><td>Apache-2.0 and MPL-2.0 for Jihad's own code; the bundled libraries under their own licences (see <code>NOTICE</code>)</td></tr>
</table>

---

## What only Jihad does on webOS

The stock browser is a 2011 WebKit that opens little of today's web. Atlas Browser brings a current
WebKit to webOS. Neither does what follows.

### Flash and the modern web, on one page

<img src="docs/screenshots/flash.webp" alt="The Flash game Bloxorz running full-page in Jihad Enyo" width="300" align="right">

webOS promised the whole web, Flash included; every modern browser since has dropped plug-ins.
**Jihad runs today's web and Flash together**, with the device's own Flash Player 10.3: animation,
sound, touch and keys. Flash runs in its own process and keeps its own frame clock, so games play
smoothly. Bloxorz runs straight from the Internet Archive.

<br clear="right">

### Add-ons and Personas, from the UXP world

<p align="center">
  <img src="docs/screenshots/addons-personas.webp" alt="Left: about:addons in Jihad Mojo, with Palefill installed. Right: about:personas in Jihad Mochi, the gallery of Personas with Wear and Take off" width="760">
</p>

- **XPI add-ons** install from a web page and are managed in `about:addons`, as in Pale Moon and
  Basilisk. **Palefill**, which fills in web features the engine lacks, comes pre-installed.
- **Personas** give the toolbar and buttons a picture and colours of their own. Two come with Jihad:
  **Free Palestine**, Battir's terraced olive hills from a CC BY 2.0 photograph by Labour Palestine,
  and **Aurora Silk**, drawn for Jihad in the glossy style of the screens webOS grew up with.
  `about:personas` is Jihad's own gallery: **Wear**, **Take off**, **Remove**, **Undo**, **Install
  from file**, and a search of Jihad's add-on site, addons.riverstonerelay.org, and of
  addons.mozilla.org, whose themes install with one confirmation. Persona sites made for Firefox,
  Pale Moon and Basilisk work too. Each interface keeps its own, so every card can have a different
  look.

### Every preference

<p align="center">
  <img src="docs/screenshots/settings.webp" alt="Left: the Privacy pane of Jihad's Settings page in Jihad Enyo. Right: about:config in Jihad Mochi" width="760">
</p>

A **Settings** page in Pale Moon's panes (General, Content, Privacy, Security, Advanced, About):
search engine, home page, start-page links, cookies, history, fonts per script, smooth scrolling,
frame rate, more than 55 options in all. Under it, **`about:config`** with every preference the
engine has.

### HTTPS that needs nothing else

<img src="docs/screenshots/tls.webp" alt="howsmyssl.com in Jihad Mojo: Version Good, 'Your client is using TLS 1.3'" width="300" align="right">

Every webOS release shipped a 2009 OpenSSL that speaks TLS 1.0 at best, with a root store from 2010,
so the stock browser lost almost every HTTPS site, and other browsers on webOS need the community's
TLS packages or webOS CE to get online. **Jihad does not use the device's OpenSSL at all.** Goanna
brings its own NSS with Mozilla's root list, so HTTPS works the same on a stock 3.0.5 device as on
CE 3.1.0: howsmyssl.com rates the protocol version Good, **TLS 1.3** (the one thing it calls
improvable is post-quantum key agreement). It covers Jihad only; the stock browser, Mail and
system services still use the device's own TLS (the [webOS Archive
guide](https://docs.webosarchive.org/site/modern-tls/) covers those). Keep your clock set:
certificate checks need the right date.

<br clear="right">

### Ads and trackers, stopped in the engine

Jihad refuses loads from the hosts on **EasyList** and **EasyPrivacy** before they leave the device,
and hides the frames adverts leave, in all three interfaces, with no add-on to install. The lists
are bundled and refreshed from easylist.to every four days; Settings → Privacy switches it, shows
the list versions and counts what was refused. (The lists are by The EasyList authors, used under
the GNU GPL v3 or later; see `CREDITS.md`.)

Jihad says it is Firefox 52.9, which it is at heart. When a site's bot check turns that away
(Cloudflare's "verifying you are human" page, Startpage's captcha gate), Jihad comes back once as
a current Firefox, header and script alike, and remembers the site; Settings → General lists the
sites and forgets them on request. YouTube is shown its mobile site, a quarter of the desktop
page's script. The few web platform pieces the engine lacks and modern pages assume (popover,
`inert`, `CSSStyleSheet.replace`, `Intl.ListFormat` …) are supplied to each page before its
scripts run, only where the engine has none.

### And

- **Three interfaces, one engine:** lightweight **Mojo** with round buttons, the classic **Enyo**
  browser, or the newer **Mochi** design. Install one, or all three; each keeps its own profile,
  add-ons, Personas, history and bookmarks.
- **Printing** to the IPP or AirPrint printer on your network.
- **Privacy by default:** DuckDuckGo Lite for search and home; referrer rules honoured.

---

## Installing

Install the `.ipk` for the interface you want with **Preware** or **WebOS Quick Install**:

| Interface | Package |
|---|---|
| **Jihad Mojo** (lightweight) | `net.riverstonerelay.jihad-browser-mojo` |
| **Jihad Enyo** (the classic browser) | `net.riverstonerelay.jihad-browser` |
| **Jihad Mochi** (newer design) | `net.riverstonerelay.jihad-browser-mochi` |

**Known issues:** the engine draws in software, so frame rates are the device's; double-tap zoom
takes about a second to sharpen; Mochi's text can look soft at some zoom levels. Sites built as
ES-module apps load their frame but not their script-driven parts (GitHub's commit list and
README, Mastodon's timeline): the engine's module loading is behind them. How today's sites
behave, site by site, with causes and fixes: `docs/MODERN-WEB.md`.

**Developers:** start at `docs/PICKUP.md` (current state), `docs/DEVICE-BUILD.md` (building) and
`docs/ENGINE.md` (how Jihad uses the engine, and how to update it).

---

## Building

Jihad builds on any x86_64 Linux from this repository alone. Everything it compiles or packages is
in the repository, in a git submodule it pins, or fetched by one of its scripts and checked by
sha256; the parts that care about the host's age run in podman containers. The host needs git,
podman, curl, binutils and xz (and gcc, make and the libusb-compat headers for novacom), about
40 GB of disk, and several hours the first time, most of it the engine.

```bash
git clone --recurse-submodules https://github.com/NotAlexNoyle/Jihad-Browser.git
cd Jihad-Browser
tools/host/build-novacom.sh          # novacom + novacomd, into tools/host/bin/
sudo -b tools/host/bin/novacomd      # keep it running while a device is plugged in
build/webos-oe/build-all-device.sh   # plug in a TouchPad the first time: eight of its libraries are copied once
```

The packages land in `build/webos-oe/out-ipk/`. Install them with Preware or WebOS Quick Install,
or with `build/webos-oe/push-variant.sh mojo` (or `enyo`, `mochi`) over novacom. `palm-install`
skips the install scripts, so a package installed with it does not work. The packages per
distribution, what each step fetches, and the OpenEmbedded and desktop routes are in
`docs/DEVICE-BUILD.md`.

---

## Thank you

HP's **isis-project** (the browser this forks), the **UXP** platform, **Pale Moon**, **Basilisk**
and **Palefill**, **Herrie82** and **Atlas Browser**, the **webOS Archive**, **webOS Internals**,
**Preware**, **PivotCE** and **webOS Ports**, and everyone still carrying a webOS device in 2026.

## Licensing

**Apache-2.0** (the interfaces, the adapter, the IPC daemon) and **MPL-2.0** (the Goanna backend,
and the engine, which is modified: its source is the `jihad/uxp-RB_20261006` branch of
[UXP-Jihad](https://github.com/NotAlexNoyle/UXP-Jihad), the patches in `build/desktop/patches/`).
The packages also carry the libraries the engine runs on, each under its own licence: glibc
(LGPL-2.1), the GCC runtime (GPL-3.0 with the Runtime Library Exception), ALSA and GLib (LGPL),
Expat, fontconfig, PCRE, libpng, zlib (permissive), FreeType (FreeType License), the engine's own
`about:license` contents, Lato (SIL OFL, Mochi package), and the Personas by other authors (CC BY /
BY-SA, credited in `CREDITS.md`). `NOTICE` names every one, with where its text ships and where its
source is. Palefill is MPL-2.0, shipped unmodified. Pale Moon, Basilisk and Moonchild branding is
removed from the engine build; "Goanna" names the engine, Moonchild Productions' trademark, used
to say what the browser runs on. Flash Player is the device's own and is not distributed. HP's SDK,
downloaded by the build for its compiler, stays under HP's terms and nothing of it is
redistributed. See `LICENSE`, `NOTICE` and `CREDITS.md`.

<sub>Palm, webOS, Pre, TouchPad and Synergy are trademarks of their respective owners. Jihad Browser
is an independent community project, not made, endorsed or supported by HP, Palm or LG. Screenshots
were taken on a real TouchPad.</sub>
