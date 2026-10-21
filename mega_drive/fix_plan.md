# Fix Plan: Game-loop Palette Problems

> [!IMPORTANT] Fix in order. FIX-01 unblocks visibility. FIX-02 fixes debug. FIX-03 is the big structural fix.

---

## FIX-01 — Resolve PAL0 conflict (font vs stage) `[30 min]`

**Addresses**: FIND-01

**Strategy**: Move the stage to **PAL1**, free PAL0 for the font exclusively.  
The player is already on PAL1 — move it to **PAL2** instead.  
This requires 3 mechanical changes:

| Symbol | Old | New |
|--------|-----|-----|
| `STAGE_PAL` in `stage.h` | `PAL0` | `PAL1` |
| `PLAYER_PAL` in `player.h` | `PAL1` | `PAL2` |
| Font palette init (`main.c:268`) | keep on `PAL0` | keep, no change |

**Steps**:
1. `stage.h:10` → `#define STAGE_PAL PAL1`
2. `player.h:13` → `#define PLAYER_PAL PAL2`
3. In `stage.c:43-44`: `MAP_create(... TILE_ATTR_FULL(STAGE_PAL ...))` — no change needed (uses the define).
4. Rebuild & verify: map tiles render in correct colors, font visible (white/grey).

> [!NOTE] PAL3 remains available for the debug overlay (already used). PAL0 = font only.

---

## FIX-02 — Fix debug block palette line `[5 min]`

**Addresses**: FIND-02

**Change** in [`main.c:224`](file:///home/iuri/Desenvolvimento/rockbot/mega_drive/src/main.c#L224):

```c
// BEFORE
TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, DBG_TILE)
// AFTER
TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, DBG_TILE)
```

`dbg_pal` is loaded to PAL3; the tile must reference PAL3 so the purple color (index 1) is used.

---

## FIX-03 — Unify tileset palette across all maps `[2-3 hours]`

**Addresses**: FIND-05 (and partially FIND-03)

**Problem**: each map has its own auto-generated palette with colors in arbitrary order, causing color flashes on map transitions and making a shared HUD palette impossible.

**Strategy**: generate one master palette from ALL maps combined, then re-index all maps against it.

**Changes to `tools/convert_stage.py`**:

1. **Collect all colors across all maps before writing any PNG**:
   ```python
   all_rgbs = []
   for m in range(MAPS_PER_STAGE):
       all_rgbs += collect_map_colors(stage, m, ts_px)  # extract, don't render
   master_palette, master_lut = build_palette(all_rgbs)  # one shared palette
   ```

2. **Render each map with `master_palette`/`master_lut`** (not per-map).

3. **All map PNGs share identical palette** → `stage0_pal0`, `stage0_pal1`, `stage0_pal2` all contain the same data. SGDK `PAL_setPalette` calls on map load become no-ops (same data already in CRAM).

4. If 15 color slots are insufficient for the combined set (both maps use the same tileset `default.png`, so overlap is high — should be fine), merge similar colors using nearest-neighbor in MD color space.

**Verification**: diff the `stage0_pal*.data` arrays in the built `.h` — they must be byte-identical.

> [!WARNING] After FIX-03, rebuild assets: `python3 tools/convert_stage.py` then `make`. Do not commit the PNG files without the accompanying data rebuild.

---

## FIX-04 — Clear PAL2 on stage entry `[5 min]`

**Addresses**: FIND-03

After FIX-01 (player moves to PAL2), PAL2 is now used.  
But if FIND-03 is not done and PAL2 still has stale intro data at entry, add:

```c
// runStage(), before stage_load():
static const u16 black_pal[16] = {0};
PAL_setPalette(PAL2, black_pal, DMA_QUEUE);  // clear stale intro colors
```

After PAL2 is assigned to the player (`player_init()` loads `player_sprite.palette`), this is overwritten anyway — so this is just a guard for the frames before `player_init()`.

---

## FIX-05 — Fix corrupted background tiles on stage start `[15 min]`

**Addresses**: FIND-07

**Problem**: On stage boot, the DMA queue overflows the 7,200-byte per-frame cap (8,160 bytes queued), dropping trailing tiles and leaving stale `title_logo` tiles (128..383) visible on the screen.

**Steps**:
1. **Disable display during transition**:
   Turn off the VDP display (`VDP_setEnable(FALSE)`) when leaving `showTitle()` before `stage_load()`.
2. **Clear user VRAM completely**:
   Use `VDP_clearPlane(BG_A, TRUE)` and clear user tile VRAM slots (`TILE_USER_INDEX` through 512) so no title screen tiles linger.
3. **Raise per-frame DMA transfer capacity during stage load**:
   In `main()` or before `stage_load()`:
   ```c
   DMA_setMaxTransferSize(0); // 0 = unlimited capacity in SGDK (transfers all queued data)
   ```
   Or load tiles synchronously via CPU/DMA while the screen is blanked:
   ```c
   VDP_loadTileSet(tile_sets[map_index], TILE_USER_INDEX, DMA); // immediate DMA with display off
   ```
4. **Flush DMA and re-enable display**:
   ```c
   SYS_doVBlankProcess();
   VDP_setEnable(TRUE);
   ```

---

## FIX-06 — Fix white screen background `[5 min]`

**Addresses**: FIND-08

1. In `src/main.c`:
   - Set backdrop color index explicitly: `VDP_setBackgroundColor(0);`.
   - Ensure `PAL0[0] = 0x0000` (black) and `PAL0[15] = 0x0EEE` (white text) so text is white on black.
2. In `runStage()`:
   - Remove `VDP_fillTileMapRect(BG_B, ... DBG_TILE)` so BG_B remains clean black behind the sky instead of drawing the test block.

---

## FIX-07 — Fix player collision and "bounce back" `[20 min]`

**Addresses**: FIND-09

1. **Fix horizontal wall snap**:
   In `src/player.c:215`:
   - Replace the aggressive snapping equation with exact edge contact (`nx = (tx * 16) - PLAYER_BOX_R`) so the player does not get pushed backward into the preceding tile.
2. **Fix falling collision row check**:
   In `src/player.c:239`:
   - Start scan from `row = (iy + PLAYER_BOX_B) >> 4` instead of `+ 1` to prevent tunneling when feet already enter the tile boundary.
3. **Align terrain conversion for Stage 0 Map 0**:
   - Check if rows 7–8 at x=59..67 in `maps_v2.dat` are intended as solid blocks or if easy/hard mode masks (`TERRAIN_EASYBLOCK`/`TERRAIN_HARDBLOCK`) were mapped to `SOLID` during `convert_stage.py`.
   - Re-run `python3 tools/convert_stage.py` and `make`.

---

*Written: 2026-10-14. Status: ready for implementation.*
