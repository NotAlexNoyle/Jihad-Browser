# T-655, the daemon's segfault on Etsy's search: not reproduced (2026-11-16, Enyo, engine 0113)

`finger-run.py --shells enyo --only etsy-search --with internet`, 18 times in a row, a fresh daemon each time: 18 pass,
no fault. The loads are the kind that crashed once in six in the whole suite of the morning: load done at +33 to
+42 s, the load watchdog's forced stop at 12.8 s in the log (it reports only, it does not stop the engine: the
"forced stop in play" of the first reading is no cause).

What the first report could not give is now kept: the fatal report ends with the stack's words that point into
libxul (the unwinder gives nothing from a signal frame on this device) and `finger-run.py` writes the report into
the round as `crash-<shell>-<id>-<time>.txt` when a daemon dies while its page starts. The next time it happens, in
any round, the callers of `Interpret` at the fault are in that file: addr2line the `sp+` words on
`build/webos-oe/out-arm/obj-jihad-goanna-arm-pgo/toolkit/library/libxul.so`.

Read in the engine meanwhile, no cause found: a script's `scriptData_` is null only between `JSScript::Create` and
`createScriptData`, inside `shareScriptData`'s swap (under the exclusive-access lock), after a failed XDR decode, or
in a cell freed and handed out again; a failed lazy compile resets its links (`jsfun.cpp:1501`). The three helper
thread changes of 2026-11-13 (0109-0111) touch no script data.
