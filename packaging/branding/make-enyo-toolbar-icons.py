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
# same simple house (roof with eaves, chimney, walls, a door) at the family's size and finish.
#
# BOOKMARKS / HISTORY. The design is the stock one and is kept: a clock (ring, hands at twelve
# and nine) with a bookmark ribbon over its lower right. What changes is the fit: its strokes
# were 2 px where its neighbours' are 3, it stood two pixels below their line (down to y = 27)
# and a pixel right of centre, and its shading was its own. Redrawn here with the ring and
# hands at the neighbours' weight, standing on their line (y 5..25, centred), and finished by
# the same code as the home icon. Pressed, the clock face still fills with the glow, as the
# stock one does: the ring is too small for the glow to leave its middle dark.
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
    cx, cy, r, w = 14.6, 14.6, 9.6, 2.8             # clock ring: centre, outer radius, stroke
    d.ellipse([(cx - r) * SS, (cy - r) * SS, (cx + r) * SS, (cy + r) * SS], fill=255)
    d.ellipse([(cx - r + w) * SS, (cy - r + w) * SS, (cx + r - w) * SS, (cy + r - w) * SS], fill=0)
    # hands: twelve o'clock and nine o'clock, meeting at the centre
    hw = 2.6
    poly(d, [(cx - hw / 2, cy - 5.6), (cx + hw / 2, cy - 5.6), (cx + hw / 2, cy + hw / 2), (cx - hw / 2, cy + hw / 2)])
    poly(d, [(cx - 4.8, cy - hw / 2), (cx + hw / 2, cy - hw / 2), (cx + hw / 2, cy + hw / 2), (cx - 4.8, cy + hw / 2)])
    # the ribbon, over the ring's lower right; the ring is cleared around it so the two read
    # as separate shapes, as in the stock glyph
    rx0, rx1, ry0, ry1, notch, gap = 19.4, 27.0, 15.4, 25.0, 2.7, 1.5
    poly(d, [(rx0 - gap, ry0 - gap), (rx1 + gap, ry0 - gap), (rx1 + gap, ry1 + gap), (rx0 - gap, ry1 + gap)], fill=0)
    poly(d, [(rx0, ry0), (rx1, ry0), (rx1, ry1), ((rx0 + rx1) / 2, ry1 - notch), (rx0, ry1)])
    return finish(big)

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

def write(mask, path):
    sprite = Image.new("RGBA", (CELL, CELL * 2), (0, 0, 0, 0))
    sprite.alpha_composite(normal_cell(mask), (0, 0))
    sprite.alpha_composite(pressed_cell(mask), (0, CELL))
    sprite.save(path, optimize=True)
    solid = mask.point(lambda a: 255 if a > 150 else 0).getbbox()
    print("wrote", os.path.normpath(path), "glyph box", solid)

def main():
    here = os.path.dirname(os.path.abspath(__file__))
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(here, "..", "..", "app", "images", "chrome")
    write(house_mask(), os.path.join(out, "menu-icon-home.png"))
    write(bookmark_mask(), os.path.join(out, "menu-icon-bookmark.png"))

if __name__ == "__main__":
    main()
