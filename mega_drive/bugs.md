## BUG-044 follow-up — CRAM write race: fixed

Every CPU-method CRAM write now goes through `src/pal_safe.h`
(`pal_load`/`pal_load_n`/`pal_color`): wait the DMA out, keep interrupts off for
the whole address-write + data-words sequence, then write. `tools/check_pal_writes.py`
(`make check-pal`) fails if a bare `PAL_set*(.., CPU)` reappears in `src/*.c`.
`showIntro()` also re-asserts PAL0 = `pal_ui` before it draws, so a boot where
CRAM came back shifted can no longer be visible.

Verified with the CRAM readback (log-only run, `-DMD_TILE_DUMP`): intro frame 0
reads `0000 0000 0EEE 0EC8 08AA 048E 0E08 006E 0666 0880 002E 002C 0622 0260
0024 0CCC`, which is `pal_ui` word for word, and it is unchanged 24 frames later.
Before the fix the same dump returned the boot grey ramp shifted by 8 or 10
entries, a different offset per boot.

Why it was needed: a CPU-method palette write is a VDP address-port write
followed by data words, and the boot CRAM readback showed the grey ramp sitting
at entries 8..15 on one boot and 10..15 on the next - the destination slid by a
different amount each time, which is a redirection, not a torn frame. The same
mechanism can drop CRAM data into VRAM at whatever address the VDP last held.

## BUG-044 follow-up — the stripes are NOT the fill tiles (open)

Measured on the emulator with `EXTRA_FLAGS=-DMD_TILE_DUMP` (`src/tdump.c`,
registers + CRAM readback through kprintf, log only, no screenshots):

    intro: reg11=00 (plane scroll, NOT per-column VScroll), reg12=81 (H40),
           reg17=reg18=00 (window plane off), BG_A=E000 BG_B=C000,
           text plane=BG_A, text palette=PAL0
    intro PAL0: 0C00 0E00 x9 then 0000 0222 0444 0666 0888 0AAA   (run 1)
    intro PAL0: 0E00 x8 then 0000 0222 ... 0CCC 0EEE              (run 2)

So CRAM is corrupted at boot and the offset MOVES between runs (ramp at entries
10..15 one boot, 8..15 the next): a palette transfer whose destination address
is redirected mid-way, i.e. the FIND-08 class but in the boot path, before any
of our own palette calls. Making our CPU-method CRAM writes CRAM-safe
(src/pal_safe.h: wait for DMA + interrupts off) did not change it.

Fixed by that change and verified by the reporter: the stripes that ran through
TEXT are gone. Still striped after the change: intro, CONFIG, room transition.
The stage playfield is clean (screenshots/bugs/items_after_menu.png), and both
CONFIG and the transition band point their cells at the shared tile 702 while
the stage playfield points at the stage tileset - so the next measurement must
be the tile data itself.

Attempted and reverted: reading VRAM from the 68k (control-port address +
data-port words) to dump tiles 0/702 - it faults the ROM (address error at
0x318CE right after the first tile read). Do it with the display disabled at a
known point, or with the emulator debugger, not from a live frame.

## BUG-044 — white stripes: the "solid black" fill tiles were stripe patterns

Symptom: vertical white (or lightest-palette) stripes, whole-screen on the
options screen and the opening cutscene, and over the stage's parallax sky and
the room-transition band fills — reported as "white stripes/spread, visible
during the stairs/ladder transitions but broader than that".

Cause: Mega Drive 4bpp tile data holds one byte per **bitplane** in each row, so
a fill tile must be `0xFF000000` per row for a solid palette index 1. Both
hand-written "black" tiles used `0x11111111`, which decodes to pixel index 0xF
(15) on pixels 3 and 7 only — two vertical bars per tile, and index 15 is the
lightest entry of *any* palette (white in PAL0). The comments even said "filled
with palette index 1", which is what the author meant:

    src/scene.c  black_tile[8]  -> cutscene BG_B fill        (stripes)
    src/config.c cfg_black[8]   -> options screen BG_A fill  (stripes)

They spread because config loads its fill at `SPR_CFG_BLACK_TILE` (702), which
is the same index `stage_sky_tile()` returns: after a visit to the options
screen (title CONFIG or the in-game menu) every parallax row the bg art does not
cover, and every band the transition writes, pointed at an opaque striped tile
until the next map load. `bg_upload_tiles_maps()` only reloaded the blank sky
tile when the map had parallax art, and only on a map load.

Reproduction (offline, fails before the fix): `tools/check_fill_tiles.py`
(`make check-fills`) decodes every `static const u32 NAME[8]` tile literal in
`src/*.c` and requires one value on every pixel. Pre-fix output:

    config.c: cfg_black is not a solid fill
        0 0 0 F 0 0 0 F   ... (8 rows)
    scene.c: black_tile is not a solid fill
    2 fill tile(s) are accidental patterns (BUG-044)        exit 1

Fix: both constants are now `{0xFF000000, x8}`; `config.c:clear_screen()` sets
`PAL_setColor(1, 0x0000)` because the screen inherits PAL0 from whoever opened
it (title line 1 is white, the stage line 1 is a stage colour); and
`stage.c:bg_load()` reloads the blank sky tile at 702 on EVERY map load,
before the "no parallax art" early return.

Evidence caveat: the frames in `screenshots/bugs/stripes/` are 642x471 and the
ones in `screenshots/bugs/transition/` 644x476 — `tools/check_captures.py`
rejects both batches as non-game window sizes, so they are not used as proof
here; the tile decode above is. Stripes still unexplained by this bug: the
full-screen ones behind the intro logos (001/002) and the player-select band
(003), whose cells point at tiles 16..48, not 702.

Emulator follow-up (one `-e` event-log run, boot -> intro -> cutscene, no
screenshots, decoded with `tools/evtlog_planes.py`):

- the backdrop entry CRAM[0] goes 0x0000 -> 0xE307 -> 0xB6C0 (title navy) and
  is never 0x0EEE, so the intro's white field cannot be the backdrop colour;
- the planes' final state is the cutscene's blank BG_A (every cell tile 0 =
  transparent) over a filled BG_B, i.e. nothing striped;
- 196 CRAM writes do land inside visible scanlines (the illegal-active-display
  class from FIND-08) - still present, see `scene.c:86`/`stage.c` CPU palette
  loads, worth its own fix;
- the player-select map is in range (1120 cells, max index 32 of 33 tiles), so
  its cells cannot reach tile 702 either.

Conclusion: BUG-044 explains the options screen, the stage parallax sky after a
visit to it, and the transition band fills. The intro/player-select frames
stay unexplained *and* are geometry-rejected, so nothing in the ROM's VDP state
found offline (plane cells, backdrop) produces them.


## Transition capture, 2026-10-25 — the slide is clean

Captured with a **screenshot burst** (`tools/emu.sh emu_burst`, ~25 ms/frame,
geometry-validated) while the player was sent down the CHAOS CITY ladder that
links map 0 -> map 1 (`LINK map 0->1 trans 0 dst(198,0) px 3163 py 0 cam 2880`).
24/24 frames usable, no video involved.

The seam (row where the incoming screen takes over) advances 16 -> 22 -> 30 ->
32 -> 36 -> 37 -> 59 -> 69 and is gone by frame 22: that is the whole slide,
captured. Inspecting +/-20 rows around the seam in frames tr05/tr09/tr13/tr19:
both halves match their own rooms, no white bands, no garbage tiles.

Two things that earlier looked like artifacts and are not:
- the striped block at the left edge (x = 1..8, y = 12..60) is the HUD gauge;
- a burst recorded as a 1182x494 video frame was a resized window, not the game
  (`tools/check_captures.py` now rejects those; `emu_shot`/`emu_burst` drop them).

## Batch 2026-10-25 — emulator verification (silent test builds)

Test builds now run with audio off (`config_force_silent()`, applied when
`MD_NO_AUDIO`/`MD_PROFILE`/`MD_TEST_STAGE`/`MD_LINK_TRACE` is defined), so the
numbers below have no Z80/PCM noise.

- **Room 2 of the intro stage (stage 0 map 1, 21 objects) — fixed, measured.**
  `objupd` 308…418 and total busy **1037…1219** subticks of 1280 with `wait`
  400-450: the loop holds 60 fps. Before this batch the same room sat at ~1230
  busy / 40 fps. One 6801-subtick spike per session remains and lines up with
  the boss teleporter's `stage_teleport_boss`, not with per-frame work.
  `hot=` reported the fall platforms (type 10) as the most expensive single
  object while the player rides one, and pills at ~35.
- **Stairs half-body pose — verified by trace.** `MD_LADDER_TRACE` on the
  CHAOS CITY ladder printed, over 57 samples of climbing, exactly one
  `frame=1`: `mid_stair=0 feet_stair=1 y=136` (leaving the top rung), and
  `frame=0` while grabbing and during the whole climb — the PC condition
  (character.cpp:396/426) and nothing else.
- **Dropped items fall — verified by host test.** `tools/run_host_tests.sh`
  spawns a drop in mid-air and asserts the gravity flag and the descent:
  `y 35 -> 115, flags 0x0E (falls)`.
- **BUG-033 (SEAHORSE half speed)** — see its entry: 642 of 1280 subticks, the
  animated-tile bucket down from 1135 to 61.
- Menu (in-game): weapon grid, lives, ammo, ARMOR and ITEMS lines all render;
  the ITEMS line was too wide for the panel and is shortened.
- Graphics: no white stripes or palette damage in the Seahorse or CHAOS CITY
  captures taken during these runs.

## Batch 2026-10-24 (c) — the rest of the fifteen reports

Fixed, each with its offline reproduction:

- **Room transition drew the current room's parallax behind the incoming
  screen**, and **applied the outgoing horizontal offset to the incoming half
  in upward (B2T) changes** — the 1..15px "unaligned" jump. Both planes now take
  their source row from the same tilemap index (`base + y/8`), so the answer
  cannot disagree with what the screen shows, and the destination room's bg
  tiles are copied into the transition's spare VRAM and filled in as the split
  claims them (sky instead when the two bg palettes differ, because both halves
  share PAL3). `check_rules.py` transition-row-split.
