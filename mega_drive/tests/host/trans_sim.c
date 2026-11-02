/* Host simulation of the vertical room transition (BUG: flicker / misalignment /
 * stutter, plan: transition_plan.md phase 1).
 *
 * Compiles the REAL src/transition.c against a model of the VDP: two 64x32 name
 * tables, scroll modes, the DMA queue (applied at each vblank) and the sprite
 * list. Map cells are synthetic: every cell encodes (map, source column, source
 * row) in its tile id / palette bits, so what a frame DISPLAYS can be decoded
 * back to "which source line of which map is on screen line y, at which x".
 *
 *   trans_sim <t2b> <from_w> <to_w> <from_cam> <to_cam> <old_shift> <n_prev>
 *
 * Output (one line per metric, parsed by tools/check_transitions.py):
 *   SLIDE frames=N p=[...]            picture offset p (px) at every vblank
 *   TEAR  n=...                       frames whose lines disagree about p
 *   HMIS  old=.. new=..               cells whose column is not the expected one
 *   FIRST p=.. LAST p=..              first / last picture offset
 *   DMA   max_bytes=.. max_entries=..
 *   SPR   max_per_line=..
 */
#include <genesis.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stage_data.h"
#include "stage.h"
#include "player.h"
#include "transition.h"

/* ---- stage fixtures ----------------------------------------------------- */
#define MAXMAPS 3
static StageBgInfo bg_info[MAXMAPS];
static MapDefinition map_defs_arr[MAXMAPS];
static const MapDefinition *map_defs_ptr[MAXMAPS];
static TileSet tile_sets_arr[MAXMAPS];
static const TileSet *tile_sets_ptr[MAXMAPS];
static u8 tile3[MAXMAPS][200 * 14 / 8 + 8];
static u8 anim_slot_n[MAXMAPS];
static EnemyDef enemy_defs[1];
static EnemySpawn enemy_spawns[32];
static StageDef the_stage;
static SpriteDefinition dummy_spr;
/* parallax fixtures: bg tile k is image row k (its first word is k), map cell = row */
#define BG_H 20
static u32 bg_tiles_arr[BG_H * 8];
static u16 bg_map_arr[BG_H * 64];
static u16 bg_map_h_arr[MAXMAPS];
static u16 bg_tiles_n_arr[MAXMAPS];
static const u32 *bg_tiles_ptr[MAXMAPS];
static const u16 *bg_maps_ptr[MAXMAPS];
static u16 bg_pal_words[16];
static Palette bg_pal_arr = { 16, 1, bg_pal_words };
static const Palette *bg_pal_ptr[MAXMAPS];

struct Map { int id; };
static struct Map maps[MAXMAPS];

const StageDef *stage_def;
u8 stage_current_map;
Vect2D_s16 camera;
Map *stage_map;
u16 stage_bg_tile_base;
u16 spr_gen;
Player player;

u16 stage_sky_tile(void) { return 1300; }
u16 stage_layout_anim(u8 map) { (void)map; return 300; }   /* new tiles land >= 300 */
s16 stage_bg_auto_hscroll(u8 map) { (void)map; return 0; }
void stage_release_map(void) { stage_map = NULL; }
u32 MEM_getFree(void) { return 30000; }
s16 player_sprite_dx(void) { return 0; }
void spr_set_anim(Sprite *s, s16 a) { if (s) s->animInd = a; }
void enemies_shift_y(s16 dy) { (void)dy; }
void enemies_anim_hold(bool hold) { (void)hold; }
void objects_shift_y(s16 dy) { (void)dy; }

static int started;
/* ---- VDP model ---------------------------------------------------------- */
static u16 plane[2][32 * 64];
static u32 vram_tile_id[2048];
static int hmode, vmode;
static s16 hs_plane[2], vs_plane[2];
static s16 hs_line[2][256];

typedef struct { int kind; VDPPlane p; u16 idx, w, h; u16 data[64 * 32]; s16 line[256]; u16 len; } Op;
static Op queue[256];
static int qn;
static unsigned qbytes_frame, qent_frame;

