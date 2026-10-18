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
| M3 | HUD | DONE (HP balls, boss row, weapon gauge; lives in the in-game menu) |
| M4 | Player attack + buster projectile | DONE (verified in emulator: cooldown, max 3, kill) |
| M5 | Enemies (5 types) — spawn, patrol AI, contact damage, projectile kill | DONE (verified map0=11 / map1=10, contact dmg, kill) |
| M6 | Objects (pills, 1UP, fall/fly platforms, checkpoint) | DONE (verified; + platform bug fixes built, emulator re-verify pending user) |
| M6b | Boss teleporter object (type 32) | DONE (emulator-verified: capsule → arena drop + camera) |
| M7 | Boss MILONGA BOT + stage clear | DONE (boss spawn/AI/HP bar; death -> STAGE CLEAR -> title) |
| M7c | Enemy AI from game_ai_list.dat + enemy shots | DONE (chance/chains/delays; WALK/FLY/JUMP/WAIT_RANGE/SHOT; eshot.c) |
| M7d | Charged shots (PC projectiles 11 semi / 10 full) | DONE (real art, speed 3, dmg 2/3, 2-frame 100ms) |
| M7e | Hit pushback + HIT animation (PC character.cpp:194) | DONE |
| M7f | In-game menu (PC class_config::execute_ingame_menu) | DONE (panel from decomposed weapon_menu art; A = leave stage) |
| M10 | Classic stage select screen (PC stage_select.cpp:357) | DONE (screen + input + cursor; loads stage 0 until multi-stage) |
| M11 | 5-slot SRAM save/load + selector (PC select_save) | DONE (NEW GAME/LOAD GAME -> slot selector; stage clear saves slot) |
| M12 | Data-driven projectiles (game_projectile_list.dat) | DONE (full table; LINEAR/ARC/DIAG/TARGET/FOLLOW/PROGRESSIVE/QUAKE) |
| M13 | Multi-stage architecture + DEEP JUNGLE + FLOWER FIELDS | DONE (stages 0/1/2 convert + load; anim slots, spikes, conveyor, big sprites) |
| M14 | Classic boss intro (PC classic_style_boss_intro) | DONE (bg + stage boss sprite + name, ~6.4s, skipped when beaten) |
| M15 | Player select on NEW GAME (PC select_player) + stage-select eyes follow cursor | DONE (screen + save field; eyes sprite nudges +-2px) |
| M16 | Chosen player wired: sprite/palette/abilities + stage-select face | DONE (all abilities incl. shield; per-player hitbox height) |
| M17 | Weapons (buster + the eight boss weapons) | DONE (table, per-weapon palette, projectile, ammo gauge, menu switch) |
| M18 | All 9 selectable stages converted | DONE (ROM 640K -> 896K; 2 spot-checked in-emulator) |
| M9 | Graphics toolchain: img2tiles + tiles_merge + pal_ui, build pipeline | DONE |
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

- `tools/emu.sh` (source it): `emu_start [rom]`, `emu_start_ready [rom] [tries]
  [wait_s]`, `emu_stop`, `emu_status`, `emu_window`, `emu_key KEY [ms]`,
  `emu_shot NAME`, `emu_shot_blank [NAME]`.
- BlastEm sometimes freezes at startup with an **all-black window** (the ROM is
  fine — relaunching fixes it). `emu_shot_blank` detects it (a capture with a
  single unique colour) and `emu_start_ready` relaunches until the screen
  draws.
- **Virtual desktop**: `EMU_DESKTOP` (0-based; `1` = KDE "Desktop 2") parks the
  emulator window there so its window/keys do not collide with the user's
  session; the default `-1` leaves it on the current desktop. The session is
  **Wayland**: xdotool/wmctrl are EWMH-only and report
  `get_num_desktops == 1` here, `kde-cli-tools`' kstart no longer takes
  `--desktop`, so `emu_move_desktop` drives KWin directly with a one-shot
  script over `org.kde.KWin /Scripting`
  (`w.desktops = [workspace.desktops[N]]`). `emu_key` restores the user's
  previous desktop after sending keys.
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
  - `make` re-runs the asset pipeline when anything in `tools/*.py` or
    `images/*.png` changes, which **regenerates** `src/*_data.c` — hand-edits
    to generated files do not survive a build.

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

## Overnight session (auto-pilot) — bugs fixed

1. **Purple background** — `img2tiles.py` re-quantised art that convert_stage
   had already put in the target palette, and the tile split didn't preserve
   the source *indices*; the night sky came out purple. Fix: indexed inputs
   keep their indices 1:1, and the bg palette is taken from the source art
   (`PALETTE bg0_pal "stage0_bg0.png"`). Verified on the emulator.
2. **Title black bars** — the title art's background colour is the game
   colorkey; punching it made those pixels transparent (backdrop showed).
   `img2tiles --opaque` + `--replace 72,126,124:5,29,65` restores the PC's
   dark-blue background. Verified.
3. **Fall platforms activated the wrong one** — `objects_player_land()` matched
   by Y only, so two platforms sharing a top edge always started the first in
   the array. Now matches the X range too (callers pass l, r).
4. **Checkpoint animation** — its two frames are *states* (inactive /
   activated), not a loop; removed 29 from `is_looping()` so activation shows
   the activated frame and stays.
5. **Enemy attack sprite** — convert_enemies now ships row 2 =
   ANIM_TYPE_ATTACK for all six enemies (+ times + `attack_ms`), and the AI's
   SHOT actions play it and fire inside the animation (~30% in; the PC fires
   on its marked attack frame — close enough, noted as an approximation).
   resources.res SPRITE lines updated with the third time array.

Also done this session: all big images decomposed (title, both parallaxes,
stage maps, menu, HUD) — bg0 768->211 tiles, bg1 896->25, ROM 384K->256K,
plus the latent VRAM overlap of bg1's tiles with the animated-tile block is
gone. GIMP palettes regenerated (md_pal0/1/2 + md_bg0/md_bg1).

