#!/usr/bin/env python3
"""Make the ending's walking layers repeat over the BG planes.

The PC ending (draw::show_player_walking_ending) scrolls three layers of
320xH art at 1.1 / 0.5 / 0.1 px per frame, each repeated at its own 320 px
width. The MD planes are 512 px wide (64 tiles), so the layer strips used to be
uploaded as 40 columns and the offsets wrapped at 320: as soon as the offset
passed 192 px the 320 px window ran past the end of the plane and the grass
jumped (report 2026-10-30, "the grass jumps when it animates").

Same fix as the stage parallax: resample each layer to a 256 px period (the
largest power of two below 320 that divides the 512 px plane) and tile it to the
full 512 px, so the hardware wrap is seamless and the offsets can wrap at 512.

Usage: tools/ending_layers.py     (writes res/end_lN_src.png)
"""
import os
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
PCGAME = os.path.join(os.path.dirname(ROOT), "build/games/RockDroid1")
RES = os.path.join(ROOT, "res")
PLANE_W, PERIOD = 512, 256


def main():
    for n in (1, 2, 3):
        src = os.path.join(PCGAME, "images/ending/player_walking_layer%d.png" % n)
        if n == 2:
            src = os.path.join(RES, "end_l2_src.png")   # padded at the top first
        if not os.path.isfile(src):
            sys.exit("ending_layers: missing %s" % src)
        im = Image.open(src).convert("RGBA")
        strip = im.resize((PERIOD, im.height), Image.NEAREST)
        out = Image.new("RGBA", (PLANE_W, im.height), (0, 0, 0, 0))
        for x in range(0, PLANE_W, PERIOD):
            out.paste(strip, (x, 0))
        dst = os.path.join(RES, "end_l%d_src.png" % n)
        out.save(dst)
        print("ending layer %d: %s %s -> %s (period %d px)"
              % (n, os.path.basename(src), im.size, out.size, PERIOD))


if __name__ == "__main__":
    main()
