#!/usr/bin/env python3
"""Weapon tooltip art: the PC frame box + the weapon icons.

Sources
  images/shared/backgrounds/weapon_tooltip.png   300x32, 5 colours (the box)
  data/games/RockDroid1/images/icons.png         288x32, 18 x (16x32) cells

Frame -> a 38x4 map of 8x8 cells plus the deduplicated MD 4bpp tiles. The art
has only four colours; they land on PAL3 entries 0..3 (interior, black, mid
blue, light blue) and entry 15 keeps the font white the tooltip name is drawn
with, so the glyph cells paint the box interior instead of separate black
boxes.

Icons -> the ACTIVE half of every cell (top 16x16; the PC keeps the disabled
art in the bottom half, which the port does not need). Each icon carries its
own 11 colours in PAL3 entries 4..14: only one icon is on screen at a time, so
a shared table would just muddy them (the selectable weapons, icons 0..10,
each use 6..15 distinct colours).

Outputs src/tooltip_data.{c,h}. Run with --preview to rebuild the box and one
icon from the emitted data into /tmp/opencode and check the colours offline.
"""
import os
import re
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)

from pal_ui import MD_LEVELS, to_vdp

FRAME_SRC = os.path.join(ROOT, "images/shared/backgrounds/weapon_tooltip.png")
ICON_SRC = os.path.join(ROOT, "data/games/RockDroid1/images/icons.png")
SRC = os.path.join(ROOT, "src")

TILE = 8
# PAL3 entries: 0 interior, 1 black, 2 mid blue, 3 light blue, 4..14 icon,
# 15 font white (VDP_drawTextBG paints the glyph background with entry 0).
FRAME_COLORS = {(0, 0, 0): 1, (8, 25, 42): 0, (35, 59, 239): 2,
                (109, 146, 255): 3, (20, 1, 35): 1}
FRAME_PAL_RGB = {0: (8, 25, 42), 1: (0, 0, 0), 2: (35, 59, 239),
                 3: (109, 146, 255)}
FONT_WHITE = 0x0EEE
ICON_PAL_FIRST = 4
ICON_PAL_N = 11
CELL = 16


def snap_rgb(c):
    return tuple(min(MD_LEVELS, key=lambda l: abs(l - v)) for v in c[:3])


def nearest_frame_index(c):
    best, bd = 1, 1 << 30
    for i, p in FRAME_PAL_RGB.items():
        d = sum((p[k] - c[k]) ** 2 for k in range(3))
        if d < bd:
            bd, best = d, i
    return best


