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
- [x] `tools/host/audit-roots.py` reads the engine's `certdata.txt`, lists every certificate with its label,
  SHA-256 and server-auth trust, and compares it with a reference `certdata.txt` (UXP `release`) and with
  Mozilla's removed-CA report by SHA-256; it prints the roots only Jihad has, the roots only the reference has,
  the trust differences, and the roots on the removed list, and exits non-zero when a removed root is trusted
  or a reference TLS root is missing. References are files on disk (`--reference`, `--removed`); `--fetch`
  downloads them from UXP's repository and the CCADB.
- [x] The platform import's list (`render/goanna/certs/jihad-cert-lists.json`) is checked the same way: every
  root of the removed report is in it, by SHA-256.
- [x] Each run is recorded under `docs/rounds/certs-<date>/` with the reference's commit and date.

### R2: The built-in roots are what UXP's release branch ships
*(2026-10-25: patch 0060 = release 037cc5f6; `check-engine-pins.sh` "the built-in roots are UXP release's of
2026-10-24"; the audit after the roll: 0 findings; the device's start line "built-in roots: 169 certificates,
118 trusted for sites, 0 on Mozilla's removed report" on all three variants.)*
**Description:** Jihad ships the root list Pale Moon and Basilisk ship (UXP `release`, today Mozilla's current
list): the removed roots gone, the new ones in, the trust bits as Mozilla sets them.
**Acceptance Criteria:**
- [x] The engine fork carries UXP `release`'s `certdata.txt` as a numbered patch (the queue reproduces the pin);
  `engine-version.env` records the roots' source commit, date and SHA-256, and `check-engine-pins.sh` checks the
  shipped file against it.
- [x] `audit-roots.py` passes against that reference: no removed root trusted, no reference TLS root missing, no
  trust difference.
- [x] The device's `libnssckbi.so` is the rebuilt one: `about:preferences` → Security (or the daemon's log at
  start) reports the roots' count and date, and a site chaining to a newly added root loads with no question
  while the platform-store setting is off (the SwissSign 2022 test site, which fails on the June 2025 list).
- [x] The 17 trust changes are applied as Mozilla ships them (the sunsets), and desktop Wikipedia, DuckDuckGo,
  GitHub, the add-on site and the owner's relay still load — the chains the web sends end at roots still trusted.
  *(2026-10-25: Wikipedia, DuckDuckGo, Google, SwissSign, DigiCert load on the new roots; github.com had not finished in 18 s — a slow page, no certificate line.)*

