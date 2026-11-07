# cavekit-certificate-trust.md history, moved out of the kit by the 2026-11-07 sweep; verbatim.

## Preamble, "What was found on 2026-10-25" (removed 2026-11-07)

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

## R4, criterion 1 (removed 2026-11-07)

Revocation with an OCSP responder IS enforced: ssl.com's and DigiCert's revoked demo hosts are refused
  (`SEC_ERROR_REVOKED_CERTIFICATE` over TLS 1.2; over TLS 1.3 the same refusal ends as
  `SSL_ERROR_RX_MALFORMED_SERVER_HELLO` — PSM's version-intolerance retry at TLS 1.2 tripping the server's downgrade sentinel; engine patch 0061 stops retrying on a certificate error, T-534; verified 2026-10-25 on the packaged build: both hosts now say `SEC_ERROR_REVOKED_CERTIFICATE`).
