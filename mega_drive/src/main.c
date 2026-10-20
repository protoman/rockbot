#include <genesis.h>

#include "resources.h"

#include "stage.h"
#include "player.h"
#include "enemy.h"
#include "object.h"
#include "shot.h"
#include "explosion.h"
#include "eshot.h"
#include "hud.h"
#include "menu.h"
#include "save.h"
#include "stageselect.h"
#include "saveselect.h"
#include "bossintro.h"
#include "playerselect.h"
#include "transition.h"
#include "sfx.h"
#include "scene.h"
#include "ending.h"
#include "config.h"
#include "dialogs.h"
#include "weapon_data.h"
#include "title64_map.h"
#include "title64_tiles.h"
#include "title64_pal.h"

// Layout derived from the original game, adapted from 320x240 to 320x224.
//   show_notice()  = game.cpp:506  -> intro logos (sprites)
//   draw_main()    = sceneslib.cpp:102 -> title screen (BG image + text)
//   start_stage()  = game.cpp:251  -> INTRO_STAGE (CHAOS CITY)

// Font (VDP_drawText) uses PAL0, so sprites/image take the other lines.
#define PAL_UPPER PAL1
#define PAL_BOBER PAL2
#define PAL_JMD   PAL3
#define PAL_TITLE PAL1   // sprites are released before the title screen

#define SCREEN_TILE_W 40

#define UPPER_X  ((320 - 96) / 2)
#define UPPER_Y  40
#define BOBER_X  20
#define BOBER_Y  100
#define JMD_X    (320 - 96 - 20)
#define JMD_Y    100

#define FRAME_COUNT 6

// The 8px CONFIG_BGCOLOR band above the logo art is baked into title_logo.png
// by tools/convert_assets.py (BAND_RGB).

#define VERSION_STRING "2.22.102"
#define COPYRIGHT_STRING "(C) 2009-2026 UPPERLAND STUDIOS"
#define SITE_STRING "HTTPS://ROCKBOT.UPPERLAND.NET"

static Sprite *sprUpper;
static Sprite *sprBober;
static Sprite *sprJmd;

// Screen is 40x28 tiles (320x224 / 8). VDP_drawText takes TILE coords, not pixels.
static void drawCentered(const char *text, u16 tileY)
{
    VDP_drawText(text, (SCREEN_TILE_W - strlen(text)) / 2, tileY);
}

