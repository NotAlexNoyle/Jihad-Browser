#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Licensed under the Mozilla Public License 2.0.
"""Draws the project's own Personas: a Frutiger Aero set (glossy water, sky, a green hill, light
ribbons, lime orbs), the look of the late-2000s screens the TouchPad was born among, drawn here from
gradients and shapes, so the art is this project's own; and Free Palestine (the owner, 2026-10-12),
made from a CC BY 2.0 photograph of Battir (see BATTIR below) with the flag and the words (Noto Sans
Black, SIL Open Font License) drawn over it.

Two come with the browser (the owner, 2026-10-12: Free Palestine and Aurora Silk); the add-on site
carries the rest (Riverstone-Addons `catalogue/`, where the Personas made by others now live too).

  make-personas.py                    the bundled two: <slug>.jpg beside this script and personas.json,
                                      which components/jihadPersonas.js offers in about:addons → Personas
  make-personas.py --catalogue DIR    also the other drawn pictures, into DIR, with their entries in
                                      DIR/drawn.json for the catalogue's personas.json
  --redraw                            draw pictures that exist already (their bytes name them: a new
                                      picture is a new Persona id, so the default leaves them alone)

A header is 1600 x 160: wide enough for a landscape card, tall enough for every shell's bar.
The browser anchors it top right, as Firefox does, so the interest is in the top 80 px and the
right 1024 px.
"""
import json, math, os, random
import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
W, H, K = 1600, 160, 2            # final size, supersample factor
SW, SH = W * K, H * K


def lerp(a, b, t):
    return a + (b - a) * t


def vgrad(top, bottom, curve=1.0):
    """A vertical gradient (RGB float array) from top to bottom colour."""
    t = np.linspace(0.0, 1.0, SH) ** curve
    rows = np.stack([lerp(np.float32(top[i]), np.float32(bottom[i]), t) for i in range(3)], axis=1)
    return np.repeat(rows[:, None, :], SW, axis=1)


def hgrad_mul(img, left, right):
    """Tint across the width: multiply by a left→right factor."""
    t = np.linspace(0.0, 1.0, SW)
    f = lerp(left, right, t)[None, :, None]
    return img * f


def over(base, rgb, alpha):
    """Composite rgb (3-tuple or HxWx3) with an alpha map (HxW, 0..1) over base."""
    a = alpha[..., None]
    return base * (1 - a) + np.asarray(rgb, dtype=np.float32) * a


def mask_from(draw_fn, blur=0):
    m = Image.new("L", (SW, SH), 0)
    draw_fn(ImageDraw.Draw(m))
    if blur:
        m = m.filter(ImageFilter.GaussianBlur(blur * K))
    return np.asarray(m, dtype=np.float32) / 255.0


def gloss(base, strength=0.55, height=0.48):
    """The Aero gloss: a bright band over the upper part, fading down, with a crisp lower edge."""
    y = np.linspace(0.0, 1.0, SH)[:, None]
    band = np.clip(1.0 - y / height, 0, 1) ** 1.6 * strength
    band = np.where(y < height, band, 0.0)
    return over(base, (255, 255, 255), np.broadcast_to(band, (SH, SW)))


def bubble(base, cx, cy, r, tint=(220, 245, 255), rim=0.75):
    """A soap/water bubble: clear middle, bright rim, a specular highlight top left."""
    yy, xx = np.ogrid[:SH, :SW]
    d = np.sqrt((xx - cx) ** 2 + (yy - cy) ** 2) / r
    inside = d <= 1.0
    body = np.where(inside, np.clip(d, 0, 1) ** 4 * rim, 0.0)
    edge = np.where(np.abs(d - 0.985) < 0.03, 0.9, 0.0)
    base = over(base, tint, (body + edge).clip(0, 1) * 0.85)
    hl = mask_from(lambda dr: dr.ellipse((cx - r * 0.62, cy - r * 0.72, cx - r * 0.02, cy - r * 0.30), fill=235), blur=r / K * 0.08)
    base = over(base, (255, 255, 255), hl)
    hl2 = mask_from(lambda dr: dr.ellipse((cx + r * 0.30, cy + r * 0.42, cx + r * 0.58, cy + r * 0.62), fill=120), blur=r / K * 0.10)
    return over(base, (255, 255, 255), hl2)


