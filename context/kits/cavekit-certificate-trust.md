---
created: "2026-10-25"
last_edited: "2026-10-25"
---

# Certificate trust — the roots Jihad trusts, audited and kept current

**Why this kit exists.** The owner, 2026-10-25: "check the root/CA/DNS certs for ones that have been exploited.
remove any that shouldnt be there and compare them with the certs basilisk and pale moon ship, and pull them in
if they arent here. for making Jihad as secure as possible." A browser's trust anchors are the one list a user
cannot see and cannot fix: a compromised or abandoned root left in it lets a forged site pass the padlock, and a
current root missing from it fails an honest site. Jihad has three sources of trust and each needs its own
audit: the engine's built-in roots (NSS's `certdata.txt`, compiled into `libnssckbi.so`, shipped in the bundle),
the webOS platform store (`/etc/ssl/certs/trustedcerts`, `/var/ssl`: HP's 2011 roots plus what a community update
or the user added — read by `components/jihadCerts.js` under the Security setting, cavekit-proxies-certs.md R4),
and the user's own exceptions (Trust Once / Trust Always from the SSL dialog, cavekit-proxies-certs.md R6).

**What was found on 2026-10-25** (the measurements are the baseline every criterion below is judged against):
- The engine is UXP `RB_20261006` (NSS 3.90.14 UXP) whose `certdata.txt` was last rolled upstream on 2025-06-23:
  177 certificates, 150 trusted for server authentication.
- Mozilla central today (NSS 3.13x): 169 certificates, 118 trusted for server authentication. UXP's `master` and
  `release` branches carry that file byte for byte — so Pale Moon and Basilisk, built from `release`, ship today's
  Mozilla list, and Jihad ships the June 2025 one.
- Against Mozilla's removed-CA report (CCADB, by SHA-256): **24 of Jihad's roots have been removed** — the four
  AffirmTrust roots and Entrust Root Certification Authority (Entrust, after its 2024 distrust), GLOBALTRUST 2020,
  the four CommScope roots, Secure Global CA, SecureTrust CA, XRamp Global, the three Trustwave Global roots,
  TeliaSonera Root CA v1, certSIGN ROOT CA, ePKI Root, SecureSign Root CA12, FIRMAPROFESIONAL CA ROOT-A WEB and
  the three emSign roots. Already absent (removed long ago): DigiNotar (an explicit distrust record remains),
  WoSign/StartCom, Symantec/VeriSign/GeoTrust/thawte, CNNIC, TURKTRUST, Camerfirma, TrustCor, e-Tugra, Certinomis.
- **19 current Mozilla roots are missing**, ten of them for TLS: OISTE Server Root RSA/ECC G1, SECOM TLS RSA/ECC
  2024, SwissSign RSA TLS Root CA 2022-1, Telia RSA/EC TLS Root CA v3, TrustAsia TLS RSA/ECC, e-Szigno TLS 2023.
  On the bench five of these are trusted only through the webOS store (a community update put them there) and only
  while the Security setting is on.
- 17 roots Jihad still trusts for TLS that Mozilla now lists as `MUST_VERIFY_TRUST` for server authentication
  (the old-root sunsets and Entrust's G2/EC1: Comodo AAA, COMODO CA, DigiCert Assured ID / Global Root CA / High
  Assurance EV, Entrust G2 / EC1 / Premium 2048, GlobalSign Root CA, Go Daddy and Starfield Class 2, GTS Root R2,
  QuoVadis 2 / 3, SwissSign Gold, Atos 2011, Certigna).
- UXP has no HSTS preload list and no static key pins (Pale Moon dropped both); nothing of that kind ships.
- The platform import (R4 of cavekit-proxies-certs.md) filters by its own SHA-256 lists: HP's 211 roots and
  Mozilla's removed report of 2026-10-13 (103 rows; today's has 106).

