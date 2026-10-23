# PC features the MD port is missing (source audit)

Purpose: the list of things the PC does that the port does not, found by reading
the PC source and the converted data — **not** by waiting for a play-test. Keep
it current: when a gap is closed, tick it and note the commit; when a new one is
found, add it here with its evidence (file:line).

How to regenerate the data half of this list:

    /home/iuri/python3/bin/python3 tools/audit_data.py     # AI actions + options, objects, terrain
    /home/iuri/python3/bin/python3 tools/check_palettes.py
    /home/iuri/python3/bin/python3 tools/check_vram.py

`audit_data.py` prints, per stage, every AI **action** and every **option**
(the `param` field) the data uses and whether `src/enemy.c` implements it. A
green action list does NOT mean the movement happens: the APE BOT ceiling jump
was `AI_ACTION_JUMP` with option `TO_ROOF`, which the port ignored and turned
into a plain hop.

## Player (src/player.c, tools/convert_player.py)

- [x] `WALK_ATTACK` / `JUMP_ATTACK` rows + pose selection
      (`classplayer.cpp:480-489`, 2026-10-23).
- [x] `STAIRS_SEMI` (frame 18): the half-body pose when grabbing or leaving a
      ladder (`character.cpp:402/444`), 12 frames before the climb pose
      (2026-10-23).
- [x] **`TRAJECTORY_CHAIN` (3)** (2026-10-23): SPIKED CHAIN extends for 12
      frames, retracts for 12 and is spent (PC projectilelib CHAIN behaviour
      approximated; the art is not stretched). Every trajectory the player's
      weapons use is now handled (0 linear, 9 quake, 6 arc, 5 zigzag, 1 follow,
      4 centered, 2 freeze, 7 progressive, 8 bomb, 3 chain).
- **Not a gap (verified in the source and the data)**: `ATTACK_DIAGONAL_UP`/
      `_DOWN` (frames 23/24) are only played for weapons with
      `TARGET_DIRECTION`/`TARGET_EXACT`/`ARC_TO_TARGET` (`classplayer.cpp:449-
      461`), and no player weapon uses those (they belong to the enemies'
      missiles 14/15/18, which `eshot.c` already aims). Same for `TURN`,
      `MOVE_SEMI`, `LOOK_UP`, `THROW`, `HIT_SPECIAL`, `GRAB_WALL`,
      `ATTACK_UP/DOWN`: the PC never sets them on the player in this build.

## Enemy AI (src/enemy.c) — from `audit_data.py`

Implemented: WALK (all options), FLY (all options the PC defines), JUMP (all
eleven options), WAIT_RANGE, SHOT_AHEAD/DIR/INVERT, AIR_WALK, FALL, TELEPORT,
DASH, GRAB_WALL, SPAWN_NPC, CHANGE_MOVE, CHANGE_MOVE_REVERSE, REPLACE_NPC,
JUMP_ATTACK_UP, JUMP_ATTACK_AHEAD_ONCE, SAVE_POINT, CIRCLE_PLAYER,
WAIT_RANDOM_TIME, MORPH_INTO_NPC, PLAY_SFX, WALL_WALK, WALL_WALK_SHOOT,
SHOT_MULTIPLE_PROJECTILE, EXPLODE_ITSELF, THROW_ITEM. `audit_data.py` prints
`option gaps: 0`.

- [x] **`AI_JUMP` options** (2026-10-23): `TO_ROOF`
      (`ia_action_jump_to_roof`, `artificial_inteligence.cpp:793`) rises to the
      ceiling with gravity off and KEEPS `no_gravity`, so the following WALK
      state walks him along the roof — APE BOT's missing movement. `UP` jumps in
      place; `TO_PLAYER`/`TO_SAVED`/`RANDOM`/`AHEAD`/`LONG_AHEAD`/`ONCE` steer
      the arc; `TO_PLAYER_DIR` takes the direction at launch.
- [x] `AI_FLY` options `TO_RANDOM`, `RANDOM_X`, `RANDOM_Y`, `PLAYER_DIR`,
      `OPPOSITE_WALL`, `OPPOSITE_SHOOT` (fires on arrival) and `DRILL_DOWN`
      (ghost descent through walls at 1/4 speed).
- [x] AI actions that had no code (2026-10-23): `SAVE_POINT` (marker),
      `CIRCLE_PLAYER` (orbit), `CHANGE_MOVE_REVERSE`, `JUMP_ATTACK_AHEAD_ONCE`,
      `WAIT_RANDOM_TIME`, `MORPH_INTO_NPC`, `PLAY_SFX`, `WALL_WALK`/
      `WALL_WALK_SHOOT` (ceiling walk), `SHOT_MULTIPLE_PROJECTILE` (3-shot
      spread), `EXPLODE_ITSELF`, `THROW_ITEM` (enemy drop table). None of them
      is used by the AI data of stages 0..13 today, so they are untested beyond
      compiling — a re-exported stage can start using them.
