#!/usr/bin/env python3
"""Offline model of the two screens' horizontal alignment during a room change.

Every link is evaluated with the origin camera centring the player, clamped at
the map edges (so the player's screen X is far from 160 near an edge). Two destination-camera rules are compared:

  centre: dest camera = clamp(dest_x - 160)           (the old port rule)
  keep:   dest camera = clamp(dest_x - origin_screen_x) (PC rule, game.cpp:716)

The error is |dest link screen X - origin link screen X| at the slide. `keep`
may still be off only when the destination camera itself hits a map edge.
Exit 1 when `keep` is worse than `centre` anywhere.
"""
import importlib.util
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
spec = importlib.util.spec_from_file_location("cv", os.path.join(ROOT, "tools/check_vram.py"))
cv = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cv)

TS, MAXC = 16, 200 * 16 - 320
cl = lambda v: max(0, min(MAXC, v))


def main():
    worst_c = worst_k = 0
    n = bad_c = bad_k = 0
    for s, links in cv.read_links().items():
        for (mf, tx, ty, mt, dx, dy) in links:
            # origin camera at its natural spot: centred on the player, clamped
            # by the map edges (scroll locks only add more clamping)
            for ox_screen in (tx * TS + 8 - cl(tx * TS + 8 - 160),):
                n += 1
                ec = abs((dx * TS + 8 - cl(dx * TS + 8 - 160)) - ox_screen)
                ek = abs((dx * TS + 8 - cl(dx * TS + 8 - ox_screen)) - ox_screen)
                worst_c, worst_k = max(worst_c, ec), max(worst_k, ek)
                bad_c += ec > 0
                bad_k += ek > 0
                if ek > ec:
                    print(f"WORSE stage{s} map{mf}->{mt} link {tx}->{dx} sx={ox_screen}")
                    return 1
    print(f"{n} cases: centre misaligned {bad_c} (worst {worst_c}px), "
          f"keep misaligned {bad_k} (worst {worst_k}px; only where the "
          f"destination camera is clamped by a map edge)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
