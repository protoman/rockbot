#include <genesis.h>

#include "resources.h"

#include "stage.h"
#include "player.h"

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

// DEBUG: solid purple block on BG_B, 20x14 tiles = 160x112px = 1/4 screen.
// Lives on BG_B (the MAP engine only refreshes BG_A) and pokes through the
// transparent sky tiles. If this is not on screen, the ROM is stale.
#define DBG_TILE 512   // scratch VRAM tile, above every tileset (max 133 tiles)

static const u32 dbg_solid_tile[8] = {
    0x11111111, 0x11111111, 0x11111111, 0x11111111,
    0x11111111, 0x11111111, 0x11111111, 0x11111111
};
// color 1 = purple, color 15 = white (the font draws with index 15)
static const u16 dbg_pal[16] = {
    0x0000, 0x01C7, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0FFF
};

// DEBUG helper: print a 5-digit value (SGDK sprintf cannot be relied on for %3d)
static void dbgNum(u16 tileX, u16 tileY, s32 v)
{
    char s[6];
    s[5] = 0;
    u32 n = (v < 0) ? -v : v;
    if (v < 0) s[0] = '-';
    for (s16 i = 4; i >= (v < 0 ? 1 : 0); i--) {
        s[i] = '0' + (n % 10);
        n /= 10;
    }
    VDP_drawText(s, tileX, tileY);
}

// INTRO_STAGE = stage 0 = "CHAOS CITY" (game.cpp:439 sets currentStage).
// Stage 0 crosses two maps through ladders: link 0 is map 0 (136,14) -> map 1
// (198,0), the only way past the full-height wall at x=137..138.
static void runStage(void)
{
    SPR_init(); // released again by showTitle()

    // Queue the debug tile+palette FIRST, while the DMA queue is empty.
    // stage_load's force map redraw queues ~128 entries; the queue holds 80,
    // so anything queued after it is silently dropped (SGDK ignores the
    // DMA_queueDma failure return). That is why every earlier transfer
    // method "failed" -- the queue was already full.
    VDP_loadTileData(dbg_solid_tile, DBG_TILE, 1, DMA_QUEUE);
    PAL_setPalette(PAL3, dbg_pal, DMA_QUEUE);

    stage_load(0);
    player_init();

    // Clear BG_B with the display off. A mid-frame CPU clear is dropped by
    // accurate emulators (BlastEm, GPGX), leaving the title-screen tilemap
    // on BG_B visible through the stage's transparent sky tiles. With the
    // display disabled the VDP accepts CPU VRAM writes unconditionally.
    {
        bool on = VDP_isEnable();
        VDP_setEnable(FALSE);
        VDP_clearPlane(BG_B, FALSE);
        VDP_setReg(17, 0); // DMA clobbers the window registers — restore
        VDP_setReg(18, 0);
        VDP_setEnable(on);
    }

    while (TRUE) {
        u16 pad = JOY_readJoypad(JOY_1);

        player_update(pad);

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
        }

        // camera focus = centre of the player hitbox
        stage_update_camera((bl + br) / 2, (bt + bb) / 2);

        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0); // see above — DMA clobbers window regs
        VDP_setReg(18, 0);

        // DEBUG overlay:
        // 1. Solid rectangle on BG_B — the map's transparent sky shows BG_B
        //    through, so this proves the build is fresh (PAL0 = stage
        //    palette, index 1 = light blue, known to render).
        // 2. Tile-position number on BG_A with VDP_drawText (SGDK's default
        //    font path — TILE_FONT_INDEX + PAL0, no manual basetile). Drawn
        //    at screen position = plane position + camera, after the camera
        //    update. The map may overwrite it while scrolling; it is
        //    readable when standing still — enough to find the wall.
        {
            VDP_fillTileMapRect(BG_B,
                                TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, DBG_TILE),
                                0, 0, 20, 14);
            char s[5];
            s16 v = (bl + br) / 32;
            s[0] = '0' + ((v / 1000) % 10);
            s[1] = '0' + ((v / 100) % 10);
            s[2] = '0' + ((v / 10) % 10);
            s[3] = '0' + (v % 10);
            s[4] = '\0';
            VDP_drawText(s, 1 + camera.x / 8, 1 + camera.y / 8);
        }

        // Re-assert PAL3 every frame via DMA_QUEUE — the same method the
        // stage palette uses, which renders correctly. A one-shot transfer
        // before stage_load was silently lost.
        PAL_setPalette(PAL3, dbg_pal, DMA_QUEUE);
    }
}

int main(bool hardReset)
{
    if (!hardReset) SYS_hardReset();

    VDP_setScreenWidth320();
    VDP_setScreenHeight224();
    JOY_init();
    SPR_init();

    // The default DMA queue (80 entries) overflows during stage_load's force
    // map redraw, silently dropping queued tileset/palette transfers — the
    // map then renders stale title-screen tiles. Give the queue room.
    DMA_setMaxQueueSize(500);

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

    PAL_setPalette(PAL0, palette_grey, CPU); // font

    showIntro();
    showTitle();
    runStage();

    while (TRUE) SYS_doVBlankProcess();

    return 0;
}
