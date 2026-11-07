# cavekit-tabbed-browsing.md history, moved out of the kit by the 2026-11-07 sweep; verbatim.

## Scope, the frozen-contract paragraph (removed 2026-11-07)

The one constraint that shapes everything: **the BrowserAdapter↔BrowserServer YAP contract is
frozen** (cavekit-ipc-contract.md R1). No new YAP command or message, no change to the adapter's
JS method set. Tabs therefore cannot be "the adapter gets a tab API". What the shells may use is
the daemon's OWN Luna service (`palm://net.riverstonerelay.jihadBrowser*/`), which is this
project's and already carries printing, notifications and `pageCertificate` (cavekit-ipc-contract.md
R4 precedent).
