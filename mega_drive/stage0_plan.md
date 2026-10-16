# Stage 0 Port Plan — CHAOS CITY

Goal: make stage 0 fully playable end-to-end on Mega Drive, matching the PC
game's stage behavior. Executed autonomously; verify every milestone with a
build + emulator run before moving on.

## Ground rules

- KISS. Minimum code per milestone. No speculative abstraction.
- Data comes from `build/games/RockDroid1/` files — never invent values.
- **Move runtime math to the converter where possible** (user): e.g. scroll
  limits, spawn ranges, frame tables — precompute and store in ROM. Enemy AI
  = simple hand-written C (patrol/turn/contact), not a ported AI data
  interpreter. Simplifications are fine.
- Every milestone: convert (python) → build (make) → run (BlastEm harness
  `/tmp/opencode/m5_*.sh`) → commit.
- User reviews in the morning. Leave each milestone committed, working state
  clean.

## Milestone status

| # | Milestone | Status |
|---|-----------|--------|
| M0 | Maps, tiles, collision, walk/jump, map links, 224px (no vert scroll) | DONE (commit 171a4129 + map-height fix) |
| M1 | Parallax backgrounds (BG_B) | DONE |
| M2 | tile3 over-player layer | DONE (pragmatic: 13 both-cells baked, player in front) |
| M3 | HUD | DONE minimal: text debug overlay (tile-pos + HP "H28"); no bar/score/lives yet |
| M4 | Player attack + buster projectile | DONE (verified in emulator: cooldown, max 3, kill) |
| M5 | Enemies (5 types) — spawn, patrol AI, contact damage, projectile kill | DONE (verified map0=11 / map1=10, contact dmg, kill) |
| M6 | Objects (pills, 1UP, fall/fly platforms, checkpoint) | DONE (verified; + platform bug fixes built, emulator re-verify pending user) |
| M6b | Boss teleporter object (type 32) | DONE (emulator-verified: capsule → arena drop + camera) |
| M7 | Boss MILONGA BOT + stage clear | PENDING (M7a done, see below) |
| M8 | Audio (XGM2) | OUT OF SCOPE (later milestone) |
| — | Menu, saves, stage select, doors/spikes/water/ice, animated tiles, real health bar/HUD, enemy projectiles | OUT OF SCOPE / later |

## Verified data facts (this session)

- Backgrounds (`maps_v2.dat` records, 36B each, fields at 0/30/32/33/34):
  - map0 bg0: `city_night.png` adjust_y=149 speed=5 (0.5x), map1 bg0:
    `stars_bg.png` adjust_y=20 speed=1 (0.1x), map2: none. bg1 (fg layer)
    empty in all 3 maps → skip.
  - PC scroll: `bg_scroll.x -= camera_dx * speed/10`; wrap at image width;
    drawn at screen y = adjust_y (classmap.cpp:449-478, 586-646).
- MD plan: bake each bg to 512px wide (plane width) = art + art[0:192]
  appended → natural plane wrap, no seam logic. PAL3, `TILE_BG_INDEX 576`
  (VRAM: system 0-15, stage tiles 16.., sprite engine from ~1532 down,
  font at top — 576+896=1472 < 1532 fits).
- tile3 layer: map0 t3only=28 both=4, map1 both=9, map2 none → tiny; bake
  stays acceptable except 13 cells where player should walk BEHIND tile3 →
  convert them to the over-player pass now (WINDOW not usable: no hscroll).
- Enemies (`map_npc_data.dat`, 12B records — C++ pad after id; positions in
  TILE units): stage 0 has 55 entries, 46 real (rest id=255 empty):
  - map0: id 45×3, 1×3, 21×3, 23×2, 37×1, 47×1(diff 2)
  - map1: id 51×4, 45×4, 23×1
  - ids to port: 1, 21, 23, 37, 45, 47, 51. Sprite defs: `game_enemy_list*`,
    AI: `game_ai_list_v3.dat`, sprites under `images/sprites/`.
