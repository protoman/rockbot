# AGENTS.md — Port RockBot → Sega Mega Drive

Working dir: `mega_drive/` (SGDK/MarsDev project). Game source: **parent dir** `/home/iuri/Desenvolvimento/rockbot`.

Language: **English** for all code, comments and docs.

## SDK (already installed, do not rebuild)

- SDK root: `/mnt/ExtraData/Devel/SDK/MegaDrive/`
  - `mars/` — install dir (GCC 15.2.0 m68k-elf + SGDK libmd rev `cb114aca` = v2.11+118)
  - `marsdev/` — build repo (`examples/`, SGDK clone at `sgdk/SGDK/`)
- Env: `source /mnt/ExtraData/Devel/SDK/MegaDrive/mars/mars.sh` (already in `~/.bashrc`)
  - exports `MARSDEV`, `GDK=$MARSDEV/m68k-elf`, adds `$GDK/bin` to PATH
- Build this project: `cd mega_drive && make` → `out/rom.bin` (128K, padded)
  - `make debug` / `make clean`. Makefile sets `GDK ?=` then includes `$GDK/makefile.gen`.
- Templates: `marsdev/examples/sgdk-skeleton` (origin of this project), `sgdk/SGDK/sample/` (official)
- Known-fixed template bug: removed orphan `src/boot/rom_head.c` (conflicted with `sega.o`) and `main()` → `main(bool hardReset)`. Do not re-add `rom_head.c`.

## Hard constraints (Mega Drive)

| | PC (source) | Mega Drive |
|---|---|---|
| Resolution | 320×240 (`RES_W/RES_H` defines.h:6-7) | 320×224 NTSC (`VDP_setScreenHeight224`) |
| Color depth | 16bpp RGB (`VIDEO_MODE_COLORS 16` defines.h:1037) | 9-bit (3:3:3), 4 palettes × 16 colors |
| Tiles/screen | 20×15 | 20×14 (40×28 in 8×8 cells) |
| Sprites | unlimited software blit | 80 total, 20/line, max 32×32 each |
| CPU | x86 | 68000 @ 7.67 MHz |
| Storage | HDD | ROM ≤ 4MB (mapper for more) |

**Text API is tile-based**: `VDP_drawText(str, tileX, tileY)`, `tile = pixel / 8`.

## Game structure (source map)

**Entry/flow**
- `main.cpp:318 main()` → `:527 while(run_game) main_loop()`; `main_loop()` `:307` = `gameControl.show_game(true,true)` + `draw_lib.update_screen()`
- `game.cpp:133 game::show_game()` — per frame: input → scroll → `loaded_stage.show_stage()` → player → HUD → `fps_manager.limit()` → flush
- 60 FPS, `DEFAULT_FPS_MAX 60` defines.h:26; movement normalized by `fps_movement_multiplier = 60/max_fps`

**Intro sequence** — `game.cpp:429 show_game_intro()`, exact call order:
1. `show_notice()` `game.cpp:506` — logos. Images from `GAMEPATH/shared/images/`: `upperland.png` (6-frame strip, `width/6` per frame, 30ms/frame), `boberatu.png` (100×40), `jmd_logo.png` (95×32). Waits 400ms → anim → 3200ms.
2. `scenes.preloadScenes()` `sceneslib.cpp:77` → loads `scenes/*.dat`
3. `scenes.game_scenes_show_unbeaten_intro()` `sceneslib.cpp:700` → cutscene engine `scenes/sceneshow.cpp`
4. `scenes.main_screen()` `sceneslib.cpp:128` → `draw_main()` `:102` loads `images/logo.png` (320×240) and calls `graphLib.zoom_image()` — **only zoom in UI** — then menu (New/Load/Config/About)
5. `initGame()` `game.cpp:111`
6. `scenes.pick_stage(INTRO_STAGE)` `sceneslib.cpp:529` → returns **0** while intro unbeaten
7. `scenes.boss_intro()` `sceneslib.cpp:634`
8. `start_stage()` `game.cpp:251`

**Intro stage** = `INTRO_STAGE` = index 0 in `STAGE_LIST` (defines.h:39-61). Name in data: **"CHAOS CITY"**, boss **MILONGA BOT**, bgm `rockbot_chaos_city.mod`, tileset `default.png`.

