#!/usr/bin/env python3
"""Verify the hand-written 8x8 fill tiles really are the fill they claim.

Mega Drive tiles are packed 4bpp (one nibble per pixel, NOT bitplanes), so a
solid fill of index N is 0xNNNNNNNN. A bitplane-style literal such as
0xFF000000 paints index 15 on pixels 0-1 only: white stripes 2 px wide every
8 px. BUG-044 "fixed" the solid 0x11111111 into exactly that, by decoding the
tile as bitplanes; this checker now decodes nibbles.

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
    """8 longwords -> 8 rows of 8 pixel indices.

    Mega Drive tiles are PACKED 4bpp: each longword is one row, one nibble per
    pixel, leftmost pixel in the top nibble (0x11111111 = solid index 1).
    """
    return [[(w >> (28 - 4 * x)) & 0xF for x in range(8)] for w in words]


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
