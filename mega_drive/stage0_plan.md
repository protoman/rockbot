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
| M6 | Objects (platforms, teleporter, boss door) | PENDING |
| M7 | Boss MILONGA BOT + stage clear | PENDING |
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

## M6 — Objects

- id 0/3/9/13/37: identify types in `game_object_list.dat` (pellets?
  platforms? decorations). Port only what stage 0 uses; moving platforms
  need PC object.cpp behavior reading.
- id 41 teleporter: map0 (177,9)→(182,3) — implement if reachable in
  normal progression.

## M7 — Boss

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
