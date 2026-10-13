---
created: "2026-10-13"
last_edited: "2026-10-13"
complexity: complex
---

# Cavekit: Proxies and webOS's Certificates

## Scope
The owner, 2026-10-13: "add support for riverstone relay proxy, and webone, and self hosted proxies
in jihad. also add support for system certificates including community patches if installed."

Jihad does its own TLS (the engine's NSS, TLS 1.0–1.3, its own 177 roots) and has never read
webOS's certificate stores. Its network settings had one row for proxies, "Proxy", whose "Manual"
choice said "set in about:config". This kit makes proxies something a user sets on the settings
page, and lets Jihad trust the certificates webOS trusts where that adds something.

**What was found (2026-10-13):**
- **The Riverstone Relay** is `proxy.riverstonerelay.org:3128`: Squid 6.14 on the owner's server
  (`Via: 1.1 rs-relay-server (squid/6.14)`), open, no login. It bumps HTTPS: every site comes back
  signed by its own CA, `O=squid-sslbump-for-webos, OU=Proxy, CN=squid-proxy`, RSA 1024, SHA-256
  `B4:8F:32:4D:AC:A7:DA:1D:D3:F8:B7:37:83:4A:6A:5D:66:C6:1A:AB:D1:A0:A5:5B:D3:6E:8C:FA:27:FB:03:BB`,
  valid to 2036. Bumping exists for webOS's own old TLS; Jihad has no need of it.
- **webOS's stores** (the device, webOS CE 3.1.0; HP's 3.0.5 Wi-Fi doctor for the stock image):
  - `/etc/ssl/certs/trustedcerts/*.pem` — the system roots. HP's 3.0.5 shipped 212; CE 3.1.0 ships
    190, of which 63 are HP's and 127 came with CE's update (the community TLS work built in).
  - `/var/ssl/trustedcerts/<hash>.0` — what webOS's OpenSSL trusts: links to the system roots, plus
    links for certificates trusted in the Certificate Manager app (`libPmCertificateMgr`, files in
    `/var/ssl/certs`, listed in `/var/ssl/index.txt`).
  - `/etc/ssl/certs/ca-certificates.crt` — a bundle (one certificate on 3.0.5, the 190 on CE).
  - `/usr/lib/ssl/{cert.pem,certs}` — where the community OpenSSL 1.1.1 (`/usr/lib/ssl11`) looks;
    `certs` is a link to `/etc/ssl/certs`.
  - No system-wide proxy setting exists on webOS 3 (see R1).
- **Against the engine's own list** (NSS in UXP RB_20261006, 177 roots) and Mozilla's record of
  removed roots (CCADB "Removed CA Certificate Report", 103 entries, fetched 2026-10-13): of CE's 190,
  122 are already Jihad's, 63 are HP's, none is a removed root, and **5 are newer than Jihad's NSS**
  (TrustAsia TLS RSA and ECC, SwissSign RSA TLS 2022-1, OISTE Server RSA and ECC G1). Of HP's 212,
  25 are Jihad's, 25 Mozilla has removed, and 162 Mozilla never carried.
- **The engine:** UXP never takes a site's own certificate as a trust anchor (end entities are
  trusted per site, through the certificate override service), so only CA certificates can be
  imported as roots. Certificates made with `CERT_NewTempCertificate` and trusted with
  `CERT_ChangeCertTrust` live in memory only (UXP's own enterprise-roots import works this way).
  Necko already sends `User-Agent` on a proxy CONNECT (`nsHttpConnection::MakeConnectString`), and
  `JihadPrompter::PromptAuth` already answers a 407.

**The owner's decisions (2026-10-13):**
- **Which webOS certificates:** "Added + patched" — trust certificates added in Certificate Manager
  and roots added by community patches or CE; skip HP's stock roots, roots Jihad already has, and
  roots Mozilla removed. One switch in Settings, on by default.
- **The Relay and HTTPS:** "Pass-through only" — a Squid rule on the server lets Jihad's HTTPS pass
  through untouched; **Jihad never trusts the Relay's CA**.
- **The Relay's access:** "It should be open" — no Relay-specific login handling.

## Requirements

### R1: Proxies are set on the settings page
**Description:** about:settings has a Proxy section where a user picks how Jihad reaches the web,
in all three shells, without about:config.
**Acceptance Criteria:**
- [ ] Settings → Advanced has a **Proxy** section (`about:settings#pane=advanced&section=proxy` opens
  it) offering: **No proxy**, **Riverstone Relay** (R2), **WebOne** (R3), **My own proxy** (manual:
  an HTTP proxy, which may also carry HTTPS, or a separate HTTPS proxy; a SOCKS 4 or 5 proxy, with
  DNS through the proxy for SOCKS 5; addresses that go direct), and **Automatic configuration URL**
  (a PAC file).
- [ ] What the section shows is read from the engine's own `network.proxy.*` prefs, so a value set in
  about:config shows here and the section never claims a proxy the engine is not using. The old
  single "Proxy" row is gone. "Use the system proxy" is not offered, because webOS has no proxy
  setting for it to read (the engine's Linux system proxy is GNOME's or the `*_proxy` environment,
  neither of which a webOS card has); a profile already set to it shows as "No proxy" with a note.
- [ ] A host must be a host name or address and a port 1–65535; a bad value is refused with a reason
  and nothing is changed. A change applies to the next request without restarting the card or the
  daemon.
- [ ] Device, each shell: a page loaded through each mode reaches the server through the proxy (a
  proxy on the PC logs the request, and the CONNECT for an https page); No proxy goes direct again.
**Dependencies:** none

### R2: The Riverstone Relay
**Description:** One choice sends Jihad through the owner's relay, with HTTPS still checked end to end.
**Acceptance Criteria:**
- [ ] Choosing it sets `proxy.riverstonerelay.org`, port 3128, for HTTP and HTTPS, with no SOCKS and no
  login. The section says what the Relay is and that it is the owner's.
- [ ] A CONNECT from Jihad carries Jihad's User-Agent (`… JihadBrowser/1.0`), and the server agent is
  sent a Squid rule that splices (passes through untouched) every CONNECT carrying it, placed before
  the bump rules, with the commands to check and load it and to check the result.
- [ ] **Jihad never trusts the Relay's CA.** Its SHA-256 is pinned in Jihad as a certificate never to
  import (R4), whatever store it is found in. If the Relay re-signs a site, Jihad shows its normal
  certificate error and accepting nothing changes that.
- [ ] Device (after the server rule is in): an https page through the Relay loads, and its
  certificate is the site's own, not `squid-sslbump-for-webos`; a plain http page loads through it
  (`Via: … rs-relay-server`).
**Dependencies:** R1

### R3: WebOne
**Description:** One choice points Jihad at a WebOne proxy the user runs.
**Acceptance Criteria:**
- [ ] Choosing it asks for the WebOne machine's address and port (WebOne's default port is filled in)
  and sets it as the HTTP and HTTPS proxy.
