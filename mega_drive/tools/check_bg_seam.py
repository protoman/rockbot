#!/usr/bin/env python3
"""Offline check of the baked scrolling strips (owner's decision 2026-10-30).

TWO different rules, on purpose:

* STAGE parallax (`res/stage<N>_bg<M>.png`) keeps the PC's art at its OWN
  width. The PC repeats a background at its width (its left and right columns
  match), so a 320 px art repeats every 320 px and this check requires the bake
  to do the same. BG_B wraps at 512 px, which is not a multiple of 320, so the
  wrap joins art column 191 to column 0: a visible break every 512 px of
  parallax travel. That is ACCEPTED (bugs.md BUG-049/052): the 128-tile plane
  that would remove it does not fit the VRAM/SDK layout, and resampling to
  256 px made the art 20% denser than the PC.

* ENDING walking layers (`res/end_l<N>_src.png`) must repeat over the plane
  (a period that divides 512): their offsets wrap at 512, so a native 320 px
  strip would jump every 320 px of travel - the "grass jumps" report
  (tools/ending_layers.py bakes them at 256 px).

Usage: tools/check_bg_seam.py     exit 1 when a strip breaks its rule.
"""
import glob
import os
import re
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
RES = os.path.join(ROOT, "res")
PLANE_W = 512

sys.path.insert(0, HERE)
import convert_stage as cs          # noqa: E402  (the source-of-truth reader)


def columns(img):
    w, h = img.size
    px = img.convert("RGBA").load()
    return [tuple(px[x, y] for y in range(h)) for x in range(w)], w


def period(cols, w, limit=None):
    """Smallest horizontal repeat of `cols`, or 0."""
    limit = limit or w
    for p in range(1, limit + 1):
        if all(cols[x] == cols[x + p] for x in range(w - p)):
            return p
    return 0


def source_period(name):
    art = cs.unkey(Image.open(os.path.join(
        cs.DATA, "images/map_backgrounds", name)).convert("RGBA"))
    cols, w = columns(art)
    return period(cols, w)


def main():
    errors = 0

    # ---- stage parallax: the PC's own art spacing ------------------------
    for path in sorted(glob.glob(os.path.join(RES, "stage*_bg[0-9].png"))):
        base = os.path.basename(path)
        hit = re.match(r"stage(\d+)_bg(\d)\.png$", base)
        if not hit:
            continue
        stage, m = int(hit.group(1)), int(hit.group(2))
        _, bg0, _, _ = cs.read_map(stage, m)
        name = bg0[0]
        if not name or not os.path.exists(os.path.join(
                cs.DATA, "images/map_backgrounds", name)):
            continue
        # the PC repeats the art at its own WIDTH (not at an inner period)
        want = Image.open(os.path.join(cs.DATA, "images/map_backgrounds",
                                       name)).width
        cols, w = columns(Image.open(path))
        got = period(cols, w)
        # the bake may repeat faster than the art width only when the ART
        # itself repeats there (circuit boards/mine are 16/32 px periodic); a
        # resample to 256 does not divide 320 and is caught here.
        if not got or want % got:
            print("FAIL  %s: bakes a %d px period, the PC art is %d px wide "
                  "and does not repeat there" % (base, got, want))
            errors += 1
        else:
            print("ok    %s: PC art spacing kept (%d px period)" % (base, got))

    # ---- ending walking layers: must repeat over the plane --------------
    for path in sorted(glob.glob(os.path.join(RES, "end_l[0-9]_src.png"))):
        base = os.path.basename(path)
        cols, w = columns(Image.open(path))
        got = period(cols, w, limit=256)
        if not got or PLANE_W % got:
            print("FAIL  %s: no period dividing the %d px plane - the walking "
                  "layers would jump" % (base, PLANE_W))
            errors += 1
        else:
            print("ok    %s: %d px period over the plane (no jump)" % (base, got))

    print("\n%d strip(s) breaking their rule" % errors)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
