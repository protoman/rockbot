# PC features vs Mega Drive port

Feature inventory of the PC game (`/home/iuri/Desenvolvimento/rockbot`), taken
from `main()` down through the per-frame loop, and the state of each item in the
port (`mega_drive/src`). Written 2026-10-21 by reading both trees; no emulator
run was needed (builder/tool prints were enough).

Status legend:

| tag | meaning |
|---|---|
| **DONE** | implemented and reachable in the ROM |
| **PARTIAL** | implemented but simplified or with a known gap |
| **MISSING** | not implemented |
| **N/A** | PC-platform detail that cannot/should not exist on the MD, or data the shipped game does not use |

Line references are the ones valid at the time of writing; verify with
`grep -n` if the files have moved on.

---

## 1. Boot workflow

### PC `main()` (`main.cpp:318`)

```
parse CLI debug flags (--quickload, --invencible, --allweapons,
                     --infinitejump, --player1..3)
fio.read_game()                -> stages, maps, enemies, objects, AI, weapons,
                                  players, dialogs, scenes, music (binary .dat)
gameControl.get_drop_item_ids()
soundManager.init_audio_system()
graphicsLib.preload(); draw_lib.preload()
gameControl.currentStage = INTRO_STAGE
gameControl.show_game_intro()   -> logos, scenes, title/menu, save, player,
                                   difficulty, stage pick, boss intro, initGame
while (run_game) main_loop()    -> show_game(true, true) + update_screen
```

### MD `main(hardReset)` (`src/main.c:713`)

```
VDP 320x224, JOY, SPR, DMA queue, scroll/window state, planes
save_init(); config_init(); stage_set_data(0)
showIntro()                          (logos, src/main.c:85)
music_play(INTRO_SCENE); scene_play(OPENING_UNBEATEN)
loop:
  showTitle()  NEW / LOAD / CONFIG / ABOUT      (src/main.c:191)
    NEW  -> save_selector(TRUE) -> save_slot_clear -> player_select_run
            -> player_new_game() -> playSession(0)
    LOAD -> save_selector(FALSE) -> load_progress -> (stage 0 or stage select)
  config_run / showAbout
playSession(stage) -> runStage(stage) (src/main.c:330) -> stage_select_run
```

Design difference (intentional): the PC loads `.dat` files at runtime; the port
converts them at build time with `tools/convert_*.py` into C arrays. There is no
runtime file layer.

---

## 2. Per-frame loop

### PC `game::show_game()` (`game.cpp:133`)

```
leave_game -> exit
input.read_input()
config_manager.execute_ingame_menu()          (pause menu)
leave_stage_request -> leave_stage()
fps_manager.must_skip_frame()
test_teleport(player1)
timer not paused:
  update_stage_scrolling()
  loaded_stage.move_objects()
show_stage()                                  (map + animated tiles + bg + fg)
player1.execute()
loaded_stage.move_npcs()                      (enemies + AI)
player1.is_dead() -> restart_stage()
player1.show()
loaded_stage.show_above_objects()
loaded_stage.showAbove()                      (foreground layer)
map gfx overlay (SCREEN_GFX_MODE_OVERLAY) -> draw_lib.show_gfx()
draw_lib.show_hud(hp, player, weapon, ammo)
draw_lib.show_weapon_tooltip()
fps_manager.limit(); graphLib.flush_redraw()
```

### MD `runStage()` (`src/main.c:330`)

```
START edge -> menu_open(mask) -> RUN_LEFT / resume
objects_update()          (platforms first: they carry the player)
player_update(pad); player_check_scroll_lock(pad)
hud_update(); hud_weapon()
stage_check_link() -> room transition (main.c:470+)
stage_update_camera(player.x + 8, player.y + 20)
enemies_update(); eshot_update(); shot_update(); explosion_update()
objects_draw/objects_update tail, enemy draws, player draw
boss death / stage clear sequences
SPR_update(); SYS_doVBlankProcess()
```

Equivalent order and content. The main gaps inside the loop are listed below
(overlay GFX, weapon tooltip, PC AI depth).

---

## 3. Front-end / menus / scenes

