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

# ---- STAGE 0: CHAOS CITY (game.cpp INTRO_STAGE, 3 maps) ----
# The tileset is built from the same image as the map (as the SGDK samples do);
# a separate tileset image makes rescomp fail to resolve tile [0,0]
# (see lessons_learned.md #18).
TILESET stage0_tiles0 "stage0_map0.png"  NONE ALL
PALETTE stage0_pal0   "stage0_map0.png"
MAP     stage0_map0   "stage0_map0.png"  stage0_tiles0 NONE 0

TILESET stage0_tiles1 "stage0_map1.png"  NONE ALL
PALETTE stage0_pal1   "stage0_map1.png"
MAP     stage0_map1   "stage0_map1.png"  stage0_tiles1 NONE 0

# map 2 is empty in the source data; kept so the map array stays index-aligned
TILESET stage0_tiles2 "stage0_map2.png"  NONE ALL
PALETTE stage0_pal2   "stage0_map2.png"
MAP     stage0_map2   "stage0_map2.png"  stage0_tiles2 NONE 0

# ---- STAGE 0 BACKGROUNDS (parallax on BG_B, PAL3) ----
# Baked to 512px (plane width) by tools/convert_stage.py so the plane wraps
# naturally, then split into unique tiles + a tilemap by tools/img2tiles.py.
# Decomposed by tools/img2tiles.py (768 cells -> 232 tiles, 896 -> 26): the
# tiles/map live in src/bg0_*.{c,h} / src/bg1_*.{c,h} and stage.c bg_load()
# uploads them; rescomp only supplies the palettes (PAL3).
PALETTE stage0_bg0_pal "stage0_bg0_tiles.png"
PALETTE stage0_bg1_pal "stage0_bg1_tiles.png"

# ---- SCENES (tools/convert_scenes.py + img2tiles.py) ----
# The intro's parallax layers: art baked to 512px (plane width) and decomposed
# into unique tiles + tilemap by img2tiles, like the stage backgrounds. Only
# the palettes go through rescomp; the tiles/map load from src/*_tiles.h and
# src/*_map.h (rescomp reorders TILESET tiles).
PALETTE scene_par0_l0_pal "scene_par0_l0_tiles.png"
PALETTE scene_par0_l1_pal "scene_par0_l1_tiles.png"

PALETTE scene_img6_pal "scene_img6_tiles.png"
PALETTE scene_img8_pal "scene_img8_tiles.png"
PALETTE scene_img9_pal "scene_img9_tiles.png"
PALETTE scene_img10_pal "scene_img10_tiles.png"
PALETTE scene_img11_pal "scene_img11_tiles.png"
PALETTE scene_img12_pal "scene_img12_tiles.png"

# classic "YOU GOT" screen background (PC classic_style_got_weapon ->
# show_config_bg): images/shared/config_bg.png via img2tiles.py
PALETTE gotbg_pal "gotbg_tiles.png"
PALETTE dlg_panel_pal "dlg_panel_tiles.png"
PALETTE dlg_face_0_pal "dlg_face_0.png"
SPRITE dlg_face_0_sprite "dlg_face_0.png" 32p 32p FAST [[0]]
PALETTE dlg_face_1_pal "dlg_face_1.png"
SPRITE dlg_face_1_sprite "dlg_face_1.png" 32p 32p FAST [[0]]
PALETTE dlg_face_2_pal "dlg_face_2.png"
SPRITE dlg_face_2_sprite "dlg_face_2.png" 32p 32p FAST [[0]]
PALETTE dlg_face_3_pal "dlg_face_3.png"
SPRITE dlg_face_3_sprite "dlg_face_3.png" 32p 32p FAST [[0]]
PALETTE dlg_face_4_pal "dlg_face_4.png"
SPRITE dlg_face_4_sprite "dlg_face_4.png" 32p 32p FAST [[0]]
PALETTE dlg_face_5_pal "dlg_face_5.png"
SPRITE dlg_face_5_sprite "dlg_face_5.png" 32p 32p FAST [[0]]
PALETTE dlg_face_6_pal "dlg_face_6.png"
SPRITE dlg_face_6_sprite "dlg_face_6.png" 32p 32p FAST [[0]]
PALETTE dlg_face_7_pal "dlg_face_7.png"
SPRITE dlg_face_7_sprite "dlg_face_7.png" 32p 32p FAST [[0]]
PALETTE dlg_face_8_pal "dlg_face_8.png"
SPRITE dlg_face_8_sprite "dlg_face_8.png" 32p 32p FAST [[0]]
PALETTE end_l1_pal "end_l1_tiles.png"
PALETTE end_l2_pal "end_l2_tiles.png"
PALETTE end_l3_pal "end_l3_tiles.png"
PALETTE menu_icons_pal "menu_icons.png"
SPRITE menu_icon_sprite "menu_icons.png" 8p 8p FAST [[0,0,0,0,0,0,0,0,0,0,0]]
SPRITE menu_icon_gray_sprite "menu_icons_gray.png" 8p 8p FAST [[0,0,0,0,0,0,0,0,0,0,0]]
SPRITE menu_tank_sprite "menu_tanks.png" 8p 8p FAST [[0,0,0]]
SPRITE menu_tank_gray_sprite "menu_tanks_gray.png" 8p 8p FAST [[0,0,0]]
SPRITE hud_icon_sprite "hud_icon.png" 8p 8p FAST [[0,0,0,0,0,0,0,0,0,0,0]]
# Classic vertical HUD bars: 51 one-percent frames per 24/24/8px slice
SPRITE hud_bar_hp_top_sprite "hud_bar_hp_top.png" 8p 24p FAST [[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]]
SPRITE hud_bar_hp_mid_sprite "hud_bar_hp_mid.png" 8p 24p FAST [[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]]
SPRITE hud_bar_hp_bot_sprite "hud_bar_hp_bot.png" 8p 8p FAST [[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]]
SPRITE hud_bar_wpn_top_sprite "hud_bar_wpn_top.png" 8p 24p FAST [[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]]
SPRITE hud_bar_wpn_mid_sprite "hud_bar_wpn_mid.png" 8p 24p FAST [[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]]
SPRITE hud_bar_wpn_bot_sprite "hud_bar_wpn_bot.png" 8p 8p FAST [[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]]
SPRITE hud_bar_boss_top_sprite "hud_bar_boss_top.png" 8p 24p FAST [[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]]
SPRITE hud_bar_boss_mid_sprite "hud_bar_boss_mid.png" 8p 24p FAST [[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]]
SPRITE hud_bar_boss_bot_sprite "hud_bar_boss_bot.png" 8p 8p FAST [[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]]
# Classic vertical HUD bars (tools/convert_energy_bars.py)
PALETTE dlg_face_armor_pal "dlg_face_armor.png"
SPRITE dlg_face_armor_sprite "dlg_face_armor.png" 32p 32p FAST [[0]]

