# Known bugs / deviations — RockBot → Mega Drive

Things that are wrong or worse than the original. Track here, fix later.

## BUG-005 — Returning through a stair kills the player instead of teleporting

**Status**: **FIX IMPLEMENTED 2026-10-21, needs one emulator verification**
(arrival no longer leaves the player inside the band he just used).

**Symptom**: SUN-GOD REALM (stage 4, "Mummy"), climbing the stairs *up* works
normally (forward room change **scrolls**, no fade — confirmed 2026-10-21).
Coming back **down**, the player dies unless he first climbs a good deal
higher; climbing high enough makes the room change happen as it should.

**What is known**:
- The first staircase right of the stage start is a jump-grab ladder at tile
  x=60 (stair rows 0-2 and 6-9, body-middle sampling row 10 at ground level);
  the floor-level ladder is tile x=65 (rows 9-13). Both behave like the PC.
- `tools/audit_links.py` reproduces the arrival math of every link in both
  directions against the generated terrain. For stage 4 the arrival of the
  return link (map 2 (189,13) -> map 0 (60,0)) is **clean**: px 955, on a
  stair, no solid overlap, no out-of-map tile. Only two stage-4 arrivals are
  flagged, both `NO-FLOOR` bottom-edge arrivals (link4 rev, link5 rev).
- So this is not a static placement bug: the death happens dynamically after
  the arrival (or on a different link than the one assumed).

**Repro**: stage 4, climb the x=60 ladder to the top (room change), then use
the stair back. Watch for the death animation instead of the return room.

**Next step**: one KDEBUG run (`kprintf` in `stage_check_link`,
`transition_scroll`, `stage_load`, `player_place`) around the return link —
see `blastem_debug.md` for the recipe.

**Latch analysis (2026-10-21, offline)** — user's hypothesis: *the player never
leaves the link, so the return link cannot re-activate*.

- `stage_check_link` keeps `link_latch` set until the link's `hit` becomes
  false ("cleared on region miss only"), the port's version of the PC
  `player.teleporter != -1` gate + the `else { set_teleporter(-1) }` reset.
- The PC arrival for non-teleporter links keeps the player's screen position
  and only scrolls the map (`test_player->set_position(real_x + new_map_pos_x,
  player.y)`), plus the `new_map_pos_x -= diff_h + 2` offset. It never places
  the player on the destination tile centre.
- The port places the player **exactly on the destination cell** and sets
  `on_ladder` from its terrain. For a stair link whose destination is a
  top/bottom row, the trigger band of the return link overlaps the arrival and
  its margin points outside the map, so the only way to clear the latch is to
  climb away by a large amount — this matches "dies unless I go up a lot, then
  it changes rooms as it should".
- Candidate fixes:
  A. Offset the arrival a couple of tiles away from the trigger cell (PC-like),
     so the player starts outside the band and re-entering fires the return.
  B. Clear the latch once the player's *tile* differs from the arrival tile
     (then the return fires from inside a wide band) — needs a guard against
     immediate ping-pong.
  A is closer to the reference implementation and does not loosen the latch.

**Implemented (A)**: `stage_link_band_hit()` in `stage.c` reports whether the
player box sits inside any link band of the current map, with the same
geometry `stage_check_link` uses. `main.c` now, after a link arrival clamped to
the top or bottom map edge, steps the player off that edge in 8 px steps (max
four) until the box is outside every band, then sets `on_ladder` as before.

Worked example (map 0 (60,0) -> map 2 (189,13)): the bottom-map-row rule now
judges by the **body centre** against the row's top edge (`centre >= ty*16`)
instead of the feet sample against `ty*16 - 8`. Arrival py=195 (feet at the
224 screen bottom) has centre 211 >= 208, so it is inside the band; the nudge
moves him up just **4 px** (py=191, feet 220), centre 207 < 208, and the
latch can clear. Pressing down re-enters the band and fires the return — the
player now stands at the bottom of the screen instead of a tile up the ladder.

Top-edge arrivals whose band is already missed (e.g. map 2 -> map 0 (60,0),
feet sample py=21 vs band -19..16) are left untouched.


