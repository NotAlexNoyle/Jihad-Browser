# Credits

## Personas bundled with Jihad Browser

Jihad Browser comes with two Personas (render/goanna/personas/), offered in about:addons → Personas
and about:personas:

- **Free Palestine** — from the photograph "Village of Battir" by
  [Labour Palestine](https://www.flickr.com/photos/labour2palestine/16428074717/) (via
  [Wikimedia Commons](https://commons.wikimedia.org/wiki/File:Village_of_Battir.jpg)), licensed
  [CC BY 2.0](https://creativecommons.org/licenses/by/2.0/): Battir's terraces, the UNESCO site
  "Palestine: Land of Olives and Vines". Cropped, scaled, its sky deepened, and the flag and the
  words (Noto Sans Black, SIL Open Font License 1.1) drawn over it by
  `render/goanna/personas/make-personas.py`, which keeps the photograph at 1600 px in
  `render/goanna/personas/source/`. The Persona is under CC BY 2.0.
- **Aurora Silk** — drawn for Jihad Browser by `make-personas.py` from gradients and shapes;
  MPL-2.0 like the rest of the engine-side code.

Every other Persona comes from the add-on site, addons.riverstonerelay.org, and is credited there and
in the site's catalogue (Riverstone-Addons `catalogue/CREDITS.md`). A profile that was offered the
earlier, larger set keeps the ones it has.

## Personas shown in the README's screenshots

The README's art (`docs/screenshots/`, composed by `docs/screenshots/readme-art.py` from raw
device captures) shows the two bundled Personas worn: **Free Palestine** on Jihad Enyo's toolbar
(`flash.webp`, `settings.webp`) and **Aurora Silk** on Jihad Mochi's (`addons-personas.webp`,
`settings.webp`), both credited above; `cardview.jpg` shows the interfaces unthemed, on a backdrop
drawn by `readme-art.py` with the Persona primitives (the project's own, MPL-2.0). The `about:personas`
gallery in `addons-personas.webp` also lists, with their thumbnails, the project's own Lime Fizz,
Bubble Lagoon, Cloud Nine and Meadow Morning (MPL-2.0, drawn by `make-personas.py`) and two
Personas by other authors, used under their Creative Commons licences, the pictures unmodified
apart from being cut and scaled into the gallery's rows:

| Persona | Author | Licence | Source |
|---|---|---|---|
| Space Fantasy | fx5800p | [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/) | [addons.mozilla.org](https://addons.mozilla.org/firefox/addon/space-fantasy/) |
| Linen Light | DVemer | [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/) | [addons.mozilla.org](https://addons.mozilla.org/firefox/addon/linen-light/) |

## The filter lists

The content blocker uses **EasyList** and **EasyPrivacy**, by The EasyList authors
(https://easylist.to/), bundled as unmodified snapshots of upstream version 202610151429 (in the
source tree `third_party/easylist/`, in every package the engine's `blocklists/` directory) and
refreshed from easylist.to. Their authors offer them under the GNU GPL v3 or later, or CC BY-SA 3.0
or later; Jihad Browser uses them under the GPL v3 or later (`licenses/GPL-3.0.txt`, in every
package).

## Everything else

See `NOTICE` (third-party software) and `LICENSE`.

## The logo and the icons

`docs/jihad-browser-logo.png` was supplied by the project owner (NotAlexNoyle) in 2026; the app
icons (`app*/icon*.png`, `packaging/branding/content/*.png`) were derived from it by the project
(a checkerboard keyed out, resized). They are the project's own, under the project's licence.

## Screenshots

`docs/screenshots/addons-personas.webp` shows two Personas by other authors (credited above); as a
derivative of CC BY-SA 3.0 works it is itself CC BY-SA 3.0
(https://creativecommons.org/licenses/by-sa/3.0/). `flash.webp` and `settings.webp` show Free
Palestine, a derivative of a CC BY 2.0 photograph (credited above). The other screenshots
show web pages as a browser shows them.