- **GAMBIT fell through the floor.** `AI_TELEPORT` ignored its option and placed
  the enemy at `player.y - 16`, i.e. inside the floor. Now the PC's options
  (`ia_action_teleport`): slide to the first wall within `walk_range`, Y never
  changes. `check_rules.py` teleport-options.
- **ANGRY BUNNY hopped in place.** A spawned NPC inherited the spawner's patrol
  span, so every AHEAD move clamped back. A child is a fresh NPC on the PC and
  now gets a span around its own spawn point (its own `range`, clamped to the
  map). `check_rules.py` spawn-own-span.
- **"The old bug returned" on the stage-5 second ladder (death / no return).**
  It really had: BUG-005's arrival nudge lost its caller when the ladder-scan
  arrival landed, so `stage_link_band_hit()` was dead code and a top/bottom
  stair arrival stayed inside the return link's band - the latch never clears
  and the room cannot be left the way it was entered. Restored (8px steps along
  the ladder, max four). `check_rules.py` arrival-band-nudge.
- **BOUNCING projectile (23)** implemented (gravity + floor/wall bounce);
  `check_trajectories.py` now only lists CHAIN, BOMB_RAIN and LARGE_BEAM
  (stage 13 only) as known fallbacks.

New tool: `tools/preview_got_weapon.py` renders the stage-clear screen from the
generated art. It shows the text at tile column 9 (72px) and the player sprite
at x=20 **do not overlap**, so the "text over the player" report needs the
player's screenshot to identify which text is meant.

One trace-build emulator run (stage 5, `MD_LINK_TRACE` + `MD_TEST_TILE`, start
on the map-1 ladder) captured:
`LINK map 1->0 trans 0 dst(105,0) stair 105 px 1675 py 0 cam 1389 latch_off 8`
- the arrival numbers match `tools/audit_links.py` exactly, and the room change
  ran with no crash. The follow-up climb (down, up, down) fired **no further
  link** and left the player ~40px left of the ladder, which is the next thing
  to look at: either the nudge or the ladder grab is releasing him after the
  arrival.

## Batch 2026-10-24 (b) — fifteen play-test reports, offline reproductions

Standing rule from here on (AGENTS.md): a bug is reproduced by an offline
check **before** the fix, and the same check proves the fix.

Fixed (each with its check):

- **HUD showed no gauge at 1 HP** — `set_bar()` clamps a non-zero percentage to
  `HUD_BAR_MIN_FRAME` (3), the first frame that shows a whole segment.
  `tools/check_hud_bars.py`.
- **Boss door re-opened when walking left into it** — the PC only starts a door
  once (`is_started()`), inside the player's vertical span. The port never set
  `started`, so the whole sequence (and the screen move) ran again.
- **Door stayed at its old screen position during a room scroll** — a door is a
  stack of slice sprites and `objects_sync_sprites()` only moved `o->spr`.
- **Knockback pushed the player inside a door** (stuck off-screen) — the hit
  push applied the terrain column rule only; it now also consults
  `objects_block_x()`.
- **Slide passed a horizontal scroll lock** — `player_check_scroll_lock()`
  returned early while sliding and probed a fixed 2 px ahead. It now runs while
  sliding and sweeps every column the leading edge crossed.
- **WOOD SHIELDER walked through the destructible block** — enemy movement
  (`step_axis`) stopped only at terrain; it now stops at solid map objects
  (`objects_block_box`), like the player.
- **Charged shot destroyed the destructible block** — the PC
  (`classplayer.cpp:800 object_is_affected_by_projectile`) lets *only an
  exploding bomb* break `OBJ_DESTRUCTIBLE_WALL` (and no RockDroid1 projectile
  sets the `is_explosive` data flag), so the block types 36/37/42 are the ones
  any shot may break. `objects_try_hit()` takes a `bomb` flag; a bomb's
  explosion passes it.
- **Wall shooter fired a straight normal shot** — the converters read the stale
  30-record `game_projectile_list.dat` at the data root instead of
  `data/game_projectile_list_v3.dat` (36 records, what the PC loads), and
  `PROJ_MAX` was 23; the data asks for ids up to 34 (SMALL MISSILE EXACT,
  `TRAJECTORY_TARGET_EXACT`). `eshot_fire()` clamped those to projectile 0.
  Fixed: v3 table, `PROJ_MAX 35`, `TRAJ_TARGET_EXACT` aiming, 128 px projectile
  art guard (CHAIN is 70x7 and used to be dropped entirely).
  `tools/check_projectiles.py` (was 13 bad references), `tools/check_trajectories.py`.
- **Children of a left-facing spawner faced right** — `ai_spawn_npc()` passed the
  signed runtime dir (-1) where `enemy_add()` wants the data byte (0 = left).
- **Airborne hops 20% fast and through walls** — the jump moved `d->speed` per
  60 Hz frame instead of the shared 50 Hz stepper; it now uses `step_axis()`.

Open (next session, see HANDOFF.md): ladder arrivals (stage 5 + APE), the room
transition drawing the current room's background behind the incoming screen, the
death when climbing back down the stage-5 second ladder, GAMBIT falling through
the floor after two cart throws, the ANGRY BUNNY hopping in place, and the
"YOU GOT" screen text over the player. Known trajectory fallbacks: CHAIN,
BOUNCING, BOMB_RAIN, LARGE_BEAM, LASER (stages 7/10/13 only).

## Batch 2026-10-24 — six reports (charge, shots, enemy speed, door, shooter)

