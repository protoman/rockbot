#include "ending.h"
#include "dbgprint.h"
#include "sprite_guard.h"

#include <genesis.h>
#include <string.h>

#include "resources.h"
#include "player.h"
#include "music.h"
#include "stage_data.h"
#include "stage.h"
#include "ssel_data.h"
#include "credits_data.h"
#include "parade_data.h"
#include "tdump.h"
#include "walkdiag.h"
#include "pal_safe.h"
#include "starfield_data.h"
#include "boss_intro_bg_tiles.h"
#include "boss_intro_bg_map.h"
#include "end_l1_tiles.h"
#include "end_l1_map.h"
#include "end_l2_tiles.h"
#include "end_l2_map.h"
#include "end_l3_tiles.h"
#include "end_l3_map.h"

// PC scenesLib::show_player_walking_ending, show_enemies_ending and
// draw_lib::show_credits. The three walking layers use BG_B for the back
// layer and BG_A for the two front layers; the front layers scroll at
// different speeds through the per-line horizontal scroll table (the MD has
// only two planes but one scroll value per line).

#define END_BASE3 TILE_USER_INDEX
#define END_BASE2 (END_BASE3 + END_L3_TILE_COUNT)
#define END_BASE1 (END_BASE2 + END_L2_TILE_COUNT)

#define END_PAL3 PAL0
#define END_PAL2 PAL1
#define END_PAL1 PAL3

#define END_L3_ROW 1
#define END_L2_ROW 8
#define END_L1_ROW 15

#define END_WALK_X 336
#define END_WALK_Y 91

#define BLANK_TILE TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_FONT_INDEX)


static void black_screen(void)
{
    u16 blank = BLANK_TILE;
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_B, 0);
    VDP_setBackgroundColor(0);
    VDP_fillTileMapRect(BG_A, blank, 0, 0, 64, 32);
    VDP_fillTileMapRect(BG_B, blank, 0, 0, 64, 32);
}

static void center_text(const char *s, u16 row)
{
    u16 len = (u16)strlen(s);
    if (len > 40) len = 40;
    VDP_drawText(s, (40 - len) / 2, row);
}

// Hold for n frames. No input: the PC ending parts are timer-driven
// (`show_player_walking_ending` walks on `timer.delay`, `show_enemies_ending`
// holds 2000/3000ms, the credits pages scroll on a timer) - none of them is
// skippable. Only the final "press A" wait and the scene player accept input.
static void wait_frames(u16 n)
{
    while (n--) {
        SPR_update();
        SYS_doVBlankProcess();
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
    }
}

// Upload one repeated map into a plane. Every row repeats the source map
// horizontally so a 40-tile (320px) image can scroll across the 64-tile plane.
static void layer_upload(u16 plane, const u16 *map, u16 w, u16 h,
                         u16 base, u16 pal, u16 row)
{
    // One row at a time: this buffer used to be 64x20 (2.5 KB of BSS), which
    // shrank the heap enough that MAP_create could fail on some stages (black
    // screen, then a crash in stage_update_camera).
    static u16 cells[64];
    for (u16 y = 0; y < h; y++) {
        for (u16 x = 0; x < 64; x++) {
            u16 idx = map[y * w + (x % w)];
            cells[x] = TILE_ATTR_FULL(pal, FALSE, FALSE, FALSE, base + idx);
        }
        VDP_setTileMapDataRect(plane, cells, 0, row + y, 64, 1, 64, CPU);
    }
}

// The walking scene's tiles and palettes, queued for the next vblank flush.
#ifdef MD_WALK_DIAG
// Source/length/destination of each transfer about to be queued (the queue itself
// is private to SGDK): a source in RAM, a length of 0 or a transfer that crosses a
// 128 KB ROM bank would explain a flush that lands nothing.
static void wd_q(const char *what, const void *src, u16 bytes, u16 dst)
{
    const u32 a = (u32)src;
    dbg_print("Q %s: src=%06X len=%u dst=%04X bank_cross=%u\n", what, (unsigned)a,
              (unsigned)bytes, (unsigned)dst,
              (unsigned)(((a & 0x1FFFF) + bytes) > 0x20000));
}
#define WD_Q(what, src, bytes, dst) wd_q(what, src, bytes, dst)
#else
#define WD_Q(what, src, bytes, dst) ((void)0)
#endif

