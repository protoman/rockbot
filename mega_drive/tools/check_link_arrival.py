#!/usr/bin/env python3
"""Offline model of the ladder snap on a room change (src/main.c, stair_col).

A link band can be wide (size up to 18 tiles). The port lands the player at the
destination X plus his offset inside the band, so the arrival column is
`dst_x + band_off/16`, not `dst_x`. The ladder scan used to look only around
`dst_x` (+-2): on a wide link the player arrived ON a ladder tile (APE/DEEP
JUNGLE map 0 col 98 and map 1 col 17, measured with MD_LINK_DEBUG: stair -1,
on_stair 0, he fell off the ladder) but was not put into the hanging state.

For every link and every offset inside its band, this checks that when the arrival
cell is a stair, the scan reports a stair column. `--old` runs the old rule
(reproduces the bug); the default is the fixed rule. Exit 1 if any case fails.
"""
import re
import sys

ROOT = __import__("os").path.dirname(__import__("os").path.dirname(
    __import__("os").path.abspath(__file__)))
STAIR = 2
OLD = "--old" in sys.argv


def terrain(stage):
    t = open(f"{ROOT}/src/stage{stage}_data.c").read()
    m = re.search(rf"stage{stage}_terrain\[\d+\]\[2800\] = \{{(.*?)\n\}};", t, re.S)
    n = [int(x) for x in re.findall(r"\b\d+\b", m.group(1))]
    return [n[i * 2800:(i + 1) * 2800] for i in range(len(n) // 2800)]


def links(stage):
    t = open(f"{ROOT}/src/stage{stage}_data.c").read()
    m = re.search(rf"stage{stage}_links\[\d+\] = \{{(.*?)\}};", t, re.S)
    out = []
    for line in (m.group(1).splitlines() if m else []):
        v = [int(x) for x in re.findall(r"-?\d+", line.split("//")[0])]
        if len(v) >= 9:
            out.append(v[:9])
    return out


def main():
    bad = cases = 0
    for s in range(14):
        T = terrain(s)
        at = lambda mp, c, r: (1 if c < 0 or c >= 200 else
                               0 if r < 0 or r >= 14 else T[mp][r * 200 + c])
        for (mf, tx, ty, mt, dx, dy, size, bidir, typ) in links(s):
            dirs = [(mf, tx, ty, mt, dx, dy)]
            if bidir:
                dirs.append((mt, dx, dy, mf, tx, ty))
            for (a, ox, oy, b, ddx, ddy) in dirs:
                row0 = ddy
                for off in range(0, max(size, 1) * 16, 4):
                    arr = ddx + off // 16
                    if arr >= 200:
                        continue
                    on_ladder_cell = (at(b, arr, row0) == STAIR
                                      or at(b, arr, row0 + 1) == STAIR)
                    if not on_ladder_cell:
                        continue
                    cases += 1
                    bases = [ddx] if OLD else [arr, ddx]
                    found = -1
                    for base in bases:
                        for d in range(3):
                            for sg in (-1, 1):
                                c = base + (d * sg if d else 0)
                                if 0 <= c < 200 and (at(b, c, row0) == STAIR
                                                     or at(b, c, row0 + 1) == STAIR):
                                    found = c
                                    break
                            if found >= 0:
                                break
                        if found >= 0:
                            break
                    if found < 0:
                        bad += 1
                        if bad <= 8:
                            print(f"FAIL stage{s} map{a}->{b}: arrives at col {arr} "
                                  f"(dst_x {ddx} + band offset {off}px) on a ladder, "
                                  f"scan found none")
    print(f"{cases} ladder arrivals, {bad} not snapped ({'old' if OLD else 'new'} rule)")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