static void apply(Op *o)
{
    if (o->kind == 0) {                 /* tilemap rect: rows of w cells at plane (x=idx, y=...) */
        for (u16 r = 0; r < o->h; r++)
            for (u16 c = 0; c < o->w; c++)
                plane[o->p][((r + (o->idx >> 8)) & 31) * 64 + ((c + (o->idx & 255)) & 63)] =
                    o->data[r * o->w + c];
    } else if (o->kind == 1) {          /* tilemap data, linear index */
        for (u16 i = 0; i < o->w * o->h; i++)
            plane[o->p][(o->idx + i) & (32 * 64 - 1)] = o->data[i];
    } else if (o->kind == 2) {          /* hscroll lines */
        for (u16 i = 0; i < o->len; i++) hs_line[o->p][o->idx + i] = o->line[i];
    }
}

void VDP_setReg(u16 r, u8 v) { (void)r; (void)v; }
void VDP_setScrollingMode(u16 h, u16 v) { hmode = h; vmode = v; }
static int vs_immediate;
static s16 vs_pending[2]; static int vs_pend[2];
void VDP_setVerticalScroll(VDPPlane p, s16 v) { vs_plane[p] = v; if (started && p == BG_A) vs_immediate++; }
void VDP_setVerticalScrollVSync(VDPPlane p, s16 v) { vs_pending[p] = v; vs_pend[p] = 1; }
void VDP_setHorizontalScroll(VDPPlane p, s16 v) { hs_plane[p] = v; }

void VDP_setTileMapDataRect(VDPPlane p, const u16 *data, u16 x, u16 y, u16 w, u16 h, u16 wm, u16 method)
{
    Op o = { .kind = 0, .p = p, .idx = (u16)((y << 8) | x), .w = w, .h = h };
    for (u16 r = 0; r < h; r++) memcpy(&o.data[r * w], &data[r * wm], w * 2);
    if (method == CPU) { apply(&o); return; }
    queue[qn++] = o;
    qent_frame += h;                    /* assumption: one DMA entry per row (stride != plane width) */
    qbytes_frame += w * h * 2;
}

void VDP_setTileMapData(VDPPlane p, const u16 *data, u16 index, u16 w, u16 h, u16 method)
{
    /* SGDK: VDP_setTileMapData(planeAddr, data, ind, num, vramStep, tm): `w` is num, `h` the step */
    Op o = { .kind = 1, .p = p, .idx = index, .w = w, .h = 1 };
    (void)h;
    memcpy(o.data, data, w * 2);
    if (method == CPU) { apply(&o); return; }
    queue[qn++] = o;
    qent_frame += 1;
    qbytes_frame += w * 2;
}

void VDP_setHorizontalScrollLine(VDPPlane p, u16 line, s16 *values, u16 len, u16 method)
{
    Op o = { .kind = 2, .p = p, .len = len, .idx = line };
    memcpy(o.line, values, len * 2);
    if (method == CPU) { apply(&o); return; }
    queue[qn++] = o;
    qent_frame += 1;
    qbytes_frame += len * 2;
}

void VDP_loadTileData(const u32 *data, u16 index, u16 num, u16 method)
{
    (void)method;
    for (u16 k = 0; k < num; k++) vram_tile_id[index + k] = data[k * 8];
    qent_frame += 1;
    qbytes_frame += num * 32;
}

/* ---- sprites ------------------------------------------------------------ */
static Sprite spool[64];
static int sused;
static const int SPR_H = 32;
Sprite *SPR_addSprite(const SpriteDefinition *d, s16 x, s16 y, u16 a)
{
    (void)a;
    if (sused >= 64) return NULL;
    Sprite *s = &spool[sused++];
    s->definition = d; s->x = x; s->y = y; s->vis = VISIBLE;
    return s;
}
void SPR_releaseSprite(Sprite *s) { if (s) s->definition = NULL; }
void SPR_setPosition(Sprite *s, s16 x, s16 y) { if (s) { s->x = x; s->y = y; } }
void SPR_setVisibility(Sprite *s, u16 v) { if (s) s->vis = v; }
void SPR_setHFlip(Sprite *s, bool f) { (void)s; (void)f; }
void SPR_setAutoAnimation(Sprite *s, bool o) { (void)s; (void)o; }
void SPR_update(void) {}

