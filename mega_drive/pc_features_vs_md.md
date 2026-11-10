# PC features vs Mega Drive port

Feature inventory of the PC game (`/home/iuri/Desenvolvimento/rockbot`), taken
from `main()` down through the per-frame loop, against the current state of the
port (`mega_drive/src`). Rewritten 2026-10-22 after a fresh sweep of both trees
(the first version was 2026-10-21).

Status legend:

| tag | meaning |
|---|---|
| **DONE** | implemented and reachable in the ROM |
| **PARTIAL** | implemented but simplified or with a known gap |
| **MISSING** | not implemented |
| **N/A** | deliberately not ported, or data the shipped game does not use |

Line references are valid at the time of writing; verify with `grep -n` if the
files have moved on.

---

## 1. Boot workflow

### PC `main()` (`main.cpp:278`)

```
CLI flags (--quickload --invencible --allweapons --infinitejump --playerN
           --gamename --stage), language detection, save path per OS
fio.read_game() -> stages, maps, enemies, objects, AI, weapons, players,
                   dialogs, scenes, music (binary .dat)
soundManager.init_audio_system(); graphicsLib.preload(); draw_lib.preload()
gameControl.currentStage = INTRO_STAGE
gameControl.show_game_intro()  -> notice logos, scenes, main_screen (title),
                                  select_save, select_difficulty, select_player,
                                  pick_stage, initGame, boss_intro, start_stage
while (run_game) main_loop()   -> show_game(true,true) + update_screen
```

### MD `main(hardReset)` (`src/main.c:778`)

```
VDP 320x224, JOY, SPR_initEx(504), DMA queue, scroll/window state
save_init(); config_init(); stage_set_data(0)
showIntro() (logos) -> scene_play(OPENING_UNBEATEN)
loop: showTitle() NEW / LOAD / CONFIG / ABOUT
        NEW  -> save_selector -> player_select_run -> player_new_game -> stage 0
        LOAD -> save_selector -> load_progress -> stage 0 or stage select
playSession(stage) -> runStage(stage) -> stage_select_run
```

Design difference (intentional): the PC loads `.dat` files at runtime; the port
converts them at build time (`tools/convert_*.py`) into C arrays. There is no
runtime file layer. The editable master copy lives in `data/games/Rockbot1`
(see `AGENTS.md`).

---

## 2. Per-frame loop

### PC `game::show_game()` (`game.cpp:126`)

```
exit -> input -> ingame menu -> leave stage -> test_teleport ->
update_stage_scrolling (autoscroll out-of-screen damage) ->
loaded_stage.move_objects -> show_stage -> player.execute -> move_npcs ->
death -> player.show -> show_above_objects -> showAbove (foreground) ->
map gfx overlay -> HUD -> weapon tooltip -> fps limit -> flush
```

### MD `runStage()` (`src/main.c:299`)

```
START -> menu_open -> RUN_LEFT / resume
objects_update -> player_update -> player_check_scroll_lock -> HUD ->
stage_check_link (transition + arrival) -> stage_update_camera ->
anim/colourcycle -> enemies_update -> shoryuken hit -> boss dialog ->
boss music -> explosion/eshot/shot -> boss HUD -> SPR_update/vblank
```

Order and content match. Gaps inside the loop: overlay GFX, autoscroll
damage, PC AI depth (below).

---

## 3. Front-end / menus / scenes