## BUG-001 — Title screen art loses color depth

**Status**: **FIXED 2026-10-19** — the title draws the PC art at 59 colours
over the four palette lines, verified on hardware: navy backdrop, bright
yellow logotype and the characters' own hues. Three bugs had to go: the
near-colorkey background (below) and an off-by-one palette slot
(`lessons_learned.md` #27), and the palette *selection* - lines filled in
tile scan order let the characters' greys take the slots. The final art uses
one palette line per part with a 15-colour characters image: 15 colours in
15 slots, and folded tiles went from 249 to 1. See the
update at the end of this entry for the recipe and the two traps.

**Symptom**: the title logo and character art look banded/washed out compared to the
PC version. Colors that should be distinct blend together; gradients on the robots
show visible steps.

**Why**:
1. Current pipeline quantizes each image to **15 colors + reserved index 0** (index 0
   must stay unused so no pixel becomes transparent by accident). That is 1 usable
   palette of 16 on a machine that has **4 palettes x 16 = 64**.
2. RGB is additionally rounded to the MD **3:3:3** grid (`MD_LEVELS` in
   `tools/convert_assets.py`), which discards precision before quantizing even
   happens.
3. The source is 24-bit (`logo.png` has 8417 distinct colors).

**What the hardware actually allows**:
- `rescomp` reads an **8bpp indexed** PNG and uses **bits 4-5 of the palette index**
  to select which of the 4 sub-palettes a pixel belongs to, so a single image can use
  64 colors.
- Hard constraint (from `rescomp.txt`): **every pixel in an 8x8 tile must use the
  same palette line** (bits 4-5 constant per tile), otherwise rescomp errors out.
  This means the converter needs a per-tile palette-line assignment pass — a
  constraint-satisfaction step that does not exist yet.

**Fix path** (not started):
- Quantize to 64 colors, lay the PNG palette out as 4 rows of 16.
- For each 8x8 tile, pick one palette line such that all tile colors fit in that
  line's 16 slots; merge/snap colors that do not fit.
- Emit index `(line << 4) | slot`, keep slot 0 of every line empty for transparency.
- Optionally: dither within a tile only, so the dedup does not explode the tile count.

**Update 2026-10-19 (measured, premise corrected)**: the port does not draw the
PC's title art — it draws a hand-drawn 15-colour redraw in
`mega_drive/images/logo.png` (**15** raw colours, 15 after the MD 3:3:3 ramp).
So the 15-colour limit is the art, not the pipeline: extra palette lines have
nothing to recover from it. The PC's own art
(`build/games/RockDroid1/images/logo.png`) has 8417 raw colours and **176** after
the ramp, so reproducing *that* would need the 60-colour / four-line quantiser
described above. The title is drawn from C-side tile + map arrays (not a rescomp
IMAGE), and the tilemap's 2-bit palette field can carry the line per 8x8 tile, so
that path needs no rescomp changes. **Decision (2026-10-19)**: port the PC art at 60 colours. The quantiser is
written (`tools/title64.py`: ~60 colours over four palette lines, one line per
8x8 tile, emitted as C tile/map/palette arrays) and emits sane data (59 colours,
lines 14/15/15/15, 399 tiles), but the first on-screen attempt rendered garbage -
the wiring is reverted and the title still uses the hand-drawn 15-colour logo, so
nothing is broken. Next attempt: check the plane write and the tile-base offset
(`TITLE_TILE_BASE + (entry & TILE_INDEX_MASK)`, palette bits kept from the map
entry), and whether BG_B needs the 64-wide fill to stay.

**Update 2026-10-19 (probe screen)**: a TEMPV probe that loads the 60-colour
tiles, the four palettes and the emitted map *renders the title art correctly*
on BG_B - the data and the four-line mechanism are proven on hardware. It also
showed that the two earlier "still garbage" results were stale ROMs: the capture
script had a hardcoded ROM path, so both retries tested the build with the broken
16-word tile packing. The remaining problem is only in `showTitle()`'s sequence:
wiring the same data there black-screened the ROM from boot, so that needs a
step-by-step comparison against the probe (the probe does: SPR_end, clear both
planes, palettes, backdrop, tileset, map; showTitle also does a stage release,
scroll resets, a BG_A font fill and a 64-wide BG_B fill around the same calls).
The title is back on the hand-drawn art; `tools/title64.py` (with `--preview`)
is committed and ready.

**FIXED 2026-10-19**: the wiring works once three things were right -
1. `tools/title64.py` packs **8 longs per tile** (the first version wrote 16 words
   and emitted them as u32, 64 bytes instead of 32);
2. the tileset goes in with **`DMA`** (immediate), and the four palettes with
   **`CPU`** writes - queued CRAM DMA is unreliable here;
3. `PAL0` slot 0 = the backdrop `0x0210` and slot 15 = the font's white, because
   the font glyphs are index 15 while the art's own white sits at slot 1.

Two traps that cost most of the debugging: a capture script with a **hardcoded
ROM path** (so two "still garbage" verdicts tested an old build), and a
keypress reaching the emulator while it opened (an all-black run that was not the
ROM's fault). Recipe for reusing this on the scenes: `stage0_plan.md` ->
"Four palette lines per screen".

**Files**: `tools/convert_assets.py`, `res/resources.res`

---

## BUG-002 — Title screen top band is 8px, original was 20px

**Status**: **closed 2026-10-19, by design.** The title art is a single image
whose content is 216 px tall; the MD screen is 224, so 8 px of band is what
remains. A 20 px band would either clip the art or need it scaled/redrawn -
and the title now shows the PC art at 60 colours, so the band is the least of
the differences.

**Symptom**: the dark-blue strip above the logo art is thinner than on PC.

**Why**: the original screen is 320x240 and draws `logo.png` at y=20, leaving a 20px
band. The Mega Drive NTSC screen is **320x224**. The logo art itself is 216px tall
(art bbox y=0..215), so a 20px band would clip the bottom of the robots' feet.
Compromise: 8px band, version number drawn at tile row 0.

**Fix path**: either scale the art down ~7% offline, or accept the thinner band.

---

## BUG-T01 — TEMPORARY TEST HACK: slot 1 always has the intro stage cleared

**Status**: intentional, TEMPORARY. Remove before release.

**What it does**: `save_init()` (`src/save.c`) seeds save slot 1 with stage 0
(the intro stage, CHAOS CITY) marked complete:
`save_slot_clear(0); save_slot_set_stage(0, 0); save_slot_set_player(0, 0);`
It only does so when slot 1 is **empty**, so it never overwrites a real save.

**Why**: testing any stage otherwise means replaying the intro stage and its boss
intro first. With this, the title shows LOAD GAME, and
LOAD GAME -> SLOT 1 -> STAGE SELECT reaches any converted stage in a few presses,
and the intro stage's boss intro is skipped (it is gated on the save mask).

**Caveats**:
- Slot 1 is seeded only while empty, so a real save there is left alone — but do
  not rely on slot 1 for save-persistence tests.
- NEW GAME still clears the slot it picks; the hack re-applies on the next boot.
- Other stages' boss intros still play (skippable with any button) because their
  bits are not set — add `save_slot_set_stage(0, n)` for a stage you are testing.

**Removal**: delete the marked block in `save_init()` and this entry.

---

## BUG-003 — Two anim blocks cannot use palette colourcycle (shape really changes)

**Status**: **FIXED 2026-10-19** — both were redrawn as single-frame 16x16
three-colour bands in `mega_drive/images/tilesets/anim/` and are converted by the
art-width rule (`roadmap.md` item 12).

**What**: `waterfall_top.png` and `water_ground.png` (PC
`build/games/RockDroid1/images/tilesets/anim/`) are the only animated blocks whose
**pixel shape** changes between frames, so the CRAM-rotation technique we are
adopting for the rest of the anim set cannot reproduce them: every other block
differs from frame to frame only by a colour permutation, these two move pixels
around. They must either keep the tile-frame animation (current behaviour) or be
redrawn as a repeating colour band like the rest.

**Verified**: shape-mask comparison + per-step colour-permutation test over all 34
anim blocks; these two (plus the five `palmtree_00*` blocks, which genuinely change
shape and are not meant to be cycled) are the exceptions.

**Files**: `tools/convert_stage.py` (block classification), the source PNG above.

---

## BUG-004 — Stage art is over the 15-colour palette budget (FROZEN LAIR now the worst)

**Status**: open, known deviation from the PC art (user: "I noticed some lack colors,
like the moving platforms").

**Symptom**: distinct shades in the source art merge into one palette entry, so
e.g. the moving platforms look flat. The platform art has five shades —
`(255,102,0) (255,231,163) (243,191,63) (219,43,0) (167,0,0)` — which collapse into
about two orange entries.

**Why**: the port puts the whole stage (all three maps **plus** every anim block) on
ONE 15-entry master palette (`MAX_COLORS = 15`, index 0 reserved transparent;
`tools/convert_stage.py` `build_palette`), merging the closest colours when the art
needs more. The PC art is authored as true colour (the shared tileset `default.png`
has 124 distinct colours), so it does not fit.

Measured colours needed vs the 15 available:

| stage | needed | merged away |
|---|---|---|
| 0 CHAOS CITY | 17 | 2 |
| 1 DEEP JUNGLE | 38 | 23 |
| 2 FLOWER FIELDS | 35 | 20 |
| 3 ATLANTIDA | 31 | 16 |
| 4 SUN-GOD REALM | 23 | 8 |
| 5 TRICKY CASTLE | 26 | 11 |
| 6 DARK MINE | 28 | 13 |
| 7 CRYSTAL MAZE | 16 | 1 |
| 8 INFOWAY | 14 | fits |
| 9 LAVA PIT | 14 | fits |
| 10 SPACE BASE | 20 | 5 |
| 11 POWER GRID | 26 | 11 |
| 12 RECYCLE FACTORY | 12 | fits |
| 13 FROZEN LAIR | 56 | 41 (weighted error 2324) |

Also: index 0 of the generated palette is a reserved black, so any black in the art
takes a second entry (stage 1's palette has `(0,0,0)` at both 0 and 1). Where that
black is only the *surrounding* empty area, marking it transparent in the source art
reclaims the slot; where it is part of the design (the moving platform's segment
gaps) it cannot be reclaimed.

**Fix path**: the reserved-slot design being discussed for colourcycle
(anim art drawn with generic colours, mapped to per-stage entries; cycling bands in
the free part of PAL3) — plus a deliberate re-quantisation of the art per stage.
See `stage0_plan.md`.

**Update 2026-10-19 (stages 9..13)**: FROZEN LAIR is the new outlier — 56
distinct colours for 15 slots (weighted error 2324, vs DEEP JUNGLE's 1946).
LAVA PIT (14) and RECYCLE FACTORY (12) fit exactly; SPACE BASE 164 / POWER
GRID 245 are between. FROZEN LAIR needs its own palette strategy (per-map
palette or a deliberate art re-quantisation) before it can look right.

**Update 2026-10-19 (palette merge)**: `build_palette` no longer deletes one
colour of the closest pair - it replaces the pair with its pixel-count weighted
average, so rare shades fold into the dominant colour instead of dragging it off.
It also reports a weighted error per stage, now visible in every build:

| stage | colours | error |
|---|---|---|
| 0 CHAOS CITY | 16 | 22 |
| 1 DEEP JUNGLE | 33 | **1946** |
| 2 FLOWER FIELDS | 31 | 437 |
| 3 ATLANTIDA | 20 | 299 |
| 4 SUN-GOD REALM | 23 | 557 |
| 5 TRICKY CASTLE | 26 | 350 |
| 6 DARK MINE | 25 | 216 |
| 7 CRYSTAL MAZE | 16 | 7 |
| 8 INFOWAY | 9 | 0 |

DEEP JUNGLE is the outlier and is the one worth re-quantising by hand (its art
uses 33 MD-ramp colours for 15 slots, most of them foliage shades).

**Update 2026-10-19 (colourcycle landed)**: the cycled blocks' colours left PAL1
for PAL3, so the map palette no longer has to hold them. Colours still needed for
PAL1, before -> now: stage 0: 17 -> 16, 1: 38 -> 33, 2: 35 -> 31, 3: 31 -> **20**,
6: 28 -> 25, 8: 14 -> **9** (fits, with room). Stages 4, 5, 7 unchanged (no
cycled blocks). ATLANTIDA gained the most (16 fewer shades merged); DEEP JUNGLE is
still the worst. What remains is the map art itself: it needs the deliberate
per-stage re-quantisation. The anim side of this bug is done.

**Files**: `tools/convert_stage.py`, `tools/img2tiles.py`, the source art.


## BUG-006 — Sub-boss door opens but the screen never scrolls to the boss room

**Status**: **FIXES IN, needs an in-game check** (reported 2026-10-21, Mummy
stage / SUN-GOD REALM). Two changes from the source review:
`column_locked()` now gives the **live door state priority over the converted
scroll-lock table** (a door column that is also in the map table stayed locked
after opening, so the camera could never scroll into the boss room), and
`objects_reset_doors()` re-closes opened doors when the player respawns (the
port does not reload the map on death, so a door stayed open for the rest of
the stage and its lock never came back).

**Symptom**: touching the sub-boss door lets the player through, but the
horizontal scroll that reveals the boss room never happens. Worse, the scroll
stays unlocked afterwards — even after dying and coming back.

**Suspects** (both in the port's door handling):
- `objects_boss_door_locked()` (`object.c:594`) releases the lock as soon as
  the door is `started`; the PC additionally scrolls the screen to the boss
  room when the door opens (`classmap.cpp:1611` area). The port has no
  equivalent camera/scroll step.
- Doors live in the object list and are **not reset on death**: the death path
  reloads neither objects nor the door state, so an opened door stays open
  forever in that stage attempt. Reloading the map (or re-closing doors) on
  respawn/stage re-entry should restore the lock.

**Measurement 2026-10-21** (spawn x=1664 left of the first door, walked right;
`kprintf` heartbeat with the camera):
```
F855 x1713 cam1437 d109=1
F870 x1743 cam1582 d109=0   <- door opens, camera jumps +145 px
F930 x1860 cam1700 d109=0   <- camera follows the player
F2205 x2027 cam1741 d109=0  <- stopped at the *second* door (x=2048)
```
So the lock is released and the camera does move (145 px at the door, then it
tracks the player), but there is no full 320 px screen step. The player is then
stopped at x=2027 = 2048-21 by the **second** door while the UFO sub-boss is
alive, so the boss room stays out of reach until that sub-boss dies. Repeating
this with the UFO killed decides whether a camera step is still missing.

## BUG-007 — Charged shot resets when crossing a room link

**Status**: **FIXED and verified 2026-10-21**. Holding the attack button
through a stair/link transition now keeps the charge, as in the PC.

**Cause**: `player_place()` cleared `player.charge_frames`, and the room-link
handler calls `player_place()` on arrival. Immediately after the transition
`handle_fire()` saw a "fresh press" and fired a normal shot, restarting the
charge from zero.

**Fix**: `player_place()` no longer touches the charge (it is used by the link
arrival too); the reset moved to `respawn_player()` (plus the existing one in
`player_init()`), together with `fire_prev = FALSE` so a held button after a
death does not fire.

**Verified** with `kprintf` around the link and at release:
`CHG pre=124 -> CHG post=124 -> REL chg=124` (before the fix: `111 -> 0`).

## BUG-008 — Boss dialogs are a black box (PC shows an image)

**Status**: **OPEN, next chunk**. `dialogs.c` fills the page with one black
tile; the PC blits `images/backgrounds/dialog.png` (310x65) centred and draws
`btn_a.png` (12x12) at its bottom-right corner instead of the "JUMP: NEXT"
text (`graphicslib.cpp:1801` `show_dialog`, `dialogs.cpp:176`
`draw_dialog_bg`).

Needs: convert the 310x65 panel + the A button with `tools/img2tiles.py` (same
pipeline as the menu panel), draw them on BG_A in `show_page`, and keep the
text/face positions inside the panel. The panel is wider than the current
DLG_W box, so the layout constants move too.

## BUG-009 — Rockbot has no eyes in boss dialogs

**Status**: **FIXED and verified 2026-10-21**. The face art's white eye sockets
are palette index 0, i.e. sprite-transparent, and the pupils live in the
separate `ssel_eyes_pN` layer that only the stage select drew. `dialogs.c` now
adds that eyes sprite (offset +10,+18, same as the PC's face (144,104) /
eyes (154,122)) behind the face on the player's pages only. Verified in a
forced boss-room dialog: the portrait now has eyes.

## BUG-010 — Mummy keeps facing away from the player

**Status**: **OPEN — analysis done, fix needs a decision.**
- The Mummy's AI rows (`stage4_enemies.c`, MUMMY BOT) use actions 2 (jump),
  10 (teleport) and 5 (shot ahead). In the PC, action 5 is
  `AI_ACTION_SHOT_PROJECTILE_AHEAD`, which explicitly does **not** turn to the
  player (`artificial_inteligence.cpp:1164`: only non-ahead actions do). So the
  port currently matches the PC here.
- Spawn direction is converted correctly: map_npc `dir` is 0/1 and
  `enemy_add()` maps it to -1/+1 (`enemy.c:643`).
- Therefore the wrong facing most likely comes from the **missing teleport and
  jump animations** (BUG-011): without them the boss keeps the previous art row
  while the AI repositions him, so he visually "faces" the old direction.
  Fix BUG-011 first, then re-check this before adding any facing rule.

## BUG-011 — Enemies do not switch to teleport/jump animations

**Status**: **OPEN** (probably global). `enemy.c` has stand/walk/attack frames;
teleport (AI action) and jump use no dedicated animation. The PC plays
`ANIM_TYPE_TELEPORT` / `ANIM_TYPE_JUMP` for those actions. Needs the anim rows
in the converted enemy sheets plus the AI hooks.

## BUG-012 — Projectiles not cleared when the boss dies

**Status**: **OPEN, needs a reproduction**. `main.c:602` already calls
`shots_reset()` + `eshots_reset()` when `enemies_boss_died()` fires, so either a
different projectile list is involved (boss beams?) or something respawns them
during the 2 s death sequence. Check with one run, noting the projectile type
on screen.

## BUG-013 — The "YOU GOT ..." screen differs from the PC

**Status**: **OPEN**. The port's post-boss screen is a text page; the PC shows
the weapon art/dialog. Needs the PC layout (weapon icon + text) converted.

## BUG-014 — Stage select shows garbage text after beating Mummy

**Status**: **OPEN**. After returning to the stage select, text appears over a
garbled background. Likely the stage-select plane/tiles were not fully rebuilt
after the stage (stale VRAM/tilemap or a palette left over from the boss
screen). Reproduce and compare with a normal return.

## BUG-015 — Missing boss intro for Seahorse (maybe global)

**Status**: **NOT REPRODUCED / by design (2026-10-21)**. The intro for stage 3
plays: a forced stage-3 run showed the `SEAHORSE BOT` intro screen with the
boss backdrop (screenshot `scr3/scr4` of that session). `runStage()` skips the
intro when the stage is already beaten (`save_slot_stages & (1<<stage)`), which
is the PC behaviour, so the report was most likely a stage that was already
cleared in the loaded save.

## BUG-016 — Seahorse screen transitions are broken; player X shifts

**Status**: **FIXES IN, needs a re-test.** Traced 2026-10-21 with `kprintf`
captured via `script -qf` (see `blastem_debug.md`).

Trace (ATLANTIDA, map 0 -> 1 -> 0):
```
LINK m0->1 tx176 ty0 off22
TRANS 0->1 cam-3 px185 py0
TRANS ret=1 code=0
ARR m1 x185 y0 lad0
LINK m1->0 tx928 ty0 off141
TRANS 1->0 cam909 px1056 py0
ARR m0 x1056 y0 lad0
```
Two defects shown:

1. **X shift**: the port snapped every arrival to the destination tile centre
   (`px = dst*16 + 8 - 13`). PC keeps the player's horizontal position and
   scrolls the map (`game.cpp:762`), which matters for wide links (stage 3 has
   sizes 4, 7, 8, 18). Fixed: `stage_check_link` stores the player's centre
   offset inside the trigger band (`stage_link_band_off`) and the arrival uses
   `dst_tile*16 + off - box_centre`, clamped to the map. The trace above now
   shows `off22 -> x185` and `off141 -> x1056`, i.e. preserved.
2. **Corrupted tiles / negative camera**: `scroll_lock_span()` can set
   `camera.x = (c<<4) - 307`, which is **-3** for column 19, and
   `stage_arrival_camera_x()` returned it unclamped (`TRANS 0->1 cam-3`). The
   transition then drew the destination screen with a negative camera, putting
   garbage tiles on the plane. Fixed: clamp the result in
   `stage_arrival_camera_x()`.

**Verified 2026-10-21** (spawn forced at x=751, walked right into the link):
```
LINK m0->1 tx176 ty0 off56
TRANS 0->1 cam0 px219 py0
TRANS ret=1 code=0
ARR m1 x219 y0 lad0
```
`cam0` (was -3), `x219 = 176+56-13` so the offset is preserved, no corrupt
tiles on the destination screen, HUD survives the transition.

## BUG-017 — Falling into water lands on the surface

**Status**: **FIXED and verified 2026-10-21**. `ground_pair()`
(`player.c:369`) treated "different feet tile vs centre tile" as ground, so
feet-in-WATER over centre-UNBLOCKED counted as floor and the player stood on
the surface. Water is now excluded from the ground pair; the water gravity /
jump rules already in the port take over.

Verified in ATLANTIDA (tile x=103: water rows 7-10, solid row 11, air above):
spawned at y=80 above the surface, the player fell **through** the water and
landed on the floor at y=147 (feet row 11). Before the fix he stopped at the
surface (y=83). Screenshot confirms him under the water column.

## BUG-018 — Fish leaves the water to chase the player

**Status**: **OPEN** (probably global). Water-bound enemies must stay in WATER
tiles; `enemy.c` only checks `TERRAIN_UNBLOCKED`/`WATER` for walking/cliff
decisions, not a "stay in water" constraint. Check the PC AI for the water
enemies (ATLANTIDA fish) and the `is_water`/terrain constraints, then add a
flying-into-water rule (or the opposite) per enemy type.

## BUG-019 — DR. DESTRIN boss intro shows APE BOT

**Status**: **FIXED 2026-10-21** (runtime re-check pending with the user).

**Report**: starting the castle stage whose boss is DR. DESTRIN, the boss intro
drew an Ape sprite.

**Cause**: `EnemyDef.is_boss` came from the NPC record's own `is_boss` byte
(`file_npc`, `b[6026]`). That flag is per enemy TYPE, and castle stages place
other stages' bosses as regular enemies — stage 13 spawns APE BOT (id 2),
DAISIE BOT, MUMMY BOT, SPIKE BOT, TECHNO BOT, DYNAMITE BOT, SEAHORSE BOT and
MAGE BOT, all with the flag set. `bossintro.c` draws the first `is_boss` def,
so APE BOT (id 2, sorted before DR. DESTRIN id 24) won. The same conflation
also gave the stage-13 APE BOT the boss drop-in and the boss HP row.

PC reference: `classmap.cpp:561` — only `stage_data.boss.id_npc` becomes the
stage boss (`set_stage_boss`: 36 HP, drop from the top, hit window); other
boss-flagged types only get `set_is_boss(true)`.

**Fix**:
- `tools/convert_enemies.py` reads the stage record's `file_boss.id_npc`
  (`stages_v301.dat` offset 360) and sets `is_boss` for that npc id only.
  Stage 12 was also wrong in the other direction: WASPER is its boss but its
  record flag is 0, so that stage had no boss def at all.
- `enemy.c` treats the live `boss_i` index as the stage boss for the death
  path, and an `AI_REPLACE_NPC` child of the boss inherits the status with
  full boss HP (PC `artificial_inteligence.cpp:2294`), so DR. DESTRIN's
  tank -> ship replacement continues the fight instead of clearing the stage.