- [ ] The section says plainly what WebOne does to pages and to HTTPS for Jihad, and what Jihad needs
  for WebOne's HTTPS to work (its CA added in webOS's Certificate Manager, trusted through R4), from
  WebOne's own documentation.
- [ ] Device or PC: a page loads through a WebOne instance run on the PC.
**Dependencies:** R1, R4

### R4: The certificates webOS trusts, where they add something
**Description:** Jihad also trusts the CA certificates webOS trusts that were added after HP's image,
by the user or by a community update.
**Acceptance Criteria:**
- [ ] Settings → Security has **Also trust certificates added to webOS** (on by default). With it on,
  Jihad reads webOS's stores (above) and trusts, for identifying websites, each CA certificate in
  them that is **not** one of HP's 3.0.5 roots, **not** a root Mozilla removed, **not** already
  Jihad's, **not** expired, and **not** the Relay's CA (R2). A site's own certificate found there is
  not taken as a root (the engine trusts those per site), and is counted as skipped.
- [ ] The lists it checks against are in the source with where each came from (HP's 212 from the
  3.0.5 doctor; Mozilla's from CCADB, with the date), and a script rebuilds them.
- [ ] The trust lives in memory only: nothing is written to the profile's certificate database, the
  stores are read again at every start, so a certificate removed from webOS is no longer trusted
  after the next start; switching the setting off stops the trust at once.
- [ ] The section shows how many certificates are trusted this way and lists them (name, store),
  and how many were skipped and why.
- [ ] A store that is missing or unreadable, a file that is not a certificate, or a link that leads
  nowhere is skipped, logged once, and never stops the rest or the daemon.
- [ ] Device (CE 3.1.0): the five newer roots are trusted, HP's are not, nothing else is; a CA put in
  through Certificate Manager's library is trusted after a start and a page signed by it loads with
  no certificate question; removed again, it is not trusted after the next start; with the setting
  off, that page asks again.
**Dependencies:** none

### R5: A proxy that asks for a login says it is the proxy
**Description:** A 407 from a user's own proxy is a login for the proxy, and the dialog says so.
**Acceptance Criteria:**
- [ ] The login dialog for a proxy challenge names the proxy (host and port) and says it is the proxy
  asking, not the site; a site's 401 is unchanged.
- [ ] Device: a page through a PC proxy that demands a login raises the dialog naming the proxy, and
  the right login loads the page.
**Dependencies:** R1

## Out of Scope
- Remembering a proxy's login across restarts (the engine keeps it for the daemon's life).
- Trusting the Relay's CA in any form (the owner's decision).
- A webOS-wide proxy for other apps; the stock browser's and Mail's TLS (the community updates).
- Server-side changes other than the Relay's splice rule, which the server agent applies.

## Cross-References
- cavekit-browser-services.md R5 — TLS and certificate errors (the dialog, Trust Once/Always, the
  write to webOS's store).
- cavekit-preferences-ui.md — the settings page these sections join.
- cavekit-addons-site.md — the same server runs the Relay.

## Changelog
- 2026-10-13: Created from the owner's request and decisions; the Relay, the device's stores and
  HP's 3.0.5 image measured.
