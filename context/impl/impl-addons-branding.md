---
created: "2026-08-02"
last_edited: "2026-08-02"
status: FIXED — about:addons renders on device
---

# `about:addons` was dead on a missing `chrome://branding/` package — fixed

**Symptom.** `about:addons` failed with
`No chrome package registered for chrome://branding/locale/brand.dtd`, and rendered nothing.
This had been recorded as a known blocker across several sessions and dismissed as "the branding
strip", with no fix attempted.

**Root cause.** `chrome/toolkit/content/mozapps/extensions/extensions.xul` opens with

```xml
<!DOCTYPE page [
<!ENTITY % brandDTD SYSTEM "chrome://branding/locale/brand.dtd" >
%brandDTD;
```

In Firefox/Pale Moon the `branding` chrome package is supplied by the **application**
(`browser/branding`), not by the GRE. This build embeds the GRE with no application above it, and
the branding strip (cavekit-licensing-branding.md R3) removed the vendor's — so `chrome://branding/`
resolved to nothing. A DTD that fails to load is a **hard XML parse error**, which is why the page
died before rendering a single element rather than merely showing an unstyled title.

The GRE *does* ship `chrome/en-US/locale/en-US/global/brand.dtd`, which is why this looked
addressed. It does not help: that file is **empty**, there is no `brand.properties` beside it, and
it is registered under `chrome://global/`, not `chrome://branding/`.

**Fix.** Ship our own branding package — our names, none of the upstream vendor's trademarked
assets, so the branding strip's intent is preserved.

- `packaging/branding/jihad-branding.manifest` — `content branding branding/content/` +
  `locale branding en-US branding/locale/en-US/`
- `packaging/branding/locale/en-US/brand.dtd` — `brandShorterName`, `brandShortName`,
  `brandFullName`, `vendorShortName`, `trademarkInfo.part1`
- `packaging/branding/locale/en-US/brand.properties` — same keys; toolkit reads
  `getString("brandShortName")`
- `packaging/branding/content/icon32.png` — the only `chrome://branding/content/*` reference in
  the whole toolkit tree is `icon32.png`; generated from `app/icon.png`

`make-device-bundle.sh` copies these into the GRE and appends `manifest jihad-branding.manifest`
to `chrome.manifest`. The append is guarded (a duplicate `manifest` directive re-registers the
package and warns on every start), and the four files are **asserted non-empty in the bundle** —
a branding package that silently fails to land breaks `about:addons` again with a symptom (XML
parse error) that points nowhere near the bundler.

**Verified on device 2026-08-02.** `palm-launch -p '{"target":"about:addons"}'`:

```
[jihad-bs] load done uri=about:addons
[jihad-bs] titleAndUrl title=[Add-ons Manager] uri=about:addons
```

and the fb1 capture shows the real Add-ons Manager: the **Extensions / Themes / Plugins**
category list, the "Search all add-ons" field, and the XUL chrome. No branding or chrome error
remains in the daemon log.

This closes the R6 AC4 blocker and unblocks the add-ons work (XPI install, extension effect,
NPAPI plugin listing) that could not previously be observed at all, because the manager itself
would not open.