static void walking_gfx_queue(void)
{
    WD_Q("tiles L3", end_l3_tiles_data, END_L3_TILE_COUNT * 32, END_BASE3 * 32);
    WD_Q("tiles L2", end_l2_tiles_data, END_L2_TILE_COUNT * 32, END_BASE2 * 32);
    WD_Q("tiles L1", end_l1_tiles_data, END_L1_TILE_COUNT * 32, END_BASE1 * 32);
    WD_Q("pal L3", end_l3_pal.data, 32, END_PAL3 * 32);
    WD_Q("pal L2", end_l2_pal.data, 32, END_PAL2 * 32);
    WD_Q("pal L1", end_l1_pal.data, 32, END_PAL1 * 32);
    WD_Q("pal player", player_palette_def()->data, 32, PAL2 * 32);
    VDP_loadTileData(end_l3_tiles_data, END_BASE3, END_L3_TILE_COUNT, DMA_QUEUE);
    VDP_loadTileData(end_l2_tiles_data, END_BASE2, END_L2_TILE_COUNT, DMA_QUEUE);
    VDP_loadTileData(end_l1_tiles_data, END_BASE1, END_L1_TILE_COUNT, DMA_QUEUE);
    PAL_setPalette(END_PAL3, end_l3_pal.data, DMA_QUEUE);
    PAL_setPalette(END_PAL2, end_l2_pal.data, DMA_QUEUE);
    PAL_setPalette(END_PAL1, end_l1_pal.data, DMA_QUEUE);
    PAL_setPalette(PAL2, player_palette_def()->data, DMA_QUEUE);
}

