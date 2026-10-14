# tools/

Helpers for working on Jihad that are not part of the build itself.

| Directory | What | How |
|---|---|---|
| `host/` | `build-novacom.sh`: novacom and novacomd from the `webos-sdk-redux` submodule into `host/bin/`. `check-pins.sh`: every submodule pin fetched from its `.gitmodules` URL into a throwaway repository, so a clone elsewhere is known to get them | `tools/host/build-novacom.sh`, then `sudo -b tools/host/bin/novacomd`; `tools/host/check-pins.sh` before a tier that moves a pin is called done |
| `device/` | Small BusyBox scripts that run on the TouchPad: the three variants' state, a daemon's log, the adapter paint log, prefs with the daemon stopped, `/` back to read-only, the certificate store, the self-test log | `build/webos-oe/dev-sh.sh tools/device/<script> [args]` (each script's header says what it takes) |
| `test-servers/` | `cert-server.py`: certificate files served the way a router offers its CA | `tools/test-servers/cert-server.py --port 8896 /ca.crt=testca.crt` |
| `review/` | `codex-review.sh`: Cavekit's adversarial review of a commit range through `codex exec` | see the script's header; when Codex is out of credits the Cavekit inspector reviews, and Codex reviews the same range later |

More test servers (a logging proxy, a TLS page, a print sink, a Persona feed) live beside the tests
that use them in `render/goanna/test/`. Device scripts run one at a time: there is one TouchPad.
