#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Jihad Browser -- the instrumented movies (cavekit-flash.md R2, T-571): one for AVM1 (SWF 8) and one for AVM2 (SWF 10), which
# report to the page what the player gives them: each mouse event in stage coordinates, each key code, frames elapsed, a timer,
# the stage's size, the player's version, and on AVM2 a video's time, codec and dropped frames (FlashVars video=URL).
# A movie's inside is opaque to a page, so without these a test of input can only guess.
#
# There is no ActionScript compiler in this tree: the bytecode is assembled here, and read back by read_back() below and by
# tools/host/flash-corpus.py read before a result from a movie is believed.
#
# Both report by ExternalInterface.call("jihadFlash", ...). Events are queued and sent once a frame at most, joined by ';', so a
# report does not pace the movie.
# THE TRAP OF THE FIRST DAY (2026-11-06): the first AVM1 movie reported once and then nothing, and every theory about the player
# (its enterFrame, its listeners, getURL) was wrong. A hand-written DefineShape was malformed, the first frame never painted, and
# the player stood still waiting on it. The shape now comes from make-anim-swf.py, which has drawn on the device for months. jihad-flash-instr.html sorts them into
# elements a check reads. FlashVars: scale=showAll|noScale|exactFit|noBorder (the stage's scale mode).
#
# Usage: python3 render/goanna/test/make-instrument-swf.py [-d DIR]
import argparse, importlib.util, os, struct

_spec = importlib.util.spec_from_file_location('jihad_anim_swf', os.path.join(os.path.dirname(os.path.abspath(__file__)), 'make-anim-swf.py'))
_anim = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_anim)

TWIPS = 20
WIDTH, HEIGHT, FPS = 400, 300, 30


# --- the SWF around them ---------------------------------------------------------------------------------------------
def tag(code, body=b""):
    if len(body) < 0x3f: return struct.pack("<H", (code << 6) | len(body)) + body
    return struct.pack("<HI", (code << 6) | 0x3f, len(body)) + body


