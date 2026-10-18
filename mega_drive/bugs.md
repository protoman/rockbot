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