| feature | PC | port | status |
|---|---|---|---|
| publisher/dev logos | `show_notice` `game.cpp:501` | `showIntro` `main.c:85` | **DONE** |
| "in memoriam" tribute | `show_in_memorian` `game.cpp:583` | — | **MISSING** |
| unbeaten intro scene | `game_scenes_show_unbeaten_intro` `sceneslib.cpp:700` | `src/scene.c` | **DONE** |
| title screen (logo zoom) | `draw_main` `sceneslib.cpp:102` | `title64.py` art, static | **DONE** (no zoom) |
| main menu new/load/config/about | `main_screen` `sceneslib.cpp:128` | `showTitle` `main.c:127` | **DONE** |
| cheats menu | `show_cheats_menu` `sceneslib.cpp:228` | — | **N/A** |
| save slot select | `select_save` `sceneslib.cpp:755` | `saveselect.c:104` | **DONE** |
| difficulty select | `select_difficulty` `sceneslib.cpp:708` | — | **N/A** |
| player select (4) | `select_player` `sceneslib.cpp:546` | `playerselect.c:76` | **DONE** |
| stage select (grid, faces, castle) | `stage_select::pick_stage` `stage_select.cpp:207` | `stageselect.c:118` | **DONE** |
| boss intro screen | `boss_intro` `sceneslib.cpp:634` | `bossintro.c:56` | **DONE** |
| castle #1 intro | `classic_style_castle_intro` `sceneslib.cpp:863` | `bossintro.c:30` | **DONE** |
| boss dialog (panel + face) | `dialogs::show_dialog` `scenes/dialogs.cpp:39` | `dialogs.c:118` | **DONE** (panel added 2026-10-22) |
| ending scenes + parades + credits | `show_ending_scenes` `sceneslib.cpp:848`, `show_*_ending` | `ending.c:246` | **PARTIAL** (enemy parade approximated by bosses) |
| about screen | `draw::show_about` `draw.cpp:609` | `main.c:226` | **DONE** |
| scene player ops | `scenes/sceneshow.cpp:41` | `scene.c:277` | **PARTIAL** — MOVE_IMAGE, PARALLAX, TEXT, CLEAR* done; MOVE_VIEWPOINT, SHOW_ANIMATION, SUBSCENE, PLAY_MUSIC/SFX, STOP_MUSIC missing |
| game picker (multi-game) | `select_game_screen` `game.cpp:2047` | — | **N/A** (single game) |
| CLI debug flags | `main.cpp:325` | — | **N/A** |

---

## 4. Player

| feature | PC | port | status |
|---|---|---|---|
| walk/run accel + ice friction | `character::charMove` `character.cpp:124` | `player.c:970` | **DONE** |
| jump (variable, buffered, coyote) | `jump` `character.cpp:1760` | `player.c:987` | **DONE** |
| double jump | `can_double_jump` `classplayer.cpp:1189` | `player.c:1007` | **DONE** |
| ground slide / dash | `slide` `character.cpp:1585` | `player.c:936` | **DONE** |
| air dash (LEGS) | `can_air_dash` `classplayer.cpp:1201` | `player.c:942` | **DONE** |
| stairs/ladder climb + attack | `is_on_stairs` `character.cpp:2428` | `player.c:722` | **DONE** |
| shield (ROCKBOT) + reflect | `is_shielded` `character.cpp:3357` | `player.c:891`, `eshot.c:276` | **DONE** |
| charge shot (semi/full) | `check_must_attack` `character.cpp:551` | `player.c:637` | **DONE** |
| 8 boss weapons + COIL/JET | `attack` `classplayer.cpp:290` | `shot.c`, `weapon_data.c` | **DONE** |
| weapon ammo + refill | `refill_weapons` `classplayer.cpp:1078` | `player.c:305` | **DONE** |
| weapon palette swap | `change_player_color` `classplayer.cpp:997` | `player.c:294` | **DONE** |
| weapon tooltip on switch | `show_weapon_tooltip` `draw.cpp:841` | `tooltip.c:122` | **DONE** (box + icon + name; tank counts missing) |
| armor pieces + abilities | `update_armor_properties` `classplayer.cpp:1251` | `player.c:192` | **DONE** |
| tanks (E/W/S) + menu use | `get_item` `classplayer.cpp:109` | `player.c:1355` | **DONE** |
| lives / game over / continue | `death` `classplayer.cpp:942` | `player.c:1250` | **DONE** |
| hit pushback (+armor variants) | `damage` `character.cpp:2990` | `player.c:873` | **DONE** |
| invulnerability + blink | `character.cpp:2990` | `player.c:681` | **DONE** |
| water physics + splash SFX/anim | `character.cpp:2278` | `player.c:1002` | **PARTIAL** (physics + SFX; no splash animation) |
| ice inertia object | `character.cpp:329` | `player.c:958` | **PARTIAL** (friction only) |
| quicksand | `is_on_quicksand` `character.cpp:2668` | — | **MISSING** (no quicksand tiles in stages 0..13) |
| teleport animation + after-images | `character.cpp:899` | `main.c:342` | **PARTIAL** (no after-image trail) |
| shoryuken (LEGS) | `shouryuken` `classplayer.cpp:216` | `player.c:906` | **DONE** |
| GOT_WEAPON / victory pose | `classplayer.cpp:625` | `main.c:711` | **DONE** (2026-10-22) |
| ground-quake `block_jump` | `block_jump` `classplayer.cpp:281` | — | **MISSING** |
| heart pieces (max HP) | `get_max_hp` `classplayer.cpp:276` | — | **N/A** (not in the shipped data) |
| XP / level / shop upgrades | `st_items` `file_save_v4.h:13` | — | **N/A** |

