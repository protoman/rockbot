# Findings: Game-loop Palette & Asset Issues

> [!NOTE] Scope: game-loop (`runStage()`) only. Intro/title palette issues are separate (BUG-001/002).

---

## FIND-01 — Stage overwrites font palette (PAL0 conflict)

**Severity**: High — debug text invisible, any future HUD text broken.

**Root cause**: `VDP_drawText` uses `text_basetile` which defaults to **PAL0**.  
`stage_load()` calls `PAL_setPalette(STAGE_PAL=PAL0, ...)` — overwriting the grey font palette set in `main()` line 268.

After `stage_load()`:
- PAL0[1] = `(146,146,255)` light blue (stage tile color)
- PAL0[15] = `(0,0,0)` black — the font's foreground color

**Effect**: font renders in stage colors; foreground (index 15) is black on black = **invisible text**.

**Files**: [`stage.h:10`](file:///home/iuri/Desenvolvimento/rockbot/mega_drive/src/stage.h#L10), [`stage.c:39`](file:///home/iuri/Desenvolvimento/rockbot/mega_drive/src/stage.c#L39), [`main.c:268`](file:///home/iuri/Desenvolvimento/rockbot/mega_drive/src/main.c#L268)

---

## FIND-02 — Debug block uses wrong palette line

**Severity**: Medium — debug overlay wrong color, misleading.

**Root cause**: [`main.c:224`](file:///home/iuri/Desenvolvimento/rockbot/mega_drive/src/main.c#L224):
```c
VDP_fillTileMapRect(BG_B, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, DBG_TILE), 0, 0, 20, 14);
```
`dbg_solid_tile` is colored by **PAL3** (`dbg_pal`), but `TILE_ATTR_FULL` specifies **PAL0** — the stage palette.  
The debug block renders in stage color[1] = light blue, not the intended purple (PAL3[1] = `0x01C7`).  
`PAL3` is loaded correctly (`DMA_QUEUE`), but the tilemap attribute points to the wrong line.

**Fix**: change `PAL0` → `PAL3` in `TILE_ATTR_FULL(...)` for the debug fillTileMapRect.

---

## FIND-03 — PAL2 uninitialized in game loop

**Severity**: Medium — any tiles/sprites referencing PAL2 render with garbage intro colors.

**Root cause**: PAL2 is set in `showIntro()` for `boberatu_sprite` and never cleared or reassigned before `runStage()`. CRAM entry for PAL2 retains intro data (or title image data). No game-loop asset uses PAL2 intentionally, but any VRAM tile that still has PAL2 bits from the cleared title screen could show unexpected colors.

**Files**: [`main.c:15,71`](file:///home/iuri/Desenvolvimento/rockbot/mega_drive/src/main.c#L15)

---

## FIND-04 — Player palette index 1 = black (duplicate of transparent)

**Severity**: Low — visually works, but fragile.

The player palette has:
- Index 0 = `(0,0,0)` — transparent for sprites ✓
- Index 1 = `(0,0,0)` — same value, used for **1716 solid pixels** (dark outlines)

`convert_player.py` builds the palette by prepending `(0,0,0)` as index 0, then inserting unique MD-rounded colors. The player art contains genuine `(0,0,0)` pixels which md_round maps to `(0,0,0)`, entering the palette as index 1. They render as solid black — visually correct for outlines — but index 0 and index 1 are visually identical, wasting a palette slot.

**Files**: [`tools/convert_player.py:87-90`](file:///home/iuri/Desenvolvimento/rockbot/mega_drive/tools/convert_player.py#L87)

---

## FIND-05 — stage map PNG palette varies per map (no shared tileset palette)

**Severity**: High — when switching maps (`stage_load(1)`), PAL0 changes, causing all on-screen tiles to momentarily flash the new palette.

**Root cause**: Each map generates its own independent palette (`build_palette()` in `convert_stage.py`). Map0 and Map1 have **different color orders** at the same indices:

| Index | Map 0 | Map 1 |
|-------|-------|-------|
| 1 | `(146,146,255)` | `(146,146,255)` ✓ |
| 2 | `(255,255,255)` | `(73,36,0)` ✗ |
| 3 | `(73,36,0)` | `(255,255,255)` ✗ |

Tiles that share index numbers across maps will appear with swapped colors after the palette update.

**Files**: [`tools/convert_stage.py:104-114`](file:///home/iuri/Desenvolvimento/rockbot/mega_drive/tools/convert_stage.py#L104)

---

---

## FIND-06 — "Invisible wall" — real SOLID step/ledge at tile x=45

**Severity**: High — player blocked by what appears to be empty space.

**Root cause**:
1. **Actual collision**: In `maps_v2.dat` (Map 0), row `y=11` is ground (`SOLID`). At tile `x=45` (`pixel x=720`), row `y=10` becomes `SOLID` (`locked=1`, `tile1=(46,6)`).
2. **Player box check**: Player standing on row 11 has hitbox `[top=151, bottom=176]`. `y0 = 151 >> 4 = 9`, `y1 = (176 - 1) >> 4 = 10`.
3. Horizontal check tests `y0..y1` (rows 9 and 10). Row 10 is `SOLID` starting at `x=45`. Player hits this step at `world x=695` and stops dead.
4. **Visual invisibility**: Due to FIND-01/palette corruption or transparent tile rendering (color 0), the tiles at `y=10` render black/transparent (matching sky/background), making a real 1-tile step look completely invisible. The player must jump over it, but cannot see it.

---

## FIND-07 — Game background tiles corrupted / scrambled on stage start

**Severity**: Critical — stage background displays scrambled/garbage/title-screen tiles.

**Root causes**:
1. **DMA Queue overflow on frame 0 (`DMA_TRANSFER_CAPACITY_NTSC`)**:
   - In `stage_load(0)`, `VDP_loadTileSet` queues 127 tiles = **4,064 bytes**.
   - Immediately following, `MAP_scrollToEx(stage_map, 0, 0, TRUE)` queues full screen row tilemap updates: 16 metatile rows = **4,096 bytes**.
   - Total queued DMA in that single frame = **8,160 bytes**.
   - Default SGDK NTSC transfer budget (`DMA_TRANSFER_CAPACITY_NTSC`) is **7,200 bytes**.
   - Because `flag & DMA_OVERCAPACITY_IGNORE` is active when limit is hit, SGDK drops/ignores the trailing **960 bytes** of tile/map transfers.
2. **Title screen tiles lingering in VRAM**:
   - `showTitle()` loads `title_logo_tileset` (383 tiles) into VRAM starting at `TILE_USER_INDEX`.
   - `stage_load()` loads `stage0_tiles0` (127 tiles) starting at `TILE_USER_INDEX`.
   - Because tiles 128..383 from `title_logo` are never cleared from VRAM, and the trailing ~960 bytes of new tile transfers are dropped by DMA capacity limits, any map cells referencing unwritten or dropped tile slots display stale title screen graphics.
3. **No VBlank wait after `stage_load()` before displaying/drawing**:
   - `stage_load(0)` uses `DMA_QUEUE` to load tiles and palettes, but does not call `SYS_doVBlankProcess()` to flush the transfers into VRAM before starting the game loop. The map begins drawing before tile data has arrived in VRAM.

**Files**: [`src/stage.c:35-48`](file:///home/iuri/Desenvolvimento/rockbot/mega_drive/src/stage.c#L35), [`src/main.c:160-176`](file:///home/iuri/Desenvolvimento/rockbot/mega_drive/src/main.c#L160)

---

## FIND-08 — White screen background in intro & stage

**Severity**: Medium — visual polish / contrast issue.

**Root cause**:
1. VDP border/backdrop color defaults to index 0 of Palette 0. In `main.c`, `PAL0` is assigned `palette_grey` (where non-zero entries are bright), and `VDP_setBackgroundColor(0)` is never explicitly configured with a black palette entry.
2. The debug code in `runStage()` fills `BG_B` with `DBG_TILE` using `PAL3` (or previously `PAL0`), which renders across `20x14` tiles on the plane right behind the transparent sky, covering the backdrop with color.

---

## FIND-09 — Player "bounces back" / cannot pass counter 0032

**Severity**: Critical — gameplay progression blocked.

**Root cause**:
1. **Horizontal collision snap (`player.c:215`)**:
   ```c
   if (dx > 0) nx = ((r >> 4) << 4) - PLAYER_BOX_R;
   ```
   When `nx` enters a solid tile, `((r >> 4) << 4)` calculates the left edge of that tile and subtracts `PLAYER_BOX_R`. If `PLAYER_BOX_R` (25) > 16, subtracting 25 pulls the player **further back than 1 tile** (snaps back by 9 pixels into the previous tile), causing an abrupt, visible "bounce back".
2. **Falling collision row check starts too late (`player.c:239`)**:
   ```c
   for (s16 row = ((iy + PLAYER_BOX_B) >> 4) + 1; ...
   ```
   If player jumps and falls slightly while overlapping a tile horizontally, starting at `+ 1` skips the tile row right beneath their feet when already slightly inside it, causing vertical clipping or snapping against vertical ledges.
3. **Terrain discrepancy / Ceiling collision at tile 60–68**:
   In `stage0_data.c`, rows 7 and 8 have `SOLID` tiles at x=59..67. When jumping over the lower step, the jump arc hits the overhead solid blocks (`row 8`), abruptly zeroing vertical speed (`player.vy = 0`), which drops the player back onto the horizontal wall, where the horizontal snap pushes them backward again.

---

*Written: 2026-10-14. Status: pending fixes.*
