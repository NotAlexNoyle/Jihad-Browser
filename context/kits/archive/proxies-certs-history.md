# cavekit-proxies-certs.md history, moved out of the kit by the 2026-11-07 sweep; verbatim.

## Scope (removed 2026-11-07)

Jihad does its own TLS (the engine's NSS, TLS 1.0–1.3, its own 177 roots) and has never read
webOS's certificate stores. Its network settings had one row for proxies, "Proxy", whose "Manual"
choice said "set in about:config". This kit makes proxies something a user sets on the settings
page, and lets Jihad trust the certificates webOS trusts where that adds something.

## R1, criterion 2 (removed 2026-11-07)

(Inspector round 24: the daemon cannot ask until
  it is on the bus, which can take minutes after a start, and going direct meanwhile sent pages past
  the proxy the user chose.) *(Device, Enyo: webOS set to the PC proxy with
  `configureNwProxies`; Jihad followed it at the next card ("following webOS: … 8898"), the section
  read "Now: webOS's proxy: 192.168.1.245:8899", a page went through it; webOS's setting then
  cleared. The unreachable path is code-read: the daemon answers returnValue false, and it cannot be
  brought about on demand.)*

## R2, criterion 1 (removed 2026-11-07)

*(Device: picked in Enyo's
  and Mochi's section, the prefs as stated; the section redraws within a second of the pick, 15 of
  15 picks in both shells. The "old picture" first recorded here (T-465) was a screenshot taken
  before the pick had run.)*

## R2, criterion 4 (removed 2026-11-07)

*(Device, Enyo, 2026-10-13, the Relay chosen and
  its CA off: https://example.com/ loads (certErr=0) and the padlock shows issuer Cloudflare TLS
  Issuing ECC CA 3, TLS 1.3. http://httpbin.org/ip says `108.52.189.109, 116.202.108.111` (the
  forwarded client, then the Relay) and its headers carry Squid's `Cache-Control: max-age=259200`;
  httpbin's front strips `Via`, so the server's address stands in for it. Over https the same pages
  show only `116.202.108.111` and no Squid header: the tunnel was not opened. The second half done,
  above.)*

## R6, criterion 1 (removed 2026-11-07)

*(Device:
  "Offered by 192.168.1.245:8897", Yes trusted it; a hand-built v1 self-signed certificate refused.
  Round 24, Enyo: the test CA as an HTTP link asked, "Offered by 192.168.1.245:8896", and unanswered
  was declined; a router's certificate (cA=TRUE, IP subjectAltName) as a link was refused:
  ""192.168.1.1" is not a certificate authority's certificate".)*

## R6, criterion 2 (removed 2026-11-07)

*(Device: a page with a data: and
  an HTTP certificate frame raised nothing; both logged as stopped. Round 24, Enyo: a hidden blob:
  frame carrying the test CA reached the import and was refused, no dialog open.)* (Inspector round
  24: the blob frame used to raise the question over the page. On this build a `.crt` opened from
  `file:` is `application/octet-stream` and downloads, so it never asks.)

## R4, criterion 1 (removed 2026-11-07)

*(Device: 190 read, 5 trusted; a v1 self-signed certificate skipped as notCA. Round 24,
  Enyo, three certificates placed as Certificate Manager places one: a router's with an IP
  subjectAltName skipped as siteLike, one without keyUsage as noCertSign, a CA with keyCertSign
  trusted (6); removed again, 5. Trust Once on a site with the router's certificate: the platform
  store took serial 4 and the component logged its SHA-256 as never to be taken; the card closed,
  the serial was swept.)* (Inspector round 24: the earlier cA=TRUE test let a Trust Always router
  certificate become a root for every site at the next start.)

## R4, criterion 6 (removed 2026-11-07)

file in /var/ssl/certs and a hash link in /var/ssl/trustedcerts.)*

## R5, criterion 1 (removed 2026-11-07)

*(Device: Enyo and Mochi "The proxy
  192.168.1.245:8898 - jihad-test requires a username and password"; Mojo's is webOS's own sheet,
  titled "proxy 192.168.1.245:8898 - jihad-test". Round 24: the marker is "[proxy] ", which no site's
  host can produce (a site named "proxy" could pose as one); Enyo and Mochi read "The proxy
  192.168.1.245:8897 - jihad-test requires…", Mojo's sheet "[proxy] 192.168.1.245:8897 -
  jihad-test".)*

## Changelog (removed 2026-11-07)

- 2026-10-13: Created from the owner's request and decisions; the Relay, the device's stores and
  HP's 3.0.5 image measured. Same day: webOS's proxy setting (R1), the Relay's CA as an option off by
  default (R2), and opening a CA certificate asks (R6), from the owner's further answers. Review
  round 23 (inspector): only v3 CAs (R4, R6), the question only for a page itself and naming who
  offered it (R6), the Relay pinned in the component and recognised by key and subject (R2), its
  switch wherever it is in use (R2). Device evidence added. Same day: the server agent applied the
  Relay's splice rule; R2 met on the device (the site's own certificate through the Relay). Then
  the owner: the Relay hides users' addresses (R2, a new criterion, applied and met); the
  stale-picture finding (T-465) withdrawn, a screenshot taken before the pick. Review round 24
  (the inspector, for Codex): R4 takes roots only and never a certificate Jihad's SSL dialog put in
  webOS's store; R6 asks once, only for a page, and refuses a site's certificate that says cA=TRUE;
  R5's marker cannot be a site's host; R1 keeps following webOS's last setting when a read fails.
