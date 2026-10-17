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
# naturally, then split into unique tiles + a tilemap by tools/img2tiles.py.
# Decomposed by tools/img2tiles.py (768 cells -> 232 tiles, 896 -> 26): the
# tiles/map live in src/bg0_*.{c,h} / src/bg1_*.{c,h} and stage.c bg_load()
# uploads them; rescomp only supplies the palettes (PAL3).
PALETTE stage0_bg0_pal "stage0_bg0_tiles.png"
PALETTE stage0_bg1_pal "stage0_bg1_tiles.png"

# ---- MUSIC (XGM2 driver; rescomp runs xgmtool on the .vgm) ----
XGM2 stage_music "music.vgm"

# ---- PLAYER (RockBot) ----
# 4 cols x 7 anim rows, 32x32 cells. Green/magenta placeholders already swapped
# to weapon_colors[0] by tools/convert_player.py.
# rows: STAND, WALK, JUMP, ATTACK, HIT, TELEPORT, STAIRS, SLIDE, STAIRS_ATTACK
SPRITE  player_p1_sprite "player_p1.png" 32p 32p FAST [[120,6][9,9,9,9][9][9][6,6][12,6,6][0][0][9]]
SPRITE  player_p2_sprite "player_p2.png" 32p 32p FAST [[120,6][9,9,9,9][9][9][6,6][12,6,6][0][0][9]]
SPRITE  player_p3_sprite "player_p3.png" 32p 32p FAST [[120,6][9,9,9,9][9][9][6,6][12,6,6][0][0][9]]
SPRITE  player_p4_sprite "player_p4.png" 32p 32p FAST [[120,6][9,9,9,9][9][9][6,6][12,6,6][0][0][9]]

# ---- STAGE 0 ENEMIES + PLAYER SHOT (tools/convert_enemies.py) ----
# Generated sheets: row 0 = ANIM_STAND, row 1 = ANIM_WALK. Cells are padded
# to multiples of 8 (SGDK requirement). Shared PAL0 palette
# (palette_ui_enemies in stage0_enemies.c): index 0 transparent, 15 = white.
SPRITE enemy_shield_sprite "enemy_shield.png" 32p 32p FAST [[12,12][12,12][0]]
SPRITE enemy_spinner_ball_red_sprite "enemy_spinner_ball_red.png" 24p 24p FAST [[1][6,6,6,6,6,6][12,12,48,12]]
SPRITE enemy_spinner_ball_sprite "enemy_spinner_ball.png" 24p 24p FAST [[6,6,6,6,6,6][6,6,6,6,6,6][6,6,6,30]]
SPRITE enemy_classic_metool_sprite "enemy_classic_metool.png" 24p 24p FAST [[12][12,12][9,9,30,6,9,9]]
SPRITE enemy_cannon_ball_sprite "enemy_cannon_ball.png" 24p 24p FAST [[120][18,18][12,12,60]]
# Stage boss MILONGA BOT (id 37): 36x33 art in 40x40 cells, stand + walk rows
SPRITE enemy_milonga_sprite "enemy_milonga.png" 40p 40p FAST [[120][120][12,48]]
SPRITE shot_sprite "shot.png" 8p 8p FAST [[1]]
# Player + enemy charged projectiles (game_projectile_list ids 11/10).
# Frames are 100ms each (defines.h:644); semi art restored by the enemy
# converter (charged_semi.png), full by charged_shot.png.
SPRITE enemy_shot_semi_sprite "enemy_shot_semi.png" 16p 16p FAST [[6,6]]
SPRITE shot_charge_full_sprite "shot_charge_full.png" 32p 32p FAST [[6,6]]

