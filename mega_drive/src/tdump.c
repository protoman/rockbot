// Temporary diagnostics for the "white stripes" bug. Enabled with
// EXTRA_FLAGS=-DMD_TILE_DUMP, printed through kprintf (BlastEm: stdout).
#include "tdump.h"

#ifdef MD_TILE_DUMP

#include <genesis.h>

// VDP registers that decide what a transparent pixel shows and where the
// planes live: 2/4 plane bases, 3/5 plane sizes, 7 backdrop colour index,
// 11/12 scroll modes+interlace, 17/18 window plane.
void tdump_vdp(const char *tag)
{
    kprintf("TDUMP %s: reg2=%02X reg3=%02X reg4=%02X reg5=%02X reg7=%02X "
            "reg11=%02X reg12=%02X reg17=%02X reg18=%02X\n",
            tag, VDP_getReg(2), VDP_getReg(3), VDP_getReg(4), VDP_getReg(5),
            VDP_getReg(7), VDP_getReg(11), VDP_getReg(12), VDP_getReg(17),
            VDP_getReg(18));
    kprintf("TDUMP %s: BGA=%04X BGB=%04X win_w=%u textplane=%d textpal=%u\n",
            tag, VDP_getBGAAddress(), VDP_getBGBAddress(), windowWidth,
            (int)VDP_getTextPlane(), VDP_getTextPalette());
}

void tdump_cram(const char *tag)
{
    u16 pal[16];
    for (u16 line = 0; line < 4; line++) {
        PAL_getColors(line * 16, pal, 16);
        kprintf("TDUMP %s PAL%u: %04X %04X %04X %04X %04X %04X %04X %04X "
                "%04X %04X %04X %04X %04X %04X %04X %04X\n", tag, line,
                pal[0], pal[1], pal[2], pal[3], pal[4], pal[5], pal[6],
                pal[7], pal[8], pal[9], pal[10], pal[11], pal[12], pal[13],
                pal[14], pal[15]);
    }
}

#endif