# ---- MUSIC (tools/convert_music.py -> VGM -> XGM2) ----
XGM2 mus_opening "music/mus_opening.vgm"
XGM2 mus_intro_scene "music/mus_intro_scene.vgm"
XGM2 mus_selector "music/mus_selector.vgm"
XGM2 mus_selector_alt "music/mus_selector_alt.vgm"
XGM2 mus_boss_intro "music/mus_boss_intro.vgm"
XGM2 mus_castle_intro "music/mus_castle_intro.vgm"
XGM2 mus_castle_intro_alt "music/mus_castle_intro_alt.vgm"
XGM2 mus_boss "music/mus_boss.vgm"
XGM2 mus_final_boss "music/mus_final_boss.vgm"
XGM2 mus_got_weapon "music/mus_got_weapon.vgm"
XGM2 mus_game_over "music/mus_game_over.vgm"
XGM2 mus_stage_complete "music/mus_stage_complete.vgm"
XGM2 mus_enemy_parade "music/mus_enemy_parade.vgm"
XGM2 mus_end_animation "music/mus_end_animation.vgm"
XGM2 mus_end_credits "music/mus_end_credits.vgm"
XGM2 mus_transporters "music/mus_transporters.vgm"
XGM2 mus_stage0 "music/mus_stage0.vgm"
XGM2 mus_stage1 "music/mus_stage1.vgm"
XGM2 mus_stage2 "music/mus_stage2.vgm"
XGM2 mus_stage3 "music/mus_stage3.vgm"
XGM2 mus_stage4 "music/mus_stage4.vgm"
XGM2 mus_stage5 "music/mus_stage5.vgm"
XGM2 mus_stage6 "music/mus_stage6.vgm"
XGM2 mus_stage7 "music/mus_stage7.vgm"
XGM2 mus_stage8 "music/mus_stage8.vgm"
XGM2 mus_stage9 "music/mus_stage9.vgm"
XGM2 mus_stage10 "music/mus_stage10.vgm"
XGM2 mus_stage11 "music/mus_stage11.vgm"
XGM2 mus_stage12 "music/mus_stage12.vgm"
XGM2 mus_stage13 "music/mus_stage13.vgm"

# ---- PLAYER (RockBot) ----
# 4 cols x 7 anim rows, 32x32 cells. Green/magenta placeholders already swapped
# to weapon_colors[0] by tools/convert_player.py.
# rows: STAND, WALK, JUMP, ATTACK, HIT, TELEPORT, STAIRS, SLIDE, STAIRS_ATTACK
SPRITE player_p1_sprite "player_p1.png" 32p 32p FAST [[120,6][9,9,9,9][9][9][6,6][12,6,6][0,0][0][9][100][200][200,200][9,9,9][5]]
SPRITE player_p2_sprite "player_p2.png" 32p 32p FAST [[120,6][9,9,9,9][9][9][6,6][12,6,6][0,0][0][9][100][200][200,200][9,9,9][5]]
SPRITE player_p3_sprite "player_p3.png" 32p 32p FAST [[120,6][9,9,9,9][9][9][6,6][12,6,6][0,0][0][9][100][200][200,200][9,9,9][5]]
SPRITE player_p4_sprite "player_p4.png" 32p 32p FAST [[120,6][9,9,9,9][9][9][6,6][12,6,6][0,0][0][9][100][200][200,200][9,9,9][5]]

