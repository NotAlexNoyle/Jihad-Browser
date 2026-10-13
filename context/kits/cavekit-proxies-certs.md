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
  - `/etc/ssl/certs/trustedcerts/*.pem` — the system roots. HP's 3.0.5 shipped 212 files (211
    distinct roots); CE 3.1.0 ships 190, of which 63 are HP's and 127 came with CE's update (the
    community TLS work built in).
  - `/var/ssl/trustedcerts/<hash>.0` — what webOS's OpenSSL trusts: links to the system roots, plus
    links for certificates trusted in the Certificate Manager app (`libPmCertificateMgr`, files in
    `/var/ssl/certs`, listed in `/var/ssl/index.txt`).
  - `/etc/ssl/certs/ca-certificates.crt` — a bundle (one certificate on 3.0.5, the 190 on CE).
  - `/usr/lib/ssl/{cert.pem,certs}` — where the community OpenSSL 1.1.1 (`/usr/lib/ssl11`) looks;
    `certs` is a link to `/etc/ssl/certs`.
  - webOS's proxy setting: `com.palm.connectionmanager/getNwProxiesConfig` (since webOS 2.2.4;
    `{"returnValue":true,"proxyInfoList":[]}` on the device, nothing set); entries carry
    `proxyConfigType` (`noProxy`, `manualProxy`, `autoConfigUrl`, `autoDetectFromNetwork`),
    `proxyServer`, `proxyPort`, `proxyAutoConfigUrl`. The engine's own "system proxy" (type 5) reads
    GNOME's settings and is not built into the headless engine, so it silently goes direct here.
- **WebOne** (v0.18.3): an HTTP proxy on port 8080 that re-signs every HTTPS site it carries with
  its own CA (SHA-1 by default, which the engine allows under a root a user added); the CA is served
  at `/weboneca.crt` (DER) and `/weboneca.pem`, a PAC file at `/auto.pac`.
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
  through untouched. Amended the same day: "make using riverstone relay ca an option if riverstone
  relay proxy is installed but off by default" — trusting the Relay's CA is a setting, off by
  default, offered only while the Relay is the proxy in use.
- **The Relay's access:** "It should be open" — no Relay-specific login handling.
- **The Relay and users' addresses:** "I do want the relay to hide users addresses" (later the same
  day, after the device showed Squid's `X-Forwarded-For` carrying the user's address on plain http).
  A server change, sent to the server agent.
- **A proxy's own CA (WebOne, a self-hosted bumping Squid):** "Ask when opening it" — opening the
  proxy's CA certificate link asks whether to trust it to identify websites (name and SHA-256 shown),
  the standard browser way; trusted in Jihad only, listed and removable in Settings.
- **Default:** "No proxy". webOS's own proxy setting (`com.palm.connectionmanager`, which apps like
  webOS Archive Proxy set) is offered as a choice, not the default.

## Requirements

