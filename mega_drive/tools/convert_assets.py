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
import hashlib
import os
import sys
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)                      # mega_drive/
ROCKBOT = os.path.dirname(ROOT)                   # rockbot/
IMAGES = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "images")  # hand-owned art
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
    # NOTE: the title logo is no longer an IMAGE here — it is decomposed by
    # tools/img2tiles.py (res/title_tiles.png + src/title_tiles.c) and drawn
    # by showTitle(), so no title_logo entry.
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


# HUD energy balls: 5 frames of 10x10 (fill: full..empty) drawn with PAL0,
# so pixels are quantized into the shared enemy/font palette
# (stage0_enemies.c palette_ui_enemies) instead of their own.
from pal_ui import PAL_UI_RGB, nearest_index      # shared PAL0 (see pal_ui.py)

HUD_SRC = os.path.join(ROCKBOT, "build/shared/images")
CODE_DIR = os.path.join(ROOT, "src")


COLORKEY = (75, 125, 125)


def load_rgba(path):
    """RGBA with the game colorkey punched to alpha 0 (convert_enemies style)."""
    im = Image.open(path).convert("RGBA")
    px = im.load()
    w, h = im.size
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < 255 or (r, g, b) == COLORKEY:
                px[x, y] = (0, 0, 0, 0)
    return im


def convert_hud_ball(src_name="hp_ball.png", out_name="hud_hp_ball.png"):
    """One 5-frame 16x16 HP ball sheet (PAL0 via tools/pal_ui.py).

    Player and boss sheets differ only in the fill color; they cannot share
    tiles through SGDK's per-frame tilesets, so both sheets are emitted —
    but with the SAME palette table as the enemies (they used to disagree,
    which made the fill resolve to the wrong color at runtime)."""
    src = os.path.join(HUD_SRC, src_name)
    if not os.path.exists(src):
        print(f"  SKIP {out_name}: {src} not found")
        return False
    img = load_rgba(src)
    fw = img.height            # square frames (10x10)
    n = img.size[0] // fw
    px = img.load()
    out = Image.new("P", (n * 16, 16))
    out.putpalette([v for c in PAL_UI_RGB for v in c] + [0] * (768 - 16 * 3))
    data = [0] * (n * 16 * 16)
    for f in range(n):
        for y in range(fw):
            for x in range(fw):
                p = px[f * fw + x, y]
                if p[3] == 0:
                    idx = 0
                else:
                    c = (md_round(p[0]), md_round(p[1]), md_round(p[2]))
                    idx = 15 if c == (219, 219, 219) else nearest_index(c)
                data[y * n * 16 + f * 16 + x] = idx
    out.putdata(data)
    os.makedirs(OUT_DIR, exist_ok=True)
    out.save(os.path.join(OUT_DIR, out_name))
    print(f"  OK {out_name}: {img.size} -> {out.size} ({n} frames, PAL0)")
    return True


def main():
    explicit = sys.argv[1:]
    targets = explicit or list(ASSETS)
    ok = sum(1 for n in targets if n in ASSETS and convert(n, ASSETS[n]))
    n = len(targets)
    if not explicit or "hud_hp_ball" in explicit:
        ok += convert_hud_ball()
        n += 1
    if not explicit or "hud_boss_ball" in explicit:
        ok += convert_hud_ball("boss_hp_ball.png", "hud_boss_ball.png")
        n += 1
    if not explicit or "hud_wpn_ball" in explicit:
        ok += convert_hud_ball("wpn_ball.png", "hud_wpn_ball.png")
        n += 1
    print(f"{ok}/{n} converted -> {OUT_DIR}")


if __name__ == "__main__":
    main()
