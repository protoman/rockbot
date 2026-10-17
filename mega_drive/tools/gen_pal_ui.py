#!/usr/bin/env python3
"""Generate the shared PAL0 palette (tools/pal_ui_data.py) from the art.

PAL0 serves enemies, objects, projectiles and the HUD balls. The previous
table was a frozen snapshot of an early hand-tuned pool; it had no black or
white for the art (outlines quantized to dark red) and no true blues, so the
sprites read dark and desaturated.

This builds the table with a weighted k-means over every pixel the line will
actually draw:

  1 slot  (index 0)  transparent/backdrop black
  1 slot  (index 1)  art black      (sprite index 0 = transparent, so the
  1 slot  (index 2)  art white       art needs its own black/white entries)
 12 slots (index 3..14) k-means colors, mid-tones only
  1 slot  (index 15) font white 0x0EEE (VDP_drawText needs it; pinned)

Deterministic: fixed init (most frequent distinct colours), fixed iteration
count, MD 3:3:3 rounding at every step.

Usage: python3 tools/gen_pal_ui.py
"""
import array
import hashlib
import os
import sys
from collections import Counter
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ROCKBOT = os.path.dirname(ROOT)
DATA = os.path.join(ROCKBOT, "build/games/RockDroid1")

ENEMY_IDS = [1, 21, 23, 37, 45, 51]
K = 12                      # free slots (index 3..14)
ITER = 30
MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)
COLORKEY = (75, 125, 125)
FONT_WHITE = (219, 219, 219)


def md_round(v):
    return min(MD_LEVELS, key=lambda x: abs(x - v))


def load_rgba(path):
    im = Image.open(path).convert("RGBA")
    px = im.load()
    for y in range(im.size[1]):
        for x in range(im.size[0]):
            r, g, b, a = px[x, y]
            if a < 255 or (r, g, b) == COLORKEY:
                px[x, y] = (0, 0, 0, 0)
    return im


def art_files():
    out = []
    # enemies: only the frames the game ships (stand/walk), same as the converter
    sys.path.insert(0, HERE)
    from convert_enemies import read_enemy_defs, NPC_SZ
    blob = open(os.path.join(DATA, "game_enemy_list_3_1_2.dat"), "rb").read()
    defs = read_enemy_defs(blob, set(ENEMY_IDS))
    for eid, d in defs.items():
        p = os.path.join(DATA, "images/sprites", "sprites", "enemies", d["gfx"])
        if not os.path.exists(p):
            p = os.path.join(DATA, "images/sprites/enemies", d["gfx"])
        out.append((p, d["fw"], d["fh"],
                    sorted({px for px, _, _ in d["stand"] + d["walk"]})))
    # projectiles drawn with PAL0
    for name in ("projectile_normal.png", "charged_semi.png", "charged_shot.png"):
        p = os.path.join(DATA, "images/projectiles", name)
        if os.path.exists(name and p):
            out.append((p, None, None, None))       # whole image
    # objects
    objdir = os.path.join(DATA, "images/sprites/objects")
    if os.path.isdir(objdir):
        for name in sorted(os.listdir(objdir)):
            if name.endswith(".png"):
                out.append((os.path.join(objdir, name), None, None, None))
    # HUD balls (player + boss share the art)
    for name in ("hp_ball.png", "boss_hp_ball.png"):
        p = os.path.join(ROCKBOT, "build/shared/images", name)
        if os.path.exists(p):
            out.append((p, None, None, None))
    return out


def gather():
    counts = Counter()
    own = []
    for (path, fw, fh, frames) in art_files():
        im = load_rgba(path)
        px = im.load()
        w, h = im.size
        for y in range(h):
            for x in range(w):
                if fw:      # enemy sheet: only the shipped frames
                    if (x // fw) not in frames:
                        continue
                r, g, b, a = px[x, y]
                if a == 0:
                    continue
                counts[(md_round(r), md_round(g), md_round(b))] += 1
    return counts


def dist2(a, b):
    return (a[0]-b[0])**2 + (a[1]-b[1])**2 + (a[2]-b[2])**2


def kmeans(samples, weights, k, iters):
    centers = [list(s) for s in samples[:k]]
    for _ in range(iters):
        acc = [[0, 0, 0, 0] for _ in range(k)]
        for s, w in zip(samples, weights):
            bi, bd = 0, 1 << 30
            for i, c in enumerate(centers):
                d = dist2(s, c)
                if d < bd:
                    bd, bi = d, i
            acc[bi][0] += s[0]*w; acc[bi][1] += s[1]*w
            acc[bi][2] += s[2]*w; acc[bi][3] += w
        for i in range(k):
            if acc[i][3] == 0:
                continue
            centers[i] = [md_round(acc[i][0]/acc[i][3]),
                          md_round(acc[i][1]/acc[i][3]),
                          md_round(acc[i][2]/acc[i][3])]
    return [tuple(c) for c in centers]


def main():
    counts = gather()
    # mid-tones only: black/white are pinned slots
    mid = [(c, n) for c, n in counts.most_common()
           if sum(c) > 40 and min(c) < 200]
    if len(mid) < K:
        sys.exit("not enough colors to fit the palette")
    centers = kmeans([c for c, _ in mid], [n for _, n in mid], K, ITER)
    # deterministic order + de-dup
    seen = set()
    colors = []
    for c in sorted(centers, key=lambda c: -sum(c)):
        if c not in seen and sum(c) > 40:
            seen.add(c)
            colors.append(c)
    # top up from the most common mid-tones if k-means collapsed slots
    for c, _ in mid:
        if len(colors) >= K:
            break
        if c not in seen:
            seen.add(c)
            colors.append(c)
    colors = colors[:K]
    while len(colors) < K:
        colors.append(MD_LEVELS[3])          # deterministic filler

    pal = [(0, 0, 0), (0, 0, 0), (255, 255, 255)] + colors + [FONT_WHITE]
    assert len(pal) == 16, pal

    total = sum(counts.values())
    err = sum(n * dist2(c, min(pal, key=lambda p: dist2(c, p)))
              for c, n in counts.items())
    print(f"art pixels {total}, weighted mean color error "
          f"{err/total:.0f} (squared RGB), colors: {pal}")

    out = os.path.join(HERE, "pal_ui_data.py")
    with open(out, "w") as f:
        f.write('"""Generated by tools/gen_pal_ui.py from the PC art. '
                'Do not edit."""\n\n')
        f.write("PAL_UI_RGB = [\n")
        for i, c in enumerate(pal):
            f.write(f"    {c},   # {i}\n")
        f.write("]\n")
    print(f"wrote tools/pal_ui_data.py")
    return 0


if __name__ == "__main__":
    sys.exit(main())
