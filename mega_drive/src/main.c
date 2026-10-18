#include <genesis.h>

#include "resources.h"

#include "stage.h"
#include "player.h"
#include "enemy.h"
#include "object.h"
#include "shot.h"
#include "eshot.h"
#include "hud.h"
#include "menu.h"
#include "save.h"
#include "stageselect.h"
#include "saveselect.h"
#include "bossintro.h"
#include "playerselect.h"
#include "transition.h"
#include "title_map.h"
#include "title_tiles.h"

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

    VDP_loadTileData(title_tiles_data, TITLE_TILE_BASE, TITLE_TILE_COUNT, DMA_QUEUE);
    PAL_setPalette(PAL_TITLE, title_pal.data, DMA_QUEUE);
    // The logo's colorkey background is the title backdrop; on MD it comes
    // from the font-space tile sheet behind it (PAL0 entry 0), which is
    // black. PC fills the title with CONFIG_BGCOLOR (5,29,65) -> RGB333
    // 0x0210. The font glyphs use entry 15, so text stays white.
    static const u16 title_font_pal[16] = {
        0x0210, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x0EEE
    };
    PAL_setPalette(PAL0, title_font_pal, DMA_QUEUE);
    // the plane is 64 tiles wide: wipe the whole thing first so cells the
    // 40x28 image does not cover cannot show stale stage data
    VDP_fillTileMapRect(BG_B,
                        TILE_ATTR_FULL(PAL_TITLE, FALSE, FALSE, FALSE, TITLE_TILE_BASE),
                        0, 0, 64, 32);
    static u16 tcell[TITLE_MAP_W * TITLE_MAP_H];
    for (u16 i = 0; i < TITLE_MAP_W * TITLE_MAP_H; i++) {
        tcell[i] = TILE_ATTR_FULL(PAL_TITLE, FALSE, FALSE, FALSE,
                                  TITLE_TILE_BASE + title_map[i]);
    }
    // CPU: the queued variant raced with the neighbouring writes when the
    // title was re-entered (stale cells at the plane edges)
    VDP_setTileMapDataRect(BG_B, tcell, 0, 0, TITLE_MAP_W, TITLE_MAP_H,
                           TITLE_MAP_W, CPU);

    // PC draw_main: version at (8,8) -> tile (1,1), over the logo's top
    VDP_drawText(VERSION_STRING, 1, 1);
    VDP_drawText(COPYRIGHT_STRING, 5, 25);
    drawCentered(SITE_STRING, 27);

    // PC start menu (draw_main): NEW GAME / LOAD GAME. LOAD only appears
    // when the SRAM holds at least one save slot.
    bool can_load = save_exists();
    s16 sel = 0;
    u16 prev = JOY_readJoypad(JOY_1);    // don't pick on a held button
    while (TRUE) {
        VDP_drawText("NEW GAME", 14, 18);
        if (can_load) {
            VDP_drawText("LOAD GAME", 14, 20);
            VDP_drawText(sel ? " " : ">", 12, 18);
            VDP_drawText(sel ? ">" : " ", 12, 20);
        }
        u16 pad = JOY_readJoypad(JOY_1);
        if (can_load && (pad & BUTTON_DOWN) && !(prev & BUTTON_DOWN)) sel = 1;
        if ((pad & BUTTON_UP) && !(prev & BUTTON_UP)) sel = 0;
        if ((pad & BUTTON_START) && !(prev & BUTTON_START)) break;
        prev = pad;
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
    VDP_drawText("          ", 12, 18);
    VDP_drawText("          ", 12, 20);
    return sel;
}