# ---- STAGE 0 ENEMIES + PLAYER SHOT (tools/convert_enemies.py) ----
# Generated sheets: row 0 = ANIM_STAND, row 1 = ANIM_WALK. Cells are padded
# to multiples of 8 (SGDK requirement). Shared PAL0 palette
# (palette_ui_enemies in stage0_enemies.c): index 0 transparent, 15 = white.
SPRITE enemy_shield_sprite "enemy_shield.png" 32p 32p FAST [[12,12][12,12][0][11,11,11,11,11]]
SPRITE enemy_spinner_ball_red_sprite "enemy_spinner_ball_red.png" 24p 24p FAST [[1][6,6,6,6,6,6][12,12,48,12]]
SPRITE enemy_spinner_ball_sprite "enemy_spinner_ball.png" 24p 24p FAST [[6,6,6,6,6,6][6,6,6,6,6,6][6,6,6,30]]
SPRITE enemy_classic_metool_sprite "enemy_classic_metool.png" 24p 24p FAST [[12][12,12][9,9,30,6,9,9]]
SPRITE enemy_cannon_ball_sprite "enemy_cannon_ball.png" 24p 24p FAST [[120][12,12][12,12,60]]
# Stage boss MILONGA BOT (id 37): 36x33 art in 40x40 cells, stand + walk rows
SPRITE enemy_milonga_sprite "enemy_milonga.png" 40p 40p FAST [[120][120][12,48][120][12,36][120][120]]
SPRITE shot_sprite "shot.png" 8p 8p FAST [[1]]
# Player + enemy charged projectiles (game_projectile_list ids 11/10).
# Frames are 100ms each (defines.h:644); semi art restored by the enemy
# converter (charged_semi.png), full by charged_shot.png.
SPRITE enemy_shot_semi_sprite "enemy_shot_semi.png" 16p 16p FAST [[6,6]]
SPRITE shot_charge_full_sprite "shot_charge_full.png" 32p 32p FAST [[6,6]]
# enemy death puff (PC classmap.cpp:2232 add_animation 80ms x2)
SPRITE explosion_sprite "explosion.png" 32p 32p FAST [[5]]
# boss-death ring (PC game::classic_style_draw_boss_death): 5 frames of 24x24,
# 8 sprites travel outward from the centre. tools/convert_assets.py.
# bomb / fall-bomb explosion (PC graphicsLib::draw_explosion): 7 frames of 56x56, 100ms
SPRITE bomb_explosion_sprite "bomb_explosion.png" 56p 56p FAST [[6,6,6,6,6,6,6]]
SPRITE death_explosion_sprite "death_explosion.png" 24p 24p FAST [[3,3,3,3,3]]

// Transient effects (src/effects.c), PAL0, art from the PC's images/:
// animations/dash_dust.png (3 frames of 8x16), animations/water_splash.png
// (4 x 32x23), sprites/teleport_small.png (2 x 32x32), animations/bubble.png.
SPRITE fx_dust_sprite "fx_dust.png" 8p 8p FAST [[4,4,4]]
SPRITE fx_splash_sprite "fx_splash.png" 32p 24p FAST [[5,5,5,5]]
SPRITE fx_teleport_sprite "fx_teleport.png" 32p 32p FAST [[6,6]]
SPRITE fx_bubble_sprite "fx_bubble.png" 8p 8p FAST [[1]]
SPRITE fx_flash_sprite "fx_flash.png" 8p 8p FAST [[4,4,4]]

SPRITE enemy_rhino_sprite "enemy_rhino.png" 96p 72p FAST [[540][540][0]]
# ---- ENEMY PROJECTILES (tools/convert_enemies.py): one SPRITE per
# projectile id, art from game_projectile_list.dat graphic_filename.
# The enemy_proj[] table in stage0_enemies.c references these.
SPRITE proj_0_sprite "proj_0.png" 8p 8p FAST [[0]]
SPRITE proj_1_sprite "proj_1.png" 24p 24p FAST [[6,6,6,6]]
SPRITE proj_2_sprite "proj_2.png" 24p 24p FAST [[0]]
SPRITE proj_3_sprite "proj_3.png" 16p 16p FAST [[6,6,6,6,6]]
SPRITE proj_4_sprite "proj_4.png" 56p 56p FAST [[3,3,3,3,3,3,3,3,3]]
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
SPRITE proj_20_sprite "proj_20.png" 32p 32p FAST [[6,6]]
SPRITE proj_21_sprite "proj_21.png" 24p 24p FAST [[6,6]]
SPRITE proj_22_sprite "proj_22.png" 32p 24p FAST [[6,6,6,6]]