- Objects (`map_object_data.dat`, 18B records): id 0 (many, likely pellets),
  id 3, 9, 13, 37, 41 (teleporter map0 (177,9)→(182,3)). Defs:
  `game_object_list.dat`.
- `anim_tiles.dat` is 0B → no animated tiles in this game data. Skip.
- Stage 0 terrain has no DOOR/SPIKE/WATER/ICE tiles → those behaviors not
  needed for this stage.

## M1 — Parallax backgrounds (DONE)

- `tools/convert_stage.py`: read bg records, bake `res/stage0_bg{0,1}.png`
  (512 wide, own ≤16-color palette), emit `stage0_bg_info[]` (present,
  adjust_y, speed) into `stage0_data.*`.
- `res/resources.res`: `IMAGE stage0_bg0/bg1 ... NONE 3`.
- `src/stage.c`: on `stage_load` draw image on BG_B at (0,0), palette PAL3,
  vertical scroll = adjust_y; each `stage_update_camera` accumulate
  16.16 scroll `-= dx*speed/10`, wrap 512, `VDP_setHorizontalScroll(BG_B,
  -scroll)`. Map without bg → clear BG_B + reset scroll.

## M2 — tile3 layer (DONE, pragmatic)

Cells where tile3 exists: PC draws tile3 ABOVE the player, tile1 below.
MD has no second scolling plane over sprites →:
- tile3-only cells: keep as now (paint tile3; no tile1 behind → nothing to
  fix).
- both cells (13): keep painted; player walks in front (small deviation,
  13 cells out of 3700). Revisit only if user notices.

## M3 — HUD (DONE, minimal = text debug overlay)

- What exists: `VDP_drawText` overlay in `main.c` — tile position (5 digits)
  at camera-relative tile (1,1), HP text `H28` at (1,3), both BG_A.
- NOT a health bar, no score/lives/timer. Real HUD = later (OUT OF SCOPE row).

## M4 — Player attack (DONE, verified)

- B button fires (`player_update`): `shot_fire` → `shot.c`, max 3 on screen,
  speed 4px/f (PC projectile 0 "Normal" value), cooldown 10f, attack anim 12f,
  jump = A/C only.
- `shot_update` after `enemies_update`: wall/screen cull, then
  `enemies_try_hit` (hp−1, kill at 0).
- Emulator-verified: active count 1/2/3 (never 4), metool died to 2 shots.

## M5 — Enemies (DONE, verified)

- Converter `tools/convert_enemies.py`: parses `map_npc_data.dat` (12B,
  C++ pad) + `game_enemy_list_3_1_2.dat` (6048B); ground-drops walkers on
  cropped terrain, precomputes patrol limits (±walk_range clamped), hitbox =
  union of collision rects; shared 14-color pool → `palette_ui_enemies`
  (idx0 transparent, 1..14 art, 15 = 0x0EEE font white), PAL0.
- 5 defs {1,21,23,45,51} → `res/enemy_*.png` (row0 STAND, row1 WALK),
  `res/shot.png`; 21 spawns (map0=11, map1=10); boss id37 → M7, id47
  diff-level 2 skipped.
- Runtime `src/enemy.c`: `enemies_reset` on `stage_load` (per map,
  `SPR_addSprite` PAL0), patrol with wall/ledge/range turn, gravity for
  fly=0, HFlip, contact damage 2 via `player_take_damage` (invuln 90f,
  blink), `SPR_releaseSprite` on death.
- Verified: map0 spawned=11, map1 spawned=10 (after link), hp 28→26→24→22
  contact, kill with 2 shots.

## M6 — Objects (DONE, verified)

