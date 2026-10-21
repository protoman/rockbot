#!/usr/bin/env python3
"""Find pieces shared between decomposed screens ("piece-maps").

Screens often repeat motifs (a panel frame, a border, an icon). Decomposing
each screen already removes duplicates *inside* one image; this tool finds
the duplicates *across* images so a piece is stored once for all screens.

Input: sets produced by tools/img2tiles.py —
    res/<name>_tiles_rgba.png   tile strip (RGBA, pixel-exact comparison)
    res/<name>_map.bin          u16 tile index per 8x8 cell

Output:
    res/<out>_tiles_rgba.png    combined atlas (one copy of every piece)
    res/<out>_tiles.png         indexed atlas, shared MD palette
    res/<out>_map_<name>.h/.c   per-screen map, remapped into the atlas
    report: per screen unique/shared counts, savings, and the atlas index
            ranges each screen touches (feeds the on-the-fly ROM->VRAM load:
            a contiguous range is one VDP_loadTileSet; fragmented ranges
            cost gap tiles — the numbers tell you when to reorder/pack).

Usage:
  python3 tools/tiles_merge.py <out> <name1> <name2> [...] [--colors 15]
                               [--res res]
"""
import argparse
import hashlib
import os
import struct
import sys
from PIL import Image

TILE = 8
MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)


def md_round(v):
    return min(MD_LEVELS, key=lambda x: abs(x - v))


def load_tiles(path):
    """RGBA strip -> list of 64-tuples."""
    im = Image.open(path).convert("RGBA")
    n = im.size[0] // TILE
    px = im.load()
    tiles = []
    for t in range(n):
        tiles.append(tuple(px[t * TILE + x, y]
                           for y in range(TILE) for x in range(TILE)))
    return tiles