## Scope
In: the built-in root list (what it contains, where it comes from, how it is refreshed), the platform import's
removed-root list, the audit that proves both, and the device checks. Out: TLS cipher and protocol policy (the
engine's defaults, TLS 1.3 verified on howsmyssl, cavekit-device-build.md), OCSP/CRL behaviour, the Relay's CA
(cavekit-proxies-certs.md R2), client certificates.

## Requirements

### R1: The trust anchors are inventoried and audited by a script
**Description:** One script says, from the repository alone, what Jihad trusts and what is wrong with it.
**Acceptance Criteria:**
- [ ] `tools/host/audit-roots.py` reads the engine's `certdata.txt`, lists every certificate with its label,
  SHA-256 and server-auth trust, and compares it with a reference `certdata.txt` (UXP `release`) and with
  Mozilla's removed-CA report by SHA-256; it prints the roots only Jihad has, the roots only the reference has,
  the trust differences, and the roots on the removed list, and exits non-zero when a removed root is trusted
  or a reference TLS root is missing. References are files on disk (`--reference`, `--removed`); `--fetch`
  downloads them from UXP's repository and the CCADB.
- [ ] The platform import's list (`render/goanna/certs/jihad-cert-lists.json`) is checked the same way: every
  root of the removed report is in it, by SHA-256.
- [ ] Each run is recorded under `docs/rounds/certs-<date>/` with the reference's commit and date.

### R2: The built-in roots are what UXP's release branch ships
**Description:** Jihad ships the root list Pale Moon and Basilisk ship (UXP `release`, today Mozilla's current
list): the removed roots gone, the new ones in, the trust bits as Mozilla sets them.
**Acceptance Criteria:**
- [ ] The engine fork carries UXP `release`'s `certdata.txt` as a numbered patch (the queue reproduces the pin);
  `engine-version.env` records the roots' source commit, date and SHA-256, and `check-engine-pins.sh` checks the
  shipped file against it.
- [ ] `audit-roots.py` passes against that reference: no removed root trusted, no reference TLS root missing, no
  trust difference.
- [ ] The device's `libnssckbi.so` is the rebuilt one: `about:preferences` → Security (or the daemon's log at
  start) reports the roots' count and date, and a site chaining to a newly added root loads with no question
  while the platform-store setting is off (the SwissSign 2022 test site, which fails on the June 2025 list).
- [ ] The 17 trust changes are applied as Mozilla ships them (the sunsets), and desktop Wikipedia, DuckDuckGo,
  GitHub, the add-on site and the owner's relay still load — the chains the web sends end at roots still trusted.

### R3: Nothing else is a trust anchor
**Description:** Only the built-ins, the audited platform import and the user's own exceptions are trusted.
**Acceptance Criteria:**
- [ ] HP's 2011 roots are never trust anchors: with the Security setting on, the import skips every root in HP's
  list and every root on the removed report (today's report, refreshed with R5); with it off, nothing from the
  platform store is trusted. *(The filter exists — cavekit-proxies-certs.md R4; this criterion re-verifies it
  against the refreshed removed list and records the device's counts.)*
- [ ] A Trust Always exception trusts that site's certificate only, never as a CA for other sites (the router
  case of R4's inspector round), re-verified on the device.
- [ ] No preloaded pins, no HSTS preload and no enterprise-roots import exist in the engine (recorded, with the
  prefs that would turn them on named).

### R4: The trust decisions are verified on the device
**Description:** The bench shows the right answer for the cases a user meets.
**Acceptance Criteria:**
- [ ] badssl.com: `untrusted-root`, `self-signed`, `expired`, `wrong.host` and `revoked` each raise the SSL
  dialog (or the error page) naming the problem; `sha256` and `tls-v1-2` load; `rc4`, `dh480` and `null` are
  refused. Recorded with the daemon's `loaderr` lines.
- [ ] A site under a newly added root loads (R2), one under a removed root is refused (a live one if any remains;
  else the removed root's test chain from badssl or a self-made chain served by the host server).
- [ ] The padlock names the site's issuer for a current chain (cavekit-proxies-certs.md R2's criterion, re-run).

### R5: The list stays current
**Description:** Refreshing the roots is a documented, scripted step, not an archaeology.
**Acceptance Criteria:**
- [ ] `docs/CERTIFICATE-TRUST.md` says where the roots come from, how to re-run the audit, how to roll
  `certdata.txt` forward (fetch UXP `release`, commit on the fork, `patch-queue.sh --add`, `update-engine.sh
  --pins`, rebuild `libnssckbi.so`, bundle), and how to refresh the platform import's removed list
  (`make-cert-lists.py`).
- [ ] `make-cert-lists.py` fetches the removed report from the current CCADB address (the 2026-10-13 one moved)
  and the list in the bundle is rebuilt from today's report.
- [ ] `update-engine.sh` warns when the release branch's `certdata.txt` differs from the shipped one.

## Out of Scope
TLS protocol and cipher policy; OCSP/CRL; certificate transparency (none in UXP); client certificates; the
Relay's CA; DNS-level trust (DANE/DNSSEC: not in the engine).

## Cross-References
cavekit-proxies-certs.md R4, R6 — the platform import and the CA-file question. cavekit-engine-update.md — the
patch queue and the pins. cavekit-device-build.md — the bundle and TLS 1.3.

## Changelog
- 2026-10-25: created from the owner's direction; the baseline measured (24 removed roots shipped, 19 missing,
  17 trust differences; UXP release = Mozilla central).