// Wait N frames at 60Hz. SPR_update() is mandatory: the sprite engine is
// deferred, it only really draws here. See lessons_learned.md #10.
static void waitFrames(u16 n)
{
    while (n--) {
        SPR_update();
        SYS_doVBlankProcess();
        // DMA setup clobbers reg 17/18 (shared between window position and
        // DMA source address). Restore the disabled window every frame or
        // the window plane covers part of the screen with garbage.
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
}

static void showIntro(void)
{
    VDP_clearPlane(BG_A, FALSE);

    sprUpper = SPR_addSprite(&upperland_sprite, UPPER_X, UPPER_Y, TILE_ATTR(PAL_UPPER, FALSE, FALSE, FALSE));
    sprBober = SPR_addSprite(&boberatu_sprite, BOBER_X, BOBER_Y, TILE_ATTR(PAL_BOBER, FALSE, FALSE, FALSE));
    sprJmd   = SPR_addSprite(&jmd_sprite, JMD_X, JMD_Y, TILE_ATTR(PAL_JMD, FALSE, FALSE, FALSE));

    PAL_setPalette(PAL_UPPER, upperland_sprite.palette->data, DMA_QUEUE);
    PAL_setPalette(PAL_BOBER, boberatu_sprite.palette->data, DMA_QUEUE);
    PAL_setPalette(PAL_JMD, jmd_sprite.palette->data, DMA_QUEUE);

    drawCentered("HOMEBREW GAMES TEAM", 8);
    // under the JMD logo (which occupies y=JMD_Y..JMD_Y+32 at x=JMD_X);
    // the intro places its elements in MD space, so this is y = 136
    VDP_drawText("JMD AMIGA MUSIC", 23, 17);
    drawCentered("PRESENTS", 22);

    waitFrames(24); // ~400ms

    // upperland strip animation: 30ms per frame (~2 frames at 60Hz)
    for (u16 i = 1; i < FRAME_COUNT; i++) {
        SPR_setFrame(sprUpper, i);
        waitFrames(2);
    }
    SPR_setFrame(sprUpper, 0);

    drawCentered("HTTPS://TWITTER.COM/BOBERATU", 24);
    drawCentered("HTTPS://JMDAMIGAMUSIC.BANDCAMP.COM", 25);
    drawCentered("HTTPS://ROCKBOT.UPPERLAND.NET", 26);

    waitFrames(192); // ~3200ms
}

// Title screen. Logo on BG_B, text on BG_A so the font's transparent pixels
// reveal the logo instead of a solid backdrop.
// The logo comes from the decomposed PC art (tools/img2tiles.py, 350 unique
// tiles packed as src/title_tiles.h/.c). The old rescomp IMAGE + VDP_drawImageEx
// re-drew the logo wrong whenever the title was re-entered from the stage; the
// explicit 4bpp upload + tilemap is deterministic in both cases.
#define TITLE_TILE_BASE TILE_USER_INDEX

static s16 showTitle(void)
{
    SPR_end();                 // release sprite VRAM so the image fits
    // Let any scheduled MAP update flush BEFORE we take over the planes,
    // then drop the map so nothing repaints them afterwards.
    SYS_doVBlankProcess();
    stage_release_map();

    // The title can be reached from the stage (in-game menu -> leave stage),
    // which leaves both planes scrolled to the camera/parallax; the logo and
    // the text need them at 0.
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_B, 0);

    // Wipe BG_A with the font's space tile: VDP_clearPlane left the intro
    // texts ("HOMEBREW GAMES TEAM", "JMD AMIGA MUSIC", ...) on screen.
    VDP_fillTileMapRect(BG_A,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_FONT_INDEX),
                        0, 0, 40, 28);

    // 60-colour logo (BUG-001): four palette lines, one per 8x8 tile. Tiles
    // and palettes go in before the planes are written (probe order).
    VDP_loadTileData(title64_tiles_data, TITLE_TILE_BASE, TITLE64_TILE_COUNT, DMA);
    for (u8 L = 0; L < 4; L++)
        PAL_setPalette((u16)(PAL0 + L), title64_pal[L], CPU);
    // The logo's colorkey background is the title backdrop; on MD it comes
    // from the font-space tile sheet behind it (PAL0 entry 0), which is
    // black. PC fills the title with CONFIG_BGCOLOR (5,29,65) -> RGB333
    // 0x0210. The font glyphs use entry 15, so text stays white.
    // Line 0 slot 0 is the backdrop (PC CONFIG_BGCOLOR) seen through the logo's
    // transparent pixels; slot 15 is the font's white (emitted as such by
    // tools/title64.py), so VDP_drawText keeps working.
    PAL_setColor(0, 0x0420);   // PC CONFIG_BGCOLOR (5,29,65) -> (0,36,73)
    PAL_setColor(15, 0x0EEE);   // font white: the art keeps its own white at slot 1
    // Pixels at index 0 are transparent on a plane and show CRAM's backdrop
    // entry, so point it at PAL0 slot 0 (the PC CONFIG_BGCOLOR) - otherwise the
    // logo's background is black instead of the title's navy.
    VDP_setBackgroundColor(0);
    // the plane is 64 tiles wide: wipe the whole thing first so cells the
    // 40x28 image does not cover cannot show stale stage data
    VDP_fillTileMapRect(BG_B,
                        TILE_ATTR_FULL(PAL_TITLE, FALSE, FALSE, FALSE, TITLE_TILE_BASE),
                        0, 0, 64, 32);
    static u16 tcell[TITLE64_W * TITLE64_H];
    for (u16 i = 0; i < TITLE64_W * TITLE64_H; i++) {
        // the palette line travels in the map entry (bits 13-14); keep it and
        // add the tileset base
        u16 e = title64_map[i];
        tcell[i] = (u16)((e & ~TILE_INDEX_MASK) |
                         (TITLE_TILE_BASE + (e & TILE_INDEX_MASK)));
    }
    // CPU: the queued variant raced with the neighbouring writes when the
    // title was re-entered (stale cells at the plane edges)
    VDP_setTileMapDataRect(BG_B, tcell, 0, 0, TITLE64_W, TITLE64_H,
                           TITLE64_W, CPU);

    // PC draw_main: version at (8,8) -> tile (1,1), over the logo's top
    VDP_drawText(VERSION_STRING, 1, 1);
    VDP_drawText(COPYRIGHT_STRING, 5, 25);
    drawCentered(SITE_STRING, 27);

    // PC main menu: NEW GAME / LOAD GAME / CONFIG / ABOUT. LOAD only appears
    // when the SRAM holds at least one save slot.
    bool can_load = save_exists();
    static const char *const items[4] = {
        "NEW GAME", "LOAD GAME", "CONFIG", "ABOUT",
    };
    s16 sel = can_load ? 1 : 0;         // default to LOAD when a save exists
    u16 prev = JOY_readJoypad(JOY_1);    // don't pick on a held button
    while (TRUE) {
        for (u8 i = 0; i < 4; i++) {
            bool shown = (i != 1) || can_load;
            VDP_drawText(shown ? items[i] : "         ", 14, 18 + i * 2);
            VDP_drawText((shown && sel == i) ? ">" : " ", 12, 18 + i * 2);
        }
        u16 pad = JOY_readJoypad(JOY_1);
        if ((pad & BUTTON_DOWN) && !(prev & BUTTON_DOWN)) {
            do { sel = (s16)((sel + 1) & 3); } while (sel == 1 && !can_load);
        }
        if ((pad & BUTTON_UP) && !(prev & BUTTON_UP)) {
            do { sel = (s16)((sel + 3) & 3); } while (sel == 1 && !can_load);
        }
        if ((pad & BUTTON_START) && !(prev & BUTTON_START)) break;
        prev = pad;
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
    for (u8 i = 0; i < 4; i++)
        VDP_drawText("          ", 12, 18 + i * 2);
    return sel;
}

