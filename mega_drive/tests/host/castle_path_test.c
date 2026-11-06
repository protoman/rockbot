/* The port's castle-path step generator (src/castle_path_draw.c) must plot exactly
 * what the PC's draw_castle_path plots. The reference (iteration counts and a CRC32
 * of the ordered pixel sequence per segment) is an independent Python
 * transliteration of the PC code: tools/convert_castle_path.py ->
 * tests/host/fixtures/castle_path_ref.h. */
#include <stdio.h>
#include <stdlib.h>

#include "genesis.h"
#include "castle_path_draw.h"
#include "fixtures/castle_path_ref.h"

static unsigned crc32_update(unsigned crc, const unsigned char *d, unsigned n)
{
    crc = ~crc;
    while (n--) {
        crc ^= *d++;
        for (int k = 0; k < 8; k++) crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

typedef struct { unsigned crc; unsigned n; } Acc;

static void plot(void *ctx, s16 x, s16 y, u8 w, u8 h, u8 color)
{
    Acc *a = ctx;
    for (s16 yy = y; yy < y + h; yy++) {
        for (s16 xx = x; xx < x + w; xx++) {
            const int ycr = yy - CASTLE_PATH_CROP_Y;
            const unsigned char b[5] = { (unsigned char)(xx & 0xFF), (unsigned char)((xx >> 8) & 0xFF),
                                         (unsigned char)(ycr & 0xFF), (unsigned char)((ycr >> 8) & 0xFF),
                                         (unsigned char)(color == CASTLE_PATH_GREY ? 1 : 2) };
            a->crc = crc32_update(a->crc, b, 5);
            a->n++;
        }
    }
}

int main(void)
{
    int bad = 0;
    for (unsigned s = 0; s < sizeof castle_ref / sizeof castle_ref[0]; s++) {
        u16 v, h;
        castle_seg_info((u8)s, &v, &h);
        Acc a = { 0, 0 };
        for (u16 i = 0; i < v + h; i++) castle_seg_iter((u8)s, i, plot, &a);
        const int ok = (v == castle_ref[s].v && h == castle_ref[s].h &&
                        a.n == castle_ref[s].n && a.crc == castle_ref[s].crc);
        printf("castle segment %u: v=%u h=%u pixels=%u crc=%08X  %s\n", s, v, h, a.n, a.crc,
               ok ? "ok" : "MISMATCH (reference v/h/n/crc differ)");
        if (!ok) bad++;
    }
    if (bad) { printf("FAIL: %d segment(s) differ from the PC draw_castle_path\n", bad); return 1; }
    printf("castle path: every segment matches the PC reference\n");
    return 0;
}