# ---- STAGE 0 OBJECTS (tools/convert_objects.py) ----
# Single anim row; shared PAL0 (palette_ui_enemies). Checkpoint cell is
# 32x64 (SGDK splits it into two 32x32 hardware sprites).
SPRITE object_energy_pill_small_sprite "object_energy_pill_small.png" 16p 16p FAST [[6,6]]
SPRITE object_fall_block_sprite "object_fall_block.png" 16p 16p FAST [[6,6]]
SPRITE object_1up_sprite "object_1up.png" 16p 16p FAST [[12,12,12,12]]
SPRITE object_item_jump_sprite "object_item_jump.png" 32p 24p FAST [[2,2,2,2,2]]
SPRITE object_item_jet_sprite "object_item_jet.png" 40p 16p FAST [[18,18,18,18]]
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
# Weapon gauge at x=62 (PC draw.cpp:1217); the PC tints it with the selected
# weapon's color1 at runtime, the port keeps the sheet's own colour
SPRITE hud_wpn_ball_sprite "hud_wpn_ball.png" 16p 16p FAST [[0,0,0,0,0]]

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
SPRITE ssel_castle_sprite "ssel_castle.png" 32p 32p FAST [[0]]
SPRITE ssel_eyes_p4_sprite "ssel_eyes_p4.png" 16p 8p FAST [[0]]

# ---- SAVE-SLOT ICONS (tools/convert_save_icons.py: PC images/icons.png,
# 18 icons x 16x16, 2 frames = disabled/enabled). PAL3 on that screen.
PALETTE sicon_pal "sicon_00.png"
SPRITE sicon_00_sprite "sicon_00.png" 16p 16p FAST 0
SPRITE sicon_01_sprite "sicon_01.png" 16p 16p FAST 0
SPRITE sicon_02_sprite "sicon_02.png" 16p 16p FAST 0
SPRITE sicon_03_sprite "sicon_03.png" 16p 16p FAST 0
SPRITE sicon_04_sprite "sicon_04.png" 16p 16p FAST 0
SPRITE sicon_05_sprite "sicon_05.png" 16p 16p FAST 0
SPRITE sicon_06_sprite "sicon_06.png" 16p 16p FAST 0
SPRITE sicon_07_sprite "sicon_07.png" 16p 16p FAST 0
SPRITE sicon_08_sprite "sicon_08.png" 16p 16p FAST 0
SPRITE sicon_14_sprite "sicon_14.png" 16p 16p FAST 0
SPRITE sicon_15_sprite "sicon_15.png" 16p 16p FAST 0
SPRITE sicon_16_sprite "sicon_16.png" 16p 16p FAST 0
SPRITE sicon_17_sprite "sicon_17.png" 16p 16p FAST 0
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
SPRITE enemy_apebot_sprite "enemy_apebot.png" 40p 40p FAST [[48][12,12][24,24,24,24][12][12,12,12,120]]
SPRITE enemy_bee_sprite "enemy_bee.png" 24p 24p FAST [[6,6][6,6][0][3]]
SPRITE enemy_flower_metool_sprite "enemy_flower_metool.png" 24p 24p FAST [[1][12,12][9,9,30,6,9,9]]
SPRITE enemy_side_shooter_sprite "enemy_side_shooter.png" 16p 32p FAST [[6][6][48]]
SPRITE enemy_tamandua_sprite "enemy_tamandua.png" 40p 32p FAST [[18,18][12,12][0]]
SPRITE enemy_scorpion_sprite "enemy_scorpion.png" 24p 32p FAST [[120][9,8][120]]
SPRITE enemy_larva_sprite "enemy_larva.png" 40p 16p FAST [[12][10,10,10,10][0]]
TILESET stage1_tiles0 "stage1_map0.png"  NONE ALL
PALETTE stage1_pal0   "stage1_map0.png"
MAP     stage1_map0   "stage1_map0.png"  stage1_tiles0 NONE 0
TILESET stage1_tiles1 "stage1_map1.png"  NONE ALL
PALETTE stage1_pal1   "stage1_map1.png"
MAP     stage1_map1   "stage1_map1.png"  stage1_tiles1 NONE 0
TILESET stage1_tiles2 "stage1_map2.png"  NONE ALL
PALETTE stage1_pal2   "stage1_map2.png"
MAP     stage1_map2   "stage1_map2.png"  stage1_tiles2 NONE 0
PALETTE stage1_bg0_pal "stage1_bg0_tiles.png"
PALETTE stage1_bg1_pal "stage1_bg1_tiles.png"
PALETTE stage1_bg2_pal "stage1_bg2_tiles.png"

# ---- STAGE 2 (FLOWER FIELDS) ----
SPRITE enemy_flower_vase_sprite "enemy_flower_vase.png" 24p 24p FAST [[120,18][120,18][24,180]]
SPRITE enemy_daisiebot_sprite "enemy_daisiebot.png" 32p 40p FAST [[12][7,7,7,7][12][12][6][12,12,12,12,60]]
SPRITE enemy_roof_shooter_sprite "enemy_roof_shooter.png" 16p 16p FAST [[6,6,6,6,6,6][6,6,6,6,6,6][1,6]]
SPRITE enemy_ema_sprite "enemy_ema.png" 40p 48p FAST [[6,6,6][6,6,6][0]]
SPRITE object_platform_down_sprite "object_platform_down.png" 32p 16p FAST [[6,6]]
TILESET stage2_tiles0 "stage2_map0.png"  NONE ALL
PALETTE stage2_pal0   "stage2_map0.png"
MAP     stage2_map0   "stage2_map0.png"  stage2_tiles0 NONE 0
TILESET stage2_tiles1 "stage2_map1.png"  NONE ALL
PALETTE stage2_pal1   "stage2_map1.png"
MAP     stage2_map1   "stage2_map1.png"  stage2_tiles1 NONE 0
TILESET stage2_tiles2 "stage2_map2.png"  NONE ALL
PALETTE stage2_pal2   "stage2_map2.png"
MAP     stage2_map2   "stage2_map2.png"  stage2_tiles2 NONE 0
PALETTE stage2_bg0_pal "stage2_bg0_tiles.png"
PALETTE stage2_bg1_pal "stage2_bg1_tiles.png"
PALETTE stage2_bg2_pal "stage2_bg2_tiles.png"

