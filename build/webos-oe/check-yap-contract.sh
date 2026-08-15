#!/bin/bash
# Copyright 2026 NotAlexNoyle.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Fail loudly if the generated YAP interface stops matching the frozen wire contract.
#
# HOW TO REGENERATE (2026-08-15): the generator is VENDORED at
# render/browserserver/CodeGen/YapCodeGen.cpp and it is the only valid one. Build it with host
# Qt 5 and run it over render/browserserver/CodeGen/BrowserYapCommandMessages.defs, which is the
# only valid input. Full instructions in render/browserserver/CodeGen/README.md. Regeneration now
# needs NO hand edit afterwards, and this script passes on the generated pair with zero defects.
#
# WHY THIS EXISTS, and why it is a script rather than a .defs edit (2026-08-10, MEASURED):
# BrowserServerBase.{h,cpp} and BrowserClientBase.{h,cpp} are GENERATED files that used to be
# kept here without their generator. The upstream generator and .defs are in
# ../ref-BrowserServer/CodeGen, which is a SEPARATE upstream git checkout, and regenerating from
# them does NOT reproduce what ships. Built YapCodeGen.cpp against Qt 5.15 and ran it over that
# .defs:
#
#   1. 0x1600 asyncCmdSetExtraBuffer is absent from the .defs, so the whole dispatch arm
#      disappears. The daemon then answers "Unknown async cmd", the third shared buffer is
#      never accepted, and the frame rate regresses with nothing failing. That is the
#      criterion this script was written for (cavekit-ipc-contract.md R1 AC3).
#   2. WORSE, AND NOBODY HAD RECORDED IT: the .defs is stale against its OWN upstream output.
#      It declares "uint16_t key, uint16_t modifiers" for KeyDown/KeyUp (0x1008/0x1009) while
#      both the upstream ref-BrowserServer/Src/BrowserServerBase.cpp and ours read a THIRD
#      int32 "chr". Upstream hand-edited the generated file and never updated the .defs (the
#      giveaway is the six-space indent on the chr lines in a tab-indented file). Regenerating
#      therefore rewrites two LIVE commands from 12 wire bytes to 4 and breaks every keystroke.
#
# A third defect used to make the .defs route structurally insufficient even for 0x1600:
# upstream YapCodeGen emits every async command as "= 0;", so it could not produce the
# non-pure-virtual default body that lets a daemon which does not implement the third buffer
# still compile — the default that R1 AC2's "optional in both directions" argument rests on.
# CLOSED 2026-08-15 by vendoring and patching the generator: an "optional" flag on the async
# line emits that empty default body. The upstream generator still cannot, so the vendored one
# is the only valid generator, exactly as the in-tree .defs is the only valid input.
#
# Exit 0 = contract intact. Exit 1 = a violation, named, with the fix.
set -uo pipefail

ROOT=${1:-$(cd "$(dirname "$0")/../.." && pwd)}
SRV_H=$ROOT/render/browserserver/Src/BrowserServerBase.h
SRV_C=$ROOT/render/browserserver/Src/BrowserServerBase.cpp
CLI_H=$ROOT/render/adapter/BrowserClientBase.h
CLI_C=$ROOT/render/adapter/BrowserClientBase.cpp

fail=0
bad() { echo "FAIL: $*" >&2; fail=1; }

for f in "$SRV_H" "$SRV_C" "$CLI_H" "$CLI_C"; do
  [ -r "$f" ] || { echo "FAIL: missing generated file $f" >&2; exit 1; }
done

# Ids the wire carries, lowercased. Commands are what the server accepts and the client sends;
# messages are what the server sends and the client accepts. Both sets are frozen: a regeneration
# that drops one on BOTH sides at once still has to fail, so the expected sets are spelled out
# rather than only cross-checked. 0x1600 is the one deliberate Jihad addition.
EXPECT_CMD="0x0014 0x1000 0x1001 0x1003 0x1004 0x1005 0x1007 0x1008 0x1009 0x100a 0x100b 0x100c \
0x100d 0x1010 0x1011 0x1015 0x1016 0x1017 0x101a 0x101b 0x101c 0x1103 0x1104 0x1105 0x1106 0x1107 \
0x1108 0x1109 0x110a 0x110b 0x110c 0x110d 0x110e 0x110f 0x1111 0x1112 0x1113 0x1114 0x1115 0x1116 \
0x1117 0x1118 0x1119 0x111a 0x111b 0x111c 0x111d 0x111e 0x111f 0x1120 0x1121 0x1122 0x1123 0x1124 \
0x1125 0x1500 0x1501 0x1502 0x1503 0x1504 0x1505 0x1506 0x1507 0x1508 0x1509 0x150a 0x150b 0x150c \
0x150d 0x150e 0x150f 0x1510 0x1600"
EXPECT_MSG="0x2000 0x2001 0x2002 0x2004 0x2005 0x2006 0x2007 0x2008 0x2009 0x200a 0x200b 0x200c \
0x200d 0x200e 0x200f 0x2010 0x2011 0x2012 0x2013 0x2014 0x2015 0x2016 0x2017 0x2018 0x2019 0x201a \
0x201c 0x201d 0x201e 0x201f 0x2020 0x2021 0x2022 0x2023 0x2024 0x2025 0x2026 0x2027 0x2028 0x2029 \
0x202a 0x202b 0x202c 0x202d 0x202e 0x202f 0x2030 0x2031 0x2032 0x2033 0x2034 0x2035 0x2036 0x2037 \
0x2038 0x2039 0x203a 0x203b"

