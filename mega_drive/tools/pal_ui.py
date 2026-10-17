#!/usr/bin/env python3
"""The single source of truth for MD palette line PAL0.

PAL0 carries the font white (index 15) plus the "gameplay UI" colors used by
enemies, objects, player shots, enemy shots and the HUD balls. Both
convert_enemies.py (which emits palette_ui_enemies) and convert_assets.py
(HUD balls) must quantize against THIS table — they used two different
palettes before, so the ball art indexed entry 5 expecting red and got the
white that the enemy pool put there (the boss balls also read wrong).

index 0  = transparent/backdrop (sprite transparency lives here, so the
           art's own BLACK must have a real entry below)
1..14    = colors (tuned enemy pool)
15       = font white (CRAM 0x0EEE) — VDP_drawText relies on it

The black entry at 12 is load-bearing: every enemy/object/ball outline is
black in the PC art. While the table had no black in 1..14 those pixels were
quantized to the nearest dark color (73,0,0), which is what made the sprites
look dark and muddy. (Sage (109,182,109) was the least used entry: 16 px.)
"""
MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)

# The table itself is generated from the art by tools/gen_pal_ui.py (run by
# the build before the converters): index 0 transparent/backdrop, 1 art
# black, 2 art white, 3..14 k-means mid-tones, 15 font white (0x0EEE).
from pal_ui_data import PAL_UI_RGB


def to_vdp(c):
    r = MD_LEVELS.index(c[0])
    g = MD_LEVELS.index(c[1])
    b = MD_LEVELS.index(c[2])
    # MD CRAM word: R bits 1-3, G bits 5-7, B bits 9-11 (white = 0x0EEE).
    return (r << 1) | (g << 5) | (b << 9)


def nearest_index(c, first=1, last=14):
    """Nearest palette index for an RGB triple (never 0/15 by default)."""
    best, bd = first, 1 << 30
    for i in range(first, last + 1):
        p = PAL_UI_RGB[i]
        d = (p[0] - c[0]) ** 2 + (p[1] - c[1]) ** 2 + (p[2] - c[2]) ** 2
        if d < bd:
            bd, best = d, i
    return best
