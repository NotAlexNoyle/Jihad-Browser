#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. MPL-2.0.
"""engine-loose-files.py — cavekit-engine-update.md R6 (T-387): the engine's own files that this
project's engine branch changes and that ship as LOOSE files next to libxul (script modules,
components, chrome bindings), so a libxul push alone would leave them stale on the device.

Derived, not named: the files the branch changes since the release it sits on
(`git diff --name-only UXP_TAG UXP_PIN` in the engine tree), mapped to where the build put them
in dist/bin by the objdir's own records: the install manifest (`_build_manifests/install/dist_bin`),
the preprocessor's dependency files (`**/.deps/*.pp`, for preprocessed modules such as
XPIProvider.jsm) and dist/bin's links into the source (chrome files). Prints the dist-relative
paths that also exist in the bundle, one per line.

  engine-loose-files.py --objdir O --uxp TREE --base TAG --head REV [--bundle DIR] [--why]
"""
import argparse, glob, os, subprocess, sys

SRC_PREFIX = "/src/uxp/"   # the engine tree as the build container mounts it


def changed_files(uxp, base, head):
    out = subprocess.check_output(["git", "-C", uxp, "diff", "--name-only", base, head], text=True)
    return {l.strip() for l in out.splitlines() if l.strip()}


def source_rel(path, uxp):
    for p in (SRC_PREFIX, os.path.abspath(uxp) + "/"):
        if path.startswith(p):
            return path[len(p):]
    return None


def dist_map(objdir, uxp):
    """source path (engine-relative) -> set of dist/bin-relative destinations"""
    m = {}
    def add(src, dest):
        rel = source_rel(src, uxp)
        if rel:
            m.setdefault(rel, set()).add(os.path.normpath(dest))
    # 1. the install manifest: 1 symlink, 2 copy, 7 preprocess: type, dest, source, ...
    man = os.path.join(objdir, "_build_manifests", "install", "dist_bin")
    if os.path.exists(man):
        with open(man, encoding="utf-8", errors="replace") as f:
            for line in f.read().splitlines()[1:]:
                p = line.split("\x1f")
                if len(p) >= 3 and p[0] in ("1", "2", "7"):
                    add(p[2], p[1])
    # 2. the preprocessor's records: "<target>: <source> <includes...>"
    distbin = os.path.normpath(os.path.join(objdir, "dist", "bin")) + "/"
    for pp in glob.glob(os.path.join(objdir, "**", ".deps", "*.pp"), recursive=True):
        try:
            first = open(pp, encoding="utf-8", errors="replace").readline()
        except OSError:
            continue
        if ":" not in first:
            continue
        target, deps = first.split(":", 1)
        tgt = os.path.normpath(os.path.join(os.path.dirname(os.path.dirname(pp)), target.strip()))
        if not tgt.startswith(distbin):
            continue
        for d in deps.split():
            add(d, tgt[len(distbin):])
    # 3. dist/bin's links into the source (chrome files, jar.mn)
    root = os.path.join(objdir, "dist", "bin")
    for dp, dirs, files in os.walk(root):
        for n in files:
            fp = os.path.join(dp, n)
            if os.path.islink(fp):
                add(os.readlink(fp), os.path.relpath(fp, root))
    return m


def main():
    a = argparse.ArgumentParser()
    a.add_argument("--objdir", required=True)
    a.add_argument("--uxp", required=True)
    a.add_argument("--base", required=True)
    a.add_argument("--head", required=True)
    a.add_argument("--bundle")
    a.add_argument("--why", action="store_true", help="also print the source each path comes from")
    o = a.parse_args()
    changed = changed_files(o.uxp, o.base, o.head)
    m = dist_map(o.objdir, o.uxp)
    if not m:
        sys.exit("engine-loose-files: no install records in " + o.objdir)
    out = []
    for src in sorted(changed):
        for dest in sorted(m.get(src, ())):
            if o.bundle and not os.path.exists(os.path.join(o.bundle, dest)):
                continue
            out.append((dest, src))
    for dest, src in out:
        print(dest + ("  <- " + src if o.why else ""))


if __name__ == "__main__":
    main()
