#!/usr/bin/env python3
"""Build-time VRAM fit check for the Mega Drive port.

The tile layout is fixed (src/stage.h): system 0..15, stage tileset from 16,
parallax bg from TILE_BG_INDEX, animated-tile slots from TILE_ANIM_BASE, sprite
pool up to TILE_SPRITE_INDEX. Nothing checked that a stage's data actually fits
those blocks, and twice it did not:

  * the stage tileset (up to 144 tiles from index 16) overlapped the bg block
    that used to start at 144 -> the "artifact on the right side of some
    tileset pillars";
  * the palm anim tiles ran into the sprite pool -> corrupted HUD/sprites.

Run after the link (the Makefile does): the per-map tileset sizes only exist
once rescomp has packed them, so they are read from the ROM symbols.

Exits non-zero on a ladder violation, and prints the room-transition fallbacks
(links whose destination window needs more free tiles than exist — those fade
instead of scrolling; see src/transition.c).
"""
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SGDK = os.environ.get("GDK", "")
# Number of converted stages, straight from the registry (single source of
# truth); this used to be hardcoded to 9 and silently skipped new stages.
_h = open(os.path.join(ROOT, "src/stage_data.h")).read()
STAGES = int(re.search(r"#define STAGE_DEFINED_COUNT (\d+)", _h).group(1))
MAPS = 3
ANIM_STRIDE = 24


def read_layout():
    """The VRAM constants, straight from src/stage.h (single source of truth)."""
    h = open(os.path.join(ROOT, "src/stage.h")).read()

    def num(name):
        m = re.search(rf"^#define\s+{name}\s+(\d+)", h, re.M)
        if not m:
            sys.exit(f"check_vram: {name} not found in src/stage.h")
        return int(m.group(1))

    return {
        # SGDK's TILE_USER_INDEX (vdp.h: TILE_SYSTEM_INDEX + TILE_SYSTEM_LENGTH).
        "user": 16,
        "stride": num("TILE_ANIM_STRIDE"),
    }


def read_tileset_sizes(rom_path, sym_path):
    """{stage: {map: numTile}} from the packed TileSet structs (compression,
    numTile, tiles[])."""
    sym = {}
    for line in open(sym_path):
        p = line.split()
        if len(p) >= 3:
            sym[p[2]] = int(p[0], 16)
    rom = open(rom_path, "rb").read()
    out = {}
    for s in range(STAGES):
        out[s] = {}
        for m in range(MAPS):
            off = sym.get(f"stage{s}_tiles{m}")
            if off is None:
                continue
            comp, num = struct.unpack(">HH", rom[off:off + 4])
            out[s][m] = num
    return out


