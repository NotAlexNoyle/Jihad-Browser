# EasyList and EasyPrivacy (bundled filter lists)

`easylist.txt` and `easyprivacy.txt` are **unmodified snapshots** of the two lists as published
at easylist.to, taken for the content blocker (`render/goanna/components/jihadContentBlocker.js`,
kit `context/kits/cavekit-content-blocking.md`):

| | EasyList | EasyPrivacy |
|---|---|---|
| Purpose | advertising hosts and the generic element-hiding rules | tracking hosts |
| Upstream version (`! Version:`) | 202610151429 | 202610151429 |
| Last modified (upstream) | 15 Oct 2026 14:29 UTC | 15 Oct 2026 14:29 UTC |
| Downloaded from | https://easylist.to/easylist/easylist.txt | https://easylist.to/easylist/easyprivacy.txt |
| SHA-256 | `241caa88e276e1707c605eb44bcf53053c2eaf8f108aabd278e29a5a2ee92102` | `744da111e46b1f0f14afc6423d09fd569dc86506272f2208c96ac820753392f8` |
| Authors | The EasyList authors (https://easylist.to/) | The EasyList authors |
| Licence | dual: GNU GPL v3 or later, **or** CC BY-SA 3.0 or later (https://easylist.to/pages/licence.html) | the same |
| Taken under | **GPLv3+** — the text ships with every package as `licenses/GPL-3.0.txt` | **GPLv3+** |

Nothing in either file has been changed. `build/webos-oe/make-device-bundle.sh` refuses to bundle
a file whose SHA-256 is not the one above, so a different snapshot cannot ship under this notice
by accident. To move to a newer snapshot: replace the files, update the versions, dates and hashes
here and in `make-device-bundle.sh`, and the lines in `NOTICE`.

## How they are used

Every Jihad Browser package carries both files raw in the engine's `blocklists/` directory. The
content blocker compiles them on the device, in JavaScript, into what it blocks with — host rules,
third-party-only and typed host rules, path-prefix rules, the exceptions (plain and optioned),
the generichide sites — the generic element-hiding stylesheet and the per-site hiding rules, and
caches the compiled form in the profile. The compiled form is a derivative of the lists and is covered by the
same licence. The blocker refreshes both lists from easylist.to no more often than every four
days (the lists' own `! Expires`); the bundled snapshot is the fallback when a download fails.

The lists are data the engine reads, not code linked into it: the GPL covers the list files and
what is derived from them, not the browser (Apache-2.0 and MPL-2.0 code, see `LICENSE`).

## Attribution

"EasyList and EasyPrivacy, by The EasyList authors, https://easylist.to/, used under the GNU
General Public License v3 or later." Named in `NOTICE`, `CREDITS.md` and about:jihad.
