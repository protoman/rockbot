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

// The heap is tiny on this screen (about 8 KB free in fragments, measured from a
// loaded save), so the tile data is built in small chunks, there is no RAM copy of
// the plane (cells are written to the VDP when their tile is uploaded) and
// everything goes to the VDP with CPU writes.
#define CHUNK_SLOTS 44
#define CHUNKS ((OVL_SLOTS + CHUNK_SLOTS - 1) / CHUNK_SLOTS)

static u32 *chunk[CHUNKS];          // CHUNK_SLOTS tiles each, built in RAM
static u16 *cell_of_slot;           // slot -> plane cell index
static u8 *slot_of;                 // cell -> slot + 1 (0 = none)
static u16 slots_used;
static u16 cells_written;           // slots whose plane cell is already on the VDP
static s16 dirty_lo, dirty_hi;      // tile slots changed since the last flush

static inline u32 *slot_rows(u16 slot)
{
    return &chunk[slot / CHUNK_SLOTS][(slot % CHUNK_SLOTS) * 8];
}

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
        cell_of_slot[slot] = cell;
    } else {
        slot = (u16)(slot_of[cell] - 1);
    }
    u32 *row = &slot_rows(slot)[y & 7];
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

// Upload every changed tile, then the plane cells of the slots that are new (tile
// first, so a cell never shows an unwritten tile).
static void ovl_flush(void)
{
    for (s16 sl = dirty_lo; sl <= dirty_hi; sl++)
        VDP_loadTileData(slot_rows((u16)sl), (u16)(OVL_TILE_BASE + sl), 1, CPU);
    dirty_lo = 0;
    dirty_hi = -1;
    for (; cells_written < slots_used; cells_written++) {
        const u16 cell = cell_of_slot[cells_written];
        VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE,
                                              OVL_TILE_BASE + cells_written),
                         (u16)(cell % CELLS_W), (u16)(cell / CELLS_W));
    }
}

static void ovl_commit_all(void)
{
    ovl_flush();
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
    MEM_pack();
    u8 got = 0;
    cell_of_slot = MEM_alloc(OVL_SLOTS * sizeof(u16));
    slot_of = MEM_alloc(CELLS_W * CELLS_H);
    for (; got < CHUNKS; got++) {
        chunk[got] = MEM_alloc(CHUNK_SLOTS * 32);
        if (!chunk[got]) break;
    }
    if (!cell_of_slot || !slot_of || got < CHUNKS) {    // no heap: the castle shows without its path
        kprintf("castle_path: out of heap (free=%u largest=%u)\n",
                (unsigned)MEM_getFree(), (unsigned)MEM_getLargestFreeBlock());
        for (u8 i = 0; i < got; i++) MEM_free(chunk[i]);
        if (cell_of_slot) MEM_free(cell_of_slot);
        if (slot_of) MEM_free(slot_of);
        return;
    }
    for (u8 i = 0; i < CHUNKS; i++) memset(chunk[i], 0, CHUNK_SLOTS * 32);
    memset(slot_of, 0, CELLS_W * CELLS_H);
    slots_used = 0;
    cells_written = 0;
    dirty_lo = 0;
    dirty_hi = -1;

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
    MEM_free(cell_of_slot);
    for (u8 i = 0; i < CHUNKS; i++) MEM_free(chunk[i]);
}