def orb(base, cx, cy, r, color):
    """A glossy sphere: shaded body, a bright cap, a soft glow under it."""
    yy, xx = np.ogrid[:SH, :SW]
    d = np.sqrt((xx - cx) ** 2 + ((yy - cy) * 1.0) ** 2) / r
    inside = (d <= 1.0).astype(np.float32)
    shade = np.clip(1.15 - d * 0.55 - (yy - cy) / r * 0.25, 0.35, 1.2)
    col = np.asarray(color, dtype=np.float32)[None, None, :] * shade[..., None]
    base = base * (1 - inside[..., None]) + np.clip(col, 0, 255) * inside[..., None]
    cap = mask_from(lambda dr: dr.ellipse((cx - r * 0.62, cy - r * 0.86, cx + r * 0.62, cy - r * 0.08), fill=200), blur=r / K * 0.12)
    return over(base, (255, 255, 255), cap * inside)


def glow(base, cx, cy, r, color, strength=1.0):
    yy, xx = np.ogrid[:SH, :SW]
    d = np.sqrt((xx - cx) ** 2 + (yy - cy) ** 2) / r
    a = np.clip(1.0 - d, 0, 1) ** 2 * strength
    return over(base, color, a.clip(0, 1))


def ribbon(base, phase, amp, freq, yc, thick, color, strength):
    """A luminous flowing band across the width (the Aero 'aurora' swoosh)."""
    xs = np.arange(SW, dtype=np.float32)
    centre = yc + amp * np.sin(xs / SW * freq * 2 * math.pi + phase)
    yy = np.arange(SH, dtype=np.float32)[:, None]
    d = np.abs(yy - centre[None, :]) / thick
    a = np.exp(-d * d) * strength
    core = np.exp(-(d * 4.0) ** 2) * strength * 0.9
    base = over(base, color, a.clip(0, 1))
    return over(base, (255, 255, 255), core.clip(0, 1))


def finish(arr, name):
    im = Image.fromarray(np.clip(arr, 0, 255).astype(np.uint8), "RGB").resize((W, H), Image.LANCZOS)
    path = os.path.join(HERE, name)
    im.save(path, "JPEG", quality=90, optimize=True, progressive=True)
    return path


def aqua(out_name):
    rnd = random.Random(7)
    a = vgrad((0, 150, 220), (120, 225, 255), 0.8)
    a = hgrad_mul(a, 0.92, 1.05)
    # caustic light: thin wavy bright lines
    for i in range(14):
        a = ribbon(a, rnd.uniform(0, 6.28), rnd.uniform(6, 18) * K, rnd.uniform(3, 9),
                   rnd.uniform(20, 150) * K, rnd.uniform(1.0, 2.2) * K, (200, 250, 255), 0.18)
    a = gloss(a, 0.45, 0.45)
    for i in range(26):
        x = rnd.uniform(560, 1590) * K
        y = rnd.uniform(-10, 150) * K
        r = rnd.choice([9, 12, 16, 22, 30, 44]) * K * rnd.uniform(0.8, 1.2)
        a = bubble(a, x, y, r)
    return finish(a, out_name)


