# Roadmap — RockBot Mega Drive port

Open work left after M16. `stage0_plan.md` stays the milestone history /
single source of truth; this file is the forward-looking list.

Numbering is an id, not an execution order — see **Execution order** at the
bottom for what actually gets done in which sequence.

## 1. Shield move (ROCKBOT only)  — M16 leftover  **[DONE]**

PC `character/classplayer.cpp:916`: while `have_shield`, holding the shield
button with NO direction / jump pressed enters `ANIM_TYPE_SHIELD`
(`defines.h:193`). While shielding the player cannot attack
(`classplayer.cpp:300`) and enemy shots that hit the shielded side are
destroyed/reflected (`classplayer.cpp:704-708`). Only ROCKBOT has
`have_shield` (from `player_list_v3_1_1.dat`; KITTYBOT does not).

Done: gesture is **DOWN+A** (no free button on a 3-button pad), sprite frame 22
added as `ANIM_SHIELD`, attack blocked, enemy shots are blocked (destroyed —
reflection is not ported). See `stage0_plan.md` M16.

## 2. Dash vs slide hitbox height  — M16 leftover  **[DONE]**

PC differs dash (not `can_slide`) from slide only by hitbox height:
`character.cpp:2477` + `SLIDE_Y_ADJUST 12` — a SLIDING player with
`can_slide` (ROCKBOT) uses `sprite_hit_area.y + 12`, `h - 12`; a dash keeps
the full standing box.

Done: `box_t`/`box_b` track the current box (`update_hitbox()`), so only a
sliding ROCKBOT shrinks; everything else (including the air dash) keeps the
standing box.

## 3. Stage select honesty (slots 4..9)  **[DONE]**

The select screen (`src/stageselect.c`, `ssel_boss_names[9]`) offers 9 slots,
but only 3 stages are converted (`STAGE_DEFINED_COUNT 3`), and
`stage_set_data()` clamps anything >= 3 to stage 0 — so picking slot 4..9
silently loaded CHAOS CITY.

Done: `stage_ready()` gates START (an unimplemented slot no longer launches
anything) and the bottom bar prints `NOT CONVERTED YET` for those slots.
Converting the remaining stages is still open (the converters are already
parameterised by stage index).

## 4. Terrain: DOOR / WATER / ICE  **[DONE — nothing to do]**

Scanned the converted terrain (`src/stage0..2_data.c`): **none of the three
appears in any converted stage.** Only SOLID, STAIR, SPIKE, MOVE_LEFT/RIGHT
and SCROLL_LOCK are used.

- DOOR (3) / WATER (5) / ICE (6): 0 cells in stages 0..2 → not implemented
  (nothing to test against; YAGNI). Implement them when a stage that uses
  them is converted.
- SCROLL_LOCK (10) *is* used (stage 1 map 1 col 59, map 2 col 97) and is
  already handled: `tools/convert_stage.py` uses `lock_terrain = {1,3,4,6,10}`
  (mirroring PC `classmap.cpp:186`) to emit the per-column camera locks, and
  the surrounding tiles are solid anyway.

## 5. Weapons 1..8 (of 11)  **[DONE]**

PC `game_weapons_v301.dat` (`file_weapon`: id_projectile, name, damage) gives
the eight boss weapons + the buster:

| id | name | projectile | traj |
|---|---|---|---|
| 0 | NORMAL WEAPON | 0 | LINEAR |
| 1 | APE QUAKE | 9 | QUAKE |
| 2 | PETALS RAINBOW | 6 | ARC |
| 3 | SONAR WAVE | 5 | (12) |
| 4 | DESERT STORM | 1 | FOLLOW |
| 5 | MAGIC STAR | 4 | (11) |
| 6 | DYNAMITE FLASH | 2 | (8) |
| 7 | SPIKED CHAIN | 7 | no art -> buster |
| 8 | TIMED BOMB | 8 | (5) |

Ownership is NOT a separate flag: PC `dataUtil::has_weapon()`
(`data/datautil.cpp:23`) = `game_save.stages[n] == 1`, i.e. a weapon is owned
once its stage is beaten — the port already tracks that in the save mask.

