#!/usr/bin/env python3
"""Build the classic vertical HUD bars from art/energy_bar_vertical.png.

The source art is 8x51: black gaps plus purple/blue columns.  It is emitted as
three 8-pixel-tall slices (24+24+8) for the sprite engine; each slice has 51
horizontal frames, one per 1% step (0..50 px filled).

Palette lines (a MD sprite only has 4, so the HUD borrows the player's line):
  HP bar    PAL2 2/3/4 = the SELECTED player's weapon-0 colour 1/2 + black.
            Those three entries are reserved in the player art by
            tools/convert_player.py (its pixels are remapped away), because the
            bar must show the player's own colours ("the HUD should use his
            colours") while the sprite itself keeps 8/9/12 (BETABOT's whole
            black body is index 8 - writing the bar there turned him red).
  weapon    PAL2: 13/14 = the selected weapon's colours (the player sheet's
            own swap placeholders) and index 0 = transparent, so the empty
            part shows the black backdrop sprite hud.c places behind it.
            Index 12 was the old black, which CANDYBOT/KITTYBOT use in their
            art (cream/light grey gaps).
  boss      PAL0: black/white/light blue, unchanged.
"""
import os
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ART = os.path.join(ROOT, "art", "energy_bar_vertical.png")
RES = os.path.join(ROOT, "res")

VARIANTS = {
    "hud_bar_hp":  {(139, 0, 115): 2,  (0, 113, 238): 3,  (0, 0, 0): 4},
    "hud_bar_wpn": {(139, 0, 115): 13, (0, 113, 238): 14, (0, 0, 0): 0},
    "hud_bar_boss": {(139, 0, 115): 2, (0, 113, 238): 3,  (0, 0, 0): 1},
}
# Palette written into the emitted PNGs so they preview correctly. The runtime
# colours come from the palette lines named above (hud.c sets the sprite
# attribute); these are just the same values in RGB.
# Preview palette written into the emitted PNGs (the runtime colours come from
# the sprite attribute's palette line and, for PAL2 2/3/4 and 13/14, from
# player.c). The HP and boss bars share indices 2/3 on different lines, so the
# boss sheet previews with the HP colours - cosmetic only.
PAL_COLOURS = {
    1: (0, 0, 0),            # black
    2: (139, 0, 115),        # PAL2 HP colour 1 (player.c writes the real one)
    3: (0, 113, 238),        # PAL2 HP colour 2
    4: (0, 0, 0),            # PAL2 HP bar black
    13: (139, 0, 115),       # PAL2 weapon colour 1
    14: (0, 113, 238),       # PAL2 weapon colour 2
}
# (name suffix, source start row, height)
# The source art is 51 rows (0..50). The bottom slice must overlap the middle
# one instead of running past row 50: the old ("bot", 48, 8) read rows 48..55,
# and rows 51..55 do not exist, so 5 rows of the bar were permanently black
# (the black strip under the HP bar). Rows 43..47 are drawn twice with
# identical content, which is invisible.
PARTS = (("top", 0, 24), ("mid", 24, 24), ("bot", 43, 8))
FRAMES = 51


def main():
    src = Image.open(ART).convert("RGB")
    if src.size != (8, 51):
        raise SystemExit("energy_bar_vertical: expected 8x51, got %s" % (src.size,))
    for name, mapping in VARIANTS.items():
        for suffix, start, height in PARTS:
            out = Image.new("P", (8 * FRAMES, height))
            pal = [0] * 48
            for idx, c in PAL_COLOURS.items():
                pal[idx * 3:idx * 3 + 3] = list(c)
            out.putpalette(pal)
            for f in range(FRAMES):
                for y in range(height):
                    sy = start + y
                    for x in range(8):
                        if sy >= 51 or sy < (51 - f):
                            idx = mapping[(0, 0, 0)]
                        else:
                            idx = mapping.get(src.getpixel((x, sy)), mapping[(0, 0, 0)])
                        out.putpixel((f * 8 + x, y), idx)
            path = os.path.join(RES, "%s_%s.png" % (name, suffix))
            out.save(path)
            print("wrote", path)


if __name__ == "__main__":
    main()