def sky(out_name):
    rnd = random.Random(11)
    a = vgrad((40, 130, 230), (160, 215, 255), 1.1)
    # clouds: clusters of soft white ellipses
    for c in range(9):
        cx = rnd.uniform(500, 1500) * K
        cy = rnd.uniform(40, 150) * K
        m = mask_from(lambda dr: [dr.ellipse((cx + dx * K - rw * K, cy + dy * K - rh * K, cx + dx * K + rw * K, cy + dy * K + rh * K), fill=255)
                                   for dx, dy, rw, rh in [(rnd.uniform(-70, 70), rnd.uniform(-14, 10), rnd.uniform(22, 48), rnd.uniform(14, 26)) for _ in range(7)]],
                      blur=9)
        a = over(a, (255, 255, 255), m * 0.88)
    # sun and lens flare, top right
    a = glow(a, 1460 * K, 18 * K, 260 * K, (255, 250, 220), 0.9)
    a = glow(a, 1460 * K, 18 * K, 60 * K, (255, 255, 255), 1.0)
    for i, (t, rr, s) in enumerate([(0.18, 16, 0.35), (0.34, 9, 0.30), (0.55, 24, 0.18), (0.72, 7, 0.35)]):
        fx, fy = lerp(1460, 1000, t) * K, lerp(18, 140, t) * K
        a = glow(a, fx, fy, rr * K, (200, 235, 255), s)
    a = gloss(a, 0.30, 0.40)
    return finish(a, out_name)


def meadow(out_name):
    rnd = random.Random(3)
    a = vgrad((30, 120, 225), (150, 210, 250), 1.0)
    a = glow(a, 1300 * K, 0, 380 * K, (255, 255, 240), 0.55)
    # two rolling hills, glossy green
    def hill(base, y0, amp, freq, phase, top, bottom):
        xs = np.arange(SW, dtype=np.float32)
        crest = y0 + amp * np.sin(xs / SW * freq * math.pi + phase)
        yy = np.arange(SH, dtype=np.float32)[:, None]
        inside = (yy >= crest[None, :]).astype(np.float32)
        t = np.clip((yy - crest[None, :]) / (SH - crest.min() + 1), 0, 1)
        colr = np.stack([lerp(top[i], bottom[i], t) for i in range(3)], axis=-1)
        base = base * (1 - inside[..., None]) + colr * inside[..., None]
        lit = np.exp(-((yy - crest[None, :]) / (6 * K)) ** 2) * inside * 0.55
        return over(base, (220, 255, 190), lit)
    a = hill(a, 34 * K, 12 * K, 1.4, 0.6, (90, 190, 60), (40, 120, 25))
    a = hill(a, 50 * K, 11 * K, 2.2, 2.4, (120, 210, 70), (50, 140, 30))
    a = gloss(a, 0.28, 0.42)
    for i in range(5):
        a = bubble(a, rnd.uniform(900, 1580) * K, rnd.uniform(4, 22) * K, rnd.choice([5, 7, 10]) * K, rim=0.6)
    return finish(a, out_name)


def ribbons(out_name):
    a = vgrad((4, 28, 60), (0, 70, 110), 0.9)
    a = hgrad_mul(a, 0.7, 1.15)
    a = glow(a, 1350 * K, 60 * K, 420 * K, (0, 140, 200), 0.6)
    for ph, amp, fr, yc, th, col, s in [
        (0.4, 30, 1.1, 70, 16, (0, 200, 255), 0.55), (1.8, 24, 1.5, 85, 10, (120, 255, 200), 0.45),
        (3.0, 36, 0.9, 60, 22, (40, 120, 255), 0.40), (4.2, 18, 2.2, 95, 7, (200, 255, 255), 0.55),
        (5.1, 28, 1.3, 50, 12, (0, 255, 170), 0.35)]:
        a = ribbon(a, ph, amp * K, fr, yc * K, th * K, col, s)
    a = gloss(a, 0.22, 0.38)
    return finish(a, out_name)