// PC ABOUT/credits entry. Compact one-screen version (full credits scrolling
// is part of the ending/credits milestone).
static void showAbout(void)
{
    // PC draw::show_about (graphic/draw.cpp:609): site, bugs mail, Twitter
    // and Facebook, then "press start".
    VDP_fillTileMapRect(BG_A,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_FONT_INDEX),
                        0, 0, 40, 28);
    VDP_fillTileMapRect(BG_B,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_FONT_INDEX),
                        0, 0, 40, 28);
    drawCentered("ROCKBOT", 2);
    drawCentered("HOMEBREW GAMES TEAM", 3);
    drawCentered("SITE:", 7);
    drawCentered("HTTP://ROCKDOT.UPPERLAND.NET", 8);
    drawCentered("BUGS:", 11);
    drawCentered("BUGS@UPPERLAND.NET", 12);
    drawCentered("TWITTER:", 15);
    drawCentered("@ROCKBOTGAME", 16);
    drawCentered("FACEBOOK:", 19);
    drawCentered("FACEBOOK.COM/ROCKBOTGAME", 20);
    drawCentered("PRESS START", 24);
    u16 prev = JOY_readJoypad(JOY_1);
    while (TRUE) {
        u16 pad = JOY_readJoypad(JOY_1);
        if ((pad & (BUTTON_START | BUTTON_A | BUTTON_C)) &&
            !(prev & (BUTTON_START | BUTTON_A | BUTTON_C))) break;
        prev = pad;
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
}

