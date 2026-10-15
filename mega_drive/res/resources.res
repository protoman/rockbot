# ---- INTRO (show_notice) ----
# upperland: 6-frame strip, 96x24 each. time 0 = no auto-anim (driven in code)
SPRITE upperland_sprite "upperland.png"  96p 24p FAST 0
SPRITE boberatu_sprite  "boberatu.png"  104p 40p FAST 0
SPRITE jmd_sprite       "jmd_logo.png"   96p 32p FAST 0

# ---- TITLE (sceneslib.cpp:102 draw_main) ----
# 320x224 with an 8px CONFIG_BGCOLOR band on top. Index 0 is reserved/unused,
# so it is drawn on BG_B while text goes on BG_A (font index 0 reveals the logo).
IMAGE title_logo "title_logo.png" NONE 1

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
# rows: STAND, WALK, JUMP, ATTACK, HIT, TELEPORT, STAIRS
SPRITE  player_sprite "player.png" 32p 32p FAST [[120,6][9,9,9,9][9][9][6,6][12,6,6][0]]

# ---- STAGE 0 ENEMIES + PLAYER SHOT (tools/convert_enemies.py) ----
# Generated sheets: row 0 = ANIM_STAND, row 1 = ANIM_WALK. Cells are padded
# to multiples of 8 (SGDK requirement). Shared PAL0 palette
# (palette_ui_enemies in stage0_enemies.c): index 0 transparent, 15 = white.
SPRITE enemy_shield_sprite "enemy_shield.png" 32p 32p FAST [[12,12][12,12]]
SPRITE enemy_spinner_ball_red_sprite "enemy_spinner_ball_red.png" 24p 24p FAST [[1][6,6,6,6,6,6]]
SPRITE enemy_spinner_ball_sprite "enemy_spinner_ball.png" 24p 24p FAST [[6,6,6,6,6,6][6,6,6,6,6,6]]
SPRITE enemy_classic_metool_sprite "enemy_classic_metool.png" 24p 24p FAST [[12][12,12]]
SPRITE enemy_cannon_ball_sprite "enemy_cannon_ball.png" 24p 24p FAST [[120][18,18]]
SPRITE shot_sprite "shot.png" 8p 8p FAST [[1]]
