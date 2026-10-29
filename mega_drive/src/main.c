#include "dbgprint.h"
#include <genesis.h>

#include "resources.h"
#include "sprite_guard.h"

#include "dbglog.h"

#ifdef MD_LINK_DEBUG
static u8 lk_dbg_frames;   // frames still logged after a room change
#endif
#include "stage.h"
#include "tdump.h"
#include "pal_safe.h"
#include "player.h"
#include "enemy.h"
#include "object.h"
#include "shot.h"
#include "explosion.h"
#include "effects.h"
#include "tooltip.h"
#include "eshot.h"
#include "hud.h"
#include "menu.h"
#include "gotbg_tiles.h"
#include "gotbg_map.h"
#include "save.h"
#include "stageselect.h"
#include "saveselect.h"
#include "bossintro.h"
#include "playerselect.h"
#include "transition.h"
#include "sfx.h"
#include "music.h"
#include "scene.h"
#include "ending.h"
#include "walkdiag.h"
extern DMAOpInfo *dmaQueues;   // dma.c: global, read by the flush routine
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

#ifdef MD_PROFILE
// Temporary build for the "SEAHORSE is slow" report: time spent per frame in
// each part of the stage loop, printed once per second over KDEBUG.
// Build with `make profile`. Time base is getSubTick(): 1/76800 s (~13 us),
// 1280 units per 60 Hz frame. (GET_VCOUNTER wraps every 256 lines and
// getTick() only moves once per frame - neither can split a frame.)
// The stage loop is SPLIT BY A MARK PLACED AFTER EACH UNIT OF WORK: the value
// printed for a name is the time spent since the previous mark.
#include <string.h>
#define PROF_OBJ     7      // objects_update
#define PROF_PLAYER  8      // player_update + scroll lock
#define PROF_HUD     9      // hud_update + hud_weapon
#define PROF_SETUP   0      // loop tail, input, menu, links, transitions
#define PROF_CAMERA  1      // stage_update_camera (MAP_scrollTo)
#define PROF_ANIM    2      // stage_anim_update + stage_cycle_update
#define PROF_ENEMY   3      // enemies_update + boss checks
#define PROF_SHOT    4      // explosions, eshots, shots, boss HUD
#define PROF_SPRITE  5      // SPR_update
#define PROF_VBLANK  6      // SYS_doVBlankProcess (idle time if the frame fits)
#define PROF_EXPL    7      // explosion_update + effects_update
#define PROF_ESHOT   8      // eshots_update
#define PROF_PSHOT   9      // shot_update (player shots)
#define PROF_N       10
static u32 prof_last;
static u32 prof_acc[PROF_N];
// object.c traces its most expensive object of the second (see objects_update)
extern u8 prof_obj_hot_i;
extern u16 prof_obj_hot_cost;
extern u8 prof_obj_hot_type;
static u16 prof_frames;
#ifdef MD_PROFILE_ENEMY
extern u32 enemy_prof[6];
#endif
#ifdef MD_PROFILE_PLAYER
extern u32 player_prof[8];
#endif
static u32 prof_cur, prof_max, prof_over, prof_frames_n;
static inline void prof_tick(u8 i)
{
    const u32 t = getSubTick();
    prof_acc[i] += t - prof_last;
    if (i != PROF_VBLANK) {
        prof_cur += t - prof_last;
    } else {                              // frame ends: did the work fit the 60 Hz budget?
        if (prof_cur > 1150) prof_over++;
        if (prof_cur > prof_max) prof_max = prof_cur;
        prof_cur = 0;
        prof_frames_n++;
    }
    prof_last = t;
}
static void prof_report(void)
{
    if (++prof_frames < 60) return;
    dbg_print("PROF map=%d obj=%d npc=%d hot=%d/t%d/%d px=%d py=%d "
            "setup=%lu objupd=%lu plr=%lu hud=%lu "
            "cam=%lu anim=%lu npcs=%lu shot=%lu spr=%lu wait=%lu "
            "over=%lu max=%lu fx=%lu esh=%lu psh=%lu (subticks/frame, 1280 = 60fps)\n",
            (int)stage_current_map, (int)objects_live(), (int)enemies_live(),
            (int)prof_obj_hot_i, (int)prof_obj_hot_type,
            (int)prof_obj_hot_cost, (int)player.x, (int)player.y,
            (unsigned long)(prof_acc[PROF_SETUP] / 60),
            (unsigned long)(prof_acc[PROF_OBJ] / 60),
            (unsigned long)(prof_acc[PROF_PLAYER] / 60),
            (unsigned long)(prof_acc[PROF_HUD] / 60),
            (unsigned long)(prof_acc[PROF_CAMERA] / 60),
            (unsigned long)(prof_acc[PROF_ANIM] / 60),
            (unsigned long)(prof_acc[PROF_ENEMY] / 60),
            (unsigned long)(prof_acc[PROF_SHOT] / 60),
            (unsigned long)(prof_acc[PROF_SPRITE] / 60),
            (unsigned long)(prof_acc[PROF_VBLANK] / 60),
            (unsigned long)prof_over, (unsigned long)prof_max,
            (unsigned long)(prof_acc[PROF_EXPL] / 60), (unsigned long)(prof_acc[PROF_ESHOT] / 60),
            (unsigned long)(prof_acc[PROF_PSHOT] / 60));
#ifdef MD_PROFILE_PLAYER
    dbg_print("PPROF pre=%lu ladder=%lu shield=%lu slide=%lu horiz=%lu jump=%lu vert=%lu plat=%lu\n",
            (unsigned long)(player_prof[0] / 60), (unsigned long)(player_prof[1] / 60), (unsigned long)(player_prof[2] / 60),
            (unsigned long)(player_prof[3] / 60), (unsigned long)(player_prof[4] / 60), (unsigned long)(player_prof[5] / 60),
            (unsigned long)(player_prof[6] / 60), (unsigned long)(player_prof[7] / 60));
    for (u8 i = 0; i < 8; i++) player_prof[i] = 0;
#endif
#ifdef MD_PROFILE_ENEMY
    dbg_print("EPROF pre=%lu ai=%lu rest=%lu grav=%lu draw=%lu water=%lu (subticks/frame)\n",
            (unsigned long)(enemy_prof[4] / 60), (unsigned long)(enemy_prof[5] / 60),
            (unsigned long)(enemy_prof[0] / 60), (unsigned long)(enemy_prof[1] / 60),
            (unsigned long)(enemy_prof[2] / 60), (unsigned long)(enemy_prof[3] / 60));
    for (u8 i = 0; i < 6; i++) enemy_prof[i] = 0;
#endif
    prof_over = 0;
    prof_max = 0;
    for (u8 i = 0; i < PROF_N; i++) prof_acc[i] = 0;
    prof_obj_hot_cost = 0;
    prof_obj_hot_i = 0xFF;
    prof_obj_hot_type = 0xFF;
    prof_frames = 0;
}
#endif

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
    // Diagnostic (stripes, BUG-044 follow-up): the plane BELOW must not be
    // showing through. BG_A cells here are tile 0 with priority 0, so anything
    // BG_B holds wins the pixel; if BG_B is striped, the whole screen is.
    VDP_clearPlane(BG_B, FALSE);
    // Boot CRAM is not trustworthy: the grey ramp read back shifted to a
    // different CRAM entry on every boot (BUG-044 follow-up: a CRAM write whose
    // destination is redirected mid-transfer). Assert the UI/line-0 palette
    // here, race-free, before anything of this screen is drawn.
    pal_load(PAL0, pal_ui.data);
    tdump_vdp("intro-clear");
    tdump_cram("intro-clear");

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
    tdump_vdp("intro-24f");
    tdump_cram("intro-24f");
    tdump_tile("intro-24f", 0);
    tdump_tile("intro-24f", 702);
    tdump_cell("intro-24f", VDP_getBGAAddress(), 0, 0);
    tdump_cell("intro-24f", VDP_getBGAAddress(), 12, 10);
    tdump_cell("intro-24f", VDP_getBGBAddress(), 12, 10);

    // upperland strip animation: 30ms per frame (~2 frames at 60Hz)
    for (u16 i = 1; i < FRAME_COUNT; i++) {
        spr_set_frame(sprUpper, i);
        waitFrames(2);
    }
    spr_set_frame(sprUpper, 0);

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
    tdump_vdp("title");
    sprites_end();                 // release sprite VRAM so the image fits
    // Let any scheduled MAP update flush BEFORE we take over the planes,
    // then drop the map so nothing repaints them afterwards.
    SYS_doVBlankProcess();
    stage_release_map();
    music_play(MUSIC_OPENING);

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
        pal_load((u16)(PAL0 + L), title64_pal[L]);
    // The logo's colorkey background is the title backdrop; on MD it comes
    // from the font-space tile sheet behind it (PAL0 entry 0), which is
    // black. PC fills the title with CONFIG_BGCOLOR (5,29,65) -> RGB333
    // 0x0210. The font glyphs use entry 15, so text stays white.
    // Line 0 slot 0 is the backdrop (PC CONFIG_BGCOLOR) seen through the logo's
    // transparent pixels; slot 15 is the font's white (emitted as such by
    // tools/title64.py), so VDP_drawText keeps working.
    pal_color(0, 0x0420);   // PC CONFIG_BGCOLOR (5,29,65) -> (0,36,73)
    pal_color(15, 0x0EEE);   // font white: the art keeps its own white at slot 1
    // Pixels at index 0 are transparent on a plane and show CRAM's backdrop
    // entry, so point it at PAL0 slot 0 (the PC CONFIG_BGCOLOR) - otherwise the
    // logo's background is black instead of the title's navy.
    VDP_setBackgroundColor(0);
    // the plane is 64 tiles wide: wipe the whole thing first so cells the
    // 40x28 image does not cover cannot show stale stage data
    VDP_fillTileMapRect(BG_B,
                        TILE_ATTR_FULL(PAL_TITLE, FALSE, FALSE, FALSE, TITLE_TILE_BASE),
                        0, 0, 64, 32);
    u16 *tcell = screen_cells(TITLE64_W * TITLE64_H);
    if (tcell) {
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
    }

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

