#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. MPL-2.0 (the art it draws is the project's own).
"""Theme packages for the Persona formats tests (context/kits/cavekit-persona-formats.md).

    make-format-tests.py OUTDIR

writes, drawn here from shapes (so they are the project's own):

  jihad-test-colours.xpi   colour-only: frame, toolbar and the R3 colour slots, no picture (R1, R3)
  jihad-test-layers.xpi    four pictures and no theme_frame: a moon at left top, a sun at right
                           top, hills along the bottom (repeat-x) and a tiled dot pattern (R2)
  jihad-test-svg.xpi       an SVG header only, with colours: becomes colour-only (R2's skipped
                           layer, R1's fallback)
"""
import io, json, sys, zipfile
from PIL import Image, ImageDraw


def png(im):
    b = io.BytesIO()
    im.save(b, "PNG", optimize=True)
    return b.getvalue()


def moon():
    im = Image.new("RGBA", (120, 120), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.ellipse((20, 20, 100, 100), fill=(240, 240, 220, 255))
    d.ellipse((44, 12, 116, 84), fill=(0, 0, 0, 0))
    return im


def sun():
    im = Image.new("RGBA", (160, 120), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for r, a in ((56, 60), (46, 120), (36, 255)):
        d.ellipse((80 - r, 60 - r, 80 + r, 60 + r), fill=(255, 190, 40, a))
    return im


def hills():
    im = Image.new("RGBA", (240, 40), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.pieslice((-40, 10, 140, 110), 180, 360, fill=(40, 120, 70, 255))
    d.pieslice((100, 18, 280, 118), 180, 360, fill=(30, 100, 60, 255))
    return im


def dots():
    im = Image.new("RGBA", (24, 24), (0, 0, 0, 0))
    ImageDraw.Draw(im).ellipse((9, 9, 15, 15), fill=(255, 255, 255, 70))
    return im


def xpi(path, manifest, files):
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("manifest.json", json.dumps(manifest, indent=1))
        for name, data in files.items():
            z.writestr(name, data)


def main(out):
    base = {"manifest_version": 2, "version": "1.0", "author": "Jihad Browser",
            "browser_specific_settings": {"gecko": {"id": "{test}"}}}
    xpi(out + "/jihad-test-colours.xpi", dict(base, name="Jihad test: colours only", theme={"colors": {
        "frame": "#3b1d5e", "tab_background_text": "#fbe9ff", "toolbar": "#7a3fa8", "toolbar_text": "#ffffff",
        "toolbar_field": "#fff3c4", "toolbar_field_text": "#3b1d5e", "toolbar_field_border": "#e0a800",
        "toolbar_field_focus": "#ffffff", "toolbar_field_text_focus": "#000000",
        "popup": "#2a1240", "popup_text": "#fbe9ff", "popup_border": "#e0a800",
        "popup_highlight": "#e0a800", "popup_highlight_text": "#2a1240",
        "icons": "#ffd54a", "ntp_background": "#1c0d2e", "ntp_text": "#fbe9ff"}}), {})
    xpi(out + "/jihad-test-layers.xpi", dict(base, name="Jihad test: four layers", theme={
        "images": {"additional_backgrounds": ["moon.png", "sun.png", "hills.png", "dots.png"]},
        "properties": {"additional_backgrounds_alignment": ["left top", "right top", "left bottom", "left top"],
                       "additional_backgrounds_tiling": ["no-repeat", "no-repeat", "repeat-x", "repeat"]},
        "colors": {"frame": "#16324f", "tab_background_text": "#ffffff"}}),
        {"moon.png": png(moon()), "sun.png": png(sun()), "hills.png": png(hills()), "dots.png": png(dots())})
    svg = b'<svg xmlns="http://www.w3.org/2000/svg" width="300" height="60"><rect width="300" height="60" fill="#c00"/></svg>'
    xpi(out + "/jihad-test-svg.xpi", dict(base, name="Jihad test: SVG header", theme={
        "images": {"theme_frame": "h.svg"}, "colors": {"frame": "#004d40", "tab_background_text": "#e0f2f1",
                                                       "toolbar": "#00796b"}}), {"h.svg": svg})


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else ".")
