## BUG-046 — Teleporter entered while sliding leaves the slide "shadows" on the next screen (fixed 2026-10-31)

**Report (2026-10-31, owner)**: entering a teleporter while sliding, the slide
"shadows" appear on the destination screen at the position they had on the
previous one, and disappear after a while.

**Cause (found, suspect above was wrong)**: `player_place()` zeroed `trail_len`, so the next `trail_update` (`if (trail_len) trail_hide()`) never hid the three ghost sprites; they kept their last screen position after the teleporter. Fix: `player_place` calls `trail_hide()`. Rule `place-hides-slide-ghosts` fails on HEAD~, passes now.

Old suspect (wrong): the dash-dust effects
(`src/effects.c`, spawned from `src/player.c` while sliding) are not cleared
when the room changes through a teleporter, so they stay alive in screen space
for their lifetime. Check what `effects_reset()` (if any) a teleporter arrival
calls, against the ladder and walking room changes.

**To do**: an offline reproduction first (host test or rule check: effect pool
empty after a teleporter room change), then the fix.

## BUG-045 — Ending: intermittent tile/palette garbage on the first frame of the walking scene (open)

**Report (2026-10-29)**: `screenshots/bugs/ending/walking_glitch.png` - the first
frame of the walking parade shows stripes, red/white garbage and the player's
shapes as tiles; sometimes it lasts a split second, sometimes it stays. Repeats
after a Tab reset of the same ROM, so it is timing/state dependent. Found with
the `-DMD_TEST_ENDING` boot hook (src/main.c); not yet reproduced in a normal
game finish.

**Measured** (log-only run, `-DMD_TILE_DUMP`, walking frames 0/1/30): the walking
scene's tile data (tile END_BASE3+1 reads all zeros) and the first palette lines
(PAL0/PAL1 still hold the dialog scene's grey/blue ramps) never arrived, the map
cell values read back wrong (BG_B cell (5,4) = tile 273, the art has 89 tiles),
while the CPU-written things (scroll, sprites) ran. Registers were normal
(reg12=81, BGA=E000, BGB=C000).

**Ruled out by reading SGDK / the project** (no emulator needed):
- V-INT handler (`src/boot/sega.s`): only the counter, `z80VIntCB` (XGM2 volume
  fade) and the user callback; no VDP writes, no DMA.
- Immediate and queued DMA both halt the Z80 (`HALT_Z80_ON_DMA 1`; the queue
  flush is also wrapped in Z80 bus protection).
- `SYS_disableInts/enableInts` nest correctly; no raw reg-1 write anywhere in
  `src/` (DMA-enable bit), no palette fade in the scene/ending path.
- ROM layout: no walking-scene table straddles a 128 KB boundary (SGDK splits
  those anyway); the sheets/`ending.c` changes were bisected and are not the cause.
- A display-OFF upload (VDP_setEnable(FALSE) + immediate DMA) made it worse
  (one frame per second, no player) and was reverted - that variant is a clue
  that something in the VDP/sprite state is fragile, not a fix.

**Strongest remaining suspect**: heap exhaustion/fragmentation. 45 KB of the
64 KB RAM is static, DMA queue (192 x 16 B) + DMA data buffer (8 KB) sit on the
heap, and `walking_parade()` tears the sprite engine down and rebuilds it
(`SPR_end` -> `POOL_destroy`/`MEM_pack`, `SPR_initEx` -> `POOL_create(MAX_SPRITE,
sizeof(Sprite))` + a 1.5 KB VRAM map). If any `MEM_alloc` returns NULL the sprite
engine (and the DMA queue, if it is ever re-created) is left half-built, which
would also explain the "no player" symptom of the display-off attempt.

**Changes made while investigating** (kept, unverified): the walking scene queues
its tiles/palettes, then flushes them at a vblank BEFORE the long CPU map writes,
and re-queues them at frames 2/8/30 (idempotent, ~5 KB) so a missed transfer
shows for a couple of frames instead of for good.

**Measured 2026-10-29 (diagnostic runs)**: immediate DMA (ROM and RAM source) and the CPU copy
work at every step; the QUEUED flush (SYS_doVBlankProcess, manual DMA_flushQueue, music
paused/stopped) lands nothing from the very start of main() and takes ~5.5 frames - the
music/Z80/scene are not involved. `flushQueue` (src/dma_a.s) reads `dmaQueues` and executes
16-byte entries, so a NULL/bad queue pointer would run garbage commands from address 0.
`main()` reallocates the queue (`DMA_setMaxQueueSize(192)`) with the heap almost full.

**After the cause is known (checklist, prepared in advance)**:
1. fix is in the normal build (`make clean && make`, no MD_* flags): confirm the walking
   scene and one stage start with the log build (`-DMD_WALK_DIAG`) showing `QCHK ... entry=OK`
   and every `PROBE ... queued_rom=OK`;
2. remove the walking scene's scaffolding that only existed to survive this (flush before the
   map writes, re-queue at frames 2/8/30) - or keep it if the fix is not at the root;
3. re-check the other screens that rely on queued DMA and were reported striped (BUG-044:
   intro, CONFIG, room transitions) - they probably shared this cause;
4. a heap-budget line in the boot log of normal builds when the queue is smaller than 192, so it
   can never again be silent.

**Next run** (one launch, log only, `script -qf`, `-DMD_TEST_ENDING -DMD_TILE_DUMP`):
read the `WALK:` lines - heap free/largest, DMA queue size/bytes before the first
flush, `SPR_isInitialized()` and whether the player sprite was created - on a good
and on a glitching boot. A NULL sprite pool or a tiny largest-free-block settles
the heap theory; a full queue (192) settles the overflow one.

## BUG-044 follow-up — CRAM write race: fixed

Every CPU-method CRAM write now goes through `src/pal_safe.h`
(`pal_load`/`pal_load_n`/`pal_color`): wait the DMA out, keep interrupts off for
the whole address-write + data-words sequence, then write. `tools/check_pal_writes.py`
(`make check-pal`) fails if a bare `PAL_set*(.., CPU)` reappears in `src/*.c`.
`showIntro()` also re-asserts PAL0 = `pal_ui` before it draws, so a boot where
CRAM came back shifted can no longer be visible.

Verified with the CRAM readback (log-only run, `-DMD_TILE_DUMP`): intro frame 0
reads `0000 0000 0EEE 0EC8 08AA 048E 0E08 006E 0666 0880 002E 002C 0622 0260
0024 0CCC`, which is `pal_ui` word for word, and it is unchanged 24 frames later.
Before the fix the same dump returned the boot grey ramp shifted by 8 or 10
entries, a different offset per boot.

Why it was needed: a CPU-method palette write is a VDP address-port write
followed by data words, and the boot CRAM readback showed the grey ramp sitting
at entries 8..15 on one boot and 10..15 on the next - the destination slid by a
different amount each time, which is a redirection, not a torn frame. The same
mechanism can drop CRAM data into VRAM at whatever address the VDP last held.

## BUG-044 follow-up — the stripes are NOT the fill tiles (open)

Measured on the emulator with `EXTRA_FLAGS=-DMD_TILE_DUMP` (`src/tdump.c`,
registers + CRAM readback through kprintf, log only, no screenshots):

    intro: reg11=00 (plane scroll, NOT per-column VScroll), reg12=81 (H40),
           reg17=reg18=00 (window plane off), BG_A=E000 BG_B=C000,
           text plane=BG_A, text palette=PAL0
    intro PAL0: 0C00 0E00 x9 then 0000 0222 0444 0666 0888 0AAA   (run 1)
    intro PAL0: 0E00 x8 then 0000 0222 ... 0CCC 0EEE              (run 2)

So CRAM is corrupted at boot and the offset MOVES between runs (ramp at entries
10..15 one boot, 8..15 the next): a palette transfer whose destination address
is redirected mid-way, i.e. the FIND-08 class but in the boot path, before any
of our own palette calls. Making our CPU-method CRAM writes CRAM-safe
(src/pal_safe.h: wait for DMA + interrupts off) did not change it.

Fixed by that change and verified by the reporter: the stripes that ran through
TEXT are gone. Still striped after the change: intro, CONFIG, room transition.
The stage playfield is clean (screenshots/bugs/items_after_menu.png), and both
CONFIG and the transition band point their cells at the shared tile 702 while
the stage playfield points at the stage tileset - so the next measurement must
be the tile data itself.

Attempted and reverted: reading VRAM from the 68k (control-port address +
data-port words) to dump tiles 0/702 - it faults the ROM (address error at
0x318CE right after the first tile read). Do it with the display disabled at a
known point, or with the emulator debugger, not from a live frame.

## BUG-044 — white stripes: the "solid black" fill tiles were stripe patterns

Symptom: vertical white (or lightest-palette) stripes, whole-screen on the
options screen and the opening cutscene, and over the stage's parallax sky and
the room-transition band fills — reported as "white stripes/spread, visible
during the stairs/ladder transitions but broader than that".

Cause: Mega Drive 4bpp tile data holds one byte per **bitplane** in each row, so
a fill tile must be `0xFF000000` per row for a solid palette index 1. Both
hand-written "black" tiles used `0x11111111`, which decodes to pixel index 0xF
(15) on pixels 3 and 7 only — two vertical bars per tile, and index 15 is the
lightest entry of *any* palette (white in PAL0). The comments even said "filled
with palette index 1", which is what the author meant:

    src/scene.c  black_tile[8]  -> cutscene BG_B fill        (stripes)
    src/config.c cfg_black[8]   -> options screen BG_A fill  (stripes)

They spread because config loads its fill at `SPR_CFG_BLACK_TILE` (702), which
is the same index `stage_sky_tile()` returns: after a visit to the options
screen (title CONFIG or the in-game menu) every parallax row the bg art does not
cover, and every band the transition writes, pointed at an opaque striped tile
until the next map load. `bg_upload_tiles_maps()` only reloaded the blank sky
tile when the map had parallax art, and only on a map load.

Reproduction (offline, fails before the fix): `tools/check_fill_tiles.py`
(`make check-fills`) decodes every `static const u32 NAME[8]` tile literal in
`src/*.c` and requires one value on every pixel. Pre-fix output:

    config.c: cfg_black is not a solid fill
        0 0 0 F 0 0 0 F   ... (8 rows)
    scene.c: black_tile is not a solid fill
    2 fill tile(s) are accidental patterns (BUG-044)        exit 1

Fix: both constants are now `{0xFF000000, x8}`; `config.c:clear_screen()` sets
`PAL_setColor(1, 0x0000)` because the screen inherits PAL0 from whoever opened
it (title line 1 is white, the stage line 1 is a stage colour); and
`stage.c:bg_load()` reloads the blank sky tile at 702 on EVERY map load,
before the "no parallax art" early return.