## Hand art import + index-preserving sprite converters (2026-10-17)

Goal: hand-edited art under `mega_drive/images/` (indexed, one palette per
group) flows into the build without the converters re-quantizing it and
destroying the identity the art was authored with.

- **Importer** `tools/import_pc_art.py` (manual, overwrites `images/`):
  - p1..p4 keep the two palette-swap placeholders on their own indices:
    **14 = green (55,255,0), 15 = magenta (255,0,255)** (exact-index
    overrides, not nearest-match).
  - reserved indices are never nearest-match targets: PAL0 index 15 (font
    white) and PAL2 indices 14/15 (placeholders). Without this, CIELAB stole
    art pixels into font-white/green.
  - **bug fixed**: P-mode sources with a tRNS transparency index (e.g.
    `backgrounds/menu.png`, transparent index 0 = grey (97,97,97)) were read
    as RGB, so the transparent pixels survived as grey. The importer now
    builds an alpha mask from `info["transparency"]` for P sources.
  - colorkey tolerance: art whose backdrop is the colorkey a few units off
    (logo.png uses (72,126,124) instead of (75,125,125)) is punched too via
    `COLORKEY_TOLERANCE = 6` (per channel). Covers logo.png,
    shared/death_animation.png and the palmtree anim tiles; everything else
    within tolerance is isolated 1px anti-alias fringe.
  - `img2tiles.py` preserves an indexed input's palette entry 0 instead of
    forcing black, so the title logo's backdrop slot keeps CONFIG_BG
    (5,29,65) — the art's own dark blue. bg0/bg1 outputs are byte-identical
    (their entry 0 was already black).
- **convert_player.py**: reads `images/sprites/p1.png`, copies palette
  indices, swaps placeholder index 14/15 -> the weapon-colour indices (6/3)
  **by index** (no RGB lookup). Output pixels are byte-identical to the old
  PC-art version (`res/player.png` verified equal); only palette entries
  14/15 now hold the (unused) placeholders.
- **convert_enemies.py / convert_objects.py**: read `images/sprites/...` and
  `images/projectiles/...` with a new `load_indexed()` (asserts P mode),
  copy pixel indices 1:1, and take PAL0 **from the art** instead of
  rebuilding it (warns if it differs from `tools/pal_ui.py`, which the HUD
  balls still use). No more `load_rgba` + RGB nearest + white-pin pass.
- **Makefile**: `ASSET_DATA` now also depends on `images/**/*.png`, so a
  hand edit triggers the asset pipeline.
- Colour shifts vs the old PC-art output are expected: the art is CIELAB-
  matched to PAL0 by the importer (e.g. classic metool green now maps to
  PAL0 green 13 instead of beige 4; near-white art no longer uses font
  white 15). User to confirm visually in the emulator.
- `tools/gen_gpl.py` writes `res/md_pal2.gpl` with 14/15 = placeholders so
  painters keep them.

## Start menu + save (5 slots + selector, 2026-10-17)

`src/save.{c,h}`: **5 save slots** like PC (`SAVE_MAX_SLOT_NUMBER 4`), each an
8-byte SRAM record at byte `8 + slot*8`: `[magic 'RB'][stages mask][reserved]`.
The title shows NEW GAME / LOAD GAME; LOAD appears only when any slot is valid.
NEW GAME opens the selector (any slot, it is cleared first), LOAD opens it in
"occupied only" mode. The chosen slot is `current_save_slot` in `main.c`;
stage clear writes bit 0 there (`save_slot_set_stage`).

Selector screen (`src/saveselect.c`, art from `tools/convert_save_selector.py`):
PC `scenesLib::select_save` (sceneslib.cpp:755). The enabled/disabled bar
PNGs share pixel indices, so ONE tileset is used with two palette lines
(PAL1 selected, PAL2 normal, index 0 = black interior). Rows at tile row 2,
40px each; "SLOT n" + "NO SAVE"/"N CLEARED"; d-pad wraps, START/A confirms,
B backs out. `saveselect_data.c` holds the packed 4bpp tiles + 40x5 row map +
both palettes. Verified in BlastEm: NEW GAME and LOAD selectors render,
occupied/empty slots read back from SRAM, LOAD slot -> stage select.

**Previous note was wrong**: the ROM header DOES declare SRAM. sega.s
`.incbin`s `out/rom_header.bin`, generated from `src/rom_header.c`
(SGDK `ROMHeader`): `sram_sig "RA"`, `sram_type 0xF820` (odd-byte SRAM),
start `0x00200000`, end `0x0020FFFF`. Built header bytes at 0x1B0:
`52 41 F8 20 00 20 00 00 00 20 FF FF`.

Verified end-to-end:
- reads: seeding `save.sram` (slot magic+mask) drives the selector state;
- writes: stage clear + BlastEm Esc-menu **Exit** persists the slot;
- full flow: persisted slot bit 0 -> title LOAD GAME -> selector ->
  **stage select**.

Important for testing: BlastEm only flushes SRAM on a **normal exit**;
`flatpak kill`/SIGKILL loses the write. `tools/emu.sh stop` quits via the
Esc menu (Exit) first, so SRAM is saved. The harness's `quit` also works.

Still no resume-stage index (multi-stage); the stage select loads stage 0.

## Stage select screen (M10, 2026-10-17)

PC `stage_select::pick_stage_classic_style` (stage_select.cpp:357), the
GAME_STYLE_VINTAGE path the RockBot1 data uses.