// INTRO_STAGE = stage 0 = "CHAOS CITY" (game.cpp:439 sets currentStage).
// Stage 0 crosses two maps through ladders: link 0 is map 0 (136,14) -> map 1
// (198,0), the only way past the full-height wall at x=137..138.
// runStage outcome: the caller drives the PC stage loop
// (leave_stage -> stage select / game over -> restart intro or select).
typedef enum {
    RUN_LEFT = 0,        // LEAVE STAGE from the pause menu -> title
    RUN_CLEARED,         // boss defeated, stage mask + progress saved
    RUN_GAME_OVER,       // lives exhausted, GAME OVER shown
} RunResult;

static void showStageClear(u8 stage, bool first_clear);
static void showGameOver(void);
static u8 current_save_slot = 0;
static u8 current_player = 0;      // 0..3, chosen in the player-select screen

// Persistent progress for the active slot (PC game_save.items + armor).
static void persist_progress(void)
{
    SaveItems it;
    it.lives = player_lives();
    it.tanks_hp = player.tanks_hp;
    it.tanks_wp = player.tanks_wp;
    it.tanks_sp = player.tanks_sp;
    it.armor = player.armor;
    save_slot_write_items(current_save_slot, &it);
}

static void load_progress(void)
{
    SaveItems it;
    if (save_slot_read_items(current_save_slot, &it)) {
        player_set_progress(it.lives, it.tanks_hp, it.tanks_wp,
                            it.tanks_sp, it.armor);
    } else {
        player_new_game();     // saves from before the progress record
    }
}

