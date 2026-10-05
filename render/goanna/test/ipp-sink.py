#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. MPL-2.0.
#
# ipp-sink.py — a printer that prints nothing. It answers the two IPP operations
# components/jihadPrint.js uses (Get-Printer-Attributes, Print-Job) and writes every document it
# is sent to a directory, so a print can be tested, and looked at, without paper or ink.
#
# WHY THIS EXISTS. On 2026-10-05 a test meant as a dry run was sent to the owner's real printer
# and came out as a full dark sheet. Nothing about printing is tested against a real printer
# until it has passed here first.
#
#   usage: ipp-sink.py [--port 8631] [--out DIR] [--tls cert.pem] [--media iso_a4_210x297mm]
#          [--no-jpeg]      (announce no image/jpeg, to see the browser refuse it)
#
# On the device:  findPrinters {"address":"<this host>:8631"}   then printPage with what it found.
# The address must be on the device's own network (the component refuses anything else).
import argparse, http.server, os, ssl, struct, sys, time

def attr(tag, name, value):
    n = name.encode()
    if isinstance(value, int) and not isinstance(value, bool):
        v = struct.pack(">i", value)
    elif isinstance(value, bool):
        v = b"\x01" if value else b"\x00"
    else:
        v = str(value).encode()
    return bytes([tag]) + struct.pack(">H", len(n)) + n + struct.pack(">H", len(v)) + v

def attrs(tag, name, values):
    out = b""
    for i, v in enumerate(values):
        out += attr(tag, name if i == 0 else "", v)
    return out

def parse_request(d):
    """-> (operation, request id, {name: [values]}, document bytes)"""
    op, rid = struct.unpack(">HI", d[2:8])
    i, got, last = 8, {}, None
    while i < len(d):
        tag = d[i]; i += 1
        if tag < 0x10:
            if tag == 3:
                break
            continue
        nl = struct.unpack(">H", d[i:i + 2])[0]; i += 2
        name = d[i:i + nl].decode("latin1"); i += nl
        vl = struct.unpack(">H", d[i:i + 2])[0]; i += 2
        v = d[i:i + vl]; i += vl
        v = struct.unpack(">i", v)[0] if (tag in (0x21, 0x23) and vl == 4) else v.decode("utf-8", "replace")
        if name:
            last = name; got[last] = []
        if last:
            got[last].append(v)
    return op, rid, got, d[i:]

class Sink(http.server.BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"
    jobs = 0

    def log_message(self, f, *a):
        pass

    def say(self, s):
        sys.stdout.write("%s  %s\n" % (time.strftime("%H:%M:%S"), s)); sys.stdout.flush()

    def answer(self, status, rid, groups):
        b = struct.pack(">BBHI", 2, 0, status, rid) + b"\x01"
        b += attr(0x47, "attributes-charset", "utf-8") + attr(0x48, "attributes-natural-language", "en")
        b += groups + b"\x03"
        self.send_response(200)
        self.send_header("Content-Type", "application/ipp")
        self.send_header("Content-Length", str(len(b)))
        self.end_headers()
        self.wfile.write(b)

    def do_POST(self):
        o = self.server.opts
        if self.headers.get("Transfer-Encoding", "").lower() == "chunked":
            body = b""
            while True:
                n = int(self.rfile.readline().split(b";")[0].strip() or b"0", 16)
                if n == 0:
                    self.rfile.readline(); break
                body += self.rfile.read(n); self.rfile.readline()
        else:
            body = self.rfile.read(int(self.headers.get("Content-Length", "0")))
        if len(body) < 9:
            self.send_error(400); return
        op, rid, got, doc = parse_request(body)
        who = self.client_address[0]
        if op == 0x000B:                                   # Get-Printer-Attributes
            self.say("%s asks what this printer is (%s)" % (who, self.path))
            formats = ["application/octet-stream"] + ([] if o.no_jpeg else ["image/jpeg"])
            g = b"\x04"
            g += attr(0x41, "printer-make-and-model", "Jihad IPP sink (prints nothing)")
            g += attr(0x42, "printer-name", "ipp-sink")
            g += attr(0x23, "printer-state", 3)
            g += attrs(0x49, "document-format-supported", formats)
            g += attr(0x44, "media-default", o.media)
            g += attrs(0x44, "print-scaling-supported", ["auto", "auto-fit", "fill", "fit", "none"])
            self.answer(0x0000, rid, g)
        elif op == 0x0002:                                 # Print-Job
            Sink.jobs += 1
            fmt = (got.get("document-format") or ["?"])[0]
            name = (got.get("job-name") or ["?"])[0]
            ext = "jpg" if fmt == "image/jpeg" else "bin"
            path = os.path.join(o.out, "job-%03d.%s" % (Sink.jobs, ext))
            with open(path, "wb") as f:
                f.write(doc)
            dims = ""
            if doc[:2] == b"\xff\xd8":
                i = 2
                while i + 9 < len(doc):                    # walk the JPEG segments to the frame header
                    if doc[i] != 0xFF: break
                    m, ln = doc[i + 1], struct.unpack(">H", doc[i + 2:i + 4])[0]
                    if 0xC0 <= m <= 0xCF and m not in (0xC4, 0xC8, 0xCC):
                        h, w = struct.unpack(">HH", doc[i + 5:i + 9]); dims = " %dx%d" % (w, h); break
                    i += 2 + ln
            self.say("%s PRINT-JOB %d: \"%s\" %s %d bytes%s copies=%s scaling=%s -> %s" % (
                who, Sink.jobs, name, fmt, len(doc), dims, (got.get("copies") or ["-"])[0],
                (got.get("print-scaling") or ["-"])[0], path))
            g = b"\x02" + attr(0x21, "job-id", Sink.jobs) + attr(0x23, "job-state", 9)
            g += attr(0x45, "job-uri", "ipp://sink/jobs/%d" % Sink.jobs)
            self.answer(0x0000, rid, g)
        else:
            self.say("%s sent operation 0x%04x, not supported here" % (who, op))
            self.answer(0x0501, rid, b"")

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=8631)
    ap.add_argument("--out", default="ipp-sink-out")
    ap.add_argument("--tls", default="")
    ap.add_argument("--media", default="na_letter_8.5x11in")
    ap.add_argument("--no-jpeg", action="store_true")
    o = ap.parse_args()
    os.makedirs(o.out, exist_ok=True)
    srv = http.server.ThreadingHTTPServer(("0.0.0.0", o.port), Sink)
    srv.opts = o
    if o.tls:
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER); ctx.load_cert_chain(o.tls)
        srv.socket = ctx.wrap_socket(srv.socket, server_side=True)
    print("ipp-sink on port %d%s, documents to %s/" % (o.port, " (TLS)" if o.tls else "", o.out)); sys.stdout.flush()
    srv.serve_forever()

if __name__ == "__main__":
    main()
