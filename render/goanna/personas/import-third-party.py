#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Licensed under the Mozilla Public License 2.0.
"""Fetch the redistributable Personas listed in third-party.json and keep their pictures here.

For each entry: download the AMO package (cached in ~/.cache/jihad-personas), pin its sha256 in
third-party.json the first time and refuse a changed package afterwards, take the header picture
exactly as the browser's converter would (theme_frame / headerURL / additional_backgrounds[0]),
and the colours (frame / accentcolor, tab_background_text / textcolor). The picture is kept
byte-for-byte unless it is wallpaper-sized (400 px or taller: scaled to 1600 px wide, as the
browser does at install) — any such change is recorded in the entry ("modified"), as the
licences ask. Then run make-personas.py to rebuild personas.json.
"""
import hashlib, io, json, os, re, sys, urllib.request, zipfile
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
CACHE = os.path.expanduser("~/.cache/jihad-personas")
LIST = os.path.join(HERE, "third-party.json")
WALLPAPER_MIN_HEIGHT, WALLPAPER_WIDTH = 400, 1600


def colour(c):
    if isinstance(c, list) and len(c) >= 3:
        return "rgb(%s)" % ", ".join(str(int(v)) for v in c[:3])
    if isinstance(c, str) and re.match(r"^[#a-zA-Z0-9(),.%\s-]{1,40}$", c.strip()):
        return c.strip()
    return None


def main():
    os.makedirs(CACHE, exist_ok=True)
    doc = json.load(open(LIST))
    for p in doc["personas"]:
        path = os.path.join(CACHE, os.path.basename(p["xpi"]))
        if not os.path.exists(path):
            print("fetching", p["xpi"])
            urllib.request.urlretrieve(p["xpi"], path)
        data = open(path, "rb").read()
        sha = hashlib.sha256(data).hexdigest()
        if p.get("sha256") and p["sha256"] != sha:
            sys.exit("%s: the package changed (sha256 %s, pinned %s) — review it before taking it" % (p["slug"], sha, p["sha256"]))
        p["sha256"] = sha
        z = zipfile.ZipFile(io.BytesIO(data))
        m = json.loads(z.read("manifest.json").decode("utf-8-sig"))
        theme = m.get("theme", {}); images = theme.get("images", {}); colors = theme.get("colors", {})
        header = images.get("theme_frame") or images.get("headerURL") or (images.get("additional_backgrounds") or [None])[0]
        header = header.lstrip("./")
        ext = os.path.splitext(header)[1].lower().replace(".jpeg", ".jpg")
        raw = z.read(header)
        im = Image.open(io.BytesIO(raw))
        p["modified"] = None
        out = os.path.join(HERE, p["slug"] + ext)
        if im.height >= WALLPAPER_MIN_HEIGHT and im.width > WALLPAPER_WIDTH:
            h = round(im.height * WALLPAPER_WIDTH / im.width)
            im = im.convert("RGB").resize((WALLPAPER_WIDTH, h), Image.LANCZOS)
            out = os.path.join(HERE, p["slug"] + ".jpg")
            im.save(out, "JPEG", quality=88, optimize=True)
            p["modified"] = "scaled from %dx%d to %dx%d" % (Image.open(io.BytesIO(raw)).width, Image.open(io.BytesIO(raw)).height, WALLPAPER_WIDTH, h)
        else:
            open(out, "wb").write(raw)
        p["file"] = os.path.basename(out)
        p["accentcolor"] = colour(colors.get("frame") or colors.get("accentcolor")) or "#3a3a3a"
        p["textcolor"] = colour(colors.get("tab_background_text") or colors.get("textcolor") or colors.get("toolbar_text")) or "#ffffff"
        p["version"] = m.get("version") if isinstance(m.get("version"), str) else "1.0"
        print("%-22s %s %dx%d %s %s" % (p["slug"], p["file"], im.width, im.height, p["license"], p["modified"] or ""))
    with open(LIST, "w") as f:
        json.dump(doc, f, indent=2, ensure_ascii=False)
        f.write("\n")


if __name__ == "__main__":
    main()
