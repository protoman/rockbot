#!/usr/bin/env python3
"""Offline model of the vertical-link arrival ladder scan in src/main.c.

The ROM looks for a stair within +-2 columns of the real arrival column
(dst_x + the player's offset inside the trigger band), then of dst_x. A ladder
that does not belong to the link (CASTLE 4 link 6: dst col 108, size 2, the
ladder at col 111 is link 10's mouth) must not capture the player.

usage (from mega_drive/): python3 tools/check_arrival_ladders.py [cur|fix]
Prints every (stage, link, band offset) whose snapped ladder column lies outside
[dst_x - 2, dst_x + max(size, 1)]; exits 1 when there are any.
"""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
import audit_links as A

def scan(st, dmap, row, bases, lo, hi, fix):
    for base in bases:
        for d in range(0, 3):
            for sgn in (-1, 1):
                c = base + (d * sgn if d else 0)
                if c < 0 or c >= st["width"]:
                    continue
                if fix and not (lo <= c <= hi):
                    continue
                if A.terrain(st, dmap, c, row) == A.STAIR or \
                   A.terrain(st, dmap, c, row + 1) == A.STAIR:
                    return c
    return -1

fix = len(sys.argv) > 1 and sys.argv[1] == 'fix'
bad = 0
for n in range(0, 14):
    st = A.parse_stage(n)
    if not st:
        continue
    for i, (mfrom, tx, ty, mto, dx, dy, size, bidir, ltype) in enumerate(st["links"]):
        trips = [(mfrom, tx, ty, mto, dx, dy, size, "fwd")]
        if bidir:
            trips.append((mto, dx, dy, mfrom, tx, ty, size, "rev"))
        for smap, sx, sy, dmap, ax, ay, sz, kind in trips:
            if not (ay == 0 or ay == st["map_h"] - 1):
                continue                      # not a vertical transition
            lo, hi = ax - 2, ax + max(sz, 1)
            for off in range(0, max(sz, 1) * A.TILE, 8):
                arr = min(ax + off // A.TILE, st["width"] - 1)
                c = scan(st, dmap, ay, [arr, ax], lo, hi, fix)
                if c >= 0 and not (lo <= c <= hi):
                    print("stage%d link%-2d %s m%d(%d,%d)->m%d(%d,%d) size %d offset %d: snaps to col %d"
                          % (n, i, kind, smap, sx, sy, dmap, ax, ay, sz, off, c))
                    bad += 1
                    break
print("%d bad arrivals (%s)" % (bad, "fix" if fix else "cur"))
sys.exit(1 if bad else 0)
