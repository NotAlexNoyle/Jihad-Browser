# Persona test assets

- `carbon_light-2.0.xpi` — **Carbon Light by Jaxivo** as addons.mozilla.org serves it
  (addons.mozilla.org/firefox/addon/carbon-light/, CC BY-SA 3.0; sha256
  `01f5dcc11c0d0ce7383cc6563e345530919185da2443ff6a962af5c8b319c1d5`), for the static-theme
  conversion path (`components/jihadPersonas.js`). `echo-server.py` serves its `header.png` straight
  out of this package (Jihad no longer bundles Carbon Light); its credit is the line above.
- `persona-header.png` — drawn for the tests by this project.
- `echo-server.py` — the host test server: `/persona?v=carbon` serves a theme site that installs
  Carbon Light as a lightweight theme (and `?v=1,2,…` test Personas with `persona-header.png`);
  `/themes` lists the `.xpi` files in `$JIHAD_THEMES_DIR` as a gallery; `/effects` + `/echo` are
  the settings effect audit.

(Until 2026-10-08 these tests used Water-Like by stick177, which is All Rights Reserved; it was
removed from the repository.)