Evidence caveat: the frames in `screenshots/bugs/stripes/` are 642x471 and the
ones in `screenshots/bugs/transition/` 644x476 — `tools/check_captures.py`
rejects both batches as non-game window sizes, so they are not used as proof
here; the tile decode above is. Stripes still unexplained by this bug: the
full-screen ones behind the intro logos (001/002) and the player-select band
(003), whose cells point at tiles 16..48, not 702.

Emulator follow-up (one `-e` event-log run, boot -> intro -> cutscene, no
screenshots, decoded with `tools/evtlog_planes.py`):

- the backdrop entry CRAM[0] goes 0x0000 -> 0xE307 -> 0xB6C0 (title navy) and
  is never 0x0EEE, so the intro's white field cannot be the backdrop colour;
- the planes' final state is the cutscene's blank BG_A (every cell tile 0 =
  transparent) over a filled BG_B, i.e. nothing striped;
- 196 CRAM writes do land inside visible scanlines (the illegal-active-display
  class from FIND-08) - still present, see `scene.c:86`/`stage.c` CPU palette
  loads, worth its own fix;
- the player-select map is in range (1120 cells, max index 32 of 33 tiles), so
  its cells cannot reach tile 702 either.

Conclusion: BUG-044 explains the options screen, the stage parallax sky after a
visit to it, and the transition band fills. The intro/player-select frames
stay unexplained *and* are geometry-rejected, so nothing in the ROM's VDP state
found offline (plane cells, backdrop) produces them.


## Transition capture, 2026-10-25 — the slide is clean

Captured with a **screenshot burst** (`tools/emu.sh emu_burst`, ~25 ms/frame,
geometry-validated) while the player was sent down the CHAOS CITY ladder that
links map 0 -> map 1 (`LINK map 0->1 trans 0 dst(198,0) px 3163 py 0 cam 2880`).
24/24 frames usable, no video involved.

The seam (row where the incoming screen takes over) advances 16 -> 22 -> 30 ->
32 -> 36 -> 37 -> 59 -> 69 and is gone by frame 22: that is the whole slide,
captured. Inspecting +/-20 rows around the seam in frames tr05/tr09/tr13/tr19:
both halves match their own rooms, no white bands, no garbage tiles.

Two things that earlier looked like artifacts and are not:
- the striped block at the left edge (x = 1..8, y = 12..60) is the HUD gauge;
- a burst recorded as a 1182x494 video frame was a resized window, not the game
  (`tools/check_captures.py` now rejects those; `emu_shot`/`emu_burst` drop them).

## Batch 2026-10-25 — emulator verification (silent test builds)

Test builds now run with audio off (`config_force_silent()`, applied when
`MD_NO_AUDIO`/`MD_PROFILE`/`MD_TEST_STAGE`/`MD_LINK_TRACE` is defined), so the
numbers below have no Z80/PCM noise.

- **Room 2 of the intro stage (stage 0 map 1, 21 objects) — fixed, measured.**
  `objupd` 308…418 and total busy **1037…1219** subticks of 1280 with `wait`
  400-450: the loop holds 60 fps. Before this batch the same room sat at ~1230
  busy / 40 fps. One 6801-subtick spike per session remains and lines up with
  the boss teleporter's `stage_teleport_boss`, not with per-frame work.
  `hot=` reported the fall platforms (type 10) as the most expensive single
  object while the player rides one, and pills at ~35.
- **Stairs half-body pose — verified by trace.** `MD_LADDER_TRACE` on the
  CHAOS CITY ladder printed, over 57 samples of climbing, exactly one
  `frame=1`: `mid_stair=0 feet_stair=1 y=136` (leaving the top rung), and
  `frame=0` while grabbing and during the whole climb — the PC condition
  (character.cpp:396/426) and nothing else.
- **Dropped items fall — verified by host test.** `tools/run_host_tests.sh`
  spawns a drop in mid-air and asserts the gravity flag and the descent:
  `y 35 -> 115, flags 0x0E (falls)`.
- **BUG-033 (SEAHORSE half speed)** — see its entry: 642 of 1280 subticks, the
  animated-tile bucket down from 1135 to 61.
- Menu (in-game): weapon grid, lives, ammo, ARMOR and ITEMS lines all render;
  the ITEMS line was too wide for the panel and is shortened.
- Graphics: no white stripes or palette damage in the Seahorse or CHAOS CITY
  captures taken during these runs.

## Batch 2026-10-24 (c) — the rest of the fifteen reports

Fixed, each with its offline reproduction:

- **Room transition drew the current room's parallax behind the incoming
  screen**, and **applied the outgoing horizontal offset to the incoming half
  in upward (B2T) changes** — the 1..15px "unaligned" jump. Both planes now take
  their source row from the same tilemap index (`base + y/8`), so the answer
  cannot disagree with what the screen shows, and the destination room's bg
  tiles are copied into the transition's spare VRAM and filled in as the split
  claims them (sky instead when the two bg palettes differ, because both halves
  share PAL3). `check_rules.py` transition-row-split.
- **GAMBIT fell through the floor.** `AI_TELEPORT` ignored its option and placed
  the enemy at `player.y - 16`, i.e. inside the floor. Now the PC's options
  (`ia_action_teleport`): slide to the first wall within `walk_range`, Y never
  changes. `check_rules.py` teleport-options.
- **ANGRY BUNNY hopped in place.** A spawned NPC inherited the spawner's patrol
  span, so every AHEAD move clamped back. A child is a fresh NPC on the PC and
  now gets a span around its own spawn point (its own `range`, clamped to the
  map). `check_rules.py` spawn-own-span.
- **"The old bug returned" on the stage-5 second ladder (death / no return).**
  It really had: BUG-005's arrival nudge lost its caller when the ladder-scan
  arrival landed, so `stage_link_band_hit()` was dead code and a top/bottom
  stair arrival stayed inside the return link's band - the latch never clears
  and the room cannot be left the way it was entered. Restored (8px steps along
  the ladder, max four). `check_rules.py` arrival-band-nudge.
- **BOUNCING projectile (23)** implemented (gravity + floor/wall bounce);
  `check_trajectories.py` now only lists CHAIN, BOMB_RAIN and LARGE_BEAM
  (stage 13 only) as known fallbacks.

New tool: `tools/preview_got_weapon.py` renders the stage-clear screen from the
generated art. It shows the text at tile column 9 (72px) and the player sprite
at x=20 **do not overlap**, so the "text over the player" report needs the
player's screenshot to identify which text is meant.

One trace-build emulator run (stage 5, `MD_LINK_TRACE` + `MD_TEST_TILE`, start
on the map-1 ladder) captured:
`LINK map 1->0 trans 0 dst(105,0) stair 105 px 1675 py 0 cam 1389 latch_off 8`
- the arrival numbers match `tools/audit_links.py` exactly, and the room change
  ran with no crash. The follow-up climb (down, up, down) fired **no further
  link** and left the player ~40px left of the ladder, which is the next thing
  to look at: either the nudge or the ladder grab is releasing him after the
  arrival.

## Batch 2026-10-24 (b) — fifteen play-test reports, offline reproductions

Standing rule from here on (AGENTS.md): a bug is reproduced by an offline
check **before** the fix, and the same check proves the fix.

Fixed (each with its check):

- **HUD showed no gauge at 1 HP** — `set_bar()` clamps a non-zero percentage to
  `HUD_BAR_MIN_FRAME` (3), the first frame that shows a whole segment.
  `tools/check_hud_bars.py`.
- **Boss door re-opened when walking left into it** — the PC only starts a door
  once (`is_started()`), inside the player's vertical span. The port never set
  `started`, so the whole sequence (and the screen move) ran again.
- **Door stayed at its old screen position during a room scroll** — a door is a
  stack of slice sprites and `objects_sync_sprites()` only moved `o->spr`.
- **Knockback pushed the player inside a door** (stuck off-screen) — the hit
  push applied the terrain column rule only; it now also consults
  `objects_block_x()`.
- **Slide passed a horizontal scroll lock** — `player_check_scroll_lock()`
  returned early while sliding and probed a fixed 2 px ahead. It now runs while
  sliding and sweeps every column the leading edge crossed.
- **WOOD SHIELDER walked through the destructible block** — enemy movement
  (`step_axis`) stopped only at terrain; it now stops at solid map objects
  (`objects_block_box`), like the player.
- **Charged shot destroyed the destructible block** — the PC
  (`classplayer.cpp:800 object_is_affected_by_projectile`) lets *only an
  exploding bomb* break `OBJ_DESTRUCTIBLE_WALL` (and no RockDroid1 projectile
  sets the `is_explosive` data flag), so the block types 36/37/42 are the ones
  any shot may break. `objects_try_hit()` takes a `bomb` flag; a bomb's
  explosion passes it.
- **Wall shooter fired a straight normal shot** — the converters read the stale
  30-record `game_projectile_list.dat` at the data root instead of
  `data/game_projectile_list_v3.dat` (36 records, what the PC loads), and
  `PROJ_MAX` was 23; the data asks for ids up to 34 (SMALL MISSILE EXACT,
  `TRAJECTORY_TARGET_EXACT`). `eshot_fire()` clamped those to projectile 0.
  Fixed: v3 table, `PROJ_MAX 35`, `TRAJ_TARGET_EXACT` aiming, 128 px projectile
  art guard (CHAIN is 70x7 and used to be dropped entirely).
  `tools/check_projectiles.py` (was 13 bad references), `tools/check_trajectories.py`.
- **Children of a left-facing spawner faced right** — `ai_spawn_npc()` passed the
  signed runtime dir (-1) where `enemy_add()` wants the data byte (0 = left).
- **Airborne hops 20% fast and through walls** — the jump moved `d->speed` per
  60 Hz frame instead of the shared 50 Hz stepper; it now uses `step_axis()`.

Open (next session, see HANDOFF.md): ladder arrivals (stage 5 + APE), the room
transition drawing the current room's background behind the incoming screen, the
death when climbing back down the stage-5 second ladder, GAMBIT falling through
the floor after two cart throws, the ANGRY BUNNY hopping in place, and the
"YOU GOT" screen text over the player. Known trajectory fallbacks: CHAIN,
BOUNCING, BOMB_RAIN, LARGE_BEAM, LASER (stages 7/10/13 only).

## Batch 2026-10-24 — six reports (charge, shots, enemy speed, door, shooter)

