# Handoff — Mega Drive port (2026-10-23)

Start here in a new session. `bugs.md` has the entries, `pc_gaps.md` is the
PC-parity backlog, `PLAN.md` the roadmap.

## Where the tree is

- ROM built, `make` exit 0, `make assets` clean, ROM fresh. Checks green:
  `check_palettes`, `check_vram` (unchanged), `audit_data` (`option gaps: 0`,
  `animation index problems: 0`).
- Last commits: `mega drive:` series ending with the AI/anim + scene-music
  batch. Only dirty files are the user's own (parent `game.cpp`,
  `build/games/RockDroid1/stages_v301.dat`, `TODO.txt`).

## Done this session (2026-10-23, all offline)

1. **Walk-attack / jump-attack** player rows + pose selection; **STAIRS_SEMI**
   (frame 18) half-body pose when grabbing a ladder.
2. **Boss door** opening: the PC art has one frame, so the sprite slides into
   the floor and back around the scroll.
3. **Dialog palette leak**: the panel borrows PAL3 (also the parallax line) —
   the whole line is saved/restored now, so the stage colours no longer change
   after a boss dialog.
4. **YOU GOT screen**: real `config_bg.png` background + text at the PC's x.
5. **Sprite lifetime**: `sprites_begin/sprites_end` bump `spr_gen` and every
   module releases only same-generation handles. This fixed the invisible
   player + `SPR_update` crash after re-entering a stage (`SPR_end` leaves the
   freed Sprite structs in the heap block the new pool reuses).
6. **AI parity**: AI_JUMP options (incl. TO_ROOF → ceiling walk for APE),
   AI_FLY options (TO_RANDOM/DRILL_DOWN/PLAYER_DIR/OPPOSITE_WALL/OPPOSITE_SHOOT/
   RANDOM_X/RANDOM_Y), the twelve AI actions that had no code, and enemy
   ANIM_TYPE_TURN rows (BLUE SHIELDER's rotating shield).
7. **SPIKED CHAIN** (TRAJECTORY_CHAIN) extends/retracts instead of flying on.
8. **Scene PLAY_MUSIC / STOP_MUSIC** wired (converter emits `scene_music_ids[]`).
9. Quake screen shake for enemy QUAKE projectiles; boss-death ring (earlier).

## Next, in order (see pc_gaps.md for evidence per item)

1. **Scene `SHOW_ANIMATION`** — used once (scene 0). Needs
   `images/scenes/animations/` art converted from `scenes_show_animation.dat`
   (6 records, frame_w/h/delay) and a timed frame loop on BG_B.
2. **Effects batch** (needs one sprite-effect pool like `explosion.c` +
   converted art): dash dust (`dash_dust.png`), water splash
   (`water_splash.png`), bubble (`bubble.png`), teleport flash.
3. **Weapon tooltip** (`draw.cpp:841`) — the port has no clean way to draw text
   over the scrolling stage; plan: draw on BG_A, hold ~60 frames, then
   `stage_force_redraw()`.
4. **Quicksand terrain** — deferred on purpose: no converted stage uses it
   (PC: 0.2px/frame sink + halved move speed).
5. Play-test everything above (ask before launching BlastEm; one run covers:
   walking shots, door slide, dialog → stage colours, YOU GOT screen, stage
   re-entry (no invisible player / no crash), APE ceiling jump, BLUE SHIELDER
   turn animation, SPIKED CHAIN, cutscene music).

## Reminders

- **Verify builds by `make`'s exit code** — the toolchain reports `erro:` in
  Portuguese, lowercase (lesson 26 in `lessons_learned.md`).
- `tools/audit_data.py` now prints AI **options**, object types, terrain and
  sprite animation indices: run it before claiming a feature is complete.
- A fully transparent sprite frame is fine (rescomp emits 0 VDP sprites).
- Sprite pool 768 tiles; ring explosion uses 72 of them.