def read_bg_tiles():
    """{stage: {map: tiles}} from the decomposed bg tile strips (8px wide each)."""
    from PIL import Image
    out = {}
    for s in range(STAGES):
        out[s] = {}
        for m in range(MAPS):
            p = os.path.join(ROOT, "res", f"stage{s}_bg{m}_tiles.png")
            out[s][m] = (Image.open(p).size[0] // 8) if os.path.exists(p) else 0
    return out


def read_anim_slots():
    out = {}
    for s in range(STAGES):
        t = open(os.path.join(ROOT, "src", f"stage{s}_data.c")).read()
        m = re.search(r"anim_slot_n\[\d+\] = \{([^}]*)\}", t)
        out[s] = [int(v) for v in m.group(1).split(",")] if m else [0] * MAPS
    return out


def read_links():
    out = {}
    for s in range(STAGES):
        t = open(os.path.join(ROOT, "src", f"stage{s}_data.c")).read()
        m = re.search(rf"stage{s}_links\[\d+\] = \{{(.*?)\}};", t, re.S)
        ls = []
        for line in (m.group(1).splitlines() if m else []):
            line = line.split("//")[0].strip().rstrip(",")
            v = [int(x) for x in re.findall(r"-?\d+", line)]
            if len(v) >= 6:
                ls.append(v[:6])
        out[s] = ls
    return out


def window_tiles(stage, map_n, camx, map_w=200):
    """Distinct 8x8 tiles in the 20x14-map-tile window at camera x (the same
    slice a room transition copies)."""
    from PIL import Image
    im = Image.open(os.path.join(ROOT, "res", f"stage{stage}_map{map_n}.png")).convert("RGB")
    px = im.load()
    camx = max(0, min(camx, map_w * 16 - 320))
    a = (camx >> 4) * 2
    seen, cache = set(), {}
    for y in range(28):
        for cx in range(a, a + 40):
            sig = tuple(px[cx * 8 + dx, y * 8 + dy] for dy in range(8) for dx in range(8))
            seen.add(cache.setdefault(sig, len(cache)))
    return len(seen)


def check_cycle_pool():
    """The colourcycle pool lives in PAL3 entries the parallax must leave free.

    Reads the pool definition from tools/convert_stage.py and checks, for every
    stage that has colourcycle enabled, that the decomposed parallax art does
    not use those palette indices. Returns a list of problems.
    """
    import re
    from PIL import Image
    src = open(os.path.join(ROOT, "tools/convert_stage.py")).read()
    stages = re.search(r"^CYCLE_STAGES = \{([^}]*)\}", src, re.M)
    band = re.search(r"^CYCLE_BAND = \(([^)]*)\)", src, re.M)
    stable = re.search(r"^CYCLE_STABLE = (\d+)", src, re.M)
    if not (stages and band and stable):
        return []
    on = [int(x) for x in re.findall(r"\d+", stages.group(1))]
    pool = [int(x) for x in re.findall(r"\d+", band.group(1))] + [int(stable.group(1))]
    problems = []
    for st in on:
        for m in range(MAPS):
            p = os.path.join(ROOT, "res", f"stage{st}_bg{m}_tiles.png")
            if not os.path.exists(p):
                continue
            im = Image.open(p).convert("P")
            px = im.load()
            used = {px[x, y] for y in range(im.size[1]) for x in range(im.size[0])}
            clash = sorted(set(pool) & used)
            if clash:
                problems.append(f"stage{st} bg{m}: parallax uses colourcycle "
                                f"palette entries {clash} (pool is {pool})")
    print(f"check_vram: colourcycle pool {pool} free in stages {on}")
    return problems


def main():
    rom = os.path.join(ROOT, "out/release/rom.bin")
    sym = os.path.join(ROOT, "out/release/symbol.txt")
    if not (os.path.exists(rom) and os.path.exists(sym)):
        print("check_vram: no ROM yet (run make)", file=sys.stderr)
        return 0

    L = read_layout()
    ts = read_tileset_sizes(rom, sym)
    bg = read_bg_tiles()
    anim = read_anim_slots()
    links = read_links()
    # TILE_FONT_INDEX - SPRITE_VRAM_TILES (src/stage.h): the 30 tiles
    # below the pool hold the tooltip art, the dialog panel and the config tile.
    sprite_base = 1440 - 737

    def layout_end(s, m):
        """First free tile after tileset + bg + anim (runtime derives the same
        bases in stage_load; see src/stage.h)."""
        return (L["user"] + ts.get(s, {}).get(m, 0) + bg[s][m]
                + anim[s][m] * L["stride"])

    bad, tight, fallback = [], [], []
    for s in range(STAGES):
        for m in range(MAPS):
            anim_end = layout_end(s, m)
            if anim_end > sprite_base:
                bad.append(f"stage{s} map{m}: tileset+bg+anim end at {anim_end}, "
                           f"sprite pool starts at {sprite_base}")
            free = sprite_base - anim_end
            if free < 100:
                tight.append(f"stage{s} map{m}: only {free} tiles free for a room transition")
            # room-transition fallbacks: the destination window must fit in the
            # free block above the map that is currently loaded (`f`)
            for (mf, tx, ty, mt, dx, dy) in links[s]:
                for (f, t, dxx) in ((mf, mt, dx), (mt, mf, tx)):
                    need = window_tiles(s, t, dxx * 16 - 152)
                    room = sprite_base - layout_end(s, f)
                    if need > room:
                        fallback.append(
                            f"stage{s} map{f}->map{t} needs {need} tiles, has "
                            f"{room} -> falls back to the fade")

    bad += check_cycle_pool()

    for msg in sorted(set(bad)):
        print(f"VRAM OVERFLOW: {msg}")
    for msg in sorted(set(tight)):
        print(f"VRAM tight:    {msg}")
    for msg in sorted(set(fallback)):
        print(f"transition:    {msg}")
    if not bad and not tight:
        print(f"check_vram: OK ({STAGES} stages, {len(fallback)} transition fallbacks)")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
