#!/usr/bin/env python3
"""Offline check: the baked parallax strips must repeat over the BG_B plane.

BUG it reproduces (2026-10-30, `clouds_misalign.png`): the clouds jumped at the
seam. The PC background art repeats at its own width - its left and right
columns are identical (`clouds.png` 320x240: column 0 == column 319 on every
row), so the PC draws it side by side and it looks continuous. BG_B however
wraps at its plane width, 512 px, and 512 is NOT a multiple of 320: the strip
was baked as art + the first 192 columns, so the wrap joined art column 191 to
art column 0 - a visible break every 512 px of scroll.

The strips therefore have to be periodic over the plane width: the converter
resamples the art to a period that divides 512 (320 -> 256, 241 -> 256, 64 stays
64). This check reads the baked `res/stage*_bg<N>.png` and accepts a strip when

  * it is exactly periodic with a period that divides the plane width (the wrap
    then always lands on the next repeat, so there is nothing to line up), or
  * it has no shorter period but its last and first columns are the same
    (the PC art's own duplicated edge column), which is the PC's wrap.

Usage: tools/check_bg_seam.py     exit 1 on a seam.
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
PERIODS = (32, 64, 128, 256)        # divisors of the plane width, smallest first


def columns(img):
    w, h = img.size
    px = img.load()
    return [tuple(px[x, y] for y in range(h)) for x in range(w)]


def used_backgrounds():
    """(stage, map) pairs whose stage data actually loads a parallax strip.

    The converter only writes the maps that have a background, so a stale
    `res/stageN_bgM.png` can linger after a data edit - it is not part of the
    stage any more and must not fail this check."""
    out = set()
    for path in glob.glob(os.path.join(ROOT, "src", "stage*_data.c")):
        if not os.path.basename(path).startswith("stage") or "_bg" in path:
            continue
        text = open(path, encoding="utf-8", errors="replace").read()
        m = re.search(r"_bg_pal\[\d+\]\s*=\s*\{([^}]*)\}", text)
        if not m:
            continue
        for i, entry in enumerate(s.split(",") for s in m.group(1).split(",")):
            if "NULL" in entry[0]:
                continue
            stage = os.path.basename(path)[len("stage"):-len("_data.c")]
            out.add((stage, i))
    return out


def ending_layers():
    """The ending's walking layers scroll over the same 512 px planes (their
    offsets wrap at 512); they are baked by tools/ending_layers.py."""
    return sorted(glob.glob(os.path.join(RES, "end_l[0-9]_src.png")))


def main():
    used = used_backgrounds()
    files = sorted(glob.glob(os.path.join(RES, "stage*_bg[0-9].png"))) + ending_layers()
    if not files:
        print("check_bg_seam: no baked backgrounds (run make assets)")
        return 1

    errors = 0
    for path in files:
        name = os.path.basename(path)
        m = re.match(r"stage(\d+)_bg(\d)\.png$", name)
        if m and used and (m.group(1), int(m.group(2))) not in used:
            print("skip  %s: not referenced by the stage data" % name)
            continue
        img = Image.open(path).convert("RGBA")
        if img.size[0] != PLANE_W:
            print("FAIL  %s: width %d, the plane is %d"
                  % (name, img.size[0], PLANE_W))
            errors += 1
            continue
        cols = columns(img)
        period = next((p for p in PERIODS
                       if all(cols[x] == cols[x + p]
                              for x in range(PLANE_W - p))), None)
        if period:
            print("ok    %s: %d px period over the %d px plane"
                  % (name, period, PLANE_W))
            continue
        if cols[PLANE_W - 1] == cols[0]:
            print("ok    %s: single %d px pass, edges match"
                  % (name, PLANE_W))
            continue
        # name where it breaks: the first column that repeats the plane's start
        bad = next((x for x in range(PLANE_W // 2, PLANE_W)
                    if cols[x] == cols[0]), PLANE_W)
        print("FAIL  %s: no repeat period over the %d px plane, so the wrap "
              "joins column %d to column 0 (art restarts at column %d)"
              % (name, PLANE_W, PLANE_W - 1, bad if bad < PLANE_W else -1))
        errors += 1

    print("\n%d seam(s)" % errors)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
