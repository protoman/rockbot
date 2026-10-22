# Port plan (roadmap)

Working list for the Mega Drive port. Derived from `pc_features_vs_md.md`
(refreshed 2026-10-22). One line per item, with the state kept current; the
detailed traces live in `bugs.md` and `HANDOUT.md`.

## 1. Water / terrain constraints

- [x] Water enemies must not leave water (and land enemies must not enter it):
      PC `character.cpp:1921-1923`. Implemented in `enemies_update()`.
      **BUG-018** (first attempt compared the position with itself; fixed
      2026-10-22: the pre-move position is now passed in).

## 2. Sound effects the PC has and the port does not

- [x] Disappearing block: appear SFX + 5-frame appear animation.
- [x] Object break SFX for destructible walls.
- [x] Beam SFX for rays / lava beams.
- [x] Stage-selected SFX (already wired) and water enter/leave (already wired).
- [x] Teleporter-activated SFX (already wired).

## 3. Small features

- [x] HUD bars: the sheets are 51 frames ordered empty..full; `set_bar()` used
      the old 5-frame scale, so full HP showed frame 0 (the empty art).
      **BUG-032**.
- [ ] Weapon tooltip when switching weapons (`draw.cpp:841`). Needs sprite
      art: a plane-drawn text leaves trails on the scrolling stage, so the
      text version was reverted.
- [ ] Water bubble animation on BG (`classmap.cpp:1215`).
- [ ] Teleport-small colour flash (`draw.cpp:429`).
- [ ] Dark mode (draw only animations/projectiles) (`game.h:273`).
- [x] Autoscroll out-of-screen damage (`game.cpp:1652`):
      `player_screen_edge_update()` nudges the player 1px back and kills him
      past 2 tiles.
- [ ] Ground-quake `block_jump` after a quake hit (`classplayer.cpp:281`).

## 4. Ending / scene player

- [ ] Scene ops still missing: MOVE_VIEWPOINT, SHOW_ANIMATION, SUBSCENE,
      PLAY_MUSIC / PLAY_SFX / STOP_MUSIC (`scenes/sceneshow.cpp:41`).

## 5. Boss behaviour

- [ ] Boss intro sprites (fall-in + intro frames) `draw.cpp:280`.
- [ ] Boss ground shake on landing `classmap.cpp:2428`.
- [ ] Sub-boss revive after the boss dies `classmap.cpp:2201`.
- [ ] Boss teleport/jump animation rows (BUG-011, unblocks BUG-010).

## 6. Enemy AI (low priority: the shipped data does not use them)

The port handles every action the stages 0..13 AI data references
(walk/fly/jump/wait/shoot/invert/air-walk/fall/teleport/dash/grab-wall/
spawn/replace/change-move/jump-attack-up). Still absent, for future data:
jump family (to player/point/random, ahead, long ahead, once, up, teleport
in/out, to roof), quake attack, accelerate up/down, circle player, wall walk
(+shoot), multi-projectile spreads, morph, explode itself, throw item, play
sfx, wait random, change-animation, fly drill/sin/dash/opposite-wall.

## 7. Animated tiles (SEAHORSE slowdown — solved)

- [x] **BUG-033**: profiled with `make profile` (getSubTick, 1280 = one 60 Hz
      frame). SEAHORSE map 0 was `anim=1135` of ~2460 subticks per iteration,
      i.e. 30 fps. A slot kept 4 tiles per animation frame and the map cells
      pointed at the current frame, so each frame change rewrote the tilemap of
      every visible cell (~15 ms/frame). Now a slot owns ONE 4-tile block: a
      frame change is a single 4-tile DMA, no tilemap writes.
      `TILE_ANIM_STRIDE` 24 -> 4 frees 260 tiles (`check_vram`: all stages OK,
      0 transition fallbacks).
- [x] The batch writer added earlier (one rect call per run) is gone with it;
      the correct plane-window mapping stays documented in `stage_map_patch`.

## 8. Runtime verification (instrumented run 2026-10-22)

Found: with SEAHORSE loaded, `MEM_getFree()` is only **7 KB** after
`stage_load`, so `MAP_create()` inside `transition_scroll()` failed and every
link took the fade path (BUG-023) leaving a half-loaded room (BUG-022/024).
Fix in place: `transition_scroll()` builds the outgoing screen first, then
releases the current stage map before creating the destination Map, which frees
that heap. Needs a re-run to confirm the links now scroll.

- [ ] BUG-022 SEAHORSE: wrong/incomplete tileset at stage start.
- [ ] BUG-023 SEAHORSE: fade on every link.
- [ ] BUG-024 SEAHORSE: tileset corrupt after the second link / after a ladder.
- [ ] Re-check the 2026-10-22 fixes in game: BUG-021, 025, 026, 027, 028, 029,
      030, 031.

## Out of scope (decided)

Difficulty select, cheats menu, game picker, CLI flags, static tile3 foreground
art, rain/lightning overlays, save copy, language/platform/performance, the
XP/shop/heart-piece systems.
