## Sweep 2026-10-24 (tools/audit_pc.py)

`tools/audit_pc.py` walks the PC source and data and prints, side by side, what
the PC has and what the port implements: player animation types, the weapon
table, in-game menu/HUD pieces, config fields, seventeen player abilities, and
the data-driven lists (reusing audit_data/check_terrain/check_projectiles/
check_trajectories/check_vram/audit_ai). Run it instead of waiting for a
play-test.

Closed by this sweep:

- [x] **Reserve tanks could not be used.** The menu printed the counts but
      `player_use_tank()` had no caller; the PC keeps E/W/S-TANK in an extra row
      of the weapon grid and uses one with BTN_ATTACK (`class_config.cpp:218`).
      The cursor now walks weapons + tanks (count > 0), shows `NAME [n] B:USE`
      and B uses one. `check_rules.py menu-tank-use`.
- [x] **Armor pieces were not shown in the in-game menu** (PC
      `draw.cpp:1021` draws the three installed pieces next to the portrait).
      The port now prints `ARMOR: ARMS + BODY - LEGS +` on the panel; the PC
      icon art still needs a free 16-tile VRAM block (see below).
      `check_rules.py menu-armor-line`.

Still open, with evidence:

- [x] **Armor piece ICONS** in the menu: `tools/convert_menu_armor.py` pre-
      composites the PC art (`player_armor_pieces.png`, 4 players x 8 masks of
      28x29) into 4x4 plane tiles quantised to the player's PAL2 and the menu
      draws them at the PC position (x=284, y=157) using the dialog-panel tile
      block (685..700), which is free while the menu is open.
      `check_rules.py menu-armor-icons`.
- [x] **Music volume** (PC `volume_music`): 4 steps in the AUDIO menu, applied
      with `XGM2_setFMVolume`/`XGM2_setPSGVolume`, persisted in SRAM and
      re-applied on every track start. `check_rules.py music-volume`.
      SFX volume stays on/off: XGM2 has no PCM volume (see AGENTS.md scope).
