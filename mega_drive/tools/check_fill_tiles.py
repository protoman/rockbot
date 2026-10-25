#!/usr/bin/env python3
"""Verify the hand-written 8x8 fill tiles really are the fill they claim.

A tile literal is easy to get wrong: Mega Drive 4bpp stores one byte per
bitplane inside every row, so "all bytes = 0x11" is NOT "solid colour 1" - it
sets bit 0 of all four planes on pixels 3 and 7 only, i.e. a STRIPE pattern of
pixel index 0xF (the lightest entry of any palette = white in PAL0).

That is exactly BUG-044: src/scene.c `black_tile` and src/config.c `cfg_black`
were both declared "filled with palette index 1" as 0x11111111, so the cutscene
background and the options screen showed white stripes, and because config.c
loads that tile at SPR_CFG_BLACK_TILE - the index src/stage.c returns from
stage_sky_tile() - the stage's parallax sky and the room-transition band fills
turned into the same stripes after a visit to the options screen.

A fill tile must decode to a single value on every pixel (0 = transparent,
1..15 = a solid colour). Any other pattern is a bug.

Usage: tools/check_fill_tiles.py        (exit 1 on an accidental pattern)
"""
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

ARRAY = re.compile(
    r"static\s+const\s+u32\s+(\w+)\s*\[\s*8\s*\]\s*=\s*\{([^}]*)\}", re.S)


def decode(words):
    """8 longwords (big-endian) -> 8 rows of 8 pixel indices."""
    rows = []
    for w in words:
        planes = [(w >> 24) & 0xFF, (w >> 16) & 0xFF, (w >> 8) & 0xFF, w & 0xFF]
        row = []
        for x in range(8):
            v = 0
            for p in range(4):
                if planes[p] & (0x80 >> x):
                    v |= 1 << p
            row.append(v)
        rows.append(row)
    return rows


def describe(rows):
    flat = [v for r in rows for v in r]
    if len(set(flat)) == 1:
        return None
    return rows


def main():
    bad = 0
    for path in sorted(glob.glob(os.path.join(ROOT, "src", "*.c"))):
        if path.endswith("_tiles.c"):        # generated art, not a fill
            continue
        text = open(path, encoding="utf-8", errors="replace").read()
        for name, body in ARRAY.findall(text):
            words = [int(v, 0) for v in re.findall(r"0x[0-9A-Fa-f]+|\d+", body)]
            if len(words) != 8:
                print(f"{os.path.basename(path)}: {name}: expected 8 words, "
                      f"got {len(words)}")
                bad += 1
                continue
            rows = decode(words)
            if describe(rows) is None:
                continue
            print(f"{os.path.basename(path)}: {name} is not a solid fill")
            for r in rows:
                print("    " + " ".join(f"{v:X}" for v in r))
            print("    ^ index 0 is transparent, 1..15 are palette entries;"
                  " a fill tile must be one value everywhere")
            bad += 1
    if bad:
        print(f"\n{bad} fill tile(s) are accidental patterns (BUG-044)")
        return 1
    print("fill tiles: all solid")
    return 0


if __name__ == "__main__":
    sys.exit(main())