static RunResult runStage(u8 stage)
{
    // Pick the stage's data bundle (clamped to the converted stages), then
    // load its map 0. Links/map changes stay within the current stage.
    stage_set_data(stage);

    // PC boss intro (sceneslib.cpp:681) before the stage; skipped once the
    // stage is beaten (game_save.stages[stage] != 0).
    if (!(save_slot_stages(current_save_slot) & (1u << stage))) {
        boss_intro_run(stage);
    }

    player_set_number(current_player);

    // SPR_initEx(504): sprite VRAM = TILE_SPRITE_INDEX(1440) - 504 =
    // 936..1439, right after the animated tiles (end 935). TILE_FONT_INDEX
    // is 1440 (0xC000/32 - FONT_LEN 96), NOT 1952 as older comments said —
    // a bigger pool would overlap the parallax background tiles.
    // The engine only holds the tiles of the frames currently on screen.
    SPR_initEx(504); // released again by showTitle()

    // PAL2 still holds intro/title colors; clear it before the player
    // sprite claims it (player_init loads the player palette over this).
    static const u16 black_pal[16] = {0};
    PAL_setPalette(PAL2, black_pal, DMA_QUEUE);

    stage_load(0);
    player_init();
    hud_init();

    if (config_is_sound_on()) XGM2_play(stage_music);   // stage only

    // ---- stage start sequence (PC game::start_stage, game.cpp:244) --------
    // show_player_teleport(): the player beams in with the TELEPORT animation
    // (frames 0/1 alternating) for 1500ms, then frame 2, then back to STAND.
    // show_ready(): "MISSION STARTED!" blinks 6 times over the frozen stage
    // (200ms per phase). The PC does not read input in either function, so the
    // sequence always plays through.
    {
        if (player.spr) {
            SPR_setAnim(player.spr, ANIM_TELEPORT);
            for (u16 i = 0; i < 90; i++) {                   // ~1500ms
                SPR_setFrame(player.spr, (i / 15) & 1);
                SPR_update();
                SYS_doVBlankProcess();
            }
            SPR_setFrame(player.spr, 2);
            for (u16 i = 0; i < 10; i++) {
                SPR_update();
                SYS_doVBlankProcess();
            }
            SPR_setAnim(player.spr, ANIM_STAND);
        }
        // Three blinks, each with a full second of readable on-screen time
        // (the PC's 200ms flashes were too short on the MD).
        for (u16 blink = 0; blink < 3; blink++) {
            stage_force_redraw();
            // stage_force_redraw queues the map rewrite; flush it before
            // drawing the text or the first vblank painted over the message.
            SYS_doVBlankProcess();
            drawCentered("MISSION STARTED!", 13);
            for (u16 i = 0; i < 30; i++) {                   // 500ms on
                SPR_update();
                SYS_doVBlankProcess();
            }
            stage_force_redraw();
            SYS_doVBlankProcess();
            for (u16 i = 0; i < 8; i++) {                    // 133ms off
                SPR_update();
                SYS_doVBlankProcess();
            }
        }
        // Hold the final message for one more second, then clear the plane.
        stage_force_redraw();
        SYS_doVBlankProcess();
        drawCentered("MISSION STARTED!", 13);
        for (u16 i = 0; i < 30; i++) {
            SPR_update();
            SYS_doVBlankProcess();
        }
        stage_force_redraw();
        player_sync_sprite();
        SYS_doVBlankProcess();
    }

    // NOTE: the old "clear BG_B here" block is gone — stage_load now draws
    // the parallax background on BG_B, and clearing it afterwards wiped the
    // tilemap (sky showed stale/garbage tiles).

    bool boss_dialog_shown = FALSE;
    u16 prev_pad = JOY_readJoypad(JOY_1);  // START edge (a press held from
                                           // the title must not open the menu)
    bool stage_cleared = FALSE;
    bool death_fx = FALSE;                 // puffs already spawned
    bool boss_death_seq = FALSE;
    u16 boss_death_t = 0;
    s16 boss_death_x = 0, boss_death_y = 0;
    while (TRUE) {
        // ---- boss death sequence (PC kill_all_enemies + ring explosion) ---
        if (boss_death_seq) {
            if (boss_death_t < 72 && (boss_death_t % 12) == 0)
                explosion_spawn(boss_death_x + (s16)(boss_death_t % 24) - 12,
                                boss_death_y + (s16)(boss_death_t % 18) - 9);
            explosion_update();
            SPR_update();
            SYS_doVBlankProcess();
            VDP_setReg(17, 0);
            VDP_setReg(18, 0);
            if (++boss_death_t >= 120) { stage_cleared = TRUE; break; }
            continue;
        }

        // ---- death sequence: freeze the world while the player is gone ----
        if (player_is_dying()) {
            if (!death_fx) {
                s16 dl, dt, dr, db;
                player_get_box(&dl, &dt, &dr, &db);
                s16 cx = (dl + dr) / 2, cy = (dt + db) / 2;
                explosion_spawn(cx, cy);
                explosion_spawn(cx - 12, cy);
                explosion_spawn(cx + 12, cy);
                death_fx = TRUE;
            }
            player_set_visible(FALSE);
            if (!player_death_update()) {
                if (player_out_of_lives()) {
                    player_reset_lives();
                    persist_progress();
                    showGameOver();
                    return RUN_GAME_OVER;
                }
                death_fx = FALSE;
            }
            SPR_update();
            SYS_doVBlankProcess();
            VDP_setReg(17, 0);
            VDP_setReg(18, 0);
            continue;
        }

        u16 pad = JOY_readJoypad(JOY_1);

        // START opens the paused in-game menu (PC class_config.cpp:295)
        if ((pad & BUTTON_START) && !(prev_pad & BUTTON_START)) {
            // LEAVE STAGE returns from runStage directly (title), it is
            // NOT a stage clear
            if (menu_open(save_slot_weapon_mask(current_save_slot))) {
                persist_progress();     // PC leave_stage() saves
                return RUN_LEFT;
            }
            pad = JOY_readJoypad(JOY_1);
        }
        prev_pad = pad;
        objects_update();   // platforms carry the player before his physics
        player_update(pad);
        player_check_scroll_lock(pad);
        hud_update(player.hp);
        hud_weapon(player_weapon(), player_weapon_ammo());

        s16 bl, bt, br, bb;
        player_get_box(&bl, &bt, &br, &bb);

        u8 to_map;
        u8 to_trans;
        s16 to_x, to_y;
        if (stage_check_link(bl, bt, br, bb, &to_map, &to_x, &to_y, &to_trans)) {
            const s16 tile = stage_def->tile;
            const s16 dst_tx = to_x / tile;
            // Links land on the mouth of a ladder and/or on the last map row,
            // where the destination tile's centre sits past the bottom of the
            // map. Standing there left the player on the out-of-map "solid"
            // (in mid-air, STAND pose). Clamp into the map and, when the
            // destination tile is a stair, arrive ON the ladder so he hangs
            // instead of falling. That also leaves him just outside the return
            // link's band, so climbing back into it fires the return link
            // (before, he arrived inside the band, the arrival latch held and
            // climbing out of the room never triggered anything).
            const bool on_stair =
                stage_terrain_at(to_map, dst_tx, to_y / tile) == TERRAIN_STAIR;
            s16 px = to_x + tile / 2 - (PLAYER_BOX_L + PLAYER_BOX_R) / 2;
            s16 py = to_y + tile / 2 - (PLAYER_BOX_T + PLAYER_BOX_B) / 2;
            const s16 py_max = stage_def->map_h * tile - PLAYER_BOX_B;
            if (py > py_max) py = py_max;
            if (py < 0) py = 0;
            if (on_stair)
                px = dst_tx * tile + tile / 2
                     - (PLAYER_BOX_L + PLAYER_BOX_R) / 2;
            // The arrival camera must be known before the destination map is
            // loaded, so the transition knows which screen to slide in.
            const s16 cam_x = stage_arrival_camera_x(
                to_map, px + (PLAYER_BOX_L + PLAYER_BOX_R) / 2);

            // PC removes all projectiles before the room change
            // (game.cpp:679 remove_all_projectiles).
            shots_reset();
            eshots_reset();

            // PC changes room by scrolling the old screen out and the new one
            // in vertically (game.cpp:1067). Where the destination screen's
            // tiles do not fit in the free VRAM block the palette fade is
            // still used (see transition.c).
            const bool scrolled = transition_scroll(stage_current_map, to_map,
                                                    cam_x, px, py,
                                                    to_trans == TRANS_T2B);
            if (!scrolled) PAL_fadeOutAll(10, FALSE);

            // Show the destination at its arrival camera on the first drawn
            // frame; otherwise stage_load displays screen 0 first and the
            // correction to the stair position flashes for a frame.
            stage_pending_camera_x = cam_x;
            // Keep the normal "display off while rebuilding" path: the
            // in-place path let MAP_scrollToEx write base tile indices before
            // the tileset/palette were fully live, showing the dark/orange
            // intermediate frames on the downward Mummy stair.
            stage_load_in_place = FALSE;
            stage_load(to_map);
            player_place(px, py);
            if (on_stair) player.on_ladder = TRUE;   // arrive hanging on it
            if (!scrolled) player_restore_palette();  // fade blacked out PAL2
            player_get_box(&bl, &bt, &br, &bb);
            stage_arrival_camera_at(cam_x, (bt + bb) / 2);
            player_sync_sprite();   // else it flashes at the old screen spot

            if (!scrolled) {
                // fade back in: PAL_fadeInAll needs all 64 CRAM entries, so
                // build them from the palettes the stage just loaded.
                static u16 all[64];
                const Palette *lines[4] = {
                    &pal_ui,
                    stage_def->palettes[stage_current_map],
                    player_palette_def(),
                    stage_def->bg_pal[stage_current_map],
                };
                for (u8 ln = 0; ln < 4; ln++) {
                    for (u8 i = 0; i < 16; i++)
                        all[ln * 16 + i] = lines[ln] ? lines[ln]->data[i] : 0;
                }
                PAL_fadeInAll(all, 10, FALSE);
            }
        }

        // camera focus = centre of the player hitbox
        stage_update_camera((bl + br) / 2, (bt + bb) / 2);

        stage_anim_update();
        stage_cycle_update();

        enemies_update();   // patrol/contact first, shots test fresh positions
        // SHORYUKEN (LEGS armor): PC collision_player_special_attack(..., 9)
        // damages enemies the rising player passes through.
        if (player_is_shoryuken()) {
            s16 sl, st, sr, sb;
            player_get_box(&sl, &st, &sr, &sb);
            enemies_try_hit(sl, st, sr, sb, 9, player.facing, TRUE, 0);
        }
        // PC map_present_boss: the stage boss's dialog appears the first time
        // he is on screen (stages 0..8; the castle stages have no boss text).
        if (!boss_dialog_shown && enemies_boss_on_screen()) {
            dialog_show_boss(stage, current_player);
            boss_dialog_shown = TRUE;
            pad = JOY_readJoypad(JOY_1);
            prev_pad = pad;
        }
        explosion_update(); // enemy death puffs (PC death animation)
        eshots_update();    // enemy AI projectiles -> player
        shot_update();
        hud_boss(enemies_boss_on_screen() ? (s16)enemies_boss_hp() : -1);

        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0); // see above — DMA clobbers window regs
        VDP_setReg(18, 0);

        // PC classmap.cpp:2273 -> boss explosion + got_weapon() after the
        // stage boss falls. The sequence itself runs at the top of the loop.
        if (!boss_death_seq && enemies_boss_died()) {
            boss_death_seq = TRUE;
            boss_death_t = 0;
            enemies_boss_death_pos(&boss_death_x, &boss_death_y);
            shots_reset();
            eshots_reset();
            explosion_spawn(boss_death_x, boss_death_y);
        }
    }

    if (stage_cleared) {
        bool first_clear = !(save_slot_stages(current_save_slot) &
                             (1u << stage));
        save_slot_set_stage(current_save_slot, stage);
        persist_progress();     // PC leave_stage()/got_weapon() save_game()
        showStageClear(stage, first_clear);
        // PC game_scenes_map[ENDING_PLAYER1..4] = scene list 2..5: the final
        // castle stage (13) plays the ending for the chosen player, then the
        // title (main's loop).
        if (stage == 13) {
            // PC game::show_ending: game_config.game_finished unlocks players
            // 3/4 in the selection screen.
            config.game_finished = 1;
            config_save();
            scene_play((u8)(2 + (current_player & 3)));
            ending_run();
        }
        return RUN_CLEARED;
    }
    return RUN_LEFT;
}

