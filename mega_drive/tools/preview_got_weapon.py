#!/usr/bin/env python3
"""Preview the stage-clear "YOU GOT <weapon>" screen offline.

Report 2026-10-24: "on the 'you got' weapon screen, the text is over the
player". The screen is fully static (a tile background, the player sprite at a
fixed spot and three text lines on a tile grid), so it can be rendered here
instead of on the emulator: `--preview out.png` composes

  * BG_A   : src/gotbg_tiles.c + gotbg_map.c at tile 0 (PAL3 palette)
  * sprite : the player's ATTACK frame of res/player_p1.png at (20, 98)
  * text   : the three lines at tile column 9, rows 14/16/(16|17), drawn as
             filled glyph cells (the exact glyphs do not matter for placement)

and then reports every pixel where a glyph cell and the player sprite overlap.

Usage: tools/preview_got_weapon.py [--preview out.png] [--weapon "APE QUAKE"]
"""
import argparse
import os
import re
import sys

from PIL import Image, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

SPRITE_X, SPRITE_Y = 20, 98          # main.c player_place(20, 98)
TEXT_COL = 9                         # 72 px, the closest tile to the PC's 70
LINES = (14, 16, 17)                 # tile rows


def parse_u32_array(path, symbol):
    text = open(path).read()
    m = re.search(re.escape(symbol) + r"\s*\[[^\]]*\]\s*=\s*\{(.*?)\};", text, re.S)
    if not m:
        sys.exit(f"{path}: {symbol} not found")
    return [int(v, 16) if v.strip().lower().startswith("0x") else int(v)
            for v in re.findall(r"0x[0-9a-fA-F]+|\d+", m.group(1))]


def parse_u16_array(path, symbol):
    return parse_u32_array(path, symbol)


def mdtile_to_rgba(tile, palette):
    """SGDK tile = 8 u32, one per row, 8 nibbles (pixels) high to low."""
    img = Image.new("RGBA", (8, 8))
    px = img.load()
    for y in range(8):
        row = tile[y]
        for i in range(8):
            idx = (row >> (28 - i * 4)) & 0xF
            r, g, b = palette[idx]
            px[i, y] = (r, g, b, 255)
    return img


def palette_from_png(name):
    """res/<name>.png holds the palette rescomp will use (PALETTE line)."""
    path = os.path.join(ROOT, "res", name)
    im = Image.open(path)
    if im.mode != "P":
        im = im.convert("P")
    pal = im.getpalette()
    # rescomp converts 0..255 to the MD 3:3:3 grid itself; pre-compensate so
    # the preview shows what the console would.
    return [(pal[i * 3] * 7 // 255 * 255 // 7,
             pal[i * 3 + 1] * 7 // 255 * 255 // 7,
             pal[i * 3 + 2] * 7 // 255 * 255 // 7) for i in range(16)]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--preview")
    ap.add_argument("--weapon", default="APE QUAKE")
    args = ap.parse_args()

    tiles = parse_u32_array(os.path.join(ROOT, "src/gotbg_tiles.c"),
                            "gotbg_tiles_data")
    tmap = parse_u16_array(os.path.join(ROOT, "src/gotbg_map.c"), "gotbg_map")
    palette = palette_from_png("gotbg_tiles.png")

    screen = Image.new("RGB", (320, 224), (0, 0, 0))
    for i in range(0, len(tiles), 8):
        t = mdtile_to_rgba(tiles[i:i + 8], palette)
        for y in range(28):
            for x in range(40):
                if tmap[y * 40 + x] == i // 8:
                    screen.paste(t, (x * 8, y * 8))
    # tiles the map does not reference (the tile art may be reused) are skipped

    # player: ATTACK frame of the ROCKBOT sheet (res/player_p1.png, 29x29 cells)
    sheet = Image.open(os.path.join(ROOT, "res/player_p1.png")).convert("RGBA")
    fw, fh = 29, 29
    sheet.palette  # indexed art: index 0 = transparent
    frames = sheet.size[0] // fw
    # the sheet is written one animation row per ATTACK row; frame 0 of each
    # animation is at y = anim*fh - take a mid row that holds the attack pose
    attack = None
    for f in range(frames):
        cell = sheet.crop((f * fw, 0, (f + 1) * fw, fh))
        if cell.getcolors(256) is not None and len(cell.getcolors(256)) > 4:
            attack = cell
            break
    if attack is None:
        attack = sheet.crop((0, 0, fw, fh))
    mask = attack.split()[3] if attack.mode == "RGBA" else None
    screen.paste(attack, (SPRITE_X, SPRITE_Y + 2), attack)
    sprite_box = (SPRITE_X, SPRITE_Y + 2, SPRITE_X + fw, SPRITE_Y + fh + 2)

    # text: 8x8 cells at column 9
    draw = ImageDraw.Draw(screen)
    cells = []
    lines = (args.weapon, "HAS BEEN ACQUIRED")
    for i, row in enumerate(LINES[:len(lines)]):
        text = lines[i]
        for c, ch in enumerate(text):
            x = (TEXT_COL + c) * 8
            y = row * 8
            if ch != " ":
                draw.rectangle((x, y, x + 7, y + 7), fill=(255, 255, 255))
            cells.append((x, y))

    overlap = 0
    for (x, y) in cells:
        ox = min(x + 8, sprite_box[2]) - max(x, sprite_box[0])
        oy = min(y + 8, sprite_box[3]) - max(y, sprite_box[1])
        if ox > 0 and oy > 0:
            overlap += 1

    print(f"player sprite box: {sprite_box} (frame {fw}x{fh})")
    print(f"text columns {TEXT_COL}..{TEXT_COL + max(len(l) for l in lines)}"
          f" = x {TEXT_COL * 8}..{(TEXT_COL + max(len(l) for l in lines)) * 8}")
    print(f"text rows {LINES} = y {[r * 8 for r in LINES]}")
    print(f"glyph cells overlapping the sprite: {overlap}")
    if args.preview:
        screen.save(args.preview)
        print(f"wrote {args.preview}")
    return 1 if overlap else 0


if __name__ == "__main__":
    sys.exit(main())