1. **Charged shot must be consumed by the enemy it hits.** It pierced for
   `level >= 2` (a comment claimed the PC's full charge is permanent). Now every
   charged shot dies on the first enemy (or destructible wall). Needs a
   play-test.
2. **No full-charge indication.** PAL2 cannot spare another highlight (the HUD
   bar owns 2/3/4), so the existing two-step armour flash now strobes at
   **2 frames** instead of 8 once `charge_frames >= CHARGE_FULL_FRAMES`
   (132 = the PC's 2.2 s). Verified in-game: the full-charge orb fires.
3. **Wall shooter must aim at the player.** The port fired straight along the
   wall. `eshot_fire_aimed()` now sends the missile toward the player (full
   speed on the dominant axis, proportional on the other; the PC only knows
   45-degree steps). The art stays upright — no rotation on the MD — which is
   the accepted part of the report.
4. **Enemies ~30% faster than the PC.** `enemy.c` now scales every enemy
   movement through `enemy_step()` = 70% of the old rate (AI steps, wall
   walking, the auto-hop). Start point for the report's "slow them down a bit".
5. **Boss door animation.** The old "slide the door down" was wrong (the PC
   paints the door on the map layer and opens it from the CENTRE outward) and
   glitchy. `res/resources.res` now declares the door art as 16x16 slices (one
   animation per slice) and `object.c` draws it as 3/4 slice sprites, hiding
   them centre-out on open and showing them borders-in on close. Collision is
   unchanged (solid until opened).

**Status**: code complete, needs a play-test of all five (door: reach a boss
room; shooter: find a wall-walker).

# Known bugs / deviations — RockBot → Mega Drive

Things that are wrong or worse than the original. Track here, fix later.

## BUG-043 — ADDRESS ERROR after a slide in the APE stage

**Report (2026-10-23, `crash_slide.png`)**: sliding in APE crashed to the MD
ADDRESS ERROR screen.

**Evidence** (from the dump + `out/release/symbol.txt`): fault address
`0x2D302F` (odd), PC `0x2D70C` = `SPR_update+1004`. The disassembly there is

```
2d4ee: moveal %a2@(16),%a4     ; a4 = sprite->frame
2d4fa: bnew   2d704
2d704: moveal %a4@(2),%a5      ; a5 = frame->tileset
2d708: movew  %a5@(2),%d0      ; <-- faults: a5+2 = 0x302D302F -> 0x2D302F
```

with `a4 = 0x02400EEE` and `a5 = 0x302D302D` in the dump: `Sprite.frame` is
garbage, i.e. **an out-of-range animation/frame index** was accepted earlier.
SGDK only validates those inside `#if (LIB_LOG_LEVEL >= LOG_LEVEL_ERROR)`,
and a release build sets `LIB_LOG_LEVEL` to DISABLE (`DEBUG == 0` in
`config.h`), so the bad index reaches `SPR_update`, which then reads
`animation->frames[i]` past the end and dereferences whatever is there. Same
class as BUG-031 (HUD ball frame index).

**Ruled out offline**: enemy `jump`/`teleport`/`intro`/`turn` animation rows
(`audit_data.py` now checks `turn` too: "animation index problems: 0" for both
games), the player's animation set (highest index 13 of 14 animations),
HUD bars/icons, effect frames (`% frames`), object frames (`def->frames`).

**Fix (defensive, 2026-10-23)**: `src/sprite_guard.{c,h}` wraps every
`SPR_setFrame`/`SPR_setAnim` call in the port: it validates against
`definition->numAnimation` / `animation->numFrame`, rejects instead of passing
the index on, and prints the offender over KDEBUG (max 12 lines):

```
sprite guard: animation out of range (index %d, %u valid, def %p)
sprite guard: frame index %d >= %u
```

The crash class is therefore gone (no more garbage frame pointers); the next
reproduction names the call site's art so the *real* index bug can be fixed.

**ROOT CAUSE found 2026-10-24** (the guard paid off): a logged run in the APE
stage printed, nine times,

```
sprite guard: animation out of range (index 14, 14 valid, def 00034030)
```

and `0x34030` is `player_p1_sprite`. ROCKBOT's sprite got **14 animations**
while the other three players got 15: rescomp dropped the trailing
`STAIRS_SEMI` animation for player 1 only, so `SPR_setAnim(ANIM_STAIRS_SEMI)`
(= 14) was out of range and `SPR_update` read a garbage frame pointer - the
ADDRESS ERROR in `crash_slide.png`. The trigger was grabbing/leaving a ladder
(12-frame half-body pose), not the slide itself.

**Fix**: the half-body art (sheet frame 18) is now the **second frame of the
STAIRS animation** (`tools/convert_player.py`: `("STAIRS", [17, 18], [0, 0])`,
15th animation removed, `ANIM_STAIRS_SEMI` deleted from player.h) and
`player.c` selects frame 1 for the semi pose. All four players now ship 14
animations with the same layout, and the guard reports nothing.

**Status**: fixed 2026-10-24, needs one play-test on a ladder (grab and leave
should still show the half-body pose, and nothing may log).

## BUG-042 — Tooltip box interior and name background showed the parallax

**Report (2026-10-23, first play-test of BUG-041)**: the box, the icon and the
name "APE QUAKE" all appeared and the HUD stayed intact, but the box's interior
band showed the street/parallax instead of a solid fill, and the name had
coloured speckles around the glyphs.

**Cause**: the PC art's interior colour is index 0 of the box palette, and the
SGDK font's glyph background is index 0 as well — but on the Mega Drive a
**BG pixel with palette index 0 is transparent** (the other plane, then the
backdrop, shows through). So every interior cell and every glyph cell let
`BG_B` (the parallax) show. `VDP_setTextPalette(PAL3)` cannot help: index 0 is
transparent regardless of what CRAM holds there. Confirmed offline by decoding
the emitted tiles: the interior tile is `00000000` on every row, and the ROM's
`hud_bar_*`/`tooltip_frame_*` sheets render fine, so the art and the converter
were correct.

**Fix**: `tools/convert_tooltip.py` appends one **solid index-1 tile**
(`TOOLTIP_FILL_TILE`) to the box tiles, and `tooltip.c` backs the whole box with
it on `BG_B` (`back_fill_draw()`) before drawing the opaque parts on `BG_A`
(borders, icon ink, glyph ink). Expense: the box now also claims `BG_B` cells
for its 1.5 s, restored by `stage_redraw_bg()` at expiry (next to the existing
`stage_force_redraw()`). VRAM block grew by one tile: tooltip 672..684 (13),
dialog 685..701, config 702, sprite pool 703..1439 (`SPRITE_VRAM_TILES 737`);
`check_vram` still reports 0 transition fallbacks.

**Status**: fixed, needs one play-test (same check as BUG-041: solid dark box,
clean white name, HUD intact, parallax back afterwards).

## BUG-041 — Weapon tooltip never drawn, and it corrupted the HUD

**Report (2026-10-23)**: after picking a weapon in the START menu the tooltip
never appeared (six screenshots at 0.2 s intervals, none of them with the box's
light-blue border anywhere on screen), and the in-game HUD then showed
multicoloured stripes instead of the two gauges.

**Cause 1 — the tooltip was painted over.** `menu_open()` closing calls
`stage_force_redraw()` → `MAP_scrollToEx(..., TRUE)`. SGDK implements the row
and column writes through `DMA_allocateTemp` (`srclib/src/map.c`
`setMapRow/setMapColumn`), so the whole visible plane is *queued* and lands at
the next vblank — after `tooltip_update()` had written the box and the name.
The port already knew this (the start sequence flushes before drawing
"MISSION STARTED!"), the tooltip did not.

**Cause 2 — the HUD garble.** `TOOLTIP_TILE_BASE` was 912, "the free window the
dialog panel uses". That window is inside the **sprite pool**: `SPR_initEx`
puts it at `TILE_FONT_INDEX - size` = 672..1439, and by the time the menu had
been used the stage's enemies/objects had taken the low tiles, so the HUD bar
sprites owned 912..923. The tooltip's `VDP_loadTileData(.., 912, 8)` then wrote
the box art over the bars' tiles; the bars only re-upload on a frame change, so
they kept showing that art (the weapon icon's orange/white plus the box
colours). Same latent bug for the boss dialog panel (912..928) and the config
screen's black tile (935), both reachable while sprites are live.

**Fix**:
1. `tooltip_update()` flushes the DMA queue (`SYS_doVBlankProcess()`) before
   writing its cells, so the menu's queued repaint happens first.
2. Fixed tile blocks moved out of the sprite pool into a reserved block below
   it (`src/stage.h` owns the layout): 672..683 tooltip (12), 684..700 dialog
   panel (17), 701 config black tile (1); the pool is
   `SPRITE_VRAM_TILES 738` = 702..1439. `tools/check_vram.py` uses the same
   number, and still reports **0 transition fallbacks** (the tooltip redraws
   its own tiles on every show, so a transition window sliding over them is
   harmless).

**Status**: fixed, needs one play-test (menu → change weapon → box + icon +
name for 1.5 s, HUD intact, map restored afterwards).

## BUG-040 — Crash entering a stage from NEW GAME (out of heap, NULL map)

**Report (2026-10-23)**: `crash_new_game.png` — ADDRESS ERROR, fault PC inside
`setMapRow` (`out/release/symbol.txt`), odd fault address `0xF90003`. A second
reproduction gave LINE 1111 EMULATOR with the internal PC holding `4EF90002`
(a jump through garbage). Both die while drawing the stage map.

**Cause**: `stage_load()` calls `MAP_create()`, which allocated
`sizeof(Map) + compressed blocks`. `sizeof(Map)` alone is ~580 B because the
map object carries `hScrollTable[240]` + `vScrollTable[20]`; the stage MAP
resources were compressed (`FAST`), so a 45-block map needed another 2880 B.
Measured over KDEBUG at that point in the boot:

```
SL: enter map=0 / SL: tileset loaded
SL: MEM free=4554 largest=3272 alloc=16990
SL: MAP_create FAILED free=4554 largest=3272
```

`free` was enough (4554) but the *largest contiguous block* (3272) was not
(3460 needed): the heap is fragmented by the earlier screens. `stage_load()`
`return`ed with `stage_map == NULL`, the caller ignored it and kept drawing the
stage → `setMapRow` through a NULL/garbage map. Earlier RAM growth (objects /
AI / tooltip state, ~230 B of BSS) had eaten the last few hundred bytes of a
margin that was already negative for fragmented heaps.

**Fix** (2026-10-23):
1. `MAP_create()` now only needs `sizeof(Map)`: the stage MAP resources are
   `NONE` instead of `FAST` (`res/resources.res`, `tools/add_stage.py`), the
   same trick the tilesets already used. Blocks then point straight into ROM
   (`blocksSize = 0`). RAM per map drops ~2880 B; the room-transition
   `MAP_create` (BUG-023) gets the same margin. Cost: ~78 KB of ROM, which
   puts the image at 2.09 MB — the next ~2 KB of code pads it to 2.25 MB.
2. `stage_map_ready()`: `runStage()` returns to the title instead of entering
   the stage loop with a NULL map, and `stage_force_redraw()` /
   `stage_update_camera()` / the autoscroll path no longer call `MAP_*` with a
   NULL map. A failure now logs one KDEBUG line instead of crashing.

**Status**: fixed, needs one play-test (NEW GAME → stage 0).

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

**Status**: **FIXED** — the panel landed in BUG-028 (PC `dialog.png` through
`tools/img2tiles.py`, drawn on BG_A in PAL3); the 2026-10-22 play-test report
"dialog text background / A arrow" is BUG-037.

## BUG-037 — Dialog text background black, A-button arrow a black box

**Report (2026-10-22 play test)**: the panel shows, but the three text lines sit
on a black band and the A-button icon is a black square
(`screenshots/bugs/playtest/boss_dialog_text_bg.png`).

**Cause**: two separate palette mistakes. SGDK's font paints a whole 8x8 cell
per character, background = palette index 0 and ink = index 15, so drawing the
lines with the default PAL0 painted black cells over the panel. And the 12x12
`btn_a.png` goes through `img2tiles.py`, which re-quantizes even an indexed
source: its white landed on index 1, which in PAL3 is the panel's black border.

**Fix**: `dialogs.c` builds PAL3 with entry 0 = the panel fill
(`dlg_panel_pal.data[2]`, the dark teal) and entry 15 = font white, and the
pages are written with `VDP_setTextPalette(PAL3)`; the button is a font "A"
glyph in the same palette (the icon art and its build steps are gone).

## BUG-009 — Rockbot has no eyes in boss dialogs

**Status**: **FIXED and verified 2026-10-21**. The face art's white eye sockets
are palette index 0, i.e. sprite-transparent, and the pupils live in the
separate `ssel_eyes_pN` layer that only the stage select drew. `dialogs.c` now
adds that eyes sprite (offset +10,+18, same as the PC's face (144,104) /
eyes (154,122)) behind the face on the player's pages only. Verified in a
forced boss-room dialog: the portrait now has eyes.

## BUG-010 — Mummy keeps facing away from the player

**Status**: **FIXED 2026-10-22** (play-test pending). Two things:
1. `AI_TELEPORT` now lands facing the player (`e->dir` from the new position),
   and
2. the teleport/jump animation rows exist (BUG-011), so the boss no longer
   keeps the old art while he repositions.
The old analysis stands: action 5 (SHOT_AHEAD) does not turn, as on the PC.
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

**Status**: **FIXED 2026-10-22** (play-test pending), including the boss
`ANIM_TYPE_INTRO` row (file anim 16) played after the landing drop. The
converter now ships
the `ANIM_TYPE_JUMP` (file anim 2) and `ANIM_TYPE_TELEPORT` (anim 4) rows, but
ONLY for enemies whose AI uses those actions (action 2 / 10): the extra row
costs sprite VRAM, and crowded stages are already near the 504-tile pool. The
row index is stored per def (`jump_anim` / `teleport_anim`, 255 = none) because
the row order is stand, walk, attack, [jump], [teleport]. `enemy.c` picks the
row while the action runs, and `convert_enemies.py` now rewrites the matching
`SPRITE` lines in `res/resources.res` (they must list one animation per row).

## BUG-012 — Projectiles not cleared when the boss dies

**Status**: **FIXED 2026-10-22** (play-test pending). `eshots_reset()` dropped
the sprite HANDLES without releasing the sprites, so every enemy projectile
stayed allocated and frozen on screen (and the sprite VRAM leaked for the rest
of the stage). It now calls `SPR_releaseSprite()` first, exactly like `kill()`.

## BUG-013 — The "YOU GOT ..." screen differs from the PC

**Status**: **FIXED 2026-10-22** (play-test pending). The port now follows
`game::classic_style_got_weapon()` (game.cpp:1347): white blink, CONFIG_BGCOLOR
(5,29,65), player at x=20, and the text typed out (`draw_progressive_text`)
at rows 14/16/17. The second line names the ITEM of the COIL/JET stage
(E-TANK / W-TANK, the PC's `game_data.player_items` names uppercased) - it used
to print weapons 9/10, which are not items at all. Total wait stays 5000 ms,
typing included.

## BUG-014 — Stage select shows garbage text after beating Mummy

**Status**: **FIXED 2026-10-22** (play-test pending).

**Cause**: the name lines were drawn with the *global* PAL0. The font paints
its cell background with palette index 0 (= CRAM 0, the backdrop, which the
stage writes per map via `apply_bg_color` and the YOU GOT screen overwrites
with its colour flash), so the names came out in that leftover colour on the
same colour (peach on peach in the report) and unreadable.

**Fix** (`stageselect.c`): `draw_names()` sets the text plane explicitly and
draws with **PAL1** (the grey ramp already loaded for the beaten faces: 0 =
black, 15 = light grey), restoring PAL0 afterwards; `stage_select_run()` also
re-uploads the default font (`VDP_loadDefaultFont(CPU)`) so a previous screen
that wrote past the user tile area cannot leave the glyphs garbled.
`tools/check_palettes.py` asserts the 0/15 palette entries.

## BUG-035 — BETABOT colours wrong / HP bar colours wrong

**Report (2026-10-22 play test)**: BETABOT's body is red instead of black/grey
(`screenshots/bugs/playtest/betabot_colors.png`), and the HP bar shows the
player's weapon colours instead of the PC's fixed magenta/blue.

**Cause**: `player.c::rebuild_player_palette()` wrote the classic HUD bar
colours into PAL2 entries 8/9 (and 12) — but PAL2 is the *player* palette and
the sheets use every entry: BETABOT's entire black body is index 8, CANDYBOT's
armour index 8/9/12, KITTYBOT's index 9. The HP bar silently recoloured the
sprite (ROCKBOT was unaffected because his sheet leaves 8/9 unused, which is
why only some players looked wrong).

**Fix**: the HP bar moved to **PAL0** with the PC's fixed colours
(`draw_energy_bar` always uses PLAYER 1's weapon-0 colours: PAL0 6 purple +
3 light blue + 1 black), the weapon bar keeps PAL2 13/14 (the art's own swap
placeholders) with a transparent empty part and a black PAL0 backdrop sprite,
and `rebuild_player_palette()` no longer touches 8/9/12.

## BUG-036 — Explosions do not move away from the centre

**Report (2026-10-22 play test)**: the death explosion stays as one centred
puff.

**Cause**: two things. The port froze the burst: the player-death branch never
called `explosion_update()`, so the puffs sat on frame 0 and were never
released; and it burned three puffs in a horizontal row instead of the PC's
outward pattern.

**Fix** (`main.c`): the death branch now advances the puffs and spawns the PC
pattern (`game.cpp:1707/1734`): the centre puff repeats over the 90-frame death
window (0/30/60) and four more spawn at ±32 px diagonals at 18/36/54/72 frames.

**Boss half (same report)**: the port played a pseudo-random scatter of puffs for
2 s. The PC (`game::classic_style_draw_boss_death`, game.cpp:1798) sends **eight**
explosions outward from the boss (right/left/up/down + the four diagonals), the
radius growing until it leaves the screen. `explosion.c` now has
`explosion_ring_start/update` doing exactly that with 8 sprites using the PC art
`classic_style_death_explosion.png` (5 frames of 24x24, emitted as
`res/death_explosion.png` by `tools/convert_assets.py`), PAL0, 3 px/frame so the
speed matches the PC's ~200 px/s. `runStage` starts the ring instead of the
scatter and holds the stage until it finishes (~108 frames).

## BUG-026 — Checkpoint shows no animation when activated

**Status**: **FIXED 2026-10-22** (play-test pending).

**Cause**: the art, not the object logic. `checkpoint.png` is a terminal whose
two frames are "screen off" (dark green) and "CHECK POINT" (bright green + red
LEDs). PAL0 has only ONE green, so both greens quantized to index 13 and the
two frames ended up 6 pixels apart — the loop (`anim_loop = 1`, 300ms/frame)
was running all along and could not be seen. `object.c` itself was correct.

**Fix**: `tools/import_pc_art.py` pins the terminal's colours apart
(`CHECKPOINT_OVERRIDES`: screen background -> PAL0 black, text -> white, LEDs
-> red) and the hand art was rebuilt; the frames now differ by 130 px.
`tools/check_palettes.py` guards the pair.

## BUG-015 — Missing boss intro for Seahorse (maybe global)

**Status**: **FIXED 2026-10-22.** The intro was not skipped - it was skipped
*instantly*. `boss_intro_run()` skipped on any button of the pad, and the stage
select returns on a START/A/C press, so the player is still holding that button
on the first intro frame: the loop broke immediately, the screen flashed the
blue backdrop for one frame and only the jingle was heard. A forced stage-3 run
from boot looked fine because no button was held.

Proof offline: the live BlastEm SRAM (`~/.local/share/blastem/rom/save.sram`)
holds the save the user played - slot 0, `stages = 0x0001` (only CHAOS CITY
cleared), 5 lives - so the gate `!mask & (1 << 3)` was TRUE and the intro did
run.

**Fix** (`bossintro.c`): the skip is gone entirely. The PC's
`classic_style_boss_intro` ends with `graphLib.wait_and_update_screen(6400)`,
which reads no input: the intro is a fixed 6.4 s hold and is NOT skippable. The
port had added a "press any button to skip" that the PC does not have.

## BUG-038 — Port-only input skips (PC sequences that cannot be skipped)

**Report**: "it should not be skippable. I believe the PC version has nothing
like that."

Same class as BUG-015 - the port had invented skips the PC does not have.
Audited every `JOY_readJoypad` in a non-menu loop against the PC source:

| port site | PC reference | verdict |
|---|---|---|
| `bossintro.c` boss intro | `wait_and_update_screen(6400)`: no input | skip REMOVED |
| `main.c` `hold_frames()` (stage clear: victory flash, 3.5 s pose, YOU GOT 5 s) | `player_victory()` delays only, `classic_style_got_weapon()` ends with `wait_and_update_screen(5000)` | skip REMOVED |
| `ending.c` walking parade | `show_player_walking_ending()`: `while` on the player x, `timer.delay`, no input | skip REMOVED |
| `ending.c` boss parade + credits pages | `show_enemies_ending()`: 2000/3000 ms delays; credits scroll on a timer | skip REMOVED |
| `ending.c` final "press" | `draw::show_credits(can_leave == false)` waits for JUMP | kept |
| `scene.c` cutscenes | `sceneshow.cpp:59` aborts on START | kept (PC-faithful) |
| `main.c` GAME OVER | `game_over()` -> `input.wait_keypress()` | kept |
| `dialogs.c` page advance | `show_dialog()` -> `input.wait_keypress()` | kept |

Also fixed while auditing: `CASTLE_FRAMES` was 180 (3 s) with a comment claiming
the PC uses 3000 ms; `classic_style_castle_intro` actually holds **6500 ms**, so
it is 390 frames now.

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

**Status**: **FIXED** (confirmed by the 2026-10-22 play test — "fish staying in
water" is in the do-not-regress list). First attempt compared the position with
itself; see the analysis further down for the final fix (pre-move probe, PC
`character.cpp:1921-1923`).

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

## BUG-020 — No boss door in the DR. DESTRIN room (objects beyond 24 dropped)

**Status**: **FIXED 2026-10-21** (in-game re-check pending).

**Report**: the DR. DESTRIN room had no door; the same stage also seemed to be
missing objects.

**Cause**: `OBJECT_MAX` was 24, but stage 13 map 0 places **27** objects and the
boss door is the last one. `objects_reset` stops at `obj_count < OBJECT_MAX`,
so the door (and the two objects before it) never spawned. The converter was
not dropping anything: it reports 0 skipped spawns for every stage 0..13.
`ENEMY_MAX` had the same problem — stage 10 map 2 spawns 27 enemies and 11 were
dropped.

**Fix**: `OBJECT_MAX` 24 -> 32, `ENEMY_MAX` 16 -> 32 (max per map across the
converted stages is 27 for both).

## BUG-021 — MAGE: enemy projectile is not flipped when fired left

**Report (2026-10-22)**: GAMBIT's card flies left but is drawn facing right.

**Status**: **FIXED 2026-10-22** (play-test pending). `eshot.c` only set
`SPR_setHFlip` when a shot bounced; the spawn and the target-direction paths
left the art facing right. `eshot_flip()` now mirrors every enemy shot from its
travel direction (`dx < 0`), at fire time, after the target-direction resolve
and on a bounce.

## BUG-022 — SEAHORSE: wrong/incomplete tileset at stage start

**Report**: starting the SEAHORSE BOT stage shows a wrong or incomplete
tileset.

**Status**: **FIXED / confirmed by the 2026-10-22 play test** ("SEAHORSE
tileset/fade/corruption after links" is listed under "do not regress" in
HANDOFF.md). The per-map tileset/palette reload and the colourcycle band are
in `stage_load()`.

## BUG-023 — SEAHORSE: every link transition has a fade

**Report**: every room link in the SEAHORSE stage shows a fade; suspects it
started after playing MAGE BOT first.

**Status**: **FIXED / confirmed by the 2026-10-22 play test** (same group as
BUG-022).

## BUG-024 — SEAHORSE: tileset corrupt/incomplete after the second link

**Report**: after leaving through the second link the tileset is corrupted or
incomplete.

**Status**: **FIXED / confirmed by the 2026-10-22 play test** (same group as
BUG-022).

## BUG-025 — Sub-boss door scroll is too fast and leaves the player on the right

**Status**: **FIXED 2026-10-22** (play-test pending). Port now follows PC
`game::horizontal_screen_move` (game.cpp:1174): 4 px steps
(`TRANSITION_STEP`), 76 of them, and the player is pushed only ~2.5 tiles
(40 px) in the same direction, so he ends near the LEFT edge of the new screen.
Two steps per frame (the PC runs one every ~2 ms), and the door handler no
longer carries him the full screen.

**Remaining half (2026-10-22)**: the door sprite stayed glued to the screen
while the stage slid under it — the scroll moved the camera and the player but
never re-placed the objects/enemies (their sprites are positioned from world
coordinates only in the normal frame loop). `stage_horizontal_screen_move()`
now calls `enemies_sync_sprites()` and `objects_sync_sprites()` after every
step, so the door keeps its world position.

## BUG-026 — Checkpoint shows no animation when activated

**Status**: superseded by the entry above (the object flags were fine; the two
art frames quantized to the same colours).

## BUG-027 — Slide speed equals walking speed

**Status**: **FIXED 2026-10-22** (play-test pending). The slide/dash now use
`SLIDE_SPEED` = 2.5x walk, matching PC `character::slide`
(character.cpp:1724: `max_speed = move_speed * 2.5`). Note: the report said
1.5x, the PC source says 2.5x - easy to change if it still feels wrong.

## BUG-028 — Boss dialog background still missing

**Status**: **IMPLEMENTED 2026-10-22** (play-test pending). The PC panel
(`images/backgrounds/dialog.png`, 310x65) and its A-button icon (`btn_a.png`)
are converted by `tools/img2tiles.py` (`dlg_panel`, `dlg_btn_a`, wired into the
Makefile) and drawn on BG_A at the PC position (panel rows 0..8, button at
column 34/row 5) in PAL3 with their own palette, replacing the old black box
and the "JUMP: NEXT" text. The stage colourcycle band is re-applied through
`stage_restore_cycle()` when the dialog closes (PAL3 is the cycle line in
stages 1/2/3/6/8).

**Follow-up (2026-10-22)**: the art is 310x65, i.e. 38.75 x 8.125 tiles, so
`img2tiles` padded the panel to 39x9 with TRANSPARENT cells: the last 2 px of
the right column and the whole 9th row were see-through, and the stage showed
through them (the vertical strip of water at the right edge of the panel in
`screenshots/bugs/boss_dialog.png`). The source art is now padded to 320x72
with its own black border (`images/backgrounds/dialog.png`), which is exactly
40x9 tiles: every cell of the panel is opaque.

## BUG-029 — "YOU GOT" screen (PC stage-clear sequence)

**Status**: **IMPLEMENTED 2026-10-22** (play-test pending). `showStageClear()`
now follows the PC:

- `game::player_victory` (game.cpp:1276): music stops, explosion SFX, the
  player takes the GOT_WEAPON pose (new animation row 26/27 in the player
  sheet), the screen flashes the PC's four colours twice (300ms each) and then
  the stage is shown for 3.5s;
- `game::classic_style_got_weapon` (game.cpp:1347): dark blue CONFIG_BGCOLOR
  screen, player at x=20 with the attack pose, and the three lines
  "<WEAPON>" / "AND <ITEM>" (stages 1 and 6) / "HAVE BEEN ACQUIRED" (or
  "HAS BEEN ACQUIRED"), held 5s or until START.

The MD has no full-screen colour fill, so the flash writes the colour into
PAL0 slot 0 (the backdrop) with a queued palette write and clears both planes
to a transparent tile.

## BUG-030 — Font corrupted after winning a stage

**Report**: after clearing a stage the font is corrupted.

**Status**: **FIXED 2026-10-22** (play-test pending). Two screens were hit:

1. The stage-clear / YOU GOT / game-over text screens draw with PAL0 while the
   stage backdrop is still in CRAM 0: `textScreenBegin()` reloads PAL0 from
   `pal_ui` (see the fix note further down).
2. The stage select drew its name lines with PAL0 as well, so the per-map
   backdrop tinted the glyph cells (the "peach on peach" names, BUG-014). It
   now draws them with PAL1 and re-uploads the default font first.

## BUG-031 — ADDRESS ERROR (crash) when standing on a disappearing block

**Report (2026-10-22)**: standing over a disappearing block crashed with the
Mega Drive ADDRESS ERROR screen. The offset in the dump (0x272E4) resolves
inside `SPR_update` in `out/release/symbol.txt`, and the fault address
(0x20302F) is an ODD ROM address: a misaligned read.

**Cause**: `hud.c` `set_bar()` computed `frame = (pct * 50) / 100` (0..50) and
passed it to `SPR_setFrame()` on the HUD ball sheets, which have **5** frames
(`tools/convert_assets.py`, ordered full..empty). Any frame above 4 makes
`SPR_update` read a garbage frame pointer out of the sprite definition - the
odd address it then tries to read is the crash. It is intermittent because the
memory after the frame table sometimes looks like a valid frame.

**Fix**: `frame = ((100 - pct) * (HUD_BALL_FRAMES - 1)) / 100`, clamped to
0..4 (frame 0 = full). Applies to the HP, boss and weapon bars.

## BUG-026 — Checkpoint shows no animation when activated

**Cause (first half)**: the PC object flag `animation_loop` (file_object byte
82) is 1 for the checkpoint, so its flag keeps waving after activation; the
port hardcoded `is_looping()` to type 32 (boss teleporter) only, so the
checkpoint played one frame and stopped.

**Fix (first half)**: the object converter now emits `anim_auto`/`anim_loop`
per def and `is_looping()` uses the data flag.

**Second half (still invisible after that)**: the two art frames were only 6
pixels apart because PAL0 has a single green - see the entry above (fixed
2026-10-22 by pinning the terminal's colours apart in `import_pc_art.py`).

## BUG-030 — Font corrupted after winning a stage

**Fix (pending play-test)**: `textScreenBegin()` now reloads PAL0 from
`pal_ui`, so the text screens (stage clear / YOU GOT / game over) cannot
inherit the per-map backdrop (CRAM 0, written by `apply_bg_color`) or any other
palette the stage left behind.

## BUG-018 — Water enemies leave the water (still open after the first fix)

**Cause**: the first fix (`enemies_update()` reverts a move that crosses the
water line) compared the NEW position with itself: `enemy_water_ok(e, e->x,
e->y)` read `e->x/e->y`, which the AI had already moved, for the "old" probe
too, so `now == was` was always true and the rule never fired. The SEAHORSE
fish therefore kept swimming onto the pipes.

**Fix**: pass the pre-move position (`pre_x/pre_y`, captured before the AI runs)
as the "old" probe. A non-boss NPC now cannot leave water or enter it, exactly
like `character.cpp:1921-1923`.

## BUG-032 — HP/weapon/boss bars show empty

**Cause**: the bar sheets are 51 frames ordered **empty..full** (frame 0 = no
fill, frame 50 = full, `tools/convert_energy_bars.py`), but `set_bar()` still
used the old 5-frame ball sheet scale: `((100 - pct) * 4) / 100`, i.e. 0..4 of
51. Full HP selected frame 0, which is the empty art, so the bars never showed
a fill (and the gauge could not move).

**Fix**: `HUD_BAR_FRAMES 51`, `frame = (pct * 50) / 100`, clamped. Applies to
the HP, boss and weapon bars (all three sheets are the same 51-step art).

## BUG-033 — SEAHORSE runs at half speed (animated tiles)

**Measured** (`make profile`, per-frame cost in subticks, 1280 = one 60 Hz
frame, SEAHORSE map 0 idle):

```
setup=213  cam=43  anim=1135  npc=160  shot=32  spr=99  wait=766
```

Total ~2460 subticks per iteration = the loop only completes every other
vblank: **~30 fps**, exactly the reported "VERY slow". APE map 0 has no
animated slots, which is why it felt fine.

**Cause**: every animated slot kept **one 4-tile block per frame** (6 frames x
4 = 24 tiles, `TILE_ANIM_STRIDE`) and the map cells pointed at the current
frame's block, so a frame change had to rewrite the tilemap of **every visible
cell of the slot** (4 `VDP_setTileMapXY` per cell, and before that even at raw
map coordinates that ran past the plane window into the H-scroll table).
SEAHORSE has 13 slots over a 200-metatile map: ~15 ms of tilemap writes per
frame.

**Fix**: a slot now owns ONE 4-tile block. The map cells point at it
(frame-independent, so `stage_map_patch` no longer looks at `anim_frame`), and
a frame change is a **single 4-tile DMA** into that block. No tilemap writes at
all. `TILE_ANIM_STRIDE` 24 -> 4 also frees 260 tiles of VRAM: `check_vram` now
reports `OK (14 stages, 0 transition fallbacks)` (the three stage-2 fade
fallbacks are gone too).

## BUG-033 — SEAHORSE runs at half speed (animated tiles) — VERIFIED FIXED

**Re-measured in the emulator 2026-10-25** (profile build, stage 3): the busy
time per frame is **642 subticks of 1280** (was 2460, i.e. one frame every other
vblank) and the animated-tile bucket (`anim`) is **61** (was 1135). The stage
holds 60 fps.

Two spikes remain in that run, both in `objects_update`: 2028 subticks twice and
14637 once, at the stage-start position — they line up with the respawn/teleport
path (`stage_teleport_boss` reloads the stage, and `dialog_show_armor` runs its
own blocking input loop), so they are transitions, not per-frame cost.

## BUG-034 — Death respawns at the wrong checkpoint (SEAHORSE)

**Report**: activated the second SEAHORSE checkpoint, died, and came back at
the THIRD one, next to the boss teleporter.

**Cause**: `respawn_player()` honoured the checkpoint only when
`cp_map == stage_current_map`; dying in the next room fell through to that
room's start position. The PC (`game.cpp:395`) treats the checkpoint as the
STAGE-wide respawn point and resets the stage maps, so a death always returns
to the checkpoint room. `cp_active` was also never cleared, so a checkpoint
from a previous stage could survive into the next one.

**Fix**: a new stage clears the checkpoint (`player_clear_checkpoint()` from
`runStage`), and the respawn now loads the checkpoint's map when the player
died elsewhere (`stage_load(cp_map)`), then places him at the stored point.

## BUG-039 — APE BOT crash (line 1111) and a silent quake attack

**Report (2026-10-22 play test)**: fighting APE BOT his quake attack did
nothing, then the game crashed with `LINE 1111 EMULATOR` /
`screenshots/bugs/ape/crash.png` (PC 0xF90008, SP garbage).

**Cause (offline, from the data)**: every stage boss except MILONGA had a
**3-animation** sheet while its `EnemyDef` still pointed at rows 3/4:

```
stage 1 def 1 APE BOT: jump_anim=3 but enemy_apebot_sprite has 3 animations
stage 1 def 1 APE BOT: intro_anim=4 but enemy_apebot_sprite has 3 animations
... 17 mismatches over stages 1..8
```

A sheet is shared by every stage that spawns that art, and the castle stages
spawn the eight bosses as *plain enemies* (3 rows). `make assets` converts stages
0..13 in order, so stage 13's 3-row sheet and SPRITE line overwrote the boss
stage's 5-row one. `SPR_setAnim(spr, 3)` past the end of the animation table
leaves `sprite->animation` on garbage (SGDK only guards it when the lib log
level is high enough), and the next `SPR_update` reads a bogus frame pointer -
the same mechanism as BUG-031. It fires when the boss **jumps** (APE's jump =
his quake attack), which is why it happened mid-fight.

**Fix**: `convert_enemies.py` keeps the TALLEST existing sheet for an art name
and, when it does, leaves its SPRITE line alone, so a 3-row castle pass can no
longer shrink a 5/6-row boss sheet (regardless of stage order). Re-running the
converters restored every boss sheet. `tools/audit_data.py` now cross-checks
every stage's `jump/teleport/intro_anim` against the SPRITE animation counts
(prints `animation index problems: 0`).

**Quake**: the PC's `ia_action_quake_attack()` (the damage + jump lock) has **no
caller** in that build - the observable effect of a QUAKE projectile is the
screen shake (`projectilelib.cpp:677`: +-2 px every 20 ms). The port's enemy
shot did nothing at all. `eshot.c` now exposes `eshots_shake_x()` and
`stage_update_camera()` adds it to `shot_shake_x()`, so APE's quake shakes the
stage like the PC.

## BUG-040 — Boss-intro name is a solid bar

**Report (2026-10-22 play test)**: the intro screen shows now, but the boss name
is just a bar.

**Cause**: the text palette is global state. `draw_names()` in the stage select
set `VDP_setTextPalette(PAL1)` and only the *first* draw restored PAL0 - every
cursor move after that left PAL1 installed. `boss_intro_run()` then loaded PAL1
with the *background* palette, so the name's glyph cells were painted with that
line (white cells with black ink / black on black) instead of the font's black
background + white ink.

**Fix**: `draw_names()` restores PAL0 on every exit path, and `boss_intro_run()`
sets the text plane and palette explicitly before drawing the name.

## BUG-035 addendum — HUD in the player's own colours

The HP bar first moved to PAL0 with the PC's fixed PLAYER 1 colours (which
stopped the sprite recolour but ignored the chosen player). Per the play test
("the HUD should use his colours"), the bar is back on **PAL2** with
**2/3/4 reserved** for it: `tools/convert_player.py` remaps any art pixel on
those entries to the nearest free colour (~700 px over the four sheets, 1-2 px
details), `player.c` writes the selected player's weapon-0 colours 1/2 and black
there, and the hit-flash palettes leave 2/3/4 untouched so the gauge does not
blink. `check_palettes.py` asserts both halves.

## Play-test batch 2026-10-22 (after the boss/animation commits)

Reports, in the user's order, with what is known so far:

1. **BETABOT colours all wrong** — ROOT CAUSE (2026-10-22): not the enemy
   sheet; `rebuild_player_palette()` wrote the HUD bar colours into PAL2 8/9/12,
   entries the player art uses. See BUG-035, fixed.
2. **HP bar broken** — same cause; the bar is PAL0 with the PC's fixed colours
   now (BUG-035, fixed).
3. **Checkpoint still does not animate when activated** — the object logic was
   already correct (anim_loop=1); the two art frames quantized to the same
   colours. See BUG-026, fixed.
4. **Slowdown in CHAOS CITY around the active platforms + small pill**: still
   needs numbers. `make profile` (MD_PROFILE) boots straight into a hardcoded
   stage - set that to 0 first. No code added until the profile exists.
5. **Dialog text background must match the panel colour**; the A-button arrow
   must be WHITE - fixed, BUG-037.
6. **Explosions must move away from the centre** - fixed, BUG-036 (the burst
   was also frozen: the death branch never advanced the puffs).
7. **Stage-select font** (`stage_select_bad_font.png`) - fixed, BUG-014.
8. **SEAHORSE stage intro did not play** (only music) - FIXED, BUG-015: it was
   not the stage mask (the live SRAM shows only CHAOS CITY cleared), it was the
   held stage-select button skipping the intro on its first frame.
9. **After entering SEAHORSE the player sprite is invisible** (projectiles
   visible), then **CRASH** on the second play (`crash_seahorse_second_played_stage.png`).
   Root cause found: the new jump/teleport animation rows for NORMAL enemies
   pushed those rooms past the 504-tile sprite pool, so the sprite engine could
   not allocate the player's frames (invisible) and read garbage frame data
   (ADDRESS ERROR). Fixed by shipping those rows for bosses/sub-bosses only and
   growing the pool to 768 tiles (`SPR_initEx(768)`, stage data ends at 573).
   Needs a re-test.

## Session 2026-10-27 (owner reports, worked one at a time)

- [x] Config menus auto-repeat UP/DOWN (waited only for A/B/START release) - f15f4f21
- [x] Auto-charge behaved like turbo (PC inverts the attack command) - f15f4f21
- [x] APE BOT does not walk on the ceiling: AIR_WALK latched its target at
      `act_t == 0`, but the tick increments act_t before the action runs, so the
      latch never happened and he walked to a stale dest_x (usually already
      reached) until FALL.
- [x] YOU GOT screen: background and text shifted right (plane scroll from the
      stage not reset in got_weapon_bg). Screenshot named by the owner was not
      on disk (only the older screenshots/bugs/you_got_screen.png); cause
      inferred from code, not seen. Needs a re-check.
- [x] APE QUAKE did no damage (quake shot had no collision path of its own and
      the generic box test never matched). Now PC damage_ground_npcs: every
      on-screen enemy standing on solid ground takes the weakness multiplier per
      frame while the quake lives (hit window still limits the rate).
- [x] Boss door closed again the moment the transition scroll started
      (objects_sync_sprites -> door_sync re-showed the slices). Added
      MapObj.door_open: slices stay hidden through the scroll, then the close
      animation (borders in, reverse of the opening) runs.
- [x] Dropping off a ladder with JUMP, then changing room mid-fall: the player
      re-grabbed the arrival ladder (link arrival always hung him on a stair) and
      a held UP re-grabbed once the 12-frame lock ran out. Arrival now hangs only
      if he left on a ladder; a dropped ladder needs UP/DOWN released first.
- [x] Jump interrupted when the head reaches the top of the room (any room with
      open sky): objects_head_block() returned -1 for "no block", and the caller
      took any value above the new y as a ceiling, so once the head passed y -2
      the player was snapped to y -1 with vy 0. Sentinel is now OBJ_NO_HEAD_BLOCK
      (-32768).
- [x] Slide ended inside a 1-tile tunnel (button released / wall / distance) and
      stood the player up into the ceiling. The slide now cannot end while a
      solid tile is in the strip between the standing and the slide head.
- [x] APE QUAKE left a static projectile image: it is an effect, its slot is now
      never shown (also kept hidden by shots_set_visible).
- [x] Dynamite (stage 6) / stage 1 clear screen named E-TANK / W-TANK: PC
      player_items = [16, 17] = object ids COIL FROG / JET EAGLE (checked in
      game_object_list.dat); the tank ids are 5/6. Text now COIL FROG / JET EAGLE.
- [x] Enemy DYNAMITE FLASH (FREEZE projectile) did not freeze the player: the
      eshot treated it as a bomb. PC freeze_weapon_effect=PLAYER: no input for
      3500 ms (210 frames) unless the post-hit invulnerability runs. White
      screen blink not ported.
- [x] JET EAGLE flipped left/right while flying: its art is PC's two direction
      rows flattened (frames 0,1 = LEFT, 2,3 = RIGHT) and the loop cycled all
      four. Now it stays on the spawn-direction pair (and no hardware flip,
      which also showed the wrong facing for a left spawn).
- [x] DYNAMITE FLASH (player): stars effect added (10 fixed random 8x8 points,
      3 frames / 260 ms, while the freeze runs); during the freeze the weapon
      button shoots the normal buster shot (no ammo), and the first press after
      it ends starts the effect again (PC classplayer.cpp:348).
- [x] BAT BALL (DARK MINE) did not return to its origin: step_axis' vertical
      test took the WHOLE box at the new position when moving up, so a bat
      resting on/next to a pillar top was "blocked" while moving away and the
      FLY TO_SAVED state ended on the spot. Only the leading edge blocks now.
      Reproduced with tools/sim_bat_return.py: 43 of 858 chase/return pairs
      ended away from the spawn, 22 after the fix (the rest are real walls
      between the bat and its origin on one axis, as on PC).
- [x] MAGE BOT with his star circle up took damage. PC: an NPC whose only
      projectile is a held CENTERED one is invulnerable (shots are consumed,
      classplayer.cpp:738). The stars now also stay on the owner for 3.6 s
      (PC _effect_timer) instead of staying at the firing point for 0.5 s.
- [x] AI JUMP options: TO_PLAYER_DIRECTION (MAGE HELMUT) kept a stale dest_x so
      it only hopped up; destinations now follow PC for every option.
- [ ] AI audit (in progress): audit_data.py marks an option "ok" when code
      exists, not when it behaves like PC (that hid the JUMP one).
- [x] AI audit, round 1 (PC artificial_inteligence.cpp vs enemy.c, only the
      options the data uses): FLY AHEAD no longer drifts to the spawn y (ZIGZAG
      oscillates around its start y), UP/DOWN/FALL keep x, OPPOSITE_SHOOT fires
      every 120 px (not once at the end), OPPOSITE_WALL turns on arrival, DASH
      honours its option (TO_PLAYER / OPPOSITE / LEFT / RIGHT / AHEAD) at 2x speed
      instead of a fixed 20-frame burst forward, WALK TURN_TO_PLAYER is instant,
      GRAB_WALL leaps to the wall at mid-screen height (gravity off until FALL).
      Not ported (no data uses them): FLY VERTICAL_CENTER, SIN_AHEAD,
      DOWN_RANDOM_DIAGONAL, UP_UNTIL_GROUND, DOWN_INTO_GROUND, DIAGONAL_UP/DOWN;
      JUMP TO_SAVED is a no-op on PC (the port hops back to the spawn).
      Still to review: WAIT_RANGE, SHOT_*, SPAWN_NPC, CHANGE_MOVE, JUMP_ATTACK_UP.
- [x] Config menus now accept the JUMP button (as mapped in CONFIG BUTTONS) to
      select, besides A / START; B stays BACK even when jump is mapped to it.
- [x] Auto-charge showed no charge flash/sound (my auto-charge branch skipped the
      feedback). Extracted charge_feedback(); also gated on player_can_charge()
      and the palette/sound timer reset on release.
- [x] Water splash appeared where the player jumped / entered (it fired when the
      "feet in water" flag flipped, at the feet). It now follows PC
      character.cpp:2283 at the water surface: head and body in air, feet in
      water, rising (leaving) and only after having been in water, latched until
      the three probe points agree again. Entering the water no longer splashes
      (owner: splash only when leaving) - say if entering should splash too.
- [x] Fish (MECHAFISHER, DOLGOLPHIN) and a bat stood still; SPINNASTER waded into
      water and stuck. The water-line rule (non-boss NPCs never enter/leave water)
      reverted the move AFTER the AI step, so the AI never saw a block and its
      state never finished (e.g. a fish chasing a player above the surface). It is
      now a block inside step_axis (state finishes like against a wall), and
      walkers test their feet line (both corners + centre) instead of only the
      body centre. Likely exposed by the vertical leading-edge change (flyers now
      reach the water line instead of stopping early).
- [x] SEAHORSE boss teleporter dropped RockBot at row 0 (in the ceiling): the
      destination column (map 2, col 43) is water down to the floor and
      first_lock_on_bottom() counted WATER as a lock/obstacle, so it found no
      ground. PC treats WATER as passable. Offline: result row 0 -> 11.
- [x] MAGIC STAR was released by itself after 0.5 s (looked like it fired on one
      press). PC: the first press creates the star circle around the player, it
      stays until the next press (or 3.6 s), which releases it. A held star also
      absorbs one enemy projectile (both vanish), like the NPC rule.
- [x] Audit sweep (owner: "those bugs should not be there in the first place"):
      enemy hops / jump attacks had no ceiling check (only landing), so they
      rose through ceilings; added ceiling_clip(). Reviewed and found OK:
      act_t==0 latches (none left), menus' edge detection (all other screens
      use pressed-edge), WAIT_RANGE, AI stale dest_x/dest_y per action.
      Known remaining gaps, not bugs of the same kind: SHOT_MULTIPLE ignores
      PC's pattern options (no data uses it); flyers' step_axis shares one
      accumulator between x and y (a one-axis flyer moves ~25% slower than a
      walker of the same speed).
- [x] Fish / bats pinned in place while chasing: every enemy was clamped to the
      converter's patrol limits (spawn +- range) after the AI step, so a flyer
      chasing a player further away than that stayed at the border and its state
      never finished (probably the real cause behind the BAT / MECHAFISHER /
      DOLGOLPHIN reports). The leash now applies to walkers only, and as a block
      inside step_axis.
- [x] LARVA (FLOWER FIELDS map 1, left) sat inside the ground: the room's floor
      has conveyor tiles (terrain 8) in the row above the solid floor, and enemy
      collision only knew TERRAIN_SOLID. PC blocks NPCs on every terrain but
      UNBLOCKED/STAIR/difficulty blocks, so conveyors, ice, sand and spikes are
      floors for enemies now (ENEMY_SOLID in enemy.c): spawn y 161 -> 145.
- [x] PAMPA METALEMA (WALK AHEAD whose state loops on itself = PC
      always_move_ahead) stopped at gaps: PC skips the hole test for such walkers
      so they march on and fall; the port kept the ledge stop (and the patrol
      leash). Both are skipped while always_ahead is set.

## Report 2026-10-27 (batch of 9, to be worked one at a time)
- [x] disappearing blocks: hidden countdown was cancelled one frame later by the visible-branch (vis_t != 0 re-entered the countdown); hidden phase now counts down from BLOCK_HIDDEN_FRAMES
- [x] wall shooter must not shoot backwards (PC fires along facing side, down from ceiling, up from floor; the port aimed at the player anywhere)
- [x] diagonal shots: enemy projectiles with trajectory DIAGONAL_UP/DOWN never got a dy (flew straight)
- [ ] many slowdowns in the TECHNO stage
- [x] bomb / fall-bomb: now the PC 56x56 7-frame explosion (3.6 s / 1.6 s), damaging only while exploding, with the big-explosion sound
- [x] checkpoint respawn: the object settles 1px into the floor (PC platform_can_move tests y+h-2), so feet spawned 1px inside the ground in 39/39 checkpoints; respawn now lifts him out
- [ ] MUMMY link 5 scrolls left ignoring the wall-lock
- [x] helmets: attack row looped, re-showing its first (stand) frame when the SHOT action ran a tick past the animation; shot attacks now hold the last frame (unverified: needs a play-test)
- [x] PHARAOH vanished: e->teleporting was only cleared on a fall landing (stray line), so after the first TELEPORT he kept the dissolve frame forever; cleared when the action ends

## Report 2026-10-27 (follow-ups)
- [x] SPIKE BOT wall grab: JUMP pose while leaping, WALK_AIR pose clinging (new air row in the enemy sheet; GRAB_WALL leap also forces the jump row)
- [x] SPIKE BOT wall shot: not a bug - the PC data fires DIAGONAL DOWN (id 13); the chain shot was lost in the PC long ago (owner confirmed)
- [x] CHAIN projectile grows from nothing and retracts (frames = slices of the art; hitbox follows)
- [x] enemies act only while in the same room as the player (no scroll-lock column between them)
- [x] CASTLE 1 flame objects (fire rays 18/19, small horizontal beams 25): sprite origin was the base tile instead of the PC grow-from-base cell, art blinked fully on/off every 1.5 s instead of growing/shrinking tile by tile (30 ms steps, 1500 ms holds, 70 ms flicker), hit area ignored the length, damage was 4 for the deadly beams (PC 999). Baked slice frames per length/flicker/side; PC state machine ported. STILL OLD: vertical lava beam (24) blinks.
- [x] audit 2026-10-27: PC animation rows vs shipped rows - flyers use WALK_AIR frames (9 had different timing/order), DASH -> SLIDE row, jump attacks / shots in the air -> JUMP_ATTACK row (bosses); missiles pick the direction row instead of cycling the 7 art rows; CENTERED/SPIRAL/LIGHTING projectile frame periods
- [ ] known gaps left: SPECIAL_ATTACK rows (10 bosses, PC AI never plays them), MOVE_SEMI (SPINNASTER), vertical lava beam art slides per tile in the PC (port shows it whole), small beams capped at 14 tiles (rescomp frame width limit), 4 beams x 30 tiles of sprite VRAM in CASTLE 1 map 1 (check on hardware)
- [x] audit: enemy QUAKE shot (APE BOT) hurt a grounded player every frame (3) and locked his jump 1 s in the PC (classnpc.cpp:519); the port only shook the screen. sub-boss death explosion (2 x 56x56 + sfx) was the small puff
- [x] audit: contact damage - PC hurts TOUCH_DAMAGE_BIG (4) unless the NPC frame is taller than the player's 29px (then SMALL 2); the port always did 2 (def has frame_h now)
- [x] audit: PC vulnerable_area - player shots only hit that rectangle (WASPER body, GOLD CART rider, UFO core, DESTRIN TANK, IRON TANK, SKULLPUNK, DOLGOLPHIN, RHINOZER...; APE BOT's is the whole frame); the port used the animation collision box. Mirrored for the flipped side (unverified against PC: its x formula for facing right is position.x - vul.x)
- [x] TECHNO slowdowns (profile via scripted autopilot, 3 rooms): 23/38/44 frames per second over budget -> ~10/16/10 after: sprite calls cached per enemy/player/HUD, terrain lookup without the multiply, water probes skipped without water, reaction maths skipped for enemies without reactions, parked enemies skip gravity, room span cached. Still ~15% of frames over in the worst room with the autopilot firing constantly (profile: enemies ~300-360, player ~200, SPR_update ~140, camera ~95 subticks of 1280).
- [x] MUMMY link 5 trace (MD_LINK_TRACE): arrival is dst (181,0) with no ladder, camera 2653, residual 0 - matches the PC arithmetic; the visible symptom could not be reproduced from numbers (the two locks 288px apart leave the camera 5px behind the left wall). Needs a visual check by the owner.

## BUG-046 — Boss-intro jingle heard without the boss-intro presentation

**Report** (2026-10-30): replaying an already-beaten stage from the stage select
skips the 6.4 s boss-intro screen, but the jingle was still heard.

**Cause**: the beaten-stage test lived in the *caller* (`main.c runStage`), while
the jingle is started inside `boss_intro_run()`, so "presentation" and "music"
were two separate decisions. The PC keeps them together:
`scenesLib::classic_style_boss_intro` starts with
`if (game_save.stages[stage_n] != 0) return;` (sceneslib.cpp:682) - before the
castle screen, the sprites and `load_music`/`play_music_once`. Compounding it,
`music_stop()` returned early whenever the port's own `playing` flag said
nothing was running, so a flag that disagreed with the XGM2 driver left the
previous track (e.g. a jingle) audible across the "stop".

**Fix**: `boss_intro_run(stage, beaten_mask)` now returns FALSE before touching
anything when the stage is beaten, and `main.c` only calls `music_stop()` when
it did not run; `music_stop()` always issues `XGM2_stop()`.

Reproduction: `make check-boss-intro` (`tools/check_boss_intro_gate.py`) -
fails on the old tree with 3 violations, passes after.

## BUG-047 — Stage-select names unreadable again after a stage (BUG-014 relapse)

**Report** (2026-10-30): the boss/stage name lines at the bottom of the stage
select were "still bad sometimes" after finishing a stage.

**Cause**: the names were drawn over the PC bottom-bar art and borrowed PAL1
(the grey ramp loaded for beaten boss faces), so both the glyph background and
the palette came from whatever the finished stage left in CRAM / video memory.
`models/cells` restore only put the bar art back; a stage that had put a
different ramp in CRAM line 1 (or left the bar tileset overwritten) still made
the text unreadable.

**Fix**: `draw_names()` now clears rows 25..27 to font spaces (`VDP_clearTextArea`
writes palette-0 cells = solid black) and draws with PAL0 = `pal_ui`
(entry 0 black, entry 15 ink) loaded with `pal_load()` from `pal_safe.h`, so no
stage palette or bar tile can affect the text.

Reproduction: `make check-ssel-text` (`tools/check_stage_select_text.py`) - 4
violations on the old tree, 0 after.

## BUG-048 — DR. DESTRIN SHIP teleports inside the boss door

**Report** (2026-10-30, `boss_stuck_door.png`): the ship reappeared inside the
closed boss door of the final castle. The port is supposed to block this (the
teleport slides back out of solid objects and the door), so it was a state bug,
not a missing guard.

**Cause**: `objects.c solid_now()` answered a boss door with `!o->started`.
`started` is the "the open/scroll sequence already ran" latch, not the door's
current state: the port opens the door, scrolls the room and *closes* it again
(`door_anim(o, FALSE)`), leaving `started = TRUE, door_open = FALSE`. From then
on the closed-looking door was not solid, so `objects_block_box()` - the very
test the teleport guard and the enemies' horizontal movement use - no longer saw
it and the ship could land (or walk) inside it.

**Fix**: the door's collision follows the visible slices:
`solid_now()` -> `!o->door_open` for type 30.

Reproduction: `tests/host/door_block_test.c` (real `src/object.c` + stubs,
`tools/run_host_tests.sh`): closed door blocks, open door passes, and the door
after the open/close cycle blocks again - the last two checks fail on the old
tree ("closed again after opening -> passes").

## BUG-049 — Parallax background seam at the plane wrap (accepted)

**Report** (2026-10-30): a hard break in the parallax sky every so often. The PC
art repeats seamlessly at its own width: `clouds.png` is 320x240 and its column 0
equals column 319 on every row, so the PC draws it side by side.

**Cause**: BG_B is a 64-tile = 512 px plane and its scroll wraps at 512. 512 is
not a multiple of 320, so at the wrap the strip (art + its first 192 columns)
joins art column 191 to column 0 instead of 319 to 0: a break every 512 px of
parallax travel.

**Decision (owner, 2026-10-30)**: keep the PC art at its own 320 px width and
accept the wrap seam. Resampling to a 256 px period removed the seam but made
every background 20% denser than the PC (and cost tiles: stage 1's parallax
354 -> 451, room-transition fallbacks 12 -> 35), which the owner rejected. A
128-tile-wide plane (1024 px) would remove the seam with the untouched art, but
the MD has ONE plane-size setting for both planes: 8 KB each fills the whole
0xC000-0xFFFF area, leaving no room for the 1 KB hscroll table, the 640 B sprite
list and SGDK's font (pinned at tile 1440). It would need a patched libmd or a
custom text API plus a 64-tile smaller sprite pool.

`tools/convert_stage.py bg_plane` bakes art + its head (native art, PC spacing)
and `make check-bg-seam` asserts every stage strip keeps the PC's art width, so a
future resample regression is caught.

## BUG-050 — CASTLE 5: room arrives one tile to the left after a transition

**Report** (2026-10-30): after the first room change in CASTLE 5 (FROZEN LAIR,
stage 13) the second room's scroll sat one tile to the left.

**Cause**: `scroll_lock_span()` answered "room narrower than the view" (`lo > hi`,
which is every room of stage 13: 16..18 columns = 256..288 px against a 320 px
view) with `camera.x = lo`, pinning the view to the left wall + 2 px. The PC has
no such rule: `classMap::changeScrolling` only refuses to move a locked column
into the view, so a narrow room keeps the scroll it arrived with. Stage 13 room
1 therefore arrived at camera 18 instead of 0 - 18 px, "one tile left".

**Fix** (`src/stage.c`): when `lo > hi` the camera is left as the caller set it
(PC behaviour); the `lo`/`hi` clamp still applies to rooms wide enough for both
bounds.

Reproduction: `make check-narrow-scroll` (`tools/check_narrow_room_scroll.py`) -
it models every link arrival into a narrow room from the real scroll-lock data
and fails while the pin exists (3 480 arrivals would shift; stage 13 room 1:
camera 2 -> 296 for keep 0).

## BUG-051 — The boss door is not a scroll lock (the view scrolls into the arena)

**Report** (2026-10-30, right after BUG-050): "on the transition to the room with
the door, the scroll does not respect the door being there as a scroll-lock" -
CASTLE 5's boss room view scrolled over the closed door.

**Cause**: the port's scroll-lock tables are generated from the terrain only,
but the PC builds `wall_scroll_lock` counting a cell covered by a **boss door
object** as wall too (`classMap::load_map`, classmap.cpp:173-190: `obj_locked`).
FROZEN LAIR map 0 column 179 is solid except rows 7..10, where the 16x64 door
sits, so without the object it was not a lock and the camera passed it. The same
hole existed in 7 stages / 8 maps (stage 1 map 0, stage 3 map 1, stage 4 map 0,
stage 10 map 2, stage 11 map 1, stage 12 map 1, stage 13 maps 0 and 1).

**Fix** (`tools/convert_stage.py read_boss_door_cells`): the converter reads the
boss door spawns (type 30) and counts their cells as wall when it decides which
columns lock the scroll, exactly like the PC. Static data, no runtime cost.

Reproduction: `make check-door-lock` (`tools/check_door_scroll_lock.py`)
recomputes the PC rule from the raw map + object tables and compares it with the
generated lock tables: 8 missing door columns before, 0 now.

## BUG-052 — Ending walking parade: the layers jump (grass/clouds)

**Report** (2026-10-30, `clouds_misalign.png` - the file is a capture of the
ENDING, not a stage): "the clouds are misaligned... other layers suffer too, the
grass jumps when it animates".

**Cause**: the three walking layers are 320 px wide art repeated at the PC's own
320 px width, uploaded as 40 of the 64 plane columns and scrolled with per-tile
row offsets that wrapped at 320 (`% 320`). The window is 320 px wide, so as soon
as the offset passed 192 px it ran past the end of the plane and wrapped to the
plane's first columns instead of the art's continuation: the ground/trees/sky
jumped every 320 px of travel (the ground at 1.1 px/frame = every ~5 s).

**Fix**: `tools/ending_layers.py` bakes each layer to the 512 px plane width with
a 256 px period (same rule as the stage parallax), the maps become 64 columns and
`ending.c` wraps the offsets at 512, so the hardware wrap is seamless.

Verified with `MD_TEST_ENDING` captures 12 s apart (past three old jump points):
the ground band is 256-periodic on screen (mean mismatch 0.2 of 29 rows) and the
sky band has no full-height edge. `make check-bg-seam` also checks the strips.

## BUG-053 — Image WIDTH cut to the plane width (audit of the same class)

**Report** (2026-10-30, follow-up to BUG-049/052): "check if there are other
places we cut a background/scene image width like this".

**Audit** (`make check-width`, `tools/audit_scroll_width.py`): every scrolling
layer in the game is compared with its source art (width, repeat period) and
against the wrap the runtime uses.

- **Fixed**: the cutscene parallax wrapped at a hard-coded 512 px whatever the
  baked period was. `SceneLayer` now carries the baked `period` (tools/
  convert_scenes.py reports what it tiled) and `scene.c` wraps there, so a layer
  whose art repeats at 256 px inside a 384 px image no longer jumps 128 px.
- **Known, accepted**: CASTLE 5 etc. stage parallax keeps the PC's 320 px art and
  wraps at the 512 px plane (BUG-049).
- **Known, open**: the intro cutscene's back layer `city_bg_3.png` is **963 px
  wide** and the MD plane is 512, so the bake keeps only the left 512 px - 451 px
  (47%) of the city never shows and the layer wraps twice per scene (the scene
  scrolls 1200 px). Showing all of it needs the window trick: keep the full art's
  tiles in VRAM and rebuild the 64-column plane map from the wide map as the
  scroll advances (the plane is exactly one window wide).
- Nothing else changes an image width: `img2tiles`/`convert_assets` crop height
  only, `convert_stage` bakes art + its head, `ending_layers` is the deliberate
  256 px bake for the walk.

## BUG-054 — Ending dialog: face background greenish, faces too low

**Report** (2026-10-30, `ending_faces_issue.png`): in the ending dialog the face
portraits sat in a greenish box of their own instead of blending into the blue
clear-area box behind them, and they were drawn too low relative to that box.

**Causes**
1. *Colour*: the portraits carry the PC clear colour (8,24,40) as their own
   opaque background, and the image palette quantised it to a **greenish** entry
   while the box was painted with the clear record's colour. `md_color()`
   additionally packed the CRAM word at bits 8/4/0 instead of the hardware's
   11-9/7-5/3-1 (`0000 BBB0 GGG0 RRR0`), so the two paths could never agree.
2. *Position*: the CLEAR_AREA op converts the PC's 240-line y with `-16`, the
   MOVE_IMAGE op did not - the faces were drawn two tiles lower than their box.

**Fix** (`tools/convert_scenes.py`, `src/scene.c`): `md_color()` packs the CRAM
word correctly (and uses the same 8->3 bit rule rescomp applies to palette
PNGs); the portrait background is repainted a pure dark blue (`PORTRAIT_BG`
(0,0,73), "closest possible to the box, not necessarily equal" - owner's call,
no transparency); `show_image()` converts y with `-24` (PC crop plus one tile
up, so the face sits inside the box instead of hanging out of its bottom edge).

Verified with `MD_TEST_ENDING` captures.
