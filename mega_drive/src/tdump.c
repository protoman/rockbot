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


#ifdef MD_TILE_DUMP
// Tile/plane readback, display OFF. The first attempt (hand-rolled control
// writes) left the VDP address register at 0x10003/CD=0xC0 and BlastEm froze
// the CPU; SGDK's own macro does the 32-bit control-port write correctly.
static void vram_read_words(u16 byte_addr, u16 *dst, u16 words)
{
    *((vu32 *)VDP_CTRL_PORT) = VDP_READ_VRAM_ADDR((u32)byte_addr);
    for (u16 i = 0; i < words; i++) dst[i] = *((vu16 *)VDP_DATA_PORT);
}

void tdump_tile(const char *tag, u16 tile)
{
    u16 w[16];
    u16 was = VDP_isEnable();
    VDP_setEnable(FALSE);
    SYS_disableInts();
    VDP_setAutoInc(2);
    vram_read_words(tile * 32, w, 16);
    SYS_enableInts();
    VDP_setEnable(was);
    kprintf("TDUMP %s tile %u: %04X %04X %04X %04X %04X %04X %04X %04X "
            "%04X %04X %04X %04X %04X %04X %04X %04X\n", tag, tile,
            w[0], w[1], w[2], w[3], w[4], w[5], w[6], w[7],
            w[8], w[9], w[10], w[11], w[12], w[13], w[14], w[15]);
}

void tdump_cell(const char *tag, u16 plane, u16 col, u16 row)
{
    u16 w = 0;
    u16 was = VDP_isEnable();
    VDP_setEnable(FALSE);
    SYS_disableInts();
    VDP_setAutoInc(2);
    vram_read_words((u16)(plane + (row * 64 + col) * 2), &w, 1);
    SYS_enableInts();
    VDP_setEnable(was);
    kprintf("TDUMP %s cell(%u,%u) = %04X tile=%u pal=%u prio=%u\n",
            tag, col, row, w, w & 0x7FF, (w >> 13) & 3, (w >> 15) & 1);
}
#endif
