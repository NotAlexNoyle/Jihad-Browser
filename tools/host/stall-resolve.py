#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# stall-resolve.py — the daemon's stall samples (/tmp/jihad-stall.txt on the device, written by the
# sampler in render/goanna/GoannaRenderPage.cpp when $JIHAD_STALL_SAMPLER is set) turned into
# function names and buckets: which part of the engine held the main thread.
#
#   python3 -I tools/host/stall-resolve.py <jihad-stall.txt> [--xul <unstripped libxul.so>] [--top N]
#
# Each "stall N: <ms> ms, <n> samples" block is resolved on its own. libxul offsets go through the
# toolchain's addr2line against the UNSTRIPPED libxul of the same build
# (build/webos-oe/out-arm/obj-jihad-goanna-arm/toolkit/library/libxul.so); other modules are
# counted by name; "jit" is code the JIT emitted (no module).
import sys, os, re, subprocess, collections

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
XUL = os.path.join(REPO, "build/webos-oe/out-arm/obj-jihad-goanna-arm/toolkit/library/libxul.so")
A2L = os.path.join(REPO, "build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin/arm-webos-linux-gnueabi-addr2line")

BUCKETS = [
    ("JS parse / bytecode",  r"js::frontend::|BytecodeEmitter|TokenStream|js::Parser|FullParseHandler|SyntaxParseHandler|CompileScript|CompileModule"),
    ("JIT compile (Ion/Baseline)", r"js::jit::(Ion|MIR|LIR|Lowering|Codegen|Baseline|Range|Scalar|Alias|Valu|Licm|GVN|Linear|Backtracking|Register|Macro|Assembler|JitCompiler|OptimizeMIR|CompileBackEnd|IonBuilder|BaselineCompiler|ICStub|CacheIR)|js::jit::Compile|jit::Assembler"),
    ("GC",                   r"js::gc::|GCRuntime|js::Nursery|TenuringTracer|GCMarker|js::TraceEdge|TraceChildren|js::MarkingTracer|Arena::|Chunk::|Zone::sweep|SweepArena|FinalizeTypedArenas|IncrementalCollectSlice"),
    ("Cycle collector",      r"nsCycleCollector|CCGraph|nsPurpleBuffer|CycleCollect|Traverse\(|Unlink\(|NS_CYCLE_COLLECTION"),
    ("Style (selectors, restyle)", r"nsCSSRuleProcessor|SelectorMatches|RuleProcessorData|RestyleManager|ElementRestyler|nsStyleSet::|nsRuleNode|ResolveStyleFor|nsStyleContext|StyleSetHandle|nsCSSSelector|RuleHash|ComputeStyle|GetStyleContext|nsTransitionManager|nsAnimationManager"),
    ("Layout (reflow, frames)", r"::Reflow|nsBlockFrame|nsInlineFrame|nsFlexContainerFrame|nsGridContainerFrame|nsTableFrame|nsLineLayout|nsCSSFrameConstructor|nsFrameManager|ReflowInput|nsHTMLReflowState|PresShell::DoReflow|ProcessReflowCommands|FlushPendingNotifications|nsTextFrame|nsContainerFrame|nsLayoutUtils|BlockReflowInput|nsFloatManager|nsBidi"),
    ("Paint / display list", r"nsDisplayList|nsDisplayItem|PaintRoot|BuildDisplayList|gfxContext|gfxPlatform|mozilla::gfx::|DrawTarget|cairo_|pixman_|PaintFrame|nsLayoutUtils::Paint|ImageLayer|BasicLayer|nsImageRenderer|gfxTextRun|gfxFont|FrameLayerBuilder|JihadRenderDocument|RenderDocument"),
    ("HTML parse / DOM build", r"nsHtml5|nsContentSink|nsXMLContentSink|nsGenericHTMLElement::|Element::Set|nsINode::|nsDocument::|nsContentUtils|mozilla::dom::Element|nsAttrValue|nsContentList|nsDOMMutationObserver"),
    ("JS interpreter / runtime", r"js::Interpret|js::RunScript|js::Call\(|js::InternalCallOrConstruct|js::jit::DoCallFallback|js::NativeObject|js::NativeGetProperty|js::GetProperty|js::SetProperty|js::ObjectGroup|js::Shape|js::TypeSet|js::jit::ICStubCompiler|js::StringBuffer|js::ConcatStrings|js::Invoke|JSObject::|js::NewObject|js::ArrayObject|js::str_|js::array_|js::RegExp|js::irregexp|js::ToString|js::ValueToSource|js::jit::VMFunction|js::jit::Invalidate|js::DeepCloneObject|js::Proxy|js::Wrapper|js::CrossCompartmentWrapper|XPCWrappedNative|xpc::|XrayWrapper|DOMProxyHandler|JSAutoCompartment"),
    ("DOM bindings / XPCOM glue", r"mozilla::dom::.*Binding|GenericBindingMethod|GenericBindingGetter|binding_detail|nsXPCWrappedJS|XPCCallContext|XPCConvert|nsCOMPtr|NS_Addref|NS_Release|AddRef|Release\(|QueryInterface"),
    ("Events / timers / tasks", r"nsTimer|TimerThread|nsRefreshDriver|EventDispatcher|nsEventDispatcher|EventListenerManager|nsThread::|nsRunnable|mozilla::Runnable|TaskQueue|MessageLoop|nsAppShell|ProcessNextEvent"),
    ("Network / cache / sqlite", r"sqlite3|mozStorage|nsHttp|nsSocketTransport|nsCacheEntry|CacheFile|CacheIndex|nsInputStream|nsOutputStream|nsStreamLoader|nsCookieService|nsPermissionManager|nsDiskCache|nsUrlClassifier|nsStandardURL|nsIOService|nsLoadGroup|nsChannel"),
    ("Images / media",       r"imgLoader|imgRequest|RasterImage|Decoder|nsJPEG|nsPNG|nsGIF|SurfaceCache|VectorImage|MediaDecoder|MediaCache|MediaStream|FFmpeg|VPX|Opus|AudioStream|cubeb"),
    ("Memory (malloc, strings)", r"jemalloc|arena_|malloc|free\b|moz_x|nsTArray|nsTString|nsAString|nsACString|nsString|memcpy|memmove|memset|strlen|std::"),
]

