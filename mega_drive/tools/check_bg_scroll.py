#!/usr/bin/env python3
"""Background auto-scroll modes used by the PC data vs the modes the port runs.

PC `st_map_background.auto_scroll` (`E_BG_SCROLL_MODE`, defines.h:1332) makes a
layer scroll on its own, independent of the camera: 1 UP, 2 DOWN, 3 LEFT,
4 RIGHT, 5 DIAGONAL, 6 ANIM_BG. `classmap.cpp:592` also stops the camera-driven
scrolling while a mode is set.

The MD port bakes backgrounds[1] into the bg0 plane, so when bg0 is static the
baked layer inherits backgrounds[1]'s mode (tools/convert_stage.py prints the
substitution). This checks both games' maps against the modes src/stage.c
implements, so a re-export cannot silently lose a scrolling backdrop.

Usage: tools/check_bg_scroll.py [--game RockDroid1]
"""
import argparse
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MAP_SZ = 15078
BG_SZ = 36
BG_SPEED = 32          # s8 speed
BG_AUTO = 33           # u8 auto_scroll

MODE = {0: "NONE", 1: "UP", 2: "DOWN", 3: "LEFT", 4: "RIGHT",
        5: "DIAGONAL", 6: "ANIM_BG"}


def handled_modes():
    src = open(os.path.join(ROOT, "src/stage.c")).read()
    ok = {0}
    for value, name in ((1, "1"), (2, "2"), (3, "3"), (4, "4"), (5, "5")):
        # bg_auto_hscroll / bg_auto_vscroll list the modes they serve
        if re.search(r"auto_scroll\s*!=\s*" + name, src):
            ok.add(value)
    return ok


def read_map(game, index):
    for base in (os.path.join(ROOT, "data/games", game),
                 os.path.join(ROOT, "..", "build/games", game)):
        path = os.path.join(base, "maps_v2.dat")
        if os.path.exists(path):
            break
    else:
        return None
    with open(path, "rb") as f:
        f.seek(index * MAP_SZ)
        raw = f.read(MAP_SZ)
    out = []
    for bg in (0, 1):
        o = bg * BG_SZ
        speed = struct.unpack("<b", raw[o + BG_SPEED:o + BG_SPEED + 1])[0]
        auto = raw[o + BG_AUTO]
        name = raw[o:o + 30].split(b"\0")[0].decode("latin1")
        out.append((name, speed, auto))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--game", default=None)
    args = ap.parse_args()
    games = [args.game] if args.game else ["RockDroid1", "RockDroid2"]
    ok = handled_modes()
    bad = 0
    for game in games:
        maps = read_map(game, 0)
        if maps is None:
            print(f"{game}: no maps_v2.dat")
            continue
        print(f"{game}: port implements modes "
              f"{sorted(MODE[m] for m in ok)}")
        for i in range(60):
            m = read_map(game, i)
            if not m:
                continue
            for bg, (name, speed, auto) in enumerate(m):
                if auto == 0:
                    continue
                if auto > 6:
                    print(f"  map {i} bg{bg}: auto_scroll byte {auto} "
                          "(unset/garbage) -> treated as static")
                    continue
                stage, room = divmod(i, 3)
                state = "ok" if auto in ok else "MISSING"
                if auto not in ok:
                    bad += 1
                print(f"  {state:7s} stage {stage} map {room} bg{bg}: "
                      f"{MODE[auto]} speed {speed} ({name or 'no art'})")
    if bad:
        print(f"\ncheck_bg_scroll: {bad} background mode(s) not implemented")
        return 1
    print("\ncheck_bg_scroll: OK (every auto-scroll mode in the data is handled)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
