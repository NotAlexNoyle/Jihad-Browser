#!/usr/bin/env python3
"""Compose the README's art from raw device screenshots: TouchPad-framed shots and a webOS-style
card view on a Frutiger Aero backdrop, the interfaces in order of age (Mojo, Enyo, Mochi).

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
    """A Frutiger Aero backdrop, drawn with the project's own Persona primitives
    (render/goanna/personas/make-personas.py): a sky with soft clouds and a sun, glossy green
    hills, a few bubbles and orbs, the gloss band. The owner's brief (2026-10-15): the card view
    shows the interfaces unthemed, on a Frutiger Aero background."""
    import importlib.util, math, random
    import numpy as np
    spec = importlib.util.spec_from_file_location(
        "make_personas", os.path.join(REPO, "render", "goanna", "personas", "make-personas.py"))
    mp = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mp)
    w, h = size
    K = 2                                        # supersample, as the Personas are drawn
    mp.K, mp.SW, mp.SH = K, w * K, h * K         # the primitives read these at call time
    SW, SH = mp.SW, mp.SH
    rnd = random.Random(7)
    a = mp.vgrad((36, 122, 226), (178, 224, 255), 1.1)
    # clouds: clusters of soft white ellipses across the upper half
    for c in range(8):
        cx = rnd.uniform(0.05, 0.95) * SW
        cy = rnd.uniform(0.10, 0.42) * SH
        puffs = [(rnd.uniform(-0.045, 0.045) * SW, rnd.uniform(-0.012, 0.010) * SH,
                  rnd.uniform(0.016, 0.034) * SW, rnd.uniform(0.016, 0.030) * SH) for _ in range(7)]
        m = mp.mask_from(lambda dr: [dr.ellipse((cx + dx - rw, cy + dy - rh, cx + dx + rw, cy + dy + rh), fill=255)
                                     for dx, dy, rw, rh in puffs], blur=10)
        a = mp.over(a, (255, 255, 255), m * 0.80)
    # the sun and its lens flare, top right
    sx, sy = 0.91 * SW, 0.07 * SH
    a = mp.glow(a, sx, sy, 0.32 * SW, (255, 250, 220), 0.75)
    a = mp.glow(a, sx, sy, 0.05 * SW, (255, 255, 255), 1.0)
    for t, rr, s in [(0.16, 0.012, 0.35), (0.30, 0.007, 0.30), (0.48, 0.018, 0.18), (0.64, 0.006, 0.35)]:
        a = mp.glow(a, mp.lerp(sx, 0.45 * SW, t), mp.lerp(sy, 0.60 * SH, t), rr * SW, (200, 235, 255), s)
    # two rolling hills, glossy green, across the lower third
    def hill(base, y0, amp, freq, phase, top, bottom):
        xs = np.arange(SW, dtype=np.float32)
        crest = y0 + amp * np.sin(xs / SW * freq * math.pi + phase)
        yy = np.arange(SH, dtype=np.float32)[:, None]
        inside = (yy >= crest[None, :]).astype(np.float32)
        t = np.clip((yy - crest[None, :]) / (SH - crest.min() + 1), 0, 1)
        colr = np.stack([mp.lerp(top[i], bottom[i], t) for i in range(3)], axis=-1)
        base = base * (1 - inside[..., None]) + colr * inside[..., None]
        lit = np.exp(-((yy - crest[None, :]) / (7 * K)) ** 2) * inside * 0.55
        return mp.over(base, (220, 255, 190), lit)
    a = hill(a, 0.70 * SH, 0.045 * SH, 1.3, 0.5, (95, 195, 65), (35, 112, 25))
    a = hill(a, 0.79 * SH, 0.035 * SH, 2.1, 2.3, (125, 215, 75), (45, 135, 30))
    # orbs resting on the near hill, bubbles drifting up
    for x, y, r, col in [(0.07, 0.86, 0.030, (120, 220, 40)), (0.13, 0.90, 0.020, (60, 180, 220)),
                         (0.90, 0.88, 0.026, (170, 240, 60)), (0.95, 0.93, 0.016, (90, 200, 30))]:
        a = mp.glow(a, x * SW, (y + r * 0.6) * SH, r * 1.6 * SW, (30, 90, 10), 0.25)
        a = mp.orb(a, x * SW, y * SH, r * SW, col)
    for i in range(9):
        a = mp.bubble(a, rnd.uniform(0.02, 0.98) * SW, rnd.uniform(0.45, 0.97) * SH,
                      rnd.choice([0.006, 0.009, 0.013]) * SW, tint=(235, 250, 255), rim=0.65)
    a = mp.gloss(a, 0.22, 0.36)
    return Image.fromarray(np.clip(a, 0, 255).astype(np.uint8), "RGB").resize((w, h), Image.LANCZOS)


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
    def text(xy, s, font, fill):
        d.text((xy[0] + 2, xy[1] + 2), s, font=font, fill=(10, 30, 60, 110))
        d.text(xy, s, font=font, fill=fill)
    text((x0 + 104, 40), "Jihad Browser", F("PreludeWGL-Medium", 50), (255, 255, 255))
    text((x0 + 106, 98), "for webOS", F("PreludeWGL-Light", 24), (228, 240, 255))
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
        text((x + (cw - tw) / 2, y0 + ch + 22), t, F("PreludeWGL-Medium", 26), (255, 255, 255))
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