Done: `tools/convert_weapons.py` -> `src/weapon_data.{c,h}`;
`tools/convert_players.py` now emits all 12 `weapon_colors` per player as
`s8` (-1 = unset); the player palette swaps indices 13/14/15 with the selected
weapon's colours; the shot takes sprite/speed/damage/size/trajectory from the
projectile the weapon fires; the ammo gauge (28, refilled each stage) is
consumed per shot (QUAKE 4, else 1) and only the buster is unlimited; the
in-game menu lists and switches the owned weapons with LEFT/RIGHT.

Deviations: trajectories QUAKE and ARC are ported, the rest fall back to
LINEAR; projectiles with no art in the port (7 SPIKED CHAIN) fall back to the
buster; the item weapons (coil/jet/tanks) and the weapon grid UI are not
modelled.

## 6. Real HUD  **[DONE]**

- Weapon gauge: `hud_weapon(weapon, ammo)` — 5 balls at x=62, shown only for a
  non-default weapon (PC draw.cpp:1217).
- Lives: 5 per stage (PC `reset_lifes()`), one per death, shown in the in-game
  menu as `x[N]`; running out leaves the stage (PC game over).
- HP balls + boss row were already there. There is no on-screen score in the
  PC either (only the modern/vintage energy bars + the menu).

## 8. Boss intro background art  **[DONE — nothing to do]**

The PC draws the boss's `bg_graphic_filename` from
`images/sprites/enemies/backgrounds/` behind the intro sprite
(`draw::show_boss_intro_sprites`, draw.cpp:289-330, 320x117 centred at y=34).
Scanned all three enemy lists (`game_enemy_list_3_1_2/_3_1_1/.dat`): **every
record's bg field is empty** — only `graphic_filename` is set. The one file in
that directory (`destrin_ship.png`) is not referenced by any enemy. Nothing to
emit.

## 7. Playtest pass  **[PARTIAL]**

Spawn position verified in all 9 stages (2026-10-18, CANDYBOT): every one now
starts with the player standing on the ground (see `stage0_plan.md`).

Still untested by playing: spike damage, conveyor tiles, per-stage content
beyond the spawn, the weapons (task 5) and the platform crush fix. The
emulator harness can drive inputs but cannot judge feel — this needs the user
at the controls.

**Verified in-emulator (2026-10-18)**: the boss fight (MILONGA BOT renders,
walks, fires, HP row drops as shots land) and the shield (ROCKBOT, DOWN+A
draws the shield frame). See the boss investigation in `stage0_plan.md` for how
to reach an arena — add a temporary link near the spawn, do not warp the
player.

## 9. Per-stage music — DEFERRED (needs tooling)

Every stage currently plays the single `res/music.vgm` (one shared XGM2
track). The PC build has **29 `.mod` (Amiga/ProTracker) tracks + 1 `.mp3`** in
`build/games/RockDroid1/music/` (e.g. `rockbot_chaos_city.mod`,
`rockbot_jungle.mod`, `rockbot_stage_complete_00.mod`, `rockbot_opening.mod`).

Two possible routes, both need work:

1. **Amiga MOD -> VGM.** Requires writing a MOD-to-VGM converter. Repo
   already carries pieces: `tools/mid2vgm-beta5/` (MIDI -> VGM),
   `tools/rymcast-1.0.6-win-x64/` (RYMCast, MOD player/streamer),
   `tools_zanake/midi2vgm.py`. A MOD -> MIDI step would have to be written.
2. **Render -> transcribe.** Render the MOD (or use the `.mp3`) to audio, then
   transcribe to MIDI with NeuralNote
   (`/mnt/ExtraData/programas/src/NeuralNote/build/NeuralNote_artefacts/Release/Standalone/NeuralNote`),
   then MIDI -> VGM. **Blocker: NeuralNote is a GUI app** — driving 29 tracks
   needs a CLI mode added to its source and a rebuild.

Note: MD XGM2 is YM2612 FM + PSG + PCM, so a converted track will not sound
identical to the MOD.

## 10. Ending / final boss

Not started (castle / credits / end sequence). Needs the castle stages and the
final boss first — currently only 3 of the 9 selectable stages exist.

## 11. HUD ball art: one sheet + a runtime colour swap

`res/hud_hp_ball.png`, `res/hud_wpn_ball.png` and `res/hud_boss_ball.png` are
the **same 5-frame 10x10 art** with only the fill colour changed. The PC does
exactly this and swaps the colour at load time
(`hud_player_wpn_ball.change_colorkey_color(COLOR_KEY_GREEN,
player_list[].weapon_colors[selected_weapon].color1)`, `draw.cpp:889/1215`).

