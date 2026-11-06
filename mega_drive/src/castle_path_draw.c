#include "castle_path_draw.h"

static s16 iabs(s16 v) { return (v < 0) ? -v : v; }

// Everything an iteration needs, derived from the two end points exactly as the
// PC computes it before its loops.
typedef struct {
    s16 pos_x;          // x of the vertical run (left border at pos_x - 1)
    s16 pos_y0;         // y of the first vertical iteration
    s16 vdir;           // -1 going up, +1 going down
    u16 v;              // vertical iterations
    s16 hy;             // y of the horizontal run (after the PC "remove extra bit")
    s16 hx0;            // x of the first horizontal iteration
    s16 hdir;           // -1 going left, +1 going right
    u16 h;              // horizontal iterations
} SegParams;

static void seg_params(u8 seg, SegParams *p)
{
    const s16 ix = castle_points[seg].x, iy = castle_points[seg].y;
    const s16 fx = castle_points[seg + 1].x, fy = castle_points[seg + 1].y;
    const s16 dist_x = ix - fx, dist_y = iy - fy;

    s16 pos_y = iy - 1;
    p->pos_x = ix + 2;
    if (dist_y < 0) pos_y += 9;
    p->pos_y0 = pos_y;
    p->vdir = (dist_y > 0) ? -1 : 1;
    s16 v = dist_y ? (iabs(dist_y) - 2) : 0;
    if (v < 0) v = 0;
    p->v = (u16)v;
    // the PC moves pos_y once per iteration, then trims it
    pos_y = (s16)(pos_y + p->vdir * v);
    pos_y += (dist_y > 0) ? 1 : -4;
    p->hy = pos_y;

    s16 temp_x = p->pos_x;
    s16 max_x = iabs(dist_x) - 2;
    if (dist_x > 0) { temp_x -= 1; max_x -= 4; }
    else if (dist_x < 0) { temp_x += 4; max_x -= 4; }
    p->hx0 = temp_x;
    p->hdir = (dist_x > 0) ? -1 : 1;
    p->h = (dist_x && max_x > 0) ? (u16)max_x : 0;
}

void castle_seg_info(u8 seg, u16 *v, u16 *h)
{
    SegParams p;
    seg_params(seg, &p);
    *v = p.v;
    *h = p.h;
}

void castle_seg_iter(u8 seg, u16 i, CastlePlotRect plot, void *ctx)
{
    SegParams p;
    seg_params(seg, &p);
    if (i < p.v) {
        const s16 y = (s16)(p.pos_y0 + p.vdir * (s16)i);
        plot(ctx, p.pos_x - 1, y, 1, 1, CASTLE_PATH_DARK);     // border left
        plot(ctx, p.pos_x, y, 4, 1, CASTLE_PATH_GREY);         // middle
        plot(ctx, p.pos_x + 4, y, 1, 1, CASTLE_PATH_DARK);     // border right
    } else {
        const u16 j = (u16)(i - p.v);
        if (j >= p.h) return;
        const s16 x = (s16)(p.hx0 + p.hdir * (s16)j);
        plot(ctx, x, p.hy - 1, 1, 1, CASTLE_PATH_DARK);        // top
        plot(ctx, x, p.hy, 1, 4, CASTLE_PATH_GREY);            // middle
        plot(ctx, x, p.hy + 4, 1, 1, CASTLE_PATH_DARK);        // bottom
    }
}
