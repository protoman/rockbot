# Handoff — Mega Drive port (2026-10-22, session 2)

Start here in a new session. `bugs.md` holds the full entries; `PLAN.md` holds
the roadmap; this file is the short state + what to do next.

## Where the tree is

- ROM built: `out/rom.bin` (normal build, no profiler).
- This session (offline only, no emulator launched): fixed every report from the
  2026-10-22 play test except the two that need a run — see "Still open".
  `736867c0` was the last commit before it; the fixes are in the commit that
  carries this file.
- Only dirty files are the user's own (parent `game.cpp`,
  `build/games/RockDroid1/stages_v301.dat`, `mega_drive/TODO.txt`). Do not
  commit those.

## Fixed this session (all offline, needs one play-test)

1. **BETABOT colours + HP bar colours (BUG-035)** — one cause, not two.
   `player.c::rebuild_player_palette()` wrote the classic HUD bar colours into
   PAL2 entries 8/9/12 — the *player* palette. BETABOT's whole black body is
   index 8, so his body rendered red; ROCKBOT was unaffected (his sheet leaves
   8/9 unused), which is why only some players looked wrong. The HP bar now
   lives on **PAL0** with the PC's fixed colours (`draw_energy_bar` always uses
   PLAYER 1's weapon-0 colours: purple + light blue + black), the weapon bar
   keeps PAL2 13/14 (the art's own swap placeholders) with a transparent empty
   part and 3 black PAL0 backdrop sprites behind it.
2. **Dialog text background + A button (BUG-037)** — the font paints a whole
   8x8 cell per character with palette index 0 as the background, so drawing
   the lines with the default PAL0 painted black cells over the panel. PAL3 is
   now built with entry 0 = the panel fill and 15 = white, and the pages are
   written with `VDP_setTextPalette(PAL3)`. The A-button is a font "A" glyph
   (the 12x12 art went through `img2tiles.py`, which re-quantizes even an
   indexed source, and its white landed on a PAL3 border index = black box).
   `images/backgrounds/btn_a.png` and both build steps are gone.
3. **Checkpoint animation (BUG-026)** — the object logic was right all along
   (`anim_loop = 1`, 2 frames, 300 ms). The art was the bug: PAL0 has one green,
   so "screen off" and "CHECK POINT" quantized to the same index and the two
   frames differed by 6 px. `tools/import_pc_art.py` now pins them apart
   (`CHECKPOINT_OVERRIDES`: screen -> black, text -> white, LEDs -> red) and
   `images/sprites/objects/checkpoint.png` was rebuilt: 130 px differ now.
4. **Explosions (BUG-036)** — the player-death branch never called
   `explosion_update()` (puffs froze on frame 0 and were never released) and
   the port burned 3 puffs in a horizontal row. Now the centre puff repeats
   over the 90-frame death (0/30/60) and four more spawn at ±32 px diagonals at
   18/36/54/72, like `game.cpp:1707/1734`. The **boss** death is a different
   animation: the PC sends eight explosions outward from the centre
   (`classic_style_draw_boss_death`), so `explosion.c` grew
   `explosion_ring_start/update` (8 sprites, PC art 5x24x24 from
   `tools/convert_assets.py`, 3 px/frame) and `runStage` runs it instead of the
   old pseudo-random scatter.
5. **Stage-select names (BUG-014 / BUG-030)** — the lines were drawn with the
   global PAL0, whose entry 0 (CRAM 0, the backdrop) the stage leaves on a map
   colour; the names rendered that colour on itself (peach on peach). They are
   drawn with **PAL1** (the grey ramp already loaded for the beaten faces: 0 =
   black, 15 = grey) and the default font is re-uploaded first
   (`VDP_loadDefaultFont(CPU)`).

New: `tools/check_palettes.py` — offline asserts for all of the above (HUD/PAL2
split, checkpoint frame difference, stage-select text palette). Run it after
`make assets`; it is the tripwire for the palette-line regressions.

## Still open

- **CHAOS CITY slowdown** near the active platforms + small pill: needs
  numbers. `make profile` (MD_PROFILE) boots into the stage hardcoded in
  `main()` — set it to 0 first. No code change until the profile exists.
- **Boss intro on a replayed stage**: the PC skips it
  (`classic_style_boss_intro` returns when `game_save.stages[stage] != 0`) and
  so does the port. If the user wants it back, that is a one-line change in
  `runStage` — ask first, it is a deviation from the PC.
- BUG-004 (per-stage palette budget) and roadmap item 15 (waterfall gap) as
  before.
- The 2026-10-22 re-tests the handoff already carried (sprite VRAM fix /
  invisible player / crash) are still pending: the pool change was not touched
  this session.

## Next session, suggested order

1. One emulator run (ask first) covering: BETABOT stage-select + in-game colours
   and the HP bar, the boss dialog text/arrow, a checkpoint activation, a death
   explosion, the boss-death ring, and the stage select names after a boss.
   Ask before launching.
2. If the CHAOS CITY slowdown persists, `make profile` with stage 0.

## Reminders

- Offline first: `make`, `tools/check_vram.py`, `tools/check_palettes.py`,
  `tools/audit_data.py`. Ask before every BlastEm launch (`blastem_debug.md`).
- `make assets` regenerates `src/stage*_enemies.*`, `res/*` and rewrites the
  matching `SPRITE` lines in `res/resources.res`.
- Sprite pool is 768 tiles; if a new sheet makes `SPR_addSprite` fail the
  symptoms are invisible sprites and ADDRESS ERROR crashes, not compile errors.
- A sprite frame that is fully transparent is safe (rescomp emits 0 VDP sprites
  for it) — that is how the weapon bar's empty frames work now.