| feature | PC | port | status |
|---|---|---|---|
| publisher/dev logos | `show_notice` (`game.cpp:506`) | `showIntro` (`main.c:85`) | **DONE** |
| unbeaten intro scene (text/images/parallax) | `scenesLib::game_scenes_show_unbeaten_intro` | `src/scene.c` + `scenes_data.c` | **DONE** |
| title screen (logo, zoom-in) | `scenesLib::main_screen` + `zoom_image` | `title64.py` art, static | **DONE** (no zoom; MD shows the composed art) |
| main menu (new/load/config/about) | `main_screen` | `showTitle` | **DONE** |
| cheats menu | `scenesLib::show_cheats_menu` | — | **N/A** (deliberately not ported, user decision) |
| save slot select + details | `select_save` | `src/saveselect.c` | **DONE** |
| difficulty select (easy/normal/hard) | `select_difficulty` | — | **N/A** (deliberately not ported; normal values are fixed: 5 lives, normal drop table) |
| player select (4 characters) | `select_player` | `src/playerselect.c` | **DONE** |
| stage select (map, boss faces, castle slot) | `pick_stage` | `src/stageselect.c` | **DONE** |
| boss intro screen | `boss_intro` | `src/bossintro.c` | **DONE** |
| castle #1 intro | `classic_style_castle_intro` | `bossintro.c` (castle art) | **DONE** |
| boss dialog pages | `dialogs.cpp` | `src/dialogs.c` | **DONE** (panel art still BUG-008) |
| ending scenes + parades + credits | `show_ending_scenes`, `show_*_ending` | `src/ending.c` | **PARTIAL** (enemy parade approximated by bosses; fixed layer offsets) |
| about screen | `draw_lib.show_about` | `showAbout` | **DONE** |
| CLI debug flags | `main.cpp:321` | — | **N/A** (ROM) |

---

## 4. Player

| feature | PC reference | port | status |
|---|---|---|---|
| walk / run with accel + friction | `character::execute` | `player_update` | **DONE** |
| jump (variable height), double jump | `character` | `player.c` | **DONE** |
| slide (dash on ground) | `character.cpp:1672` | `player.c` slide | **DONE** |
| air dash | ability flags | `player_can_air_dash` | **DONE** |
| stairs / ladder (climb, attack, semi) | `ANIM_TYPE_STAIRS*` | `ANIM_STAIRS*` | **DONE** |
| attack, walk-attack, jump-attack | anim types | `ANIM_*` | **DONE** |
| diagonal up/down shots | `ANIM_TYPE_ATTACK_DIAGONAL_*` | `player.c` | **DONE** (per-player ability) |
| charge shot (normal/semi/full) | `check_must_attack` | `charge_frames` | **DONE** |
| 8 boss weapons + COIL/JET items | `WEAPON_ICONS_ENUM` | `weapon_data.c` (12 slots) | **DONE** |
| ammo gauge, refill on stage start | `refill_weapons` | `player_ammo`, `player_refill_weapons` | **DONE** |
| weapon palette swap | `weapon_colors` | `player_weapon_color` | **DONE** |
| shield (ROCKBOT) + shot reflection | `ANIM_TYPE_SHIELD` | `shielding` + `eshot.c` | **DONE** |
| armor pieces + per-piece abilities | `game_save.armor_pieces` | `player.armor`, `player_armor_ability` | **DONE** |
| E/W/S tanks + menu use | `class_config` | `tanks_hp/wp/sp` | **DONE** |
| lives / game over / continue | `reset_lifes` | `player_lives`, `showGameOver` | **DONE** |
| invulnerability + blink | `PLAYER_INVULN` | `invuln` | **DONE** |
| hit pushback animation | `character.cpp:194` | `hit_push` | **DONE** |
| death animation + checkpoint respawn | `restart_stage`, `reset_scroll` | `player_death_update`, `player_set_checkpoint` | **DONE** |
| water / ice terrain | `classmap` terrain | `player.c` | **DONE** |
| moving-platform carry | `obj_move_player` | `objects_update` | **DONE** |
| SHORYUKEN special | `ANIM_TYPE_SPECIAL_ATTACK` | `ANIM_SPECIAL` | **DONE** |
| 1UP pickup, item pickups | `OBJ_LIFE`, pills | `object.c` | **DONE** |

No player-side feature was found missing; what remains is tuning (see
`bugs.md`).

---

## 5. Enemies and AI

