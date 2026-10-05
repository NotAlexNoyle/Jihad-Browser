#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Draws the two Enyo toolbar icons this project supplies, in the style of the stock webOS
# toolbar glyphs they sit between (menu-icon-back / -forward / -share / -newcard):
#
#   app/images/chrome/menu-icon-home.png       Home
#   app/images/chrome/menu-icon-bookmark.png   Bookmarks / History (a clock with a ribbon)
#
# Those are 32x64 sprites: the top 32x32 cell is the normal state, the bottom one the pressed
# state. Read off the stock files:
#   normal   a WHITE glyph whose opacity falls from about 86% at its top to about 56% at its
#            bottom (which is what makes it read as brushed light grey on the dark bar), over a
#            soft black shadow a pixel or two below it;
#   pressed  the same glyph solid white inside a blue glow, rgb(92,172,230), that is opaque at
#            the glyph's edge and gone about six pixels out.
# and their glyphs leave a margin: about 23 px wide, standing on a common line at y = 25
# (share and new-card: x 6..29, y 5..25), with outlines about 3 px thick.
#
# HOME. The first home icon (2026-08-05) was a flat grey house filling the cell edge to edge,
# with a flat blue pressed state — bigger and flatter than everything beside it. This is the
# same simple house (roof with eaves, chimney, walls, a door) at the family's size, finished
# like the solid glyphs (the arrows).
#
# BOOKMARKS / HISTORY. A clock with a bookmark ribbon, as before, but BUILT THE WAY ITS TWO
# NEIGHBOURS ARE. Share and New Card are the same construction, read off their pixels:
#   - a body drawn as a 2 px outline, standing on rows 24-25, its top at row 10, its left
#     edge at column 6;
#   - a solid accent (the arrow, the plus) in the upper right, rows 5-15, 3 px strokes;
#   - the outline is cut away for 2 px around the accent, and resumes at row 18 below it;
#   - white at an opacity that falls 6.4 a row from 224 at row 5 to 96 at row 25, with every
#     upward-facing edge one step brighter (+16 at the top of the cell, +73 at the bottom —
#     which is what gives the bottom stroke its lit inner edge);
#   - a black shadow one row down, about a pixel soft;
#   - pressed: solid white in a glow of rgb(83,183,255), half strength beside a stroke and
#     gone four pixels out, so the middle of the body stays dark.
# So here the clock's ring is the outlined body (18 px across, standing on the same rows),
# its hands are 2 px, and the ribbon is the accent, where the plus and the arrow are. The stock icon
# had the ribbon at the lower right, hanging two rows below the line; a first redraw
# (2026-10-05) kept that and made the strokes heavy like the house, which left it bolder and
# higher than the two icons it stands next to.
#
#   usage: make-enyo-toolbar-icons.py [outdir]     (default: app/images/chrome)
import math, os, sys
from PIL import Image, ImageDraw, ImageFilter, ImageChops

SS = 16                                   # supersampling factor
CELL = 32

def canvas():
    big = Image.new("L", (CELL * SS, CELL * SS), 0)
    return big, ImageDraw.Draw(big)

def poly(d, pts, fill=255):
    d.polygon([(x * SS, y * SS) for x, y in pts], fill=fill)

def finish(big):
    return big.resize((CELL, CELL), Image.LANCZOS)

def house_mask():
    big, d = canvas()
    # roof: a triangle whose eaves overhang the walls
    poly(d, [(16.0, 6.0), (28.5, 16.6), (26.0, 16.6), (26.0, 17.4), (6.0, 17.4), (6.0, 16.6), (3.5, 16.6)])
    # walls
    poly(d, [(7.0, 16.0), (25.0, 16.0), (25.0, 25.0), (7.0, 25.0)])
    # chimney, on the right slope
    poly(d, [(21.4, 7.6), (24.2, 7.6), (24.2, 13.4), (21.4, 11.0)])
    # the door is cut out of the walls
    poly(d, [(13.4, 19.2), (18.6, 19.2), (18.6, 25.0), (13.4, 25.0)], fill=0)
    return finish(big)

def bookmark_mask():
    big, d = canvas()
    # clock ring: centre, outer radius, stroke. 2.5 px, not 2: a curve is antialiased all the
    # way round and reads lighter than the neighbours' pixel-true 2 px straight strokes.
    cx, cy, r, w = 15.0, 17.0, 9.0, 2.5
    d.ellipse([(cx - r) * SS, (cy - r) * SS, (cx + r) * SS, (cy + r) * SS], fill=255)
    d.ellipse([(cx - r + w) * SS, (cy - r + w) * SS, (cx + r - w) * SS, (cy + r - w) * SS], fill=0)
    # hands: twelve o'clock and nine o'clock, meeting at the centre
    poly(d, [(cx - 1, cy - 5.5), (cx + 1, cy - 5.5), (cx + 1, cy + 1), (cx - 1, cy + 1)])
    poly(d, [(cx - 4.5, cy - 1), (cx + 1, cy - 1), (cx + 1, cy + 1), (cx - 4.5, cy + 1)])
    # the ribbon, where the neighbours carry their plus and arrow; the ring is cleared for
    # 2 px around it and picks up again at row 18, as their outlines do
    rx0, rx1, ry0, ry1, notch, gap = 21.0, 28.0, 5.0, 16.0, 3.5, 2.0
    poly(d, [(rx0 - gap, 0), (rx1 + gap, 0), (rx1 + gap, ry1 + gap), (rx0 - gap, ry1 + gap)], fill=0)
    poly(d, [(rx0, ry0), (rx1, ry0), (rx1, ry1), ((rx0 + rx1) / 2, ry1 - notch), (rx0, ry1)])
    return finish(big)