---

## 5. Enemies and AI

| feature | PC | port | status |
|---|---|---|---|
| per-room spawn + respawn delay | `load_map_npcs` `classmap.cpp:528` | `enemy.c:854` | **DONE** |
| off-screen AI freeze | `classnpc::move` | `enemy.c:940` | **DONE** |
| contact damage | `collision_player_npcs` `classmap.cpp:1886` | `enemy.c:998` | **DONE** |
| HP + shields (`SHIELD_*`) | `is_shielded` `character.cpp:3357` | `enemy.c:1036` | **DONE** |
| weakness multipliers | `classplayer.cpp:747` | `enemy.c:1093` | **DONE** |
| freeze weapon | `is_weak_to_freeze` | `enemy.c:66` | **DONE** |
| sub-boss flag, door gate, drops | `subboss_alive_on_left` `classmap.cpp:1250` | `enemy.c:1187` | **DONE** |
| drop table (difficulty ratios) | `drop_item` `classmap.cpp:996` | `object.c:224` | **PARTIAL** (normal difficulty row only) |
| **AI state graph + reactions** | `execute_ai` `artificial_inteligence.cpp:67`, `check_ai_reaction:117` | `enemy.c:745`, `enemy.c:412` | **DONE** (2026-10-21) |
| **AI actions** | 28 actions + options (`defines.h:702`) | 16 (`enemy.c:24`) | **PARTIAL** |
| — jump family (to player/point/random, ahead, long ahead, once, up, teleport in/out, to roof) | `artificial_inteligence.cpp:428-793` | AI_JUMP only | **MISSING** |
| — quake attack, accelerate up/down | `:866-904` | — | **MISSING** |
| — circle player, wall walk (+shoot), play sfx, wait random, morph, change animation | `:2372`, `:1832`, `:2630`, `:1823`, `:2283`, `:1793` | — | **MISSING** |
| — multi-projectile spreads (5/3/8) | `execute_shot_multiple_projectile` `:1193` | — | **MISSING** |
| — fly options (drill, zigzag, sin, dash, opposite-wall) | `execute_ai_step_fly` `:1337` | some (zigzag) | **PARTIAL** |
| — explode itself, throw item | `:1252`, `:1261` | — | **MISSING** |
| **water-enemy constraint (fish stays in water)** | `character.cpp:1921`, AI `:2053` | — | **MISSING** (BUG-018) |
| ghost NPCs / player friends | `npc_is_ghost`, `is_player_friend` `classnpc.cpp:279` | — | **MISSING** |
| child NPC spawn cap | `child_npc_count` `classmap.cpp:2168` | partial (AI_SPAWN_NPC) | **PARTIAL** |
| enemy projectiles + trajectories | `projectilelib.cpp` | `eshot.c` | **PARTIAL** — linear/arc/sin/bomb/freeze/centered/zigzag/fall-bomb/progressive/follow/quake; others fall back |
| boss drop-in, HP row, hurt window, dialog, death, weapon drop | `classmap.cpp:2187` | `enemy.c:908`, `main.c:402` | **DONE** |
| **boss intro sprites (fall + intro frames)** | `show_boss_intro_sprites` `draw.cpp:280` | — | **MISSING** |
| boss ground shake on landing | `boss_hit_ground` `classmap.cpp:2428` | — | **MISSING** |
| per-boss attack patterns | `boss_move` `classnpc.cpp:404` | generic AI | **PARTIAL** |
| sub-boss revive after boss death | `classmap.cpp:2201` | — | **MISSING** |
| final boss teleporter activation (8/8) | `finish_player_teleporter` `game.cpp:2088` | — | **N/A** (second castle unused) |

---

## 6. Objects

The port handles every type the converted stages place (1-19, 21-26, 29-32).
Types the shipped stages never place (0, 20, 27, 28, 33-45) are not
implemented, so crusher, track platform, timed bomb, expanding spike, damaging
platform and the platform walker are **MISSING** but unreachable in the
converted data.

