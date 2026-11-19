# T-672: a page that runs the device out of memory in one task (2026-11-19)

cavekit-eaglercraft.md R6, cavekit-device-headroom.md R1. EaglercraftX 1.8.8's start hung the Adreno 220 three times
(`docs/rounds/t666-2026-11-18/README.md`): the daemon grew to 621 MB on the 940 MB TouchPad, MemFree fell to 30-40 MB,
and the GPU, whose kernel driver draws on that free memory, never idled again; every card and the system UI stuck until a
reboot. The memory guard never acted: its "available" (free plus the page cache) stayed over 98 MB, and it ran only from
the pump, between tasks that lasted minutes.

## What changed

- **Engine patch 0124** (`jihad_js_stop_scripts`): from any thread, a three-second window in which the next interrupt
  callback ends the running content script, as the slow-script dialog's "Stop script" does; chrome is never stopped.
- **The guard** (`render/goanna/EngineHost.cpp`): past the budget (half the RAM) with MemFree itself under
  `jihad.memory.freeFloorKb` (64 MB) is critical; a thread of the guard's own reads it every 2 s whatever the main
  thread does, stops the running script, and the pump's guard then flushes and stops the page. A page whose script the
  thread stopped is stopped even when the flush brings the memory back (a page without its script is a dead screen).
- **about:stopped** read the stopped page's address from `location.search`, which an `about:` address does not have:
  the address and "Try it again" never showed. It reads `location.href`.
- **finger:** `expect gpu-calm` (no new kgsl fault in the kernel's log since the script's start); `fl_dialogs_ok`
  prints the daemons' memory each minute.

## The scripts

| script | shells | result |
|---|---|---|
| `memory-guard-task` (`jihad-memhog.html?mb=750&rate=12&task=1`: 750 MB at 12 MB/s in one task) | Enyo, Mochi, Mojo | pass: the thread at 470-490 MB with 28-30 MB free, the script stopped, the page shed to about:stopped with its address and link, no GPU fault, the card usable after |
| `eagler-guard-188` (EaglercraftX 1.8.8's start, Continue and Launch as they come) | Enyo | pass: about:stopped, no GPU fault, the card usable after (in its extraction this time; three runs before the guard hung the GPU) |

Found on the way: a script that starts its card on the one-task page loses the guard's lines (the page's task held the
card's first frames until the guard had acted, Mochi): the card starts on a plain page and opens the hog after the mark.

EaglercraftX itself stays where T-666 left it: its start is too slow and too large for this device without T-673
(BigInt, deferred by the owner); the guard now ends it instead of the device.