#ifdef MD_WALK_DIAG
// BUG-045: read VRAM/CRAM back and compare with what the walking scene put there
// (tiles, the four palette lines, every plane row), and print one line per class
// with the count of bad items and the first bad one. VRAM reads go through
// tdump_read (DMA waited out, interrupts off), CRAM through PAL_getColors.
static void walk_verify(const char *tag)
{
    static u16 buf[64];
    struct { const u32 *data; u16 n; u16 base; const char *name; } sets[3] = {
        { end_l3_tiles_data, END_L3_TILE_COUNT, END_BASE3, "L3" },
        { end_l2_tiles_data, END_L2_TILE_COUNT, END_BASE2, "L2" },
        { end_l1_tiles_data, END_L1_TILE_COUNT, END_BASE1, "L1" },
    };
    for (u8 k = 0; k < 3; k++) {
        u16 bad = 0; s16 first = -1;
        for (u16 t = 0; t < sets[k].n; t++) {
            tdump_read((u16)((sets[k].base + t) * 32), buf, 16);
            const u16 *exp = (const u16 *)(sets[k].data + (u32)t * 8);
            for (u8 w = 0; w < 16; w++)
                if (buf[w] != exp[w]) { bad++; if (first < 0) first = (s16)t; break; }
        }
        dbg_print("VER %s tiles %s: bad=%u/%u first=%d\n", tag, sets[k].name, bad,
                  sets[k].n, (int)first);
    }
    const u16 *pals[4] = { end_l3_pal.data, end_l2_pal.data, player_palette_def()->data,
                           end_l1_pal.data };          // PAL0, PAL1, PAL2, PAL3
    for (u8 line = 0; line < 4; line++) {
        PAL_getColors((u16)(line * 16), buf, 16);
        u8 bad = 0; s8 first = -1;
        for (u8 i = 0; i < 16; i++)
            if ((buf[i] & 0x0EEE) != (pals[line][i] & 0x0EEE)) { bad++; if (first < 0) first = (s8)i; }
        dbg_print("VER %s PAL%u: bad=%u/16 first=%d got=%04X want=%04X\n", tag, line, bad,
                  (int)first, first >= 0 ? buf[(u8)first] : 0,
                  first >= 0 ? pals[line][(u8)first] : 0);
    }
    struct { u16 plane; const u16 *map; u16 w, h, base, pal, row; const char *name; } L[3] = {
        { BG_B, end_l3_map, END_L3_MAP_W, END_L3_MAP_H, END_BASE3, END_PAL3, END_L3_ROW, "BG_B L3" },
        { BG_A, end_l2_map, END_L2_MAP_W, END_L2_MAP_H, END_BASE2, END_PAL2, END_L2_ROW, "BG_A L2" },
        { BG_A, end_l1_map, END_L1_MAP_W, END_L1_MAP_H, END_BASE1, END_PAL1, END_L1_ROW, "BG_A L1" },
    };
    for (u8 k = 0; k < 3; k++) {
        const u16 addr = (L[k].plane == BG_A) ? VDP_getBGAAddress() : VDP_getBGBAddress();
        u16 bad = 0, firstRow = 0, firstCol = 0, got = 0, want = 0;
        for (u16 y = 0; y < L[k].h; y++) {
            tdump_read((u16)(addr + (L[k].row + y) * 128), buf, 64);
            for (u16 x = 0; x < 64; x++) {
                const u16 exp = TILE_ATTR_FULL(L[k].pal, FALSE, FALSE, FALSE,
                                               L[k].base + L[k].map[y * L[k].w + (x % L[k].w)]);
                if (buf[x] != exp) {
                    if (!bad) { firstRow = L[k].row + y; firstCol = x; got = buf[x]; want = exp; }
                    bad++;
                }
            }
        }
        dbg_print("VER %s map %s: bad=%u/%u first=(%u,%u) got=%04X want=%04X\n", tag,
                  L[k].name, bad, (unsigned)(L[k].h * 64), firstCol, firstRow, got, want);
    }
}
#define WALK_VERIFY(tag) walk_verify(tag)
#else
#define WALK_VERIFY(tag) ((void)0)
#endif