def orbs(out_name):
    rnd = random.Random(21)
    a = vgrad((150, 225, 40), (40, 150, 20), 0.9)
    a = glow(a, 1250 * K, 30 * K, 400 * K, (240, 255, 170), 0.55)
    for i in range(12):
        x = rnd.uniform(620, 1580) * K
        y = rnd.uniform(15, 140) * K
        r = rnd.choice([10, 14, 20, 28, 36]) * K
        a = glow(a, x, y + r * 0.6, r * 1.6, (30, 90, 10), 0.25)
        a = orb(a, x, y, r, rnd.choice([(120, 220, 40), (170, 240, 60), (60, 180, 220), (90, 200, 30)]))
    for i in range(10):
        a = bubble(a, rnd.uniform(560, 1590) * K, rnd.uniform(0, 150) * K, rnd.choice([5, 7, 10]) * K, tint=(240, 255, 220), rim=0.7)
    a = gloss(a, 0.40, 0.45)
    return finish(a, out_name)


FONT = "/usr/share/fonts/noto/NotoSans-Black.ttf"   # SIL Open Font License; only its rendering is kept


def keffiyeh(base, y0, y1):
    """The keffiyeh's fishnet: two sets of fine diagonals in a band, with its heavier edge lines."""
    m = Image.new("L", (SW, SH), 0)
    d = ImageDraw.Draw(m)
    step = 14 * K
    for x in range(-SH, SW + SH, step):
        d.line([(x, y1), (x + (y1 - y0), y0)], fill=120, width=K)
        d.line([(x, y0), (x + (y1 - y0), y1)], fill=120, width=K)
    for yy in (y0, y0 + 4 * K, y1 - 4 * K, y1):
        d.line([(0, yy), (SW, yy)], fill=200, width=2 * K)
    # the "olive leaf" chain along the band: small leaves end to end
    mid = (y0 + y1) // 2
    for x in range(0, SW, 22 * K):
        d.ellipse([x, mid - 4 * K, x + 18 * K, mid + 4 * K], fill=210)
    band = np.zeros((SH, SW), np.float32)
    band[y0:y1 + 1, :] = 1.0
    return over(base, (235, 235, 228), np.asarray(m, np.float32) / 255.0 * band * 0.55)


def flag(base, x0, y0, w, h, pole_x):
    """The Palestinian flag waving from a pole: black, white and green bands, the red triangle at the
    hoist, a soft wave and its shading."""
    out = base.copy()
    ys, xs = np.mgrid[0:SH, 0:SW].astype(np.float32)
    u = (xs - x0) / w                                   # 0 at the hoist, 1 at the fly
    wave = np.sin(u * 2.4 * math.pi - 0.6) * (6 * K) * np.clip(u * 1.6, 0, 1)
    v = (ys - y0 - wave) / h                            # 0 at the top edge
    inside = (u >= 0) & (u <= 1) & (v >= 0) & (v <= 1)
    col = np.zeros((SH, SW, 3), np.float32)
    col[v < 1 / 3] = (16, 16, 16)
    col[(v >= 1 / 3) & (v < 2 / 3)] = (245, 245, 240)
    col[v >= 2 / 3] = (0, 122, 61)
    tri = u * 3.0 < (0.5 - np.abs(v - 0.5)) * 2.0       # the triangle, a third of the length deep
    col[tri] = (206, 17, 38)
    shade = 0.82 + 0.18 * np.cos(u * 2.4 * math.pi - 0.6)
    col *= shade[..., None]
    shape = Image.fromarray((inside * 255).astype(np.uint8))
    # A faint light behind the cloth and a hairline rim, so its black band reads against the night.
    halo = np.asarray(shape.filter(ImageFilter.GaussianBlur(9 * K)), np.float32) / 255.0
    out = over(out, (150, 170, 160), halo * 0.35)
    rim = np.asarray(shape.filter(ImageFilter.MaxFilter(2 * K + 1)), np.float32) / 255.0
    out = over(out, (205, 210, 205), rim * 0.85)
    a = np.asarray(shape.filter(ImageFilter.GaussianBlur(0.8 * K)), np.float32) / 255.0
    out = over(out, col, a)
    pole = mask_from(lambda d: d.rectangle([pole_x - 3 * K, 6 * K, pole_x + 1 * K, SH], fill=255), blur=0.5)
    out = over(out, (196, 198, 192), pole)
    knob = mask_from(lambda d: d.ellipse([pole_x - 5 * K, 2 * K, pole_x + 5 * K, 12 * K], fill=255), blur=0.5)
    return over(out, (225, 205, 120), knob)