### R3: Nothing else is a trust anchor
*(2026-10-25 with the setting on: "webOS: 190 distinct certificate(s) in 3 store(s); trusting 0; skipped
stock 63, removed 12, builtin 115" — the five roots the import used to add are built in now. The override
is per host:port and certificate (`RememberValidityOverride`), never a CA. docs/CERTIFICATE-TRUST.md records
that UXP has no HSTS preload, no static pins and no enterprise-roots import.)*
**Description:** Only the built-ins, the audited platform import and the user's own exceptions are trusted.
**Acceptance Criteria:**
- [x] HP's 2011 roots are never trust anchors: with the Security setting on, the import skips every root in HP's
  list and every root on the removed report (today's report, refreshed with R5); with it off, nothing from the
  platform store is trusted. *(The filter exists — cavekit-proxies-certs.md R4; this criterion re-verifies it
  against the refreshed removed list and records the device's counts.)*
- [x] A Trust Always exception trusts that site's certificate only, never as a CA for other sites (the router
  case of R4's inspector round), re-verified on the device.
- [x] No preloaded pins, no HSTS preload and no enterprise-roots import exist in the engine (recorded, with the
  prefs that would turn them on named).

### R4: The trust decisions are verified on the device
**Description:** The bench shows the right answer for the cases a user meets.
**Acceptance Criteria:**
- [x] badssl.com: `untrusted-root`, `self-signed`, `expired`, `wrong.host` and `sha1-intermediate` each raise the
  SSL dialog naming the problem (unknown issuer ×2, expired, name mismatch, disabled signature algorithm);
  `sha256` and `tls-v1-2` load; `rc4`, `dh480`, `null` and `3des` are refused with the engine's alert
  (no cipher overlap / weak ephemeral DH). `revoked` (Let's Encrypt, CRL-only since 2025, no OCSP responder)
  LOADS: the engine has no CRL fetching and no CRLite, as Pale Moon has not — recorded, out of scope.
  Revocation with an OCSP responder IS enforced: ssl.com's and DigiCert's revoked demo hosts are refused
  (`SEC_ERROR_REVOKED_CERTIFICATE` over TLS 1.2; over TLS 1.3 the same refusal ends as
  `SSL_ERROR_RX_MALFORMED_SERVER_HELLO` — PSM's version-intolerance retry at TLS 1.2 tripping the server's downgrade sentinel; engine patch 0061 stops retrying on a certificate error, T-534; verified 2026-10-25 on the packaged build: both hosts now say `SEC_ERROR_REVOKED_CERTIFICATE`). Recorded with the daemon's
  `loaderr` lines: `docs/rounds/certs-2026-10-25/device-checks.txt` (2026-10-25).
- [x] A site under each of the ten newly added TLS roots loads with the platform setting OFF (the CCADB's
  test sites: OISTE ×2, SECOM 2024 ×2, SwissSign 2022, Telia v3 ×2, TrustAsia ×2, e-Szigno 2023); no live
  site under a removed root remains (Entrust's and GLOBALTRUST's test hosts are gone; telia.se now anchors at
  Telia Root CA v2), so the refusal is shown structurally: the daemon's start line counts the built-in
  roots on Mozilla's removed report — 0 after the roll (24 before), `components/jihadCerts.js _logBuiltins`.
- [x] The padlock names the site's issuer for a current chain (cavekit-proxies-certs.md R2's criterion, re-run
  2026-10-25 on the new roots).

### R6: The TLS floor is 1.2
**Description:** The engine's own default allows TLS 1.0 and 1.1 (`security.tls.version.min` = 1, kept
upstream for old sites); RFC 8996 deprecated both and every current browser refuses them by default.
**Acceptance Criteria:**
- [x] `packaging/prefs/jihad-platform-prefs.js` sets `security.tls.version.min` to 3; `tls-v1-0.badssl.com:1010`
  and `tls-v1-1.badssl.com:1011` are refused with the engine's alert (`SSL_ERROR_UNSUPPORTED_VERSION`),
  `tls-v1-2` loads. Measured before: both loaded (2026-10-25).
- [x] Settings → Connection Security → Minimum TLS version (the existing row) still lets a user lower it, and
  the fallback limit stays at the engine's 3 (no insecure version fallback).

### R7: Only an error the engine lets a user override gets the Trust dialog
**Description:** The daemon classed every failure of the security module on the document as a certificate
question (an SSL confirm with Trust Once / Trust Always). The engine's override check
(`DetermineCertOverrideErrors`) accepts only an unknown or invalid issuer, a disabled signature algorithm,
a weak key, a validity-time problem and a name mismatch; for anything else — a REVOKED certificate first
of all — the override is ignored and the reload fails the same way, so the dialog asked a question with no
answer (measured 2026-10-25: revoked.grc.com, DigiCert's revoked demos).
**Acceptance Criteria:**
- [x] `GoannaRenderPage.cpp` reads the handshake's own error (`nsITransportSecurityInfo.errorCode`, the
  nsresult's low half as the fallback) and raises the SSL confirm only for the engine's overridable set
  (`JihadOverridableCertError`); every other security failure is a failed load with the engine's alert.
- [x] On the device a revoked certificate with an OCSP responder gets the engine's alert and no Trust buttons
  (ssl.com's and DigiCert's demo hosts; DigiCert's `global-root-ca-revoked` came WITH a certificate and had
  put up Trust Always before — now "SSL_ERROR_RX_MALFORMED_SERVER_HELLO (-12259) is not an error the engine
  lets a user override"); `untrusted-root`, `expired` and `wrong.host` still get the dialog ("may be
  overridden — asking") and Trust Once on `untrusted-root` records the override and loads the page.
  Measured: the security info's errorCode was 0 on these, so the fallback (the socket's final error) decided.

### R5: The list stays current
**Description:** Refreshing the roots is a documented, scripted step, not an archaeology.
**Acceptance Criteria:**
- [x] `docs/CERTIFICATE-TRUST.md` says where the roots come from, how to re-run the audit, how to roll
  `certdata.txt` forward (fetch UXP `release`, commit on the fork, `patch-queue.sh --add`, `update-engine.sh
  --pins`, rebuild `libnssckbi.so`, bundle), and how to refresh the platform import's removed list
  (`make-cert-lists.py`).
- [x] `make-cert-lists.py` fetches the removed report from the current CCADB address (the 2026-10-13 one moved)
  and the list in the bundle is rebuilt from today's report.
- [x] `update-engine.sh` warns when the release branch's `certdata.txt` differs from the shipped one.

## Out of Scope
Cipher policy; CRL fetching and CRLite (none in UXP — a CRL-only revocation is invisible, R4); certificate
transparency (none in UXP); client certificates; the Relay's CA; DNS-level trust (DANE/DNSSEC: not in the
engine). The TLS floor (R6) and the overridable-error set (R7) are in scope since 2026-10-25.

## Cross-References
cavekit-proxies-certs.md R4, R6 — the platform import and the CA-file question. cavekit-engine-update.md — the
patch queue and the pins. cavekit-device-build.md — the bundle and TLS 1.3.

## Changelog
- 2026-10-25: created from the owner's direction; the baseline measured (24 removed roots shipped, 19 missing,
  17 trust differences; UXP release = Mozilla central).
- 2026-10-25 (later): R1–R5 done — the roll to UXP release 037cc5f6 (patch 0060), the device checks; R6 (TLS floor
  1.2) and R7 (the Trust dialog only for overridable errors) added from the device checks' findings.
- 2026-10-25 (night): T-534's cause found and patched (engine 0061: PSM's intolerance retry excluded certificate
  errors); the three variants pushed as packages (the dev copies became the build).