- [x] `ANIM_TYPE_TURN` rows (2026-10-23): shipped for every enemy whose art has
      them and played by the AI turn options (WALK TURN/TURN_TO_PLAYER, FLY
      TURN), the facing flipping when the animation ends — BLUE SHIELDER
      rotates its shield, as on the PC.
- [ ] Remaining enemy rows: `VERTICAL_TURN`, `WALK_AIR` (AI_AIR_WALK reuses the
      walk row), `HIT`/`HIT_SPECIAL` (the port blinks the sprite), `STAIRS_*`.
      The PC art of the converted enemies only carries TURN among these (checked
      while adding it), so nothing else is missing in practice.
- [ ] **Quake attack half**: `ia_action_quake_attack()`
      (`artificial_inteligence.cpp:866`) — player damage + 1 s jump lock. It
      has **no caller** in this PC build, so the port only does the screen
      shake (`projectilelib.cpp:677`, added 2026-10-23). Nothing to port unless
      the PC wires it up.

## Enemy projectiles (src/eshot.c)

All trajectories used by the converted stages are handled (0,1,2,4,5,6,7,8,9,
10,11,12,13,16,17). `TRAJECTORY_LASER` (17) is drawn as a straight shot, which
matches its behaviour closely enough to keep.

## Objects (src/object.c)

Every type the converted data spawns is handled (`audit_data.py` lists the
spawned ids per stage and flags unsupported types; it is clean). PC types the
data never uses, therefore unimplemented:

- [ ] `OBJ_FINAL_BOSS_TELEPORTER` (27), `OBJ_ACTIVE_OPENING_SLIM_PLATFORM` (28),
      `OBJ_CRUSHER` (33), `OBJ_MOVING_PLATFORM_UP_LOOP` (34),
      `OBJ_MOVING_PLATFORM_DOWN` (35), `OBJ_DESTRUCTIBLE_NO_DROP` (36),
      `OBJ_DESTRUCTIBLE_WITH_DROP` (37), `OBJ_ACTIVE_PLATFORM_DIAGONAL_UP/DOWN/
      AHEAD/UP` (38-41), `OBJ_JUMP_SHOOT_DESTRUCTIBLE_NO_DROP` (42),
      `OBJ_TIMED_BOMB` (43), `OBJ_EXPANDING_SPIKE` (44), `OBJ_PLATFORM_WALKER`
      (45).
- [x] Boss door opening animation — the PC art has one frame, so the port
      slides the door into the floor (2026-10-23).

## Terrain (src/player.c, src/stage.c)

- [ ] `TERRAIN_QUICKSAND` (9): no sink/slow behaviour. **Not used by any
      converted stage** (`audit_data.py` terrain column), so this is deferred
      rather than guessed at: the PC sinks 0.2px/frame and halves the move
      speed (`QUICKSAND_GRAVITY`/`QUICKSAND_JUMP_LIMIT`).
- **Not a gap**: `TERRAIN_DOOR` (3) only exists for the PC's *tile3* boss-door
      redraw (`classmap.cpp:2069`); the port draws the door as an object
      (sprite) with the slide animation, so there is no terrain rule to port.
- [ ] Water/ice are minimal (sink + slow fall / low friction) — PC has extra
      movement rules (`character.cpp`).

## Scenes (src/scene.c)

