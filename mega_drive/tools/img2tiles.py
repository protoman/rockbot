#!/usr/bin/env python3
"""Decompose a full-screen image into the minimal set of 8x8 tiles + a map.

The PC blits big RGBA images (menu, dialogs, title, ...); the Mega Drive
cannot afford the pixels. What it needs is the *unique tiles* once in VRAM
plus a tilemap that recomposes the screen by reference. This tool does
exactly that:

  image -> <name>_tiles.png   indexed 8x8 tile strip (palettized, MD 3:3:3)
           <name>_tiles_rgba.png  same tiles as RGBA (for tiles_merge.py)
           <name>_map.h/.c    u16 tile index per 8x8 screen cell
           <name>_tiles.h     extern TileSet declaration (rescomp TILESET)

Rules:
  - alpha 0 and the game colorkey (75,125,125) become palette index 0
    (transparent/blank; on a BG plane it renders as the palette's color 0,
    which the caller can set to the panel backdrop color).
  - the image is padded right/bottom to a multiple of 8; padded cells use
    the reserved all-blank tile 0.
  - exact 8x8 duplicates are stored once (hash of the index bytes).
  - colors are rounded to the MD 3:3:3 levels and quantized to at most
    --colors usable entries (default 15) + the transparent one.

Usage:
  python3 tools/img2tiles.py <image.png> [--name NAME] [--colors 15]
                             [--out res] [--src-only]

Prints the tile count and the rescomp lines to paste in res/resources.res:
  TILESET <name>_tiles "<name>_tiles.png" FAST ALL
"""
import argparse
import hashlib
import os
import sys
from PIL import Image

TILE = 8
COLORKEY = (75, 125, 125)
MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)