### R1: Proxies are set on the settings page
**Description:** about:settings has a Proxy section where a user picks how Jihad reaches the web,
in all three shells, without about:config.
**Acceptance Criteria:**
- [x] Settings → Advanced has a **Proxy** section (`about:settings#pane=advanced&section=proxy` opens
  it) offering: **No proxy** (the default), **Use webOS's proxy setting**, **Riverstone Relay** (R2),
  **WebOne** (R3), **My own proxy** (manual: an HTTP proxy, which may also carry HTTPS, or a separate
  HTTPS proxy; a SOCKS 4 or 5 proxy, with DNS through the proxy for SOCKS 5; addresses that go
  direct), and **Automatic configuration URL** (a PAC file). *(Device, 2026-10-13: the section in
  Enyo, Mochi and Mojo; the card's list offers the six choices; WPAD appears only when in effect.)*
- [x] **Use webOS's proxy setting** reads `com.palm.connectionmanager/getNwProxiesConfig` through the
  daemon's own Luna connection when the daemon starts, when a card opens and when the section is
  shown, and applies it (manual → that proxy for HTTP and HTTPS; a PAC URL → that URL; auto-detect →
  the engine's WPAD; none → direct). The section shows what webOS said. If the service cannot be
  reached, Jihad goes direct and says so. *(Device, Enyo: webOS set to the PC proxy with
  `configureNwProxies`; Jihad followed it at the next card ("following webOS: … 8898"), the section
  read "Now: webOS's proxy: 192.168.1.245:8899", a page went through it; webOS's setting then
  cleared. The unreachable path is code-read: the daemon answers returnValue false.)*
- [x] What the section shows is read from the engine's own `network.proxy.*` prefs, so a value set in
  about:config shows here and the section never claims a proxy the engine is not using. The old
  single "Proxy" row is gone, and with it the engine's "system proxy", which does nothing in this
  build; a profile set to it is shown, and behaves, as No proxy. *(Device: prefs written between
  daemon starts read back as My own proxy, WebOne and PAC with their values. WebOne is shown only
  while the engine goes to the WebOne it names, the Relay only on its host and both ports
  (inspector, round 23).)*
- [~] A host must be a host name or address and a port 1–65535; a bad value is refused with a reason
  and nothing is changed; a locked pref stops the change before anything is written. A change
  applies to the next request without restarting the card or the daemon. *(Code; a change applied
  at once on the device. Typing a bad value needs the on-screen keyboard: the finger round.)*
- [x] Device, each shell: a page loaded through each mode reaches the server through the proxy (a
  proxy on the PC logs the request, and the CONNECT for an https page); No proxy goes direct again.
  *(Enyo: My own proxy (GET and CONNECT logged), PAC (the PAC fetched, then the page through the
  proxy it names), webOS's setting, WebOne, the Relay; No proxy picked in the section and the next
  page went direct. Mochi and Mojo: a page through the PC's login proxy.)*
**Dependencies:** none

### R2: The Riverstone Relay
**Description:** One choice sends Jihad through the owner's relay, with HTTPS still checked end to end.
**Acceptance Criteria:**
- [x] Choosing it sets `proxy.riverstonerelay.org`, port 3128, for HTTP and HTTPS, with no SOCKS and no
  login. The section says what the Relay is and that it is the owner's. *(Device: picked in Enyo's
  and Mochi's section, the prefs as stated; the section redraws within a second of the pick, 15 of
  15 picks in both shells. The "old picture" first recorded here (T-465) was a screenshot taken
  before the pick had run.)*
- [x] A CONNECT from Jihad carries Jihad's User-Agent (`… JihadBrowser/1.0`), and the server agent is
  sent a Squid rule that splices (passes through untouched) every CONNECT carrying it, placed before
  the bump rules, with the commands to check and load it and to check the result. *(Device: the
  CONNECT reads `Mozilla/5.0 (X11; Linux armv7l; rv:6.9) Goanna/20261012 JihadBrowser/1.0` (the
  engine's network UA), page requests the page UA; both carry `JihadBrowser/`. The server agent
  applied it 2026-10-13 17:07 UTC, `squid.conf` lines 31–33 above `ssl_bump peek step1`;
  `squid -k parse` clean; through the Relay, example.com's issuer is Cloudflare TLS Issuing ECC CA 3
  for `JihadBrowser/1.0` and `squid-proxy` for any other agent; rollback `jihad-splice.sh --undo`.)*
- [x] **The Relay's CA is trusted only by choice.** While the Relay is the proxy in use (chosen here,
  webOS's setting pointing at it, or a proxy typed by hand naming it), the section offers **Trust the
  Relay's certificate**, off by default, saying that with it on the Relay can read the pages it
  carries. Jihad carries the CA itself, pinned by its SHA-256 and its key in the component (the list
  script refuses another), and trusts it in memory only while the setting is on and the Relay is in
  use; switching either off stops the trust at once. Found in any webOS store, or offered through
  R6, it is never imported by those routes; the store import also never takes any squid-for-webos
  CA (a re-keyed Relay would look like one). *(Device, Enyo, the Relay bumping: example.com fails
  untrusted with the option off, loads with it on, fails at once when switched off; with webOS's
  setting on the Relay the switch is shown; the CA offered as a link is refused.)*
- [x] Device (after the server rule is in): an https page through the Relay loads, and its
  certificate is the site's own, not `squid-sslbump-for-webos`; a plain http page loads through it
  (`Via: … rs-relay-server`). With the rule missing, a bumped page asks about its certificate while
  the setting is off, and loads with the setting on. *(Device, Enyo, 2026-10-13, the Relay chosen and
  its CA off: https://example.com/ loads (certErr=0) and the padlock shows issuer Cloudflare TLS
  Issuing ECC CA 3, TLS 1.3. http://httpbin.org/ip says `108.52.189.109, 116.202.108.111` (the
  forwarded client, then the Relay) and its headers carry Squid's `Cache-Control: max-age=259200`;
  httpbin's front strips `Via`, so the server's address stands in for it. Over https the same pages
  show only `116.202.108.111` and no Squid header: the tunnel was not opened. The second half done,
  above.)*
- [ ] The Relay does not tell sites who its users are: a plain http page and a re-signed (bumped)
  https page fetched through it carry no `X-Forwarded-For` naming the user, and Jihad's spliced
  https never did. The server agent is sent the Squid change (`forwarded_for delete`) with checks
  before and after; then, on the device, http://httpbin.org/ip through the Relay shows only the
  server's address. *(Sent 2026-10-13, received 13:35; waiting on the reply.)*
**Dependencies:** R1

### R3: WebOne
**Description:** One choice points Jihad at a WebOne proxy the user runs.
**Acceptance Criteria:**
- [~] Choosing it asks for the WebOne machine's address and port (WebOne's default port is filled in)
  and sets it as the HTTP and HTTPS proxy. *(Device: the section with its values; typing them is the
  finger round.)*
- [x] The section says plainly what WebOne does to pages and to HTTPS for Jihad, and links to the
  WebOne machine's CA certificate (`http://<host>:<port>/weboneca.crt`), which opens R6's question.
  *(WebOne sends its CA as a download; Jihad opens it anyway, R6.)*
- [x] Device: a page loads through a WebOne instance run on the PC, an https page after its CA is
  trusted. *(WebOne 0.18.3 in a .NET 8 container on the PC: the link, Yes, then example.com over
  https through it. WebOne's default SHA-1 CA was refused by WebOne's own OpenSSL 3 ("ca md too
  weak"); `SslHashAlgorithm=SHA256` in webone.conf fixed it. Jihad itself accepts SHA-1 under a root
  the user added.)*
**Dependencies:** R1, R6

### R6: Opening a CA certificate asks whether to trust it
**Description:** As in Pale Moon, opening a CA certificate (`application/x-x509-ca-cert`) asks whether
to trust it to identify websites, so a proxy's own CA can be added from the proxy itself.
**Acceptance Criteria:**
- [x] Opening such a link raises a question on the card naming the certificate (its name and
  organisation, sanitised), who offered it, its SHA-256, and that a proxy with this certificate can
  read the pages it carries. Yes adds it to Jihad's own certificate database, trusted for websites
  only; No adds nothing. Only a v3 certificate whose basicConstraints says cA=TRUE is offered; any
  other (a site's own, a v1 one the engine would call a CA) is refused with a reason. *(Device:
  "Offered by 192.168.1.245:8897", Yes trusted it; a hand-built v1 self-signed certificate refused.)*
- [x] The question appears only for a certificate opened as the page itself: one in a frame (HTTP,
  data: or file:) is stopped, so a hidden frame cannot raise it. *(Device: a page with a data: and an
  HTTP certificate frame raised nothing; both logged as stopped.)*
- [x] The Relay's CA offered this way is refused (R2 is its one route), and so are a root Mozilla
  removed and a certificate Jihad already trusts from webOS. *(Device: the Relay's refused.)*
- [x] Settings → Security lists the CAs added this way (name, SHA-256) with a Remove for each;
  Remove stops the trust at once. *(Device: listed; Remove, then the re-signed page asked again.)*
- [x] Device: a CA served by the PC is trusted after Yes, and a page signed by it loads; after
  Remove it asks again.
**Dependencies:** none

### R4: The certificates webOS trusts, where they add something
**Description:** Jihad also trusts the CA certificates webOS trusts that were added after HP's image,
by the user or by a community update.
**Acceptance Criteria:**
- [x] Settings → Security has **Also trust certificates added to webOS** (on by default). With it on,
  Jihad reads webOS's stores (above) and trusts, for identifying websites, each v3 CA certificate
  (basicConstraints cA=TRUE) in them that is **not** one of HP's 3.0.5 roots, **not** a root Mozilla
  removed, **not** already Jihad's, **not** expired, and **not** the Relay's or another
  squid-for-webos CA (R2). A site's own certificate found there, v1 ones included, is not taken as a
  root and is counted as skipped. *(Device: 190 read, 5 trusted; a v1 self-signed certificate in
  Certificate Manager's store skipped as notCA.)*
- [x] The lists it checks against are in the source with where each came from (HP's 3.0.5 doctor:
  212 files, 211 distinct roots; Mozilla's from CCADB, 103, fetched 2026-10-13), and a script
  rebuilds them (`render/goanna/certs/make-cert-lists.py`).
- [x] The trust lives in memory only: nothing is written to the profile's certificate database, the
  stores are read again at every start, so a certificate removed from webOS is no longer trusted
  after the next start; switching the setting off stops the trust at once (TLS sessions cleared).
  *(Device: cert9.db held only NSS's untrusted intermediates afterwards.)*
- [x] The section shows how many certificates are trusted this way and lists them (name, store),
  and how many were skipped and why. *(Device, all three shells.)*
- [x] A store that is missing or unreadable, a file that is not a certificate, or a link that leads
  nowhere is skipped, logged once, and never stops the rest or the daemon. *(Code; the device's
  /usr/lib/ssl/cert.pem is absent and was skipped.)*
- [x] Device (CE 3.1.0): the five newer roots are trusted, HP's are not, nothing else is; a CA put in
  through Certificate Manager's library is trusted after a start and a page signed by it loads with
  no certificate question; removed again, it is not trusted after the next start; with the setting
  off, that page asks again. *(TrustAsia RSA/ECC, OISTE RSA/ECC G1, SwissSign 2022-1; the SwissSign
  test site loads on, fails off. The Certificate Manager CA placed as its library places one: a
  file in /var/ssl/certs and a hash link in /var/ssl/trustedcerts.)*
**Dependencies:** none

### R5: A proxy that asks for a login says it is the proxy
**Description:** A 407 from a user's own proxy is a login for the proxy, and the dialog says so.
**Acceptance Criteria:**
- [x] The login dialog for a proxy challenge names the proxy (host and port) and says it is the proxy
  asking, not the site; a site's 401 is unchanged. *(Device: Enyo and Mochi "The proxy
  192.168.1.245:8898 - jihad-test requires a username and password"; Mojo's is webOS's own sheet,
  titled "proxy 192.168.1.245:8898 - jihad-test".)*
- [x] Device: a page through a PC proxy that demands a login raises the dialog naming the proxy, and
  the right login loads the page. *(Enyo and Mochi, answered through their own dialogs by script:
  407, then 200 with the login. Typing it is the finger round.)*
**Dependencies:** R1

## Out of Scope
- Remembering a proxy's login across restarts (the engine keeps it for the daemon's life).
- A webOS-wide proxy for other apps; the stock browser's and Mail's TLS (the community updates).
- Server-side changes other than the Relay's splice rule, which the server agent applies.

## Cross-References
- cavekit-browser-services.md R5 — TLS and certificate errors (the dialog, Trust Once/Always, the
  write to webOS's store).
- cavekit-preferences-ui.md — the settings page these sections join.
- cavekit-addons-site.md — the same server runs the Relay.

## Changelog
- 2026-10-13: Created from the owner's request and decisions; the Relay, the device's stores and
  HP's 3.0.5 image measured. Same day: webOS's proxy setting (R1), the Relay's CA as an option off by
  default (R2), and opening a CA certificate asks (R6), from the owner's further answers. Review
  round 23 (inspector): only v3 CAs (R4, R6), the question only for a page itself and naming who
  offered it (R6), the Relay pinned in the component and recognised by key and subject (R2), its
  switch wherever it is in use (R2). Device evidence added. Same day: the server agent applied the
  Relay's splice rule; R2 met on the device (the site's own certificate through the Relay). Then
  the owner: the Relay hides users' addresses (R2, a new criterion, sent to the server); the
  stale-picture finding (T-465) withdrawn, a screenshot taken before the pick.