/* ---- map / tileset ------------------------------------------------------- */
Map *MAP_create(const MapDefinition *def, VDPPlane plane_, u16 base)
{
    (void)plane_; (void)base;
    for (int i = 0; i < MAXMAPS; i++) if (def == &map_defs_arr[i]) return &maps[i];
    return NULL;
}
void MAP_release(Map *m) { (void)m; }

void MAP_getTilemapRect(Map *m, u16 x, u16 y, u16 w, u16 h, bool prio, u16 *out)
{
    (void)prio;
    for (u16 cy = 0; cy < h * 2; cy++)
        for (u16 cx = 0; cx < w * 2; cx++) {
            const u16 gcol = (u16)(x * 2 + cx);
            const u16 id = (u16)((gcol % 9) * 28 + (y * 2 + cy));
            out[cy * (w * 2) + cx] = (u16)(TILE_ATTR_FULL(m->id, FALSE, FALSE, FALSE, id));
        }
}

static u32 tile_words[256 * 8];
bool unpackTileSet(const TileSet *src, TileSet *dst)
{
    (void)src;
    for (int i = 0; i < 256; i++) dst->tiles[i * 8] = (u32)i;
    dst->numTile = 256;
    (void)tile_words;
    return TRUE;
}

/* ---- the observer -------------------------------------------------------- */
static int t2b, from_w, to_w;
static s16 ocam_exp, dcam_exp;
#define MAXF 400
static int frame_n;
static int pseq[MAXF];
static int tear_n, hmis_old, hmis_new, max_bytes, max_entries, max_per_line;
static int hscroll_equal_halves;
static int first_bytes, first_entries;
static int old_py, bgmis_n, bgadj_from, bgadj_to;
#define MAXF2 400
static int pyseq[MAXF2];

typedef struct { int map, colmod, row; } Dec;
static Dec decode(u16 cell)
{
    const u32 id = vram_tile_id[cell & 0x7FF];
    Dec d = { (cell >> 13) & 3, (int)(id / 28), (int)(id % 28) };
    return d;
}

