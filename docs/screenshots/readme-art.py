#!/usr/bin/env python3
"""Compose the README's art from raw device screenshots: TouchPad-framed shots and a webOS-style
card view, the interfaces in order of age (Mojo, Enyo, Mochi).

Usage: readme-art.py SHOTS_DIR FONTS_DIR OUT_DIR REPO_DIR

SHOTS_DIR holds raw framebuffer captures (1024x768 PNG, as the device's screenshot service writes
them) under the names used at the bottom of this file. FONTS_DIR holds the TouchPad's own Prelude
fonts, copied from the device's /usr/share/fonts/ (PreludeWGL-Light/Medium .ttf); they are only used
to render text into the images and are not part of this repository. Needs Pillow with WebP support.
"""
import os, sys
from PIL import Image, ImageDraw, ImageFilter, ImageFont, ImageChops

SH, FONTS, OUT, REPO = sys.argv[1:5]
os.makedirs(OUT, exist_ok=True)
F = lambda name, size: ImageFont.truetype(os.path.join(FONTS, name + ".ttf"), size)


def shot(name):
    im = Image.open(os.path.join(SH, name + ".png")).convert("RGB")
    if im.size == (1024, 768):           # raw framebuffer is landscape; the card is portrait
        im = im.rotate(-90, expand=True)
    return im


def rounded_mask(size, r, scale=4):
    w, h = size
    m = Image.new("L", (w * scale, h * scale), 0)
    ImageDraw.Draw(m).rounded_rectangle((0, 0, w * scale - 1, h * scale - 1), r * scale, fill=255)
    return m.resize(size, Image.LANCZOS)


