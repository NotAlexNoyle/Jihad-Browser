#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Draws app/images/chrome/menu-icon-home.png — the Enyo toolbar's Home button — in the style of
# the stock webOS toolbar glyphs it sits between (menu-icon-back / -forward / -share / …).
#
# Those are 32x64 sprites: the top 32x32 cell is the normal state, the bottom one the pressed
# state. Read off the stock files:
#   normal   a WHITE glyph whose opacity falls from about 86% at its top to about 56% at its
#            bottom (which is what makes it read as brushed light grey on the dark bar), over a
#            soft black shadow a pixel or two below it;
#   pressed  the same glyph solid white inside a blue glow, rgb(92,172,230), that is opaque at
#            the glyph's edge and gone about six pixels out.
# and their glyphs leave a margin: about 24 px wide and 18-19 px tall, not the whole cell.
#
# The first home icon (2026-08-05) was a flat grey house filling the cell edge to edge, with a
# flat blue pressed state — bigger and flatter than everything beside it. This one is the same
# simple house (roof with eaves, walls, a door), drawn at the family's size and finished the
# family's way.
#
#   usage: make-enyo-home-icon.py [out.png]        (default: the app's own icon path)
import os, sys
from PIL import Image, ImageDraw, ImageFilter, ImageChops

SS = 16                                   # supersampling factor
CELL = 32

def house_mask():
    """The glyph as an antialiased 32x32 alpha mask."""
    big = Image.new("L", (CELL * SS, CELL * SS), 0)
    d = ImageDraw.Draw(big)
    def P(pts): d.polygon([(x * SS, y * SS) for x, y in pts], fill=255)
    # roof: a triangle whose eaves overhang the walls
    P([(16.0, 6.0), (28.5, 16.6), (26.0, 16.6), (26.0, 17.4), (6.0, 17.4), (6.0, 16.6), (3.5, 16.6)])
    # walls
    P([(7.0, 16.0), (25.0, 16.0), (25.0, 25.0), (7.0, 25.0)])
    # chimney, on the right slope
    P([(21.4, 7.6), (24.2, 7.6), (24.2, 13.4), (21.4, 11.0)])
    # the door is cut out of the walls
    d.polygon([(x * SS, y * SS) for x, y in [(13.4, 19.2), (18.6, 19.2), (18.6, 25.0), (13.4, 25.0)]], fill=0)
    return big.resize((CELL, CELL), Image.LANCZOS)

def normal_cell(mask):
    top, bottom = mask.getbbox()[1], mask.getbbox()[3]
    cell = Image.new("RGBA", (CELL, CELL), (0, 0, 0, 0))
    # shadow: the glyph dropped 1.5 px, softened, black
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

def main():
    here = os.path.dirname(os.path.abspath(__file__))
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(here, "..", "..", "app", "images", "chrome", "menu-icon-home.png")
    mask = house_mask()
    sprite = Image.new("RGBA", (CELL, CELL * 2), (0, 0, 0, 0))
    sprite.alpha_composite(normal_cell(mask), (0, 0))
    sprite.alpha_composite(pressed_cell(mask), (0, CELL))
    sprite.save(out, optimize=True)
    print("wrote", os.path.normpath(out), sprite.size, "glyph bbox", mask.getbbox())

if __name__ == "__main__":
    main()
