#!/usr/bin/env python3
"""Offline guards for the palette/art pairings that caused sprite recolours.

Run after `make assets`. Every check is an assert; the script prints one line
per group and exits non-zero on a regression. It catches the three bugs that
came from palette lines fighting over the same indices:

  1. HUD bars sharing PAL2 indices with the player art.  The HP bar used to
     write PAL2 8/9/12 with the weapon colours, and BETABOT's whole black body
     is index 8 - the player turned red.  Now the HP bar lives on PAL0 and the
     weapon bar only touches 13/14 (the sheet's swap placeholders) + index 0.
  2. The checkpoint's two frames quantizing to the same colours, so the
     activation animation was invisible (only 6 pixels differed).
  3. The stage-select name text borrowing PAL0, whose entry 0 was left tinted
     by the previous screen: the font paints its cell background with index 0,
     so the names rendered peach-on-peach.

Usage: /home/iuri/python3/bin/python3 tools/check_palettes.py
"""
import os
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
RES = os.path.join(ROOT, "res")
SRC = os.path.join(ROOT, "src")

# Indices the HUD weapon bar is allowed to use on PAL2: 13/14 are the sheet's
# weapon-colour placeholders, 0 is the sprite-transparent "empty" cell.
WPN_ALLOWED = {0, 13, 14}


def used(path, crop=None):
    im = Image.open(path)
    if im.mode != "P":
        im = im.convert("P")
    if crop:
        im = im.crop(crop)
    return {i for i in im.getdata() if i}


def main():
    bad = 0

    # 1. HUD bars vs player sheets
    #    The HP bar must be drawn on PAL0 (its art indices are PAL0 entries,
    #    which overlap PAL2 names by number and mean nothing) and the player
    #    palette builder must not write the HUD's old PAL2 slots.
    hud = open(os.path.join(SRC, "hud.c")).read()
    if "hp[i], HUD_BAR_HP_X, HUD_BAR_Y + part_y[i], PAL0" not in hud:
        print("!! the HP bar is not drawn with PAL0")
        bad += 1
    pl = open(os.path.join(SRC, "player.c")).read()
    for idx in ("base_pal[8]", "base_pal[9]", "base_pal[12]"):
        if idx in pl:
            print("!! player.c still writes %s (HUD bar colour over player art)"
                  % idx)
            bad += 1
    hp = set()
    for part in ("top", "mid", "bot"):
        hp |= used(os.path.join(RES, "hud_bar_hp_%s.png" % part))
    print("hud hp bar art indices (PAL0) %s" % sorted(hp))
    wpn = set()
    for part in ("top", "mid", "bot"):
        wpn |= used(os.path.join(RES, "hud_bar_wpn_%s.png" % part))
    print("hud weapon bar indices %s (allowed %s)"
          % (sorted(wpn), sorted(WPN_ALLOWED)))
    if not wpn <= WPN_ALLOWED:
        print("!! weapon bar uses %s outside %s"
              % (sorted(wpn - WPN_ALLOWED), sorted(WPN_ALLOWED)))
        bad += 1

    # 2. checkpoint frames must be visibly different
    im = Image.open(os.path.join(RES, "object_checkpoint.png"))
    a = im.crop((0, 0, 32, 64))
    b = im.crop((32, 0, 32 * 2, 64))
    diff = sum(1 for pa, pb in zip(a.getdata(), b.getdata()) if pa != pb)
    print("checkpoint frames differ in %d pixels" % diff)
    if diff < 100:
        print("!! checkpoint frames are nearly identical: no visible animation")
        bad += 1

    # 3. stage-select name palette: index 0 black (cell background), 15 light
    src = open(os.path.join(SRC, "ssel_data.c")).read()
    start = src.index("ssel_pal_gray[16]")
    body = src[start:src.index("}", start)]
    pal = [int(v, 16) for v in __import__("re").findall(r"0x([0-9A-Fa-f]+)", body)]
    print("stage-select name palette: 0=0x%03X 15=0x%03X" % (pal[0], pal[15]))
    if pal[0] != 0 or pal[15] == 0:
        print("!! names would draw with a tinted background or invisible ink")
        bad += 1

    if bad:
        print("%d palette check(s) FAILED" % bad)
        return 1
    print("palette checks OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
