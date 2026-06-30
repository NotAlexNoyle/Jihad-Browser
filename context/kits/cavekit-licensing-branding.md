---
created: "2026-06-30"
last_edited: "2026-06-30"
---

# Cavekit: Licensing & Branding Compliance

## Scope
Cross-cutting requirement that the composite work respects every component's
license and ships no protected trademark. Applies to all source domains.
Reference: `LICENSE`, `NOTICE`, `licenses/`.

## Requirements

### R1: Per-file license headers correct and intact
**Description:** Each file carries the right license header for its origin.
**Acceptance Criteria:**
- [ ] Files derived from the Apache-2.0 upstreams (UI, IPC daemon) retain their original Apache-2.0 headers unmodified.
- [ ] New engine-backend files carry an MPL-2.0 header.
- [ ] No upstream license header is removed or altered.
**Dependencies:** none

### R2: Accurate top-level LICENSE and NOTICE
**Description:** The repository documents its composite licensing and attributions.
**Acceptance Criteria:**
- [ ] `LICENSE` enumerates each component, its directory, and its license.
- [ ] `NOTICE` attributes HP, LG, and Mozilla/Moonchild as required.
- [ ] Full Apache-2.0 and MPL-2.0 texts are included under `licenses/`.
**Dependencies:** none

### R3: No protected trademarks shipped
**Description:** The shipped engine carries no Pale Moon/Basilisk/Moonchild/Mozilla branding.
**Acceptance Criteria:**
- [ ] The engine build is configured to remove Pale Moon/Basilisk branding (name, about pages, logos, default UA token).
- [ ] Shipped artifacts present only the "Jihad Browser" product name.
- [ ] A scan of shipped artifacts finds no "Pale Moon"/"Basilisk"/"Moonchild" branding strings.
**Dependencies:** cavekit-engine-embedding.md (R1)

### R4: MPL source availability preserved
**Description:** Copyleft obligations on MPL files are met.
**Acceptance Criteria:**
- [ ] Any modification to an MPL-2.0 file stays under MPL-2.0 and its source is available.
- [ ] The engine source origin and any patches are documented.
**Dependencies:** cavekit-engine-embedding.md (R4)

### R5: License compatibility documented
**Description:** The Apache + MPL combination is explained.
**Acceptance Criteria:**
- [ ] `LICENSE` states how the Apache-2.0 and MPL-2.0 parts combine (file-level copyleft, compatible).
**Dependencies:** none

## Out of Scope
- Choosing a product license for net-new project glue beyond stating it per file.
- Legal sign-off (this captures engineering compliance; not legal advice).

## Cross-References
- See also: cavekit-ui-shell.md, cavekit-ipc-contract.md, cavekit-engine-embedding.md, cavekit-browser-services.md

## Changelog
- 2026-06-30: Initial draft.