SPRITE enemy_rhino_sprite "enemy_rhino.png" 96p 72p FAST [[540][540][0]]
# ---- ENEMY PROJECTILES (tools/convert_enemies.py): one SPRITE per
# projectile id, art from game_projectile_list.dat graphic_filename.
# The enemy_proj[] table in stage0_enemies.c references these.
SPRITE proj_0_sprite "proj_0.png" 8p 8p FAST [[0]]
SPRITE proj_1_sprite "proj_1.png" 24p 24p FAST [[6,6,6,6]]
SPRITE proj_2_sprite "proj_2.png" 24p 24p FAST [[0]]
SPRITE proj_3_sprite "proj_3.png" 16p 16p FAST [[6,6,6,6,6]]
SPRITE proj_4_sprite "proj_4.png" 56p 56p FAST [[6,6,6,6,6,6,6,6,6]]
SPRITE proj_5_sprite "proj_5.png" 24p 16p FAST [[6,6,6,6]]
SPRITE proj_6_sprite "proj_6.png" 16p 16p FAST [[0]]
SPRITE proj_8_sprite "proj_8.png" 16p 16p FAST [[6,6]]
SPRITE proj_9_sprite "proj_9.png" 8p 8p FAST [[0]]
SPRITE proj_10_sprite "proj_10.png" 32p 32p FAST [[6,6]]
SPRITE proj_11_sprite "proj_11.png" 16p 16p FAST [[6,6]]
SPRITE proj_12_sprite "proj_12.png" 8p 8p FAST [[0]]
SPRITE proj_13_sprite "proj_13.png" 8p 8p FAST [[0]]
SPRITE proj_14_sprite "proj_14.png" 32p 32p FAST [[6,6,6,6,6,6,6]]
SPRITE proj_15_sprite "proj_15.png" 16p 16p FAST [[6,6,6,6,6,6,6]]
SPRITE proj_16_sprite "proj_16.png" 16p 16p FAST [[6,6]]
SPRITE proj_17_sprite "proj_17.png" 24p 16p FAST [[6,6]]
SPRITE proj_18_sprite "proj_18.png" 16p 16p FAST [[6,6,6,6,6,6,6]]
SPRITE proj_19_sprite "proj_19.png" 24p 24p FAST [[6,6,6,6]]

# ---- STAGE 0 OBJECTS (tools/convert_objects.py) ----
# Single anim row; shared PAL0 (palette_ui_enemies). Checkpoint cell is
# 32x64 (SGDK splits it into two 32x32 hardware sprites).
SPRITE object_energy_pill_small_sprite "object_energy_pill_small.png" 16p 16p FAST [[6,6]]
SPRITE object_fall_block_sprite "object_fall_block.png" 16p 16p FAST [[6,6]]
SPRITE object_1up_sprite "object_1up.png" 16p 16p FAST [[12,12,12,12]]
SPRITE object_platform_up_sprite "object_platform_up.png" 32p 16p FAST [[6,6]]
SPRITE object_checkpoint_sprite "object_checkpoint.png" 32p 64p FAST [[18,18]]
SPRITE object_teleporter_stage_boss_sprite "object_teleporter_stage_boss.png" 32p 64p FAST [[7,7,7]]

# ---- PAL0 (enemies/objects/shots/HUD) as a rescomp PALETTE resource:
# loaded in stage_load exactly like the working PAL1/PAL3 lines; generated
# from tools/pal_ui_data.py by tools/gen_gpl.py (also emitted as md_pal0.gpl)
PALETTE pal_ui "pal_ui.png"

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
PALETTE boss_intro_bg_pal "boss_intro_bg_tiles.png"
PALETTE psel_bg_pal "psel_bg_tiles.png"
PALETTE psel_p1_pal "psel_p1_tiles.png"
PALETTE psel_p2_pal "psel_p2_tiles.png"
PALETTE psel_p3_pal "psel_p3_tiles.png"
PALETTE psel_p4_pal "psel_p4_tiles.png"

SPRITE ssel_face_p1_sprite "ssel_face_p1.png" 32p 32p FAST [[0]]
SPRITE ssel_eyes_p1_sprite "ssel_eyes_p1.png" 16p 8p FAST [[0]]
SPRITE ssel_face_p2_sprite "ssel_face_p2.png" 32p 32p FAST [[0]]
SPRITE ssel_eyes_p2_sprite "ssel_eyes_p2.png" 16p 8p FAST [[0]]
SPRITE ssel_face_p3_sprite "ssel_face_p3.png" 32p 32p FAST [[0]]
SPRITE ssel_eyes_p3_sprite "ssel_eyes_p3.png" 16p 8p FAST [[0]]
SPRITE ssel_face_p4_sprite "ssel_face_p4.png" 32p 32p FAST [[0]]
SPRITE ssel_eyes_p4_sprite "ssel_eyes_p4.png" 16p 8p FAST [[0]]
# ---- STAGE SELECT cursor (tools/convert_stage_select.py) ----
# classic_style_stage_select_cursor.png, 2 frames of 48x48 split into 4
# 24x24 quadrants x 2 frames (frame 0 = black brackets, frame 1 = yellow
# pulse). The static screen ships as src/ssel_data.h/.c instead.
SPRITE ssel_cursor_tl_b_sprite "ssel_cursor_tl_b.png" 24p 24p FAST [[0]]
SPRITE ssel_cursor_tl_y_sprite "ssel_cursor_tl_y.png" 24p 24p FAST [[0]]
SPRITE ssel_cursor_tr_b_sprite "ssel_cursor_tr_b.png" 24p 24p FAST [[0]]
SPRITE ssel_cursor_tr_y_sprite "ssel_cursor_tr_y.png" 24p 24p FAST [[0]]
SPRITE ssel_cursor_bl_b_sprite "ssel_cursor_bl_b.png" 24p 24p FAST [[0]]
SPRITE ssel_cursor_bl_y_sprite "ssel_cursor_bl_y.png" 24p 24p FAST [[0]]
SPRITE ssel_cursor_br_b_sprite "ssel_cursor_br_b.png" 24p 24p FAST [[0]]
SPRITE ssel_cursor_br_y_sprite "ssel_cursor_br_y.png" 24p 24p FAST [[0]]

