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
