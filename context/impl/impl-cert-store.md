---
created: "2026-08-15"
last_edited: "2026-08-15"
---

# TLS certificate handling and the webOS certificate store

Scope: `cavekit-browser-services.md` R5. What T-135 built, what it proved on desktop, and what
is still device-gated. Predecessor: T-121, which was read-only and produced the API surface in
the kit.

## 2026-08-15 — T-135: the three live defects are fixed; the platform write path is wired but unproven

Host-code and desktop-compile wave only. **No device, no ARM build.**

### What now happens on a certificate error

`BrowserPageGoanna::emitLocationAndTitle` (the R5 branch) does five things instead of two:

1. Pulls the DER off the captured `nsIX509Cert` (`GoannaRenderPage::GetCertDer` ->
   `nsIX509Cert::getRawDER`), armours it to PEM and writes it to a file.
2. Asks the engine WHAT is wrong with the certificate (`GetCertProblem`) and maps that to the
   ordinal the card's message table matches on.
3. Emits `msgSSLConfirm2(pipe, host, ordinal, certFile)` — both of the last two fields now
   carry real values.
4. Reads the THREE-WAY answer back ("0" reject / "1" trust always / "2" trust once) and applies
   the matching Goanna override: permanent for 1, temporary for 2.
5. Write-through to the platform store (`jihad::certstore::Install`), then unlinks the PEM.

### Defect 1 — the empty certFile

`msgSSLConfirm2(..., "")` made the card's "View Certificate" button dead by construction:
`CertificateDetail.certFileChanged` skips `com.palm.certificatemanager/getcertificatedetails`
whenever `certFile` is falsy, and only that service's reply re-enables the button.

Fixed by writing the real certificate out. **Where** it is written is a deliberate choice, in
`JihadCertStore.cpp::certDir()`:

- **`/var/ssl/jihad/<variant>`** when `/var/ssl` exists and a 0755 subdirectory under it is
  usable by us. That is the platform store root (upstream `Main.cpp:274-281` chmod/chowns it to
  `root.luna` at startup). We take our own subdirectory rather than writing beside the openssl
  hash symlinks.
- **`<runtime state dir>/certs`** otherwise — always writable, per-variant, R8-clean. This is
  the fallback that keeps the field NON-EMPTY when `/var/ssl` turns out not to be ours, which is
  the unverified case the kit flags. Desktop has no `/var/ssl` and lands here deterministically.

Per-variant either way, because the startup sweep deletes stale PEMs by directory and three
variant daemons can run at once.

The file is mode 0644 because `com.palm.certificatemanager` reads it from ANOTHER process. It is
unlinked once the dialog resolves (upstream did the same); the card fetches the details at
show time, which is strictly inside the window the daemon is blocked in.

### Defect 2 — the error code was an nsresult

The card matches `0`, `2-4`, `5-9`, `10-17`, `18-23`, `24-29`, `30/31/50` — OpenSSL
`X509_V_ERR_*` codes. We passed `mErrorStatus`, so no branch matched and `sslConfirmMessage`
was never given content.

Two mappers now live in `JihadCertStore.cpp`:

- `PalmSslErrorOrdinal(nsresult)` — decodes the security-module nsresult back to its NSPR error
  (`psm::GetXPCOMFromNSSError` stores `-1 * code` in the low 16 bits) and maps the SEC_/SSL_/
  mozilla::pkix cases the kit named. Generic = 27 ("is invalid"), never a raw nsresult.
