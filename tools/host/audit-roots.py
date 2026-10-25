#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
"""Jihad Browser — audit the engine's built-in roots (cavekit-certificate-trust.md R1).

    tools/host/audit-roots.py [--certdata PATH] [--reference PATH] [--removed PATH] [--lists PATH] [--fetch DIR]

Reads NSS's certdata.txt (the engine's, third_party/uxp/security/nss/lib/ckfw/builtins/certdata.txt by
default), and compares it, by SHA-256 of each certificate, with
  - a reference certdata.txt (UXP's `release` branch: what Pale Moon and Basilisk ship),
  - Mozilla's removed-CA report (CCADB CSV, "SHA-256 Fingerprint" column),
  - a short list of CAs with a public compromise or misissuance history, matched by name as a tripwire,
and checks the platform import's list (render/goanna/certs/jihad-cert-lists.json) carries every removed root.
Exit 0 when nothing is wrong: no trusted root on the removed report, no reference TLS root missing, no trust
difference, no incident CA by name. --fetch DIR downloads the reference and the report into DIR first.
"""
import argparse, csv, hashlib, io, json, os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
CERTDATA = os.path.join(REPO, "third_party/uxp/security/nss/lib/ckfw/builtins/certdata.txt")
LISTS = os.path.join(REPO, "render/goanna/certs/jihad-cert-lists.json")
REFERENCE_URL = "https://repo.palemoon.org/MoonchildProductions/UXP/raw/branch/release/security/nss/lib/ckfw/builtins/certdata.txt"
REMOVED_URL = "https://ccadb.my.salesforce-sites.com/mozilla/RemovedCACertificateReportCSVFormat"
# CAs with a public compromise, misissuance or distrust history. A match by label is a tripwire to read, not
# a verdict: Comodo (2011 breach) and GlobalSign (2011) stayed trusted after their incidents, the rest were removed.
INCIDENTS = ["DigiNotar", "WoSign", "StartCom", "CNNIC", "TURKTRUST", "Camerfirma", "TrustCor", "e-Tugra",
             "E-Tugra", "Certinomis", "Symantec", "VeriSign", "GeoTrust", "thawte", "Thawte", "Entrust",
             "AffirmTrust", "GLOBALTRUST", "Visa", "PROCERT", "Certplus", "OpenTrust", "Izenpe",
             "MITM", "Comodo", "COMODO", "GlobalSign", "Hongkong Post Root CA 1"]
TRUSTED = "CKT_NSS_TRUSTED_DELEGATOR"

def parse(path):
    txt = open(path, encoding="utf-8", errors="replace").read()
    certs, trust = {}, {}
    for block in re.split(r"\n(?=CKA_CLASS )", txt):
        cls = re.search(r"^CKA_CLASS CK_OBJECT_CLASS (\S+)", block, re.M)
        lab = re.search(r'^CKA_LABEL UTF8 "(.*)"', block, re.M)
        if not cls or not lab:
            continue
        if cls.group(1) == "CKO_CERTIFICATE":
            m = re.search(r"^CKA_VALUE MULTILINE_OCTAL\n(.*?)\nEND", block, re.S | re.M)
            if not m:
                continue
            der = bytes(int(o, 8) for o in re.findall(r"\\([0-7]{3})", m.group(1)))
            certs[hashlib.sha256(der).hexdigest().upper()] = lab.group(1)
        elif cls.group(1) == "CKO_NSS_TRUST":
            sa = re.search(r"^CKA_TRUST_SERVER_AUTH CK_TRUST (\S+)", block, re.M)
            sha = re.search(r"^CKA_CERT_SHA1_HASH MULTILINE_OCTAL\n(.*?)\nEND", block, re.S | re.M)
            trust[lab.group(1)] = sa.group(1) if sa else "?"
    return certs, trust

def removed_rows(path):
    rows = list(csv.DictReader(io.StringIO(open(path, encoding="utf-8", errors="replace").read())))
    col = next((c for c in rows[0].keys() if "SHA-256" in c), None) if rows else None
    if not col:
        sys.exit("the removed report has no SHA-256 column: " + path)
    return {r[col].replace(":", "").upper(): r for r in rows}