# ---- STAGE 3 (ATLANTIDA) ----
SPRITE enemy_bat_sprite "enemy_bat.png" 32p 24p FAST [[48][12,12,12,12][120][18,18]]
SPRITE enemy_seahorsebot_sprite "enemy_seahorsebot.png" 32p 40p FAST [[12,12][12,12][72][12,6,6,6,6,6,36][7,7,7,7]]
SPRITE enemy_giant_dolphin_sprite "enemy_giant_dolphin.png" 104p 64p FAST [[3,3][6,6][3,3][6,6,6,6]]
SPRITE enemy_small_fish_sprite "enemy_small_fish.png" 24p 16p FAST [[12,12,12][6,6,6][0]]
SPRITE enemy_killer_fish_sprite "enemy_killer_fish.png" 32p 32p FAST [[6,6][6][0]]
SPRITE enemy_spiked_floor_sprite "enemy_spiked_floor.png" 24p 16p FAST [[12,12][12,12][0]]
SPRITE enemy_wood_shield_sprite "enemy_wood_shield.png" 32p 32p FAST [[12,12][12,12][0][11,11,11,11,11]]
TILESET stage3_tiles0 "stage3_map0.png"  NONE ALL
PALETTE stage3_pal0   "stage3_map0.png"
MAP     stage3_map0   "stage3_map0.png"  stage3_tiles0 NONE 0
TILESET stage3_tiles1 "stage3_map1.png"  NONE ALL
PALETTE stage3_pal1   "stage3_map1.png"
MAP     stage3_map1   "stage3_map1.png"  stage3_tiles1 NONE 0
TILESET stage3_tiles2 "stage3_map2.png"  NONE ALL
PALETTE stage3_pal2   "stage3_map2.png"
MAP     stage3_map2   "stage3_map2.png"  stage3_tiles2 NONE 0
PALETTE stage3_bg0_pal "stage3_bg0_tiles.png"
PALETTE stage3_bg1_pal "stage3_bg1_tiles.png"
PALETTE stage3_bg2_pal "stage3_bg2_tiles.png"

# ---- STAGE 4 ----
TILESET stage4_tiles0 "stage4_map0.png"  NONE ALL
PALETTE stage4_pal0   "stage4_map0.png"
MAP     stage4_map0   "stage4_map0.png"  stage4_tiles0 NONE 0
TILESET stage4_tiles1 "stage4_map1.png"  NONE ALL
PALETTE stage4_pal1   "stage4_map1.png"
MAP     stage4_map1   "stage4_map1.png"  stage4_tiles1 NONE 0
TILESET stage4_tiles2 "stage4_map2.png"  NONE ALL
PALETTE stage4_pal2   "stage4_map2.png"
MAP     stage4_map2   "stage4_map2.png"  stage4_tiles2 NONE 0
PALETTE stage4_bg0_pal "stage4_bg0_tiles.png"
PALETTE stage4_bg1_pal "stage4_bg1_tiles.png"
PALETTE stage4_bg2_pal "stage4_bg2_tiles.png"

# ---- STAGE 5 ----
SPRITE enemy_magebot_sprite "enemy_magebot.png" 32p 40p FAST [[120][12,12][18,18][120][12,12,12,36][120]]
SPRITE enemy_bunny_sprite "enemy_bunny.png" 32p 40p FAST [[24][6][0]]
SPRITE enemy_top_hat_sprite "enemy_top_hat.png" 24p 16p FAST [[120][120][0]]
SPRITE enemy_gambit_sprite "enemy_gambit.png" 40p 40p FAST [[120][120][12,12,12,12,36]]
SPRITE enemy_metool_tophat_sprite "enemy_metool_tophat.png" 24p 24p FAST [[120][12,12][9,9,30,9,9,9]]
TILESET stage5_tiles0 "stage5_map0.png"  NONE ALL
PALETTE stage5_pal0   "stage5_map0.png"
MAP     stage5_map0   "stage5_map0.png"  stage5_tiles0 NONE 0
TILESET stage5_tiles1 "stage5_map1.png"  NONE ALL
PALETTE stage5_pal1   "stage5_map1.png"
MAP     stage5_map1   "stage5_map1.png"  stage5_tiles1 NONE 0
TILESET stage5_tiles2 "stage5_map2.png"  NONE ALL
PALETTE stage5_pal2   "stage5_map2.png"
MAP     stage5_map2   "stage5_map2.png"  stage5_tiles2 NONE 0
PALETTE stage5_bg0_pal "stage5_bg0_tiles.png"
PALETTE stage5_bg1_pal "stage5_bg1_tiles.png"
PALETTE stage5_bg2_pal "stage5_bg2_tiles.png"