#ifdef MD_HEAP_TRACE
#define HEAP_PROBE(tag) dbg_print("HEAP %s free=%u largest=%u\n", tag, \
                                (unsigned)MEM_getFree(), (unsigned)MEM_getLargestFreeBlock())
#else
#define HEAP_PROBE(tag) ((void)0)
#endif

static RunResult runStage(u8 stage)
{
    HEAP_PROBE("runStage");
    // Pick the stage's data bundle (clamped to the converted stages), then
    // load its map 0. Links/map changes stay within the current stage.
    stage_set_data(stage);
    player_clear_checkpoint();   // per-stage respawn point (PC resets it)

    // PC boss intro (sceneslib.cpp:681) before the stage; skipped once the
    // stage is beaten (game_save.stages[stage] != 0).
#ifndef MD_TEST_STAGE      // test builds boot straight into the stage
    if (!(save_slot_stages(current_save_slot) & (1u << stage))) {
#else
    if (0) {
#endif
        boss_intro_run(stage);
    } else {
        // Skip the intro and make sure its jingle did not carry over from a
        // previous replay/selection.
        music_stop();
    }

    player_set_number(current_player);

    // Sprite pool = TILE_FONT_INDEX(1440) - SPRITE_VRAM_TILES = 684..1439
    // (stage.h owns the constant and why the pool shrank by 12 tiles).
    // TILE_FONT_INDEX is 1440 (0xC000/32 - FONT_LEN 96), NOT 1952 as older
    // comments said — a bigger pool would overlap the parallax tiles.
    // The engine only holds the tiles of the frames currently on screen.
    sprites_begin(SPRITE_VRAM_TILES); // released again by showTitle()

    // PAL2 still holds intro/title colors; clear it before the player
    // sprite claims it (player_init loads the player palette over this).
    static const u16 black_pal[16] = {0};
    PAL_setPalette(PAL2, black_pal, DMA_QUEUE);

#ifdef MD_TEST_TILE
    // Test build: start on a specific map so a link can be reproduced without
    // walking the whole stage (MD_TEST_MAP/MD_TEST_TX/MD_TEST_TY).
    stage_load((u8)MD_TEST_MAP);
#else
    stage_load(0);
#endif
    if (!stage_map_ready()) {
        // BUG-040: no heap for MAP_create. Entering the stage loop with a NULL
        // map crashed while drawing it; go back to the title instead.
        return RUN_LEFT;
    }
    player_init();
    hud_init();

    music_play_stage(stage);

    // ---- stage start sequence (PC game::start_stage, game.cpp:244) --------
    // show_player_teleport(): the player beams in with the TELEPORT animation
    // (frames 0/1 alternating) for 1500ms, then frame 2, then back to STAND.
    // show_ready(): "MISSION STARTED!" blinks 6 times over the frozen stage
    // (200ms per phase). The PC does not read input in either function, so the
    // sequence always plays through.
    {
        if (player.spr) {
            spr_set_anim(player.spr, ANIM_TELEPORT);
            for (u16 i = 0; i < 90; i++) {                   // ~1500ms
                spr_set_frame(player.spr, (i / 15) & 1);
                SPR_update();
                SYS_doVBlankProcess();
            }
            spr_set_frame(player.spr, 2);
            for (u16 i = 0; i < 10; i++) {
                SPR_update();
                SYS_doVBlankProcess();
            }
            spr_set_anim(player.spr, ANIM_STAND);
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

#ifdef MD_TEST_TILE
    // Test build: drop the player on the requested tile, centred in it.
    player_place((s16)(MD_TEST_TX * stage_def->tile + stage_def->tile / 2
                       - (PLAYER_BOX_L + PLAYER_BOX_R) / 2),
                 (s16)(MD_TEST_TY * stage_def->tile + stage_def->tile / 2
                       - (PLAYER_BOX_T + PLAYER_BOX_B) / 2));
    camera.x = player.x - 160;                 // snap the camera to the placed player
    camera.y = 0;
    stage_update_camera(player.x, player.y);
    stage_force_redraw();
    player_sync_sprite();
#endif

    // NOTE: the old "clear BG_B here" block is gone — stage_load now draws
    // the parallax background on BG_B, and clearing it afterwards wiped the
    // tilemap (sky showed stale/garbage tiles).

    bool boss_dialog_shown = FALSE;
    bool boss_music_started = FALSE;
    u16 prev_pad = JOY_readJoypad(JOY_1);  // START edge (a press held from
                                           // the title must not open the menu)
    bool stage_cleared = FALSE;
    bool death_fx = FALSE;                 // death burst started
    u16 death_fx_t = 0;                    // frames inside the death burst
    bool boss_death_seq = FALSE;
    s16 boss_death_x = 0, boss_death_y = 0; // ring centre (world)
    while (TRUE) {
        // ---- boss death sequence (PC kill_all_enemies + ring explosion) ---
        if (boss_death_seq) {
            // PC game::classic_style_draw_boss_death: the 8-explosion ring
            // travels outward until it crosses the screen (320 steps).
            if (!explosion_ring_update()) { stage_cleared = TRUE; break; }
            SPR_update();
            SYS_doVBlankProcess();
            VDP_setReg(17, 0);
            VDP_setReg(18, 0);
            continue;
        }

        // ---- death sequence: freeze the world while the player is gone ----
        if (player_is_dying()) {
            s16 dl, dt, dr, db;
            player_get_box(&dl, &dt, &dr, &db);
            s16 cx = (dl + dr) / 2, cy = (dt + db) / 2;
            if (!death_fx) {
                death_fx = TRUE;
                death_fx_t = 0;
            }
            // PC game.cpp:1707/1734: the 47x47 death explosion repeats over the
            // player, then four more spawn at +-32px diagonals 300ms apart, so
            // the burst travels OUTWARD (the port used to burn three puffs in a
            // horizontal row and freeze them). The 90-frame death window holds
            // three centre puffs (0/30/60) and the 18/36/54/72 diagonals.
            if (death_fx_t < 90 && (death_fx_t % 30) == 0)
                explosion_spawn(cx, cy);
            if (death_fx_t == 18 || death_fx_t == 36 ||
                death_fx_t == 54 || death_fx_t == 72) {
                static const s8 ddx[4] = { -32, 32, 32, -32 };
                static const s8 ddy[4] = { -32, 32, -32, 32 };
                u8 k = (u8)(death_fx_t / 18 - 1);
                explosion_spawn((s16)(cx + ddx[k]), (s16)(cy + ddy[k]));
            }
            death_fx_t++;
            explosion_update();          // the puffs animate (and free)
            effects_update();
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
#ifdef MD_AUTOPILOT
        // Test build: a scripted player so a profile / trace run needs no input.
        // Walks right (or DOWN/UP alternately with MD_AUTOPILOT_LADDER), jumps when
        // stuck or every 80 frames, fires continuously, never dies.
        {
            static u16 ap_t, ap_stuck;
            static s16 ap_x;
            ap_t++;
#ifdef MD_AUTOPILOT_LADDER
            pad = ((ap_t / 600) & 1) ? BUTTON_UP : BUTTON_DOWN;
#else
            pad = BUTTON_RIGHT;
            if (player.x == ap_x) ap_stuck++; else ap_stuck = 0;
            ap_x = player.x;
            if (ap_stuck > 30 || (ap_t % 80) < 14) pad |= config_jump_mask();
#endif
#ifdef MD_AUTOPILOT_TELEPORT
            // sample the whole map: every 420 frames drop the player at the next column
            if ((ap_t % 420) == 1) {
                static u8 ap_i;
                static const u8 cols[] = { 8, 24, 40, 56, 72, 88, 104, 120, 136, 152, 168, 184 };
                player_place((s16)(cols[ap_i % 12] * 16), 0);
                ap_i++;
            }
#endif
            if ((ap_t % 20) == 0) pad |= config_fire_mask();
            if ((ap_t % 30) == 0 && player.hp < 28) player.hp = 28;
#ifdef MD_LINK_TRACE
            if ((ap_t % 20) == 0)
                dbg_print("AP t=%d map=%d px=%d py=%d cam=%d ladder=%d\n", (int)ap_t,
                        (int)stage_current_map, (int)player.x, (int)player.y,
                        (int)camera.x, (int)player.on_ladder);
#endif
        }
#endif

        // START opens the paused in-game menu (PC class_config.cpp:295)
        if ((pad & BUTTON_START) && !(prev_pad & BUTTON_START)) {
            // LEAVE STAGE returns from runStage directly (title), it is
            // NOT a stage clear
            if (menu_open(save_slot_weapon_mask(current_save_slot))) {
                persist_progress();     // PC leave_stage() saves
                return RUN_LEFT;
            }
            shots_reset();              // the shots thrown before the menu are gone
            pad = JOY_readJoypad(JOY_1);
        }
        prev_pad = pad;
        objects_update();   // platforms carry the player before his physics
#ifdef MD_PROFILE
        prof_tick(PROF_OBJ);
#endif
        player_update(pad);
        player_check_scroll_lock(pad);
#ifdef MD_PROFILE
        prof_tick(PROF_PLAYER);
#endif
        hud_update(player.hp);
        hud_weapon(player_weapon(), player_weapon_ammo());
#ifdef MD_PROFILE
        prof_tick(PROF_HUD);
#endif

#ifdef MD_LINK_DEBUG
        if (lk_dbg_frames) {
            lk_dbg_frames--;
            if ((lk_dbg_frames % 6) == 0)
                dbg_log("LKF t=%u x=%d y=%d vy=%d ladder=%u cam=%d under=%u pad=%u\n",
                        120 - lk_dbg_frames, player.x, player.y,
                        (s16)F16_toInt(player.vy), player.on_ladder, camera.x,
                        stage_terrain_at(stage_current_map,
                            (player.x + (PLAYER_BOX_L + PLAYER_BOX_R) / 2) / 16,
                            (player.y + PLAYER_BOX_B) / 16),
                        JOY_readJoypad(JOY_1));
        }
#endif
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
            // Ladder mouth: the link's dst_x can sit one column beside the
            // actual ladder in the destination map (APE link2 points at tile 1
            // while its stair is at tile 2, link4 at 21 vs 22), so trusting it
            // dropped the player next to the ladder and he fell
            // (transition_ladder_align_error). The PC never snaps X on a
            // vertical transition - it keeps the screen X and relies on the
            // ladder lining up - so look for the stair the link means: same
            // column first, then +-1, then +-2, on the arrival row or the one
            // below it.
            s16 stair_col = -1;
            if (to_trans == TRANS_T2B || to_trans == TRANS_B2T) {
                const s16 row0 = to_y / tile;
                // The player lands at dst_x PLUS his offset inside the link band
                // (stage_link_band_off), so on a wide link (size up to 18) the
                // ladder he arrives on is far from dst_x: DEEP JUNGLE map 0 col
                // 98 and map 1 col 17 (measured: stair -1, he arrived on the
                // ladder cell but not hanging, and fell). Scan around the real
                // arrival column first, then around dst_x as before.
                s16 arr_tx = (s16)((to_x + stage_link_band_off) / tile);
                if (arr_tx >= stage_def->map_w) arr_tx = stage_def->map_w - 1;
                const s16 bases[2] = { arr_tx, dst_tx };
                for (u8 bi = 0; bi < 2 && stair_col < 0; bi++) {
                    for (s16 d = 0; d <= 2 && stair_col < 0; d++) {
                        for (s16 sgn = -1; sgn <= 1; sgn += 2) {
                            const s16 c = (s16)(bases[bi] + (d ? d * sgn : 0));
                            if (c < 0 || c >= stage_def->map_w) continue;
                            // Only a ladder inside the link's own destination
                            // columns: CASTLE 4 link 6 (dst col 108, size 2)
                            // captured the ladder at col 111, which is link
                            // 10's mouth, and left through it.
                            if (c < dst_tx - 2 ||
                                c > dst_tx + (stage_link_size ? stage_link_size : 1))
                                continue;
                            if (stage_terrain_at(to_map, c, row0) == TERRAIN_STAIR ||
                                stage_terrain_at(to_map, c, row0 + 1) == TERRAIN_STAIR) {
                                stair_col = c;
                                break;
                            }
                        }
                    }
                }
            }
            const bool on_stair = stair_col >= 0 ||
                stage_terrain_at(to_map, dst_tx, to_y / tile) == TERRAIN_STAIR;
            // Hang on the arrival ladder only when the player left on one; a
            // fall (jump off a ladder) keeps falling until he presses UP.
            const bool hang = player.on_ladder;
            // Keep the player's offset inside the link band (PC keeps his
            // screen X and scrolls the map instead of snapping to the tile
            // centre, which made wide links jump sideways).
            s16 px = to_x + stage_link_band_off
                     - (PLAYER_BOX_L + PLAYER_BOX_R) / 2;
            s16 py = to_y + tile / 2 - (PLAYER_BOX_T + PLAYER_BOX_B) / 2;
            const s16 py_max = stage_def->map_h * tile - PLAYER_BOX_B;
            if (py > py_max) py = py_max;
            if (py < 0) py = 0;
            const s16 px_max = stage_def->map_w * tile - PLAYER_BOX_R;
            if (px > px_max) px = px_max;
            if (px < 0) px = 0;
            if (on_stair) {
                const s16 col = (stair_col >= 0) ? stair_col : dst_tx;
                px = col * tile + tile / 2
                     - (PLAYER_BOX_L + PLAYER_BOX_R) / 2;
            }
            // The arrival camera must be known before the destination map is
            // loaded, so the transition knows which screen to slide in.
            // PC keeps the player at the same SCREEN X across the slide (the
            // two link tiles line up): place the destination camera so the
            // arrival point lands where the player stands now.
            // Teleporter links (type >= 2) jump across the map: the PC fades
            // instead of sliding (game.cpp:709) and centres the camera on the
            // arrival, so only the scroll links keep the screen X.
            const bool teleporter = stage_link_type >= 2;
            s16 keep_sx = teleporter ? 160
                : player.x + (PLAYER_BOX_L + PLAYER_BOX_R) / 2 - camera.x;
            if (keep_sx < 0) keep_sx = 0;
            if (keep_sx > 319) keep_sx = 319;
            const s16 cam_x = stage_arrival_camera_keep(
                to_map, px + (PLAYER_BOX_L + PLAYER_BOX_R) / 2, keep_sx);
#ifdef MD_LINK_DEBUG
            dbg_log("LK from=%u to=%u trans=%u type=%u old_x=%d old_y=%d "
                    "cam_old=%d keep_sx=%d band_off=%d to_x=%d to_y=%d "
                    "stair_col=%d on_stair=%u px=%d py=%d cam_new=%d "
                    "was_ladder=%u dst_tx=%d t_dst=%u\n",
                    stage_current_map, to_map, to_trans, stage_link_type,
                    player.x, player.y, camera.x, keep_sx,
                    stage_link_band_off, to_x, to_y, stair_col, on_stair, px,
                    py, cam_x, player.on_ladder, dst_tx,
                    stage_terrain_at(to_map, (stair_col >= 0) ? stair_col : dst_tx,
                                     to_y / tile));
#endif
#ifdef MD_LINK_TRACE
            // Offline-first rule then one emulator run: print exactly what the
            // arrival computes for the ladder / room-change reports.
            dbg_print("LINK map %d->%d trans %d dst(%d,%d) stair %d px %d py %d cam %d latch_off %d",
                    stage_current_map, to_map, to_trans, dst_tx, to_y / tile,
                    stair_col, px, py, cam_x, stage_link_band_off);
#endif


            // PC removes all projectiles before the room change
            // (game.cpp:679 remove_all_projectiles).
            shots_reset();
            eshots_reset();

            // PC changes room by scrolling the old screen out and the new one
            // in vertically (game.cpp:1067). Where the destination screen's
            // tiles do not fit in the free VRAM block the palette fade is
            // still used (see transition.c).
            const bool scrolled = !teleporter &&
                transition_scroll(stage_current_map, to_map, cam_x, px, py,
                                  to_trans == TRANS_T2B,
                                  // residual the destination camera could not
                                  // absorb: move the outgoing screen by it
                                  (s16)((px + (PLAYER_BOX_L + PLAYER_BOX_R) / 2
                                         - cam_x) - keep_sx));
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
            // BUG-005 regression (2026-10-24 report: "the old bug returned in
            // this ladder"): the arrival nudge lost its caller when the
            // ladder-scan arrival landed - stage_link_band_hit() was dead
            // code. A stair link arrives on the top/bottom map row, inside the
            // return link's band, so the latch never clears and the room
            // cannot be left the way it was entered. Step along the ladder in
            // 8px steps until the box is out of every band (PC keeps the
            // player's screen position and lets him re-enter instead).
            if (py == 0 || py == py_max) {
                const s16 step = (py == 0) ? 8 : -8;
                for (u8 i = 0; i < 4; i++) {
                    if (!stage_link_band_hit(px + PLAYER_BOX_L,
                                             py + PLAYER_BOX_T,
                                             px + PLAYER_BOX_R,
                                             py + PLAYER_BOX_B))
                        break;
                    const s16 ny = (s16)(py + step);
                    if (ny < 0 || ny > py_max) break;
                    py = ny;
                }
            }
            player_place(px, py);
            if (on_stair && hang) player.on_ladder = TRUE;   // arrive hanging on it
            if (!scrolled) player_restore_palette();  // fade blacked out PAL2
            player_get_box(&bl, &bt, &br, &bb);
            stage_arrival_camera_at(cam_x, (bt + bb) / 2);
            player_sync_sprite();   // else it flashes at the old screen spot
#ifdef MD_LINK_DEBUG
            {
                const s16 cx_ = (player.x + (PLAYER_BOX_L + PLAYER_BOX_R) / 2) / 16;
                const s16 cy_ = (player.y + PLAYER_BOX_B) / 16;
                dbg_log("LKN %u%u%u/%u%u%u/%u%u%u fail=%u failmap=%u freekb=%u\n",
                    stage_terrain_at(stage_current_map, cx_ - 1, cy_ - 1),
                    stage_terrain_at(stage_current_map, cx_, cy_ - 1),
                    stage_terrain_at(stage_current_map, cx_ + 1, cy_ - 1),
                    stage_terrain_at(stage_current_map, cx_ - 1, cy_),
                    stage_terrain_at(stage_current_map, cx_, cy_),
                    stage_terrain_at(stage_current_map, cx_ + 1, cy_),
                    stage_terrain_at(stage_current_map, cx_ - 1, cy_ + 1),
                    stage_terrain_at(stage_current_map, cx_, cy_ + 1),
                    stage_terrain_at(stage_current_map, cx_ + 1, cy_ + 1),
                    transition_fail_code, transition_fail_map,
                    transition_fail_free_kb);
            }
            dbg_log("LKA scrolled=%u x=%d y=%d on_ladder=%u cam=%d under=%u\n",
                    scrolled, player.x, player.y, player.on_ladder, camera.x,
                    stage_terrain_at(stage_current_map,
                        (player.x + (PLAYER_BOX_L + PLAYER_BOX_R) / 2) / 16,
                        (player.y + PLAYER_BOX_B) / 16));
            lk_dbg_frames = 120;
#endif

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

#ifdef MD_PROFILE
        prof_tick(PROF_SETUP);  // closes loop tail + input + player + HUD
        prof_report();
#endif
        // camera focus = centre of the player hitbox
        stage_bg_tick();       // PC bg auto-scroll phase
        stage_update_camera((bl + br) / 2, (bt + bb) / 2);
        player_screen_edge_update();   // PC update_stage_scrolling

#ifdef MD_PROFILE
        prof_tick(PROF_CAMERA);
#endif
        stage_anim_update();
        stage_cycle_update();
#ifdef MD_PROFILE
        prof_tick(PROF_ANIM);
#endif

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
        if (!boss_music_started && enemies_boss_on_screen()) {
            music_play((stage == 13) ? MUSIC_FINAL_BOSS : MUSIC_BOSS);
            boss_music_started = TRUE;
        }
#ifdef MD_PROFILE
        prof_tick(PROF_ENEMY);
#endif
        explosion_update(); // enemy death puffs (PC death animation)
        effects_update();   // dust / splash / teleport / bubble
#ifdef MD_PROFILE
        prof_tick(PROF_EXPL);
#endif
        eshots_update();    // enemy AI projectiles -> player
#ifdef MD_PROFILE
        prof_tick(PROF_ESHOT);
#endif
        shot_update();
#ifdef MD_PROFILE
        prof_tick(PROF_PSHOT);
#endif
        hud_boss(enemies_boss_on_screen() ? (s16)enemies_boss_hp() : -1);
        tooltip_update();   // weapon name after a menu pick (PC 1.5 s)


#ifdef MD_PROFILE
        prof_tick(PROF_SHOT);
#endif
        SPR_update();
#ifdef MD_PROFILE
        prof_tick(PROF_SPRITE);
#endif
        SYS_doVBlankProcess();
#ifdef MD_PROFILE
        prof_tick(PROF_VBLANK);
#endif
        VDP_setReg(17, 0); // see above — DMA clobbers window regs
        VDP_setReg(18, 0);

        // PC classmap.cpp:2273 -> boss explosion + got_weapon() after the
        // stage boss falls. The sequence itself runs at the top of the loop.
        if (!boss_death_seq && enemies_boss_died()) {
            boss_death_seq = TRUE;
            enemies_boss_death_pos(&boss_death_x, &boss_death_y);
            shots_reset();
            eshots_reset();
            explosion_ring_start(boss_death_x, boss_death_y);
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
            const bool first_finish = !config.game_finished;
            config.game_finished = 1;
            config_save();
            music_play(MUSIC_END_ANIMATION);
            scene_play((u8)(2 + (current_player & 3)));
            ending_run(first_finish);
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
    music_stop();
    SYS_doVBlankProcess();      // flush pending MAP updates
    stage_release_map();
    // Reload the font/UI palette: the stage may have left CRAM 0 (the backdrop,
    // written per map by apply_bg_color) on a stage colour, which would tint
    // every transparent pixel of the text screen (BUG-030).
    pal_load(PAL0, pal_ui.data);
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
    music_play(MUSIC_GAME_OVER);
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
// PC defines.h:914/915: the stages that also grant an item weapon.
#define COIL_GOT_STAGE 1
#define JET_GOT_STAGE  6

// PC classic_style_got_weapon background: show_config_bg() blits
// images/shared/config_bg.png (320x240, the navy frame). Decomposed into tiles
// by tools/img2tiles.py; drawn on BG_A in PAL3 (the "screen art" line) while
// the text stays PAL0 and the player sprite PAL2.
#define GOTBG_TILE_BASE TILE_USER_INDEX

static void got_weapon_bg(void)
{
    // The stage leaves its camera scroll in both planes; the framed art and
    // the text are laid out from tile 0, so any leftover offset shifts them.
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_B, 0);
    VDP_loadTileData(gotbg_tiles_data, GOTBG_TILE_BASE, GOTBG_TILE_COUNT,
                     DMA_QUEUE);
    PAL_setPalette(PAL3, gotbg_pal.data, DMA_QUEUE);
    u16 *cells = screen_cells(GOTBG_MAP_W * GOTBG_MAP_H);
    if (cells) {
        for (u16 i = 0; i < GOTBG_MAP_W * GOTBG_MAP_H; i++)
            cells[i] = TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE,
                                      GOTBG_TILE_BASE + gotbg_map[i]);
        VDP_setTileMapDataRect(BG_A, cells, 0, 0, GOTBG_MAP_W, GOTBG_MAP_H,
                               GOTBG_MAP_W, CPU);
    }
    SYS_doVBlankProcess();
}

static void clear_to_color(u8 r, u8 g, u8 b)
{
    static u16 pal[16];
    for (u8 i = 0; i < 16; i++) pal[i] = pal_ui.data[i];
    pal[0] = RGB3_3_3_TO_VDPCOLOR((u8)((r * 7) / 255), (u8)((g * 7) / 255),
                                  (u8)((b * 7) / 255));
    PAL_setPalette(PAL0, pal, DMA_QUEUE);
    SYS_doVBlankProcess();
    const u16 blank = TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_FONT_INDEX);
    VDP_fillTileMapRect(BG_A, blank, 0, 0, 64, 32);
    VDP_fillTileMapRect(BG_B, blank, 0, 0, 64, 32);
}

// Hold for n frames with no input: the PC's stage-clear sequence is a fixed
// delay (`player_victory` 300ms x4 x2, then 3500ms; `classic_style_got_weapon`
// `wait_and_update_screen(5000)`), neither of which reads the pad. The port
// used to let START cut it short.
static void hold_frames(u16 n)
{
    for (u16 i = 0; i < n; i++) {
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
}

// PC graphLib::draw_progressive_text: the letters appear one by one. Returns
// the number of frames spent typing so the caller can keep the PC's total
// 5000 ms wait.
static u16 draw_progressive(const char *str, u16 tx, u16 ty)
{
    u16 frames = 0;
    for (const char *p = str; *p; p++) {
        char ch[2] = { *p, '\0' };
        VDP_drawTextBG(BG_A, ch, tx++, ty);
        SPR_update();
        SYS_doVBlankProcess();
        frames++;
    }
    return frames;
}

// PC game::player_victory / got_weapon / classic_style_got_weapon
// (game.cpp:1276 / 1320 / 1347). The MD cannot fill the screen with an
// arbitrary colour, so the flash puts the colour into PAL0 slot 0 (CRAM 0, the
// backdrop) with a queued palette write and clears both planes to a
// transparent tile: the screen then shows that colour with the player sprite on
// top, which is what the PC does (clear_area + player.show()).
static void showStageClear(u8 stage, bool first_clear)
{
    // PC shows the acquired weapon on a first clear of stages 1..8 only
    // (game::got_weapon: currentStage != 0 && <= 8).
    if (!first_clear || stage < 1 || stage > 8) return;

    // ---- game::player_victory(): pose, colour flash, 3.5s of the stage ----
    music_stop();
    sfx_play(SFX_EXPLOSION);
    hud_set_visible(FALSE);
    shots_reset();
    eshots_reset();
    player_force_anim(ANIM_GOT_WEAPON);

    static const u8 flash[4][3] = {
        { 251, 225, 101 }, { 101, 105, 251 },
        { 112, 251, 101 }, { 251, 101, 101 },
    };
    for (u8 round = 0; round < 2; round++) {
        for (u8 j = 0; j < 4; j++) {
            clear_to_color(flash[j][0], flash[j][1], flash[j][2]);
            player_sync_sprite();
            hold_frames(18);                       // PC timer.delay(300)
        }
    }

    music_play(MUSIC_GOT_WEAPON);
    // The stage again, with the player holding the pose (PC 3500ms).
    PAL_setPalette(PAL0, pal_ui.data, DMA_QUEUE);
    stage_restore_palette();
    stage_force_redraw();
    player_sync_sprite();
    hold_frames(210);

    // ---- game::classic_style_got_weapon(): the PC screen ----
    // blank_screen + blink_screen(255,255,255) + blank_screen, then
    // show_config_bg() (the framed navy art, NOT a flat fill) and the player
    // at x=20, vertically centred (game.cpp:1339-1360).
    clear_to_color(255, 255, 255);
    hold_frames(6);
    PAL_setPalette(PAL0, pal_ui.data, DMA_QUEUE);
    got_weapon_bg();
    camera.x = 0;
    camera.y = 0;
    player_place(20, 98);
    player_force_anim(ANIM_ATTACK);
    player_sync_sprite();

    // PC text x = RES_W*0.5-90 = 70px, i.e. 2px right of tile column 9 (the
    // closest the MD tile grid gets), lines at RES_H*0.5-4 / +8 / +20 =
    // rows 14 / 16 / 17. The second line names the ITEM the stage also gives
    // (game_data.player_items -> the object's name, uppercased): the port used
    // to print weapons 9/10 here, which are not the items at all.
    const char *item = NULL;
    if (stage == COIL_GOT_STAGE)      item = "COIL FROG";
    if (stage == JET_GOT_STAGE)       item = "JET EAGLE";

    u16 typed = draw_progressive(weapon_data[stage].name, 9, 14);
    u8 line3 = 16;
    if (item) {
        typed += draw_progressive("AND ", 9, 16);
        typed += draw_progressive(item, 13, 16);
        line3 = 17;
    }
    typed += draw_progressive(item ? "HAVE BEEN ACQUIRED" : "HAS BEEN ACQUIRED",
                              9, line3);
    player_sync_sprite();
    hold_frames((typed < 300) ? (u16)(300 - typed) : 0);   // PC wait 5000ms
    VDP_clearTextArea(0, 12, 40, 7);
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
    sprites_begin(SPRITE_VRAM_TILES); // see runStage()

    // The default DMA queue (80 entries) overflows during stage_load's force
    // map redraw / animated-frame upload (up to 17 slots x 6 frames), silently
    // dropping queued tileset/palette transfers. 192 entries covers that and
    // costs 3 KB instead of the old 8 KB, leaving the heap room for the
    // destination MAP allocated by a room transition (Mummy stairs).
    DMA_setMaxQueueSize(192);
    // The heap is tiny (BUG-040/BUG-045): if the bigger queue could not be allocated the
    // library is left with a NULL queue, and every queued transfer then reads garbage
    // commands from address 0 (slow flush, nothing lands). Fall back to a smaller one.
    // Largest queue that fits first (the stage load needs ~190 entries; 80 overflows it,
    // see the comment above), and only then the small one.
    for (u16 q = 160; dmaQueues == NULL && q >= 80; q -= 32) DMA_setMaxQueueSize(q);
    DMA_setMaxTransferSize(0); // unlimited DMA transfer per frame
#ifdef MD_TEST_ENDING
    dbg_print("=== BOOT: ending test hook ===\n");
#endif
    wd_ckq("main: after DMA_setMaxQueueSize");

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

    wd_ckq("main: before save_init");
    save_init();
    wd_ckq("main: after save_init");
    config_init();
    wd_ckq("main: after config_init");
#if defined(MD_NO_AUDIO) || defined(MD_PROFILE) || defined(MD_TEST_STAGE) || \
    defined(MD_LINK_TRACE) || defined(MD_TEST_CASTLE)
    config_force_silent();      // see config.c: silent test builds
#endif
    stage_set_data(0);   // valid stage_def before anything reads it
    wd_ckq("main: after stage_set_data(0)");

#ifdef MD_PROFILE
    // Profiling build only: skip intro/menu and boot straight into the stage
    // under test (MD_TEST_STAGE, default 0 = CHAOS CITY, the intro stage).
    current_save_slot = 0;
    current_player = 0;
    player_new_game();
#ifdef MD_TEST_STAGE
    playSession((u8)MD_TEST_STAGE);
#else
    playSession(0);
#endif
#endif

#ifdef MD_TEST_STAGE
    // Test build: boot straight into one stage with a fresh ROCKBOT, so a
    // reproduction does not need the stage unlocked (or a replay of the intro).
    current_save_slot = 0;
    current_player = 0;
    player_new_game();
    playSession((u8)MD_TEST_STAGE);
#endif

#ifdef MD_TEST_CASTLE
    // Test build: the castle intro (stage MD_TEST_CASTLE, 9 = first castle stage).
    current_save_slot = 0;
    current_player = 0;
    player_new_game();
    stage_set_data((u8)MD_TEST_CASTLE);
    dbg_print("castle test: heap free=%u largest=%u\n", (unsigned)MEM_getFree(),
            (unsigned)MEM_getLargestFreeBlock());
    boss_intro_run((u8)MD_TEST_CASTLE);
    dbg_print("castle test: done\n");
    for (;;) SYS_doVBlankProcess();
#endif

#ifdef MD_TEST_ENDING
    // Test build: the final castle's ending (scene + walking parade + enemy/boss
    // parade + credits), sound ON.
    current_save_slot = 0;
    current_player = 0;
    player_new_game();
    wd_ckq("hook: after player_new_game");
    stage_set_data(13);
    wd_ckq("hook: after stage_set_data(13)");
    wd_ckq("hook start");
    music_play(MUSIC_END_ANIMATION);
    wd_ck("hook after music");
    scene_play((u8)(2 + (current_player & 3)));
    ending_run(TRUE);
    // falls through to the title, like the real flow after the ending
#endif

#ifdef MD_MINIMAL
    // Bisection build: normal init has run (font + boot palettes in place), then
    // a hand-drawn options screen with no stage, sprites, music or SRAM.
    config_minimal_app();       // never returns
#endif
#ifndef MD_TEST_ENDING
    showIntro();
    // PC game_scenes_show_unbeaten_intro(): scene list 0 (scenes_data.c)
    music_play(MUSIC_INTRO_SCENE);
    scene_play(SCENE_OPENING_UNBEATEN);
#endif
    while (TRUE) {
        HEAP_PROBE("title");
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
            HEAP_PROBE("loaded");
            // PC scenesLib::pick_stage (sceneslib.cpp:529): the intro stage
            // is played first; once it is beaten the stage select appears.
            u8 stage = 0;
            if (save_slot_stages((u8)slot) & 1) {
                stage = stage_select_run(current_player,
                                         save_slot_stages(current_save_slot));
            }
            HEAP_PROBE("after select");
            playSession(stage);
        }
    }

    return 0;
}