| feature | PC | port | status |
|---|---|---|---|
| platform state machine (stand/move/return) | `object.cpp:884` | `object.c:381` | **DONE** |
| disappearing block (1500/2000ms) | `object.cpp:1021` | `object.c:769` | **PARTIAL** — timings match; the PC also plays the 5-frame *appear* animation + SFX (`object.cpp:607`) |
| active disappearing block | `object.cpp:1039` | `object.c:779` | **DONE** |
| rays / lava beams (90/90 + BEAM sfx) | `object.cpp:1069` | `object.c:753` | **PARTIAL** (no BEAM sfx) |
| boss door (open → pan → close) | `object.cpp:1186` | `object.c:866` | **DONE** |
| checkpoint | `object.cpp:1515` | `object.c:911` | **DONE** (flag loops, 2026-10-22) |
| destructible wall + break SFX | `object.cpp:1785` | `object.c:663` | **PARTIAL** (no break SFX) |
| timed bomb tick / explosion | `object.cpp:1276` | — | **MISSING** (not placed) |
| crusher, track platform, expanding spike, walker | `object.cpp:1207-1319` | — | **MISSING** (not placed) |
| pickups, armor, tanks, 1UP | `object.cpp` | `object.c:828` | **DONE** |
| COIL/JET item spawn + durations | `classplayer.cpp:1096/1130` | `object.c:300` | **DONE** |
| teleporters (castle, boss, stage boss) | `object.cpp:1186+` | `object.c:887` | **DONE** |

---

## 7. Stage / room

| feature | PC | port | status |
|---|---|---|---|
| 3 maps per stage, links, trigger bands | `test_teleport` `game.cpp:607` | `stage.c:461` | **DONE** |
| terrain: solid/stair/door/spike/water/ice | `map_collision` `character.cpp:2030` | `stage.c:39` | **DONE** |
| scroll locks / easy+hard blocks | `get_first_lock_on_left` `classmap.cpp:925` | `stage.c:100` | **DONE** |
| autoscroll (timer + 800ms delay + 20px step) | `stage.cpp:115` | `stage.c:686` | **PARTIAL** (no out-of-screen damage) |
| parallax background (speed) | `draw_dynamic_backgrounds` `classmap.cpp:576` | `stage.c:54` | **DONE** (1 layer; PC up to 2) |
| background scroll modes (up/down/left/right/anim) | `classmap.cpp:576` | horizontal only | **PARTIAL** |
| background colour (CRAM 0) | `classmap.cpp:244` | `stage.c:729` | **DONE** (2026-10-22) |
| animated tiles | `draw_animated_tiles` `classmap.cpp:304` | `stage.c:286` | **DONE** |
| colourcycle (CRAM rotation) | *PC uses animated tiles, no CRAM rotation* | `stage.c:270` | **DONE** (MD-specific technique) |
| foreground layer (3rd level tiles) | `showAbove` `classmap.cpp:343` | folded into the anim mask | **N/A** |
| map gfx overlays (rain/snow/train/lightning/shadow/inferno/wave) | `draw::show_gfx` `draw.cpp:168` | — | **N/A** |
| water bubble animation | `add_bubble_animation` `classmap.cpp:1215` | — | **MISSING** |
| castle path animation | `draw_castle_path` `draw.cpp:1048` | — | **MISSING** |
| room transition scroll | `transition_screen` `game.cpp:1032` | `transition.c:136` | **DONE** (fade fallback) |
| boss room gating | `subboss_alive_on_left` | `stage.c`/`enemy.c` | **DONE** |
| boss teleporter (`first_lock_on_bottom`) | `classmap.cpp:925` | `stage.c:844` | **DONE** |

---

## 8. HUD and effects

| feature | PC | port | status |
|---|---|---|---|
| HP / weapon / boss bars | `show_hud` `draw.cpp:1202` | `hud.c:140` | **DONE** |
| weapon tooltip | `show_weapon_tooltip` `draw.cpp:841` | `tooltip.c:122` | **DONE** (box + icon + name; tank counts missing) |
| in-game menu (weapons, tanks, config, leave) | `class_config.cpp:290` | `menu.c:297` | **DONE** |
| screen shake (quake) | `set_screen_shake` | `stage.c:598` | **PARTIAL** (shots only) |
| fades | `fade_in/out_screen` `draw.cpp:716` | `main.c`, `transition.c` | **DONE** |
| blink screen | `blink_screen` `graphicslib.cpp:1204` | stage-clear flash | **PARTIAL** |
| teleport-small effect (colour flash) | `show_teleport_small` `draw.cpp:429` | — | **MISSING** |
| pixelate transition | `pixelate_screen` `draw.cpp:779` | — | **MISSING** |
| explosion / death animations | `draw_explosion` `draw.cpp:1137` | `explosion.c` | **DONE** |
| dark mode (draw only anim/projectiles) | `_dark_mode` `game.h:273` | — | **MISSING** |

