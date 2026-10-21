# Known bugs / deviations — RockBot → Mega Drive

Things that are wrong or worse than the original. Track here, fix later.

## BUG-001 — Title screen art loses color depth

**Status**: open, accepted for now (user decision: "let's not mind the colors for now").

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

**Files**: `tools/convert_assets.py`, `res/resources.res`

---

## BUG-002 — Title screen top band is 8px, original was 20px

**Status**: open, cosmetic.

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

**Status**: open, deferred by user decision ("I will convert them to colorcycle later").

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

**Files**: `tools/convert_stage.py`, `tools/img2tiles.py`, the source art.