# ---- STAGE 1 (DEEP JUNGLE) ----
SPRITE enemy_rock_metool_sprite "enemy_rock_metool.png" 24p 24p FAST [[180][12,12][9,9,30,6,9,9]]
SPRITE enemy_apebot_sprite "enemy_apebot.png" 40p 40p FAST [[48][12,12][24,24,24,24]]
SPRITE enemy_bee_sprite "enemy_bee.png" 24p 24p FAST [[6,6][6,6][0]]
SPRITE enemy_flower_metool_sprite "enemy_flower_metool.png" 24p 24p FAST [[1][12,12][9,9,30,6,9,9]]
SPRITE enemy_side_shooter_sprite "enemy_side_shooter.png" 16p 32p FAST [[6][600][48]]
SPRITE enemy_tamandua_sprite "enemy_tamandua.png" 40p 32p FAST [[18,18][12,12][0]]
SPRITE enemy_scorpion_sprite "enemy_scorpion.png" 24p 32p FAST [[120][9,8][120]]
SPRITE enemy_larva_sprite "enemy_larva.png" 40p 16p FAST [[12][10,10,10,10][0]]
TILESET stage1_tiles0 "stage1_map0.png" FAST ALL
PALETTE stage1_pal0   "stage1_map0.png"
MAP     stage1_map0   "stage1_map0.png"  stage1_tiles0 FAST 0
TILESET stage1_tiles1 "stage1_map1.png" FAST ALL
PALETTE stage1_pal1   "stage1_map1.png"
MAP     stage1_map1   "stage1_map1.png"  stage1_tiles1 FAST 0
TILESET stage1_tiles2 "stage1_map2.png" FAST ALL
PALETTE stage1_pal2   "stage1_map2.png"
MAP     stage1_map2   "stage1_map2.png"  stage1_tiles2 FAST 0
PALETTE stage1_bg0_pal "stage1_bg0_tiles.png"
PALETTE stage1_bg1_pal "stage1_bg1_tiles.png"
PALETTE stage1_bg2_pal "stage1_bg2_tiles.png"

# ---- STAGE 2 (FLOWER FIELDS) ----
SPRITE enemy_flower_vase_sprite "enemy_flower_vase.png" 24p 24p FAST [[120,18][120,18][24,180]]
SPRITE enemy_daisiebot_sprite "enemy_daisiebot.png" 32p 40p FAST [[12][7,7,7,7][12]]
SPRITE enemy_roof_shooter_sprite "enemy_roof_shooter.png" 16p 16p FAST [[6,6,6,6,6,6][6,6,6,6,6,6][1,6]]
SPRITE enemy_ema_sprite "enemy_ema.png" 40p 48p FAST [[6,6,6][6,6,6][0]]
SPRITE object_platform_down_sprite "object_platform_down.png" 32p 16p FAST [[6,6]]
TILESET stage2_tiles0 "stage2_map0.png" FAST ALL
PALETTE stage2_pal0   "stage2_map0.png"
MAP     stage2_map0   "stage2_map0.png"  stage2_tiles0 FAST 0
TILESET stage2_tiles1 "stage2_map1.png" FAST ALL
PALETTE stage2_pal1   "stage2_map1.png"
MAP     stage2_map1   "stage2_map1.png"  stage2_tiles1 FAST 0
TILESET stage2_tiles2 "stage2_map2.png" FAST ALL
PALETTE stage2_pal2   "stage2_map2.png"
MAP     stage2_map2   "stage2_map2.png"  stage2_tiles2 FAST 0
PALETTE stage2_bg0_pal "stage2_bg0_tiles.png"
PALETTE stage2_bg1_pal "stage2_bg1_tiles.png"
PALETTE stage2_bg2_pal "stage2_bg2_tiles.png"