def tkey(t):
    return hashlib.md5(repr(t).encode()).hexdigest()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out", help="base name for the combined atlas")
    ap.add_argument("names", nargs="+", help="screen names from img2tiles.py")
    ap.add_argument("--colors", type=int, default=15)
    ap.add_argument("--res", default=None)
    args = ap.parse_args()

    here = os.path.dirname(os.path.abspath(__file__))
    res = args.res or os.path.join(os.path.dirname(here), "res")

    sets = {}
    used = []            # atlas order: first-seen order keeps it deterministic
    index = {}
    for name in args.names:
        tiles = load_tiles(os.path.join(res, f"{name}_tiles_rgba.png"))
        with open(os.path.join(res, f"{name}_map.bin"), "rb") as f:
            raw = f.read()
        tmap = list(struct.unpack(f"<{len(raw)//2}H", raw))
        sets[name] = (tiles, tmap)

        remap = []
        for ti in tiles:
            k = tkey(ti)
            gi = index.get(k)
            if gi is None:
                gi = len(used)
                index[k] = gi
                used.append(ti)
            remap.append(gi)
        sets[name] = (tiles, tmap, remap)

    # --- report -------------------------------------------------------------
    print(f"combined atlas: {len(used)} unique pieces")
    per_screen_ranges = {}
    for name in args.names:
        tiles, tmap, remap = sets[name]
        ref = sorted({remap[ti] for ti in tmap})
        shared = len(tiles) - len(set(remap))
        runs, start, prev = 0, None, None
        for v in ref:
            if start is None:
                start = prev = v
            elif v == prev + 1:
                prev = v
            else:
                runs += 1
                start = prev = v
        if start is not None:
            runs += 1
        per_screen_ranges[name] = (ref[0], ref[-1], len(ref), runs)
        print(f"  {name:16s} cells {len(tmap):5d}  own tiles {len(tiles):4d}  "
              f"into atlas (dedup) {len(set(remap)):4d}  "
              f"atlas range {ref[0]}..{ref[-1]} ({len(ref)} used, {runs} runs)")
    total_own = sum(len(sets[n][0]) for n in args.names)
    print(f"total pieces before {total_own} -> after {len(used)} "
          f"({100 - 100*len(used)//max(1, total_own)}% saved)")

    # --- shared palette + indexed atlas ------------------------------------
    colors = sorted({px for t in used for px in t if px[3] > 0})
    if len(colors) > args.colors:
        counts = {}
        for t in used:
            for px in t:
                if px[3] > 0:
                    counts[px] = counts.get(px, 0) + 1
        flat = Image.new("RGB", (sum(counts.values()), 1))
        fl = flat.load()
        cx = 0
        for px, n in counts.items():
            for _ in range(n):
                fl[cx, 0] = px[:3]
                cx += 1
        q = flat.quantize(colors=args.colors, method=Image.Quantize.MEDIANCUT)
        qpal = q.getpalette()
        palette_rgb = [(md_round(qpal[i * 3]), md_round(qpal[i * 3 + 1]),
                        md_round(qpal[i * 3 + 2])) for i in range(args.colors)]
    else:
        palette_rgb = [(0, 0, 0)] + [c[:3] for c in colors][:args.colors]
    palette = [(0, 0, 0)] + [c for c in palette_rgb if c != (0, 0, 0)][:args.colors]

    def lut(px):
        if px[3] == 0:
            return 0
        best, bd = 1, 1 << 30
        for i in range(1, len(palette)):
            c = palette[i]
            d = (c[0] - px[0])**2 + (c[1] - px[1])**2 + (c[2] - px[2])**2
            if d < bd:
                bd, best = d, i
        return best

    n = len(used)
    out = Image.new("P", (n * TILE, TILE))
    out.putpalette([v for c in palette for v in c] + [0] * (768 - len(palette) * 3))
    data = [0] * (n * TILE * TILE)
    for gi, t in enumerate(used):
        for y in range(TILE):
            row = y * (n * TILE) + gi * TILE
            for x in range(TILE):
                data[row + x] = lut(t[y * TILE + x])
    out.putdata(data)
    out.save(os.path.join(res, f"{args.out}_tiles.png"))

    rstrip = Image.new("RGBA", (len(used) * TILE, TILE))
    rp = rstrip.load()
    for gi, t in enumerate(used):
        for y in range(TILE):
            for x in range(TILE):
                rp[gi * TILE + x, y] = t[y * TILE + x]
    rstrip.save(os.path.join(res, f"{args.out}_tiles_rgba.png"))
    print(f"wrote res/{args.out}_tiles.png and _tiles_rgba.png "
          f"({len(used)} tiles, {len(palette)} palette entries)")

    # --- per-screen maps remapped into the atlas ---------------------------
    for name in args.names:
        tiles, tmap, remap = sets[name]
        out_map = [remap[ti] for ti in tmap]
        base = f"{args.out}_map_{name}"
        guard = base.upper() + "_H"
        W = max(1, len(out_map) // 28)   # square-ish guess for the header
        with open(os.path.join(res, base + ".h"), "w") as f:
            f.write(f"#ifndef {guard}\n#define {guard}\n\n"
                    f"#include <genesis.h>\n\n"
                    f"// Generated by tools/tiles_merge.py "
                    f"(screen '{name}' in atlas '{args.out}').\n"
                    f"extern const u16 {base}[{len(out_map)}];\n\n#endif\n")
        with open(os.path.join(res, base + ".c"), "w") as f:
            f.write(f'#include "{base}.h"\n\n'
                    f"const u16 {base}[{len(out_map)}] = {{\n")
            for i in range(0, len(out_map), W):
                f.write("    " + ", ".join(str(v) for v in out_map[i:i + W]) + ",\n")
            f.write("};\n")
        with open(os.path.join(res, base + ".bin"), "wb") as f:
            f.write(struct.pack(f"<{len(out_map)}H", *out_map))
    print(f"wrote per-screen maps: " +
          ", ".join(f"res/{args.out}_map_{n}.h/.c" for n in args.names))
    return 0


if __name__ == "__main__":
    sys.exit(main())