// INTRO_STAGE = stage 0 = "CHAOS CITY" (game.cpp:439 sets currentStage).
// Stage 0 crosses two maps through ladders: link 0 is map 0 (136,14) -> map 1
// (198,0), the only way past the full-height wall at x=137..138.
static void showStageClear(void);
static u8 current_save_slot = 0;
static u8 current_player = 0;      // 0..3, chosen in the player-select screen
static void runStage(u8 stage)
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

    XGM2_play(stage_music);   // stage only — intro/title stay silent

    // NOTE: the old "clear BG_B here" block is gone — stage_load now draws
    // the parallax background on BG_B, and clearing it afterwards wiped the
    // tilemap (sky showed stale/garbage tiles).

    u16 prev_pad = JOY_readJoypad(JOY_1);  // START edge (a press held from
                                           // the title must not open the menu)
    bool stage_cleared = FALSE;
    while (TRUE) {
        u16 pad = JOY_readJoypad(JOY_1);

        // START opens the paused in-game menu (PC class_config.cpp:295)
        if ((pad & BUTTON_START) && !(prev_pad & BUTTON_START)) {
            // LEAVE STAGE returns from runStage directly (title), it is
            // NOT a stage clear
            if (menu_open(save_slot_weapon_mask(current_save_slot))) {
                return;
            }
            pad = JOY_readJoypad(JOY_1);
        }
        prev_pad = pad;

        objects_update();   // platforms carry the player before his physics
        player_update(pad);
        hud_update(player.hp);
        hud_weapon(player_weapon(), player_weapon_ammo());

        // PC game over (out of lives): leave the stage, back to the title.
        if (player_out_of_lives()) return;

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

            stage_load(to_map);
            player_place(px, py);
            if (on_stair) player.on_ladder = TRUE;   // arrive hanging on it
            if (!scrolled) player_restore_palette();  // fade blacked out PAL2
            player_get_box(&bl, &bt, &br, &bb);
            stage_arrival_camera((bl + br) / 2, (bt + bb) / 2);
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

        enemies_update();   // patrol/contact first, shots test fresh positions
        eshots_update();    // enemy AI projectiles -> player
        shot_update();
        hud_boss(enemies_boss_on_screen() ? (s16)enemies_boss_hp() : -1);

        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0); // see above — DMA clobbers window regs
        VDP_setReg(18, 0);

        // PC classmap.cpp:2273 -> got_weapon() after the stage boss falls
        if (enemies_boss_died()) { stage_cleared = TRUE; break; }
    }

    if (stage_cleared) {
        save_slot_set_stage(current_save_slot, 0);
        showStageClear();
    }
}

// Minimal stand-in for PC got_weapon()/player_victory() (game.cpp:1320):
// victory flash + weapon screen come in a later milestone. Overlay on the
// frozen stage, then return to the title (PC leave_stage -> stage select,
// which does not exist yet).
static void showStageClear(void)
{
    XGM2_stop();
    // flush pending MAP updates, then drop the map (see showTitle)
    SYS_doVBlankProcess();
    stage_release_map();
    // The stage leaves the planes scrolled to the arena camera (BG_A x~2866,
    // BG_B parallax) — text and the title after it need both planes at 0.
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_B, 0);
    // A plain clear leaves the stage tileset's tile 0 (a visible pattern);
    // fill with the font's space tile for a black backdrop.
    VDP_fillTileMapRect(BG_A,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_FONT_INDEX),
                        0, 0, 40, 28);
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

int main(bool hardReset)
{
    if (!hardReset) SYS_hardReset();

    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    JOY_init();
    SPR_initEx(504); // see runStage() — sprite VRAM 936..1439

    // The default DMA queue (80 entries) overflows during stage_load's force
    // map redraw, silently dropping queued tileset/palette transfers — the
    // map then renders stale title-screen tiles. Give the queue room.
    DMA_setMaxQueueSize(500);
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
    stage_set_data(0);   // valid stage_def before anything reads it

    showIntro();
    while (TRUE) {
        s16 action = showTitle();
        if (action == 0) {                       // NEW GAME
            s16 slot = save_selector(TRUE);
            if (slot < 0) continue;              // backed out -> title
            save_slot_clear((u8)slot);
            current_save_slot = (u8)slot;
            // PC main_screen: NEW GAME -> select_player() before the stage.
            current_player = player_select_run();
            save_slot_set_player(current_save_slot, current_player);
            runStage(0);
        } else {                                 // LOAD GAME
            s16 slot = save_selector(FALSE);
            if (slot < 0) continue;
            current_save_slot = (u8)slot;
            current_player = save_slot_player(current_save_slot);
            // PC scenesLib::pick_stage (sceneslib.cpp:529): the intro stage
            // is played first; once it is beaten the stage select appears.
            u8 stage = 0;
            if (save_slot_stages((u8)slot) & 1) {
                stage = stage_select_run(current_player);
            }
            runStage(stage);
        }
    }

    return 0;
}
