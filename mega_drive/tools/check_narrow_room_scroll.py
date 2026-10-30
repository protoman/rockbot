#!/usr/bin/env python3
"""Offline check: a room narrower than the screen must keep the camera it had.

BUG it reproduces (2026-10-30): "Castle 5 - after the first room transition the
second room scroll is one tile to the left". CASTLE 5 (FROZEN LAIR, stage 13)
rooms are 16..18 columns = 256..288 px, narrower than the 320 px view, so no
camera can keep BOTH locks out of view. The PC keeps whatever scroll it had
(`classMap::changeScrolling` only blocks moving a locked column into the view,
it never repositions the camera), but `scroll_lock_span()` answered
`lo > hi` with `camera.x = lo` - pinning the view to the left wall - so every
arrival into such a room jumped the room left by the difference (stage 13 room
1: camera 0 -> 18 = "one tile").

The check models the arrival camera (`stage_arrival_camera_keep`) from the real
scroll-lock/links data for every stage and reports every narrow-room case where
the pin changes the camera, then asserts src/stage.c no longer pins.

Usage: tools/check_narrow_room_scroll.py     exit 1 on a pin.
"""
import glob
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
SRC = os.path.join(ROOT, "src")
MAP_W, TILE, VIEW = 200, 16, 320


def clamp(v, lo=0, hi=MAP_W * TILE - VIEW):
    return max(lo, min(hi, v))


def load(stage):
    text = open(os.path.join(SRC, "stage%d_data.c" % stage), encoding="utf-8",
                errors="replace").read()
    m = re.search(r'scroll_lock\[(\d+)\]\[(\d+)\] = \{(.*?)\n\};', text, re.S)
    if not m:
        return None, []
    counts = [int(x) for x in re.findall(
        r"\d+", re.search(r"scroll_lock_n\[\d+\] = \{([^}]*)\}", text).group(1))]
    rows = re.findall(r"\{([^}]*)\}", m.group(3))
    locks = [set(int(x) for x in re.findall(r"\d+", r)[:counts[i]])
             for i, r in enumerate(rows)]
    links = []
    lm = re.search(r"stage%d_links\[\d+\] = \{(.*?)\n\};" % stage, text, re.S)
    if lm:
        for line in lm.group(1).splitlines():
            v = [int(x) for x in re.findall(r"-?\d+", line.split("//")[0])]
            if len(v) >= 6:
                links.append(v)
    return locks, links


def port_camera(locks, focus, keep, pin):
    """stage_arrival_camera_keep(): caller camera, then scroll_lock_span()."""
    cam = clamp(focus - keep)
    fc = (focus - 8) >> 4
    lo, hi = -32000, 32000
    for c in range(fc, max(fc - 25, -1), -1):
        if c in locks:
            lo = (c << 4) + 2
            break
    for c in range(fc + 1, fc + 25):
        if c in locks:
            hi = (c << 4) - 307
            break
    if lo > hi:
        if pin:
            cam = lo
    elif cam < lo:
        cam = lo
    elif cam > hi:
        cam = hi
    return clamp(cam)


def main():
    # first the code itself: a narrow-room pin must not be there
    code = open(os.path.join(SRC, "stage.c"), encoding="utf-8").read()
    span = re.search(r"static void scroll_lock_span.*?\n\}", code, re.S)
    pins = False
    if not span:
        print("FAIL  stage.c: scroll_lock_span not found")
        return 1
    if re.search(r"if\s*\(\s*lo\s*>\s*hi\s*\)\s*camera\.x\s*=\s*lo", span.group(0)):
        pins = True
        print("FAIL  stage.c: scroll_lock_span pins a narrow room to lo")

    worst = []
    for path in sorted(glob.glob(os.path.join(SRC, "stage*_data.c"))):
        hit = re.match(r"stage(\d+)_data\.c$", os.path.basename(path))
        if not hit:                     # stage_data.c, the shared header data
            continue
        stage = int(hit.group(1))
        locks, links = load(stage)
        if not locks:
            continue
        for link in links:
            src_map, src_x, _sy, dst_map, dst_x, _dy = link[:6]
            for m, col in ((src_map, src_x), (dst_map, dst_x)):
                focus = col * TILE + TILE // 2
                for keep in range(0, VIEW, TILE):
                    pinned = port_camera(locks[m], focus, keep, True)
                    kept = port_camera(locks[m], focus, keep, False)
                    if pinned != kept:
                        worst.append((stage, m, col, keep, pinned, kept))

    if pins:
        for stage, m, col, keep, pinned, kept in worst[:8]:
            print("FAIL  stage %d map %d col %d keep %d: narrow room, camera "
                  "pinned to %d instead of %d (%.1f tiles left)"
                  % (stage, m, col, keep, pinned, kept, (kept - pinned) / 16.0))
        if len(worst) > 8:
            print("... %d more" % (len(worst) - 8))
        print("\n%d narrow-room camera shift(s)" % len(worst))
        return 1

    print("ok    scroll_lock_span leaves a narrow room's camera alone "
          "(would have shifted %d arrivals)" % len(worst))
    print("\n0 narrow-room camera shift(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