| feature | PC | port | status |
|---|---|---|---|
| per-room spawn + respawn delay | `classmap` | `enemies_reset`, `respawn_delay` | **DONE** |
| off-screen freeze | `classnpc` | `enemies_update` | **DONE** |
| contact damage | `TOUCH_DAMAGE` | `TOUCH_DAMAGE_SMALL` | **DONE** |
| enemy HP + shields (`SHIELD_*`) | `character::is_shielded` | `enemy_is_shielded` | **DONE** |
| scripted AI actions | `artificial_inteligence.cpp` | `enemy.c` (`AI_*`) | **PARTIAL** — walk/fly/jump/wait/shoot/air-walk/fall/teleport/dash/grab-wall/spawn/change-move/jump-attack-up only |
| AI state graph (links, reactions, 20 states) | `AI_MAX_STATES`, reactions | — | **MISSING** |
| enemy projectiles + trajectories | `projectilelib.cpp` | `src/eshot.c` | **PARTIAL** (linear, arc, sin, bomb, freeze, centered, zigzag, fall-bomb, laser, quake; others fall back) |
| freeze weapon | `FREEZE` | `enemies_freeze` | **DONE** |
| sub-boss flag + door gating + drop | `is_sub_boss` | `enemies_subboss_alive_left` | **DONE** |
| enemy death drops (difficulty table) | `drop_item` | `objects_drop_item` | **DONE** (normal row) |
| boss drop-in, HP row, hurt window, dialog, death explosion, weapon drop | `classmap` boss code | `enemy.c`, `main.c` | **DONE** (presentation) |
| boss attack patterns / phases | boss AI + `ANIM_TYPE_INTRO` / special | generic AI tables | **PARTIAL** |
| final boss / second castle | `use_second_castle` | — | **N/A** (`use_second_castle = 0` in the shipped data) |
| teleport/jump animation rows for bosses | `ANIM_TYPE_TELEPORT`, `JUMP` | converter emits only 3 rows | **MISSING** (BUG-011, BUG-010) |

---

## 6. Objects

PC object types are `defines.h:363` (`OBJ_*`). The port handles every type that
appears in the converted stages (types 1-19, 21-26, 29-32); the types the
shipped stages never place (0, 20, 27, 28, 33-45) are not implemented.

| PC type | name | port |
|---|---|---|
| 0 | moving platform up/down | not used by converted stages |
| 1, 2 | moving platform L/R, disappearing block | **DONE** |
| 3-9 | tanks, pills, 1UP | **DONE** |
| 10, 11 | fall platform, fly platform | **DONE** |
| 12, 13 | JET EAGLE, COIL FROG | **DONE** |
| 14 | active disappearing block | **DONE** |
| 15-17 | armor arms/body/legs | **DONE** |
| 18, 19, 24, 25 | rays / lava beams | **DONE** (90/90 toggle) |
| 20 | track platform | not used |
| 21 | boss teleporter (final) | **DONE** |
| 22 | destructible wall | **DONE** |
| 23 | special tank | **DONE** |
| 26 | final boss teleporter capsule | **DONE** |
| 27, 28 | slim opening platform, damaging platform | not used |
| 29 | checkpoint | **DONE** |
| 30 | boss door (open, one-screen scroll, close = one-way) | **DONE** |
| 31 | teleport base | **DONE** |
| 32 | stage boss teleporter | **DONE** |
| 33-45 | crusher, loop/down platforms, destructible variants, diagonal/ahead/up platforms, jump-shoot block, timed bomb, expanding spike, walker | not used by converted stages |

---

## 7. Stage / room

| feature | PC | port | status |
|---|---|---|---|
| 3 maps per stage, links with trigger bands | `file_link`, `classmap` | `stage_load`, `stage_check_link` | **DONE** |
| terrain: solid/stair/door/spike/water/ice | `TERRAIN_TYPES` | `stage_terrain_at` + `player.c` | **DONE** |
| move-left / move-right conveyor tiles | `TERRAIN_MOVE_*` | `stage.c` | **DONE** |
| scroll-lock columns, easy/hard blocks | `process_special_map_points` | `scroll_lock_camera`, `column_locked` | **DONE** |
| autoscroll | `autoscroll[]` | `stage_autoscroll` | **DONE** |
| parallax background (speed per map) | `classmap` bg layers | `stage.c` BG_B | **DONE** (1 layer; PC uses up to 2) |
| animated tiles + colourcycle | `draw_animated_tiles` | converter + `stage.c` | **DONE** (stages 1, 2, 3, 6, 8 cycle CRAM) |
| foreground layer (tile3 over the player) | `showAbove` | collapsed into the anim mask | **N/A** (deliberately not ported; static tile3 art is merged, only animated tile3 keeps its "above" bit) |
| boss room locked until boss/sub-boss dies | `subboss_alive_on_left` | `enemies_subboss_alive_left` | **DONE** |
| map gfx overlay (rain/lightning) | `set_gfx` / `show_gfx` | — | **N/A** (deliberately not ported) |
| castle stages 9-13 | `classic_style_castle.png` | **DONE** |
| stages 14-19 | placeholders in the shipped data | **N/A** |

---

## 8. HUD and UI

