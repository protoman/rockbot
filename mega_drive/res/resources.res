# ---- INTRO (show_notice) ----
# upperland: 6-frame strip, 96x24 each. time 0 = no auto-anim (driven in code)
SPRITE upperland_sprite "upperland.png"  96p 24p FAST 0
SPRITE boberatu_sprite  "boberatu.png"  104p 40p FAST 0
SPRITE jmd_sprite       "jmd_logo.png"   96p 32p FAST 0

# ---- TITLE (sceneslib.cpp:102 draw_main) ----
# Decomposed by tools/img2tiles.py (350 unique tiles, not a 320x224 bitmap):
# the tiles are packed as src/title_tiles.h/.c and drawn on BG_B by main.c,
# the text goes on BG_A. rescomp TILESET/IMAGE resources reordered/deduped
# tiles and mis-drew the logo when the title was re-entered.
PALETTE title_pal "title_tiles.png"

# ---- STAGE 0: CHAOS CITY (game.cpp INTRO_STAGE, 3 maps) ----
# The tileset is built from the same image as the map (as the SGDK samples do);
# a separate tileset image makes rescomp fail to resolve tile [0,0]
# (see lessons_learned.md #18).
TILESET stage0_tiles0 "stage0_map0.png" FAST ALL
PALETTE stage0_pal0   "stage0_map0.png"
MAP     stage0_map0   "stage0_map0.png"  stage0_tiles0 FAST 0

TILESET stage0_tiles1 "stage0_map1.png" FAST ALL
PALETTE stage0_pal1   "stage0_map1.png"
MAP     stage0_map1   "stage0_map1.png"  stage0_tiles1 FAST 0

# map 2 is empty in the source data; kept so the map array stays index-aligned
TILESET stage0_tiles2 "stage0_map2.png" FAST ALL
PALETTE stage0_pal2   "stage0_map2.png"
MAP     stage0_map2   "stage0_map2.png"  stage0_tiles2 FAST 0

# ---- STAGE 0 BACKGROUNDS (parallax on BG_B, PAL3) ----
# Baked to 512px (plane width) by tools/convert_stage.py so the plane wraps
# naturally. Palette is embedded in the Image and reloaded on every
# stage_load (intro reuses PAL3).
IMAGE stage0_bg0 "stage0_bg0.png" NONE 1
IMAGE stage0_bg1 "stage0_bg1.png" NONE 1

# ---- MUSIC (XGM2 driver; rescomp runs xgmtool on the .vgm) ----
XGM2 stage_music "music.vgm"

# ---- PLAYER (RockBot) ----
# 4 cols x 7 anim rows, 32x32 cells. Green/magenta placeholders already swapped
# to weapon_colors[0] by tools/convert_player.py.
# rows: STAND, WALK, JUMP, ATTACK, HIT, TELEPORT, STAIRS, SLIDE, STAIRS_ATTACK
SPRITE  player_sprite "player.png" 32p 32p FAST [[120,6][9,9,9,9][9][9][6,6][12,6,6][0][0][9]]

# ---- STAGE 0 ENEMIES + PLAYER SHOT (tools/convert_enemies.py) ----
# Generated sheets: row 0 = ANIM_STAND, row 1 = ANIM_WALK. Cells are padded
# to multiples of 8 (SGDK requirement). Shared PAL0 palette
# (palette_ui_enemies in stage0_enemies.c): index 0 transparent, 15 = white.
SPRITE enemy_shield_sprite "enemy_shield.png" 32p 32p FAST [[12,12][12,12]]
SPRITE enemy_spinner_ball_red_sprite "enemy_spinner_ball_red.png" 24p 24p FAST [[1][6,6,6,6,6,6]]
SPRITE enemy_spinner_ball_sprite "enemy_spinner_ball.png" 24p 24p FAST [[6,6,6,6,6,6][6,6,6,6,6,6]]
SPRITE enemy_classic_metool_sprite "enemy_classic_metool.png" 24p 24p FAST [[12][12,12]]
SPRITE enemy_cannon_ball_sprite "enemy_cannon_ball.png" 24p 24p FAST [[120][18,18]]
# Stage boss MILONGA BOT (id 37): 36x33 art in 40x40 cells, stand + walk rows
SPRITE enemy_milonga_sprite "enemy_milonga.png" 40p 40p FAST [[120][120]]
SPRITE shot_sprite "shot.png" 8p 8p FAST [[1]]
# Player + enemy charged projectiles (game_projectile_list ids 11/10).
# Frames are 100ms each (defines.h:644); semi art restored by the enemy
# converter (charged_semi.png), full by charged_shot.png.
SPRITE enemy_shot_semi_sprite "enemy_shot_semi.png" 16p 16p FAST [[6,6]]
SPRITE shot_charge_full_sprite "shot_charge_full.png" 32p 32p FAST [[6,6]]

# ---- STAGE 0 OBJECTS (tools/convert_objects.py) ----
# Single anim row; shared PAL0 (palette_ui_enemies). Checkpoint cell is
# 32x64 (SGDK splits it into two 32x32 hardware sprites).
SPRITE object_energy_pill_small_sprite "object_energy_pill_small.png" 16p 16p FAST [[6,6]]
SPRITE object_fall_block_sprite "object_fall_block.png" 16p 16p FAST [[6,6]]
SPRITE object_1up_sprite "object_1up.png" 16p 16p FAST [[12,12,12,12]]
SPRITE object_platform_up_sprite "object_platform_up.png" 32p 16p FAST [[6,6]]
SPRITE object_checkpoint_sprite "object_checkpoint.png" 32p 64p FAST [[18,18]]
SPRITE object_teleporter_stage_boss_sprite "object_teleporter_stage_boss.png" 32p 64p FAST [[7,7,7]]

# ---- HUD (tools/convert_assets.py: HP balls, PAL0 via tools/pal_ui.py) ----
# 5 frames: fill = full, 3/4, 2/4, 1/4, empty
SPRITE hud_hp_ball_sprite "hud_hp_ball.png" 16p 16p FAST [[0,0,0,0,0]]
# Boss row at RES_W-55 (PC draw.cpp:1237), same frame contract
SPRITE hud_boss_ball_sprite "hud_boss_ball.png" 16p 16p FAST [[0,0,0,0,0]]

# ---- IN-GAME MENU panel (tools/img2tiles.py: PC weapon_menu.png panel,
# cropped 16px for the 320x224 screen, decomposed to ~46 unique tiles) ----
# Tiles are NOT a rescomp TILESET: it dedups/reorders by color and breaks
# the hand-made tilemap. They are packed as src/menu_tiles.h/.c (MD 4bpp)
# and uploaded with VDP_loadTileData in menu.c.
PALETTE menu_pal   "menu_tiles.png"