#ifdef MD_WALK_DIAG
// BUG-045: both glitching boots showed the queued DMA flush taking 17 vblanks and
// landing nothing (tiles/palettes unchanged), while CPU map writes verified fine.
// Try each way of getting the same data into VRAM/CRAM in one boot and verify after
// each, so the log says which ones work here.
static void walk_dma_experiments(void)
{
    vu16 *ctrl = (vu16 *)VDP_CTRL_PORT;
    VDP_waitVBlank(TRUE);
    u32 t0 = vtimer;
    u16 vc0 = GET_VCOUNTER;
    const u32 stA0 = getSubTick();
    const u16 busA0 = Z80_isBusTaken();
    DMA_flushQueue();                       // A: the same flush SYS_doVBlankProcess does
    dbg_print("EXP A subticks=%u bus %u->%u\n", (unsigned)(getSubTick() - stA0),
              (unsigned)busA0, (unsigned)Z80_isBusTaken());
    dbg_print("EXP A manual flush: dv=%u vc %u->%u status=%04X\n",
              (unsigned)((u32)vtimer - t0), vc0, (unsigned)GET_VCOUNTER, (unsigned)*ctrl);
    walk_verify("EXP A");

    t0 = vtimer;                            // B: immediate DMA (tiles L3 + PAL0)
    VDP_loadTileData(end_l3_tiles_data, END_BASE3, END_L3_TILE_COUNT, DMA);
    PAL_setPalette(END_PAL3, end_l3_pal.data, DMA);
    dbg_print("EXP B immediate DMA: dv=%u status=%04X\n", (unsigned)((u32)vtimer - t0),
              (unsigned)*ctrl);
    walk_verify("EXP B");

    t0 = vtimer;                            // C: CPU copy (tiles L2 + PAL1)
    VDP_loadTileData(end_l2_tiles_data, END_BASE2, END_L2_TILE_COUNT, CPU);
    pal_load(END_PAL2, end_l2_pal.data);
    dbg_print("EXP C CPU copy: dv=%u\n", (unsigned)((u32)vtimer - t0));
    walk_verify("EXP C");

    XGM2_pause();                           // D: queued flush with the music paused
    SYS_doVBlankProcess();
    SYS_doVBlankProcess();
    VDP_loadTileData(end_l1_tiles_data, END_BASE1, END_L1_TILE_COUNT, DMA_QUEUE);
    PAL_setPalette(END_PAL1, end_l1_pal.data, DMA_QUEUE);
    PAL_setPalette(PAL2, player_palette_def()->data, DMA_QUEUE);
    t0 = vtimer;
    SYS_doVBlankProcess();
    dbg_print("EXP D flush with music paused: dv=%u\n", (unsigned)((u32)vtimer - t0));
    walk_verify("EXP D");
    XGM2_resume();

    // E: whatever did not land goes in by CPU so the screen is right regardless
    VDP_loadTileData(end_l3_tiles_data, END_BASE3, END_L3_TILE_COUNT, CPU);
    VDP_loadTileData(end_l2_tiles_data, END_BASE2, END_L2_TILE_COUNT, CPU);
    VDP_loadTileData(end_l1_tiles_data, END_BASE1, END_L1_TILE_COUNT, CPU);
    pal_load(END_PAL3, end_l3_pal.data);
    pal_load(END_PAL2, end_l2_pal.data);
    pal_load(END_PAL1, end_l1_pal.data);
    pal_load(PAL2, player_palette_def()->data);
    walk_verify("EXP E (all by CPU)");
}
#endif