The port currently ships three 16x16 sheets = 3 x 5 x 4 = **60 tiles of SPRITE
VRAM** (the engine only has 504, shared with the player/enemies/shots). One
sheet + a runtime swap would cut it to 20 tiles — and would also give the
weapon row the *real* weapon colour instead of the sheet's fixed green.

Needs a palette line per row (all four are in use during gameplay: PAL0
enemies/objects/shots/HUD, PAL1 stage, PAL2 player, PAL3 parallax) or the fill
index pointed at PAL2's 13/14/15 for the weapon row — the same trick the
player palette already uses.

Investigated (2026-10-17): the PAL2 route works for the *fill* but not for the
rest of the ball — index 0 of the player palette is the light/transparent slot
(sprites treat index 0 as transparent) and the black outline lives at a
**different index per player** (p1:7, p2:8, p3:11, p4:9), so one sheet cannot
borrow PAL2 for the outline without remapping the player art. Options if this
becomes worth it: (a) accept one colour for all three rows, (b) draw the gauges
on the VDP **window plane** with tiles (per-tile palette bits, no scrolling) —
that also frees the sprite pool entirely.

## 12. Animated tiles: colour cycling instead of frame sets

`anim_tiles.dat` is **0 bytes** — the animated tiles come from
`anim_block_list.dat` + the map data. The payoff is ROM + per-frame upload
bandwidth (the tiles are re-uploaded per frame; VRAM is not saved), and the
ROM is only 640K of 4MB, so this is low priority.

The PC used **colour cycling** for some animations — the game's other colorkeys,
green `(55,255,0)`, magenta `(255,0,255)`, cyan `(0,255,255)`
(`defines.h:1039-1049`), exist for exactly that (they are palette-swap keys,
not transparency; only `(75,125,125)` is the transparency colorkey). Worth
doing where an animated slot is a pure palette pulse (same tile shape, only
colours change) — check the frames before assuming.

Cycling a CRAM entry per frame costs no tiles and no ROM. Worth doing where the
animation is a pure palette pulse (the port currently bakes every frame).

## 13. More stages  **[DONE]**

The select offers 9 slots; only 3 stages were converted, so 6 slots were
dead. All nine are now converted (`STAGE_DEFINED_COUNT 9`): CHAOS CITY,
DEEP JUNGLE, FLOWER FIELDS, ATLANTIDA, DESERT, MUMMY, SPIKE, TECHNO, CASTLE —
`tools/convert_stage.py N` + `convert_enemies.py N` + `convert_objects.py N`
+ the `img2tiles` bg steps, one resources.res block per stage, and the
`stage_defs[]` registry. ROM went 640K -> 896K.

Stage 3 (ATLANTIDA) and stage 8 (TECHNO) were verified in-emulator (load,
render, scroll); the rest use the same pipeline but are **not** played through
— see 7.

Fixed on the way: `convert_enemies.py` asserted `record[i].id == i`, but the
list is positional and records 59/60 (UFO/FLYBOT) carry 0xFF in the id byte —
stage 4 spawns one of them, so the conversion aborted. The assert now accepts
0xFF and trusts the index.

## Execution order

1. ~~**1 (shield) + 2 (hitbox)**~~ — DONE.
2. ~~**3 (stage select honesty)**~~ — DONE.
3. ~~**4 (terrain)**~~ — DONE (nothing to implement yet).
4. ~~**5 (weapons)**~~ — DONE (buster + the eight boss weapons).
5. ~~**6 (real HUD)**~~ — DONE.
6. ~~**8 (boss intro bg)**~~ — DONE (nothing to implement).
7. ~~**More stages**~~ — DONE: all 9 selectable stages are converted.
8. **7 (playtest pass)** — needs the user at the controls. Now also covers the
   6 newly converted stages (only ATLANTIDA and TECHNO were spot-checked).
9. **11 (HUD ball dedup)** and **12 (colour cycling)** — VRAM/ROM savings, do
   them when VRAM pressure or ROM size bites.
10. **10 (ending)** — needs the castle stages + final boss first.
11. **9 (music)** last — blocked on the tooling above.
