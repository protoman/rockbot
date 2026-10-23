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
- [ ] **Aimed attack poses**: `ANIM_TYPE_ATTACK_DIAGONAL_UP`/`_DOWN`
      (frames 23/24) when firing a weapon whose trajectory is
      `TARGET_DIRECTION` / `TARGET_EXACT` / `ARC_TO_TARGET`
      (`classplayer.cpp:449-461`). The player's boss weapons 14/15/18 use
      `TARGET_DIRECTION`, so on the PC the shot *aims at the nearest enemy* and
      the pose turns; the port fires it straight.
- [ ] **`TRAJECTORY_CHAIN` (3)** — the "Chain" weapon (projectile 7). The port
      has no chain physics, so weapon 7 behaves like a straight shot.
- [ ] Pose-only rows never played: `ANIM_TYPE_TURN`, `ANIM_TYPE_MOVE_SEMI`,
      `ANIM_TYPE_STAIRS_SEMI`, `ANIM_TYPE_STAIRS_MOVE`, `ANIM_TYPE_LOOK_UP`,
      `ANIM_TYPE_THROW`, `ANIM_TYPE_HIT_SPECIAL`, `ANIM_TYPE_GRAB_WALL`. The
      port reuses STAND/STAIRS for those moments, so they look static.

## Enemy AI (src/enemy.c) — from `audit_data.py`

Implemented: WALK, FLY, JUMP (plain), WAIT_RANGE, SHOT_AHEAD/DIR/INVERT,
AIR_WALK, FALL, TELEPORT, DASH, GRAB_WALL, SPAWN_NPC, CHANGE_MOVE, REPLACE_NPC,
JUMP_ATTACK_UP.

- [ ] **`AI_JUMP` ignores its option** — the biggest functional gap:
      `AHEAD` (npcs 9,17,54,55), `TO_PLAYER` (2,6,7,8,9,10,17,41),
      `TO_PLAYER_DIR` (8) and **`TO_ROOF` (2 = APE BOT, 48)**. PC
      `ia_action_jump_to_roof()` (`artificial_inteligence.cpp:793`) rises
      straight to the ceiling with gravity off and keeps `_ignore_gravity`, so
      the NPC then walks along the roof — APE's missing movement.
- [ ] `AI_FLY` options not implemented: `TO_RANDOM` (npcs 2,12),
      `DRILL_DOWN` (1,48), `PLAYER_DIR` (12), `OPPOSITE_WALL` (44),
      `OPPOSITE_SHOOT` (18), `RANDOM_X` (44), `RANDOM_Y` (12,20).
- [ ] AI actions with no port code: `SAVE_POINT`, `CIRCLE_PLAYER`,
      `CHANGE_MOVE_TYPE_REVERSE`, `JUMP_ATTACK_AHEAD_ONCE`, `WAIT_RANDOM_TIME`,
      `MORPH_INTO_NPC`, `PLAY_SFX`, `WALL_WALK`, `WALL_WALK_SHOOT`,
      `SHOT_MULTIPLE_PROJECTILE`, `EXPLODE_ITSELF`, `THROW_ITEM`.
      (Not used by the AI data of stages 0..13 today, so nothing is visibly
      wrong yet; they appear in the editor's palette.)
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

- [ ] `TERRAIN_QUICKSAND` (9): no sink/slow behaviour (not used by the
      converted stages).
- [ ] `TERRAIN_DOOR` (3): passable, no door logic.
- [ ] Water/ice are minimal (sink + slow fall / low friction) — PC has extra
      movement rules (`character.cpp`).

## Scenes (src/scene.c)

- [ ] `PLAY_MUSIC`, `PLAY_SFX`, `STOP_MUSIC` ops are accepted and **ignored**
      ("audio is not wired yet"), so a cutscene that changes music is silent.
      The port has XGM2 music + PCM SFX now, so these can be wired to
      `music_play`/`sfx_play`/`music_stop`.
- [ ] `MOVE_VIEWPOINT`, `SHOW_ANIMATION` (animated scene art), `SUBSCENE`.

## Effects (PC draw.cpp / graphicslib.cpp)

- [ ] Dash dust (`dash_dust.png`), water splash (`water_splash.png`), bubble
      (`bubble.png`), teleport-small colour flash, boss ground shake, dark mode,
      hit flash on the sprite palette (the port blinks the sprite instead).
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
