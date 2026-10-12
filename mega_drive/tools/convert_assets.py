#!/usr/bin/env python3
"""Convert original PNGs (RGBA/RGB/P) -> MD indexed PNG (index 0 reserved = transparent).

Usage:
    python3 tools/convert_assets.py            # convert all
    python3 tools/convert_assets.py upperland  # convert one

Output: mega_drive/res/<name>.png (8bpp indexed)

Rules:
  - MD: 4 palettes x 16 colors. Index 0 of any palette = transparent.
  - RockBot's only transparent color is (75,125,125) = 0x4B7D7D (defines.h:961).
    The other colorkeys were for colorcycle, disabled in the game -- ignored.
  - MD colors are 3:3:3, RGB is rounded to those 8 levels per channel.
  - Index 0 is always left unused so no pixel can become transparent by accident.
"""
import os
import sys
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)                      # mega_drive/
ROCKBOT = os.path.dirname(ROOT)                   # rockbot/
OUT_DIR = os.path.join(ROOT, "res")

COLORKEY = (75, 125, 125)

# CONFIG_BGCOLOR from defines.h:1421-1423
BAND_RGB = (5, 29, 65)

# name -> dict
#   src         path relative to rockbot root
#   colors      max colors to keep (index 0 excluded, so this + 1 <= 16)
#   pad         pad to multiple of 8 px
#   dither      Floyd-Steinberg dithering
#   crop        (left, upper, right, lower) or None
#   band        px of BAND_RGB prepended at the top, then cropped to screen height
#   replace     list of (src_rgb, dst_rgb, tolerance) applied before quantizing
ASSETS = {
    # intro: show_notice()
    "upperland": dict(src="build/shared/images/upperland.png", colors=15, pad=True),
    "boberatu":  dict(src="build/shared/images/boberatu.png",  colors=15, pad=True),
    "jmd_logo":  dict(src="build/shared/images/jmd_logo.png",  colors=15, pad=True),
    # title screen: sceneslib.cpp:102 draw_main() -> images/logo.png
    # The PNG background is (72,126,124): the game's colorkey, off by a few
    # units from (75,125,125). It must read as dark blue, not teal.
    "title_logo": dict(src="build/games/RockDroid1/images/logo.png",
                       colors=15, pad=False, dither=True, band=8,
                       replace=[((72, 126, 124), BAND_RGB, 20)]),
}

MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)
SCREEN_H = 224


def md_round(v):
    return min(MD_LEVELS, key=lambda x: abs(x - v))


def prepare(img, spec):
    """Crop / add top band / crop to screen height. Keeps alpha."""
    if spec.get("crop"):
        img = img.crop(spec["crop"])
    if spec.get("band"):
        band = spec["band"]
        canvas = Image.new("RGBA", (img.width, img.height + band), BAND_RGB + (255,))
        canvas.paste(img, (0, band))
        img = canvas
    if img.height > SCREEN_H:
        img = img.crop((0, 0, img.width, SCREEN_H))
    return img


def is_transparent(pixel, mask_cache):
    """alpha == 0 or the RockBot colorkey."""
    r, g, b, a = pixel
    if a == 0:
        return True
    return (r, g, b) == COLORKEY


def convert(name, spec):
    src = os.path.join(ROCKBOT, spec["src"])
    if not os.path.exists(src):
        print(f"  SKIP {name}: {src} not found")
        return False

    img = prepare(Image.open(src).convert("RGBA"), spec)
    w, h = img.size
    data = list(img.getdata())

    # color remapping, applied before quantizing. Used to turn the logo's teal
    # colorkey background into the dark blue backdrop.
    for src_rgb, dst_rgb, tol in spec.get("replace", []):
        data = [
            dst_rgb + (p[3],) if (
                abs(p[0] - src_rgb[0]) <= tol
                and abs(p[1] - src_rgb[1]) <= tol
                and abs(p[2] - src_rgb[2]) <= tol
            ) else p
            for p in data
        ]

    mask = [is_transparent(p, None) for p in data]

    opaque = [p[:3] for p, m in zip(data, mask) if not m]
    if not opaque:
        print(f"  SKIP {name}: fully transparent")
        return False

    # dominant opaque color fills the transparent holes so they do not steal
    # palette slots during quantization (they are forced to index 0 afterwards)
    fill = max(set(opaque), key=opaque.count)
    rgb = Image.new("RGB", (w, h), fill)
    rgb.putdata([p[:3] if not m else fill for p, m in zip(data, mask)])

    dither = Image.Dither.FLOYDSTEINBERG if spec.get("dither") else Image.Dither.NONE
    q = rgb.quantize(colors=spec["colors"], method=Image.Quantize.MEDIANCUT, dither=dither)
    qpal = q.getpalette()
    colors = [tuple(md_round(v) for v in qpal[i*3:i*3+3]) for i in range(spec["colors"])]

    # index 0 reserved/transparent, real colors at 1..15
    palette = [(0, 0, 0)] + colors
    while len(palette) < 16:
        palette.append((0, 0, 0))

    lut = {}
    for i, c in enumerate(colors):
        lut.setdefault(c, i + 1)

    qidx = list(q.getdata())
    out = []
    for qi, m in zip(qidx, mask):
        if m:
            out.append(0)
            continue
        c = colors[qi] if qi < len(colors) else colors[0]
        if c not in lut:
            best = min(lut.keys(), key=lambda k: (k[0]-c[0])**2 + (k[1]-c[1])**2 + (k[2]-c[2])**2)
            c = best
        out.append(lut[c])

    im = Image.new("P", (w, h))
    im.putpalette([v for c in palette for v in c])
    im.putdata(out)

    if spec.get("pad"):
        w, h = im.size
        nw, nh = (w + 7) & ~7, (h + 7) & ~7
        if (nw, nh) != (w, h):
            p = Image.new("P", (nw, nh))
            p.putpalette(im.getpalette())
            p.paste(0, (0, 0, nw, nh))
            p.paste(im, (0, 0))
            im = p

    os.makedirs(OUT_DIR, exist_ok=True)
    im.save(os.path.join(OUT_DIR, f"{name}.png"))

    data = list(im.getdata())
    print(f"  OK {name}: {img.size} -> {im.size}, "
          f"{len(set(data))} idx used, {data.count(0)} transparent px")
    return True


def main():
    targets = sys.argv[1:] or list(ASSETS)
    ok = sum(1 for n in targets if n in ASSETS and convert(n, ASSETS[n]))
    print(f"{ok}/{len(targets)} converted -> {OUT_DIR}")


if __name__ == "__main__":
    main()