def fetch(url, path):
    subprocess.run(["curl", "-sL", "--max-time", "90", "-o", path, url], check=True)
    if os.path.getsize(path) < 1000:
        sys.exit("fetch failed: " + url)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--certdata", default=CERTDATA)
    ap.add_argument("--reference", help="a certdata.txt to compare with (UXP release)")
    ap.add_argument("--removed", help="Mozilla's removed-CA report CSV")
    ap.add_argument("--lists", default=LISTS, help="the platform import's lists JSON")
    ap.add_argument("--fetch", metavar="DIR", help="download the reference and the report into DIR")
    a = ap.parse_args()
    if a.fetch:
        os.makedirs(a.fetch, exist_ok=True)
        a.reference = a.reference or os.path.join(a.fetch, "uxp-release-certdata.txt")
        a.removed = a.removed or os.path.join(a.fetch, "removed-ca-report.csv")
        fetch(REFERENCE_URL, a.reference); fetch(REMOVED_URL, a.removed)
    bad = 0
    ours, ot = parse(a.certdata)
    tls = sum(1 for v in ot.values() if v == TRUSTED)
    print("engine: %s" % os.path.relpath(a.certdata, REPO))
    print("  %d certificates, %d trusted for server authentication" % (len(ours), tls))
    hits = [l for l in ours.values() if any(k in l for k in INCIDENTS)]
    if hits:
        print("  incident CAs by name (read each): " + "; ".join(sorted(hits)))
    if a.removed:
        rem = removed_rows(a.removed)
        gone = [(ours[f], rem[f]) for f in ours if f in rem]
        print("removed report: %s (%d rows)" % (os.path.relpath(a.removed, REPO) if a.removed.startswith(REPO) else a.removed, len(rem)))
        if gone:
            print("  REMOVED BY MOZILLA BUT SHIPPED (%d):" % len(gone)); bad += 1
            for l, r in sorted(gone):
                print("    ! %s | NSS %s / Firefox %s | %s" % (l, r.get("NSS Release When Removed", "?"), r.get("Firefox Release When Removed", "?"), (r.get("Reason for Removal") or r.get("Comments") or "")[:80].replace("\n", " ")))
        else:
            print("  none of the engine's roots is on it")
        if a.lists and os.path.exists(a.lists):
            # make-cert-lists.py's shape: {"mozillaRemoved": {"source", "fetched", "count", "sha256": [hex, …]}, …}
            lst = json.load(open(a.lists))
            have = {h.replace(":", "").upper() for h in (lst.get("mozillaRemoved") or {}).get("sha256", [])}
            print("platform import's removed list fetched %s, %d roots" % ((lst.get("mozillaRemoved") or {}).get("fetched", "?"), len(have)))
            missing = [r.get("Root Certificate Name", "?") for f, r in rem.items() if f not in have]
            print("platform import's list: %s — %d of the report's %d roots present" % (os.path.relpath(a.lists, REPO), len(rem) - len(missing), len(rem)))
            if missing:
                print("  MISSING FROM THE LIST (%d): %s" % (len(missing), "; ".join(sorted(missing)[:12]) + (" …" if len(missing) > 12 else ""))); bad += 1
    if a.reference:
        ref, rt = parse(a.reference)
        print("reference: %s" % a.reference)
        print("  %d certificates, %d trusted for server authentication" % (len(ref), sum(1 for v in rt.values() if v == TRUSTED)))
        only_ours = sorted(ours[f] for f in ours if f not in ref)
        only_ref = sorted(ref[f] for f in ref if f not in ours)
        if only_ours:
            print("  only the engine has (%d): %s" % (len(only_ours), "; ".join(only_ours)))
        miss_tls = [l for l in only_ref if rt.get(l) == TRUSTED]
        if only_ref:
            print("  only the reference has (%d, %d of them TLS roots): %s" % (len(only_ref), len(miss_tls), "; ".join(only_ref)))
        if miss_tls: bad += 1
        both = [ours[f] for f in ours if f in ref]
        diff = sorted((l, ot.get(l), rt.get(l)) for l in both if ot.get(l) != rt.get(l))
        if diff:
            print("  trust differs (%d): " % len(diff) + "; ".join("%s %s->%s" % d for d in diff)); bad += 1
        if not only_ours and not only_ref and not diff:
            print("  identical set and trust")
    print("RESULT: " + ("OK" if bad == 0 else "ACTION NEEDED (%d findings)" % bad))
    return 1 if bad else 0

if __name__ == "__main__":
    sys.exit(main())
