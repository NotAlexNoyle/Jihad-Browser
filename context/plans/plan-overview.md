---
created: "2026-06-30"
last_edited: "2026-08-10"
---

# Plan Overview

## Build Sites
| Site | File | Tasks | Done | Status |
|------|------|-------|------|--------|
| Jihad Browser — remaining open criteria | build-site.md | 56 | 1 | ACTIVE |
| Jihad Browser port (original, 2026-06-30) | build-site-2026-06-30.archived.md | 54 | — | ARCHIVED |

**The active site covers ONLY the criteria that are still open** — the 41 boxes marked `[~]` or
`[ ]` across `context/kits/` on 2026-08-10. The other 327 are `[x]` and are recorded as met on the
criteria themselves; restating them here would be a second source of truth that can drift from the
kits, which is the failure this project has hit repeatedly in its own docs.

The 2026-06-30 site is archived rather than deleted. It planned the port from nothing and is the
record of how the project got here, but as a work driver it is obsolete: it predates essentially
everything now built, so every one of its 54 tasks is either long done or re-scoped.

## Where the open work actually is

Eight of the 56 tasks are quarantined in the final tier as **BLOCKED-EXTERNAL** — they need
hardware this device does not have (no keyboard, no TouchPad Go), a human physically at the device
(a real tap, a real pinch, an untrusted-cert session), an interactive sudo password, or a user
decision. They are not schedulable and must never sit in front of work that is.

Two ordering facts drive the graph:
- The **codec verdict** (T-101) gates the whole `<video>` chain and is cheap — half the answer is
  already in `mozconfig.goanna-arm` (`--disable-alsa`/`--disable-pulseaudio`, so no cubeb output
  backend) against `MOZ_FFVPX`/`MOZ_FFMPEG`/`MOZ_FMP4` in `autoconf.mk` (decoders present).
- The **frame-pacing chain** (T-105 → T-123 → T-136) is the user-visible one. T-105 is DONE and
  its answer changed the shape of the rest: see its row.

Next: `/ck:make` to implement, or work Tier 0 directly. Judge the pacing tasks by the frame-gap
HISTOGRAM, never by average fps — the average read 35-42 fps for 30 fps content throughout the
entire period the animation looked worst.
