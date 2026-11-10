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

## 4. Terrain: DOOR / WATER / ICE  **[WATER+ICE MINIMAL, DOOR UNUSED]**

Stages 0..8 use only SOLID, STAIR, SPIKE, MOVE_LEFT/RIGHT and SCROLL_LOCK.

- FROZEN LAIR (13) is the first stage with WATER (map 0: 216 cells) and ICE
  (map 0: 73, map 1: 79). Minimal port: water is no longer ground (the player
  falls in), gravity drops to 0.09 and max fall to 2; ice uses accel 0.15 /
  decel 0.03 so the player slides. Both are untested in play.
- DOOR (3) still has 0 cells in every converted stage.
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
`build/games/Rockbot1/music/` (e.g. `rockbot_chaos_city.mod`,
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

## 12. Animated tiles: colour cycling instead of frame sets  **[DONE]**

Done 2026-10-19. The art width decides: a 16px-wide strip is one frame and
becomes a CRAM rotation (3 band entries in PAL3, plus a stable entry for art
black), anything wider stays on tile animation. One pool per stage, chosen by
the colour set with the most cells. Live in stages 1, 2, 3, 6, 8: water +
waterfalls + water surface (10 blocks), lava, and the moving platforms.
Animation slots dropped 10,8,14 -> 6,0,6 (stage 2) and 11,11,9 -> 3,3,1
(stage 3); the water borders, magnets and palmtrees stay multi-frame by
decision. Full detail in `stage0_plan.md` -> "Colourcycle".

Original note follows.


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

## 13. More stages  **[DONE — 14 converted]**

`STAGE_DEFINED_COUNT 14`: stages 0..13 (intro, 8 bosses, castle #1 stages
9..13 = LAVA PIT, SPACE BASE, POWER GRID, RECYCLE FACTORY, FROZEN LAIR).
`tools/add_stage.py N` now does the whole add (`convert_stage` +
`convert_enemies` + `convert_objects` + the bg `img2tiles`, the resources
block, the registry and the Makefile `STAGES` list). ROM 1.0M -> 1.31M.

The asset recipe now loops over `STAGES` instead of one block per stage, and
the VRAM layout is derived per map (tileset -> bg -> anim) because FROZEN LAIR
map 1 needs 202 unique 8x8 tiles (the old fixed 144-tile block overflowed).

Castle flow: after intro + 8 bosses the select centre unlocks and returns the
first unbeaten castle #1 stage (`get_last_castle_stage`); castle stages draw
`classic_style_castle.png` for 3s before their boss intro. `castle_data_*.dat`
/ `castle*_points.dat` are editor-only in the PC code (nothing reads them at
runtime), so they are not ported. Ending after stage 13 is still open.

Stage 3 (ATLANTIDA) and stage 8 (TECHNO) were verified in-emulator (load,
render, scroll); the rest use the same pipeline but are **not** played through
— see 7.

Fixed on the way: `convert_enemies.py` asserted `record[i].id == i`, but the
list is positional and records 59/60 (UFO/FLYBOT) carry 0xFF in the id byte —
stage 4 spawns one of them, so the conversion aborted. The assert now accepts
0xFF and trusts the index.

## 14. Room transition (the real scroll)  **[DONE]**

Replaced the palette fade between rooms with the PC's vertical screen scroll
(`game.cpp:1067`): `src/transition.c`, per-scanline horizontal scroll so both
halves sit at their own camera offset, the destination screen's tiles copied
into the free VRAM block, plus the arrival placement (clamp into the map, hang
on the ladder when the destination is a stair), the `stage_load()` Map leak, the
img2tiles black-palette bug (the green sky), enemy/object spawn visibility, the
thick-wall horizontal collision, the intro logo padding and the flipped-frame
offset. Full detail in `stage0_plan.md` -> "Room transition: the real scroll".

**Update 2026-10-19**: the colourcycle conversion freed enough animation tiles
that the last fallback is gone — `tools/check_vram.py` now reports **0
transition fallbacks** (stage 2 map 2 has 80+ free where 81 were needed before
the waterfall family was converted). The verification pass also covered the
other 8 stages' parallax and the flipped-frame offset (both OK).
Still not verified by hand: the return trip, and the fade path is now unreachable
in the converted stages.

## 15. Waterfall gap (stage 2 map 2, cols 133/134 row 7)

Still open (handout item 2). Those two cells are static water tiles inside an
animated column (rows 0..6 and 8..14 are anim cells; columns 135/136 are fully
animated). The PC's data follows the same rule as our converter
(`init_animated_tiles`), so it is a rendering difference, not a converter bug —
compare against the PC at that spot; the anim art may span two cells.

**Update 2026-10-19**: re-checked the data — in stage 2 map 2 the columns 133/134
of row 7 are *empty* (`locked=0, t1=-1,-1, t3=-1,-1`), and the anim cells in
that map are the water surfaces at row 11 plus the row-2 over-player decor, so
the original report's coordinates no longer describe the data. The waterfalls in
that stage are now colourcycled, which changes how the column renders. Needs a
fresh reproduction with the camera position before touching anything.

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