**Data files** — `build/games/RockDroid1/`, read by `file/file_io.cpp`:
- `stages_v301.dat` — 20 × `sizeof(file_stage)=438` = 8760 B. `read_stage()` `file_io.cpp:428` seeks `438*stage_n`.
  - layout: name[30], bgmusic[30], links[30]×10=300 (off 60), boss off 360 (22 B), tileset[30] off 382, autoscroll[3] off 412
- `maps_v2.dat` — 60 maps (20 stages × 3) × `sizeof(file_map_v2)=15078` = 904680 B.
  - `read_stage_maps_v2()` `file_io.cpp:534` seeks `stage_id * 3 * 15078`
  - layout: `backgrounds[2]` (36 B each = 72), `background_color` (6 B, Sint16 r/g/b) → **tiles at offset 78**
  - `map_tile` = 5 B: `Sint8 locked; Sint8 tile1.x,y; Sint8 tile3.y,x` — tile1 = mid layer, tile3 = over player
  - tile coords are **tile indices into the tileset PNG** (16px units), `-1` = empty
  - `tiles[MAP_W=200][MAP_H=15]`
- Stage 0 map 0: bg0 `city_night.png` (320×96, adj_y=149, speed=5), bgcolor black, ground rows y=11..12 (`locked=1`), decorative y=13..14
- `map_tile.locked` → `TERRAIN_TYPES` defines.h:285 (`0=UNBLOCKED, 1=SOLID, 2=STAIR, 3=DOOR, 4=SPIKE, 5=WATER, 6=ICE, ...`)

**Colorkey**: `(75,125,125)` = `0x4B7D7D` defines.h:961 is **the only transparent colorkey**. The others (`55,255,0`, `255,0,255`, `0,255,255`, defines.h:1039-1049) were for colorcycle, disabled in the game — ignore.

**Graphics**
- `graphicslib.cpp` (2557 l) — seam class; `sdl_layer.cpp/h` wraps SDL via `SDLL_*` (graphicslib.h:21 says "clone this class for non-SDL ports")
- Backbuffer `graphLib.gameScreen` 320×240; `updateScreen()` :255; blits `copyArea` :472 / `copySDLPortion` :433
- Text = SDL_ttf `fonts/pressstart2p.ttf`, `FONT_SIZE 8` defines.h:927 → needs a bitmap font port
- **Zoom/rotate to drop**: `graphLib.zoom_image()` graphicslib.cpp:2341 ← only caller `sceneslib.cpp:112` (title logo); `graphLib.rotate_image()` :2374 ← only caller `projectilelib.cpp:461` (diagonal projectiles)

**Sprites**
- Sheet `images/sprites/p1.png` = 928×29 → frame size **29×29** (from `file_player_v3_1.sprite_size`, `file/v4/file_game_v4.h:295`)
- Frame registration `classplayer.cpp:545 initFrames()`: `addSpriteFrame(anim_type, frame_index_in_sheet, sheet, delay_ms)`
  - STAND 3,4 / WALK 7,8,7,6 (150ms) / JUMP 9 / ATTACK 11 / JUMP_ATTACK 10 / WALK_ATTACK 12,13,14 / HIT 15,16 / TELEPORT 0,1,2 / STAIRS 17,18 / SLIDE 20 / SHIELD 22 / DIAG_UP 23 / DIAG_DOWN 24 / GOT_WEAPON 26,27 / SPECIAL 29
- `character::addSpriteFrame()` `character.cpp:2345` slices `posX*frameW × frameH` and pre-generates the LEFT copy via `flip_image()`
- Anims enum `ANIM_TYPE_*` defines.h:174+; `ANIM_FRAMES_COUNT 10`, dirs = 2 (L/R)

**Map/stage render**: `stage.cpp` (`load_stage` :65, `show_stage` :95) → `classmap.cpp` (`show_map` :183, `draw_map_tiles` :239, `draw_animated_tiles` :283, backgrounds :555/:634, fg layer :643)

**Disable for now (audio)**
- `soundlib.cpp` (557 l) — `init_audio_system` :31, `load_all_sfx` :112, `Mix_LoadMUS` :272, `Mix_PlayMusic` :336
- Callers sprinkled in `game.cpp`, `sceneslib.cpp` (`stop_music`/`load_music`/`play_music`) — stub them