# ── the finish of the outlined glyphs (share, new card) ─────────────────────────────────────
GLOW = (83, 183, 255)

def outline_normal_cell(mask):
    cell = Image.new("RGBA", (CELL, CELL), (0, 0, 0, 0))
    sh = ImageChops.offset(mask, 0, 1).filter(ImageFilter.GaussianBlur(1.1))
    shadow = Image.new("RGBA", (CELL, CELL), (0, 0, 0, 255)); shadow.putalpha(sh.point(lambda a: int(a * 0.78)))
    cell.alpha_composite(shadow)
    a = Image.new("L", (CELL, CELL), 0); ap = a.load(); mp = mask.load()
    for y in range(CELL):
        base = max(96.0, min(224.0, 224.0 - (y - 5) * 6.4))
        lift = 16.0 + max(0, y - 5) * 3.0
        for x in range(CELL):
            m = mp[x, y] / 255.0
            if m <= 0: continue
            above = mp[x, y - 1] / 255.0 if y > 0 else 0.0
            edge = max(0.0, m - above)                 # 1 where this pixel is a top edge
            ap[x, y] = int(min(255.0, m * (base + lift * edge)))
    glyph = Image.new("RGBA", (CELL, CELL), (255, 255, 255, 255)); glyph.putalpha(a)
    cell.alpha_composite(glyph)
    return cell

def outline_pressed_cell(mask):
    cell = Image.new("RGBA", (CELL, CELL), (0, 0, 0, 0))
    glow_a = mask.filter(ImageFilter.GaussianBlur(2.0)).point(lambda a: min(255, int(a * 1.8)))
    glow = Image.new("RGBA", (CELL, CELL), GLOW + (255,)); glow.putalpha(glow_a)
    cell.alpha_composite(glow)
    core = Image.new("RGBA", (CELL, CELL), (255, 255, 255, 255)); core.putalpha(mask)
    cell.alpha_composite(core)
    return cell

# ── the finish of the solid glyphs (the house) ──────────────────────────────────────────────
def normal_cell(mask):
    top, bottom = mask.getbbox()[1], mask.getbbox()[3]
    cell = Image.new("RGBA", (CELL, CELL), (0, 0, 0, 0))
    # shadow: the glyph dropped two pixels, softened, black
    sh = ImageChops.offset(mask, 0, 2).filter(ImageFilter.GaussianBlur(1.3))
    shadow = Image.new("RGBA", (CELL, CELL), (0, 0, 0, 255)); shadow.putalpha(sh.point(lambda a: int(a * 0.62)))
    cell.alpha_composite(shadow)
    # glyph: white, opacity falling with height
    grad = Image.new("L", (CELL, CELL), 0); gp = grad.load(); mp = mask.load()
    for y in range(CELL):
        t = min(1.0, max(0.0, (y - top) / float(max(1, bottom - 1 - top))))
        k = 0.87 - 0.30 * t
        for x in range(CELL):
            gp[x, y] = int(mp[x, y] * k)
    glyph = Image.new("RGBA", (CELL, CELL), (255, 255, 255, 255)); glyph.putalpha(grad)
    cell.alpha_composite(glyph)
    return cell

def pressed_cell(mask):
    cell = Image.new("RGBA", (CELL, CELL), (0, 0, 0, 0))
    glow_a = mask.filter(ImageFilter.GaussianBlur(2.6)).point(lambda a: min(255, int(a * 2.4)))
    glow = Image.new("RGBA", (CELL, CELL), (92, 172, 230, 255)); glow.putalpha(glow_a)
    cell.alpha_composite(glow)
    core = Image.new("RGBA", (CELL, CELL), (255, 255, 255, 255)); core.putalpha(mask)
    cell.alpha_composite(core)
    return cell

def write(mask, path, normal=None, pressed=None):
    sprite = Image.new("RGBA", (CELL, CELL * 2), (0, 0, 0, 0))
    sprite.alpha_composite((normal or normal_cell)(mask), (0, 0))
    sprite.alpha_composite((pressed or pressed_cell)(mask), (0, CELL))
    sprite.save(path, optimize=True)
    print("wrote", os.path.normpath(path), "glyph box", mask.point(lambda a: 255 if a > 127 else 0).getbbox())

def main():
    here = os.path.dirname(os.path.abspath(__file__))
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(here, "..", "..", "app", "images", "chrome")
    write(house_mask(), os.path.join(out, "menu-icon-home.png"))
    write(bookmark_mask(), os.path.join(out, "menu-icon-bookmark.png"), outline_normal_cell, outline_pressed_cell)

if __name__ == "__main__":
    main()
