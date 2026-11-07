# cavekit-addons-site.md history, moved out of the kit by the 2026-11-07 sweep; verbatim.

## R3, criterion 2 (removed 2026-11-07)

*(Since the inspector's 2026-10-11 review PHP takes the panel only from nginx's /panel/ location: /panel/../ and /panel/%2e%2e/ answer 404, in the smoke test.)*

## R3, criterion 3 (removed 2026-11-07)

*(Test: a POST without the token → refused.)* 

## R3, criterion 6 (removed 2026-11-07)

 Real IP: round 21, inspector — a grep across /etc/nginx deciding whether to install it was
  wrong both ways; moved into the server block.)*

## R3, criterion 9 (removed 2026-11-07)

 Codex reviews it again when its credits return, before the deploy.)*

## R5, criterion 5 (removed 2026-11-07)

*(Device: a tap-equivalent in Jihad installs one through the usual confirm.)* 

## R7, criterion 3 (removed 2026-11-07)

*(Device.)* 

## R9, criterion 2 (removed 2026-11-07)

*(Device: the CA is gone from the profile at the
  end of every test.)* *(Device, Enyo. The first restore's name-matching clean-up deleted everything in `jihad-themes/` and `extensions/` twice (BusyBox grep has no -x) — recovered, see docs/PICKUP.md. Since round 15 a restore swaps the folders back from whole copies: on a scratch profile on the device a test's added, deleted and rewritten files all came back md5-identical, an incomplete backup and a short folder copy each restored nothing, and a restore interrupted halfway finished on a second run; on Enyo itself two real cycles left its pictures, Palefill and Persona pictures md5-identical.)* *(Device,
  2026-10-12, round 17's md5-judged script under BusyBox 1.17: a scratch app's setup, test changes and
  restore left it md5-identical, a tampered backup was refused untouched, a restore cut off between
  its moves finished; real cycles on Enyo and Mojo left both profiles md5-identical.)*

## R10, criterion 4 (removed 2026-11-07)

*(Server: 43/43 at every update since
  2026-10-12, from the server itself; for `f9bcb79` (2026-10-14) also 43/43 from the PC through
  Cloudflare; Pale Moon installed a Persona from the live site and Jihad read
  the feed (the owner, 2026-10-12). Still the owner's: a colour Persona worn in Pale Moon and on the
  TouchPad from the live site.)* *(2026-10-26: the smoke test 43/43 from the PC through Cloudflare against `f9bcb79`
  (still the site's head; the certificate good until 2026-12-28); on the TouchPad, Dracula (a colour Persona) worn
  from the live site by the emulated finger in Enyo — Wear, Jihad's confirm, OK, the toolbar in its colours — and
  taken off again. Left: the same in Pale Moon on a desktop, the owner's.)*

## R13, criterion 2 (removed 2026-11-07)

*(Device, against the local copy.)* *(Device, Enyo: the test Persona's picture replaced, md5 changed.)* *(Device, Enyo, 2026-10-12: installed
  at 1.0.3, the site at 1.0.2 → "Your Personas are up to date.", at 1.0.4 → an Update card for 1.0.4.)*

## R13, criterion 3 (removed 2026-11-07)

*(Device, against the local copy, with a test extension at two versions.)* *(Device, Enyo: 1.0 → 1.1.)*

## R10, criterion 3 (removed 2026-11-07)

(Inspector, 2026-10-11 and
  rounds 23-24. Scratch harness of install.sh's site-block section: missing, already the
  repository's with a stale `.new`, edited without the include, the challenge block left, no
  certificate over the owner's block, untouched with a hash, edited after the hash. On the server,
  2026-10-13: install.sh ran for `68caed1` and `d9359ea`, recorded the hash, and said so.)

## R10, criterion 6 (removed 2026-11-07)

(Inspector, 2026-10-11. Scratch runs: a real datastore folder, a cut archive, a tar
  ending between members, a changed file and a stray file in the old copy were each refused with
  nothing changed; a rollback and its return worked; the links survived every case.)

## R10, criterion 9 (removed 2026-11-07)

(Inspector, 2026-10-12. A throwaway MariaDB 10.11: recorded on a fresh database; dumped and
  applied once on an existing one; nothing and no new dump on a re-run; a failing change not
  recorded, applied when fixed; a bad file name refused before the database is touched; through
  swap-in.sh: applied before the swap, a failing change leaving the live copy alone, a rollback
  naming the change. The local copy restarted onto its existing database: 34/34.)

## R10, criterion 10 (removed 2026-11-07)

Scratch: the check's well, unreachable, near-expiry, old-backup and first-backup
  cases; the panel's front page on the local copy for each state, markup shown as text. Cron and the
  login message on the server: code.)
