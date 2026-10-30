export GDK ?= /opt/toolchains/mars/m68k-elf

# keep the ROM build as the default goal (the asset rules below come first)
.DEFAULT_GOAL := all

# ---- asset pipeline ------------------------------------------------------
# Every file in res/ is generated from the PC game data by tools/*.py. A
# plain `make` regenerates whatever changed (stamp vs converter scripts and
# PC source assets) before rescomp runs, so resources are never stale.
# Force a full regeneration with `make assets` (tools/build_assets.py --force).
PYTHON   ?= /home/iuri/python3/bin/python3
PCROOT   := ..
# Editable game data: a COPY of the PC build/games/RockDroid1 that the editor
# (editor/, tools/editor.sh) edits and every converter reads. The parent
# rockbot/build/games copy is never used again, so map edits survive a rebuild.
GAMEDATA := data/games/RockDroid1
IMAGES   := images
ASSET_STAMP := res/.assets.stamp
# Converted stages, in index order (tools/add_stage.py appends here).
STAGES := 0 1 2 3 4 5 6 7 8 9 10 11 12 13

# Stage editor (Qt5/Qt6, CMake): the PC editor copied into editor/, pointed at
# data/. Build files stay in editor/build/, the binary lands in editor/editor
# where tools/editor.sh expects it.
editor:
	cmake -S editor -B editor/build -DCMAKE_BUILD_TYPE=Release
	cmake --build editor/build -j4

# tools/build_assets.py decides by CONTENT which converters have to run (the
# editor rewrites every .dat on save, so mtimes say nothing) and gives
# unchanged outputs their old mtime back; it touches the stamp only when some
# generated file really changed, so rescomp / gcc rebuild just what moved.
$(ASSET_STAMP): FORCE
	$(PYTHON) tools/build_assets.py

FORCE:

.PHONY: assets editor
assets:
	$(PYTHON) tools/build_assets.py --force

# Verify the tile layout still fits the fixed VRAM blocks (tileset / parallax
# / animated slots / sprite pool) and list the room transitions that fall back
# to the fade. The per-map tileset sizes only exist after rescomp packs them,
# so this runs after the link.
.PHONY: check-vram
check-vram: all
	$(PYTHON) tools/check_vram.py

# The hand-written fill tiles (scene/config "solid black") are 8 longwords of
# 4bpp tile data, where a wrong constant silently becomes a stripe pattern
# (BUG-044: 0x11111111 instead of 0xFF000000 = two white bars per tile).
.PHONY: check-fills
check-fills:
	$(PYTHON) tools/check_fill_tiles.py

.PHONY: check-sram
check-sram:
	$(PYTHON) tools/check_sram_window.py

# A bare PAL_set*(.., CPU) can be redirected to the wrong CRAM entry by a DMA or
# the vblank handler between its address write and its data words (BUG-044
# follow-up: the boot grey ramp landed 8 or 10 entries down). All CPU-method
# writes must go through src/pal_safe.h.
.PHONY: check-pal
check-pal:
	$(PYTHON) tools/check_pal_writes.py

# The "got armor" pages must describe the ability the piece actually installs
# (BUG: ROCKBOT's legs page said air dash while the piece grants double jump -
# dialogs.c keyed its labels off a different enum order than player.c). Also
# checks the per-player ability tables against game_armorPieces_v301.dat.
.PHONY: check-armor
check-armor:
	$(PYTHON) tools/check_armor_dialog.py

# The boss-intro jingle must never be reachable without the presentation: a
# stage already beaten shows nothing (PC classic_style_boss_intro returns before
# the screen and the music). Also guards music_stop() against early-returning on
# the port's own 'playing' flag, which can leave a stale track audible.
.PHONY: check-boss-intro
check-boss-intro:
	$(PYTHON) tools/check_boss_intro_gate.py

# The stage-select boss/stage names sit on their own black band, drawn with
# PAL0 (pal_ui) loaded through pal_safe - borrowing the PC bar art and PAL1 made
# them unreadable again after some stages (BUG-014 relapse).
.PHONY: check-ssel-text
check-ssel-text:
	$(PYTHON) tools/check_stage_select_text.py

# Run the built ROM in BlastEm (native binary, see tools/emu.sh EMU_BIN).
# This goes through tools/emu.sh instead of launching the emulator directly:
# that kills a previous instance and relaunches while the window comes up
# black/wedged, so a bad start does not need a manual kill.
#   -g forces software rendering. It was required for the flatpak build under
#   Wayland (the GL renderer froze ~half the time); kept by default here as it
#   is the known-good path — try `make run EMU_ARGS=` if you want GL.
# Profiling build: per-second scanline cost of each part of the stage loop,
# printed over KDEBUG (see images/../PLAN.md "Runtime verification"). Needs a
# clean build because the flags are not part of the object dependencies.
.PHONY: profile
profile:
	$(MAKE) clean
	$(MAKE) EXTRA_FLAGS=-DMD_PROFILE

.PHONY: run
run: check-vram
	bash tools/emu.sh start out/rom.bin

include $(GDK)/makefile.gen

# rescomp consumes the generated res/ files: wait for the pipeline
$(RES_O): $(ASSET_STAMP)
