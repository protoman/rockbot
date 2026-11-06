# Handoff — Mega Drive port (2026-10-24, second session)

Start here in a new session. `bugs.md` has the entries, `pc_gaps.md` is the
PC-parity backlog, `PLAN.md` the roadmap.

## Standing rule added this session

**Every reported bug gets an offline reproduction first** (see `AGENTS.md` →
"Bug workflow"). It must fail on the tree before the fix and pass after. The
emulator is only for things no check can see (feel, colour, timing).

## Where the tree is

- `make` exit 0, ROM ~2.25 MB, no emulator left running.
- Checks, all green: `check_rules.py`, `check_hud_bars.py`,
  `check_projectiles.py`, `check_trajectories.py` (known fallbacks listed),
  `check_palettes.py`, `check_vram.py`, `check_fx.py`, `audit_data.py`,
  `audit_links.py` (57 pre-existing NO-FLOOR notes, every `LADDER@` ok),
  `audit_ai.py` (prints option usage).
- Commits this session: report batch 2 (`e3607e58`), wall shooter projectile
  table + TARGET_EXACT (`a16e65bc`), AI spawn dir + hop stepper (`07c533f2`).

## Fixed this session (each with its offline reproduction)

| report | fix | reproduction |
|---|---|---|
| HUD showed no bar at 1 HP | `set_bar` clamps to `HUD_BAR_MIN_FRAME` when pct > 0 | `check_hud_bars.py` |
| boss door re-opened when walking left into it | `o->started` set/checked + PC's vertical-span test | `check_rules.py` door-once |
| door stayed at its old screen position during a room scroll | `objects_sync_sprites` syncs the slice sprites (`door_sync`) | `check_rules.py` door-scroll-sync |
| knockback pushed the player inside a door | pushback applies `objects_block_x` | `check_rules.py` pushback-collision |
| slide passed a horizontal scroll lock | the lock probe runs while sliding and sweeps every column crossed | `check_rules.py` slide-scroll-lock |
| WOOD SHIELDER walked through the destructible block | enemies stop at solid map objects (`objects_block_box` in `step_axis`) | `check_rules.py` enemy-object-collision |
| charged shot broke the destructible block | only an exploding BOMB breaks type 22 (PC classplayer.cpp:800) | `check_rules.py` wall-bomb-only |
| wall shooter fired a straight normal shot | converters read the **v3** projectile table (36 records), `PROJ_MAX` 35, `TRAJ_TARGET_EXACT` implemented | `check_projectiles.py`, `check_trajectories.py` |
| spawned children always faced right; airborne hops 20% fast and through walls | `ai_spawn_npc` passes a data dir byte, airborne hop uses `step_axis` | `audit_ai.py` (context) |

## Open from the 2026-10-24 report (in the order I would take them)

1. **Ladder arrival unaligned (stage 5 room 0, first descent; APE too)** and
   **death when going back down the second ladder**. `audit_links.py` says the
   landing box is legal for every stage-5 ladder, so the model and the game
   disagree somewhere the audit does not cover: next step is to log the actual
   `px/py` and `cam_x` the link handler computes on the first descent (KDEBUG
   one-liner in main.c's link block, one emulator run) and compare with
   `audit_links.arrival()`. Suspects: the ±2 stair scan picking a column that
   belongs to a *different* ladder when two ladders are close, and
   `player.on_ladder = TRUE` being forced on an arrival whose stair column is
   not the one the climb code later uses.
2. **Transition draws the CURRENT room's parallax behind the incoming screen.**
   Known design: `transition_scroll()` sets BG_B's per-line scroll from
   `bg_info[from_map]`, and BG_B's *tiles* are the current map's, so the
   incoming half shows the old room's art. Each map has its own bg tiles/palette
   (`bg_tiles[map]`, `bg_pal[map]`) and both halves share PAL3, so the fix must
   be: copy the destination bg tiles into the transition's spare VRAM, fill the
   newly-claimed rows of BG_B with the destination bg map row-by-row as the
   split moves (rows < split keep the old art), and use the destination
   `bg_info[to_map].speed` for those rows via HSCROLL_LINE. When the two bg
   palettes differ (compare the 16 CRAM words) fill those rows with tile 0
   instead, so the wrong art never shows. `stage_load(to_map)` rebuilds BG_B
   afterwards, so only the slide frames need this.
3. **GAMBIT falls through the floor after two cart throws** (stage 5).
   `AI_THROW_ITEM` is the only path not covered by the 50 Hz stepper audit;
   check whether it leaves `no_gravity`/`jumping` set, or moves `e->y` without
   re-probing the floor.
4. **ANGRY BUNNY spawned by TOP HAT jumps straight up.** The spawn direction is
   fixed, but confirm the chain: TOP HAT `{100,13,1,5000,54}` → child def 54,
   whose chain starts `{100,2,1,800,0}` = JUMP option 0 (AHEAD). If it still
   hops in place, log `e->dir`, `e->jump_opt`, `e->dest_x` on the first jump.
5. **"YOU GOT <weapon>" screen draws its text over the player** (stage 5 pickup
   screen). Layout only: move the text block out of the player's column.
6. Known trajectory fallbacks (`check_trajectories.py` prints them):
   CHAIN, BOUNCING, BOMB_RAIN, LARGE_BEAM, LASER. Only stages 7/10/13 use them.

## Never launch the emulator without asking

See `AGENTS.md` → Emulator. One permission = one launch; `tools/emu.sh stop`
afterwards.
