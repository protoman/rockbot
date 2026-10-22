#!/usr/bin/env python3
"""Count the VDP tilemap calls the animated tiles cost per frame.

Mirrors src/stage.c: stage_anim_apply_cells() used to write four
VDP_setTileMapXY() per animated cell at raw map coordinates; it now writes one
VDP_setTileMapDataRect() per contiguous run at windowed (cx & 31) * 2 plane
coordinates. Reads the generated stage data, so it is exact.

Usage: tools/anim_calls.py [stage]
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
W, H = 200, 14
PLANE_METATILES = 32      # 64-cell wide plane (SGDK default)


def read_array(path, name):
    s = open(path).read()
    i = s.index(name)
    j = s.index('};', i)
    return [int(x) for x in re.findall(r'\b\d+\b', s[i:j].split('=', 1)[1])]


def read_slots(path, name):
    s = open(path).read()
    i = s.index(name)
    j = s.index('};', i)
    return [(int(m.group(1)), [int(x) for x in m.group(2).split(',')])
            for m in re.finditer(r'\{\s*(\d+),\s*\{([^}]*)\}\s*\}', s[i:j])]


def runs(cell, mp, s, cx0, cy0):
    """Contiguous runs of slot `s` in the visible window (src/stage.c)."""
    out = []
    for cy in range(max(0, cy0), min(H, cy0 + 15)):
        row = cell[mp * W * H + cy * W:(mp + 1) * W * H][:W]
        cx = cx0
        while cx <= cx0 + 20:
            if cx < 0 or cx >= W or row[cx] != s + 1:
                cx += 1
                continue
            a = b = cx
            while b + 1 <= cx0 + 20 and b + 1 < W and row[b + 1] == s + 1:
                b += 1
            b = min(b, a | (PLANE_METATILES - 1))     # plane window wrap
            # every cell of the run must map into one plane row segment
            assert (a & (PLANE_METATILES - 1)) + (b - a) < PLANE_METATILES
            out.append((cy, a, b))
            cx = b + 1
    return out


def main():
    st = sys.argv[1] if len(sys.argv) > 1 else '3'
    p = os.path.join(ROOT, 'src', 'stage%s_data.c' % st)
    cell = read_array(p, 'stage%s_anim_cell_slot' % st)
    slots = read_slots(p, 'stage%s_anim_slots' % st)
    n = read_array(p, 'stage%s_anim_slot_n' % st)[:3]
    smax = max(n) if n else 0
    for mp in range(3):
        old = new = 0.0
        for sx in range(n[mp]):
            frames, delay = slots[mp * smax + sx]
            cells = 0
            for cam in range(0, W * 16 - 320, 128):
                cells += sum(b - a + 1 for _, a, b in
                             runs(cell, mp, sx, cam >> 4, 0))
            per_frame = cells / (len(range(0, W * 16 - 320, 128)) or 1)
            d = max(1, delay[0])
            old += per_frame * 4 / d                  # 4 setTileMapXY per cell
            new += per_frame / d * 0 + 0              # cells -> runs below
            # count runs (not cells) for the new writer
            nruns = 0.0
            for cam in range(0, W * 16 - 320, 128):
                nruns += len(runs(cell, mp, sx, cam >> 4, 0))
            new += (nruns / (len(range(0, W * 16 - 320, 128)) or 1)) / d
        print(f'stage{st} map{mp}: {n[mp]} slots, '
              f'{old:.0f} tilemap calls/frame -> {new:.0f}')


if __name__ == '__main__':
    main()
