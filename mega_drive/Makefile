export GDK ?= /opt/toolchains/mars/m68k-elf

# keep the ROM build as the default goal (the asset rules below come first)
.DEFAULT_GOAL := all

# ---- asset pipeline ------------------------------------------------------
# Every file in res/ is generated from the PC game data by tools/*.py. A
# plain `make` regenerates whatever changed (stamp vs converter scripts and
# PC source assets) before rescomp runs, so resources are never stale.
# Force a full regeneration with `make assets` after touching res/.
PYTHON   ?= /home/iuri/python3/bin/python3
PCROOT   := ..
IMAGES   := images
ASSET_STAMP := res/.assets.stamp
ASSET_TOOLS := $(wildcard tools/*.py)
ASSET_DATA  := $(shell find $(PCROOT)/build/games/RockDroid1 \
                            $(PCROOT)/build/shared/images \
                            -type f \( -name '*.png' -o -name '*.dat' \) \
                            2>/dev/null)
# hand-owned art: the sprite converters read images/, so a hand edit must
# invalidate the asset stamp too
ASSET_DATA  += $(shell find $(IMAGES) -type f -name '*.png' 2>/dev/null)

$(ASSET_STAMP): $(ASSET_TOOLS) $(ASSET_DATA)
	$(PYTHON) tools/convert_assets.py
	$(PYTHON) tools/convert_stage.py 0
	$(PYTHON) tools/convert_enemies.py 0
	$(PYTHON) tools/convert_objects.py 0
	$(PYTHON) tools/img2tiles.py res/stage0_bg0.png --name stage0_bg0
	$(PYTHON) tools/img2tiles.py res/stage0_bg1.png --name stage0_bg1
	$(PYTHON) tools/convert_stage.py 1
	$(PYTHON) tools/convert_enemies.py 1
	$(PYTHON) tools/convert_objects.py 1
	$(PYTHON) tools/img2tiles.py res/stage1_bg0.png --name stage1_bg0
	$(PYTHON) tools/img2tiles.py res/stage1_bg1.png --name stage1_bg1
	$(PYTHON) tools/img2tiles.py res/stage1_bg2.png --name stage1_bg2
	$(PYTHON) tools/convert_stage.py 2
	$(PYTHON) tools/convert_enemies.py 2
	$(PYTHON) tools/convert_objects.py 2
	$(PYTHON) tools/img2tiles.py res/stage2_bg0.png --name stage2_bg0
	$(PYTHON) tools/img2tiles.py res/stage2_bg1.png --name stage2_bg1
	$(PYTHON) tools/img2tiles.py res/stage2_bg2.png --name stage2_bg2
	$(PYTHON) tools/convert_stage.py 3
	$(PYTHON) tools/convert_enemies.py 3
	$(PYTHON) tools/convert_objects.py 3
	$(PYTHON) tools/img2tiles.py res/stage3_bg0.png --name stage3_bg0
	$(PYTHON) tools/img2tiles.py res/stage3_bg1.png --name stage3_bg1
	$(PYTHON) tools/img2tiles.py res/stage3_bg2.png --name stage3_bg2
	$(PYTHON) tools/convert_stage.py 4
	$(PYTHON) tools/convert_enemies.py 4
	$(PYTHON) tools/convert_objects.py 4
	$(PYTHON) tools/img2tiles.py res/stage4_bg0.png --name stage4_bg0
	$(PYTHON) tools/img2tiles.py res/stage4_bg1.png --name stage4_bg1
	$(PYTHON) tools/img2tiles.py res/stage4_bg2.png --name stage4_bg2
	$(PYTHON) tools/convert_stage.py 5
	$(PYTHON) tools/convert_enemies.py 5
	$(PYTHON) tools/convert_objects.py 5
	$(PYTHON) tools/img2tiles.py res/stage5_bg0.png --name stage5_bg0
	$(PYTHON) tools/img2tiles.py res/stage5_bg1.png --name stage5_bg1
	$(PYTHON) tools/img2tiles.py res/stage5_bg2.png --name stage5_bg2
	$(PYTHON) tools/convert_stage.py 6
	$(PYTHON) tools/convert_enemies.py 6
	$(PYTHON) tools/convert_objects.py 6
	$(PYTHON) tools/img2tiles.py res/stage6_bg0.png --name stage6_bg0
	$(PYTHON) tools/img2tiles.py res/stage6_bg1.png --name stage6_bg1
	$(PYTHON) tools/img2tiles.py res/stage6_bg2.png --name stage6_bg2
	$(PYTHON) tools/convert_stage.py 7
	$(PYTHON) tools/convert_enemies.py 7
	$(PYTHON) tools/convert_objects.py 7
	$(PYTHON) tools/img2tiles.py res/stage7_bg0.png --name stage7_bg0
	$(PYTHON) tools/img2tiles.py res/stage7_bg1.png --name stage7_bg1
	$(PYTHON) tools/img2tiles.py res/stage7_bg2.png --name stage7_bg2
	$(PYTHON) tools/convert_stage.py 8
	$(PYTHON) tools/convert_enemies.py 8
	$(PYTHON) tools/convert_objects.py 8
	$(PYTHON) tools/img2tiles.py res/stage8_bg0.png --name stage8_bg0
	$(PYTHON) tools/img2tiles.py res/stage8_bg1.png --name stage8_bg1
	$(PYTHON) tools/img2tiles.py res/stage8_bg2.png --name stage8_bg2
	$(PYTHON) tools/convert_players.py
	$(PYTHON) tools/convert_weapons.py
	$(PYTHON) tools/convert_player.py
	$(PYTHON) tools/img2tiles.py $(IMAGES)/backgrounds/boss_intro/boss_intro_bg.png \
	    --name boss_intro_bg --crop-h 224
	$(PYTHON) tools/img2tiles.py $(IMAGES)/backgrounds/player_selection.png \
	    --name psel_bg --crop-h 224
	$(PYTHON) tools/img2tiles.py $(IMAGES)/backgrounds/player_select_p1.png --name psel_p1
	$(PYTHON) tools/img2tiles.py $(IMAGES)/backgrounds/player_select_p2.png --name psel_p2
	$(PYTHON) tools/img2tiles.py $(IMAGES)/backgrounds/player_select_p3.png --name psel_p3
	$(PYTHON) tools/img2tiles.py $(IMAGES)/backgrounds/player_select_p4.png --name psel_p4
	$(PYTHON) tools/convert_stage_select.py
	$(PYTHON) tools/convert_save_selector.py
	$(PYTHON) tools/img2tiles.py \
	    $(PCROOT)/build/games/RockDroid1/images/backgrounds/weapon_menu.png \
	    --name menu --crop-top 16
	$(PYTHON) tools/img2tiles.py \
	    $(IMAGES)/logo.png \
	    --name title --crop-h 224
	$(PYTHON) tools/convert_sfx.py
	$(PYTHON) tools/gen_gpl.py
	@touch $@

.PHONY: assets
assets: $(ASSET_STAMP)

# Run the built ROM in BlastEm (native binary, see tools/emu.sh EMU_BIN).
# This goes through tools/emu.sh instead of launching the emulator directly:
# that kills a previous instance and relaunches while the window comes up
# black/wedged, so a bad start does not need a manual kill.
#   -g forces software rendering. It was required for the flatpak build under
#   Wayland (the GL renderer froze ~half the time); kept by default here as it
#   is the known-good path — try `make run EMU_ARGS=` if you want GL.
.PHONY: run
run: all
	bash tools/emu.sh start-ready out/rom.bin

include $(GDK)/makefile.gen

# rescomp consumes the generated res/ files: wait for the pipeline
$(RES_O): $(ASSET_STAMP)