- `PalmSslErrorOrdinalFor(Problem, nsresult)` — **preferred**, and the reason it exists is
  measured, not theoretical: **the document-stop nsresult for our own self-signed test is
  `0x805a2fe3`, which decodes to `SSL_ERROR_BASE + 29` (a transport-level "malformed server
  hello"), not to any certificate error at all.** The nsresult is simply not a reliable
  description of what was wrong with the certificate. `nsISSLStatus` is:
  `isUntrusted` / `isDomainMismatch` / `isNotValidAtThisTime`, plus `nsIX509Cert::isSelfSigned`
  and `nsIX509CertValidity::notAfter` to separate "expired" from "not yet valid". Those are
  captured in `PageChrome::NotifyCertProblem` alongside the certificate itself and reset in
  `BeginLoad`. The nsresult map is the fallback when no flags were captured.

Where the OpenSSL code and the card's wording disagree, the wording wins, because the ordinal's
only job is to select a sentence. Two places: expiry maps to **0** (the card's one explicit "is
expired") rather than OpenSSL's 10, and not-yet-valid maps to **10** ("has some invalid
information") rather than OpenSSL's 9, whose sentence is "could not be read completely" and is
untrue. Both are commented at the site.

The card is untouched — its table is upstream's and is shared with the stock Mojo webview.

### Defect 3 — "Trust Always" and "Trust Once" were the same thing

`awaitDialogReply` collapsed "1" and "2" into one bool and `AcceptCurrentCert` hardcoded
`temporary=true`. It now takes an optional `int* answer` carrying the raw choice, and
`AcceptCurrentCert(bool aPermanent)` passes `temporary=!aPermanent`.

Session tracking, three layers deep:

- `nsICertOverrideService` permanent override for "always" (this is what writes
  `cert_override.txt` in the profile — the temporary form writes nothing).
- Platform store: `CertInstallKeyPackage` then `CertAddAuthorizedCert` for always,
  `CertAddTrustedCert` for once. **Not swapped** — upstream's note says `CertAddAuthorizedCert`
  is the one that creates the correct openssl hash symlinks.
- Session serials are tracked per owning page and swept in `~BrowserPageGoanna`, **and**
  persisted to `<state>/cert-session-serials` and swept at STARTUP. The startup sweep is the new
  part: upstream's own comment (`BrowserPage.cpp:1904-1910`) flags that a daemon dying without
  destructing turns every session cert permanent, and upstream never swept. It runs from the
  first `BrowserPageGoanna` construction, which is the earliest point this task's file set owns
  and is still before any install this daemon can perform.

Improvement over upstream: when the trust call fails after a successful install, the
half-installed certificate is removed instead of being left in the store.

### The platform store module

`render/goanna/JihadCertStore.{h,cpp}` — self-contained, no NSS/XPCOM/OpenSSL headers. Only a
file path and an int serial ever cross the `libPmCertificateMgr` boundary, which is what keeps
the daemon off OpenSSL entirely (device has 0.9.8, the sysroot 1.0.0, `-lssl` would bake the
wrong SONAME).

`dlopen("libPmCertificateMgr.so", RTLD_NOW)` at first use, all nine entry points typedef'd from
the kit's call-site-derived list and NULL-checked. Only the five the write path calls are
REQUIRED (`CertInitCertMgr`, `CertInstallKeyPackage`, `CertAddAuthorizedCert`,
`CertAddTrustedCert`, `CertRemoveCertificate`); the other four are resolved anyway so the one
log line doubles as the device evidence for whether the read direction is reachable.
`CertInitCertMgr("/etc/ssl/openssl.cnf")` runs once, guarded, and a non-zero return is reported
but not treated as fatal.

Every failure path is one log line and `false`, and the caller treats that as "Goanna-only
trust". **An absent store never fails the dialog flow.**

### The READ direction is COSTED, not built

Direction: platform store -> `nsIX509CertDB::addCertFromBase64`, so an enterprise CA installed
through the Certificate Manager app becomes visible to Goanna. Not landed, and the reason is a
hard blocker rather than a schedule call:

- Enumeration needs `CertGetDatabaseInfo(CERT_DATABASE_SIZE, &n)`,
  `CertGetDatabaseStrValue(i, CERT_DATABASE_ITEM_SERIAL, buf, len)` and
  `makePathToCert(serial, buf, MAX_CERT_PATH)`. Call sites give us those NAMES. dlopen gives us
  functions, never enum values — and **the numeric values of `CERT_DATABASE_SIZE`,
  `CERT_DATABASE_ITEM_SERIAL` and `MAX_CERT_PATH` cannot be derived from a call site.** Guessing
  is not a degradable error: a wrong property selector makes the library fill `buf` with
  something else, and a wrong `MAX_CERT_PATH` is a stack write with the wrong bound.
- Trust scope: importing everything in the store as a CA trust anchor is a real trust-boundary
  change and has to be limited to the CA bucket, which is again `CERT_DATABASE_ITEM_*` knowledge.
- It has to run once after XPCOM init, i.e. in `EngineHost`, which this task does not own.

Cost once unblocked: recovering the enum values is one device session or one read of the
Open-webOS `pmcertificatemgr` source (`meta-webos/recipes-webos/pmcertificatemgr`), then roughly
120 lines (enumerate, read PEM, strip the armour, `addCertFromBase64(body, "C,,", name)`) plus an
`EngineHost` call site. Half a day, device-gated, **not startable before the enum values exist**.
The base64 armour it needs is already written (in reverse) as `armourPem()`.

## Desktop evidence (2026-08-15)

`bash build/desktop/build-daemon.sh` in the pinned container: compiles clean (only the
pre-existing ATK version-redefinition warnings), links, `DAEMON_UP`.

`build/desktop/build-tls-test.sh`: **TLS PASS**, unchanged — invalid cert detected, host and
security-module status correct, reject still aborts the load. The log line now reads
`RememberValidityOverride(127.0.0.1:18443, session)`.

Full daemon-plus-adapter drive against a self-signed HTTPS endpoint, answering the real dialog
FIFO (scratch harness, not committed):

```
cert: certificate files -> <state>/certs (/var/ssl not writable by us)
ssl: cert error on 127.0.0.1:18443 secInfo=1 prov=1 status=0 cert=1
cert: wrote 798-byte DER as PEM -> <state>/certs/jihad-cert-43-1.pem
dialog ssl-confirm -> card (pipe …) host=127.0.0.1 status=0x805a2fe3
  flags=1(untrusted=1 selfSigned=1 mismatch=0 badTime=0 expired=0) -> ordinal 18 cert=<…>.pem
dialog ssl-confirm -> ACCEPT (answer=1)
ssl: RememberValidityOverride(127.0.0.1:18443, permanent) rv=0x0
cert: libPmCertificateMgr.so unavailable (…) — Goanna-only trust
ssl: permanent override remembered for 127.0.0.1 — reloading
```

- The PEM was captured mid-dialog and `openssl x509` reads it: `subject=CN = 127.0.0.1`,
  SHA1 fingerprint **identical to the server's own certificate**. So the armour is correct and
  it is the right certificate. It is gone from disk after the dialog, as intended.
- `cert_override.txt` appeared in the profile with the `127.0.0.1:18443` entry — the permanent
  arm really is permanent.
- The same run with answer **"2"**: `RememberValidityOverride(…, session)` and **no**
  `cert_override.txt` at all. The two arms are now genuinely different.
- The absent platform store produced exactly one line and did not disturb the flow.

Also re-ran `build-rules-test.sh` (RULES PASS) to prove the harness build-script edit pattern.

## What is NOT proven, and how to prove it

- **Everything about `libPmCertificateMgr` itself.** Desktop only ever exercises the degrade
  path. First device check, in order: `ls /usr/lib/libPmCertificateMgr.so*`, then `nm -D` for the
  nine names, then whether the daemon's group covers `/var/ssl` (the log line says which
  directory it chose, so that answer is free). If the library is absent the fallback is
  file-level: write the PEM into the store directory and let
  `com.palm.certificatemanager/listcertificates` prove it took.
- **The card's dialog body.** The ordinal is right in the daemon log; that it renders the
  expected sentence needs one device screenshot.
- **The session-serial sweep with real serials.** No library on desktop means no serials, so
  neither the teardown sweep nor the startup sweep has ever removed anything. Both paths run and
  are inert.
- **ARM build.** Deferred to the next wave (another agent owned `out-arm` this wave).
  `build/webos-oe/build-daemon-arm.sh` has the one-line compile-list addition. Nothing in the new
  code is ARM-hostile: no 64-bit assumptions, no unaligned access, `%zu`/`(long)` casts on the
  printf paths, and no new link dependency beyond `-ldl`, which the daemon already carries.

## Traps

- `render/browserserver/Src/BrowserServer.cpp` and `Src/Main.cpp` still carry upstream's
  `USE_CERT_MGR` blocks and **neither file is compiled**. Finding `CertInitCertMgr` there is not
  evidence anything calls it. The live call is in `render/goanna/JihadCertStore.cpp`.
- `build/desktop/build-adapter-roundtrip.sh` was ALREADY broken before this task: it links
  `Main.cpp` without `JihadLunaService.o`, so it fails on `jihad::LunaServiceNameFor`. Unrelated
  to certificates; noted because it is the obvious harness to reach for and it will not run.
- Do not re-attempt recovering the certificate AFTER the failure via `securityInfo`/`SSLStatus`
  (null) or `nsIRecentBadCerts` (does not exist in UXP). `NotifyCertProblem` is the only source,
  and it is where the new problem flags are captured too.
