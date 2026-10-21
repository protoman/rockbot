# RockBot → Mega Drive: conversion process

Decisions and verified facts. Update on every session.

Language: **English only** (code, comments, docs).

## Architecture decision

**Rewrite from scratch in C for SGDK.** Do not reuse the C++/SDL code.

Verified reasons:
- SGDK is pure C. Its headers have **no `extern "C"` guards** (only 2 of 36 in `inc/`, both irrelevant: `jsmn.h`, `stb_sprintf.h`) → C++ would fail to link due to name mangling.
- The MarsDev toolchain was built **without `g++`** (`m68k-toolchain.d` = target `without-newlib`, `--enable-languages=c`). No libstdc++, no newlib.
- `std::` appears 123× in the game, leaking through 22 headers.
- SDL surface: 62 `SDLL_*` wrappers + 186 `SDL_*`. Only 4 game files touch raw SDL; the rest is isolated in `sdl_layer.*` (1579 l).
- MD RAM = 64KB. Host STL + heap do not fit comfortably.

**Base code to adapt**: `sgdk/SGDK/sample/game/platformer` (896 l) — player with accel/decel, coyote time, jump buffer, ladders, one-way platforms; camera with deadzone; tile collision; MAP scroll. Exactly the skeleton of a Mega Man-like.
- Complementary: `sample/game/sonic` (1032 l) — 2-plane parallax, HUD, entities, `XGM2`, `SYS_setVBlankCallback`, `DMA_setBufferSize`.

## Emulator (verified working)

**Kega Fusion 3.63x** (flatpak, primary):
```bash
flatpak run --branch=stable --arch=x86_64 --command=Fusion --file-forwarding \
  com.carpeludum.KegaFusion @@ /path/to/rom.bin @@
```

**RetroArch + Genesis Plus GX** (alternative):
```bash
flatpak run --filesystem=/tmp org.libretro.RetroArch \
  -L "$HOME/.var/app/org.libretro.RetroArch/config/retroarch/cores/genesis_plus_gx_libretro.so" \
  /path/to/rom.bin
```
`--filesystem=/tmp` required (flatpak sandbox). Core downloaded from
`https://buildbot.libretro.com/nightly/linux/x86_64/latest/genesis_plus_gx_libretro.so.zip`
(the `stable/<ver>/` and `libretro-database/dist/` URLs are 404).

blastem standalone crashed on load — not investigated, low priority.

## Hardware constraints

| | PC (origin) | Mega Drive |
|---|---|---|
| Resolution | 320×240 | **320×224** (NTSC) |
| Tiles/screen | 20×15 | 20×14 |
| Color | 16bpp RGB | 9-bit 3:3:3, **4 palettes × 16 colors** |
| Sprites | unlimited software blit | 80 total, **20/line**, max 32×32 |
| Tilemap | software | 2 hardware planes (BG_A, BG_B) |
| Audio | SDL_mixer | YM2612 + PSG via XGM/XGM2 |
| Storage | HDD | ROM ≤ 4MB (mapper for more) |

**MAP_H = 15 → 14.** Loss of 16px (one tile row). See "Vertical adjustment" below.

**Text API**: `VDP_drawText(str, tileX, tileY)` — tiles, so `tile = pixel / 8`.
Screen in cells = 40×28.

## Asset pipeline (offline, not at runtime)

Principle: convert everything on the PC before the build. Zero runtime parsing, zero allocation.

```
assets_src/  (original PNGs + .dat)
    |  tools/convert_assets.py  (PIL)
    v
res/          (indexed PNGs, <=16 colors, index 0 = transparent)
    |  rescomp (.res -> TILESET/MAP/SPRITE/PALETTE/IMAGE)
    v
src/resources.h + resources.s
```

- **rescomp** (`$GDK/bin/rescomp.jar`) accepts **indexed** PNGs (preferred: exact palette control) and RGB (then it infers the palette from the top-left — fragile, avoid).
- Formats: `TILESET`, `MAP`, `IMAGE`, `SPRITE`, `PALETTE`, `BIN`, `XGM`/`XGM2`, `WAV`.
- rescomp rule: in RGB images the 4 sub-palettes go in separate tile rows at the top-left; never put tileset data to the right of the palette rows.
- Paths **inside a `.res` are relative to the `.res` directory**.

