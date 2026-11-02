/* Minimal stand-in for SGDK's <genesis.h>, used only by the host cost tests.
 *
 * The point of tests/host is to compile the REAL src/object.c for the host so
 * the per-frame work of objects_update() can be counted (terrain lookups,
 * predicate evaluations, sprite callbacks) without an emulator. Only the
 * declarations that src/object.c and its headers actually touch are here.
 */
#ifndef HOST_GENESIS_H
#define HOST_GENESIS_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef s16 fix16;

#define FIX16(x)      ((fix16)((x) * 64))
#define F16_toInt(x)  ((s16)((x) >> 6))

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#define VISIBLE 1
#define HIDDEN  0

/* planes / palettes */
typedef enum { BG_A = 0, BG_B = 1, WINDOW = 2 } VDPPlane;
#define PAL0 0
#define PAL1 1
#define PAL2 2
#define PAL3 3
#define TILE_ATTR(pal, prio, flipV, flipH) \
    ((u16)(((pal) << 13) | ((prio) ? 0x8000 : 0) | ((flipV) ? 0x1000 : 0) | \
           ((flipH) ? 0x0800 : 0)))
#define TILE_ATTR_FULL(pal, prio, flipV, flipH, index) \
    ((u16)(TILE_ATTR(pal, prio, flipV, flipH) | ((index) & 0x07FF)))
#define TILE_ATTR_PALETTE_MASK 0x6000
#define TILE_ATTR_PALETTE_SFT  13
#define TILE_ATTR_PRIORITY_MASK 0x8000
#define TILE_INDEX_MASK        0x07FF
#define TILE_USER_INDEX        0
#define TILE_FONT_INDEX        1440

#define RGB3_3_3_TO_VDPCOLOR(r, g, b) ((u16)(((r) << 1) | ((g) << 5) | ((b) << 9)))

/* resources */
typedef struct { u16 w, h; } SpriteDefinitionSize;
typedef struct {
    u16 w, h;
    u16 numTile;
    const u32 *data;
    u16 compression;       /* host transition sim: unpackTileSet target */
    u32 *tiles;
} TileSet;
#define COMPRESSION_NONE 0
#define DMA_QUEUE 1
#define CPU 0
#define VDP_BG_B BG_B
#define HSCROLL_PLANE 0
#define HSCROLL_TILE  2
#define HSCROLL_LINE  3
#define VSCROLL_PLANE 0
#define VSCROLL_2TILE 1
typedef struct {
    u16 w, h;
    const u16 *data;
} Palette;
typedef struct {
    u16 numAnimation;
    const u16 **animations;
    const TileSet **tilesets;
    const Palette *palette;
} SpriteDefinition;

typedef struct { s16 x, y; } VDPSprite;
typedef struct { s16 x, y; } Vect2D_s16;
typedef struct { u16 compression, numTile; const u32 *tiles; } TileSetRaw;
typedef struct {
    u16 w, h;
    const unsigned short *metaTiles;
    const unsigned short *tiles;
} MapDefinition;
typedef struct Map Map;
typedef struct Sprite Sprite;
struct Sprite {
    const SpriteDefinition *definition;
    s16 x, y;
    s16 animInd, frameInd;
    u16 status;
    u16 vis;        /* host test: last SPR_setVisibility value (VISIBLE until set) */
};

/* player controls (only the masks src/ uses at compile time) */
#define BUTTON_UP     0x0001
#define BUTTON_DOWN   0x0002
#define BUTTON_LEFT   0x0004
#define BUTTON_RIGHT  0x0008
#define BUTTON_A      0x0010
#define BUTTON_B      0x0020
#define BUTTON_C      0x0040
#define BUTTON_START  0x0080
#define JOY_1         0
#define JOY_readJoypad(joy) (0)

/* misc SDK entry points used by the code under test (all stubbed) */
void SPR_setPosition(Sprite *s, s16 x, s16 y);
void SPR_setVisibility(Sprite *s, u16 v);
void SPR_setHFlip(Sprite *s, bool flip);
void SPR_setAutoAnimation(Sprite *s, bool on);
Sprite *SPR_addSprite(const SpriteDefinition *def, s16 x, s16 y, u16 attr);
void SPR_releaseSprite(Sprite *s);
void SPR_update(void);
void SYS_doVBlankProcess(void);
void VDP_setReg(u16 reg, u8 value);
void VDP_setScrollingMode(u16 h, u16 v);
void VDP_setHorizontalScroll(VDPPlane p, s16 v);
void VDP_setVerticalScroll(VDPPlane p, s16 v);
void VDP_setVerticalScrollVSync(VDPPlane p, s16 v);
void VDP_setHorizontalScrollLine(VDPPlane p, u16 line, s16 *values, u16 len, u16 method);
void VDP_setTileMapData(VDPPlane p, const u16 *data, u16 index, u16 w, u16 h, u16 method);
void VDP_setTileMapXY(VDPPlane p, u16 tile, u16 x, u16 y);
void VDP_fillTileMapRect(VDPPlane p, u16 tile, u16 x, u16 y, u16 w, u16 h);
void VDP_clearTextArea(u16 x, u16 y, u16 w, u16 h);
void VDP_drawText(const char *str, u16 x, u16 y);
void VDP_drawTextBG(VDPPlane p, const char *str, u16 x, u16 y);
void VDP_setTextPlane(VDPPlane p);
void VDP_setTextPalette(u16 pal);
void VDP_setBackgroundColor(u16 c);
void VDP_setEnable(bool on);
void VDP_setTileMapDataRect(VDPPlane p, const u16 *data, u16 x, u16 y, u16 w, u16 h, u16 wm, u16 method);
bool unpackTileSet(const TileSet *src, TileSet *dst);
Map *MAP_create(const MapDefinition *def, VDPPlane plane, u16 base);
void MAP_getTilemapRect(Map *m, u16 x, u16 y, u16 w, u16 h, bool usePrio, u16 *out);
void MAP_release(Map *m);
void VDP_loadTileData(const u32 *data, u16 index, u16 num, u16 method);
void PAL_setPalette(u16 pal, const u16 *data, u16 method);
void PAL_setColor(u16 index, u16 value);
u16  PAL_getColor(u16 index);
void PAL_setColors(u16 index, const u16 *values, u16 num, u16 method);
void PAL_fadeOutAll(u16 num, bool async);
void PAL_fadeInAll(const u16 *pal, u16 num, bool async);
void DMA_setMaxQueueSize(u16 size);
void DMA_setMaxTransferSize(u16 size);
void kprintf(const char *fmt, ...);
u32  getSubTick(void);
u16  sdk_random(void);
#define random() sdk_random()

#endif /* HOST_GENESIS_H */
