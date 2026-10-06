# Persona test assets (2026-10-06)

- `water-like-header.jpg` — the header image of **Water-Like by stick177** (addons.mozilla.org/firefox/addon/water-like/), taken from AMO's converted static theme `water_like-2.0.xpi` (sha256 `c6b8b50d556174bfeb280a254c69c3e5d17f58aec766100a9624367e4a356a04`); the persona's colours there are frame `#00ccff`, text `#000000`. The owner named this persona for testing all three shells.
- `water_like-2.0.xpi` — that package as AMO serves it, for the static-theme conversion path (`components/jihadPersonas.js`).
- `echo-server.py` — the host test server: `/persona?v=water` serves a theme site that installs Water-Like as a lightweight theme (and `?v=1,2,…` test personas); `/effects` + `/echo` are the settings effect audit; paths in it point at the scratch dir and must be adjusted to this directory.

The artwork is the author's; it is here as a test asset only, not shipped in any package.