- `tools/convert_stage_select.py` composes the static screen from the hand
  art (`images/backgrounds/classic_style_stage_select_bg.png`,
  `images/faces/boss_1..8.png`, `images/faces/p1.png` + eyes) into ONE
  per-tile-palette BG_A tilemap: bg -> PAL3, boss faces -> PAL0, player
  face -> PAL2. Emits `src/ssel_data.{c,h}` (packed 4bpp tiles + u16 map
  with palette bits + the 3 palettes + stage/boss name strings).
  - PC 320x240 is cropped 8px top+bottom to 224; faces use y=40 instead of
    41 so they sit on 8px tile boundaries (1px up, invisible).
  - Cursor `classic_style_stage_select_cursor.png` is 2 frames of 48x48,
    split into 4 x 24x24 quadrant SPRITEs x 2 frames (sprite max is 32x32).
    Each frame is its OWN single-frame SPRITE resource: a 2-frame SPRITE
    auto-animates in the sprite engine (2 frames each) and flickers far
    faster than the intended pulse. The yellow layer is added before the
    black one (lower sprite index = higher priority) so it can cover the
    black brackets when visible.
  - Blink: 48 frames per phase = 0.8s light / 0.8s no-light.
  - Player face eyes are baked in (PC nudges them 2px with the cursor;
    skipped as sub-tile motion).
- `src/stageselect.c`: renders the plane, four cursor sprites (black layer +
  blinking yellow layer, 48 frames = 0.8s per light/no-light phase), d-pad
  navigation with the PC wrap and centre slot skipped for text, and the
  BOSS/STAGE lines over the bottom bar. Returns stage 1..8.
- `main.c`: after the title, if bit 0 of the save mask is set (intro stage
  cleared) the stage select runs (PC `scenesLib::pick_stage` only shows it
  after the intro), else the intro stage starts.
- **Limitation**: `runStage(stage)` still ignores the selection and loads
  stage 0 — only stage 0 is converted so far. Selecting a stage is wired
  through, but multi-stage data/loading is the next task.
- Emulator helper added: `tools/emu.sh` (start/stop/status/window/shot/key)
  — kills by exact `blastem.bin` name and `flatpak kill`, never
  `pkill -f` (which matched the caller shell), detects the NEW window id,
  and launches under `EMU_TIMEOUT` seconds (default 600) so it cannot linger.

## Title backdrop colour (2026-10-17)

The logo's colorkey background was grey because the PC art paints it with
(72,126,124) — see the importer tolerance note. After that fix the title
backdrop was black: the title's visible backdrop is the PAL0 blank-tile
sheet behind the logo, not the logo palette. `showTitle` now loads PAL0 with
entry 0 = CONFIG_BGCOLOR (5,29,65 -> 0x0210) and entry 15 = white, so the
backdrop is PC's dark blue and the font stays white. (Setting the title
logo palette's entry 0 has no effect — rescomp zeroes palette entry 0.)

## Still to do (user list, untouched tonight)

- Multi-stage parameterisation + loading (the stage select currently always
  loads stage 0; converters are stage-0 hardcoded).
- Resume-stage index inside a save slot (5-slot progress save/load is DONE; the stage select still loads stage 0 because multi-stage data is not converted).
- Convert the Ape and Daisie Bot stages.

## Data-driven projectiles (M12, 2026-10-17)

PC `projectilelib.cpp` + `game_projectile_list.dat` (file_projectilev3, 104B).

- `tools/convert_enemies.py` now parses the whole projectile table and emits
  one SPRITE per sane id (0..19): `res/proj_<id>.png`, cell rounded to 8,
  frames from the art strip (horizontal OR vertical — e.g. big_missile is a
  7x28x28 vertical strip). `EnemyProj` carries `traj` and `max_shots` now
  (ENEMY_PROJ_MAX 20). Ids with no art or width >64 are skipped (proj 7
  chain is 70px wide; none of stages 0..2 use it).
