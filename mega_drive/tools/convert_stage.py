#!/usr/bin/env python3
"""Convert the INTRO stage (stage 0 / CHAOS CITY) from the PC data files
into SGDK resources.

Reads:
  build/games/RockDroid1/stages_v301.dat      -> stage 0 record + map links
  build/games/RockDroid1/maps_v2.dat          -> all 3 maps of stage 0
  build/games/RockDroid1/images/tilesets/<ts> -> tileset PNG

Writes:
  res/stage0_map<N>.png     full map rendered with the stage tileset
  res/stage0_bg<N>.png      parallax backgrounds baked to 512px (plane width)
  src/stage0_data.h/.c      terrain grids, links, background info

Stage 0 progression (from stages_v301.dat links, positions are Uint8;
y values after the top-row crop done by tools/import_pc_data.py):
  link2: map 0 (136,13) -> map 1 (198,0)  climb down the ladder at x=136
  link3: map 1 (1,0)    -> map 0 (139,13) climb up the ladder at x=1

Usage: python3 tools/convert_stage.py
"""
import os
import struct
import sys
from collections import Counter
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ROCKBOT = os.path.dirname(ROOT)
DATA = os.path.join(ROOT, "data/games/RockDroid1")
IMAGES = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "images")  # hand-owned art
RES = os.path.join(ROOT, "res")
SRC = os.path.join(ROOT, "src")

STAGE_STRUCT = 438
MAP_STRUCT = 15078
MAP_TILES_OFF = 78
TILE_STRUCT = 5
MAP_W, MAP_H = 200, 14   # 320x224 NTSC; the data is already cropped to 14 rows
MAP_H_SRC = 15           # file slot stride (slot 14 is unused padding)
TILE = 16
STAGE_INDEX = int(sys.argv[1]) if len(sys.argv) > 1 else 0
PFX = "stage%d" % STAGE_INDEX
UP = PFX.upper()
MAPS_PER_STAGE = 3
# object tables (same records convert_objects.py reads)
OBJ_DEF_SIZE, OBJ_SPAWN_SIZE = 84, 18
OBJ_TYPE_BOSS_DOOR = 30

COLORKEY = (75, 125, 125)
KEY_TOL = 6   # art sometimes paints the backdrop with the colorkey a few units
              # off (palmtree anim = 72,126,124); punch those too


def is_key(c):
    return (abs(c[0] - COLORKEY[0]) <= KEY_TOL
            and abs(c[1] - COLORKEY[1]) <= KEY_TOL
            and abs(c[2] - COLORKEY[2]) <= KEY_TOL)
MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)
MAX_COLORS = 15  # index 0 stays reserved for transparency
MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)   # 3-bit ramp (see pal_ui)


def md3(v):
    """8-bit channel -> MD 3-bit level (0..7)."""
    v = max(0, min(255, v))
    return min(range(8), key=lambda i: abs(MD_LEVELS[i] - v))

# Animated tiles: file_anim_block is char filename[30] + int frame_delay[6]
# with C++ alignment (56 bytes), stored as a raw array in anim_block_list.dat.
ANIM_BLOCK_SIZE = 56
ANIM_MAX_FRAMES = 6
# Fixed VRAM region for animated frames. Free zone: bg tiles end at 911
# (TILE_BG_INDEX 144 + 768), sprite engine starts at TILE_SPRITE_INDEX =
# TILE_FONT_INDEX - spriteVramSize = 1952 - 512 = 1440. One slot per
# animation: 6 frames x 4 tiles (a 16x16 frame = 2x2 tiles).
TILE_ANIM_BASE = 912
TILE_ANIM_STRIDE = 24

TERRAIN_NAMES = {
    0: "UNBLOCKED", 1: "SOLID", 2: "STAIR", 3: "DOOR", 4: "SPIKE",
    5: "WATER", 6: "ICE", 7: "MOVE_LEFT", 8: "MOVE_RIGHT", 9: "QUICKSAND",
    10: "SCROLL_LOCK", 11: "CHECKPOINT", 12: "EASYMODEBLOCK", 13: "HARDMODEBLOCK",
}


def md_round(v):
    return min(MD_LEVELS, key=lambda x: abs(x - v))


def read_stage(n):
    with open(os.path.join(DATA, "stages_v301.dat"), "rb") as f:
        f.seek(STAGE_STRUCT * n)
        raw = f.read(STAGE_STRUCT)
    return {
        "name": raw[0:30].split(b"\0")[0].decode("latin1"),
        "bgm": raw[30:60].split(b"\0")[0].decode("latin1"),
        "tileset": raw[382:412].split(b"\0")[0].decode("latin1"),
        "boss": raw[362:382].split(b"\0")[0].decode("latin1"),
    }


def read_links(n):
    """file_link = 10 bytes at offset 60, 30 of them.
    { Sint8 from_map, Sint8 to_map, st_position_uint8 origin,
      st_position_uint8 destiny, Uint8 size, bool bidir, bool door, Sint8 type }
    Positions are unsigned; the PC data stores some as negative bytes."""
    with open(os.path.join(DATA, "stages_v301.dat"), "rb") as f:
        f.seek(STAGE_STRUCT * n + 60)
        raw = f.read(30 * 10)
    links = []
    for i in range(30):
        b = raw[i * 10:(i + 1) * 10]
        frm, to, ox, oy, dx, dy, size, bidir, door, typ = struct.unpack("bbbbbbbbbb", b)
        if frm < 0 or to < 0:
            continue  # unused slot
        links.append((i, frm & 0xFF, to & 0xFF, ox & 0xFF, oy & 0xFF,
                      dx & 0xFF, dy & 0xFF, size & 0xFF,
                      1 if bidir else 0, (typ + 256) & 0xFF))
    return links