# ---- STAGE 6 ----
SPRITE enemy_dynamitebot_sprite "enemy_dynamitebot.png" 48p 40p FAST [[12,18][18,18][12,12,12][1][12,12,12,12][12,12]]
SPRITE enemy_spinning_blade_sprite "enemy_spinning_blade.png" 24p 24p FAST [[15,15][6,6][0]]
SPRITE enemy_mine_cart_sprite "enemy_mine_cart.png" 40p 40p FAST [[7,7][6,6][0]]
TILESET stage6_tiles0 "stage6_map0.png"  NONE ALL
PALETTE stage6_pal0   "stage6_map0.png"
MAP     stage6_map0   "stage6_map0.png"  stage6_tiles0 NONE 0
TILESET stage6_tiles1 "stage6_map1.png"  NONE ALL
PALETTE stage6_pal1   "stage6_map1.png"
MAP     stage6_map1   "stage6_map1.png"  stage6_tiles1 NONE 0
TILESET stage6_tiles2 "stage6_map2.png"  NONE ALL
PALETTE stage6_pal2   "stage6_map2.png"
MAP     stage6_map2   "stage6_map2.png"  stage6_tiles2 NONE 0
PALETTE stage6_bg0_pal "stage6_bg0_tiles.png"
PALETTE stage6_bg1_pal "stage6_bg1_tiles.png"
PALETTE stage6_bg2_pal "stage6_bg2_tiles.png"

# ---- STAGE 7 ----
SPRITE enemy_spikebot_sprite "enemy_spikebot.png" 40p 40p FAST [[12,12][6,6][120][120][18,18,36][120][180][180]]
SPRITE enemy_driller_sprite "enemy_driller.png" 32p 24p FAST [[6,6][6,6][0]]
SPRITE enemy_metal_metool_sprite "enemy_metal_metool.png" 24p 24p FAST [[120][12,12][9,9,30,6,9,9]]
TILESET stage7_tiles0 "stage7_map0.png"  NONE ALL
PALETTE stage7_pal0   "stage7_map0.png"
MAP     stage7_map0   "stage7_map0.png"  stage7_tiles0 NONE 0
TILESET stage7_tiles1 "stage7_map1.png"  NONE ALL
PALETTE stage7_pal1   "stage7_map1.png"
MAP     stage7_map1   "stage7_map1.png"  stage7_tiles1 NONE 0
TILESET stage7_tiles2 "stage7_map2.png"  NONE ALL
PALETTE stage7_pal2   "stage7_map2.png"
MAP     stage7_map2   "stage7_map2.png"  stage7_tiles2 NONE 0
PALETTE stage7_bg0_pal "stage7_bg0_tiles.png"
PALETTE stage7_bg1_pal "stage7_bg1_tiles.png"
PALETTE stage7_bg2_pal "stage7_bg2_tiles.png"

# ---- STAGE 8 ----
SPRITE enemy_technobot_sprite "enemy_technobot.png" 48p 40p FAST [[120,1,12,12][120,1,12,12][120][120][6,6,6,6,12,30][120]]
SPRITE enemy_ground_shooter_sprite "enemy_ground_shooter.png" 16p 16p FAST [[6,6,6,6,6,6][6,6,6,6,6,6][1,6]]
SPRITE enemy_android_sprite "enemy_android.png" 24p 24p FAST [[600][6,6,6][600]]
TILESET stage8_tiles0 "stage8_map0.png"  NONE ALL
PALETTE stage8_pal0   "stage8_map0.png"
MAP     stage8_map0   "stage8_map0.png"  stage8_tiles0 NONE 0
TILESET stage8_tiles1 "stage8_map1.png"  NONE ALL
PALETTE stage8_pal1   "stage8_map1.png"
MAP     stage8_map1   "stage8_map1.png"  stage8_tiles1 NONE 0
TILESET stage8_tiles2 "stage8_map2.png"  NONE ALL
PALETTE stage8_pal2   "stage8_map2.png"
MAP     stage8_map2   "stage8_map2.png"  stage8_tiles2 NONE 0
PALETTE stage8_bg0_pal "stage8_bg0_tiles.png"
PALETTE stage8_bg1_pal "stage8_bg1_tiles.png"
PALETTE stage8_bg2_pal "stage8_bg2_tiles.png"

# ---- STAGE 9 (LAVA PIT) ----
SPRITE enemy_spinning_drill_sprite "enemy_spinning_drill.png" 16p 24p FAST [[3,3,3][6,6,6][0]]
SPRITE enemy_tech_circle_sprite "enemy_tech_circle.png" 64p 64p FAST [[60,6,6,6,6,6][6,6,6,6,6,6][0][1][6,6,6,6,6,6,6]]
TILESET stage9_tiles0 "stage9_map0.png"  NONE ALL
PALETTE stage9_pal0   "stage9_map0.png"
MAP     stage9_map0   "stage9_map0.png"  stage9_tiles0 NONE 0
TILESET stage9_tiles1 "stage9_map1.png"  NONE ALL
PALETTE stage9_pal1   "stage9_map1.png"
MAP     stage9_map1   "stage9_map1.png"  stage9_tiles1 NONE 0
TILESET stage9_tiles2 "stage9_map2.png"  NONE ALL
PALETTE stage9_pal2   "stage9_map2.png"
MAP     stage9_map2   "stage9_map2.png"  stage9_tiles2 NONE 0
PALETTE stage9_bg0_pal "stage9_bg0_tiles.png"
PALETTE stage9_bg1_pal "stage9_bg1_tiles.png"
PALETTE stage9_bg2_pal "stage9_bg2_tiles.png"

