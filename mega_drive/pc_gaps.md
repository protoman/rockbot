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
- [ ] `MOVE_VIEWPOINT` and `SUBSCENE`: not used by any converted scene
      (`scenes_data.c` op counter: CLEAR_SCREEN/AREA, SHOW_TEXT, MOVE_IMAGE,
      PARALLAX, PLAY_MUSIC, SHOW_ANIMATION only), so they are unimplemented on
      purpose. `SHOW_ANIMATION` **is** used once (op in scene 0) and is still
      missing: it needs the PC `images/scenes/animations/` art converted
      (`scenes_show_animation.dat`: 6 records, frame_w/h + delay) plus a timed
      frame loop on BG_B.

## Effects (PC draw.cpp / graphicslib.cpp)

- [ ] Dash dust (`dash_dust.png` 24x16), water splash (`water_splash.png`
      128x23), bubble (`bubble.png` 4x4), teleport-small colour flash
      (`draw.cpp:429`), dark mode (`game.h:273`), hit flash on the sprite
      palette (the port blinks the sprite instead). All need a sprite-effect
      pool like `explosion.c` plus one converted art each.
- **Not a gap**: boss ground shake — the PC function at `classmap.cpp:2428` is
  `boss_hit_ground()` (an animation switch), not a screen shake; there is no
  shake to port.
- [x] Quake screen shake (2026-10-23), death ring (2026-10-23), enemy death
      puff, underwater tint?

## HUD / menus

- [ ] Weapon tooltip when switching weapons (`draw.cpp:841`) — needs sprite art
      (plane text left trails on the scrolling stage).
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