def ridge_line(rnd, base_y, amps):
    """A hill's skyline across the width: base_y plus a few long, gentle swells (final pixels)."""
    x = np.linspace(0.0, 1.0, SW)
    y = np.full(SW, float(base_y))
    for amp, waves in amps:
        y += amp * np.sin(x * waves * 2 * math.pi + rnd.uniform(0, 2 * math.pi))
    return y * K


def hill(base, line, top, bottom, terraces=0, rnd=None):
    """Land below a skyline, lighter where the low sun catches its brow, with terrace lines when
    asked: the stepped hillsides of the West Bank's olive country."""
    ys = np.arange(SH, dtype=np.float32)[:, None]
    depth = ys - line[None, :]
    inside = (depth >= 0).astype(np.float32)
    t = np.clip(depth / (40.0 * K), 0, 1)
    col = (np.asarray(top, np.float32)[None, None, :] * (1 - t[..., None]) +
           np.asarray(bottom, np.float32)[None, None, :] * t[..., None])
    out = over(base, col, inside)
    if terraces:
        m = np.zeros((SH, SW), np.float32)
        for i in range(terraces):
            off = (8 + i * 9) * K
            row = (line + off + 2 * K * np.sin(np.linspace(0, 9, SW) + i)).astype(int)
            xs = np.arange(SW)
            ok = (row >= 0) & (row < SH)
            m[row[ok], xs[ok]] = 1.0
        m = np.asarray(Image.fromarray((m * 255).astype(np.uint8)).filter(ImageFilter.MaxFilter(3)), np.float32) / 255.0
        out = over(out, (0, 0, 0), m * inside * 0.26)
        lit = np.roll(m, -K, axis=0)                      # each step's sunlit lip, just above its shadow
        out = over(out, (255, 214, 150), lit * inside * 0.12)
    return out


def grove(rnd, d, line, x0, x1, size, avoid=()):
    """Olive trees in clusters along a hillside, as they are planted: a few together, then open ground."""
    x = x0
    while x < x1:
        n = rnd.randint(2, 5)
        for _ in range(n):
            xx = x + rnd.uniform(-6, 6)
            if 0 <= xx < W and not any(lo <= xx <= hi for lo, hi in avoid):
                olive_tree(d, xx * K, line[int(xx * K)] + rnd.uniform(5, 10) * K, size * rnd.uniform(0.75, 1.2), 255)
            x += rnd.uniform(14, 24)
        x += rnd.uniform(40, 110)


def olive_tree(d, x, ground, size, fill):
    """An olive tree's silhouette: a short twisted trunk and a broad, broken crown."""
    s = size * K
    d.polygon([(x - 0.12 * s, ground), (x + 0.12 * s, ground), (x + 0.05 * s, ground - 0.55 * s),
               (x + 0.2 * s, ground - 0.8 * s), (x - 0.02 * s, ground - 0.6 * s), (x - 0.18 * s, ground - 0.85 * s),
               (x - 0.06 * s, ground - 0.5 * s)], fill=fill)
    for dx, dy, r in ((-0.42, -1.0, 0.36), (0.0, -1.18, 0.42), (0.4, -1.0, 0.36), (-0.2, -0.82, 0.3), (0.24, -0.8, 0.3)):
        cx, cy, rr = x + dx * s, ground + dy * s, r * s
        d.ellipse([cx - rr * 1.25, cy - rr * 0.8, cx + rr * 1.25, cy + rr * 0.8], fill=fill)


def cypress(d, x, ground, size, fill):
    """A cypress: a tall, narrow flame of dark green."""
    s = size * K
    d.polygon([(x, ground - 2.6 * s), (x + 0.32 * s, ground - 1.2 * s), (x + 0.24 * s, ground),
               (x - 0.24 * s, ground), (x - 0.32 * s, ground - 1.2 * s)], fill=fill)