# ---- STAGE 10 (SPACE BASE) ----
SPRITE enemy_giant_metool_sprite "enemy_giant_metool.png" 104p 120p FAST [[540][6][6,60][12,600][24,6,24]]
TILESET stage10_tiles0 "stage10_map0.png"  NONE ALL
PALETTE stage10_pal0   "stage10_map0.png"
MAP     stage10_map0   "stage10_map0.png"  stage10_tiles0 NONE 0
TILESET stage10_tiles1 "stage10_map1.png"  NONE ALL
PALETTE stage10_pal1   "stage10_map1.png"
MAP     stage10_map1   "stage10_map1.png"  stage10_tiles1 NONE 0
TILESET stage10_tiles2 "stage10_map2.png"  NONE ALL
PALETTE stage10_pal2   "stage10_map2.png"
MAP     stage10_map2   "stage10_map2.png"  stage10_tiles2 NONE 0
PALETTE stage10_bg0_pal "stage10_bg0_tiles.png"
PALETTE stage10_bg1_pal "stage10_bg1_tiles.png"
PALETTE stage10_bg2_pal "stage10_bg2_tiles.png"

# ---- STAGE 11 (POWER GRID) ----
SPRITE enemy_tank_sprite "enemy_tank.png" 104p 80p FAST [[6,6][6,6][90][1][6,6,6,6]]
TILESET stage11_tiles0 "stage11_map0.png"  NONE ALL
PALETTE stage11_pal0   "stage11_map0.png"
MAP     stage11_map0   "stage11_map0.png"  stage11_tiles0 NONE 0
TILESET stage11_tiles1 "stage11_map1.png"  NONE ALL
PALETTE stage11_pal1   "stage11_map1.png"
MAP     stage11_map1   "stage11_map1.png"  stage11_tiles1 NONE 0
TILESET stage11_tiles2 "stage11_map2.png"  NONE ALL
PALETTE stage11_pal2   "stage11_map2.png"
MAP     stage11_map2   "stage11_map2.png"  stage11_tiles2 NONE 0
PALETTE stage11_bg0_pal "stage11_bg0_tiles.png"
PALETTE stage11_bg1_pal "stage11_bg1_tiles.png"
PALETTE stage11_bg2_pal "stage11_bg2_tiles.png"

# ---- STAGE 12 (RECYCLE FACTORY) ----
SPRITE enemy_giant_bee_sprite "enemy_giant_bee.png" 112p 72p FAST [[3,3][4,4][3,3][6,6,6,6]]
TILESET stage12_tiles0 "stage12_map0.png"  NONE ALL
PALETTE stage12_pal0   "stage12_map0.png"
MAP     stage12_map0   "stage12_map0.png"  stage12_tiles0 NONE 0
TILESET stage12_tiles1 "stage12_map1.png"  NONE ALL
PALETTE stage12_pal1   "stage12_map1.png"
MAP     stage12_map1   "stage12_map1.png"  stage12_tiles1 NONE 0
TILESET stage12_tiles2 "stage12_map2.png"  NONE ALL
PALETTE stage12_pal2   "stage12_map2.png"
MAP     stage12_map2   "stage12_map2.png"  stage12_tiles2 NONE 0
PALETTE stage12_bg0_pal "stage12_bg0_tiles.png"
PALETTE stage12_bg1_pal "stage12_bg1_tiles.png"
PALETTE stage12_bg2_pal "stage12_bg2_tiles.png"

# ---- STAGE 13 (FROZEN LAIR) ----
SPRITE enemy_destrin_tank_sprite "enemy_destrin_tank.png" 88p 96p FAST [[6,6,6][6,6,6][6][1]]
SPRITE enemy_destrin_ship_sprite "enemy_destrin_ship.png" 48p 48p FAST [[12,12][18,18][6]]
SPRITE enemy_multidrill_sprite "enemy_multidrill.png" 16p 24p FAST [[6,6,6][6,6,6][0]]
SPRITE enemy_shocker_sprite "enemy_shocker.png" 40p 40p FAST [[12,12][12,12,12][7,7,48]]
TILESET stage13_tiles0 "stage13_map0.png"  NONE ALL
PALETTE stage13_pal0   "stage13_map0.png"
MAP     stage13_map0   "stage13_map0.png"  stage13_tiles0 NONE 0
TILESET stage13_tiles1 "stage13_map1.png"  NONE ALL
PALETTE stage13_pal1   "stage13_map1.png"
MAP     stage13_map1   "stage13_map1.png"  stage13_tiles1 NONE 0
TILESET stage13_tiles2 "stage13_map2.png"  NONE ALL
PALETTE stage13_pal2   "stage13_map2.png"
MAP     stage13_map2   "stage13_map2.png"  stage13_tiles2 NONE 0
PALETTE stage13_bg0_pal "stage13_bg0_tiles.png"
PALETTE stage13_bg1_pal "stage13_bg1_tiles.png"
PALETTE stage13_bg2_pal "stage13_bg2_tiles.png"