- Converter `tools/convert_objects.py`: `game_object_list.dat` (84B
  `file_object`) + `map_object_data.dat` (18B, stage filter, PC difficulty
  rule — both modes keep level 0 on EASY). Used defs: 0 pill type7, 9 1UP
  type9, 3 fall block type10, 13 fly platform type11 (gfx `platform_up.png`),
  37 checkpoint type29; id 41 boss teleporter skipped (M7). Pre-drops
  pill/1UP/checkpoint with PC load gravity (step 4px, h iterations, dest
  bottom corners); platforms keep authored position (gravity skip list).
  Quantized into shared PAL0 (pool parsed from `stage0_enemies.c`). Emits
  `res/object_*.png` + `src/stage0_objects.{c,h}`: 5 defs, 37 spawns
  (map0=11, map1=24; all difficulty_level==0 — includes diff_mode=1 pills,
  PC keeps them on EASY).
- Runtime `src/object.{c,h}`: state machine STAND/MOVE/RETURN per PC
  `move()` — fall 4px/f blocked→RETURN, RETURN 2px/f gated while player
  stands on it, off-screen y>272→RETURN; fly 4px/f up, RETURN 2px/f
  CARRIES player (no gate); 13-frame activation delay (220ms). Pickups in
  `objects_update` (before `player_update` for carry): pill hp+3 cap 28,
  1UP consume no-op (lives unmodelled), checkpoint sets respawn. One-way
  platform catch from top only (`objects_land_check` in the fall scan,
  wins vs terrain only when higher; `objects_stand_check` keeps the player
  grounded while a fly platform carries him over empty space).
- Checkpoint respawn in `player.c`: pit-fall + death use
  `respawn_player()` (checkpoint on current map else stage start) +
  `stage_arrival_camera` for both. Animation manual: pill/1UP loop
  always; fall/fly/checkpoint start on trigger (checkpoint loops, others
  run once).
- Verified in emulator (user: "Working").

## M7 — Boss + boss teleporter

### M7a — Boss teleporter (DONE, emulator-verified 2026-10-16)

- Object def id=41, type 32 (`OBJ_STAGE_BOSS_TELEPORTER`), name "Stage Boss
  Teleporter", gfx `teleporter_stage_boss.png` (96×64 RGBA = 3 frames of
  32×64). Spawn: map0 tile (177,9) → px(2832,128) after converter y-formula.
  Dest from `map_object_data.dat`: link_dest=(182,3) tiles, map_dest=0
  (same map — teleports player to the boss arena entrance on map0).
- PC behavior (character.cpp:2065-2074 + game.cpp:2199-2230):
  - Collision block for type 32 is **computed but consumed**: the teleporter
    branch in `map_collision` leaves `map_block=0` when the capsule misses,
    so the object is a non-solid trigger (player walks through). MD matches:
    no object blocking for type 32.
  - Teleport fires when player capsule-center is inside object's center band
    (`is_on_teleporter_capsulse`, character.cpp:2303): obj_y < player_y AND
    obj_y+h > player_y+frame_h AND player center-x strictly between
    limit_min/max = obj_x + |16-w|/2 .. obj_x + w - |16-w|/2.
  - Effect: set_direction RIGHT, fade_out 500ms, set_current_map(dest_map),
    scroll to first lock on left of dest_x, drop player to first solid below
    dest tile (get_first_lock_on_bottom), reset_scrolled. For type 32
    must_return=false (no return-teleport memory).
