# Known bugs / deviations — RockBot → Mega Drive

Things that are wrong or worse than the original. Track here, fix later.

## BUG-001 — Title screen art loses color depth

**Status**: **FIXED 2026-10-19** — the title draws the PC art at 59 colours
over the four palette lines, verified on hardware: navy backdrop, bright
yellow logotype and the characters' own hues. Two bugs had to go: the
near-colorkey background (below) and an off-by-one palette slot
(`lessons_learned.md` #27). See the
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

## BUG-004 — Stage art is over the 15-colour palette budget in 8 of 9 stages

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

Also: index 0 of the generated palette is a reserved black, so any black in the art
takes a second entry (stage 1's palette has `(0,0,0)` at both 0 and 1). Where that
black is only the *surrounding* empty area, marking it transparent in the source art
reclaims the slot; where it is part of the design (the moving platform's segment
gaps) it cannot be reclaimed.

**Fix path**: the reserved-slot design being discussed for colourcycle
(anim art drawn with generic colours, mapped to per-stage entries; cycling bands in
the free part of PAL3) — plus a deliberate re-quantisation of the art per stage.
See `stage0_plan.md`.

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

