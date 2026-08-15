# YAP code generator (vendored)

`BrowserServerBase.{h,cpp}` (in `../Src/`) and `BrowserClientBase.{h,cpp}` (in `../../adapter/`)
are GENERATED files. This directory holds the generator that produces them and the `.defs` that
drives it, so a regeneration is reproducible inside this tree with nothing else checked out.

| File | Role |
|------|------|
| `BrowserYapCommandMessages.defs` | The authoritative interface definition for this tree. |
| `YapCodeGen.cpp` | The generator. Vendored from isis-project/BrowserServer and patched — see below. |
| `CodeGen.pro` | qmake project for the generator (host Qt 5, `QtCore` only). |

## The only valid generator, and the only valid `.defs`

Use **this** copy of both. `../../../ref-BrowserServer/CodeGen` is a separate upstream checkout
and is wrong in three ways that all fail silently:

1. Its `.defs` has no `0x1600 SetExtraBuffer`, so the third shared framebuffer stops being
   accepted and the Flash frame rate regresses with nothing erroring.
2. Its `.defs` is stale against its OWN output: it declares `uint16_t key, uint16_t modifiers`
   for KeyDown/KeyUp (0x1008/0x1009) while every generated file in both trees reads a third
   `int32 chr`. Regenerating from it rewrites two live commands from 12 wire bytes to 4 and
   breaks every keystroke.
3. Its `YapCodeGen.cpp` emits every async command as `= 0;` and hangs forever on any `.defs`
   line that starts with a space.

## Build and regenerate

```sh
# build the generator (host Qt 5; needs qmake + g++, no device toolchain)
mkdir -p /tmp/yapcodegen && cd /tmp/yapcodegen
qmake <repo>/render/browserserver/CodeGen/CodeGen.pro && make        # produces ./CodeGen

# regenerate. The generator writes <basename>{Server,Client}Base.{h,cpp} into the CWD.
DEFS=<repo>/render/browserserver/CodeGen/BrowserYapCommandMessages.defs
./CodeGen server Browser "$DEFS"      # -> BrowserServerBase.{h,cpp}
./CodeGen client Browser "$DEFS"      # -> BrowserClientBase.{h,cpp}

# then, ALWAYS:
<repo>/build/webos-oe/check-yap-contract.sh          # must print "YAP contract OK"
```

Copy the server pair over `../Src/` and the client pair over `../../adapter/` only if you mean
to adopt the regeneration. **No hand edit is required afterwards** — that is the point of the
patches below. `check-yap-contract.sh` passes on the generated pair with zero defects.

## Patches applied to the vendored generator

The file keeps its upstream Apache-2.0 header and carries a change notice below it, as
Apache-2.0 section 4(b) requires. Attribution is recorded in the repo `NOTICE`.

1. **`ignoreLine()` no longer hangs.** Upstream scanned for a non-space character without ever
   advancing its pointer, so a `.defs` line beginning with a space spun forever with no output.
   Blank runs are now skipped, tabs and CR count as blank, and an indented comment is allowed.
2. **The type field takes flags**, space separated: `async optional; …`. Unknown flags are a
   parse error rather than being ignored.
3. **`optional` async commands are not pure virtual.** The server-side declaration is emitted
   with an empty default body instead of `= 0;`, so a server that does not implement the command
   still compiles and the generated dispatch arm calls a default that ignores it. This is what
   makes an added id optional in BOTH directions (`context/kits/cavekit-ipc-contract.md` R1 AC2)
   and it is why `0x1600` no longer has to be re-applied by hand after every regeneration.
   `optional` is accepted on async commands only; on `sync` or `msg` it is a parse error.

The patches change nothing for input the upstream generator already accepted: built both
generators and ran them over the upstream `.defs`, and all four output files are byte-identical.

## Known textual differences from the files that ship today

Regeneration reproduces the **wire** exactly. It does not reproduce every byte of the files
currently in the tree, because those carry cosmetic residue from hand edits and, on the client
side, from an older generator vintage. None of these need any hand edit re-applied.

Server pair (`../Src/BrowserServerBase.{h,cpp}`) — five differences, all cosmetic:

- the copyright year (the generator stamps the current year; the shipped files say 2012);
- `// SetExtraBuffer` in the `.cpp` case arm lost its hand-written parenthetical;
- the two `int32_t chr = 0;` lines are re-indented from spaces to a tab (upstream's hand edit);
- the `.h` comment above `asyncCmdSetExtraBuffer` is worded generically;
- `asyncCmdSetExtraBuffer` is **declared** one position earlier in the `.h`, right after
  `asyncCmdConnect`, matching the `.defs` and matching where the `.cpp` case arm already sits.
  The shipped `.h` and the shipped `.cpp` disagree with each other about this position because
  each was hand-edited separately, so no `.defs` ordering can reproduce both at once. It is
  declaration order within one class in one binary; it has no effect on the wire.

Client pair (`../../adapter/BrowserClientBase.{h,cpp}`) — the shipped files came from an OLDER
YapCodeGen (four-space indent, no license block emitted), so they are not byte-comparable at all.
They are wire-identical, and `check-yap-contract.sh` passes on the regenerated pair. Adopting a
regeneration there would also change two things for the better, neither of which touches the wire:
`sendRawCmd` would gain the missing `SetExtraBuffer` arm, and KeyDown/KeyUp's raw-command
argument parsing would move from `strtoul` (left over from when they were `uint16_t`) to `atol`,
which is what `int` maps to.
