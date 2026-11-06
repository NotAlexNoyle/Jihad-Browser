#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# make-pad-xpi.py OUTDIR [N]: N tiny bootstrapped test extensions (Jihad List Pad 01..N, ids padNN@jihad.test) for the
# finger suite's add-on scripts: a list longer than the screen, an add-on to enable, disable, remove and undo.
import sys, os, zipfile
out = sys.argv[1]; n = int(sys.argv[2]) if len(sys.argv) > 2 else 12
os.makedirs(out, exist_ok=True)
RDF = """<?xml version="1.0"?>
<RDF xmlns="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:em="http://www.mozilla.org/2004/em-rdf#">
  <Description about="urn:mozilla:install-manifest">
    <em:id>pad%02d@jihad.test</em:id><em:version>1.0</em:version><em:type>2</em:type><em:bootstrap>true</em:bootstrap>
    <em:name>Jihad List Pad %02d</em:name><em:description>A test extension of the finger suite; it does nothing.</em:description>
    <em:targetApplication><Description><em:id>toolkit@mozilla.org</em:id><em:minVersion>1.0</em:minVersion><em:maxVersion>*</em:maxVersion></Description></em:targetApplication>
    <em:targetApplication><Description><em:id>{8de7fcbb-c55c-4fbe-bfc5-fc555c87dbc4}</em:id><em:minVersion>1.0</em:minVersion><em:maxVersion>*</em:maxVersion></Description></em:targetApplication>
  </Description>
</RDF>
"""
BOOT = "function startup(d, r) {}\nfunction shutdown(d, r) {}\nfunction install(d, r) {}\nfunction uninstall(d, r) {}\n"
for i in range(1, n + 1):
    with zipfile.ZipFile(os.path.join(out, "pad%02d@jihad.test.xpi" % i), "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("install.rdf", RDF % (i, i)); z.writestr("bootstrap.js", BOOT)