static void walking_parade(void)
{
    wd_ck("walk enter");
    music_play(MUSIC_END_ANIMATION);
    wd_ck("walk after music");
    sprites_end();
    wd_ck("walk after sprites_end");
    sprites_begin(SPRITE_VRAM_TILES);
    wd_ck("walk after sprites_begin");

    black_screen();
    wd_ck("walk after black_screen");
    walking_gfx_queue();
    wd_ck("walk queued");
    // Flush the queued transfers at a vblank NOW. The map rows below are long CPU
    // writes; with the transfers still pending, whether the tiles and the first
    // palette lines landed (or got overtaken) depended on where the next vblank
    // fell - the intermittent colour/tile garbage at the start of the walk.
    dbg_print("WALK: heap free=%u largest=%u dmaq=%u/%u bytes=%u\n",
              (unsigned)MEM_getFree(), (unsigned)MEM_getLargestFreeBlock(),
              (unsigned)DMA_getQueueSize(), (unsigned)DMA_getMaxQueueSize(),
              (unsigned)DMA_getQueueTransferSize());
#ifdef MD_WALK_DIAG
    walk_dma_experiments();
#else
    SYS_doVBlankProcess();
#endif
    wd_ck("walk flushed");
    WALK_VERIFY("after flush");

    layer_upload(BG_B, end_l3_map, END_L3_MAP_W, END_L3_MAP_H,
                 END_BASE3, END_PAL3, END_L3_ROW);
    wd_ck("walk after L3 map");
    layer_upload(BG_A, end_l2_map, END_L2_MAP_W, END_L2_MAP_H,
                 END_BASE2, END_PAL2, END_L2_ROW);
    layer_upload(BG_A, end_l1_map, END_L1_MAP_W, END_L1_MAP_H,
                 END_BASE1, END_PAL1, END_L1_ROW);
    wd_ck("walk after maps");
    WALK_VERIFY("after maps");

    Sprite *ply = SPR_addSprite(player_definition(), END_WALK_X, END_WALK_Y,
                                TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    dbg_print("WALK: sprite engine=%d player sprite=%d\n", (int)SPR_isInitialized(), (int)(ply != NULL));
    wd_ck("walk sprite added");
    if (ply) {
        spr_set_anim(ply, ANIM_WALK);
        SPR_setHFlip(ply, TRUE);   // PC: ANIM_DIRECTION_LEFT
    }

    // MD per-line scrolling proved unreliable here (only one plane moved), so
    // use the two planes as two speed layers: BG_A carries the front+mid art
    // (PC bg1 1.1 px/frame) and BG_B the back layer (0.1). PC mid is 0.5.
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);

    u16 frame = 0;
    for (;;) {
        // PC player_speed = -0.2 px/frame (1 px every 5 frames).
        s16 px = (s16)(END_WALK_X - ((u32)frame * 2 / 10));
        if (ply) SPR_setPosition(ply, px, END_WALK_Y);

        s16 o1 = (s16)(((u32)frame * 11 / 10) % 320);
        s16 o3 = (s16)(((u32)frame * 1 / 10) % 320);
        VDP_setHorizontalScroll(BG_A, o1);
        VDP_setHorizontalScroll(BG_B, o3);

#ifdef MD_WALK_DIAG
        const u32 st0 = getSubTick();
        const u16 bus0 = Z80_isBusTaken();
#endif
        SPR_update();
#ifdef MD_WALK_DIAG
        const u32 st1 = getSubTick();
#endif
        SYS_doVBlankProcess();
#ifdef MD_WALK_DIAG
        const u32 st2 = getSubTick();
        if (frame < 12)
            dbg_print("TIME f=%u spr=%u vblank=%u subticks (1280 = one frame) bus %u->%u\n",
                      (unsigned)frame, (unsigned)(st1 - st0), (unsigned)(st2 - st1),
                      (unsigned)bus0, (unsigned)Z80_isBusTaken());
#endif
        VDP_setReg(17, 0);
        VDP_setReg(18, 0);
#ifdef MD_WALK_DIAG
        {
            static u32 last_vt;
            const u32 dv = (u32)vtimer - last_vt;
            last_vt = (u32)vtimer;
            if (frame > 0 && dv != 1 && frame < 3000)
                dbg_print("SLOWFRAME f=%u dv=%u\n", (unsigned)frame, (unsigned)dv);
            if (frame < 5 || frame == 6 || frame == 8 || frame == 12 || frame == 20 ||
                frame == 30 || frame == 45 || (frame % 60) == 0) {
                char t[16];
                intToStr((s32)frame, t, 1);
                dbg_print("FRAME %u\n", (unsigned)frame);
                wd_ck(t);
                WALK_VERIFY(t);
            }
        }
#endif

        // Safety net for the intermittent start-of-walk garbage (tiles/palettes
        // that never landed): the transfers are idempotent and ~5 KB, so a few
        // re-sends turn a persistent glitch into at most a couple of bad frames.
        if (frame == 2 || frame == 8 || frame == 30) walking_gfx_queue();

        if (px < -48) break;
        frame++;
    }

    if (ply) SPR_releaseSprite(ply);
    SPR_update();
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
}

// PC show_enemies_ending is a long line of normal enemies; the port shows the
// bosses, which are the sprites every stage already carries. One boss per
// stage, name under it, fixed 100-frame hold (the PC ending reads no input).
// PC draw::show_boss_intro_bg: the boss-intro backdrop behind every parade page
// (same tiles/map/palette as bossintro.c). No enemy of the data carries a
// backdrop art of its own (bg_graphic_filename is empty in all 61 records).
#define PARADE_BG_PAL    PAL1
#define PARADE_BG_BASE   TILE_USER_INDEX
#define PARADE_SPR_Y     80             // BOSS_INTRO_BOSS_Y: sprite centre row
#define PARADE_NAME_ROW  23