- OUT OF SCOPE (owner's decision, recorded in AGENTS.md): language selection
  (English only), separate SFX volume.
- NOT GAPS (the PC declares these animation types but never sets them):
  `ATTACK_UP`, `ATTACK_DOWN`, `THROW`, `ATTACK_THROW`, `MOVE_SEMI`,
  `VERTICAL_TURN`, `LOOK_UP`, `GRAB_WALL`, `HIT_SPECIAL`, `HOLDING_HAND`,
  `ATTACK_SPECIAL`, the `EFFECT_*` placeholders. `tools/audit_pc.py` now checks
  the PC's `set_animation_type()` calls before reporting a pose as missing.
- [x] **Players 3/4 lock is explained on screen**: the select still limits to
      two until `game_config.game_finished` (PC `sceneslib.cpp:547`), and now
      says "FINISH THE GAME TO UNLOCK PLAYERS 3 AND 4" instead of looking
      broken. `check_rules.py unlock-hint`.
- [x] **Trajectories CHAIN / BOMB_RAIN / LARGE_BEAM implemented** (the last
      known fallbacks; `check_trajectories.py` KNOWN_FALLBACK is now empty):
      CHAIN is attached to its owner (the firing enemy is published as
      `eshot_current_owner`, queried per frame through `enemies_get_box`) and
      extends/retracts; BOMB_RAIN drops 4 FALL_BOMB children 1600 ms apart,
      spaced across the screen; LARGE_BEAM grows one frame width per step up to
      the screen edge. The MD art cannot clip, so CHAIN/LARGE_BEAM show the
      full sprite while their hitbox grows.
- OUT OF SCOPE (owner's decision, recorded in AGENTS.md): the trophies/extras
      screen is not in this version.

## Audit 2026-10-26 (room changes, AI options, menu state)

Closed (each with its offline reproduction, see `git log`):
- [x] FLY option `DOWN` (3) had no case in `enemy.c` and flew horizontally; used by
      8 stages. `FALL` (7) now heads for the screen bottom like the PC
      (`artificial_inteligence.cpp:1372/1375`). `audit_ai.py` now FAILS when an
      option the data uses has no `#define`/case in `enemy.c` - the earlier
      "option gaps: 0" in `audit_data.py` only checked that a constant existed.
- [x] Room change: teleporter-type links fade (PC `game.cpp:709`), the destination
      camera keeps the player's screen X (`game.cpp:716`), the outgoing screen
      absorbs what the destination camera cannot, the ladder scan starts at the
      real arrival column of wide links, the slide draws the player with the
      same sprite offset as normal play. Models: `tools/check_link_align.py`,
      `tools/check_link_arrival.py`; rules in `check_rules.py`.
- [x] In-game menu close showed off-screen objects (and dead enemies) frozen on
      screen: it restored "everything visible" while `objects_update` caches the
      visibility it last sent. Host test `run_menu_visibility`.

Still open (PC does it, the port does not):
- [x] (2026-10-31, needs a play-test) The slide shows only the incoming TILES: the PC also draws the incoming
      map's objects, enemies and above-player layer during it
      (`game.cpp:1090` onward, `show_objects`/`show_above_objects`).
- [x] (2026-10-31, nearest-colour remap into the outgoing PAL3) Incoming parallax is replaced by sky whenever the two maps' background
      palettes differ (PAL3 is shared): 12 links incl. CHAOS CITY map 0 -> 1.
- [x] (2026-10-31, TRANS_HOLD=2) Slide speed: 8 px/frame here, 4 px per step with delays on the PC.
- [ ] Owner's TODO.txt BUGS list (charged shot consumed on hit, full-charge
      indicator, wall-shooter diagonal aim, door opening from the centre, enemy
      speed vs the PC) is NOT audited against the PC source yet.

### Verification pass 2026-10-26 (the "unverified" part of the list above)

Checked against the PC source; the older `pc_features_vs_md.md` is stale for these:
- [x] BEAM / disappearing-block / wall-break SFX already existed. Three PC sounds
      had NO counterpart and were added: shot reflected (`projectilelib.cpp:1232/
      1248`), shoryuken voice (`classplayer.cpp:222`, replaces the jump sound),
      big energy pill `recharge.wav` (`character.cpp:2721`).
- [x] Autoscroll out-of-screen damage exists (`player_screen_edge_update`).
- NOT GAPS (PC dead code, verified by grep): `show_in_memorian` (defined, never
      called), `set_screen_shake` (no callers), `block_jump` / `ground_damage_players`
      (only reachable from `ia_action_quake_attack`, which has no caller),
      `classnpc::boss_move` (generic boss intro; the attack patterns are the
      data-driven AI the port already runs).
- [x] Castle-path line animation between castle stages (2026-10-26): `castle_path.c`
      overlays the PC's path (`draw_castle_path`, `sceneslib.cpp:871-901`) on BG_A over
      the art, earlier segments at once and the current one over 1000ms, a marker on
      each reached point. `tests/host/castle_path_test.c` checks all 7 segments
      against a Python transliteration of the PC code.
- [x] After-image trail (2026-10-26): it is the PC's slide/dash effect
      (`character::show_previous_sprites`), NOT the teleport - the old table
      mislabelled it. Three flickering ghost sprites; the enemy variant
      (`FLY DASH_TO_PLAYER`) is not used by any converted stage.
- DROPPED by the owner (2026-10-26): drop-table difficulty rows - the port has one
      difficulty.
- Boss door: the port opens centre-out, the PC retracts bottom-up one tile per
      100ms (`classMap::redraw_boss_door`). Owner's decision: ours is better, keep it.
- Kept as documented deviations: slide speed (4px steps need sub-tile vertical
      scrolling of both planes), charge indicator (owner asked for a faster blink
      instead of the PC's 3-colour cycle).

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

- [x] `TERRAIN_QUICKSAND` (9) (2026-10-23): passable and not floor — the
      player sinks at `QUICKSAND_GRAVITY` (0.2 px/frame, `player.c`
      `sink_acc`) and walks at `QUICKSAND_SPEED_MULT` (0.4) of the normal
      speed, both from PC `defines.h:443/1461` and `character.cpp:237/1346`.
      The quicksand tile blocks NPCs and objects (`TERRAIN_BLOCKS`), which is
      what the PC's `check_map_collision_point` does with `BLOCK_QUICKSAND`.
      Water now gets its `WATER_SPEED_MULT` (0.7) walk slowdown on the same
      path. **No converted stage paints terrain 9**, so this is engine
      readiness for RockDroid2 (whose maps do not use it yet either);
      `tools/check_terrain.py` fails the moment a map uses an id the engine
      does not name.
- **Not a gap**: `TERRAIN_DOOR` (3) only exists for the PC's *tile3* boss-door
      redraw (`classmap.cpp:2069`); the port draws the door as an object
      (sprite) with the slide animation, so there is no terrain rule to port.
- [x] Water/ice movement (2026-10-23): water caps the walk at 0.7 and keeps
      its reduced gravity/jump; ice keeps its low-friction acceleration.

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

## RockDroid2 (sequel data) — what its maps need that the port lacks

`tools/audit_data.py --game RockDroid2` (reads the PC build tree, which this
repo does not convert yet) reports the sequel's own feature set, 20 stages of
which the port converts none. Differences from RockDroid1 worth knowing before
its stages are added:

- **Terrain**: same ids as RockDroid1, plus WATER in stages 8/9 and ICE in
  stage 8. Quicksand (9) is still unused in the shipped data; the engine now
  handles it (above), so a map may start using it.
- **AI_FLY options 13/14/19** (2026-10-23): the sequel's tables use
  `TO_PLAYER_X`, `TO_PLAYER_Y` and `DASH_TO_PLAYER` (npcs 1, 4, 15, 47, 75),
  which RockDroid1 never did. Implemented in `enemy.c`
  (`AI_FLY_TO_PLAYER_X/_Y`, `AI_FLY_DASH`, 2x speed, no dash sprite effect).
  `audit_data.py` now reports option gaps for both games (`option gaps: 0`).
- [x] **`OBJ_ACTIVE_OPENING_SLIM_PLATFORM` (27)** (2026-10-23): 5x in stage 5,
      5x in stage 11. Modelled as a one-way platform (PC `classmap.cpp:1447/
      1560`: no side or ceiling block, no collision while moving up); standing
      on it starts the PC's open cycle — visible for the object's timer, then
      gone for `DISAPPEARNING_HIDDEN_TIME` (2000 ms) and back
      (`object.cpp:1039`).
- [x] **`OBJ_DAMAGING_PLATFORM` (28)** (2026-10-23): 9x in stage 11. Carries the
      player like any platform; once stood on, the damage tick runs
      (`object.cpp:1051`): whoever is on top takes `TOUCH_DAMAGE_SMALL`, then
      one more tick resets it. Its two frames are the idle/damage states.
- [x] **`OBJ_CRUSHER` (type 33)** (2026-10-23): 7x in stage 5. Falls out of the
      ceiling accelerating 0.1 px/frame^2, rests `CRUSHER_DELAY` (800 ms) on
      the ground and climbs back at the object's speed
      (`object::move_crusher`, `object.cpp:1343`); anything it catches between
      it and the floor dies via `player_crush()` (`check_player_crushed`,
      `classmap.cpp:1644`). **Divergence**: the PC also paints the shaft above
      the crusher (`show_crusher`, frame 1 repeated), which would need a plane
      redraw per step; the port draws the body only.
- [x] **Second background layer** (2026-10-23): `backgrounds[1]` is the PC's
      *foreground* layer (`classMap::draw_foreground_layer`, `classmap.cpp:664`),
      drawn over the map with its own speed and auto-scroll mode. Stage 3 (all
      three maps) is the only user, `fg_clouds.png` at speed 4 over
      `valkyrie.png` at 18. The MD has two scrollable planes (BG_A = map,
      BG_B = bg0), so `tools/convert_stage.py` now composites the layer into
      the bg0 bake (`convert_background(..., over=...)`), which puts it behind
      the map instead of in front and scrolls it at bg0's rate. Print-out says
      so on every conversion. RockDroid1 data has no `backgrounds[1]`, so
      nothing changes for the converted stages.
- [x] **bg `auto_scroll` modes** (`E_BG_SCROLL_MODE`, `defines.h:1332`;
      2026-10-24): the backdrop now moves by itself when a mode is set —
      LEFT/RIGHT on the plane's horizontal scroll, UP/DOWN on its vertical
      scroll with the 256 px wrap, DIAGONAL on both (the PC build only
      implements LEFT/RIGHT/UP/DOWN, so DIAGONAL is the port's own choice,
      documented in stage.c). The phase comes from a frame counter reset on map
      load, so it also holds in the middle of a room transition
      (`transition.c bg_offset`). The converter keeps `backgrounds[1]`'s mode
      when bg0 is static (the second layer is baked into bg0 on MD):
      RockDroid1 stage 6 and several RockDroid2 maps rely on it.
      `tools/check_bg_scroll.py` lists every mode in both games' data.
- **Enemies**: npc ids up to 75 (RockDroid1 stops at 60), i.e. new art and
  `game_ai_list` rows to convert; every AI *action* they use is already
  implemented.

### Scene ops (item 5 of the 2026-10-24 queue)

`tools/check_scenes.py` walks every scene: the opening (0) and the four endings
(2..5) use only implemented ops; the LAB opening (1, not played by the port) is
the sole user of `SHOW_ANIMATION`, and `MOVE_VIEWPOINT`/`SUBSCENE` are used by
no scene at all. `tools/convert_scenes.py` now REFUSES to emit a played scene
with an unimplemented op, and `scene.c` logs one if it somehow reaches it, so
this cannot regress silently. `SHOW_ANIMATION` would need the
`city_bg_animation.png` (320x150) strip converted to plane animation, only if
the LAB opening is ever wired.

### Accepted deviations / data-side items (item 5)

- **Crusher shaft**: the PC paints the shaft it leaves behind (`show_crusher`,
  frame 1 repeated down to the start point). The port draws the body only — the
  shaft would need the crusher art as plane tiles plus a plane redraw on every
  step. Recorded as accepted.
- **Enemy anim rows `VERTICAL_TURN` / `WALK_AIR` / `HIT`**: the converted
  enemies' art carries none of them (checked when TURN was added), so there is
  nothing to show; `audit_data.py` fails if a future export has art for a row
  the port does not drive.
- **Stages 14..19**: placeholders in the data (`Stage [16]`, empty). Not engine
  work; they need a data conversion like the first fourteen got.

## Performance (2026-10-24)

- [x] "Slowdown when several objects are on screen": reproduced offline with
      `tools/run_host_tests.sh` (real `src/object.c` + counting stubs). The type
      predicates answered 3-4x per object per frame (7/21/36/50 answers for
      2/6/10/14 visible objects, ≈347 subticks of `objects_update` at 14, the
      emulator measured 350); they are now answered once per object at spawn
      (`MapObj.flags`), the looping animation steps without a per-object
      division, and `stage_terrain_at` reads through a cached map base. Model:
      ≈171 subticks at 14 objects.

## Process

The lesson from APE: **"action supported" is not "option supported"**, and a
support table that only lists the enum hides it. `audit_data.py` now prints
both, so the next gap of this kind shows up offline. When a play-test finds
something, also ask "which PC enum/option/field did we not read?" and add it to
the audit, not just to the code.
