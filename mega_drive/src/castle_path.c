#include "castle_path.h"

#include "castle_path_data.h"
#include "castle_path_draw.h"

// Overlay tiles live above the castle art (castle64: tiles 16..713). No sprite is
// alive on this screen, so the sprite pool region is free for them.
#define OVL_TILE_BASE 714
#define CELLS_W 40
#define CELLS_H 28
#define OVL_SLOTS (CASTLE_OVL_CELLS + 8)
#define ANIM_FRAMES 60              // PC CASTLE_PATH_DURATION 1000ms at 60Hz
#define UPLOAD_CHUNK 32             // tiles per vblank: the DMA queue holds 80 entries

static u32 *ovl_tiles;              // OVL_SLOTS tiles, built in RAM
static u16 *ovl_cells;              // BG_A plane entries, 40x28
static u8 *slot_of;                 // cell -> slot + 1 (0 = none)
static u16 slots_used;
static s16 dirty_lo, dirty_hi;      // tile slots changed since the last flush
static bool map_dirty;

static void ovl_pixel(s16 x, s16 y, u8 color)
{
    y -= CASTLE_PATH_CROP_Y;        // the port art is the PC image shifted up 8 rows
    if (x < 0 || x >= CELLS_W * 8 || y < 0 || y >= CELLS_H * 8) return;
    const u16 cell = (u16)((y >> 3) * CELLS_W + (x >> 3));
    u16 slot;
    if (!slot_of[cell]) {
        if (slots_used >= OVL_SLOTS) return;
        slot = slots_used++;
        slot_of[cell] = (u8)(slot + 1);
        ovl_cells[cell] = TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, OVL_TILE_BASE + slot);
        map_dirty = TRUE;
    } else {
        slot = (u16)(slot_of[cell] - 1);
    }
    u32 *row = &ovl_tiles[slot * 8 + (y & 7)];
    const u8 sh = (u8)(28 - 4 * (x & 7));
    *row = (*row & ~(0xFUL << sh)) | ((u32)color << sh);
    if (dirty_hi < dirty_lo) dirty_lo = dirty_hi = (s16)slot;
    if ((s16)slot < dirty_lo) dirty_lo = (s16)slot;
    if ((s16)slot > dirty_hi) dirty_hi = (s16)slot;
}

static void ovl_rect(void *ctx, s16 x, s16 y, u8 w, u8 h, u8 color)
{
    (void)ctx;
    for (s16 yy = y; yy < y + h; yy++)
        for (s16 xx = x; xx < x + w; xx++)
            ovl_pixel(xx, yy, color);
}

// 0-indexed nibble rows: `wpr` words per row (1 for 8 px wide, 2 for 16)
static void stamp(const u32 *bmp, u8 wpr, s16 x, s16 y)
{
    for (u8 r = 0; r < wpr * 8; r++) {
        for (u8 c = 0; c < wpr; c++) {
            const u32 w = bmp[r * wpr + c];
            for (u8 k = 0; k < 8; k++) {
                const u8 nib = (u8)((w >> (28 - 4 * k)) & 0xF);
                if (nib) ovl_pixel((s16)(x + c * 8 + k), (s16)(y + r), nib);
            }
        }
    }
}

static void ovl_flush(void)
{
    if (dirty_hi >= dirty_lo) {
        VDP_loadTileData(&ovl_tiles[dirty_lo * 8], (u16)(OVL_TILE_BASE + dirty_lo),
                         (u16)(dirty_hi - dirty_lo + 1), DMA_QUEUE);
        dirty_lo = 0;
        dirty_hi = -1;
    }
    if (map_dirty) {
        VDP_setTileMapDataRect(BG_A, ovl_cells, 0, 0, CELLS_W, CELLS_H, CELLS_W, DMA_QUEUE);
        map_dirty = FALSE;
    }
}

// Upload everything changed, chunked so the DMA queue never overflows, and show it.
static void ovl_commit_all(void)
{
    s16 lo = dirty_lo;
    const s16 hi = dirty_hi;
    dirty_lo = 0;
    dirty_hi = -1;
    for (; hi >= lo; lo = (s16)(lo + UPLOAD_CHUNK)) {
        const s16 end = (s16)((hi < lo + UPLOAD_CHUNK - 1) ? hi : lo + UPLOAD_CHUNK - 1);
        VDP_loadTileData(&ovl_tiles[lo * 8], (u16)(OVL_TILE_BASE + lo), (u16)(end - lo + 1),
                         DMA_QUEUE);
        SYS_doVBlankProcess();
    }
    if (map_dirty) {
        VDP_setTileMapDataRect(BG_A, ovl_cells, 0, 0, CELLS_W, CELLS_H, CELLS_W, DMA_QUEUE);
        map_dirty = FALSE;
    }
    SYS_doVBlankProcess();
}

static void marker(u8 point)
{
    const CastlePt *p = &castle_points[point];
    if (point == CASTLE_POINTS - 1) stamp(castle_skull_bmp, 2, p->x, p->y);   // the skull
    else stamp(castle_point_bmp, 1, p->x, p->y);
}

void castle_path_run(u8 idx)
{
    ovl_tiles = MEM_alloc(OVL_SLOTS * 32);
    ovl_cells = MEM_alloc(CELLS_W * CELLS_H * sizeof(u16));
    slot_of = MEM_alloc(CELLS_W * CELLS_H);
    if (!ovl_tiles || !ovl_cells || !slot_of) {         // no heap: the castle shows without its path
        kprintf("castle_path: out of heap (free=%u largest=%u)\n",
                (unsigned)MEM_getFree(), (unsigned)MEM_getLargestFreeBlock());
        if (ovl_tiles) MEM_free(ovl_tiles);
        if (ovl_cells) MEM_free(ovl_cells);
        if (slot_of) MEM_free(slot_of);
        return;
    }
    memset(ovl_tiles, 0, OVL_SLOTS * 32);
    memset(ovl_cells, 0, CELLS_W * CELLS_H * sizeof(u16));
    memset(slot_of, 0, CELLS_W * CELLS_H);
    slots_used = 0;
    dirty_lo = 0;
    dirty_hi = -1;
    map_dirty = FALSE;

    const u8 last = (idx < CASTLE_POINTS - 1) ? idx : (u8)(CASTLE_POINTS - 2);
    stamp(castle_point_bmp, 1, castle_points[0].x, castle_points[0].y);

    for (u8 s = 0; s <= last; s++) {
        u16 v, h;
        castle_seg_info(s, &v, &h);
        const u16 total = (u16)(v + h);
        if (s < last) {                                 // earlier segments: instant
            for (u16 i = 0; i < total; i++) castle_seg_iter(s, i, ovl_rect, NULL);
            marker((u8)(s + 1));
            ovl_commit_all();
        } else {                                        // the current one: ~1s
            ovl_commit_all();
            u16 done = 0;
            for (u16 f = 1; f <= ANIM_FRAMES; f++) {
                const u16 target = (u16)(((u32)total * f) / ANIM_FRAMES);
                while (done < target) castle_seg_iter(s, done++, ovl_rect, NULL);
                if (f == ANIM_FRAMES) marker((u8)(s + 1));
                ovl_flush();
                SYS_doVBlankProcess();
                VDP_setReg(17, 0);
                VDP_setReg(18, 0);
            }
        }
    }
    MEM_free(slot_of);
    MEM_free(ovl_cells);
    MEM_free(ovl_tiles);
}