def pack(tiles):
    """8x8 index matrices -> MD 4bpp u32 words (2 pixels per byte)."""
    out = []
    for t in tiles:
        packed = []
        for y in range(TILE):
            for xb in range(TILE // 2):
                packed.append((t[y][xb * 2] << 4) | t[y][xb * 2 + 1])
        for i in range(0, len(packed), 4):
            out.append((packed[i] << 24) | (packed[i + 1] << 16) |
                       (packed[i + 2] << 8) | packed[i + 3])
    return out


def frame():
    im = Image.open(FRAME_SRC).convert("RGB")
    w, h = im.size
    if h != 32:
        raise SystemExit(f"{FRAME_SRC}: expected 32 px tall, got {h}")
    cw = (w + TILE - 1) // TILE
    px = im.load()
    unknown = 0
    grid = []
    for y in range(h):
        row = []
        for x in range(w):
            i = FRAME_COLORS.get(px[x, y])
            if i is None:
                i = nearest_frame_index(px[x, y])
                unknown += 1
            row.append(i)
        row += [1] * (cw * TILE - w)       # right edge pads with black
        grid.append(row)

    tiles, index, tmap = [], {}, []
    for cy in range(h // TILE):
        for cx in range(cw):
            px8 = tuple(tuple(grid[cy * TILE + y][cx * TILE:cx * TILE + TILE])
                        for y in range(TILE))
            ti = index.get(px8)
            if ti is None:
                ti = len(tiles)
                tiles.append(px8)
                index[px8] = ti
            tmap.append(ti)
    print(f"tooltip frame: {w}x{h} -> {cw}x{h // TILE} cells, "
          f"{len(tiles)} unique tiles, {unknown} px off-palette")
    return tiles, tmap, cw, h // TILE


def icons():
    im = Image.open(ICON_SRC).convert("RGBA")
    if im.height != 2 * CELL:
        raise SystemExit(f"{ICON_SRC}: expected {2 * CELL} px tall, "
                         f"got {im.height}")
    n = im.width // CELL
    tiles, pals = [], []
    for i in range(n):
        cell = im.crop((i * CELL, 0, i * CELL + CELL, CELL))
        cpx = cell.load()
        opaque = [cpx[x, y] for y in range(CELL) for x in range(CELL)
                  if cpx[x, y][3]]
        strip = Image.new("RGB", (len(opaque), 1))
        sp = strip.load()
        for k, c in enumerate(opaque):
            sp[k, 0] = c[:3]
        q = strip.quantize(colors=ICON_PAL_N,
                           method=Image.Quantize.MEDIANCUT)
        raw = list(q.getpalette())
        raw += [0] * (3 * ICON_PAL_N - len(raw))    # <16 colours: short table
        pal = [snap_rgb(raw[j * 3:j * 3 + 3]) for j in range(ICON_PAL_N)]
        # map every source colour onto the snapped table, keep the error
        # (transparent pixels become entry 0 = the box interior)
        lut, worst = {}, 0
        for c in set(opaque):
            best, bd = 0, 1 << 30
            for j, p in enumerate(pal):
                d = sum((p[k] - c[k]) ** 2 for k in range(3))
                if d < bd:
                    bd, best = d, j
            lut[c] = ICON_PAL_FIRST + best
            worst = max(worst, max(abs(pal[best][k] - c[k])
                                   for k in range(3)))
        grid = [[0 if not cpx[x, y][3] else lut[cpx[x, y]]
                 for x in range(CELL)] for y in range(CELL)]
        icon_tiles = []
        for ty in range(CELL // TILE):
            for tx in range(CELL // TILE):
                icon_tiles.append([grid[ty * TILE + y][tx * TILE:
                                                       tx * TILE + TILE]
                                   for y in range(TILE)])
        tiles += icon_tiles
        pals += pal
        print(f"  icon {i:2d} ({len(set(opaque))} colours, "
              f"{len(opaque)} px): worst channel error {worst}")
    return tiles, pals, n


def emit(frame_tiles, fmap, fw, fh, icon_tiles, pals, icon_n):
    packed = pack(frame_tiles)
    ipacked = pack(icon_tiles)
    pal = [0] * 16
    for i, c in FRAME_PAL_RGB.items():
        pal[i] = to_vdp(snap_rgb(c))
    pal[15] = FONT_WHITE

    h = []
    h.append("#ifndef TOOLTIP_DATA_H")
    h.append("#define TOOLTIP_DATA_H")
    h.append("")
    h.append("#include <genesis.h>")
    h.append("")
    h.append("// Generated by tools/convert_tooltip.py from the PC weapon")
    h.append("// tooltip art (images/shared/backgrounds/weapon_tooltip.png)")
    h.append("// and the weapon icons (images/icons.png). Do not edit.")
    h.append(f"#define TOOLTIP_FRAME_W {fw}")
    h.append(f"#define TOOLTIP_FRAME_H {fh}")
    h.append(f"#define TOOLTIP_FRAME_TILE_COUNT {len(frame_tiles)}")
    h.append(f"#define TOOLTIP_ICON_N {icon_n}")
    h.append(f"#define TOOLTIP_ICON_TILES 4          // 16x16 cells")
    h.append(f"#define TOOLTIP_ICON_PAL_N {ICON_PAL_N}")
    h.append(f"#define TOOLTIP_ICON_PAL_FIRST {ICON_PAL_FIRST}")
    h.append("")
    h.append("extern const u8 tooltip_frame_map[TOOLTIP_FRAME_W * "
             "TOOLTIP_FRAME_H];")
    h.append("extern const u32 "
             "tooltip_frame_tiles[TOOLTIP_FRAME_TILE_COUNT * 8];")
    h.append("extern const u32 "
             "tooltip_icon_tiles[TOOLTIP_ICON_N * TOOLTIP_ICON_TILES * 8];")
    h.append("extern const u16 "
             "tooltip_icon_pal[TOOLTIP_ICON_N * TOOLTIP_ICON_PAL_N];")
    h.append("// PAL3 with the box colours; entries 4..14 are the icon's and")
    h.append("// are overwritten per weapon at show time.")
    h.append("extern const u16 tooltip_pal_fixed[16];")
    h.append("")
    h.append("#endif")
    with open(os.path.join(SRC, "tooltip_data.h"), "w") as f:
        f.write("\n".join(h) + "\n")

    def words(name, typ, vals, count):
        f.write(f"const {typ} {name}[{count}] = {{\n")
        for i in range(0, len(vals), 8):
            if typ == "u32":
                f.write("    " + ", ".join(f"0x{v:08X}"
                                           for v in vals[i:i + 8]) + ",\n")
            else:
                f.write("    " + ", ".join(f"0x{v:04X}"
                                           for v in vals[i:i + 8]) + ",\n")
        f.write("};\n\n")

    with open(os.path.join(SRC, "tooltip_data.c"), "w") as f:
        f.write('#include "tooltip_data.h"\n\n')
        words("tooltip_frame_map", "u8", fmap, len(fmap))
        words("tooltip_frame_tiles", "u32", packed, len(packed))
        words("tooltip_icon_tiles", "u32", ipacked, len(ipacked))
        words("tooltip_icon_pal", "u16", [to_vdp(p) for p in pals], len(pals))
        words("tooltip_pal_fixed", "u16", pal, 16)
    print(f"wrote src/tooltip_data.h/.c ({len(frame_tiles)} frame tiles, "
          f"{len(icon_tiles)} icon tiles)")
    verify(packed, ipacked, fmap, frame_tiles, fw, fh)


def verify(packed, ipacked, fmap, frame_tiles, fw, fh):
    """Decode the emitted bytes back into indices and compare with the source.

    The tiles are packed here, not by rescomp, so a packing slip would only
    show as garbled art on hardware: check the C arrays rebuild the art.
    """
    text = open(os.path.join(SRC, "tooltip_data.c")).read()
    # u32 only: the u8 map and the u16 palettes must not match this pattern
    got = [int(v, 16) for v in re.findall(r"0x([0-9A-F]{8})", text)]
    n = len(packed)
    if got[:n] != packed or got[n:] != ipacked:
        raise SystemExit("tooltip_data.c: emitted tile bytes do not match")
    for ti, t in enumerate(frame_tiles):
        for y in range(TILE):
            row = []
            for byte in got[ti * 8 + y].to_bytes(4, "big"):
                row += [byte >> 4, byte & 15]
            if row != list(t[y]):
                raise SystemExit(f"tooltip_data.c: frame tile {ti} row {y} "
                                 "does not round-trip")
    print(f"verify: {len(packed)} frame + {len(ipacked)} icon tile words, "
          f"{len(set(fmap))} distinct cells used - round-trip OK")


def preview(frame_tiles, fmap, fw, fh, icon_tiles, pals, icon_n, which=0):
    out = Image.new("RGB", (fw * TILE, fh * TILE), (255, 0, 255))
    op = out.load()
    pal = {i: snap_rgb(c) for i, c in FRAME_PAL_RGB.items()}
    for j in range(ICON_PAL_N):
        pal[ICON_PAL_FIRST + j] = pals[which * ICON_PAL_N + j]
    for cy in range(fh):
        for cx in range(fw):
            t = frame_tiles[fmap[cy * fw + cx]]
            for y in range(TILE):
                for x in range(TILE):
                    op[cx * TILE + x, cy * TILE + y] = pal[t[y][x]]
    # icon at cells (1,1) of the box = pixel (8, 8)
    for ty in range(2):
        for tx in range(2):
            t = icon_tiles[which * 4 + ty * 2 + tx]
            for y in range(TILE):
                for x in range(TILE):
                    i = t[y][x]
                    if i:
                        op[(tx + 1) * TILE + x, (ty + 1) * TILE + y] = pal[i]
    os.makedirs("/tmp/opencode", exist_ok=True)
    dst = f"/tmp/opencode/tooltip_preview_{which}.png"
    out.save(dst)
    print(f"preview icon {which} -> {dst}")


def main():
    frame_tiles, fmap, fw, fh = frame()
    icon_tiles, pals, icon_n = icons()
    emit(frame_tiles, fmap, fw, fh, icon_tiles, pals, icon_n)
    if "--preview" in sys.argv:
        args = [a for a in sys.argv[1:] if not a.startswith("--")]
        preview(frame_tiles, fmap, fw, fh, icon_tiles, pals, icon_n,
                int(args[0]) if args else 0)
    return 0


if __name__ == "__main__":
    sys.exit(main())