def old_city(d, x0, ground, fill, gold):
    """Jerusalem's old city on the far ridge: the wall with its crenellations, towers and roofs,
    and the Dome of the Rock's octagon and golden dome rising above them."""
    k = K
    d.rectangle([x0, ground - 9 * k, x0 + 150 * k, ground], fill=fill)                 # the wall
    for x in range(x0, x0 + 150 * k, 6 * k):
        d.rectangle([x, ground - 12 * k, x + 3 * k, ground - 9 * k], fill=fill)         # crenellations
    for x, h, w in ((14, 22, 9), (50, 16, 14), (98, 26, 7), (122, 18, 12)):          # towers and roofs
        d.rectangle([x0 + x * k, ground - h * k, x0 + (x + w) * k, ground], fill=fill)
    d.polygon([(x0 + 98 * k, ground - 26 * k), (x0 + 101.5 * k, ground - 34 * k), (x0 + 105 * k, ground - 26 * k)], fill=fill)
    cx = x0 + 74 * k
    d.rectangle([cx - 16 * k, ground - 22 * k, cx + 16 * k, ground - 9 * k], fill=fill)   # the octagon
    d.rectangle([cx - 9 * k, ground - 27 * k, cx + 9 * k, ground - 22 * k], fill=fill)    # the drum
    d.pieslice([cx - 10 * k, ground - 37 * k, cx + 10 * k, ground - 17 * k], 180, 360, fill=gold)
    d.line([(cx, ground - 37 * k), (cx, ground - 42 * k)], fill=gold, width=k)


# Free Palestine's land: "Village of Battir" by Labour Palestine, CC BY 2.0
# (https://commons.wikimedia.org/wiki/File:Village_of_Battir.jpg, from
# https://www.flickr.com/photos/labour2palestine/16428074717/) — Battir's terraces, the UNESCO site
# "Palestine: Land of Olives and Vines". Kept here at 1600 px wide (source/village-of-battir.jpg) so the
# Persona can be made again; credited on every card, in CREDITS.md and in NOTICE.
BATTIR = os.path.join(HERE, "source", "village-of-battir.jpg")


def palestine(out_name):
    """Free Palestine (the owner, 2026-10-12: "an artistic depiction of palestinian land", then "photo
    realistic"): a photograph of Battir's terraced hillside under a clear sky — olive trees, cypresses,
    the village on the ridge — cut to a header's band; the sky deepened towards the top, where the
    toolbar's light glyphs and the words sit; the flag on its pole at the right. Changes made to the
    photograph: cropped, scaled, the sky deepened, the words and the flag drawn over it."""
    photo = Image.open(BATTIR).convert("RGB")
    photo = photo.resize((SW, round(photo.height * SW / photo.width)), Image.LANCZOS)
    a = np.asarray(photo.crop((0, 230 * K, SW, 230 * K + SH)), np.float32)
    # The sky deepened towards the top: a light glyph on a pale sky reads poorly.
    t = np.clip(1.0 - np.arange(SH, dtype=np.float32) / (70.0 * K), 0, 1)[:, None, None]
    a = a * (1.0 - 0.45 * t) + np.array([10, 30, 80], np.float32) * 0.45 * t
    # The words, white with a soft shadow, a hairline of the flag's red beneath.
    text = Image.new("L", (SW, SH), 0)
    font = ImageFont.truetype(FONT, 34 * K)
    words = "FREE PALESTINE"
    tw = ImageDraw.Draw(text).textlength(words, font=font)
    tx, ty = 1170 * K - tw, 4 * K
    ImageDraw.Draw(text).text((tx, ty), words, font=font, fill=255)
    shadow = np.asarray(text.filter(ImageFilter.GaussianBlur(3 * K)), np.float32) / 255.0
    a = over(a, (0, 0, 0), np.roll(np.roll(shadow, 2 * K, 0), 2 * K, 1) * 0.75)
    a = over(a, (250, 250, 246), np.asarray(text, np.float32) / 255.0)
    rule = mask_from(lambda d: d.rectangle([tx, 53 * K, 1170 * K, 55 * K], fill=255))
    a = over(a, (206, 17, 38), rule)
    a = flag(a, 1214 * K, 10 * K, 360 * K, 86 * K, 1209 * K)
    return finish(a, out_name)


