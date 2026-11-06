#!/usr/bin/env python3
"""HUD gauges must show at least one segment while alive (offline check).

Report 2026-10-24: at 1 HP the HP bar drew no fill at all. The bar art has
HUD_BAR_FRAMES fill steps (frame 0 = empty, frame HUD_BAR_FRAMES-1 = full), so
a low percentage lands on a frame that is a fraction of the first segment and
reads as empty.

This mirrors `set_bar()` from src/hud.c and asserts:
    hp == 0  -> frame 0                      (dead: empty gauge)
    hp >= 1  -> frame >= HUD_BAR_MIN_FRAME   (one visible segment)

Read-only. Usage: tools/check_hud_bars.py
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
SRC = os.path.join(ROOT, "src")


def define(path, name):
    text = open(os.path.join(SRC, path)).read()
    m = re.search(r"#define\s+%s\s+(-?\d+)" % name, text)
    if not m:
        sys.exit(f"{path}: #define {name} not found")
    return int(m.group(1))


def bar_frame(hp, hp_max, frames, min_frame):
    pct = (100 * hp) // hp_max
    frame = (pct * (frames - 1)) // 100
    if frame < 0:
        frame = 0
    if frame > frames - 1:
        frame = frames - 1
    if 0 < frame < min_frame:
        frame = min_frame
    return frame


def main():
    frames = define("hud.c", "HUD_BAR_FRAMES")
    min_frame = define("hud.c", "HUD_BAR_MIN_FRAME")
    hp_max = define("player.h", "PLAYER_INITIAL_HP")
    boss_max = define("enemy.h", "BOSS_INITIAL_HP")

    bad = 0
    for label, maximum in (("HP", hp_max), ("boss", boss_max)):
        for value in range(0, maximum + 1):
            f = bar_frame(value, maximum, frames, min_frame)
            if value == 0 and f != 0:
                print(f"{label} 0 -> frame {f}, expected 0")
                bad += 1
            if value > 0 and f < min_frame:
                print(f"{label} {value}/{maximum} -> frame {f} "
                      f"(< {min_frame}: reads as empty)")
                bad += 1
    if bad:
        print(f"check_hud_bars: {bad} gauge value(s) render empty while alive")
        return 1
    print(f"check_hud_bars: OK (frames {frames}, min {min_frame}, "
          f"HP<={hp_max}, boss<={boss_max})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
