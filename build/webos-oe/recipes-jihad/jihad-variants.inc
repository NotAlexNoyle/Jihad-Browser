# Per-variant identity for the three INDEPENDENT Jihad Browser packages (cavekit-device-build.md
# R7/R8). This is the OE side's single source of truth; it mirrors, field for field:
#     context/plans/plan-variant-identity.md    the authoritative table
#     packaging/gen-variant-scripts.sh          the direct-build generator's TABLE
#
# WHY A TABLE AND NOT PER-RECIPE LITERALS: R7's first acceptance criterion is that no two
# variants resolve to the same MIME / shim / impl path / YAP name / socket / upstart job. A
# literal repeated across five recipes is how two variants quietly end up sharing one — the
# 2026-07-29 review found exactly that (finding #5: the shim's impl path was hard-coded to the
# Enyo app id, so Mochi and Mojo loaded the Enyo impl). Here each product recipe states ONE fact,
# `JIHAD_VARIANT`, and every other name is derived; jihad-app.inc additionally asserts at parse
# time that the rows are mutually distinct.

# Every variant the deviceroot assembles parts for.
JIHAD_VARIANTS ?= "enyo mochi mojo"

# Rows are "<variant>:<app id>:<NPAPI shim .so>:<upstart job>:<YAP service name>".
# Derived elsewhere, never spelled out per recipe: the socket is /tmp/yapserver.<YAP name>, the
# adapter impl is /usr/lib/jihad/<variant>/BrowserAdapterImpl.so, the runtime state dir is
# /var/palm/jihad/<variant>/. The Enyo variant keeps the unsuffixed names it already has
# deployed on-device — renaming the one deployment known to work buys nothing.
JIHAD_VARIANT_TABLE ?= "\
    enyo:net.riverstonerelay.jihad-browser:BrowserAdapterJihad.so:jihad:jihad-browser \
    mochi:net.riverstonerelay.jihad-browser.mochi:BrowserAdapterJihadMochi.so:jihad-mochi:jihad-browser-mochi \
    mojo:net.riverstonerelay.jihad-browser.mojo:BrowserAdapterJihadMojo.so:jihad-mojo:jihad-browser-mojo"

# Field lookup for the recipe that sets JIHAD_VARIANT. Indices match the row above:
#   1 app id   2 shim .so   3 upstart job   4 YAP service name
# bb.fatal on an unknown variant: a typo must stop the parse, not silently yield an empty path
# that would later install a file named "" or, worse, collide with another variant's.
def jihad_variant_field(d, idx):
    v = d.getVar('JIHAD_VARIANT', True)
    if not v:
        bb.fatal("JIHAD_VARIANT is unset — every Jihad product recipe must set it (see jihad-variants.inc)")
    for row in (d.getVar('JIHAD_VARIANT_TABLE', True) or '').split():
        f = row.split(':')
        if f[0] == v:
            return f[idx]
    bb.fatal("JIHAD_VARIANT '%s' is not in JIHAD_VARIANT_TABLE" % v)
