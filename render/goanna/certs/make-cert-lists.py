#!/usr/bin/env python3
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
"""Jihad Browser — the lists components/jihadCerts.js checks webOS's certificates against.

Writes jihad-cert-lists.json beside this script (cavekit-proxies-certs.md R4):

  hpStock         the SHA-256 of every root HP's webOS 3.0.5 image carries in
                  /etc/ssl/certs/trustedcerts. Jihad does not take these from webOS: they are
                  either Jihad's own already or roots Mozilla removed or never carried.
  mozillaRemoved  the SHA-256 of every root Mozilla has removed from its store, from CCADB's
                  "Removed CA Certificate Report". A community update made from an older Mozilla
                  bundle may still carry one.
  relay           the Riverstone Relay's CA (riverstone-relay-ca.pem): never imported from a
                  store, trusted only through the Relay's own setting (R2).

Usage:
  make-cert-lists.py --rootfs nova-cust-image-topaz.rootfs.tar.gz --ccadb removed.csv
  make-cert-lists.py --rootfs … --ccadb https://ccadb.my.salesforce-sites.com/mozilla/RemovedCACertificateReportCSVFormat

The rootfs is inside HP's webOS Doctor for the TouchPad (webosdoctorp305hstnhwifi.jar →
resources/webOS.tar → nova-cust-image-topaz.rootfs.tar.gz). Needs openssl on PATH.
"""
import argparse
import csv
import datetime
import hashlib
import io
import json
import os
import re
import subprocess
import sys
import tarfile
import urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
# The Relay's CA as the kit records it (cavekit-proxies-certs.md) and components/jihadCerts.js pins it
# (RELAY_SHA256). A riverstone-relay-ca.pem that is anything else is refused, not written to the list.
RELAY_SHA256 = "b48f324daca7da1dd3f8b737834a6a5d66c61aabd1a0a55bd36e8cfa27fb03bb"
PEM_RE = re.compile(rb"-----BEGIN CERTIFICATE-----(.+?)-----END CERTIFICATE-----", re.S)


def der_of_pem(blob):
    """Every certificate in a PEM blob, as DER."""
    out = []
    for m in PEM_RE.finditer(blob):
        pem = b"-----BEGIN CERTIFICATE-----" + m.group(1) + b"-----END CERTIFICATE-----\n"
        der = subprocess.run(["openssl", "x509", "-outform", "DER"], input=pem,
                             capture_output=True, check=True).stdout
        out.append(der)
    return out


def hp_stock(rootfs):
    """SHA-256 -> file name for every certificate in the image's trustedcerts directory."""
    found = {}
    with tarfile.open(rootfs, "r:*") as tf:
        for m in tf.getmembers():
            name = m.name.lstrip("./")
            if not name.startswith("etc/ssl/certs/trustedcerts/") or not m.isfile():
                continue
            for der in der_of_pem(tf.extractfile(m).read()):
                found.setdefault(hashlib.sha256(der).hexdigest(), os.path.basename(name))
    if not found:
        sys.exit("no certificates under etc/ssl/certs/trustedcerts in " + rootfs)
    return found


def mozilla_removed(src):
    if re.match(r"https?://", src):
        req = urllib.request.Request(src, headers={"User-Agent": "Mozilla/5.0"})
        text = urllib.request.urlopen(req, timeout=60).read().decode("utf-8", "replace")
    else:
        text = open(src, encoding="utf-8", errors="replace").read()
    rows = {}
    for r in csv.DictReader(io.StringIO(text)):
        fp = (r.get("SHA-256 Fingerprint") or "").replace(":", "").strip().lower()
        if re.fullmatch(r"[0-9a-f]{64}", fp):
            rows[fp] = (r.get("Root Certificate Name") or "").strip()
    if len(rows) < 50:
        sys.exit("the CCADB report has only %d rows; refusing to write a short list" % len(rows))
    return rows


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--rootfs", help="HP's 3.0.5 nova-cust-image-topaz.rootfs.tar.gz; without it the HP list "
                    "already in jihad-cert-lists.json is kept (cavekit-certificate-trust.md R5: a refresh of the "
                    "removed report needs no image)")
    ap.add_argument("--ccadb", required=True, help="CCADB Removed CA Certificate Report (CSV file or URL)")
    ap.add_argument("--fetched", default=datetime.date.today().isoformat(),
                    help="the day the CCADB report was fetched (default: today)")
    args = ap.parse_args()

    if args.rootfs:
        stock = hp_stock(args.rootfs)
    else:
        prev = json.load(open(os.path.join(HERE, "jihad-cert-lists.json")))
        stock = set(prev["hpStock"]["sha256"])
        print("HP's list kept from the existing file: %d roots" % len(stock))
    removed = mozilla_removed(args.ccadb)
    relay_der = der_of_pem(open(os.path.join(HERE, "riverstone-relay-ca.pem"), "rb").read())[0]
    if hashlib.sha256(relay_der).hexdigest() != RELAY_SHA256:
        sys.exit("riverstone-relay-ca.pem is not the Relay CA the kit pins (%s); if the Relay was re-keyed, "
                 "update RELAY_SHA256 here, RELAY_SHA256 and RELAY_SPKI in components/jihadCerts.js, and the kit"
                 % hashlib.sha256(relay_der).hexdigest())
    # Re-armoured from the DER, so nothing but the one certificate reaches the list.
    relay_pem = subprocess.run(["openssl", "x509", "-inform", "DER"], input=relay_der,
                               capture_output=True, check=True).stdout

    out = {
        "_comment": "Generated by make-cert-lists.py; do not edit by hand. cavekit-proxies-certs.md R4.",
        "hpStock": {
            "source": "HP webOS 3.0.5 for the TouchPad (Wi-Fi doctor), /etc/ssl/certs/trustedcerts",
            "count": len(stock),
            "sha256": sorted(stock),
        },
        "mozillaRemoved": {
            "source": "CCADB, Mozilla's Removed CA Certificate Report",
            "fetched": args.fetched,
            "count": len(removed),
            "sha256": sorted(removed),
        },
        "relay": {
            "name": "Riverstone Relay (squid-sslbump-for-webos)",
            "sha256": hashlib.sha256(relay_der).hexdigest(),
            "pem": relay_pem.decode("ascii").strip(),
        },
    }
    path = os.path.join(HERE, "jihad-cert-lists.json")
    with open(path, "w") as f:
        json.dump(out, f, indent=1, sort_keys=False)
        f.write("\n")
    print("%s: %d HP roots, %d removed by Mozilla, the Relay %s"
          % (path, len(stock), len(removed), out["relay"]["sha256"][:16]))


if __name__ == "__main__":
    main()