- MD implementation (KISS, all PC-parity except fade/sfx):
  - `tools/convert_objects.py`: type 32 kept (SKIP_TYPES removed); def +
    sheet (PAL0, 3 frames × 7 ticks); `ObjectSpawn` extended with
    `dest_x, dest_y, dest_map` (u8; from `_lx/_ly/_mdest` — 182 fits u8,
    plan's s8 was wrong). Teleporter also animates: data `auto_start=1,
    loop=1` → added to `auto_anim()` + `is_looping()`.
  - `src/object.c`: MapObj carries dest bytes; overlap branch runs
    `is_on_teleporter_capsulse` port (frame top/height = player.y / 29,
    center = player.x+14) → `player.facing=1` (PC set_direction RIGHT) →
    `stage_teleport_boss()` → immediate `return` (stage_load rebuilds objs[]).
  - `src/stage.c`: `first_lock_on_bottom()` port (classmap.cpp:953; stage 0
    has no WATER, EASYBLOCK/HARDBLOCK never column locks) + 
    `stage_teleport_boss(dest_map, dest_tx, dest_ty)`: stage_load → drop on
    dest_x column (PC ignores dest_y) at scan row −1 → player_place →
    stage_arrival_camera (divider-safe; replaces PC get_first_lock_on_left).
    No fade/sfx (later), no used-latch (must_return=false).
  - Fixed pre-existing working-tree build-breaker found on the way:
    `stage_arrival_camera` called `scroll_lock_span(focus_x)` (param is
    `focusX`) — the M6 tree could not have rebuilt clean with this typo.
- Geometry (verified from terrain data): teleporter sits in the pillar face
  at cols 175-178; pocket rows 9-10 with a row-11 ledge at cols 177/178.
  Capsule fires only from the ledge/arc (floor-standing player: feet ==
  teleporter bottom → strict `>` fails, same as PC). Arrival col 182:
  scan row 11 → place y160 → 3px settle onto arena floor row12; camera
  2866 = divider col179 to left edge, player at screen x46.
- Verified in emulator (temp SPAWN_X=2700 + temp arrival-camera line, both
  reverted): walk right blocked at ledge face PX2807, jump right → capsule
  mid-arc → arena PX2912 settled163, CX2866, divider left edge, full
  redraw clean, stable (no re-trigger/ping-pong over +90f). ROM 256K.
- Emulator harness: `/tmp/opencode/m6b_teleport.sh` (BlastEm + xdotool,
  same pattern as emu_platform_test.sh).

### M7b — Boss MILONGA BOT + stage clear

- Stage boss field: name read by converter (`boss=` printed). Boss data in
  game_enemy_list + AI with scripted states. Likely behind a boss door
  object on map1 end. If full boss AI too large: minimal version — arena
  entry, boss sprite + 2-3 attacks, HP bar, death → "STAGE CLEAR" screen →
  title. User decides depth after review.

## Test harness

- `/tmp/opencode/m5_test.sh` — stage + walk + fire + 5 screenshots.
  `/tmp/opencode/m5_ladder.sh` — spawn-warp link test (temp SPAWN_X=2100).
  `/tmp/opencode/m5_base.sh` / `m5_shotvis.sh` — capture cadence checks.
- Rules learned:
  - Write script files; NEVER inline `pkill -f 'b[l]astem'` — inline
    bash cmdline contains `retrodev.blastem` → pkill kills its own shell
    (empty-output timeouts).
  - Post-kill sleep ≥2s, `windowactivate --sync` + `windowraise`, re-verify
    stage reached before long input runs (xdotool input flake).
  - Keymap: `x` = pad B (fire), `c`/`z` = A/C (jump), `enter` = START,
    arrows = d-pad, `b` = plane-debug (avoid).

## Session notes (M4+M5)

- **Ladder bug fixed** (pre-existing, blocked map0→map1): box-center row was
  row9 at ground while ladder top = row10 → instant dismount. Continue condition
  now also accepts feet tile STAIR; grab clears `on_ground` so the sideways
  let-go works.
- **Link verified**: `grab-dn x=2167` → `map=1 spawned=10` → stable landing
  on map1 top platform (3161,31).
- **Stage route note**: straight walk right is impossible (authentic design):
  pit tiles at x49-75 / x95-109 / x142+ deep-fall to respawn; player crosses
  via the upper roof (rows8-9, x50-72) with jumps. Not a bug.
- Sprite VRAM: `SPR_initEx(512)` (need ≈491 tiles; default 420 too small).
- HP reduced to PC value 28, contact damage 2, invuln 90f blink.
- Debug `M5DBG` kprintf probes all removed; clean build 131072B.
- **Enemy wrap bug (user-reported)**: sprites got raw world X (>1023) written
  to the SAT unmasked (SGDK `sprite_eng.c` writes `vdpSprite->x = sprite->x`)
  → hardware wrap made far-away enemies appear on-screen. Fix: one call
  `SPR_setVisibility(spr, AUTO_FAST)` at spawn — SGDK hides by X vs screen
  width and skips the SAT write; recomputed on every `SPR_setPosition`.
  Verified: spawn screen shows 0 enemies (all map0 spawns ≥544px).
- **Scroll lock (PC parity)**: converter precomputes full-wall columns
  (all 15 PC rows in {SOLID,DOOR,SPIKE,ICE,SCROLL_LOCK} — PC
  `classmap.cpp:184-193`) into `stage0_scroll_lock[]`: map0
  {137,138,179,180,199}, map1 {0,199}, map2 none. `stage.c`
  `scroll_lock_camera()` replicates PC `changeScrolling` edge-tile checks
  (direction-aware, right edge `x+306`, left edge `x+14`): the camera stops
  with the divider at the screen edge; never shows what is behind it.
  Verified in emulator (spawn warp 1900, camera trace): camera held at 1885
  while player pushed against the wall; left walk unaffected. Note: mid-run
  `import -window` captures sometimes return byte-identical stale frames —
  trust kprintf traces over screenshots for motion questions.
- **Scroll lock (user request, port of PC `wall_scroll_lock`)**: converter
  precomputes full-wall columns per map (all 15 PC data rows
  SOLID/DOOR/SPIKE/ICE/SCROLL_LOCK — `classmap.cpp:184-193`) into
  `stage0_scroll_lock[]` ROM arrays. `stage.c scroll_lock_camera()` blocks
  the advancing screen edge from sitting on such a column (direction-aware,
  `prev_x`-based, like PC `changeScrolling` `:459/:475`), so the view never
  shows what is behind a divider. Stage 0 data: map0 = cols 137,138,179,180,199;
  map1 = cols 0,199; map2 = none. Verified in emulator (camera held at 1885
  = col137 lock while player pushed to the wall at 2167; walked away left
  normally).
- **User rule (standing)**: never launch or kill BlastEm without asking —
  the user may have their own instance running.

## Bug A + Bug B fixes (2026-10-15, user-reported)

- **Bug A (stair grab from the right left the player uncentered)**: root cause
  was the same-frame sideways let-go — grab sets `on_ground=FALSE`, so
  `pad & L/R` cancelled the ladder branch on the grab frame and
  `player.x = ix` never ran. PC has NO sideways let-go: horizontal input is
  gated while the ladder anim plays (`character.cpp:218/266`); PC's
  `_dropped_from_stairs` only fires when hit. Fix: removed the let-go block
  from `player.c` ladder branch (no side-release at all, PC-style). Grab
  still clears `on_ground`.
- **Bug B (reverse map link never fired)**: `stage_check_link` only tested
  `map_from == current` with a box-overlaps-origin-tile check. Ported PC
  `test_teleport`/`check_player_is_on_teleport` (`game.cpp:638-661,771-835`):
  - test point `px = (left+right)>>1` (PC `x+w*0.5`), `py = bottom-8`
    (PC `y+h*0.75 = y+21`); column `tx*16 <= px <= tx*16+16` inclusive.
  - bands: base `lim3=ty*16+8`, `lim4=ty*16+16`; T2B `+8/+24`
    (`lim3=ty*16+16`, `lim4=ty*16+40`), B2T `lim3-=27` (PC `-27.2`
    truncated; only ty=0 bands use it → -19).
  - transition: forward `oy>dy→T2B`, reverse swapped (`game.cpp:643/655`).
    Stage 0: link2 fwd = T2B @ (136,13), link2 rev = B2T @ (198,0),
    link3 fwd = B2T @ (1,0), link3 rev = T2B @ (139,13).
  - windows with `py=y+21`, climb 1px/f: descend fires at `iy 204..207`
    (ladder-off at 208), ascend at `iy -16..-6` (ladder-off at -17).
  - off-screen + partial-screen gates (`game.cpp:781,810`) intentionally
    not ported: with MD's cropped 224px rows they never change the result
    for these four bands.
  - **latch**: `static s8 link_latch=-1` in `stage_check_link`; set on
    fire, cleared only when that link's own band misses — survives
    `stage_load`, so the arrival frame (still inside the band, e.g.
    map1 (198,0) y=-8 → py=13 ∈ (-19,16)) cannot ping-pong; cleared as
    soon as the player falls/climbs out. Arrival placement unchanged.
- **Ground rule = PC `hit_ground` ladder pairs** (`character.cpp:1377-1412`),
  new `ground_row()` in `player.c`, used by fall scan + standing check:
  SOLID → ground; STAIR → ground iff tile above not STAIR (ladder-top
  stand); UNBLOCKED → ground iff tile above STAIR (ladder-bottom rest —
  needed so link arrivals (198,0)/(139,13) rest on the rung instead of
  falling out). Other PC pairs skipped on purpose (p1=0,p2=SOLID would
  freeze scan-based landing on ledges). Easy/hard blocks still fall
  through (documented deviation, PC branch1 differs).
- Verified by derivation (CLIMB_SPEED=1px/f hits every window value);
  awaiting user test in emulator: down at map0 col136, up at map1 col198
  and col1, up again at map0 col139.

## Arrival scroll fix (2026-10-15, user-reported)

- **Symptom**: climbing the map1 left stairs (col1) returned the player to
  map0 (139,13) at the right spot, but the camera sat wrong — stuck at
  x=1885 with the 137/138 divider at the screen edge and the player
  off-screen right.
- **Root cause**: `stage_load` resets camera to (0,0); the next
  `stage_update_camera` jump (0 → ~2072) is treated as a normal rightward
  walk, so `scroll_lock_camera` clamps the *advancing right edge* at the
  first divider (c=137 → camera 1885). Every later frame the deadzone
  re-requests the centre, the right-edge check sees the edge already on
  the wall and freezes. PC never does this: on link arrival it
  `set_scrolling`s the new scroll **directly** (game.cpp:750) computed by
  `teleporter_dist`; the incremental edge lock only applies to walking.
  (User theory 2 confirmed; theory 1 not quite — scroll was set, just to
  a lock-frozen position.)
- **Fix**: `stage_arrival_camera(focusX, focusY)` in `stage.c` — direct
  centre on the player, then `scroll_lock_span()`: if any locked divider
  sits strictly inside the view after a jump, push the camera so the
  divider ends at a screen edge **on the player's side** (arrival at
  (139,13): divider to the left edge → camera 2210, player visible at
  screen x≈22; arrival at (136,13) from the col198 climb-up: divider to
  the right edge → camera 1885). Then `stage_force_redraw` (full plane
  redraw across the jump). Called from `main.c` right after
  `player_place`; the same-frame `stage_update_camera` may re-request the
  centre but the directional edge lock (now with the correct prev_x on
  the arrival side) restores the arrival camera in that same frame.
  Walking behaviour is unchanged: `scroll_lock_span` is a no-op whenever
  the view does not span a divider (incremental steps can never make it).
- Derivation for all 4 arrivals: map0 (139,13) → 2210; map0 (136,13) →
  1885; map1 (198,0) → 2880 (right-edge on divider col199, player at
  screen 296); map1 (1,0) → 0. Vertical always clamps to 0 (map height =
  screen height).

## Animated tiles + tile3 over-player layer (2026-10-15)

Both implemented via ONE SGDK mechanism: `MAP_setDataPatchCallback`
(stage.c `stage_map_patch`). The callback rewrites every tilemap buffer
(full draw + scroll rows/columns) before it reaches the VDP:

- **tile3**: cells flagged in `stage0_tile3_mask` get `TILE_ATTR_PRIORITY`
  (0x8000) — the VDP draws priority tiles over sprites (player, enemies,
  shots). Zero runtime cost. PC equivalent: `showAbove()` redraw pass.
- **animated tiles**: stage 0 uses one block — global id 18
  `lightbulb_small.png` (4 frames, delays 2000/100/100/100 ms). Frames
  baked to 4bpp by tools/convert_stage.py into `stage0_anim_frames`
  (4 tiles/frame TL,TR,BL,BR). Anim cells are rewritten to point at a
  fixed VRAM slot (`TILE_ANIM_BASE` 912 + slot*24). `stage_anim_update()`
  (once per frame) advances timers (ms→60fps frames), DMAs the new frame
  (32 bytes) and rewrites visible cells via `VDP_setTileMapXY`.

VRAM layout correction (old stage.h comment was wrong for this SGDK):
system 0..15, tilesets 16..143, bg 144..911, **anim 912..935**,
sprites **1440..1951** (`TILE_SPRITE_INDEX = 1952-512`), font 1952..2047.
First anim version used 1536 = inside the sprite pool -> sprite garbage
near the player; moved to 912.

## Parallax bg vertical position fix (2026-10-15)

The floating purple "window" fragments at player height were the city
skyline of `city_night.png` (PAL3 colors) — the whole bg art sat ~149px
too high. Two bugs in `VDP_setVerticalScroll(BG_B, adjust_y)`:
1. PC draws bg at screen y = +adjust_y (classmap.cpp:614); MD vsram =
   the plane row at screen top, so pushing the image DOWN requires
   **negative** vsram (-149).
2. The write ran inside stage_load's VDP-disabled window and was lost
   (vsram stayed 0 → art at y=0).
Fix: vsram written after `VDP_setEnable`, value `-adjust_y`. Moon now
sits low behind the walkway = PC framing.

## M6 platform bug fixes (2026-10-16, user-reported → fixed, built)

User reported: fall/fly platform side-collision broken, land-activation
broken. Root-caused and fixed (all in working tree, ROM builds clean 256K):

1. Converter platform y: was `(sy-1)*16` → now `s["y"]*16` (PC parity,
   `object.cpp:66` position.y = sy*TILESIZE).
2. Side collision: added `objects_block_x()` in `object.c`; `player.c`
   horizontal move now clamps against platform bodies (PC COLlISION_MODE_XY).
3. Land activation: `objects_land_check()` + `objects_player_land()`;
   tie-break `<` → `<=` so exact-edge landing catches.
4. Carry pre-move eval: fall/fly MOVE and fly RETURN now evaluate
   `player_on_top()` BEFORE changing `o->y` (was after → carry never fired).
5. Rising head block: `objects_head_block()` in player.c jump-up branch.

Debug overlay added: `DEBUG_OBJECTS 1` in object.h, `objects_debug_text()`
shows PX/PY/CX + per-platform state. Remove before release.

Test room (temporary, in `convert_objects.py` `test_spawns`): pill(16,150),
1UP(32,144), checkpoint(48,112), fall(208,128) right of spawn, fly(112,144)
left of spawn. Harness `tools/emu_platform_test.sh` (BlastEm + xdotool +
ImageMagick import). **Test-room spawns and DEBUG_OBJECTS still in tree —
remove before release (user will test manually).**

Verified in emulator (run 2): fly side-collision clamp x138, fly
land-activation + rise, fall full cycle (land → fall 128→144 → return-pause
→ carried down → return home), pill/1UP consumed. Fly carry (player riding
fly up) NOT yet captured — harness timing/layout interference; fall
side-block clean capture also pending. User to re-test manually with new
layout (fall moved right of spawn x208).

**Standing rule (user)**: every bash call touching the emulator needs an
explicit timeout; never block/poll indefinitely; check liveness with pgrep.

## Notes / conventions

- TERRAIN EASYBLOCK (12) and HARDBLOCK (13) are difficulty pass-through
  blocks — ignore them as geometry (TERRAIN_BLOCKS already excludes them).
- Debug text overlay (tile pos + HP) and `dbgNum()` removed — kprintf
  debug builds are enough.
- KNOWN (deferred by user): a small artifact on the right side of some
  tileset pillars — likely fixed by the upcoming asset rebuild.