def read_boss_door_cells():
    """Map cells covered by a boss door object, per map.

    PC `classMap::load_map` (classmap.cpp:173-190) builds `wall_scroll_lock`
    from the tiles BUT counts a cell covered by a boss door as wall, so the door
    column locks the scroll even though the tiles behind it are open (CASTLE 5's
    boss room is entered through a 16x64 door in a hole in the wall). Without
    this the view scrolled over the closed door.
    """
    defs = open(os.path.join(DATA, "game_object_list.dat"), "rb").read()
    spawns = open(os.path.join(DATA, "map_object_data.dat"), "rb").read()
    cells = [set() for _ in range(MAPS_PER_STAGE)]
    for i in range(len(spawns) // OBJ_SPAWN_SIZE):
        r = spawns[i * OBJ_SPAWN_SIZE:(i + 1) * OBJ_SPAWN_SIZE]
        (oid, _pad, sx, sy, _dir, _p2, _lx, _ly, _mdest,
         st, mp, _dl, _dm, _p3) = struct.unpack_from("<bbhhbBhhbbbbbb", r)
        if oid < 0 or st != STAGE_INDEX or mp >= MAPS_PER_STAGE:
            continue
        b = defs[oid * OBJ_DEF_SIZE:(oid + 1) * OBJ_DEF_SIZE]
        if struct.unpack_from("<b", b, 60)[0] != OBJ_TYPE_BOSS_DOOR:
            continue
        w = struct.unpack_from("<b", b, 76)[0]
        h = struct.unpack_from("<b", b, 77)[0]
        # spawn positions are TILE positions (PC multiplies by TILESIZE)
        for x in range(sx, sx + max(1, w // TILE)):
            for y in range(sy, sy + max(1, h // TILE)):
                if 0 <= x < MAP_W and 0 <= y < MAP_H:
                    cells[mp].add((x, y))
        print(f"  map {mp}: boss door at ({sx},{sy}) {w}x{h} -> scroll lock "
              f"column {sx}")
    return cells


def read_anim_blocks():
    with open(os.path.join(DATA, "anim_block_list.dat"), "rb") as f:
        raw = f.read()
    blocks = []
    for off in range(0, len(raw) - ANIM_BLOCK_SIZE + 1, ANIM_BLOCK_SIZE):
        rec = raw[off:off + ANIM_BLOCK_SIZE]
        name = rec[:30].split(b"\0")[0].decode("latin1")
        delays = struct.unpack("<6i", rec[32:56])
        blocks.append((name, delays))
    return blocks


def read_map(stage_n, map_n):
    with open(os.path.join(DATA, "maps_v2.dat"), "rb") as f:
        f.seek(MAP_STRUCT * (stage_n * MAPS_PER_STAGE + map_n))
        raw = f.read(MAP_STRUCT)
    bg = struct.unpack("<hhh", raw[72:78])
    # st_map_background: char filename[30], s16 adjust_y, s8 speed, u8 auto_scroll,
    # u8 gfx, pad -> 36 bytes (C++ alignment); two of them, bg0 at offset 0.
    rec = raw[0:36]
    bg0_name = rec[0:30].split(b"\0")[0].decode("latin1")
    bg0_adjust_y, = struct.unpack("<h", rec[30:32])
    bg0_speed = struct.unpack("<b", rec[32:33])[0]
    bg0_auto = rec[33]      # E_BG_SCROLL_MODE: 0 = NONE (static, PC)
    # backgrounds[1] (offset 36) is a SECOND scrolling layer. The MD has only
    # two free scrollable planes (BG_A = map, BG_B = bg0), so the two layers
    # have to share one scroll rate: convert_background() composites it.
    rec1 = raw[36:72]
    bg1_name = rec1[0:30].split(b"\0")[0].decode("latin1")
    bg1_adjust_y, = struct.unpack("<h", rec1[30:32])
    bg1_speed = struct.unpack("<b", rec1[32:33])[0]
    t = raw[MAP_TILES_OFF:MAP_TILES_OFF + MAP_W * MAP_H_SRC * TILE_STRUCT]
    cells = []
    for x in range(MAP_W):
        col = []
        for y in range(MAP_H):          # game rows only (tools/import_pc_data.py)
            v = t[(x * MAP_H_SRC + y) * TILE_STRUCT:(x * MAP_H_SRC + y) * TILE_STRUCT + TILE_STRUCT]
            locked = v[0]
            t1x, t1y, t3x, t3y = struct.unpack("bbbb", v[1:5])
            col.append((locked, t1x, t1y, t3x, t3y))
        cells.append(col)
    return (bg, (bg0_name, bg0_adjust_y, bg0_speed, bg0_auto),
            (bg1_name, bg1_adjust_y, bg1_speed), cells)


# ---- colourcycle ---------------------------------------------------------
# Replaces tile-frame animation with a CRAM colour rotation where the art
# allows it (see stage0_plan.md). A map cell can select any of the four
# palette lines, so a cycled cell is drawn with PAL3-relative pixel values and
# marked in a per-cell mask; the runtime switches that cell's palette bits to
# PAL3 and rotates the band entries. The pool lives in the PAL3 entries the
# decomposed parallax leaves free, so the map palette (PAL1) is untouched.
# Colourcycle stages: the pool (band 12/13/14 + stable black 15) must sit in
# the PAL3 entries the decomposed parallax leaves free — measured: stage 1 uses
# 1..8, stage 2 1..9, stage 3 1..3, stage 6 1..5, stage 8 1..11, stage 0 map 0
# uses 1..15 (no room, so its lightbulb stays tile-animated).
CYCLE_STAGES = {1, 2, 3, 6, 8}
CYCLE_BAND = (12, 13, 14)          # PAL3 entries that rotate, in order
CYCLE_STABLE = 15                  # PAL3 stable colour for art black
CYCLE_FRAMES = 12                  # fallback step (200ms at 60fps)
# The art itself declares the technique: a strip that is exactly TILE (16px)
# wide holds ONE frame and is drawn as a CRAM rotation; anything wider holds
# several frames and stays on the tile-animation path. A single-frame strip
# must therefore have exactly 3 colours (the band) - the converter fails the
# build if it has fewer, and reports the fold if it has more.

def block_is_single_frame(name):
    """True for a one-frame strip (16px wide) -> CRAM rotation candidate."""
    return Image.open(anim_path(name)).width == TILE


def to_cram(c):
    """MD CRAM word for an MD-level RGB triple (R bits 1-3, G 5-7, B 9-11)."""
    r, g, b = (MD_LEVELS.index(v) for v in c)
    return (r << 1) | (g << 5) | (b << 9)


def anim_path(name):
    """Hand-owned anim art wins over the PC data: the port's copies live in
    mega_drive/images/tilesets/anim and may be re-drawn (3-colour bands for the
    colourcycle work) without touching the PC game's own images."""
    p = os.path.join(IMAGES, "tilesets/anim", name)
    return p if os.path.exists(p) else os.path.join(DATA, "images/tilesets/anim", name)


def block_colours(fn):
    """MD-rounded colours a block's art uses, minus near-black and the
    transparent colorkey."""
    im = Image.open(anim_path(fn)).convert("RGBA")
    px = im.load()
    cols = set()
    for f in range(max(1, im.width // TILE)):
        for y in range(TILE):
            for x in range(TILE):
                r, g, b, a = px[f * TILE + x, y]
                if a == 0 or is_key((r, g, b)):
                    continue
                c = (md_round(r), md_round(g), md_round(b))
                if sum(c) > 90:
                    cols.add(c)
    return cols


def cycle_band(cols):
    """The band: the three brightest colours of the set, ordered bright ->
    dark. That order is the rotation direction; shades outside the band are
    folded into the nearest entry by cyc_frame0_px(), which is the colour
    reduction colourcycle costs."""
    band = sorted(cols, key=lambda c: -(c[0] * 299 + c[1] * 587 + c[2] * 114))
    while len(band) < len(CYCLE_BAND):
        band.append(band[-1] if band else (0, 0, 0))
    return band[:len(CYCLE_BAND)]


def cyc_frame0_px(fn, band):
    """Frame 0 of a block as PAL3-relative pixel values: band colours -> the
    band entries, near-black -> the stable entry, anything else -> its nearest
    band entry, transparent -> 0."""
    im = Image.open(anim_path(fn)).convert("RGBA")
    px = im.load()
    out = []
    for y in range(TILE):
        for x in range(TILE):
            r, g, b, a = px[x, y]
            if a == 0 or is_key((r, g, b)):
                out.append(0)
                continue
            c = (md_round(r), md_round(g), md_round(b))
            if sum(c) <= 90:
                out.append(CYCLE_STABLE)
                continue
            best = min(range(len(band)),
                       key=lambda i: sum((c[k] - band[i][k]) ** 2 for k in range(3)))
            out.append(CYCLE_BAND[best])
    return out


from palfit import _d2, _ward_merge, _lloyd   # noqa: E402


def build_palette(rgbs):
    """One master palette shared by every map of the stage.

    `rgbs` is a Counter {colour: weight}: how many screen pixels show it (tiles
    are weighted by how often the maps place them, not once per unique tile).
    Colours beyond MAX_COLORS are merged by weighted error (Ward) and then
    refined (Lloyd) - see BUG-004.

    Returns (palette, lut, weighted mean squared error).
    """
    weight = dict(rgbs)
    art = dict(weight)                       # the art's own colours
    uniq = _ward_merge(weight, MAX_COLORS)
    if len(art) > MAX_COLORS:
        uniq = _lloyd(weight, uniq)
    pal = [(0, 0, 0)] + uniq
    while len(pal) < 16:
        pal.append((0, 0, 0))
    lut, err, total = {}, 0, 0
    for c, n in art.items():
        best, bd = 1, 1 << 30
        for i, p in enumerate(uniq):
            d = _d2(c, p)
            if d < bd:
                bd, best = d, i + 1
        lut[c] = best
        err += bd * n
        total += n
    return pal, lut, err / max(1, total)


TILE_USES = Counter()   # (cx,cy) -> placements in the map last read


def collect_map(map_n):
    """Return (cells, used-tile coords) for one map."""
    _, _, _, cells = read_map(STAGE_INDEX, map_n)
    used, seen = [], set()
    TILE_USES.clear()
    for x in range(MAP_W):
        for y in range(MAP_H):
            _, t1x, t1y, t3x, t3y = cells[x][y]
            for (cx, cy) in ((t1x, t1y), (t3x, t3y)):
                if cx < 0 or cy < 0:
                    continue
                TILE_USES[(cx, cy)] += 1
                if (cx, cy) not in seen:
                    seen.add((cx, cy))
                    used.append((cx, cy))
    return cells, used


def tile_weight(uses):
    """Weight of a tile in the palette fit: a ground tile placed 150 times must
    not drown a platform placed 3 times, so the weight grows like sqrt(uses)."""
    return 1 + int(uses ** 0.5)


def collect_colors(used, ts_px, uses=None):
    """Counter {colour: weight} for the tiles in `used` ((cx,cy) list)."""
    rgbs = Counter()
    for (cx, cy) in used:
        w = tile_weight(uses.get((cx, cy), 1)) if uses else 1
        for yy in range(TILE):
            for xx in range(TILE):
                r, g, b, a = ts_px[cx * TILE + xx, cy * TILE + yy]
                if a == 0 or is_key((r, g, b)):
                    continue
                rgbs[(md_round(r), md_round(g), md_round(b))] += w
    return rgbs


def convert_map(map_n, ts_px, cells, used, palette, lut, frame0_px, slot_of,
                cyc=None):
    def tile_indexed(cx, cy):
        out = []
        for yy in range(TILE):
            for xx in range(TILE):
                r, g, b, a = ts_px[cx * TILE + xx, cy * TILE + yy]
                if a == 0 or is_key((r, g, b)):
                    out.append(0)
                else:
                    c = (md_round(r), md_round(g), md_round(b))
                    out.append(lut.get(c, 1))
        return out

    def anim_frame0(gid):
        # marker: x < -1 (tile1 also needs y == 0) -> anim id = -x - 2
        # Cycled blocks have no animation slot: their art is frame 0 rebuilt
        # from PAL3-relative indices.
        if cyc and gid in cyc:
            return cyc[gid]
        return frame0_px[slot_of[gid]] if gid in slot_of else None

    tiles = [tile_indexed(cx, cy) for (cx, cy) in used]
    idx_of = {coord: i for i, coord in enumerate(used)}

    img = Image.new("P", (MAP_W * TILE, MAP_H * TILE))
    img.putpalette([v for c in palette for v in c])
    data = [0] * (MAP_W * MAP_H * TILE * TILE)
    for x in range(MAP_W):
        for y in range(MAP_H):
            _, t1x, t1y, t3x, t3y = cells[x][y]
            src = None
            if t1x >= 0 and t1y >= 0:
                src = tiles[idx_of[(t1x, t1y)]]
            elif t1x < -1 and t1y == 0:
                src = anim_frame0((-t1x) - 2)
            if t3x >= 0 and t3y >= 0:
                src = tiles[idx_of[(t3x, t3y)]]
            elif t3x < -1:
                src = anim_frame0((-t3x) - 2)
            if src is None:
                continue
            for yy in range(TILE):
                base = (y * TILE + yy) * MAP_W * TILE + x * TILE
                data[base:base + TILE] = src[yy * TILE:(yy + 1) * TILE]
    img.putdata(data)

    # Difficulty-mode blocks are NOT part of this port: the game has no
    # Easy/Hard difficulty, so BOTH variants are ignored as geometry
    # (standing rule, stage0_plan.md: TERRAIN EASYBLOCK/HARDBLOCK are
    # pass-through). PC only materialises 12 in Easy and 13 in Hard
    # (classmap.cpp:284-289). Emitting the raw values made the player land
    # on the phantom 13s under the map1 left ladder.
    terrain = []
    for y in range(MAP_H):
        for x in range(MAP_W):
            v = cells[x][y][0]
            if v == 12 or v == 13:
                v = 0
            terrain.append(v)
    hist = {}
    for v in terrain:
        hist[v] = hist.get(v, 0) + 1
    print(f"  map {map_n}: {len(used)} tiles, "
          f"{ {TERRAIN_NAMES.get(k, k): v for k, v in sorted(hist.items())} }")
    return img, terrain


BG_PLANE_W = 512  # BG_B plane is 64 tiles wide; the art is baked to this width


def bg_plane(src, h):
    """Plane content: the PC art at its own width, then its first
    (512 - w) columns, padded to `h` (a multiple of 8).

    The PC repeats a background at its own width (its left and right columns
    match: `clouds.png` column 0 == column 319) and the owner wants that exact
    spacing, so the art is NOT resampled or cropped. BG_B wraps at 512 px, which
    is not a multiple of the 320 px art, so the wrap joins art column 191 to
    column 0 - a break every 512 px of parallax travel. That is accepted as the
    price of the PC's art: the 128-tile plane that would fix it does not fit the
    VRAM/SDK layout (bugs.md BUG-049/052)."""
    bg = Image.new("RGBA", (BG_PLANE_W, h), (0, 0, 0, 0))
    bg.paste(src, (0, 0))
    tail = BG_PLANE_W - src.width
    if tail > 0:
        bg.paste(src.crop((0, 0, tail, src.height)), (src.width, 0))
    return bg


def background_exists(name):
    """TRUE for a real background file. The PC data has records with unused
    slots full of 0xFF bytes (RockDroid2 stage 14), which are not names."""
    if not name or not name.lower().endswith(".png"):
        return False
    if not all(c.isalnum() or c in "_-." for c in name):
        return False
    return os.path.exists(os.path.join(DATA, "images/map_backgrounds", name))


def unkey(im):
    """The PC colorkey (and any alpha-0 pixel) becomes fully transparent."""
    px = im.load()
    for y in range(im.height):
        for x in range(im.width):
            r, g, b, a = px[x, y]
            if a == 0 or is_key((r, g, b)):
                px[x, y] = (0, 0, 0, 0)
    return im


def convert_background(map_n, name, adjust_y, speed, over=None):
    """Bake one parallax background to the 512 px plane, repeating a strip
    whose period divides that width so the wrap has no seam (bg_period).

    `over` is the PC's backgrounds[1] (name, adjust_y, speed): a second layer
    composited on top of this one. It is a compromise, not a port - see the
    caller."""
    src = unkey(Image.open(os.path.join(DATA, "images/map_backgrounds", name)).convert("RGBA"))
    if over:
        o_name, o_adj, o_speed = over
        top = unkey(Image.open(os.path.join(DATA, "images/map_backgrounds",
                                            o_name)).convert("RGBA"))
        dy = o_adj - adjust_y
        art = Image.new("RGBA", src.size, (0, 0, 0, 0))
        art.paste(src, (0, 0))
        for x in range(0, src.width, max(1, top.width)):
            art.paste(top, (x, dy), top)
        src = art
        print(f"  bg{map_n}: layer1 {o_name} {top.size} composited at "
              f"dy={dy} (its own speed {o_speed} vs {speed} cannot be "
              f"independent on one plane)")
    h = (src.height + 7) & ~7   # SGDK wants height as a multiple of 8
    bg = bg_plane(src, h)
    px = bg.load()
    # The colorkey is transparent (PC): those pixels are palette index 0, so the
    # plane shows the map's background colour (CRAM 0) through them.
    clear = [px[x, y][3] == 0 for y in range(bg.height) for x in range(bg.width)]
    rgbs = [(md_round(px[x, y][0]), md_round(px[x, y][1]), md_round(px[x, y][2]))
            for y in range(bg.height) for x in range(bg.width)]
    pal, lut, _bg_err = build_palette(Counter(c for c, t in zip(rgbs, clear) if not t))
    img = Image.new("P", (BG_PLANE_W, bg.height))
    img.putpalette([v for c in pal for v in c])
    img.putdata([0 if t else lut.get(c, 1) for c, t in zip(rgbs, clear)])
    out = os.path.join(RES, f"{PFX}_bg{map_n}.png")
    img.save(out, transparency=0)   # img2tiles must read index 0 as alpha 0, not black
    print(f"  bg{map_n}: {name} {src.size} -> {img.size} "
          f"(adjust_y={adjust_y} speed={speed}) -> res/{PFX}_bg{map_n}.png")
    return {"present": 1, "adjust_y": adjust_y, "speed": speed}


def main():
    stage = read_stage(STAGE_INDEX)
    links = read_links(STAGE_INDEX)
    print(f"stage {STAGE_INDEX}: '{stage['name']}' tileset={stage['tileset']} "
          f"boss={stage['boss']}")
    print(f"links: {links}")

    ts = Image.open(os.path.join(DATA, "images/tilesets", stage["tileset"])).convert("RGBA")
    ts_px = ts.load()
    os.makedirs(RES, exist_ok=True)
    os.makedirs(SRC, exist_ok=True)

    maps_data, all_rgbs, scroll_locks = [], Counter(), []
    lock_terrain = {1, 3, 4, 6, 10}
    door_cells = read_boss_door_cells()
    for m in range(MAPS_PER_STAGE):
        cells, used = collect_map(m)
        all_rgbs += collect_colors(used, ts_px, TILE_USES)
        maps_data.append((cells, used))
        scroll_locks.append([
            x for x in range(MAP_W)
            if all(cells[x][y][0] in lock_terrain
                   or (x, y) in door_cells[m] for y in range(MAP_H))
        ])
    maps_data = [(cells, used)
                 for m, (cells, used) in enumerate(maps_data)]

    anim_blocks = read_anim_blocks()

    # A map cell has only one tile on the MD (PC can draw tile1 and tile3 on
    # top of each other). When both layers have art, keep the one with fewer
    # transparent pixels: the more opaque tile is the one that actually fills
    # the cell. This is what fixes Daisie's one-tile waterfall gap (the
    # half-transparent waterfall_top was winning over the opaque water tile).
    def tile_opacity(tx, ty):
        if tx < 0 or ty < 0:
            return -1
        opaque = 0
        for yy in range(TILE):
            for xx in range(TILE):
                r, g, b, a = ts_px[tx * TILE + xx, ty * TILE + yy]
                if a != 0 and not is_key((r, g, b)):
                    opaque += 1
        return opaque

    def gid_opacity(gid):
        im = Image.open(anim_path(anim_blocks[gid][0])).convert("RGBA")
        px = im.load()
        opaque = 0
        for yy in range(TILE):
            for xx in range(TILE):
                r, g, b, a = px[xx, yy]
                if a != 0 and not is_key((r, g, b)):
                    opaque += 1
        return opaque

    def resolve_layers(cells):
        for col in cells:
            for i, (locked, t1x, t1y, t3x, t3y) in enumerate(col):
                p1 = None
                p3 = None
                if t1x >= 0 and t1y >= 0:
                    p1 = tile_opacity(t1x, t1y)
                elif t1x < -1 and t1y == 0:
                    gid = (-t1x) - 2
                    if gid < len(anim_blocks):
                        p1 = gid_opacity(gid)
                if t3x >= 0 and t3y >= 0:
                    p3 = tile_opacity(t3x, t3y)
                elif t3x < -1:
                    gid = (-t3x) - 2
                    if gid < len(anim_blocks):
                        p3 = gid_opacity(gid)
                # Only the two-animated-tile overlap needs this rule (Daisie's
                # waterfall); static/static and static/animated overlaps are
                # meaningful foreground art and must keep their normal order.
                a1 = (t1x < -1 and t1y == 0)
                a3 = (t3x < -1)
                if (STAGE_INDEX == 2 and a1 and a3 and
                        p1 is not None and p3 is not None and p1 > p3):
                    col[i] = (locked, t1x, t1y, -1, -1)

    for cells, _ in maps_data:
        resolve_layers(cells)

    # colourcycle pool: which blocks of this stage are drawn as a CRAM rotation.
    # There is ONE pool per stage (a set of PAL3 entries), so the colour set
    # used by the most cells sets the band; blocks whose colours differ cannot
    # share it and stay on tile animation (reported at conversion time).
    stage_band, cycle_gids, cyc_frame0 = [], set(), {}
    cycle_frames = CYCLE_FRAMES
    if STAGE_INDEX in CYCLE_STAGES:
        cand, cells_of = set(), {}
        for cells, _ in maps_data:
            for col in cells:
                for (_, t1x, t1y, t3x, t3y) in col:
                    gid = None
                    if t1x < -1 and t1y == 0: gid = (-t1x) - 2
                    if t3x < -1: gid = (-t3x) - 2
                    if (gid is not None and gid < len(anim_blocks)
                            and block_is_single_frame(anim_blocks[gid][0])):
                        cand.add(gid)
                        cells_of[gid] = cells_of.get(gid, 0) + 1
        for gid in sorted(cand):
            fn = anim_blocks[gid][0]
            n = len(block_colours(fn))
            if n < 3:
                sys.exit(
                    f"ERROR: {fn} has only {n} colour(s). A colourcycled block "
                    f"needs exactly 3 rotating colours (transparent, plus black "
                    f"if the art needs a non-rotating outline, are free). "
                    f"Fix the art in images/tilesets/anim/{fn}.")
            if n > 3:
                print(f"    colourcycle: {fn} has {n} colours -> folding into "
                      f"the band (draw 3 for an exact rotation)")
        if cand:
            groups = {}
            for gid in sorted(cand):
                key = tuple(sorted(block_colours(anim_blocks[gid][0])))
                groups.setdefault(key, []).append(gid)
            best = max(groups.values(),
                       key=lambda g: sum(cells_of[x] for x in g))
            cycle_gids = set(best)
            for g in groups.values():
                if g is best:
                    continue
                for gid in g:
                    print(f"    colourcycle: {anim_blocks[gid][0]} stays on tile "
                          f"animation in stage {STAGE_INDEX}: its colours differ "
                          f"from the stage band")
            ms = anim_blocks[sorted(best)[0]][1][0]
            cycle_frames = max(2, min(60, (ms * 60 + 500) // 1000))
            cols = set()
            for gid in best:
                cols |= block_colours(anim_blocks[gid][0])
            stage_band = cycle_band(cols)
            for gid in sorted(cycle_gids):
                cyc_frame0[gid] = cyc_frame0_px(anim_blocks[gid][0], stage_band)
            print(f"  colourcycle: {len(cycle_gids)} blocks, band {stage_band} "
                  f"at PAL3 {CYCLE_BAND} + stable {CYCLE_STABLE}, "
                  f"{cycle_frames} frames/step")

    mask_bytes = (MAP_W * MAP_H + 7) // 8
    tile3_masks = []
    pal3_masks = []              # per map: cells drawn with PAL3 (colourcycled)
    anim_cell_slot = []          # per map: u8 per cell, 0 = none, else slot+1
    anim_slot_gids = []          # per map: gid per compacted slot
    for cells, _ in maps_data:
        mask3 = bytearray(mask_bytes)
        pal3 = bytearray(mask_bytes)
        cell_slot = bytearray(MAP_W * MAP_H)
        gid_to_slot, slot_gids = {}, []
        for x in range(MAP_W):
            for y in range(MAP_H):
                _, t1x, t1y, t3x, t3y = cells[x][y]
                n = y * MAP_W + x
                if t3x >= 0 and t3y >= 0:
                    mask3[n >> 3] |= 1 << (n & 7)
                gid = None
                if t1x < -1 and t1y == 0:
                    gid = (-t1x) - 2
                if t3x < -1:
                    gid = (-t3x) - 2
                    mask3[n >> 3] |= 1 << (n & 7)   # anim on tile3 = above player
                if gid is None or gid >= len(anim_blocks):
                    continue
                if gid in cycle_gids:
                    # colourcycled: no animation slot, the cell just needs its
                    # palette line (PAL3) instead of the map's PAL1
                    pal3[n >> 3] |= 1 << (n & 7)
                    continue
                if gid not in gid_to_slot:
                    gid_to_slot[gid] = len(slot_gids)
                    slot_gids.append(gid)
                cell_slot[n] = gid_to_slot[gid] + 1
        tile3_masks.append(mask3)
        pal3_masks.append(pal3)
        anim_cell_slot.append(cell_slot)
        anim_slot_gids.append(slot_gids)
    anim_slot_max = max(1, max((len(g) for g in anim_slot_gids), default=0))
    print(f"anim slots per map: {[len(g) for g in anim_slot_gids]} (max {anim_slot_max})")

    all_gids = sorted({g for gs in anim_slot_gids for g in gs})
    anim_uses = Counter()          # placements per animated block (palette weight)
    for slots, gids in zip(anim_cell_slot, anim_slot_gids):
        for v in slots:
            if v:
                anim_uses[gids[v - 1]] += 1
    strips = {}
    for gid in all_gids:
        fn = anim_blocks[gid][0]
        im = Image.open(anim_path(fn)).convert("RGBA")
        strips[gid] = im
        nf = max(1, im.width // TILE)
        all_rgbs += collect_colors([(f, 0) for f in range(nf)], im.load(),
                                   {(f, 0): anim_uses.get(gid, 1) for f in range(nf)})

    palette, lut, pal_err = build_palette(all_rgbs)
    print(f"master palette: {len(all_rgbs)} unique colors -> {MAX_COLORS} "
          f"slots max, weighted error {pal_err:.0f}")

    def frame_tiles(im, frame):
        px = im.load()
        out = []
        for qx, qy in ((0, 0), (8, 0), (0, 8), (8, 8)):
            words = []
            for yy in range(8):
                for k in (0, 4):
                    w = 0
                    for p in range(4):
                        r, g, b, a = px[frame * TILE + qx + k + p, qy + yy]
                        if a == 0 or is_key((r, g, b)):
                            idx = 0
                        else:
                            idx = lut.get((md_round(r), md_round(g), md_round(b)), 1)
                        w = (w << 4) | idx
                    words.append(w)
            out.append(words)
        return out

    def tiles_pixels(t4):
        px = [0] * (TILE * TILE)
        for q, words in enumerate(t4):
            qx, qy = (q & 1) * 8, (q >> 1) * 8
            for yy in range(8):
                for xi in range(8):
                    w = words[yy * 2 + (xi >> 2)]
                    shift = 12 - (xi & 3) * 4
                    px[(qy + yy) * TILE + qx + xi] = (w >> shift) & 0xF
        return px

    # per-map: slot -> frame0 pixels, and slot -> (frames, delays, name)
    frame0_map, slots_meta = [], []
    for slot_gids in anim_slot_gids:
        f0map, meta = {}, []
        for s, gid in enumerate(slot_gids):
            im = strips[gid]
            nf = max(1, im.width // TILE)
            f0 = frame_tiles(im, 0)
            frames = []
            for f in range(ANIM_MAX_FRAMES):
                t = frame_tiles(im, f) if f < nf else f0
                frames.append([w for tile in t for w in tile])
            f0map[s] = tiles_pixels(f0)
            fn, delays = anim_blocks[gid]
            dfs = [min(65535, max(1, (ms * 60 + 500) // 1000))
                   for ms in (delays[f] if f < len(delays) else 200
                              for f in range(ANIM_MAX_FRAMES))]
            meta.append({"nf": nf, "frames": frames, "delays": dfs, "name": fn})
        frame0_map.append(f0map)
        slots_meta.append(meta)

    terrains = []
    for m, (cells, used) in enumerate(maps_data):
        slot_of = {g: s for s, g in enumerate(anim_slot_gids[m])}
        img, terrain = convert_map(m, ts_px, cells, used, palette, lut,
                                   frame0_map[m], slot_of, cyc_frame0)
        img.save(os.path.join(RES, f"{PFX}_map{m}.png"))
        print(f"    -> res/{PFX}_map{m}.png {img.size}")
        terrains.append(terrain)
    for m in range(MAPS_PER_STAGE):
        n = sum(1 for v in anim_cell_slot[m] if v)
        print(f"  map {m}: tile3={sum(bin(b).count('1') for b in tile3_masks[m])} "
              f"anim_cells={n}")

    bg_infos, bg_names, bg_colors = [], [], []
    for m in range(MAPS_PER_STAGE):
        bg_rgb, bg0, bg1, _ = read_map(STAGE_INDEX, m)
        # PC classmap.cpp:244 clears the screen with map_data.background_color
        # before drawing the background image: everything the image does not
        # cover is this colour (the MD backdrop, CRAM 0).
        bg_colors.append(tuple(md3(v) for v in bg_rgb))
        name, adj, spd, auto = bg0
        over = bg1 if background_exists(bg1[0]) else None
        if name:
            info = convert_background(m, name, adj, spd, over)
            # backgrounds[1] is composited into the bg0 bake (the MD has one
            # parallax plane), and the PC scrolls that layer on its own
            # (backgrounds[1].auto_scroll: UP/DOWN on stage 6 and on several
            # RockDroid2 maps). When bg0 is static, the baked layer inherits
            # the mode/speed so the composite still moves.
            if not auto and bg1[0] and over is not None:
                b1_auto = read_map(STAGE_INDEX, m)[2]
                # read_map returns name/adj/speed only for bg1; re-read the mode
                import struct as _st
                with open(os.path.join(DATA, "maps_v2.dat"), "rb") as _f:
                    _f.seek((STAGE_INDEX * MAPS_PER_STAGE + m) * MAP_STRUCT)
                    _raw = _f.read(MAP_STRUCT)
                b1_mode = _raw[36 + 33]
                if 1 <= b1_mode <= 5:
                    auto = b1_mode
                    spd = bg1[2]
                    print(f"    !! map {m}: bg0 is static, backgrounds[1] "
                          f"auto_scroll={b1_mode} speed={bg1[2]} -> the baked "
                          f"layer scrolls with that mode")
            info["auto_scroll"] = auto
            bg_infos.append(info)
            bg_names.append(name)
        else:
            bg_infos.append({"present": 0, "adjust_y": 0, "speed": 0,
                             "auto_scroll": 0})
            bg_names.append("")

    lock_max = max(1, max(len(l) for l in scroll_locks))

    # ---- header ----
    with open(os.path.join(SRC, f"{PFX}_data.h"), "w") as f:
        f.write(f"#ifndef {UP}_DATA_H\n#define {UP}_DATA_H\n\n"
                "#include <genesis.h>\n#include \"stage_data.h\"\n\n"
                f"// Generated by tools/convert_stage.py from stage "
                f"{STAGE_INDEX} (\"{stage['name']}\").\n"
                f"extern const StageDef {PFX}_def;\n\n#endif\n")

    # ---- source ----
    c = [f'#include "{PFX}_data.h"\n',
         '#include "resources.h"\n',
         f'#include "{PFX}_enemies.h"\n',
         f'#include "{PFX}_objects.h"\n']
    for m in range(MAPS_PER_STAGE):
        if bg_names[m]:
            c.append(f'#include "{PFX}_bg{m}_tiles.h"\n')
            c.append(f'#include "{PFX}_bg{m}_map.h"\n')
    c.append("\n")

    c.append(f"static const u8 {PFX}_terrain[{MAPS_PER_STAGE}]"
             f"[{MAP_W * MAP_H}] = {{\n")
    for terr in terrains:
        c.append("    {\n")
        for y in range(MAP_H):
            row = ",".join(str(terr[y * MAP_W + x]) for x in range(MAP_W))
            c.append("        " + row + ",\n")
        c.append("    },\n")
    c.append("};\n\n")

    c.append(f"static const StageLink {PFX}_links[{max(1,len(links))}] = {{\n")
    for (i, frm, to, ox, oy, dx, dy, size, bidir, typ) in links:
        c.append(f"    {{ {frm}, {ox}, {oy}, {to}, {dx}, {dy}, "
                 f"{size}, {bidir}, {typ} }}, // link{i}\n")
    c.append("};\n\n")

    c.append(f"static const u8 {PFX}_bg_color[{MAPS_PER_STAGE}][3] = {{\n")
    for m, col in enumerate(bg_colors):
        c.append(f"    {{ {col[0]}, {col[1]}, {col[2]} }}, // map {m}\n")
    c.append("};\n\n")

    c.append(f"static const StageBgInfo {PFX}_bg_info[{MAPS_PER_STAGE}] = {{\n")
    for m, info in enumerate(bg_infos):
        c.append(f"    {{ {info['present']}, {info['adjust_y']}, "
                 f"{info['speed']}, {info['auto_scroll']} }}, // map {m}\n")
    c.append("};\n\n")

    c.append(f"static const u8 {PFX}_scroll_lock_n[{MAPS_PER_STAGE}] = {{\n    ")
    c.append(", ".join(str(len(l)) for l in scroll_locks))
    c.append("\n};\n\n")
    c.append(f"static const u8 {PFX}_scroll_lock[{MAPS_PER_STAGE}][{lock_max}] = {{\n")
    for locks in scroll_locks:
        padded = locks + [0] * (lock_max - len(locks))
        c.append("    { " + ", ".join(str(v) for v in padded) + " },\n")
    c.append("};\n\n")

    def emit_mask(name, masks):
        c.append(f"static const u8 {name}[{MAPS_PER_STAGE}][{mask_bytes}] = {{\n")
        for mask in masks:
            c.append("    {\n")
            for i in range(0, len(mask), 16):
                chunk = ", ".join("0x%02X" % b for b in mask[i:i + 16])
                c.append("        " + chunk + ",\n")
            c.append("    },\n")
        c.append("};\n\n")

    emit_mask(f"{PFX}_tile3_mask", tile3_masks)
    if stage_band:                       # only stages with a pool pay for it
        emit_mask(f"{PFX}_pal3_mask", pal3_masks)

    if stage_band:
        c.append(f"static const u16 {PFX}_cycle_colors[{len(stage_band)}] = {{ "
                 + ", ".join("0x%04X" % to_cram(x) for x in stage_band)
                 + " };\n\n")
    else:
        c.append(f"static const u16 {PFX}_cycle_colors[1] = {{ 0 }};\n\n")

    c.append(f"static const u8 {PFX}_anim_cell_slot[{MAPS_PER_STAGE}]"
             f"[{MAP_W * MAP_H}] = {{\n")
    for cs in anim_cell_slot:
        c.append("    {\n")
        for i in range(0, len(cs), 32):
            c.append("        " + ", ".join(str(b) for b in cs[i:i + 32]) + ",\n")
        c.append("    },\n")
    c.append("};\n\n")
    c.append(f"static const u8 {PFX}_anim_slot_n[{MAPS_PER_STAGE}] = {{\n    ")
    c.append(", ".join(str(len(g)) for g in anim_slot_gids) + "\n};\n\n")

    c.append(f"static const StageAnimSlot {PFX}_anim_slots"
             f"[{MAPS_PER_STAGE}][{anim_slot_max}] = {{\n")
    for meta in slots_meta:
        rows = [f"{{ {m['nf']}, {{ " + ", ".join(str(d) for d in m['delays'])
                + " } }" for m in meta]
        rows += ["{ 1, { 1, 1, 1, 1, 1, 1 } }"] * (anim_slot_max - len(rows))
        c.append("    { " + ", ".join(rows) + " },\n")
    c.append("};\n\n")

    c.append(f"static const u16 {PFX}_anim_frames[{MAPS_PER_STAGE}]"
             f"[{anim_slot_max}][6][64] = {{\n")
    for meta in slots_meta:
        c.append("    {\n")
        for m in meta:
            c.append("      {\n")
            for words in m["frames"]:
                c.append("        { " + ", ".join("0x%04X" % w for w in words) + " },\n")
            c.append("      },\n")
        for _ in range(anim_slot_max - len(meta)):
            c.append("      { { 0 } },\n")
        c.append("    },\n")
    c.append("};\n\n")

    c.append(f"static const MapDefinition *const {PFX}_map_defs[{MAPS_PER_STAGE}] = {{\n")
    c.append("    " + ", ".join(f"&{PFX}_map{m}" for m in range(MAPS_PER_STAGE)) + "\n};\n")
    c.append(f"static const TileSet *const {PFX}_tile_sets[{MAPS_PER_STAGE}] = {{\n")
    c.append("    " + ", ".join(f"&{PFX}_tiles{m}" for m in range(MAPS_PER_STAGE)) + "\n};\n")
    c.append(f"static const Palette *const {PFX}_palettes[{MAPS_PER_STAGE}] = {{\n")
    c.append("    " + ", ".join(f"&{PFX}_pal{m}" for m in range(MAPS_PER_STAGE)) + "\n};\n")
    c.append(f"static const Palette *const {PFX}_bg_pal[{MAPS_PER_STAGE}] = {{\n    ")
    c.append(", ".join(f"&{PFX}_bg{m}_pal" if bg_names[m] else "NULL"
                       for m in range(MAPS_PER_STAGE)))
    c.append("\n};\n")
    c.append(f"static const u32 *const {PFX}_bg_tiles[{MAPS_PER_STAGE}] = {{\n    ")
    c.append(", ".join(f"{PFX}_bg{m}_tiles_data" if bg_names[m] else "NULL"
                       for m in range(MAPS_PER_STAGE)))
    c.append("\n};\n")
    c.append(f"static const u16 {PFX}_bg_tiles_n[{MAPS_PER_STAGE}] = {{\n    ")
    c.append(", ".join(f"{UP}_BG{m}_TILE_COUNT" if bg_names[m] else "0"
                       for m in range(MAPS_PER_STAGE)))
    c.append("\n};\n")
    c.append(f"static const u16 *const {PFX}_bg_maps[{MAPS_PER_STAGE}] = {{\n    ")
    c.append(", ".join(f"{PFX}_bg{m}_map" if bg_names[m] else "NULL"
                       for m in range(MAPS_PER_STAGE)))
    c.append("\n};\n")
    c.append(f"static const u16 {PFX}_bg_map_h[{MAPS_PER_STAGE}] = {{\n    ")
    c.append(", ".join(f"{UP}_BG{m}_MAP_H" if bg_names[m] else "0"
                       for m in range(MAPS_PER_STAGE)))
    c.append("\n};\n\n")

    c.append(f"const StageDef {PFX}_def = {{\n")
    c.append(f"    .map_count = {MAPS_PER_STAGE}, .map_w = {MAP_W}, "
             f".map_h = {MAP_H}, .tile = {TILE},\n")
    c.append(f"    .map_defs = {PFX}_map_defs, .tile_sets = {PFX}_tile_sets, "
             f".palettes = {PFX}_palettes,\n")
    c.append(f"    .terrain = &{PFX}_terrain[0][0],\n")
    c.append(f"    .links = {PFX}_links, .link_count = {len(links)},\n")
    c.append(f"    .bg_info = {PFX}_bg_info, .bg_color = &{PFX}_bg_color[0][0],\n")
    c.append(f"    .bg_pal = {PFX}_bg_pal, "
             f".bg_tiles = {PFX}_bg_tiles,\n")
    c.append(f"    .bg_tiles_n = {PFX}_bg_tiles_n, .bg_maps = {PFX}_bg_maps, "
             f".bg_map_h = {PFX}_bg_map_h,\n")
    c.append(f"    .scroll_lock_n = {PFX}_scroll_lock_n, "
             f".scroll_lock = &{PFX}_scroll_lock[0][0], .scroll_lock_max = {lock_max},\n")
    c.append(f"    .tile3_mask = &{PFX}_tile3_mask[0][0],\n")
    c.append(f"    .pal3_mask = "
             + (f"&{PFX}_pal3_mask[0][0],\n" if stage_band else "NULL,\n"))
    c.append(f"    .cycle_base = {CYCLE_BAND[0]}, "
             f".cycle_n = {len(stage_band) if stage_band else 0}, "
             f".cycle_frames = {cycle_frames if stage_band else 0},\n")
    c.append(f"    .cycle_colors = {PFX}_cycle_colors,\n")
    c.append(f"    .anim_cell_slot = &{PFX}_anim_cell_slot[0][0], "
             f".anim_slot_n = {PFX}_anim_slot_n, .anim_slot_max = {anim_slot_max},\n")
    c.append(f"    .anim_slots = &{PFX}_anim_slots[0][0], "
             f".anim_frames = &{PFX}_anim_frames[0][0],\n")
    c.append(f"    .enemy_defs = {PFX}_enemy_defs, "
             f".enemy_def_count = {UP}_ENEMY_DEF_COUNT,\n")
    c.append(f"    .enemy_spawns = {PFX}_enemy_spawns, "
             f".enemy_spawn_count = {UP}_ENEMY_SPAWN_COUNT,\n")
    c.append(f"    .enemy_ai = {PFX}_enemy_ai, .enemy_react = {PFX}_enemy_react,\n")
    c.append(f"    .enemy_proj = {PFX}_enemy_proj, "
             f".enemy_proj_max = {UP}_ENEMY_PROJ_MAX,\n")
    c.append(f"    .object_defs = {PFX}_object_defs, "
             f".object_def_count = {UP}_OBJECT_DEF_COUNT,\n")
    c.append(f"    .object_spawns = {PFX}_object_spawns, "
             f".object_spawn_count = {UP}_OBJECT_SPAWN_COUNT,\n")
    c.append("};\n")

    with open(os.path.join(SRC, f"{PFX}_data.c"), "w") as f:
        f.write("".join(c))
    print(f"wrote src/{PFX}_data.c/h ({len(terrains)} maps, {len(links)} links, "
          f"{anim_slot_max} anim slots)")
    print(f"\n--- res/resources.res ---")
    for m in range(MAPS_PER_STAGE):
        print(f'TILESET {PFX}_tiles{m} "{PFX}_map{m}.png" FAST ALL')
        print(f'PALETTE {PFX}_pal{m}   "{PFX}_map{m}.png"')
        print(f'MAP     {PFX}_map{m}   "{PFX}_map{m}.png"  {PFX}_tiles{m} FAST 0')
    for m in range(MAPS_PER_STAGE):
        if bg_names[m]:
            print(f'PALETTE {PFX}_bg{m}_pal "{PFX}_bg{m}_tiles.png"')
    return 0


if __name__ == "__main__":
    sys.exit(main())