static void observe(void)
{
    if (!started) return;
    int p_common = -9999, tear = 0;
    for (int y = 0; y < 224; y++) {
        const s16 hs = (hmode == HSCROLL_LINE) ? hs_line[0][y] : hs_plane[0];
        const int prow = ((y + vs_plane[0]) >> 3) & 31;
        /* the cell shown at the pixel column x=4 and the line inside the tile */
        Dec d = decode(plane[0][prow * 64 + ((((0 + 4) - hs) & 511) >> 3)]);
        const int s = d.row * 8 + ((y + vs_plane[0]) & 7);
        const int old = (d.map == 0) ? 1 : 0;
        (void)old;
        /* maps: from = 1 (id 1), to = 2 (id 2) */
        const int is_old = (d.map == 1);
        int z = is_old ? (t2b ? s : s + 224) : (t2b ? s + 224 : s);
        int p = z - y;
        if (p_common == -9999) p_common = p;
        else if (p != p_common) tear = 1;
        /* horizontal: every sampled cell must be the expected source column */
        const s16 cam = is_old ? ocam_exp : dcam_exp;
        for (int x = 4; x < 320; x += 8) {
            Dec e = decode(plane[0][prow * 64 + (((x - hs) & 511) >> 3)]);
            const int want = ((cam + x) >> 3) % 9;
            if (e.colmod != want) { if (is_old) hmis_old++; else hmis_new++; }
        }
    }
    /* parallax (BG_B): line y of the OLD half must show image line y + v_old (v = -(adjust_y-16)),
     * the NEW half y + v_new within 7 px (one VSRAM value serves both) */
    for (int y = 0; y < 224; y++) {
        const int prow = ((y + vs_plane[1]) >> 3) & 31;
        const s16 hsb = (hmode == HSCROLL_LINE) ? hs_line[1][y] : hs_plane[1];
        const u16 cell = plane[1][prow * 64 + (((4 - hsb) & 511) >> 3)];
        const u32 id = vram_tile_id[cell & 0x7FF];
        /* which half is this line? from the BG_A decode */
        Dec d = decode(plane[0][(((y + vs_plane[0]) >> 3) & 31) * 64 + (((4 - ((hmode == HSCROLL_LINE) ? hs_line[0][y] : hs_plane[0])) & 511) >> 3)]);
        const int is_old = (d.map == 1);
        const int v = -((is_old ? bgadj_from : bgadj_to) - 16);
        const int want = (y + v) & 255;              /* plane line wanted at this screen line (the plane wraps at 256) */
        if (want < 0 || want >= BG_H * 8) { if (id != 9999 && id < BG_H) { bgmis_n++; if (getenv("BGDBG") && frame_n < 2) fprintf(stderr, "f%d y=%d old=%d want=%d (outside art) shows id=%u\n", frame_n, y, is_old, want, id); } continue; }   /* outside the art: sky */
        if (id == 9999 || id >= BG_H) { bgmis_n++; if (getenv("BGDBG") && frame_n < 2) fprintf(stderr, "f%d y=%d old=%d want=%d SKY/other id=%u\n", frame_n, y, is_old, want, id); continue; }
        const int got = (int)id * 8 + ((y + vs_plane[1]) & 7);
        int err = got - want; if (err < 0) err = -err;
        if (err > (is_old ? 0 : 7)) { bgmis_n++; if (getenv("BGDBG") && frame_n < 2) fprintf(stderr, "f%d y=%d old=%d want=%d got=%d id=%u vsB=%d\n", frame_n, y, is_old, want, got, id, vs_plane[1]); }
    }
    if (tear) tear_n++;
    if (frame_n < MAXF) { pseq[frame_n] = tear ? -1 : p_common; pyseq[frame_n] = player.spr ? player.spr->y : -999; }
    frame_n++;
    if (frame_n == 1) { first_bytes = (int)qbytes_frame; first_entries = (int)qent_frame; }
    else {
        if ((int)qbytes_frame > max_bytes) max_bytes = (int)qbytes_frame;
        if ((int)qent_frame > max_entries) max_entries = (int)qent_frame;
    }
    /* sprites per scanline */
    int cnt[224 + 64] = {0};
    for (int i = 0; i < sused; i++) {
        Sprite *s = &spool[i];
        if (!s->definition || s->vis != VISIBLE) continue;
        for (int l = s->y; l < s->y + SPR_H; l++)
            if (l >= 0 && l < 224) cnt[l]++;
    }
    for (int l = 0; l < 224; l++) if (cnt[l] > max_per_line) max_per_line = cnt[l];
}

void SYS_doVBlankProcess(void)
{
    for (int i = 0; i < qn; i++) apply(&queue[i]);
    for (int k = 0; k < 2; k++) if (vs_pend[k]) { vs_plane[k] = vs_pending[k]; vs_pend[k] = 0; }
    if (!started && hmode == HSCROLL_LINE) started = 1;
    observe();
    qn = 0; qbytes_frame = 0; qent_frame = 0;
}