def md_round(v):
    return min(MD_LEVELS, key=lambda x: abs(x - v))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("image")
    ap.add_argument("--name", default=None, help="base name (default: file stem)")
    ap.add_argument("--colors", type=int, default=15, help="usable colors (<=15)")
    ap.add_argument("--out", default=None, help="output dir (default: mega_drive/res)")
    ap.add_argument("--codedir", default=None,
                    help="dir for the generated .c/.h (default: mega_drive/src, "
                         "the build compiles src/ only)")
    ap.add_argument("--opaque", action="store_true",
                    help="do not punch the game colorkey: only alpha 0 is "
                         "transparent. Use for full-screen art whose "
                         "background colour is the colorkey (title logo): "
                         "punching it made the backdrop show through as "
                         "black bars.")
    ap.add_argument("--replace", action="append", default=[],
                    metavar="R,G,B:RRGGBB",
                    help="replace one source colour before quantising "
                         "(repeatable). The title art's background is the "
                         "game colorkey off by a few units; it must become "
                         "the config background blue (5,29,65).")
    ap.add_argument("--crop-h", type=int, default=0,
                    help="keep only the TOP N rows (art authored/taller than the 224-line screen)")
    ap.add_argument("--crop-top", type=int, default=0,
                    help="drop N pixel rows at the top (PC 240 -> MD 224 screens)")
    ap.add_argument("--src-only", action="store_true",
                    help="only emit the RGBA tile strip (no map/quantize)")
    args = ap.parse_args()

    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.dirname(here)
    res = args.out or os.path.join(root, "res")
    code = args.codedir or os.path.join(root, "src")
    name = args.name or os.path.splitext(os.path.basename(args.image))[0]

    _orig = Image.open(args.image)
    # Indexed inputs carry a meaningful palette entry 0 (the backdrop slot):
    # the art's transparent index 0 colour. Preserve it, or the plane renders
    # index 0 as black (the title logo's dark-blue backdrop was lost).
    src_pal0 = None
    if _orig.mode == "P" and _orig.palette is not None:
        _raw0 = _orig.palette.getdata()[1]
        if len(_raw0) >= 3:
            src_pal0 = tuple(_raw0[0:3])
    im = _orig.convert("RGBA")
    if args.crop_h and im.size[1] > args.crop_h:
        im = im.crop((0, 0, im.size[0], args.crop_h))
    if args.crop_top:
        im = im.crop((0, args.crop_top, im.size[0], im.size[1]))
    w, h = im.size
    cw, ch = (w + TILE - 1) // TILE, (h + TILE - 1) // TILE
    pw, ph = cw * TILE, ch * TILE

    # Indexed input: its palette is already the target MD palette (the bg art
    # comes from convert_stage's quantiser) — keep it instead of re-running
    # median cut, which shifted the night sky to purple.
    keep_pal = (im.mode == "P")

    repl = {}
    for spec in args.replace:
        a, b = spec.split(":")
        repl[tuple(int(v) for v in a.split(","))] = tuple(int(v) for v in b.split(","))

    src = im.load()
    # grid of RGBA pixels, transparent where alpha 0 or colorkey
    grid = []
    for y in range(ph):
        row = []
        for x in range(pw):
            if keep_pal:
                # indexed source (convert_stage's bg art): keep the colour
                # INDEX as-is, so the emitted tiles use the art's own palette
                # entries 1:1 (a nearest-colour remap shifted the night sky,
                # Pillow's palette round-trip made it purple).
                if x < w and y < h:
                    row.append((src[x, y], 0, 0, 255) if src[x, y] else (0, 0, 0, 0))
                else:
                    row.append((0, 0, 0, 0))
                continue
            if x < w and y < h:
                r, g, b, a = src[x, y]
                if (r, g, b) in repl:
                    r, g, b = repl[(r, g, b)]
                key = (not args.opaque) and (r, g, b) == COLORKEY
                row.append((0, 0, 0, 0) if (a == 0 or key)
                           else (md_round(r), md_round(g), md_round(b), 255))
            else:
                row.append((0, 0, 0, 0))     # padding
        grid.append(row)

    # --- unique tiles ------------------------------------------------------
    # Keyed by the RGBA bytes: this runs before quantization so tiles_merge.py
    # can compare pixel data across different screens/palettes.
    tiles = []                     # list of 64-tuples (rgba)
    index = {}                     # hash -> tile index
    blank = tuple([(0, 0, 0, 0)] * 64)
    tiles.append(blank)            # tile 0 is always the blank tile
    index[hashlib.md5(repr(blank).encode()).hexdigest()] = 0

    tilemap = []
    for cy in range(ch):
        for cx in range(cw):
            px = []
            for y in range(TILE):
                for x in range(TILE):
                    px.append(grid[cy * TILE + y][cx * TILE + x])
            key = hashlib.md5(repr(tuple(px)).encode()).hexdigest()
            ti = index.get(key)
            if ti is None:
                ti = len(tiles)
                tiles.append(tuple(px))
                index[key] = ti
            tilemap.append(ti)

    print(f"{name}: image {w}x{h} -> {cw}x{ch} cells ({cw*ch}), "
          f"unique tiles {len(tiles)} (including blank)")

    os.makedirs(res, exist_ok=True)

    # --- RGBA tile strip (always emitted; merge input) ---------------------
    # --src-only writes these raw tiles; the normal path rewrites the file
    # after quantization/dedup so tiles_merge.py compares the same set the
    # runtime uses.
    strip = Image.new("RGBA", (len(tiles) * TILE, TILE))
    sp = strip.load()
    for ti, px in enumerate(tiles):
        for y in range(TILE):
            for x in range(TILE):
                sp[ti * TILE + x, y] = px[y * TILE + x]
    strip.save(os.path.join(res, f"{name}_tiles_rgba.png"))

    if args.src_only:
        print(f"wrote res/{name}_tiles_rgba.png ({len(tiles)} tiles)")
        return 0

    # --- palette (index 0 transparent, 1..N quantized) --------------------
    if keep_pal:
        raw = im.palette.getdata()[1]
        palette = [tuple(raw[i*3:i*3+3]) for i in range(16)]

        def lut(px):
            return 0 if px[3] == 0 else px[0]     # identity: source index
        colors = []
        print("  (indexed input: tiles keep the source indices/palette)")
    else:
        colors = sorted({px for t in tiles for px in t if px[3] > 0})
    if not keep_pal and len(colors) > args.colors:
        # MEDIANCUT on a strip of every used color, weighted by pixel count
        counts = {}
        for t in tiles:
            for px in t:
                if px[3] > 0:
                    counts[px] = counts.get(px, 0) + 1
        flat = Image.new("RGB", (sum(counts.values()), 1))
        fl = flat.load()
        cx = 0
        for px, n in counts.items():
            for _ in range(n):
                fl[cx, 0] = (px[0], px[1], px[2])
                cx += 1
        q = flat.quantize(colors=args.colors, method=Image.Quantize.MEDIANCUT)
        qpal = q.getpalette()
        palette_rgb = [(md_round(qpal[i * 3]), md_round(qpal[i * 3 + 1]),
                        md_round(qpal[i * 3 + 2])) for i in range(args.colors)]
    else:
        palette_rgb = [c[:3] for c in colors][:args.colors]
    # Index 0 is the blank/backdrop slot (a BG plane cannot be transparent);
    # the art's real colours follow. Black is deliberately KEPT among them even
    # though it duplicates the blank entry: the parallax art's dominant colour
    # IS black (its transparent colorkey background), and filtering black out
    # here made those pixels snap to the nearest remaining colour — the night
    # sky rendered green (FLOWER FIELDS) / purple (CHAOS CITY) instead of
    # black, and every screen with true black art was off the same way.
    palette = [(0, 0, 0)] + palette_rgb[:args.colors]
    if src_pal0 is not None:
        palette[0] = src_pal0     # indexed input's own backdrop colour

    if not keep_pal:
        def lut(px):
            if px[3] == 0:
                return 0
            best, bd = 1, 1 << 30
            for i in range(1, len(palette)):
                c = palette[i]
                d = (c[0]-px[0])**2 + (c[1]-px[1])**2 + (c[2]-px[2])**2
                if d < bd:
                    bd, best = d, i
            return best

    # Deduplicate AFTER quantization: two tiles that differed before the
    # palette reduction can collapse to the same indices, and rescomp would
    # drop those duplicates from the TILESET (numTile < our count) shifting
    # every index — the map below must reference the final, deduplicated set.
    qtiles = []
    qindex = {}
    qmap_of_old = []
    for t in tiles:
        q = tuple(lut(px) for px in t)
        k = hashlib.md5(bytes(q)).hexdigest()
        gi = qindex.get(k)
        if gi is None:
            gi = len(qtiles)
            qtiles.append(q)
            qindex[k] = gi
        qmap_of_old.append(gi)
    remapped = [qmap_of_old[ti] for ti in tilemap]

    # The strip is laid out horizontally (N tiles side by side), so the
    # flattened pixel list must be written row-major across tiles: writing
    # tile-by-tile scrambles every tile (the "menu is a mess of stripes").
    n = len(qtiles)
    out = Image.new("P", (n * TILE, TILE))
    out.putpalette([v for c in palette for v in c] +
                   [0] * (768 - len(palette) * 3))
    data = [0] * (n * TILE * TILE)
    for ti, q in enumerate(qtiles):
        for y in range(TILE):
            row = y * (n * TILE) + ti * TILE
            for x in range(TILE):
                data[row + x] = q[y * TILE + x]
    out.putdata(data)
    tiles = qtiles               # downstream: counts, map, rgba strip
    tilemap = remapped
    out.save(os.path.join(res, f"{name}_tiles.png"))
    print(f"wrote res/{name}_tiles.png ({out.size[0]}x{out.size[1]}, "
          f"{len(palette)} palette entries)")

    # final RGBA strip mirroring the quantized/deduped set (merge input)
    strip = Image.new("RGBA", (len(tiles) * TILE, TILE))
    sp = strip.load()
    for ti, qv in enumerate(tiles):
        for y in range(TILE):
            for x in range(TILE):
                idx = qv[y * TILE + x]
                sp[ti * TILE + x, y] = (0, 0, 0, 0) if idx == 0 \
                    else tuple(palette[idx]) + (255,)
    strip.save(os.path.join(res, f"{name}_tiles_rgba.png"))
    print(f"wrote res/{name}_tiles_rgba.png ({len(tiles)} tiles)")

    # --- map as C (u16 per cell) -------------------------------------------
    hdr = os.path.join(code, f"{name}_map.h")
    src_c = os.path.join(code, f"{name}_map.c")
    guard = f"{name.upper()}_MAP_H"
    with open(hdr, "w") as f:
        f.write(f"#ifndef {guard}\n#define {guard}\n\n"
                f"#include <genesis.h>\n\n"
                f"// Generated by tools/img2tiles.py from "
                f"{os.path.basename(args.image)}. Do not edit by hand.\n"
                f"#define {name.upper()}_MAP_W {cw}\n"
                f"#define {name.upper()}_MAP_H {ch}\n"
                f"#define {name.upper()}_TILE_COUNT {len(tiles)}\n\n"
                f"extern const u16 {name}_map[{cw * ch}];\n\n#endif\n")
    with open(src_c, "w") as f:
        f.write(f'#include "{name}_map.h"\n\n'
                f"const u16 {name}_map[{cw * ch}] = {{\n")
        for cy in range(ch):
            row = tilemap[cy * cw:(cy + 1) * cw]
            f.write("    " + ", ".join(str(v) for v in row) + ",\n")
        f.write("};\n")
    print(f"wrote res/{name}_map.h/.c")

    # --- packed tile data as C (exact order/count) --------------------------
    # rescomp's TILESET resource deduplicates by color and can reorder the
    # tiles, which silently breaks a hand-made tilemap (the panel rendered
    # one repeated tile). Emit the MD 4bpp data ourselves and load it with
    # VDP_loadTileData: what the map indexes is exactly what gets uploaded.
    packed = []
    for q in qtiles:
        for y in range(TILE):
            for xb in range(TILE // 2):
                packed.append((q[y * TILE + xb * 2] << 4) | q[y * TILE + xb * 2 + 1])
    words = []
    for i in range(0, len(packed), 4):
        words.append((packed[i] << 24) | (packed[i + 1] << 16) |
                     (packed[i + 2] << 8) | packed[i + 3])
    th = os.path.join(code, f"{name}_tiles.h")
    tc = os.path.join(code, f"{name}_tiles.c")
    tguard = f"{name.upper()}_TILES_H"
    with open(th, "w") as f:
        f.write(f"#ifndef {tguard}\n#define {tguard}\n\n"
                f"#include <genesis.h>\n\n"
                f"// Generated by tools/img2tiles.py from "
                f"{os.path.basename(args.image)} — MD 4bpp tile data.\n"
                f"#define {name.upper()}_TILE_COUNT {len(qtiles)}\n\n"
                f"extern const u32 {name}_tiles_data[{name.upper()}_TILE_COUNT * 8];\n\n"
                f"#endif\n")
    with open(tc, "w") as f:
        f.write(f'#include "{name}_tiles.h"\n\n'
                f"const u32 {name}_tiles_data[{name.upper()}_TILE_COUNT * 8] = {{\n")
        for i in range(0, len(words), 8):
            f.write("    " + ", ".join(f"0x{w:08X}" for w in words[i:i + 8]) + ",\n")
        f.write("};\n")
    print(f"wrote src/{name}_tiles.h/.c ({len(qtiles)} tiles packed)")
    # machine-readable map for tiles_merge.py (u16 LE per cell)
    import struct
    with open(os.path.join(res, f"{name}_map.bin"), "wb") as f:
        f.write(struct.pack(f"<{len(tilemap)}H", *tilemap))
    print(f"wrote res/{name}_map.bin")
    print(f"\n--- res/resources.res ---")
    print(f'PALETTE {name}_pal "{name}_tiles.png"   (tiles load from '
          f'{name}_tiles.h via VDP_loadTileData)')
    return 0


if __name__ == "__main__":
    sys.exit(main())