def touchpad(screen, width=560):
    """A portrait HP TouchPad around a 768x1024 screenshot, scaled so the device is `width` wide.
    Proportions from the real thing: 190 x 242 mm body, 148 x 197 mm screen, center button below."""
    k = 4                                   # supersample
    sw, sh = 768, 1024
    side, top = 109, 117
    W, H = sw + 2 * side, sh + 2 * top
    big = Image.new("RGBA", (W * k, H * k), (0, 0, 0, 0))
    d = ImageDraw.Draw(big)
    R = 64 * k
    # body: near-black piano finish, a faint lighter rim
    d.rounded_rectangle((0, 0, W * k - 1, H * k - 1), R, fill=(34, 35, 39, 255))
    d.rounded_rectangle((3 * k, 3 * k, W * k - 1 - 3 * k, H * k - 1 - 3 * k), R - 3 * k, fill=(14, 15, 17, 255))
    # gloss: a soft diagonal sheen over the upper-left of the bezel
    sheen = Image.new("L", (W * k, H * k), 0)
    sd = ImageDraw.Draw(sheen)
    sd.polygon([(0, 0), (W * k * 0.75, 0), (0, H * k * 0.55)], fill=34)
    sheen = sheen.filter(ImageFilter.GaussianBlur(60 * k))
    white = Image.new("RGBA", (W * k, H * k), (255, 255, 255, 255))
    body_mask = rounded_mask((W, H), 64, 1).resize((W * k, H * k))
    big = Image.composite(Image.alpha_composite(big, Image.merge("RGBA", (*white.split()[:3], sheen))), big,
                          body_mask)
    d = ImageDraw.Draw(big)
    # screen well
    d.rectangle(((side - 3) * k, (top - 3) * k, (side + sw + 3) * k, (top + sh + 3) * k), fill=(4, 4, 5, 255))
    # camera
    cx = W * k // 2
    d.ellipse((cx - 6 * k, 52 * k, cx + 6 * k, 64 * k), fill=(26, 28, 34, 255), outline=(48, 50, 56, 255), width=k)
    # center button: a soft rounded square with a thin light ring
    by = top + sh + top // 2
    bw = 40
    d.rounded_rectangle((cx - bw * k // 2, (by - bw // 2) * k, cx + bw * k // 2, (by + bw // 2) * k), 14 * k,
                        fill=(20, 21, 24, 255), outline=(120, 124, 132, 255), width=2 * k)
    dev = big.resize((W, H), Image.LANCZOS)
    dev.paste(screen.resize((sw, sh), Image.LANCZOS), (side, top))
    # drop shadow
    pad = 40
    canvas = Image.new("RGBA", (W + 2 * pad, H + 2 * pad), (0, 0, 0, 0))
    sh_mask = Image.new("L", canvas.size, 0)
    ImageDraw.Draw(sh_mask).rounded_rectangle((pad + 6, pad + 16, pad + W - 6, pad + H + 8), 64, fill=110)
    sh_mask = sh_mask.filter(ImageFilter.GaussianBlur(18))
    canvas.paste((0, 0, 0, 255), (0, 0), sh_mask)
    canvas.alpha_composite(dev, (pad, pad))
    scale = width / canvas.size[0]
    return canvas.resize((width, round(canvas.size[1] * scale)), Image.LANCZOS)


def save_webp(im, name, q=86):
    p = os.path.join(OUT, name)
    im.save(p, "WEBP", quality=q, method=6)
    return p


def save_png(im, name):
    p = os.path.join(OUT, name)
    im.save(p, "PNG", optimize=True)
    return p


def wallpaper(size):
    """An abstract webOS-era backdrop: deep blue-black with soft streaks of light (drawn, not HP's)."""
    w, h = size
    bg = Image.new("RGB", size, (8, 12, 22))
    grad = Image.linear_gradient("L").resize(size)
    top = Image.new("RGB", size, (20, 38, 66))
    bg = Image.composite(bg, top, grad)
    glow = Image.new("RGB", size, (0, 0, 0))
    g = ImageDraw.Draw(glow)
    for (x, y, rx, ry, col) in [(0.18, 0.30, 0.55, 0.10, (40, 110, 170)), (0.70, 0.62, 0.65, 0.08, (60, 40, 130)),
                                (0.45, 0.85, 0.80, 0.06, (20, 90, 120)), (0.92, 0.15, 0.30, 0.05, (90, 60, 150))]:
        g.ellipse(((x - rx) * w, (y - ry) * h, (x + rx) * w, (y + ry) * h), fill=col)
    glow = glow.filter(ImageFilter.GaussianBlur(min(w, h) * 0.08))
    glow = glow.rotate(-8, resample=Image.BICUBIC)
    return ImageChops.add(bg, glow)


def cardview(cards, titles, out_name, size=(1600, 900)):
    w, h = size
    bg = wallpaper(size).convert("RGBA")
    cw = 400
    ch = round(cw * 1024 / 768)
    gap = 70
    x0 = (w - (3 * cw + 2 * gap)) // 2
    y0 = 150
    logo = Image.open(os.path.join(REPO, "docs", "jihad-browser-logo.png")).convert("RGBA").resize((86, 86), Image.LANCZOS)
    bg.alpha_composite(logo, (x0, 36))
    d = ImageDraw.Draw(bg)
    d.text((x0 + 104, 40), "Jihad Browser", font=F("PreludeWGL-Medium", 50), fill=(245, 247, 250))
    d.text((x0 + 106, 98), "for webOS", font=F("PreludeWGL-Light", 24), fill=(170, 190, 215))
    for i, (im, t) in enumerate(zip(cards, titles)):
        x = x0 + i * (cw + gap)
        card = im.resize((cw, ch), Image.LANCZOS).convert("RGBA")
        m = rounded_mask((cw, ch), 18)
        sh = Image.new("L", (cw + 80, ch + 80), 0)
        ImageDraw.Draw(sh).rounded_rectangle((40, 52, cw + 40, ch + 46), 18, fill=150)
        sh = sh.filter(ImageFilter.GaussianBlur(20))
        bg.paste((0, 0, 0, 255), (x - 40, y0 - 40), sh)
        bg.paste(card, (x, y0), m)
        tw = d.textlength(t, font=F("PreludeWGL-Medium", 26))
        d.text((x + (cw - tw) / 2, y0 + ch + 22), t, font=F("PreludeWGL-Medium", 26), fill=(235, 240, 246))
    bg = bg.convert("RGB")
    bg.save(os.path.join(OUT, out_name), "JPEG", quality=86, optimize=True, progressive=True)



def pair(a, b, out_name, width=1000):
    l = touchpad(a, width // 2)
    r = touchpad(b, width // 2)
    c = Image.new("RGBA", (width, max(l.size[1], r.size[1])), (0, 0, 0, 0))
    c.alpha_composite(l, (0, 0))
    c.alpha_composite(r, (width // 2, 0))
    return save_webp(c, out_name)


if __name__ == "__main__":
    cardview([shot("mojo-wiki"), shot("enyo-wiki"), shot("mochi-wiki")],
             ["Jihad Mojo", "Jihad Enyo", "Jihad Mochi"], "cardview.jpg")
    save_webp(touchpad(shot("enyo-flash"), 520), "flash.webp")
    pair(shot("mojo-addons"), shot("mochi-personas"), "addons-personas.webp")
    pair(shot("enyo-privacy"), shot("mochi-config"), "settings.webp")
    save_webp(touchpad(shot("mojo-tls"), 520), "tls.webp")
    for f in sorted(f for f in os.listdir(OUT) if os.path.isfile(os.path.join(OUT, f))):
        print(f, os.path.getsize(os.path.join(OUT, f)) // 1024, "KB", Image.open(os.path.join(OUT, f)).size)