---

## 9. Audio

| feature | PC | port | status |
|---|---|---|---|
| music playback | `Mix_PlayMusic` / MOD | XGM2 `music.c:52` | **DONE** |
| SFX (25 in the PC set) | `load_all_sfx` `soundlib.cpp:112` | `sfx.c:48` | **PARTIAL** — see the missing triggers below |
| missing SFX triggers | water enter/leave splash `character.cpp:2287`, disappearing-block appear `object.cpp:607`, object break `object.cpp:1785`, beam `object.cpp:1102`, timed-bomb tick `object.cpp:1281`, classic door open `object.cpp:1647`, enter platform `object.cpp:1654`, stage selected `sceneslib.cpp:648`, teleporter activated `game.cpp:2208` | — | **MISSING** |
| volume control / mute | config | on/off only | **PARTIAL** |
| sound test | config | EXTRAS | **DONE** |

---

## 10. Saves / config

| feature | PC | port | status |
|---|---|---|---|
| 5 save slots | `select_save` | `save.c:30` | **DONE** |
| stage/boss progress bits | `game_save.stages[]` | `save.c:48` | **DONE** |
| player, lives, weapons ammo, tanks, armor | `st_items` `file_save_v4.h:13` | `save.c:95` | **DONE** |
| XP, level, bolts, shop flags, heart pieces | `st_save` `file_save_v4.h:141` | — | **N/A** |
| config persistence | `load_config` `file_io.cpp:709` | `config.c:22` | **DONE** |
| input remap | `inputlib` key map | 3 buttons | **PARTIAL** |
| language / platform / performance | `game_menu.cpp` | — | **N/A** |

---

## 11. Out of scope (decided with the user)

- difficulty select, cheats menu, game picker, CLI flags
- static tile3 foreground art, map gfx overlays (rain/lightning)
- save copy, language, platform, performance and quit options
- XP/level/shop/heart-piece systems (not in the shipped Rockbot1 data)

---

## 12. Gap list, in the order that matters

1. **Enemy AI depth** — the PC has 28 actions with options (jump family, wall
   walk, multi-shot spreads, morph, circle player, explode, throw item, play
   sfx) plus the water constraint for water enemies; the port has 16 actions and
   no water constraint (BUG-018). This is the biggest behavioural gap left.
2. **Boss behaviour** — intro sprites/fall animation, ground shake, sub-boss
   revive, per-boss attack patterns (BUG-011 blocks part of this).
3. **Enemy projectile trajectories** — the ones that still fall back to LINEAR.
4. **Runtime bugs from the Seahorse cluster** — BUG-022/023/024 (tileset, link
   fade, after-ladder corruption); needs the instrumented run.
5. **SFX coverage** — water splash, block appear, object break, beam, bomb
   tick, classic door, stage selected, teleporter activated.
6. **Small features** — weapon tooltip, disappearing-block appear animation,
   water bubbles, teleport-small flash, dark mode, autoscroll damage,
   quicksand, ground-quake `block_jump`.
7. **Scene player ops** — MOVE_VIEWPOINT, SHOW_ANIMATION, SUBSCENE,
   PLAY_MUSIC/SFX (used by the ending scenes).
8. Known bugs live in `bugs.md` (BUG-006, 008, 010-014, 018, 022-024 open).

## 13. How this table was produced

- PC: `main.cpp:278` → `game.cpp:126` → `classplayer.cpp`, `character.cpp`,
  `artificial_inteligence.cpp`, `object.cpp`, `classmap.cpp`, `draw.cpp`,
  `soundlib.cpp`, `sceneslib.cpp`, `file_io.cpp`, `defines.h`.
- Port: `src/main.c` → `runStage` → `player.c`, `enemy.c`, `eshot.c`,
  `object.c`, `stage.c`, `transition.c`, `hud.c`, `menu.c`, `dialogs.c`,
  `scene.c`, `ending.c`, `music.c`, `sfx.c`, `save.c`, `config.c`.
- Both inventories were produced by a read-only sweep of the two trees
  (2026-10-22) rather than from memory.
