#!/usr/bin/env python3
"""Save-slot icons: the PC's images/icons.png -> one SGDK SPRITE per icon.

The PC's save screen (scenesLib::draw_save_details, sceneslib.cpp:916) shows,
next to each occupied slot, one icon per stage/weapon plus the selected
player's icon. The source sheet is 288x32 RGBA: 18 icons of 16x16, each with
TWO frames stacked vertically — top = "not owned" (disabled), bottom = "owned"
(graphicsLib::draw_weapon_tooltip_icon, graphicslib.cpp:1357).

Index map (same order the PC uses, draw_save_details):
    0..8   stages 0..8 / weapons  (save.stages[i] == 1 -> enabled)
    11     energy tank   } the port does not model the tanks
    12     weapon tank   } (or the difficulty), so only 0..8 and the
    13     special tank  } player icons are emitted
    14..17 players 1..4  (save.selected_player)

Output: res/sicon_<i>.png, 32x16 = 2 frames of 16x16 laid out HORIZONTALLY
(SGDK SPRITE sheets are horizontal strips), frame 0 = disabled, frame 1 =
enabled. One 16-colour palette + index 0 transparent, shared by every icon
(rescomp dedups the identical PALETTE resources).
"""
import os
import sys
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ROCKBOT = os.path.dirname(ROOT)
SRC = os.path.join(ROCKBOT, "build/games/RockDroid1/images/icons.png")
RES = os.path.join(ROOT, "res")

ICON = 16
ICON_COUNT = 18
EMIT = list(range(0, 9)) + [14, 15, 16, 17]   # stages/weapons + players

MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)
COLORKEY = (75, 125, 125)


def md_round(v):
    return min(MD_LEVELS, key=lambda x: abs(x - v))


def is_key(c):
    return (abs(c[0] - COLORKEY[0]) <= 6 and abs(c[1] - COLORKEY[1]) <= 6
            and abs(c[2] - COLORKEY[2]) <= 6)


def main():
    if not os.path.exists(SRC):
        print(f"missing {SRC}", file=sys.stderr)
        return 1
    im = Image.open(SRC).convert("RGBA")
    w, h = im.size
    if h != ICON * 2:
        print(f"unexpected sheet {im.size}", file=sys.stderr)
        return 1
    n = min(ICON_COUNT, w // ICON)
    px = im.load()

    # palette: every opaque colour of the emitted icons, quantized to the MD
    # 3:3:3 grid (same rule the rest of the pipeline uses)
    colours = {}
    for i in range(n):
        for fy in range(ICON * 2):
            for x in range(ICON):
                r, g, b, a = px[i * ICON + x, fy]
                if a < 128 or is_key((r, g, b)):
                    continue
                c = (md_round(r), md_round(g), md_round(b))
                colours[c] = colours.get(c, 0) + 1
    if len(colours) > 15:
        # keep the most frequent 15 (the icons are flat-coloured art)
        top = sorted(colours.items(), key=lambda kv: -kv[1])[:15]
        keep = {c for c, _ in top}
        for c, cnt in list(colours.items()):
            if c in keep:
                continue
            best = min(keep, key=lambda k: sum((a - b) ** 2 for a, b in zip(k, c)))
            colours[best] = colours.get(best, 0) + cnt
            del colours[c]
    pal = [(0, 0, 0)] + [c for c in sorted(colours)][:15]
    while len(pal) < 16:
        pal.append((0, 0, 0))

    def slot(c):
        if c in colours:
            return pal.index(c) if c in pal else 1
        best, bd = 1, 1 << 30
        for idx in range(1, 16):
            p = pal[idx]
            d = sum((a - b) ** 2 for a, b in zip(p, c))
            if d < bd:
                bd, best = d, idx
        return best

    for i in range(n):
        out = Image.new("P", (ICON * 2, ICON))
        out.putpalette([v for c in pal for v in c])
        data = [0] * (ICON * 2 * ICON)
        for f in range(2):                     # 0 = disabled, 1 = enabled
            for y in range(ICON):
                for x in range(ICON):
                    r, g, b, a = px[i * ICON + x, f * ICON + y]
                    if a < 128 or is_key((r, g, b)):
                        continue
                    data[y * ICON * 2 + f * ICON + x] = slot(
                        (md_round(r), md_round(g), md_round(b)))
        out.putdata(data)
        if i in EMIT:
            out.save(os.path.join(RES, f"sicon_{i:02d}.png"))
    print(f"wrote {len(EMIT)} res/sicon_NN.png (16x16 x2 frames each)")
    print("\n--- paste into res/resources.res ---")
    print(f'PALETTE sicon_pal "sicon_00.png"')
    for i in EMIT:
        print(f'SPRITE sicon_{i:02d}_sprite "sicon_{i:02d}.png" 16p 16p FAST 0')
    return 0


if __name__ == "__main__":
    sys.exit(main())
