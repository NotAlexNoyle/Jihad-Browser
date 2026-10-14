# deploy/relay — what the Riverstone Relay runs for Jihad

The Riverstone Relay is the owner's Squid proxy (`squid-sslbump`, Squid 6.14 built from source at
`/usr/local/squid`, on a Devuan server). Jihad offers it as a proxy choice (Settings → Advanced →
Proxy → Riverstone Relay; `context/kits/cavekit-proxies-certs.md` R2). Two things on the Relay exist
for Jihad, and both were made by the two scripts kept here. They are the server's files with one
change: the admin's home directory in their paths reads `/home/<admin>` (the owner ships no login
name). `SHA256SUMS` is for the copies here; the server's own hashes are in the table. Nothing here
runs from this repository: the scripts are the record, and the server copies in
`~/squid-hardening/` are what the Relay's admin runs.

| Script | What it does to `/usr/local/squid/etc/squid.conf` | Applied |
|---|---|---|
| `jihad-splice.sh` | Above the first `ssl_bump` line: the marker comment, `acl jihad_browser browser JihadBrowser/`, `ssl_bump splice jihad_browser`. Jihad's HTTPS passes through untouched, so the browser verifies every site's certificate itself (the owner, 2026-10-13); every other client is bumped as before. | 2026-10-13 17:07 UTC (lines 31–33) |
| `hide-client-addresses.sh` | Above `cache deny all`: the marker comment and `forwarded_for delete`. Sites are no longer told who the Relay's users are; an existing `forwarded_for` line would be changed in place. `via` stays: it names the Relay, not the user. | 2026-10-13 18:12 UTC (lines 102–103) |

**How each is run:** as root, from the admin's terminal, `doas bash ~/squid-hardening/<script>`
(`--undo` takes its lines out again). `bash` is needed: the files are mode 0664, not executable
(jihad-splice.sh's own usage comment omits it). Both are idempotent: a second run prints "already in
… nothing to change".

**What each touches:** squid.conf is backed up first as `squid.conf.bak.<YYYY-MM-DD>-jihad` or
`-xff` beside it (`.1`, `.2`, … if the name exists; a backup is never overwritten); the new file is
installed with `install -m 0644 -o root -g root`; `squid -k parse` must succeed and print no
`| FATAL: ` or `| ERROR: ` line, or the backup is copied back and nothing is reloaded; then
`squid -k reconfigure` (in place, under the supervise-daemon `squid -N` setup). Each run writes its
evidence world-readable to `/home/<admin>/squid-diag/<script>-<UTC stamp>.txt` (`OUTDIR`), and
checks through the proxy at `127.0.0.1:3128` with `curl` (example.com's issuer per User-Agent;
httpbin.org/ip for a plain, a bumped and a spliced request). hide-client-addresses.sh also reads,
report only, the last line of the access log (client address masked), `logrotate.conf` and
`/etc/cron.d/squid-logrotate`. Neither sources or ships a helper.

**Assumptions:** Squid from source at `/usr/local/squid`; root via `doas`; `curl`; the proxy on
`127.0.0.1:3128`.

**The admin's login name** was in the scripts' absolute paths (`/home/<name>/squid-hardening`,
`OUTDIR=/home/<name>/squid-diag`) and is replaced by `<admin>` here, so these copies differ from the
server's by those bytes only (the owner, 2026-10-14: no login names shipped).

History: the splice rule and the `forwarded_for` change were asked of the server agent from the PC
on 2026-10-13 (T-460 and T-468 in `context/plans/build-site.md`); the scripts came back on
2026-10-14 (T-478). `docs/PICKUP.md` has the measurements before and after.