# LC_ALL=C so the two sides are compared under one collation, whatever the build image sets.
norm() { tr 'A-F' 'a-f' | LC_ALL=C sort -u | tr '\n' ' '; }
cases()  { grep -oE "case 0x[0-9a-fA-F]{4}" "$1" | grep -oE "0x[0-9a-fA-F]{4}" | norm; }
sends()  { grep -oE "\(int16_t\) 0x[0-9a-fA-F]{4}" "$1" | grep -oE "0x[0-9a-fA-F]{4}" | norm; }
want()   { echo "$1" | tr ' ' '\n' | grep -v '^$' | norm; }

diffset() { # name expected actual
  [ "$2" = "$3" ] && return 0
  bad "$1 id set changed."
  diff <(echo "$2" | tr ' ' '\n') <(echo "$3" | tr ' ' '\n') | grep -E '^[<>]' | sed 's/^/       /' >&2
}

diffset "server-accepted command" "$(want "$EXPECT_CMD")" "$(cases "$SRV_C")"
diffset "client-sent command"     "$(want "$EXPECT_CMD")" "$(sends "$CLI_C")"
diffset "server-sent message"     "$(want "$EXPECT_MSG")" "$(sends "$SRV_C")"
diffset "client-accepted message" "$(want "$EXPECT_MSG")" "$(cases "$CLI_C")"

# 0x1600 is more than an id: the arm must still read TWO int32s in this order and hand them to
# asyncCmdSetExtraBuffer, and the server-side declaration must stay NON-pure-virtual so a daemon
# without a third buffer still compiles and simply ignores the command.
grep -q "asyncCmdSetExtraBuffer(proxy, sharedBufferKey3, sharedBufferSize)" "$SRV_C" \
  || bad "0x1600 dispatch arm in BrowserServerBase.cpp no longer calls asyncCmdSetExtraBuffer(proxy, sharedBufferKey3, sharedBufferSize)."
grep -q "(\*cmd) >> sharedBufferKey3;" "$SRV_C" \
  || bad "0x1600 dispatch arm no longer reads sharedBufferKey3 off the packet."
grep -q "asyncCmdSetExtraBuffer(YapProxy\* proxy, int32_t sharedBufferKey3, int32_t sharedBufferSize)$" "$SRV_H" \
  || bad "BrowserServerBase.h: asyncCmdSetExtraBuffer is missing or became pure virtual. It must keep its empty default body (R1 AC2: optional in BOTH directions)."
grep -q "asyncCmdSetExtraBuffer(int32_t sharedBufferKey3, int32_t sharedBufferSize)" "$CLI_C" \
  || bad "BrowserClientBase.cpp: the 0x1600 sender is gone."

# KeyDown/KeyUp carry a THIRD int32 "chr" that the upstream .defs does not know about.
# Regeneration from it silently reverts both to 2 uint16s: 12 wire bytes become 4. Counted on the
# packet operators rather than the identifier, so an unrelated "chrome" cannot mask a real loss.
[ "$(grep -c "(\*cmd) >> chr;" "$SRV_C")" -ge 2 ] \
  || bad "BrowserServerBase.cpp: KeyDown/KeyUp no longer read the third int32 'chr' off the packet (0x1008/0x1009 wire format changed)."
[ "$(grep -c "(\*_cmd) << chr;" "$CLI_C")" -ge 2 ] \
  || bad "BrowserClientBase.cpp: KeyDown/KeyUp no longer write the third int32 'chr' to the packet (0x1008/0x1009 wire format changed)."
grep -q "asyncCmdKeyDown(YapProxy\* proxy, int32_t key, int32_t modifiers, int32_t chr)" "$SRV_H" \
  || bad "BrowserServerBase.h: asyncCmdKeyDown is not the 3-argument int32 form."
grep -q "asyncCmdKeyUp(YapProxy\* proxy, int32_t key, int32_t modifiers, int32_t chr)" "$SRV_H" \
  || bad "BrowserServerBase.h: asyncCmdKeyUp is not the 3-argument int32 form."

if [ "$fail" -ne 0 ]; then
  cat >&2 <<'EOF'

The YAP wire contract is frozen (context/kits/cavekit-ipc-contract.md R1).
If you just regenerated, that is almost certainly the cause. Use THIS tree's generator AND
.defs, both in render/browserserver/CodeGen/, never the upstream copies in
../ref-BrowserServer/CodeGen: the upstream .defs has no 0x1600 and is stale on KeyDown/KeyUp,
and the upstream generator emits every async command as "= 0;", which strips the empty default
body that makes 0x1600 optional. See render/browserserver/CodeGen/README.md.
If you MEANT to change the contract, update the expected sets above and say so on R1.
EOF
  exit 1
fi
echo "YAP contract OK: 73 commands (incl. the 0x1600 Jihad addition), 58 messages, both sides agree."
