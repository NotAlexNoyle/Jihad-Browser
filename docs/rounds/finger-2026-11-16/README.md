# The whole finger suite, 2026-11-16 (engine 0113, the three shells)

`finger-run.py --with internet --round docs/rounds/finger-2026-11-16` (101 scripts, 285 runs, 21862 s, `run.log`), then
the three failed scripts again in every shell with `--merge` (`rerun.log`). `finger-report-first.json` is the first
pass alone; `finger-report.json` / `.md` the merged one.

| | first pass | after the rerun |
|---|---|---|
| pass | 251 | 254 |
| fail | 3 | 0 |
| known defect (T-567: `pan-long-flick`, `pan-speeds`, `wikipedia-zoom`) | 9 | 9 |
| not run (`tabbed-browsing`, `touchpad-go`, `webone`) | 9 | 9 |
| waiting for a tool (T-653: `audio-heard`, `flash-sound`, `rotate`, `mojo-launch-from-app`) | 10 | 10 |
| by a tool of its own (`settings-every-control`) | 3 | 3 |

## The three failures of the first pass

- **Enyo, `etsy-search`: the daemon died (open, T-655).** `FATAL SIGNAL sig=11 faultaddr=0xc`, pc `0x2bf1fefc`
  lr `0x2bf1fecd`, libxul at `0x2aabf000` (md5 on the device `c0412009…`): pc − base = `0x1460efc` →
  `JSScript::codeEnd` ← `containsPC` ← `getName` ← `Interpret`, `js/src/vm/Interpreter.cpp:3756` (`JSOP_INITPROP`,
  `script->getName(REGS.pc)`); 0xc is `codeLength` in a null `scriptData_`: a script being interpreted whose bytecode
  is gone. The log before it: Etsy's title set, `load watchdog: forcing load-stopped after 14653ms (engine still
  loading)`. Once in six loads of the page today (the other five pass, Enyo's second among them).
- **Mochi, `flash-input-hold`:** the release came at the right point (`up 361 282`); the one move read for the check
  was 328,260 for 360,277 (the page's readout a move behind). Passes in the rerun in all three shells. The check's
  timing, not the browser's.
- **Mojo, `appmenu-print`:** the dialog came; no mDNS answer inside the bound (Enyo and Mochi found the printer in the
  same run). Passes in the rerun in all three shells.

## What it settles

- The kits' criteria that waited "for a person" are ticked or reworded by script id (11 met, 9 open as the tools' work:
  T-567, T-653, T-654, the stock 3.0.5 hardware).
- `flash-text` passes in the three shells: `fl_type`'s capitals (the shift key at 34,933) are right.
- The end check's only findings were the agent's own screenshots left in the device's `/tmp` (removed).