**Ignore**: `editor/` (Qt), `android/`, `wasm/`, `ports/`, `build/packages/`, `aux_tools/PasswordGenerator`

**Size**: game = 117 .cpp/.h ≈ 43k lines. Hot: `character.cpp` 3575, `artificial_inteligence.cpp` 2643, `graphicslib.cpp` 2557, `classmap.cpp` 2490, `game.cpp` 2320

## C++ → C reality

SGDK is **C only**. Verified: SGDK headers have **no `extern "C"` guards**; the MarsDev toolchain was built without `g++`. The port is **C, from scratch** — see `conversion_process.md`.

## Base code to copy

`marsdev/sgdk/SGDK/sample/game/platformer/` (896 l) — platformer skeleton:
- `player.c` — `fix16` physics, accel/decel, coyote time, jump buffer, ladders, one-way platforms
- `camera.c` — deadzone + `MAP_scrollTo`
- `levelgenerator.c` — collision map in RAM, `getTileValue(x,y)`
- `physics.c` — tile↔pixel helpers
- `global.h` — `TILEMAP_PLANE BG_A`, `GROUND_TILE 1`, `LADDER_TILE 2`, `ONE_WAY_PLATFORM_TILE 4`

Complementary: `sample/game/sonic/` (1032 l) — 2-plane parallax, HUD, entities, `XGM2`, `SYS_setVBlankCallback`, `DMA_setBufferSize`.

## Emulator (verified)

```bash
flatpak run --branch=stable --arch=x86_64 --command=Fusion --file-forwarding \
  com.carpeludum.KegaFusion @@ <rom.bin> @@
```
Kega Fusion 3.63x (flatpak). RetroArch + Genesis Plus GX also works but needs `--filesystem=/tmp` for ROMs under /tmp.

## Port status

- Milestone 1: intro logos (`src/main.c` `showIntro()`).
- Milestone 2: title screen (`showTitle()`). Menu **not** done — START skips straight to the stage.
- Milestone 3: **intro stage CHAOS CITY** — map scroll, player walk/jump/collision.

Flow in `main.c`: `showIntro()` → `showTitle()` (waits for START) → `runStage()`.

**Files**

| file | role |
|---|---|
| `src/main.c` | boot, intro, title, stage loop |
| `src/stage.{c,h}` | `stage_load`, `stage_terrain(x,y)`, `stage_update_camera` |
| `src/stage0_data.{c,h}` | generated `u8 terrain[200*15]` + `STAGE0_TERRAIN(x,y)` |
| `src/player.{c,h}` | physics, AABB collision, sprite. `PLAYER_BOX_*`, `ANIM_*` |
| `tools/convert_assets.py` | logos → indexed PNG |
| `tools/convert_stage.py` | stage/map data → `res/stage0_map.png` + `src/stage0_data.*` |
| `tools/convert_player.py` | `p1.png` → `res/player.png` (anim rows, palette swap) |
| `res/resources.res` | rescomp manifest |

**Rebuild assets then ROM**
```
/home/iuri/python3/bin/python3 tools/convert_assets.py
/home/iuri/python3/bin/python3 tools/convert_stage.py
/home/iuri/python3/bin/python3 tools/convert_player.py
make
```

**Controls in Kega** (`Fusion.ini` `Player1Keys`): arrows = D-pad, **C/X/Z** = A/B/C, **Return** = Start.

**Not implemented**: stairs (terrain 2) beyond basic ladder climbing, doors, spikes,
water, ice, enemies, projectiles, tile3 layer drawn over the player (baked into the
map for now), animated tiles, the parallax `city_night.png` plane (baked into map
tiles), HUD, audio, menu, saves.

**Stage 0 progression (verified from data, NOT guesswork)**
```
map 0: walk right -> ladder at tile x=136 (STAIR, rows 11-14) -> climb down
       link2: map 0 (136,14) -> map 1 (198,0)
map 1: appear at top-right ladder (198,0), descend, traverse left
       link3: map 1 (1,0) -> map 0 (139,14)
map 0: appear at ladder x=139 (rows 9-14), climb up to the platform at rows 9-10
```
The full-height SOLID column at x=137..138 of map 0 is a **map divider** — it is
not climbable and must not be "fixed". Positions in `file_link` are **unsigned
bytes**; the PC data stores e.g. -120 meaning 136.
