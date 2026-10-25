---
created: "2026-06-30"
last_edited: "2026-10-25"
---

# Plan Overview

## Build Sites
| Site | File | Tasks | Done | Status |
|------|------|-------|------|--------|
| Jihad Browser — CE 3.1.0 open criteria (2026-10-01), tiers added through 90 (2026-10-25) | build-site.md | 7 | 7 | ACTIVE |
| Jihad Browser — remaining open criteria (2026-08-10) | build-site-2026-08-10.archived.md | 56 | — | ARCHIVED |
| Jihad Browser port (original, 2026-06-30) | build-site-2026-06-30.archived.md | 54 | — | ARCHIVED |

**Tiers 84–90 (2026-10-14 … 2026-10-25)** were added to the active site as the work went — the one-repository build, the GPU path, the finger round, the certificate-trust kit, the fixes the finger found. `tools/host/cavekit-sync.py` regenerates `.cavekit/tasks.json` and reconciles `.cavekit/task-status.json` from the site's tables, so Cavekit 3.1's loop sees every tier (run it after editing the site by hand).

**The active site covers ONLY the criteria that are still open** — the 53 boxes marked `[~]` (31)
or `[ ]` (22) across `context/kits/` on 2026-10-01, after the kits were brought in line with the
device (now webOS CE 3.1.0). The other 338 are `[x]` and are recorded as met on the criteria
themselves; restating them here would be a second source of truth that can drift from the kits.

The two older sites are archived rather than deleted: 2026-06-30 planned the port from nothing,
2026-08-10 (T-101…T-156) drove the 3.0.5-era open criteria. New rows that say "continues T-1xx"
inherit that task's remaining work and gating.

## Where the open work actually is

- **One device, one lane.** 51 of the 91 tasks touch the TouchPad and form a single total order
  (the site's `## Device lane`); host work runs beside it. All three variants stay on one engine +
  daemon build: fixes are batched, built, and pushed to all three together.
- **Order (the user's):** housekeeping → `<video>` decode fix (engine push 1a, verified before the
  daemon changes land) → idle daemon (log bound, idle tick; daemon push 1b) → `about:blank` repaint
  storm → fresh install / Luna registration / Atlas → TLS version capture → media controls → the
  remaining partials and the CE 3.1.0 re-verification sweep → batch 2.
- **13 tasks are BLOCKED-EXTERNAL** in the final tier: a human finger/eye/ear at the device, a
  keyboard that does not exist, a credentialed push, or an owner decision. They are not
  schedulable and never sit in front of work that is. A daemon-level inject is never counted as
  the human test.

- **Out of this plan: Global Jihad** (system-wide Goanna for every app) is a distinct project that
  begins only when Jihad Browser is finished; its strategy and the device facts are in
  `docs/PICKUP.md` ("After Jihad Browser: Global Jihad"). The bench is webOS CE 3.1.0 (updated
  from 3.0.5) and the only one; stock-only work waits for the owner.

Next: `/ck:make`. Judge any pacing task by the frame-gap HISTOGRAM, never by average fps.
