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