def resolve_xul(offsets, xul, a2l):
    """offsets: set of ints → dict offset → 'function' (the innermost inlined name)"""
    out = {}
    if not offsets or not os.path.exists(xul) or not os.path.exists(a2l): return out
    offs = sorted(offsets)
    for i in range(0, len(offs), 2000):
        chunk = offs[i:i+2000]
        p = subprocess.run([a2l, "-f", "-C", "-e", xul] + ["0x%x" % o for o in chunk], capture_output=True, text=True)
        lines = p.stdout.splitlines()
        for j, o in enumerate(chunk):
            fn = lines[2*j] if 2*j < len(lines) else "??"
            out[o] = fn
    return out

def bucket(fn):
    for name, rx in BUCKETS:
        if re.search(rx, fn): return name
    return "other"

def main():
    args = sys.argv[1:]
    if not args: print(__doc__); sys.exit(2)
    path = args[0]; xul = XUL; top = 30
    if "--xul" in args: xul = args[args.index("--xul") + 1]
    if "--top" in args: top = int(args[args.index("--top") + 1])
    stalls = []; cur = None
    for line in open(path, encoding="utf8", errors="replace"):
        line = line.strip()
        m = re.match(r"stall (\d+): (\d+) ms, (\d+) samples", line)
        if m: cur = {"n": int(m.group(1)), "ms": int(m.group(2)), "count": int(m.group(3)), "samples": []}; stalls.append(cur); continue
        if cur is not None and "+0x" in line:
            mod, off = line.rsplit("+0x", 1); cur["samples"].append((mod, int(off, 16)))
    xul_offsets = set(off for st in stalls for mod, off in st["samples"] if mod.startswith("libxul"))
    names = resolve_xul(xul_offsets, xul, A2L)
    for st in stalls:
        n = len(st["samples"]) or 1
        byfn = collections.Counter(); bybk = collections.Counter(); bymod = collections.Counter()
        for mod, off in st["samples"]:
            bymod[mod] += 1
            if mod.startswith("libxul"):
                fn = names.get(off, "libxul+0x%x" % off); byfn[fn] += 1; bybk[bucket(fn)] += 1
            elif mod == "jit": byfn["<JIT code>"] += 1; bybk["JIT code (the page's compiled JavaScript)"] += 1
            else: byfn[mod] += 1; bybk["other module: " + mod] += 1
        print("=== stall %d: %d ms, %d samples (10 ms each)" % (st["n"], st["ms"], n))
        print("--- by bucket:")
        for k, v in bybk.most_common(): print("  %5.1f%%  %s" % (100.0 * v / n, k))
        print("--- top %d functions:" % top)
        for k, v in byfn.most_common(top): print("  %5.1f%%  %s" % (100.0 * v / n, k[:140]))
        print()

if __name__ == "__main__": main()
