# Certificate trust — where Jihad's roots come from and how to keep them current

The kit: `context/kits/cavekit-certificate-trust.md`. The records: `docs/rounds/certs-<date>/`.

## The three sources of trust

1. **The engine's built-in roots** — NSS's `certdata.txt` (`third_party/uxp/security/nss/lib/ckfw/builtins/`),
   compiled into `libnssckbi.so` and shipped in every variant's `deviceroot/hl`. This is the list; it is UXP's
   `release` branch's file (what Pale Moon and Basilisk ship, which is Mozilla central's byte for byte), pinned in
   `build/engine/engine-version.env` as `ROOTS_SHA256` / `ROOTS_SOURCE` (the release branch's commit for the
   file) / `ROOTS_DATE`, and checked by `build/engine/check-engine-pins.sh`.
2. **The webOS platform store**, only under Settings → Security → *Also trust certificates added to webOS*
   (`jihad.security.webos_certs`, on by default): `components/jihadCerts.js` reads `/etc/ssl/certs/trustedcerts`
   and `/var/ssl` at every start and trusts, for sites, each root that is not HP's, not on Mozilla's removed
   report, not already built in, not expired, not the Relay's and not one of Jihad's own SSL-dialog installs.
   The lists it checks against are `render/goanna/certs/jihad-cert-lists.json`, made by `make-cert-lists.py`.
3. **The user's exceptions** — Trust Once / Trust Always from the SSL dialog, for that site's certificate only.

Nothing else: UXP has no HSTS preload list, no static key pins and no enterprise-roots import.

## The audit

```
tools/host/audit-roots.py --fetch /tmp/roots          # downloads UXP release's certdata and the CCADB report
tools/host/audit-roots.py --reference R --removed CSV  # or from files
```
It lists the engine's roots and says which are on Mozilla's removed report (by SHA-256), which the reference has
and the engine lacks, and where the server-auth trust differs; it names CAs with an incident history as a
tripwire to read; and it checks the platform import's list carries every removed root. Exit 0 means nothing to
do. Keep each run under `docs/rounds/certs-<date>/` with the report's CSV.

## Rolling the roots forward

1. `curl -L -o certdata.txt https://repo.palemoon.org/MoonchildProductions/UXP/raw/branch/release/security/nss/lib/ckfw/builtins/certdata.txt`
   and note the branch's last commit for the file
   (`…/api/v1/repos/MoonchildProductions/UXP/commits?path=security/nss/lib/ckfw/builtins/certdata.txt&sha=release&limit=1`).
2. Copy it over the engine's, commit on the fork branch (`third_party/uxp`, `jihad/uxp-<UXP_TAG>`), then
   `build/engine/patch-queue.sh --add <commit>` and `build/engine/update-engine.sh --pins`.
3. Set `ROOTS_SHA256`, `ROOTS_SOURCE`, `ROOTS_DATE` in `engine-version.env`; `check-engine-pins.sh` must agree.
4. Rebuild the engine (`in-container.sh --arm build-goanna-arm.sh build` — incremental, `libnssckbi.so` is what
   changes), `make-device-bundle.sh`, and ship `libnssckbi.so` with the next build of the three variants (or as
   a dev copy into each `deviceroot/hl`, the daemons restarted).
5. Run the audit against the same reference: it must pass. Then the device checks of the kit's R4.
6. `nssckbi.h`'s `NSS_BUILTINS_LIBRARY_VERSION` follows upstream; bump it only when upstream does.

## Refreshing the platform import's removed list

```
python3 render/goanna/certs/make-cert-lists.py --ccadb https://ccadb.my.salesforce-sites.com/mozilla/RemovedCACertificateReportCSVFormat
```
Without `--rootfs` the HP list already in the file is kept (HP's image is not needed for a refresh). The file
ships in the bundle with the components; the daemon reads it at start.

## 2026-10-25

The engine (RB_20261006, roots of 2025-06-23) shipped 24 roots Mozilla had removed — Entrust and the four
AffirmTrust roots, GLOBALTRUST 2020, the CommScope four, the SSL.com-acquired Trustwave / SecureTrust / Secure
Global / XRamp, TeliaSonera v1, certSIGN, ePKI, SecureSign CA12, FIRMAPROFESIONAL ROOT-A WEB, the three emSign
roots (removed upstream the day before after the eMudhra compromise) — and lacked 19 current ones, ten of them
TLS roots. Rolled to UXP release 037cc5f6 (2026-10-24) as patch 0060: 169 certificates, 118 trusted for sites.
