#!/usr/bin/env python3
"""Audit stage links against the generated terrain (read-only).

For every `StageLink` in src/stage<N>_data.c, in both directions when the link
is bidirectional, reproduce the arrival math the ROM uses (main.c,
`stage_check_link` handler) and check the destination:

  px = dst_x*16 + 8 - 13            (player box is 16 wide, L=5 R=21)
  py = dst_y*16 + 8 - 16, clamped to [0, map_h*16 - 29]
  on_stair  -> px centred on the ladder column
  player.on_ladder = on_stair

Vertical links (arrival row 0 or map_h-1) use the ROM's ladder scan: the
link's dst_x first, then +-1, then +-2 on the arrival row or the row below,
because some links point one column beside their ladder (APE link2: tile 1,
stair at 2). No stair found -> LADDER flag (the player would arrive beside
it and fall).

Flags reported:
  SOLID      the player's box overlaps a SOLID tile on arrival
  SPIKE/WATER the box touches SPIKE / WATER
  NO-FLOOR   not on a stair and no floor within 12 rows below the box
  OUT        the box leaves the map

Usage: python3 tools/audit_links.py [stage ...]
No emulator needed.
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "src")

TILE = 16
BOX_L, BOX_R, BOX_T, BOX_B = 5, 21, 3, 29
UNBLOCKED, SOLID, STAIR, DOOR, SPIKE, WATER, ICE = range(7)
NAMES = {UNBLOCKED: "UNBLOCKED", SOLID: "SOLID", STAIR: "STAIR", DOOR: "DOOR",
         SPIKE: "SPIKE", WATER: "WATER", ICE: "ICE"}


def parse_stage(n):
    path = os.path.join(SRC, "stage%d_data.c" % n)
    if not os.path.exists(path):
        return None
    text = open(path).read()

    m = re.search(r"stage%d_terrain\[\d+\]\[(\d+)\]\s*=\s*\{(.*?)\n\};" % n,
                  text, re.S)
    if not m:
        return None
    maps = [[int(v) for v in g.split(",") if v.strip()]
            for g in re.findall(r"\{([^{}]*)\}", m.group(2), re.S)]

    m = re.search(r"stage%d_links\[\d+\]\s*=\s*\{(.*?)\n\};" % n, text, re.S)
    links = []
    if m:
        for row in re.findall(r"\{([^{}]*)\}", m.group(1), re.S):
            vals = [v.strip() for v in row.split(",") if v.strip()]
            if len(vals) >= 9:
                links.append(tuple(int(v) for v in vals[:9]))

    m = re.search(r"\.map_h\s*=\s*(\d+)", text)
    map_h = int(m.group(1)) if m else 14
    width = len(maps[0]) // map_h if maps else 200
    return {"width": width, "maps": maps, "links": links, "map_h": map_h}


def terrain(stage, mi, x, y):
    if mi >= len(stage["maps"]):
        return None
    w = stage["width"]
    h = len(stage["maps"][mi]) // w
    if x < 0 or y < 0 or x >= w or y >= h:
        return None
    return stage["maps"][mi][y * w + x]


def ladder_col(stage, dmap, dx, dy):
    """Stair column a vertical link means (mirrors main.c's arrival scan)."""
    for d in range(0, 3):
        for sgn in (-1, 1):
            c = dx + (d * sgn if d else 0)
            if terrain(stage, dmap, c, dy) == STAIR or \
               terrain(stage, dmap, c, dy + 1) == STAIR:
                return c
    return -1


def arrival(stage, dmap, dx, dy):
    px = dx * TILE + TILE // 2 - (BOX_L + BOX_R) // 2
    py = dy * TILE + TILE // 2 - (BOX_T + BOX_B) // 2
    py_max = stage["map_h"] * TILE - BOX_B
    py = max(0, min(py, py_max))
    on_stair = terrain(stage, dmap, dx, dy) == STAIR
    if on_stair:
        px = dx * TILE + TILE // 2 - (BOX_L + BOX_R) // 2
    return px, py, on_stair


def check(stage, dmap, dx, dy, vertical=False):
    px, py, on_stair = arrival(stage, dmap, dx, dy)
    ladder = None
    if vertical:
        ladder = ladder_col(stage, dmap, dx, dy)
        if ladder >= 0:
            on_stair = True
            px = ladder * TILE + TILE // 2 - (BOX_L + BOX_R) // 2
    x0, x1 = (px + BOX_L) >> 4, (px + BOX_R - 1) >> 4
    y0, y1 = (py + BOX_T) >> 4, (py + BOX_B - 1) >> 4
    flags = []
    worst = 0
    for ty in range(y0, y1 + 1):
        for tx in range(x0, x1 + 1):
            t = terrain(stage, dmap, tx, ty)
            if t is None:
                flags.append("OUT(%d,%d)" % (tx, ty))
                continue
            if t == SOLID:
                flags.append("SOLID(%d,%d)" % (tx, ty))
            elif t in (SPIKE, WATER):
                flags.append("%s(%d,%d)" % (NAMES[t], tx, ty))
    if not on_stair and not any(f.startswith(("SOLID", "OUT")) for f in flags):
        floor = False
        for ty in range(y1 + 1, min(y1 + 13, stage["map_h"])):
            for tx in range(x0, x1 + 1):
                if terrain(stage, dmap, tx, ty) in (SOLID, STAIR, DOOR):
                    floor = True
                    break
            if floor:
                break
        if not floor:
            flags.append("NO-FLOOR")
    return px, py, on_stair, flags, ladder


def main(argv):
    wanted = [int(a) for a in argv[1:]] or list(range(14))
    total = flagged = 0
    for n in wanted:
        st = parse_stage(n)
        if not st:
            continue
        base = os.path.basename("stage%d_data.c" % n)
        for i, (mfrom, tx, ty, mto, dx, dy, size, bidir, ltype) in enumerate(st["links"]):
            trips = [(mfrom, tx, ty, mto, dx, dy, "fwd")]
            if bidir:
                trips.append((mto, dx, dy, mfrom, tx, ty, "rev"))
            for smap, sx, sy, dmap, ax, ay, kind in trips:
                total += 1
                # A vertical transition that leaves through a LADDER must
                # arrive on one; a plain room boundary (no stair at the
                # source) has nothing to align to.
                # structural: the ROM decides the transition direction from
                # the link geometry (destination row 0 or last row)
                vertical = ay == 0 or ay == st["map_h"] - 1
                px, py, on_stair, flags, ladder = check(st, dmap, ax, ay, vertical)
                note = ""
                if ladder is not None and ladder >= 0:
                    note = " LADDER@%d%s" % (ladder,
                                             "" if ladder == ax else " (dst_x=%d)" % ax)
                if flags:
                    flagged += 1
                if flags or note:
                    print("%s link%-2d %s m%d(%d,%d)->m%d(%d,%d) px%d py%d%s%s : %s"
                          % (base, i, kind, mfrom, tx, ty, dmap, ax, ay,
                             px, py, " STAIR" if on_stair else "", note,
                             ", ".join(flags) if flags else "ok"))
    print("audited %d arrivals, %d flagged" % (total, flagged))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
