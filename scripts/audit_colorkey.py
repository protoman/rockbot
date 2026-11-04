#!/usr/bin/env python3
"""Lists game images whose transparent colour is not exactly the colour key (75,125,125).

The game makes (75,125,125) transparent. Art exported with colour management ends up with a nearby colour
(for example 72,126,124) that is NOT transparent and shows as a coloured box. The loader repairs it only when
the top-left pixel is that colour (and not in the SDL3 build), so images whose top-left pixel is something
else stay broken.

Usage: scripts/audit_colorkey.py [folder ...]      (default: build/shared build/games/RockDroid1 build/games/RockDroid2;
       RockMix and NEW-PROJ are test beds and are skipped)
       scripts/audit_colorkey.py --all             also list images that are fine
Exit status: 1 when something needs fixing.
"""
import os
import sys
from collections import Counter

from PIL import Image

KEY = (75, 125, 125)
TOLERANCE = 12          # per channel; the loader uses 8
# colours the game uses on purpose to recolour sprites (not a transparency problem)
SPECIAL = {(55, 255, 0), (255, 0, 255), (0, 255, 255)}


def near_key(rgb):
    return max(abs(rgb[i] - KEY[i]) for i in range(3)) <= TOLERANCE and rgb != KEY


def analyse(path):
    im = Image.open(path).convert("RGBA")
    px = list(im.getdata())
    counts = Counter((r, g, b) for r, g, b, a in px)
    exact = counts.get(KEY, 0)
    near = {c: n for c, n in counts.items() if near_key(c) and c not in SPECIAL}
    corner = px[0][:3] if px else (0, 0, 0)
    has_alpha = any(a < 255 for _, _, _, a in px)
    return {
        "size": im.size, "exact": exact, "near": near, "near_total": sum(near.values()),
        "corner": corner, "alpha": has_alpha,
        # the loader repairs the image only when the corner is a near-key colour
        "loader_fixes": near_key(corner),
        "pixels": im.size[0] * im.size[1],
    }


def likely_bug(info):
    """A real transparency problem is a flat fill of ONE near-key colour covering a noticeable part of the
    image; stray similar pixels in opaque art (a few teal pixels in a background) are not."""
    if info["near_total"] < 8 or info["near_total"] < 0.01 * info["pixels"]:
        return False
    top = max(info["near"].values())
    return top >= 0.8 * info["near_total"]


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    show_all = "--all" in sys.argv
    roots = args or ["build/shared", "build/games/RockDroid1", "build/games/RockDroid2"]
    bad, ok = [], 0
    for root in roots:
        for dirpath, _, files in os.walk(root):
            for name in sorted(files):
                if not name.lower().endswith(".png"):
                    continue
                path = os.path.join(dirpath, name)
                try:
                    info = analyse(path)
                except Exception as exc:  # unreadable image
                    print("UNREADABLE %s: %s" % (path, exc))
                    continue
                if info["near_total"] == 0:
                    ok += 1
                    if show_all:
                        print("ok    %s" % path)
                    continue
                bad.append((path, info))
    def show(path, info):
        top = sorted(info["near"].items(), key=lambda kv: -kv[1])[:2]
        colours = ", ".join("%s x%d" % (c, n) for c, n in top)
        status = ("fixed at load by the SDL1/SDL2 loader only (corner is near-key); broken in the SDL3 build"
                  if info["loader_fixes"]
                  else "NOT fixed by the loader (corner pixel %s is not near-key)" % (info["corner"],))
        print("%s\n    near-key pixels: %d of %d (%s); exact key pixels: %d\n    %s" %
              (path, info["near_total"], info["pixels"], colours, info["exact"], status))

    real = sorted((p, i) for p, i in bad if likely_bug(i))
    maybe = sorted((p, i) for p, i in bad if not likely_bug(i))
    print("=== LIKELY WRONG COLOUR KEY (flat near-key fill: re-export with exactly %s) ===" % (KEY,))
    for path, info in real:
        show(path, info)
    print("\n=== PROBABLY FINE (a few similar pixels inside opaque art; check only if something looks wrong) ===")
    for path, info in maybe:
        print("%s   (%d px, %s)" % (path, info["near_total"], sorted(info["near"].items(), key=lambda kv: -kv[1])[0][0]))
    print("\n%d likely wrong, %d probably fine, %d clean" % (len(real), len(maybe), ok))
    sys.exit(1 if real else 0)


if __name__ == "__main__":
    main()
