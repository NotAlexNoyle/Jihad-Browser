# cavekit-engine-update.md history, moved out of the kit by the 2026-11-07 sweep; verbatim.

## Scope, the 2026-10-08 inventory (removed 2026-11-07)

The engine is UXP (Pale Moon's platform) with our own branch of changes, embedded by a headless
daemon. An inventory taken the same day found: the patch queue had drifted from the engine
branch (eight commits and part of the squashed series missing — a desktop build lacked the Flash
child's self-drive switch); the engine docs described an older setup; the engine's own exports
were declared inline in two files, eleven of them weak (a dropped one fails silently); pins and
version strings lived in six places; and nothing could tell, before building, what a candidate
engine would break. The engine is 76 upstream commits behind.

## R8, criterion 3 (device, all three variants) (removed 2026-11-07)

- [x] Device, all three variants: the daemon starts with all optional exports, pages load and paint,
  an https page, Palefill enabled, a Persona painted, about:addons, about:config and about:settings
  open, a modern-CSS test page renders (`adoptedStyleSheets`, `revert-layer`), and `:has()` works
  when its switch is on. (The finger checks go to docs/FINGER-TESTS.md.) *(2026-10-12, a full
  variant push to each: "11 of 11 optional jihad_* exports present" in all three. Enyo: the feature
  page (render/goanna/test/modern-web.html) — before: no adoptedStyleSheets, replaceSync, URL.parse,
  revert-layer, :has; after: all yes, :has with its switch on (then off again); howsmyssl.com TLS 1.3
  "Good"; Space Fantasy worn and taken off; Palefill 1.30 enabled; about:config; Flash anim.swf at
  23–25 fps through the Thumb-2 plugin-container. Mochi: the feature page. Mojo: Wikipedia.)*
  *(2026-10-13, the rest (inspector round 22): Mochi and Mojo — howsmyssl.com TLS 1.3 "Good",
  Palefill 1.30 enabled in about:addons, about:config, about:settings, a Persona worn (Aurora Silk,
  Lime Fizz: the toolbar painted) and taken off; Enyo — about:addons (Palefill's preferences) and
  about:settings. All three on daemon `7b02e00b`.)*