// Drop the stage and leave a plain black BG_A for a text screen. The stage
// leaves the planes scrolled to the arena camera (BG_A x~2866, BG_B
// parallax), so reset both; a plain clear would show the tileset's tile 0.
static void textScreenBegin(void)
{
    XGM2_stop();
    SYS_doVBlankProcess();      // flush pending MAP updates
    stage_release_map();
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_B, 0);
    VDP_fillTileMapRect(BG_A,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_FONT_INDEX),
                        0, 0, 40, 28);
}

// PC game_over() screen (game.cpp:2160): GAME OVER + PRESS START. The caller
// already reset the lives (PC reset_lifes in death()).
static void showGameOver(void)
{
    textScreenBegin();
    drawCentered("GAME OVER", 12);
    drawCentered("PRESS START", 15);
    while (TRUE) {
        if (JOY_readJoypad(JOY_1) & BUTTON_START) break;
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
}

// PC got_weapon() (game.cpp:1320): after the boss falls, the first clear of
// stages 1..8 shows the weapon acquired before leaving the stage. The PC
// player_victory() flash/pose is not reproduced yet (the stage is frozen on
// its last frame); the text screen and the 5s hold match.
static void showStageClear(u8 stage, bool first_clear)
{
    textScreenBegin();

    // Got-weapon screen: stage 0 and the castle stages have no weapon.
    if (first_clear && stage >= 1 && stage <= 8) {
        sfx_play(SFX_GOT_WEAPON);
        drawCentered("YOU GOT", 10);
        drawCentered(weapon_data[stage].name, 12);
        for (u16 i = 0; i < 300; i++) {        // PC wait 5000ms
            if (JOY_readJoypad(JOY_1) & BUTTON_START) break;
            SPR_update();
            SYS_doVBlankProcess();
            VDP_setReg(17, 0);
            VDP_setReg(18, 0);
        }
        VDP_clearTextArea(0, 8, 40, 6);
    }

    drawCentered("STAGE CLEAR", 12);
    drawCentered("PRESS START", 15);
    while (TRUE) {
        if (JOY_readJoypad(JOY_1) & BUTTON_START) break;
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
}

// PC game loop after leave_stage(): the stage select drives the next stage.
// A game over on the intro stage restarts it (sceneslib pick_stage); any
// other game over / clear goes back to the select. The final castle stage
// plays its ending and returns to the title.
static void playSession(u8 stage)
{
    while (TRUE) {
        RunResult r = runStage(stage);
        if (r == RUN_LEFT) return;
        if (r == RUN_CLEARED && stage == 13) return;
        if (r == RUN_GAME_OVER &&
            !(save_slot_stages(current_save_slot) & 1u)) {
            stage = 0;              // intro still unbeaten: restart it
            continue;
        }
        stage = stage_select_run(current_player,
                                 save_slot_stages(current_save_slot));
    }
}

int main(bool hardReset)
{
    if (!hardReset) SYS_hardReset();

    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    JOY_init();
    SPR_initEx(504); // see runStage() — sprite VRAM 936..1439

    // The default DMA queue (80 entries) overflows during stage_load's force
    // map redraw / animated-frame upload (up to 17 slots x 6 frames), silently
    // dropping queued tileset/palette transfers. 192 entries covers that and
    // costs 3 KB instead of the old 8 KB, leaving the heap room for the
    // destination MAP allocated by a room transition (Mummy stairs).
    DMA_setMaxQueueSize(192);
    DMA_setMaxTransferSize(0); // unlimited DMA transfer per frame

    // Force known scroll/window state. SGDK's boot only writes the first
    // HScroll table word; if power-on VRAM differs (BlastEm vs Kega), the
    // full-scroll mode reads garbage and shifts the whole plane.
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    VDP_setHorizontalScrollVSync(BG_A, 0);
    VDP_setHorizontalScrollVSync(BG_B, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_B, 0);
    SYS_doVBlankProcess();
    VDP_clearPlane(WINDOW, FALSE);
    VDP_setBackgroundColor(0);

    // NOTE: no font palette load here. SGDK boot (VDP_resetScreen) already
    // loads palette_grey into PAL0 — identical grey ramp, index 0 = black
    // backdrop, 15 = white text. Re-writing it with the CPU method from
    // main() lands during active display, which is illegal for CRAM and
    // corrupts it on accurate emulators (white screen). See debugging.md.

    save_init();
    config_init();
    stage_set_data(0);   // valid stage_def before anything reads it

    showIntro();
    // PC game_scenes_show_unbeaten_intro(): scene list 0 (scenes_data.c)
    scene_play(SCENE_OPENING_UNBEATEN);
    while (TRUE) {
        s16 action = showTitle();
        if (action == 0) {                       // NEW GAME
            s16 slot = save_selector(TRUE);
            if (slot < 0) continue;              // backed out -> title
            save_slot_clear((u8)slot);
            current_save_slot = (u8)slot;
            // PC main_screen: NEW GAME -> select_player() before the stage.
            current_player = player_select_run();
            player_new_game();          // 5 lives, empty inventory
            persist_progress();
            save_slot_set_player(current_save_slot, current_player);
            playSession(0);
        } else if (action == 2) {                // CONFIG
            config_run(FALSE);
        } else if (action == 3) {                // ABOUT
            showAbout();
        } else {                                 // LOAD GAME
            s16 slot = save_selector(FALSE);
            if (slot < 0) continue;
            current_save_slot = (u8)slot;
            current_player = save_slot_player(current_save_slot);
            load_progress();
            // PC scenesLib::pick_stage (sceneslib.cpp:529): the intro stage
            // is played first; once it is beaten the stage select appears.
            u8 stage = 0;
            if (save_slot_stages((u8)slot) & 1) {
                stage = stage_select_run(current_player,
                                         save_slot_stages(current_save_slot));
            }
            playSession(stage);
        }
    }

    return 0;
}
