#include <genesis.h>

#include "resources.h"

#include "stage.h"
#include "player.h"
#include "enemy.h"
#include "object.h"
#include "shot.h"
#include "eshot.h"
#include "hud.h"

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
    VDP_drawText("JMD AMIGA MUSIC", 3, 14);
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
static void showTitle(void)
{
    SPR_end(); // release sprite VRAM so the full-screen image fits

    VDP_clearPlane(BG_A, FALSE);
    VDP_clearPlane(BG_B, FALSE);

    // Sync to the start of vblank before the image DMA — immediate DMA during
    // active display corrupts VRAM on accurate emulators (garbled BlastEm
    // title). VDP_drawImageEx has no DMA_QUEUE option, only DMA/CPU.
    SYS_doVBlankProcess();
    SYS_disableInts();
    VDP_drawImageEx(BG_B, &title_logo,
                    TILE_ATTR_FULL(PAL_TITLE, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                    0, 0, TRUE, DMA);
    SYS_enableInts();

    // rows are tiles (px/8). Row 0 = the 8px band above the logo art.
    VDP_drawText(VERSION_STRING, 1, 0);
    VDP_drawText(COPYRIGHT_STRING, 5, 25);
    drawCentered(SITE_STRING, 27);

    // hold until START; the main menu comes in the next milestone
    while (TRUE) {
        if (JOY_readJoypad(JOY_1) & BUTTON_START) break;
        SYS_doVBlankProcess();
        VDP_setReg(17, 0); // see waitFrames() — DMA clobbers window regs
        VDP_setReg(18, 0);
    }
}

// INTRO_STAGE = stage 0 = "CHAOS CITY" (game.cpp:439 sets currentStage).
// Stage 0 crosses two maps through ladders: link 0 is map 0 (136,14) -> map 1
// (198,0), the only way past the full-height wall at x=137..138.
static void showStageClear(void);
static void runStage(void)
{
    // SPR_initEx(640): sprite VRAM = 1952-640 = 1312..1951 (player + enemy
    // sheets incl. the 40x40 boss + shots + objects + HUD rows, ~500 tiles
    // deduped). Animated tiles end at 936, so 937..1311 stays free.
    SPR_initEx(640); // released again by showTitle()

    // PAL2 still holds intro/title colors; clear it before the player
    // sprite claims it (player_init loads the player palette over this).
    static const u16 black_pal[16] = {0};
    PAL_setPalette(PAL2, black_pal, DMA_QUEUE);

    stage_load(0);
    player_init();
    hud_init();
    // TEMP M7b test (revert with player.c SPAWN_X): the temp east spawn is
    // past divider col137, which scroll_lock_camera blocks from camera 0.
    stage_arrival_camera(player.x + 8, player.y);

    XGM2_play(stage_music);   // stage only — intro/title stay silent

    // NOTE: the old "clear BG_B here" block is gone — stage_load now draws
    // the parallax background on BG_B, and clearing it afterwards wiped the
    // tilemap (sky showed stale/garbage tiles).

    while (TRUE) {
        u16 pad = JOY_readJoypad(JOY_1);

        objects_update();   // platforms carry the player before his physics
        player_update(pad);
        hud_update(player.hp);
#ifdef DEBUG_OBJECTS
        objects_debug_text(25, player.x, player.y, camera.x);
        {
            // TEMP ladder debug (revert): row 26 = ladder/ground/vy/rows
            char d[32];
            u8 n = 0;
            s16 v = F16_toInt(player.vy);
            d[n++] = 'L'; d[n++] = player.on_ladder ? '1' : '0';
            d[n++] = ' '; d[n++] = player.on_ground ? '1' : '0';
            d[n++] = ' ';
            if (v < 0) { d[n++] = '-'; v = -v; }
            d[n++] = '0' + (v > 9 ? 9 : v);
            d[n++] = ' ';
            d[n++] = 'F'; d[n++] = '0' + (((player.y + PLAYER_BOX_B) >> 4) % 10);
            d[n++] = ' ';
            d[n++] = 'C';
            d[n++] = '0' + (((player.y + (PLAYER_BOX_T + PLAYER_BOX_B) / 2) >> 4) % 10);
            d[n] = 0;
            VDP_drawText(d, 0, 26);
        }
#endif

        s16 bl, bt, br, bb;
        player_get_box(&bl, &bt, &br, &bb);

        u8 to_map;
        s16 to_x, to_y;
        if (stage_check_link(bl, bt, br, bb, &to_map, &to_x, &to_y)) {
            stage_load(to_map);
            // centre the player on the destination tile
            player_place(to_x + STAGE0_TILE / 2 - (PLAYER_BOX_L + PLAYER_BOX_R) / 2,
                         to_y + STAGE0_TILE / 2 - (PLAYER_BOX_T + PLAYER_BOX_B) / 2);
            player_get_box(&bl, &bt, &br, &bb);
            stage_arrival_camera((bl + br) / 2, (bt + bb) / 2);
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
        if (enemies_boss_died()) break;
    }

    showStageClear();
}

// Minimal stand-in for PC got_weapon()/player_victory() (game.cpp:1320):
// victory flash + weapon screen come in a later milestone. Overlay on the
// frozen stage, then return to the title (PC leave_stage -> stage select,
// which does not exist yet).
static void showStageClear(void)
{
    XGM2_stop();
    // The stage leaves the planes scrolled to the arena camera (BG_A x~2866,
    // BG_B parallax) — text and the title after it need both planes at 0.
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_B, 0);
    VDP_clearPlane(BG_A, FALSE);
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
    SPR_initEx(640); // see runStage() — sprite VRAM 1312..1951

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

    showIntro();
    while (TRUE) {
        showTitle();
        runStage();   // returns from showStageClear() once the boss falls
    }

    return 0;
}