# ---- CONVERTED STAGES END (tools/add_stage.py inserts above) ----


# ---- sprites missed by the first pass (converter assert fixed) ----
SPRITE enemy_mummybot_sprite "enemy_mummybot.png" 40p 40p FAST [[120][120][120][120][6,6,6,6,6,6,6,6,6,6][12,12,120][48]]
SPRITE enemy_giant_fly_sprite "enemy_giant_fly.png" 64p 48p FAST [[9,9][6,6][0][12,12,12,12]]
SPRITE enemy_fly_sprite "enemy_fly.png" 32p 24p FAST [[4,4,4,4][6,6,6,6][0]]

# ---- objects/pickups added with the Techno fixes ----
SPRITE object_energy_pill_big_sprite "object_energy_pill_big.png" 16p 16p FAST [[6,6]]
SPRITE object_weapons_pill_big_sprite "object_weapons_pill_big.png" 16p 16p FAST [[6,6]]
SPRITE object_disappearing_block_sprite "object_disappearing_block.png" 16p 16p FAST [[4,4,4,4,4]]
SPRITE object_disappearing_block_green_sprite "object_disappearing_block_green.png" 16p 16p FAST [[4,4,4,4,4]]
SPRITE object_disappearing_block_blue_sprite "object_disappearing_block_blue.png" 16p 16p FAST [[4,4,4,4,4]]
SPRITE object_weapons_pill_small_sprite "object_weapons_pill_small.png" 16p 16p FAST [[6,6]]

# ---- tanks / pickups added by the data audit ----
SPRITE object_energy_tank_sprite "object_energy_tank.png" 16p 16p FAST [[18,18]]
SPRITE object_weapons_tank_sprite "object_weapons_tank.png" 16p 16p FAST [[18,18]]

# ---- active disappearing blocks ----
SPRITE object_disappearing_block_orange_sprite "object_disappearing_block_orange.png" 16p 16p FAST [[4,4,4,4,4]]

# ---- remaining object types (audit) ----
SPRITE object_armor_body_sprite "object_armor_body.png" 24p 16p FAST [[566]]
# boss doors: 16x16 slices, ONE ANIMATION PER SLICE (object.c picks slice i
# from the centre outward; the PC paints the door as map tiles)
SPRITE object_door_4_sprite "object_door_4.png" 16p 16p FAST [[0][0][0][0]]
SPRITE object_door_3_sprite "object_door_3.png" 16p 16p FAST [[0][0][0]]
SPRITE object_armor_legs_sprite "object_armor_legs.png" 24p 16p FAST [[566]]
SPRITE object_block003_sprite "object_block003.png" 32p 32p FAST [[1]]
SPRITE object_special_tank_sprite "object_special_tank.png" 16p 16p FAST [[566,566]]
SPRITE object_armor_arms_sprite "object_armor_arms.png" 24p 16p FAST [[566]]
# fire rays: every (flicker, length[, side]) slice baked as a frame (tools/convert_objects.py)
SPRITE object_fire_ray_vertical_sprite "object_fire_ray_vertical.png" 16p 80p FAST [[1,1,1,1,1,1,1,1,1,1]]
SPRITE object_fire_ray_horizontal_sprite "object_fire_ray_horizontal.png" 80p 16p FAST [[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]]
SPRITE object_lava_beam_sprite "object_lava_beam.png" 48p 240p FAST [[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]]
SPRITE object_deathray_small_horz_sprite "object_deathray_small_horz.png" 240p 16p FAST [[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]]
SPRITE object_platform_sprite "object_platform.png" 16p 16p FAST [[6]]
SPRITE object_teleporter_sprite "object_teleporter.png" 32p 64p FAST [[9,9,9,9]]
SPRITE object_teleport_base_sprite "object_teleport_base.png" 32p 16p FAST [[4,4,4,4,4]]
SPRITE proj_7_sprite "proj_7.png" 72p 8p FAST [[6,6,6,6,6,6,6,6,6]]
SPRITE proj_23_sprite "proj_23.png" 16p 8p FAST [[0]]
SPRITE proj_24_sprite "proj_24.png" 32p 16p FAST [[6,6]]
SPRITE proj_25_sprite "proj_25.png" 8p 8p FAST [[0]]
SPRITE proj_26_sprite "proj_26.png" 8p 8p FAST [[0]]
SPRITE proj_30_sprite "proj_30.png" 24p 24p FAST [[2,2]]
SPRITE proj_31_sprite "proj_31.png" 16p 16p FAST [[6,6]]
SPRITE proj_32_sprite "proj_32.png" 24p 24p FAST [[6,6,6,6]]
SPRITE proj_33_sprite "proj_33.png" 48p 32p FAST [[6,6]]
SPRITE proj_34_sprite "proj_34.png" 16p 16p FAST [[6,6,6,6,6,6,6]]
SPRITE proj_35_sprite "proj_35.png" 24p 24p FAST [[6,6,6,6]]