# slug (file name), name, drawing, accent colour, text colour, earlier names (a profile that has a
# Persona under an earlier name gets it renamed in place, and one uninstalled under an earlier
# name stays uninstalled — components/jihadPersonas.js offerBundledOnce).
PERSONAS = [
    ("bubble-lagoon", "Bubble Lagoon", aqua, "rgb(0, 150, 220)", "#0a2c4c", ["Aero Aqua"]),
    ("cloud-nine", "Cloud Nine", sky, "rgb(40, 130, 230)", "#0b2a52", ["Aero Sky"]),
    ("meadow-morning", "Meadow Morning", meadow, "rgb(30, 120, 225)", "#0a2a12", ["Aero Meadow"]),
    ("aurora-silk", "Aurora Silk", ribbons, "rgb(4, 28, 60)", "#ffffff", ["Aero Ribbons"]),
    ("lime-fizz", "Lime Fizz", orbs, "rgb(60, 160, 25)", "#0f2a05", ["Aero Orbs"]),
    ("free-palestine", "Free Palestine", palestine, "rgb(30, 64, 110)", "#ffffff", []),
]
BUNDLED = ("free-palestine", "aurora-silk")    # in this order, first in the gallery
# A Persona made from someone else's work carries that work's licence and credit (shown on its card).
CREDIT = {
    "free-palestine": {"license": "CC BY 2.0 (photo: Labour Palestine)",
                       "licenseURL": "https://creativecommons.org/licenses/by/2.0/",
                       "source": "https://commons.wikimedia.org/wiki/File:Village_of_Battir.jpg"},
}

if __name__ == "__main__":
    import argparse
    ap = argparse.ArgumentParser()
    ap.add_argument("--catalogue")
    ap.add_argument("--redraw", action="store_true")
    args = ap.parse_args()
    bundled, drawn = [], []
    for slug, name, fn, accent, text, former in PERSONAS:
        where = HERE if slug in BUNDLED else args.catalogue
        if not where:
            continue
        path = os.path.join(where, slug + ".jpg")
        if args.redraw or not os.path.exists(path):
            fn(path)
            print("drew", path, os.path.getsize(path) // 1024, "KB")
        entry = {"slug": slug, "name": name, "file": os.path.basename(path), "accentcolor": accent,
                 "textcolor": text, "author": "Jihad Browser", "version": "1.0", "formerNames": former,
                 "license": "MPL-2.0"}
        entry.update(CREDIT.get(slug, {}))
        drawn.append(entry)
        if slug in BUNDLED:
            bundled.append(entry)
    bundled.sort(key=lambda e: BUNDLED.index(e["slug"]))
    with open(os.path.join(HERE, "personas.json"), "w") as f:
        # "edition" goes up whenever the set, its names or its art change: a profile offered an
        # earlier edition gets the changes — new pictures replace their older ones, new names
        # rename in place (edition 2 drew the meadow lower; 3 retired edition 1's meadow; 4 gave
        # the set its own names; 5 added the redistributable Personas made by others; 6 kept two in
        # the browser, Free Palestine new among them, and moved the rest to the add-on site — a
        # profile keeps those it has; 7 drew Free Palestine as the land at dusk; 8 made it from a
        # photograph of Battir).
        json.dump({"collection": "Jihad Browser", "edition": 8, "personas": bundled}, f, indent=2, ensure_ascii=False)
        f.write("\n")
    if args.catalogue:
        with open(os.path.join(args.catalogue, "drawn.json"), "w") as f:
            json.dump({"personas": drawn}, f, indent=2, ensure_ascii=False)
            f.write("\n")