static void parade_bg_load(void)
{
    VDP_loadTileData(boss_intro_bg_tiles_data, PARADE_BG_BASE,
                     BOSS_INTRO_BG_TILE_COUNT, DMA);
    PAL_setPalette(PARADE_BG_PAL, boss_intro_bg_pal.data, DMA_QUEUE);
    VDP_setTextPlane(BG_A);
    VDP_setTextPalette(PAL0);
    SYS_doVBlankProcess();
}

// Redraws the backdrop (it also wipes the previous page's text).
static void parade_bg_draw(void)
{
    static u16 cells[BOSS_INTRO_BG_MAP_W * BOSS_INTRO_BG_MAP_H];
    for (u16 i = 0; i < BOSS_INTRO_BG_MAP_W * BOSS_INTRO_BG_MAP_H; i++)
        cells[i] = TILE_ATTR_FULL(PARADE_BG_PAL, FALSE, FALSE, FALSE,
                                  PARADE_BG_BASE + boss_intro_bg_map[i]);
    VDP_fillTileMapRect(BG_A, BLANK_TILE, 0, 0, 64, 32);
    VDP_setTileMapDataRect(BG_A, cells, 0, 0, BOSS_INTRO_BG_MAP_W,
                           BOSS_INTRO_BG_MAP_H, BOSS_INTRO_BG_MAP_W, CPU);
}

// PC graphLib.draw_progressive_text: one character per ~60 ms (4 frames).
static void typed_text(const char *s, u16 col, u16 row)
{
    char ch[2] = { 0, 0 };
    for (u16 i = 0; s[i]; i++) {
        ch[0] = s[i];
        VDP_drawText(ch, col + i, row);
        wait_frames(4);
    }
}