1. **Charged shot must be consumed by the enemy it hits.** It pierced for
   `level >= 2` (a comment claimed the PC's full charge is permanent). Now every
   charged shot dies on the first enemy (or destructible wall). Needs a
   play-test.
2. **No full-charge indication.** PAL2 cannot spare another highlight (the HUD
   bar owns 2/3/4), so the existing two-step armour flash now strobes at
   **2 frames** instead of 8 once `charge_frames >= CHARGE_FULL_FRAMES`
   (132 = the PC's 2.2 s). Verified in-game: the full-charge orb fires.
3. **Wall shooter must aim at the player.** The port fired straight along the
   wall. `eshot_fire_aimed()` now sends the missile toward the player (full
   speed on the dominant axis, proportional on the other; the PC only knows
   45-degree steps). The art stays upright — no rotation on the MD — which is
   the accepted part of the report.
4. **Enemies ~30% faster than the PC.** `enemy.c` now scales every enemy
   movement through `enemy_step()` = 70% of the old rate (AI steps, wall
   walking, the auto-hop). Start point for the report's "slow them down a bit".
5. **Boss door animation.** The old "slide the door down" was wrong (the PC
   paints the door on the map layer and opens it from the CENTRE outward) and
   glitchy. `res/resources.res` now declares the door art as 16x16 slices (one
   animation per slice) and `object.c` draws it as 3/4 slice sprites, hiding
   them centre-out on open and showing them borders-in on close. Collision is
   unchanged (solid until opened).

**Status**: code complete, needs a play-test of all five (door: reach a boss
room; shooter: find a wall-walker).

# Known bugs / deviations — RockBot → Mega Drive

Things that are wrong or worse than the original. Track here, fix later.

## BUG-043 — ADDRESS ERROR after a slide in the APE stage

**Report (2026-10-23, `crash_slide.png`)**: sliding in APE crashed to the MD
ADDRESS ERROR screen.

**Evidence** (from the dump + `out/release/symbol.txt`): fault address
`0x2D302F` (odd), PC `0x2D70C` = `SPR_update+1004`. The disassembly there is

```
2d4ee: moveal %a2@(16),%a4     ; a4 = sprite->frame
2d4fa: bnew   2d704
2d704: moveal %a4@(2),%a5      ; a5 = frame->tileset
2d708: movew  %a5@(2),%d0      ; <-- faults: a5+2 = 0x302D302F -> 0x2D302F
```

with `a4 = 0x02400EEE` and `a5 = 0x302D302D` in the dump: `Sprite.frame` is
garbage, i.e. **an out-of-range animation/frame index** was accepted earlier.
SGDK only validates those inside `#if (LIB_LOG_LEVEL >= LOG_LEVEL_ERROR)`,
and a release build sets `LIB_LOG_LEVEL` to DISABLE (`DEBUG == 0` in
`config.h`), so the bad index reaches `SPR_update`, which then reads
`animation->frames[i]` past the end and dereferences whatever is there. Same
class as BUG-031 (HUD ball frame index).

**Ruled out offline**: enemy `jump`/`teleport`/`intro`/`turn` animation rows
(`audit_data.py` now checks `turn` too: "animation index problems: 0" for both
games), the player's animation set (highest index 13 of 14 animations),
HUD bars/icons, effect frames (`% frames`), object frames (`def->frames`).

**Fix (defensive, 2026-10-23)**: `src/sprite_guard.{c,h}` wraps every
`SPR_setFrame`/`SPR_setAnim` call in the port: it validates against
`definition->numAnimation` / `animation->numFrame`, rejects instead of passing
the index on, and prints the offender over KDEBUG (max 12 lines):

```
sprite guard: animation out of range (index %d, %u valid, def %p)
sprite guard: frame index %d >= %u
```

The crash class is therefore gone (no more garbage frame pointers); the next
reproduction names the call site's art so the *real* index bug can be fixed.

**ROOT CAUSE found 2026-10-24** (the guard paid off): a logged run in the APE
stage printed, nine times,

```
sprite guard: animation out of range (index 14, 14 valid, def 00034030)
```

and `0x34030` is `player_p1_sprite`. ROCKBOT's sprite got **14 animations**
while the other three players got 15: rescomp dropped the trailing
`STAIRS_SEMI` animation for player 1 only, so `SPR_setAnim(ANIM_STAIRS_SEMI)`
(= 14) was out of range and `SPR_update` read a garbage frame pointer - the
ADDRESS ERROR in `crash_slide.png`. The trigger was grabbing/leaving a ladder
(12-frame half-body pose), not the slide itself.

**Fix**: the half-body art (sheet frame 18) is now the **second frame of the
STAIRS animation** (`tools/convert_player.py`: `("STAIRS", [17, 18], [0, 0])`,
15th animation removed, `ANIM_STAIRS_SEMI` deleted from player.h) and
`player.c` selects frame 1 for the semi pose. All four players now ship 14
animations with the same layout, and the guard reports nothing.

**Status**: fixed 2026-10-24, needs one play-test on a ladder (grab and leave
should still show the half-body pose, and nothing may log).

## BUG-042 — Tooltip box interior and name background showed the parallax

**Report (2026-10-23, first play-test of BUG-041)**: the box, the icon and the
name "APE QUAKE" all appeared and the HUD stayed intact, but the box's interior
band showed the street/parallax instead of a solid fill, and the name had
coloured speckles around the glyphs.

**Cause**: the PC art's interior colour is index 0 of the box palette, and the
SGDK font's glyph background is index 0 as well — but on the Mega Drive a
**BG pixel with palette index 0 is transparent** (the other plane, then the
backdrop, shows through). So every interior cell and every glyph cell let
`BG_B` (the parallax) show. `VDP_setTextPalette(PAL3)` cannot help: index 0 is
transparent regardless of what CRAM holds there. Confirmed offline by decoding
the emitted tiles: the interior tile is `00000000` on every row, and the ROM's
`hud_bar_*`/`tooltip_frame_*` sheets render fine, so the art and the converter
were correct.

**Fix**: `tools/convert_tooltip.py` appends one **solid index-1 tile**
(`TOOLTIP_FILL_TILE`) to the box tiles, and `tooltip.c` backs the whole box with
it on `BG_B` (`back_fill_draw()`) before drawing the opaque parts on `BG_A`
(borders, icon ink, glyph ink). Expense: the box now also claims `BG_B` cells
for its 1.5 s, restored by `stage_redraw_bg()` at expiry (next to the existing
`stage_force_redraw()`). VRAM block grew by one tile: tooltip 672..684 (13),
dialog 685..701, config 702, sprite pool 703..1439 (`SPRITE_VRAM_TILES 737`);
`check_vram` still reports 0 transition fallbacks.

**Status**: fixed, needs one play-test (same check as BUG-041: solid dark box,
clean white name, HUD intact, parallax back afterwards).

## BUG-041 — Weapon tooltip never drawn, and it corrupted the HUD

**Report (2026-10-23)**: after picking a weapon in the START menu the tooltip
never appeared (six screenshots at 0.2 s intervals, none of them with the box's
light-blue border anywhere on screen), and the in-game HUD then showed
multicoloured stripes instead of the two gauges.

**Cause 1 — the tooltip was painted over.** `menu_open()` closing calls
`stage_force_redraw()` → `MAP_scrollToEx(..., TRUE)`. SGDK implements the row
and column writes through `DMA_allocateTemp` (`srclib/src/map.c`
`setMapRow/setMapColumn`), so the whole visible plane is *queued* and lands at
the next vblank — after `tooltip_update()` had written the box and the name.
The port already knew this (the start sequence flushes before drawing
"MISSION STARTED!"), the tooltip did not.

**Cause 2 — the HUD garble.** `TOOLTIP_TILE_BASE` was 912, "the free window the
dialog panel uses". That window is inside the **sprite pool**: `SPR_initEx`
puts it at `TILE_FONT_INDEX - size` = 672..1439, and by the time the menu had
been used the stage's enemies/objects had taken the low tiles, so the HUD bar
sprites owned 912..923. The tooltip's `VDP_loadTileData(.., 912, 8)` then wrote
the box art over the bars' tiles; the bars only re-upload on a frame change, so
they kept showing that art (the weapon icon's orange/white plus the box
colours). Same latent bug for the boss dialog panel (912..928) and the config
screen's black tile (935), both reachable while sprites are live.

**Fix**:
1. `tooltip_update()` flushes the DMA queue (`SYS_doVBlankProcess()`) before
   writing its cells, so the menu's queued repaint happens first.
2. Fixed tile blocks moved out of the sprite pool into a reserved block below
   it (`src/stage.h` owns the layout): 672..683 tooltip (12), 684..700 dialog
   panel (17), 701 config black tile (1); the pool is
   `SPRITE_VRAM_TILES 738` = 702..1439. `tools/check_vram.py` uses the same
   number, and still reports **0 transition fallbacks** (the tooltip redraws
   its own tiles on every show, so a transition window sliding over them is
   harmless).

**Status**: fixed, needs one play-test (menu → change weapon → box + icon +
name for 1.5 s, HUD intact, map restored afterwards).

## BUG-040 — Crash entering a stage from NEW GAME (out of heap, NULL map)

**Report (2026-10-23)**: `crash_new_game.png` — ADDRESS ERROR, fault PC inside
`setMapRow` (`out/release/symbol.txt`), odd fault address `0xF90003`. A second
reproduction gave LINE 1111 EMULATOR with the internal PC holding `4EF90002`
(a jump through garbage). Both die while drawing the stage map.

**Cause**: `stage_load()` calls `MAP_create()`, which allocated
`sizeof(Map) + compressed blocks`. `sizeof(Map)` alone is ~580 B because the
map object carries `hScrollTable[240]` + `vScrollTable[20]`; the stage MAP
resources were compressed (`FAST`), so a 45-block map needed another 2880 B.
Measured over KDEBUG at that point in the boot:

```
SL: enter map=0 / SL: tileset loaded
SL: MEM free=4554 largest=3272 alloc=16990
SL: MAP_create FAILED free=4554 largest=3272
```

`free` was enough (4554) but the *largest contiguous block* (3272) was not
(3460 needed): the heap is fragmented by the earlier screens. `stage_load()`
`return`ed with `stage_map == NULL`, the caller ignored it and kept drawing the
stage → `setMapRow` through a NULL/garbage map. Earlier RAM growth (objects /
AI / tooltip state, ~230 B of BSS) had eaten the last few hundred bytes of a
margin that was already negative for fragmented heaps.

**Fix** (2026-10-23):
1. `MAP_create()` now only needs `sizeof(Map)`: the stage MAP resources are
   `NONE` instead of `FAST` (`res/resources.res`, `tools/add_stage.py`), the
   same trick the tilesets already used. Blocks then point straight into ROM
   (`blocksSize = 0`). RAM per map drops ~2880 B; the room-transition
   `MAP_create` (BUG-023) gets the same margin. Cost: ~78 KB of ROM, which
   puts the image at 2.09 MB — the next ~2 KB of code pads it to 2.25 MB.
2. `stage_map_ready()`: `runStage()` returns to the title instead of entering
   the stage loop with a NULL map, and `stage_force_redraw()` /
   `stage_update_camera()` / the autoscroll path no longer call `MAP_*` with a
   NULL map. A failure now logs one KDEBUG line instead of crashing.

**Status**: fixed, needs one play-test (NEW GAME → stage 0).

## BUG-005 — Returning through a stair kills the player instead of teleporting

**Status**: **FIX IMPLEMENTED 2026-10-21, needs one emulator verification**
(arrival no longer leaves the player inside the band he just used).

**Symptom**: SUN-GOD REALM (stage 4, "Mummy"), climbing the stairs *up* works
normally (forward room change **scrolls**, no fade — confirmed 2026-10-21).
Coming back **down**, the player dies unless he first climbs a good deal
higher; climbing high enough makes the room change happen as it should.

**What is known**:
- The first staircase right of the stage start is a jump-grab ladder at tile
  x=60 (stair rows 0-2 and 6-9, body-middle sampling row 10 at ground level);
  the floor-level ladder is tile x=65 (rows 9-13). Both behave like the PC.
- `tools/audit_links.py` reproduces the arrival math of every link in both
  directions against the generated terrain. For stage 4 the arrival of the
  return link (map 2 (189,13) -> map 0 (60,0)) is **clean**: px 955, on a
  stair, no solid overlap, no out-of-map tile. Only two stage-4 arrivals are
  flagged, both `NO-FLOOR` bottom-edge arrivals (link4 rev, link5 rev).
- So this is not a static placement bug: the death happens dynamically after
  the arrival (or on a different link than the one assumed).

**Repro**: stage 4, climb the x=60 ladder to the top (room change), then use
the stair back. Watch for the death animation instead of the return room.

**Next step**: one KDEBUG run (`kprintf` in `stage_check_link`,
`transition_scroll`, `stage_load`, `player_place`) around the return link —
see `blastem_debug.md` for the recipe.

**Latch analysis (2026-10-21, offline)** — user's hypothesis: *the player never
leaves the link, so the return link cannot re-activate*.

- `stage_check_link` keeps `link_latch` set until the link's `hit` becomes
  false ("cleared on region miss only"), the port's version of the PC
  `player.teleporter != -1` gate + the `else { set_teleporter(-1) }` reset.
- The PC arrival for non-teleporter links keeps the player's screen position
  and only scrolls the map (`test_player->set_position(real_x + new_map_pos_x,
  player.y)`), plus the `new_map_pos_x -= diff_h + 2` offset. It never places
  the player on the destination tile centre.
- The port places the player **exactly on the destination cell** and sets
  `on_ladder` from its terrain. For a stair link whose destination is a
  top/bottom row, the trigger band of the return link overlaps the arrival and
  its margin points outside the map, so the only way to clear the latch is to
  climb away by a large amount — this matches "dies unless I go up a lot, then
  it changes rooms as it should".
- Candidate fixes:
  A. Offset the arrival a couple of tiles away from the trigger cell (PC-like),
     so the player starts outside the band and re-entering fires the return.
  B. Clear the latch once the player's *tile* differs from the arrival tile
     (then the return fires from inside a wide band) — needs a guard against
     immediate ping-pong.
  A is closer to the reference implementation and does not loosen the latch.

**Implemented (A)**: `stage_link_band_hit()` in `stage.c` reports whether the
player box sits inside any link band of the current map, with the same
geometry `stage_check_link` uses. `main.c` now, after a link arrival clamped to
the top or bottom map edge, steps the player off that edge in 8 px steps (max
four) until the box is outside every band, then sets `on_ladder` as before.

Worked example (map 0 (60,0) -> map 2 (189,13)): the bottom-map-row rule now
judges by the **body centre** against the row's top edge (`centre >= ty*16`)
instead of the feet sample against `ty*16 - 8`. Arrival py=195 (feet at the
224 screen bottom) has centre 211 >= 208, so it is inside the band; the nudge
moves him up just **4 px** (py=191, feet 220), centre 207 < 208, and the
latch can clear. Pressing down re-enters the band and fires the return — the
player now stands at the bottom of the screen instead of a tile up the ladder.

Top-edge arrivals whose band is already missed (e.g. map 2 -> map 0 (60,0),
feet sample py=21 vs band -19..16) are left untouched.


## BUG-001 — Title screen art loses color depth

**Status**: **FIXED 2026-10-19** — the title draws the PC art at 59 colours
over the four palette lines, verified on hardware: navy backdrop, bright
yellow logotype and the characters' own hues. Three bugs had to go: the
near-colorkey background (below) and an off-by-one palette slot
(`lessons_learned.md` #27), and the palette *selection* - lines filled in
tile scan order let the characters' greys take the slots. The final art uses
one palette line per part with a 15-colour characters image: 15 colours in
15 slots, and folded tiles went from 249 to 1. See the
update at the end of this entry for the recipe and the two traps.

**Symptom**: the title logo and character art look banded/washed out compared to the
PC version. Colors that should be distinct blend together; gradients on the robots
show visible steps.

**Why**:
1. Current pipeline quantizes each image to **15 colors + reserved index 0** (index 0
   must stay unused so no pixel becomes transparent by accident). That is 1 usable
   palette of 16 on a machine that has **4 palettes x 16 = 64**.
2. RGB is additionally rounded to the MD **3:3:3** grid (`MD_LEVELS` in
   `tools/convert_assets.py`), which discards precision before quantizing even
   happens.
3. The source is 24-bit (`logo.png` has 8417 distinct colors).

**What the hardware actually allows**:
- `rescomp` reads an **8bpp indexed** PNG and uses **bits 4-5 of the palette index**
  to select which of the 4 sub-palettes a pixel belongs to, so a single image can use
  64 colors.
- Hard constraint (from `rescomp.txt`): **every pixel in an 8x8 tile must use the
  same palette line** (bits 4-5 constant per tile), otherwise rescomp errors out.
  This means the converter needs a per-tile palette-line assignment pass — a
  constraint-satisfaction step that does not exist yet.

**Fix path** (not started):
- Quantize to 64 colors, lay the PNG palette out as 4 rows of 16.
- For each 8x8 tile, pick one palette line such that all tile colors fit in that
  line's 16 slots; merge/snap colors that do not fit.
- Emit index `(line << 4) | slot`, keep slot 0 of every line empty for transparency.
- Optionally: dither within a tile only, so the dedup does not explode the tile count.

**Update 2026-10-19 (measured, premise corrected)**: the port does not draw the
PC's title art — it draws a hand-drawn 15-colour redraw in
`mega_drive/images/logo.png` (**15** raw colours, 15 after the MD 3:3:3 ramp).
So the 15-colour limit is the art, not the pipeline: extra palette lines have
nothing to recover from it. The PC's own art
(`build/games/RockDroid1/images/logo.png`) has 8417 raw colours and **176** after
the ramp, so reproducing *that* would need the 60-colour / four-line quantiser
described above. The title is drawn from C-side tile + map arrays (not a rescomp
IMAGE), and the tilemap's 2-bit palette field can carry the line per 8x8 tile, so
that path needs no rescomp changes. **Decision (2026-10-19)**: port the PC art at 60 colours. The quantiser is
written (`tools/title64.py`: ~60 colours over four palette lines, one line per
8x8 tile, emitted as C tile/map/palette arrays) and emits sane data (59 colours,
lines 14/15/15/15, 399 tiles), but the first on-screen attempt rendered garbage -
the wiring is reverted and the title still uses the hand-drawn 15-colour logo, so
nothing is broken. Next attempt: check the plane write and the tile-base offset
(`TITLE_TILE_BASE + (entry & TILE_INDEX_MASK)`, palette bits kept from the map
entry), and whether BG_B needs the 64-wide fill to stay.

**Update 2026-10-19 (probe screen)**: a TEMPV probe that loads the 60-colour
tiles, the four palettes and the emitted map *renders the title art correctly*
on BG_B - the data and the four-line mechanism are proven on hardware. It also
showed that the two earlier "still garbage" results were stale ROMs: the capture
script had a hardcoded ROM path, so both retries tested the build with the broken
16-word tile packing. The remaining problem is only in `showTitle()`'s sequence:
wiring the same data there black-screened the ROM from boot, so that needs a
step-by-step comparison against the probe (the probe does: SPR_end, clear both
planes, palettes, backdrop, tileset, map; showTitle also does a stage release,
scroll resets, a BG_A font fill and a 64-wide BG_B fill around the same calls).
The title is back on the hand-drawn art; `tools/title64.py` (with `--preview`)
is committed and ready.

**FIXED 2026-10-19**: the wiring works once three things were right -
1. `tools/title64.py` packs **8 longs per tile** (the first version wrote 16 words
   and emitted them as u32, 64 bytes instead of 32);
2. the tileset goes in with **`DMA`** (immediate), and the four palettes with
   **`CPU`** writes - queued CRAM DMA is unreliable here;
3. `PAL0` slot 0 = the backdrop `0x0210` and slot 15 = the font's white, because
   the font glyphs are index 15 while the art's own white sits at slot 1.

Two traps that cost most of the debugging: a capture script with a **hardcoded
ROM path** (so two "still garbage" verdicts tested an old build), and a
keypress reaching the emulator while it opened (an all-black run that was not the
ROM's fault). Recipe for reusing this on the scenes: `stage0_plan.md` ->
"Four palette lines per screen".

**Files**: `tools/convert_assets.py`, `res/resources.res`

---

## BUG-002 — Title screen top band is 8px, original was 20px

**Status**: **closed 2026-10-19, by design.** The title art is a single image
whose content is 216 px tall; the MD screen is 224, so 8 px of band is what
remains. A 20 px band would either clip the art or need it scaled/redrawn -
and the title now shows the PC art at 60 colours, so the band is the least of
the differences.

**Symptom**: the dark-blue strip above the logo art is thinner than on PC.

**Why**: the original screen is 320x240 and draws `logo.png` at y=20, leaving a 20px
band. The Mega Drive NTSC screen is **320x224**. The logo art itself is 216px tall
(art bbox y=0..215), so a 20px band would clip the bottom of the robots' feet.
Compromise: 8px band, version number drawn at tile row 0.

**Fix path**: either scale the art down ~7% offline, or accept the thinner band.

---

## BUG-T01 — TEMPORARY TEST HACK: slot 1 always has the intro stage cleared

**Status**: intentional, TEMPORARY. Remove before release.

**What it does**: `save_init()` (`src/save.c`) seeds save slot 1 with stage 0
(the intro stage, CHAOS CITY) marked complete:
`save_slot_clear(0); save_slot_set_stage(0, 0); save_slot_set_player(0, 0);`
It only does so when slot 1 is **empty**, so it never overwrites a real save.

**Why**: testing any stage otherwise means replaying the intro stage and its boss
intro first. With this, the title shows LOAD GAME, and
LOAD GAME -> SLOT 1 -> STAGE SELECT reaches any converted stage in a few presses,
and the intro stage's boss intro is skipped (it is gated on the save mask).

**Caveats**:
- Slot 1 is seeded only while empty, so a real save there is left alone — but do
  not rely on slot 1 for save-persistence tests.
- NEW GAME still clears the slot it picks; the hack re-applies on the next boot.
- Other stages' boss intros still play (skippable with any button) because their
  bits are not set — add `save_slot_set_stage(0, n)` for a stage you are testing.

**Removal**: delete the marked block in `save_init()` and this entry.

---

## BUG-003 — Two anim blocks cannot use palette colourcycle (shape really changes)

**Status**: **FIXED 2026-10-19** — both were redrawn as single-frame 16x16
three-colour bands in `mega_drive/images/tilesets/anim/` and are converted by the
art-width rule (`roadmap.md` item 12).

**What**: `waterfall_top.png` and `water_ground.png` (PC
`build/games/RockDroid1/images/tilesets/anim/`) are the only animated blocks whose
**pixel shape** changes between frames, so the CRAM-rotation technique we are
adopting for the rest of the anim set cannot reproduce them: every other block
differs from frame to frame only by a colour permutation, these two move pixels
around. They must either keep the tile-frame animation (current behaviour) or be
redrawn as a repeating colour band like the rest.

**Verified**: shape-mask comparison + per-step colour-permutation test over all 34
anim blocks; these two (plus the five `palmtree_00*` blocks, which genuinely change
shape and are not meant to be cycled) are the exceptions.

**Files**: `tools/convert_stage.py` (block classification), the source PNG above.

---

## BUG-004 — Stage art is over the 15-colour palette budget (FROZEN LAIR now the worst)

**Status**: open, known deviation from the PC art (user: "I noticed some lack colors,
like the moving platforms").

**Symptom**: distinct shades in the source art merge into one palette entry, so
e.g. the moving platforms look flat. The platform art has five shades —
`(255,102,0) (255,231,163) (243,191,63) (219,43,0) (167,0,0)` — which collapse into
about two orange entries.

**Why**: the port puts the whole stage (all three maps **plus** every anim block) on
ONE 15-entry master palette (`MAX_COLORS = 15`, index 0 reserved transparent;
`tools/convert_stage.py` `build_palette`), merging the closest colours when the art
needs more. The PC art is authored as true colour (the shared tileset `default.png`
has 124 distinct colours), so it does not fit.

Measured colours needed vs the 15 available:

| stage | needed | merged away |
|---|---|---|
| 0 CHAOS CITY | 17 | 2 |
| 1 DEEP JUNGLE | 38 | 23 |
| 2 FLOWER FIELDS | 35 | 20 |
| 3 ATLANTIDA | 31 | 16 |
| 4 SUN-GOD REALM | 23 | 8 |
| 5 TRICKY CASTLE | 26 | 11 |
| 6 DARK MINE | 28 | 13 |
| 7 CRYSTAL MAZE | 16 | 1 |
| 8 INFOWAY | 14 | fits |
| 9 LAVA PIT | 14 | fits |
| 10 SPACE BASE | 20 | 5 |
| 11 POWER GRID | 26 | 11 |
| 12 RECYCLE FACTORY | 12 | fits |
| 13 FROZEN LAIR | 56 | 41 (weighted error 2324) |

Also: index 0 of the generated palette is a reserved black, so any black in the art
takes a second entry (stage 1's palette has `(0,0,0)` at both 0 and 1). Where that
black is only the *surrounding* empty area, marking it transparent in the source art
reclaims the slot; where it is part of the design (the moving platform's segment
gaps) it cannot be reclaimed.

**Fix path**: the reserved-slot design being discussed for colourcycle
(anim art drawn with generic colours, mapped to per-stage entries; cycling bands in
the free part of PAL3) — plus a deliberate re-quantisation of the art per stage.
See `stage0_plan.md`.

**Update 2026-10-19 (stages 9..13)**: FROZEN LAIR is the new outlier — 56
distinct colours for 15 slots (weighted error 2324, vs DEEP JUNGLE's 1946).
LAVA PIT (14) and RECYCLE FACTORY (12) fit exactly; SPACE BASE 164 / POWER
GRID 245 are between. FROZEN LAIR needs its own palette strategy (per-map
palette or a deliberate art re-quantisation) before it can look right.

**Update 2026-10-19 (palette merge)**: `build_palette` no longer deletes one
colour of the closest pair - it replaces the pair with its pixel-count weighted
average, so rare shades fold into the dominant colour instead of dragging it off.
It also reports a weighted error per stage, now visible in every build:

| stage | colours | error |
|---|---|---|
| 0 CHAOS CITY | 16 | 22 |
| 1 DEEP JUNGLE | 33 | **1946** |
| 2 FLOWER FIELDS | 31 | 437 |
| 3 ATLANTIDA | 20 | 299 |
| 4 SUN-GOD REALM | 23 | 557 |
| 5 TRICKY CASTLE | 26 | 350 |
| 6 DARK MINE | 25 | 216 |
| 7 CRYSTAL MAZE | 16 | 7 |
| 8 INFOWAY | 9 | 0 |

DEEP JUNGLE is the outlier and is the one worth re-quantising by hand (its art
uses 33 MD-ramp colours for 15 slots, most of them foliage shades).

**Update 2026-10-19 (colourcycle landed)**: the cycled blocks' colours left PAL1
for PAL3, so the map palette no longer has to hold them. Colours still needed for
PAL1, before -> now: stage 0: 17 -> 16, 1: 38 -> 33, 2: 35 -> 31, 3: 31 -> **20**,
6: 28 -> 25, 8: 14 -> **9** (fits, with room). Stages 4, 5, 7 unchanged (no
cycled blocks). ATLANTIDA gained the most (16 fewer shades merged); DEEP JUNGLE is
still the worst. What remains is the map art itself: it needs the deliberate
per-stage re-quantisation. The anim side of this bug is done.

**Files**: `tools/convert_stage.py`, `tools/img2tiles.py`, the source art.


## BUG-006 — Sub-boss door opens but the screen never scrolls to the boss room

**Status**: **FIXES IN, needs an in-game check** (reported 2026-10-21, Mummy
stage / SUN-GOD REALM). Two changes from the source review:
`column_locked()` now gives the **live door state priority over the converted
scroll-lock table** (a door column that is also in the map table stayed locked
after opening, so the camera could never scroll into the boss room), and
`objects_reset_doors()` re-closes opened doors when the player respawns (the
port does not reload the map on death, so a door stayed open for the rest of
the stage and its lock never came back).

**Symptom**: touching the sub-boss door lets the player through, but the
horizontal scroll that reveals the boss room never happens. Worse, the scroll
stays unlocked afterwards — even after dying and coming back.

**Suspects** (both in the port's door handling):
- `objects_boss_door_locked()` (`object.c:594`) releases the lock as soon as
  the door is `started`; the PC additionally scrolls the screen to the boss
  room when the door opens (`classmap.cpp:1611` area). The port has no
  equivalent camera/scroll step.
- Doors live in the object list and are **not reset on death**: the death path
  reloads neither objects nor the door state, so an opened door stays open
  forever in that stage attempt. Reloading the map (or re-closing doors) on
  respawn/stage re-entry should restore the lock.

**Measurement 2026-10-21** (spawn x=1664 left of the first door, walked right;
`kprintf` heartbeat with the camera):
```
F855 x1713 cam1437 d109=1
F870 x1743 cam1582 d109=0   <- door opens, camera jumps +145 px
F930 x1860 cam1700 d109=0   <- camera follows the player
F2205 x2027 cam1741 d109=0  <- stopped at the *second* door (x=2048)
```
So the lock is released and the camera does move (145 px at the door, then it
tracks the player), but there is no full 320 px screen step. The player is then
stopped at x=2027 = 2048-21 by the **second** door while the UFO sub-boss is
alive, so the boss room stays out of reach until that sub-boss dies. Repeating
this with the UFO killed decides whether a camera step is still missing.

## BUG-007 — Charged shot resets when crossing a room link

**Status**: **FIXED and verified 2026-10-21**. Holding the attack button
through a stair/link transition now keeps the charge, as in the PC.

**Cause**: `player_place()` cleared `player.charge_frames`, and the room-link
handler calls `player_place()` on arrival. Immediately after the transition
`handle_fire()` saw a "fresh press" and fired a normal shot, restarting the
charge from zero.

**Fix**: `player_place()` no longer touches the charge (it is used by the link
arrival too); the reset moved to `respawn_player()` (plus the existing one in
`player_init()`), together with `fire_prev = FALSE` so a held button after a
death does not fire.

**Verified** with `kprintf` around the link and at release:
`CHG pre=124 -> CHG post=124 -> REL chg=124` (before the fix: `111 -> 0`).

## BUG-008 — Boss dialogs are a black box (PC shows an image)

**Status**: **FIXED** — the panel landed in BUG-028 (PC `dialog.png` through
`tools/img2tiles.py`, drawn on BG_A in PAL3); the 2026-10-22 play-test report
"dialog text background / A arrow" is BUG-037.

## BUG-037 — Dialog text background black, A-button arrow a black box

**Report (2026-10-22 play test)**: the panel shows, but the three text lines sit
on a black band and the A-button icon is a black square
(`screenshots/bugs/playtest/boss_dialog_text_bg.png`).

**Cause**: two separate palette mistakes. SGDK's font paints a whole 8x8 cell
per character, background = palette index 0 and ink = index 15, so drawing the
lines with the default PAL0 painted black cells over the panel. And the 12x12
`btn_a.png` goes through `img2tiles.py`, which re-quantizes even an indexed
source: its white landed on index 1, which in PAL3 is the panel's black border.

**Fix**: `dialogs.c` builds PAL3 with entry 0 = the panel fill
(`dlg_panel_pal.data[2]`, the dark teal) and entry 15 = font white, and the
pages are written with `VDP_setTextPalette(PAL3)`; the button is a font "A"
glyph in the same palette (the icon art and its build steps are gone).

## BUG-009 — Rockbot has no eyes in boss dialogs

**Status**: **FIXED and verified 2026-10-21**. The face art's white eye sockets
are palette index 0, i.e. sprite-transparent, and the pupils live in the
separate `ssel_eyes_pN` layer that only the stage select drew. `dialogs.c` now
adds that eyes sprite (offset +10,+18, same as the PC's face (144,104) /
eyes (154,122)) behind the face on the player's pages only. Verified in a
forced boss-room dialog: the portrait now has eyes.

## BUG-010 — Mummy keeps facing away from the player

**Status**: **FIXED 2026-10-22** (play-test pending). Two things:
1. `AI_TELEPORT` now lands facing the player (`e->dir` from the new position),
   and
2. the teleport/jump animation rows exist (BUG-011), so the boss no longer
   keeps the old art while he repositions.
The old analysis stands: action 5 (SHOT_AHEAD) does not turn, as on the PC.
- The Mummy's AI rows (`stage4_enemies.c`, MUMMY BOT) use actions 2 (jump),
  10 (teleport) and 5 (shot ahead). In the PC, action 5 is
  `AI_ACTION_SHOT_PROJECTILE_AHEAD`, which explicitly does **not** turn to the
  player (`artificial_inteligence.cpp:1164`: only non-ahead actions do). So the
  port currently matches the PC here.
- Spawn direction is converted correctly: map_npc `dir` is 0/1 and
  `enemy_add()` maps it to -1/+1 (`enemy.c:643`).
- Therefore the wrong facing most likely comes from the **missing teleport and
  jump animations** (BUG-011): without them the boss keeps the previous art row
  while the AI repositions him, so he visually "faces" the old direction.
  Fix BUG-011 first, then re-check this before adding any facing rule.

## BUG-011 — Enemies do not switch to teleport/jump animations

**Status**: **FIXED 2026-10-22** (play-test pending), including the boss
`ANIM_TYPE_INTRO` row (file anim 16) played after the landing drop. The
converter now ships
the `ANIM_TYPE_JUMP` (file anim 2) and `ANIM_TYPE_TELEPORT` (anim 4) rows, but
ONLY for enemies whose AI uses those actions (action 2 / 10): the extra row
costs sprite VRAM, and crowded stages are already near the 504-tile pool. The
row index is stored per def (`jump_anim` / `teleport_anim`, 255 = none) because
the row order is stand, walk, attack, [jump], [teleport]. `enemy.c` picks the
row while the action runs, and `convert_enemies.py` now rewrites the matching
`SPRITE` lines in `res/resources.res` (they must list one animation per row).

## BUG-012 — Projectiles not cleared when the boss dies

**Status**: **FIXED 2026-10-22** (play-test pending). `eshots_reset()` dropped
the sprite HANDLES without releasing the sprites, so every enemy projectile
stayed allocated and frozen on screen (and the sprite VRAM leaked for the rest
of the stage). It now calls `SPR_releaseSprite()` first, exactly like `kill()`.

## BUG-013 — The "YOU GOT ..." screen differs from the PC

**Status**: **FIXED 2026-10-22** (play-test pending). The port now follows
`game::classic_style_got_weapon()` (game.cpp:1347): white blink, CONFIG_BGCOLOR
(5,29,65), player at x=20, and the text typed out (`draw_progressive_text`)
at rows 14/16/17. The second line names the ITEM of the COIL/JET stage
(E-TANK / W-TANK, the PC's `game_data.player_items` names uppercased) - it used
to print weapons 9/10, which are not items at all. Total wait stays 5000 ms,
typing included.

## BUG-014 — Stage select shows garbage text after beating Mummy

**Status**: **FIXED 2026-10-22** (play-test pending).

**Cause**: the name lines were drawn with the *global* PAL0. The font paints
its cell background with palette index 0 (= CRAM 0, the backdrop, which the
stage writes per map via `apply_bg_color` and the YOU GOT screen overwrites
with its colour flash), so the names came out in that leftover colour on the
same colour (peach on peach in the report) and unreadable.

**Fix** (`stageselect.c`): `draw_names()` sets the text plane explicitly and
draws with **PAL1** (the grey ramp already loaded for the beaten faces: 0 =
black, 15 = light grey), restoring PAL0 afterwards; `stage_select_run()` also
re-uploads the default font (`VDP_loadDefaultFont(CPU)`) so a previous screen
that wrote past the user tile area cannot leave the glyphs garbled.
`tools/check_palettes.py` asserts the 0/15 palette entries.

## BUG-035 — BETABOT colours wrong / HP bar colours wrong

**Report (2026-10-22 play test)**: BETABOT's body is red instead of black/grey
(`screenshots/bugs/playtest/betabot_colors.png`), and the HP bar shows the
player's weapon colours instead of the PC's fixed magenta/blue.

**Cause**: `player.c::rebuild_player_palette()` wrote the classic HUD bar
colours into PAL2 entries 8/9 (and 12) — but PAL2 is the *player* palette and
the sheets use every entry: BETABOT's entire black body is index 8, CANDYBOT's
armour index 8/9/12, KITTYBOT's index 9. The HP bar silently recoloured the
sprite (ROCKBOT was unaffected because his sheet leaves 8/9 unused, which is
why only some players looked wrong).

**Fix**: the HP bar moved to **PAL0** with the PC's fixed colours
(`draw_energy_bar` always uses PLAYER 1's weapon-0 colours: PAL0 6 purple +
3 light blue + 1 black), the weapon bar keeps PAL2 13/14 (the art's own swap
placeholders) with a transparent empty part and a black PAL0 backdrop sprite,
and `rebuild_player_palette()` no longer touches 8/9/12.

## BUG-036 — Explosions do not move away from the centre

**Report (2026-10-22 play test)**: the death explosion stays as one centred
puff.

**Cause**: two things. The port froze the burst: the player-death branch never
called `explosion_update()`, so the puffs sat on frame 0 and were never
released; and it burned three puffs in a horizontal row instead of the PC's
outward pattern.

**Fix** (`main.c`): the death branch now advances the puffs and spawns the PC
pattern (`game.cpp:1707/1734`): the centre puff repeats over the 90-frame death
window (0/30/60) and four more spawn at ±32 px diagonals at 18/36/54/72 frames.

**Boss half (same report)**: the port played a pseudo-random scatter of puffs for
2 s. The PC (`game::classic_style_draw_boss_death`, game.cpp:1798) sends **eight**
explosions outward from the boss (right/left/up/down + the four diagonals), the
radius growing until it leaves the screen. `explosion.c` now has
`explosion_ring_start/update` doing exactly that with 8 sprites using the PC art
`classic_style_death_explosion.png` (5 frames of 24x24, emitted as
`res/death_explosion.png` by `tools/convert_assets.py`), PAL0, 3 px/frame so the
speed matches the PC's ~200 px/s. `runStage` starts the ring instead of the
scatter and holds the stage until it finishes (~108 frames).

## BUG-026 — Checkpoint shows no animation when activated

**Status**: **FIXED 2026-10-22** (play-test pending).

**Cause**: the art, not the object logic. `checkpoint.png` is a terminal whose
two frames are "screen off" (dark green) and "CHECK POINT" (bright green + red
LEDs). PAL0 has only ONE green, so both greens quantized to index 13 and the
two frames ended up 6 pixels apart — the loop (`anim_loop = 1`, 300ms/frame)
was running all along and could not be seen. `object.c` itself was correct.

**Fix**: `tools/import_pc_art.py` pins the terminal's colours apart
(`CHECKPOINT_OVERRIDES`: screen background -> PAL0 black, text -> white, LEDs
-> red) and the hand art was rebuilt; the frames now differ by 130 px.
`tools/check_palettes.py` guards the pair.

## BUG-015 — Missing boss intro for Seahorse (maybe global)

**Status**: **FIXED 2026-10-22.** The intro was not skipped - it was skipped
*instantly*. `boss_intro_run()` skipped on any button of the pad, and the stage
select returns on a START/A/C press, so the player is still holding that button
on the first intro frame: the loop broke immediately, the screen flashed the
blue backdrop for one frame and only the jingle was heard. A forced stage-3 run
from boot looked fine because no button was held.

Proof offline: the live BlastEm SRAM (`~/.local/share/blastem/rom/save.sram`)
holds the save the user played - slot 0, `stages = 0x0001` (only CHAOS CITY
cleared), 5 lives - so the gate `!mask & (1 << 3)` was TRUE and the intro did
run.

**Fix** (`bossintro.c`): the skip is gone entirely. The PC's
`classic_style_boss_intro` ends with `graphLib.wait_and_update_screen(6400)`,
which reads no input: the intro is a fixed 6.4 s hold and is NOT skippable. The
port had added a "press any button to skip" that the PC does not have.

## BUG-038 — Port-only input skips (PC sequences that cannot be skipped)

**Report**: "it should not be skippable. I believe the PC version has nothing
like that."

Same class as BUG-015 - the port had invented skips the PC does not have.
Audited every `JOY_readJoypad` in a non-menu loop against the PC source:

| port site | PC reference | verdict |
|---|---|---|
| `bossintro.c` boss intro | `wait_and_update_screen(6400)`: no input | skip REMOVED |
| `main.c` `hold_frames()` (stage clear: victory flash, 3.5 s pose, YOU GOT 5 s) | `player_victory()` delays only, `classic_style_got_weapon()` ends with `wait_and_update_screen(5000)` | skip REMOVED |
| `ending.c` walking parade | `show_player_walking_ending()`: `while` on the player x, `timer.delay`, no input | skip REMOVED |
| `ending.c` boss parade + credits pages | `show_enemies_ending()`: 2000/3000 ms delays; credits scroll on a timer | skip REMOVED |
| `ending.c` final "press" | `draw::show_credits(can_leave == false)` waits for JUMP | kept |
| `scene.c` cutscenes | `sceneshow.cpp:59` aborts on START | kept (PC-faithful) |
| `main.c` GAME OVER | `game_over()` -> `input.wait_keypress()` | kept |
| `dialogs.c` page advance | `show_dialog()` -> `input.wait_keypress()` | kept |

Also fixed while auditing: `CASTLE_FRAMES` was 180 (3 s) with a comment claiming
the PC uses 3000 ms; `classic_style_castle_intro` actually holds **6500 ms**, so
it is 390 frames now.

## BUG-016 — Seahorse screen transitions are broken; player X shifts

**Status**: **FIXES IN, needs a re-test.** Traced 2026-10-21 with `kprintf`
captured via `script -qf` (see `blastem_debug.md`).

Trace (ATLANTIDA, map 0 -> 1 -> 0):
```
LINK m0->1 tx176 ty0 off22
TRANS 0->1 cam-3 px185 py0
TRANS ret=1 code=0
ARR m1 x185 y0 lad0
LINK m1->0 tx928 ty0 off141
TRANS 1->0 cam909 px1056 py0
ARR m0 x1056 y0 lad0
```
Two defects shown:

1. **X shift**: the port snapped every arrival to the destination tile centre
   (`px = dst*16 + 8 - 13`). PC keeps the player's horizontal position and
   scrolls the map (`game.cpp:762`), which matters for wide links (stage 3 has
   sizes 4, 7, 8, 18). Fixed: `stage_check_link` stores the player's centre
   offset inside the trigger band (`stage_link_band_off`) and the arrival uses
   `dst_tile*16 + off - box_centre`, clamped to the map. The trace above now
   shows `off22 -> x185` and `off141 -> x1056`, i.e. preserved.
2. **Corrupted tiles / negative camera**: `scroll_lock_span()` can set
   `camera.x = (c<<4) - 307`, which is **-3** for column 19, and
   `stage_arrival_camera_x()` returned it unclamped (`TRANS 0->1 cam-3`). The
   transition then drew the destination screen with a negative camera, putting
   garbage tiles on the plane. Fixed: clamp the result in
   `stage_arrival_camera_x()`.

**Verified 2026-10-21** (spawn forced at x=751, walked right into the link):
```
LINK m0->1 tx176 ty0 off56
TRANS 0->1 cam0 px219 py0
TRANS ret=1 code=0
ARR m1 x219 y0 lad0
```
`cam0` (was -3), `x219 = 176+56-13` so the offset is preserved, no corrupt
tiles on the destination screen, HUD survives the transition.

## BUG-017 — Falling into water lands on the surface

**Status**: **FIXED and verified 2026-10-21**. `ground_pair()`
(`player.c:369`) treated "different feet tile vs centre tile" as ground, so
feet-in-WATER over centre-UNBLOCKED counted as floor and the player stood on
the surface. Water is now excluded from the ground pair; the water gravity /
jump rules already in the port take over.

Verified in ATLANTIDA (tile x=103: water rows 7-10, solid row 11, air above):
spawned at y=80 above the surface, the player fell **through** the water and
landed on the floor at y=147 (feet row 11). Before the fix he stopped at the
surface (y=83). Screenshot confirms him under the water column.

## BUG-018 — Fish leaves the water to chase the player

**Status**: **FIXED** (confirmed by the 2026-10-22 play test — "fish staying in
water" is in the do-not-regress list). First attempt compared the position with
itself; see the analysis further down for the final fix (pre-move probe, PC
`character.cpp:1921-1923`).

## BUG-019 — DR. DESTRIN boss intro shows APE BOT

**Status**: **FIXED 2026-10-21** (runtime re-check pending with the user).

**Report**: starting the castle stage whose boss is DR. DESTRIN, the boss intro
drew an Ape sprite.

**Cause**: `EnemyDef.is_boss` came from the NPC record's own `is_boss` byte
(`file_npc`, `b[6026]`). That flag is per enemy TYPE, and castle stages place
other stages' bosses as regular enemies — stage 13 spawns APE BOT (id 2),
DAISIE BOT, MUMMY BOT, SPIKE BOT, TECHNO BOT, DYNAMITE BOT, SEAHORSE BOT and
MAGE BOT, all with the flag set. `bossintro.c` draws the first `is_boss` def,
so APE BOT (id 2, sorted before DR. DESTRIN id 24) won. The same conflation
also gave the stage-13 APE BOT the boss drop-in and the boss HP row.

PC reference: `classmap.cpp:561` — only `stage_data.boss.id_npc` becomes the
stage boss (`set_stage_boss`: 36 HP, drop from the top, hit window); other
boss-flagged types only get `set_is_boss(true)`.

**Fix**:
- `tools/convert_enemies.py` reads the stage record's `file_boss.id_npc`
  (`stages_v301.dat` offset 360) and sets `is_boss` for that npc id only.
  Stage 12 was also wrong in the other direction: WASPER is its boss but its
  record flag is 0, so that stage had no boss def at all.
- `enemy.c` treats the live `boss_i` index as the stage boss for the death
  path, and an `AI_REPLACE_NPC` child of the boss inherits the status with
  full boss HP (PC `artificial_inteligence.cpp:2294`), so DR. DESTRIN's
  tank -> ship replacement continues the fight instead of clearing the stage.

## BUG-020 — No boss door in the DR. DESTRIN room (objects beyond 24 dropped)

**Status**: **FIXED 2026-10-21** (in-game re-check pending).

**Report**: the DR. DESTRIN room had no door; the same stage also seemed to be
missing objects.

**Cause**: `OBJECT_MAX` was 24, but stage 13 map 0 places **27** objects and the
boss door is the last one. `objects_reset` stops at `obj_count < OBJECT_MAX`,
so the door (and the two objects before it) never spawned. The converter was
not dropping anything: it reports 0 skipped spawns for every stage 0..13.
`ENEMY_MAX` had the same problem — stage 10 map 2 spawns 27 enemies and 11 were
dropped.

**Fix**: `OBJECT_MAX` 24 -> 32, `ENEMY_MAX` 16 -> 32 (max per map across the
converted stages is 27 for both).

## BUG-021 — MAGE: enemy projectile is not flipped when fired left

**Report (2026-10-22)**: GAMBIT's card flies left but is drawn facing right.

**Status**: **FIXED 2026-10-22** (play-test pending). `eshot.c` only set
`SPR_setHFlip` when a shot bounced; the spawn and the target-direction paths
left the art facing right. `eshot_flip()` now mirrors every enemy shot from its
travel direction (`dx < 0`), at fire time, after the target-direction resolve
and on a bounce.

## BUG-022 — SEAHORSE: wrong/incomplete tileset at stage start

**Report**: starting the SEAHORSE BOT stage shows a wrong or incomplete
tileset.

**Status**: **FIXED / confirmed by the 2026-10-22 play test** ("SEAHORSE
tileset/fade/corruption after links" is listed under "do not regress" in
HANDOFF.md). The per-map tileset/palette reload and the colourcycle band are
in `stage_load()`.

## BUG-023 — SEAHORSE: every link transition has a fade

**Report**: every room link in the SEAHORSE stage shows a fade; suspects it
started after playing MAGE BOT first.

**Status**: **FIXED / confirmed by the 2026-10-22 play test** (same group as
BUG-022).

## BUG-024 — SEAHORSE: tileset corrupt/incomplete after the second link

**Report**: after leaving through the second link the tileset is corrupted or
incomplete.

**Status**: **FIXED / confirmed by the 2026-10-22 play test** (same group as
BUG-022).

## BUG-025 — Sub-boss door scroll is too fast and leaves the player on the right

**Status**: **FIXED 2026-10-22** (play-test pending). Port now follows PC
`game::horizontal_screen_move` (game.cpp:1174): 4 px steps
(`TRANSITION_STEP`), 76 of them, and the player is pushed only ~2.5 tiles
(40 px) in the same direction, so he ends near the LEFT edge of the new screen.
Two steps per frame (the PC runs one every ~2 ms), and the door handler no
longer carries him the full screen.

**Remaining half (2026-10-22)**: the door sprite stayed glued to the screen
while the stage slid under it — the scroll moved the camera and the player but
never re-placed the objects/enemies (their sprites are positioned from world
coordinates only in the normal frame loop). `stage_horizontal_screen_move()`
now calls `enemies_sync_sprites()` and `objects_sync_sprites()` after every
step, so the door keeps its world position.

## BUG-026 — Checkpoint shows no animation when activated

**Status**: superseded by the entry above (the object flags were fine; the two
art frames quantized to the same colours).

## BUG-027 — Slide speed equals walking speed

**Status**: **FIXED 2026-10-22** (play-test pending). The slide/dash now use
`SLIDE_SPEED` = 2.5x walk, matching PC `character::slide`
(character.cpp:1724: `max_speed = move_speed * 2.5`). Note: the report said
1.5x, the PC source says 2.5x - easy to change if it still feels wrong.

## BUG-028 — Boss dialog background still missing

**Status**: **IMPLEMENTED 2026-10-22** (play-test pending). The PC panel
(`images/backgrounds/dialog.png`, 310x65) and its A-button icon (`btn_a.png`)
are converted by `tools/img2tiles.py` (`dlg_panel`, `dlg_btn_a`, wired into the
Makefile) and drawn on BG_A at the PC position (panel rows 0..8, button at
column 34/row 5) in PAL3 with their own palette, replacing the old black box
and the "JUMP: NEXT" text. The stage colourcycle band is re-applied through
`stage_restore_cycle()` when the dialog closes (PAL3 is the cycle line in
stages 1/2/3/6/8).

**Follow-up (2026-10-22)**: the art is 310x65, i.e. 38.75 x 8.125 tiles, so
`img2tiles` padded the panel to 39x9 with TRANSPARENT cells: the last 2 px of
the right column and the whole 9th row were see-through, and the stage showed
through them (the vertical strip of water at the right edge of the panel in
`screenshots/bugs/boss_dialog.png`). The source art is now padded to 320x72
with its own black border (`images/backgrounds/dialog.png`), which is exactly
40x9 tiles: every cell of the panel is opaque.

## BUG-029 — "YOU GOT" screen (PC stage-clear sequence)

**Status**: **IMPLEMENTED 2026-10-22** (play-test pending). `showStageClear()`
now follows the PC:

- `game::player_victory` (game.cpp:1276): music stops, explosion SFX, the
  player takes the GOT_WEAPON pose (new animation row 26/27 in the player
  sheet), the screen flashes the PC's four colours twice (300ms each) and then
  the stage is shown for 3.5s;
- `game::classic_style_got_weapon` (game.cpp:1347): dark blue CONFIG_BGCOLOR
  screen, player at x=20 with the attack pose, and the three lines
  "<WEAPON>" / "AND <ITEM>" (stages 1 and 6) / "HAVE BEEN ACQUIRED" (or
  "HAS BEEN ACQUIRED"), held 5s or until START.

The MD has no full-screen colour fill, so the flash writes the colour into
PAL0 slot 0 (the backdrop) with a queued palette write and clears both planes
to a transparent tile.

## BUG-030 — Font corrupted after winning a stage

**Report**: after clearing a stage the font is corrupted.

**Status**: **FIXED 2026-10-22** (play-test pending). Two screens were hit:

1. The stage-clear / YOU GOT / game-over text screens draw with PAL0 while the
   stage backdrop is still in CRAM 0: `textScreenBegin()` reloads PAL0 from
   `pal_ui` (see the fix note further down).
2. The stage select drew its name lines with PAL0 as well, so the per-map
   backdrop tinted the glyph cells (the "peach on peach" names, BUG-014). It
   now draws them with PAL1 and re-uploads the default font first.

## BUG-031 — ADDRESS ERROR (crash) when standing on a disappearing block

**Report (2026-10-22)**: standing over a disappearing block crashed with the
Mega Drive ADDRESS ERROR screen. The offset in the dump (0x272E4) resolves
inside `SPR_update` in `out/release/symbol.txt`, and the fault address
(0x20302F) is an ODD ROM address: a misaligned read.

**Cause**: `hud.c` `set_bar()` computed `frame = (pct * 50) / 100` (0..50) and
passed it to `SPR_setFrame()` on the HUD ball sheets, which have **5** frames
(`tools/convert_assets.py`, ordered full..empty). Any frame above 4 makes
`SPR_update` read a garbage frame pointer out of the sprite definition - the
odd address it then tries to read is the crash. It is intermittent because the
memory after the frame table sometimes looks like a valid frame.

**Fix**: `frame = ((100 - pct) * (HUD_BALL_FRAMES - 1)) / 100`, clamped to
0..4 (frame 0 = full). Applies to the HP, boss and weapon bars.

## BUG-026 — Checkpoint shows no animation when activated

**Cause (first half)**: the PC object flag `animation_loop` (file_object byte
82) is 1 for the checkpoint, so its flag keeps waving after activation; the
port hardcoded `is_looping()` to type 32 (boss teleporter) only, so the
checkpoint played one frame and stopped.

**Fix (first half)**: the object converter now emits `anim_auto`/`anim_loop`
per def and `is_looping()` uses the data flag.

**Second half (still invisible after that)**: the two art frames were only 6
pixels apart because PAL0 has a single green - see the entry above (fixed
2026-10-22 by pinning the terminal's colours apart in `import_pc_art.py`).

## BUG-030 — Font corrupted after winning a stage

**Fix (pending play-test)**: `textScreenBegin()` now reloads PAL0 from
`pal_ui`, so the text screens (stage clear / YOU GOT / game over) cannot
inherit the per-map backdrop (CRAM 0, written by `apply_bg_color`) or any other
palette the stage left behind.

## BUG-018 — Water enemies leave the water (still open after the first fix)

**Cause**: the first fix (`enemies_update()` reverts a move that crosses the
water line) compared the NEW position with itself: `enemy_water_ok(e, e->x,
e->y)` read `e->x/e->y`, which the AI had already moved, for the "old" probe
too, so `now == was` was always true and the rule never fired. The SEAHORSE
fish therefore kept swimming onto the pipes.

**Fix**: pass the pre-move position (`pre_x/pre_y`, captured before the AI runs)
as the "old" probe. A non-boss NPC now cannot leave water or enter it, exactly
like `character.cpp:1921-1923`.

## BUG-032 — HP/weapon/boss bars show empty

**Cause**: the bar sheets are 51 frames ordered **empty..full** (frame 0 = no
fill, frame 50 = full, `tools/convert_energy_bars.py`), but `set_bar()` still
used the old 5-frame ball sheet scale: `((100 - pct) * 4) / 100`, i.e. 0..4 of
51. Full HP selected frame 0, which is the empty art, so the bars never showed
a fill (and the gauge could not move).

**Fix**: `HUD_BAR_FRAMES 51`, `frame = (pct * 50) / 100`, clamped. Applies to
the HP, boss and weapon bars (all three sheets are the same 51-step art).

## BUG-033 — SEAHORSE runs at half speed (animated tiles)

**Measured** (`make profile`, per-frame cost in subticks, 1280 = one 60 Hz
frame, SEAHORSE map 0 idle):

```
setup=213  cam=43  anim=1135  npc=160  shot=32  spr=99  wait=766
```

Total ~2460 subticks per iteration = the loop only completes every other
vblank: **~30 fps**, exactly the reported "VERY slow". APE map 0 has no
animated slots, which is why it felt fine.

**Cause**: every animated slot kept **one 4-tile block per frame** (6 frames x
4 = 24 tiles, `TILE_ANIM_STRIDE`) and the map cells pointed at the current
frame's block, so a frame change had to rewrite the tilemap of **every visible
cell of the slot** (4 `VDP_setTileMapXY` per cell, and before that even at raw
map coordinates that ran past the plane window into the H-scroll table).
SEAHORSE has 13 slots over a 200-metatile map: ~15 ms of tilemap writes per
frame.

**Fix**: a slot now owns ONE 4-tile block. The map cells point at it
(frame-independent, so `stage_map_patch` no longer looks at `anim_frame`), and
a frame change is a **single 4-tile DMA** into that block. No tilemap writes at
all. `TILE_ANIM_STRIDE` 24 -> 4 also frees 260 tiles of VRAM: `check_vram` now
reports `OK (14 stages, 0 transition fallbacks)` (the three stage-2 fade
fallbacks are gone too).

## BUG-033 — SEAHORSE runs at half speed (animated tiles) — VERIFIED FIXED

**Re-measured in the emulator 2026-10-25** (profile build, stage 3): the busy
time per frame is **642 subticks of 1280** (was 2460, i.e. one frame every other
vblank) and the animated-tile bucket (`anim`) is **61** (was 1135). The stage
holds 60 fps.

Two spikes remain in that run, both in `objects_update`: 2028 subticks twice and
14637 once, at the stage-start position — they line up with the respawn/teleport
path (`stage_teleport_boss` reloads the stage, and `dialog_show_armor` runs its
own blocking input loop), so they are transitions, not per-frame cost.

## BUG-034 — Death respawns at the wrong checkpoint (SEAHORSE)

**Report**: activated the second SEAHORSE checkpoint, died, and came back at
the THIRD one, next to the boss teleporter.

**Cause**: `respawn_player()` honoured the checkpoint only when
`cp_map == stage_current_map`; dying in the next room fell through to that
room's start position. The PC (`game.cpp:395`) treats the checkpoint as the
STAGE-wide respawn point and resets the stage maps, so a death always returns
to the checkpoint room. `cp_active` was also never cleared, so a checkpoint
from a previous stage could survive into the next one.

**Fix**: a new stage clears the checkpoint (`player_clear_checkpoint()` from
`runStage`), and the respawn now loads the checkpoint's map when the player
died elsewhere (`stage_load(cp_map)`), then places him at the stored point.

## BUG-039 — APE BOT crash (line 1111) and a silent quake attack

**Report (2026-10-22 play test)**: fighting APE BOT his quake attack did
nothing, then the game crashed with `LINE 1111 EMULATOR` /
`screenshots/bugs/ape/crash.png` (PC 0xF90008, SP garbage).

**Cause (offline, from the data)**: every stage boss except MILONGA had a
**3-animation** sheet while its `EnemyDef` still pointed at rows 3/4:

```
stage 1 def 1 APE BOT: jump_anim=3 but enemy_apebot_sprite has 3 animations
stage 1 def 1 APE BOT: intro_anim=4 but enemy_apebot_sprite has 3 animations
... 17 mismatches over stages 1..8
```

A sheet is shared by every stage that spawns that art, and the castle stages
spawn the eight bosses as *plain enemies* (3 rows). `make assets` converts stages
0..13 in order, so stage 13's 3-row sheet and SPRITE line overwrote the boss
stage's 5-row one. `SPR_setAnim(spr, 3)` past the end of the animation table
leaves `sprite->animation` on garbage (SGDK only guards it when the lib log
level is high enough), and the next `SPR_update` reads a bogus frame pointer -
the same mechanism as BUG-031. It fires when the boss **jumps** (APE's jump =
his quake attack), which is why it happened mid-fight.

**Fix**: `convert_enemies.py` keeps the TALLEST existing sheet for an art name
and, when it does, leaves its SPRITE line alone, so a 3-row castle pass can no
longer shrink a 5/6-row boss sheet (regardless of stage order). Re-running the
converters restored every boss sheet. `tools/audit_data.py` now cross-checks
every stage's `jump/teleport/intro_anim` against the SPRITE animation counts
(prints `animation index problems: 0`).

**Quake**: the PC's `ia_action_quake_attack()` (the damage + jump lock) has **no
caller** in that build - the observable effect of a QUAKE projectile is the
screen shake (`projectilelib.cpp:677`: +-2 px every 20 ms). The port's enemy
shot did nothing at all. `eshot.c` now exposes `eshots_shake_x()` and
`stage_update_camera()` adds it to `shot_shake_x()`, so APE's quake shakes the
stage like the PC.

## BUG-040 — Boss-intro name is a solid bar

**Report (2026-10-22 play test)**: the intro screen shows now, but the boss name
is just a bar.

**Cause**: the text palette is global state. `draw_names()` in the stage select
set `VDP_setTextPalette(PAL1)` and only the *first* draw restored PAL0 - every
cursor move after that left PAL1 installed. `boss_intro_run()` then loaded PAL1
with the *background* palette, so the name's glyph cells were painted with that
line (white cells with black ink / black on black) instead of the font's black
background + white ink.

**Fix**: `draw_names()` restores PAL0 on every exit path, and `boss_intro_run()`
sets the text plane and palette explicitly before drawing the name.

## BUG-035 addendum — HUD in the player's own colours

The HP bar first moved to PAL0 with the PC's fixed PLAYER 1 colours (which
stopped the sprite recolour but ignored the chosen player). Per the play test
("the HUD should use his colours"), the bar is back on **PAL2** with
**2/3/4 reserved** for it: `tools/convert_player.py` remaps any art pixel on
those entries to the nearest free colour (~700 px over the four sheets, 1-2 px
details), `player.c` writes the selected player's weapon-0 colours 1/2 and black
there, and the hit-flash palettes leave 2/3/4 untouched so the gauge does not
blink. `check_palettes.py` asserts both halves.

## Play-test batch 2026-10-22 (after the boss/animation commits)

Reports, in the user's order, with what is known so far:

1. **BETABOT colours all wrong** — ROOT CAUSE (2026-10-22): not the enemy
   sheet; `rebuild_player_palette()` wrote the HUD bar colours into PAL2 8/9/12,
   entries the player art uses. See BUG-035, fixed.
2. **HP bar broken** — same cause; the bar is PAL0 with the PC's fixed colours
   now (BUG-035, fixed).
3. **Checkpoint still does not animate when activated** — the object logic was
   already correct (anim_loop=1); the two art frames quantized to the same
   colours. See BUG-026, fixed.
4. **Slowdown in CHAOS CITY around the active platforms + small pill**: still
   needs numbers. `make profile` (MD_PROFILE) boots straight into a hardcoded
   stage - set that to 0 first. No code added until the profile exists.
5. **Dialog text background must match the panel colour**; the A-button arrow
   must be WHITE - fixed, BUG-037.
6. **Explosions must move away from the centre** - fixed, BUG-036 (the burst
   was also frozen: the death branch never advanced the puffs).
7. **Stage-select font** (`stage_select_bad_font.png`) - fixed, BUG-014.
8. **SEAHORSE stage intro did not play** (only music) - FIXED, BUG-015: it was
   not the stage mask (the live SRAM shows only CHAOS CITY cleared), it was the
   held stage-select button skipping the intro on its first frame.
9. **After entering SEAHORSE the player sprite is invisible** (projectiles
   visible), then **CRASH** on the second play (`crash_seahorse_second_played_stage.png`).
   Root cause found: the new jump/teleport animation rows for NORMAL enemies
   pushed those rooms past the 504-tile sprite pool, so the sprite engine could
   not allocate the player's frames (invisible) and read garbage frame data
   (ADDRESS ERROR). Fixed by shipping those rows for bosses/sub-bosses only and
   growing the pool to 768 tiles (`SPR_initEx(768)`, stage data ends at 573).
   Needs a re-test.

## Session 2026-10-27 (owner reports, worked one at a time)

- [x] Config menus auto-repeat UP/DOWN (waited only for A/B/START release) - f15f4f21
- [x] Auto-charge behaved like turbo (PC inverts the attack command) - f15f4f21
- [x] APE BOT does not walk on the ceiling: AIR_WALK latched its target at
      `act_t == 0`, but the tick increments act_t before the action runs, so the
      latch never happened and he walked to a stale dest_x (usually already
      reached) until FALL.
- [x] YOU GOT screen: background and text shifted right (plane scroll from the
      stage not reset in got_weapon_bg). Screenshot named by the owner was not
      on disk (only the older screenshots/bugs/you_got_screen.png); cause
      inferred from code, not seen. Needs a re-check.
