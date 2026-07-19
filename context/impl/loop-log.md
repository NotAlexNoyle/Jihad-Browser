---
created: "2026-07-19"
last_edited: "2026-07-19"
---
# Cavekit Loop Log

Build site: context/plans/build-site.md

### Iteration 1 (Wave 1) — 2026-07-18/19
- T-049: Mochi app shell + bundling — DONE. Files: app-mochi/appinfo.json, app-mochi/README.md, build/webos-oe/build-mochi-ipk.sh. Build P (ipk 1.4 MB, 394 entries, re-verified on merged main), Acceptance 4/5 ([~] dual-install device-gated). Commit c6d0e9a.
- T-054: TouchPad Go machine config — DONE. Files: build/webos-oe/conf/machine/{tenderloin,opal}.conf + include/jihad-touchpad.inc, recipes-jihad/*.bb, docs/DEVICE-BUILD.md. Parse-sane P, Acceptance 2/3 + 1 [~] (Opal install device-gated). Commit 41d49f0. Finding: both models one ARMv7 softfp binary set; only DPI differs.
- Infra: ck:task-builder agent def broken (tools: [All, tools] → zero tools) — used general-purpose agents. Worktrees branch from stale origin/main (50 behind) — agents fast-forward to local main first.
- Device: offline all wave (novacom -l empty) — T1–T5 retest still pending.
- Next: Wave 2 = T-050 + T-051 + T-052 (single packet, shared app-mochi/source surface), then T-053.

### Tier gate (after Wave 1) — 2026-07-19
- Codex cycle 1: 9 unique findings, 2 P1 (layer.conf missing; UI recipes depended on stock adapter). Fixed 8081387: layer.conf, browser-adapter-jihad recipe, PACKAGE_ARCH=all, ipk ships LICENSE/NOTICE + BUNDLED-VERSIONS provenance, pipefail globs, kit/doc honesty.
- Codex cycle 2: 6 P1, one class — OE skeletons non-executable (stub do_compile, external SRC_URI, LIC checksums, 2014-era class/override syntax, missing upstart job, Impl.so-in-app-bundle). Fixed 044f295 by honest re-scope: Full-OE path documented NON-RUNNABLE with gap list; daemon recipe installs event.d/jihad; underscore overrides; host-path leak out of BUNDLED-VERSIONS; impl statuses annotated; .claude/ untracked+ignored. F-390 NOTICE attribution → T-050. F-391 enforcement deferred, documented.
- 2-cycle cap reached → ADVANCE.

### Iteration 2 (Wave 2) — 2026-07-19 — IN FLIGHT
- Packet: T-050 (NOTICE/headers) + T-051 (WebView kind, MIME application/x-jihad-browser, frozen method set) + T-052 (Mochi shell layout). Single agent — shared app-mochi/source surface.
