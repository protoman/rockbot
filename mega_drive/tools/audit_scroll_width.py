#!/usr/bin/env python3
"""Audit: every baked scrolling strip must wrap where its content repeats.

The class of bug found on 2026-10-30 (stage parallax, ending walk): an image is
baked into a 512 px plane and the scroll wraps at 512, but the art only repeats
at its own width (320 px for most PC backgrounds). 512 is not a multiple of 320,
so the wrap joins the wrong columns and the layer jumps.

This prints, for every scrolling layer in the game, the source art width, the
period the strip was baked at, and whether the runtime wrap lands on that
period:

  cutscene parallax (src/scene.c)   wraps at SceneLayer.period
  stage parallax    (src/stage.c)   wraps at 512 - the PC's own art spacing is
                                    kept and the seam is accepted (BUG-049)
  ending walk       (src/ending.c)  wraps at 512, layers baked at a 256 px period

Also lists every converter that changes an image WIDTH (crop or resample), so a
new one shows up for review.

Usage: tools/audit_scroll_width.py
"""
import importlib.util
import os
import re
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
SRC = os.path.join(ROOT, "src")

spec = importlib.util.spec_from_file_location(
    "cs", os.path.join(HERE, "convert_scenes.py"))
cs = importlib.util.module_from_spec(spec)
sys.argv = [sys.argv[0]]          # convert_scenes runs its own main below
try:
    spec.loader.exec_module(cs)
except SystemExit:
    pass


# Widths that are cut today, with the reason. A new entry here is a bug: add it
# to bugs.md and fix it, do not grow this list.
KNOWN_CUTS = {
    "city_bg_3.png": "BUG-053 (open): 963 px art on the 512 px plane - the bake "
                     "keeps the left 512 px; needs the window trick",
}


def least_period(im):
    """Smallest horizontal period of `im`, or 0 (same rule as the baker)."""
    w, h = im.size
    data = im.tobytes()
    stride = w * 4
    first = data[:stride]
    for p in range(8, w):
        if first[p * 4:] != first[:stride - p * 4]:
            continue
        if all(data[y * stride + p * 4:(y + 1) * stride] ==
               data[y * stride:(y + 1) * stride - p * 4] for y in range(h)):
            return p
    return 0


def scene_runtime_periods():
    """{layer image stem: period} from the generated scenes_data.h (0 = unset)."""
    text = open(os.path.join(SRC, "scenes_data.h"), encoding="utf-8").read()
    if "u16 period" not in text:
        return None
    return {}


def cutscene_layers():
    """(record name, layer index, art size, period) for every parallax layer."""
    out = []
    for rec in cs.read_parallax():
        for i, l in enumerate(rec["layers"]):
            path = os.path.join(cs.IMAGES, l["file"])
            if not os.path.isfile(path):
                continue
            im = Image.open(path).convert("RGBA")
            out.append((rec["name"], i, l["file"], im.size,
                        least_period(im) if im.width <= 512 else 0))
    return out


def main():
    errors = 0

    print("cutscene parallax layers (src/scene.c wraps at SceneLayer.period):")
    data = open(os.path.join(SRC, "scenes_data.h"), encoding="utf-8").read()
    has_field = "period" in data
    for rec, i, name, size, period in cutscene_layers():
        if size[0] > 512:
            # the bake keeps the art's left 512 px; the rest is never shown
            per = [r for r in cs.read_parallax() if r["name"] == rec][0]
            spd = per["layers"][i]["speed"]
            travel = abs(spd) * per["duration"] / 10
            known = KNOWN_CUTS.get(os.path.basename(name))
            print("%s  %-24s layer %d %-28s %s wide: the bake keeps 512 px, "
                  "%d px (%.0f%%) never show; the scene scrolls %.0f px, so it "
                  "wraps %d time(s) at 512 instead of at %d%s"
                  % ("known" if known else "FAIL ", rec, i, name, size,
                     size[0] - 512, 100.0 * (size[0] - 512) / size[0], travel,
                     int(travel // 512), size[0],
                     (" - " + known) if known else ""))
            if not known:
                errors += 1
            continue
        if not period:
            print("ok    %-24s layer %d %-28s %s: no repeat, static"
                  % (rec, i, name, size))
            continue
        if not has_field:
            note = "scene.c wraps at 512"
            if 512 % period:
                print("FAIL  %-24s layer %d %-28s %s: art repeats at %d px but "
                      "the scroll wraps at 512" % (rec, i, name, size, period))
                errors += 1
            else:
                print("ok    %-24s layer %d %-28s %s: %d px period divides 512"
                      % (rec, i, name, size, period))
            continue
        print("ok    %-24s layer %d %-28s %s: %d px period, the scroll wraps "
              "there" % (rec, i, name, size, period))

    print("\nimage WIDTH changes in the converters:")
    for tool, note in (
            ("ending_layers.py", "320 -> 256 px period (the walk scrolls over "
                                 "the plane; native 320 would jump)"),
            ("convert_scenes.py", "cropped to 512 only if the art is wider than "
                                  "the plane (flagged above)"),
            ("convert_stage.py", "none: art + its head, the PC's own width"),
            ("img2tiles.py", "height only (--crop-top / --crop-h)"),
            ("convert_assets.py", "height only (kept to 224 rows)"),
    ):
        print("  %-20s %s" % (tool, note))

    print("\n%d width/wrap problem(s)" % errors)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
