#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Licensed under the Mozilla Public License 2.0.
"""Draws the Personas that come with Jihad Browser: a Frutiger Aero set (glossy water, sky, a
green hill, light ribbons, lime orbs), the look of the late-2000s screens the TouchPad was born
among. Every pixel is drawn here from gradients and shapes, so the art is this project's own.

Output (beside this script): one <slug>.jpg header picture per Persona and personas.json, which the browser's
Persona component (components/jihadPersonas.js) offers in about:addons → Personas.

A header is 1600 x 160: wide enough for a landscape card, tall enough for every shell's bar.
The browser anchors it top right, as Firefox does, so the interest is in the top 80 px and the
right 1024 px.
"""
import json, math, os, random
import numpy as np
from PIL import Image, ImageDraw, ImageFilter

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


# slug (file name), name, drawing, accent colour, text colour, earlier names (a profile that has a
# Persona under an earlier name gets it renamed in place, and one uninstalled under an earlier
# name stays uninstalled — components/jihadPersonas.js offerBundledOnce).
PERSONAS = [
    ("bubble-lagoon", "Bubble Lagoon", aqua, "rgb(0, 150, 220)", "#0a2c4c", ["Aero Aqua"]),
    ("cloud-nine", "Cloud Nine", sky, "rgb(40, 130, 230)", "#0b2a52", ["Aero Sky"]),
    ("meadow-morning", "Meadow Morning", meadow, "rgb(30, 120, 225)", "#0a2a12", ["Aero Meadow"]),
    ("aurora-silk", "Aurora Silk", ribbons, "rgb(4, 28, 60)", "#ffffff", ["Aero Ribbons"]),
    ("lime-fizz", "Lime Fizz", orbs, "rgb(60, 160, 25)", "#0f2a05", ["Aero Orbs"]),
]

if __name__ == "__main__":
    out = []
    for slug, name, fn, accent, text, former in PERSONAS:
        path = fn(slug + ".jpg")
        out.append({"slug": slug, "name": name, "file": os.path.basename(path), "accentcolor": accent,
                    "textcolor": text, "author": "Jihad Browser", "version": "1.0", "formerNames": former,
                    "license": "MPL-2.0"})
        print(path, os.path.getsize(path) // 1024, "KB")
    # Personas made by others that we may redistribute (import-third-party.py keeps their pictures
    # and third-party.json their credits): offered the same way, credited on every card.
    for p in json.load(open(os.path.join(HERE, "third-party.json")))["personas"]:
        out.append({k: p[k] for k in ("slug", "name", "file", "accentcolor", "textcolor", "author", "version",
                                       "license", "licenseURL", "source", "modified", "formerNames") if k in p})
    with open(os.path.join(HERE, "personas.json"), "w") as f:
        # "edition" goes up whenever the set, its names or its art change: a profile offered an
        # earlier edition gets the changes — new pictures replace their older ones, new names
        # rename in place (edition 2 drew the meadow lower; 3 retired edition 1's meadow; 4 gave
        # the set its own names; 5 added the redistributable Personas made by others).
        json.dump({"collection": "Jihad Browser", "edition": 5, "personas": out}, f, indent=2, ensure_ascii=False)
        f.write("\n")
