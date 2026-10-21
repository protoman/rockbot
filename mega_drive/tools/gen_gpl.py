#!/usr/bin/env python3
"""Write the runtime palettes as GIMP palettes (.gpl) for the art.

The five palettes the game actually loads, in the MD 3:3:3 values:

  res/md_pal0.gpl  PAL0  enemies + objects + projectiles + HUD balls
                         (tools/pal_ui_data.py — THE table to convert art to)
  res/md_pal1.gpl  PAL1  stage tileset (current stage-0 map art)
  res/md_pal2.gpl  PAL2  player 1 (each player now has its own palette)
  res/md_pal3.gpl  PAL3  parallax background (current stage-0 bg art)

Convert images in GIMP with Image > Mode > Indexed, "Use custom palette",
and pick the matching .gpl. Index 0 is the transparent/backdrop slot; for
PAL0 the entries are: 0 transparent, 1 art black, 2 art white, 3..14 the
art colours, 15 font white (leave that one for text).

Usage: python3 tools/gen_gpl.py
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
RES = os.path.join(ROOT, "res")

sys.path.insert(0, HERE)
from pal_ui_data import PAL_UI_RGB
from PIL import Image


def png_palette(path, n=16):
    im = Image.open(path)
    pal = im.palette.getdata()[1]
    return [tuple(pal[i*3:i*3+3]) for i in range(n)]


def write_gpl(name, title, colors):
    path = os.path.join(RES, name)
    with open(path, "w") as f:
        f.write("GIMP Palette\n")
        f.write(f"Name: {title}\n")
        f.write("Columns: 4\n#\n")
        for i, (r, g, b) in enumerate(colors):
            tag = ""
            if i == 0:
                tag = "  transparent/backdrop"
            elif name == "md_pal0.gpl":
                tag = {1: "  art black", 2: "  art white",
                       15: "  font white (text only)"}.get(i, "")
            f.write(f"{r:3d} {g:3d} {b:3d}\t{i}{tag}\n")
    print(f"wrote res/{name}")


def write_palette_png():
    """16x1 indexed PNG holding the PAL0 table: rescomp turns it into a
    PALETTE resource, the same mechanism the working PAL1/PAL3 lines use."""
    im = Image.new("P", (16, 1))
    flat = [v for c in PAL_UI_RGB for v in c]
    im.putpalette(flat + [0] * (768 - len(flat)))
    im.putdata(list(range(16)))
    im.save(os.path.join(RES, "pal_ui.png"))
    print("wrote res/pal_ui.png")


def main():
    write_gpl("md_pal0.gpl", "MD PAL0 enemies/objects/shots/HUD", PAL_UI_RGB)
    write_gpl("md_title.gpl", "MD title/intro screen (PAL_TITLE)",
              png_palette("res/title_tiles.png"))
    write_gpl("md_pal1.gpl", "MD PAL1 stage tileset", png_palette("res/stage0_map0.png"))
    # PAL2: the player sheet's weapon-color entries live at 3 and 6; indices
    # 14/15 are the p1..p4 palette-swap placeholders (green/magenta) the hand
    # art keeps, so painters pick this palette and leave 14/15 alone.
    pal2 = png_palette("res/player_p1.png")
    pal2[13] = (55, 255, 0)       # COLORKEY1
    pal2[14] = (255, 0, 255)      # COLORKEY2
    pal2[15] = (0, 255, 255)      # COLORKEY3
    write_gpl("md_pal2.gpl", "MD PAL2 player1 (13/14/15 = swap placeholders)", pal2)
    write_gpl("md_bg0.gpl", "MD PAL3 bg (map 0, city)", png_palette("res/stage0_bg0.png"))
    write_gpl("md_bg1.gpl", "MD PAL3 bg (map 1, stars)", png_palette("res/stage0_bg1.png"))
    write_palette_png()
    return 0


if __name__ == "__main__":
    sys.exit(main())