- `src/eshot.c` fires by data: `max_shots` (0 = unlimited), `speed`, `dmg`,
  size, `traj`. Trajectories ported:
    LINEAR (0), ARC (1), QUAKE (4, stationary ~1s; screen shake not ported),
    FOLLOW (6, homes to the player), PROGRESSIVE (7, moves on the animation
    reset and snaps to ground), DIAG_UP/DOWN (9/10), TARGET_DIRECTION (13,
    resolved at fire time from the player's y like PC set_target_position).
  Anything else falls back to LINEAR.
  ARC uses the PC trajectory_parabola(RES_W/3) shape, integer-approximated
  (verified: peak ~62px at half distance, crosses 0 at the full distance).
- Player shots are unchanged on purpose (shot.c own sprites/speeds); can be
  folded into the table later.

## Multi-stage architecture (M13, 2026-10-17)

Goal: run stages other than the intro. Converters were stage-0 hardcoded.

- **`src/stage_data.h`** defines every shared type (StageLink, StageBgInfo,
  StageAnim*, EnemyDef/Ai/Proj/Spawn, ObjectDef/Spawn) plus **`StageDef`**,
  the per-stage bundle (maps/tilesets/palettes/terrain/links/bg/scroll-lock/
  tile3+anim tables/enemy+object tables). **`src/stage_data.c`** holds the
  registry `stage_defs[]` + `stage_set_data(stage)`; `stage_def` is the
  current bundle. Terrain macros moved here (shared).
- Converters take the stage index as argv[1] and emit `stageN_*` files:
  `convert_stage.py N` (arrays + a `stageN_def`), `convert_enemies.py N`,
  `convert_objects.py N` (drop the typedefs; types are shared). Projectile
  sprites (`proj_<id>`) are emitted once (stage 0) and referenced by every
  stage's `enemy_proj[]`.
- `stage.c`/`enemy.c`/`object.c`/`player.c`/`main.c` read `stage_def->…`
  instead of `STAGE0_*`; `stage_terrain_at()` is generic in stage.c.
- Makefile runs the pipeline per stage; `res/resources.res` gets per-stage
  TILESET/PALETTE/MAP lines and per-bg `stageN_bgM_pal`.
- **Stage 1 DEEP JUNGLE** converted and loadable (verified in BlastEm via a
  temp `runStage(1)`): parallax trees, terrain, player all render. Selecting
  the stage-select "stage 1" slot now loads it.
- Stage 2 FLOWER FIELDS converts too (3 maps, 10 enemy defs, bgs) but is
  NOT registered yet — see gaps.

### Resolved (2026-10-17)
- **Animated tiles now use PER-MAP slots**: only the visible map needs VRAM,
  so the converter compacts each map's distinct animation ids to slots
  0..k-1 and emits a per-cell slot map (`anim_cell_slot`, 0 = none). Stage 1
  needs 6, stage 2 needs 16 (per-map max). The anim block moved from
  912 (24 tiles) to **TILE_ANIM_BASE 464** (the decomposed-bg region is
  mostly free: max bg is ~211 tiles), giving 19 slots before the sprite pool.
  `stage_map_patch` now patches the cell's own slot; `stage_anim_reset` /
  `stage_anim_update` load the current map's slots. VRAM is fully rebuilt
  per `stage_load`, so nothing accumulates across stages.
- **SPIKE (4)**: damages the player on box overlap (PC `damage_spikes`;
  PC's 188 would be instant death at HP 28, so 4 = 2x contact damage).
- **MOVE_LEFT/MOVE_RIGHT (7/8)**: not walls (already), standable, and carry
  the player 1px/frame while standing (PC `check_platform_move`).
- **RHINOZER** (rhino.png, 96x72) is now emitted: rescomp/SGDK compose the
  frame from multiple VDP sprites; the 64px cap was raised to 128.
- **Stage 2 FLOWER FIELDS** registered: 3 maps, 10 enemy defs, 16 anim
  slots, bgs on all maps. ROM is now 512K (3 stages of art).

### Bug fixes (2026-10-17, user-reported)
- **Background drifting a few px on its own** (Daisie stage): the parallax
  accumulated `camera_dx * speed/10` every frame; when the camera sits on a
  deadzone edge that accumulation drifts/jitters. Now the scroll is derived
  directly from `camera.x * speed/10` (wrapped into (-512,0]) — deterministic,
  no drift. Verified: the sky band is byte-identical across idle frames.
- **Player spawning 1 tile inside the ground** (Ape stage): spawn Y was the
  hardcoded CHAOS CITY walkway (160). `spawn_ground_y()` now scans the spawn
  column for the first ground row and puts the feet there (stage 0 unchanged).
- An emulator crash ("M68K execute at unmapped address") was seen once with a
  temporary on-screen debug overlay; it did not reproduce after removing it
  and running stages 1/2 for several seconds. Watch for it.

- **Walkers fell into cliffs** (Tamandua, Ape stage): `step_axis` only
  checked for walls, never ledges, so AI_WALK enemies walked off edges. PC
  `check_can_move_to_point` (artificial_inteligence.cpp:2052) samples the
  tile BELOW the leading edge and stops (turn around) on a hole unless the
  walk option is HORIZONTAL_AHEAD_FALLING (7). Ported: `step_axis` takes a
  `can_fall` flag (true only for option 7 / flying) and turns the enemy at
  the cliff edge. Verified: Tamandua patrols and turns instead of falling.

- **Daisie-stage graphics artifacts** (mixed tiles, corrupted HUD, palm
  fragments): two causes. (1) stage 2's decomposed bg is **346 tiles**, which
  ran into the animated-tile block at 464. The block moved to
  **TILE_ANIM_BASE 496** (`TILE_BG_MAX_TILES 352` reserved; block ends 928 <
  sprite pool 936). Also `stage.h` had been reverted to 912/8, which made
  stage 2's 16 slots overflow an 8-entry array — fixed. (2) the palm anim art
  uses the near-colorkey (72,126,124); `convert_stage.py` now punches near
  colorkeys (`is_key`, tol 6) like the importer, so no grey boxes.
  `default.png` has no near-key art, so the map tileset is unaffected.

- **Stage-select background looked dark** (same class as the sprite issues):
  our hand-emitted palettes used `R | G<<4 | B<<8`, but MD CRAM wants
  `(R<<1)|(G<<5)|(B<<9)` (white `0x0EEE`) — every colour loaded at ~half
  brightness. Fixed in convert_stage_select.py / convert_save_selector.py /
  tools/pal_ui.py. See the convention note above.

### Still open
- **APE BOT / DAISIE BOT bosses** use the generic `is_boss` path; their
  attack patterns are untuned and untested.
- Stage select slots 3..8 clamp to stage 0 (only 0/1/2 are converted).
- Spike damage / conveyor / stage-2 content verified only by loading, not
  played through.

## Boss intro (M14, 2026-10-17)

`src/bossintro.{c,h}`: PC `classic_style_boss_intro` (sceneslib.cpp:681), the
VINTAGE path the RockBot1 data uses — **classic style chosen over the
non-classic `boss_intro/NN.png` splashes** (kept for reference only).

- Full-screen `images/backgrounds/boss_intro/boss_intro_bg.png` decomposed by
  img2tiles (`boss_intro_bg_tiles.*`, `PALETTE boss_intro_bg_pal`, crop-h 224)
  on BG_A, PAL1.
- The stage's boss sprite: the `is_boss` entry in `stage_def->enemy_defs`
  (STAND frame 0), centred at PC y = 96 - h/2 (shifted 16 for the crop).
  The enemy's optional `sprites/enemies/backgrounds/<bg>` art is not emitted
  yet (skipped).
- Boss name centred at PC y=200 -> tile row 23, from `ssel_boss_names[stage]`.
- Waits 6.4s (PC wait_and_update_screen(6400)), skippable with any button;
  skipped entirely when the stage is already in the save mask
  (PC `game_save.stages[stage] != 0`). Hooked in `main.c runStage()` before
  `stage_load`.

## Player select + stage-select eyes (M15, 2026-10-17)

- `src/playerselect.{c,h}`: PC `scenesLib::select_player` (sceneslib.cpp:546).
  `player_selection.png` bg (img2tiles, own palette, PAL1) + the four
  `player_select_pN.png` portraits (img2tiles, own palettes, PAL2) loaded on
  demand; name under the portrait; Left/Right cycles, START confirms.
  Wired in main.c: NEW GAME -> save selector -> player select -> store the
  choice in the slot (`save_slot_player/set`, slot reserved bytes) -> stage.
  PC gates players 2..4 behind `game_config.game_finished`; the port allows
  all four. Names come from `player_list_v3_1_1.dat` (ROCKBOT / BETABOT /
  CANDYBOT / KITTYBOT) via the generated `player_data[]`.
- Stage select eyes: PC stage_select.cpp:547-559 draws the player-face eyes
  at (154,122) and nudges them +-2px toward the cursor cell. They are now a
  SPRITE (`res/ssel_eyes.png`, 16x8) instead of baked into the face, moved
  per selection. Verified: the eye region changes between cursor cells.
- NOT yet wired to the chosen player: the stage-select face and the in-game
  player sprite are still player 1. Doing so needs 4 player sprite sheets +
  per-player weapon-colour palettes and per-player face/eyes sprites
  (all art is imported).

## Chosen player wired (M16, 2026-10-17)

- **Per-player data is data-driven** from `player_list_v3_1_1.dat`, NOT the
  struct defaults in `file_game_v4.h` (those are the *editor* defaults and
  their P2/P3 labels are swapped vs the shipped game). `file_player_v3_1_1`
  is 6216 B/record (4 records); `tools/convert_players.py` compiles a tiny
  host g++ dumper that includes the PC header, reads the file with the real
  struct and emits `src/player_data.{c,h}` (name, weapon colours, abilities).
  Actual players: **0 ROCKBOT, 1 BETABOT, 2 CANDYBOT, 3 KITTYBOT** (Beta is
  p2, Candy p3).
- **Per-player palettes**: the art uses THREE swap placeholders — green
  COLORKEY1, magenta COLORKEY2 and cyan COLORKEY3 `(0,255,255)` (p2/p3 use
  cyan); only 2 were kept before. `images/sprites/p1..p4.png` now each carry
  their OWN body palette (derived from their art) with the placeholders on
  **indices 13/14/15**; `faces/pN.png` + `pN_eyes.png` get their own palette
  too (eyes reuse the face's). `md_pal2.gpl` is now player-1 only.
- `convert_player.py` emits `player_p1..p4.png` (32x32, 9 anim rows) with each
  player's real `weapon_colors[0]` placed on 13/14/15 so indices copy 1:1
  (quantised to MD 3:3:3): P1 (146,0,109)/(0,109,255)/unset; P2 (255,36,0)/
  (0,0,0)/(219,255,0); P3 (255,109,182)/(109,36,219)/(73,219,73); P4
  (0,146,73)/(146,0,109)/unset (cyan placeholder kept where a colour is unset).
- Runtime: `player_set_number()` / `player_definition()` pick the sheet +
  palette (PAL2 loaded from that sheet); the in-game menu portrait follows.
- **Abilities implemented from the file** (`player_data[]` flags/fields):
  `player_max_shots` (BETABOT/CANDYBOT = 4, others 3), `player_simultaneous`
  (BETABOT = 2), `player_damage_mod` (BETABOT -1, CANDYBOT +1),
  `player_can_charge` (ROCKBOT/KITTYBOT), `player_can_double_jump` (CANDYBOT),
  `player_can_shot_diagonal` (CANDYBOT), `player_can_air_dash` (KITTYBOT),
  `player_can_slide` (ROCKBOT).
- **Stage select**: the centre face + eyes are now SPRITES per player
  (`ssel_face_p1..4`, `ssel_eyes_p1..4`), PAL2 loaded from the chosen player's
  face sheet.
- Projectiles: `full_charged_projectile_id` is 10 (ROCKBOT) / 24 (KITTYBOT,
  invalid id) / -1 otherwise, and every `normal_shot_projectile_id` is -1 or
  an out-of-range id (25/26/35) — the port keeps the safe defaults normal 0 /
  semi 11 / full 10 for all players.
- **simultaneous_shots** (BETABOT = 2 per press) implemented in `shot_fire`:
  the extra shot spawns one tile (16px) BEHIND and 5px lower, per PC
  `character.cpp:761-771`; the max_shots cap still applies.
- **damage_modifier** is DEFENSIVE, applied the PC way (`character.cpp:3001-
  3004`) on the character RECEIVING damage, not on the player's shots: incoming
  damage is clamped to >=1 and the modifier only lands when the result stays
  > 0. BETABOT (-1) takes 1 less per hit, CANDYBOT (+1) 1 more. (Earlier wired
  to shot damage by mistake — that made BETABOT's buster do 0.)
- **DMA-clobber fix** (see Notes/conventions): the boss intro and player
  select were wiping PAL1/PAL2 on an instant confirm, which is why "stage and
  player colours were all wrong". Verified: the NEW GAME flow now renders the
  stage identically to a direct load.

### Player differences — status
- DONE (all from `player_data[]`): `max_shots` (BETA/CANDY=4),
  `simultaneous_shots=2` (BETA), `damage_modifier` (BETA -1 / CANDY +1,
  defensive — reduces/increases damage TAKEN),
  `can_charge_shot` (ROCK/KITTY), `can_double_jump` (CANDY),
  `can_shot_diagonal` (CANDY, Up/Down aims the shot: dy +-3), `can_air_dash`
  (KITTY — see below), `can_slide` (ROCK), `have_shield` (ROCK — see below).
- **Air dash** (KITTY, PC `character.cpp:1668-1735`): it is the SAME slide
  state, not a separate move — A in the air sets `player.sliding` (so the
  ANIM_SLIDE/dash sprite shows), gravity is skipped and vy zeroed for a flat
  horizontal move, up to 5 tiles (`TILESIZE*5`), once per airtime, jump
  blocked mid-dash. Speed = the port's `MOVE_SPEED` for BOTH the ground slide
  and the air dash. (PC's dash is `move_speed*2.5` = 4.125px/frame — PC walks
  at 1.65, this port at 2 — but that ratio reads as too fast here, so the
  port keeps 2; the dash's length came from gravity, not speed.) The anim
  selector had `!on_ground` (JUMP) ahead of `sliding`, which is why the dash
  showed the jump sprite — sliding now wins. Taking a hit cancels the slide
  (PC ANIM_TYPE_HIT replaces ANIM_TYPE_SLIDE) so the dash cannot leave the
  player floating.
- Dash vs slide: PC differs only by hitbox height (24 vs 16px) — this port
  does not vary the box, so ALL players use the slide button/anim.
- **Shield** (ROCKBOT, PC `classplayer.cpp:916`): gesture is **DOWN+A** (a
  3-button pad has no free button; PC has a dedicated key). While held with no
  direction/jump the player enters `ANIM_SHIELD` (sprite frame 22), all input
  is consumed (no walk/jump/attack — PC zeroes the directions at
  `classplayer.cpp:842` and `attack()` returns early), and enemy shots are
  blocked in `eshot.c` instead of damaging (PC `classnpc.cpp:532`). PC reflects
  the shot (`projectile::reflect()`); the port destroys it — reflection is not
  ported (the port's `eshot.c` has no enemy-hit path).
- **Hitbox height**: `box_t`/`box_b` (`update_hitbox()`) follow PC
  `character.cpp:2477`: only a SLIDING player with `can_slide` (ROCKBOT) drops
  the box top by `SLIDE_TOP_ADJUST 12` (PC `SLIDE_Y_ADJUST`); a dash and the
  air dash keep the standing box. A ladder grab cancels the slide (PC
  `character.cpp:1598`) and the box with it.
- NOT modelled: nothing left from the file data. Only weapon 0 (buster)
  palettes are wired; weapons 1..11 use PAL0 art.

## Weapons (M17, 2026-10-17)

- `tools/convert_weapons.py` -> `src/weapon_data.{c,h}` from
  `game_weapons_v301.dat` (33-byte `file_weapon`: id_projectile, name[30],
  damage, id_projectile_charged). 20 records: 0 = NORMAL WEAPON (buster),
  1..8 = the boss weapons, 9..13 = item weapons (coil/jet/tanks), 14.. unused.
  `WEAPON_PLAYER_N 9` = the switchable range.
- Ownership: PC `dataUtil::has_weapon()` = `game_save.stages[n] == 1` — a
  weapon is owned once its stage is beaten. `save_slot_weapon_mask()` returns
  that mask (bit n) plus the always-owned buster.
- Palette: `convert_players.py` now emits ALL 12 `weapon_colors` per player as
  **`s16`** `wcol[12][3][3]` (-1 = unset, because (0,0,0) is a real colour —
  BETABOT's weapon 0 c2 is black). `s8` was wrong: components >127 wrapped
  negative and were skipped as "unset", so the swap silently did nothing.
  `rebuild_player_palette()` copies the sheet palette and overrides indices
  13/14/15 with the selected weapon's colours (PC `change_player_color` swaps
  the COLORKEY1/2/3 entries). `rgb9()` must emit a real CRAM word
  `(R<<1)|(G<<5)|(B<<9)` — verified against rescomp's own
  `player_p1_sprite_palette_data` (index 13 = 0x0608 for (143,0,119)).
- Shot: `shot.c` takes sprite/speed/damage/size/trajectory from
  `stage_def->enemy_proj[weapon_data[weapon].proj]`. QUAKE (stationary ~1s) and
  ARC are ported; everything else falls back to LINEAR. Projectiles with no art
  in the port (7 SPIKED CHAIN is skipped by the converter) fall back to the
  buster's entry so the weapon still shoots.
- Ammo: PC gauge `game_save.items.weapons[]` = 0..28, refilled for EVERY
  weapon at each stage start (`refill_weapons()`, game.cpp:265), consumed per
  shot (QUAKE 4, else 1, classplayer.cpp:466). Only the buster is unlimited
  (classplayer.cpp:336 checks the gauge only for non-default weapons).
- Menu: `menu_open(weapon_mask)` shows the weapon name + gauge and cycles the
  owned weapons with LEFT/RIGHT (PC `find_next_weapon`, class_config.cpp:419).
- NOT modelled: the item weapons (coil/jet/tanks), the PC weapon-grid UI, the
  got-weapon screen, and trajectories other than LINEAR/ARC/QUAKE. Projectile 4
  (MAGIC STAR) is 56x56 — larger than the MD's 32x32 sprite limit; it is used
  by the enemy shots too, so it is not a regression.

## HUD: weapon gauge + lives (M3 extension, 2026-10-17)

The PC HUD is the MODERN style: `draw_enery_ball` rows at x=3 (HP), x=62
(weapon) and RES_W-55 (boss) — `draw.cpp:1202`. There is no on-screen score;
lives are drawn in the in-game menu (`weapon_menu_show_player`, draw.cpp:1021,
"x[N]" next to the portrait).

- Weapon gauge: `hud_weapon(weapon, ammo)` — 5 more balls at x=62, shown only
  for a non-default weapon (PC draw.cpp:1217), fill = ammo / 28.
  Art: `res/hud_wpn_ball.png` (PC `wpn_ball.png`, green fill). The PC tints the
  ball with the weapon's `color1` at runtime; the port keeps the sheet colour
  (see roadmap 11 for the dedup that would also fix the tint).
- Lives: `player_lives()` (5 at stage start, PC `reset_lifes()` normal
  difficulty), one per death (PC `classPlayer::death()`), and
  `player_out_of_lives()` makes `runStage()` leave the stage (PC game over).
  Shown in the menu as `x[N]`.

## All 9 stages converted (M18, 2026-10-17)

The stage select offers 9 slots (`ssel_boss_names[9]`); only 0..2 existed, so
6 slots printed `NOT CONVERTED YET` and START did nothing.

All nine are converted now — `STAGE_DEFINED_COUNT 9`, ROM 640K -> 896K:

| # | stage | boss |
|---|---|---|
| 0 | CHAOS CITY | MILONGA BOT |
| 1 | DEEP JUNGLE | APE BOT |
| 2 | FLOWER FIELDS | DAISIE BOT |
| 3 | ATLANTIDA | SEAHORSE BOT |
| 4 | DESERT | MUMMY BOT |
| 5 | MUMMY | MAGE BOT |
| 6 | SPIKE | DYNAMITE BOT |
| 7 | TECHNO | SPIKE BOT |
| 8 | CASTLE | TECHNO BOT |

Per stage: `tools/convert_stage.py N` (maps/tilesets/terrain/links/bg/anim),
`convert_enemies.py N`, `convert_objects.py N`, the three `img2tiles` bg steps
(Makefile asset rule), one `resources.res` block (new enemy SPRITEs + TILESET/
PALETTE/MAP per map + PALETTE per bg), and the `stage_defs[]` registry.

- **Bug fixed**: `convert_enemies.py` asserted `record[i].id == i`; the list is
  positional and records 59/60 (UFO/FLYBOT) carry 0xFF in the id byte. Stage 4
  spawns one of them, so its conversion aborted. The assert now accepts 0xFF.
- New sprites are shared across stages when the art matches (the converters do
  not rewrite unchanged PNGs), so only genuinely new enemies add SPRITE lines.
  Where one graphic is used with different frame times by two enemies (e.g.
  rock_metool), the FIRST declaration wins and both reuse it.
- Verified in-emulator: stage 3 (ATLANTIDA) and stage 8 (TECHNO) load, render
  and scroll. The other six use the same path but are NOT played through.

## Bug fixes (2026-10-17, user-reported #2)

- **Charged shot / projectiles did not flip when firing left.** The projectile
  art faces RIGHT (the charged shot's tail is on the left — see the column
  averages of `res/shot_charge_full.png`), so `shot.c` now calls
  `SPR_setHFlip(spr, dir < 0)` on both the normal and the charged shot.
  Verified in-emulator: the full charge fired left shows the head left and the
  tail trailing right.
- **A platform lifting the player into a ceiling did not kill him.** The fly
  platform (PC `OBJ_FLY_PLATFORM`, type 11) carries the player with
  `obj_carry_player()`; the platform's own move is wall-checked, but the player
  is taller than it, so his head could enter a solid tile. Now `obj_carry_player`
  checks the head row against TERRAIN_SOLID after an upward carry and calls
  `player_crush()` (PC `damage(999)`, classmap.cpp:1644 — instant death,
  bypassing the invulnerability window). `player_take_damage`'s death path was
  extracted into `player_die()` so both share it.

## Stage-start spawn fix + playtest (2026-10-18)

**Bug (user-reported)**: the player spawned on the CEILING in SPIKE ("Dynamite")
and was nowhere to be found in FLOWER FIELDS ("Daisie") / ATLANTIDA ("Seahorse").

Cause: `spawn_ground_y()` scanned the map **top-down** for the first non-unblocked
tile. A stage with a roof put him on the ceiling; where the first hit was inside a
wall he was buried, fell out of the map and respawned forever.

Fix: scan **bottom-up**, like the PC — `game::set_player_position_teleport_in`
(game.cpp:291) → `classMap::get_first_lock_on_bottom` (classmap.cpp:953): find the
first solid row that has room for the player's box above it and stand him on it.

**Playtest — all 9 stages, CANDYBOT, spawn position**: 0 CHAOS CITY, 1 DEEP JUNGLE,
2 FLOWER FIELDS, 3 ATLANTIDA, 4 DESERT, 5 MUMMY, 6 SPIKE, 7 TECHNO, 8 CASTLE all
spawn the player standing on the ground now. Screenshots in
`/tmp/opencode/md-emu/p*.png`.

Method: a temporary hook in `main()` booted straight into stage N (index in SRAM
word 200, low byte) with CANDYBOT forced — reverted before the commit. Normal flow
re-checked afterwards (NEW GAME → CHAOS CITY with ROCKBOT).

**Playtest — driving each stage** (`/tmp/opencode/md-emu/drive.sh`: boot, then
walk right + jump + fire in bursts, 5 screenshots per stage): all 9 stages run —
the camera scrolls, enemies spawn and damage the player, shots fire, the HP row
drops. No garbage tiles, no lost player, no freeze. Shots: `d<N>_<1..5>.png`.

Still NOT exercised: reaching and fighting a boss, the shield gesture, weapons
beyond the buster (no stage beaten in the save), spike/conveyor damage in
isolation, and the platform crush fix.

Harness: `emu_keys_up` added — BlastEm occasionally misses a keyup, leaving a pad
button stuck; the game then spins in its "wait for release" loop and looks frozen
(that was the false alarm about the pause menu not closing — START opens and closes
it correctly).

## Known issues

- Pre-stage menus (save selector, stage select) have **no cursor SFX**: menu
  beeps use XGM2 PCM, which loads the Z80 driver before any music is
  initialised; BlastEm's Z80 core aborts on it ("multi-byte Z80 instruction",
  writes through the banked area). In-stage SFX/music are fine. Re-add menu
  SFX when the audio milestone initialises the driver properly.

- (none right now)
- FIXED: re-entering the TITLE from the stage drew the logo wrong. The
  title now renders from its own decomposed tiles (src/title_tiles.h/.c),
  the leave path releases the stage map instead of restoring it, and
  showTitle/showStageClear flush a vblank before taking over the planes.
  (Which of the three did it was not isolated — the instrumented runs were
  cut short; the debug telemetry is in src/dbg.c if it ever regresses.)
- NOTES: `dbg_state()` (src/dbg.c) prints VDP state via kprintf in DEBUG
  builds only — do NOT add raw VDP port reads there: they collide with
  kprintf's Gens debug protocol on the control port and corrupt the video
  state (that broke the intro once).

## Palette ownership (importer rules, 2026-10-17)

- **auto16 generator is distinctness-aware**: MEDIANCUT alone splits by pixel
  population and returned near-identical colours (player_select_p3 had pairs
  at dLAB 7-13, wasting slots and flattening the art). `auto_palette()` now
  only SEEDS from MEDIANCUT, merges entries closer than `merge_t` (CIELAB,
  default 14), and refills the freed slots with the image's most frequent
  colours that are still `merge_t` apart (threshold relaxes only if the image
  cannot supply enough distinct colours). p1/p3 now use 15 fully distinct
  colours (min dLAB ~14.5). Low-colour screens (bg/cursor/bars) still pad
  with black, as they have fewer real colours.
- `tools/convert_save_selector.py` no longer assumes the enabled/disabled
  bars share pixel indices (they no longer do): the enabled indices are
  canonical and the disabled palette is rebuilt per enabled index from the
  colour at the same pixel.

- **Shared gameplay lines**: PAL0 = font white + enemies/bosses/objects/
  player+enemy shots/HUD; PAL1 = stage tileset + animated tiles; PAL2 =
  player sprite; PAL3 = parallax bg.
- **Own palette** (alone on screen, can reload CRAM): every full-screen
  SCREEN — `backgrounds/**` (title, menu, config, dialog, stage select bg,
  save-selector bars, cursor, boss splashes, player_select_pN, armor pieces,
  faces front), `scenes/**` (a multi-layer scene must share ONE palette
  across the layers drawn together), `ending/**`, and `logo.png`
  (= PAL_TITLE). The few consumed as sprites/tiles (stage-select cursor,
  save-selector bars, stage-select bg) already get their palette loaded from
  the art at runtime (`ssel_pal_*`, `saveselect_pal_*`).
- **Faces stay GROUPED** (they are drawn together): `faces/boss_*.png` and
  `faces/00..13.png` -> PAL0 (with the font/enemies); `faces/p1..p4.png` and
  `p*_eyes.png` -> PAL2 (one palette loaded for the selected player).
  Only 4 lines exist, so per-face palettes are not possible.
- Rationale for player_select_pN: it is a SCREEN depicting the player, not
  the player sprite — forcing PAL2 wasted 6 entries on duplicated blacks and
  left the art ~9 colours.

- **Queued DMA vs CPU palette writes (recurring class of bug)**: screens
  load palettes/tiles with `DMA_QUEUE`; the queue flushes at the next vblank.
  If the player confirms/skips on the FIRST frame, the flush happens AFTER the
  caller's (CPU) writes and clobbers them — the boss intro and player select
  were wiping the stage tileset palette (PAL1) / player palette (PAL2), so
  stages and players rendered with wrong colours. Fix: each such screen calls
  `SYS_doVBlankProcess()` once right after its loads, before accepting input.
## Notes / conventions

- **Stage defs and LTO**: `stage_defs[]` is marked
  `__attribute__((used))` — with `-flto --gc-sections` the linker dropped the
  stage 1/2 `StageDef`s when every call site had a constant stage index
  (the ROM silently shrank 512K->384K and stages 1/2 vanished). If a new
  data table must survive, mark it `used` or read it through a non-constant
  index.
- **MD CRAM palette format (do not get this wrong again)**: a palette word is
  `(R<<1) | (G<<5) | (B<<9)` with each channel a 3-bit level (white = `0x0EEE`).
  It is NOT `R | G<<4 | B<<8` (that is the same nibbles shifted one bit down,
  so every colour loads at ~half brightness and screens look "dark").
  rescomp `PALETTE` resources already emit the correct format; any palette we
  emit by hand in C (tools/convert_stage_select.py, convert_save_selector.py,
  tools/pal_ui.py `to_vdp`) must use the shift above.

- TERRAIN EASYBLOCK (12) and HARDBLOCK (13) are difficulty pass-through
  blocks — ignore them as geometry (TERRAIN_BLOCKS already excludes them).
  The port has NO difficulty setting: convert_stage.py now normalises both
  values to UNBLOCKED so the runtime only ever sees 0/1/2 (no difficulty
  blocks, no difficulty spikes).
- NO difficulty setting also for spawns: enemies/objects are kept unless
  hard-only (difficulty_level >= 2; 0=easy, 1=normal, 2=hard). Stage 0 has
  a single hard-only spawn (enemy id 47, map0) which stays skipped.
- VRAM map (tiles): system 0..15, stage tileset 16..143, bg 144..911,
  anim 912..935, sprite pool 936..1439 (SPR_initEx(504); TILE_SPRITE_INDEX =
  TILE_FONT_INDEX(1440) - size — TILE_FONT_INDEX is 1440, NOT 1952 as older
  comments claimed), font 1440..1535. The in-game menu temporarily loads its
  38 panel tiles over the bg block and restores it (stage_redraw_bg()).
- Big PC images (menu, dialogs...) are decomposed to unique 8x8 tiles by
  tools/img2tiles.py + a tilemap. WATCH OUT: rescomp's TILESET resource
  deduplicates/reorders tiles by color, which breaks a hand-made tilemap —
  the menu loads its tiles from a generated 4bpp C array with
  VDP_loadTileData instead.
- PAL0 is one shared palette (tools/pal_ui.py) for enemies, objects, shots
  and the HUD balls; the balls used to be quantized against a different
  table and rendered with wrong colors.
- `make` runs the whole asset pipeline (tools/*.py) when a tool or a PC
  source asset changes; `make assets` forces it.
- Debug text overlay (tile pos + HP) and `dbgNum()` removed — kprintf
  debug builds are enough.
- KNOWN (deferred by user): a small artifact on the right side of some
  tileset pillars — likely fixed by the upcoming asset rebuild.