### Colorkey
RockBot's only transparent color is `(75,125,125)` = `0x4B7D7D` (`defines.h:961`).
The other colorkeys (`55,255,0`, `255,0,255`, `0,255,255`) were for colorcycle, which is disabled in the game — **ignore them**.

The converter must map that RGB to index 0, otherwise it renders as a solid color
(symptom: blue/box background behind a logo).

### Color quantization
MD = 512 possible colors (3 bits/channel). Original PNGs are RGBA/RGB/P.
`tools/convert_assets.py` rounds each channel to `(0,36,73,109,146,182,219,255)`
and quantizes with PIL median-cut when > `max_colors`.
External reference: https://rilden.github.io/tiledpalettequant/

### Vertical adjustment (240 → 224)
Options (decide per asset):
- **Crop**: cut 16px from the top (background) or from the ground. A 320×240 background → 320×208 + 16px solid color.
- **Shift**: move everything up 16px; ground at `MAP_H-2`/`MAP_H-1` becomes `MAP_H-3`/`MAP_H-2`.
- Recommended: **crop the background** (the ground is gameplay, the sky is decoration). For `MAP_H=15` maps, reindex rows y>=1 to y-1.

## Original game data formats (verified)

Files in `build/games/RockDroid1/`. `sizeof` measured by compiling the headers.

| File | Record | Size | Total |
|---|---|---|---|
| `stages_v301.dat` | `file_stage` | **438 B** | 8760 B (20 stages) |
| `maps_v2.dat` | `file_map_v2` | **15078 B** | 904680 B (60 maps) |
| `game_players_v301.dat` | `file_player_v3_1` | — | 17576 B |

`file_stage` offsets: `name[30]` 0, `bgmusic[30]` 30, `links[30]` 60 (10 B each), `boss` 360 (22 B), `tileset[30]` 382, `autoscroll[3]` 412.

`file_map_v2` offsets: `backgrounds[2]` 0 (36 B each = 72), `background_color` 72 (6 B, Sint16 r/g/b), **`tiles` 78**.

`map_tile` = **5 B**: `Sint8 locked; Sint8 tile1.x; Sint8 tile1.y; Sint8 tile3.y; Sint8 tile3.x`
- `tile1` = middle layer (behind player), `tile3` = layer over the player
- coords are **tile indices into the tileset PNG** (16px units), `-1` = empty
- `locked` → `TERRAIN_TYPES` (defines.h:285): 0=UNBLOCKED 1=SOLID 2=STAIR 3=DOOR 4=SPIKE 5=WATER 6=ICE 7=MOVE_LEFT 8=MOVE_RIGHT 9=QUICKSAND 10=SCROLL_LOCK 11=CHECKPOINT 12=EASYMODEBLOCK 13=HARDMODEBLOCK

Readers: `read_stage()` `file/file_io.cpp:428` (seek `438*stage_n`), `read_stage_maps_v2()` `:534` (seek `stage_id*3*15078`).

**Stage 0** = `INTRO_STAGE` = "CHAOS CITY", boss "MILONGA BOT", tileset `default.png`.
Map 0 of stage 0: bg0 `city_night.png` (320×96, adj_y=149, speed=5), bg color black, ground at y=11..12 (`locked=1`), decorative y=13..14.

### Tileset
`default.png` = **1552×160** = 97×10 tiles = **970 tiles**. At 4bpp = 31 KB. Fits VRAM (64KB) but tight with sprites — see VRAM budget.

## VRAM budget (planning)

VRAM = 64KB = 2048 tiles of 32B.
- 970 tileset tiles = 31040 B
- Plane A (64×32 entries × 2B) = 4096 B, Plane B same = 4096 B
- Sprite table (80 × 8B) = 640 B, HSR (80 × 2B) = 160 B
- Font = 96 tiles at `TILE_FONT_INDEX` (top of VRAM)
- Sprite engine region = 420 tiles by default (`SPR_init`), located just below the font
- Remaining for sprite tile data ≈ 22KB ≈ 700 tiles

Risk: many enemies + projectiles can overflow. Mitigations: put background tiles on BG_B with priority, load tiles per region, or shrink the tileset.