class Bits:
    def __init__(self): self.v = 0; self.n = 0
    def put(self, val, n):
        self.v = (self.v << n) | (val & ((1 << n) - 1)); self.n += n
    def bytes(self):
        pad = (8 - self.n % 8) % 8
        return ((self.v << pad).to_bytes((self.n + pad) // 8, "big"))


def rect(w, h):
    b = Bits(); nb = max(((w * TWIPS).bit_length(), (h * TWIPS).bit_length())) + 1
    b.put(nb, 5); b.put(0, nb); b.put(w * TWIPS, nb); b.put(0, nb); b.put(h * TWIPS, nb)
    return b.bytes()


def swf(version, tags, frames=1):
    body = rect(WIDTH, HEIGHT) + struct.pack("<HH", int(FPS * 256), frames) + tags + tag(0)
    return b"FWS" + bytes([version]) + struct.pack("<I", 8 + len(body)) + body


# --- AVM1 ------------------------------------------------------------------------------------------------------------
class A1:
    """An AVM1 action stream assembled by name."""
    def __init__(self): self.b = bytearray()
    def op(self, code, payload=None):
        self.b.append(code)
        if code >= 0x80:
            payload = payload or b""
            self.b += struct.pack("<H", len(payload)) + payload
        return self
    def push(self, *vals):
        p = bytearray()
        for v in vals:
            if isinstance(v, str): p += b"\x00" + v.encode() + b"\x00"
            elif v is None: p += b"\x02"
            elif isinstance(v, bool): p += b"\x05" + (b"\x01" if v else b"\x00")
            elif isinstance(v, int): p += b"\x07" + struct.pack("<i", v)
            else: raise TypeError(v)
        return self.op(0x96, bytes(p))
    def getvar(self, name): return self.push(name).op(0x1C)
    def setvar(self, name, fill):
        self.push(name); fill(self); return self.op(0x1D)
    def define(self, name, params, body):
        code = bytes(body.b)
        h = name.encode() + b"\x00" + struct.pack("<H", len(params)) + b"".join(p.encode() + b"\x00" for p in params) + struct.pack("<H", len(code))
        self.op(0x9B, h); self.b += code; return self
    def func_expr(self, params, body): return self.define("", params, body)   # leaves the function on the stack
    def call(self, name, nargs):                # the arguments pushed already, last first
        return self.push(nargs, name).op(0x3D)
    def method(self, obj_fill, name, nargs):    # the arguments pushed already
        self.push(nargs); obj_fill(self); self.push(name); return self.op(0x52)
    def add(self): return self.op(0x47)
    def getmember(self, name): return self.push(name).op(0x4E)
    def pop(self): return self.op(0x17)


def avm1_program():
    """Frame 1: the queue, function R(s), the scale mode, the first line."""
    a = A1()
    a.push("q", "").op(0x1D); a.push("fr", 0).op(0x1D)
    r = A1(); r.push("_root.q"); r.getvar("_root.q"); r.getvar("s"); r.add(); r.push(";"); r.add(); r.op(0x1D)
    a.define("R", ["s"], r)
    # the stage's scale mode from FlashVars
    a.getvar("scale"); a.op(0x12); a.op(0x12); a.op(0x12)              # defined and not empty, Not: jump over when it is not
    skip = A1(); skip.getvar("Stage"); skip.push("scaleMode"); skip.getvar("scale"); skip.op(0x4F)
    a.op(0x9D, struct.pack("<h", len(skip.b))); a.b += skip.b
    # the first line: the player and the stage
    a.push("ready avm1 ver="); a.getvar("$version"); a.add(); a.push(" stage="); a.add(); a.getvar("Stage"); a.getmember("width"); a.add(); a.push("x"); a.add(); a.getvar("Stage"); a.getmember("height"); a.add()
    a.call("R", 1); a.pop()
    a.op(0x00)
    return bytes(a.b)


def clip_line(fill):
    """_root.R(<what fill leaves on the stack>)"""
    x = A1(); fill(x); x.push(1); x.getvar("_root"); x.push("R"); x.op(0x52); x.pop(); x.op(0x00)
    return bytes(x.b)


def clip_mouse(word):
    return clip_line(lambda f: (f.push(word + " "), f.getvar("_root"), f.getmember("_xmouse"), f.add(), f.push(" "), f.add(), f.getvar("_root"), f.getmember("_ymouse"), f.add()))


def clip_key(word):
    return clip_line(lambda f: (f.push(word + " "), f.push(0), f.getvar("Key"), f.push("getCode"), f.op(0x52), f.add(), f.push(" "), f.add(), f.push(0), f.getvar("Key"), f.push("getAscii"), f.op(0x52), f.add()))


def clip_frame():
    """Each frame: count, a line every 30th, the queue sent when it holds something."""
    f = A1()
    f.push("_root.fr"); f.getvar("_root.fr"); f.push(1); f.add(); f.op(0x1D)
    every = A1(); every.push("frames "); every.getvar("_root.fr"); every.add(); every.push(" t="); every.add(); every.op(0x34); every.add(); every.push(1); every.getvar("_root"); every.push("R"); every.op(0x52); every.pop()
    f.getvar("_root.fr"); f.push(30); f.op(0x3F); f.push(0); f.op(0x49); f.op(0x12)   # (fr % 30 == 0), Not
    f.op(0x9D, struct.pack("<h", len(every.b))); f.b += every.b
    send = A1()
    send.getvar("_root.q"); send.push("jihadFlash", 2); send.getvar("flash.external.ExternalInterface"); send.push("call"); send.op(0x52); send.pop()
    send.push("_root.q", ""); send.op(0x1D)
    f.getvar("_root.q"); f.push(""); f.op(0x49); f.op(0x12); f.op(0x12)               # (q != ""), Not
    f.op(0x9D, struct.pack("<h", len(send.b))); f.b += send.b
    f.op(0x00)
    return bytes(f.b)


# the SWF 6 clip event flags: KeyUp 0x80, KeyDown 0x40, MouseUp 0x20, MouseDown 0x10, MouseMove 0x08, EnterFrame 0x02
def clip_actions(records):
    allf = 0; out = bytearray()
    for flags, code in records:
        allf |= flags
        out += struct.pack("<II", flags, len(code)) + code
    return struct.pack("<HI", 0, allf) + bytes(out) + struct.pack("<I", 0)


def build_avm1():
    t = tag(9, bytes((0x20, 0x40, 0x60)))                              # SetBackgroundColor
    t += _anim.define_shape_rect(1, 40, 40, (0xF0, 0x60, 0x20))        # a mark at the stage's top left: a frame check sees the movie
    t += _anim.place_object2(1, char_id=1, translate=(0, 0))
    t += tag(12, avm1_program())
    # The events by clip actions on a one-frame sprite, as make-key-swf.py has them.
    t += tag(39, struct.pack("<HH", 2, 1) + tag(1) + tag(0))           # DefineSprite 2: one empty frame
    actions = clip_actions([(0x02, clip_frame()), (0x10, clip_mouse("down")), (0x20, clip_mouse("up")), (0x08, clip_mouse("move")), (0x40, clip_key("keydown")), (0x80, clip_key("keyup"))])
    t += tag(26, bytes([0x80 | 0x02]) + struct.pack("<HH", 2, 2) + actions)   # PlaceObject2: char 2 at depth 2, with clip actions
    t += tag(1)
    return swf(8, t)


# --- AVM2: a small ABC assembler ----------------------------------------------------------------------------------------
def u30(v):
    out = bytearray()
    while True:
        b = v & 0x7f; v >>= 7
        if v: out.append(b | 0x80)
        else: out.append(b); return bytes(out)


class Abc:
    def __init__(self):
        self.strings = [None]; self.ns = [None]; self.mn = [None]; self.methods = []; self.bodies = []; self.ints = [None]
    def s(self, text):
        if text not in self.strings: self.strings.append(text)
        return self.strings.index(text)
    def pkg(self, name):
        k = (0x16, self.s(name))
        if k not in self.ns: self.ns.append(k)
        return self.ns.index(k)
    def q(self, name, pkg=""):
        k = (0x07, self.pkg(pkg), self.s(name))
        if k not in self.mn: self.mn.append(k)
        return self.mn.index(k)
    def method(self, nparams, code, locals_=None, stack=10, init=8, depth=9):
        i = len(self.methods); self.methods.append(nparams)
        self.bodies.append((i, stack, locals_ if locals_ is not None else nparams + 1, init, depth, bytes(code)))
        return i
    def pool(self):
        o = bytearray(u30(0) + u30(0) + u30(0))                 # ints, uints and doubles: pushbyte and pushshort do
        o += u30(len(self.strings))
        for t in self.strings[1:]: e = t.encode(); o += u30(len(e)) + e
        o += u30(len(self.ns))
        for k, n in self.ns[1:]: o += bytes([k]) + u30(n)
        o += u30(0)                                              # no namespace sets
        o += u30(len(self.mn))
        for k, a, b in self.mn[1:]: o += bytes([k]) + u30(a) + u30(b)
        return bytes(o)


class C:
    """A method's code by name."""
    def __init__(self, abc): self.a = abc; self.b = bytearray()
    def raw(self, *bs): self.b += bytes(bs); return self
    def this(self): return self.raw(0xD0)
    def arg1(self): return self.raw(0xD1)
    def pushscope(self): return self.raw(0x30)
    def str(self, t): self.b += b"\x2c" + u30(self.a.s(t)); return self
    def byte(self, v): return self.raw(0x24, v & 0xff)
    def short(self, v): self.b += b"\x25" + u30(v); return self
    def lex(self, name, pkg=""): self.b += b"\x60" + u30(self.a.q(name, pkg)); return self
    def findstrict(self, name, pkg=""): self.b += b"\x5d" + u30(self.a.q(name, pkg)); return self
    def get(self, name, pkg=""): self.b += b"\x66" + u30(self.a.q(name, pkg)); return self
    def set(self, name, pkg=""): self.b += b"\x61" + u30(self.a.q(name, pkg)); return self
    def call(self, name, argc, pkg=""): self.b += b"\x46" + u30(self.a.q(name, pkg)) + u30(argc); return self
    def callvoid(self, name, argc, pkg=""): self.b += b"\x4f" + u30(self.a.q(name, pkg)) + u30(argc); return self
    def construct(self, name, argc, pkg=""): self.b += b"\x4a" + u30(self.a.q(name, pkg)) + u30(argc); return self
    def add(self): return self.raw(0xA0)
    def pop(self): return self.raw(0x29)
    def ret(self): return self.raw(0x47)
    def if_false(self, fill):                # the branch's code runs when the value on the stack is true
        inner = C(self.a); fill(inner)
        self.b += b"\x12" + struct.pack("<i", len(inner.b))[:3]; self.b += inner.b; return self
    def concat(self, *parts):                # strings and getter closures joined on the stack
        first = True
        for part in parts:
            if isinstance(part, str): self.str(part)
            else: part(self)
            if not first: self.add()
            first = False
        return self
    def report(self, *parts):                # this.R(...)
        self.this(); self.concat(*parts); return self.callvoid("R", 1)


def build_avm2():
    a = Abc()
    M = a.q("M")
    # R(s): the queue
    r = C(a); r.this().pushscope(); r.this(); r.this().get("q"); r.arg1(); r.add(); r.str(";"); r.add(); r.set("q"); r.ret()
    mR = a.method(1, r.b)
    # send(): once a frame, by ExternalInterface where the page gives one, else by a javascript: URL
    sn = C(a); sn.this().pushscope()
    sn.this().get("q"); sn.str(""); sn.raw(0xAB); sn.raw(0x96)     # equals, not
    def do_send(x):
        x.lex("ExternalInterface", "flash.external"); x.get("available")
        x.raw(0x12); j1 = len(x.b); x.b += b"\0\0\0"             # iffalse to the fallback
        x.lex("ExternalInterface", "flash.external"); x.str("jihadFlash"); x.this().get("q"); x.call("call", 2); x.pop()
        x.raw(0x10); j2 = len(x.b); x.b += b"\0\0\0"             # jump over it
        x.b[j1:j1 + 3] = struct.pack("<i", len(x.b) - (j1 + 3))[:3]
        x.findstrict("navigateToURL", "flash.net"); x.findstrict("URLRequest", "flash.net"); x.concat("javascript:void(jihadFlash('", lambda y: y.this().get("q"), "'))")
        x.construct("URLRequest", 1, "flash.net"); x.str("_self"); x.callvoid("navigateToURL", 2, "flash.net")
        x.b[j2:j2 + 3] = struct.pack("<i", len(x.b) - (j2 + 3))[:3]
        x.this(); x.str(""); x.set("q")
    sn.if_false(do_send); sn.ret()
    mSend = a.method(0, sn.b)
    # the mouse and the keys
    def mouse(word):
        c = C(a); c.this().pushscope()
        c.report(word + " ", lambda y: y.arg1().get("stageX"), " ", lambda y: y.arg1().get("stageY")); c.ret()
        return a.method(1, c.b)
    def key(word):
        c = C(a); c.this().pushscope()
        c.report(word + " ", lambda y: y.arg1().get("keyCode"), " ", lambda y: y.arg1().get("charCode")); c.ret()
        return a.method(1, c.b)
    mDown, mUp, mMove, mKd, mKu = mouse("down"), mouse("up"), mouse("move"), key("keydown"), key("keyup")
    # the frame: count, a line every 30th, the video's state, the queue sent
    f = C(a); f.this().pushscope()
    f.this(); f.this().get("fr"); f.raw(0x91); f.set("fr")          # fr = fr + 1
    f.this().get("fr"); f.byte(30); f.raw(0xA4); f.byte(0); f.raw(0xAB)   # fr % 30 == 0
    def every(x):
        x.report("frames ", lambda y: y.this().get("fr"), " t=", lambda y: y.findstrict("getTimer", "flash.utils").call("getTimer", 0, "flash.utils"))
        x.this().get("ns")
        def vid(z):
            z.report("video t=", lambda y: y.this().get("ns").get("time"), " dropped=", lambda y: y.this().get("ns").get("info").get("droppedFrames"),
                     " fps=", lambda y: y.this().get("ns").get("currentFPS"))
        x.if_false(vid)
    f.if_false(every)
    f.this(); f.callvoid("send", 0); f.ret()
    mFrame = a.method(1, f.b)
    # a video's metadata and status
    md = C(a); md.this().pushscope()
    md.report("vmeta codec=", lambda y: y.arg1().get("videocodecid"), " w=", lambda y: y.arg1().get("width"), " h=", lambda y: y.arg1().get("height"), " d=", lambda y: y.arg1().get("duration")); md.ret()
    mMeta = a.method(1, md.b)
    st = C(a); st.this().pushscope()
    st.report("vstatus ", lambda y: y.arg1().get("info").get("code")); st.ret()
    mStatus = a.method(1, st.b)
    # the constructor
    i = C(a); i.this().pushscope(); i.this(); i.raw(0x49); i.b += u30(0)     # super()
    i.this(); i.byte(0); i.set("fr"); i.this(); i.str(""); i.set("q")
    i.this().get("graphics"); i.short(0x2060 & 0x7fff); i.callvoid("beginFill", 1)
    i.this().get("graphics"); i.byte(0); i.byte(0); i.short(WIDTH); i.short(HEIGHT); i.callvoid("drawRect", 4)
    i.this().get("graphics"); i.callvoid("endFill", 0)
    i.this().get("loaderInfo").get("parameters").get("scale")
    i.if_false(lambda x: (x.this().get("stage"), x.this().get("loaderInfo").get("parameters").get("scale"), x.set("scaleMode")))
    for ev, name in (("mouseDown", "onDown"), ("mouseUp", "onUp"), ("mouseMove", "onMove"), ("keyDown", "onKd"), ("keyUp", "onKu")):
        i.this().get("stage"); i.str(ev); i.this().get(name); i.callvoid("addEventListener", 2)
    i.this(); i.str("enterFrame"); i.this().get("onFrame"); i.callvoid("addEventListener", 2)
    i.report("ready avm2 ver=", lambda y: y.lex("Capabilities", "flash.system").get("version"), " stage=", lambda y: y.this().get("stage").get("stageWidth"), "x", lambda y: y.this().get("stage").get("stageHeight"),
             " ei=", lambda y: y.lex("ExternalInterface", "flash.external").get("available"))
    def video(x):
        x.this(); x.findstrict("NetConnection", "flash.net"); x.construct("NetConnection", 0, "flash.net"); x.set("nc")
        x.this().get("nc"); x.raw(0x20); x.callvoid("connect", 1)
        x.this(); x.findstrict("NetStream", "flash.net"); x.this().get("nc"); x.construct("NetStream", 1, "flash.net"); x.set("ns")
        x.this().get("ns"); x.this(); x.set("client")
        x.this().get("ns"); x.str("netStatus"); x.this().get("onStatus"); x.callvoid("addEventListener", 2)
        x.this(); x.findstrict("Video", "flash.media"); x.short(WIDTH); x.short(HEIGHT); x.construct("Video", 2, "flash.media"); x.set("v")
        x.this().get("v"); x.this().get("ns"); x.callvoid("attachNetStream", 1)
        x.this(); x.this().get("v"); x.callvoid("addChild", 1)
        x.this().get("ns"); x.this().get("loaderInfo").get("parameters").get("video"); x.callvoid("play", 1)
    i.this().get("loaderInfo").get("parameters").get("video"); i.if_false(video)
    i.ret()
    mIinit = a.method(0, i.b)
    # the class's own init and the script
    ci = C(a); ci.this().pushscope(); ci.ret()
    mCinit = a.method(0, ci.b, init=7, depth=8)
    chain = [("Object", ""), ("EventDispatcher", "flash.events"), ("DisplayObject", "flash.display"), ("InteractiveObject", "flash.display"), ("DisplayObjectContainer", "flash.display"), ("Sprite", "flash.display")]
    sc = C(a); sc.this().pushscope()
    sc.raw(0x65, 0)                                                  # getscopeobject 0: the global
    for n, pk in chain: sc.lex(n, pk).pushscope()
    sc.lex("Sprite", "flash.display"); sc.raw(0x58); sc.b += u30(0)  # newclass 0
    for _ in chain: sc.raw(0x1D)                                     # popscope
    sc.b += b"\x68" + u30(M)                                         # initproperty M
    sc.ret()
    mScript = a.method(0, sc.b, init=1, depth=8)
    # the file
    def slot(name, n): return u30(a.q(name)) + bytes([0]) + u30(n) + u30(0) + u30(0)
    def meth(name, m): return u30(a.q(name)) + bytes([1]) + u30(0) + u30(m)
    itraits = [slot(n, k + 1) for k, n in enumerate(("fr", "q", "nc", "ns", "v"))]
    itraits += [meth("R", mR), meth("send", mSend), meth("onDown", mDown), meth("onUp", mUp), meth("onMove", mMove), meth("onKd", mKd), meth("onKu", mKu), meth("onFrame", mFrame), meth("onMetaData", mMeta), meth("onStatus", mStatus)]
    instance = u30(M) + u30(a.q("Sprite", "flash.display")) + bytes([0]) + u30(0) + u30(mIinit) + u30(len(itraits)) + b"".join(itraits)
    klass = u30(mCinit) + u30(0)
    script = u30(mScript) + u30(1) + u30(M) + bytes([4]) + u30(1) + u30(0)
    methods = b"".join(u30(n) + u30(0) + b"".join(u30(0) for _ in range(n)) + u30(0) + bytes([0]) for n in a.methods)
    bodies = b"".join(u30(m) + u30(st) + u30(lc) + u30(ini) + u30(dep) + u30(len(code)) + code + u30(0) + u30(0) for m, st, lc, ini, dep, code in a.bodies)
    abc = struct.pack("<HH", 16, 46) + a.pool() + u30(len(a.methods)) + methods + u30(0) + u30(1) + instance + klass + u30(1) + script + u30(len(a.bodies)) + bodies
    t = tag(69, struct.pack("<I", 0x08 | 0x01))                      # FileAttributes: AS3, network access
    t += tag(9, bytes((0x20, 0x40, 0x60)))
    t += tag(82, struct.pack("<I", 1) + b"jihad\x00" + abc)          # DoABC
    t += tag(76, struct.pack("<HH", 1, 0) + b"M\x00")                # SymbolClass: the main class on the root
    t += tag(1)
    return swf(10, t)


# --- reading back -------------------------------------------------------------------------------------------------------
def read_back(data):
    """The tag stream as it stands in the file: the tree's rule, a movie is parsed back before a result from it is believed."""
    assert data[:3] == b"FWS", "not uncompressed"
    assert struct.unpack("<I", data[4:8])[0] == len(data), "the header's length disagrees with the file"
    nb = data[8] >> 3; p = 8 + (5 + 4 * nb + 7) // 8 + 4; out = []
    while p + 2 <= len(data):
        h = struct.unpack("<H", data[p:p + 2])[0]; p += 2
        code, n = h >> 6, h & 0x3f
        if n == 0x3f: n = struct.unpack("<I", data[p:p + 4])[0]; p += 4
        out.append((code, n)); p += n
        if code == 0: break
    assert p == len(data), "tags end at %d of %d" % (p, len(data))
    return out


def main():
    ap = argparse.ArgumentParser(); ap.add_argument("-d", default=os.path.dirname(os.path.abspath(__file__))); o = ap.parse_args()
    d = build_avm1(); read_back(d); open(os.path.join(o.d, "jihad-instr-avm1.swf"), "wb").write(d)
    print("jihad-instr-avm1.swf", len(d), read_back(d))
    d = build_avm2(); read_back(d); open(os.path.join(o.d, "jihad-instr-avm2.swf"), "wb").write(d)
    print("jihad-instr-avm2.swf", len(d), read_back(d))


if __name__ == "__main__":
    main()