| feature | PC | port | status |
|---|---|---|---|
| HP balls (5, modern style) | `draw::show_hud` | `hud_update` | **DONE** |
| boss HP row | `draw.cpp:1237` | `hud_boss` | **DONE** |
| weapon gauge row | `draw.cpp:1217` | `hud_weapon` | **DONE** |
| weapon tooltip on switch | `show_weapon_tooltip` | — | **MISSING** |
| in-game menu (weapons grid, tanks, config, leave) | `class_config` | `src/menu.c` | **DONE** |
| aim/fps counter | `fps_count` | — | **N/A** |
| screen shake (QUAKE) | `graphLib.set_screen_shake` | `shot_shake_x` | **DONE** |
| fade in/out | `draw::fade_*` | `main.c`, `transition.c` | **DONE** |
| teleport effect | `show_teleport_small` | player teleport anim | **PARTIAL** (sprite anim, no colour flash) |
| "GOT WEAPON"/"MISSION STARTED" messages | `draw_lib` | `main.c` | **DONE** (BUG-013 open) |

---

## 9. Audio

| feature | PC | port | status |
|---|---|---|---|
| music playback | `Mix_PlayMusic` / MOD | XGM2 (`src/music.c`) | **DONE** |
| SFX | `Mix_PlayChannel` + wav | XGM2 PCM (`src/sfx.c`) | **DONE** |
| per-stage music + boss/clear/game-over/ending tracks | `load_music` | `music_tracks[]` | **DONE** |
| volume control / mute | config | on/off only | **PARTIAL** (deliberate simplification) |
| sound test | config | EXTRAS music/sfx test | **DONE** |
| music/SFX fade with transitions | `soundManager` | — | **PARTIAL** |

---

## 10. Saves / config

| feature | PC | port | status |
|---|---|---|---|
| 5 save slots | `select_save` | `src/save.c` (SRAM) | **DONE** |
| stage/boss progress bits | `game_save.stages[]` | `save_slot_stages()` | **DONE** |
| player, lives, weapons ammo, tanks, armor | `game_save.items` | `save_slot_*` | **DONE** |
| save erasing / copying | `select_save` dialogs | erase only (NEW GAME) | **N/A** (copy deliberately not ported; erase happens via NEW GAME) |
| config persistence | `game_config` | SRAM bytes 48..55 | **DONE** |
| input remap | `inputlib` key map | 3 buttons (jump/fire/dash) | **PARTIAL** (no keyboard/analog) |
| turbo / auto-charge | config | EXTRAS | **DONE** |
| language, platform, performance, quit | config menu | — | **N/A** |

---

## 11. Out of scope (decided with the user, 2026-10-21)

These PC features will **not** be ported, so their rows stay **N/A**:

- difficulty select (easy/normal/hard) — the port always plays normal values
- cheats menu
- static tile3 foreground art (only the animated tile3 mask is kept)
- map gfx overlays (rain / lightning)
- save copy, language, platform, performance and quit options

## 12. Open gaps, in the order they matter (the plan)

1. **Boss fight depth** — bosses have HP, hurt windows, drop-in and the weapon
   drop, but their attacks come from the generic scripted AI. Largest
   behavioural gap; plan entry in `HANDOUT.md` §6.
2. **Enemy AI graph** — `artificial_inteligence.cpp` supports linked states and
   reactions; the port plays a flat action list. Shows up as "dumb" enemies;
   plan entry in `HANDOUT.md` §6.
3. **Enemy projectiles** — several trajectories are approximated by LINEAR; the
   converter prints the real values, so this is a matter of adding cases.
4. **Boss teleport/jump animation rows** (BUG-011, blocks BUG-010).
5. **Weapon tooltip** on weapon switch.
6. **Teleport colour flash** (the sprite animation is in).
7. Known bugs and their traces live in `bugs.md` (BUG-006, 008, 010, 012, 013,
   014, 018 open).

## 13. How this table was produced

- PC: `main.cpp:318` → `game.cpp:133` → `classmap.cpp`, `classplayer.cpp`,
  `artificial_inteligence.cpp`, `object.cpp`, `defines.h`, `scenes/game_menu.cpp`,
  `file/v4/file_game_v4.h`.
- Port: `src/main.c` → `runStage` → `stage.c`, `player.c`, `enemy.c`,
  `object.c`, `hud.c`, `menu.c`, `music.c`, `save.c`, `config.c`, plus the
  generated `stageN_*.c` tables.
- Object coverage was cross-checked against the types actually present in
  `src/stage*_objects.c`, so "not used by converted stages" is data, not a guess.