int main(int argc, char **argv)
{
    if (argc < 8) { fprintf(stderr, "usage: trans_sim t2b from_w to_w from_cam to_cam old_shift n_prev\n"); return 2; }
    t2b = atoi(argv[1]); from_w = atoi(argv[2]); to_w = atoi(argv[3]);
    const int from_cam = atoi(argv[4]), to_cam = atoi(argv[5]), old_shift = atoi(argv[6]);
    const int n_prev = atoi(argv[7]);

    for (int i = 0; i < MAXMAPS; i++) {
        map_defs_arr[i].w = 0;
        map_defs_ptr[i] = &map_defs_arr[i];
        maps[i].id = i;
        tile_sets_ptr[i] = &tile_sets_arr[i];
        memset(tile3[i], 0, sizeof tile3[i]);
    }
    the_stage.map_count = MAXMAPS; the_stage.map_h = 14; the_stage.tile = 16;
    the_stage.map_w = (u8)((from_w > to_w) ? from_w : to_w);   /* one map_w per stage in the data */
    the_stage.map_defs = map_defs_ptr;
    the_stage.tile_sets = tile_sets_ptr;
    the_stage.bg_info = bg_info;
    the_stage.tile3_mask = &tile3[0][0];
    the_stage.anim_slot_n = anim_slot_n;
    the_stage.enemy_defs = enemy_defs;
    the_stage.enemy_spawns = enemy_spawns;
    the_stage.enemy_spawn_count = (u16)n_prev;
    enemy_defs[0].spr = &dummy_spr;
    for (int i = 0; i < n_prev; i++) {
        enemy_spawns[i].def = 0; enemy_spawns[i].map = 2;
        enemy_spawns[i].x = (s16)(to_cam + 10 + i * 20); enemy_spawns[i].y = 80;
    }
    const int adj_from = (argc > 9) ? atoi(argv[9]) : 0, adj_to = (argc > 10) ? atoi(argv[10]) : 0;
    bgadj_from = adj_from; bgadj_to = adj_to;
    for (int k = 0; k < BG_H; k++) { bg_tiles_arr[k * 8] = (u32)k; for (int x = 0; x < 64; x++) bg_map_arr[k * 64 + x] = (u16)k; }
    for (int i = 0; i < MAXMAPS; i++) {
        bg_info[i].present = 1; bg_info[i].adjust_y = (s16)((i == 1) ? adj_from : adj_to); bg_info[i].speed = 0;
        bg_tiles_ptr[i] = bg_tiles_arr; bg_maps_ptr[i] = bg_map_arr;
        bg_map_h_arr[i] = BG_H; bg_tiles_n_arr[i] = BG_H; bg_pal_ptr[i] = &bg_pal_arr;
    }
    the_stage.bg_tiles = bg_tiles_ptr; the_stage.bg_maps = bg_maps_ptr;
    the_stage.bg_map_h = bg_map_h_arr; the_stage.bg_tiles_n = bg_tiles_n_arr; the_stage.bg_pal = bg_pal_ptr;
    stage_bg_tile_base = 1200;
    stage_def = &the_stage;
    stage_map = &maps[1];
    stage_current_map = 1;
    camera.x = (s16)from_cam;
    spool[sused].definition = &dummy_spr; spool[sused].x = 100; spool[sused].y = 150;
    spool[sused].vis = VISIBLE;
    player.spr = &spool[sused++];
    player.x = 150;
    player.y = (argc > 8) ? (s16)atoi(argv[8]) : 150;   /* where the old room draws him */
    old_py = player.y;
    for (int i = 0; i < 2048; i++) vram_tile_id[i] = (u32)i;
    for (int k = 0; k < BG_H; k++) vram_tile_id[1200 + k] = (u32)k;     /* outgoing room's resident bg */
    vram_tile_id[1300] = 9999;                                           /* sky */
    hmode = HSCROLL_PLANE;

    const int max_cam_from = from_w * 16 - 320, max_cam_to = to_w * 16 - 320;
    int fc = from_cam; if (fc < 0) fc = 0; if (fc > max_cam_from) fc = max_cam_from;
    ocam_exp = (s16)(fc - old_shift);
    if (ocam_exp < 0) ocam_exp = 0; if (ocam_exp > max_cam_from) ocam_exp = (s16)max_cam_from;
    int dc = to_cam; if (dc < 0) dc = 0; if (dc > max_cam_to) dc = max_cam_to;
    dcam_exp = (s16)dc;

    /* maps: from = id 1, to = id 2 (so map 0 never appears) */
    const bool ok = transition_scroll(1, 2, (s16)to_cam, 150, 100, t2b != 0, (s16)old_shift);
    printf("RESULT ok=%d\n", ok);
    printf("SLIDE frames=%d p=[", frame_n);
    for (int i = 0; i < frame_n && i < MAXF; i++) printf("%s%d", i ? "," : "", pseq[i]);
    printf("]\n");
    printf("PLY old=%d y=[", old_py);
    for (int i = 0; i < frame_n && i < MAXF; i++) printf("%s%d", i ? "," : "", pyseq[i]);
    printf("]\n");
    printf("TEAR n=%d\n", tear_n);
    printf("HMIS old=%d new=%d\n", hmis_old, hmis_new);
    printf("DMA max_bytes=%d max_entries=%d first_bytes=%d first_entries=%d\n", max_bytes, max_entries, first_bytes, first_entries);
    printf("SPR max_per_line=%d\n", max_per_line);
    printf("VSIMM n=%d\n", vs_immediate);
    printf("BGMIS n=%d\n", bgmis_n);
    (void)hscroll_equal_halves;
    return 0;
}