- [x] `PLAY_MUSIC` (2026-10-23): the converter maps each
      `scenes_play_music.dat` record to a `MUSIC_*` id (`scene_music_ids[]`,
      3 records: intro scene, DAISIE's track, ending animation), `STOP_MUSIC`
      stops it. `PLAY_SFX` stays a no-op: no converted scene references it.
- [x] `MOVE_VIEWPOINT` and `SUBSCENE`: not used by any converted scene
      (`scenes_data.c` op counter: CLEAR_SCREEN/AREA, SHOW_TEXT, MOVE_IMAGE,
      PARALLAX, PLAY_MUSIC), so they are unimplemented on purpose.
- **Not a gap**: `SHOW_ANIMATION` is **dead data on the PC too** (checked
      2026-10-23). The only op using it is in scene 1 (`Opening Unbeaten LAB
      [#2]`, seek 4 = `rock_awakening.png`), and `game_scenes.dat` maps no
      `e_game_scenes_types` to scene 1: type 1 (intro unbeaten) and type 2
      (intro beaten) both point at scene 0, types 15..18 at scenes 2..5, type 0
      and 19 at 255. The remaining animation records (city bg, two explosions)
      are referenced by nothing. Porting it would build a cutscene nobody can
      reach; the art stays in `data/` should the PC ever wire it up.

## Effects (PC draw.cpp / graphicslib.cpp)

- [x] **Transient effects** (2026-10-23): `src/effects.c` is a small sprite
      pool like `explosion.c`; `tools/convert_assets.py` emits one PAL0 strip
      per art and `tools/check_fx.py` verifies PNG size against the
      `resources.res` frame size (the off-by-one failure mode of the intro
      logos). Wired:
      - **dash dust** at slide start (`character.cpp:1681`): 3 frames of 8x8,
        160 ms, mirrored by the facing, at the PC's adjust
        (`frameSize.height-8` below the top, 3 px behind the art). The source
        art is 8x16 per frame with the bottom half empty — the PC's
        `st_size(8,8)` takes the top rows and the converter ships exactly
        those.
      - **water splash** on entering water: 4 frames of 32x23, 80 ms, two
        rounds, placed so the painted droplets sit on the surface (the PC's
        only trigger is a LIGHTNING projectile reaching the water floor, a
        trajectory the port does not have).
      - **bubble** while the player is inside water (`classMap::
        add_bubble_animation`): one 4x4 bubble, rises 2 px/frame with the PC's
        0..-4 px wobble, dies when it leaves water or after 3 s.
      - **teleport beam** (`draw.cpp:429`, `object.cpp:520`): 2 frames of
        32x32, 100 ms, 3 rounds. The PC draws it at the teleporter while it
        warps; the port warps in one frame, so the beam plays at the
        destination. Its art uses the two colourcycle keys, which the PC swaps
        per object (`set_teleport_small_colors`); the strip is converted with
        the PC's default grey + white pair.
- [x] **Hit flash / dark mode audited** (2026-10-23): neither is missing.
      - The PC's white silhouette (`character::show_sprite_graphic`,
        `character.cpp:1096`) only fires after three blink cycles, because
        `hit_animation_timer` is armed at `character.cpp:3028` under
        `if (now > hit_duration + now)` — a condition that cannot be true when
        it was just set to `now`. The observable PC behaviour is therefore the
        blink the port already does (`invuln_blink()`), with the twinkle the
        PC's own `hit_animation_count` path produces. An MD palette flash would
        also recolour the HUD HP bar, which shares PAL2 entries 2..4.
      - `game.h:273 _dark_mode` is initialised to false (`game.cpp:71`) and
        never set to true anywhere in this PC build, so the "only show
        animation and projectiles" mode is unreachable.
- **Not a gap**: boss ground shake — the PC function at `classmap.cpp:2428` is
  `boss_hit_ground()` (an animation switch), not a screen shake; there is no
  shake to port.
- [x] Quake screen shake (2026-10-23), death ring (2026-10-23), enemy death
      puff, underwater tint?

## HUD / menus

- [x] **Weapon tooltip** (`draw.cpp:833/841`, 2026-10-23): picking a weapon in
      the START menu shows the PC's frame box, the weapon icon and its name
      (from `weapon_data[].name`) for 1500 ms in the lower-left, at the PC's
      position. Art comes from `tools/convert_tooltip.py` (PC
      `weapon_tooltip.png` + the ACTIVE half of `icons.png`): the box is a
      38x4 cell map of 8 deduplicated tiles, the icon is the selected weapon's
      four tiles loaded per show, and all of it sits in the 24-tile window the
      dialog panel uses (912..923). The name is written into the stage plane at
      the current camera position, so the whole tooltip belongs to the map
      while it is up, and `stage_force_redraw()` rebuilds the plane once when
      it expires (one redraw per tooltip, the trade-off noted in the plan).
      The art needs 16 colours and borrows PAL3 (the parallax/colourcycle line,
      as the dialog panel does): saved and restored around the 90 frames, and
      `stage_cycle_update()` holds the colourcycle while it owns the line. The
      background is therefore recoloured for those 1.5 s.
- [ ] The PC's special tooltip names for COIL/JET/E-TANK/W-TANK/S-TANK (extra
      text with the item counts, `graphicslib.cpp:1370`); the port shows
      `weapon_data[].name` only. The tank icons (14..17) are quantized hard
      against the 11 icon slots and are not reachable from the menu anyway.
- [ ] Trophies screen / "characters unlocked" warning (PC `game_config.
      game_finished` gates players 3/4 — the port gates them too, but the
      warning dialog is absent).

## Audio

- [x] Stage music, boss-intro jingle, SFX (XGM2 + PCM).
- [ ] Music/SFX inside cutscenes (see Scenes above).

## Process

The lesson from APE: **"action supported" is not "option supported"**, and a
support table that only lists the enum hides it. `audit_data.py` now prints
both, so the next gap of this kind shows up offline. When a play-test finds
something, also ask "which PC enum/option/field did we not read?" and add it to
the audit, not just to the code.