## Not porting (for now)

- **Multi-language**: English only. `file/fio_strings.cpp` (1412 l) holds PT/EN/ES/... — cut it, hardcode strings in C.
- Audio: all of `soundlib.cpp` (557 l) — stubs. Callers sprinkled in `game.cpp`, `sceneslib.cpp`.
- Zoom: `graphLib.zoom_image()` graphicslib.cpp:2341, single caller `sceneslib.cpp:112` (title logo) → pre-scale offline or just draw at scale 1.
- Rotate: `graphLib.rotate_image()` :2374, single caller `projectilelib.cpp:461` (diagonal projectiles) → pre-rotate into a sheet offline, or drop rotation.
- `editor/` (Qt), `android/`, `wasm/`, `ports/`.

## Text / font

PC: SDL_ttf + `fonts/pressstart2p.ttf`, `FONT_SIZE 8`.
MD: SGDK's built-in font via `VDP_drawText` (8×8, loaded into VRAM by `VDP_resetScreen`, uses **PAL0**).
- To use press-start: generate an 8×8 tileset with PIL and load it with `VDP_loadFont`, or use the `TEXT` engine.
- Keep PAL0 for the font; put game sprites on PAL1/PAL2/PAL3.

## Milestones (order)

1. **Intro (logos)** — DONE. `src/main.c`: upperland 6-frame strip, boberatu, jmd, text.
2. **Title + menu** — DONE (screen only). `logo.png` → `res/title_logo.png`, 320×224 with an
   8px CONFIG_BGCOLOR band; version/copyright/URL text. **Menu not implemented yet** — START
   currently jumps straight to the stage.
3. **Intro stage (CHAOS CITY)** — DONE. `tools/convert_stage.py`, `tools/convert_player.py`,
   `src/stage.c`, `src/player.c`. Map scrolls, player walks/jumps/collides.
   Still missing: ladders (terrain 2), enemies, projectiles, tile3 layer drawn over the
   player (baked into the map instead), animated tiles, the parallax `city_night.png`
   background (it is baked into the map tiles already, so no separate plane yet).
4. Enemies + projectiles.
5. Audio (XGM2).
6. Main menu (New/Load/Config/About), save files.

## Stage data path (what exists now)

```
tools/convert_stage.py
  stages_v301.dat[0]  -> stage metadata
  maps_v2.dat[0]      -> 200x15 grid: terrain + tile1/tile3 coords
  tilesets/default.png-> only the 40 tiles the map uses (12 colors, no loss)
  ->
  res/stage0_map.png  (3200x240 indexed, same palette for MAP + TILESET)
  src/stage0_data.c/h (u8 terrain[200*15], STAGE0_TERRAIN(x,y))

tools/convert_player.py
  sprites/p1.png (32 frames of 29x29)
  + weapon_colors[0] palette swap baked in (green->(143,0,119), magenta->(0,115,239))
  + re-order into SGDK animation rows, pad to 32x32
  ->
  res/player.png (128x192, 4 cols x 6 anim rows)
```

`.res` (see `res/resources.res`): the stage TILESET and MAP **must come from the same
image** or rescomp fails to resolve tile [0,0] (see lessons_learned.md #18).

Runtime: `src/stage.c` (`stage_load`, `stage_terrain`, `stage_update_camera`) +
`src/player.c` (`player_update`: accel/decel, coyote time, jump buffer, early-release
jump cut, AABB tile collision).

### Physics constants (player.c)
Tuned for fixed 60fps; the original scales by `fps_movement_multiplier = 60/max_fps`.

| constant | value | note |
|---|---|---|
| MOVE_SPEED | FIX16(2) | matches `file_player_v3_1.move_speed = 2` |
| ACCEL / DECEL | 0.35 / 0.28 | |
| GRAVITY | 0.25 | |
| JUMP_VEL | 5.3 | ~56px arc ≈ 3.5 tiles |
| MAX_FALL | 6 | |

### Player hitbox
Source `sprite_hit_area` = (4, 8, 21, 24) but the bottom exceeded the 29px art.
Used instead: **left 6, top 4, right 25, bottom 29** — feet land exactly on the
sprite's bottom edge (`PLAYER_BOX_*` in `src/player.h`).