static void boss_parade(void)
{
    music_play(MUSIC_ENEMY_PARADE);
    black_screen();
    PAL_setPalette(PAL0, pal_ui.data, DMA_QUEUE);
    SYS_doVBlankProcess();
    parade_bg_load();

    // PC show_enemies_ending: every enemy of the game list that is not a stage
    // boss, 3 s each (sprite + name), before the bosses.
    for (u8 i = 0; i < PARADE_COUNT; i++) {
        const ParadeEnemy *pe = &parade_enemies[i];
        Sprite *spr = SPR_addSprite(pe->spr, 160 - pe->w / 2, PARADE_SPR_Y - pe->h / 2,
                                    TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        if (spr) { spr_set_anim(spr, 0); SPR_setAnimationLoop(spr, FALSE); }   // intro frames play once
        parade_bg_draw();
        center_text(pe->name, PARADE_NAME_ROW);
        wait_frames(180);
        if (spr) SPR_releaseSprite(spr);
        SPR_update();
    }

    for (u8 stage = 0; stage < STAGE_DEFINED_COUNT; stage++) {
        stage_set_data(stage);

        const EnemyDef *boss = NULL;
        for (u8 i = 0; i < stage_def->enemy_def_count; i++) {
            if (stage_def->enemy_defs[i].is_boss) {
                boss = &stage_def->enemy_defs[i];
                break;
            }
        }
        if (!boss) continue;

        Sprite *spr = NULL;
        if (boss->spr)
            spr = SPR_addSprite(boss->spr,
                                160 - boss->spr->w / 2,
                                PARADE_SPR_Y - boss->spr->h / 2,
                                TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        if (spr) {
            // PC show_boss_intro_sprites: more than one INTRO frame plays once with
            // its own timing; otherwise frame 0 of the sheet, standing still.
            const u8 ia = boss->intro_anim;
            if (ia != 255 && ia < boss->spr->numAnimation &&
                boss->spr->animations[ia]->numFrame > 1) {
                spr_set_anim(spr, ia);
                SPR_setAnimationLoop(spr, FALSE);
            } else {
                spr_set_anim(spr, 0);
                spr_set_frame(spr, 0);
                SPR_setAutoAnimation(spr, FALSE);
            }
        }

        parade_bg_draw();
        if (stage < BOSS_CREDITS_COUNT) {
            // PC show_bosses_ending: id + boss name, CONCEPT and DESIGN credits typed
            // one character at a time (60 ms each), then a 2 s hold.
            const char *const *bc = &boss_credits[stage * 4];
            typed_text(bc[0], 1, 19);   typed_text(":", 1 + (u16)strlen(bc[0]), 19);
            typed_text(bc[1], 11, 19);
            typed_text("CONCEPT:", 1, 21);  typed_text(bc[2], 11, 21);
            typed_text("DESIGN:", 1, 23);   typed_text(bc[3], 11, 23);
            wait_frames(120);
        } else {
            wait_frames(180);
        }
        if (spr) SPR_releaseSprite(spr);
        SPR_update();
    }
}

// PC draw::show_credits_text: the list scrolls up 1 px per 50 ms until the last
// two lines ("PRESENTED BY" / "UPPERLAND STUDIOS") sit mid-screen, which then
// stay while "NEW CHARACTERS AVAILABLE" (first completion only) and "PRESS A TO
// CONTINUE" are shown. The MD scrolls BG_A with the vertical scroll register
// over a 32-row ring: a line every 16 px (the PC uses 12), one new tile row
// written per 8 px scrolled. Colours as the PC: "# " red, "- " blue, "@" mint.
#define CREDITS_FIRST_ROW 28            // world row of the first line (just below the screen)
#define CREDITS_TICK_FRAMES 2           // 1 px per 2 frames (PC: 1 px per 50 ms)

// TRUE when `line` contains the two-character marker `m` (PC std::string::find).
static bool has_marker(const char *line, const char *m)
{
    for (; line[0] && line[1]; line++)
        if (line[0] == m[0] && line[1] == m[1]) return TRUE;
    return FALSE;
}

static u16 credits_ink(u16 r, u16 g, u16 b)
{
    return RGB24_TO_VDPCOLOR(((u32)r << 16) | ((u32)g << 8) | b);
}

static void credits_set_palettes(void)
{
    // One buffer per line: DMA_QUEUE reads the source at the next vblank, so a
    // shared buffer left all four lines with the last one's colours (the
    // # / - / @ colours never showed).
    static u16 pal[4][16];
    const u16 ink[4] = { credits_ink(250, 250, 250), credits_ink(102, 255, 181),
                         credits_ink(249, 98, 98), credits_ink(95, 151, 255) };
    for (u8 p = 0; p < 4; p++) {
        for (u8 i = 0; i < 16; i++) pal[p][i] = ink[p];
        pal[p][0] = 0;
        // PAL3 also carries the star colours (entries 1..): the stars live on
        // BG_B, the text only uses entry 15.
        if (p == 3)
            for (u8 i = 0; i < STARFIELD_COLORS; i++) pal[p][1 + i] = starfield_pal[i];
        PAL_setPalette(p, pal[p], DMA_QUEUE);
    }
    SYS_doVBlankProcess();
}

// World row r of the credits (text lines on even rows from CREDITS_FIRST_ROW)
// into the ring row r % 32.
static void credits_put_row(s16 r)
{
    const u16 ring = (u16)(r & 31);
    VDP_fillTileMapRect(BG_A, BLANK_TILE, 0, ring, 64, 1);
    const s16 k = (s16)(r - CREDITS_FIRST_ROW);
    if (k < 0 || (k & 1) || (k >> 1) >= CREDITS_LINES) return;
    const char *line = credits_lines[k >> 1];
    u8 pal = 0;
    if (line[0] == '@') { pal = 1; line++; }
    else if (has_marker(line, "# ")) pal = 2;
    else if (has_marker(line, "- ")) pal = 3;
    if (!line[0]) return;
    VDP_setTextPalette(pal);
    u16 len = (u16)strlen(line);
    if (len > 40) len = 40;
    VDP_drawText(line, (40 - len) / 2, ring);
    VDP_setTextPalette(PAL0);
}

#define STAR_BASE TILE_USER_INDEX

// One frame of the credits screen: the text plane (BG_A) at the credits scroll
// `s`, the two star layers on BG_B in alternating 16 px bands at the PC speeds
// (star_field1 0.5 px and star_field2 3.5 px per 50 ms, both moving down).
static void credits_frame(s16 s, u32 f)
{
    static s16 va[20], vb[20];
    const s16 s1 = (s16)((f / 6) & 255);          // 1/6 px per frame
    const s16 s2 = (s16)(((f * 7) / 6) & 255);    // 7/6 px per frame
    for (u8 i = 0; i < 20; i++) {
        va[i] = s;
        vb[i] = (i & 1) ? (s16)-s2 : (s16)-s1;    // negative = the plane moves down
    }
    VDP_setVerticalScrollTile(BG_A, 0, va, 20, DMA_QUEUE);
    VDP_setVerticalScrollTile(BG_B, 0, vb, 20, DMA_QUEUE);
    SPR_update();
    SYS_doVBlankProcess();
    VDP_setReg(17, 0);
    VDP_setReg(18, 0);
}

static void credits_scroll(bool new_characters)
{
    music_play(MUSIC_END_CREDITS);
    black_screen();
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_COLUMN);
    // PAL0 holds the walking-parade palette at this point; the text palettes
    // are written through the DMA queue and flushed before drawing.
    credits_set_palettes();

    VDP_loadTileData(starfield_tiles, STAR_BASE, STARFIELD_TILES, DMA);
    {
        u16 row[40];
        for (u16 y = 0; y < 32; y++) {
            for (u16 x = 0; x < 40; x++)
                row[x] = TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE,
                                        STAR_BASE + starfield_map[y * 40 + x]);
            VDP_setTileMapDataRow(BG_B, row, y, 0, 40, CPU);
        }
    }

    for (s16 r = 0; r < 32; r++) credits_put_row(r);

    // The last line ends mid-screen (PC: posY + 12 * i = 100 for the final line).
    const s16 last_row = (s16)(CREDITS_FIRST_ROW + 2 * (CREDITS_LINES - 1));
    const s16 end_s = (s16)(((last_row * 8 - 104) + 7) & ~7);

    u32 f = 0;
    for (s16 s = 0; s <= end_s; s++) {
        if ((s & 7) == 0 && s > 0) credits_put_row((s16)((s >> 3) + 31));
        for (u8 t = 0; t < CREDITS_TICK_FRAMES; t++) credits_frame(s, f++);
    }

    // Frozen: the static messages go on the rows currently on screen, the stars
    // keep falling (PC keeps showing both until the player continues).
    const s16 top = (s16)(end_s >> 3);
    if (new_characters) {
        const char *w = "NEW CHARACTERS AVAILABLE";
        const u16 ring = (u16)((top + 24) & 31);
        VDP_fillTileMapRect(BG_A, BLANK_TILE, 0, ring, 64, 1);
        VDP_drawText(w, (40 - strlen(w)) / 2, ring);
    }
    {
        const char *c = "PRESS A TO CONTINUE";
        const u16 ring = (u16)((top + 26) & 31);
        VDP_fillTileMapRect(BG_A, BLANK_TILE, 0, ring, 64, 1);
        VDP_drawText(c, (40 - strlen(c)) / 2, ring);
    }
    // PC waits for JUMP; accept any face button or START here.
    for (u8 i = 0; i < 30; i++) credits_frame(end_s, f++);
    while (!(JOY_readJoypad(JOY_1) & (BUTTON_START | BUTTON_A | BUTTON_B |
                                      BUTTON_C)))
        credits_frame(end_s, f++);
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
}

void ending_run(bool new_characters)
{
    wd_ck("ending_run enter");
    walking_parade();
    boss_parade();
    credits_scroll(new_characters);

    sprites_end();
    black_screen();
}

void ending_credits_run(void)
{
    sprites_end();
    sprites_begin(SPRITE_VRAM_TILES);
    credits_scroll(FALSE);
    sprites_end();
    black_screen();
}
